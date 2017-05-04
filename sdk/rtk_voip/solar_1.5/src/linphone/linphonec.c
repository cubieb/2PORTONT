/*
linphonec
Copyright (C) 2002  Florian Winterstein (flox@gmx.net)
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

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <asm/types.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include "eXosip.h"
#include "eXosip2.h"
#include "linphonecore.h"
#include "sdphandler.h"
#include "ortp.h"
#include "stun_api.h"
#ifdef RTK_DNS
#include "dns_api.h"
#endif
#include "netlink.h"
#include "rtnetlink.h"
#ifdef CONFIG_RTK_VOIP_IVR
#if 1
#include "ivripc.h"
#include "ivrclient.h"
#else
#include "ivrhandler.h"
#endif
#endif

#ifdef CONFIG_RTK_VOIP_DIALPLAN
#include "dialplanhelp.h"
#endif

#ifdef CONFIG_RTK_VOIP_IP_PHONE
#include "uihandler.h"
#endif

#ifdef CONFIG_APP_TR104
#include <sys/un.h>
#include "cwmpevt.h"
#endif /*CONFIG_APP_TR104*/

#include "alarm.h"
#include "pstnroute.h"
#include "icmp.h"

#ifdef CONFIG_RTK_VOIP_SIP_TLS
#include	<netinet/tcp.h>
#include "eXosip_tls.h"
extern tlsObject_t sipTLSObj[MAX_VOIP_PORTS][MAX_PROXY];
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

#ifdef CONFIG_RTK_VOIP_DECT_SITEL_SUPPORT
#include "si_dect_api.h"
#endif

// ==============================
// DEFINE & MACRO
// ==============================
#define CONFIG_NAME     "/etc/solar.conf"
#define FIFO_CONTROL	"/var/run/solar_control.fifo"
#define FIFO_WATCHDOG	"/var/run/solar_watchdog.fifo"
/*+++added by Jack for communicate with cwmpclient+++*/
#ifdef CONFIG_APP_TR104
#define SOLAR_CHANNEL_PATH "/var/solar.chanl"
#define CWMP_CLINET_PATH "/var/cwmpclient.chanl"
#endif
/*---end---*/

#define USE_CONSOLE		0	/* if 1 , the exec(solar) in board.c MUST be disable */

// ==============================
// function prototype
// ==============================
extern gboolean linphonec_parse_command_line(LinphoneCore *lc, uint32 ssid, const gchar *cl);
void linphone_core_process_event(TAPP *pApp, eXosip_event_t *ev);
static void init_g_state( TAPP *pApp );

// ==============================
// global variable 
// ============================== 
TAPP solar;
TAPP *solar_ptr = &solar;
int g_MaxVoIPPorts = 0;
int g_nMaxCodec = 0;
voipCfgParam_t *g_pVoIPCfg = NULL;
voip_flash_share_t *g_pVoIPShare = NULL;
static char *pidfile = "/var/run/solar.pid";
int g_mapSupportedCodec[_CODEC_MAX] = {_CODEC_MAX};

SYSTEMSTSTE g_sys_state[MAX_VOIP_PORTS];
SESSIONSTATE c_session_state[MAX_VOIP_PORTS][MAX_SS];
gboolean c_active_session[MAX_VOIP_PORTS][MAX_SS];

SystemParam_t g_SystemParam;

// ==============================
// static variable 
// ============================== 
static int fax_counter[MAX_VOIP_PORTS] = {0};
/*+++added by Jack for communicate with cwmpclient+++*/
#ifdef CONFIG_APP_TR104
static int ipcSocket = 0;
static cwmpEvtMsg gCwmpEvtMsg;
#endif
/*---end---*/

void SetSessionState( guint32 chid , guint32 ssid , SESSIONSTATE value )
{
	c_session_state[chid][ssid] = value ;
}

SESSIONSTATE GetSessionState( guint32 chid , guint32 ssid )
{
	return c_session_state[chid][ssid] ;
}

int GetIdleSession( guint32 chid )
{
	int i = 0 ;
	for( i = 0 ; i < MAX_SS ; i++ )
		if( c_session_state[chid][i] == SS_STATE_IDLE)
			break ;
	return ( i >= MAX_SS ? -1 : i ) ;
}

void SetActiveSession( guint32 chid , guint32 ssid , int value )
{
	int s = ( ssid == 0 ) ? 1 : 0 ;
	if( value == TRUE )
	{
		rtk_SetTranSessionID( chid , ssid ) ;
		c_active_session[chid][ssid] = TRUE ;
		c_active_session[chid][s] = FALSE ;
	}
	else if (value == FALSE)
	{
		rtk_SetTranSessionID( chid , s ) ;
		c_active_session[chid][ssid] = FALSE ;
	}
	else 
	{
		// on hook
		rtk_SetTranSessionID(chid, 255);//set 255 to clean resource.
		c_active_session[chid][0] = FALSE;
		c_active_session[chid][1] = FALSE;
	}
}

int GetActiveSession( guint32 chid )
{
	int i = 0 ;
	for( i = 0 ; i < MAX_SS ; i++ )
		if( c_active_session[chid][i] == TRUE )
			break ;
	return ( i >= MAX_SS ? -1 : i ) ;
}

void SetSysState( guint32 chid , SYSTEMSTSTE state )
{
	g_sys_state[chid] = state ;
	
	// update LED display 
	UpdateLedDisplayMode( &solar.ports[ chid ] );	// not good style 
}

SYSTEMSTSTE GetSysState( guint32 chid )
{
	return g_sys_state[chid] ;
}

void UpdateLedDisplayMode( LinphoneCore * lc )
{
	// depends on SysState and proxy register 
	const SYSTEMSTSTE sys_state = GetSysState( lc ->chid );
	int ringing = 0;
	int offhook = 0;
	
	switch( sys_state ) {
	case SYS_STATE_IDLE:
		break;
		
#ifdef CONFIG_RTK_VOIP_IVR
	case SYS_STATE_AUTH:
	case SYS_STATE_AUTH_ERROR:
		offhook = 1;
		break;
#endif
#ifdef BYPASS_RELAY
	case SYS_STATE_AUTOBYPASS_WARNING:
		offhook = 1;
		break;
#endif
	case SYS_STATE_EDIT:
		offhook = 1;
		break;
		
	case SYS_STATE_EDIT_ERROR:
		break;
		
	case SYS_STATE_RING:
		ringing = 1;
		break;
		
	case SYS_STATE_CONNECT:
		offhook = 1;
		break;
		
	case SYS_STATE_CONNECT_DAA_RING:
		ringing = 1;
		break;
		
	case SYS_STATE_CONNECT_EDIT:
		offhook = 1;
		break;
		
	case SYS_STATE_CONNECT_EDIT_ERROR:
		break;
		
	case SYS_STATE_DAA_RING:
		ringing = 1;
		break;
		
	case SYS_STATE_DAA_CONNECT:
	case SYS_STATE_TRANSFER_EDIT:
		offhook = 1;
		break;
		
	case SYS_STATE_TRANSFER_EDIT_ERROR:
		break;
		
	case SYS_STATE_IN2CALLS_OUT:
	case SYS_STATE_IN2CALLS_IN:
	case SYS_STATE_IN3WAY:
	case SYS_STATE_TRANSFERRING:
		offhook = 1;
		break;
		
	case SYS_STATE_VOICE_IVR:
	case SYS_STATE_VOICE_IVR_DONE:
		offhook = 1;
		break;
		
#ifdef ATTENDED_TRANSFER
	case SYS_STATE_TRANSFER_CONNECT:		// 2 call here, wait attended transfer
		offhook = 1;
		break;
		
	case SYS_STATE_TRANSFER_ONHOOK:
		break;
#endif
/*+++added by Jack Chan for Planet transfer+++*/
#ifdef PLANET_TRANSFER
	case SYS_STATE_TRANSFER_BLIND:
	case SYS_STATE_TRANSFER_ATTENDED:
#endif /*PLANET_TRANSFER*/	
		break;
	}
	
	if( offhook || ringing ) {
		rtk_Set_LED_Display( lc ->chid, 0, LED_BLINKING );
	} else {
		if( lc ->default_proxy )	// proxy registered 
			rtk_Set_LED_Display( lc ->chid, 0, LED_ON );
		else
			rtk_Set_LED_Display( lc ->chid, 0, LED_OFF );
	}
}

void
stub (void)
{
}

void
linphonec_display_something (LinphoneCore * lc, char *something)
{
	g_message(something);
}

void
linphonec_display_warning (LinphoneCore * lc, char *something)
{
	g_warning(something);
}

void
linphonec_display_url (LinphoneCore * lc, char *something, char *url)
{
	g_message("%s: %s\n", something, url);
}

#if 0
static LinphoneAuthInfo *pending_auth[MAX_VOIP_PORTS] = {0};

void linphonec_prompt_for_auth(LinphoneCore *lc, const char *realm, const char *username)
{
	pending_auth[lc->chid] = linphone_auth_info_new(username,lc->sip_conf.login_id,NULL,NULL,realm);
}

void linphonec_prompt_for_auth_final(LinphoneCore *lc){
	linphone_auth_info_set_passwd(pending_auth[lc->chid], lc->sip_conf.password);
	linphone_core_add_auth_info(lc, pending_auth[lc->chid]);
	pending_auth[lc->chid] = NULL;
}
#endif

void
print_usage (void)
{
	fprintf (stdout, "\n\
usage: linphonec [-c file] [-d level -l logfile]\n\
       linphonec -v\n\
\n\
  -c  file             specify path of configuration file.\n\
  -d  level            be verbose. 0 is no output. 6 is all output\n\
  -l  logfile          specify the log file for your SIP phone\n\
  -v or --version      display version and exits.\n");
}

void digitcode_init(LinphoneCore *lc)
{
	lc->callingstate = 0;
	lc->digit_index = 0;
	lc->dial_code[0] = 0;
#ifdef CONFIG_RTK_VOIP_IVR
	lc->dial_initial_hash = 0;
#endif
#ifdef CONFIG_RTK_VOIP_DIALPLAN
	lc->dial_data_index = 0;
	lc->dial_data[0] = 0;
#endif
}

void CallInvite(LinphoneCore *lc, guint32 ssid, char *src_ptr)
{
	SetSessionState(lc->chid, ssid, SS_STATE_CALLOUT);
	strcpy(src_ptr, "call ");
	strcat(src_ptr, lc->dial_code);
	digitcode_init(lc);
}

int32 GetHookStatus( uint32 chid, uint32 *pHookStatus )
{
	TstVoipCfg stVoipCfg;
	int ret;

 #ifdef CONFIG_RTK_VOIP_IP_PHONE
	if( ChidIsOwnedByIpPhone( chid ) )
		return rtk_GetIPPhoneHookStatus( pHookStatus );
 #endif

	stVoipCfg.ch_id = chid;
	ret = rtk_Set_GetPhoneStat(&stVoipCfg);
	if (ret != 0)
	{
		g_warning("GetHookStatus faile\n");
		return ret;
	}

	g_message("GetHookStatus: %ld\n", stVoipCfg.cfg);
	*pHookStatus = stVoipCfg.cfg;
	return 0;
}

#ifdef CHECK_RESOURCE

void linphone_no_resource_handling(LinphoneCore *lc, int ssid)
{
	//if (lc->chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
	if( RTK_VOIP_IS_DAA_CH( lc->chid, g_VoIP_Feature ) )
	{
		// do nothing if no resource in FXO
		return;
	}

#ifdef CONFIG_RTK_VOIP_IVR
	char text[]={IVR_TEXT_ID_NO_RESOURCE, '\0'};

	rtk_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
	usleep(500000); // Audio Code Issue: after phone off-hook, wait for a second,and then play IVR.

#if 0
	//if (lc->chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
	if( RTK_VOIP_IS_DAA_CH( lc->chid, g_VoIP_Feature ) )
		rtk_FXO_offhook(lc->chid);
#endif

	rtk_IvrStartPlaying(lc->chid, IVR_DIR_LOCAL, text);
	lc->bPlayIVR = 1;
#else

#if 0
	//if (lc->chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
	if( RTK_VOIP_IS_DAA_CH( lc->chid, g_VoIP_Feature ) )
		rtk_FXO_offhook(lc->chid);
#endif

	rtk_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
#endif
}

#endif

