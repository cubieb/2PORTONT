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

#ifdef OPENWRT_BUILD
#include <net/if.h>
#include "voip_flash.h"
#endif
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
#include <syslog.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/ioctl.h>
#ifndef OPENWRT_BUILD
#include <rtk/utility.h>
#endif
#include <linux/sockios.h>  
#include <linux/ethtool.h>



#include "eXosip2/eXosip.h"
#include "eXosip2.h"
#include "linphonecore.h"
#include "sdphandler.h"
#include "ortp.h"
//#include "stun_api.h"
#ifdef RTK_DNS
#include "dns_api.h"
#endif
#ifdef CONFIG_DEFAULTS_KERNEL_3_10
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#else
#include "netlink.h"   
#include "rtnetlink.h"
#endif
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

#ifdef CONFIG_RTK_VOIP_TR104
#include <sys/un.h>
#include "cwmpevt.h"
#endif /*CONFIG_RTK_VOIP_TR104*/

#include "alarm.h"
#include "pstnroute.h"
#include "icmp.h"
#include "rcm_timer.h"
#include "rcm_voiplog.h"

#ifdef CONFIG_RTK_VOIP_SIP_TLS
#include	<netinet/tcp.h>
#include "eXosip_tls.h"
extern tlsObject_t sipTLSObj[MAX_VOIP_PORTS][MAX_PROXY];
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

#ifdef CONFIG_RTK_VOIP_DECT_SITEL_SUPPORT
#include "si_dect_api.h"
#endif

#include "rcm_sim_defs.h"
#include "sysconfig.h"
#include "rcm_customize.h"

// ==============================
// DEFINE & MACRO
// ==============================
#define CONFIG_NAME     "/etc/solar.conf"
char *FIFO_CONTROL=NULL;
#define FIFO_WATCHDOG	"/var/run/solar_watchdog.fifo"
/*+++added by Jack for communicate with cwmpclient+++*/
#ifdef CONFIG_RTK_VOIP_TR104
#define SOLAR_CHANNEL_PATH "/var/solar.chanl"
#define CWMP_CLINET_PATH "/var/cwmpclient.chanl"
#endif
/*---end---*/

// ==============================
// function prototype
// ==============================
static void init_g_state( TAPP *pApp );
extern gboolean linphonec_parse_command_line(LinphoneCore *lc, uint32 ssid, const gchar *cl);
void linphone_core_process_event(TAPP *pApp, eXosip_event_t *ev);

int voip_flash_server_init_variables(void) __attribute__((weak));
int voip_flash_server_update(void) __attribute__((weak));

void tr104_auto_test_hardcopy(void);

void rcm_start_IVR(void) __attribute__((weak));
void rcm_netlink_handler(int netlink_desc) __attribute__((weak));
void rcm_sim_stopcheck(void) __attribute__((weak));
void rcm_voip_flash_update(void) __attribute__((weak));
//void rcm_voip_dsp_booting(void) __attribute__((weak));
void rcm_retreive_voip_if_name(char *if_name_dst, int if_name_max_len) __attribute__((weak));
void rcm_sim_pid_proc(void) __attribute__((weak));

void rcm_linphone_register_init(TAPP *pApp); /* register to sip server */
void rcm_linphone_sip_restart(TAPP *pApp);
void rcm_init_linphone_voip_interface(void);
void rcm_set_linphone_conctrol_fifo(void);

void rcm_set_sim_conctrol_fifo(void);
void rcm_init_sim_voip_interface(void);
int rcm_set_linphone_pidfile(void);
// ==============================
// global variable 
// ============================== 
TAPP solar;
TAPP *solar_ptr = &solar;
int g_MaxVoIPPorts = 0;
int g_nMaxCodec = 0;
voipCfgParam_t *g_pVoIPCfg = NULL;
#ifdef VOIP_FLASH_ARCH	//Alex, original architecture
voip_flash_share_t *g_pVoIPShare = NULL;
#endif	//VOIP_FLASH_ARCH
char *pidfile =NULL;
int g_mapSupportedCodec[_CODEC_MAX] = {_CODEC_MAX};

extern char VersionStr[];

SYSTEMSTSTE g_sys_state[MAX_VOIP_PORTS];
SESSIONSTATE c_session_state[MAX_VOIP_PORTS][MAX_SS];
gboolean c_active_session[MAX_VOIP_PORTS][MAX_SS];
char input[MAX_VOIP_PORTS][LINE_MAX_LEN];

SystemParam_t g_SystemParam;
#ifdef CONFIG_RTK_VOIP_TR104
cwmpEvtMsg rcm_tr104_st; 
#endif


/* support RCM WATCHDOG */
int RCM_WATCHDOG_pipe=0;
int RCM_WATCHDOG_period=0;
char *wdt_fifo=NULL;

//[SD6, bohungwu, exosip 3.5 integration 
extern eXosip_t eXosip;
//]

extern int rcm_country_para_update(voipCfgParam_t * voip_ptr);


// ==============================
// static variable 
// ============================== 
//static int fax_counter[MAX_VOIP_PORTS] = {0};
/*+++added by Jack for communicate with cwmpclient+++*/
#ifdef CONFIG_RTK_VOIP_TR104
static int ipcSocket = 0;
static cwmpEvtMsg gCwmpEvtMsg;
#endif
/*---end---*/

void SetSessionState( guint32 chid , guint32 ssid , SESSIONSTATE value )
{
	c_session_state[chid][ssid] = value ;
	//printf("\r\n SetSessionState chid %d ssid %d  status %d\n",chid,ssid,value);
}

void SetCallState( guint32 chid , guint32 ssid ,LCState *callstate, LCState value ) {
        //printf("\r\n SetCallState chid %d ssid %d  status %d\n",chid,ssid,value);
	*callstate = value;
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
		rcm_tapi_SetTranSessionID( chid , ssid ) ;
		c_active_session[chid][ssid] = TRUE ;
		c_active_session[chid][s] = FALSE ;
	}
	else if (value == FALSE)
	{
		rcm_tapi_SetTranSessionID( chid , s ) ;
		c_active_session[chid][ssid] = FALSE ;
	}
	else 
	{
		// on hook
		rcm_tapi_SetTranSessionID(chid, 255);//set 255 to clean resource.
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
//printf("\r\n SetSysState chid %d , state %d\n",chid,state);
	g_sys_state[chid] = state ;
	// update LED display 
	UpdateLedDisplayMode( &solar.ports[ chid ] );	// not good style 
}

SYSTEMSTSTE GetSysState( guint32 chid )
{
//printf(" GetSysState chid [%d] is ",chid,g_sys_state[chid]);
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
		if( lc ->default_proxy && lc->default_proxy->registered )	// proxy registered 
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
	//strcpy(src_ptr, "call ");
	//strcat(src_ptr, lc->dial_code);
	sprintf(src_ptr,"%s %s","call",lc->dial_code);	
	digitcode_init(lc);
}

void Local_CallInvite(LinphoneCore *lc, guint32 ssid, char *src_ptr)
{
	SetSessionState(lc->chid, ssid, SS_STATE_CALLOUT);
	//strcpy(src_ptr, "localcall ");
	//strcat(src_ptr, lc->dial_code);
	sprintf(src_ptr,"%s %s","localcall",lc->dial_code);
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
	ret = rcm_tapi_GetPhoneState(&stVoipCfg);
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

	rcm_tapi_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
	usleep(500000); // Audio Code Issue: after phone off-hook, wait for a second,and then play IVR.

#if 0
	//if (lc->chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
	if( RTK_VOIP_IS_DAA_CH( lc->chid, g_VoIP_Feature ) )
		rcm_tapi_FXO_offhook(lc->chid);
#endif

	rcm_tapi_IvrStartPlaying(lc->chid, ssid,IVR_DIR_LOCAL, text);
	lc->bPlayIVR = 1;
#else

#if 0
	//if (lc->chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
	if( RTK_VOIP_IS_DAA_CH( lc->chid, g_VoIP_Feature ) )
		rcm_tapi_FXO_offhook(lc->chid);
#endif

	rcm_tapi_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
#endif
}

#endif


/* for different Country, need check some status to chose dial tone */
int get_dialtone_type(LinphoneCore *lc){

/* country index please reference web_voip_tone.c*/
	voipCfgPortParam_t *voip_ptr = &g_pVoIPCfg->ports[lc->chid];
	
	switch(g_pVoIPCfg->tone_of_country){
		case TONE_GERMANY: /*GERMANY */ 
				//for GERMANY country,  if enable (CFU) Call forward, play special dial tone
				if(strlen(voip_ptr->uc_forward)!=0)					
					return DSPCODEC_TONE_STUTTERDIAL;  //in dsp image,GERMANY  stutterdial = special dial tone
				else
					return DSPCODEC_TONE_DIAL;							
			break;
		case TONE_EXT1:
		/* CUSTOMER_COUNTRY_GUATEMALA */
		if(lc->fxsfeatureFlags & FXS_VOICEMAIL_TONE){
			g_message("play FXS_VOICEMAIL_TONE \n");
			return DSPCODEC_TONE_MESSAGE_WAITING;
		}

		if(lc->fxsfeatureFlags & FXS_SPECIAL_DIAL_TONE){
			g_message("DSPCODEC_TONE_CUSTOM_TONE1 \n");
			return DSPCODEC_TONE_CUSTOM_TONE1;
		}
		else{
			g_message("DSPCODEC_TONE_DIAL \n");
			return DSPCODEC_TONE_DIAL;
		}
		
		default :
			return DSPCODEC_TONE_DIAL;
			break;
	}


}

void HookAction(LinphoneCore *lc, SIGNSTATE s_sign, char *src_ptr)
{
	uint32 chid;
	SYSTEMSTSTE state;
	int i = 0;
	uint32 hookStatus;
	int bRealOnHook;
	int dialtone_type;
	int another_ssid;
	int active_session;
	int oldfxsflag=0;

	chid = lc->chid;
	state = GetSysState(chid);
	switch( s_sign )
	{
		case SIGN_OFFHOOK :
		#ifdef CHECK_RESOURCE
			if (rcm_tapi_VoIP_resource_check(chid, 0) == VOIP_RESOURCE_UNAVAILABLE)
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

					rcm_tapi_Get_DAA_CallerID(chid, caller_id, caller_date, caller_id_name);

					if (caller_id[0])
						strcpy(lc->caller_id, caller_id);
					else
						strcpy(lc->caller_id, "-");

					if (caller_id_name[0])
						strcpy(lc->caller_id_name, caller_id_name);
				}


				if ((g_pVoIPCfg->ports[lc->chid].hotline_enable==1) && 
					(g_pVoIPCfg->ports[lc->chid].hotline_number[0]))
				{ //support hotline , currently hotline not waiting , Immediately dial out.
				//need test   
					rcm_tapi_SetPlayTone(chid, 0, DSPCODEC_TONE_RINGING, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
					lc->ringstream[0] = TRUE;
					HotlineCallInviteByDialPlan(lc, 0, src_ptr, 
						g_pVoIPCfg->ports[lc->chid].hotline_number);
				}else if ((g_pVoIPCfg->ports[lc->chid].hotline_enable==2) && 
					(g_pVoIPCfg->ports[lc->chid].hotline_number[0])){

				
					if( g_SystemParam.stutter_tone &&
						( ( lc->default_proxy == NULL ) ||
						  ( !lc->default_proxy->registered ) ) )
					{
					//no register , no proxy, play stutterdial tone
//						rcm_tapi_SetPlayTone(chid, 0, DSPCODEC_TONE_STUTTERDIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
					rcm_tapi_SetPlayTone(chid, 0, DSPCODEC_TONE_CONGESTION, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);

					

					} else {
					// voip registread, play dial tone
						rcm_tapi_SetPlayTone(chid, 0, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
					}
					g_message("\r\n start hotline_timer  001\n");
				
						
					
					rcm_timerLaunch(lc->hotline_timer,5*1000); 
					

				}
				//else if (chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
				else if( RTK_VOIP_IS_DAA_CH( chid, g_VoIP_Feature ) )
				{
					if (g_pVoIPCfg->ports[lc->chid].hotline_enable)
					{
						// user dial mode if hotline enable but no number
					#ifdef CONFIG_RTK_VOIP_IVR
						rcm_tapi_SetPlayTone(chid, 0, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
							usleep(500000); // Audio Code Issue: after phone off-hook, wait for a second,and then play IVR.
						rcm_tapi_FXO_offhook(chid);
						if (g_pVoIPCfg->ports[lc->chid].offhook_passwd[0])
						{
							char text[] = {IVR_TEXT_ID_PLEASE_ENTER_PASSWORD, '\0'};
							SetSysState(chid, SYS_STATE_AUTH);
							rcm_tapi_IvrStartPlaying(chid,0, IVR_DIR_LOCAL, text);
						}
						else
						{
							char text[] = {IVR_TEXT_ID_PLZ_ENTER_NUMBER, '\0'};
							rcm_tapi_IvrStartPlaying(chid, 0,IVR_DIR_LOCAL, text);
						}
						lc->bPlayIVR = 1;
					#else
						rcm_tapi_FXO_offhook(chid);
						rcm_tapi_SetPlayTone(chid, 0, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
					#endif
					}
					else // auto dial to FXS_0 if no hotline
					{
						char sz_fxs_0[40];
						sprintf(sz_fxs_0, "sip:fxo%u_to_fxs@%s:%u",
							chid - RTK_VOIP_DAA_CH_OFFSET(g_VoIP_Feature) /*RTK_VOIP_SLIC_NUM(g_VoIP_Feature)*/,solar_ptr->bound_voip_ipaddress, g_pVoIPCfg->ports[0].sip_port);
						strcpy(lc->dial_code, sz_fxs_0);
						Local_CallInvite(lc,0,src_ptr);

					}
				}
#ifdef CONFIG_RTK_VOIP_IVR
				else if (g_pVoIPCfg->ports[lc->chid].offhook_passwd[0])
				{
					char text[] = {IVR_TEXT_ID_PLEASE_ENTER_PASSWORD, '\0'};

					SetActiveSession(chid, 0, TRUE);
					SetSysState(chid, SYS_STATE_AUTH);
					rcm_tapi_SetPlayTone(chid, 0, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
					usleep(500000); // Audio Code Issue: after phone off-hook, wait for a second,and then play IVR.
					rcm_tapi_IvrStartPlaying(chid,0, IVR_DIR_LOCAL, text);
					lc->bPlayIVR = 1;
				}
#endif
				else
				{
//				printf("g_SystemParam.stutter_tone is %d\n",g_SystemParam.stutter_tone);
					if( g_SystemParam.stutter_tone &&
						( ( lc->default_proxy == NULL ) ||
						  ( !lc->default_proxy->registered ) ) )
					{
						//no register , no proxy, play stutterdial tone
						printf("play DSPCODEC_TONE_STUTTERDIAL \n");
//						rcm_tapi_SetPlayTone(chid, 0, DSPCODEC_TONE_STUTTERDIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
					rcm_tapi_SetPlayTone(chid, 0, DSPCODEC_TONE_CONGESTION, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);

					} else {
						// voip registread, play dial tone
//						printf("play DSPCODEC_TONE_DIAL \n");
						//callwaiting tone check.
						dialtone_type=get_dialtone_type(lc);
						rcm_tapi_SetPlayTone(chid, 0, dialtone_type, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
					
						
					}
					g_message("\r\n start busytone_timer  009\n");
					rcm_active_BusyWrongTone_timer(lc);
				}
			}
			else if ( state == SYS_STATE_DAA_RING )
			{
				SetActiveSession(chid, 0, TRUE);
				if (rcm_tapi_DAA_off_hook(lc->chid) == 0xff)
				{
					/* PSTN line not connect or line busy */
					SetSysState(lc->chid, SYS_STATE_EDIT_ERROR);
				    rcm_tapi_SetPlayTone(lc->chid, 0, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
				}
				else
				{
					SetSysState(chid, SYS_STATE_DAA_CONNECT);
					SetSessionState(chid, 0, SS_STATE_CALLEE);
				}
			}
			else if( state == SYS_STATE_RING )
			{
				active_session = GetActiveSession(lc->chid);
				printf("answer the active session is %d\n",active_session);
				if(active_session==1)
					strcpy( src_ptr , "answer2" ) ;
				else
				strcpy( src_ptr , "answer" ) ;
			}
			else if(state ==SYS_STATE_CONNECT){
				/* check user offhook the phone , test must stop */


#if defined (CONFIG_RTK_VOIP_TR104) && defined (CONFIG_E8B)
				if(rcm_tr104_st.e8c_autotest.enable==1) {
					rcm_tr104_st.e8c_autotest.enable=0;
					rcm_tr104_st.e8c_autotest.TestStatus=2; /*Complete*/
					rcm_tr104_st.e8c_autotest.CallerFailReason=CALLER_OFFHOOK_RELEASE;
					rcm_tr104_st.e8c_autotest.CalledFailReason=CALLER_OFFHOOK_RELEASE;
					rcm_tr104_st.e8c_autotest.Conclusion=1;/*fail*/
					rcm_timerCancel(lc->e8c_tr104_timer);
					linphone_core_terminate_dialog(lc, 0, NULL);// terminal sip (BYE)

				}
#endif
				//eric add: offhook to answer unhold phone

				active_session = GetActiveSession(lc->chid);
				g_message("1 answer the active session is %d\n",active_session);

				//if(lc->call[active_session]){
					//printf("lc->call[active_session]->state is %d\n",lc->call[active_session]->state);

				//}
						
				if(linphone_core_offhood_event(lc, active_session)==0)
				{
					SetSysState(lc->chid, SYS_STATE_CONNECT);
					
					digitcode_init(lc);
				}

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
			
			if (state == SYS_STATE_DAA_CONNECT) //fxo
			{
				rcm_tapi_DAA_on_hook(chid);
			}
#ifdef ATTENDED_TRANSFER
//support SIP REFER
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
				//onhook, stop 3way
				memset(&stVoipMgr3WayCfg, 0, sizeof(stVoipMgr3WayCfg));
				stVoipMgr3WayCfg.ch_id = chid;
				stVoipMgr3WayCfg.enable = FALSE;
				rcm_tapi_SetConference(&stVoipMgr3WayCfg);
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
				rcm_tapi_IvrStopPlaying( chid ,0);
			}
			
			if (lc->bPlayIVR)
			{
				lc->bPlayIVR = 0;
				rcm_tapi_IvrStopPlaying(chid,0);
			}
#endif

		//	printf("bRealOnHook is %d\n",bRealOnHook);

		//SYS_STATE_CONNECT
		
		//read chid active session number.
		active_session = GetActiveSession(lc->chid);
		//printf("active_session is %d\n",active_session);
		if (active_session == -1)
			active_session = 0;

		another_ssid = (active_session == 0) ? 1 : 0;
		g_message("another_ssid is %d\n",another_ssid);
		g_message(" ssid =0 , status is %d\n",GetSessionState(lc->chid,0));
		g_message(" ssid =1 , status is %d\n",GetSessionState(lc->chid,1));

		
			for (i = 0; i < MAX_SS; i++)
			{
				rcm_tapi_SetRtpSessionState(chid, i, rtp_session_inactive);	// reset all rtp
				if (bRealOnHook) // reset all tone
						rcm_tapi_SetPlayTone(chid, i, DSPCODEC_TONE_HOLD, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);

			}
			//terminal active session.
			//if in 3way conf, onhook to disconnect all session.
			if(	state!=SYS_STATE_IN3WAY){
			linphone_core_terminate_dialog(lc,active_session,NULL);
			}else{
			for (i = 0; i < MAX_SS; i++)
			{
						linphone_core_terminate_dialog(lc, i, NULL);
					}
			}

			//if(lc->call[another_ssid]){

				//printf("lc->call[another_ssid]->state is %d \n",lc->call[another_ssid]->state);
			//}

			
			//check other ssid
			if((SS_STATE_CALLIN==GetSessionState(lc->chid,another_ssid))&&(lc->call[another_ssid])&&(lc->call[another_ssid]->state==LCStateRinging)){
				g_message("antoer call incoming still incoming \n");					
					
				SetSysState(chid,SYS_STATE_RING);
						/* play the ring */
				lc->ringstream[another_ssid] = TRUE;
				rtk_SetRingFXS(chid, 1);
				SetActiveSession(lc->chid, another_ssid, TRUE);

			 }else if(((SS_STATE_CALLER==GetSessionState(lc->chid,another_ssid))||(SS_STATE_CALLEE==GetSessionState(lc->chid,another_ssid)))&&(lc->call[another_ssid])){
				SetSysState(chid,SYS_STATE_CONNECT);

					/* play the ring */
				lc->ringstream[another_ssid] = TRUE;
				rtk_SetRingFXS(chid, 1);
				SetActiveSession(lc->chid, another_ssid, TRUE);

			}else{
				g_message("no antoer call inoming \n");
				linphone_core_terminate_dialog(lc,another_ssid,NULL);
				SetSysState(chid, SYS_STATE_IDLE);
				SetActiveSession(chid, -1, -1);	
			}
		
			

#if 0

			for (i = 0; i < MAX_SS; i++)
			{
				rcm_tapi_SetRtpSessionState(chid, i, rtp_session_inactive);	// reset all rtp
#ifdef ATTENDED_TRANSFER
				if (GetSysState(chid) != SYS_STATE_TRANSFER_ONHOOK)
#endif
				linphone_core_terminate_dialog(lc, i, NULL);	// reset all sip
				if (bRealOnHook) // reset all tone
					rcm_tapi_SetPlayTone(chid, i, DSPCODEC_TONE_HOLD, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
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
				rcm_tapi_SetPlayTone(chid, 0, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
				SetSysState(chid, SYS_STATE_EDIT_ERROR);
			}
#endif

			// reset session resource
//not need			SetActiveSession(chid, -1, -1);	
			g_message("\r\n ****stop offhook_timer 001 \n");
			//reset all timer 
			rcm_timerCancel(lc->offhook_timer);
			rcm_timerCancel(lc->dialtimeout_timer);
			rcm_timerCancel(lc->flashtimeout_timer);
			rcm_timerCancel(lc->busytone_timer);
			rcm_timerCancel(lc->stop_alltone_timer);
			rcm_timerCancel(lc->hotline_timer);
			oldfxsflag=lc->fxsfeatureFlags;
			lc->fxsfeatureFlags=0;
			/* only record special dial tone */
			if(oldfxsflag & FXS_SPECIAL_DIAL_TONE)
				lc->fxsfeatureFlags|=FXS_SPECIAL_DIAL_TONE;
			
			// reset variable
			digitcode_init(lc);
//			fax_counter[chid] = 0;
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
	rtp_config_t rtp_config;
	rtcp_config_t rtcp_config;
	t38udp_config_t t38udp_config;
	payloadtype_config_t codec_config;
	t38_payloadtype_config_t t38_config;

	if (GetRtpCfgFromSession(&rtp_config, &rtcp_config, &t38udp_config, 
							&codec_config, &t38_config, lc, ssid) != 0)
	{
		return -1;
	}

	// TODO: easy style config?

	// rtp config
	rtp_cfg->ch_id = lc->chid;
	rtp_cfg->m_id = ssid;
	rtp_cfg->ip_src_addr = rtp_config.remIp;
	rtp_cfg->ip_dst_addr = rtp_config.extIp;
	rtp_cfg->udp_src_port = rtp_config.remPort;
	rtp_cfg->udp_dst_port = rtp_config.extPort;
//	rtp_cfg->rtcp_src_port;
//	rtp_cfg->rtcp_dst_port;
//	rtp_cfg->protocol;
#ifdef SUPPORT_VOICE_QOS
	rtp_cfg->tos = rtp_config.tos;
#endif
	rtp_cfg->rfc2833_payload_type_local = rtp_config.rfc2833_payload_type_local;
	rtp_cfg->rfc2833_payload_type_remote = rtp_config.rfc2833_payload_type_remote;
	// codec config
	rtp_cfg->local_pt = codec_config.local_pt;
	rtp_cfg->remote_pt = codec_config.remote_pt;
	rtp_cfg->uLocalPktFormat = codec_config.uLocalPktFormat;
	rtp_cfg->uRemotePktFormat = codec_config.uRemotePktFormat;
	rtp_cfg->local_pt_vbd = codec_config.local_pt_vbd;
	rtp_cfg->remote_pt_vbd = codec_config.remote_pt_vbd;
	rtp_cfg->uPktFormat_vbd = codec_config.uPktFormat_vbd;
	rtp_cfg->nG723Type = codec_config.nG723Type;
	rtp_cfg->nLocalFramePerPacket = codec_config.nLocalFramePerPacket;
	rtp_cfg->nRemoteFramePerPacket = codec_config.nRemoteFramePerPacket;
	rtp_cfg->nFramePerPacket_vbd = codec_config.nFramePerPacket_vbd;
	rtp_cfg->bVAD = codec_config.bVAD;
	rtp_cfg->bPLC = lc->rtp_conf.bPLC;
	rtp_cfg->nJitterDelay = codec_config.nJitterDelay;
	rtp_cfg->nMaxDelay = codec_config.nMaxDelay;
	rtp_cfg->nMaxStrictDelay = codec_config.nMaxStrictDelay;
	rtp_cfg->nJitterFactor = codec_config.nJitterFactor;
	// RTP session state
//	rtp_cfg->state;
//	rtp_cfg->result;

	return 0;
}

int session_2_session_event(LinphoneCore *lc, int s_case)
{
	uint32 chid;
	int active_session;

	chid = lc->chid;
#if 0//eric: not need check here	
	if (GetSysState(chid) == SYS_STATE_IN3WAY)
		return -1;
#endif
	active_session = GetActiveSession(chid);
	if (lc->call[active_session] &&
		(lc->call[active_session]->state == LCStateBeHold ||
		lc->call[active_session]->state == LCStateBeHoldACK))
	{
		// do nothing
		return -1;
	}

	g_message("\r\n session_2_session_event %d\n",s_case);

	if( s_case == 1 )
	{
		SESSIONSTATE ss = GetSessionState( chid , 0 ) ;

#if 1
//in 3way conference , not support flash + 1 
		if (GetSysState(chid) == SYS_STATE_IN3WAY)
			return -1;
#endif


		// cancel pending call before switch session
		if (GetSessionState(chid, 1) == SS_STATE_CALLOUT)
		{
			linphone_core_terminate_dialog(lc, 1, NULL);
		}
		
		g_message("*** run linphone_core_onhood_event 001 \n");
		if (linphone_core_onhood_event(lc, 1) != 0)
		{
			// onhood failed, user need try again
			return -1;
		}
		else if( ss == SS_STATE_IDLE )
		{
			SetSysState( chid , SYS_STATE_CONNECT_EDIT ) ;
		}
		else if( ss == SS_STATE_CALLIN )//receive incoming call waiting
		{
			rcm_tapi_SetPlayTone( chid , 1, DSPCODEC_TONE_CALL_WAITING , FALSE , DSPCODEC_TONEDIRECTION_LOCAL ) ;
			linphone_core_accept_dialog(lc, 0, NULL);
		}else if (lc->call[0]!=NULL){
			g_message("old ssid %d state is %d",0,lc->call[0]->state);
			SetCallState(lc->chid,0,&(lc->call[0]->state),LCStateBeResume); 
			
		}
#if 0
		else if (linphone_core_offhood_event(lc, 0) != 0) //TBD: eric, when switch call, hope sequence to 
		{
			// offhood failed, user need try again
			return -1;
		}
#endif		
		
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
		else if( ss == SS_STATE_CALLIN )//receive incoming call waiting
		{
			rcm_tapi_SetPlayTone(chid, 0, DSPCODEC_TONE_CALL_WAITING, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
			linphone_core_accept_dialog(lc, 1, NULL);
		}else if (lc->call[1]!=NULL){
			g_message("old ssid %d state is %d",1,lc->call[1]->state);
			SetCallState(lc->chid,1,&(lc->call[1]->state),LCStateBeResume); 
			
		}

		
#if 0	
		else if (linphone_core_offhood_event(lc, 1) != 0)//TBD: eric, when switch call, hope sequence to 
		{
			// offhood failed, user need try again
			return -1;
		}
#endif		
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
			rcm_tapi_VoIP_resource_check(chid, stVoipMgr3WayCfg.rtp_cfg[active_session ? 0 : 1].uLocalPktFormat))
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
				case LCStateBeHold:
				case LCStateBeHoldACK:
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
	}else if( s_case == 4 ) //close 3way conference
	{
		int i;
		TstVoipMgr3WayCfg stVoipMgr3WayCfg;

		memset(&stVoipMgr3WayCfg, 0, sizeof(stVoipMgr3WayCfg));
		stVoipMgr3WayCfg.ch_id = chid;
		stVoipMgr3WayCfg.enable = FALSE;
		for (i=0; i<MAX_SS; i++)
		{
			GetRtpCfg(&stVoipMgr3WayCfg.rtp_cfg[i], lc, i);
		}

		if (VOIP_RESOURCE_UNAVAILABLE == 
			rcm_tapi_VoIP_resource_check(chid, stVoipMgr3WayCfg.rtp_cfg[active_session ? 0 : 1].uLocalPktFormat))
		{
		#ifdef CHECK_RESOURCE
		#ifdef CONFIG_RTK_VOIP_IVR
			// play ivr only, don't play busy tone here
			linphone_no_resource_handling(lc, active_session);
		#endif
		#endif
			return -1;
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
	//get current time
	//osip_gettimeofday(&lc->auto_dial_start, NULL);

	if (g_pVoIPCfg->auto_dial){
		//osip_gettimeofday(&lc->auto_dial_start, NULL);
		//ericchung: when dial is timeout ,must send to SIP server
		rcm_timerLaunch(lc->dialtimeout_timer,(g_pVoIPCfg->auto_dial & AUTO_DIAL_TIME)*1000);
	}
		g_message("lc->dial_data is %s\n",lc->dial_data);
		g_message("lc->dial_code is %s\n",lc->dial_code);
	
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
		rcm_tapi_SetPlayTone( lc ->chid, GetActiveSession( lc ->chid ), 
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

//src_ptr finial input string
int CallDigitEdit(LinphoneCore *lc, uint32 *ssid, SIGNSTATE s_sign, char *src_ptr)
{
	int active_session;
	SYSTEMSTSTE state;
	int si;
	const unsigned char *pszPSTNRouting = NULL;
	int another_ssid;

	//read chid active session number.
	active_session = GetActiveSession(lc->chid);
	if (active_session == -1)
		active_session = 0;
		
	*ssid = active_session;
	state = GetSysState(lc->chid);

	g_message("\r\nCallDigitEdit,chid %d ssid %d,state %d, s_sign %d,src_ptr %s\n",
				lc->chid,active_session,state,s_sign,src_ptr);

	switch (state)
	{
#ifdef CONFIG_RTK_VOIP_IVR
	case SYS_STATE_AUTH: //ivr
		if (lc->bPlayIVR)
		{
			lc->bPlayIVR = 0;
			rcm_tapi_IvrStopPlaying(lc->chid ,*ssid);
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
				{
#ifdef CONFIG_RTK_VOIP_DIALPLAN
					if (passwd[i] != g_pVoIPCfg->ports[lc->chid].offhook_passwd[i] &&
						! (passwd[i] == '*' && g_pVoIPCfg->ports[lc->chid].offhook_passwd[i] == '.'))
						break;
#else
					if (passwd[i] != g_pVoIPCfg->ports[lc->chid].offhook_passwd[i] &&
						! (passwd[i] == '.' && g_pVoIPCfg->ports[lc->chid].offhook_passwd[i] == '*'))
						break;
#endif
				} //for

				if (passwd[i] == 0 && g_pVoIPCfg->ports[lc->chid].offhook_passwd[i] == 0)
				{
					char text[] = {IVR_TEXT_ID_PLZ_ENTER_NUMBER, '\0'};

					// auth success
					rcm_tapi_IvrStartPlaying(lc->chid,*ssid, IVR_DIR_LOCAL, text);
					lc->bPlayIVR = 1;
//					rcm_tapi_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
					digitcode_init(lc);
//					printf("CallDigitEdit , set off_hook_start timer -1 \n");

					g_message("\r\n start off_hook timer 002\n");
					osip_gettimeofday(&lc->off_hook_start, NULL);
					rcm_timerCancel(lc->dialtimeout_timer);
					rcm_active_BusyWrongTone_timer(lc);
					SetSysState(lc->chid, SYS_STATE_EDIT);
				}
				else
				{
					// auth failed
					char text[] = {IVR_TEXT_ID_PLEASE_ENTER_PASSWORD, '\0'};

					rcm_tapi_IvrStartPlaying(lc->chid, *ssid,IVR_DIR_LOCAL, text);
					lc->bPlayIVR = 1;
					digitcode_init(lc);

					g_message("\r\n start off_hook timer 003\n");
					rcm_timerCancel(lc->dialtimeout_timer);
 
					rcm_active_BusyWrongTone_timer(lc);
				}
			}
		}
		else
		{
			g_message(" timerCancel offhook_timer auth 002\n");
			rcm_timerCancel(lc->offhook_timer);
			rcm_timerCancel(lc->busytone_timer);
			rcm_timerCancel(lc->stop_alltone_timer);
			edit_invite_digit(lc, s_sign);	// add to auth numbers
		}
		break;
#endif //CONFIG_RTK_VOIP_IVR

	case SYS_STATE_EDIT: 
	{
		if (s_sign == SIGN_FLASHHOOK) {

			if (lc->call[*ssid])
				linphone_core_terminate_dialog(lc, *ssid, NULL);

			if( g_SystemParam.stutter_tone &&
				( ( lc->default_proxy == NULL ) || ( !lc->default_proxy->registered ) ) ) {

//				rcm_tapi_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_STUTTERDIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
			rcm_tapi_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_CONGESTION, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);


			} else {

				rcm_tapi_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
			}

			digitcode_init(lc);

			//second call..
			g_message("\r\n start off_hook timer 003\n");
			osip_gettimeofday(&lc->off_hook_start, NULL);
			rcm_active_BusyWrongTone_timer(lc);

		} else if (s_sign == SIGN_HASH) {

#if defined( CONFIG_RTK_VOIP_DIALPLAN )
			if (g_pVoIPCfg->auto_dial & AUTO_DIAL_ALWAYS)
				edit_invite_digit(lc, s_sign);
#endif

#if 0 //ericchung: mark temp, because we need support # key in first digits
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

#endif //#if 0 //ericchung: mark temp, because we need support # key in first digits

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
				rcm_tapi_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
				digitcode_init(lc);
				osip_gettimeofday(&lc->off_hook_start, NULL);
				return 0;
			}
#endif
#endif
		
#ifdef CONFIG_RTK_VOIP_IVR
			if (lc->bPlayIVR) {
				lc->bPlayIVR = 0;
				rcm_tapi_IvrStopPlaying(lc->chid,*ssid);
			}

#if 1
			if( !IsInstructionForIVR_Indirect( lc ) )
#else
			if( !IsInstructionForIVR( lc ) )
#endif
#endif //CONFIG_RTK_VOIP_IVR
			{
#ifdef CONFIG_RTK_VOIP_DIALPLAN
				if (g_pVoIPCfg->auto_dial & AUTO_DIAL_ALWAYS) {

					//(Not check HASH)

					/* allow '#' in middle of dial data. */
					/* turn off dial tone. */
					if(lc->digit_index==0){
						rcm_tapi_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_DIAL, 
											 FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
					}

					g_message("\r\n check prefixkey 002 ******\n");

					//TODO Why put feature key check in CONFIG_RTK_VOIP_DIALPLAN ?
					if(rcm_checkPrefixKey(lc,s_sign,*ssid,src_ptr)){
						g_message("match phone feature key\n");
						break;
					}

					//if fisrt digit is * # , is feature key , not need immediately dial when press is #
					if((lc->dial_data[0]=='#') || (lc->dial_data[0]=='*')){

						if( DialPlanTryCallInvite( lc, *ssid, src_ptr ) ) {
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO 							
							/* Check PSTN routing prefix */
							if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, (const unsigned char*)src_ptr, 1 ) ) ) {
								goto label_fxo_call_on_SYS_STATE_EDIT;
							}
#endif							
						}

					} else {
						/* no match dialplan */
						rcm_timerCancel(lc->dialtimeout_timer);

						strcpy(lc->dial_data,lc->dial_code); /* remove last '#' */
						//printf("lc->dial_data_index is %d\n",lc->dial_data_index);
						if(lc->dial_data_index>0){
						/* check dial plan */
						if(	DoDialPlanCheck(lc)==0){
							//printf("\r\n cancel dial out \n");
							rcm_tapi_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_BUSY, 
												 TRUE, DSPCODEC_TONEDIRECTION_LOCAL);		
							SetSysState(lc->chid, SYS_STATE_EDIT_ERROR);
							break;		
						}
											
						CallInviteByDialPlan( lc, *ssid, src_ptr );
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO							
						/* Check PSTN routing prefix */
						if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, (const unsigned char*)src_ptr, 1 ) ) ) {
							goto label_fxo_call_on_SYS_STATE_EDIT;
						}
#endif							
						}
						
					}
				} else {

					/* call invite immediately */
				
					g_message("\r\n timerCancel dialtimeout_timer\n");
					rcm_timerCancel(lc->dialtimeout_timer);
				
					if(	DoDialPlanCheck(lc)==0){
						//printf("\r\n cancel dial out \n");
						rcm_tapi_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);		
						SetSysState(lc->chid, SYS_STATE_EDIT_ERROR);
						break;		
					}
					CallInviteByDialPlan( lc, *ssid, src_ptr );
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO					
					/* Check PSTN routing prefix */
					if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, (const unsigned char*)src_ptr, 1 ) ) ) 
						goto label_fxo_call_on_SYS_STATE_EDIT;
#endif					
				}
#else
				CallInvite(lc, *ssid, src_ptr);
#endif /* CONFIG_RTK_VOIP_DIALPLAN */
			}
		}
#ifdef CONFIG_RTK_VOIP_DIALPLAN
		else if (s_sign == SIGN_AUTODIAL)
		{

			g_message("\r\n CallInviteByDialPlan by SIGN_AUTODIAL");

			rcm_timerCancel(lc->dialtimeout_timer);			
			rcm_timerCancel(lc->offhook_timer);
			rcm_timerCancel(lc->busytone_timer);
			rcm_timerCancel(lc->stop_alltone_timer);

			/* Check E8C feature key */
			if( rcm_checkPrefixKey( lc,s_sign,*ssid,src_ptr )){

				//TODO: why *62 "Caller ID Display" doesn't need to DoDialPlanCheck()? 

				printf("match phone feature key\n");
				break; /* case SYS_STATE_EDIT: */
			}

			/* Check Dial plan (Digit map) */
			if(	DoDialPlanCheck(lc)==0 ) {

				/* Missmatch!! Cancel dial out procedure */
				rcm_tapi_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);			
				SetSysState(lc->chid, SYS_STATE_EDIT_ERROR);
				break; /* case SYS_STATE_EDIT: */
			}

			CallInviteByDialPlan( lc, *ssid, src_ptr );
			g_message("\r\n after CallInviteByDialPlan , src_ptr %s\n",src_ptr);
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
			/* Check PSTN routing prefix */
			if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, (const unsigned char*)src_ptr, 1 ) ) ) 
				goto label_fxo_call_on_SYS_STATE_EDIT;
