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

#include "linphonecore.h"
#include "lpconfig.h"
#include "private.h"
//#include "../mediastreamer/mediastream.h"
#include "../oRTP/payloadtype.h"
#include "../exosip/eXosip.h"
#include "../exosip/eXosip2.h"
#include "sdphandler.h"
#include "misc.h"
#include "exevents.h"
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <netdb.h>  
#include <errno.h>
#include "enum.h"
#include "../config.h"
#include "voip_manager.h"
#include "uglib.h"
#include "stun_api.h"
#ifdef RTK_DNS
#include "dns_api.h"
#endif

#ifdef CONFIG_RTK_VOIP_DIALPLAN
#include "dialplanhelp.h"
#endif
#ifdef CONFIG_RTK_VOIP_IP_PHONE
#include "uihandler.h"
#endif

extern uint32 rtk_Get_SLIC_Reg_Val(uint32 chid, uint32 reg, uint8 *regdata);
extern char caller_id_mode[];
extern char call_waiting_cid[];


#define MAX_IF 20
const double bandwidths[5]=
{
	28800.0,	/*28k modem*/
	28800.0, /*56k modem*/
	64000.0, /*64k modem (numeris)*/
	128000.0,	/*adsl or cable*/
	10e6, /* ethernet */
};/* carefull: these are the upstream bandwidth !!!*/

/* relative path where is stored local ring*/
#define LOCAL_RING "rings/oldphone.wav"
/* same for remote ring (ringback)*/
#define REMOTE_RING_FR "ringback.wav"
#define REMOTE_RING_US "ringback.wav"

/*+++++ Added by Jack for SRTP 160108+++++*/
#ifdef CONFIG_RTK_VOIP_SRTP
char base64Tbl[64] = {
                          'A','B','C','D','E','F','G','H',
                          'I','J','K','L','M','N','O','P',
                          'Q','R','S','T','U','V','W','X',
                          'Y','Z','a','b','c','d','e','f',
                          'g','h','i','j','k','l','m','n',
                          'o','p','q','r','s','t','u','v',
                          'w','x','y','z','0','1','2','3',  
                          '4','5','6','7','8','9','+','/'
                         };
#endif /*CONFIG_RTK_VOIP_SRTP*/
/*-----end-----*/

sdp_handler_t linphone_sdphandler =
{
	linphone_accept_audio_offer ,   /*from remote sdp */
	linphone_set_audio_offer ,		/*to local sdp */
	linphone_read_audio_answer ,	/*from incoming answer  */
	/*++added by Jack Chan xx/xx/07for++*/
	linphone_read_t38_answer,			/*from incoming t38 answer*/
	/*--end--*/
} ;

static void  linphone_call_init_common(LinphoneCall *call, char *from, char *to){
	sdp_context_set_user_pointer(call->sdpctx,(void*)call);
	call->state=LCStateInit;
	call->start_time=time(NULL);
	call->log=linphone_call_log_new(call, from, to);
}

#ifdef SUPPORT_CODEC_DESCRIPTOR
static const codec_mine_desc_t codecMineDesc[_CODEC_MAX] = {
	{	"PCMA",
		rtpPayloadPCMA,
		_CODEC_G711A,
	},
	{	"PCMU",
		rtpPayloadPCMU,
		_CODEC_G711U,
	},
	{	"G723",
		rtpPayloadG723,
		_CODEC_G723,
	},
 	{	"G729",
 		rtpPayloadG729,
 		_CODEC_G729,
 	},
	{	"G726-16",
		rtpPayloadG726_16,
		_CODEC_G726_16,
	},
	{	"G726-24",
		rtpPayloadG726_24,
		_CODEC_G726_24,
	},
	{	"G726-32",
		rtpPayloadG726_32,
		_CODEC_G726_32,
	},
	{	"G726-40",
		rtpPayloadG726_40,
		_CODEC_G726_40,
	},
 	{	"GSM",
 		rtpPayloadGSM,
 		_CODEC_GSMFR,
 	},
};

const codec_mine_desc_t *GetCodecMineDesc( const char *pszMineType )
{
	unsigned int i;
	
	for( i = 0; i < sizeof(codecMineDesc)/sizeof(codecMineDesc[0]); i ++ )
		if( strcmp( codecMineDesc[ i ].pszMineType, pszMineType ) == 0 )
			return &codecMineDesc[ i ];
	
	return NULL;
}

#endif /* SUPPORT_CODEC_DESCRIPTOR */

LinphoneCall * linphone_call_new_outgoing( struct _LinphoneCore * lc , const osip_from_t * from , const osip_to_t * to )
{
	LinphoneCall * call = g_new0( LinphoneCall , 1 ) ;
	char * localip = NULL ;
	const char * nat_address = NULL ;
	char * fromstr = NULL ;
	char * tostr = NULL ;
	if( call == NULL )
	{
/*		g_warning( "outgoing memory is not enough[%d][%d]" , chid , ssid ) ;*/
		g_warning( "outgoing memory is not enough\n" ) ;
		return NULL ;
	}
	call->dir = LinphoneCallOutgoing ;
	call->cid = -1 ;
	call->did = -1 ;
	call->core = lc ;

	nat_address = linphone_core_get_nat_address_if_used( lc ) ;
	if( nat_address == NULL )
		eXosip_get_localip_for(to->url->host, &localip);
	else
		localip = g_strdup( nat_address ) ;

	call->sdpctx = sdp_handler_create_context( &linphone_sdphandler , localip , from->url->username ) ;
	osip_free( localip ) ;
	osip_from_to_str( from , &fromstr ) ;
	osip_to_to_str( to , &tostr ) ;
	linphone_call_init_common( call , fromstr , tostr ) ;
	return call ;
}

LinphoneCall * linphone_call_new_incoming(LinphoneCore *lc, guint32 ssid, const char *from,
	const char *to, int cid, int did)
{
	LinphoneCall * call = g_new0( LinphoneCall , 1 ) ;
	osip_from_t * me ;
#ifdef NEW_CONTACT_RULE
	char *username;
#endif

	if( call == NULL )
	{
		g_warning("incoming memory is not enough[%d][%d]\n", lc->chid, ssid);
		return NULL ;
	}

#ifdef NEW_CONTACT_RULE
    osip_from_init(&me);
	if (osip_from_parse(me, to) != 0)
		username = NULL;
	else
		username = me->url->username;
#else
#ifdef USE_CURRENT_CONTACT
    me= linphone_core_get_current_contact_parsed(lc, from);
#else
    me= linphone_core_get_primary_contact_parsed(lc);
#endif
#endif

	call->dir = LinphoneCallIncoming ;
	call->cid = cid ;
	call->did = did ;
	call->core = lc ;
#ifdef NEW_CONTACT_RULE
	call->sdpctx = sdp_handler_create_context(&linphone_sdphandler, NULL, username);
#else
    /* Mandy add for stun support */
	if (lc->net_conf.use_nat) {
		call->sdpctx = sdp_handler_create_context( &linphone_sdphandler ,
											lc->net_conf.nat_address , me->url->username ) ;
	}
	else
		call->sdpctx = sdp_handler_create_context( &linphone_sdphandler ,
							linphone_core_get_nat_address_if_used( lc ) , me->url->username ) ;
#endif

	if( call->sdpctx == NULL )
	{
#ifdef FIX_MEMORY_LEAK
		osip_from_free(me);
#endif
		return NULL ;
	}
	linphone_call_init_common( call , osip_strdup( from ) , osip_strdup( to ) ) ;
#ifdef FIX_MEMORY_LEAK
	osip_from_free(me);
#endif
	return call ;
}

void linphone_call_destroy( LinphoneCall * obj )
{
	if( obj == NULL )
		return ;

	if (obj->state != LCStateTerminated) // has logged
		linphone_call_log_completed( obj->log , obj ) ;

	if( obj->sdpctx != NULL )
		sdp_context_free( obj->sdpctx ) ;

	g_free( obj ) ;
}

LinphoneCallLog * linphone_call_log_new(LinphoneCall *call, gchar *from, gchar *to){
	LinphoneCallLog *cl=g_new0(LinphoneCallLog,1);
	cl->lc=call->core;
	cl->dir=call->dir;
	cl->start_date=g_strdup(ctime(&call->start_time));
	cl->from=from;
	cl->to=to;
	return cl;
}
void linphone_call_log_completed(LinphoneCallLog *calllog, LinphoneCall *call){
	LinphoneCore *lc=call->core;
	calllog->duration=time(NULL)-call->start_time;
	switch(call->state){
		case LCStateInit:
			calllog->status=LinphoneCallAborted;
			break;
		case LCStateRinging:
			if (calllog->dir==LinphoneCallIncoming){
				gchar *info;
				calllog->status=LinphoneCallMissed;
				lc->missed_calls++;
				info=g_strdup_printf(_("You have missed %i call(s)."),lc->missed_calls);
				linphonec_display_something(lc,info);
				g_free(info);
			}
			else calllog->status=LinphoneCallAborted;
			break;
		case LCStateAVRunning:
			calllog->status=LinphoneCallSuccess;
			break;
		default:
			// not handled state
			break;
	}
	lc->call_logs=g_list_append(lc->call_logs,(gpointer)calllog);
	if (g_list_length(lc->call_logs)>lc->max_call_logs){
		GList *elem;
		elem=g_list_first(lc->call_logs);
		linphone_call_log_destroy((LinphoneCallLog*)elem->data);
#ifdef FIX_MEMORY_LEAK
		lc->call_logs=g_list_delete_link(lc->call_logs,elem);
#else
		lc->call_logs=g_list_remove_link(lc->call_logs,elem);
#endif
	}
	
	linphone_call_log_changed( calllog );
}

gchar * linphone_call_log_to_str(LinphoneCallLog *cl){
	gchar *status;
	switch(cl->status){
		case LinphoneCallAborted:
			status=_("aborted");
			break;
		case LinphoneCallSuccess:
			status=_("completed");
			break;
		case LinphoneCallMissed:
			status=_("missed");
			break;
		default:
			status="unknown";
	}
	return g_strdup_printf(_("%s at %sFrom: %s\nTo: %s\nStatus: %s\nDuration: %i mn %i sec\n"),
			(cl->dir==LinphoneCallIncoming) ? _("Incoming call") : _("Outgoing call"),
			cl->start_date,
			cl->from,
			cl->to,
			status,
			cl->duration/60,
			cl->duration%60);
}

void linphone_call_log_destroy(LinphoneCallLog *cl){
	if (cl->start_date!=NULL) g_free(cl->start_date);
	if (cl->from!=NULL) osip_free(cl->from);
	if (cl->to!=NULL) osip_free(cl->to);
	g_free(cl);
}

void linphone_call_log_changed(LinphoneCallLog *cl)
{
#ifdef CONFIG_APP_TR104
	solarSendChangedToCwmp();
#endif
}

#ifdef CONFIG_APP_TR104
void linphone_call_log_statistics(LinphoneCore *lc, voiceProfileLineStatus *VPLineStatus){
	int inCallsRecv=0; /* Totoal incoming calls receive */
	int inCallsAns=0; /* Totoal incoming calls answer */
	int inCallsConnect=0; /* 200 OK responses successfully */
	int inCallsFailed=0; /* e.g 4xx responses */
	int outCallsAttempt=0; /* totoal outing calls */
	int outCallsAnswer=0; /* responsed by the callee */
	int outCallsConnect=0; /* 200 OK received successfully */
	int outCallsFailed=0; /* e.g 4xx received */

	GList *callLogsList;
	for(callLogsList=lc->call_logs; callLogsList!=NULL; callLogsList=g_list_next(callLogsList)){
		LinphoneCallLog *callLogs;
		
		callLogs=(LinphoneCallLog*)callLogsList->data;
		if(LinphoneCallIncoming== callLogs->dir){
			inCallsRecv++;
			if(LinphoneCallSuccess == callLogs->status){
				inCallsConnect++;
			}else if(LinphoneCallAborted == callLogs->status || LinphoneCallMissed == callLogs->status){
				inCallsAns++;
			}
		}else if(LinphoneCallOutgoing ==callLogs->dir){
			outCallsAttempt++;
			if(LinphoneCallSuccess == callLogs->status){
				outCallsConnect++;
			}else if(LinphoneCallAborted == callLogs->status){
				outCallsAnswer++;
			}
		}
	}
	inCallsFailed = inCallsRecv - inCallsConnect;
	outCallsFailed = outCallsAttempt -outCallsConnect;
	VPLineStatus->incomingCallsReceived=inCallsRecv;
	VPLineStatus->incomingCallsAnswered=inCallsAns;
	VPLineStatus->incomingCallsConnected=inCallsConnect;
	VPLineStatus->incomingCallsFailed=inCallsFailed;
	VPLineStatus->outgoingCallsAttempted=outCallsAttempt;
	VPLineStatus->outgoingCallsAnswered=outCallsAnswer;
	VPLineStatus->outgoingCallsConnected=outCallsConnect;
	VPLineStatus->outgoingCallsFailed=outCallsFailed;
}
#endif

static FILE * lc_logfile=NULL;

void
__null_log_handler (const gchar * log_domain, GLogLevelFlags log_level,
		   const gchar * message, gpointer user_data)
{
	return;
}

void
__file_log_handler (const gchar * log_domain, GLogLevelFlags log_level,
		       const gchar * message, gpointer user_data)
{
	fprintf (lc_logfile, "%s:(GLogLevel=%i)** %s\n", log_domain, log_level,
		 message);
}

void linphone_core_enable_logs(FILE *file){
	if (file==NULL) file=stdout;
	if (file!=stdout){
		lc_logfile=file;
		g_log_set_handler ("LinphoneCore", G_LOG_LEVEL_MASK,
				   __file_log_handler, NULL);
		g_log_set_handler ("MediaStreamer", G_LOG_LEVEL_MASK,
				   __file_log_handler, NULL);
		g_log_set_handler ("oRTP", G_LOG_LEVEL_MASK,
				   __file_log_handler, NULL);
		g_log_set_handler ("oRTP-stats", G_LOG_LEVEL_MASK,
				   __file_log_handler, NULL);
	}
	osip_trace_initialize (OSIP_INFO4, file);
}
void linphone_core_disable_logs(){
	int tl;
	for (tl=0;tl<=OSIP_INFO4;tl++) osip_trace_disable_level(tl);
	g_log_set_handler ("LinphoneCore",
				   G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_WARNING,
				   __null_log_handler, NULL);
	g_log_set_handler ("MediaStreamer",
			   G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_WARNING,
			   __null_log_handler, NULL);
	g_log_set_handler ("oRTP",
			   G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_WARNING,
			   __null_log_handler, NULL);
	g_log_set_handler ("oRTP-stats", G_LOG_LEVEL_MESSAGE,
			   __null_log_handler, NULL);
}