#ifdef BYPASS_RELAY
void rtk_auto_bypass_relay(LinphoneCore *lc, char *src_ptr)
{
	//if (g_MaxVoIPPorts > RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
	if( RTK_VOIP_DAA_NUM( g_VoIP_Feature ) > 0 )
	{
		char sz_fxo_0[40];

		sprintf(sz_fxo_0, "sip:127.0.0.1:%d",
			g_pVoIPCfg->ports[RTK_VOIP_DAA_CH_OFFSET(g_VoIP_Feature) /*RTK_VOIP_SLIC_NUM(g_VoIP_Feature)*/].sip_port);
		strcpy(lc->dial_code, sz_fxo_0);
		CallInvite(lc, 0, src_ptr);
	}
	else if (rtk_DAA_off_hook(lc->chid) == 0xff)
	{
		SetSysState(lc->chid , SYS_STATE_EDIT_ERROR);
		rtk_SetPlayTone(lc->chid, 0, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
	}
	else
	{
		SetSysState(lc->chid, SYS_STATE_DAA_CONNECT);
		SetSessionState(lc->chid, 0, SS_STATE_CALLER);
	}
}
#endif

void HookAction(LinphoneCore *lc, SIGNSTATE s_sign, char *src_ptr)
{
	uint32 chid;
	SYSTEMSTSTE state;
	int i = 0;
	uint32 hookStatus;
	int bRealOnHook;
#ifdef BYPASS_RELAY
	int bypass_relay;
#endif

	chid = lc->chid;
	state = GetSysState(chid);
	switch( s_sign )
	{
		case SIGN_OFFHOOK :
			{
				static int count = 0;
				g_warning("SIGN_OFFHOOK %d\n", count++);
			}

		#ifdef CHECK_RESOURCE
			if (rtk_VoIP_resource_check(chid, 0) == VOIP_RESOURCE_UNAVAILABLE)
			{
				linphone_no_resource_handling(lc, 0);
				break;
			}
		#endif

#ifdef ATTENDED_TRANSFER
			if (state == SYS_STATE_TRANSFER_ONHOOK)
			{
				for( i = 0 ; i < MAX_SS ; i++ )
					linphone_core_terminate_dialog(lc,  i ,NULL);

				SetSysState(chid, SYS_STATE_IDLE);
				state = SYS_STATE_IDLE;
			}
#endif

			if( state == SYS_STATE_IDLE )
			{
				SetActiveSession(chid, 0, TRUE);
				SetSysState(chid, SYS_STATE_EDIT);

				// fxo caller id detection
				//if (chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
				if( RTK_VOIP_IS_DAA_CH( chid, g_VoIP_Feature ) )
				{
					char caller_id[21];
					char caller_date[9];
					char caller_id_name[51];

					rtk_Get_DAA_CallerID(chid, caller_id, caller_date, caller_id_name);

					if (caller_id[0])
						strcpy(lc->caller_id, caller_id);
					else
						strcpy(lc->caller_id, "-");

					if (caller_id_name[0])
						strcpy(lc->caller_id_name, caller_id_name);
				}

#ifdef BYPASS_RELAY
				// check bypass relay
				// TODO: check network failed (by ping DNS server!?)
				bypass_relay = 0;

  #if 0				
				printf( "=========================\n" );
				printf( "RTK_VOIP_SLIC_NUM(g_VoIP_Feature):%d\n", RTK_VOIP_SLIC_NUM(g_VoIP_Feature) );
				printf( "g_MaxVoIPPorts:%d\n", g_MaxVoIPPorts );
				printf( "g_pVoIPCfg->auto_bypass_relay:%d\n", g_pVoIPCfg->auto_bypass_relay );
				printf( "lc->default_proxy:%p\n", lc->default_proxy );
				printf( "lc->default_proxy->timeout_count:%d\n", ( lc->default_proxy ? lc->default_proxy->timeout_count : -1 ) );
				printf( "=========================\n" );				
  #endif

				//printf( "lc ->parent ->port_link_status:%lX\n", lc ->parent ->port_link_status );
				
				//if (chid < RTK_VOIP_SLIC_NUM(g_VoIP_Feature) &&
				if( ( RTK_VOIP_IS_SLIC_CH( chid, g_VoIP_Feature ) ||
					  RTK_VOIP_IS_DECT_CH( chid, g_VoIP_Feature ) ) &&
					g_pVoIPCfg->auto_bypass_relay )
				{				
					// 1. Check port link status 
					if( ( lc ->parent ->port_link_status & 
						( PORT_LINK_LAN_ALL | PORT_LINK_WAN ) ) == 0 ) 
					{
						goto label_do_bypass_relay;
					}
				
					// 2. Check default proxy 
					// 2.1.1. no default proxy 
					// 2.1.2. default proxy is registering (not registered) 
					// 2.3. Web enable at least one proxy 
					if( ( lc->default_proxy == NULL ) ||
						( !lc->default_proxy->registered ) )
					{
					} else
						goto label_not_bypass_relay;

					for (i=0; i<MAX_PROXY; i++)
 						if (g_pVoIPCfg->ports[lc->chid].proxies[i].enable)
							break;

					if (i == MAX_PROXY)
						goto label_not_bypass_relay;
						
					// Check done 
					
label_do_bypass_relay:
					bypass_relay = 1;
					
label_not_bypass_relay:
					;
				}

				if (bypass_relay)
				{
					if (g_pVoIPCfg->auto_bypass_relay & AUTO_BYPASS_WARNING_ENABLE)
					{
						SetSysState(lc->chid, SYS_STATE_AUTOBYPASS_WARNING);
						rtk_SetPlayTone(lc->chid, 0, DSPCODEC_TONE_DOUBLE_RING, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
					}
					else
					{
						rtk_auto_bypass_relay(lc, src_ptr);
					}
					break;
			    }
#endif

				if ((g_pVoIPCfg->ports[lc->chid].hotline_enable) && 
					(g_pVoIPCfg->ports[lc->chid].hotline_number[0]))
				{
					rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_RINGING, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
					lc->ringstream[0] = TRUE;
					HotlineCallInviteByDialPlan(lc, 0, src_ptr, 
						g_pVoIPCfg->ports[lc->chid].hotline_number);
				}
				//else if (chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
				else if( RTK_VOIP_IS_DAA_CH( chid, g_VoIP_Feature ) )
				{
					if (g_pVoIPCfg->ports[lc->chid].hotline_enable)
					{
						// user dial mode if hotline enable but no number
					#ifdef CONFIG_RTK_VOIP_IVR
						rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
						usleep(500000); // Audio Code Issue: after phone off-hook, wait for a second,and then play IVR.
						rtk_FXO_offhook(chid);
						if (g_pVoIPCfg->ports[lc->chid].offhook_passwd[0])
						{
							char text[] = {IVR_TEXT_ID_PLEASE_ENTER_PASSWORD, '\0'};
							SetSysState(chid, SYS_STATE_AUTH);
							rtk_IvrStartPlaying(chid, IVR_DIR_LOCAL, text);
						}
						else
						{
							char text[] = {IVR_TEXT_ID_PLZ_ENTER_NUMBER, '\0'};
							rtk_IvrStartPlaying(chid, IVR_DIR_LOCAL, text);
						}
						lc->bPlayIVR = 1;
					#else
						rtk_FXO_offhook(chid);
						rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
					#endif
					}
					else // auto dial to FXS_0 if no hotline
					{
						char sz_fxs_0[40];

						sprintf(sz_fxs_0, "sip:fxo%u_to_fxs@127.0.0.1:%u",
							chid - RTK_VOIP_DAA_CH_OFFSET(g_VoIP_Feature) /*RTK_VOIP_SLIC_NUM(g_VoIP_Feature)*/, g_pVoIPCfg->ports[0].sip_port);
						strcpy(lc->dial_code, sz_fxs_0);
						CallInvite(lc, 0, src_ptr);
					}
				}
#ifdef CONFIG_RTK_VOIP_IVR
				else if (g_pVoIPCfg->ports[lc->chid].offhook_passwd[0])
				{
					char text[] = {IVR_TEXT_ID_PLEASE_ENTER_PASSWORD, '\0'};

					SetActiveSession(chid, 0, TRUE);
					SetSysState(chid, SYS_STATE_AUTH);
					rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
					usleep(500000); // Audio Code Issue: after phone off-hook, wait for a second,and then play IVR.
					rtk_IvrStartPlaying(chid, IVR_DIR_LOCAL, text);
					lc->bPlayIVR = 1;
				}
#endif
				else
				{
					if( g_SystemParam.stutter_tone &&
						( ( lc->default_proxy == NULL ) ||
						  ( !lc->default_proxy->registered ) ) )
					{
						rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_STUTTERDIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
					} else {
						rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
					}
				}
			}
			else if ( state == SYS_STATE_DAA_RING )
			{
				SetActiveSession(chid, 0, TRUE);
				if (rtk_DAA_off_hook(lc->chid) == 0xff)
				{
					/* PSTN line not connect or line busy */
					SetSysState(lc->chid, SYS_STATE_EDIT_ERROR);
				    rtk_SetPlayTone(lc->chid, 0, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
				}
				else
				{
					SetSysState(chid, SYS_STATE_DAA_CONNECT);
					SetSessionState(chid, 0, SS_STATE_CALLEE);
				}
			}
			else if( state == SYS_STATE_RING )
			{
				strcpy( src_ptr , "answer" ) ;
			}
			break ;
		case SIGN_ONHOOK :
			// check real hook status for web apply issue
			if( GetHookStatus( chid, &hookStatus ) == 0 &&
				hookStatus == 1 )
			{
				/* off-hook */
				bRealOnHook = FALSE;
			} 
			else 
			{
				/* on-hook */
				bRealOnHook = TRUE;
			}
			
			if (state == SYS_STATE_DAA_CONNECT)
			{
				rtk_DAA_on_hook(chid);
			}
#ifdef ATTENDED_TRANSFER
			else if (state == SYS_STATE_TRANSFER_CONNECT)
			{
				char refer_to[256];
				
				if (atdfr_construct_referto(lc, refer_to) == 0)
					linphone_core_transfer_call(lc, 0, refer_to);

				//create new TRANSFER state
				SetSysState(lc->chid, SYS_STATE_TRANSFER_ONHOOK);
			}
#endif
			else if (state == SYS_STATE_IN3WAY)
			{
				// TODO: attender transfer
				TstVoipMgr3WayCfg stVoipMgr3WayCfg;

				memset(&stVoipMgr3WayCfg, 0, sizeof(stVoipMgr3WayCfg));
				stVoipMgr3WayCfg.ch_id = chid;
				stVoipMgr3WayCfg.enable = FALSE;
				rtk_SetConference(&stVoipMgr3WayCfg);
#if 0
#ifdef ATTENDED_TRANSFER
				//in 3 way conference call case,
				//when one wanna leave, it doesn't means the peers are leaving
				//so add attended transfer here also.
				SetSysState( chid , SYS_STATE_CONNECT) ;
				//----- Hold the 2nd call...-----
				if (session_2_session_event(lc, 2) == 0)
				{
					char refer_to[256];
					//prepare the refer header with replpaces 
					//And then start attended trasfer
					if (atdfr_construct_referto(lc, refer_to) == 0)
						linphone_core_attended_transfer_call(lc, 0, refer_to);
					//create new TRANSFER state
					SetSysState( chid , SYS_STATE_ATT_TRANSFR_ONHOOK) ;
				}
#endif
#endif
			}
#ifdef CONFIG_RTK_VOIP_IP_PHONE
#else
			/*++added by Jack Chan  for transfer++*/
			else if (state == SYS_STATE_TRANSFER_EDIT)
			{
				// TODO: blind transfer
				int active_session;
				int ssid;
#if 0
				osip_to_t *to=NULL;
				osip_uri_t *uri=NULL;
				char *dialCode=NULL;
				char sipUri[100];
#endif
				
				active_session = GetActiveSession(lc->chid);
				if (active_session == -1)
					active_session = 0;
				if(SS_STATE_CALLOUT == GetSessionState(lc->chid, active_session)){
					ssid = (active_session == 0) ? 1 : 0;
					SetActiveSession(lc->chid, ssid, TRUE);
#if 0
					osip_to_init(&to);
					if( -1 != osip_to_parse(to, lc->call[active_session]->log->to)){
						uri = osip_to_get_url(to);
						if( NULL == osip_uri_get_username(uri)){
							sprintf(sipUri,"%s:%s",osip_uri_get_scheme(uri), osip_uri_get_host(uri));
							dialCode = sipUri;
						}
						else{ 
							dialCode = osip_uri_get_username(uri);
						}
					}
					if( NULL != dialCode )
						linphone_core_transfer_call(lc, ssid, dialCode);
					osip_to_free(to);
#else
					linphone_core_transfer_call(lc, ssid, lc->call[active_session]->log->to);
#endif

#ifdef ATTENDED_TRANSFER
					// Cancel Invite
					linphone_core_terminate_dialog(lc, active_session, NULL); 
					//create new TRANSFER state
					SetSysState(lc->chid, SYS_STATE_TRANSFER_ONHOOK);
#endif
				}
				/*--end--*/
			}
#endif
#ifdef CONFIG_RTK_VOIP_IVR
			else if( state == SYS_STATE_VOICE_IVR ) {
				rtk_IvrStopPlaying( chid );
			}
			
			if (lc->bPlayIVR)
			{
				lc->bPlayIVR = 0;
				rtk_IvrStopPlaying(chid);
			}
#endif

			for (i = 0; i < MAX_SS; i++)
			{
				rtk_SetRtpSessionState(chid, i, rtp_session_inactive);	// reset all rtp
#ifdef ATTENDED_TRANSFER
				if (GetSysState(chid) != SYS_STATE_TRANSFER_ONHOOK)
#endif
				linphone_core_terminate_dialog(lc, i, NULL);	// reset all sip
				if (bRealOnHook) // reset all tone
					rtk_SetPlayTone(chid, i, DSPCODEC_TONE_HOLD, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
			}

			if (bRealOnHook) 
			{
#ifdef ATTENDED_TRANSFER	
				// keep the state until NEW CALL
				if (GetSysState(chid)!=SYS_STATE_TRANSFER_ONHOOK)
#endif
				SetSysState(chid, SYS_STATE_IDLE);
			}
			else
			{
				SetActiveSession(chid, 0, 1);	
				rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
				SetSysState(chid, SYS_STATE_EDIT_ERROR);
			}

			// reset session resource
			SetActiveSession(chid, -1, -1);	

			// reset variable
			digitcode_init(lc);
			fax_counter[chid] = 0;
			lc->caller_id[0] = 0;
			break;
		default:
			// never occur
			g_error("invalid input in HookAction\n");
			break;
	}
}

int GetRtpCfg(TstVoipMgrRtpCfg *rtp_cfg, LinphoneCore *lc, int ssid)
{
	StreamParams *audio_params;
	char *local_ip = NULL;
	unsigned short src_port, dst_port;
	unsigned long src_ip, dst_ip;
#ifdef SUPPORT_CODEC_DESCRIPTOR
	PayloadType *pt;
	unsigned int idxCodec = _CODEC_MAX;
	const codec_mine_desc_t *pCodecMineDesc;
#endif

	audio_params = &lc->call[ssid]->audio_params;

	eXosip_get_localip_for(audio_params->remoteaddr, &local_ip);
	src_ip = inet_addr(local_ip);
	g_free(local_ip);

	dst_ip = inet_addr(audio_params->remoteaddr);
	dst_port = (unsigned short) audio_params->remoteport;
	src_port = (unsigned short) audio_params->localport;

	rtp_cfg->ch_id = lc->chid;
	rtp_cfg->m_id = ssid;
	rtp_cfg->ip_src_addr = dst_ip;
	rtp_cfg->ip_dst_addr = src_ip;
	rtp_cfg->udp_src_port = dst_port;
	rtp_cfg->udp_dst_port = src_port;
#ifdef SUPPORT_VOICE_QOS
	rtp_cfg->tos = lc->rtp_conf.tos;
#endif				

#ifdef DYNAMIC_PAYLOAD
	rtp_cfg->rfc2833_payload_type_local = audio_params->rfc2833_local_pt;
	rtp_cfg->rfc2833_payload_type_remote = audio_params->rfc2833_remote_pt;
#else
	rtp_cfg->rfc2833_payload_type_local = 
		lc->sip_conf.rfc2833_payload_type;
	rtp_cfg->rfc2833_payload_type_remote = 
		linphone_core_get_remote_rfc2833_pt(lc->call[ssid]);
#endif // DYNAMIC_PAYLOAD

#ifdef DYNAMIC_PAYLOAD
	rtp_cfg->local_pt = audio_params->local_pt;
	rtp_cfg->remote_pt = audio_params->remote_pt;
	rtp_cfg->uPktFormat = audio_params->static_pt;
#endif

#if defined( DYNAMIC_PAYLOAD ) && defined( SUPPORT_V152_VBD )
	rtp_cfg->local_pt_vbd = audio_params->local_pt_vbd;
	rtp_cfg->remote_pt_vbd = audio_params->remote_pt_vbd;
	rtp_cfg->uPktFormat_vbd = audio_params->static_pt_vbd;	
#endif

#ifdef SUPPORT_CODEC_DESCRIPTOR
	pt = rtp_profile_get_payload(lc->local_profile, audio_params->pt);
	if (pt == NULL)	return -1;
	pCodecMineDesc = GetCodecMineDesc(pt->mime_type);
	if (pCodecMineDesc) 
	{
		rtp_cfg->uPktFormat = pCodecMineDesc->payloadType;
		idxCodec = pCodecMineDesc->idxCodec;
	} 
	else 
	{
		printf( "ERROR: Unexpected MIME type.\n" );
		rtp_cfg->uPktFormat = rtpPayloadPCMU;
		idxCodec = _CODEC_G711U;
	}
#endif // DYNAMIC_PAYLOAD

	rtp_cfg->nG723Type = lc->rtp_conf.g7231_rate;

#ifndef SUPPORT_CUSTOMIZE_FRAME_SIZE
	/* Frame per packet by hard coding */
	if (rtp_cfg->uPktFormat == rtpPayloadG723)
		rtp_cfg->nFramePerPacket = 1;
	else
		rtp_cfg->nFramePerPacket = 2;
#else
	/* Frame per packet by flash */
	if (idxCodec >= 0 && idxCodec < _CODEC_MAX &&
		g_mapSupportedCodec[idxCodec] >= 0 && g_mapSupportedCodec[idxCodec] < _CODEC_MAX) 
	{
		rtp_cfg->nFramePerPacket =  
				lc ->rtp_conf.pFrameSize[ g_mapSupportedCodec[idxCodec] ] + 1;
	} 
	else 
	{
		printf( "Unexpected codec in MINE-TYPE!\n" );
		rtp_cfg->nFramePerPacket = 1;
	}
#endif

	rtp_cfg->bVAD = lc->rtp_conf.bVAD;
	rtp_cfg->bPLC = lc->rtp_conf.bPLC;
	rtp_cfg->nJitterDelay = lc->rtp_conf.nJitterDelay;
	rtp_cfg->nMaxDelay = lc->rtp_conf.nMaxDelay;
	rtp_cfg->nJitterFactor = lc->rtp_conf.nJitterFactor;

	return 0;
}

int session_2_session_event(LinphoneCore *lc, int s_case)
{
	uint32 chid;
	int active_session;

	chid = lc->chid;
	if (GetSysState(chid) == SYS_STATE_IN3WAY)
		return -1;

	active_session = GetActiveSession(chid);
	if (lc->call[active_session] &&
		(lc->call[active_session]->state == LCStateBeHold ||
		lc->call[active_session]->state == LCStateBeHoldACK))
	{
		// do nothing
		return -1;
	}

	if( s_case == 1 )
	{
		SESSIONSTATE ss = GetSessionState( chid , 0 ) ;

		// cancel pending call before switch session
		if (GetSessionState(chid, 1) == SS_STATE_CALLOUT)
		{
			linphone_core_terminate_dialog(lc, 1, NULL);
		}
		
		if (linphone_core_onhood_event(lc, 1) != 0)
		{
			// onhood failed, user need try again
			return -1;
		}
		else if( ss == SS_STATE_IDLE )
		{
			SetSysState( chid , SYS_STATE_CONNECT_EDIT ) ;
		}
		else if( ss == SS_STATE_CALLIN )
		{
			rtk_SetPlayTone( chid , 1, DSPCODEC_TONE_CALL_WAITING , FALSE , DSPCODEC_TONEDIRECTION_LOCAL ) ;
			linphone_core_accept_dialog(lc, 0, NULL);
		}
		else if (linphone_core_offhood_event(lc, 0) != 0)
		{
			// offhood failed, user need try again
			return -1;
		}
	}
	else if( s_case == 2 )
	{
		SESSIONSTATE ss = GetSessionState( chid , 1 ) ;

		// cancel pending call before switch session
		if (GetSessionState(chid, 0) == SS_STATE_CALLOUT)
		{
			linphone_core_terminate_dialog(lc, 0, NULL);
		}

		if (linphone_core_onhood_event(lc, 0) != 0)
		{
			// onhood failed, user need try again
			return -1;
		}
		else if( ss == SS_STATE_IDLE )
		{
			SetSysState( chid , SYS_STATE_CONNECT_EDIT ) ;
		}
		else if( ss == SS_STATE_CALLIN )
		{
			rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_CALL_WAITING, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
			linphone_core_accept_dialog(lc, 1, NULL);
		}
		else if (linphone_core_offhood_event(lc, 1) != 0)
		{
			// offhood failed, user need try again
			return -1;
		}
	}
	else if( s_case == 3 )
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

		if (VOIP_RESOURCE_UNAVAILABLE == 
			rtk_VoIP_resource_check(chid, stVoipMgr3WayCfg.rtp_cfg[active_session ? 0 : 1].uPktFormat))
		{
		#ifdef CHECK_RESOURCE
		#ifdef CONFIG_RTK_VOIP_IVR
			// play ivr only, don't play busy tone here
			linphone_no_resource_handling(lc, active_session);
		#endif
		#endif
			return -1;
		}

		for (i=0; i<MAX_SS; i++)
		{
			if (lc->call[i])
			{
				switch (lc->call[i]->state)
				{
				case LCStateInit:
					// do nothing in 3-way conference
					return 0;
				case LCStateHold:
				case LCStateHoldACK:
					// resume
					if (linphone_core_offhood_event(lc, i) != 0)
					{
						// offhood failed, user need try again
						return -1;
					}
					break;
				default:
					break;
				}
			}
			else
			{
				// do nothing in 3-way conference
				return -1;
			}
		}
	}
	return 0;
}

void edit_invite_digit(LinphoneCore *lc, SIGNSTATE s_sign)
{
	if (s_sign >= SIGN_KEY1 && s_sign <= SIGN_KEY9) {
		lc->dial_code[lc->digit_index] = (int) s_sign + 48;
#ifdef CONFIG_RTK_VOIP_DIALPLAN
		lc->dial_data[lc->dial_data_index] = (int) s_sign + 48;
#endif
	}
	else if (s_sign == SIGN_KEY0) {
		lc->dial_code[lc->digit_index] = '0';
#ifdef CONFIG_RTK_VOIP_DIALPLAN
		lc->dial_data[lc->dial_data_index] = '0';
#endif
	}
	else if( s_sign == SIGN_STAR ) {
		lc->dial_code[lc->digit_index] = '.';
#ifdef CONFIG_RTK_VOIP_DIALPLAN
		lc->dial_data[lc->dial_data_index] = '*';
#endif
	}
#ifdef CONFIG_RTK_VOIP_DIALPLAN
	else if( s_sign == SIGN_HASH ) {
		lc->dial_data[lc->dial_data_index] = '#';
		goto label_add_to_dial_data_only;
	}
#endif

	if (lc->digit_index < LINE_MAX_LEN - 1)
	{
		lc->digit_index++;
		lc->dial_code[lc->digit_index] = 0;
	}

#ifdef CONFIG_RTK_VOIP_DIALPLAN
label_add_to_dial_data_only:

	if( lc->dial_data_index < LINE_MAX_LEN - 1 ) {
		lc->dial_data_index++;
		lc->dial_data[lc->dial_data_index] = 0;
	}
#endif
	
	osip_gettimeofday(&lc->auto_dial_start, NULL);
}

#if 0
void print_invite_digit(LinphoneCore *lc, SIGNSTATE s_sign)
{
int i;

	printf("dial_data_index: %d.\n", lc->dial_data_index);
	printf("The input data: ");
	for (i=0; i<lc->dial_data_index; i++)
		printf("%x ", lc->dial_data[i]);
	printf("...END \n");

}
#endif

#ifdef CONFIG_RTK_VOIP_IVR
int IsInstructionForIVR_Indirect( LinphoneCore *lc )
{
	ivr_ipc_msg_t msg;
	int ret;
	
	msg.message_type		= IVR_IPC_MSG_DO_IVR_INS;
	
    msg.do_ivr_ins.chid					= lc ->chid;
    msg.do_ivr_ins.sid					= GetActiveSession( lc ->chid );
    msg.do_ivr_ins.dial_initial_hash	= lc ->dial_initial_hash;
    msg.do_ivr_ins.digit_index			= lc ->digit_index;
    strcpy( msg.do_ivr_ins.dial_code, lc ->dial_code );
	if (lc->default_proxy)
	    strcpy( msg.do_ivr_ins.login_id, g_pVoIPCfg->ports[lc->chid].proxies[lc->default_proxy->index].login_id );
	else
    	strcpy( msg.do_ivr_ins.login_id, g_pVoIPCfg->ports[lc->chid].proxies[lc->default_proxy_index].login_id );
	
	/* Do IVR instruction */
	ret = IsInstructionForIVR_IPC( &msg );
	
	switch( ret ) {
	case IVR_IPC_RET_BUSY_TONE:
		/* Play 'busy' tone. */
		rtk_SetPlayTone( lc ->chid, GetActiveSession( lc ->chid ), 
					DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
		SetSysState( lc ->chid, SYS_STATE_VOICE_IVR_DONE );
		break;
		
	case IVR_IPC_RET_VOICE_CFG:
		/* Play 'dial' tone, or voice IVR */
		SetSysState( lc ->chid, SYS_STATE_VOICE_IVR );
		break;
	}

	return ret;
}
#endif /* CONFIG_RTK_VOIP_IVR */

int CallDigitEdit(LinphoneCore *lc, uint32 *ssid, SIGNSTATE s_sign, char *src_ptr)
{
	int active_session;
	SYSTEMSTSTE state;
	int si;
	const unsigned char *pszPSTNRouting = NULL;

	active_session = GetActiveSession(lc->chid);
	if (active_session == -1)
		active_session = 0;
		
	*ssid = active_session;
	state = GetSysState(lc->chid);

	printf("\r\nCallDigitEdit,chid %d ssid %d,state %d, s_sign %d,src_ptr %s\n",lc->chid,active_session,state,s_sign,src_ptr);
	switch (state)
	{
#ifdef CONFIG_RTK_VOIP_IVR
	case SYS_STATE_AUTH:
		if (lc->bPlayIVR)
		{
			lc->bPlayIVR = 0;
			rtk_IvrStopPlaying(lc->chid);
		}

		if (s_sign == SIGN_FLASHHOOK)
		{
			// nothing
		}
#ifdef CONFIG_RTK_VOIP_DIALPLAN
		else if (s_sign == SIGN_HASH || s_sign == SIGN_AUTODIAL)
#else
		else if (s_sign == SIGN_HASH)
#endif
		{
#ifdef CONFIG_RTK_VOIP_DIALPLAN
			if (s_sign == SIGN_HASH && g_pVoIPCfg->auto_dial & AUTO_DIAL_ALWAYS)
			{
				/* allow '#' in middle of dial data. */
				edit_invite_digit(lc, s_sign); // add to auth numbers
			}
			else
#endif
			{
				int i;
				char *passwd;

#ifdef CONFIG_RTK_VOIP_DIALPLAN
				passwd = lc->dial_data;
#else
				passwd = lc->dial_code;
#endif
				// do auth
				for (i=0; passwd[i] && g_pVoIPCfg->ports[lc->chid].offhook_passwd[i]; i++)
#ifdef CONFIG_RTK_VOIP_DIALPLAN
						if (passwd[i] != g_pVoIPCfg->ports[lc->chid].offhook_passwd[i] &&
							! (passwd[i] == '*' && g_pVoIPCfg->ports[lc->chid].offhook_passwd[i] == '.'))
#else
						if (passwd[i] != g_pVoIPCfg->ports[lc->chid].offhook_passwd[i] &&
							! (passwd[i] == '.' && g_pVoIPCfg->ports[lc->chid].offhook_passwd[i] == '*'))
#endif
							break;

				if (passwd[i] == 0 && g_pVoIPCfg->ports[lc->chid].offhook_passwd[i] == 0)
				{
					char text[] = {IVR_TEXT_ID_PLZ_ENTER_NUMBER, '\0'};

					// auth success
					rtk_IvrStartPlaying(lc->chid, IVR_DIR_LOCAL, text);
					lc->bPlayIVR = 1;
//					rtk_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
					digitcode_init(lc);
					osip_gettimeofday(&lc->off_hook_start, NULL);
					SetSysState(lc->chid, SYS_STATE_EDIT);
				}
				else
				{
					// auth failed
					char text[] = {IVR_TEXT_ID_PLEASE_ENTER_PASSWORD, '\0'};

					rtk_IvrStartPlaying(lc->chid, IVR_DIR_LOCAL, text);
					lc->bPlayIVR = 1;
					digitcode_init(lc);
					osip_gettimeofday(&lc->off_hook_start, NULL);
				}
			}
		}
		else
		{
			edit_invite_digit(lc, s_sign);	// add to auth numbers
		}
		break;
#endif

	case SYS_STATE_EDIT:
		if (s_sign == SIGN_FLASHHOOK)
		{
			if (lc->call[*ssid])
				linphone_core_terminate_dialog(lc, *ssid, NULL);

			if( g_SystemParam.stutter_tone &&
				( ( lc->default_proxy == NULL ) ||
				  ( !lc->default_proxy->registered ) ) )
			{
				rtk_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_STUTTERDIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
			} else {
				rtk_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
			}
			digitcode_init(lc);
			osip_gettimeofday(&lc->off_hook_start, NULL);
		}
		else if (s_sign == SIGN_HASH)
		{
#if defined( CONFIG_RTK_VOIP_DIALPLAN )
			if (g_pVoIPCfg->auto_dial & AUTO_DIAL_ALWAYS)
				edit_invite_digit(lc, s_sign);
#endif
			if (lc->digit_index == 0) {
#if defined( CONFIG_RTK_VOIP_DIALPLAN )
				if ((g_pVoIPCfg->auto_dial & AUTO_DIAL_ALWAYS) == 0)
					edit_invite_digit(lc, s_sign);
#endif
#ifdef CONFIG_RTK_VOIP_IVR
				lc->dial_initial_hash ++;
#endif
				return 0;
			}

#if 0
#ifdef SIP_PING
			if (strncmp(lc->dial_code, ".2", 2) == 0)
			{
				int i;

				lc->dial_data_index -= 2;
				for (i=0; i<lc->dial_data_index; i++) 
					lc->dial_data[i] = lc->dial_data[i + 2];
				lc->dial_data[i] = 0;
				strcpy(src_ptr, "ping ");
				CallTestByDialPlan(lc, *ssid, &src_ptr[5]);
				rtk_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
				digitcode_init(lc);
				osip_gettimeofday(&lc->off_hook_start, NULL);
				return 0;
			}
#endif
#endif
		
#ifdef CONFIG_RTK_VOIP_IVR
			if (lc->bPlayIVR)
			{
				lc->bPlayIVR = 0;
				rtk_IvrStopPlaying(lc->chid);
			}

  #if 1
			if( !IsInstructionForIVR_Indirect( lc ) )
  #else
			if( !IsInstructionForIVR( lc ) )
  #endif
#endif
			{
#ifdef CONFIG_RTK_VOIP_DIALPLAN
				if (g_pVoIPCfg->auto_dial & AUTO_DIAL_ALWAYS)
				{
					/* allow '#' in middle of dial data. */
					/* turn off dial tone. */
					rtk_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
					if( DialPlanTryCallInvite( lc, *ssid, src_ptr ) ) {
						/* Check PSTN routing prefix */
						if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, src_ptr, 1 ) ) ) 
							goto label_fxo_call_on_SYS_STATE_EDIT;
					}
				}
				else
				{
					/* call invite immediately */
					CallInviteByDialPlan( lc, *ssid, src_ptr );
					
					/* Check PSTN routing prefix */
					if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, src_ptr, 1 ) ) ) 
						goto label_fxo_call_on_SYS_STATE_EDIT;
				}
#else
				CallInvite(lc, *ssid, src_ptr);
#endif /* CONFIG_RTK_VOIP_DIALPLAN */
			}
		}