#endif			
		}
#endif /* CONFIG_RTK_VOIP_DIALPLAN */

		/* If dial number is larger than MAX_DIAL_LENGTH */	//Alex Hung added, 20111124
		else if(lc->dial_data_index >= MAX_DIAL_LENGTH){
			/* call invite immediately */
			g_message("\r\n timerCancel dialtimeout_timer\n");
			rcm_timerCancel(lc->dialtimeout_timer);
			
			if( DoDialPlanCheck(lc)==0){

				/* Missmatch!! Cancel dial out procedure */
				rcm_tapi_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);      
				SetSysState(lc->chid, SYS_STATE_EDIT_ERROR);
				break;      
			}
			CallInviteByDialPlan( lc, *ssid, src_ptr );
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO			
			/* Check PSTN routing prefix */
			if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, (const unsigned char*)src_ptr, 1 ) ) ) 
				goto label_fxo_call_on_SYS_STATE_EDIT;
#endif			
		}
		//End of Added
		else
		{
			//record input digit , record lc->auto_dial_start
			g_message("\r\n call edit_invite_digit 001");
			edit_invite_digit(lc, s_sign);
			
			/* turn off dial tone. */
			//ericching TBD: every time input digits ,re-call stop tone ?? need change it .
			rcm_tapi_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
			//ericchung , cancel offhook_timer			
			g_message("\r\n ****stop offhook_timer 001 \n");
			rcm_timerCancel(lc->offhook_timer);
			rcm_timerCancel(lc->busytone_timer);
			rcm_timerCancel(lc->stop_alltone_timer);
			rcm_timerCancel(lc->hotline_timer);

#ifdef CONFIG_RTK_VOIP_DIALPLAN
			if(rcm_checkInternalPhoneBook(lc,s_sign,*ssid,src_ptr)){
				g_message("dial to local \n");
				rcm_timerCancel(lc->dialtimeout_timer);
				Local_CallInvite(lc, *ssid, src_ptr);
				break;
			}

			g_message("\r\n check prefixkey 001 ******\n");
			if(rcm_checkPrefixKey(lc,s_sign,*ssid,src_ptr)){
				printf("match phone feature key\n");
				break;
			}

			g_message("\r\n DialPlanTryCallInvite 002\n");
			if( DialPlanTryCallInvite( lc, *ssid, src_ptr ) ) {
				/* dial plan do call invite */
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO				
				/* Check PSTN routing prefix */
				if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, (const unsigned char*)src_ptr, 1 ) ) ) 
					goto label_fxo_call_on_SYS_STATE_EDIT;
#endif				
			} else 
#endif
			{
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO			
				if (strcmp(lc->dial_code, g_pVoIPCfg->funckey_pstn) == 0) // dial to pstn funckey_pstn: default is *0
				{
label_fxo_call_on_SYS_STATE_EDIT:
					//if (g_MaxVoIPPorts > RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
					if( RTK_VOIP_DAA_NUM( g_VoIP_Feature ) > 0 )
					{
						char sz_fxo_0[40];
						if( pszPSTNRouting && pszPSTNRouting[ 0 ] ) {
							sprintf(sz_fxo_0, "sip:%s@%s:%d", 
								pszPSTNRouting,solar_ptr->bound_voip_ipaddress,
								g_pVoIPCfg->ports[0].sip_port);
							g_message("sz_fxo_0 is %s\n",sz_fxo_0);
							strcpy(lc->dial_code, sz_fxo_0);
						} else {
							//not need.copy sz_fxo_0

						}
						Local_CallInvite(lc, *ssid, src_ptr);
					}
					else if (rcm_tapi_DAA_off_hook(lc->chid) == 0xff)
					{
						SetSysState( lc->chid , SYS_STATE_EDIT_ERROR );
						rcm_tapi_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
					}
					else
					{
						SetSysState(lc->chid, SYS_STATE_DAA_CONNECT);
						SetSessionState(lc->chid, *ssid, SS_STATE_CALLER);
					}
				}
#endif				
			}
		}
		break; /* case SYS_STATE_EDIT: */
	} 

	case SYS_STATE_EDIT_ERROR:
		if (s_sign == SIGN_FLASHHOOK)
		{
			rcm_tapi_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
			digitcode_init(lc);
			SetSysState(lc->chid, SYS_STATE_EDIT);
			g_message("\r\n start off_hook timer 004\n");
			osip_gettimeofday(&lc->off_hook_start, NULL);
			rcm_timerCancel(lc->dialtimeout_timer);
			rcm_active_BusyWrongTone_timer(lc);
		}
		break;

	case SYS_STATE_CONNECT:
		g_message("\r\n SYS_STATE_CONNECT ssid %d\n ",*ssid);
		
		another_ssid=(*ssid == 0 ? 1 : 0);

		g_message("another_ssid is %d\n",another_ssid);
		g_message(" ssid =0 , status is %d\n",GetSessionState(lc->chid,0));
		g_message(" ssid =1 , status is %d\n",GetSessionState(lc->chid,1));
			
		/*support send Flash hook to sip server ,must enable SIP_FLASHHOOK_INFO */
		if ((s_sign == SIGN_FLASHHOOK)&&(!(g_pVoIPCfg->rfc_flags & SIP_FLASHHOOK_INFO))){
			//another session is incoming call and ringing, 
			//user can chose use R+1 , R+2 to answer the call.
			
			if (GetSessionState(lc->chid, *ssid == 0 ? 1 : 0) == SS_STATE_CALLIN) {
				g_message("\r\n one session is SS_STATE_CALLIN\n");
				if(lc->call[another_ssid]){
					g_message("lc->call[another_ssid] is %d\n",lc->call[another_ssid]->state);
				}
			
				if((lc->call[another_ssid])&&(lc->call[another_ssid]->state==LCStateRinging)){
					g_message(" set flash ssid %d call-state  is %d\n",another_ssid,lc->call[another_ssid]->state);
						lc->fxsfeatureFlags|=FXS_FLEASH_KEY;
						if(g_pVoIPCfg->tone_of_country==TONE_GERMANY)			
							rcm_tapi_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_STUTTERDIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
						else
							rcm_tapi_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
						
						rcm_timerLaunch(lc->flashtimeout_timer,TIMEOUT_FXS_FLASHTKEY); 
					break;
				}
			}else{ //normal flash key, should dial second call..
				if (session_2_session_event(lc, (active_session == 0) ? 2 : 1) == 0)
				{
					*ssid = GetActiveSession(lc->chid);
				g_message("\r\n start off_hook timer 005\n");
				osip_gettimeofday(&lc->off_hook_start, NULL);
				rcm_active_BusyWrongTone_timer(lc);
				}
				digitcode_init(lc);
		
				break;
			}
		}else{
			if(lc->fxsfeatureFlags & FXS_FLEASH_KEY){
				rcm_timerCancel(lc->flashtimeout_timer); 
				//need check country
				if(g_pVoIPCfg->tone_of_country==TONE_GERMANY)
					rcm_tapi_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_STUTTERDIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
				else
					rcm_tapi_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
				
				lc->fxsfeatureFlags &=~FXS_FLEASH_KEY; 
				switch(s_sign){
				case SIGN_KEY2:
					g_message("\r\n answer incoming call ");
		
					//R+2 , second call switch
					if (session_2_session_event(lc, (active_session == 0) ? 2 : 1) == 0){
						*ssid = GetActiveSession(lc->chid);
					}
					digitcode_init(lc);
					break;

				case SIGN_KEY1:
					g_message("\r\nClear active & switch to incoming call.");
					g_message("\r\n active_session is %d\n",active_session);
					//R+2 , second call switch
					//g_message("\r\n GetSessionState %d\n",GetSessionState(lc->chid, active_session));
					g_message("\r\n incoming session is %d\n",(active_session == 0) ? 1 : 0);
					g_message("\r\n GetSessionState %d\n",GetSessionState(lc->chid, active_session));

					rcm_tapi_SetPlayTone( lc->chid , active_session, DSPCODEC_TONE_CALL_WAITING , FALSE , DSPCODEC_TONEDIRECTION_LOCAL ) ;
					//clear current call
					linphone_core_terminate_dialog(lc, active_session, NULL);
					SetSysState(lc->chid, SYS_STATE_IDLE);
					//answer new call
					linphone_core_accept_dialog(lc, (active_session == 0) ? 1 : 0, NULL);
					digitcode_init(lc);
					break;												
				case SIGN_KEY0:
					/* clear callwaiting call , still talk with active call */
					g_message("\r\nClear callwaiting call , still talk with active call\n");
					g_message("\r\n active_session is %d\n",active_session);
					
					g_message("\r\n incoming session is %d\n",(active_session == 0) ? 1 : 0);
					g_message("\r\n GetSessionState %d\n",GetSessionState(lc->chid, active_session));
					
					rcm_tapi_SetPlayTone( lc->chid , active_session, DSPCODEC_TONE_CALL_WAITING , FALSE , DSPCODEC_TONEDIRECTION_LOCAL ) ;
					linphone_core_terminate_dialog(lc, (active_session == 0) ? 1 : 0, NULL);
					digitcode_init(lc);
					break;
				default:
					break;
				}
			}else{
#if 1	//ericchung: enable call transfer key read	
				edit_invite_digit(lc, s_sign);
#endif				
				linphone_core_send_dtmf(lc, active_session, s_sign);
#if 1     //ericchung: enable call transfer key parse
				if (strcmp(lc->dial_code, g_pVoIPCfg->funckey_transfer) == 0)
				{
					if (session_2_session_event(lc, (active_session == 0) ? 2 : 1) == 0)
					{
						*ssid = GetActiveSession(lc->chid);
						SetSysState(lc->chid, SYS_STATE_TRANSFER_EDIT);
					}
					digitcode_init(lc);
					//start call transfer edit 
					osip_gettimeofday(&lc->off_hook_start, NULL);
					g_message("\r\n start off_hook timer 006\n");
					rcm_active_BusyWrongTone_timer(lc);
				}		
#endif				
			}
		}
		break;

//[SD6, ericchung, rcm integration
#if 0	
		if (s_sign == SIGN_FLASHHOOK)
		{
			if (session_2_session_event(lc, (active_session == 0) ? 2 : 1) == 0)
			{
				*ssid = GetActiveSession(lc->chid);
			}
			digitcode_init(lc);
			g_message("\r\n start off_hook timer 005\n");//eric
			osip_gettimeofday(&lc->off_hook_start, NULL);
			rcm_timerLaunch(lc->offhook_timer,(g_pVoIPCfg->off_hook_alarm*1000)); //eric
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
				//start call transfer edit 
				osip_gettimeofday(&lc->off_hook_start, NULL);

				g_message("\r\n start off_hook timer 006\n");
				rcm_timerLaunch(lc->offhook_timer,(g_pVoIPCfg->off_hook_alarm*1000)); 
			}
		}
		break;