void net_config_read(LinphoneCore *lc)
{
	linphone_core_set_connection_type(lc, CONNECTION_TYPE_ETHERNET);
}

int give_dtmf_type( int chid )
{
	voipCfgPortParam_t *voip_ptr = &g_pVoIPCfg->ports[chid];

	return voip_ptr->dtmf_mode ;
}

void sip_config_read(LinphoneCore *lc)
{
	voipCfgPortParam_t *voip_ptr = &g_pVoIPCfg->ports[lc->chid];
	gchar contact[LINE_MAX_LEN * 2], hostname[LINE_MAX_LEN];

	contact[0] = hostname[0] = 0;

	linphone_core_set_use_info_for_dtmf(lc, voip_ptr->dtmf_mode);
	lc->sip_conf.rfc2833_payload_type = voip_ptr->payload_type;
	
#ifdef SUPPORT_V152_VBD
	if( voip_ptr ->useV152 ) {
		lc->sip_conf.v152_payload_type = voip_ptr ->v152_payload_type;
		lc->sip_conf.v152_codec_type = voip_ptr ->v152_codec_type;
	} else {
		lc->sip_conf.v152_payload_type = 0;
		lc->sip_conf.v152_codec_type = rtpPayloadUndefined;
	}
#endif

#ifdef SUPPORT_RTP_REDUNDANT
	if( voip_ptr ->rtp_redundant_codec == -1 || 
		voip_ptr ->rtp_redundant_payload_type < 96 ||
		voip_ptr ->rtp_redundant_payload_type > 127 )
	{
		lc ->sip_conf.rtp_redundant_codec_type = -1;
		lc ->sip_conf.rtp_redundant_payload_type = 0;
	} else {
		lc ->sip_conf.rtp_redundant_codec_type = voip_ptr ->rtp_redundant_codec;
		lc ->sip_conf.rtp_redundant_payload_type = voip_ptr ->rtp_redundant_payload_type;	
	}
#endif

#ifdef DYNAMIC_PAYLOAD
	dynamic_pt_set(rtpPayloadDTMF_RFC2833, lc->sip_conf.rfc2833_payload_type);
#ifdef SUPPORT_V152_VBD
	dynamic_pt_set(rtpPayloadV152_Virtual, lc->sip_conf.v152_payload_type);
#endif
#ifdef SUPPORT_G7111
	dynamic_pt_set(rtpPayloadPCMU_WB, g_SystemParam.g711u_wb_pt);
	dynamic_pt_set(rtpPayloadPCMA_WB, g_SystemParam.g711a_wb_pt);
#endif
	dynamic_pt_set(rtpPayloadG726_16, g_SystemParam.g726_16_pt);
	dynamic_pt_set(rtpPayloadG726_24, g_SystemParam.g726_24_pt);
	dynamic_pt_set(rtpPayloadG726_32, g_SystemParam.g726_32_pt);
	dynamic_pt_set(rtpPayloadG726_40, g_SystemParam.g726_40_pt);
	dynamic_pt_set(rtpPayloadRtpRedundant, lc->sip_conf.rtp_redundant_payload_type);

	if (dynamic_pt_check() != 0)
	{
		g_error("%s: dynamic_pt_check failed\n", __FUNCTION__);
	}
#endif

//	linphone_core_enable_ipv6( lc , FALSE ) ;

//	linphone_core_set_sip_port( lc , voip_ptr->sip_port ) ;

	lc->sip_conf.sip_port = voip_ptr->sip_port;

#if 0
	if (voip_ptr->display_name[0])
		strcpy(lc->sip_conf.display_name, voip_ptr->display_name);

	if (voip_ptr->number[0])
		strcpy(lc->sip_conf.username, voip_ptr->number);

	if (voip_ptr->login_id[0])
		strcpy(lc->sip_conf.login_id, voip_ptr->login_id);

	if (voip_ptr->password[0])
		strcpy(lc->sip_conf.password, voip_ptr->password);

	strcpy( hostname , "localhost.localdomain" ) ;

	if (lc->sip_conf.username[0])
		sprintf(contact, "sip:%s@%s", lc->sip_conf.username, hostname);
	else
		sprintf(contact, "sip:%s", hostname);

	linphone_core_set_primary_contact( lc , ( gchar * )contact ) ;

	linphone_core_set_guess_hostname( lc , TRUE ) ;
#endif

    /* Mandy add for stun support */
	linphone_core_set_nat_address( lc , NULL, 0, FALSE ) ;	
    /* End of Mandy add for stun support */
	linphone_core_set_default_proxy(lc, NULL);

#ifndef CONFIG_RTK_VOIP_DIALPLAN
	// set uncondition forward
	lc->sip_conf.uc_forward_enable = voip_ptr->uc_forward_enable;
	if (lc->sip_conf.uc_forward_enable == 2)/* Uncondition PSTN Forward */
	{
		sprintf(lc->sip_conf.uc_forward, "%s", voip_ptr->uc_forward);
	}
	else if (lc->sip_conf.uc_forward_enable)
	{
		if (strstr(voip_ptr->uc_forward, "."))
		{
			sprintf(lc->sip_conf.uc_forward, "sip:%s", voip_ptr->uc_forward);
		}
		else if (voip_ptr->proxy_enable & PROXY_ENABLED)
		{
			if (voip_ptr->proxy_port)
				sprintf(lc->sip_conf.uc_forward, "sip:%s@%s", voip_ptr->uc_forward, 
					voip_ptr->proxy);
			else
				sprintf(lc->sip_conf.uc_forward, "sip:%s@%s:%d", voip_ptr->uc_forward, 
					voip_ptr->proxy, voip_ptr->proxy_port);
		}
	}

	// set busy forward
	lc->sip_conf.busy_forward_enable = voip_ptr->busy_forward_enable;
	if (lc->sip_conf.busy_forward_enable)
	{
		if (strstr(voip_ptr->busy_forward, "."))
		{
			sprintf(lc->sip_conf.busy_forward, "sip:%s", voip_ptr->busy_forward);
		}
		else if (voip_ptr->proxy_enable & PROXY_ENABLED)
		{
			if (voip_ptr->proxy_port)
				sprintf(lc->sip_conf.busy_forward, "sip:%s@%s", voip_ptr->busy_forward, 
					voip_ptr->proxy);
			else
				sprintf(lc->sip_conf.busy_forward, "sip:%s@%s:%d", voip_ptr->busy_forward, 
					voip_ptr->proxy, voip_ptr->proxy_port);
		}
	}

	// set no answer forward
	lc->sip_conf.na_forward_enable = voip_ptr->na_forward_enable;
	lc->sip_conf.na_forward_time = voip_ptr->na_forward_time;
	
	if (lc->sip_conf.na_forward_enable == 2)/* No Answer PSTN Forward */
	{
		sprintf(lc->sip_conf.na_forward, "%s", voip_ptr->na_forward);
	}
	else if (lc->sip_conf.na_forward_enable)
	{
		if (strstr(voip_ptr->na_forward, "."))
		{
			sprintf(lc->sip_conf.na_forward, "sip:%s", voip_ptr->na_forward);
		}
		else if (voip_ptr->proxy_enable & PROXY_ENABLED)
		{
			if (voip_ptr->proxy_port)
				sprintf(lc->sip_conf.na_forward, "sip:%s@%s", voip_ptr->na_forward, 
					voip_ptr->proxy);
			else
				sprintf(lc->sip_conf.na_forward, "sip:%s@%s:%d", voip_ptr->na_forward, 
					voip_ptr->proxy, voip_ptr->proxy_port);
		}
	}
#endif /* !CONFIG_RTK_VOIP_DIALPLAN */

	lc->sip_conf.call_waiting_enable = voip_ptr->call_waiting_enable;
	/*++T.38 add by Jack Chan for Voip++*/
	if ( voip_ptr->useT38 )
		lc->sip_conf.T38_enable = TRUE;
	if( 0 != voip_ptr->T38_port )
		lc->sip_conf.T38_prot = voip_ptr->T38_port;
	
	//T.38 parameters 
	lc->sip_conf.T38ParamEnable	= voip_ptr->T38ParamEnable;
	lc->sip_conf.T38MaxBuffer	= voip_ptr->T38MaxBuffer;
	lc->sip_conf.T38RateMgt		= voip_ptr->T38RateMgt;
	lc->sip_conf.T38MaxRate		= voip_ptr->T38MaxRate;
	lc->sip_conf.T38EnableECM	= voip_ptr->T38EnableECM;
	lc->sip_conf.T38ECCSignal	= voip_ptr->T38ECCSignal;
	lc->sip_conf.T38ECCData		= voip_ptr->T38ECCData;
	
	/*--end--*/
/* +++++Add by Jack for VoIP security 240108+++++ */
#ifdef CONFIG_RTK_VOIP_SRTP
	lc->sip_conf.security_enable = voip_ptr->security_enable;
	lc->sip_conf.key_exchange_mode = voip_ptr->key_exchange_mode;
#endif /*CONFIG_RTK_VOIP_SRTP*/
/*-----end-----*/
}

#ifdef SUPPORT_DSCP
// convert index of Web UI to DSCP
int qos2dscp(unsigned char index)
{
	switch (index) 
	{
		case 0: return 0x0;
		case 1: return (0x08);
		case 2: return (0x10);
		case 3: return (0x18);
		case 4: return (0x20);
		case 5: return (0x28);
		case 6: return (0x30);
		case 7: return (0x38);
		case 8: return (0x2e);
		default:
			return 0;
	}
}

int qos2tos(unsigned char index)
{
	return (qos2dscp(index) << 2);
}

#endif
void rtp_config_read(LinphoneCore *lc)
{
	voipCfgPortParam_t *voip_ptr = &g_pVoIPCfg->ports[lc->chid];

	lc->rtp_conf.audio_rtp_port = voip_ptr->media_port;
	lc->rtp_conf.audio_jitt_comp = voip_ptr->maxDelay;
	lc->rtp_conf.g7231_rate = voip_ptr->g7231_rate;
	lc->rtp_conf.iLBC_mode = voip_ptr->iLBC_mode;
	lc->rtp_conf.speex_nb_rate = voip_ptr->speex_nb_rate;
	lc->rtp_conf.bVAD = voip_ptr->vad;
	lc->rtp_conf.nVAD_thr = voip_ptr->vad_thr;
	lc->rtp_conf.bCNG = voip_ptr->cng;
	lc->rtp_conf.nCNG_thr = voip_ptr->cng_thr;
	lc->rtp_conf.nCNG_mode = voip_ptr->cng_gainmode;
	lc->rtp_conf.nCNG_level = voip_ptr->cng_noiselevel;
	lc->rtp_conf.nCNG_gain = voip_ptr->cng_noisegain;
	lc->rtp_conf.bPLC = voip_ptr->PLC;
	lc->rtp_conf.nRtcpInterval = voip_ptr->RTCP_Interval;
	lc->rtp_conf.bRtcpXR = voip_ptr->RTCP_XR;
	lc->rtp_conf.nJitterDelay = ( guint32 )voip_ptr->jitter_delay;
	lc->rtp_conf.nMaxDelay = ( guint32 )voip_ptr->maxDelay;
	lc->rtp_conf.nJitterFactor = ( guint32 )voip_ptr->jitter_factor;	
	lc->rtp_conf.nG726Packing = ( guint32 )voip_ptr->g726_packing;
#ifdef SUPPORT_VOICE_QOS
	lc->rtp_conf.tos = qos2tos(voip_ptr->voice_qos);
#endif	
	lc->rtp_conf.pFrameSize = voip_ptr ->frame_size;
}

void codecs_config_read(LinphoneCore *lc)
{
	GList * audio_codecs = NULL ;

	linphone_core_set_audio_codecs( lc , audio_codecs ) ;
	linphone_core_setup_local_rtp_profile( lc ) ;
}