#ifdef CONFIG_RTK_VOIP_DIALPLAN
		else if (s_sign == SIGN_AUTODIAL)
		{
			CallInviteByDialPlan( lc, *ssid, src_ptr );

			/* Check PSTN routing prefix */
			if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, src_ptr, 1 ) ) ) 
				goto label_fxo_call_on_SYS_STATE_EDIT;
		}
#endif /* CONFIG_RTK_VOIP_DIALPLAN */
		else
		{
			edit_invite_digit(lc, s_sign);
			
			/* turn off dial tone. */
			rtk_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);

#ifdef CONFIG_RTK_VOIP_DIALPLAN
			if( DialPlanTryCallInvite( lc, *ssid, src_ptr ) ) {
				/* dial plan do call invite */
				
				/* Check PSTN routing prefix */
				if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, src_ptr, 1 ) ) ) 
					goto label_fxo_call_on_SYS_STATE_EDIT;
			} else 
#endif
			{
			    if (strcmp(lc->dial_code, g_pVoIPCfg->funckey_pstn) == 0) 
				{
label_fxo_call_on_SYS_STATE_EDIT:
					//if (g_MaxVoIPPorts > RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
					if( RTK_VOIP_DAA_NUM( g_VoIP_Feature ) > 0 )
					{
						char sz_fxo_0[40];

						if( pszPSTNRouting && pszPSTNRouting[ 0 ] ) {
							sprintf(sz_fxo_0, "sip:%s@127.0.0.1:%d", 
								pszPSTNRouting,
								g_pVoIPCfg->ports[RTK_VOIP_DAA_CH_OFFSET(g_VoIP_Feature) /*RTK_VOIP_SLIC_NUM(g_VoIP_Feature)*/].sip_port);
						} else {
							sprintf(sz_fxo_0, "sip:127.0.0.1:%d", 
								g_pVoIPCfg->ports[RTK_VOIP_DAA_CH_OFFSET(g_VoIP_Feature) /*RTK_VOIP_SLIC_NUM(g_VoIP_Feature)*/].sip_port);
						}
						strcpy(lc->dial_code, sz_fxo_0);
						CallInvite(lc, *ssid, src_ptr);
					}
					else if (rtk_DAA_off_hook(lc->chid) == 0xff)
				    {
						SetSysState( lc->chid , SYS_STATE_EDIT_ERROR );
				    	rtk_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
				    }
				    else
					{
				    	SetSysState(lc->chid, SYS_STATE_DAA_CONNECT);
						SetSessionState(lc->chid, *ssid, SS_STATE_CALLER);
					}
			    }
			}
		}
		break;

	case SYS_STATE_EDIT_ERROR:
		if (s_sign == SIGN_FLASHHOOK)
		{
			rtk_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
			digitcode_init(lc);
			SetSysState(lc->chid, SYS_STATE_EDIT);
			osip_gettimeofday(&lc->off_hook_start, NULL);
		}
		break;

	case SYS_STATE_CONNECT:
		if (s_sign == SIGN_FLASHHOOK)
		{
			if (session_2_session_event(lc, (active_session == 0) ? 2 : 1) == 0)
			{
				*ssid = GetActiveSession(lc->chid);
			}
			digitcode_init(lc);
			osip_gettimeofday(&lc->off_hook_start, NULL);
		}
		else
		{
			edit_invite_digit(lc, s_sign);
			linphone_core_send_dtmf(lc, active_session, s_sign);
			if (strcmp(lc->dial_code, g_pVoIPCfg->funckey_transfer) == 0)
			{
				if (session_2_session_event(lc, (active_session == 0) ? 2 : 1) == 0)
				{
					*ssid = GetActiveSession(lc->chid);
					SetSysState(lc->chid, SYS_STATE_TRANSFER_EDIT);
				}
				digitcode_init(lc);
				osip_gettimeofday(&lc->off_hook_start, NULL);
			}
		}
		break;

	case SYS_STATE_CONNECT_DAA_RING:
		if (s_sign == SIGN_FLASHHOOK)
		{
			// close VoIP
			linphone_core_terminate_dialog(lc, active_session, NULL);
			// Accept PSTN
			*ssid = 0;
			SetActiveSession(lc->chid, *ssid, TRUE);
			if (rtk_DAA_off_hook(lc->chid) == 0xff)
			{
				/* PSTN line not connect or line busy */
				SetSysState(lc->chid, SYS_STATE_EDIT_ERROR);
			    rtk_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
			}
			else
			{
				SetSysState(lc->chid, SYS_STATE_DAA_CONNECT);
				SetSessionState(lc->chid, *ssid, SS_STATE_CALLER);
			}
#if 0
			// close DAA ring in another channel
			for (i=0; i<g_MaxVoIPPorts; i++)
			{
				if (i == lc->chid)
					continue;

				if (GetSysState(i) == SYS_STATE_CONNECT_DAA_RING)
				{
					SetSysState(i, SYS_STATE_CONNECT);
					active_session = GetActiveSession(i);
					if (lc->parent->ports[i].call[active_session]->state == LCStateAVRunning)
						rtk_SetPlayTone(i, active_session, DSPCODEC_TONE_CALL_WAITING, FALSE, 
							DSPCODEC_TONEDIRECTION_LOCAL);
					SetSessionState(i, active_session == 0 ? 1 : 0, SS_STATE_IDLE);
				}
			}
#endif			
		}
		else
		{
			linphone_core_send_dtmf(lc, active_session, s_sign);
		}
		break;

	case SYS_STATE_CONNECT_EDIT:
		if (s_sign == SIGN_FLASHHOOK)
		{
			if (session_2_session_event(lc, (active_session == 0) ? 2 : 1) == 0)
			{
				*ssid = GetActiveSession(lc->chid);
#ifdef CONFIG_RTK_VOIP_IP_PHONE
				S2U_ConnectionEstInd( lc ->chid, *ssid, 4 );
#endif
				SetSysState(lc->chid, SYS_STATE_CONNECT);
			}
			digitcode_init(lc);
		}
		else if (s_sign == SIGN_HASH)
		{
#if defined( CONFIG_RTK_VOIP_DIALPLAN )
			if (g_pVoIPCfg->auto_dial & AUTO_DIAL_ALWAYS)
				edit_invite_digit(lc, s_sign);
#endif
			
			if (lc->digit_index == 0) {
#if defined( CONFIG_RTK_VOIP_DIALPLAN )
				if ((g_pVoIPCfg->auto_dial & AUTO_DIAL_ALWAYS) == 0)
					edit_invite_digit(lc, s_sign);
#endif
				return 0;
			}
			
			if( ( si = GetIdleSession(lc->chid) ) == -1 ) {
				digitcode_init(lc);	
				return 0;
			} 

#ifdef CONFIG_RTK_VOIP_DIALPLAN
			if (g_pVoIPCfg->auto_dial & AUTO_DIAL_ALWAYS)
			{
				/* allow '#' in middle of dial data. */
				/* turn off dial tone. */
				rtk_SetPlayTone(lc->chid, si, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
				if( DialPlanTryCallInvite( lc, si, src_ptr ) ) {
				
					/* Check PSTN routing prefix */
					if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, src_ptr, 1 ) ) ) 
						goto label_fxo_call_on_SYS_STATE_CONNECT_EDIT;
					
					*ssid = si;
				}
			}
			else
			{
				/* call invite immediately */
				CallInviteByDialPlan( lc, si, src_ptr );

				/* Check PSTN routing prefix */
				if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, src_ptr, 1 ) ) ) 
					goto label_fxo_call_on_SYS_STATE_CONNECT_EDIT;

  				*ssid = si;
			}
#else
			*ssid = si;
			CallInvite(lc, si, src_ptr);
#endif /* CONFIG_RTK_VOIP_DIALPLAN */
		}
#ifdef CONFIG_RTK_VOIP_DIALPLAN
		else if (s_sign == SIGN_AUTODIAL) {
		
			if( ( si = GetIdleSession(lc->chid) ) == -1 ) {
				digitcode_init(lc);	
				return 0;
			} 
			
			CallInviteByDialPlan( lc, si, src_ptr );

			/* Check PSTN routing prefix */
			if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, src_ptr, 1 ) ) ) 
				goto label_fxo_call_on_SYS_STATE_CONNECT_EDIT;

			*ssid = si;
		}
#endif /* CONFIG_RTK_VOIP_DIALPLAN */
		else
		{
			edit_invite_digit(lc, s_sign);
			rtk_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_DIAL, FALSE, 
				DSPCODEC_TONEDIRECTION_LOCAL);
			
#ifdef CONFIG_RTK_VOIP_DIALPLAN
			if( ( si = GetIdleSession(lc->chid) ) == -1 ) {
				digitcode_init(lc);	
				return 0;
			} 
			
			if( DialPlanTryCallInvite( lc, si, src_ptr ) ) {
				/* Check PSTN routing prefix */
				if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, src_ptr, 1 ) ) ) 
					goto label_fxo_call_on_SYS_STATE_CONNECT_EDIT;
				
				*ssid = si;
			} else