//[SD6, ericchung, rcm integration
#endif
	case SYS_STATE_CONNECT_DAA_RING:
		if (s_sign == SIGN_FLASHHOOK)
		{
			// close VoIP
			linphone_core_terminate_dialog(lc, active_session, NULL);
			// Accept PSTN
			*ssid = 0;
			SetActiveSession(lc->chid, *ssid, TRUE);
			if (rcm_tapi_DAA_off_hook(lc->chid) == 0xff)
			{
				/* PSTN line not connect or line busy */
				SetSysState(lc->chid, SYS_STATE_EDIT_ERROR);
			    rcm_tapi_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
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
						rcm_tapi_SetPlayTone(i, active_session, DSPCODEC_TONE_CALL_WAITING, FALSE, 
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
		g_message("\r\n **  CallDigitEdit SYS_STATE_CONNECT_EDIT\n");
		if (s_sign == SIGN_FLASHHOOK)
		{
			// peer not answer the call,still ringing  , iad send cancel to peer.
			// cancel pending call before switch session
			if (GetSessionState(lc->chid, *ssid) == SS_STATE_CALLOUT)
			{
				linphone_core_terminate_dialog(lc, *ssid, NULL);
			}

			rcm_tapi_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
			//if (session_2_session_event(lc, (active_session == 0) ? 2 : 1) == 0)
			//user press flashhook again , cancel second  call dial. un-held first call.
			if(linphone_core_offhood_event(lc, (active_session == 0) ? 1 : 0)==0)
			{
				*ssid = GetActiveSession(lc->chid);

#ifdef CONFIG_RTK_VOIP_IP_PHONE
				S2U_ConnectionEstInd( lc ->chid, *ssid, 4 );
#endif

				SetSysState(lc->chid, SYS_STATE_CONNECT);
			}
			g_message("****stop offhook_timer 020 \n");
			rcm_timerCancel(lc->offhook_timer);
			rcm_timerCancel(lc->busytone_timer);
			rcm_timerCancel(lc->stop_alltone_timer);
			rcm_timerCancel(lc->dialtimeout_timer);
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
				if(lc->digit_index==0){
					rcm_tapi_SetPlayTone(lc->chid, si, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
				}
				g_message("\r\n DialPlanTryCallInvite 003");
				if(rcm_checkPrefixKey(lc,s_sign,si,src_ptr)){
					g_message("match phone feature key\n");
					*ssid = si;
					break;
				}else if(lc->fxsfeatureFlags & FXS_MCID_ACTIVATE){

					//printf("enable MCID active \n");
					//printf("unheld  peer \n");
					//printf("send re-invite with xml to peer \n");
					rcm_timerCancel(lc->offhook_timer);
					rcm_timerCancel(lc->busytone_timer);
					rcm_timerCancel(lc->stop_alltone_timer);	
					
					//printf("\r\n active_session is %d\n",active_session);				
					
					if(linphone_core_offhood_event(lc, (active_session == 0) ? 1 : 0)==0)

					{
						*ssid = GetActiveSession(lc->chid);

						SetSysState(lc->chid, SYS_STATE_CONNECT);
						
					}
	
					digitcode_init(lc);
					break;
				}
				
				//if fisrt digit is * # , is feature key , not need immediately dial when press is #
				if((lc->dial_data[0]=='#')||(lc->dial_data[0]=='*')){
						
				if( DialPlanTryCallInvite( lc, si, src_ptr ) ) {
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO				
					/* Check PSTN routing prefix */
					if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, (const unsigned char*)src_ptr, 1 ) ) ) 
						goto label_fxo_call_on_SYS_STATE_CONNECT_EDIT;
#endif					
					*ssid = si;
				}
				}else{
					rcm_timerCancel(lc->dialtimeout_timer);

					strcpy(lc->dial_data,lc->dial_code);						
					CallInviteByDialPlan( lc, si, src_ptr );
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO					
					/* Check PSTN routing prefix */
					if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, (const unsigned char*)src_ptr, 1 ) ) ) 
						goto label_fxo_call_on_SYS_STATE_EDIT;
#endif					
					*ssid = si;
				}
			}
			else
			{
				/* call invite immediately */
				rcm_timerCancel(lc->dialtimeout_timer);
				CallInviteByDialPlan( lc, si, src_ptr );
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
				/* Check PSTN routing prefix */
				if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, (const unsigned char*)src_ptr, 1 ) ) ) 
					goto label_fxo_call_on_SYS_STATE_CONNECT_EDIT;
#endif
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
			
			if(rcm_checkPrefixKey(lc,s_sign,si,src_ptr)){
				g_message("match phone feature key\n");
				*ssid = si;
				break;
			}
			
			CallInviteByDialPlan( lc, si, src_ptr );
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
			/* Check PSTN routing prefix */
			if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, (const unsigned char*)src_ptr, 1 ) ) ) 
				goto label_fxo_call_on_SYS_STATE_CONNECT_EDIT;
#endif
			*ssid = si;
		}
#endif /* CONFIG_RTK_VOIP_DIALPLAN */
		else
		{
			g_message("****stop offhook_timer 030 \n");
			rcm_timerCancel(lc->offhook_timer);
			rcm_timerCancel(lc->busytone_timer);
			rcm_timerCancel(lc->stop_alltone_timer);
			edit_invite_digit(lc, s_sign);
			rcm_tapi_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_DIAL, FALSE, 
				DSPCODEC_TONEDIRECTION_LOCAL);
			
#ifdef CONFIG_RTK_VOIP_DIALPLAN
			if( ( si = GetIdleSession(lc->chid) ) == -1 ) {
				digitcode_init(lc);	
				return 0;
			} 
			
			if( DialPlanTryCallInvite( lc, si, src_ptr ) ) {
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO				
				/* Check PSTN routing prefix */
				if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, (const unsigned char*)src_ptr, 1 ) ) ) 
					goto label_fxo_call_on_SYS_STATE_CONNECT_EDIT;
#endif				
				*ssid = si;
			} else
#endif
			{
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO			
		    if (strcmp(lc->dial_code, g_pVoIPCfg->funckey_pstn) == 0) 
			{
label_fxo_call_on_SYS_STATE_CONNECT_EDIT:
				//if (g_MaxVoIPPorts > RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
				if( RTK_VOIP_DAA_NUM( g_VoIP_Feature ) > 0 )
				{
					char sz_fxo_0[40];
					if( pszPSTNRouting && pszPSTNRouting[ 0 ] ) {
						sprintf(sz_fxo_0, "sip:%s@%s:%d", 
							pszPSTNRouting,solar_ptr->bound_voip_ipaddress,
							g_pVoIPCfg->ports[0].sip_port);
						strcpy(lc->dial_code, sz_fxo_0);
					} else {
						/* Eric: not need copy */
						//sprintf(sz_fxo_0, "sip:%s:%d", solar_ptr->bound_voip_ipaddress,
						//	g_pVoIPCfg->ports[0].sip_port);
					}

					Local_CallInvite(lc, *ssid, src_ptr);
				}
				else	
			    {
					SetSysState(lc->chid, SYS_STATE_CONNECT_EDIT_ERROR);
			    	rcm_tapi_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
			    }
			}
#endif
		}
			
		}
		break;

	case SYS_STATE_CONNECT_EDIT_ERROR:
		g_message("\r\n **  CallDigitEdit SYS_STATE_CONNECT_EDIT_ERROR\n");

		if (s_sign == SIGN_FLASHHOOK)
		{
			//if (session_2_session_event(lc, (active_session == 0) ? 2 : 1) == 0)

			if (GetSessionState(lc->chid, *ssid) == SS_STATE_CALLOUT)
			{
					linphone_core_terminate_dialog(lc, *ssid, NULL);
			}
			
			if(linphone_core_offhood_event(lc, (active_session == 0) ? 1 : 0)==0)
			{
				*ssid = GetActiveSession(lc->chid);
				SetSysState(lc->chid, SYS_STATE_CONNECT);
			}
			digitcode_init(lc);
		}
		break;

	case SYS_STATE_IN2CALLS_OUT:
	case SYS_STATE_IN2CALLS_IN:
		g_message("\r\nhave 2 call\n");
		
		if (s_sign == SIGN_FLASHHOOK)
		{
			g_message("\r\nenable  FXS_FLEASH_KEY");
			
			lc->fxsfeatureFlags|=FXS_FLEASH_KEY;
			rcm_timerLaunch(lc->flashtimeout_timer,TIMEOUT_FXS_FLASHTKEY); 
//need check country
			if(g_pVoIPCfg->tone_of_country==TONE_GERMANY)
				rcm_tapi_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_STUTTERDIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
			else
				rcm_tapi_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);

//play special dial tone
			
			break;
		}else{
			if(lc->fxsfeatureFlags & FXS_FLEASH_KEY){
				//need check country
				if(g_pVoIPCfg->tone_of_country==TONE_GERMANY)
					rcm_tapi_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_STUTTERDIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
				else
					rcm_tapi_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);

				switch(s_sign){
				case SIGN_KEY4: 
					{
						char refer_to[256];
						g_message("\r\nenable  SIGN_KEY4 call transfer");
						if (atdfr_construct_referto(lc, refer_to) == 0)
							linphone_core_transfer_call(lc, 0, refer_to);

						//create new TRANSFER state
						SetSysState(lc->chid, SYS_STATE_TRANSFER_ONHOOK);
						digitcode_init(lc);
					}
					break;
				case SIGN_KEY3:
					g_message("\r\nenable  SIGN_KEY3 3way conference");
					//R+3 , 3way conference
					if (session_2_session_event(lc, 3) == 0)
					{
						SetSysState(lc->chid, SYS_STATE_IN3WAY);
					}
					digitcode_init(lc);
				break;

				case SIGN_KEY2:
					g_message("\r\n switch call ");
					//R+2 , second call switch
					if (session_2_session_event(lc, (active_session == 0) ? 2 : 1) == 0){
						*ssid = GetActiveSession(lc->chid);
					}
					digitcode_init(lc);
					break;

				case SIGN_KEY1:
					g_message("\r\nClear active & switch to held call.");
					g_message("\r\n active_session is %d\n",active_session);
					//R+2 , second call switch
					//g_message("\r\n GetSessionState %d\n",GetSessionState(lc->chid, active_session));
					g_message("\r\n on hold session is %d\n",(active_session == 0) ? 1 : 0);
								
					linphone_core_terminate_dialog(lc, active_session, NULL);
					linphone_core_offhood_event(lc, (active_session == 0) ? 1 : 0);
					SetSysState(lc->chid, SYS_STATE_CONNECT);
					#if 0
					if (session_2_session_event(lc, (active_session == 0) ? 2 : 1) == 0){
						*ssid = GetActiveSession(lc->chid);
					}
					#endif
					digitcode_init(lc);
									

					break;
				case SIGN_KEY0:
					/* clear callwaiting call , still talk with active call */
					g_message("\r\nClear wait call , still talk with active call\n");
					g_message("\r\n active_session is %d\n",active_session);
					
					g_message("\r\n incoming session is %d\n",(active_session == 0) ? 1 : 0);
					g_message("\r\n GetSessionState %d\n",GetSessionState(lc->chid, active_session));
					
//					rcm_tapi_SetPlayTone( lc->chid , active_session, DSPCODEC_TONE_CALL_WAITING , FALSE , DSPCODEC_TONEDIRECTION_LOCAL ) ;
					linphone_core_terminate_dialog(lc, (active_session == 0) ? 1 : 0, NULL);
					SetSysState(lc->chid, SYS_STATE_CONNECT);
					digitcode_init(lc);

					break;


					
				default:
					break;
				}
					
				rcm_timerCancel(lc->flashtimeout_timer); 
				lc->fxsfeatureFlags &=~FXS_FLEASH_KEY; 
			}else{
				linphone_core_send_dtmf(lc, active_session, s_sign);
			}
		}
		break;;

//[SD6, ericchung, rcm integration
#if 0
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
//[SD6, ericchung, rcm integration
#endif //#if 0


#if 0 //[SD6, ericchung, rcm integration

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
//[SD6, ericchung, rcm integration
#endif //#if 0
	case SYS_STATE_IN3WAY:
		g_message("\r\n SYS_STATE_IN3WAY ");
		if (s_sign == SIGN_FLASHHOOK)
		{
			g_message("\r\nenable  FXS_FLEASH_KEY");
			rcm_timerLaunch(lc->flashtimeout_timer,TIMEOUT_FXS_FLASHTKEY); 	
			//need check country
			if(g_pVoIPCfg->tone_of_country==TONE_GERMANY)			
				rcm_tapi_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_STUTTERDIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
			else
				rcm_tapi_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);

			lc->fxsfeatureFlags|=FXS_FLEASH_KEY;
			break;
		}else{
		//need check country
			if(g_pVoIPCfg->tone_of_country==TONE_GERMANY)					
				rcm_tapi_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_STUTTERDIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
			else
				rcm_tapi_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
			
			if(lc->fxsfeatureFlags & FXS_FLEASH_KEY){
				switch(s_sign){
				case SIGN_KEY2:
					//stop 3way conf.
					if (session_2_session_event(lc, 4) == 0)
					{
						SetSysState(lc->chid, SYS_STATE_IN2CALLS_OUT);
					}
						
					g_message("\r\nenable  SIGN_KEY2 secondcall");
					//call rtk api to stop 3way conference 
					//switch call 
					if (session_2_session_event(lc, (active_session == 0) ? 2 : 1) == 0){
						*ssid = GetActiveSession(lc->chid);
					}
					digitcode_init(lc);
					break;
								
				default:
					break;
				}

				rcm_timerCancel(lc->flashtimeout_timer); 	
				lc->fxsfeatureFlags &=~FXS_FLEASH_KEY; 
			}else{
				linphone_core_send_dtmf(lc, active_session, s_sign);
			}
		}
		break;;


#if 0
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
//[SD6, ericchung, rcm integration
#endif //#if 0
		
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
				rcm_tapi_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
				digitcode_init(lc);	
				g_message("\r\n start off_hook timer 007\n");
				osip_gettimeofday(&lc->off_hook_start, NULL);
				rcm_active_BusyWrongTone_timer(lc);
				return 0;
			} 

/*+++added by Jack Chan for Planet transfer+++*/
#ifdef PLANET_TRANSFER
			if (eXosip.bIVR_transfer && strcmp(lc->dial_code, eXosip.blind_transfer_code) == 0)	//blind transfer
			{
				SetSysState(lc->chid, SYS_STATE_TRANSFER_BLIND);
				/* turn on dial tone. */
				rcm_tapi_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
				digitcode_init(lc);
				return 0;
				}
			else if (eXosip.bIVR_transfer && strcmp(lc->dial_code, eXosip.attended_transfer_code) == 0) //attended transfer
			{
				SetSysState(lc->chid, SYS_STATE_TRANSFER_ATTENDED);
				rcm_tapi_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
				digitcode_init(lc);
				return 0;
			}
#endif /*PLANET_TRANSFER*/
/*---end---*/
/*++added by Jack Chan  for transfer++*/
#ifdef CONFIG_RTK_VOIP_DIALPLAN
				/* call invite immediately */
				CallInviteByDialPlan( lc, si, src_ptr );
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
				/* Check PSTN routing prefix */
				if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, (const unsigned char*)src_ptr, 1 ) ) ) 
					goto label_fxo_call_on_SYS_STATE_TRANSFER_EDIT;
#endif
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
			rcm_tapi_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_DIAL, FALSE, 
				DSPCODEC_TONEDIRECTION_LOCAL);
			
#ifdef CONFIG_RTK_VOIP_DIALPLAN
			if( ( si = GetIdleSession(lc->chid) ) == -1 ) {
				digitcode_init(lc);	
				return 0;
			} 
			
			if( DialPlanTryCallInvite( lc, si, src_ptr ) ) {
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO				
				/* Check PSTN routing prefix */
				if( ( pszPSTNRouting = GetPSTNRoutingNumber( lc, (const unsigned char*)src_ptr, 1 ) ) ) 
					goto label_fxo_call_on_SYS_STATE_TRANSFER_EDIT;
#endif
				*ssid = si;
			} else
#endif
			{
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO		
		    if (strcmp(lc->dial_code, g_pVoIPCfg->funckey_pstn) == 0) 
			{
label_fxo_call_on_SYS_STATE_TRANSFER_EDIT:
	/* call transfer need implement */
				//if (g_MaxVoIPPorts > RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
				if( RTK_VOIP_DAA_NUM( g_VoIP_Feature ) > 0 )
				{
					char sz_fxo_0[40];

					if( pszPSTNRouting && pszPSTNRouting[ 0 ] ) {
						sprintf(sz_fxo_0, "sip:%s@%s:%d", solar_ptr->bound_voip_ipaddress,
							pszPSTNRouting,
							g_pVoIPCfg->ports[RTK_VOIP_DAA_CH_OFFSET(g_VoIP_Feature) /*RTK_VOIP_SLIC_NUM(g_VoIP_Feature)*/].sip_port);
					} else {
						sprintf(sz_fxo_0, "sip:%s:%d", solar_ptr->bound_voip_ipaddress,
							g_pVoIPCfg->ports[RTK_VOIP_DAA_CH_OFFSET(g_VoIP_Feature) /*RTK_VOIP_SLIC_NUM(g_VoIP_Feature)*/].sip_port);
					}
					strcpy(lc->dial_code, sz_fxo_0);
					CallInvite(lc, *ssid, src_ptr);
				}
				else	
			    {
					SetSysState(lc->chid, SYS_STATE_TRANSFER_EDIT_ERROR);
			    	rcm_tapi_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
			    }
			}
#endif	
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
			rcm_tapi_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
#endif /* CONFIG_RTK_VOIP_DIALPLAN */						
		}
		else
		{
			edit_invite_digit(lc, s_sign);
			rcm_tapi_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_DIAL, FALSE, 
				DSPCODEC_TONEDIRECTION_LOCAL);
			