void linphone_core_init(TAPP *pApp, LinphoneCore *lc, int chid)
{
	int cad_idx;
	int mid;
#ifdef CONFIG_RTK_VOIP_DIALPLAN
	dpInitParam_t dpInitParam;
#endif

	lc->parent = pApp;
	lc->chid = chid;
	lc->callingstate = 0;
	lc->dial_code[0] = 0;
	lc->digit_index = 0;
#ifdef CONFIG_RTK_VOIP_IVR
	lc->dial_initial_hash = 0;
#endif
#ifdef CONFIG_RTK_VOIP_DIALPLAN
	lc->dial_data_index = 0;
	lc->dial_data[0] = 0;
#endif

//	ortp_init();
//	ms_init();

#ifdef CONFIG_RTK_VOIP_DIALPLAN
	dpInitParam.replace_rule_option		= g_pVoIPCfg->ports[chid].replace_rule_option;
	dpInitParam.p_replace_rule_source	= g_pVoIPCfg->ports[chid].replace_rule_source;
	dpInitParam.p_replace_rule_target	= g_pVoIPCfg->ports[chid].replace_rule_target;
	dpInitParam.p_dialplan				= g_pVoIPCfg->ports[chid].dialplan;
	dpInitParam.p_auto_prefix			= g_pVoIPCfg->ports[chid].auto_prefix;
	dpInitParam.p_prefix_unset_plan		= g_pVoIPCfg->ports[chid].prefix_unset_plan;
	
	Init_DialPlan( chid, &dpInitParam );
#endif


#if 0
	lc->config = lp_config_new( config_path ) ;
#endif

	net_config_read(lc);
	rtp_config_read(lc);
	codecs_config_read(lc);
	sip_config_read(lc); /* this will start eXosip */

	// T38 config
	if( TRUE == lc->sip_conf.T38_enable )
	{
		char port[9];

		snprintf(port, 9, "%i", lc->sip_conf.T38_prot);
		eXosip_set_faxport(lc->chid, port);
		eXosip_set_firewall_faxport(lc->chid, 0); 
	}

#if 0
	lc->presence_mode = LINPHONE_STATUS_ONLINE ;
#endif

	lc->max_call_logs = 15;
	linphonec_proxy_add(lc);

	/* Mandy: Keep NAT entry alive */
	//linphonec_proxy_list(lc);
	lc->net_conf.udp_keepAlive_frequency = 120;
	lc->net_conf.natEntry_build_time = time(NULL);

	/* Set CNG on/off, VAD/CNG threshold */
	for (mid = 0; mid < 2; mid++)
	{
		if (lc->rtp_conf.bCNG == 0) // CNG off
			lc->rtp_conf.nCNG_thr = 1;
		rtk_SetThresholdVadCng(chid, mid, lc->rtp_conf.nVAD_thr, lc->rtp_conf.nCNG_thr, lc->rtp_conf.nCNG_mode,
				       lc->rtp_conf.nCNG_level, lc->rtp_conf.nCNG_gain);

	}
	/* dtmf_mode should be configed from web page */ //thlin+
	//0:rfc2833  1: sip info  2: inband
	rtk_SetDTMFMODE(chid, lc->sip_conf.dtmf_type);
	
	rtk_Set_G168_LEC(chid, g_pVoIPCfg->ports[chid].lec);
	/* Get echoTail value from flash and set to L_w.*/ //thlin+
	//if (chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
	if( RTK_VOIP_IS_DAA_CH( chid, g_VoIP_Feature ) )
	{
		/* TH: for DAA channel, ehco tail length should larger 4ms to get better performance. */
		if (g_pVoIPCfg->ports[chid].echoTail < 4)
			g_pVoIPCfg->ports[chid].echoTail = 4;
	}
	rtk_Set_echoTail(chid, g_pVoIPCfg->ports[chid].echoTail, g_pVoIPCfg->ports[chid].nlp);
	
	/* Get Tx gain value from flash and set to SLIC.*/ //thlin+
	//rtk_Set_SLIC_Tx_Gain(chid, g_pVoIPCfg->ports[chid].slic_txVolumne);
	//using soft dsp gain
	/* Get Rx gain value from flash and set to SLIC.*/ //thlin+
	//rtk_Set_SLIC_Rx_Gain(chid, g_pVoIPCfg->ports[chid].slic_rxVolumne);
	//using soft dsp gain
	/* Get Speaker AGC flag from flash and set. */  // jwsyu+
	rtk_Set_SPK_AGC(chid, g_pVoIPCfg->ports[chid].speaker_agc);
	
	/* Get Speaker AGC require level from flash and set. */  // jwsyu+
	rtk_Set_SPK_AGC_LVL(chid, g_pVoIPCfg->ports[chid].spk_agc_lvl);

	/* Get Speaker AGC gain up from flash and set. */  // jwsyu+
	rtk_Set_SPK_AGC_GUP(chid, g_pVoIPCfg->ports[chid].spk_agc_gu);
	
	/* Get Speaker AGC gain down from flash and set. */  // jwsyu+
	rtk_Set_SPK_AGC_GDOWN(chid, g_pVoIPCfg->ports[chid].spk_agc_gd);
	
	/* Get MIC AGC flag from flash and set. */  // jwsyu+
	rtk_Set_MIC_AGC(chid, g_pVoIPCfg->ports[chid].mic_agc);

	/* Get MIC AGC require level from flash and set. */  // jwsyu+
	rtk_Set_MIC_AGC_LVL(chid, g_pVoIPCfg->ports[chid].mic_agc_lvl);
	
	/* Get MIC AGC gain up from flash and set. */  // jwsyu+
	rtk_Set_MIC_AGC_GUP(chid, g_pVoIPCfg->ports[chid].mic_agc_gu);
	
	/* Get MIC AGC gain down from flash and set. */  // jwsyu+
	rtk_Set_MIC_AGC_GDOWN(chid, g_pVoIPCfg->ports[chid].mic_agc_gd);

	/* Get FSK GEN MODE form flash and set. */	//jwsyu+20070205
	//rtk_Set_CID_FSK_GEN_MODE(chid, g_pVoIPCfg->ports[chid].cid_fsk_gen_mode);

	/* Get Fax Modem Det MODE form flash and set. */	//jwsyu+20080730
	rtk_SetFaxModemDet(chid, g_pVoIPCfg->ports[chid].fax_modem_det);

	/* Get Fax Modem Answer Tonde Det Config form flash and set. */	//jwsyu+20110830
	rtk_Set_AnswerTone_Det(chid, g_pVoIPCfg->ports[chid].anstone);
	
	/* Get Fax/Modem RFC2833 Config from flash and set */ //thlin+
	rtk_SetFaxModemRfc2833(chid, g_pVoIPCfg->ports[chid].faxmodem_rfc2833&0x1,
					(g_pVoIPCfg->ports[chid].faxmodem_rfc2833&0x2)>>1,
					(g_pVoIPCfg->ports[chid].faxmodem_rfc2833&0x4)>>2);

	/* Get Voice gain from flash and set. */	//jwsyu+20070315
	rtk_Set_Voice_Gain(chid, g_pVoIPCfg->ports[chid].spk_voice_gain, g_pVoIPCfg->ports[chid].mic_voice_gain);

	/* Get Caller ID mode from flash and set.*/ //thlin+
	lc->caller_id_mode = g_pVoIPCfg->ports[chid].caller_id_mode; /* 0:Bellcore 1:ETSI 2:BT 3:NTT (FSK:0-3) 4:DTMF*/
	lc->cid_dtmf_mode = g_pVoIPCfg->ports[chid].cid_dtmf_mode;
	if ((lc->caller_id_mode&7) == CID_DTMF)
	{
		g_message("Set Caller ID Mode to DTMF\n");
		rtk_Set_CID_DTMF_MODE(chid, lc->caller_id_mode, lc->cid_dtmf_mode);
	}
	else
		rtk_Set_FSK_Area(chid, lc->caller_id_mode);  /* area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */
	
	/* Get Call Waiting Caller ID falg from flash and set.*/ //thlin+
	call_waiting_cid[chid] = g_pVoIPCfg->ports[chid].call_waiting_cid;
	if (call_waiting_cid[chid])
		g_message("Enable CH%d Call Waiting Caller ID\n", chid);
	else
		g_message("Disable CH%d Call Waiting Caller ID\n", chid);
	
	/* Get Ring Cadence On/Off Time and set.*/
	//if (chid < RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
	if( RTK_VOIP_IS_SLIC_CH( chid, g_VoIP_Feature ) )
	{
		cad_idx = g_pVoIPCfg->ring_cad;
		if (cad_idx == 8) // default case
			rtk_Set_SLIC_Ring_Cadence(chid, 1000, 2000);
		else
			rtk_Set_SLIC_Ring_Cadence(chid, g_pVoIPCfg->ring_cadon[cad_idx], 
				g_pVoIPCfg->ring_cadoff[cad_idx]);/* (chid, cad_on_msec, cad_off_msec) */
	}

#ifdef NEW_STUN
	rtk_stun_init(chid);
#endif

#ifdef SIP_PING
{
	voipCfgPortParam_t *voip_ptr = &g_pVoIPCfg->ports[lc->chid];
	voipCfgProxy_t *proxy_cfg;

	// rock: sip ping use proxy0 always
	proxy_cfg = &voip_ptr->proxies[0];
	// start sip ping if nortel server
	if ((proxy_cfg->enable & PROXY_ENABLED) &&
		(proxy_cfg->enable & PROXY_NORTEL))
	{
		LinphoneProxyConfig *proxy;
		osip_list_t *routes = NULL;
		char *from, *to;

		linphone_core_get_default_proxy(lc, &proxy);
		if (proxy)
		{
			routes = linphone_proxy_config_get_routes(proxy);
			from = linphone_proxy_config_get_identity(proxy);
			to = linphone_proxy_config_get_addr(proxy);
			rtk_ping_init(chid, from, to, routes);
		}
		else
		{
			g_warning("rtk_ping_send: couldn't find proxy\n");
			rtk_ping_close(chid);
		}
	}
	else
	{
		rtk_ping_close(chid);
	}
}
#endif
}

GList *linphone_core_get_audio_codecs(LinphoneCore *lc)
{
	return g_list_copy(lc->codecs_conf.audio_codecs);
}

#ifdef NEW_CONTACT_RULE

#else // NEW_CONTACT_RULE

int linphone_core_set_primary_contact( LinphoneCore * lc , const gchar * contact )
{
	if( lc->sip_conf.contact != NULL )
		g_free( lc->sip_conf.contact ) ;
	lc->sip_conf.contact = g_strdup( contact ) ;
	if( lc->sip_conf.guessed_contact != NULL )
	{
		g_free( lc->sip_conf.guessed_contact ) ;
		lc->sip_conf.guessed_contact = NULL ;
	}
	return 0 ;
}

#ifdef USE_CURRENT_CONTACT

void linphone_core_get_current_contact(LinphoneCore *lc, const char *to, char **contact)
{
    osip_from_t *l_from;
    gchar *guessed_ip = NULL;

    // guessed ip from 'to'
    osip_from_init(&l_from);
    if (osip_from_parse(l_from, to) != 0)
        g_error("Could not parse 'to' address !\n");

    eXosip_get_localip_for(l_from->url->host, &guessed_ip);

    osip_from_free(l_from);

    // create 'from' from sip_conf.contact
    osip_from_init(&l_from);
    if (osip_from_parse(l_from, lc->sip_conf.contact) != 0)
        g_error("Could not parse identity contact !\n");

    if (strcmp(guessed_ip, "127.0.0.1") == 0 ||
        strcmp(guessed_ip, "::1") == 0 )
    {
        g_warning("Local loopback network only !\n");
        lc->sip_conf.loopback_only = TRUE;
    }
    else
    {
        lc->sip_conf.loopback_only = FALSE;
    }

    // create url host
    osip_free(l_from->url->host);
    l_from->url->host = guessed_ip;

    // create url port
    if (l_from->url->port != NULL)
    {
        osip_free(l_from->url->port);
        l_from->url->port = NULL;
    }

    if (lc->sip_conf.sip_port && lc->sip_conf.sip_port != 5060)
    {
        l_from->url->port = strdup_printf("%i", lc->sip_conf.sip_port);
    }

	/* Mandy add for stun support */
	if (lc->net_conf.use_nat == TRUE) {
		osip_free(guessed_ip);
		l_from->url->host = strdup_printf("%s", lc->net_conf.nat_address);
		l_from->url->port = strdup_printf("%i", lc->net_conf.nat_port);
	}
	
	// add displayname
	if (lc->sip_conf.display_name[0])
		l_from->displayname = osip_strdup(lc->sip_conf.display_name);

    // create contact string
    osip_from_to_str(l_from, contact);

    // free temporary 'from'
    //osip_free(guessed_ip);
    osip_from_free(l_from);
}

#endif

const gchar *linphone_core_get_primary_contact(LinphoneCore *lc)
{
	gchar *identity;
	if (lc->sip_conf.guess_hostname){
		if (lc->sip_conf.guessed_contact==NULL || lc->sip_conf.loopback_only){
			gchar *tmp=NULL,*guessed=NULL;
			osip_from_t *url;
			if (lc->sip_conf.guessed_contact!=NULL){
				g_free(lc->sip_conf.guessed_contact);
				lc->sip_conf.guessed_contact=NULL;
			}
			
			osip_from_init(&url);
			if (osip_from_parse(url,lc->sip_conf.contact)==0){
				
			}else g_error("Could not parse identity contact !\n");
			if (!lc->sip_conf.ipv6_enabled){
				eXosip_get_localip_for("15.128.128.93", &tmp);
			}else{
				eXosip_get_localip_for("3ffe:4015:bbbb:70d0:201:2ff:fe09:81b1", &tmp);
			}
			if (strcmp(tmp,"127.0.0.1")==0 || strcmp(tmp,"::1")==0 ){
				g_warning("Local loopback network only !\n");
				lc->sip_conf.loopback_only=TRUE;
			}else lc->sip_conf.loopback_only=FALSE;
			osip_free(url->url->host);
			url->url->host=tmp;
			if (url->url->port!=NULL){
				osip_free(url->url->port);
				url->url->port=NULL;
			}
			if (lc->sip_conf.sip_port && lc->sip_conf.sip_port!=5060){
				url->url->port=strdup_printf("%i",lc->sip_conf.sip_port);
			}

			if (lc->net_conf.use_nat == TRUE) {
				osip_free(tmp);
				url->url->host = strdup_printf("%s", lc->net_conf.nat_address);
				url->url->port = strdup_printf("%i", lc->net_conf.nat_port);
			}

			osip_from_to_str(url,&guessed);
			lc->sip_conf.guessed_contact=guessed;
			
			//osip_free(tmp);
			osip_from_free(url);
			
		}
		identity=lc->sip_conf.guessed_contact;
	}else{
		identity=lc->sip_conf.contact;
	}
	return identity;
}

void linphone_core_set_guess_hostname( LinphoneCore * lc , gboolean val )
{
	lc->sip_conf.guess_hostname = val ;
}

gboolean linphone_core_get_guess_hostname(LinphoneCore *lc){
	return lc->sip_conf.guess_hostname;
}

#ifdef USE_CURRENT_CONTACT

osip_from_t *linphone_core_get_current_contact_parsed(LinphoneCore *lc, const char *to)
{
    int err;
    osip_from_t *contact;
    char *contact_str;

    osip_from_init(&contact);
    linphone_core_get_current_contact(lc, to, &contact_str);
    err=osip_from_parse(contact, contact_str);
    g_free(contact_str);
    if (err<0) {
        osip_from_free(contact);
        return NULL;
    }
    return contact;
}

#endif

osip_from_t *linphone_core_get_primary_contact_parsed(LinphoneCore *lc)
{
	int err;
	osip_from_t *contact;
	osip_from_init(&contact);
	err=osip_from_parse(contact,linphone_core_get_primary_contact(lc));
	if (err<0) {
		osip_from_free(contact);
		return NULL;
	}
	return contact;
}

#endif // NEW_CONTACT_RULE

int linphone_core_set_audio_codecs( LinphoneCore * lc , GList * codecs )
{
	if( lc->codecs_conf.audio_codecs != NULL )
		g_list_free( lc->codecs_conf.audio_codecs ) ;
	lc->codecs_conf.audio_codecs = codecs ;
	return 0 ;
}

#if 0
GList * linphone_core_get_friend_list(LinphoneCore *lc)
{
	return lc->friends;
}
#endif

void linphone_core_set_connection_type( LinphoneCore * lc , int type )
{
	lc->net_conf.con_type = type ;
	lc->net_conf.bandwidth = bandwidths[type] ;
}