#endif
		
		    if (strcmp(lc->dial_code, g_pVoIPCfg->funckey_pstn) == 0) 
			{
label_fxo_call_on_SYS_STATE_CONNECT_EDIT:
				//if (g_MaxVoIPPorts > RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
				if( RTK_VOIP_DAA_NUM( g_VoIP_Feature ) > 0 )
				{
					char sz_fxo_0[40];

					if( pszPSTNRouting && pszPSTNRouting[ 0 ] ) {
						sprintf(sz_fxo_0, "sip:%s@127.0.0.1:%d", 
							pszPSTNRouting,
							g_pVoIPCfg->ports[RTK_VOIP_DAA_CH_OFFSET(g_VoIP_Feature) /*RTK_VOIP_SLIC_NUM(g_VoIP_Feature)*/].sip_port);
					} else {
						sprintf(sz_fxo_0, "sip:127.0.0.1:%d", 
							g_pVoIPCfg->ports[RTK_VOIP_DAA_CH_OFFSET(g_VoIP_Feature) /*RTK_VOIP_SLIC_NUM(g_VoIP_Feature)*/].sip_port);
					}
					strcpy(lc->dial_code, sz_fxo_0);
					CallInvite(lc, *ssid, src_ptr);
				}
				else	
			    {
					SetSysState(lc->chid, SYS_STATE_CONNECT_EDIT_ERROR);
			    	rtk_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
			    }
			}
		}
		break;

	case SYS_STATE_CONNECT_EDIT_ERROR:
		if (s_sign == SIGN_FLASHHOOK)
		{
			if (session_2_session_event(lc, (active_session == 0) ? 2 : 1) == 0)
			{
				*ssid = GetActiveSession(lc->chid);
				SetSysState(lc->chid, SYS_STATE_CONNECT);
			}
			digitcode_init(lc);
		}
		break;

	case SYS_STATE_IN2CALLS_OUT:
		if (s_sign == SIGN_FLASHHOOK)
		{
			if (session_2_session_event(lc, 3) == 0)
			{
				SetSysState(lc->chid, SYS_STATE_IN3WAY);
			}
			digitcode_init(lc);
		}
		else
		{
			linphone_core_send_dtmf(lc, active_session, s_sign);
		}
		break;

	case SYS_STATE_IN2CALLS_IN:
		if (s_sign == SIGN_FLASHHOOK)
		{
			if (session_2_session_event(lc, (active_session == 0) ? 2 : 1) == 0)
			{
				*ssid = GetActiveSession(lc->chid);
			}
			digitcode_init(lc);
		}
		else
		{
			linphone_core_send_dtmf(lc, active_session, s_sign);
		}
		break;

	case SYS_STATE_IN3WAY:
		if (s_sign == SIGN_FLASHHOOK)
		{
			digitcode_init(lc);
		}
		else
		{
			linphone_core_send_dtmf(lc, 0, s_sign);
			linphone_core_send_dtmf(lc, 1, s_sign);
		}
		break;
		
	case SYS_STATE_TRANSFER_EDIT:
	case SYS_STATE_TRANSFER_ATTENDED: //pass through
		if (s_sign == SIGN_FLASHHOOK)
		{
			if (session_2_session_event(lc, (active_session == 0) ? 2 : 1) == 0)
			{
				*ssid = GetActiveSession(lc->chid);
				SetSysState(lc->chid, SYS_STATE_CONNECT);
			}
			digitcode_init(lc);
		}
		else if 
#ifdef CONFIG_RTK_VOIP_DIALPLAN
			(s_sign == SIGN_AUTODIAL ||
			(s_sign == SIGN_HASH &&  
			(g_pVoIPCfg->auto_dial & AUTO_DIAL_ALWAYS) == 0))
#else
			(s_sign == SIGN_HASH)
#endif
		{
			if (lc->digit_index == 0) 
				return 0;

			if( ( si = GetIdleSession(lc->chid) ) == -1 ) 
			{
				digitcode_init(lc);	
				return 0;
			} 
			
			if( g_SystemParam.conference_code[ 0 ] &&
				strcmp(lc->dial_code, g_SystemParam.conference_code) == 0)
			{
				SetSysState(lc->chid, SYS_STATE_CONNECT_EDIT);
				/* turn on dial tone. */
				rtk_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
				digitcode_init(lc);	
				osip_gettimeofday(&lc->off_hook_start, NULL);
				return 0;
			} 

/*+++added by Jack Chan for Planet transfer+++*/
#ifdef PLANET_TRANSFER
			if (eXosip.bIVR_transfer && strcmp(lc->dial_code, eXosip.blind_transfer_code) == 0)	//blind transfer
			{
				SetSysState(lc->chid, SYS_STATE_TRANSFER_BLIND);
				/* turn on dial tone. */
				rtk_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
				digitcode_init(lc);
				return 0;
				}
			else if (eXosip.bIVR_transfer && strcmp(lc->dial_code, eXosip.attended_transfer_code) == 0) //attended transfer
			{
				SetSysState(lc->chid, SYS_STATE_TRANSFER_ATTENDED);
				rtk_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
				digitcode_init(lc);
				return 0;
			}
#endif /*PLANET_TRANSFER*/
/*---end---*/
/*++added by Jack Chan  for transfer++*/
#ifdef CONFIG_RTK_VOIP_DIALPLAN
				/* call invite immediately */
				CallInviteByDialPlan( lc, si, src_ptr );

				/* Check PSTN routing prefix */
				if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, src_ptr, 1 ) ) ) 
					goto label_fxo_call_on_SYS_STATE_TRANSFER_EDIT;

  				*ssid = si;
#else	/* allow '#' in middle of dial data. */
			/* turn off dial tone. */
			*ssid = si;
			CallInvite(lc, si, src_ptr);
#endif
		}
		else
		{
			edit_invite_digit(lc, s_sign);
			rtk_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_DIAL, FALSE, 
				DSPCODEC_TONEDIRECTION_LOCAL);
			
#ifdef CONFIG_RTK_VOIP_DIALPLAN
			if( ( si = GetIdleSession(lc->chid) ) == -1 ) {
				digitcode_init(lc);	
				return 0;
			} 
			
			if( DialPlanTryCallInvite( lc, si, src_ptr ) ) {
				/* Check PSTN routing prefix */
				if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, src_ptr, 1 ) ) ) 
					goto label_fxo_call_on_SYS_STATE_TRANSFER_EDIT;

				*ssid = si;
			} else
#endif
		
		    if (strcmp(lc->dial_code, g_pVoIPCfg->funckey_pstn) == 0) 
			{
label_fxo_call_on_SYS_STATE_TRANSFER_EDIT:
				//if (g_MaxVoIPPorts > RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
				if( RTK_VOIP_DAA_NUM( g_VoIP_Feature ) > 0 )
				{
					char sz_fxo_0[40];

					if( pszPSTNRouting && pszPSTNRouting[ 0 ] ) {
						sprintf(sz_fxo_0, "sip:%s@127.0.0.1:%d", 
							pszPSTNRouting,
							g_pVoIPCfg->ports[RTK_VOIP_DAA_CH_OFFSET(g_VoIP_Feature) /*RTK_VOIP_SLIC_NUM(g_VoIP_Feature)*/].sip_port);
					} else {
						sprintf(sz_fxo_0, "sip:127.0.0.1:%d", 
							g_pVoIPCfg->ports[RTK_VOIP_DAA_CH_OFFSET(g_VoIP_Feature) /*RTK_VOIP_SLIC_NUM(g_VoIP_Feature)*/].sip_port);
					}
					strcpy(lc->dial_code, sz_fxo_0);
					CallInvite(lc, *ssid, src_ptr);
				}
				else	
			    {
					SetSysState(lc->chid, SYS_STATE_TRANSFER_EDIT_ERROR);
			    	rtk_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
			    }
			}
		}		
		/*--end--*/
		break;
/*+++added by Jack Chan for Planet transfer+++*/
#ifdef PLANET_TRANSFER
	case SYS_STATE_TRANSFER_BLIND:
		if
#ifdef CONFIG_RTK_VOIP_DIALPLAN
			(s_sign == SIGN_AUTODIAL ||
			(s_sign == SIGN_HASH &&  
			(g_pVoIPCfg->auto_dial & AUTO_DIAL_ALWAYS) == 0))
#else
			(s_sign == SIGN_HASH)
#endif
		{	
			if (lc->digit_index == 0) 
				return 0;

			*ssid = (active_session == 0) ? 1 : 0;
#ifdef CONFIG_RTK_VOIP_DIALPLAN
			CallTransferByDialPlan( lc, *ssid );
			/*Bye the call immediately*/
			linphone_core_terminate_dialog(lc, *ssid, NULL);
			rtk_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
#endif /* CONFIG_RTK_VOIP_DIALPLAN */						
		}
		else
		{
			edit_invite_digit(lc, s_sign);
			rtk_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_DIAL, FALSE, 
				DSPCODEC_TONEDIRECTION_LOCAL);
			
#ifdef CONFIG_RTK_VOIP_DIALPLAN
			if (lc->digit_index == 0) 
				return 0;
			
			*ssid = (active_session == 0) ? 1 : 0;
			if( DialPlanTryCallTransfer( lc, *ssid ) ) {
				/*Bye the call immediately*/
				linphone_core_terminate_dialog(lc, *ssid, NULL);
				rtk_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);							
			}
#endif
		}
	break;
#endif /*PLANET_TRANSFER*/
/*---end---*/
	case SYS_STATE_TRANSFER_EDIT_ERROR:
		if (s_sign == SIGN_FLASHHOOK)
		{
			if (session_2_session_event(lc, (active_session == 0) ? 2 : 1) == 0)
			{
				*ssid = GetActiveSession(lc->chid);
				SetSysState(lc->chid, SYS_STATE_CONNECT);
			}
			digitcode_init(lc);
		}
		break;

	case SYS_STATE_DAA_CONNECT:
		// do nothing if daa connect
		break;

	default:
		// TODO: 
		printf("skip state = %d\n", state);
		break;
	}

	return 1;
}

void GetLineEvent(LinphoneCore *lc, uint32 *ssid, char *src_ptr, char *digit)
{
	SIGNSTATE keyin;

#if 1 // Check RFC2833 Rx Event.
	RFC2833_EVENT ent;
	uint32 mid=0;
	
	for (mid=0; mid<2; mid++)
	{
		rtk_GetRfc2833RxEvent(lc->chid, mid, &ent);
		if (ENT_NA != ent)
		{
			printf("RX RFC2833 Event %d, ch%d, mid%d\n", ent, lc->chid, mid);
		}
	}
#endif

	//if (lc->chid < RTK_VOIP_SLIC_NUM(g_VoIP_Feature)) {
	if( RTK_VOIP_IS_SLIC_CH( lc->chid, g_VoIP_Feature ) ) {
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		return;	/* IP phone has no keyin events. */
#else
		rtk_GetFxsEvent(lc->chid, &keyin);
#endif
	} else if( RTK_VOIP_IS_DECT_CH( lc->chid, g_VoIP_Feature ) ) {
		rtk_GetDectEvent(lc->chid, &keyin);
	} else
		rtk_GetFxoEvent(lc->chid, &keyin);

	switch( ( SIGNSTATE )keyin )
	{
		case SIGN_KEY1 :
		case SIGN_KEY2 :
		case SIGN_KEY3 :
		case SIGN_KEY4 :
		case SIGN_KEY5 :
		case SIGN_KEY6 :
		case SIGN_KEY7 :
		case SIGN_KEY8 :
		case SIGN_KEY9 :
		case SIGN_KEY0 :
		case SIGN_STAR :
		case SIGN_HASH :
		case SIGN_FLASHHOOK :
			if (keyin == SIGN_FLASHHOOK)
				g_message("--- Flash Hook (%d) ---\n", lc->chid);

			CallDigitEdit(lc, ssid, (SIGNSTATE) keyin, src_ptr);
			break ;
		case SIGN_ONHOOK :
			HookAction(lc, (SIGNSTATE) keyin, src_ptr);
			if (VOIP_MW_AUDIOCODES == (g_VoIP_Feature & VOIP_MW_MASK))
				rtk_Onhook_Action(lc->chid); /*thlin: new add when porting ACMW */
			*ssid = 0;
			g_message("--- Get ON Hook Event (%d) ---\n", lc->chid);
			break;
		case SIGN_OFFHOOK :
			//if (lc->chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature) && 
			if( RTK_VOIP_IS_DAA_CH( lc->chid, g_VoIP_Feature ) &&
				CheckDND(lc->chid))
			{
				// do nothing if DND enabled in FXO
				g_warning("--- DND in FXO ---\n");
				break;
			}

			if (VOIP_MW_AUDIOCODES == (g_VoIP_Feature & VOIP_MW_MASK))
				rtk_Offhook_Action(lc->chid); /*thlin: new add when porting ACMW */

			HookAction(lc, (SIGNSTATE) keyin, src_ptr);
			*ssid = 0;
			osip_gettimeofday(&lc->off_hook_start, NULL);
			g_message("--- Get OFF Hook Event (%d) ---\n", lc->chid);
			break ;
		case SIGN_OFFHOOK_2:
			/* Off-hook but no key */
			break;
		case SIGN_RING_ON:
			switch (GetSysState(lc->chid))
			{
			case SYS_STATE_IDLE:
				SetSysState(lc->chid, SYS_STATE_DAA_RING);
				rtk_SetRingFXS(lc->chid, TRUE);
				break;
			case SYS_STATE_CONNECT:
				{
				int active_session, ss;

				g_message("rtk_DAA_ring in call waiting (%d)\n", lc->chid);
				active_session = GetActiveSession(lc->chid);
				if (lc->parent->ports[lc->chid].call[active_session]->state != LCStateAVRunning)
				{
					// do nothing if nor av running
					g_warning("Active session not AV Running!?\n");
					break;
				}

				ss = active_session == 0 ? 1 : 0;
				SetSysState(lc->chid, SYS_STATE_CONNECT_DAA_RING);
				SetSessionState(lc->chid, ss, SS_STATE_CALLIN);
				rtk_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_CALL_WAITING, TRUE, 
					DSPCODEC_TONEDIRECTION_LOCAL);
				break;
			}
			default:
				// TODO: ?
				break;
			}
			break;
		case SIGN_RING_OFF:
			switch (GetSysState(lc->chid))
			{
			case SYS_STATE_DAA_RING:
				rtk_SetRingFXS(lc->chid, FALSE);
				SetSysState(lc->chid, SYS_STATE_IDLE);
				break;
			case SYS_STATE_CONNECT_DAA_RING:
			{
				int active_session, ss;

				g_message("rtk_DAA_ring_off(%d)\n", lc->chid);
				active_session = GetActiveSession(lc->chid);
				if (lc->parent->ports[lc->chid].call[active_session]->state != LCStateAVRunning)
				{
					// do nothing if nor av running
					g_warning("Active session not AV Running!?\n");
					break;
				}

				ss = active_session == 0 ? 1 : 0;
				SetSysState(lc->chid, SYS_STATE_CONNECT);
				rtk_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_CALL_WAITING, FALSE, 
					DSPCODEC_TONEDIRECTION_LOCAL);
				SetSessionState(lc->chid, ss, SS_STATE_IDLE);
				break;
			}
			default:
				// TODO: ?
				break;
			}
			break;
		default:
			if (keyin)
				g_warning("=> unknown (%d): %d !?\n", lc->chid, keyin);
			break;
	}
}

void CheckFaxOnCall(LinphoneCore *lc)
{
	int i;
	int fax_modem = FAX_IDLE;
	int fax_end=0;

	switch (GetSysState(lc->chid))
	{
	case SYS_STATE_CONNECT:
		// handle fax if simple connection
		break;
	default:
		// do nothing
		return;
	}

	i = GetActiveSession(lc->chid);
	if (i == -1)
	{
		g_warning("%s: no active session!?\n", __FUNCTION__);
		return;
	}

	if (lc->call[i] == NULL || 
		lc->call[i]->state != LCStateAVRunning)
	{
		// do nothing if active session is not av-running
		return;
	}

#ifdef SUPPORT_V152_VBD
	if( lc->call[i]->audio_params.initialized_vbd ) {
		// V.152: Processed by kernel 
		uint32 t;
		
		// feed Fax event to prevent overflow 
		rtk_GetFaxModemEvent( lc->chid, &t, 0);
		rtk_GetFaxEndDetect( lc->chid, &t );
		
		return;
	}
#endif
	rtk_GetFaxModemEvent(lc->chid, &fax_modem, 0);
	
	if (lc->call[i]->faxflag == FAX_IDLE)
	{
		if ( (fax_modem == LOW_SPEED_FAX_DETECT) ||(fax_modem == HIGH_SPEED_FAX_DETECT) )
		{
			lc->call[i]->faxflag = FAX_RUN;			/* set! */
			linphone_call_fax_reinvite(lc, i);
			printf("SIP go FAX_RUN\n");
		}
		else if (fax_modem == MODEM_DETECT)
		{
			lc->call[i]->faxflag = MODEM_RUN;
			linphone_call_fax_reinvite(lc, i);
			printf("SIP go MODEM_RUN\n");
		}
//		printf("lc->call[i]->faxflag is %d\n",lc->call[i]->faxflag);
	}
	else if ((lc->call[i]->faxflag == MODEM_RUN)&& (TRUE == lc->sip_conf.T38_enable ))
	{
		
		if (fax_modem == HIGH_SPEED_FAX_DETECT)
		{
			lc->call[i]->faxflag = FAX_RUN;
			linphone_call_fax_reinvite(lc, i);
			printf("SIP go FAX_RUN2\n");
		}
	}
	else if ((lc->call[i]->faxflag == FAX_RUN)&& (TRUE == lc->sip_conf.T38_enable ))
	{
		rtk_GetFaxEndDetect(lc->chid,&fax_end);
//		printf("\r\n fax_end is %d\n",fax_end);
		if(fax_end)
		{
			// Flush FIFO
			rtk_GetFaxModemEvent(lc->chid, &fax_modem, 1);
	//		printf("\r\n REAL  linphone_call_off_fax_reinvite \n");
			lc->call[i]->faxflag = FAX_IDLE;
			linphone_call_off_fax_reinvite(lc, i);
			printf("SIP go FAX_IDLE\n");
		}

		
	}
}

#ifdef FIX_RESPONSE_TIME

extern void linphone_core_cb_reinvite(void *user, int type, eXosip_event_t *ev);
eXosip_callback_t linphone_call_callbacks[EXOSIP_CALLBACK_COUNT];

#endif