#ifdef CONFIG_RTK_VOIP_DIALPLAN
			if (lc->digit_index == 0) 
				return 0;
			
			*ssid = (active_session == 0) ? 1 : 0;
			if( DialPlanTryCallTransfer( lc, *ssid ) ) {
				/*Bye the call immediately*/
				linphone_core_terminate_dialog(lc, *ssid, NULL);
				rcm_tapi_SetPlayTone(lc->chid, *ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);							
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

#ifndef __mips__
void GetLineEvent(LinphoneCore *lc, uint32 *ssid, char *src_ptr, char *digit)
{
	// do nothing if x86 simulation
}
#else
//src_ptr is input[],finial send to linphone call string
void GetLineEvent(LinphoneCore *lc, uint32 *ssid, char *src_ptr, char *digit)
{
	SIGNSTATE keyin;

#if 1 // Check RFC2833 Rx Event and other DSP Event
	VoipEventID ent;
	uint32 mid=0, dsp_data;
	VoipEventID dsp_ent;
	
	for (mid=0; mid<2; mid++)
	{
        rcm_tapi_GetRfc2833RxEvent(lc->chid, mid, &ent);
		if (ent != VEID_NONE)
		{
			g_message("RX RFC2833 Event 0x%08X, ch%d, mid%d\n", ent, lc->chid, mid);

#if defined(CONFIG_RTK_VOIP_TR104) && defined(CONFIG_E8B)
				if(rcm_tr104_st.e8c_autotest.enable==1){
					if((ent>=VEID_RFC2833_RX_DTMF_0)&&(ent<=VEID_RFC2833_RX_DTMF_9)){
						rcm_tr104_st.e8c_autotest.ReceiveDTMFNumber[rcm_tr104_st.e8c_autotest.receive_dtmf_index]=(char) '0'+ ent; /* int to char */
					rcm_tr104_st.e8c_autotest.receive_dtmf_index++;
					strcpy(rcm_tr104_st.e8c_autotest.Simulate_Status,"Receiving");
					}					
				}
#endif
		}
		
		rtk_GetDspEvent(lc->chid, mid, &dsp_ent, &dsp_data);
		if (dsp_ent == VEID_DSP_RTP_PAYLOAD_MISMATCH)
		{
			printf("Get PT mis-matched event, ch%d, mid%d, PT=%d\n", lc->chid, mid, dsp_data);
		}
		else if (dsp_ent != VEID_NONE)
		{
			if((dsp_ent==VEID_DSP_DTMF_CLID_END) || (dsp_ent==VEID_DSP_FSK_CLID_END)){
				if(GetSysState(lc->chid)==SYS_STATE_IDLE){
						rcm_tapi_SetRingFXS( lc->chid , FALSE );
				}
			}
			printf("Get DSP Event=0x%08X, ch%d, mid%d\n", dsp_ent, lc->chid, mid);
		}
	}
#endif
	//if (lc->chid < RTK_VOIP_SLIC_NUM(g_VoIP_Feature)) {
	if( RTK_VOIP_IS_SLIC_CH( lc->chid, g_VoIP_Feature ) ) {
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		return;	/* IP phone has no keyin events. */
#else
		rcm_tapi_GetFxsEvent(lc->chid, &keyin);
#endif
	} else if( RTK_VOIP_IS_DECT_CH( lc->chid, g_VoIP_Feature ) ) {
		rcm_tapi_GetDectEvent(lc->chid, &keyin);
	} else
		rcm_tapi_GetFxoEvent(lc->chid, &keyin);

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
			if (RTK_VOIP_MW_CHK_IS_AUDIOCODES(g_VoIP_Feature))
				rcm_tapi_Onhook_Action(lc->chid); /*thlin: new add when porting ACMW */
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

			if (RTK_VOIP_MW_CHK_IS_AUDIOCODES(g_VoIP_Feature))
				rcm_tapi_Offhook_Action(lc->chid); /*thlin: new add when porting ACMW */

			HookAction(lc, (SIGNSTATE) keyin, src_ptr);
			*ssid = 0;
			rcm_voipSyslog(LOG_SLIC_EVENT,"phone offhook ,chid %d",lc->chid);
			osip_gettimeofday(&lc->off_hook_start, NULL);//record off_hook_start timer.
			//rcm_timerLaunch(lc->offhook_timer,(g_pVoIPCfg->off_hook_alarm*1000)); 

			g_message("--- Get OFF Hook Event (%d) ---\n", lc->chid);
			break ;
		case SIGN_OFFHOOK_2:
			/* Off-hook but no key */
			break;
		case SIGN_RING_ON: //from FXO 	
			switch (GetSysState(lc->chid))
			{
			case SYS_STATE_IDLE:
				SetSysState(lc->chid, SYS_STATE_DAA_RING);
				rcm_tapi_SetRingFXS(lc->chid, TRUE);
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
				rcm_tapi_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_CALL_WAITING, TRUE, 
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
				rcm_tapi_SetRingFXS(lc->chid, FALSE);
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
				rcm_tapi_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_CALL_WAITING, FALSE, 
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
#endif

void CheckFaxOnCall(LinphoneCore *lc)
{
	int i;
	int fax_modem = FMS_IDLE;
	int fax_end=0;
	VoipEventID fax_event;

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
		VoipEventID t2;
		
		// feed Fax event to prevent overflow 
		rcm_tapi_GetFaxModemEvent( lc->chid, &t, &t2, 0);
		rcm_tapi_GetFaxEndDetect( lc->chid, &t );
		
		if ( t2==VEID_FAXMDM_LEC_AUTO_RESTORE )
			g_message( "LEC restore\n");
		else if ( t2==VEID_FAXMDM_LEC_AUTO_ON )
			g_message( "LEC on\n");
		else if ( t2==VEID_FAXMDM_LEC_AUTO_OFF )
			g_message( "LEC off\n");
		else if (t2)
			g_message( "fax_Event=0x%08X\n", t2 );
		
		return;
	}
#endif
	rcm_tapi_GetFaxModemEvent(lc->chid, &fax_modem, &fax_event, 0);

#if 1
	extern char * get_evt_name(VoipEventID evt);	
	if ( fax_event==VEID_FAXMDM_LEC_AUTO_RESTORE )
		g_message( "LEC restore\n");
	else if ( fax_event==VEID_FAXMDM_LEC_AUTO_ON )
		g_message( "LEC on\n");
	else if ( fax_event==VEID_FAXMDM_LEC_AUTO_OFF )
		g_message( "LEC off\n");
	else if (fax_event)
		g_message( "chid=%d fax_modem=%d fax_event=0x%08X %s\n", lc->chid , fax_modem , fax_event , get_evt_name(fax_event) );
#endif
	
	if (lc->call[i]->faxflag == FMS_IDLE)
	{
		if ( (fax_modem == FMS_FAX_LOW_SPEED) ||(fax_modem == FMS_FAX_HIGH_SPEED) )
		{
			lc->call[i]->faxflag = FMS_FAX_RUN;			/* set! */
			linphone_call_fax_reinvite(lc, i);
			g_message("SIP go FAX_RUN\n");
		}
		else if (fax_modem == FMS_MODEM_LOCAL)
		{
			lc->call[i]->faxflag = FMS_MODEM_RUN;
			linphone_call_fax_reinvite(lc, i);
			g_message("SIP go MODEM_RUN\n");
		}
//		printf("lc->call[i]->faxflag is %d\n",lc->call[i]->faxflag);
	}
	else if ((lc->call[i]->faxflag == FMS_MODEM_RUN)&& (TRUE == lc->sip_conf.T38_enable ))
	{
		if (fax_modem == FMS_V21_FLAG)
		{
			lc->call[i]->faxflag = FMS_FAX_RUN;
			linphone_call_fax_reinvite(lc, i);
			g_message("SIP go FAX_RUN2\n");
		}
	}
	else if ((lc->call[i]->faxflag == FMS_FAX_RUN)&& (TRUE == lc->sip_conf.T38_enable ))
	{
		rcm_tapi_GetFaxEndDetect(lc->chid,&fax_end);
//		printf("\r\n fax_end is %d\n",fax_end);
		if(fax_end)
		{
			// Flush FIFO
			rcm_tapi_GetFaxModemEvent(lc->chid, &fax_modem, NULL, 1);
	//		printf("\r\n REAL  linphone_call_off_fax_reinvite \n");
			lc->call[i]->faxflag = FMS_IDLE;
			rcm_linphone_call_off_fax_reinvite(lc, i);
			g_message("SIP go FAX_IDLE\n");
		}
	}
}


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
		{ NAME_DEF( "red_demo=" ),		&g_SystemParam.red_demo, VAR_INT, 0 },
		{ NAME_DEF( "red_audio=" ),		&g_SystemParam.red_audio, VAR_INT, 0 },
		{ NAME_DEF( "red_rfc2833=" ),	&g_SystemParam.red_rfc2833, VAR_INT, 0 },
		{ NAME_DEF( "fpp_demo=" ),	&g_SystemParam.fpp_demo, VAR_INT, 0 },
		{ NAME_DEF( "fpp=" ),		&g_SystemParam.fpp, VAR_INT, 0 },
		{ NAME_DEF( "fppvbd=" ),	&g_SystemParam.fppvbd, VAR_INT, 0 },
		{ NAME_DEF( "silence_demo=" ),		&g_SystemParam.silence_demo, VAR_INT, 0 },
		{ NAME_DEF( "silence_dbov=" ),		&g_SystemParam.silence_dbov, VAR_INT, 0 },
		{ NAME_DEF( "silence_period=" ),	&g_SystemParam.silence_period, VAR_INT, 0 },
		{ NAME_DEF( "modem_lec=" ),	&g_SystemParam.modem_lec, VAR_UCHAR_BOOL, 0 },
		{ NAME_DEF( "ivr_delay=" ),	&g_SystemParam.ivr_delay, VAR_INT, 0 },
		{ NAME_DEF( "max_strict_delay=" ),	&g_SystemParam.max_strict_delay, VAR_INT, 0 },
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
//[SD6 EricChung
/* register to SIP server */
void rcm_linphone_register_init(TAPP *pApp){
	int nPort;
	LinphoneCore *lc;
	int i;

	/* REGISTER TO SIP Server */
	for (nPort=0; nPort<g_MaxVoIPPorts; nPort++)
	{
	
		lc = &pApp->ports[nPort];
		if(g_pVoIPCfg->rfc_flags & SIP_SERVER_REDUNDANCY){
			/*redundancy case, only one proxy can useage */
			if( lc->proxies[0]){
				g_message("linphone_proxy_config_enableregister %d\n",nPort);
				g_message("in rcm_linphone_register_init \n");
				g_message("solar_ptr->current_voip_interface is %s\n",solar_ptr->current_voip_interface);			
				if(strlen(solar_ptr->current_voip_interface)!=0){
					linphone_proxy_config_enableregister(lc->proxies[0],TRUE);
					linphone_proxy_config_register(lc->proxies[0]);
				}else{
					linphone_proxy_config_enableregister(lc->proxies[0],0);
					}		
		
				}
		}else{
			/*normal case, 2 proxy can use the same time. */
		for (i=0;i<MAX_PROXY;i++){
			if( lc->proxies[i]){
			g_message("linphone_proxy_config_enableregister %d\n",nPort);
				g_message("in rcm_linphone_register_init proxy %d\n",i);
					if(strlen(solar_ptr->current_voip_interface)!=0){
					linphone_proxy_config_enableregister(lc->proxies[i],TRUE);
					linphone_proxy_config_register(lc->proxies[i]);
			}else{
					linphone_proxy_config_enableregister(lc->proxies[i],0);
				}		

		}
		}
	}
}

}

/* unregister to SIP server */
void rcm_linphone_unregister_init(TAPP *pApp){
	int nPort;
	LinphoneCore *lc;
	LinphoneProxyConfig *obj;
	int i;

	/* REGISTER TO SIP Server */
	for (nPort=0; nPort<g_MaxVoIPPorts; nPort++)
	{
		lc = &pApp->ports[nPort];
		if(g_pVoIPCfg->rfc_flags & SIP_SERVER_REDUNDANCY){
			/* only unregister first proxy */
			obj=lc->proxies[0];
			if( lc->proxies[0]){
		
				g_message("rcm_linphone_unregister_init %d\n",nPort);
				eXosip_lock();
				{
					osip_message_t *reg_msg=NULL;
					int i;
					i = eXosip_register_build_register(obj->rid, 0, &reg_msg);
					if(i == OSIP_SUCCESS) {
						osip_message_set_outboundproxy( reg_msg,obj->outbound_proxy,obj->outbound_port);
						eXosip_register_send_register(obj->rid, reg_msg);
					}
				}
				eXosip_unlock();						
			}

		}else{
		
		for (i=0;i<MAX_PROXY;i++){ //unregister all sip account
			if( lc->proxies[i] && lc->proxies[i]->registered == TRUE ){
				obj=lc->proxies[i];

				g_message("rcm_linphone_unregister_init port %d proxy %d\n",nPort,i);
			eXosip_lock();
			{
				osip_message_t *reg_msg=NULL;
				int i;
	
				i = eXosip_register_build_register(obj->rid, 0, &reg_msg);
				if(i == OSIP_SUCCESS) {
					if(!(g_pVoIPCfg->rfc_flags & SIP_DISABLE_REGISTER_OUTBOUND)){
						osip_message_set_outboundproxy( reg_msg,obj->outbound_proxy,obj->outbound_port);
					}
					eXosip_register_send_register(obj->rid, reg_msg);
				}
			}
			eXosip_unlock();
			
		}
	}
		
	}
	}/*end for */
}



#if 0
static int voip_if_link_status; //0:down, 1:up, -1:error
#define VOIP_WAN_IF "nas0"
void rcm_retreive_voip_if_name(char *if_name_dst, int if_name_max_len)
{
	int totalEntry, i, voip_if_index=-1;
		MIB_CE_ATM_VC_T Entry;
#if 0	
	//Scan all WAN interfaces
	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
	if(totalEntry == 0)
		goto l_err_end;
		
	for(i=0;i<totalEntry;i++){
		if(mib_chain_get(MIB_ATM_VC_TBL,i,&Entry)!=1)
				continue;

		g_message("IF %d-aptype=%d\n", i, Entry.applicationtype);
		//Check application type
		if (Entry.applicationtype&X_CT_SRV_VOICE)
		{
			voip_if_index = Entry.ifIndex;
			break;
		}
	}
#endif
#if 1 //Remove VoIP searching condition for considering dgw (default gateway) field as no VoIP traffic is desired if no IF is configured to it (recommended by SD5 Kaohj)
	if(i<totalEntry)
		goto l_success_end;
		
	for(i=0;i<totalEntry;i++){
		if(mib_chain_get(MIB_ATM_VC_TBL,i,&Entry)!=1)
				continue;
		if (Entry.dgw == 1)
		{
			//Record default route in case interface for VoIP is not valid
			voip_if_index = Entry.ifIndex;
			break;
		}
	}
	if(i<totalEntry)
		goto l_success_end;

l_success_end:
#endif
	if(voip_if_index != -1)
	{
		ifGetName(voip_if_index, if_name_dst, if_name_max_len);
//		voip_if_link_status = get_net_link_status(VOIP_WAN_IF);
	}
	else 
		goto l_err_end;

	return;


l_err_end:
	g_warning("Error: No valid IF for VoIP!!!\n");
	//Fill NULL string
	*if_name_dst = NULL;

	return;
}
#else

int do_ioctl(unsigned int cmd, struct ifreq *ifr)
{
	int skfd, ret;

	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return (-1);
	}

	ret = ioctl(skfd, cmd, ifr);
	close(skfd);
	return ret;
}


int voip_get_net_link_status(const char *ifname)
{
	struct ifreq ifr;
	struct ethtool_value edata;
	int ret;

	strcpy(ifr.ifr_name, ifname);
	edata.cmd = ETHTOOL_GLINK;
	ifr.ifr_data = (caddr_t)&edata;

	ret = do_ioctl(SIOCETHTOOL, &ifr);
	if (ret == 0)
		ret = edata.data;
	return ret;
}


/*get usable voip interface name*/
void rcm_retreive_voip_if_name(char *if_name_dst, int if_name_max_len)
{
#ifndef OPENWRT_BUILD
	int i;
 	struct ifreq ifr;
	int sockfd;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

//	printf("list all interface\n");
	
	for(i=0;i<MAX_VOIP_INTERFACE;i++){
		if(strlen(g_pVoIPShare->net_cfg.gateway_interface[i])!=0){	
			g_message("voip_index %d ,interface name:%s\n",i,g_pVoIPShare->net_cfg.gateway_interface[i]);
			g_message("get_net_link_status is %d\n",voip_get_net_link_status(g_pVoIPShare->net_cfg.gateway_interface[i]));	
		}	
	}
	
	g_message("get interface ip address for VoIP usage\n");
	// interface list from voip_flash.c
	for(i=0;i<MAX_VOIP_INTERFACE;i++){
		g_message("voip_index %d ,interface name:%s\n",i,g_pVoIPShare->net_cfg.gateway_interface[i]);
		if(strlen(g_pVoIPShare->net_cfg.gateway_interface[i])!=0){	
			bzero(&ifr, sizeof(ifr));
			strcpy(ifr.ifr_name, g_pVoIPShare->net_cfg.gateway_interface[i]);
			if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == 0) {
				if (ifr.ifr_flags & IFF_UP){
					/*add check ip get ready or not for multi interface */

						/* I want to get an IPv4 IP address */
						ifr.ifr_addr.sa_family = AF_INET;
						if (ioctl(sockfd,SIOCGIFADDR,&ifr) < 0){
							/*can't get interface ip address */
							g_message("this interface no ip \n");
						}else if (strstr(ifr.ifr_name,"ppp")!=NULL){
							g_message("this interface is ppp,\n");
							strcpy(if_name_dst, g_pVoIPShare->net_cfg.gateway_interface[i]);
							g_message("find voip interface : %s\n",if_name_dst);
							break;
						}else if(voip_get_net_link_status(g_pVoIPShare->net_cfg.gateway_interface[i])!=1){
							g_message("this interface have ip , but dsl status not up \n");
						}else{
							g_message("this interface have ip ,dsl status up\n");
					strcpy(if_name_dst, g_pVoIPShare->net_cfg.gateway_interface[i]);
					g_message("find voip interface : %s\n",if_name_dst);
					break;
				}
			}
		}
	}
	}
	
	close(sockfd);
#endif
	return;

}
void rcm_dump_voip_if_name(char *if_name_dst, int if_name_max_len)
{
#ifndef OPENWRT_BUILD
    int i;
    struct ifreq ifr;
    int sockfd;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    printf("get interface ip address for VoIP usage\n");
    // interface list from voip_flash.c
    for(i=0;i<MAX_VOIP_INTERFACE;i++){
        printf("voip_index %d ,interface name:%s\n",i,g_pVoIPShare->net_cfg.gateway_interface[i]);
        if(strlen(g_pVoIPShare->net_cfg.gateway_interface[i])!=0){
            bzero(&ifr, sizeof(ifr));
            strcpy(ifr.ifr_name, g_pVoIPShare->net_cfg.gateway_interface[i]);
            if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == 0) {
                if (ifr.ifr_flags & IFF_UP){
                    /*add check ip get ready or not for multi interface */

                        /* I want to get an IPv4 IP address */
                        ifr.ifr_addr.sa_family = AF_INET;
                        if (ioctl(sockfd,SIOCGIFADDR,&ifr) < 0){
                            /*can't get interface ip address */
                            printf("this interface no ip \n");
                        }else{
                            printf("this interface have ip \n");
                    strcpy(if_name_dst, g_pVoIPShare->net_cfg.gateway_interface[i]);
                    printf("find voip interface : %s\n",if_name_dst);
                    break;
                }
            }
        }
    }
    }

    close(sockfd);
#endif
    return;

}
#endif

int rcm_reset_web_register_status()
{
	FILE *fh;
	char buf[MAX_VOIP_PORTS * MAX_PROXY];
	/* To show the register status on Web page. */
	fh = fopen(_PATH_TMP_STATUS, "w");
	if (!fh) {
		printf("Warning: cannot open %s. Limited output.\n", _PATH_TMP_STATUS);
		printf("\nerrno=%d\n", errno);
	}
	else {
		printf("Reset register status...\n");
		memset(buf, (int) '0', sizeof(buf));
	
		fwrite(buf, sizeof(buf), 1, fh);
		fclose(fh);
	}

	return 0;

}

int app_init(TAPP *pApp)
{
	int i, err;
	int cust_idx;
	char *voip_bound_ipaddress=NULL;
	int use_br0;
	rcm_log_file_init();
	g_message("%s starts\n", __FUNCTION__);
	
	rcm_reset_web_register_status();		
	
	ortp_init();

	use_br0=solar_ptr->use_br0;
	memset(pApp, 0, sizeof(*pApp));
	solar_ptr->use_br0=use_br0;

	init_g_state( pApp ) ;
	
	memset( &g_SystemParam, 0, sizeof( g_SystemParam ) );

//#if 0 //old project not need it , for reduce memory usage
#ifdef CONFIG_RTK_VOIP_TR104
	memset(&rcm_tr104_st, 0, sizeof(rcm_tr104_st));
#endif	
/* disable dsp debug message */
	//rcm_tapi_debug(0);
//	linphone_core_enable_ipv6( lc , FALSE );
//	linphone_core_set_sip_port( lc , voip_ptr->sip_port );
//[SD6 ericchung , shold read from FLASH  . TBD

	rcm_init_voip_interface();
		
	// get port link status 
	rcm_tapi_GetPortLinkStatus( &pApp ->port_link_status );

	if(solar_ptr->use_br0==1)
	{
		printf("use_br0=%d\n",solar_ptr->use_br0);
		strcpy(solar_ptr->current_voip_interface,"br0");
	}
	
	if(rcm_get_localip_for_voip(solar_ptr->current_voip_interface,&voip_bound_ipaddress)==-1){
			printf("\r\n can't get VoIP interface IP address \n");
			exit(0);
		}
		strcpy(pApp->bound_voip_ipaddress,voip_bound_ipaddress);
		//printf("voip_bound_ipaddress is %s\n",pApp->bound_voip_ipaddress);


	//ericchung start timer system
	rcm_timer_subsystem_init();

	if (exosip_running)
		eXosip_quit();

	eXosip_enable_ipv6(FALSE);

	//[SD6, bohungwu, exosip 3.5 integration 
	//err = eXosip_init(NULL, stdout, ports, media_ports, t38_ports);
	err = eXosip_init();
	//]
	if (err < 0)
	{
		g_error("UDP port seems already in use ! Cannot initialize.\n");
		return -1;
	}

#ifdef SUPPORT_DSCP
	rcm_tapi_qos_reset_dscp_priority();
	rcm_tapi_qos_set_dscp_priority( qos2dscp(g_pVoIPCfg->sipDscp), 7); 
	rcm_tapi_qos_set_dscp_priority( qos2dscp(g_pVoIPCfg->rtpDscp), 7); 
	rcm_tapi_Set_Rtp_Dscp( qos2dscp(g_pVoIPCfg->rtpDscp));
    rcm_tapi_Set_Sip_Dscp( qos2dscp(g_pVoIPCfg->sipDscp));	

	i = qos2dscp(g_pVoIPCfg->sipDscp);
	eXosip_set_option(EXOSIP_OPT_SET_DSCP,&i);

	i = 7;
	eXosip_set_option(EXOSIP_OPT_SET_PRI,&i);

	sleep(3); // wait 8306 reset done
#endif

	/*eric: exosip: voip_bound_ipaddress:  the address to bind (NULL for all interface) */
	err=rcm_set_exosip_listen_port(IPPROTO_UDP, voip_bound_ipaddress, g_pVoIPCfg->ports[0].sip_port, AF_INET, 0);
	if (err < 0)
	{
		g_error("UDP port seems already in use ! Cannot initialize.\n");
		return -1;
	}
	//will be overwrited by solar.conf
	//eXosip_set_user_agent("sip phone");

	// user agent customization
	app_read_config_file( 0 );
	g_SystemParam.stutter_tone=1;
		
	
	exosip_running = TRUE;

	for (i=0; i<g_MaxVoIPPorts; i++)
	{

		g_message(" linphone_core_init %d\n",i);
		linphone_core_init(pApp, &pApp->ports[i], i);

		//ericchung , create timer function
		pApp->ports[i].offhook_timer		= rcm_timerCreate(offhook_dialtimeout,(void *) i);
		pApp->ports[i].dialtimeout_timer	= rcm_timerCreate(dial_interval_timeout,(void *) i);
		pApp->ports[i].flashtimeout_timer	= rcm_timerCreate(fxs_flashkeytimeout,(void *) i);
		pApp->ports[i].busytone_timer		= rcm_timerCreate(timeout_playbusytone,(void *) i);
		pApp->ports[i].stop_alltone_timer	= rcm_timerCreate(stop_alltone,(void *) i);
		pApp->ports[i].register_retry_timer	= rcm_timerCreate(rerty_register,(void *) i);
//#if 0 //old project not need it , for reduce memory usage
#if defined(CONFIG_RTK_VOIP_TR104) && defined(CONFIG_E8B)
		pApp->ports[i].e8c_tr104_timer		= rcm_timerCreate(e8c_dtmf_test,(void *) i);
#endif /* CONFIG_RTK_VOIP_TR104 */
		pApp->ports[i].options_ping_timer	= rcm_timerCreate(rcm_options_ping_timeout,(void *) i);
		pApp->ports[i].hotline_timer		= rcm_timerCreate(dial_to_hotline,(void *) i);
		pApp->ports[i].no_Answer_timer_id		= rcm_timerCreate(no_answer_timeout,(void *) i);

		// init led
		rcm_tapi_sip_register(i, 0);
		UpdateLedDisplayMode( &pApp->ports[i] );
		
		//if (i < RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
		if( RTK_VOIP_IS_SLIC_CH( i, g_VoIP_Feature ) )
		{/* FXS */
			/* Get flash hook time from flash and set to kernel space.*/
			rcm_tapi_Set_Flash_Hook_Time(i, g_pVoIPCfg->ports[i].flash_hook_time_min, 
				g_pVoIPCfg->ports[i].flash_hook_time);//time unit: 10ms
			
			/* Set the pulse dial detection */
			rcm_tapi_Set_Pulse_Digit_Det(i, g_pVoIPCfg->pulse_dial_det, g_pVoIPCfg->pulse_det_pause, 20/*min_break_ths*/, 70/*max_break_ths*/);
			//printf("pulse_dial_det= %d\n", g_pVoIPCfg->pulse_dial_det);
			//printf("pulse_det_pause= %d\n", g_pVoIPCfg->pulse_det_pause);
		}
		else if( RTK_VOIP_IS_DECT_CH( i, g_VoIP_Feature ) )
			;	// do nothing 
		else
		{ /* FXO */
			/* Set the Caller ID Detection Mode */
			rcm_tapi_Set_CID_Det_Mode(i, g_pVoIPCfg->cid_auto_det_select, g_pVoIPCfg->caller_id_det_mode);
			/* Set the Ring detection parameters for FXO */
  	                rcm_tapi_Set_FXO_Ring_Detection(300, 300, 4500); //must after set CID det mode
			/* Set the pulse dial generation */
			rcm_tapi_Set_Dail_Mode(i, g_pVoIPCfg->pulse_dial_gen);
			//printf("pulse_dial_gen= %d\n", g_pVoIPCfg->pulse_dial_gen);
			
			if (g_pVoIPCfg->pulse_dial_gen == 1)
			{
				rcm_tapi_PulseDial_Gen_Cfg(g_pVoIPCfg->pulse_gen_pps, g_pVoIPCfg->pulse_gen_make_time, g_pVoIPCfg->pulse_gen_interdigit_pause);
				//printf("pulse_gen_pps= %d\n", g_pVoIPCfg->pulse_gen_pps);
				//printf("pulse_gen_make_time= %d\n", g_pVoIPCfg->pulse_gen_make_time);
				//printf("pulse_gen_interdigit_pause= %d\n", g_pVoIPCfg->pulse_gen_interdigit_pause);
			}
		}
	}
	
	pApp->rcm_linphone_reset_timer=rcm_timerCreate(rcm_configure_restart,NULL);
	pApp->rcm_linphone_watchdog_timer=rcm_timerCreate(rcm_linphone_watchdog_hander,NULL);
	
	/* Get Tx/Rx gain value from flash and set to DAA.*/
	if ((g_VoIP_Feature & DAA_TYPE_MASK) == REAL_DAA || (g_VoIP_Feature & DAA_TYPE_MASK) == REAL_DAA_NEGO)
	{
		rcm_tapi_Set_DAA_Tx_Gain(g_pVoIPCfg->daa_txVolumne);
		rcm_tapi_Set_DAA_Rx_Gain(g_pVoIPCfg->daa_rxVolumne);
	}
	
	/* Get Tone of Customer from flash and set.*/ 
	for (cust_idx=0; cust_idx < TONE_CUSTOMER_MAX; cust_idx++)
	{
		rcm_tapi_Set_Custom_Tone(cust_idx, &g_pVoIPCfg->cust_tone_para[cust_idx]);
	}	
		
	/* Get Tone of Cuntury from flash and set. rtk_Set_Tone_Country()
	 * should be called after calling rcm_tapi_Set_Custom_Tone() for proper
	 * web page operation.
	 */
	/*if country is >=EXT1 , use country tone table */
	if( g_pVoIPCfg->tone_of_country >= TONE_EXT1 ){
		rcm_country_para_update(g_pVoIPCfg);
	}else{
	rcm_tapi_Set_Country(g_pVoIPCfg);
		rtk_SetCountryImpedance(g_pVoIPCfg->tone_of_country);
	}	
	
	rcm_tapi_Set_Dis_Tone_Para(g_pVoIPCfg);
	/* Initialize IVR */
#ifdef CONFIG_RTK_VOIP_IVR
	rcm_start_IVR();
#endif

#ifdef CONFIG_RTK_VOIP_WAN_VLAN
	//rcm_tapi_switch_wan_vlan(g_pVoIPCfg);
	//rcm_tapi_switch_wan_2_vlan(g_pVoIPCfg);
	rcm_tapi_switch_wan_3_vlan(g_pVoIPCfg);
#endif

#ifdef VOIP_CALL_LOG
	csv_api_init();
#endif

#if 0
	//Bandwidth Mgr
	#if defined (CONFIG_RTL_819X_SWCORE) && !defined(CONFIG_RTK_VOIP_GPIO_8972D_V100)
	rcm_tapi_Bandwidth_Mgr(0, 1, g_pVoIPCfg->bandwidth_LANPort0_Egress);
	rcm_tapi_Bandwidth_Mgr(1, 1, g_pVoIPCfg->bandwidth_LANPort1_Egress);
	rcm_tapi_Bandwidth_Mgr(2, 1, g_pVoIPCfg->bandwidth_LANPort2_Egress);
	rcm_tapi_Bandwidth_Mgr(3, 1, g_pVoIPCfg->bandwidth_LANPort3_Egress);
	rcm_tapi_Bandwidth_Mgr(4, 1, g_pVoIPCfg->bandwidth_WANPort_Egress);

	rcm_tapi_Bandwidth_Mgr(0, 0, g_pVoIPCfg->bandwidth_LANPort0_Ingress);
	rcm_tapi_Bandwidth_Mgr(1, 0, g_pVoIPCfg->bandwidth_LANPort1_Ingress);
	rcm_tapi_Bandwidth_Mgr(2, 0, g_pVoIPCfg->bandwidth_LANPort2_Ingress);
	rcm_tapi_Bandwidth_Mgr(3, 0, g_pVoIPCfg->bandwidth_LANPort3_Ingress);
	rcm_tapi_Bandwidth_Mgr(4, 0, g_pVoIPCfg->bandwidth_WANPort_Ingress);
    #else
	rcm_tapi_Bandwidth_Mgr(1, 1, g_pVoIPCfg->bandwidth_LANPort0_Egress);
	rcm_tapi_Bandwidth_Mgr(2, 1, g_pVoIPCfg->bandwidth_LANPort1_Egress);
	rcm_tapi_Bandwidth_Mgr(3, 1, g_pVoIPCfg->bandwidth_LANPort2_Egress);
	rcm_tapi_Bandwidth_Mgr(4, 1, g_pVoIPCfg->bandwidth_LANPort3_Egress);
	rcm_tapi_Bandwidth_Mgr(0, 1, g_pVoIPCfg->bandwidth_WANPort_Egress);

	rcm_tapi_Bandwidth_Mgr(1, 0, g_pVoIPCfg->bandwidth_LANPort0_Ingress);
	rcm_tapi_Bandwidth_Mgr(2, 0, g_pVoIPCfg->bandwidth_LANPort1_Ingress);
	rcm_tapi_Bandwidth_Mgr(3, 0, g_pVoIPCfg->bandwidth_LANPort2_Ingress);
	rcm_tapi_Bandwidth_Mgr(4, 0, g_pVoIPCfg->bandwidth_LANPort3_Ingress);
	rcm_tapi_Bandwidth_Mgr(0, 0, g_pVoIPCfg->bandwidth_WANPort_Ingress);
    #endif

#endif

#ifdef CONFIG_RTK_VOIP_TR104
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
#endif /*CONFIG_RTK_VOIP_TR104*/

	// flush kernel used fifo
	for (i=0; i<g_MaxVoIPPorts; i++)
		rcm_tapi_Set_flush_fifo(i);
		
	// initialize alarm 
	InitializeAlarmVariable();
#if 0 //eric mark , not need icmp	
	// open ICMP 
	open_voip_icmp_socket();
#endif
	if(voip_bound_ipaddress)
		osip_free(voip_bound_ipaddress);

	return 0;
}