void linphone_core_set_use_info_for_dtmf( LinphoneCore * lc , gint dtmf_type )
{
	lc->sip_conf.dtmf_type = dtmf_type ;
}

extern gboolean exosip_running;

#ifdef FIX_RESPONSE_TIME

void linphone_core_cb_reinvite(void *user, int type, eXosip_event_t *ev)
{
	TAPP *pApp;
	int chid, ssid;

	pApp = (TAPP *) user;
	if (linphone_find_chid_ssid(pApp, ev->cid, &chid, &ssid) == 0)
	{
		LinphoneCore *lc;
		char *answer;
		sdp_context_t *ctx;
		int status;

		lc = &pApp->ports[chid];

		ctx = lc->call[ssid]->sdpctx;
		answer = sdp_context_get_answer(ctx, ev->sdp_body);
		status = sdp_context_get_status(ctx);
		if (answer == NULL)
		{
			g_warning("get sdp answer failed [%d] [%d] [%d]\n", 
				ev->type, ev->cid, ev->did);
			goto linphone_core_cb_reinvite_end;
		}

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
				g_message("EXOSIP_CALL_HOLD[%d][%d]\n", chid, ssid);
				osip_negotiation_sdp_message_put_on_hold(ctx->answer);
				lc->call[ssid]->state = LCStateBeHold;
				// be hold quickly
				rtk_Hold_Rtp(chid, ssid, TRUE);
				if ((GetSysState(chid) != SYS_STATE_IN3WAY) && (GetActiveSession(chid) == ssid))
					rtk_SetPlayTone(chid, ssid, DSPCODEC_TONE_HOLD, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
			}
			else // EXOSIP_CALL_OFFHOLD
			{
				g_message("EXOSIP_CALL_OFFHOLD[%d][%d]\n", chid, ssid);
				osip_negotiation_sdp_message_put_off_hold(ctx->answer);
				lc->call[ssid]->state = LCStateBeResume;
			#if 0				
				// be resume quickly
				rtk_Hold_Rtp(chid, ssid, FALSE);
				rtk_SetPlayTone(chid, ssid, DSPCODEC_TONE_HOLD, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
			#endif	
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
	else
	{
		g_warning("\ninvalid event (%d): cid = %d, did = %d\n", 
			ev->type, ev->cid, ev->did);
	}

linphone_core_cb_reinvite_end:
	eXosip_event_free(ev);
}

#endif

void linphone_core_set_sip_port(LinphoneCore *lc, int port)
{
//	int err = 0 ;
	if( port == lc->sip_conf.sip_port )
		return ;
	lc->sip_conf.sip_port = port ;

#if 0
	if (lc->chid != 0)
	{
		// init only if chid == 0
		return;
	}

	if( exosip_running )
		eXosip_quit() ;

#ifdef FIX_RESPONSE_TIME
	// prepare callback event for quick response
	memset(linphone_call_callbacks, 0, sizeof(linphone_call_callbacks));
	linphone_call_callbacks[EXOSIP_CALL_HOLD] = linphone_core_cb_reinvite;
	linphone_call_callbacks[EXOSIP_CALL_OFFHOLD] = linphone_core_cb_reinvite;
	linphone_call_callbacks[EXOSIP_CALL_UPDATE] = linphone_core_cb_reinvite;
	// install callback on eXosip_init
	eXosip_register_callback(linphone_call_callbacks);
	err = eXosip_init(lc->parent, NULL, stdout, port);
#else
	err = eXosip_init(NULL, stdout, port);
#endif
	if( err < 0 )
	{
		char * msg = g_strdup_printf( "UDP port %i seems already in use ! Cannot initialize." , port ) ;
		g_warning( msg ) ;
		linphonec_display_warning(lc, msg);
		g_free( msg ) ;
		return ;
	}
	eXosip_set_user_agent( "Realtek" ) ;
	exosip_running = TRUE ;
#endif	
}

gboolean linphone_core_ipv6_enabled(LinphoneCore *lc){
	return lc->sip_conf.ipv6_enabled;
}
void linphone_core_enable_ipv6( LinphoneCore * lc , gboolean val )
{
	if( lc->sip_conf.ipv6_enabled != val )
	{
		lc->sip_conf.ipv6_enabled = val ;
//		eXosip_enable_ipv6( val ) ;
//		if( exosip_running )	/* we need to restart eXosip */
//			linphone_core_set_sip_port( lc , lc->sip_conf.sip_port ) ;
	}
}

void linphone_core_iterate(LinphoneCore *lc)
{
	eXosip_event_t *ev;
	uint32 chid;
	
#ifdef MEASURE_TIME
	struct timeval start, end;
	int waste_time;
	clock_t c_start, c_end;

 	osip_gettimeofday (&start, NULL);
	c_start = clock();
#endif



	chid = lc->chid;
	if( exosip_running )
	{
#if defined(WATCH_DOG) && !defined(DISABLE_THREAD)
		static int first = 1;
		static struct timeval timeout, now;

		while(1)
		{
			ev = eXosip_event_wait( 0 , 0 );
			if (ev)
			{
				if (ev->type == EXOSIP_WATCHDOG)
				{
					fprintf(stdout, "[R]\n");
					eXosip_event_free( ev ) ;
				}
				else
					linphone_core_process_event(lc->parent, ev);

				gettimeofday(&timeout, NULL);
				timeout.tv_sec += 3; // 3 sec timeout
			}
			else
			{
				if (first)
				{
					g_warning("watchdog is start.\n");
					first = 0;
					gettimeofday(&timeout, NULL);
					timeout.tv_sec += 3; // 3 sec timeout
				}
				else
				{
					gettimeofday(&now, NULL);
					if (timercmp(&now, &timeout, >))
					{
						time_t t;
						time(&t);

						g_warning("eXosip thread is dead: %s.\n", ctime(&t));
						gettimeofday(&timeout, NULL);
						timeout.tv_sec += 3; // 3 sec timeout
					}
				}

				// no event in fifo
				break;
			}
		}
#else
		while( ( ev = eXosip_event_wait( 0 , 0 ) ) != NULL )
			linphone_core_process_event(lc->parent, ev);
#endif



		linphone_core_update_proxy_register( lc ) ;
//		linphone_core_refresh_subscribes( lc ) ;

#ifdef NEW_STUN
		// Keep NAT entry alive in linphone_core_update_proxy_register
#else
		/* Mandy: Keep NAT entry alive */
		linphone_core_refresh_NATentry( lc );
#endif
	}

#ifdef MEASURE_TIME
	osip_gettimeofday (&end, NULL);

	if (end.tv_usec >= start.tv_usec)
		waste_time = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
	else
		waste_time = (end.tv_sec - start.tv_sec - 1) * 1000 + (1000000 + end.tv_usec - start.tv_usec) / 1000;

	if (waste_time)
	{
		c_end = clock();
		fprintf(stderr, "\n *** linphone_core_iterate waste %d ms (%ld)*** \n", 
			waste_time, (c_end - c_start) / (CLOCKS_PER_SEC / 1000));
	}
#endif
}


gboolean linphone_core_is_in_main_thread(LinphoneCore *lc){
	return TRUE;
}

static osip_to_t *osip_to_create(const char *to){
	osip_to_t *ret;
	osip_to_init(&ret);
	if (osip_to_parse(ret,to)<0){
		osip_to_free(ret);
		return NULL;
	}
	return ret;
}
#ifdef ATTENDED_TRANSFER
/*This function extract "Replaces=21321321@192.168.1.1%3Bto-tag%3D999 " 
    and put in replace_str as "213113@192.168.1.1;to-tag=xxx;from-tag=ooo"
    This function also clear url string after ?
    example:
    url	"sip:2002@192.168.1.1?Replaces=21321321@192.168.1.1%3Bto-tag@3D999... " 
    --> url sip:2002@192.168.1.1	
    --> replace_str: "21321321@192.168.1.1;to-tag=999...
*/
gboolean parse_replace_from_refer_to_str(char *url, char **replace_str)
{
	char *url_buf, *ptr;

	*replace_str = NULL;
	ptr = strstr(url,"?Replaces=");
	if ( ptr == NULL)
		return FALSE;

	*ptr = 0;// terminate url
	if (strchr(url,'<')!=0)
	{
		strcat(url,">");
	}
	ptr += strlen("?Replaces=");
	url_buf = osip_malloc(strlen(ptr) +1);
	osip_strncpy(url_buf, ptr, strlen(ptr));
	__osip_uri_unescape(url_buf);
	//if ((url_buf[strlen(url_buf)]) == '>')
	//	url_buf[strlen(url_buf)] = 0;
	if (strstr(url_buf, ">"))  *strstr(url_buf, ">") = 0;
	*replace_str = url_buf;

	return TRUE;
}

#endif

static gboolean linphone_core_interpret_url(LinphoneCore *lc, const char *url, char **real_url, osip_to_t **real_parsed_url){
	enum_lookup_res_t *enumres=NULL;
	osip_to_t *parsed_url=NULL;
	char *enum_domain=NULL;
	LinphoneProxyConfig *proxy;
	char *tmpurl;
	
	if (real_url!=NULL) *real_url=NULL;
	if (real_parsed_url!=NULL) *real_parsed_url=NULL;
	
	if (is_enum(url,&enum_domain)){
		linphonec_display_something(lc,_("Looking for telephone number destination..."));
		if (enum_lookup(enum_domain,&enumres)<0){
			linphonec_display_something(lc,_("Could not resolve this number."));
			g_free(enum_domain);
			return FALSE;
		}
		g_free(enum_domain);
		tmpurl=enumres->sip_address[0];
		if (real_url!=NULL) *real_url=g_strdup(tmpurl);
		if (real_parsed_url!=NULL) *real_parsed_url=osip_to_create(tmpurl);
		enum_lookup_res_free(enumres);
		return TRUE;
	}
	/* check if we have a "sip:" */
	if (strstr(url,"sip:")==NULL){
		/* this doesn't look like a true sip uri */
		proxy=lc->default_proxy;
		if (proxy!=NULL){
			/* append the proxy suffix */
			osip_uri_t *proxy_uri;
			char *sipaddr;
			const char *proxy_addr=linphone_proxy_config_get_addr(proxy);
			osip_uri_init(&proxy_uri);
			if (osip_uri_parse(proxy_uri,proxy_addr)<0){
				osip_uri_free(proxy_uri);
				return FALSE;
			}
			if (proxy_uri->port!=NULL)
				sipaddr=g_strdup_printf("sip:%s@%s:%s",url,proxy_uri->host,proxy_uri->port);
			else
				sipaddr=g_strdup_printf("sip:%s@%s",url,proxy_uri->host);
			if (real_parsed_url!=NULL) *real_parsed_url=osip_to_create(sipaddr);
			if (real_url!=NULL) *real_url=sipaddr;
			else g_free(sipaddr);
#ifdef FIX_MEMORY_LEAK
			osip_uri_free(proxy_uri);
#endif
			return TRUE;
		}
	}
	parsed_url=osip_to_create(url);
	if (parsed_url!=NULL){
		if (real_url!=NULL) *real_url=g_strdup(url);
		if (real_parsed_url!=NULL) *real_parsed_url=parsed_url;
		else osip_to_free(parsed_url);
		return TRUE;
	}
	/* else we could not do anything with url given by user, so display an error */
	linphonec_display_warning(lc,_("Could not parse given sip address. A sip url usually looks like sip:user@domain\n"));
	return FALSE;
}

#if 0

const char * linphone_core_get_identity(LinphoneCore *lc){
	LinphoneProxyConfig *proxy=NULL;
	const char *from;
	linphone_core_get_default_proxy(lc,&proxy);
	if (proxy!=NULL) {
		from=linphone_proxy_config_get_identity(proxy);
	}else from=linphone_core_get_primary_contact(lc);
	return from;
}

const osip_list_t * linphone_core_get_routes(LinphoneCore *lc){
	LinphoneProxyConfig *proxy=NULL;
	const osip_list_t *routes=NULL;
	linphone_core_get_default_proxy(lc,&proxy);
	if (proxy!=NULL) {
		routes=linphone_proxy_config_get_routes(proxy);
	}
	return routes;
}

gboolean linphone_get_proxy_enable( LinphoneCore *lc )
{
	gboolean result = FALSE ;

	result = lc->default_proxy->reg_sendregister ;

	return result ; 
}

char *linphone_get_proxy_addr( LinphoneCore *lc )
{
	return lc->default_proxy->reg_proxy ;
}

#endif

/* Mandy add for stun support */
int linphone_core_set_use_info_for_stun(LinphoneCore * lc, int ssid, int flag)
{
voipCfgPortParam_t *voip_ptr = &g_pVoIPCfg->ports[lc->chid];
#ifdef NEW_STUN
#else
unsigned int addr;
unsigned short port;
struct in_addr external_addr;
int i;
#endif

	g_message("The STUN status is %d.....\n", voip_ptr->stun_enable);
	if (voip_ptr->stun_enable && voip_ptr->stun_addr[0])
	{
#ifdef NEW_STUN
		return rtk_stun_send(lc->chid, ssid, flag);
#else
		char *c_address = voip_ptr->stun_addr;
		struct addrinfo *addrinfo;
		struct __eXosip_sockaddr srv_addr;

		i = eXosip_get_addrinfo(&addrinfo, voip_ptr->stun_addr, voip_ptr->stun_port);
		if (i==0)
		{
			  memcpy (&srv_addr, addrinfo->ai_addr, addrinfo->ai_addrlen);
			  freeaddrinfo (addrinfo);
			  c_address = inet_ntoa (((struct sockaddr_in *)((void *)&srv_addr))->sin_addr);
			  OSIP_TRACE (osip_trace
				  (__FILE__, __LINE__, OSIP_INFO1, NULL,
				  "Here is the resolved STUN server ip address=%s.\n", c_address));

			stun_SetServer(inet_addr(c_address), voip_ptr->stun_port);
			g_message( "Connecting STUN server.....%s.\n" , c_address ) ;
		}
		else {
			OSIP_TRACE (osip_trace
				  (__FILE__, __LINE__, OSIP_INFO1, NULL,
				  "We can't resolve the STUN server ip address...%s\n", voip_ptr->stun_addr));
			return -1;
		}


		if (flag & STUN_UPDATE_SIP) {
			if (stun_GetPublicAddr(0, voip_ptr->sip_port, &addr, &port, 
				eXosip.j_sockets[lc->chid], voip_ptr->sip_port, voip_ptr->voice_qos) == 0)
			{
				external_addr.s_addr = addr;
				g_message("sip external: %s:%d\n", inet_ntoa(external_addr), port);
				linphone_core_set_nat_address(lc, inet_ntoa(external_addr), port, TRUE);
			}
		}

		if (flag & STUN_UPDATE_RTP) {
			if (stun_GetPublicAddr(0, voip_ptr->media_port, &addr, &port, 
				eXosip.j_sockets[lc->chid], voip_ptr->sip_port, voip_ptr->voice_qos) == 0)
			{
				g_message("rtp external: %d\n", port);
				lc->net_conf.nat_voice_port[0]= port;
				if(0 == lc->chid)
					eXosip_set_firewall_media_port(0, port);
				else if( 1 == lc->chid)
					eXosip_set_firewall_media_port(2, port);
			}

			if (stun_GetPublicAddr(0, voip_ptr->media_port + 2, &addr, &port, 
				eXosip.j_sockets[lc->chid], voip_ptr->sip_port, voip_ptr->voice_qos) == 0)
			{
				g_message("rtp external: %d\n", port);
				lc->net_conf.nat_voice_port[1]= port;
				if( 0 == lc->chid )
					eXosip_set_firewall_media_port(1, port);
				else if( 1 == lc->chid )
					eXosip_set_firewall_media_port(3, port);
			}

#if 0
			// TODO: RTCP support
			if (stun_GetPublicAddr(0, voip_ptr->media_port+1, &addr, &port, 
				eXosip.j_sockets[lc->chid], voip_ptr->sip_port, voip_ptr->voice_qos) == 0)
			{
				g_message("rtcp external: %s:%d\n", inet_ntoa(addr), _port);
			}
#endif
		}

		if (flag & STUN_UPDATE_T38) {
			if (stun_GetPublicAddr(0, voip_ptr->T38_port, &addr, &port, 
				eXosip.j_sockets[lc->chid], voip_ptr->sip_port, voip_ptr->voice_qos) == 0)
			{
				g_message("t38 external: %d\n", port);
				lc->net_conf.nat_t38_port = port;
				eXosip_set_firewall_faxport(lc->chid, port);
			}
		}
#endif
	}
	else {
		linphone_core_set_nat_address( lc , NULL, 0, FALSE ) ;		
	}

	return 0;
}
/* End of Mandy add for stun support */

int linphone_core_invite(LinphoneCore *lc, uint32 ssid, const char *url)
{
	gchar * barmsg ;
	gint err = 0 ;
	gchar * sdpmesg = NULL ;
	osip_list_t * routes = NULL ;
	gchar * from = NULL ;
	osip_message_t * invite = NULL ;
	sdp_context_t * ctx = NULL ;
	LinphoneProxyConfig * proxy = NULL ;
	osip_from_t * parsed_url2 = NULL ;
	osip_to_t * real_parsed_url = NULL ;
	char * real_url = NULL ;
#ifdef ATTENDED_TRANSFER
	char	*replaces_str = NULL;
	char* url_dup;
#endif
//fprintf(stderr, "linphone_core_invite................\n");
    /* Mandy add for stun support */
	linphone_core_set_use_info_for_stun(lc, ssid, STUN_UPDATE_SIP | STUN_UPDATE_RTP);
    /* End of Mandy add for stun support */

#ifdef ATTENDED_TRANSFER
	url_dup = osip_strdup(url);
	parse_replace_from_refer_to_str(url_dup, &replaces_str);
#endif
	linphone_core_get_default_proxy( lc , &proxy ) ;
#ifdef ATTENDED_TRANSFER
	if( !linphone_core_interpret_url( lc , url_dup , &real_url , &real_parsed_url ) )
#else
	if( !linphone_core_interpret_url( lc , url , &real_url , &real_parsed_url ) )
#endif
	{
		g_warning( "bad url\n" ) ;
		err = -1;
		goto end;
	}

	barmsg = g_strdup_printf("Contacting [%d][%d] %s\n", lc->chid, ssid, real_url);
	linphonec_display_something(lc, barmsg);
	g_free(barmsg);

	if( proxy != NULL )
	{
		routes = linphone_proxy_config_get_routes( proxy ) ;
		from = osip_strdup(linphone_proxy_config_get_identity(proxy));
	}

#ifdef NEW_CONTACT_RULE
	//if (lc->chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature) &&
	if( RTK_VOIP_IS_DAA_CH( lc->chid, g_VoIP_Feature ) &&
		rtk_check_fxo_call_fxs(real_parsed_url) >= 0)
	{
		char szbuf[LINE_MAX_LEN];

		if (lc->caller_id[0])
		{
			if (lc->caller_id_name[0])
				sprintf(szbuf, "\"%s\" <sip:%s@127.0.0.1>", lc->caller_id_name, lc->caller_id);
			else
				sprintf(szbuf, "sip:%s@127.0.0.1", lc->caller_id);

			from = osip_strdup(szbuf);
		}
		else
		{
			from = osip_strdup("sip:127.0.0.1");
		}
	}
	else if (from == NULL)
	{
		voipCfgProxy_t *proxy_cfg = &g_pVoIPCfg->ports[lc->chid].proxies[lc->default_proxy_index];
		char szbuf[LINE_MAX_LEN];

		if (proxy_cfg->number[0])
		{
			if (proxy_cfg->display_name[0])
				sprintf(szbuf, "\"%s\" <sip:%s@127.0.0.1>", proxy_cfg->display_name, proxy_cfg->number);
			else
				sprintf(szbuf, "sip:%s@127.0.0.1", proxy_cfg->number);

			from = osip_strdup(szbuf);
		}
		else
		{
			from = osip_strdup("sip:127.0.0.1");
		}
	}
#else // NEW_CONTACT_RULE
	/* if no proxy or no identity defined for this proxy, default to primary contact */
#ifdef USE_CURRENT_CONTACT
	if (from==NULL) linphone_core_get_current_contact(lc, url, &from);
#else
    if (from==NULL) from=osip_strdup(linphone_core_get_primary_contact(lc));
#endif
#endif // NEW_CONTACT_RULE

	err = eXosip_build_initial_invite(lc->chid, &invite, (gchar *) real_url, (gchar *) from, routes, "Phone call");
	if( err < 0 )
	{
		g_warning( "Could not build initial invite\n" ) ;
		goto end ;
	}
#ifdef ATTENDED_TRANSFER
	if (replaces_str)
	{
		osip_message_set_header(invite,(const char *)"Replaces", replaces_str);
		osip_free(replaces_str);
	}
#endif
	
	/* make sdp message */
	osip_from_init( &parsed_url2 ) ;
	osip_from_parse( parsed_url2 , from ) ;

	lc->call[ssid] = linphone_call_new_outgoing(lc, parsed_url2, real_parsed_url);
	ctx = lc->call[ssid]->sdpctx;
	sdpmesg = sdp_context_get_offer( ctx ) ;

#ifdef MULTI_PROXY
	// record active proxy in call
	lc->call[ssid]->proxy = proxy;
#endif

	eXosip_lock() ;
	err = eXosip_initiate_call_with_body(lc->chid, invite, "application/sdp", sdpmesg, 
		(void *) lc->call[ssid], lc->caller_id);

#ifdef FIX_MUTEX
#else
	eXosip_unlock() ;
#endif
/* eric for test change sequence if() movoe up !! */
	if( err < 0 )
	{
		g_warning( "Could not initiate call.\n" ) ;
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		S2U_DisconnectInd( lc ->chid, ssid, 11 );
#endif
		linphone_call_destroy(lc->call[ssid]);
		lc->call[ssid] = NULL;
		eXosip_unlock() ;
		goto end ;
	}
	lc->call[ssid]->cid = err;
	eXosip_unlock() ;
	goto end ;

end :
	if (from != NULL)
		osip_free(from);
	if( real_url != NULL )
		g_free( real_url ) ;
	if( real_parsed_url != NULL )
		osip_to_free( real_parsed_url ) ;
	if( parsed_url2 != NULL )
		osip_from_free( parsed_url2 ) ;
#ifdef ATTENDED_TRANSFER
	if (url_dup)
		osip_free(url_dup);
#endif
	return err ;
}

#ifdef DYNAMIC_PAYLOAD
#else
int linphone_core_get_remote_rfc2833_pt(LinphoneCall *call)
{
	sdp_message_t *sdp;
	sdp_attribute_t *attr;
	int pos_media, pos;
	int pt, scanned;
	char *pt_value;

	if (call->sdpctx == NULL || call->sdpctx->remote == NULL)
	{
		g_warning("remote sdp is null!?\n");
		return 0;
	}

	sdp = call->sdpctx->remote;
	for (pos_media=-1; !sdp_message_endof_media(sdp, pos_media); pos_media++)
	{
		for (pos=0; (attr=sdp_message_attribute_get(sdp, pos_media, pos))!=NULL; pos++)
	    {
			if (strcmp(attr->a_att_field, "rtpmap") == 0)
			{
				sscanf(attr->a_att_value, "%d %n", &pt, &scanned);
				pt_value = attr->a_att_value + scanned;
				if (osip_strcasecmp(pt_value, "telephone-event/8000") == 0)
					return pt;
			}
		}
	}

	return 0;
}
#endif // DYNAMIC_PAYLOAD

#ifdef DYNAMIC_PAYLOAD
static gboolean audio_stream_start_with_dsp(LinphoneCore *lc, guint32 ssid, RtpProfile *profile, 
	StreamParams *audio_params, int jitt_comp)
#else
static gboolean audio_stream_start_with_dsp(LinphoneCore *lc, guint32 ssid, RtpProfile *profile, int locport, 
	char *remip, int remport, int payload, int jitt_comp)
#endif
{
#ifdef DYNAMIC_PAYLOAD
	int locport = audio_params->localport;
	char *remip = audio_params->remoteaddr;
	int remport = audio_params->remoteport;
#endif
#ifdef SUPPORT_CODEC_DESCRIPTOR
	PayloadType * pt ;
	unsigned int idxCodec = _CODEC_MAX;
	const codec_mine_desc_t *pCodecMineDesc;
#endif
	payloadtype_config_t codec_config ;
	rtp_config_t gdsprtp_config ;
	unsigned short src_port , dst_port ;
	guint32 chid = lc->chid;
	unsigned long src_ip, dst_ip;
	int rfc2833_local_pt, rfc2833_remote_pt;
	int offset;

	if (lc->call[ssid] == NULL || lc->call[ssid]->sdpctx == NULL)
	{
		g_warning("%s: call is null!?\n", __FUNCTION__);
		return FALSE;
	}

#ifdef DYNAMIC_PAYLOAD
	rfc2833_local_pt = audio_params->rfc2833_local_pt;
	rfc2833_remote_pt = audio_params->rfc2833_remote_pt;
#else
	rfc2833_local_pt = lc->sip_conf.rfc2833_payload_type;
	rfc2833_remote_pt = linphone_core_get_remote_rfc2833_pt(lc->call[ssid]);
#endif
	g_message("=>rfc2833 payload: local=%d, remote=%d\n", rfc2833_local_pt, rfc2833_remote_pt);

#ifdef CONFIG_RTK_VOIP_IP_PHONE
	if( lc ->call[ ssid ] )
		lc ->call[ ssid ] ->rfc2833_payload_type_remote = rfc2833_remote_pt;
#endif

	if (lc->call[ssid]->sdpctx->rtp_localip == NULL)
	{
		char *local_ip = NULL;
		g_warning("%s: rtp_localip is null!?\n", __FUNCTION__);
		eXosip_get_localip_for(remip, &local_ip);
		src_ip = inet_addr(local_ip);
		g_free(local_ip);
	}
	else
	{
		// get rtp local ip from sdp handshake
		src_ip = inet_addr(lc->call[ssid]->sdpctx->rtp_localip);
	}

	dst_ip = inet_addr( remip ) ;

	codec_config.chid = chid ;
	codec_config.sid = ssid ;
#ifdef DYNAMIC_PAYLOAD
	codec_config.local_pt = audio_params->local_pt;
	codec_config.remote_pt = audio_params->remote_pt;
	codec_config.uPktFormat = audio_params->static_pt;
#endif // DYNAMIC_PAYLOAD

#ifdef SUPPORT_V152_VBD
	if( audio_params->initialized_vbd ) {
		codec_config.local_pt_vbd = audio_params->local_pt_vbd;
		codec_config.remote_pt_vbd = audio_params->remote_pt_vbd;
		codec_config.uPktFormat_vbd = audio_params->static_pt_vbd;
	} else {
		codec_config.local_pt_vbd = rtpPayloadUndefined;
		codec_config.remote_pt_vbd = rtpPayloadUndefined;
		codec_config.uPktFormat_vbd = rtpPayloadUndefined;
	}
#endif

#ifdef SUPPORT_CODEC_DESCRIPTOR
	/* creates the couple of encoder/decoder */
	pt = rtp_profile_get_payload( profile , payload ) ;
	if( pt == NULL )
		return FALSE ;
	pCodecMineDesc = GetCodecMineDesc( pt->mime_type );
	
	if( pCodecMineDesc ) {
		codec_config.uPktFormat = pCodecMineDesc ->payloadType;
		idxCodec = pCodecMineDesc ->idxCodec;
	} else {
		printf( "ERROR: Unexpected MIME type.\n" );
		codec_config.uPktFormat = rtpPayloadPCMU;
		idxCodec = _CODEC_G711U;
	}
#endif /* SUPPORT_CODEC_DESCRIPTOR */

	codec_config.nG723Type = lc->rtp_conf.g7231_rate;
#ifndef SUPPORT_CUSTOMIZE_FRAME_SIZE
  #if 1
	// pkshih: give one to reduce packet loss impact 
	codec_config.nFramePerPacket = 1 ;	
  #else
	/* Frame per packet by hard coding */
	if (codec_config.uPktFormat == rtpPayloadG723||
		codec_config.uPktFormat == rtpPayloadGSM || 
		codec_config.uPktFormat == rtpPayload_iLBC || 
		codec_config.uPktFormat == rtpPayload_iLBC_20ms ||
		codec_config.uPktFormat == rtpPayload_SPEEX_NB_RATE8 ||
		codec_config.uPktFormat == rtpPayload_SPEEX_NB_RATE2P15 ||
		codec_config.uPktFormat == rtpPayload_SPEEX_NB_RATE5P95 ||
		codec_config.uPktFormat == rtpPayload_SPEEX_NB_RATE11 ||
		codec_config.uPktFormat == rtpPayload_SPEEX_NB_RATE15 ||
		codec_config.uPktFormat == rtpPayload_SPEEX_NB_RATE18P2 ||
		codec_config.uPktFormat == rtpPayload_SPEEX_NB_RATE24P6 ||
		codec_config.uPktFormat == rtpPayload_SPEEX_NB_RATE3P95 )
	{
		codec_config.nFramePerPacket = 1 ;
	} else
		codec_config.nFramePerPacket = 2 ;
  #endif
#else
	/* Frame per packet by flash */
	if( idxCodec >= 0 && idxCodec < _CODEC_MAX &&
		g_mapSupportedCodec[idxCodec] >= 0 && g_mapSupportedCodec[idxCodec] < _CODEC_MAX) 
	{
		codec_config.nFramePerPacket = 
							lc ->rtp_conf.pFrameSize[ g_mapSupportedCodec[idxCodec] ] + 1;
	} else {
		printf( "Unexpected codec in MINE-TYPE!\n" );
		codec_config.nFramePerPacket = 1;
	}
#endif
	codec_config.bVAD = lc->rtp_conf.bVAD;
	codec_config.bPLC = lc->rtp_conf.bPLC;
	codec_config.nJitterDelay = lc->rtp_conf.nJitterDelay;
	codec_config.nMaxDelay = lc->rtp_conf.nMaxDelay;
	codec_config.nJitterFactor = lc->rtp_conf.nJitterFactor;
	codec_config.nG726Packing = lc->rtp_conf.nG726Packing;
	codec_config.bT38ParamEnable = 0;
	codec_config.nT38MaxBuffer = 0;
	codec_config.nT38RateMgt = 0;
	codec_config.nT38MaxRate = 0;
	codec_config.bT38EnableECM = 0;
	codec_config.nT38ECCSignal = 0;
	codec_config.nT38ECCData = 0;

	gdsprtp_config.isTcp = FALSE ;
	memcpy( &(gdsprtp_config.remIp) , &dst_ip , 4 ) ;
	dst_port = ( unsigned short )remport ;
	memcpy( &(gdsprtp_config.remPort) , &dst_port , 2 ) ;
	memcpy( &(gdsprtp_config.extIp) , &src_ip , 4 ) ;
	src_port = ( unsigned short )locport ;
	memcpy( &(gdsprtp_config.extPort) , &src_port , 2 ) ;
	gdsprtp_config.chid = chid ;
	gdsprtp_config.sid = ssid ;
#ifdef SUPPORT_VOICE_QOS
	gdsprtp_config.tos = lc->rtp_conf.tos;
#endif
	gdsprtp_config.rfc2833_payload_type_local = (uint16) rfc2833_local_pt;
	gdsprtp_config.rfc2833_payload_type_remote = (uint16) rfc2833_remote_pt;
#ifdef CONFIG_RTK_VOIP_SRTP
	if(1 == lc->sip_conf.security_enable &&  -1 != audio_params->localCryptAlg){
		/* debug both remote sdp key and local sdp key to remote srtp key and local srtp key */
		base64Decode(audio_params->remoteSDPKey, gdsprtp_config.remoteSrtpKey);
		base64Decode(audio_params->localSDPKey, gdsprtp_config.localSrtpKey);
		gdsprtp_config.remoteCryptAlg = audio_params->remoteCryptAlg;
	}
#endif /* CONFIG_RTK_VOIP_SRTP */
#ifdef SUPPORT_RTP_REDUNDANT
	if( audio_params ->initialized_rtp_redundant ) {
		gdsprtp_config.rtp_redundant_payload_type_local = audio_params->local_pt_rtp_redundant;
		gdsprtp_config.rtp_redundant_payload_type_remote = audio_params->remote_pt_rtp_redundant;
	} else {
		gdsprtp_config.rtp_redundant_payload_type_local = 0;
		gdsprtp_config.rtp_redundant_payload_type_remote = 0;
	}
#endif
	gdsprtp_config.SID_payload_type_local = 0;
	gdsprtp_config.SID_payload_type_remote = 0;
	gdsprtp_config.rtcp_xr_enable = lc->rtp_conf.bRtcpXR;
	
	// This is demo purpose 
	offset = chid * 2 + ssid;
	
	gdsprtp_config.init_randomly = ( g_SystemParam.rtp_init ? 0 : 1 );
	gdsprtp_config.init_seqno = ( g_SystemParam.rtp_init ? g_SystemParam.rtp_init_seqno : 0 );
	gdsprtp_config.init_SSRC = ( g_SystemParam.rtp_init ? g_SystemParam.rtp_init_ssrc + offset : 0 );
	gdsprtp_config.init_timestamp = ( g_SystemParam.rtp_init ? g_SystemParam.rtp_init_timestamp : 0 );
	
#ifdef CHECK_RESOURCE
	if (VOIP_RESOURCE_UNAVAILABLE == rtk_VoIP_resource_check(chid, codec_config.uPktFormat))
	{
	#ifdef CONFIG_RTK_VOIP_IVR
		linphone_no_resource_handling(lc, ssid);
	#endif
		return FALSE;
	}
#endif
	// Set PCM mode according to codec
#if 1
	if (codec_config.uPktFormat == rtpPayloadG722)
		codec_config.nPcmMode = 3; // Wide-band Mode
	else
		codec_config.nPcmMode = 2; // Narrow-band Mode
#else
	codec_config.nPcmMode = 1; // Auto Mode
#endif

	rtk_SetRtpConfig( &gdsprtp_config ) ;
	if( lc->rtp_conf.nRtcpInterval )
		rtk_SetRtcpConfig( &gdsprtp_config, lc->rtp_conf.nRtcpInterval * 1000/*ms*/) ;//thlin+ for RTCP	
	rtk_SetRtpPayloadType( &codec_config ) ;
	rtk_SetRtpSessionState( chid , ssid , rtp_session_sendrecv ) ;
	
	return TRUE ;
}

/*++added by Jack Chan 02/02/07 for t38++*/
#ifdef DYNAMIC_PAYLOAD
static gboolean image_stream_start_with_dsp(LinphoneCore *lc, guint32 ssid, RtpProfile *profile, StreamParams *audio_params)
#else
static gboolean image_stream_start_with_dsp(LinphoneCore *lc, guint32 ssid, int locport, char *remip, int remport, int payload)
#endif // DYNAMIC_PAYLOAD
{
#ifdef DYNAMIC_PAYLOAD
	int locport = audio_params->localport;
	char *remip = audio_params->remoteaddr;
	int remport = audio_params->remoteport;
#endif
	payloadtype_config_t codec_config ;
	rtp_config_t gdsprtp_config ;
	unsigned short src_port , dst_port ;
	guint32 chid = lc->chid;
	unsigned long src_ip, dst_ip;

	{
		char *local_ip = NULL;

		eXosip_get_localip_for(remip, &local_ip);
		src_ip = inet_addr(local_ip);
		g_free(local_ip);
	}

	dst_ip = inet_addr( remip ) ;

	codec_config.chid = chid ;
	codec_config.sid = ssid ;

#ifdef DYNAMIC_PAYLOAD
	codec_config.local_pt = audio_params->local_pt;
	codec_config.remote_pt = audio_params->remote_pt;
	codec_config.uPktFormat = audio_params->static_pt;
#else
	codec_config.uPktFormat = payload;
#endif

#ifdef SUPPORT_V152_VBD
	codec_config.local_pt_vbd = rtpPayloadUndefined;
	codec_config.remote_pt_vbd = rtpPayloadUndefined;
	codec_config.uPktFormat_vbd = rtpPayloadUndefined;
#endif

#if 1
	// pkshih: give one to reduce packet loss impact 
	codec_config.nFramePerPacket = 1;	
#else
	codec_config.nFramePerPacket = 2;
#endif

	codec_config.bVAD = lc->rtp_conf.bVAD;
	codec_config.bPLC = lc->rtp_conf.bPLC;
	codec_config.nJitterDelay = lc->rtp_conf.nJitterDelay;
	codec_config.nMaxDelay = lc->rtp_conf.nMaxDelay;
	codec_config.nJitterFactor = lc->rtp_conf.nJitterFactor;
	codec_config.nG726Packing = lc->rtp_conf.nG726Packing;
	//T.38 parameters 
	codec_config.bT38ParamEnable = lc->sip_conf.T38ParamEnable;
	codec_config.nT38MaxBuffer = lc->sip_conf.T38MaxBuffer;
	codec_config.nT38RateMgt = lc->sip_conf.T38RateMgt;
	codec_config.nT38MaxRate = lc->sip_conf.T38MaxRate;
	codec_config.bT38EnableECM = lc->sip_conf.T38EnableECM;
	codec_config.nT38ECCSignal = lc->sip_conf.T38ECCSignal;
	codec_config.nT38ECCData = lc->sip_conf.T38ECCData;
		
	gdsprtp_config.isTcp = FALSE ;
	memcpy( &(gdsprtp_config.remIp) , &dst_ip , 4 ) ;
	dst_port = ( unsigned short )remport ;
	memcpy( &(gdsprtp_config.remPort) , &dst_port , 2 ) ;
	memcpy( &(gdsprtp_config.extIp) , &src_ip , 4 ) ;
	src_port = ( unsigned short )locport ;
	memcpy( &(gdsprtp_config.extPort) , &src_port , 2 ) ;
	gdsprtp_config.chid = chid ;
	gdsprtp_config.sid = ssid ;
#ifdef SUPPORT_RTP_REDUNDANT
	gdsprtp_config.rtp_redundant_payload_type_local = 0;
	gdsprtp_config.rtp_redundant_payload_type_remote = 0;
#endif
	gdsprtp_config.SID_payload_type_local = 0;
	gdsprtp_config.SID_payload_type_remote = 0;
	gdsprtp_config.rtcp_xr_enable = lc->rtp_conf.bRtcpXR;
	gdsprtp_config.init_randomly = 1;
	gdsprtp_config.init_seqno = 0;
	gdsprtp_config.init_SSRC = 0;
	gdsprtp_config.init_timestamp = 0;

#ifdef CHECK_RESOURCE
	if (VOIP_RESOURCE_UNAVAILABLE == rtk_VoIP_resource_check(chid, codec_config.uPktFormat))
	{
	#ifdef CONFIG_RTK_VOIP_IVR
		linphone_no_resource_handling(lc, ssid);
	#endif
		return FALSE;
	}
#endif
	// Set PCM mode according to codec
#if 1
	if (codec_config.uPktFormat == rtpPayloadG722)
		codec_config.nPcmMode = 3; // Wide-band Mode
	else
		codec_config.nPcmMode = 2; // Narrow-band Mode
#else
	codec_config.nPcmMode = 1; // Auto Mode
#endif

	rtk_SetRtpConfig( &gdsprtp_config ) ;
	//if( lc->rtp_conf.nRtcpInterval ) 
	//	rtk_SetRtcpConfig( &gdsprtp_config, lc->rtp_conf.nRtcpInterval/*ms*/ ) ;//thlin+ for RTCP	
	rtk_SetRtpPayloadType( &codec_config ) ;
	rtk_SetRtpSessionState( chid , ssid , rtp_session_sendrecv ) ;

	return TRUE;
}
/*--end--*/

#ifdef CHECK_RESOURCE
int linphone_core_start_media_streams(LinphoneCore *lc, guint32 ssid, LinphoneCall *call)
#else
void linphone_core_start_media_streams(LinphoneCore *lc, guint32 ssid, LinphoneCall *call)
#endif
{
	int jitt_comp ;
	StreamParams * audio_params = &call->audio_params ;

	if (lc->audiostream[ssid])
		linphone_core_stop_media_streams(lc, ssid);

	/*++added by Jack Chant 05/02/07 for T.38++*/
	if(1 == call->t38_params.initialized){
#ifdef DYNAMIC_PAYLOAD
		lc->audiostream[ssid] = image_stream_start_with_dsp(lc, ssid, lc->local_profile, &call->t38_params);
#else
		lc->audiostream[ssid] = image_stream_start_with_dsp(lc, ssid, call->t38_params.localport, call->t38_params.remoteaddr,
						call->t38_params.remoteport, call->t38_params.pt);
#endif // DYNAMIC_PAYLOAD
	}else if (1 == audio_params->initialized) {
	/*--end--*/
	jitt_comp = lc->rtp_conf.audio_jitt_comp ;
#ifdef DYNAMIC_PAYLOAD
	lc->audiostream[ssid] = audio_stream_start_with_dsp(lc, ssid, lc->local_profile, audio_params, jitt_comp);
#else
	lc->audiostream[ssid] = audio_stream_start_with_dsp(lc, ssid, lc->local_profile,
							audio_params->localport , audio_params->remoteaddr ,
							audio_params->remoteport , audio_params->pt , jitt_comp ) ;
#endif // DYNAMIC_PAYLOAD
	}

	if (lc->audiostream[ssid])
		lc->call[ssid]->state = LCStateAVRunning;

#ifdef CHECK_RESOURCE
	return lc->audiostream[ssid] ? 0 : -1;
#endif
}

void linphone_core_stop_media_streams(LinphoneCore *lc, guint32 ssid)
{
	if (lc->audiostream[ssid])
	{
		lc->audiostream[ssid] = FALSE ;
#ifndef __FEDORA_COMPLIER
		rtk_SetRtpSessionState(lc->chid, ssid, FALSE);
#endif
	}
}

int linphone_core_accept_dialog(LinphoneCore *lc, guint32 ssid, const char *url)
{
	char * sdpmesg ;

	if (lc->call[ssid] == NULL)
	{
		g_warning("Call null[%d][%d]\n", lc->chid, ssid);
		return -1;
	}

	/* stop ringing */
	if (lc->ringstream[ssid])
	{
		lc->ringstream[ssid] = FALSE;
#ifndef __FEDORA_COMPLIER
		rtk_SetRingFXS(lc->chid, FALSE);
		rtk_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_RINGING, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
#endif
	}

	/* sends a 200 OK */
    /* Mandy add for stun support */
	if (lc->net_conf.nat_address!=NULL && lc->net_conf.use_nat) {
#if 0
		linphone_core_set_use_info_for_stun(lc, 2);
		sdpmesg = sdp_context_set_answer(lc->call[ssid]->sdpctx , lc->call[ssid]->sdpctx->answerstr);
#else
		char *port;

		port = (char*)osip_malloc(10);
		if (lc->call[ssid]->t38_params.initialized)
		{
			linphone_core_set_use_info_for_stun(lc, ssid, STUN_UPDATE_T38);
			snprintf(port, 9, "%i", lc->net_conf.nat_t38_port);
			g_message("The nat_t38_port is %d.\n", lc->net_conf.nat_t38_port);
		}
		else
		{
			linphone_core_set_use_info_for_stun(lc, ssid, STUN_UPDATE_RTP);
			snprintf(port, 9, "%i", lc->net_conf.nat_voice_port[ssid]);
			g_message("The nat_voice_port is %d.\n", lc->net_conf.nat_voice_port[ssid]);
		}
		sdp_message_m_port_set(lc->call[ssid]->sdpctx->answer, 0, port);
		sdp_message_to_str(lc->call[ssid]->sdpctx->answer, &sdpmesg);
		osip_free(lc->call[ssid]->sdpctx->answerstr);
		lc->call[ssid]->sdpctx->answerstr = sdpmesg;
#endif
	}
	else {

	sdpmesg = lc->call[ssid]->sdpctx->answerstr ;	/* takes the sdp already computed */
	}

	eXosip_lock() ;
	eXosip_answer_call_with_body( lc->call[ssid]->did , 200 , "application/sdp" , sdpmesg ) ;
	eXosip_unlock() ;
	linphonec_display_something(lc, "Connected.\n");

	return 0 ;
}

void linphone_core_close_terminate_call(LinphoneCore *lc, uint32 ssid)
{
	LinphoneCall *call;
	eXosip_dialog_t *jd = NULL;
	eXosip_call_t *jc = NULL;

	call = lc->call_terminated[ssid];
	if (call == NULL)
		return;

	eXosip_call_dialog_find(call->did, &jc, &jd);
	if (jd)
	{
		osip_dialog_free(jd->d_dialog);
	 	jd->d_dialog = NULL;
	}

	linphone_call_destroy(call);
	lc->call_terminated[ssid] = NULL;
}

void linphone_core_update_terminate_call(LinphoneCore *lc, uint32 ssid)
{
	// rock: current call log done.
	linphone_call_log_completed(lc->call[ssid]->log, lc->call[ssid]);
	// rock: update terminated call
	linphone_core_close_terminate_call(lc, ssid);
	lc->call_terminated[ssid] = lc->call[ssid];
	lc->call_terminated[ssid]->state = LCStateTerminated;
}

int linphone_core_terminate_dialog(LinphoneCore *lc, uint32 ssid, const char *url)
{
		LinphoneCall *call;

		call = lc->call[ssid];
		if( call != NULL )
		{
			eXosip_lock() ;
			eXosip_terminate_call( call->cid , call->did ) ;
			eXosip_unlock() ;

			/* stop ringing */
			if( lc->ringstream[ssid] )
			{
				lc->ringstream[ssid] = FALSE ;
				rtk_SetRingFXS(lc->chid, FALSE);
//				rtk_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
			}

#ifdef CONFIG_RTK_VOIP_IP_PHONE
			S2U_DisconnectInd( lc ->chid, ssid, 3 );
#endif
			linphone_core_stop_media_streams(lc, ssid);
			linphonec_display_something(lc, "Communication ended.\n");
#if 0
			linphone_call_destroy(call);
#else
			// rock: save last terminated call
			linphone_core_update_terminate_call(lc, ssid);
#endif
			lc->call[ssid] = NULL;
		}
		
		SetSessionState(lc->chid, ssid, SS_STATE_IDLE);
		return 0;
}

#if 0

int linphone_core_send_publish(LinphoneCore *lc,
			       LinphoneOnlineStatus presence_mode)
{
	GList *elem;
	for (elem=linphone_core_get_proxy_config_list(lc);elem!=NULL;elem=g_list_next(elem)){
		LinphoneProxyConfig *cfg=(LinphoneProxyConfig*)elem->data;
		if (cfg->publish) linphone_proxy_config_send_publish(cfg,presence_mode);
	}
	return 0;
}

#endif

/* sound functions */
void linphone_core_send_dtmf(LinphoneCore *lc, uint32 ssid, gchar dtmf)	// hc$ DTMF mode process
{
	if (lc->sip_conf.dtmf_type==0)		// DTMF RFC2833 (out-of-band)
	{	
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		if( lc ->call[ ssid ] &&
			lc ->call[ ssid ] ->rfc2833_payload_type_remote == 0 )
		{
			/* Though we want to use 2833, remote doesn't suport it. */
			goto label_do_in_band_DTMF;
		}
#endif

		switch(dtmf)	// mapping to RFC2833 digit
		{	case 1 : dtmf = 1; 	break;	// 1 digit	
			case 2 : dtmf = 2;	break; 	
			case 3 : dtmf = 3;	break; 	
			case 4 : dtmf = 4;	break;
			case 5 : dtmf = 5;	break;
			case 6 : dtmf = 6;	break;
			case 7 : dtmf = 7;	break;
			case 8 : dtmf = 8;	break;
			case 9 : dtmf = 9;	break;	// 9 digit
			case 10 : dtmf = 0;	break;	// 0 digit
			case 11 : dtmf = 10;	break;	// * digit
			case 12 : dtmf = 11;	break;	// # digit
			default: 
				g_warning("invalid dtmf in RFC2833= %d\n", dtmf);
				return;
     	} 
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		if( GetSysState( lc ->chid ) == SYS_STATE_IN3WAY ) {
			/* 
			 * IP phone always two DTMF to two sessions, but kernel implement
			 * RFC2833 by session. 
			 * Thus, we ignore the DTMF of second session. 
			 */
			if( ssid == 1 ) {
				g_message( "Ignore DTMF(1) in 3way conf\n" );
				return;
			}
		}
#endif
		rtk_SetRTPRFC2833(lc->chid, ssid, dtmf, 100);	// keyin is digit not char
		return;
	}	


	if (lc->sip_conf.dtmf_type==2)		// DTMF In Band
	{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		DSPCODEC_TONE nTone;

label_do_in_band_DTMF:
		/* If FXO line, ignore inband tone */
		//if( lc ->chid >= RTK_VOIP_SLIC_NUM( g_VoIP_Feature ) )
		if( RTK_VOIP_IS_DAA_CH( lc ->chid, g_VoIP_Feature ) )
			goto label_ignore_dtmf_in_band_tone;
	
		switch( dtmf ) {
		case 10:
			nTone = DSPCODEC_TONE_0;
			break;
		case 11:
			nTone = DSPCODEC_TONE_STARSIGN;
			break;
		case 12:
			nTone = DSPCODEC_TONE_HASHSIGN;
			break;
		default:
			if( dtmf >= 1 && dtmf <= 9 )
				nTone = dtmf - 1 + DSPCODEC_TONE_1;
			else
				goto label_ignore_dtmf_in_band_tone;
			break;
		}

		rtk_SetPlayTone( lc ->chid, ssid, nTone, TRUE, DSPCODEC_TONEDIRECTION_REMOTE );

label_ignore_dtmf_in_band_tone:
#endif /* CONFIG_RTK_VOIP_IP_PHONE */
		return;
  	}

	if (lc->sip_conf.dtmf_type==1)		// DTMF SIP INFO (out-of-band)
  	{
		char dtmf_body[1000];
		char dtmf_char;

	    /* Out of Band DTMF (use INFO method) */
	    LinphoneCall *call=lc->call[ssid];
	    if (call==NULL)
	    {
			return;
		}
	
		dtmf_char = (dtmf == 10) ? '0' : 
					(dtmf == 11) ? '*' : 
					(dtmf == 12) ? '#' :
					(dtmf + '0');
		snprintf(dtmf_body, 999, "Signal=%c\r\nDuration=%d\r\n", dtmf_char, g_pVoIPCfg->ports[lc->chid].sip_info_duration);

		eXosip_lock();
		eXosip_info_call(call->did, "application/dtmf-relay", dtmf_body);
		eXosip_unlock();
	}
}

#if 0
void linphone_core_force_ip_address(LinphoneCore *lc, const gchar *ipaddr)
{
	linphone_core_set_nat_address( lc , ipaddr , TRUE ) ;
}
#endif


/* Mandy add for stun support */
void linphone_core_set_nat_address( LinphoneCore * lc , const gchar * addr , const gint port, gboolean use ) 
{
	char natport[9];

	if( lc->net_conf.nat_address != NULL )
		g_free( lc->net_conf.nat_address ) ;

	if (use && addr)
	{
		memset(natport, 0, sizeof(natport));
		lc->net_conf.use_nat = TRUE ;
		lc->net_conf.nat_address = osip_strdup(addr);
		lc->net_conf.nat_port = port;
		eXosip_set_firewallip( addr,lc->chid ) ;
		sprintf(natport, "%i", port);
		eXosip_set_firewallport(lc->chid, natport);
	}
	else
	{
		lc->net_conf.use_nat = FALSE ;
		lc->net_conf.nat_address = NULL ;
		lc->net_conf.nat_port = 0;
		eXosip_set_firewallip( "",lc->chid) ;		
		eXosip_set_firewallport(lc->chid, NULL);
	}
}

const gchar *linphone_core_get_nat_address(LinphoneCore *lc, gboolean *use)
{
	if (use!=NULL) *use=lc->net_conf.use_nat;
	if (lc->net_conf.nat_address!=NULL) return lc->net_conf.nat_address;
	else return NULL;
}

const gchar *linphone_core_get_nat_address_if_used(LinphoneCore *lc){
	if (lc->net_conf.use_nat && lc->net_conf.nat_address!=NULL) return lc->net_conf.nat_address;
	else return NULL;		
}

GList * linphone_core_get_call_logs(LinphoneCore *lc){
	lc->missed_calls=0;
	return lc->call_logs;
}

void net_config_uninit(LinphoneCore *lc)
{
#if 0
	net_config_t *config=&lc->net_conf;
	lp_config_set_int(lc->config,"net","con_type",config->con_type);
	lp_config_set_int(lc->config,"net","use_nat",config->use_nat);
	if (config->nat_address!=NULL)
		lp_config_set_string(lc->config,"net","nat_address",config->nat_address);
#endif
}


void sip_config_uninit(LinphoneCore *lc)
{
#if 0
	GList *elem;
	gint i;
	sip_config_t *config=&lc->sip_conf;

#if 0
	lp_config_set_int(lc->config,"sip","sip_port",config->sip_port);
	lp_config_set_int(lc->config,"sip","guess_hostname",config->guess_hostname);
	lp_config_set_string(lc->config,"sip","contact",config->contact);
	lp_config_set_int(lc->config,"sip","dtmf_type",config->dtmf_type);
	lp_config_set_int(lc->config,"sip","use_ipv6",config->ipv6_enabled);
#endif

	for(elem=config->proxies, i=0; elem!=NULL; elem=g_list_next(elem), i++){
		LinphoneProxyConfig *cfg=(LinphoneProxyConfig*)(elem->data);
#if 0
		linphone_proxy_config_write_to_config_file(lc->config,cfg,i);
#endif
		linphone_proxy_config_edit(cfg);	/* to unregister */
	}

#ifdef DISABLE_THREAD
	if (exosip_running && lc->chid == g_MaxVoIPPorts - 1)
	{
		int err;
		fd_set fdset;
		struct timeval tv;

		// send packets
		eXosip_execute();

		// wait 3 sec to clear state machine
	    for (i=0; i<30; i++) 
		{
			FD_ZERO(&fdset);
			FD_SET(eXosip.j_socket, &fdset);

			tv.tv_sec = 0;
			tv.tv_usec = 100000;
			err = select(eXosip.j_socket + 1, &fdset, NULL, NULL, &tv);

			if ((err == -1) && (errno == EINTR || errno == EAGAIN))
				continue;

			if (err == 0) // timeout
			{
				eXosip_event_t *ev;

				while((ev=eXosip_event_wait(0,0))!=NULL)
				{
		  			linphone_core_process_event(lc->parent, ev);
					eXosip_execute();
				}

				continue;
			}

			if (err > 0)
			{
				if (FD_ISSET(eXosip.j_socket, &fdset))
				{
					eXosip_read_message(lc);
					eXosip_execute();
				}
			}
			else
			{
				g_warning("Error in select(): %s\n", strerror(errno));
				break;
			}
		}
	}
#else
	if (exosip_running)
	  {
	    int i;
	    for (i=0;i<20;i++)
	      {
		eXosip_event_t *ev;
		while((ev=eXosip_event_wait(0,0))!=NULL){
		  linphone_core_process_event(lc,ev);
		}
		usleep(100000);
	      }
	  }
#endif                            	

#if 0
	linphone_proxy_config_write_to_config_file(lc->config,NULL,i);	/*mark the end */
	for(elem=lc->auth_info,i=0;elem!=NULL;elem=g_list_next(elem),i++){
		LinphoneAuthInfo *ai=(LinphoneAuthInfo*)(elem->data);
		linphone_auth_info_write_config(lc->config,ai,i);
	}
	linphone_auth_info_write_config(lc->config,NULL,i); /* mark the end */
#endif

#endif

#if 0
#ifdef FIX_MEMORY_LEAK
	linphone_core_clear_all_auth_info(lc);
#endif
#endif
}

void rtp_config_uninit(LinphoneCore *lc)
{
#if 0
	user_rtp_config_t *config=&lc->rtp_conf;
	lp_config_set_int(lc->config,"rtp","audio_rtp_port",config->audio_rtp_port);
	lp_config_set_int(lc->config,"rtp","audio_jitt_comp",config->audio_jitt_comp);
#endif
}

void codecs_config_uninit(LinphoneCore *lc)
{
#if 0
	PayloadType *pt;
	codecs_config_t *config=&lc->codecs_conf;
	GList *node;
	gchar key[50];
	gint index;
	index=0;
	for(node=config->audio_codecs;node!=NULL;node=g_list_next(node)){
		pt=(PayloadType*)(node->data);
		sprintf(key,"audio_codec_%i",index);
		lp_config_set_string(lc->config,key,"mime",pt->mime_type);
		lp_config_set_int(lc->config,key,"rate",pt->clock_rate);
		lp_config_set_int(lc->config,key,"enabled",payload_type_enabled(pt));
		index++;
	}
#endif
}

#if 0
void ui_config_uninit(LinphoneCore* lc)
{
	GList *elem;
	int i;
	for (elem=lc->friends,i=0; elem!=NULL; elem=g_list_next(elem),i++){
		linphone_friend_write_to_config_file(lc->config,(LinphoneFriend*)elem->data,i);
		linphone_friend_destroy(elem->data);
	}
	linphone_friend_write_to_config_file(lc->config,NULL,i);	/* set the end */
	g_list_free(lc->friends);
	lc->friends=NULL;
}
#endif

void linphone_core_uninit(LinphoneCore *lc)
{
	int i;

	/* save all config */
	net_config_uninit(lc);
	sip_config_uninit(lc);
#if 0
	lp_config_set_int(lc->config,"sip","default_proxy",linphone_core_get_default_proxy(lc,NULL));
#endif
	rtp_config_uninit(lc);
	codecs_config_uninit(lc);
#if 0
	ui_config_uninit(lc);
	lp_config_sync(lc->config);
	lp_config_destroy(lc->config);
#endif	

#if 0
	if (lc->chid == g_MaxVoIPPorts - 1) 
	{
		// uinit only if chid == MAX
		eXosip_quit();
		exosip_running = FALSE;
	}
#endif

#ifdef FIX_MEMORY_LEAK	
{
	GList *elem;

	for (elem=lc->sip_conf.proxies; elem!=NULL; elem=g_list_next(elem))
	{
		LinphoneProxyConfig *cfg = (LinphoneProxyConfig *) elem->data;
#if 0		
		linphone_proxy_config_edit(cfg);
#endif		
		linphone_proxy_config_destroy(cfg);
#if 0
		if (cfg->realm) g_free(cfg->realm);
#endif
		g_free(cfg);
	}
	g_list_free(lc->sip_conf.proxies);
	lc->sip_conf.proxies = NULL;
	lc->default_proxy = NULL;

#if 0
	if (lc->sip_conf.contact != NULL)
	{
		g_free( lc->sip_conf.contact ) ;
		lc->sip_conf.contact = NULL;
	}

	if( lc->sip_conf.guessed_contact != NULL )
	{
		g_free( lc->sip_conf.guessed_contact ) ;
		lc->sip_conf.guessed_contact = NULL ;
	}
#endif

	if (lc->codecs_conf.audio_codecs != NULL)
	{
		g_list_free( lc->codecs_conf.audio_codecs ) ;
		lc->codecs_conf.audio_codecs = NULL;
	}

	if( lc->net_conf.nat_address != NULL )
	{
		g_free( lc->net_conf.nat_address ) ;
		lc->net_conf.nat_address = NULL ;
	}
	
	for (i=0; i<MAX_SS; i++)
	{
		linphone_core_close_terminate_call(lc, i);
	}

    for(elem=lc->call_logs; elem!=NULL; elem=g_list_next(elem))
	{
		if (elem->data != NULL)
		{
			linphone_call_log_destroy((LinphoneCallLog*) elem->data);
			elem->data = NULL;
		}
    }
	g_list_free( lc->call_logs );
	lc->call_logs = NULL;

//	ms_destroy();
//	ortp_destroy() ;
}
#endif

#ifdef CONFIG_RTK_VOIP_DIALPLAN
	Uninit_DialPlan( lc ->chid );
#endif
}

int linphone_core_onhood_event(LinphoneCore *lc, guint32 ssid)
{
	LinphoneCall *call;
	uint32 chid;
	int err;

	chid = lc->chid;
	call = lc->call[ssid];
	if (call != NULL)
	{
		switch (call->state)
		{
		case LCStateResume:
			// waiting AVRunning
			return -1;
		case LCStateAVRunning:
//			SetActiveSession(chid, ssid == 0 ? 1 : 0, TRUE);
#ifdef NEW_STUN
#if 0
			// close media first for stun
			if (g_pVoIPCfg->ports[lc->chid].stun_enable)
				linphone_core_stop_media_streams(lc, ssid);
#endif
#endif
//			if (GetSessionState(chid, ssid == 0 ? 1 : 0) == SS_STATE_IDLE)
//				rtk_SetPlayTone(chid, ssid == 0 ? 1 : 0, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
			// send hold
			call->state = LCStateHold; // start hold
			eXosip_lock();
			call->sdpctx->version++;
			err = eXosip_on_hold_call(call->did, call->sdpctx->version);
			eXosip_unlock();
			if (err != 0) return -1;
			break;
		case LCStateHold: // not ACK, hold again
			eXosip_lock();
			call->sdpctx->version++;
			err = eXosip_on_hold_call(call->did, call->sdpctx->version);
			eXosip_unlock();
			if (err == 0)
			{
				// last hold timeout, try again
			}
			else if (err < 0)
			{
				// error or last hold not done
			}
			else if (err == 1)
			{
				// recover it if hold in early media
				call->state = LCStateAVRunning; // start hold
				if (GetSessionState(chid, ssid == 0 ? 1 : 0) == SS_STATE_IDLE)
					rtk_SetPlayTone(chid, ssid == 0 ? 1 : 0, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
				SetActiveSession(chid, ssid, TRUE);
			}
			return -1;
		case LCStateBeHold: // be holding
		case LCStateBeHoldACK: // have hold
			// do nothing
			break;
		case LCStateHoldACK: // have hold
			SetActiveSession(chid , ssid == 0 ? 1 : 0, TRUE);
//			if (lc->call[ssid == 0 ? 1 : 0] == NULL)
			if (GetSessionState(chid, ssid == 0 ? 1 : 0) == SS_STATE_IDLE)
				rtk_SetPlayTone(chid, ssid == 0 ? 1 : 0, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
			break;
		default:
			// do nothing...
			break;
		}
	}

	return 0;
}

int linphone_core_offhood_event(LinphoneCore *lc, guint32 ssid)
{
	LinphoneCall *call;
	uint32 chid;
	int err;

	chid = lc->chid;
	call = lc->call[ssid];
	if (call != NULL)
	{
		switch (call->state)
		{
		case LCStateHold:
			// waiting hold ack
			return -1;
		case LCStateHoldACK:
//			SetActiveSession(chid, ssid, TRUE);
//			rtk_SetPlayTone(chid, ssid, DSPCODEC_TONE_HOLD, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
			// send resume 
			call->state = LCStateResume;
			/* Mandy: Use new RTP port in INVITE message when we resume the call. */
			linphone_core_set_use_info_for_stun(lc, ssid, STUN_UPDATE_RTP);
			eXosip_lock();
			call->sdpctx->version++;
			/* Mandy: Use new RTP port in INVITE message when we resume the call. */
			if (lc->net_conf.nat_address!=NULL && lc->net_conf.use_nat) {
				err = eXosip_off_hold_call(call->did, 
									lc->net_conf.nat_address,
									lc->net_conf.nat_voice_port[ssid], 
									call->sdpctx->version);
			}
			else {
				err = eXosip_off_hold_call(call->did, NULL, 0, call->sdpctx->version);
			}
			eXosip_unlock();
			if (err != 0) return -1;
			break;
		case LCStateResume: // not ACK, send again
			g_message("......LCStateResume.........\n");
			/* Mandy: Use new RTP port in INVITE message when we resume the call. */
			linphone_core_set_use_info_for_stun(lc, ssid, STUN_UPDATE_RTP);
			eXosip_lock();
			call->sdpctx->version++;
			/* Mandy: Use new RTP port in INVITE message when we resume the call. */
			if (lc->net_conf.nat_address!=NULL && lc->net_conf.use_nat) {
				eXosip_off_hold_call(call->did, 
									lc->net_conf.nat_address,
									lc->net_conf.nat_voice_port[ssid], 
									call->sdpctx->version);
			}
			else {
				eXosip_off_hold_call(call->did, NULL, 0, call->sdpctx->version);
			}
			eXosip_unlock();
			return -1; 	// still wait ACK
		case LCStateBeHold:
		case LCStateBeHoldACK:
			SetActiveSession(chid , ssid, TRUE);
			rtk_SetPlayTone(chid , ssid, DSPCODEC_TONE_HOLD, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
			break;
		case LCStateAVRunning: // have running
			SetActiveSession(chid , ssid, TRUE);
			break;
		default:
			// do nothing...
			break;
		}
	}

	return 0;
}

void linphone_call_fax_reinvite(LinphoneCore *lc, guint32 ssid)
{
	LinphoneCall *call;

	call = lc->call[ssid];
	if( call != NULL && call->state == LCStateAVRunning)
	{
		/*++ T38 add by Jack Chan 24/01/07++ */
		int T38_port = 0;
		if( TRUE == lc->sip_conf.T38_enable && FAX_RUN == call->faxflag ){
			T38_port = lc->sip_conf.T38_prot;
			call->t38_params.initialized=1;
		}else{
			T38_port = 0;
			call->t38_params.initialized=0;
		}
		g_message("+++++DEBUG:PortNumber:%d+++++\n",T38_port);
		g_message("+++++DEBUG:Enable%d+++++\n",lc->sip_conf.T38_enable);
		/*--end--*/
		
#if 1
//		linphone_core_stop_media_streams(lc, ssid); 
#else
		sleep(1) ;
#endif
		call->state = LCStateFax ;
		eXosip_lock() ;
		call->sdpctx->version++;
		eXosip_on_fax_call(call->did, T38_port, call->sdpctx->version);
		eXosip_unlock() ;
	}
}

void linphone_call_off_fax_reinvite(LinphoneCore *lc, guint32 ssid)
{
	LinphoneCall *call;
	int err=0;
	
	call = lc->call[ssid];

	if(call!=NULL)
	{
		
			call->t38_params.initialized=0;
			
#if 1
//		linphone_core_stop_media_streams(lc, ssid); 
#else
		sleep(1) ;
#endif
		call->state = LCStateAVRunning ;

		printf("\r\n start send fax off invite \n");
		linphone_core_set_use_info_for_stun(lc, ssid, STUN_UPDATE_RTP);
		eXosip_lock();
		call->sdpctx->version++;
		eXosip_on_fax_call(call->did, 0, call->sdpctx->version);
		eXosip_unlock();
	}
}


#ifdef ATTENDED_TRANSFER

/*  In this case, LinphoneCore should have 2 calls. 1st is where refer-to to send, 2nd
  is the call to replace. 
	INPUT: lc  pointer to current linphone core.
	OUTPUT: refer_to: the string in 'Refer-To: 12321@1.1.1.1?Replaces=xxx...'
	return : 0 on success  */
int atdfr_construct_referto(LinphoneCore *lc, char *refer_to)
{
	eXosip_call_t	*jc;
	eXosip_dialog_t	*jd;
	char		contact_url[128];
	char		*ptr;

	if ((!lc->call[0]) || (!lc->call[1]))	return -1;

	// find the call/dialog
	eXosip_call_dialog_find(lc->call[1]->did, &jc, &jd);
	if (jd->d_dialog->remote_contact_uri)
	{
		char port[8];

		if (jd->d_dialog->remote_contact_uri->url &&
			jd->d_dialog->remote_contact_uri->url->host)
		{
			if (jd->d_dialog->remote_contact_uri->url->username)
				sprintf(contact_url, "sip:%s@%s",osip_uri_get_username(jd->d_dialog->remote_contact_uri->url), 
					osip_uri_get_host(jd->d_dialog->remote_contact_uri->url));
			else
				sprintf(contact_url, "sip:%s", osip_uri_get_host(jd->d_dialog->remote_contact_uri->url));
		}
		else
		{
			return -1;
		}

		if (osip_uri_get_port(jd->d_dialog->remote_contact_uri->url))
		{
			sprintf(port,":%s",osip_uri_get_port(jd->d_dialog->remote_contact_uri->url));
			strcat(contact_url, port);
		}
	}
	else if (lc->call[1]->log->to)
	{	
		sprintf(contact_url, "%s",lc->call[1]->log->to);
	}
	else 
		return -1;
	//encode
	sprintf(refer_to,"%s;to-tag=%s;from-tag=%s", jd->d_dialog->call_id, 
		jd->d_dialog->remote_tag, jd->d_dialog->local_tag );

	ptr = __osip_uri_escape_header_param(refer_to);
	sprintf(refer_to,"<%s?Replaces=%s>",contact_url, ptr);

	osip_free(ptr);
	return 0;
}

#endif

void linphone_core_transfer_call(LinphoneCore *lc, guint32 ssid, const char *refer)
{
	LinphoneCall *call;

	call = lc->call[ssid];
	if( call != NULL )
	{
		eXosip_lock() ;
		eXosip_transfer_call(call->did, refer);
		eXosip_unlock() ;
	}
}

void linphone_core_forward_dialog(LinphoneCore *lc, guint32 ssid, const char *forward)
{
	LinphoneCall *call;

	call = lc->call[ssid];
	if (call != NULL)
	{
		eXosip_lock();
		eXosip_set_redirection_address(call->did, forward);
		eXosip_answer_call(call->did, 302, NULL);
		eXosip_set_redirection_address(call->did, NULL);
		eXosip_unlock();
	}
}

/*+++++ Added by Jack for SRTP 160108+++++*/
#ifdef CONFIG_RTK_VOIP_SRTP
void base64Encode(unsigned char *s, char *result)
{
	int len, i;
	char *res;
	unsigned char *p;
       
	res = (char *)malloc(40 * sizeof(char) + 1);
	p = (unsigned char *)res;
	/* Transform the 3x8 bits to 4x6 bits, as required by base64. */
	for (i = 0; i < 30; i += 3){
		*p++ = base64Tbl[s[0] >> 2];
		*p++ = base64Tbl[((s[0] & 3) << 4) + (s[1] >> 4)];
		*p++ = base64Tbl[((s[1] & 0xf) << 2) + (s[2] >> 6)];
		*p++ = base64Tbl[s[2] & 0x3f];
		s += 3;
	}
	memcpy(result, res, 40);
	free(res);
}

void base64Decode (char *s, unsigned char *result)
{
	int len, i, bits;
	char *res, invertTbl[256];
	unsigned char *p;
	
	res = (char *)malloc(30 * sizeof(char) + 1);
	p = (unsigned char *)res;
	
	for(i = 0; i < 64; i++){
		invertTbl[base64Tbl[i]] = i;
	}
	for(i = 0; i < 40; i += 4){
		*p++ = (invertTbl[s[0]] << 2) + (invertTbl[s[1]] >> 4);
		*p++ = (invertTbl[s[1]] << 4) + (invertTbl[s[2]] >> 2);
		*p++ = (invertTbl[s[2]] << 6) + invertTbl[s[3]];
		s += 4;
	}
	memcpy(result, (unsigned char *)res, 30);
	free(res);
}
#endif
/*-----end-----*/