void app_read_config_file( int onlyonce )
{
	/*
	 * *onlyonce* indicates that read this configuration only once 
	 * in very early stage. 
	 * This is because that following procedures need these flags
	 * (e.g. debug flag). 
	 * Other configurations will be read later, because these variables
	 * are set to zero after app_init().
	 */
#define NAME_DEF( name )		name, sizeof( name ) - 1

	typedef enum {
		VAR_INT,
		VAR_ULONG,
		VAR_UCHAR_BOOL,
		VAR_STRING,
		VAR_FUNC1,	/* void function( const char * ) */
	} VAR_T;

	typedef struct app_config_s {
		char *cfg_name;		/* "debug=", equal sign is very important */
		int cfg_name_len;	/* sizeof( "debug=" ) - 1 */
		void *ptr_var;		/* point to variable */
		VAR_T type_var;		/* type of variable */
		int size_var;		/* size of variable (string type only) */
	} app_config_t;
	
	/* main() call this case only once */
	static const app_config_t app_config_onlyonce[] = {
		{ NAME_DEF( "debug=" ), &g_SystemDebug, VAR_ULONG, 0 },
	};
	
	/* app_init() call this case */
	static const app_config_t app_config_not_once[] = {
		{ NAME_DEF( "user_agent=" ), eXosip_set_user_agent, VAR_FUNC1, 0 },
		{ NAME_DEF( "ivr_conference=" ), g_SystemParam.conference_code, VAR_STRING, 20 },
#ifdef PLANET_TRANSFER
		{ NAME_DEF( "ivr_transfer=" ), &eXosip.bIVR_transfer, VAR_UCHAR_BOOL, 0 },
		{ NAME_DEF( "ivr_blind_transfer=" ), eXosip.blind_transfer_code, VAR_STRING, 20 },
		{ NAME_DEF( "ivr_attended_transfer=" ), eXosip.attended_transfer_code, VAR_STRING, 20 },
#endif
		{ NAME_DEF( "stutter_tone=" ), &g_SystemParam.stutter_tone, VAR_UCHAR_BOOL, 0 },
		{ NAME_DEF( "fxo_redial=" ), &g_SystemParam.fxo_redial, VAR_UCHAR_BOOL, 0 },
		{ NAME_DEF( "proxy_timeout_count_limit=" ), &g_SystemParam.proxy_timeout_count_limit, VAR_INT, 0 },
		{ NAME_DEF( "proxy_timeout_retry_sec=" ), &g_SystemParam.proxy_timeout_retry_sec, VAR_INT, 0 },
#ifdef SUPPORT_G7111
		{ NAME_DEF( "g711u_wb_pt=" ), &g_SystemParam.g711u_wb_pt, VAR_INT, 0 },
		{ NAME_DEF( "g711a_wb_pt=" ), &g_SystemParam.g711a_wb_pt, VAR_INT, 0 },
#endif
		{ NAME_DEF( "g726_16_pt=" ), &g_SystemParam.g726_16_pt, VAR_INT, 0 },
		{ NAME_DEF( "g726_24_pt=" ), &g_SystemParam.g726_24_pt, VAR_INT, 0 },
		{ NAME_DEF( "g726_32_pt=" ), &g_SystemParam.g726_32_pt, VAR_INT, 0 },
		{ NAME_DEF( "g726_40_pt=" ), &g_SystemParam.g726_40_pt, VAR_INT, 0 },
		{ NAME_DEF( "rtp_init=" ),	&g_SystemParam.rtp_init, VAR_INT, 0 },
		{ NAME_DEF( "seqno=" ),		&g_SystemParam.rtp_init_seqno, VAR_ULONG, 0 },
		{ NAME_DEF( "ssrc=" ),		&g_SystemParam.rtp_init_ssrc, VAR_ULONG, 0 },
		{ NAME_DEF( "timestamp=" ),	&g_SystemParam.rtp_init_timestamp, VAR_ULONG, 0 },
	};

#undef NAME_DEF


	FILE *fp;

	fp = fopen(CONFIG_NAME, "r");
	if (!fp)
		return;
	
	//char *p;
	char *nextLineChar=NULL;
	char line[512];	
	
	const app_config_t *app_config;
	int ap_config_count;
	
	while (!feof(fp))
	{
		fgets(line, sizeof(line), fp);
		/*remove the next line char*/
		nextLineChar=strchr(line, '\n');
		if(nextLineChar)
			*nextLineChar='\0';
			
		if( line[ 0 ] == '#' || line[ 0 ] == '\0' || line[ 0 ] == ' ' )
			continue;

		if( onlyonce ) {
			app_config = &app_config_onlyonce[ 0 ];
			ap_config_count = sizeof( app_config_onlyonce ) / sizeof( app_config_onlyonce[ 0 ] );
		} else {
			app_config = &app_config_not_once[ 0 ];
			ap_config_count = sizeof( app_config_not_once ) / sizeof( app_config_not_once[ 0 ] );
		}

		for( ; ap_config_count > 0; ap_config_count --, app_config ++ ) {
		
			if( strncmp( line, app_config ->cfg_name, app_config ->cfg_name_len ) )
				continue;
			
			const void * const ptr_src = line + app_config ->cfg_name_len;
			void * const ptr_dst = app_config ->ptr_var;
			
			switch( app_config ->type_var ) {
			case VAR_INT:
				*( ( int * )ptr_dst ) = atoi( ptr_src );
				g_message( "CFG: %s%d\n", app_config ->cfg_name, *( ( int * )ptr_dst ) );
				break;
				
			case VAR_ULONG:
				*( ( unsigned long * )ptr_dst ) = atol( ptr_src );
				g_message( "CFG: %s%lu\n", app_config ->cfg_name, *( ( unsigned long * )ptr_dst ) );
				break;
				
			case VAR_UCHAR_BOOL:
				if( atoi( ptr_src ) )
					*( ( unsigned char * )ptr_dst ) = 1;
				g_message( "CFG: %s%d\n", app_config ->cfg_name, *( ( unsigned char * )ptr_dst ) );
				break;
				
			case VAR_STRING:
				strncpy( ptr_dst, ptr_src, app_config ->size_var );
				*( ( unsigned char * )ptr_dst + 
							app_config ->size_var - 1 ) = '\x0';
				g_message( "CFG: %s%s\n", app_config ->cfg_name, ( unsigned char * )ptr_dst );
				break;
				
			case VAR_FUNC1:	/* void function( const char * ) */
				( ( void ( * )( const char * ) )ptr_dst )( ptr_src );
				g_message( "CFG: %sCall:%p,%s\n", app_config ->cfg_name, ptr_dst, ( const char * )ptr_src );
				break;
			}
			
			break;
		}

#if 0
		if( onlyonce ) {
			/* main() call this case only once */
			p = strstr( line, "debug=" );
			if( p ) {
				g_SystemDebug = atol( line + sizeof( "debug=" ) - 1 );
				continue;
			}		
		} else {
			/* app_init() call this case */
			p = strstr(line, "user_agent=");
			if (p)
			{
				eXosip_set_user_agent(p + strlen("user_agent="));
				continue;
			}
			
			p = strstr( line, "ivr_conference=" );
			if( p ) {
				strncpy( g_SystemParam.conference_code, line + strlen( "ivr_conference=" ), 20 );
				g_SystemParam.conference_code[ 19 ] = '\x0';
				continue;
			}
/*+++++added by Jack for Planet Transfer+++++*/			
#ifdef PLANET_TRANSFER
			p = strstr(line, "ivr_transfer=");
			if(p)
			{
				eXosip.bIVR_transfer=atoi(p + strlen("ivr_transfer="));
				continue;
			}
			p=strstr(line, "ivr_blind_transfer=");
			if(p)
			{
				char *tmpCode=NULL;
				
				tmpCode=p+strlen("ivr_blind_transfer=");
				if(tmpCode != NULL && strlen(tmpCode) != 0)
					memcpy(eXosip.blind_transfer_code, tmpCode, strlen(tmpCode));
				continue;
			}
			p=strstr(line, "ivr_attended_transfer=");
			if(p)
			{
				char *tmpCode=NULL;
				
				tmpCode=p+strlen("ivr_attended_transfer=");
				if(tmpCode != NULL && strlen(tmpCode) != 0)
					memcpy(eXosip.attended_transfer_code, tmpCode, strlen(tmpCode));
				continue;
			}
#endif /* PLANET_TRANSFER */
/*-----end-----*/
			p = strstr( line, "stutter_tone=" );
			if( p ) {
				if( atoi( line + sizeof( "stutter_tone=" ) - 1 ) )
					g_SystemParam.stutter_tone = 1;
				continue;
			}			
			// rock: add fxo_redial option
			p = strstr( line, "fxo_redial=" );
			if( p ) {
				if( atoi( line + sizeof( "fxo_redial=" ) - 1 ) )
					g_SystemParam.fxo_redial = 1;
				continue;
			}
			// pkshih: add proxy register option 
			p = strstr( line, "proxy_timeout_count_limit=" );
			if( p ) {
				g_SystemParam.proxy_timeout_count_limit = 
					atoi( line + sizeof( "proxy_timeout_count_limit=" ) - 1 );
				continue;
			}
			p = strstr( line, "proxy_timeout_retry_sec=" );
			if( p ) {
				g_SystemParam.proxy_timeout_retry_sec = 
					atoi( line + sizeof( "proxy_timeout_retry_sec=" ) - 1 );
				continue;
			}
		}
#endif

	}
	fclose(fp);
}

gboolean exosip_running = FALSE;

int app_init(TAPP *pApp)
{
	int i, err;
	int cust_idx;
	int ports[MAX_VOIP_PORTS];
	int media_ports[MAX_VOIP_PORTS];
	int t38_ports[MAX_VOIP_PORTS];
		
	ortp_init();

	memset(pApp, 0, sizeof(*pApp));

	init_g_state( pApp ) ;
	
	memset( &g_SystemParam, 0, sizeof( g_SystemParam ) );

//	linphone_core_enable_ipv6( lc , FALSE );
//	linphone_core_set_sip_port( lc , voip_ptr->sip_port );

	// get port link status 
	rtk_GetPortLinkStatus( &pApp ->port_link_status );

	if (exosip_running)
		eXosip_quit();

	for (i=0; i<g_MaxVoIPPorts; i++)
	{
		ports[i] = g_pVoIPCfg->ports[i].sip_port;
		media_ports[i] = g_pVoIPCfg->ports[i].media_port;
		if (g_pVoIPCfg->ports[i].useT38)
			t38_ports[i] = g_pVoIPCfg->ports[i].T38_port;
		else
			t38_ports[i] = 0;
	}

	eXosip_enable_ipv6(FALSE);

#ifdef FIX_RESPONSE_TIME
	// prepare callback event for quick response
	memset(linphone_call_callbacks, 0, sizeof(linphone_call_callbacks));
	linphone_call_callbacks[EXOSIP_CALL_HOLD] = linphone_core_cb_reinvite;
	linphone_call_callbacks[EXOSIP_CALL_OFFHOLD] = linphone_core_cb_reinvite;
	linphone_call_callbacks[EXOSIP_CALL_UPDATE] = linphone_core_cb_reinvite;
	// install callback on eXosip_init
	eXosip_register_callback(linphone_call_callbacks);
	err = eXosip_init(pApp, NULL, stdout, ports, media_ports, t38_ports);
#else
	err = eXosip_init(NULL, stdout, ports, media_ports, t38_ports);
#endif
	if (err < 0)
	{
		g_error("UDP port seems already in use ! Cannot initialize.\n");
		return -1;
	}

	eXosip_set_user_agent("sip phone");
	// user agent customization
	app_read_config_file( 0 );
	
	exosip_running = TRUE;
	
	for (i=0; i<g_MaxVoIPPorts; i++)
	{
		linphone_core_init(pApp, &pApp->ports[i], i);
		// init led
		rtk_sip_register(i, 0);
		UpdateLedDisplayMode( &pApp->ports[i] );
		
		//if (i < RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
		if( RTK_VOIP_IS_SLIC_CH( i, g_VoIP_Feature ) )
		{
			/* Get flash hook time from flash and set to kernel space.*/
			rtk_Set_Flash_Hook_Time(i, g_pVoIPCfg->ports[i].flash_hook_time_min, 
				g_pVoIPCfg->ports[i].flash_hook_time);//time unit: 10ms
			
			/* Set the pulse dial detection */
			rtk_Set_Pulse_Digit_Det(i, g_pVoIPCfg->pulse_dial_det, g_pVoIPCfg->pulse_det_pause, 20/*min_break_ths*/, 70/*max_break_ths*/);
			//printf("pulse_dial_det= %d\n", g_pVoIPCfg->pulse_dial_det);
			//printf("pulse_det_pause= %d\n", g_pVoIPCfg->pulse_det_pause);
		}
		else if( RTK_VOIP_IS_DECT_CH( i, g_VoIP_Feature ) )
			;	// do nothing 
		else
		{
			/* Set the Caller ID Detection Mode */
			rtk_Set_CID_Det_Mode(i, g_pVoIPCfg->cid_auto_det_select, g_pVoIPCfg->caller_id_det_mode);
			/* Set the Ring detection parameters for FXO */
  	                rtk_Set_FXO_Ring_Detection(300, 300, 4500); //must after set CID det mode
			/* Set the pulse dial generation */
			rtk_Set_Dail_Mode(i, g_pVoIPCfg->pulse_dial_gen);
			//printf("pulse_dial_gen= %d\n", g_pVoIPCfg->pulse_dial_gen);
			
			if (g_pVoIPCfg->pulse_dial_gen == 1)
			{
				rtk_PulseDial_Gen_Cfg(g_pVoIPCfg->pulse_gen_pps, g_pVoIPCfg->pulse_gen_make_time, g_pVoIPCfg->pulse_gen_interdigit_pause);
				//printf("pulse_gen_pps= %d\n", g_pVoIPCfg->pulse_gen_pps);
				//printf("pulse_gen_make_time= %d\n", g_pVoIPCfg->pulse_gen_make_time);
				//printf("pulse_gen_interdigit_pause= %d\n", g_pVoIPCfg->pulse_gen_interdigit_pause);
			}
		}
	}
	
	/* Get Tx/Rx gain value from flash and set to DAA.*/
	if ((g_VoIP_Feature & DAA_TYPE_MASK) == REAL_DAA || (g_VoIP_Feature & DAA_TYPE_MASK) == REAL_DAA_NEGO)
	{
		rtk_Set_DAA_Tx_Gain(g_pVoIPCfg->daa_txVolumne);
		rtk_Set_DAA_Rx_Gain(g_pVoIPCfg->daa_rxVolumne);
	}
	
	/* Get Tone of Customer from flash and set.*/ 
	for (cust_idx=0; cust_idx < TONE_CUSTOMER_MAX; cust_idx++)
	{
		rtk_Set_Custom_Tone(cust_idx, &g_pVoIPCfg->cust_tone_para[cust_idx]);
	}	
		
	/* Get Tone of Cuntury from flash and set. rtk_Set_Tone_Country()
	 * should be called after calling rtk_Set_Custom_Tone() for proper
	 * web page operation.
	 */
	rtk_Set_Country(g_pVoIPCfg);
	
	rtk_Set_Dis_Tone_Para(g_pVoIPCfg);
	/* Initialize IVR */
#ifdef CONFIG_RTK_VOIP_IVR
  #if 1
	InitIvrClient();
	
	SendGlobalConstantToIVR_IPC( g_nMaxCodec, g_mapSupportedCodec );
  #else
	InitializeIVR();
  #endif
#endif

#ifdef CONFIG_RTK_VOIP_WAN_VLAN
	//rtk_switch_wan_vlan(g_pVoIPCfg);
	//rtk_switch_wan_2_vlan(g_pVoIPCfg);
	rtk_switch_wan_3_vlan(g_pVoIPCfg);
#endif

#if defined (CONFIG_RTL865XC) || defined(CONFIG_RTL_819X)
	//Bandwidth Mgr
    #ifdef CONFIG_RTL_819X
	rtk_Bandwidth_Mgr(0, 1, g_pVoIPCfg->bandwidth_LANPort0_Egress);
	rtk_Bandwidth_Mgr(1, 1, g_pVoIPCfg->bandwidth_LANPort1_Egress);
	rtk_Bandwidth_Mgr(2, 1, g_pVoIPCfg->bandwidth_LANPort2_Egress);
	rtk_Bandwidth_Mgr(3, 1, g_pVoIPCfg->bandwidth_LANPort3_Egress);
	rtk_Bandwidth_Mgr(4, 1, g_pVoIPCfg->bandwidth_WANPort_Egress);

	rtk_Bandwidth_Mgr(0, 0, g_pVoIPCfg->bandwidth_LANPort0_Ingress);
	rtk_Bandwidth_Mgr(1, 0, g_pVoIPCfg->bandwidth_LANPort1_Ingress);
	rtk_Bandwidth_Mgr(2, 0, g_pVoIPCfg->bandwidth_LANPort2_Ingress);
	rtk_Bandwidth_Mgr(3, 0, g_pVoIPCfg->bandwidth_LANPort3_Ingress);
	rtk_Bandwidth_Mgr(4, 0, g_pVoIPCfg->bandwidth_WANPort_Ingress);
    #else
	rtk_Bandwidth_Mgr(1, 1, g_pVoIPCfg->bandwidth_LANPort0_Egress);
	rtk_Bandwidth_Mgr(2, 1, g_pVoIPCfg->bandwidth_LANPort1_Egress);
	rtk_Bandwidth_Mgr(3, 1, g_pVoIPCfg->bandwidth_LANPort2_Egress);
	rtk_Bandwidth_Mgr(4, 1, g_pVoIPCfg->bandwidth_LANPort3_Egress);
	rtk_Bandwidth_Mgr(0, 1, g_pVoIPCfg->bandwidth_WANPort_Egress);

	rtk_Bandwidth_Mgr(1, 0, g_pVoIPCfg->bandwidth_LANPort0_Ingress);
	rtk_Bandwidth_Mgr(2, 0, g_pVoIPCfg->bandwidth_LANPort1_Ingress);
	rtk_Bandwidth_Mgr(3, 0, g_pVoIPCfg->bandwidth_LANPort2_Ingress);
	rtk_Bandwidth_Mgr(4, 0, g_pVoIPCfg->bandwidth_LANPort3_Ingress);
	rtk_Bandwidth_Mgr(0, 0, g_pVoIPCfg->bandwidth_WANPort_Ingress);
    #endif

#endif
#ifdef SUPPORT_DSCP
	rtk_qos_reset_dscp_priority();
	rtk_qos_set_dscp_priority( qos2dscp(g_pVoIPCfg->sipDscp), 7); 
	rtk_qos_set_dscp_priority( qos2dscp(g_pVoIPCfg->rtpDscp), 7); 
	rtk_Set_Rtp_Dscp( qos2dscp(g_pVoIPCfg->rtpDscp));
        rtk_Set_Sip_Dscp( qos2dscp(g_pVoIPCfg->sipDscp));	
	sleep(3); // wait 8306 reset done
#endif

#ifdef CONFIG_APP_TR104
	{
		struct sockaddr_un ipcAddr;
		
		ipcSocket = socket(PF_LOCAL, SOCK_DGRAM, 0);
		if(0 > ipcSocket)
			g_warning("open IPC socket fail: %s\n", strerror(errno));
		unlink(SOLAR_CHANNEL_PATH);
		bzero(&ipcAddr, sizeof(ipcAddr));
		ipcAddr.sun_family = PF_LOCAL;
		strncpy(ipcAddr.sun_path, SOLAR_CHANNEL_PATH, sizeof(SOLAR_CHANNEL_PATH));
		if(bind(ipcSocket, (struct sockaddr*)&ipcAddr, sizeof(ipcAddr)) == -1){
			close(ipcSocket);
			g_warning("bind IPC socket fail: %s\n", strerror(errno));
		}
	}
#endif /*CONFIG_APP_TR104*/

	// flush kernel used fifo
	for (i=0; i<g_MaxVoIPPorts; i++)
		rtk_Set_flush_fifo(i);
		
	// initialize alarm 
	InitializeAlarmVariable();
	
	// open ICMP 
	open_voip_icmp_socket();

	return 0;
}