int app_close(TAPP *pApp)
{
	int i, nPort;
	GList *elem;
	LinphoneCore *lc;
	int h_max=0;	//Alex, 20111122, Valgrind: uninitialized value
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
			linphone_proxy_send_unregister(cfg);	/* to unregister */
		}

	}

	// send pending packets (include unregister)
	eXosip_execute();
	
	//SD6, bohungwu, socket manipulation is allowed only inside eXosip 3.5
	//h_max = eXosip.j_sockets[0];
	//for (nPort=0; nPort<g_MaxVoIPPorts; nPort++)
	//{
	//	if (h_max < eXosip.j_sockets[nPort])
	//		h_max = eXosip.j_sockets[nPort];
	//}

#ifdef MALLOC_DEBUG
	// if use memwatch, don't handle any packet on exit
#else
	// wait 2 sec to clear state machine
	for (i=0; i<20; i++) 
	{
		int err;
		fd_set fdset;
		struct timeval tv;
		//[SD6, bohungwu, exosip 3.5 integration 

		FD_ZERO(&fdset);		
		//for (nPort=0; nPort<g_MaxVoIPPorts; nPort++)
		//{
		//	FD_SET(eXosip.j_sockets[nPort], &fdset);
#ifdef CONFIG_RTK_VOIP_SIP_TLS
		//	for(proxyIndex = 0; proxyIndex < MAX_PROXY; proxyIndex++)
		//		if(eXosip.j_tls_sockets[nPort][proxyIndex] != -1)
		//			FD_SET(eXosip.j_tls_sockets[nPort][proxyIndex], &fdset);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
		//}
		//]

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
                eXosip_event_free(ev);		//Added, Alex, 20111124, unregister event doesn't free
			}

			continue;
		}

		//err > 0 won't happen
		#if 0
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
		#endif //#if 0
	}
#endif

	eXosip_lock();
	eXosip_clear_authentication_info();
	eXosip_unlock();

	for (nPort=0; nPort<g_MaxVoIPPorts; nPort++)
	{
		sip_config_t *config;

		lc = &pApp->ports[nPort];
		config = &lc->sip_conf;
		for(elem=config->proxies; elem!=NULL; elem=g_list_next(elem))
		{
			LinphoneProxyConfig *cfg;
			
			cfg = (LinphoneProxyConfig*)(elem->data);
			linphone_update_and_display_registration_status( lc, cfg, REGSTATUS_RESTART );
			
		}

	}


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

#ifdef CONFIG_RTK_VOIP_TR104
	close(ipcSocket);
#endif
#if 0	
	close_voip_icmp_socket();
#endif	
	return 0;
}


int rcm_exosip_init(TAPP *pApp)
{
	int i, err;
	int cust_idx;
	char *voip_bound_ipaddress=NULL;
	
	
	g_message("rcm_exosip_init\n");

	if(strlen(solar_ptr->current_voip_interface)==0)
		rcm_retreive_voip_if_name(solar_ptr->current_voip_interface, DNS_LEN);
	//rcm_voip_flash_if_name(solar_ptr->current_voip_interface, DNS_LEN);
	
//	g_message("\r\n solar_ptr->current_voip_interface is %s\n",solar_ptr->current_voip_interface);
	
	// get port link status 
	rcm_tapi_GetPortLinkStatus( &pApp ->port_link_status );

	//if(strlen(solar_ptr->current_voip_interface)!=0){
	
		if(rcm_get_localip_for_voip(solar_ptr->current_voip_interface,&voip_bound_ipaddress)==-1){
			printf("\r\n can't get VoIP interface IP address \n");
			exit(0);
		}
		strcpy(pApp->bound_voip_ipaddress,voip_bound_ipaddress);
	
	eXosip_enable_ipv6(FALSE);
	
	err = eXosip_init();
	if (err < 0)
	{
		printf("UDP port seems already in use ! Cannot initialize.\n");
		return -1;
	}

	//voip_bound_ipaddress: the address to bind (NULL for all interface)
	err = eXosip_listen_addr (IPPROTO_UDP, voip_bound_ipaddress, g_pVoIPCfg->ports[0].sip_port, AF_INET, 0);
	/* configure 100/180 contact */
	if(voip_bound_ipaddress!=NULL){
		eXosip_set_option(EXOSIP_OPT_SET_IPV4_FOR_GATEWAY,voip_bound_ipaddress);
	}
	if (err < 0)
	{
		g_error("UDP port seems already in use ! Cannot initialize.\n");
		return -1;
	}
	if(voip_bound_ipaddress!=NULL){
		eXosip_masquerade_contact(voip_bound_ipaddress,g_pVoIPCfg->ports[0].sip_port);
	}

	eXosip_set_user_agent("sip phone");
	// user agent customization
	app_read_config_file( 0 );
		
	exosip_running = TRUE;
	
	// flush kernel used fifo
	for (i=0; i<g_MaxVoIPPorts; i++)
		rcm_tapi_Set_flush_fifo(i);
			
	// initialize alarm 
	InitializeAlarmVariable();

	if(voip_bound_ipaddress)
		osip_free(voip_bound_ipaddress);
	
	return 0;
}

int rcm_exosip_close(TAPP *pApp)
{
	int i;
	//int i, nPort;
	//GList *elem;
	//LinphoneCore *lc;
	int h_max=0;


	// send pending packets 
	eXosip_execute();
	
	g_message("rcm_exsiop_close\n");

	for (i=0; i<20; i++) 
	{
		int err;
		fd_set fdset;
		struct timeval tv;

		
		FD_ZERO(&fdset);		

		
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
				eXosip_event_free(ev);
			}

			continue;
		}

	}


	eXosip_lock();
	eXosip_clear_authentication_info();
	eXosip_unlock();

	g_message("eXosip_quit\n");

	eXosip_quit();
	exosip_running = FALSE;

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
		//[SD6, bohungwu, exosip 3.5 integration
		//if (eXosip_on_update_call(call->did, call->sdpctx->version) == 0)
		if (rcm_eXosip_on_update_call(call->did, call->sdpctx->version) == 0)
		//]
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
				//lc->call[i]->state = LCStateUpdate;
				SetCallState(lc->chid,i,&(lc->call[i]->state),LCStateUpdate);
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
		//[SD6, bohungwu, *_options_tr are no longer valid in eXosip 3.5
		//if (jc->c_inc_options_tr!=NULL)
		//{
		//	tr = jc->c_inc_options_tr;
		//	printf("transaction %d, state=%s, method=%s\n", 
		//		tr->transactionid, str_state[tr->state], tr->cseq->method);
	    //}
		//
		//if (jc->c_out_options_tr!=NULL)
		//{
		//	tr = jc->c_out_options_tr;
		//	printf("transaction %d, state=%s, method=%s\n", 
		//		tr->transactionid, str_state[tr->state], tr->cseq->method);
		//}
		//]

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
	//[SD6, bohungwu, exosip 3.5 integration
	//while (!osip_list_eol(eXosip.j_transactions, pos))
	while (!osip_list_eol(&(eXosip.j_transactions), pos))	
	//]
	{
		//[SD6, bohungwu, exosip 3.5 integration
		//tr = (osip_transaction_t*) osip_list_get(eXosip.j_transactions, pos);
		tr = (osip_transaction_t*) osip_list_get(&(eXosip.j_transactions), pos);
		//]
		printf("transaction %d, state=%s, method=%s\n", 
			tr->transactionid, str_state[tr->state], tr->cseq->method);
		pos++;
	}

	printf("\n");
	printf("==== dump transactions in ICT =====\n");
	pos = 0;
	//[SD6, bohungwu, exosip 3.5 integration
	//while (!osip_list_eol(&(eXosip.j_osip->osip_ict_transactions), pos))
	while (!osip_list_eol(&(eXosip.j_osip->osip_ict_transactions), pos))
	//]
	{
		//[SD6, bohungwu, exosip 3.5 integration
		//tr = (osip_transaction_t*) osip_list_get(eXosip.j_osip->osip_ict_transactions, pos);
		tr = (osip_transaction_t*) osip_list_get(&(eXosip.j_osip->osip_ict_transactions), pos);
		//]
		printf("transaction %d, state=%s, method=%s\n", 
			tr->transactionid, str_state[tr->state], tr->cseq->method);
		pos++;
	}

	printf("\n");
	printf("==== dump transactions in NICT =====\n");
	pos = 0;
	//[SD6, bohungwu, exosip 3.5 integration
	//while (!osip_list_eol(eXosip.j_osip->osip_nict_transactions, pos))
	while (!osip_list_eol(&(eXosip.j_osip->osip_nict_transactions), pos))
	//]
	{
		//[SD6, bohungwu, exosip 3.5 integration
		//tr = (osip_transaction_t*) osip_list_get(eXosip.j_osip->osip_nict_transactions, pos);
		tr = (osip_transaction_t*) osip_list_get(&(eXosip.j_osip->osip_nict_transactions), pos);
		//]
		printf("transaction %d, state=%s, method=%s\n", 
			tr->transactionid, str_state[tr->state], tr->cseq->method);
		pos++;
	}

	printf("\n");
	printf("==== dump transactions in IST =====\n");
	pos = 0;
	//[SD6, bohungwu, exosip 3.5 integration
	//while (!osip_list_eol(eXosip.j_osip->osip_ist_transactions, pos))
	while (!osip_list_eol(&(eXosip.j_osip->osip_ist_transactions), pos))
	//]
	{
		//[SD6, bohungwu, exosip 3.5 integration
		//tr = (osip_transaction_t*) osip_list_get(eXosip.j_osip->osip_ist_transactions, pos);
		tr = (osip_transaction_t*) osip_list_get(&(eXosip.j_osip->osip_ist_transactions), pos);
		//]
		printf("transaction %d, state=%s, method=%s\n", 
			tr->transactionid, str_state[tr->state], tr->cseq->method);
		pos++;
	}

	printf("\n");
	printf("==== dump transactions in NIST =====\n");
	pos = 0;
	//[SD6, bohungwu, exosip 3.5 integration
	//while (!osip_list_eol(eXosip.j_osip->osip_nist_transactions, pos))
	while (!osip_list_eol(&(eXosip.j_osip->osip_nist_transactions), pos))
	//]
	{
		//[SD6, bohungwu, exosip 3.5 integration
		//tr = (osip_transaction_t*) osip_list_get(eXosip.j_osip->osip_nist_transactions, pos);
		tr = (osip_transaction_t*) osip_list_get(&(eXosip.j_osip->osip_nist_transactions), pos);
		//]
		printf("transaction %d, state=%s, method=%s\n", 
			tr->transactionid, str_state[tr->state], tr->cseq->method);
		pos++;
	}

	printf("\n");
	printf("==== dump transactions in IXT =====\n");
	pos = 0;
	//[SD6, bohungwu, exosip 3.5 integration
	//while (!osip_list_eol(eXosip.j_osip->ixt_retransmissions, pos))
	while (!osip_list_eol(&(eXosip.j_osip->ixt_retransmissions), pos))
	//]
	{
		//[SD6, bohungwu, exosip 3.5 integration
		//tr = (osip_transaction_t*) osip_list_get(eXosip.j_osip->ixt_retransmissions, pos);
		tr = (osip_transaction_t*) osip_list_get(&(eXosip.j_osip->ixt_retransmissions), pos);
		//]
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
        &to->url.username[0] == '\0' ||
		&to->url.host[0] == '\0' ||
		strcmp(to->url.host, solar_ptr->bound_voip_ipaddress) != 0)
		return -1;

	if (sscanf(to->url.username, "fxo%d_to_fxs", &i) == 1)
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


/* SD6, bohungwu, netlink integration */
void rcm_netlink_handler(int netlink_desc)
{
	struct sockaddr_nl nl_remote;
	struct nlmsghdr *nlh;
	int nl_remote_len;
	char buf[512];
	int err;
	char *voip_new_ip=NULL;
	int voip_if_link_status_new;
	char voip_if_name_new[DNS_LEN] = {0};
						
	memset(buf, 0x0, 512);
	nl_remote_len = sizeof( nl_remote );
	err = recvfrom(netlink_desc, buf, sizeof(buf), 0, (struct sockaddr *) &nl_remote, &nl_remote_len);
	if (err < 0)
	{
		g_warning("netlink recv failed \n");
		return;
	}

	if (err == 0)
	{
		g_message("netlink: EOF\n");
		return;
	}

	if (nl_remote_len != sizeof(nl_remote))
	{
		g_warning("netlink: sender address len = %d\n", nl_remote_len);
		return;
	}

	nlh = (struct nlmsghdr *) buf;
	for (; NLMSG_OK(nlh, err); nlh = NLMSG_NEXT(nlh, err))
	{
		switch (nlh->nlmsg_type)
		{
		case NLMSG_DONE:
		case NLMSG_ERROR:
			break;
		case RTM_NEWADDR:
			g_message("Maserati: Receive IP Change Event : RTM_NEWADDR\n");
			g_message("voip_interface is (%d): %s\n",strlen(solar_ptr->current_voip_interface),solar_ptr->current_voip_interface);

			if(strlen(g_pVoIPCfg->voip_interface)==0){
				rcm_retreive_voip_if_name(solar_ptr->current_voip_interface, DNS_LEN);
			}
			
			rcm_get_localip_for_voip(solar_ptr->current_voip_interface,&voip_new_ip);
	
			g_message("voip_new_ip=%s;bound_voip_ipaddress=%s\n", voip_new_ip, solar_ptr->bound_voip_ipaddress);
			if(strcmp(voip_new_ip,solar_ptr->bound_voip_ipaddress)!=0)
			{
				g_message("IP change, restart Maserati\n");
				rcm_timerLaunch(solar_ptr->rcm_linphone_reset_timer,1000);  //after 1 sec check again.
			}
			break;
		case RTM_DELADDR:
			break;
		case RTM_DELLINK:
		case RTM_NEWLINK:
			//printf("netlink: link status change\n");
			g_message("Maserati: Receive link status change event :RTM_NEWLINK/RTM_DELLINK\n");

				rcm_retreive_voip_if_name(voip_if_name_new, DNS_LEN);

				g_message("VoIP WAN interface, original=%s, new=%s\n", solar_ptr->current_voip_interface, voip_if_name_new);
				if(strcmp(solar_ptr->current_voip_interface, voip_if_name_new) !=0)
				{
					g_message("VoIP interface changed, restart Maseratic\n");
					rcm_timerLaunch(solar_ptr->rcm_linphone_reset_timer,1000);  //after 1 sec check again.
				}

			break;
		default:
			g_warning("Maserati/netlink: wrong msg\n");
			break;
		}
	}
}
//#else
//void rcm_netlink_handler(int netlink_desc){}
//
//#endif

static
int
app_run(TAPP *pApp)
{

	LinphoneCore *opm;
	int i;
	int err;
	fd_set fdset;
	struct timeval timeout;
	struct timeval *timerlisthead_p;
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


	// timer check variable
	struct timeval now;

#ifdef NEW_STUN
#else
	FILE *fh;
	char buf[10];
	int STUN_timeout_counter=0;
#endif

	/* SD6, bohungwu, netlink integration */
	int h_nl;
	int h_exosip;

	struct sockaddr_nl nl_local;

#ifdef SUPPORT_IVR_HOLD_TONE 
	extern void InitG723IvrFile( void );
	extern void RefillG723IvrFile( int chid, int sid );
	
	InitG723IvrFile();
#endif /* SUPPORT_IVR_HOLD_TONE */
	rcm_set_conctrol_fifo();


	// Use FIFO to do restart and auto-testing
    if (access(FIFO_CONTROL, F_OK) == -1)
	{
        if (mkfifo(FIFO_CONTROL, 0755) == -1)
	        g_error("access %s failed: %s\n", FIFO_CONTROL, strerror(errno));
	}

    h_control = open(FIFO_CONTROL, O_RDWR);
    if (h_control == -1)
        g_error("open %s failed\n", FIFO_CONTROL);


	/* SD6, bohungwu, netlink integration */
	h_nl = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	//if (h_nl == -1)
	if (h_nl <0) //robustness consideration
	{
		g_error("netlink: open socket failed\n");
		return -1;
	}
	bzero(&nl_local, sizeof(nl_local));
	nl_local.nl_family = AF_NETLINK;
	nl_local.nl_pad = 0;
	nl_local.nl_pid = getpid();
	nl_local.nl_groups = RTMGRP_IPV4_IFADDR | RTMGRP_NOTIFY | RTMGRP_LINK;

	err = bind(h_nl, (struct sockaddr *) &nl_local, sizeof(nl_local));
	if (err == -1){
		g_error("netlink: bind failed\n");
		return -1;
	}


	/* REGISTER TO SIP Server */
	rcm_linphone_unregister_init(pApp);
	rcm_linphone_register_init(pApp);
	g_message("app_run 002\n");

#if 1 //ericchung: E8C , IOT with HG255, HG255 can't receive 101 response. so we not send 101 to SIP server	    
		eXosip.dontsend_101 = 1;
#endif

#if 0 //thlin
	//rtk_GetDTMFMODE(0, 0, &dtmf_mode);	
	/* dtmf_mode should be configed from web page */
	//dtmf_mode = 2;	//0:rfc2833  1: sip info  2: inband  3: DTMF delete
	rtk_SetDTMFMODE(0, 0, opm->sip_conf.dtmf_type);

#endif

	for (i=0; i<MAX_VOIP_PORTS; i++)
		input[i][0] = 0;
		
	while (run)
	{
		ActiveLine = (ActiveLine + 1) % g_MaxVoIPPorts;

		opm = &pApp->ports[ActiveLine];

		//[SD6, ericchung, rcm integration
		//g_message("\r\n input addr is %x\n",input[ActiveLine][0]);
		opm->lc_input = input[ActiveLine];

		// set descriptor, and find max
		FD_ZERO(&fdset);

		if(h_control >1024)
			printf("WRONG:fdset %s %d  > 1024\n",__FILE__,__LINE__);

		FD_SET(h_control, &fdset);
		h_max = h_control;

#ifdef CONFIG_RTK_VOIP_TR104
		FD_SET(ipcSocket, &fdset);
		if(h_max < ipcSocket)
			h_max = ipcSocket;
#endif
		if(exosip_running){
			h_exosip=jpipe_get_read_descr(eXosip.j_socketctl_event);
			FD_SET(h_exosip, &fdset);
			if( h_max < h_exosip)
				h_max = h_exosip;
		}

		/* SD6, bohungwu, netlink integration */
		FD_SET(h_nl, &fdset);

		if(h_max < h_nl)
			h_max = h_nl;
		
		/* check RCM timer list for select timeout */

		timerlisthead_p = rcm_getHeadTimer();

		if(timerlisthead_p==NULL){

			timeout.tv_sec= 0;
			timeout.tv_usec = 100000/g_MaxVoIPPorts; // 100ms

		} else {

			osip_gettimeofday(&now, NULL);
			timersub(timerlisthead_p,&now,&timeout);

			if(timeout.tv_sec < 0){
                /* thread has been time up, no need to wait*/
   				timeout.tv_sec = 0;
				timeout.tv_usec = 0;
			}

			if((timeout.tv_sec * 1000000 + timeout.tv_usec)>(100000/g_MaxVoIPPorts)){
				timeout.tv_sec= 0;
				timeout.tv_usec = 100000/g_MaxVoIPPorts; // 100ms
			}
		}

		rcm_sim_stopcheck();

		// wait event by select
		if((timeout.tv_usec<0) || (timeout.tv_usec>1000000))
			printf("\r\n apprun , time usec error !!!!!!!\n");

		if((h_max+1) > 1024)
			printf("WRONG: %s %d  > 1024\n",__FILE__,__LINE__);
        
		err = select(h_max + 1, &fdset, NULL, NULL, &timeout);
#if 0 //eric mark
		if (err  == 0) // timeout
		{
#ifdef SUPPORT_IVR_HOLD_TONE 
			RefillG723IvrFile( ActiveLine ,0 );
#endif
		
			CheckFaxOnCall(opm); 	/* Check FAX status */

			rcm_TimerProc(ActiveLine); 		/* process timer system */

			GetLineEvent(opm, &SessionID, input[ActiveLine], &dtmf_digit); /* read event from kerne/dsp */
			
			ProcessAlarmEvent( opm ); //support morning call , alarm

#ifdef CONFIG_RTK_VOIP_IVR
			rcm_IvrPolling_proc(opm);	/* process irv */
#endif

			if (input[ActiveLine][0] == 0) 
			{
				//printf("app_run(): no input from slic, check and process sip event...\n");
				// no input from slic, check and process sip event
				//eric.chunug this func is query eXosip event and process it.
				linphone_core_iterate(opm);
				rcm_linphone_NoAnswerCheck(opm,ActiveLine);
				continue;
			}
			else
			{
				// got input from slic
				run = linphonec_parse_command_line(opm, SessionID, input[ActiveLine]);
			}
		}// end of timeout
	
		else if (err > 0) // has input
#endif				
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
							if(s[1] == '0')
							{
								char *voip_new_ip=NULL;
								printf("Receive IP Change Event\n");

								if(strlen(solar_ptr->current_voip_interface)==0)
									rcm_retreive_voip_if_name(solar_ptr->current_voip_interface, DNS_LEN);
								
								rcm_get_localip_for_voip(solar_ptr->current_voip_interface,&voip_new_ip);
								if(strcmp(voip_new_ip,solar_ptr->bound_voip_ipaddress)!=0){
									printf("IP change\n");
									rcm_timerLaunch(solar_ptr->rcm_linphone_reset_timer, 1000);  //after 1 sec check again.
								}

								//for (i=0; i<g_MaxVoIPPorts; i++)
								//	linphone_core_refresh(&pApp->ports[i]);
							}
							else if(s[1] == '1')
							{
								printf("LINK Change \n");

				                // get port link status 
				                rcm_tapi_GetPortLinkStatus( &pApp ->port_link_status );

			                // try to register all proxy again 
				                for (nPort = 0; nPort < g_MaxVoIPPorts; nPort++)
					                linphone_core_update_proxy_register_unconditionally( &pApp ->ports[ nPort ] );
							}
						}else if (strlen(s) == 2 && s[0] == 'd'){
							//dtmf
							  CallDigitEdit(ctl_lc, &SessionID, SIGN_KEY1 + s[1] - '1', input[ctl_line]);

						}
						else if (strlen(s) == 2 && s[0] == 'f') // fax test
						{
							if (s[1] == '0')
							{
								ctl_lc->call[0]->faxflag = FMS_IDLE;
								rcm_linphone_call_off_fax_reinvite(ctl_lc, 0);
								printf("SIP go FAX_IDLE\n");
							}
							else if (s[1] == '1')
							{
								ctl_lc->call[0]->faxflag = FMS_FAX_RUN;
								linphone_call_fax_reinvite(ctl_lc, 0);
								printf("SIP go FAX_RUN2 (%d)\n", ctl_lc->sip_conf.T38_enable);
							}
							else if (s[1] == '2')
							{
								ctl_lc->call[0]->faxflag = FMS_MODEM_RUN;
								linphone_call_fax_reinvite(ctl_lc, 0);
								printf("SIP go MODEM_RUN\n");
							}
						}
						else  if (s[1] == 0) // command
						{
							int i;

							switch (s[0])
							{
							case 'h':
								printf("solar ver is 02\n");
								printf("x , Restart VoIP\n");
								printf("s , OFFHOOK\n");
								printf("e , On HOOK\n");
								printf("r , flash update\n");
								printf("d , dump_transaction \n");
								printf("g , enable debug to log file \n");
								printf("c , enable debug to console\n");
								printf("b , reset statistics\n");
								printf("f , display statistics\n");
								printf("v , call mgnt version\n");
								printf("r , update IVR \n");
								printf("I , show voip interface \n");

								break;		
							case 'v':
								printf("Ver: %s\n",VersionStr);
								break;
								
#ifdef VOIP_FLASH_ARCH	//Alex, original architecture
                            case 'X':
//								rcm_voip_flash_update();
#endif	//VOIP_FLASH_ARCH
							case 'x':
								printf("\r\nreceive %s command ,VoIP call manager restart\n", s);
								//printf("RCM_WATCHDOG_period is %d\n",RCM_WATCHDOG_period);

									rcm_timerLaunch(solar_ptr->rcm_linphone_reset_timer,1000);  //after 1 sec check again.
								#if 0// move to reset timer rcm_configure_restart, cancel watchdog need wait line state change idle 	
								if(RCM_WATCHDOG_period!=0)	{
									write(RCM_WATCHDOG_pipe, "123", 4);
									rcm_timerCancel(solar_ptr->rcm_linphone_watchdog_timer); //use watchdog for reset.
								}
								#endif
									
                                break;
							case 'w':
								printf("only reset eXoSIP \n");
#ifdef VOIP_FLASH_ARCH	//Alex, original architecture
								rcm_voip_flash_update();
#endif	//VOIP_FLASH_ARCH
								rcm_linphone_sip_restart(pApp);
                                break;
								
							case '0':
							case '1':
							case '2':
							case '3':
								ctl_line = s[0] - '0';
								printf("chagne to line %d\n", ctl_line);
								break;
                            case 's':
                                rcm_tapi_enable_pcm(ctl_line, 1);
                                HookAction(ctl_lc, SIGN_OFFHOOK, input[ctl_line]);
								osip_gettimeofday(&ctl_lc->off_hook_start, NULL);
                                run = linphonec_parse_command_line(ctl_lc, SessionID, input[ctl_line]);
                                break;
                            case 'e':
                                HookAction(ctl_lc, SIGN_ONHOOK, input[ctl_line]);
                                rcm_tapi_enable_pcm(ctl_line, 0);
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
#if 0 //old project not need it , for reduce memory usage
//#ifdef CONFIG_RTK_VOIP_TR104

							case 't': /* test TR104 auto test for caller */
								{
									tr104_auto_test_hardcopy();
//									ctl_lc=&(solar.ports[0]);
									ctl_lc = &pApp->ports[0]; //use fxs0 for test 

									printf("\r\n start E8C TEST for Caller \n");
									rcm_timerLaunch(ctl_lc->e8c_tr104_timer,3*1000); 

				
									#if 0
									char temp_str[40]="";
									ctl_line=0;
									SessionID=0;
									rcm_tr104_st.e8c_autotest.enable=1; //enable auto dial
									rcm_tr104_st.e8c_autotest.TestType=0; //for caller
									rcm_tr104_st.e8c_autotest.status=1;
									ctl_lc = &pApp->ports[0]; //use fxs0 for test 
									sprintf(temp_str,"call %s",rcm_tr104_st.e8c_autotest.callednumber);
									/* offhook */
									printf("\r\n try offhook , dial to %s\n",temp_str);
									rcm_tapi_enable_pcm(ctl_line, 1);
       		                        HookAction(ctl_lc, SIGN_OFFHOOK, input[ctl_line]);
									run = linphonec_parse_command_line(ctl_lc, SessionID, temp_str);										
									#endif
								}
							
								break;
							case 'y':
								{ /* test for callee */

									rcm_tr104_st.e8c_autotest.enable=1;
									rcm_tr104_st.e8c_autotest.TestType=1;
									rcm_tr104_st.e8c_autotest.receive_dtmf_index=0;
									
									memset( &(rcm_tr104_st.e8c_autotest.ReceiveDTMFNumber), 0,FW_LEN );
									
								}
                            	case 'f':
								{
								voiceProfileLineStatus myVPLineStatus;
								printf("Display call log\n");
								ctl_lc = &pApp->ports[0]; //use fxs0 for test 
								linphone_call_log_statistics(ctl_lc,&myVPLineStatus);

								printf("incomingCallsReceived %d\n",myVPLineStatus.incomingCallsReceived);
								printf("incomingCallsAnswered %d\n",myVPLineStatus.incomingCallsAnswered);
								printf("incomingCallsConnected %d\n",myVPLineStatus.incomingCallsConnected);
								printf("incomingCallsFailed %d\n",myVPLineStatus.incomingCallsFailed);
								printf("outgoingCallsAttempted %d\n",myVPLineStatus.outgoingCallsAttempted);
								printf("outgoingCallsAnswered %d\n",myVPLineStatus.outgoingCallsAnswered);
								printf("outgoingCallsConnected %d\n",myVPLineStatus.outgoingCallsConnected);
								printf("outgoingCallsFailed %d\n",myVPLineStatus.outgoingCallsFailed);
								printf("total call time is %d\n",myVPLineStatus.totalCallTime);
	
								}
								break;
               					case 'b':
								{
								
								printf("reset statistics\n");
								ctl_lc = &pApp->ports[0]; //use fxs0 for test 
								
								rcm_linphone_reset_call_log_statistics(ctl_lc);
								}
								break;
#endif /* CONFIG_RTK_VOIP_TR104 */                           
							case 'g':
								printf("enable debug to log file \n");
								g_SystemDebug= DEBUG_B_ERROR | DEBUG_B_WARNING | DEBUG_B_MESSAGE;
								break;
							case 'c':
								printf("enable debug to Console\n");
								g_SystemDebug= DEBUG_B_ERROR | DEBUG_B_WARNING | DEBUG_B_MESSAGE|DEBUG_B_CONSOLE;
								break;	
							case 'i':
								printf("send fxs0 register to sip server\n");
								ctl_lc = &pApp->ports[0]; //use fxs0 for test 
								linphone_proxy_config_enableregister(ctl_lc->proxies[0],TRUE);
								linphone_proxy_config_register(ctl_lc->proxies[0]);
								break;								
							case 'k':
								printf("enable Kernel DSP debug to Console\n");
								rcm_tapi_debug(1);
								break;
							case 'a':
								printf("led test 1,LED_BLINKING \n");
								rtk_Set_LED_Display( 0, 1, LED_BLINKING );
								rtk_Set_LED_Display( 1, 1, LED_ON );
								break;
							case 'I':
								printf("voip interface info\n");
								rcm_dump_voip_if_name(solar_ptr->current_voip_interface, DNS_LEN);
								break;
							case 'L':
								printf("voip UpdateLed\n");
								for (nPort = 0; nPort < g_MaxVoIPPorts; nPort++)
									UpdateLedDisplayMode( &pApp ->ports[ nPort ] );
								break;
							case 'b':
								printf("voip interface br0\n");
								solar_ptr->use_br0=1;
								rcm_timerLaunch(solar_ptr->rcm_linphone_reset_timer,1000);  //after 1 sec check again.
								break;
								
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

				}
				else
				{
                			g_error("read %s failed\n", FIFO_CONTROL);
				}
			}


#ifdef CONFIG_RTK_VOIP_TR104
			if(FD_ISSET(ipcSocket, &fdset)){
				unsigned char *buf = NULL;
				if((buf = (unsigned char*)malloc(cwmpEvtMsgSizeof())) != NULL){
					cwmpEvtMsg *evtMsg;
					if(recvfrom(ipcSocket, (void*)buf, cwmpEvtMsgSizeof(), 0, NULL, NULL)>1){
							evtMsg = (cwmpEvtMsg*) buf;
						g_message("+++++get the status of line %d+++++\n",evtMsg->event);		
						solarSendResponseToCwmp(evtMsg);
					}
					free(buf);
				}
			}

#endif /*CONFIG_RTK_VOIP_TR104*/
#if 1		/* read exosip event */
			if (FD_ISSET(h_exosip, &fdset)) {
					
				char buf[500];
				jpipe_read(eXosip.j_socketctl_event, buf, 499);
				//handle exosip event
//				printf("\r\n rcm_linphone_core_iterate_exosip \n");
				rcm_linphone_core_iterate_exosip();
			}
#endif
			/* SD6, bohungwu, netlink integration */
			if(FD_ISSET(h_nl, &fdset))
			{
				rcm_netlink_handler(h_nl);
			}
#if 1//eric

#ifdef SUPPORT_IVR_HOLD_TONE 
			RefillG723IvrFile( ActiveLine ,0 );
#endif
		
			CheckFaxOnCall(opm); 	/* Check FAX status */

			rcm_TimerProc(ActiveLine); 		/* process timer system */

			GetLineEvent(opm, &SessionID, input[ActiveLine], &dtmf_digit); /* read event from kerne/dsp */
			
			ProcessAlarmEvent( opm ); //support morning call , alarm

#ifdef CONFIG_RTK_VOIP_IVR
			rcm_IvrPolling_proc(opm);	/* process irv */
#endif

			if (input[ActiveLine][0] == 0) 
			{
				//printf("app_run(): no input from slic, check and process sip event...\n");
				// no input from slic, check and process sip event
				//eric.chunug this func is query eXosip event and process it.
				linphone_core_iterate(opm);
				rcm_linphone_NoAnswerCheck(opm,ActiveLine);
				continue;
			}
			else
			{
				// got input from slic
				run = linphonec_parse_command_line(opm, SessionID, input[ActiveLine]);
			}
			



#endif
		}