int app_close(TAPP *pApp)
{
	int i, nPort;
	GList *elem;
	LinphoneCore *lc;
	int h_max;
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	int proxyIndex;
#endif

	if (!exosip_running)
		return -1;
	
	// do unregister first
	for (nPort=0; nPort<g_MaxVoIPPorts; nPort++)
	{
		sip_config_t *config;

		lc = &pApp->ports[nPort];
		config = &lc->sip_conf;
		for(elem=config->proxies; elem!=NULL; elem=g_list_next(elem))
		{
			LinphoneProxyConfig *cfg;
			
			cfg = (LinphoneProxyConfig*)(elem->data);
			linphone_proxy_config_edit(cfg);	/* to unregister */
		}

	}

	// send pending packets (include unregister)
	eXosip_execute();
	
	h_max = eXosip.j_sockets[0];
	for (nPort=0; nPort<g_MaxVoIPPorts; nPort++)
	{
		if (h_max < eXosip.j_sockets[nPort])
			h_max = eXosip.j_sockets[nPort];
	}

#ifdef MALLOC_DEBUG
	// if use memwatch, don't handle any packet on exit
#else
	// wait 2 sec to clear state machine
	for (i=0; i<20; i++) 
	{
		int err;
		fd_set fdset;
		struct timeval tv;

		FD_ZERO(&fdset);		
		for (nPort=0; nPort<g_MaxVoIPPorts; nPort++)
		{
			FD_SET(eXosip.j_sockets[nPort], &fdset);
#ifdef CONFIG_RTK_VOIP_SIP_TLS
			for(proxyIndex = 0; proxyIndex < MAX_PROXY; proxyIndex++)
				if(eXosip.j_tls_sockets[nPort][proxyIndex] != -1)
					FD_SET(eXosip.j_tls_sockets[nPort][proxyIndex], &fdset);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
		}

		tv.tv_sec = 0;
		tv.tv_usec = 100000; // 100ms
		err = select(h_max + 1, &fdset, NULL, NULL, &tv);
		if ((err == -1) && (errno == EINTR || errno == EAGAIN))
			continue;

		if (err == 0) // timeout
		{
			eXosip_event_t *ev;

			while((ev=eXosip_event_wait(0,0))!=NULL)
			{
	  			linphone_core_process_event(pApp, ev);
				eXosip_execute();
			}

			continue;
		}

		if (err > 0)
		{
			for (nPort=0; nPort<g_MaxVoIPPorts; nPort++)
			{
				if (FD_ISSET(eXosip.j_sockets[nPort], &fdset))
				{
#ifdef NEW_STUN
					char *sip_buf;
					int sip_len;

#ifdef CONFIG_RTK_VOIP_SIP_TLS
					if (!eXosip_read_message(&pApp->ports[nPort], &sip_buf, &sip_len, TRANSPORT_UDP, 0))
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
					if (!eXosip_read_message(&pApp->ports[nPort], &sip_buf, &sip_len))
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
						osip_free(sip_buf);
#else
					eXosip_read_message(&pApp->ports[nPort]);
#endif
					eXosip_execute();
				}
#ifdef CONFIG_RTK_VOIP_SIP_TLS
				for(proxyIndex = 0; proxyIndex < MAX_PROXY; proxyIndex++){
					if (eXosip.j_tls_sockets[nPort][proxyIndex] != -1 && FD_ISSET(eXosip.j_tls_sockets[nPort][proxyIndex], &fdset))
					{
#ifdef NEW_STUN
						char *sip_buf;
						int sip_len;

						if(eXosip_get_TCP_status(eXosip.j_tls_sockets[nPort][proxyIndex])!=TCP_CLOSE_WAIT && eXosip_get_TCP_status(eXosip.j_tls_sockets[nPort][proxyIndex])!=TCP_CLOSE){
							if (!eXosip_read_message(&pApp->ports[nPort], &sip_buf, &sip_len, TRANSPORT_TLS, proxyIndex))
								osip_free(sip_buf);
						}						
						else{
							/*clear the tls object*/
							close(eXosip.j_tls_sockets[nPort][proxyIndex]);
							tls_free(sipTLSObj[nPort][proxyIndex]);
							/*do reconnect*/
							sleep(1);
							if(!eXosip_create_TCP_socket(&eXosip.j_tls_sockets[nPort][proxyIndex], g_pVoIPCfg->ports[nPort].sip_port, nPort, proxyIndex)){
								g_warning("create TCP socket for fxs:%d,proxy:%d fail\n",nPort, proxyIndex);
								return -1;
							}else{
								/*Do ssl initial*/
								sipTLSObj[nPort][proxyIndex].ssl_ctx=initialize_client_ctx(nPort, proxyIndex);
								if(sipTLSObj[nPort][proxyIndex].ssl_ctx == NULL){
									close(eXosip.j_tls_sockets[nPort][proxyIndex]);
								}else{
									sipTLSObj[nPort][proxyIndex].ssl_conn=initialize_client_ssl(sipTLSObj[nPort][proxyIndex].ssl_ctx,eXosip.j_tls_sockets[nPort][proxyIndex]);
								}
								/*do ssl handshake to sip proxy*/
								if(sipTLSObj[nPort][proxyIndex].ssl_conn == NULL){
									close(eXosip.j_tls_sockets[nPort][proxyIndex]);
									tls_free(sipTLSObj[nPort][proxyIndex]);
								}else{
									if(SSL_Handshake(sipTLSObj[nPort][proxyIndex].ssl_conn) == 0){
										close(eXosip.j_tls_sockets[nPort][proxyIndex]);
										g_warning("ssl handshake fail!\n");
									}
								}
							}
						}
#else
						if(eXosip_get_TCP_status(eXosip.j_tls_sockets[nPort][proxyIndex])!=TCP_CLOSE_WAIT && eXosip_get_TCP_status(eXosip.j_tls_sockets[nPort][proxyIndex])!=TCP_CLOSE){
							eXosip_read_message(&pApp->ports[nPort]);
						}else{
							close(eXosip.j_tls_sockets[nPort][proxyIndex]);
							tls_free(sipTLSObj[nPort][proxyIndex]);
							if(!eXosip_create_TCP_socket(&eXosip.j_tls_sockets[nPort][proxyIndex], g_pVoIPCfg->ports[nPort].sip_port, nPort, proxyIndex))
								return -1;
						}
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
						eXosip_execute();
					}
				}
#endif				
			}
		}
		else
		{
			g_warning("Error in select(): %s\n", strerror(errno));
			break;
		}
	}
#endif

	eXosip_lock();
	eXosip_clear_authentication_info();
	eXosip_unlock();

	for (i=0; i<g_MaxVoIPPorts; i++)
	{
		linphone_core_uninit(&pApp->ports[i]);
#if 0 
#ifdef FIX_MEMORY_LEAK
		if (pending_auth[i])
		{
			linphone_auth_info_destroy(pending_auth[i]);
			pending_auth[i] = NULL;
		}
#endif
#endif
	}
	
	eXosip_quit();
	exosip_running = FALSE;

	ortp_destroy();

#ifdef CONFIG_APP_TR104
	close(ipcSocket);
#endif
	
	close_voip_icmp_socket();
	
	return 0;
}

int linphone_call_refresh(LinphoneCall *call)
{
	switch (call->state)
	{
	case LCStateInit:
	case LCStateRinging:
		break;
	case LCStateAVRunning:
		call->sdpctx->version++;
		if (eXosip_on_update_call(call->did, call->sdpctx->version) == 0)
			return 1;
		break;
	case LCStateFax:
	case LCStateHold:
	case LCStateHoldACK:
	case LCStateResume:
	case LCStateResumeACK:
	case LCStateBeHold:
	case LCStateBeHoldACK:
	case LCStateBeResume:
	case LCStateUpdate:
		break;
	default:
		printf("unknown state: %d\n", call->state);
		break;
	}

	return 0;
}

void linphone_core_refresh(LinphoneCore *lc)
{
	int i;

	for (i=0; i<MAX_SS; i++)
	{
		if (lc->call[i])
		{
			if (linphone_call_refresh(lc->call[i]))
			{
				linphone_core_stop_media_streams(lc, i);
				lc->call[i]->state = LCStateUpdate;
			}
		}
	}
}

#ifdef RTK_DEBUG

char *str_state[] = {
    /* STATES for invite client transaction */
    "ICT_PRE_CALLING",
    "ICT_CALLING",
    "ICT_PROCEEDING",
    "ICT_COMPLETED",
    "ICT_TERMINATED",

    /* STATES for invite server transaction */
    "IST_PRE_PROCEEDING",
    "IST_PROCEEDING",
    "IST_COMPLETED",
    "IST_CONFIRMED",
    "IST_TERMINATED",

    /* STATES for NON-invite client transaction */
    "NICT_PRE_TRYING",
    "NICT_TRYING",
    "NICT_PROCEEDING",
    "NICT_COMPLETED",
    "NICT_TERMINATED",

    /* STATES for NON-invite server transaction */
    "NIST_PRE_TRYING",
    "NIST_TRYING",
    "NIST_PROCEEDING",
    "NIST_COMPLETED",
    "NIST_TERMINATED",

#ifndef DOXYGEN
    "DIALOG_EARLY",
    "DIALOG_CONFIRMED",
    "DIALOG_CLOSE"        /* ?? */
#endif
};

void rtk_dump_transactions(void)
{
	eXosip_call_t *jc, *jcnext;
	eXosip_dialog_t *jd, *jdnext;
	osip_transaction_t *tr;
	int pos;

	printf("\n");
	printf("==== dump transactions in call =====\n");
	for (jc = eXosip.j_calls ; jc != NULL; )
	{
		jcnext=jc->next;

		printf("== cid=%d ==\n", jc->c_id);
		if (jc->c_inc_options_tr!=NULL)
		{
			tr = jc->c_inc_options_tr;
			printf("transaction %d, state=%s, method=%s\n", 
				tr->transactionid, str_state[tr->state], tr->cseq->method);
	    }

		if (jc->c_out_options_tr!=NULL)
		{
			tr = jc->c_out_options_tr;
			printf("transaction %d, state=%s, method=%s\n", 
				tr->transactionid, str_state[tr->state], tr->cseq->method);
		}

		if (jc->c_inc_tr!=NULL)
		{
			tr = jc->c_inc_tr;
			printf("transaction %d, state=%s, method=%s\n", 
				tr->transactionid, str_state[tr->state], tr->cseq->method);
		}

		if (jc->c_out_tr!=NULL)
		{
			tr = jc->c_out_tr;
			printf("transaction %d, state=%s, method=%s\n", 
				tr->transactionid, str_state[tr->state], tr->cseq->method);
		}

		/* free call terminated with a BYE */
		for (jd = jc->c_dialogs ; jd != NULL; )
		{
			jdnext=jd->next;
			pos = 0;
			printf("== did=%d ==\n", jd->d_id);
			while (!osip_list_eol(jd->d_inc_trs, pos))
			{
				tr = osip_list_get(jd->d_inc_trs, pos);
				printf("transaction %d, state=%s, method=%s\n", 
					tr->transactionid, str_state[tr->state], tr->cseq->method);
				pos++;
			}
			pos = 0;
			while (!osip_list_eol(jd->d_out_trs, pos))
			{
				tr = osip_list_get(jd->d_out_trs, pos);
				printf("transaction %d, state=%s, method=%s\n", 
					tr->transactionid, str_state[tr->state], tr->cseq->method);
				pos++;
			}
			jd=jdnext;
		}

		jc=jcnext;
	}

	printf("\n");
	printf("==== dump transactions in eXosip =====\n");
	pos = 0;
	while (!osip_list_eol(eXosip.j_transactions, pos))
	{
		tr = (osip_transaction_t*) osip_list_get(eXosip.j_transactions, pos);
		printf("transaction %d, state=%s, method=%s\n", 
			tr->transactionid, str_state[tr->state], tr->cseq->method);
		pos++;
	}

	printf("\n");
	printf("==== dump transactions in ICT =====\n");
	pos = 0;
	while (!osip_list_eol(eXosip.j_osip->osip_ict_transactions, pos))
	{
		tr = (osip_transaction_t*) osip_list_get(eXosip.j_osip->osip_ict_transactions, pos);
		printf("transaction %d, state=%s, method=%s\n", 
			tr->transactionid, str_state[tr->state], tr->cseq->method);
		pos++;
	}

	printf("\n");
	printf("==== dump transactions in NICT =====\n");
	pos = 0;
	while (!osip_list_eol(eXosip.j_osip->osip_nict_transactions, pos))
	{
		tr = (osip_transaction_t*) osip_list_get(eXosip.j_osip->osip_nict_transactions, pos);
		printf("transaction %d, state=%s, method=%s\n", 
			tr->transactionid, str_state[tr->state], tr->cseq->method);
		pos++;
	}

	printf("\n");
	printf("==== dump transactions in IST =====\n");
	pos = 0;
	while (!osip_list_eol(eXosip.j_osip->osip_ist_transactions, pos))
	{
		tr = (osip_transaction_t*) osip_list_get(eXosip.j_osip->osip_ist_transactions, pos);
		printf("transaction %d, state=%s, method=%s\n", 
			tr->transactionid, str_state[tr->state], tr->cseq->method);
		pos++;
	}

	printf("\n");
	printf("==== dump transactions in NIST =====\n");
	pos = 0;
	while (!osip_list_eol(eXosip.j_osip->osip_nist_transactions, pos))
	{
		tr = (osip_transaction_t*) osip_list_get(eXosip.j_osip->osip_nist_transactions, pos);
		printf("transaction %d, state=%s, method=%s\n", 
			tr->transactionid, str_state[tr->state], tr->cseq->method);
		pos++;
	}

	printf("\n");
	printf("==== dump transactions in IXT =====\n");
	pos = 0;
	while (!osip_list_eol(eXosip.j_osip->ixt_retransmissions, pos))
	{
		tr = (osip_transaction_t*) osip_list_get(eXosip.j_osip->ixt_retransmissions, pos);
		printf("transaction %d, state=%s, method=%s\n", 
			tr->transactionid, str_state[tr->state], tr->cseq->method);
		pos++;
	}
}

#endif

// rock: fxo call fxs if "to" = "sip:fxs@127.0.0.1"
int rtk_check_fxo_call_fxs(osip_to_t *to)
{
	int i;

	if (to == NULL ||
		to->url == NULL ||
		to->url->username == NULL ||
		to->url->host == NULL ||
		strcmp(to->url->host, "127.0.0.1") != 0)
		return -1;

	if (sscanf(to->url->username, "fxo%d_to_fxs", &i) == 1)
		return i;

	return -1;
}

int rtk_call_check_fxo_call_fxs(LinphoneCall *call)
{
	eXosip_dialog_t *jd = NULL;
	eXosip_call_t *jc = NULL;

	if (call == NULL || call->did <= 0)
		return -1;

	eXosip_call_dialog_find(call->did, &jc, &jd);
	if (jd == NULL ||
		jd->d_dialog == NULL ||
		jd->d_dialog->local_uri == NULL)
		return -1;

	return rtk_check_fxo_call_fxs(jd->d_dialog->local_uri);
}