#if 0
		else
		{
			if ((err == -1) && (errno == EINTR || errno == EAGAIN))
				continue;
			else
				g_error("Error in select(): %s\n",strerror(errno));
		}
#endif
		input[ActiveLine][0] = 0;
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
	int i=0;
	idx = 0;
	for (i=0;i<_CODEC_MAX;i++)
		g_mapSupportedCodec[i]=_CODEC_MAX;


	g_mapSupportedCodec[_CODEC_G711U] = idx++;

	g_mapSupportedCodec[_CODEC_G711A] = idx++;
	g_mapSupportedCodec[_CODEC_G729] =(g_VoIP_Feature & CODEC_G729_SUPPORT) ? idx++ : _CODEC_MAX;;

#ifdef CONFIG_RTK_VOIP_G7231
	g_mapSupportedCodec[_CODEC_G723] =  (g_VoIP_Feature & CODEC_G723_SUPPORT) ? idx++ : _CODEC_MAX;;
#endif
#ifdef CONFIG_RTK_VOIP_G726	
	g_mapSupportedCodec[_CODEC_G726_16] =  (g_VoIP_Feature & CODEC_G726_SUPPORT) ? idx++ : _CODEC_MAX;;
	g_mapSupportedCodec[_CODEC_G726_24] =  (g_VoIP_Feature & CODEC_G726_SUPPORT) ? idx++ : _CODEC_MAX;;
	g_mapSupportedCodec[_CODEC_G726_32] =  (g_VoIP_Feature & CODEC_G726_SUPPORT) ? idx++ : _CODEC_MAX;;
	g_mapSupportedCodec[_CODEC_G726_40] =  (g_VoIP_Feature & CODEC_G726_SUPPORT) ? idx++ : _CODEC_MAX;;
#endif	
#ifdef CONFIG_RTK_VOIP_GSMFR
	g_mapSupportedCodec[_CODEC_GSMFR] = (g_VoIP_Feature & CODEC_GSMFR_SUPPORT) ? idx++ : _CODEC_MAX;
#endif
#ifdef CONFIG_RTK_VOIP_ILBC
	g_mapSupportedCodec[_CODEC_ILBC] =  (g_VoIP_Feature & CODEC_iLBC_SUPPORT) ? idx++ : _CODEC_MAX;;
#endif

#ifdef CONFIG_RTK_VOIP_G722
	g_mapSupportedCodec[_CODEC_G722] =	 (g_VoIP_Feature & CODEC_G722_SUPPORT) ? idx++ : _CODEC_MAX;;
#endif

#ifdef CONFIG_RTK_VOIP_RTK_VOIP_SPEEX_NB 
	g_mapSupportedCodec[_CODEC_SPEEX_NB] =  (g_VoIP_Feature & CODEC_SPEEX_NB_SUPPORT) ? idx++ : _CODEC_MAX;;
#endif

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

	if (level > DNS_CURRENT)
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

#ifdef CONFIG_RTK_VOIP_TR104
void solarSendResponseToCwmp(cwmpEvtMsg *evtMsg){
	int i, sendResponse=0;
    int iMaxVoIPPorts = RTK_VOIP_SLIC_NUM(g_VoIP_Feature);// If FXO support, check g_MaxVoIPPorts
	LinphoneCore *lc;
	voiceProfileLineStatus *VPLineStatus=NULL;
//	TstVoipRtpStatistics rtpStatistics;
	TstRtpRtcpStatistics rtpStatistics;
	g_message("solarSendResponseToCwmp\n");
	
	switch(cwmpEvtMsgGetEvent(evtMsg)){
		case EVT_VOICEPROFILE_LINE_GET_STATUS:
			for(i=0; i < iMaxVoIPPorts; i++){
				if((VPLineStatus = (voiceProfileLineStatus*)malloc(sizeof(voiceProfileLineStatus))) != NULL){
					memset( VPLineStatus, 0, sizeof( *VPLineStatus ) );
					linphone_call_log_statistics(&(solar.ports[i]), VPLineStatus);
					cwmpEvtMsgSetVPLineStatus(evtMsg,VPLineStatus,i);
					free(VPLineStatus);
				}
				/* read SIP IMS server setting ,from SUBSCRIBE/NOTIFY*/

				evtMsg->voiceProfileLineStatusMsg[i].featureFlags=rcm_tr104_st.voiceProfileLineStatusMsg[i].featureFlags;

			}

#if defined (CONFIG_E8B)
			/* copy tr104 auto test result to evtMsg*/
			memcpy(&evtMsg->e8c_autotest, &(rcm_tr104_st.e8c_autotest), sizeof(LinphoneTR104Test_t));
#endif
			sendResponse=1;
            cwmpEvtMsgSetEvent(evtMsg,EVT_VOICEPROFILE_LINE_GET_STATUS);
		break;
        case EVT_VOICEPROFILE_LINE_SET_STATUS:

			/* check run time setting , reset */
			for(i=0; i < iMaxVoIPPorts; i++){
				if(evtMsg->voiceProfileLineStatusMsg[i].resetStatistics==1){
					/* clean statistics */
					rcm_tapi_Get_Rtp_Statistics( i,0, 1, &rtpStatistics ) ;
					rcm_linphone_reset_call_log_statistics(&(solar.ports[i]));
				}
					
			}		
//#if 0 //old project not need it , for reduce memory usage	
#if defined (CONFIG_E8B)

			/* check tr104 auto */
			
			memcpy(&(rcm_tr104_st.e8c_autotest), &(evtMsg->e8c_autotest), sizeof(LinphoneTR104Test_t));


			printf("e8c_autotest.enable is %d\n",rcm_tr104_st.e8c_autotest.enable);
			printf("e8c_autotest.callednumber is %s\n",rcm_tr104_st.e8c_autotest.callednumber);
			printf("e8c_autotest.TestType is %d\n",rcm_tr104_st.e8c_autotest.TestType);
            printf("e8c_autotest.TestSelector is %d\n",rcm_tr104_st.e8c_autotest.TestSelector);
			printf("e8c_autotest.DailDTMFConfirmEnable is %d\n",rcm_tr104_st.e8c_autotest.DailDTMFConfirmEnable);		
			printf("e8c_autotest.DailDTMFConfirmNumber is %s\n",rcm_tr104_st.e8c_autotest.DailDTMFConfirmNumber);

			if((rcm_tr104_st.e8c_autotest.enable==1))
			{

				if(rcm_tr104_st.e8c_autotest.TestSelector==1)
				{
				if (rcm_tr104_st.e8c_autotest.TestType==0){
					/* caller test case */
				lc=&(solar.ports[0]);
				printf("\r\n start E8C TEST for Caller \n");
				rcm_timerLaunch(lc->e8c_tr104_timer,3*1000); 
				/* TBD */
				}else{
					/* callee test case , waiting incoming call */
					rcm_tr104_st.e8c_autotest.CalledFailReason=CALLEE_NO_INCOMINGCALL;
					rcm_tr104_st.e8c_autotest.Conclusion=1; /* fail */				

				}
				}else if(rcm_tr104_st.e8c_autotest.TestSelector==0)
				{
					rcm_tr104_st.e8c_autotest.PhoneConnectivity=rtk_GetPortStatus(0);//hard code get port 0
					rcm_tr104_st.e8c_autotest.TestStatus=2;
				}
			}
#endif
			break;
        case EVT_VOICEPROFILE_LINE_SET_DIAGNOSTICS:

            sendResponse=1;
            cwmpEvtMsgSetEvent(evtMsg,EVT_VOICEPROFILE_LINE_SET_DIAGNOSTICS);
			/* not need send response to CWMP client ? */
            //FIXME, DO SET
            //cwmpEvtMsgSetEvent(evtMsg,EVT_VOICEPROFILE_LINE_SET_STATUS);
            break;
		default:
		break;
	}
    
    if(sendResponse){
        /* Prepare to send response back to cwmpClient*/
        int sendSock=0;
        struct sockaddr_un addr;
		sendSock=socket(PF_LOCAL, SOCK_DGRAM, 0);
		bzero(&addr, sizeof(addr));
		addr.sun_family = PF_LOCAL;
		strncpy(addr.sun_path, CWMP_CLINET_PATH, sizeof(CWMP_CLINET_PATH));
		sendto(sendSock, (void*)evtMsg, cwmpEvtMsgSizeof(), 0, (struct sockaddr*)&addr, sizeof(addr));
		close(sendSock);
	}
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
#endif /*#ifdef CONFIG_RTK_VOIP_TR104*/

int
main (int argc, char *argv[])
{
	int i;
	int bAutoAnswer;
	//int ret;
	
	// read config file 
	app_read_config_file( 1 );


	// destroy old process and create a PID file
	if(rcm_set_pidfile()==0){
		printf("create pid file fail \n");
			return 0;
	}

	rcm_tapi_Get_VoIP_Feature();
	g_MaxVoIPPorts = RTK_VOIP_CH_NUM(g_VoIP_Feature);
	//RTK_VOIP_SLIC_NUM(g_VoIP_Feature) +RTK_VOIP_DAA_NUM(g_VoIP_Feature);

	if (g_MaxVoIPPorts > MAX_VOIP_PORTS || g_MaxVoIPPorts <=0 )
	{
		fprintf(stderr, "Incorrect g_MaxVoIPPorts=%d, MAX_VOIP_PORTS=%d !?\n",
			g_MaxVoIPPorts, MAX_VOIP_PORTS);
		return -1;
	}

#ifdef OPENWRT_BUILD
	g_pVoIPCfg = (voipCfgParam_t *) malloc(sizeof(voipCfgParam_t));
	voip_uci_config_get(g_pVoIPCfg);
#else

#ifdef VOIP_FLASH_ARCH	//Alex, original architecture
	if (rcm_voip_flash_client_init(&g_pVoIPShare, VOIP_FLASH_WRITE_CLIENT_SOLAR) == -1)
	{
		fprintf(stderr, "voip_flash_client_init failed.\n");
		return -1;
	}

#if 0
    if (voip_flash_server_init_variables() == -1)
    {
		fprintf(stderr, "voip_flash_server_init_variables failed.\n");
		//return -1;
    }
#endif

	g_pVoIPCfg = &g_pVoIPShare->voip_cfg;
#else
	if (rcm_voip_flash_client_init(&g_pVoIPCfg, VOIP_FLASH_WRITE_CLIENT_SOLAR) == -1)
	{
		fprintf(stderr, "voip_flash_client_init failed.\n");
		return -1;
	}
    
/*	if (voip_flash_get(&g_pVoIPCfg)==-1)
    {
    	fprintf(stderr, "voip flash get failed. \n");
    	return -1;
    }*/
#endif	//VOIP_FLASH_ARCH
#endif
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
#if 0	
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
			rcm_tapi_DisableRingFXS(TRUE);
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
#endif
	
	if (argc==4) {
 	   if (strcmp(argv[1], "-wdt")==0) {
    	    // ex_app runs in pipe-mode.
       	 // the handle was passed into
       	 printf("watch dog pipe mode \n");
       	 RCM_WATCHDOG_pipe=atoi(argv[2]);
       	 if ((RCM_WATCHDOG_pipe<0) || (RCM_WATCHDOG_pipe>4096)) {
       	     g_message("VoIP:Error: a strange pipe-number = %d\n", RCM_WATCHDOG_pipe);       	    
       	 }
   		} else if (strcmp(argv[1], "-wdt_fifo")==0) {
	        // ex_app runs in fifo-mode
	        g_message("VoIP:watch dog fifo  mode \n");
			wdt_fifo=argv[2];
			// open the fifo in order of the succession of gwdt
			if ((RCM_WATCHDOG_pipe=open(wdt_fifo, O_WRONLY))<0) {
				g_message("VoIP:can't open fifo '%s'\n", wdt_fifo);
           	 return 1;
        	}
		
		}
		RCM_WATCHDOG_period=atoi(argv[3]);
		g_message("RCM_WATCHDOG_period is %d\n",RCM_WATCHDOG_period);
		if ((RCM_WATCHDOG_period<0) || (RCM_WATCHDOG_period>86400)) {
		   g_message("VoIP:Watchdog: an illegal timeout = %d\n", RCM_WATCHDOG_period);
		}
			
	}


	//rcm_voip_dsp_booting();

	for (i=0; i<g_MaxVoIPPorts; i++)
	{
		/* init */
		rcm_tapi_InitDSP(i);
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

/* support RCM Watch dog */
	if(RCM_WATCHDOG_period>0){
		rcm_timerLaunch(solar_ptr->rcm_linphone_watchdog_timer, ((RCM_WATCHDOG_period*2)/5)*1000);  
	}

	// set debug flag
	solar.bAutoAnswer = bAutoAnswer;

	rcm_reset_web_register_status();

	app_run(&solar);

	app_close(&solar);

#ifdef RTK_DNS
	my_getaddrinfo_destroy();
#endif

#ifdef MALLOC_DEBUG
	mwTerm();
#endif

#ifdef VOIP_FLASH_ARCH	//Alex, original architecture
	voip_flash_client_close();
#endif	//VOIP_FLASH_ARCH

	return 0 ;
}

//[SD6, ericchung, rcm integration
/*______________________________________________________________________________
**	offhook_dialtimeout
**
**	descriptions: when user offhook , start offhook timeout timer, 
                  if offhook dial timeout , start play warning tone 
**	parameters: chid 
**	local:
**	global:
**	return:
**	called by:
**	call:
**	revision:
**____________________________________________________________________________*/


void offhook_dialtimeout(void *arg){
	
	LinphoneCore *lc;
	int nPort=	(int)arg;
	g_message("offhook_dialtimeout %d\n",nPort);
	
	lc = &solar_ptr->ports[nPort];

	//g_message("\r\n off_hook_start is %u:%u\n",lc->off_hook_start.tv_sec,lc->off_hook_start.tv_usec);

	//g_message("\r\nLinphone: offhook_dialtimeout off_hook_alarm is %d\n ",g_pVoIPCfg->off_hook_alarm);
	//osip_gettimeofday(&now, NULL);
	//diff = now.tv_sec - opm->off_hook_start.tv_sec;
	//if (diff >= g_pVoIPCfg->off_hook_alarm)
	//{
		//if (opm->chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
		if( RTK_VOIP_IS_DAA_CH( lc->chid, g_VoIP_Feature ) )
			rcm_tapi_FXO_Busy(lc->chid);
		else
			rcm_tapi_SetPlayTone(lc->chid, GetActiveSession(lc->chid), 
				DSPCODEC_TONE_OFFHOOKWARNING, OFFHOOKWARNING, DSPCODEC_TONEDIRECTION_LOCAL);


		//ericchung: fxs offhook too long, play offhook warning tone

		SetSysState(lc->chid, GetSysState(lc->chid) + 1); //change to SYS_STATE_XXX_ERROR
	//}
}



/*______________________________________________________________________________
**	timeout_playbusytone
**
**	descriptions: when timeout , play some tone.
**	parameters: chid 
**	local:
**	global:
**	return:
**	called by:
**	call:
**	revision:
**____________________________________________________________________________*/


void timeout_playbusytone(void *arg){
	
	LinphoneCore *lc;
	int nPort=	(int)arg;
	g_message("timeout_playbusytone %d\n",nPort);
	
	lc = &solar_ptr->ports[nPort];

///	printf("\r\n off_hook_start is %u:%u\n",lc->off_hook_start.tv_sec,lc->off_hook_start.tv_usec);

//	printf("\r\nLinphone: timeout_playbusytone off_hook_alarm is %d\n ",g_pVoIPCfg->off_hook_alarm);
//	osip_gettimeofday(&now, NULL);
	//diff = now.tv_sec - opm->off_hook_start.tv_sec;
//	if (diff >= g_pVoIPCfg->off_hook_alarm)
//	{
		//if (opm->chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
	//	if( RTK_VOIP_IS_DAA_CH( lc->chid, g_VoIP_Feature ) )
		//	rcm_tapi_FXO_Busy(lc->chid);
//		else
			rcm_tapi_SetPlayTone(lc->chid, GetActiveSession(lc->chid), 
				DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);

		//ericchung: fxs offhook too long, play offhook warning tone

//		SetSysState(lc->chid, GetSysState(lc->chid) + 1); //change to SYS_STATE_XXX_ERROR
//	}


}

/*______________________________________________________________________________
**	rcm_options_ping_timeout
**
**	descriptions: when send OPTIONS to SIP Server (check Server alive) start timer.
                  , when not receive 200ok ,it timeout , run this function.
**	parameters: chid 
**	local:
**	global:
**	return:
**	called by:
**	call:
**	revision:
**____________________________________________________________________________*/


void rcm_options_ping_timeout(void *arg){
	
	LinphoneCore *lc;
	LinphoneProxyConfig *proxy;
	voipCfgPortParam_t *voip_ptr = NULL;
	voipCfgProxy_t *proxy_cfg=NULL;
	int nPort=(int)arg;
	int i;

	g_message("rcm_options_ping_timeout nport is %d\n",nPort);
	lc = &solar_ptr->ports[nPort];
	voip_ptr = &g_pVoIPCfg->ports[lc->chid];
	proxy_cfg = &voip_ptr->proxies[0];
	linphone_core_get_default_proxy(lc, &proxy);	

/*      for sip redundancy:
        when main proxy register success, 
        send options messgse to Main proxy.
        if Main proxy shutdown happen SIP options no response.
        call manager send register to BACKUP proxy and success.
        call manager send SIP option to BACKUP proxy check backup proxy alive.
        if BACKUP proxy no resopnse , call manger will back send Register to MAIN proxy.

		no SIP redundancy:
		OPTIONS only send to proxy for keep live.
		
*/
	if(g_pVoIPCfg->rfc_flags & SIP_SERVER_REDUNDANCY){	
		if(lc->OptionsPingStatus==Option_WaitACK){ /* proxy not response 200ok for OPTIONS */
			
			g_message("rcm_options_ping_timeout proxy index is proxy->index %d\n",proxy->index);

			/*main proxy no response options , if have backup proxy, send register to backup proxy 
	          else send re-register to main proxy again  */
			if(proxy->index==0) {
				g_message("main proxy options fail \n");
				if(( lc->proxies[1])&& (lc->proxies[1]->registered != TRUE) ){
						//case 1, second proxy not register , try second proxy
						printf("rcm_options_ping_timeout try to second proxy \n");
						//linphone_proxy_send_unregister(proxy); //unregister 
						
						linphone_core_set_default_proxy(lc, lc->proxies[1]);					
						linphone_proxy_config_enableregister(lc->proxies[1],1); //enable proxy conf						
						linphone_proxy_config_register(lc->proxies[1]);

				}else{ /* only have main proxy, re-send register to main proxy*/
					g_message("rcm_options_ping_timeout only main proxy,retry to main proxy\n");
					linphone_proxy_send_register_again(lc->proxies[0]);
				}
			
			}else{/* second proxy fail */
				g_message("\r\n options to second proxy fail \n");
				g_message("\r\n retry main proxy after 5s\n");
				rcm_timerLaunch(lc->register_retry_timer,5*1000);	//5

			}
			
		}else if (lc->OptionsPingStatus==Option_Next_Ping){
			g_message("\r\n Option_Next_Ping \n");
			lc->OptionsPingStatus=Option_WaitACK;		
			rcm_linphone_core_send_options(lc,proxy);
			rcm_timerLaunch(lc->options_ping_timer,proxy_cfg->HeartbeatCycle*1000); 
		}

	}else{	//no SIP redundancy feature case:
		/* only send options to sip server */

		if(lc->OptionsPingStatus==Option_WaitACK){
			//option no response from proxy 
			g_message("\r\n OPTION send fail, proxy no response ,register again!\n");

			/* re-register for all proxy */
			for (i=0;i<MAX_PROXY; i++){
				if( lc->proxies[i]){							
					linphone_proxy_config_register(lc->proxies[i]);
				}
			}
			return;
		}

		/* resend option to all proxy */
		for (i=0;i<MAX_PROXY; i++){
			if(( lc->proxies[i])&& (lc->proxies[i]->registered == TRUE) ){				
				lc->OptionsPingStatus=Option_WaitACK;
				rcm_linphone_core_send_options(lc,lc->proxies[i]);
				rcm_timerLaunch(lc->options_ping_timer,proxy_cfg->HeartbeatCycle*1000); 
				
			}
		}
	}
}


/*______________________________________________________________________________
**	fxs_flashkeytimeout
**
**	descriptions: when user press flash hook, not enter next function digits, 
                  , it will timeout to reset flashkey bit
**	parameters: chid 
**	local:
**	global:
**	return:
**	called by:
**	call:
**	revision:
**____________________________________________________________________________*/


void fxs_flashkeytimeout(void *arg){
	
	LinphoneCore *lc;
	int nPort=	(int)arg;
	g_message("offhook_dialtimeout %d\n",nPort);
	int active_session;
	lc = &solar_ptr->ports[nPort];
	active_session = GetActiveSession(lc->chid);

	lc->fxsfeatureFlags &= ~FXS_FLEASH_KEY;

	//need check country
	if(g_pVoIPCfg->tone_of_country==TONE_GERMANY)	
		rcm_tapi_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_STUTTERDIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
	else
		rcm_tapi_SetPlayTone(lc->chid, active_session, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
		



}

/*______________________________________________________________________________
**	rerty_register
**
**	descriptions: rerty_register main proxy.
**	parameters: chid 
**	local:
**	global:
**	return:
**	called by:
**	call:
**	revision:
**____________________________________________________________________________*/

void rerty_register(void *arg){
	
	LinphoneCore *lc;
	int nPort=	(int)arg;
	int i;
	g_message("rerty_register %d\n",nPort);
	
	
	lc = &solar_ptr->ports[nPort];

	if(g_pVoIPCfg->rfc_flags & SIP_SERVER_REDUNDANCY){
		/*if sip server redundancy ,only retry main proxy */
		if( lc->proxies[0] && (lc->proxies[0]->registered!=TRUE)){
			//printf("try to main  proxy  register again\n");
			//linphone_core_set_default_proxy(lc, lc->proxies[0]);
			linphone_proxy_config_enableregister(lc->proxies[0],TRUE); //enable proxy conf					
			linphone_proxy_config_register(lc->proxies[0]);
		}
	}else{
		/* retry for all proxy */
		for (i=0;i<MAX_PROXY; i++){
			if(( lc->proxies[i])&& (lc->proxies[i]->registered != TRUE) ){				
				linphone_proxy_config_enableregister(lc->proxies[i],TRUE); //enable proxy conf					
				linphone_proxy_config_register(lc->proxies[i]);
			}
		}
	}

}




/*______________________________________________________________________________
**	stop_alltone
**
**	descriptions: stop any tone.
**	parameters: chid 
**	local:
**	global:
**	return:
**	called by:
**	call:
**	revision:
**____________________________________________________________________________*/


void stop_alltone(void *arg){
	
	LinphoneCore *lc;
	int nPort=	(int)arg;
	g_message("stop_alltone %d\n",nPort);
	
	lc = &solar_ptr->ports[nPort];


		rcm_tapi_SetPlayTone(lc->chid, GetActiveSession(lc->chid), 
				DSPCODEC_TONE_OFFHOOKWARNING, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);


}


/*______________________________________________________________________________
**	dial_to_hotline
**
**	descriptions: dial to hotline
**	parameters: chid 
**	local:
**	global:
**	return:
**	called by:
**	call:
**	revision:
**____________________________________________________________________________*/


void dial_to_hotline(void *arg){
	
	LinphoneCore *lc;
	char temp_str[40]="";
	int nPort=	(int)arg;
	
	lc = &solar_ptr->ports[nPort];

	sprintf(temp_str,"call %s",g_pVoIPCfg->ports[lc->chid].hotline_number);
	linphonec_parse_command_line(lc, GetActiveSession(lc->chid), temp_str);
	//start auto dial, stop dial tone, cancel offhook timer.
	rcm_tapi_SetPlayTone(lc->chid, GetActiveSession(lc->chid), DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
	rcm_timerCancel(lc->offhook_timer);
	rcm_timerCancel(lc->busytone_timer);
	rcm_timerCancel(lc->stop_alltone_timer);
}
void no_answer_timeout(void *arg){
	
	LinphoneCore *lc;
	int nPort=	(int)arg;
	SYSTEMSTSTE state;
	int i;
	
	lc = &solar_ptr->ports[nPort];
	for (i=0;i<MAX_SS;i++)
	{
		if (GetSessionState(lc->chid, i)== SS_STATE_CALLIN)
		{
			g_message("Port %d session %d no_answer_timeout\n",nPort, i);
			if (lc->call[i])
			{
				eXosip_lock() ;
				eXosip_call_send_answer(lc->call[i]->tid, 408, NULL);
				eXosip_unlock() ;

				linphone_call_destroy(lc->call[i]);
				if (lc->ringstream[i])
				{
					lc->ringstream[i] = FALSE;
					rcm_tapi_SetRingFXS(lc->chid, FALSE);
				}
				lc->call[i] = NULL;
				SetSessionState(lc->chid, i, SS_STATE_IDLE);
				if (GetSysState(lc->chid) == SYS_STATE_RING)
					SetSysState(lc->chid, SYS_STATE_IDLE);

				return;
			}
		}
	}
	printf("error : Port %d no_answer_timeout but no call\n",nPort);
	return;
}

/*______________________________________________________________________________
**	dial_interval_timeout
**
**	descriptions: when user dial digit ,start the dial timer, 
**                if timeout , start dialing to sip/pstn
**	parameters: chid
**	local:
**	global:
**	return:
**	called by:
**	call:
**	revision:
**____________________________________________________________________________*/

void dial_interval_timeout(void *arg){
	
	LinphoneCore *lc;
	int nPort=	(int)arg;
	uint32 SessionID = 0 ;


	//rcm_tapi_print(RTK_DBG_INFO, "Linphone", "*** dial_interval_timeout");
	syslog(LOG_DEBUG, "Syslog test test test ");
	//g_message("*** dial_interval_timeout %d\n",nPort);
	
	lc = &solar_ptr->ports[nPort];

	//printf("\r\n fxsdialtimeout is %u:%u\n",lc->off_hook_start.tv_sec,lc->off_hook_start.tv_usec);
	//printf("\r\nLinphone: offhook_dialtimeout off_hook_alarm is %d\n ",g_pVoIPCfg->off_hook_alarm);
	
	if(lc->dial_data_index==0){
		g_message("*** dial_interval_timeout ,bug dial_data_index is 0, no input\n");
		return;
	}
	
	switch (GetSysState(lc->chid)){
#ifdef CONFIG_RTK_VOIP_IVR
	case SYS_STATE_AUTH:
#endif		
	case SYS_STATE_EDIT:
	case SYS_STATE_CONNECT_EDIT:
	case SYS_STATE_TRANSFER_EDIT:
		/* Add a virtual key to compatible with dial plan. */						   
#ifdef CONFIG_RTK_VOIP_DIALPLAN //eric.chung: currently support DIALPLAN
		//g_message("\r\n aa lc->lc_input is %s, input[chid] is %s\n",lc->lc_input,input[lc->chid]);
		CallDigitEdit(lc, &SessionID, SIGN_AUTODIAL, lc->lc_input);
		//g_message("\r\n bb chid %d  lc->lc_input is %s, input[chid] is %s\n",lc->chid,lc->lc_input,input[lc->chid]);
#else
		//ericchung: not run here
		CallDigitEdit(lc, &SessionID, SIGN_HASH, lc->lc_input);
#endif
		//ericchung:remove it , to fix dial number twnic issue 
		//linphonec_parse_command_line(lc, SessionID, lc->lc_input);
		break;
	default:
		break;
   	}
}


void rcm_active_BusyWrongTone_timer(LinphoneCore *lc)
{

	//TBD: Eric, e8c case, timer must can modify.
	rcm_timerLaunch(lc->busytone_timer,(g_pVoIPCfg->off_hook_alarm*1000));  //after offhook alarm time, play busy tone
	rcm_timerLaunch(lc->offhook_timer,((g_pVoIPCfg->off_hook_alarm+g_pVoIPCfg->BusyToneTimer)*1000)); //play wrong tone
	rcm_timerLaunch(lc->stop_alltone_timer,((g_pVoIPCfg->off_hook_alarm+g_pVoIPCfg->BusyToneTimer+g_pVoIPCfg->HangingReminderToneTimer)*1000)); 

	return;
}

void rcm_configure_restart(void *arg){

	int i;
	LinphoneCore *lc;
	int nPort=	(int)arg;

	lc = &solar_ptr->ports[nPort];

	printf("VoIP: rcm_configure_restart \n");
	for (i=0; i<g_MaxVoIPPorts; i++)
	{
		if(GetSysState(i)!=SYS_STATE_IDLE){
			printf("\r\nFxs port not IDLE, delay 3 sec for restart\n");
			rcm_timerLaunch(solar_ptr->rcm_linphone_reset_timer,3000);	//every 5sec check again.
			return;
		}
					
	}

	// move to reset timer rcm_configure_restart, cancel watchdog need wait line state change idle 	
	if(RCM_WATCHDOG_period!=0){
		write(RCM_WATCHDOG_pipe, "123", 4);
		rcm_timerCancel(solar_ptr->rcm_linphone_watchdog_timer); //use watchdog for reset.
	}
	
	for (i=0; i<g_MaxVoIPPorts; i++)
	{
		HookAction(&solar_ptr->ports[i], SIGN_ONHOOK, input[i]);
		
	}
//	printf("\r\n Receive FIFO Command x , for restart solars \n");
	if(RCM_WATCHDOG_period>0)
		write(RCM_WATCHDOG_pipe, "123", 4);


	app_close(solar_ptr);
#ifdef VOIP_FLASH_ARCH	//Alex, original architecture
	voip_flash_client_update();
#else
	if (voip_flash_get(&g_pVoIPCfg)==-1)
	{
	    fprintf(stderr, "voip flash get failed. \n");
	    return -1;
	}
#endif	//VOIP_FLASH_ARCH

#ifdef CONFIG_RTK_VOIP_IVR
	NoticeIVRUpdateFlash_IPC();
#endif



	app_init(solar_ptr);
	
	if(RCM_WATCHDOG_period>0)
		write(RCM_WATCHDOG_pipe, "123", 4);	

	/* when restart , need register again */
	rcm_linphone_register_init(solar_ptr);
	if(RCM_WATCHDOG_period>0){
			rcm_timerLaunch(solar_ptr->rcm_linphone_watchdog_timer, ((RCM_WATCHDOG_period*2)/5)*1000);	
		}

	g_message("restart done.\n");


}

void rcm_linphone_watchdog_hander(void *arg){
//	printf("rcm_linphone_watchdog_hander \n");
	write(RCM_WATCHDOG_pipe, "123", 4);
	if(RCM_WATCHDOG_period>0){
		rcm_timerLaunch(solar_ptr->rcm_linphone_watchdog_timer, ((RCM_WATCHDOG_period*2)/5)*1000);  
	}

//eric add check interface
	if(strlen(solar_ptr->current_voip_interface)==0){
		rcm_retreive_voip_if_name(solar_ptr->current_voip_interface, DNS_LEN);
		if(strlen(solar_ptr->current_voip_interface)!=0){
				rcm_timerLaunch(solar_ptr->rcm_linphone_reset_timer,1000);  //after 1 sec check again.
		}
	}

}



void rcm_linphone_sip_restart(TAPP *pApp)
{

	//int i;
	//int err;
	//GList *elem;
	//LinphoneCore *lc;
printf("\r\n rcm_linphone_sip_restart \n");
	if(RCM_WATCHDOG_period>0)
		write(RCM_WATCHDOG_pipe, "123", 4);

	rcm_exosip_close( pApp);

	if(RCM_WATCHDOG_period>0)
		write(RCM_WATCHDOG_pipe, "123", 4);

#ifdef VOIP_FLASH_ARCH	//Alex, original architecture
	voip_flash_client_update();
#else
     if (voip_flash_get(&g_pVoIPCfg)==-1)
     {
         fprintf(stderr, "voip flash get failed. \n");
         return -1;
     }
#endif	//VOIP_FLASH_ARCH

	rcm_exosip_init(pApp);
	
	/* when restart , need register again */
	rcm_linphone_register_init(solar_ptr);

}

#if defined (CONFIG_RTK_VOIP_TR104) && defined (CONFIG_E8B) 
/*______________________________________________________________________________
**	e8c_dtmf_test
**
**	descriptions: e8c_dtmf_test.
**	parameters: chid 
**	local:
**	global:
**	return:
**	called by:
**	call:
**	revision:
**____________________________________________________________________________*/


void e8c_dtmf_test(void *arg){
	
	LinphoneCore *lc;
	int nPort=	(int)arg;
	int digits;	
	lc = &solar_ptr->ports[nPort];
	char src_temp[40];
	LinphoneTR104Test_t *e8c_autotest=&(rcm_tr104_st.e8c_autotest);	
	
	switch (e8c_autotest->TestType){
		case 0: //caller 

			if(e8c_autotest->status==0){
				/* caller start test,  step 1, offhook and dial number */
				{
					char temp_str[40]="";
					//ctl_line=0;
//					SessionID=0;
					//rcm_tr104_st.e8c_autotest.enable=1; //enable auto dial
					//rcm_tr104_st.e8c_autotest.TestType=0; //for caller
					//ctl_lc = &pApp->ports[0]; //use fxs0 for test 
					sprintf(temp_str,"call %s",rcm_tr104_st.e8c_autotest.callednumber);
					/* offhook */
					printf("\r\n try offhook , dial to %s\n",temp_str);
					rcm_tapi_enable_pcm(lc->chid, 1);
					e8c_autotest->status=1;
					e8c_autotest->Conclusion=1; /* 1 is fail */
					e8c_autotest->dtmfindex=0;

					if( ( lc->default_proxy == NULL ) ||( !lc->default_proxy->registered ) ){
						/* not register success */
						
						e8c_autotest->CallerFailReason=CALLER_NO_DIALTONE;
						break;
					}
	

#if 1
					HookAction(lc, SIGN_OFFHOOK, input[0]);
					linphonec_parse_command_line(lc, 0, temp_str);										
					/*start dial to peer, record status is no answer */
					e8c_autotest->CallerFailReason=CALLER_NO_ANSWER;
#endif					
					break;
				}	

			}


			
			if(e8c_autotest->dtmfindex <strlen(e8c_autotest->DailDTMFConfirmNumber)){
				/* step2 , dial dtmf to called */
				digits=SIGN_KEY1 + e8c_autotest->DailDTMFConfirmNumber[e8c_autotest->dtmfindex] - '1';				
				printf("\r\n[TR104] auto dial dtmf %d  \n",digits);			
				linphone_core_send_dtmf(lc, 0,digits);
				e8c_autotest->dtmfindex++;
				strcpy(e8c_autotest->Simulate_Status,"Connected");
				e8c_autotest->CallerFailReason=CALLER_AFTERDIAL_RELEASE;
				rcm_timerLaunch(lc->e8c_tr104_timer,2*1000); //every 2 sec , dail one digis
			}else{
				/* step3. no dtmf need dial, offhook it*/
				printf("\r\n[TR104] auto onhook \n");
				HookAction(lc, SIGN_ONHOOK, NULL);
				rcm_tapi_enable_pcm(lc->chid, 0);
				linphone_core_terminate_dialog(lc, 0, NULL);// terminal sip (BYE)
				e8c_autotest->dtmfindex=0;
				printf("\r\ne8c_autotest.ReceiveDTMFNumber is %s\n",e8c_autotest->ReceiveDTMFNumber);
				/*TBD: compare receive dtmf with DailDTMFConfirmNumber */
				e8c_autotest->CallerFailReason=CALLER_NONE;
				strcpy(e8c_autotest->Simulate_Status,"Tested");
				
				
				e8c_autotest->enable=0;
				e8c_autotest->TestStatus=2; /*Complete*/

                
				if(strcmp(e8c_autotest->DailDTMFConfirmNumber,e8c_autotest->ReceiveDTMFNumber)!=0){ /* fail case */
					printf("\r\n check receive dtmf and dial dtmf ,  no match \n");					
					e8c_autotest->Conclusion=1; /* 1 is fail */
					e8c_autotest->DailDTMFConfirmResult=0; /* 0 is fail */
				}else{ /* test ok */
				printf("\r\n check receive dtmf and dial dtmf ,  match \n");	
					e8c_autotest->Conclusion=0; /* 0 is success */
					e8c_autotest->DailDTMFConfirmResult=1; /* 1 is true */
				}
                //send to cwmpclient. //Alex
                printf("Do event send back....\n");
                cwmpEvtMsgSetEvent(&rcm_tr104_st, EVT_VOICEPROFILE_LINE_SET_DIAGNOSTICS);
                solarSendResponseToCwmp(&rcm_tr104_st);
			}
			break;
		case 1: //callee
			if(e8c_autotest->status==0){
					printf("\r\n auto answer the call \n");
					HookAction(lc, SIGN_OFFHOOK, src_temp);
					printf("\r\n src_temp is %s\n",src_temp);
					linphonec_parse_command_line(lc, 0, src_temp);
					e8c_autotest->status=1; //start send dtmf
					e8c_autotest->dtmfindex=0;
					rcm_timerLaunch(lc->e8c_tr104_timer,3*1000); //every 3 sec , dail one digi 
			}else if (e8c_autotest->status==1){
				if(e8c_autotest->dtmfindex <strlen(e8c_autotest->DailDTMFConfirmNumber)){
					digits=SIGN_KEY1 + e8c_autotest->DailDTMFConfirmNumber[e8c_autotest->dtmfindex] - '1';
		
					printf("\r\n[TR104] auto dial dtmf %d  \n",digits);						
					linphone_core_send_dtmf(lc, 0,digits);					
					e8c_autotest->dtmfindex++;
					rcm_timerLaunch(lc->e8c_tr104_timer,2*1000); //every 2 sec , dail one digi 
				}else{
						/* no dtmf need dial, offhook it*/

						printf("\r\n[TR104] auto onhook \n");			
						printf("\r\n	e8c_autotest.ReceiveDTMFNumber is %s\n",e8c_autotest->ReceiveDTMFNumber);
						HookAction(lc, SIGN_ONHOOK, NULL);
						linphone_core_terminate_dialog(lc, 0, NULL);	// reset all sip
						e8c_autotest->status=0; //0 , not start
						e8c_autotest->dtmfindex=0;
						e8c_autotest->TestStatus=2; /*Complete*/
						/*TBD: compare receive dtmf with DailDTMFConfirmNumber */
						e8c_autotest->enable=0;
			            //send to cwmpclient. //Alex
			            printf("Do event send back....\n");
            			cwmpEvtMsgSetEvent(&rcm_tr104_st, EVT_VOICEPROFILE_LINE_SET_DIAGNOSTICS);
			            solarSendResponseToCwmp(&rcm_tr104_st);
				}
			}
			break;	
	}
}



//ericchung: for tr104 test  ,data is hardcopy 
void tr104_auto_test_hardcopy(void){
	LinphoneTR104Test_t *e8c_autotest=&(rcm_tr104_st.e8c_autotest); 
	e8c_autotest->enable=1; //enable auto test , 0 is disable
	strcpy(e8c_autotest->callednumber,"66002");
	e8c_autotest->TestType=0; //called or caller
	e8c_autotest->DailDTMFConfirmEnable = 1; //enable dtmf  test
	strcpy(e8c_autotest->DailDTMFConfirmNumber,"1234"); //dtmf play string
	e8c_autotest->status=0; //0 , not start
	e8c_autotest->dtmfindex=0;
	e8c_autotest->receive_dtmf_index=0;
	e8c_autotest->TestSelector=1;
	e8c_autotest->DailDTMFConfirmResult =0;	
}
#endif /* CONFIG_RTK_VOIP_TR104 */  


void rcm_IvrPolling_proc( LinphoneCore *lc )
{
	if (GetSessionState(lc->chid, GetActiveSession(lc->chid)) == SS_STATE_IDLE)
	{
		switch (GetSysState(lc->chid))
		{

#ifdef CONFIG_RTK_VOIP_IVR
		case SYS_STATE_VOICE_IVR:
			if( rcm_tapi_IvrPollPlaying( lc->chid ,0) == 0 ) {
 #ifdef PLAY_BUSY_TONE_AFTER_IVR
				/* playing IVR done -> play busy tone */
				rcm_tapi_SetPlayTone(lc->chid, GetActiveSession(lc->chid), 
							DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
				SetSysState( lc ->chid, SYS_STATE_VOICE_IVR_DONE );
 #else
 						/* playing IVR done -> play dial tone & edit to dial */
				rcm_tapi_SetPlayTone(lc->chid, GetActiveSession(lc->chid), 
							DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
					digitcode_init( lc );
					SetSysState( lc ->chid, SYS_STATE_EDIT );
				g_message("\r\n start lc->off_hook_start 001");
				osip_gettimeofday(&lc->off_hook_start, NULL);
				rcm_active_BusyWrongTone_timer(lc);
#endif
			}
			break;
#endif /* CONFIG_RTK_VOIP_IVR */
		default:
			// do nothing
			break;
			}
	}
}

void rcm_linphone_NoAnswerCheck( LinphoneCore *lc ,int ActiveLine)
{
	int ss;
#ifdef CONFIG_RTK_VOIP_DIALPLAN
	voipCfgPortParam_t *voip_ptr = &g_pVoIPCfg->ports[ lc->chid ];
#endif
	struct timeval now;
	unsigned long diff;
	
	// check no answer time
		for (ss=0; ss<MAX_SS; ss++)
		{
			int fxo_call;
			int fxo_idx;

			if (GetSessionState(ActiveLine, ss) != SS_STATE_CALLIN ||
						lc->call[ss] == NULL)
						continue;

		      	// rock: check fxo redial
				fxo_idx = rtk_call_check_fxo_call_fxs(lc->call[ss]);
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
					lc->sip_conf.na_forward_enable &&
					lc->sip_conf.na_forward[0])
#endif
				{
					osip_gettimeofday(&now, NULL);
					diff = now.tv_sec - lc->call[ss]->no_answer_start.tv_sec;
#ifdef CONFIG_RTK_VOIP_DIALPLAN
					if ((!fxo_call&&
							diff > voip_ptr->na_forward_time) ||
						(fxo_call &&
							diff > g_pVoIPCfg->ports[fxo_idx].na_forward_time)
						)
#else
					if (diff > lc->sip_conf.na_forward_time)
#endif
					{
#ifdef CONFIG_RTK_VOIP_DIALPLAN
						if (!fxo_call)
						{
							char szForward[100] = {0};
								CallForwardByDialPlan(lc, voip_ptr->na_forward, szForward, ".");
							linphone_core_forward_dialog(lc, ss, szForward);
						}
						else if (g_pVoIPCfg->ports[fxo_idx].na_forward_enable == 1)
						{
							// 2 stage dialing
							linphone_core_forward_dialog(lc, ss, "sip:fxo_redial@127.0.0.1");
						}
						else
						{
							// direct forward
							char szForward[100] = {0};
							CallForwardByDialPlan(lc, g_pVoIPCfg->ports[fxo_idx].na_forward, szForward, ".");
							linphone_core_forward_dialog(lc, ss, szForward);
						}
#else
						linphone_core_forward_dialog(lc, ss, lc->sip_conf.na_forward);
#endif
						SetSessionState(ActiveLine, ss, SS_STATE_IDLE);
						lc->ringstream[ss] = FALSE;
						rcm_tapi_SetRingFXS(ActiveLine, FALSE);
						rcm_tapi_SetPlayTone(ActiveLine, ss, DSPCODEC_TONE_HOLD, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
						switch (GetSessionState(ActiveLine, ss == 0 ? 1 : 0))
						{
							case SS_STATE_IDLE:
							case SS_STATE_CALLOUT:
								SetSysState(ActiveLine, SYS_STATE_IDLE);
								break;
							case SS_STATE_CALLER:
							case SS_STATE_CALLEE:

								SetSysState(ActiveLine, SYS_STATE_CONNECT);
								break;
							default:
								g_error("unknown state in checking no answer\n");
								break;
						}
					}
				}
			}			
}



int rcm_set_linphone_exosip_listen_port(int transport, const char *addr, int port, int family,
				   int secure)
{
	int err=0;
		err = eXosip_listen_addr (transport, addr, port, family, 0);
/* configure 100/180 contact */
	if(addr!=NULL)
		eXosip_set_option(EXOSIP_OPT_SET_IPV4_FOR_GATEWAY,addr);
	
	if (err < 0)
	{
		g_error("UDP port seems already in use ! Cannot initialize.\n");
		return -1;
	}
	
	if(addr!=NULL)
		eXosip_masquerade_contact(addr,port);


}

#ifdef CONFIG_RTK_VOIP_IVR
void rcm_start_IVR(){
	
	InitIvrClient();
	SendGlobalConstantToIVR_IPC( g_nMaxCodec, g_mapSupportedCodec );
}
#endif

void rcm_init_linphone_voip_interface(void){

	/* if voip_mib no voip interface, get all interface from wan */
	if(strlen(g_pVoIPCfg->voip_interface)==0){
		rcm_retreive_voip_if_name(solar_ptr->current_voip_interface, DNS_LEN);
	}else{
		strcpy(solar_ptr->current_voip_interface,g_pVoIPCfg->voip_interface);
	}
	
}

void rcm_init_sim_voip_interface(void){
/* for test */
	const char *sim_ifp=getenv("SIM_VOIP_IFACE");
	const char *sim_iface=NULL;
	if(sim_ifp==NULL)
		sim_iface="eth0";
	else{
		if( !strcmp(sim_ifp, "eth0_0") ){
			sim_iface="eth0:0";
		}
		else if( !strcmp(sim_ifp, "eth0_1") ){
			sim_iface="eth0:1";
		}
		else if( !strcmp(sim_ifp, "lo") ){
			sim_iface="lo";
		}
		else {
			sim_iface="eth0";
		}
	}
	strcpy(solar_ptr->current_voip_interface , sim_iface);
	//rcm_dp("sim_ifp=%s sim_iface=%s\n" , sim_ifp , solar_ptr->current_voip_interface);
	rcm_dp("SIM_VOIP interface=%s\n" , solar_ptr->current_voip_interface);


}

int rcm_set_sim_exosip_listen_port(int transport, const char *addr, int port, int family,
				   int secure)
{

	const char *listen_port=getenv("SIP_LISTEN_PORT");
	const char *assign_port=NULL;
	int err=0;
	if(listen_port)	
		assign_port = listen_port;
	else
		assign_port = "5060";

	rcm_sim_log("listen_port=%s assign_port=%s\n" , listen_port , assign_port);

		err = eXosip_listen_addr (transport, addr, atoi(assign_port), family, 0);
/* configure 100/180 contact */
//		eXosip_set_option(EXOSIP_OPT_SET_IPV4_FOR_GATEWAY,addr);
	if (err < 0)
	{
		g_error("UDP port seems already in use ! Cannot initialize.\n");
		return -1;
	}
	
	if(addr!=NULL)
		eXosip_masquerade_contact(addr,atoi(assign_port));

	return err;

}

void rcm_set_sim_conctrol_fifo(void){

	const char* fptr = getenv("RCM_NAME");
	if(fptr==NULL)
		FIFO_CONTROL = "/var/run/maserati_control.fifo" ;
	else{
		if( !strcmp(fptr,"maserati0") ){
			FIFO_CONTROL = "/var/run/maserati0_control.fifo" ;
		}	
		else if( !strcmp(fptr,"maserati1") ){
			FIFO_CONTROL = "/var/run/maserati1_control.fifo" ;
		}
		else{ 
			FIFO_CONTROL = "/var/run/maserati_control.fifo" ;
		}
	}
	if( !FIFO_CONTROL )
		rcm_dp( "FIFO_CONTROL=%s\n" , FIFO_CONTROL );


}



void rcm_set_linphone_conctrol_fifo(void){

	FIFO_CONTROL="/var/run/solar_control.fifo";
}


void rcm_sim_stopcheck(){
	return ;
}

#ifdef VOIP_FLASH_ARCH	//Alex, original architecture
void rcm_voip_flash_update(){
	printf("voip_flash_server_update \n");
	voip_flash_server_update();
}
#endif	//VOIP_FLASH_ARCH

int rcm_set_linphone_pidfile(void){

	int pid_fd;
	FILE *fp;
	char line[20];
	pid_t pid;
		
	pidfile = "/var/run/solar.pid";

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
		return 0; //fail
	pidfile_write_release(pid_fd);

	return 1; //success
}

int rcm_set_sim_pidfile(){
	
	int pid_fd;
	FILE *fp;
	char line[20];
	pid_t pid;
	
	const char* iptr = getenv("RCM_NAME");
    //const char *seq_file=getenv("EVENT_SCRIPT");

	
	if(iptr==NULL)
		pidfile = "/var/run/maserati.pid" ;
	else{
		if( !strcmp(iptr,"maserati0") ){
			pidfile = "/var/run/maserati0.pid" ;
		}	
		else if( !strcmp(iptr,"maserati1") ){
			pidfile = "/var/run/maserati1.pid" ;
		}
		else{ 
			pidfile = "/var/run/maserati.pid" ;
		}
	}
	if( !pidfile )
		rcm_dp( "pidfile=%s\n" , pidfile );


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

	rcm_sim_pid_proc();
#if 0
    assert(seq_file);
    script_parser_init();
    if (script_parser(seq_file, 1)!=SPE_OK) {
        fprintf(stderr,"program ends with parsing error(s)\n");
        return 1;
    }
	fprintf(stderr,"parsing finished and start simulation....\n");
    sim_init();
	
#endif		
}

void rcm_sim_pid_proc(){

	printf("%s(%d)\n",__FUNCTION__,__LINE__);
	
}

#if 0
void rcm_voip_dsp_booting(){
	int i;
	// TH: add for ethernet DSP booting
	if (RTK_VOIP_ETHERNET_DSP_CHECK(g_VoIP_Feature))
	{
		for (i=0; i < DSP_DEVICE_NUMBER(g_VoIP_Feature); i++)
		{
			printf("Wait DSP%d booting ......\n", i);
			fflush(stdout);//force above message to display before while loop
			while(!rcm_tapi_CheckDspAllSoftwareReady(i));
			printf("DSP phy_id%d All Software Ready!\n", i);
			if (i == 0)
				rcm_tapi_SetDspIdToDsp(15);  // force DSP to gen mido interrupt
			rcm_tapi_SetDspIdToDsp(i);
		}
	}
}
#endif

char * get_evt_name(VoipEventID evt)
{

	switch (evt)
	{
		case VEID_MASK_ID: 
			return("VEID_MASK_ID"); 
		case VEID_MASK_TYPE: 
			return("VEID_MASK_TYPE"); 
		case VEID_NONE: 
			return("VEID_NONE");
		case VEID_DTMF_WILDCARD: 
			return("VEID_DTMF_WILDCARD"); 
		case VEID_DTMF_DIGIT_MASK: 
			return("VEID_DTMF_DIGIT_MASK");
		case VEID_DTMF_0: 
			return("VEID_DTMF_0");
		case VEID_DTMF_1: 
			return("VEID_DTMF_1");
		case VEID_DTMF_2: 
			return("VEID_DTMF_2");
		case VEID_DTMF_3: 
			return("VEID_DTMF_3");
		case VEID_DTMF_4: 
			return("VEID_DTMF_4");
		case VEID_DTMF_5: 
			return("VEID_DTMF_5");
		case VEID_DTMF_6: 
			return("VEID_DTMF_6");
		case VEID_DTMF_7: 
			return("VEID_DTMF_7");
		case VEID_DTMF_8: 
			return("VEID_DTMF_8");
		case VEID_DTMF_9: 
			return("VEID_DTMF_9");
		case VEID_DTMF_STAR: 
			return("VEID_DTMF_STAR");
		case VEID_DTMF_SHARP: 
			return("VEID_DTMF_SHARP");
		case VEID_DTMF_A: 
			return("VEID_DTMF_A");
		case VEID_DTMF_B: 
			return("VEID_DTMF_B");
		case VEID_DTMF_C: 
			return("VEID_DTMF_C");
		case VEID_DTMF_D: 
			return("VEID_DTMF_D");
		case VEID_DTMF_ENERGY: 
			return("VEID_DTMF_ENERGY");
		case VEID_HOOK_PHONE_ON_HOOK: 
			return("VEID_HOOK_PHONE_ON_HOOK");
		case VEID_HOOK_PHONE_OFF_HOOK: 
			return("VEID_HOOK_PHONE_OFF_HOOK");
		case VEID_HOOK_PHONE_FLASH_HOOK: 
			return("VEID_HOOK_PHONE_FLASH_HOOK");
		case VEID_HOOK_PHONE_STILL_ON_HOOK: 
			return("VEID_HOOK_PHONE_STILL_ON_HOOK");
		case VEID_HOOK_PHONE_STILL_OFF_HOOK: 
			return("VEID_HOOK_PHONE_STILL_OFF_HOOK");
		case VEID_HOOK_FXO_ON_HOOK: 
			return("VEID_HOOK_FXO_ON_HOOK");
		case VEID_HOOK_FXO_OFF_HOOK: 
			return("VEID_HOOK_FXO_OFF_HOOK");
		case VEID_HOOK_FXO_FLASH_HOOK: 
			return("VEID_HOOK_FXO_FLASH_HOOK");
		case VEID_HOOK_FXO_STILL_ON_HOOK: 
			return("VEID_HOOK_FXO_STILL_ON_HOOK");
		case VEID_HOOK_FXO_STILL_OFF_HOOK: 
			return("VEID_HOOK_FXO_STILL_OFF_HOOK");
		case VEID_HOOK_FXO_RING_ON: 
			return("VEID_HOOK_FXO_RING_ON");
		case VEID_HOOK_FXO_RING_OFF: 
			return("VEID_HOOK_FXO_RING_OFF");
		case VEID_HOOK_FXO_BUSY_TONE: 
			return("VEID_HOOK_FXO_BUSY_TONE");
		case VEID_HOOK_FXO_CALLER_ID: 
			return("VEID_HOOK_FXO_CALLER_ID");
		case VEID_HOOK_FXO_RING_TONE_ON: 
			return("VEID_HOOK_FXO_RING_TONE_ON");
		case VEID_HOOK_FXO_RING_TONE_OFF: 
			return("VEID_HOOK_FXO_RING_TONE_OFF");
		case VEID_HOOK_FXO_POLARITY_REVERSAL: 
			return("VEID_HOOK_FXO_POLARITY_REVERSAL");
		case VEID_HOOK_FXO_CURRENT_DROP: 
			return("VEID_HOOK_FXO_CURRENT_DROP");
		case VEID_HOOK_FXO_DIS_TONE: 
			return("VEID_HOOK_FXO_DIS_TONE");
		case VEID_HOOK_OUTBAND_FLASH_EVENT: 
			return("VEID_HOOK_OUTBAND_FLASH_EVENT");
		case VEID_ENERGY: 
			return("VEID_ENERGY");
		case VEID_DECT_BUTTON_PAGE: 
			return("VEID_DECT_BUTTON_PAGE");
		case VEID_DECT_BUTTON_REGISTRATION_MODE: 
			return("VEID_DECT_BUTTON_REGISTRATION_MODE");
		case VEID_DECT_BUTTON_DEL_HS: 
			return("VEID_DECT_BUTTON_DEL_HS");
		case VEID_DECT_BUTTON_NOT_DEFINED: 
			return("VEID_DECT_BUTTON_NOT_DEFINED");
		case VEID_FAXMDM_AUDIOCODES_FAX: 
			return("VEID_FAXMDM_AUDIOCODES_FAX");
		case VEID_FAXMDM_AUDIOCODES_MODEM: 
			return("VEID_FAXMDM_AUDIOCODES_MODEM");
		case VEID_FAXMDM_LEC_AUTO_RESTORE: 
			return("VEID_FAXMDM_LEC_AUTO_RESTORE");
		case VEID_FAXMDM_LEC_AUTO_ON: 
			return("VEID_FAXMDM_LEC_AUTO_ON");
		case VEID_FAXMDM_LEC_AUTO_OFF: 
			return("VEID_FAXMDM_LEC_AUTO_OFF");
		case VEID_FAXMDM_FAX_CED: 
			return("VEID_FAXMDM_FAX_CED");
		case VEID_FAXMDM_FAX_DIS_TX: 
			return("VEID_FAXMDM_FAX_DIS_TX");
		case VEID_FAXMDM_FAX_DIS_RX: 
			return("VEID_FAXMDM_FAX_DIS_RX");
		case VEID_FAXMDM_FAX_DCN_TX: 
			return("VEID_FAXMDM_FAX_DCN_TX");
		case VEID_FAXMDM_FAX_DCN_RX: 
			return("VEID_FAXMDM_FAX_DCN_RX");
		case VEID_FAXMDM_MODEM_LOCAL: 
			return("VEID_FAXMDM_MODEM_LOCAL");
		case VEID_FAXMDM_MODEM_LOCAL_DELAY: 
			return("VEID_FAXMDM_MODEM_LOCAL_DELAY");
		case VEID_FAXMDM_MODEM_REMOTE: 
			return("VEID_FAXMDM_MODEM_REMOTE");
		case VEID_FAXMDM_ANSTONE_CNG_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_CNG_LOCAL");
		case VEID_FAXMDM_ANSTONE_ANS_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_ANS_LOCAL");
		case VEID_FAXMDM_ANSTONE_ANSAM_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_ANSAM_LOCAL");
		case VEID_FAXMDM_ANSTONE_ANSBAR_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_ANSBAR_LOCAL");
		case VEID_FAXMDM_ANSTONE_ANSAMBAR_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_ANSAMBAR_LOCAL");
		case VEID_FAXMDM_ANSTONE_BELLANS_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_BELLANS_LOCAL");
		case VEID_FAXMDM_ANSTONE_V22_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_V22_LOCAL");
		case VEID_FAXMDM_ANSTONE_V8BIS_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_V8BIS_LOCAL");
		case VEID_FAXMDM_V21FLAG_LOCAL: 
			return("VEID_FAXMDM_V21FLAG_LOCAL");
		case VEID_FAXMDM_HS_FAX_SEND_V21FLAG_LOCAL: 
			return("VEID_FAXMDM_HS_FAX_SEND_V21FLAG_LOCAL");
		case VEID_FAXMDM_HS_FAX_RECV_V21FLAG_LOCAL: 
			return("VEID_FAXMDM_HS_FAX_RECV_V21FLAG_LOCAL");
		case VEID_FAXMDM_ANSTONE_OFF_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_OFF_LOCAL");
		case VEID_FAXMDM_ANSTONE_V21C2_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_V21C2_LOCAL");
		case VEID_FAXMDM_ANSTONE_V21C1_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_V21C1_LOCAL");
		case VEID_FAXMDM_ANSTONE_V23_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_V23_LOCAL");
		case VEID_FAXMDM_ANSTONE_BELL202ANS_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_BELL202ANS_LOCAL");
		case VEID_FAXMDM_ANSTONE_BELL202CP_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_BELL202CP_LOCAL");
		case VEID_FAXMDM_ANSTONE_CNG_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_CNG_REMOTE");
		case VEID_FAXMDM_ANSTONE_ANS_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_ANS_REMOTE");
		case VEID_FAXMDM_ANSTONE_ANSAM_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_ANSAM_REMOTE");
		case VEID_FAXMDM_ANSTONE_ANSBAR_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_ANSBAR_REMOTE");
		case VEID_FAXMDM_ANSTONE_ANSAMBAR_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_ANSAMBAR_REMOTE");
		case VEID_FAXMDM_ANSTONE_BELLANS_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_BELLANS_REMOTE");
		case VEID_FAXMDM_ANSTONE_V22_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_V22_REMOTE");
		case VEID_FAXMDM_ANSTONE_V8BIS_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_V8BIS_REMOTE");
		case VEID_FAXMDM_V21FLAG_REMOTE: 
			return("VEID_FAXMDM_V21FLAG_REMOTE");
		case VEID_FAXMDM_HS_FAX_SEND_V21FLAG_REMOTE: 
			return("VEID_FAXMDM_HS_FAX_SEND_V21FLAG_REMOTE");
		case VEID_FAXMDM_HS_FAX_RECV_V21FLAG_REMOTE: 
			return("VEID_FAXMDM_HS_FAX_RECV_V21FLAG_REMOTE");
		case VEID_FAXMDM_ANSTONE_OFF_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_OFF_REMOTE");
		case VEID_FAXMDM_ANSTONE_V21C2_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_V21C2_REMOTE");
		case VEID_FAXMDM_ANSTONE_V21C1_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_V21C1_REMOTE");
		case VEID_FAXMDM_ANSTONE_V23_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_V23_REMOTE");
		case VEID_FAXMDM_ANSTONE_BELL202ANS_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_BELL202ANS_REMOTE");
		case VEID_FAXMDM_ANSTONE_BELL202CP_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_BELL202CP_REMOTE");
		case VEID_FAXMDM_FAST_FAXTONE_LOCAL: 
			return("VEID_FAXMDM_FAST_FAXTONE_LOCAL");
		case VEID_FAXMDM_FAST_MODEMTONE_LOCAL: 
			return("VEID_FAXMDM_FAST_MODEMTONE_LOCAL");
		case VEID_FAXMDM_FAST_FAXTONE_REMOTE: 
			return("VEID_FAXMDM_FAST_FAXTONE_REMOTE");
		case VEID_FAXMDM_FAST_MODEMTONE_REMOTE: 
			return("VEID_FAXMDM_FAST_MODEMTONE_REMOTE");
		case VEID_FAXMDM_V152_RTP_VBD: 
			return("VEID_FAXMDM_V152_RTP_VBD");
		case VEID_FAXMDM_V152_RTP_AUDIO: 
			return("VEID_FAXMDM_V152_RTP_AUDIO");
		case VEID_FAXMDM_V152_SIG_CED: 
			return("VEID_FAXMDM_V152_SIG_CED");
		case VEID_FAXMDM_V152_TDM_SIG_END: 
			return("VEID_FAXMDM_V152_TDM_SIG_END");
		case VEID_FAXMDM_V152_BI_SILENCE: 
			return("VEID_FAXMDM_V152_BI_SILENCE");
		case VEID_FAXMDM_V150_SIG_MODEM: 
			return("VEID_FAXMDM_V150_SIG_MODEM");
		case VEID_FAXMDM_SILENCE_TDM: 
			return("VEID_FAXMDM_SILENCE_TDM");
		case VEID_FAXMDM_SILENCE_IP: 
			return("VEID_FAXMDM_SILENCE_IP");
		case VEID_FAXMDM_SILENCE_TDM_IP: 
			return("VEID_FAXMDM_SILENCE_TDM_IP");
		case VEID_FAXMDM_T38_FAX_END:
			return("VEID_FAXMDM_T38_FAX_END");
		case VEID_RFC2833_RX_WILDCARD: 
			return("VEID_RFC2833_RX_WILDCARD");

		#if 0 // duplicate case value as VEID_DTMF_DIGIT_MASK
		case VEID_RFC2833_RX_MASK: 
			return("VEID_RFC2833_RX_MASK");
		#endif

		#if 0 // duplicate case value as VEID_RFC2833_RX_WILDCARD
		case VEID_RFC2833_RX_DTMF_0: 
			returreturn("VEID_RFC2833_RX_DTMF_0");
		#endif

		case VEID_RFC2833_RX_DTMF_1: 
			return("VEID_RFC2833_RX_DTMF_1");
		case VEID_RFC2833_RX_DTMF_2: 
			return("VEID_RFC2833_RX_DTMF_2");
		case VEID_RFC2833_RX_DTMF_3: 
			return("VEID_RFC2833_RX_DTMF_3");
		case VEID_RFC2833_RX_DTMF_4: 
			return("VEID_RFC2833_RX_DTMF_4");
		case VEID_RFC2833_RX_DTMF_5: 
			return("VEID_RFC2833_RX_DTMF_5");
		case VEID_RFC2833_RX_DTMF_6: 
			return("VEID_RFC2833_RX_DTMF_6");
		case VEID_RFC2833_RX_DTMF_7: 
			return("VEID_RFC2833_RX_DTMF_7");
		case VEID_RFC2833_RX_DTMF_8: 
			return("VEID_RFC2833_RX_DTMF_8");
		case VEID_RFC2833_RX_DTMF_9: 
			return("VEID_RFC2833_RX_DTMF_9");
		case VEID_RFC2833_RX_DTMF_STAR: 
			return("VEID_RFC2833_RX_DTMF_STAR");
		case VEID_RFC2833_RX_DTMF_POUND: 
			return("VEID_RFC2833_RX_DTMF_POUND");
		case VEID_RFC2833_RX_DTMF_A: 
			return("VEID_RFC2833_RX_DTMF_A");
		case VEID_RFC2833_RX_DTMF_B: 
			return("VEID_RFC2833_RX_DTMF_B");
		case VEID_RFC2833_RX_DTMF_C: 
			return("VEID_RFC2833_RX_DTMF_C");
		case VEID_RFC2833_RX_DTMF_D: 
			return("VEID_RFC2833_RX_DTMF_D");
		case VEID_RFC2833_RX_DTMF_FLASH: 
			return("VEID_RFC2833_RX_DTMF_FLASH");
		case VEID_RFC2833_RX_DATA_FAX_ANS: 
			return("VEID_RFC2833_RX_DATA_FAX_ANS");
		case VEID_RFC2833_RX_DATA_FAX_ANSBAR: 
			return("VEID_RFC2833_RX_DATA_FAX_ANSBAR");
		case VEID_RFC2833_RX_DATA_FAX_ANSAM: 
			return("VEID_RFC2833_RX_DATA_FAX_ANSAM");
		case VEID_RFC2833_RX_DATA_FAX_ANSAMBAR: 
			return("VEID_RFC2833_RX_DATA_FAX_ANSAMBAR");
		case VEID_RFC2833_RX_DATA_FAX_CNG: 
			return("VEID_RFC2833_RX_DATA_FAX_CNG");
		case VEID_RFC2833_RX_DATA_FAX_V21_CH1_B0: 
			return("VEID_RFC2833_RX_DATA_FAX_V21_CH1_B0");
		case VEID_RFC2833_RX_DATA_FAX_V21_CH1_B1: 
			return("VEID_RFC2833_RX_DATA_FAX_V21_CH1_B1");
		case VEID_RFC2833_RX_DATA_FAX_V21_CH2_B0: 
			return("VEID_RFC2833_RX_DATA_FAX_V21_CH2_B0");
		case VEID_RFC2833_RX_DATA_FAX_V21_CH2_B1: 
			return("VEID_RFC2833_RX_DATA_FAX_V21_CH2_B1");
		case VEID_RFC2833_RX_DATA_FAX_CRDI: 
			return("VEID_RFC2833_RX_DATA_FAX_CRDI");
		case VEID_RFC2833_RX_DATA_FAX_CRDR: 
			return("VEID_RFC2833_RX_DATA_FAX_CRDR");
		case VEID_RFC2833_RX_DATA_FAX_CRE: 
			return("VEID_RFC2833_RX_DATA_FAX_CRE");
		case VEID_RFC2833_RX_DATA_FAX_ESI: 
			return("VEID_RFC2833_RX_DATA_FAX_ESI");
		case VEID_RFC2833_RX_DATA_FAX_ESR: 
			return("VEID_RFC2833_RX_DATA_FAX_ESR");
		case VEID_RFC2833_RX_DATA_FAX_MRDI: 
			return("VEID_RFC2833_RX_DATA_FAX_MRDI");
		case VEID_RFC2833_RX_DATA_FAX_MRDR: 
			return("VEID_RFC2833_RX_DATA_FAX_MRDR");
		case VEID_RFC2833_RX_DATA_FAX_MRE: 
			return("VEID_RFC2833_RX_DATA_FAX_MRE");
		case VEID_RFC2833_RX_DATA_FAX_CT: 
			return("VEID_RFC2833_RX_DATA_FAX_CT");
		case VEID_DSP_DTMF_CLID_END: 
			return("VEID_DSP_DTMF_CLID_END");
		case VEID_DSP_FSK_CLID_END: 
			return("VEID_DSP_FSK_CLID_END");
		case VEID_DSP_FSK_CLID_TYPE2_NO_ACK: 
			return("VEID_DSP_FSK_CLID_TYPE2_NO_ACK");
		case VEID_DSP_LOCAL_TONE_END: 
			return("VEID_DSP_LOCAL_TONE_END");
		case VEID_DSP_REMOTE_TONE_END: 
			return("VEID_DSP_REMOTE_TONE_END");
		case VEID_DSP_LOCAL_TONE_USER_STOP: 
			return("VEID_DSP_LOCAL_TONE_USER_STOP");
		case VEID_DSP_REMOTE_TONE_USER_STOP: 
			return("VEID_DSP_REMOTE_TONE_USER_STOP");
		default:
			return("unknown");
	}

	return("unknown");

}