static
int
app_run(TAPP *pApp)
{
	LinphoneCore *opm;
	char input[MAX_VOIP_PORTS][LINE_MAX_LEN];
	int err;
	fd_set fdset;
	struct timeval timeout;
	gboolean run=TRUE;
	int ActiveLine = 1;
	int nPort;
	uint32 SessionID = 0 ;
	char dtmf_digit = -1;	// hc+
#ifdef WATCH_DOG
	int h_pipe;
#endif
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	int proxyIndex;
#endif
	int h_control;
	int h_max;
#ifdef DISABLE_THREAD
//	extern eXosip_t eXosip;
	int wakeup_socket;
	struct timeval lower_tv;
#endif
	int bRestart = 0;
	// timer check variable
	struct timeval now;
	unsigned long diff;
#ifdef NEW_STUN
#else
	FILE *fh;
	char buf[10];
	int STUN_timeout_counter=0;
#endif
	int slow;

#ifdef SUPPORT_IVR_HOLD_TONE 
	extern void InitG723IvrFile( void );
	extern void RefillG723IvrFile( int chid );
	
	InitG723IvrFile();
#endif /* SUPPORT_IVR_HOLD_TONE */

	// Use FIFO to do restart and auto-testing
    if (access(FIFO_CONTROL, F_OK) == -1)
	{
        if (mkfifo(FIFO_CONTROL, 0755) == -1)
	        g_error("access %s failed: %s\n", FIFO_CONTROL, strerror(errno));
	}

    h_control = open(FIFO_CONTROL, O_RDWR);
    if (h_control == -1)
        g_error("open %s failed\n", FIFO_CONTROL);

#ifdef WATCH_DOG
	// Use FIFO to implement watch dog 
    if (access(FIFO_WATCHDOG, F_OK) == -1)
	{
        if (mkfifo(FIFO_WATCHDOG, 0755) == -1)
	        g_error("access %s failed: %s\n", FIFO_WATCHDOG, strerror(errno));
	}

	g_message("wait control fifo... \n");
    h_pipe = open(FIFO_WATCHDOG, O_WRONLY);
	g_message("done\n");
    if (h_pipe == -1)
        g_error("open %s failed\n", FIFO_WATCHDOG);
#endif

#if USE_CONSOLE
	printf( "linphonec> " ) ;fflush(stdout) ;
#endif

#if 0 //thlin
	//rtk_GetDTMFMODE(0, 0, &dtmf_mode);	
	/* dtmf_mode should be configed from web page */
	//dtmf_mode = 2;	//0:rfc2833  1: sip info  2: inband
	rtk_SetDTMFMODE(0, 0, opm->sip_conf.dtmf_type);

#endif

	while (run)
	{
#ifdef WATCH_DOG
		static int i = 0;

#ifndef DISABLE_THREAD
		if (i++ % 10 == 0)
		{
			// send msg to eXosip thread every 1/port sec
			eXosip_lock() ;
			fprintf(stderr, "[T]");
			__eXosip_wakeup();
			eXosip_unlock() ;
		}
#endif

		// send msg to watch dog evey 1/port sec
		if (i++ % 10 == 0)
		{
			write(h_pipe, "x", 1);
		}
#endif

		ActiveLine = (ActiveLine + 1) % g_MaxVoIPPorts;

		opm = &pApp->ports[ActiveLine];
		input[ActiveLine][0] = 0;

		// set descriptor, and find max
		FD_ZERO(&fdset);
#if USE_CONSOLE
		FD_SET(fileno(stdin),&fdset);
#endif
		FD_SET(h_control, &fdset);
		h_max = h_control;
#ifdef DISABLE_THREAD
		wakeup_socket = jpipe_get_read_descr(eXosip.j_socketctl);
		FD_SET(wakeup_socket, &fdset);
		if (h_max < wakeup_socket)
			h_max = wakeup_socket;

		for (nPort = 0; nPort < g_MaxVoIPPorts; nPort++)
		{
			int ssid;

			FD_SET(eXosip.j_sockets[nPort], &fdset);
			if (h_max < eXosip.j_sockets[nPort])
				h_max = eXosip.j_sockets[nPort];

#ifdef CONFIG_RTK_VOIP_SIP_TLS
			for(proxyIndex = 0; proxyIndex < MAX_PROXY; proxyIndex++){
				if(eXosip.j_tls_sockets[nPort][proxyIndex] != -1){
					FD_SET(eXosip.j_tls_sockets[nPort][proxyIndex], &fdset);
					if (h_max < eXosip.j_tls_sockets[nPort][proxyIndex])
						h_max = eXosip.j_tls_sockets[nPort][proxyIndex];	
				}
			}
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
#ifdef NEW_STUN
			for (ssid=0; ssid < MAX_SS; ssid++)
			{
				if (eXosip.j_media_sockets[nPort * 2 + ssid] == -1)
					continue;

				FD_SET(eXosip.j_media_sockets[nPort * 2 + ssid], &fdset);
				if (h_max < eXosip.j_media_sockets[nPort * 2 + ssid])
					h_max = eXosip.j_media_sockets[nPort * 2 + ssid];
			}

			if (eXosip.j_t38_sockets[nPort] != -1)
			{
				FD_SET(eXosip.j_t38_sockets[nPort], &fdset);
				if (h_max < eXosip.j_t38_sockets[nPort])
					h_max = eXosip.j_t38_sockets[nPort];
			}
#endif
		}
#endif

#ifdef CONFIG_APP_TR104
		FD_SET(ipcSocket, &fdset);
		if(h_max < ipcSocket)
			h_max = ipcSocket;
#endif
		
		if( ++ slow > 5 ) {
			/*
			 * A large amount of ICMP will cause SIP hang, because
			 * ICMP socket *seems* to be the highest priority.
			 * Thus, we receive ICMP socket slowly.
			 */
			slow = 0;

			FD_SET( voip_icmp_socket, &fdset );
			if( h_max < voip_icmp_socket )
				h_max = voip_icmp_socket;
		}

		// set timeout
		timeout.tv_sec= 0;
		timeout.tv_usec = 100000/g_MaxVoIPPorts; // 100ms

		// wait event by select
		bRestart = 0;
		err = select(h_max + 1, &fdset, NULL, NULL, &timeout);

		if (err  == 0) // timeout
		{
#ifdef SUPPORT_IVR_HOLD_TONE 
			RefillG723IvrFile( ActiveLine );
#endif
		
			CheckFaxOnCall(opm);

			GetLineEvent(opm, &SessionID, input[ActiveLine], &dtmf_digit);
			
			ProcessAlarmEvent( opm );

			// check off-hook alarm and auto dial
			if (GetSessionState(opm->chid, GetActiveSession(opm->chid)) == SS_STATE_IDLE)
			{
				switch (GetSysState(opm->chid))
				{
#ifdef BYPASS_RELAY
				case SYS_STATE_AUTOBYPASS_WARNING:
					osip_gettimeofday(&now, NULL);
					diff = now.tv_sec - opm->off_hook_start.tv_sec;
					if (diff >= 2) // wait 3 sec for warning tone
					{
						// pkshih: Set to editing state to handle error case. 
						SetSysState(opm->chid, SYS_STATE_EDIT);
						rtk_SetPlayTone(opm->chid, 0, DSPCODEC_TONE_DOUBLE_RING, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
						rtk_auto_bypass_relay(opm, input[ActiveLine]);
					}
					break;
#endif
				case SYS_STATE_AUTH:
				case SYS_STATE_EDIT:
				case SYS_STATE_CONNECT_EDIT:
				case SYS_STATE_TRANSFER_EDIT:
#ifndef CONFIG_RTK_VOIP_IP_PHONE
					if (opm->digit_index == 0)
					{
						// off-hook alarm check
						if (g_pVoIPCfg->off_hook_alarm)
						{
							osip_gettimeofday(&now, NULL);
							diff = now.tv_sec - opm->off_hook_start.tv_sec;
							if (diff >= g_pVoIPCfg->off_hook_alarm)
							{
								//if (opm->chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
								if( RTK_VOIP_IS_DAA_CH( opm->chid, g_VoIP_Feature ) )
									rtk_FXO_Busy(opm->chid);
								else
									rtk_SetPlayTone(opm->chid, GetActiveSession(opm->chid), 
										DSPCODEC_TONE_OFFHOOKWARNING, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);

								SetSysState(opm->chid, GetSysState(opm->chid) + 1);
							}
						}
					}
					else
					{
						// auto dial check
						if (g_pVoIPCfg->auto_dial)
						{
							osip_gettimeofday(&now, NULL);
							diff = now.tv_sec - opm->auto_dial_start.tv_sec;
							if (diff >= (g_pVoIPCfg->auto_dial & AUTO_DIAL_TIME))
							{
								/* Add a virtual key to compatible with dial plan. */							
 #ifdef CONFIG_RTK_VOIP_DIALPLAN
								CallDigitEdit(opm, &SessionID, SIGN_AUTODIAL, input[ActiveLine]);
 #else
								CallDigitEdit(opm, &SessionID, SIGN_HASH, input[ActiveLine]);
 #endif
							}
						}
					}
#endif /* !CONFIG_RTK_VOIP_IP_PHONE */
					break;
#ifdef CONFIG_RTK_VOIP_IVR
				case SYS_STATE_VOICE_IVR:
					if( rtk_IvrPollPlaying( opm ->chid ) == 0 ) {
 #ifdef PLAY_BUSY_TONE_AFTER_IVR
						/* playing IVR done -> play busy tone */
						rtk_SetPlayTone(opm->chid, GetActiveSession(opm->chid), 
									DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
						SetSysState( opm ->chid, SYS_STATE_VOICE_IVR_DONE );
 #else
 						/* playing IVR done -> play dial tone & edit to dial */
						rtk_SetPlayTone(opm->chid, GetActiveSession(opm->chid), 
									DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
 						digitcode_init( opm );
 						SetSysState( opm ->chid, SYS_STATE_EDIT );
 						osip_gettimeofday(&opm->off_hook_start, NULL);
 #endif
					}
					break;
#endif /* CONFIG_RTK_VOIP_IVR */
				default:
					// do nothing
					break;
				}
			}

#ifdef NEW_STUN
			// Resend STUN packets in cb_udp_snd_message
#else
			// Resend STUN packets when need
			#if 1
			STUN_timeout_counter++;
			if (STUN_timeout_counter >= 100) {
				voipCfgPortParam_t *voip_ptr = &g_pVoIPCfg->ports[opm->chid];

				fh = fopen(_PATH_TMP_STATUS, "r");
				if (!fh) {
					printf("Warning: cannot open %s. Limited output.\n", _PATH_TMP_STATUS);
					printf("\nerrno=%d\n", errno);
				}
				else {
					memset(buf, 0, sizeof(buf));
					if (fgets(buf, sizeof(buf), fh) == NULL) {
						printf("app_run(): The content of /tmp/status is NULL!!\n");
						printf("\nerrno=%d\n", errno);
					}
					else {
						//printf("app_run(): buf is %s.\n", buf);
						if ((buf[ActiveLine] == '0') & (voip_ptr->stun_enable)) {
							printf("app_run(): Resend STUN packets when need(Not Registered).\n");
			            			linphone_core_set_use_info_for_stun(opm, -1, STUN_UPDATE_SIP);
						}
					}	
					fclose(fh);
				}
				STUN_timeout_counter = 0;
			}
			#endif			
#endif

#ifdef CONFIG_RTK_VOIP_IP_PHONE
#ifdef UI_EVENTS_USE_IPC_MODE
			/* If no command, try to execute UI command. */
			/* Now, it execute command only if chid == 0. */
			if( ActiveLine == 0 && input[ActiveLine][0] == 0 )
				U2S_EvnetExecutive( input[ActiveLine], &SessionID );
#endif
#endif

			if (input[ActiveLine][0] == 0) 
			{
				int ss;
#ifdef CONFIG_RTK_VOIP_DIALPLAN
				voipCfgPortParam_t *voip_ptr = &g_pVoIPCfg->ports[ opm->chid ];
#endif

		//printf("app_run(): no input from slic, check and process sip event...\n");
				// no input from slic, check and process sip event
#ifdef DISABLE_THREAD
				osip_timers_gettimeout(eXosip.j_osip, &lower_tv);
				if (lower_tv.tv_sec == 0 && lower_tv.tv_usec == 0)
					eXosip_execute();
#endif
				linphone_core_iterate(opm);

				// check no answer time
				for (ss=0; ss<MAX_SS; ss++)
				{
					int fxo_call;
					int fxo_idx;

					if (GetSessionState(ActiveLine, ss) != SS_STATE_CALLIN ||
						opm->call[ss] == NULL)
						continue;

			      	// rock: check fxo redial
					fxo_idx = rtk_call_check_fxo_call_fxs(opm->call[ss]);
					if (g_SystemParam.fxo_redial && fxo_idx >= 0)
					{
						//fxo_idx += RTK_VOIP_SLIC_NUM(g_VoIP_Feature);
						fxo_idx += RTK_VOIP_DAA_CH_OFFSET(g_VoIP_Feature);
						fxo_call = 1;
					}
					else
					{
						fxo_call = 0;
					}

#ifdef CONFIG_RTK_VOIP_DIALPLAN
					if ((!fxo_call &&
							voip_ptr ->na_forward_enable &&
							voip_ptr ->na_forward[0]) ||
						(fxo_call &&
							g_pVoIPCfg->ports[fxo_idx].na_forward_enable)
						)
#else
					if (!fxo_call &&
						opm->sip_conf.na_forward_enable &&
						opm->sip_conf.na_forward[0])
#endif
					{
						osip_gettimeofday(&now, NULL);
						diff = now.tv_sec - opm->call[ss]->no_answer_start.tv_sec;
#ifdef CONFIG_RTK_VOIP_DIALPLAN
						if ((!fxo_call&&
								diff > voip_ptr->na_forward_time) ||
							(fxo_call &&
								diff > g_pVoIPCfg->ports[fxo_idx].na_forward_time)
							)
#else
						if (diff > opm->sip_conf.na_forward_time)
#endif
						{
#ifdef CONFIG_RTK_VOIP_DIALPLAN
							if (!fxo_call)
							{
								char szForward[100] = {0};

								CallForwardByDialPlan(opm, voip_ptr->na_forward, szForward, ".");
								linphone_core_forward_dialog(opm, ss, szForward);
							}
							else if (g_pVoIPCfg->ports[fxo_idx].na_forward_enable == 1)
							{
								// 2 stage dialing
								linphone_core_forward_dialog(opm, ss, "sip:fxo_redial@127.0.0.1");
							}
							else
							{
								// direct forward
								char szForward[100] = {0};

								CallForwardByDialPlan(opm, g_pVoIPCfg->ports[fxo_idx].na_forward, szForward, ".");
								linphone_core_forward_dialog(opm, ss, szForward);
							}
#else
							linphone_core_forward_dialog(opm, ss, opm->sip_conf.na_forward);
#endif
							SetSessionState(ActiveLine, ss, SS_STATE_IDLE);
							opm->ringstream[ss] = FALSE;
							rtk_SetRingFXS(ActiveLine, FALSE);
							rtk_SetPlayTone(ActiveLine, ss, DSPCODEC_TONE_HOLD, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
							switch (GetSessionState(ActiveLine, ss == 0 ? 1 : 0))
							{
							case SS_STATE_IDLE:
							case SS_STATE_CALLOUT:
								SetSysState(ActiveLine, SYS_STATE_IDLE);
								break;
							case SS_STATE_CALLER:
							case SS_STATE_CALLEE:
#ifdef CONFIG_RTK_VOIP_IP_PHONE
								S2U_ConnectionEstInd( ActiveLine, ss, 6 );
#endif
								SetSysState(ActiveLine, SYS_STATE_CONNECT);
								break;
							default:
								g_error("unknown state in checking no answer\n");
								break;
							}
						}
					}
				}
			
			#if 0
				if (pending_auth[ActiveLine] != NULL) 
					linphonec_prompt_for_auth_final(opm);
			#endif

				continue;
			}
			else
			{
				// got input from slic
				run = linphonec_parse_command_line(opm, SessionID, input[ActiveLine]);
			}
		}// end of timeout
		else if (err > 0) // has input
		{
			if (FD_ISSET(h_control, &fdset))
			{
				char buffer[256];
				char *s;
				static int ctl_line = 0;
				LinphoneCore *ctl_lc = &pApp->ports[ctl_line];

                		err = read(h_control, buffer, sizeof(buffer));
				if (err == 0) 
				{
					// EOF !?
					close(h_control);
					h_control = open(FIFO_CONTROL, O_RDWR);
					if (h_control == -1)
						g_error("open %s failed\n", FIFO_CONTROL);
				}
				else if (err > 0)
				{
					if (err == sizeof(buffer))
						g_error("buffer is full\n");
					else
						buffer[err] = 0; // add null terminated

					//fprintf(stderr, "buffer = %s (%d)\n", buffer, strlen(buffer));
					s = strtok(buffer, "\n");
					while (s)
					{
						if (strlen(s) == 2 && s[0] == 'e') // DSP event command
						{
							// 'ef': event FAX 
							printf( "DSP Event\n" );
							
							CheckFaxOnCall(opm);
						}
						else if (strlen(s) == 2 && s[0] == 'n') // netlink command
						{
							int re_register = 0;

							// flush dns cache first
							my_gethostbyname("flush");

							if(s[1] == '0')
							{
								printf("IP Change \n");
								for (i=0; i<g_MaxVoIPPorts; i++)
									linphone_core_refresh(&pApp->ports[i]);

								re_register = 1;
							}
							else if(s[1] == '1')
							{
								printf("LINK Change \n");

				                // get port link status 
				                rtk_GetPortLinkStatus( &pApp ->port_link_status );

								re_register = 1;
							}

			                // try to register all proxy again 
							if (re_register)
							{
				                for (nPort = 0; nPort < g_MaxVoIPPorts; nPort++)
					                linphone_core_update_proxy_register_unconditionally( &pApp ->ports[ nPort ] );
							}
						}
						else  if (s[1] == 0) // command
						{
							int i;

							switch (s[0])
							{
							case 'x':
								for (i=0; i<g_MaxVoIPPorts; i++)
								{
									HookAction(&pApp->ports[i], SIGN_ONHOOK, input[i]);
									run = linphonec_parse_command_line(&pApp->ports[i], SessionID, input[i]);
								}

								app_close(pApp);

								voip_flash_client_update();
#ifdef CONFIG_RTK_VOIP_IVR
								NoticeIVRUpdateFlash_IPC();
#endif
								
								app_init(pApp);
                                g_message("restart done.\n");
								bRestart = 1;
#ifdef CONFIG_RTK_VOIP_IP_PHONE
								S2U_SolarRestartInd( 0 );
								S2U_SipRegisterInd( IP_TEMP_CHID, SIP_NOT_REGISTER );
#endif
                                break;
							case '0':
							case '1':
							case '2':
							case '3':
								ctl_line = s[0] - '0';
								printf("chagne to line %d\n", ctl_line);
								break;
                            case 's':
                                rtk_enable_pcm(ctl_line, 1);
                                HookAction(ctl_lc, SIGN_OFFHOOK, input[ctl_line]);
								osip_gettimeofday(&ctl_lc->off_hook_start, NULL);
                                run = linphonec_parse_command_line(ctl_lc, SessionID, input[ctl_line]);
                                break;
                            case 'e':
                                HookAction(ctl_lc, SIGN_ONHOOK, input[ctl_line]);
                                rtk_enable_pcm(ctl_line, 0);
                                run = linphonec_parse_command_line(ctl_lc, SessionID, input[ctl_line]);
                                break;
							case 'q':
								for (i=0; i<g_MaxVoIPPorts; i++)
								{
									HookAction(&pApp->ports[i], SIGN_ONHOOK, input[i]);
									run = linphonec_parse_command_line(&pApp->ports[i], SessionID, input[i]);
								}
								strcpy(input[ActiveLine], "quit");
                                run = linphonec_parse_command_line(ctl_lc, SessionID, input[ActiveLine]);
								break;
#ifdef CONFIG_RTK_VOIP_IVR
							case 'r':
								//apmib_reinit();
								voip_flash_client_update();
								NoticeIVRUpdateFlash_IPC();
								printf( "AP: Refresh flash\n" );
								break;
#endif
#ifdef RTK_DEBUG
							case 'd':
								rtk_dump_transactions();
								break;
#endif
                            }
                        }
                        else // dial string
                        {
                            int i;
                            fprintf(stderr, "s = %s (%d)\n", s, strlen(s));
                            for (i=0; i<strlen(s); i++)
                            {
                                switch (s[i])
                                {
                                case '1':
                                case '2':
                                case '3':
                                case '4':
                                case '5':
                                case '6':
                                case '7':
                                case '8':
                                case '9':
                                    CallDigitEdit(ctl_lc, &SessionID, SIGN_KEY1 + s[i] - '1', input[ctl_line]);
                                    break;
                                case '0':
                                    CallDigitEdit(ctl_lc, &SessionID, SIGN_KEY0, input[ctl_line]);
                                    break;
                                case '.':
                                    CallDigitEdit(ctl_lc, &SessionID, SIGN_STAR, input[ctl_line]);
                                    break;
                            	case '#':
                                    CallDigitEdit(ctl_lc, &SessionID, SIGN_HASH, input[ctl_line]);
    	                            run = linphonec_parse_command_line(ctl_lc, SessionID, input[ctl_line]);
        	                        break;
                                default:
                                    g_error("unknown button: %c\n", s[i]);
                                }
                            }
                        }
                        s = strtok(NULL, "\n");
					}

					if (bRestart)
					{
						// avoid read old socket data
						continue;
					}
				}
				else
				{
                			g_error("read %s failed\n", FIFO_CONTROL);
				}
			}

#if USE_CONSOLE
			if (FD_ISSET(fileno(stdin), &fdset))
			{
				fgets(input[ActiveLine], LINE_MAX_LEN - 1, stdin);
				run = linphonec_parse_command_line(opm, 0, input[ActiveLine]);
			}
#endif

#ifdef DISABLE_THREAD
			if (FD_ISSET(wakeup_socket, &fdset))
			{
				char buf2[500];
				jpipe_read (eXosip.j_socketctl, buf2, 499);
				eXosip_execute();
			}

#ifdef NEW_STUN
			for (nPort = 0; nPort < g_MaxVoIPPorts; nPort++)
			{
				int ssid;
				char *sip_buf;
				int sip_len;
				unsigned int mapIP;
				unsigned short mapPort;

				if (FD_ISSET(eXosip.j_sockets[nPort], &fdset))
				{
#ifdef CONFIG_RTK_VOIP_SIP_TLS
					if (eXosip_read_message(&pApp->ports[nPort], &sip_buf, &sip_len, TRANSPORT_UDP, 0))
#else
					if (eXosip_read_message(&pApp->ports[nPort], &sip_buf, &sip_len))
#endif
					{
						// handle sip message ok
						eXosip_execute();
					}
					else
					{
						// check stun and handle it
						if (sip_len > 0 &&
							stun_handlemessage(sip_buf, sip_len, &mapIP, &mapPort))
						{
							struct in_addr mapAddr;
							mapAddr.s_addr = mapIP;
							linphone_core_set_nat_address(&pApp->ports[nPort], inet_ntoa(mapAddr), mapPort, TRUE);
							rtk_stun_update(nPort, -1, STUN_UPDATE_SIP);
						}
						
						osip_free(sip_buf);
					}
				}
#ifdef CONFIG_RTK_VOIP_SIP_TLS
				for(proxyIndex = 0; proxyIndex < MAX_PROXY; proxyIndex++){
					if(eXosip.j_tls_sockets[nPort][proxyIndex] != -1 && FD_ISSET(eXosip.j_tls_sockets[nPort][proxyIndex], &fdset))
					{
						if(eXosip_get_TCP_status(eXosip.j_tls_sockets[nPort][proxyIndex]) != TCP_CLOSE_WAIT && eXosip_get_TCP_status(eXosip.j_tls_sockets[nPort][proxyIndex]) != TCP_CLOSE){
							if (eXosip_read_message(&pApp->ports[nPort], &sip_buf, &sip_len, TRANSPORT_TLS, proxyIndex))
							{
								// handle sip message ok
								eXosip_execute();
							}
						}
						/*handle the connection of the server break down suddenly*/
						else{
							/*clear the tls object*/
							close(eXosip.j_tls_sockets[nPort][proxyIndex]);
							tls_free(sipTLSObj[nPort][proxyIndex]);
							/*do reconnect*/
							sleep(1);
							if(!eXosip_create_TCP_socket(&eXosip.j_tls_sockets[nPort][proxyIndex], g_pVoIPCfg->ports[nPort].sip_port, nPort, proxyIndex)){
								g_warning("create TCP socket for fxs:%d,proxy:%d fail\n",nPort, proxyIndex);
							}else{
							/*Do ssl initial*/
								sipTLSObj[nPort][proxyIndex].ssl_ctx=initialize_client_ctx(nPort, proxyIndex);
								if(sipTLSObj[nPort][proxyIndex].ssl_ctx == NULL){
									close(eXosip.j_tls_sockets[nPort][proxyIndex]);
								}else{
									sipTLSObj[nPort][proxyIndex].ssl_conn=initialize_client_ssl(sipTLSObj[nPort][proxyIndex].ssl_ctx,eXosip.j_tls_sockets[nPort][proxyIndex]);
								}
								/*do ssl handshake to sip proxy*/
								if(sipTLSObj[nPort][proxyIndex].ssl_conn == NULL){
									close(eXosip.j_tls_sockets[nPort][proxyIndex]);
									tls_free(sipTLSObj[nPort][proxyIndex]);
								}else{
									if(SSL_Handshake(sipTLSObj[nPort][proxyIndex].ssl_conn) == 0){
										close(eXosip.j_tls_sockets[nPort][proxyIndex]);
										g_warning("ssl handshake fail!\n");
									}
								}
							}

						}
					}
				}
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
				for (ssid=0; ssid < MAX_SS; ssid++)
				{
					if (eXosip.j_media_sockets[nPort * 2 + ssid] != -1 &&
						FD_ISSET(eXosip.j_media_sockets[nPort * 2 + ssid], &fdset))
					{
						if (stun_readmessage(eXosip.j_media_sockets[nPort * 2 + ssid], 
							&sip_buf, &sip_len))
						{
							if (stun_handlemessage(sip_buf, sip_len, &mapIP, &mapPort))
							{
								printf("stun read %d bytes\n", sip_len);
								eXosip_set_firewall_media_port(nPort * 2 + ssid, mapPort);
								rtk_stun_update(nPort, ssid, STUN_UPDATE_RTP);
							}
							else
							{
//								printf("stun drop %d bytes\n", sip_len);
								linphone_core_iterate(opm);
							}
							osip_free(sip_buf);
						}
					}
				}

				if (eXosip.j_t38_sockets[nPort] != -1 &&
					FD_ISSET(eXosip.j_t38_sockets[nPort], &fdset))
				{
					if (stun_readmessage(eXosip.j_t38_sockets[nPort], &sip_buf, &sip_len))
					{
						if (stun_handlemessage(sip_buf, sip_len, &mapIP, &mapPort))
						{
							eXosip_set_firewall_faxport(nPort, mapPort);
							rtk_stun_update(nPort, -1, STUN_UPDATE_T38);
						}
						else 
						{
							linphone_core_iterate(opm);
						}
						osip_free(sip_buf);
					}
				}
			}
#else
			for (nPort = 0; nPort < g_MaxVoIPPorts; nPort++)
			{
				if (FD_ISSET(eXosip.j_sockets[nPort], &fdset))
				{
					eXosip_read_message(&pApp->ports[nPort]);
					eXosip_execute();
				}
			}
#endif

#endif
#ifdef CONFIG_APP_TR104
			if(FD_ISSET(ipcSocket, &fdset)){
				unsigned char * const buf = ( unsigned char * )&gCwmpEvtMsg;
				if(recvfrom(ipcSocket, (void*)buf, cwmpEvtMsgSizeof(), 0, NULL, NULL)>1){
					cwmpEvtMsg * const evtMsg = (cwmpEvtMsg*) buf;
					g_message("+++++get the status of line %d+++++\n",evtMsg->event);		
					solarSendResponseToCwmp(evtMsg);
				}
			}
#endif /*CONFIG_APP_TR104*/

			if( FD_ISSET( voip_icmp_socket, &fdset ) ) {	
				
				unsigned long ip;
				unsigned short port;
				int count;
				
				count = recv_voip_icmp_message( &ip, &port );
				
				//printf( "ICMP:%d,IP:%08X,Port:%u\n", count, ip, port );
				
				if( count ) {
					eXosip_process_icmp_message( ip, port );
					
					eXosip_execute();
				}
			}

		}
		else
		{
			if ((err == -1) && (errno == EINTR || errno == EAGAIN))
				continue;
			else
				g_error("Error in select(): %s\n",strerror(errno));
		}
#if USE_CONSOLE
		printf("linphonec> "); fflush(stdout);
#endif

	}
	printf("\n");
	return 0;
}

static void init_g_state( TAPP *pApp )
{
	int i = 0 , j = 0 ;
	
	for( i = 0 ; i < g_MaxVoIPPorts ; i++ )
	{
		g_sys_state[i] = SYS_STATE_IDLE ;

		for( j = 0 ; j < MAX_SS ; j++ )
		{
			c_session_state[i][j] = SS_STATE_IDLE ;
			c_active_session[i][j] = FALSE ;
		}
	}
}

int BuildSupportedCodecMap(void)
{
	int idx;

	idx = 0;
	g_mapSupportedCodec[_CODEC_G711U] = idx++;
	g_mapSupportedCodec[_CODEC_G711A] = idx++;
	g_mapSupportedCodec[_CODEC_G729] = (g_VoIP_Feature & CODEC_G729_SUPPORT) ? idx++ : _CODEC_MAX;
	g_mapSupportedCodec[_CODEC_G723] = (g_VoIP_Feature & CODEC_G723_SUPPORT) ? idx++ : _CODEC_MAX;
	g_mapSupportedCodec[_CODEC_G726_16] = (g_VoIP_Feature & CODEC_G726_SUPPORT) ? idx++ : _CODEC_MAX;
	g_mapSupportedCodec[_CODEC_G726_24] = (g_VoIP_Feature & CODEC_G726_SUPPORT) ? idx++ : _CODEC_MAX;
	g_mapSupportedCodec[_CODEC_G726_32] = (g_VoIP_Feature & CODEC_G726_SUPPORT) ? idx++ : _CODEC_MAX;
	g_mapSupportedCodec[_CODEC_G726_40] = (g_VoIP_Feature & CODEC_G726_SUPPORT) ? idx++ : _CODEC_MAX;
	g_mapSupportedCodec[_CODEC_GSMFR] = (g_VoIP_Feature & CODEC_GSMFR_SUPPORT) ? idx++ : _CODEC_MAX;
	g_mapSupportedCodec[_CODEC_ILBC] = (g_VoIP_Feature & CODEC_iLBC_SUPPORT) ? idx++ : _CODEC_MAX;
	g_mapSupportedCodec[_CODEC_G722] = (g_VoIP_Feature & CODEC_G722_SUPPORT) ? idx++ : _CODEC_MAX;
	g_mapSupportedCodec[_CODEC_SPEEX_NB] = (g_VoIP_Feature & CODEC_SPEEX_NB_SUPPORT) ? idx++ : _CODEC_MAX;

	return idx;
}

static int pidfile_acquire(char *pidfile)
{
	int pid_fd;

	if(pidfile == NULL)
		return -1;

	pid_fd = open(pidfile, O_CREAT | O_WRONLY, 0644);
	if (pid_fd < 0) 
		printf("Unable to open pidfile %s\n", pidfile);
	else 
		lockf(pid_fd, F_LOCK, 0);

	return pid_fd;
}

static void pidfile_write_release(int pid_fd)
{
	FILE *out;

	if(pid_fd < 0)
		return;

	if((out = fdopen(pid_fd, "w")) != NULL) {
		fprintf(out, "%d\n", getpid());
		fclose(out);
	}
	lockf(pid_fd, F_UNLCK, 0);
	close(pid_fd);
}

#ifdef RTK_DNS
void dbgprintf(const int level, const char *format, ...)
{
	va_list args;

	if (level > DBG_CURRENT)
		return;

	fprintf(stderr, "%d :- ", getpid());
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}
#endif

void solar_abort(int sig)
{
	printf("solar: abort\n");
	app_close(&solar);
#ifdef RTK_DNS
	my_getaddrinfo_destroy();
#endif
#ifdef MALLOC_DEBUG
	mwTerm();
#endif
	voip_flash_client_close();
	exit(0);
}

#ifdef CONFIG_APP_TR104
void solarSendResponseToCwmp(cwmpEvtMsg *evtMsg){
	int sendSock=0, i;
	struct sockaddr_un addr;
	voiceProfileLineStatus *VPLineStatus=NULL;
	
	g_message("solarSendResponseToCwmp\n");
	switch(cwmpEvtMsgGetEvent(evtMsg)){
		case EVT_VOICEPROFILE_LINE_GET_STATUS:
			for(i=0; i < g_MaxVoIPPorts; i++){
				if((VPLineStatus = (voiceProfileLineStatus*)malloc(sizeof(voiceProfileLineStatus))) != NULL){
					memset( VPLineStatus, 0, sizeof( *VPLineStatus ) );
					linphone_call_log_statistics(&(solar.ports[i]), VPLineStatus);
					cwmpEvtMsgSetVPLineStatus(evtMsg,VPLineStatus,i);
					free(VPLineStatus);
				}
			}
			cwmpEvtMsgSetEvent(evtMsg,EVT_VOICEPROFILE_LINE_SET_STATUS);
		break;
	}
	sendSock=socket(PF_LOCAL, SOCK_DGRAM, 0);
	bzero(&addr, sizeof(addr));
	addr.sun_family = PF_LOCAL;
	strncpy(addr.sun_path, CWMP_CLINET_PATH, sizeof(CWMP_CLINET_PATH));
	sendto(sendSock, (void*)evtMsg, cwmpEvtMsgSizeof(), 0, (struct sockaddr*)&addr, sizeof(addr));
	close(sendSock);
}

void solarSendChangedToCwmp( void )
{
	/*
	 * Originally, it is polling mode, and we try to implement
	 * interrupt mode. 
	 * Thus, we should fill 'gCwmpEvtMsg' as a received message. 
	 */
	
	gCwmpEvtMsg.event = EVT_VOICEPROFILE_LINE_GET_STATUS;
	
	solarSendResponseToCwmp( &gCwmpEvtMsg );
}
#endif /*#ifdef CONFIG_APP_TR104*/

int
main (int argc, char *argv[])
{
	int i, c;
	int bAutoAnswer;
	
	// read config file 
	app_read_config_file( 1 );

	// destroy old process and create a PID file
	{
		int pid_fd;
		FILE *fp;
		char line[20];
		pid_t pid;

		if ((fp = fopen(pidfile, "r")) != NULL) {
			fgets(line, sizeof(line), fp);
			if (sscanf(line, "%d", &pid)) {
				if (pid > 1)
					kill(pid, SIGTERM);
			}
			fclose(fp);
		}

		pid_fd = pidfile_acquire(pidfile);
		if (pid_fd < 0)
			return 0;
		pidfile_write_release(pid_fd);
	}

	rtk_Get_VoIP_Feature();
	g_MaxVoIPPorts = RTK_VOIP_CH_NUM(g_VoIP_Feature);
	//RTK_VOIP_SLIC_NUM(g_VoIP_Feature) +RTK_VOIP_DAA_NUM(g_VoIP_Feature);

	if (g_MaxVoIPPorts > MAX_VOIP_PORTS)
	{
		fprintf(stderr, "g_MaxVoIPPorts(%d) > MAX_VOIP_PORTS(%d)!?\n",
			g_MaxVoIPPorts, MAX_VOIP_PORTS);
		return -1;
	}

	if (voip_flash_client_init(&g_pVoIPShare, VOIP_FLASH_WRITE_CLIENT_SOLAR) == -1)
	{
		fprintf(stderr, "voip_flash_client_init failed.\n");
		return -1;
	}

	g_pVoIPCfg = &g_pVoIPShare->voip_cfg;

#ifdef MALLOC_DEBUG
	mwInit("memwatch_solar.log");
	mwDoFlush(1);
//	mwLimit(300000);
#endif

#ifdef RTK_DNS
	if (my_getaddrinfo_init(dbgprintf) != 0)
	{
		fprintf(stderr, "my_getaddrinfo_init failed\n");
		return -1;
	}
#endif

	signal(SIGINT, solar_abort);

	bAutoAnswer = 0;
	while ((c = getopt(argc, argv, "adsq")) != -1)
	{
		switch (c)
		{
		case 'd':
			linphone_core_enable_logs(stdout);
			g_SystemDebug |= DEBUG_B_SOLAR_D;
			break;
		case 's':
			linphone_core_enable_logs(stdout);
			stun_message_on = 1;
			break;
		case 'q':
			rtk_DisableRingFXS(TRUE);
			break;
		case 'a':
			bAutoAnswer = 1;
			break;
		default:
#if 0
			printf("usage: solar [option]\n");
			printf("	-d: show debug message\n");
			printf("	-q: quiet mode (no FXS ring)\n");
			return 0;
#else
			// do nothing
			break;
#endif
		}
	}

	// TH: add for ethernet DSP booting
	if (RTK_VOIP_ETHERNET_DSP_CHECK(g_VoIP_Feature))
	{
		for (i=0; i < DSP_DEVICE_NUMBER(g_VoIP_Feature); i++)
		{
			printf("Wait DSP%d booting ......\n", i);
			fflush(stdout);//force above message to display before while loop
			while(!rtk_CheckDspAllSoftwareReady(i));
			printf("DSP phy_id%d All Software Ready!\n", i);
			if (i == 0)
				rtk_SetDspIdToDsp(15);  // force DSP to gen mido interrupt
			rtk_SetDspIdToDsp(i);
		}
	}

	for (i=0; i<g_MaxVoIPPorts; i++)
	{
		/* init */
		rtk_InitDSP(i);
#if 0
		rtk_SetRingFXS(i, TRUE);
		sleep(1);
		rtk_SetRingFXS(i, FALSE);
#endif
	}

	// build codec table by current voip feature
	g_nMaxCodec = BuildSupportedCodecMap();

#ifdef CONFIG_RTK_VOIP_DECT_SITEL_SUPPORT
	dect_api_S2R_init();
#endif
#ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT
	dspg_cmbs_init();
#endif

	app_init(&solar);

#ifdef CONFIG_RTK_VOIP_IP_PHONE
#ifdef UI_EVENTS_USE_IPC_MODE
	InitializeUIEvent();
#endif
#endif

	// set debug flag
	solar.bAutoAnswer = bAutoAnswer;

	app_run(&solar);

	app_close(&solar);

#ifdef RTK_DNS
	my_getaddrinfo_destroy();
#endif

#ifdef MALLOC_DEBUG
	mwTerm();
#endif

	voip_flash_client_close();
	return 0 ;
}

