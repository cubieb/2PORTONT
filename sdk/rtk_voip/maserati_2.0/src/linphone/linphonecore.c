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
//[SD6, bohungwu, exosip 3.5 integration
//#include "../exosip/eXosip.h"
//#include "../exosip/eXosip2.h"
#include "eXosip2/eXosip.h"
#include "eXosip2.h"
//]
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
//#include "stun_api.h"
#ifdef RTK_DNS
#include "dns_api.h"
#endif

#ifdef CONFIG_RTK_VOIP_DIALPLAN
#include "dialplanhelp.h"
#endif
#ifdef CONFIG_RTK_VOIP_IP_PHONE
#include "uihandler.h"
#endif

#ifdef RCM_VOIP_INTERFACE_ROUTE
#include <signal.h> 
#endif

extern TAPP *solar_ptr;
//extern uint32 rtk_Get_SLIC_Reg_Val(uint32 chid, uint32 reg, uint8 *regdata);
extern char caller_id_mode[];
extern char call_waiting_cid[];

#ifdef VOIP_CALL_LOG
static unsigned int call_seq_num=0;
#include "csv_parser.h"
#endif

void linphone_call_log_changed(int chid);

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


//porting from linphone 3.5
void _osip_list_set_empty(osip_list_t *l, void (*freefunc)(void*)){
	void *data;
	while(!osip_list_eol(l,0)) {
		data=osip_list_get(l,0);
		osip_list_remove(l,0);
		if (data) freefunc(data);
	}
}


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
//		eXosip_get_localip_for(to->url->host, &localip);
		rcm_get_localip_for_voip(solar_ptr->current_voip_interface,&localip);
	else
		localip = g_strdup( nat_address ) ;
	call->sdpctx = sdp_handler_create_context( &linphone_sdphandler , localip , from->url.username ) ;
	osip_free( localip ) ;
//	osip_from_to_str( from , &fromstr ) ;
//	osip_to_to_str( to , &tostr ) ;

	/*get pure from/to number */
	fromstr = (char *) osip_malloc(strlen(from->url.username));
	sprintf(fromstr, "%s", from->url.username);
	tostr = (char *) osip_malloc(strlen(to->url.username));
	sprintf(tostr, "%s", to->url.username);
	linphone_call_init_common( call , fromstr , tostr ) ;
	return call ;
}

LinphoneCall * linphone_call_new_incoming(LinphoneCore *lc, guint32 ssid, const char *from,
	const char *to, const osip_from_t * fromurl , const osip_to_t * tourl,int cid, int did)
{
	LinphoneCall * call = g_new0( LinphoneCall , 1 ) ;
	osip_from_t * me ;
	char * fromstr = NULL ;
	char * tostr = NULL ;
	
#ifdef NEW_CONTACT_RULE
	char username[usernameSize];
#endif

	if( call == NULL )
	{
		g_warning("incoming memory is not enough[%d][%d]\n", lc->chid, ssid);
		return NULL ;
	}

#ifdef NEW_CONTACT_RULE
    osip_from_init(&me);
	if (osip_from_parse(me, to) != 0)
		username[0] = '\0';
	else
		osip_strncpy(username, me->url.username, usernameSize-1);
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
	fromstr = (char *) osip_malloc(strlen(fromurl->url.username));
	sprintf(fromstr, "%s", fromurl->url.username);
	tostr = (char *) osip_malloc(strlen(tourl->url.username));
	sprintf(tostr, "%s", tourl->url.username);

	linphone_call_init_common( call ,fromstr , tostr ) ;

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
#ifdef VOIP_CALL_LOG
	char dir[20] = {0};
	call->seq_num = call_seq_num++;
	call->from    = from;
	call->to      = to;

	switch (call->dir){
		case LinphoneCallOutgoing:
			sprintf( dir , "Outgoing" );
			break;
		case LinphoneCallIncoming:
			sprintf( dir , "Incoming" );			
			break;
		default:
			dir[0] = '\0';
	}
	g_message("%s %s %s %s %u\n" , utoa(call->seq_num) , call->from , call->to , dir , call->start_time);	
	csv_api_call_start( utoa(call->seq_num) , call->from , call->to , dir , call->start_time);
#endif	
	return cl;
}
void linphone_call_log_completed(LinphoneCallLog *calllog, LinphoneCall *call){
	LinphoneCore *lc=call->core;
#ifdef VOIP_CALL_LOG
	time_t end_time = time(NULL);
	call->end_time  = end_time;
	call->duration  = call->end_time - call->start_time;
	calllog->duration=end_time-call->start_time;
#else
	calllog->duration=time(NULL)-call->start_time;
#endif
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

	linphone_call_log_changed(lc->chid);

	
#ifdef VOIP_CALL_LOG
	switch( calllog->status ){
		case LinphoneCallMissed:
			g_message("Missed state=%d status=%d\n" , call->state , calllog->status);
			g_message("%s %u %u\n" , utoa(call->seq_num) , call->start_time , call->end_time);
			csv_api_call_end( utoa(call->seq_num) , call->start_time , CALL_LOG_END_MISSED );
			break;	
		case LinphoneCallAborted:
			g_message("Aborted state=%d status=%d\n" , call->state , calllog->status);
			g_message("%s %u %u\n" , utoa(call->seq_num) , call->start_time , call->end_time);
			csv_api_call_end( utoa(call->seq_num) , call->start_time , CALL_LOG_END_ABORTED );
			break;
		case LinphoneCallSuccess:
			g_message("Success state=%d status=%d\n" , call->state , calllog->status);			
			g_message("%s %u %u\n" , utoa(call->seq_num) , call->start_time , call->end_time);
			csv_api_call_end( utoa(call->seq_num) , call->end_time, CALL_LOG_END_SUCCESS);
			break;
		default:
			g_message("Unknown state=%d status=%d\n" , call->state , calllog->status);
			g_message("%s %u %u\n" , utoa(call->seq_num) , call->start_time , call->end_time );
			csv_api_call_end( utoa(call->seq_num) , call->end_time, CALL_LOG_END_SUCCESS);
		}
#endif
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

/*
 <tr><td>Wed Jan  7 22:31:09 1970</td><td>66001</td><td>9001</td><td>Outgoing call</td><td>aborted</td><td>0 mn 0 sec</td></tr>
 */

return g_strdup_printf(_("<tr><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%02d:%02d</td></tr>\n"),
			cl->start_date,
			cl->from,
			cl->to,
	(cl->dir==LinphoneCallIncoming) ? _("Incoming call") : _("Outgoing call"),
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

void linphone_call_log_changed(int chid) 
{
	FILE *fh;
	GList *elem=NULL;
	LinphoneCore *templc=NULL;
	char filebuf[40];
#ifdef CONFIG_RTK_VOIP_TR104
    solarSendChangedToCwmp();
#endif
		memset(filebuf, 0, sizeof(filebuf));

		/*add call log output file, 
		fxs 0 is /tmp/call_log_p0
		fxs 1 is /tmp/call_log_p1
		*/

	sprintf(filebuf,"%s_p%d",_PATH_CALL_LOGS,chid);
		fh = fopen(filebuf, "w");
		if (!fh) {
			printf("Warning: cannot open %s. Limited output.\n", filebuf);
			return;
		}
	templc = &solar_ptr->ports[chid];
		elem=linphone_core_get_call_logs(templc);

		for (;elem!=NULL;elem=g_list_next(elem)){
			LinphoneCallLog *cl=(LinphoneCallLog*)elem->data;
			gchar *str=linphone_call_log_to_str(cl);
			fprintf(fh,"%s\n",str);
			g_free(str);
		}
		fclose(fh);		

}

#ifdef CONFIG_RTK_VOIP_TR104
void rcm_linphone_reset_call_log_statistics(LinphoneCore *lc){

GList *elem;
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

}

void linphone_call_log_statistics(LinphoneCore *lc, voiceProfileLineStatus *VPLineStatus){
	int inCallsRecv=0; /* Totoal incoming calls receive */
	int inCallsAns=0; /* Totoal incoming calls answer */
	int inCallsConnect=0; /* 200 OK responses successfully */
	int inCallsFailed=0; /* e.g 4xx responses */
	int outCallsAttempt=0; /* totoal outing calls */
	int outCallsAnswer=0; /* responsed by the callee */
	int outCallsConnect=0; /* 200 OK received successfully */
	int outCallsFailed=0; /* e.g 4xx received */
	unsigned int totalduration=0;	/* total call time */
	rtp_config_t rtp_config;
	rtcp_config_t rtcp_config;
	t38udp_config_t t38udp_config;
	payloadtype_config_t codec_config;
	t38_payloadtype_config_t t38_config;	
	TstRtpRtcpStatistics stRtpRtcpStatistics;


	GList *callLogsList;
	for(callLogsList=lc->call_logs; callLogsList!=NULL; callLogsList=g_list_next(callLogsList)){
		LinphoneCallLog *callLogs;

		callLogs=(LinphoneCallLog*)callLogsList->data;
		if(LinphoneCallIncoming== callLogs->dir){
			inCallsRecv++;
			totalduration=totalduration+callLogs->duration;
			if(LinphoneCallSuccess == callLogs->status){
				inCallsConnect++;
			}else if(LinphoneCallAborted == callLogs->status || LinphoneCallMissed == callLogs->status){
				inCallsAns++;
			}
		}else if(LinphoneCallOutgoing ==callLogs->dir){
			outCallsAttempt++;
			totalduration=totalduration+callLogs->duration;
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
	VPLineStatus->totalCallTime=totalduration;
    VPLineStatus->resetStatistics=0;	/* reset statistics, always 0 while get */


	if (lc->call[0] == NULL || lc->call[0]->sdpctx == NULL)
	{
		/* current no any call */
		//printf("linphone_call_log_statistics NO call \n");
		
	}else{
		//get ssid 0 information
		
		if (GetRtpCfgFromSession(&rtp_config, &rtcp_config, &t38udp_config, 
							&codec_config, &t38_config, lc, 0) == 0){
			memset(VPLineStatus->codec_string, 0, sizeof(VPLineStatus->codec_string));
			get_codec_name_with_pt(codec_config.local_pt,VPLineStatus->codec_string);
			//printf("VPLineStatus->codec_string is %s\n",VPLineStatus->codec_string);									
		}		
	}	
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
	char *local_ip = NULL;

	contact[0] = hostname[0] = 0;

	linphone_core_set_use_info_for_dtmf(lc, voip_ptr->dtmf_mode);
	lc->sip_conf.rfc2833_payload_type = voip_ptr->dtmf_2833_pt;

#ifdef SUPPORT_V152_VBD
	if( voip_ptr ->useV152 ) {
		lc->sip_conf.v152_payload_type = voip_ptr ->v152_payload_type;
		lc->sip_conf.v152_codec_type = voip_ptr ->v152_codec_type;
	} else {
		lc->sip_conf.v152_payload_type = 0;
		lc->sip_conf.v152_codec_type = rtpPayloadUndefined;
	}
#endif



#ifdef DYNAMIC_PAYLOAD
	dynamic_pt_set(rtpPayloadDTMF_RFC2833, lc->sip_conf.rfc2833_payload_type);
	dynamic_pt_set(rtpPayloadFaxModem_RFC2833, voip_ptr->fax_modem_2833_pt);

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
#ifdef SUPPORT_RTP_REDUNDANT_APP
	dynamic_pt_set(rtpPayloadRtpRedundant, lc->sip_conf.rtp_redundant_payload_type);
#endif

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
	//linphone_core_set_nat_address( lc , NULL, 0, FALSE ) ;	
	//eric.chung  temp add
	rcm_get_localip_for_voip(solar_ptr->current_voip_interface,&local_ip);


	linphone_core_set_nat_address( lc ,local_ip, 5060, 1 ) ;	
	

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
	lc->sip_conf.T38EnableSpoof	= voip_ptr->T38EnableSpoof;
	lc->sip_conf.T38DuplicateNum	= voip_ptr->T38DuplicateNum;
	
	/*--end--*/
/* +++++Add by Jack for VoIP security 240108+++++ */
#ifdef CONFIG_RTK_VOIP_SRTP
	lc->sip_conf.security_enable = voip_ptr->security_enable;
	lc->sip_conf.key_exchange_mode = voip_ptr->key_exchange_mode;
#endif /*CONFIG_RTK_VOIP_SRTP*/
/*-----end-----*/
	g_free(local_ip);
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
	lc->rtp_conf.nSID_mode = voip_ptr->sid_gainmode;
	lc->rtp_conf.nSID_level = voip_ptr->sid_noiselevel;
	lc->rtp_conf.nSID_gain = voip_ptr->sid_noisegain;
	lc->rtp_conf.bPLC = voip_ptr->PLC;
	lc->rtp_conf.nRtcpInterval = voip_ptr->RTCP_Interval;
	lc->rtp_conf.bRtcpXR = voip_ptr->RTCP_XR;
	lc->rtp_conf.nJitterDelay = ( guint32 )voip_ptr->jitter_delay;
	lc->rtp_conf.nMaxDelay = ( guint32 )voip_ptr->maxDelay;
	lc->rtp_conf.nMaxStrictDelay = ( g_SystemParam.max_strict_delay ? 
									g_SystemParam.max_strict_delay : 1000 );
	lc->rtp_conf.nJitterFactor = ( guint32 )voip_ptr->jitter_factor;
	lc->rtp_conf.nG726Packing = ( guint32 )voip_ptr->g726_packing;
#ifdef SUPPORT_G7111
	memcpy(lc->rtp_conf.g7111_precedence, voip_ptr->g7111_precedence,
		sizeof(voip_ptr->g7111_precedence));
#endif
#ifdef SUPPORT_VOICE_QOS
	lc->rtp_conf.tos = qos2tos(voip_ptr->voice_qos);
#endif
//printf("voip_ptr ->frame_size is %d\n");
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
	//dpInitParam.replace_rule_option		= g_pVoIPCfg->ports[chid].replace_rule_option;
	dpInitParam.dial_plan_enable		= g_pVoIPCfg->ports[chid].digitmap_enable;
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

#ifdef SUPPORT_RTP_REDUNDANT_APP
	if( g_pVoIPCfg->ports[chid].rtp_redundant_codec == -1 ||
		g_pVoIPCfg->ports[chid].rtp_redundant_payload_type < 96 ||
		g_pVoIPCfg->ports[chid].rtp_redundant_payload_type > 127 )
	{
		lc ->sip_conf.rtp_redundant_codec_type = -1;
		lc ->sip_conf.rtp_redundant_payload_type = 0;
	} else {
		lc ->sip_conf.rtp_redundant_codec_type = g_pVoIPCfg->ports[chid].rtp_redundant_codec;
		lc ->sip_conf.rtp_redundant_payload_type = g_pVoIPCfg->ports[chid].rtp_redundant_payload_type;
	}
#endif

	net_config_read(lc);//read WAN type and bandwidth.
	rtp_config_read(lc);//copy rtp configure from flash struct to linphone struct
	codecs_config_read(lc); //read codec
	sip_config_read(lc); /* this will start eXosip */

	// T38 config
	if( TRUE == lc->sip_conf.T38_enable )
	{
		char port[9];

		snprintf(port, 9, "%i", lc->sip_conf.T38_prot);
#ifdef OLD_API	 //eric temp mark , note need 
		//[SD6, bohungwu, exosip 3.5 integration
		//eXosip_set_faxport(lc->chid, port);
		//eXosip_set_firewall_faxport(lc->chid, 0); 
		rcm_eXosip_set_faxport(lc->chid, port);
		rcm_eXosip_set_firewall_faxport(lc->chid, 0); 
#endif
		//]
	}

#if 0
	lc->presence_mode = LINPHONE_STATUS_ONLINE ;
#endif

	lc->no_Answer_timeout=g_pVoIPCfg->NoAnswerTimer;

	lc->max_call_logs = 15;
	linphonec_proxy_add(lc); //read proxy and register it.
	/* Mandy: Keep NAT entry alive */
	//linphonec_proxy_list(lc);
	lc->net_conf.udp_keepAlive_frequency = 120;
	lc->net_conf.natEntry_build_time = time(NULL);

	/* Set CNG on/off, VAD/CNG threshold */
	for (mid = 0; mid < MAX_SS; mid++)
	{
		if (lc->rtp_conf.bCNG == 0) // CNG off
			lc->rtp_conf.nCNG_thr = 1;
		rcm_tapi_SetThresholdVadCng(chid, mid, lc->rtp_conf.nVAD_thr, lc->rtp_conf.nCNG_thr, lc->rtp_conf.nSID_mode,
				       lc->rtp_conf.nSID_level, lc->rtp_conf.nSID_gain);

	}
	/* dtmf_mode should be configed from web page */ //thlin+
	//0:rfc2833  1: sip info  2: inband  3: DTMF delete
	rcm_tapi_SetDTMFMODE(chid, lc->sip_conf.dtmf_type);

	/* Set the RFC2833 Packet interval */
	for (mid = 0; mid < MAX_SS; mid++)
	{
		rtk_SetRfc2833PacketInterval(chid, mid, 0, g_pVoIPCfg->ports[chid].dtmf_2833_pi);
		rtk_SetRfc2833PacketInterval(chid, mid, 1, g_pVoIPCfg->ports[chid].fax_modem_2833_pi);
	}
	
#if 0	
	rcm_tapi_SetRFC2833SendByAP(chid,1);
#endif
	rcm_tapi_Set_G168_LEC(chid, g_pVoIPCfg->ports[chid].lec);
	rtk_Set_VBD_EC(chid, 1, 2, g_pVoIPCfg->ports[chid].lec);
	/* Get echoTail value from flash and set to L_w.*/ //thlin+
	//if (chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
	if( RTK_VOIP_IS_DAA_CH( chid, g_VoIP_Feature ) )
	{
		/* TH: for DAA channel, ehco tail length should larger 4ms to get better performance. */
		if (g_pVoIPCfg->ports[chid].echoTail < 4)
			g_pVoIPCfg->ports[chid].echoTail = 4;
	}
	if (g_pVoIPCfg->ports[chid].nlp == 1)
		rcm_tapi_Set_echoTail(chid, g_pVoIPCfg->ports[chid].echoTail, 1,5,5,0); //1:nlp_mute mode 2: nlp_shift mode
	else if (g_pVoIPCfg->ports[chid].nlp == 0)
		rcm_tapi_Set_echoTail(chid, g_pVoIPCfg->ports[chid].echoTail, 0,5,5,0); // NLP disable

	/* Get Tx gain value from flash and set to SLIC.*/ //thlin+
	//rtk_Set_SLIC_Tx_Gain(chid, g_pVoIPCfg->ports[chid].slic_txVolumne);
	//using soft dsp gain
	/* Get Rx gain value from flash and set to SLIC.*/ //thlin+
	//rtk_Set_SLIC_Rx_Gain(chid, g_pVoIPCfg->ports[chid].slic_rxVolumne);
	//using soft dsp gain
	/* Get Speaker AGC flag from flash and set. */  // jwsyu+
	rcm_tapi_Set_SPK_AGC(chid, g_pVoIPCfg->ports[chid].speaker_agc, 55);

	/* Get Speaker AGC require level from flash and set. */  // jwsyu+
	rcm_tapi_Set_SPK_AGC_LVL(chid, g_pVoIPCfg->ports[chid].spk_agc_lvl);

	/* Get Speaker AGC gain up from flash and set. */  // jwsyu+
	rcm_tapi_Set_SPK_AGC_GUP(chid, g_pVoIPCfg->ports[chid].spk_agc_gu);

	/* Get Speaker AGC gain down from flash and set. */  // jwsyu+
	rcm_tapi_Set_SPK_AGC_GDOWN(chid, g_pVoIPCfg->ports[chid].spk_agc_gd);

	/* Get MIC AGC flag from flash and set. */  // jwsyu+
	rcm_tapi_Set_MIC_AGC(chid, g_pVoIPCfg->ports[chid].mic_agc, 55);

	/* Get MIC AGC require level from flash and set. */  // jwsyu+
	rcm_tapi_Set_MIC_AGC_LVL(chid, g_pVoIPCfg->ports[chid].mic_agc_lvl);

	/* Get MIC AGC gain up from flash and set. */  // jwsyu+
	rcm_tapi_Set_MIC_AGC_GUP(chid, g_pVoIPCfg->ports[chid].mic_agc_gu);

	/* Get MIC AGC gain down from flash and set. */  // jwsyu+
	rcm_tapi_Set_MIC_AGC_GDOWN(chid, g_pVoIPCfg->ports[chid].mic_agc_gd);

	/* Get FSK GEN MODE form flash and set. */	//jwsyu+20070205
	//rcm_tapi_Set_CID_FSK_GEN_MODE(chid, g_pVoIPCfg->ports[chid].cid_fsk_gen_mode);

	/* Get Fax Modem Det MODE form flash and set. */	//jwsyu+20080730
	rcm_tapi_SetFaxModemDet(chid, g_pVoIPCfg->ports[chid].fax_modem_det);

	/* Get Fax Modem Answer Tonde Det Config form flash and set. */	//jwsyu+20110830
	rcm_tapi_Set_AnswerTone_Det(chid, g_pVoIPCfg->ports[chid].anstone|0x3c0000, 0x0, 10);
	
	/* Set Fax Silence Det threshold */
	if( g_SystemParam.silence_demo )
	{
		rtk_Set_Silence_Det_Threshold(chid, g_SystemParam.silence_dbov, 
											g_SystemParam.silence_period );
	} else
		rtk_Set_Silence_Det_Threshold(chid, 55, 10 * 1000 );	// -55 dBov, 10 seconds 
	
	/* Get Fax/Modem RFC2833 Config from flash and set */ //thlin+
	rcm_tapi_SetFaxModemRfc2833(chid, g_pVoIPCfg->ports[chid].faxmodem_rfc2833&0x1,
					(g_pVoIPCfg->ports[chid].faxmodem_rfc2833&0x2)>>1,
					(g_pVoIPCfg->ports[chid].faxmodem_rfc2833&0x4)>>2);

	/* Get Voice gain from flash and set. */	//jwsyu+20070315
	rcm_tapi_Set_Voice_Gain(chid, g_pVoIPCfg->ports[chid].spk_voice_gain, g_pVoIPCfg->ports[chid].mic_voice_gain);

#if 0
	// TODO:
	// rtk_set_dtmf_det_param
#endif

	/* Get Caller ID mode from flash and set.*/ //thlin+
	lc->caller_id_mode = g_pVoIPCfg->ports[chid].caller_id_mode; /* 0:Bellcore 1:ETSI 2:BT 3:NTT (FSK:0-3) 4:DTMF*/
	lc->cid_dtmf_mode = g_pVoIPCfg->ports[chid].cid_dtmf_mode;
	if ((lc->caller_id_mode&7) == CID_DTMF)
	{
		g_message("Set Caller ID Mode to DTMF\n");
 		if (lc->caller_id_mode&8) //priori 1st ring
 		{
			rtk_SetDtmfCidParam(chid, DTMF_DIGIT_A + (lc->cid_dtmf_mode&0x3), DTMF_DIGIT_A + ((lc->cid_dtmf_mode>>2)&0x3), 0xF, 80, 80, 300, 300);
			//printf("=> 0x%x\n", lc->cid_dtmf_mode|0xE60);
 		}
  		else
 		{
			rtk_SetDtmfCidParam(chid, DTMF_DIGIT_A + (lc->cid_dtmf_mode&0x3), DTMF_DIGIT_A + ((lc->cid_dtmf_mode>>2)&0x3), 0xB, 80, 80, 300, 300);
			//printf("-> 0x%x\n", lc->cid_dtmf_mode|0xA0);
 		}
  
  	}
  	else
		rtk_Set_FSK_Area(chid, lc->caller_id_mode|0x100);  /* area -> 0:Bellcore 1:ETSI 2:BT 3:NTT, 0x100: auto SLIC action */

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
			rcm_tapi_Set_SLIC_Ring_Cadence(chid, 2000, 4000);
		else
			rcm_tapi_Set_SLIC_Ring_Cadence(chid, g_pVoIPCfg->ring_cadon[cad_idx], 
				g_pVoIPCfg->ring_cadoff[cad_idx]);/* (chid, cad_on_msec, cad_off_msec) */
	}

#ifdef NEW_STUN
//ericchung:  E8C not need STUN, but we still need NEW_STUN complier flag
//	rtk_stun_init(chid);
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

#ifndef __mips__
	// x86 simulation
#else
	//E8C configure only
	g_pVoIPCfg->auto_dial |= AUTO_DIAL_ALWAYS;
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

//    eXosip_get_localip_for(l_from->url->host, &guessed_ip);
	rcm_get_localip_for_voip(solar_ptr->current_voip_interface,&guessed_ip);


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
				//eXosip_get_localip_for("15.128.128.93", &tmp);
				rcm_get_localip_for_voip(solar_ptr->current_voip_interface,&tmp);
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


void linphone_core_set_sip_port(LinphoneCore *lc, int port)
{
//	int err = 0 ;
	if( port == lc->sip_conf.sip_port )
		return ;
	lc->sip_conf.sip_port = port ;

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

	if( exosip_running )
	{

#if 1 //ericchung move to linphonec.c , select
		while((ev=eXosip_event_wait(0,0))!=NULL){
			g_message("\r\n get ev->type %d\n",ev->type);
			linphone_core_process_event(solar_ptr, ev);
					eXosip_event_free( ev ) ;
				}
#endif		
		//run exosip other event , for example: register timeout, update timeout 
		eXosip_lock();
		eXosip_automatic_action();
		eXosip_unlock();


#if OLD_API //ericchung temp mark , need check , because re-try register is move to exEvent.c
		linphone_core_update_proxy_register( lc ) ;
#endif

//		linphone_core_refresh_subscribes( lc ) ;

	}


	}



void rcm_linphone_core_iterate_exosip()
{
	eXosip_event_t *ev;

	if( exosip_running )
	{

		while((ev=eXosip_event_wait(0,0))!=NULL){
			g_message("\r\n get ev->type %d\n",ev->type);
			linphone_core_process_event(solar_ptr, ev);
			eXosip_event_free(ev);
	}
//ericchung:  need eXosip_automatic_action for exosip		
		eXosip_lock();
		eXosip_automatic_action();
		eXosip_unlock();
	}

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
			osip_uri_t proxy_uri;
			char *sipaddr;
			voipCfgProxy_t *lc_proxy=&g_pVoIPCfg->ports[lc->chid].proxies[proxy->index];


			const char *proxy_addr=linphone_proxy_config_get_addr(proxy);

			osip_uri_init(&proxy_uri);
			if (osip_uri_parse(&proxy_uri,proxy_addr)<0){
			    osip_uri_free(&proxy_uri);
			    return FALSE;
			}
			if (proxy_uri.port > 0)
			{
				if( (strlen(lc_proxy->domain_name) != 0) &&(g_pVoIPCfg->X_CT_servertype==0)){
					sipaddr=g_strdup_printf("sip:%s@%s:%d",url, lc_proxy->domain_name,proxy_uri.port);
				}else{
			    sipaddr=g_strdup_printf("sip:%s@%s:%d",url,proxy_uri.host,proxy_uri.port);
				}
			}
			else
			{
			    	if( (strlen(lc_proxy->domain_name) != 0) &&(g_pVoIPCfg->X_CT_servertype==0)){
					sipaddr=g_strdup_printf("sip:%s@%s",url, lc_proxy->domain_name);
				}else{
			    sipaddr=g_strdup_printf("sip:%s@%s",url,proxy_uri.host);
				}
			}
			if (real_parsed_url!=NULL) *real_parsed_url=osip_to_create(sipaddr);
			if (real_url!=NULL) *real_url=sipaddr;
			else g_free(sipaddr);
#ifdef FIX_MEMORY_LEAK
			osip_uri_free(&proxy_uri);
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
//ericchung: for e8c project, not need stun
#if OLD_API
		return rtk_stun_send(lc->chid, ssid, flag);
#endif
		return;
#else
		char *c_address = voip_ptr->stun_addr;
		struct addrinfo *addrinfo;
		struct __eXosip_sockaddr srv_addr;

		//[SD6, bohungwu, exosip 3.5 integration */
		//i = eXosip_get_addrinfo(&addrinfo, voip_ptr->stun_addr, voip_ptr->stun_port);
		i = eXosip_get_addrinfo(&addrinfo, voip_ptr->stun_addr, voip_ptr->stun_port, IPPROTO_UDP);
		//]
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
	//osip_list_t * routes = NULL ;
	gchar * from = NULL ;
	gchar * anonymous_from = NULL ;
	osip_message_t * invite = NULL ;
	sdp_context_t * ctx = NULL ;
	LinphoneProxyConfig * proxy = NULL ;
	osip_from_t * parsed_url2 = NULL ;
	osip_to_t * real_parsed_url = NULL ;
	char * real_url = NULL ;
	char * routes=NULL;
	char  routes_url[LINE_MAX_LEN];
	int callerid_hidden=0;
#ifdef ATTENDED_TRANSFER
	char	*replaces_str = NULL;
	char* url_dup;
#endif
	voipCfgPortParam_t *voip_ptr = &g_pVoIPCfg->ports[lc->chid];

	voipCfgProxy_t *proxy_cfg = &g_pVoIPCfg->ports[lc->chid].proxies[lc->default_proxy_index];
	char szbuf[LINE_MAX_LEN];

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
#if 0 //eric
		if(proxy_cfg->outbound_enable){

			printf("have outbound proxy\n");
			if (proxy_cfg->outbound_port && proxy_cfg->outbound_port != 5060)
				sprintf(routes_url, "sip:%s:%d", proxy_cfg->outbound_addr, proxy_cfg->outbound_port);
			else
				sprintf(routes_url, "sip:%s", proxy_cfg->outbound_addr);			

			routes = osip_strdup(routes_url);
		}
#endif		


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
		if (proxy_cfg->number[0])
		{
			if (proxy_cfg->display_name[0])
				sprintf(szbuf, "\"%s\" <sip:%s@%s>", proxy_cfg->display_name, proxy_cfg->number,solar_ptr->bound_voip_ipaddress);
			else
				sprintf(szbuf, "sip:%s@%s", proxy_cfg->number,solar_ptr->bound_voip_ipaddress);

			from = osip_strdup(szbuf);
		}
		else
		{
			sprintf(szbuf, "sip:%s", solar_ptr->bound_voip_ipaddress);
			from = osip_strdup(szbuf);
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

	//[SD6, bohungwu, exosip 3.5 integration 
	//err = eXosip_build_initial_invite(lc->chid, &invite, (gchar *) real_url, (gchar *) from, routes, "Phone call");

	printf("voip_ptr->fxsport_flag is %d\n",voip_ptr->fxsport_flag);
	if(voip_ptr->fxsport_flag & FXS_CALLERID_HIDDEN){
			callerid_hidden=1;
			if(lc->fxsfeatureFlags & FXS_ONE_SHOT_CID_DISPLAY)
			callerid_hidden=0;
	}else{
		//mib caller id enable
			callerid_hidden=0;
			if(lc->fxsfeatureFlags & FXS_ONE_SHOT_CID_HIDDEN)
				callerid_hidden=1;
	}
	anonymous_from = osip_strdup("sip:anonymous@anonymous.invalid");

	if((callerid_hidden==1)&& (g_pVoIPCfg->rfc_flags & SIP_ANONYMOUSE_FROM)){
	
		err = eXosip_call_build_initial_invite(&invite, (gchar *) real_url, (gchar *) anonymous_from, routes,NULL);
	}else
		err = eXosip_call_build_initial_invite(&invite, (gchar *) real_url, (gchar *) from, routes, NULL);
	//]
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
//	if(lc->fxsfeatureFlags & FXS_ONE_SHOT_CID_HIDDEN)
	if(callerid_hidden)
		osip_from_parse( parsed_url2 , anonymous_from ) ;
	else
	osip_from_parse( parsed_url2 , from ) ;

	lc->call[ssid] = linphone_call_new_outgoing(lc, parsed_url2, real_parsed_url);
	ctx = lc->call[ssid]->sdpctx;
	sdpmesg = sdp_context_get_offer( ctx ) ;

#ifdef MULTI_PROXY
	// record active proxy in call
	lc->call[ssid]->proxy = proxy;
#endif

	if(lc->call[ssid]->proxy!=NULL){
	g_message("lc->call[ssid]->proxy->contact is %s\n",lc->call[ssid]->proxy->contact);

	_osip_list_set_empty(&invite->contacts,(void (*)(void*))osip_contact_free);
	osip_message_set_contact(invite,lc->call[ssid]->proxy->contact);
	}
	
	/* invite option */
	if(proxy_cfg==NULL){
		g_message("\r\n proxy_cfg is null \n");
	}


	if(proxy!=NULL){
	g_message("proxy_cfg->SessionUpdateTimer is  %d\n",proxy_cfg->SessionUpdateTimer);
	if(proxy_cfg->SessionUpdateTimer!=0){
		sprintf(szbuf, "%d", proxy_cfg->SessionUpdateTimer);
	//	printf("szbuf is %s\n",szbuf);
		osip_message_set_header(invite, "Session-expires",  szbuf);
	osip_message_set_header(invite, "Min-SE", "90");
		if(!(g_pVoIPCfg->rfc_flags & SIP_DISABLE_PRACK)){
		osip_message_set_supported(invite, "timer,100rel");
	}else{
			osip_message_set_supported(invite, "timer");
		}
	}else{
		if(!(g_pVoIPCfg->rfc_flags & SIP_DISABLE_PRACK)){
		osip_message_set_supported(invite, "100rel");
	}
	}

	strncpy(invite->orig_number,g_pVoIPCfg->ports[lc->chid].proxies->login_id,strlen(g_pVoIPCfg->ports[lc->chid].proxies->login_id));
	}
//	if(lc->fxsfeatureFlags & FXS_ONE_SHOT_CID_HIDDEN){
	if(callerid_hidden){

		osip_message_set_header(invite, "P-Preferred-Identity", from);
		osip_message_set_header(invite, "Privacy", "id");
		//for some sip server, need P-AoC header
	//	osip_message_set_header(invite, "P-AoC", "Inform,type=AOC-D");

		
	}
	osip_message_set_allow(invite, "INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, NOTIFY, MESSAGE, SUBSCRIBE, INFO, UPDATE");
	osip_service_route_t * service_route=NULL;
	int i=0;

	if(proxy!=NULL){
		osip_message_get_knownheaderlist(&proxy->service_routes, i, &service_route);
		while((service_route!=NULL))
		{
			osip_message_set_service_route(invite,service_route->value);
			i++;
			osip_message_get_knownheaderlist(&proxy->service_routes, i, &service_route);
		}
		osip_message_set_outboundproxy(	invite,proxy->outbound_proxy,proxy->outbound_port);
	}	
	osip_message_set_body(invite, sdpmesg, strlen(sdpmesg));
	osip_message_set_content_type(invite, "application/sdp");
	rcm_osip_message_set_content_length(invite, strlen(sdpmesg));

	eXosip_lock() ;
	err = eXosip_call_send_initial_invite(invite);


/* eric for test change sequence if() movoe up !! */
	if( err < 0 )
	{
		g_warning( "Could not initiate call.\n" ) ;
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
	if (anonymous_from!=NULL)
		osip_free(anonymous_from);
	
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

int GetRtpCfgFromSession(rtp_config_t *rtp_config,
	rtcp_config_t *rtcp_config,
	t38udp_config_t *t38udp_config, 
	payloadtype_config_t *codec_config, 
	t38_payloadtype_config_t *t38_config, 
	LinphoneCore *lc, int ssid)
{
	unsigned long src_ip;
	StreamParams *media_params;
	int jitt_comp;
	int offset;

	if (lc->call[ssid] == NULL || lc->call[ssid]->sdpctx == NULL)
	{
		g_warning("%s: call is null!?\n", __FUNCTION__);
		return FALSE;
	}

	if (1 == lc->call[ssid]->t38_params.initialized) {
		media_params = &lc->call[ssid]->t38_params;
	}
	else if (1 == lc->call[ssid]->audio_params.initialized) {
		jitt_comp = lc->rtp_conf.audio_jitt_comp;
		media_params = &lc->call[ssid]->audio_params;
	}
	else {
		g_warning("%s: no stream ready\n", __FUNCTION__);
		return FALSE;
	}

	/* src ip */
	if (lc->call[ssid]->sdpctx->rtp_localip == NULL)
	{
		char *local_ip = NULL;

		g_warning("%s: rtp_localip is null!?\n", __FUNCTION__);
	//	eXosip_get_localip_for(media_params->remote_addr, &local_ip);
		rcm_get_localip_for_voip(solar_ptr->current_voip_interface,&local_ip);
		src_ip = inet_addr(local_ip);
		g_free(local_ip);
	}
	else
	{
		// get rtp local ip from sdp handshake
		src_ip = inet_addr(lc->call[ssid]->sdpctx->rtp_localip);
	}

	/*
	 * Setup rtp
	 */
	memset(rtp_config, 0, sizeof(*rtp_config));

	rtp_config->chid = lc->chid;
	rtp_config->sid = ssid;
	rtp_config->isTcp = 0;

	rtp_config->remIp = inet_addr(media_params->remote_addr);
	rtp_config->remPort = htons((unsigned short) media_params->remoteport);
	rtp_config->extIp = src_ip;
	rtp_config->extPort = htons((unsigned short) media_params->localport);

#ifdef SUPPORT_VOICE_QOS
	rtp_config->tos = lc->rtp_conf.tos;
#endif

	rtp_config->rfc2833_payload_type_local = (uint16)
		media_params->rfc2833_local_pt;
	rtp_config->rfc2833_payload_type_remote = (uint16)
		media_params->rfc2833_remote_pt;
	g_message("=>rfc2833 payload: local=%d, remote=%d\n",
		rtp_config->rfc2833_payload_type_local,
		rtp_config->rfc2833_payload_type_remote);


	if (g_pVoIPCfg->ports[rtp_config->chid].fax_modem_2833_pt_same_dtmf == 1)
	{
		// Fax/Modem RFC2833 PT are the same to the DTMF RFC2833 PT
		rtp_config->rfc2833_fax_modem_pt_local = (uint16) media_params->rfc2833_local_pt;
		rtp_config->rfc2833_fax_modem_pt_remote = (uint16) media_params->rfc2833_remote_pt;
	}
	else
	{
		// Fax/Modem RFC2833 PT is set by flash value
		rtp_config->rfc2833_fax_modem_pt_local = g_pVoIPCfg->ports[rtp_config->chid].fax_modem_2833_pt;
		rtp_config->rfc2833_fax_modem_pt_remote = g_pVoIPCfg->ports[rtp_config->chid].fax_modem_2833_pt;
	}

#ifdef CONFIG_RTK_VOIP_SRTP
	if (media_params->static_pt != rtpPayloadT38_Virtual &&
		1 == lc->sip_conf.security_enable &&
		-1 != media_params->localCryptAlg) {
		/* debug both remote sdp key and local sdp key to remote srtp key and local srtp key */
		base64Decode(media_params->remoteSDPKey, rtp_config->remoteSrtpKey);
		base64Decode(media_params->localSDPKey, rtp_config->localSrtpKey);
		rtp_config->remoteCryptAlg = media_params->remoteCryptAlg;
		rtp_config->localCryptAlg = NULL_AUTH;
	}
	else {
		rtp_config->remoteCryptAlg = NULL_AUTH;
		rtp_config->localCryptAlg = NULL_AUTH;
	}
#endif /* CONFIG_RTK_VOIP_SRTP */

#ifdef SUPPORT_RTP_REDUNDANT_APP
	if (media_params->static_pt != rtpPayloadT38_Virtual &&
		media_params->initialized_rtp_redundant) {
		rtp_config->rtp_redundant_payload_type_local = media_params->local_pt_rtp_redundant;
		rtp_config->rtp_redundant_payload_type_remote = media_params->remote_pt_rtp_redundant;
		rtp_config->rtp_redundant_max_Audio = ( g_SystemParam.red_demo ? g_SystemParam.red_audio : 2 );
		rtp_config->rtp_redundant_max_RFC2833 = ( g_SystemParam.red_demo ? g_SystemParam.red_rfc2833 : 5 );
	} else {
		rtp_config->rtp_redundant_payload_type_local = 0;
		rtp_config->rtp_redundant_payload_type_remote = 0;
		rtp_config->rtp_redundant_max_Audio = 0; 
		rtp_config->rtp_redundant_max_RFC2833 = 0;
	}
#endif

	rtp_config->SID_payload_type_local = 0;
	rtp_config->SID_payload_type_remote = 0;
	
	// This is demo purpose 
	offset = lc->chid * 2 + ssid;
	
	rtp_config->init_randomly = ( g_SystemParam.rtp_init ? 0 : 1 );
	rtp_config->init_seqno = ( g_SystemParam.rtp_init ? g_SystemParam.rtp_init_seqno : 0 );
	rtp_config->init_SSRC = ( g_SystemParam.rtp_init ? g_SystemParam.rtp_init_ssrc + offset : 0 );
	rtp_config->init_timestamp = ( g_SystemParam.rtp_init ? g_SystemParam.rtp_init_timestamp : 0 );
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
	//IPv4 or IPv6
	rtp_config->ipv6 = 0;
#endif	
	// RTCP: copy most settings from RTP 
	memset(rtcp_config, 0, sizeof(*rtcp_config));
	rtcp_config->chid = rtp_config->chid;
	rtcp_config->sid = rtp_config->sid;
	rtcp_config->remIp = rtp_config->remIp;
	rtcp_config->remPort = rtp_config->remPort + 1;
	rtcp_config->extIp = rtp_config->extIp;
	rtcp_config->extPort = rtp_config->extPort + 1;
#ifdef SUPPORT_VOICE_QOS
	rtcp_config->tos = rtp_config->tos;
#endif
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
	//IPv4 or IPv6
	rtcp_config->ipv6 = 0;
#endif		
	rtcp_config->txInterval = lc->rtp_conf.nRtcpInterval * 1000;/*ms*/
	rtcp_config->enableXR = lc->rtp_conf.bRtcpXR;
	
	// T.38 config 
	memset(t38udp_config, 0, sizeof(*t38udp_config));
	t38udp_config->chid = rtp_config->chid;
	t38udp_config->sid = rtp_config->sid;
	t38udp_config->isTcp = rtp_config->isTcp;
	t38udp_config->remIp = rtp_config->remIp;
	t38udp_config->remPort = rtp_config->remPort;
	t38udp_config->extIp = rtp_config->extIp;
	t38udp_config->extPort = rtp_config->extPort;
#ifdef SUPPORT_VOICE_QOS
	t38udp_config->tos = rtp_config->tos;
#endif
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
	//IPv4 or IPv6
	t38udp_config->ipv6 = 0;
#endif
	
	/*
	 * Setup codec
	 */
	memset(codec_config, 0, sizeof(*codec_config));

	codec_config->chid = lc->chid;
	codec_config->sid = ssid;
	codec_config->local_pt = media_params->local_pt;
	codec_config->remote_pt = media_params->remote_pt;
	codec_config->uLocalPktFormat = media_params->static_pt;
	codec_config->uRemotePktFormat = media_params->static_pt;

#ifdef SUPPORT_V152_VBD
	if (media_params->static_pt != rtpPayloadT38_Virtual &&
		media_params->initialized_vbd) {
		codec_config->local_pt_vbd = media_params->local_pt_vbd;
		codec_config->remote_pt_vbd = media_params->remote_pt_vbd;
		codec_config->uPktFormat_vbd = media_params->static_pt_vbd;
	} else {
		codec_config->local_pt_vbd = rtpPayloadUndefined;
		codec_config->remote_pt_vbd = rtpPayloadUndefined;
		codec_config->uPktFormat_vbd = rtpPayloadUndefined;
	}
#endif

#ifndef SUPPORT_CUSTOMIZE_FRAME_SIZE

  #if 0
	// pkshih: give one to reduce packet loss impact 
	codec_config->nLocalFramePerPacket = ( g_SystemParam.fpp_demo ? g_SystemParam.fpp : 1 );
	codec_config->nRemoteFramePerPacket = ( g_SystemParam.fpp_demo ? g_SystemParam.fpp : 1 );
	codec_config->nFramePerPacket_vbd = ( g_SystemParam.fpp_demo ? g_SystemParam.fppvbd : 1 );
  #else
	/* Frame per packet by hard coding */
	if (codec_config->uLocalPktFormat == rtpPayloadG723||
		codec_config->uLocalPktFormat == rtpPayloadGSM ||
		codec_config->uLocalPktFormat == rtpPayload_iLBC ||
		codec_config->uLocalPktFormat == rtpPayload_iLBC_20ms ||
		codec_config->uLocalPktFormat == rtpPayload_SPEEX_NB_RATE8 ||
		codec_config->uLocalPktFormat == rtpPayload_SPEEX_NB_RATE2P15 ||
		codec_config->uLocalPktFormat == rtpPayload_SPEEX_NB_RATE5P95 ||
		codec_config->uLocalPktFormat == rtpPayload_SPEEX_NB_RATE11 ||
		codec_config->uLocalPktFormat == rtpPayload_SPEEX_NB_RATE15 ||
		codec_config->uLocalPktFormat == rtpPayload_SPEEX_NB_RATE18P2 ||
		codec_config->uLocalPktFormat == rtpPayload_SPEEX_NB_RATE24P6 ||
		codec_config->uLocalPktFormat == rtpPayload_SPEEX_NB_RATE3P95 )
		codec_config->nLocalFramePerPacket = 1 ;
	else
		codec_config->nLocalFramePerPacket = 2 ;
	
	if (codec_config->uRemotePktFormat == rtpPayloadG723||
		codec_config->uRemotePktFormat == rtpPayloadGSM ||
		codec_config->uRemotePktFormat == rtpPayload_iLBC ||
		codec_config->uRemotePktFormat == rtpPayload_iLBC_20ms ||
		codec_config->uRemotePktFormat == rtpPayload_SPEEX_NB_RATE8 ||
		codec_config->uRemotePktFormat == rtpPayload_SPEEX_NB_RATE2P15 ||
		codec_config->uRemotePktFormat == rtpPayload_SPEEX_NB_RATE5P95 ||
		codec_config->uRemotePktFormat == rtpPayload_SPEEX_NB_RATE11 ||
		codec_config->uRemotePktFormat == rtpPayload_SPEEX_NB_RATE15 ||
		codec_config->uRemotePktFormat == rtpPayload_SPEEX_NB_RATE18P2 ||
		codec_config->uRemotePktFormat == rtpPayload_SPEEX_NB_RATE24P6 ||
		codec_config->uRemotePktFormat == rtpPayload_SPEEX_NB_RATE3P95 )
		codec_config->nRemoteFramePerPacket = 1 ;
	else
		codec_config->nRemoteFramePerPacket = 2 ;
  #endif
//  asdf
#else
	#if (!defined(SUPPORT_CODEC_DESCRIPTOR)) //Alex, for checking codec enum number with payload number
	//eric mark
    unsigned int idxCodec = _CODEC_MAX;
	idxCodec = get_list_number_with_pt(	media_params->local_pt);
	#endif

	//printf("idxCodec  is %d\n",idxCodec );
	//printf("media_params->local_pt is %d\n",media_params->local_pt);	
	/* Frame per packet by flash */
	if( idxCodec >= 0 && idxCodec < _CODEC_MAX &&
		g_mapSupportedCodec[idxCodec] >= 0 && g_mapSupportedCodec[idxCodec] < _CODEC_MAX)
	{
		codec_config->nLocalFramePerPacket =
							lc ->rtp_conf.pFrameSize[ g_mapSupportedCodec[idxCodec] ] + 1;
		codec_config->nRemoteFramePerPacket =
							lc ->rtp_conf.pFrameSize[ g_mapSupportedCodec[idxCodec] ] + 1;
	} else {
		printf( "Unexpected codec in MINE-TYPE!\n" );
		codec_config->nLocalFramePerPacket = 1;
		codec_config->nRemoteFramePerPacket = 1;
	}
#endif

	codec_config->bVAD = lc->rtp_conf.bVAD;
	codec_config->bPLC = lc->rtp_conf.bPLC;
	codec_config->nJitterDelay = lc->rtp_conf.nJitterDelay;
	codec_config->nMaxDelay = lc->rtp_conf.nMaxDelay;
	codec_config->nMaxStrictDelay = lc->rtp_conf.nMaxStrictDelay;
	codec_config->nJitterFactor = lc->rtp_conf.nJitterFactor;

	codec_config->nG723Type = lc->rtp_conf.g7231_rate;
	codec_config->nG726Packing = lc->rtp_conf.nG726Packing;
#ifdef SUPPORT_G7111
	// use first precedence for DSP
	codec_config->nG7111Mode = media_params->g7111_precedence[0];
#endif

	//T.38 parameters
	t38_config->chid = lc->chid;
	t38_config->sid = ssid;
	t38_config->bT38ParamEnable = lc->sip_conf.T38ParamEnable;
	t38_config->nT38MaxBuffer = lc->sip_conf.T38MaxBuffer;
	t38_config->nT38RateMgt = lc->sip_conf.T38RateMgt;
	t38_config->nT38MaxRate = lc->sip_conf.T38MaxRate;
	t38_config->bT38EnableECM = lc->sip_conf.T38EnableECM;
	t38_config->nT38ECCSignal = lc->sip_conf.T38ECCSignal;
	t38_config->nT38ECCData = lc->sip_conf.T38ECCData;
	t38_config->bT38EnableSpoof = lc->sip_conf.T38EnableSpoof;
	t38_config->nT38DuplicateNum = lc->sip_conf.T38DuplicateNum;
	t38_config->nPcmMode = 2; // T.38 use Narrow-band Mode

#if 0 // also workable
	// Set PCM mode according to codec
	if ( (codec_config->uPktFormat == rtpPayloadG722)
		|| (codec_config->uPktFormat == rtpPayloadPCMU_WB)
		|| (codec_config->uPktFormat == rtpPayloadPCMA_WB))
		codec_config->nPcmMode = 3; // Wide-band Mode
	else
		codec_config->nPcmMode = 2; // Narrow-band Mode
#else
	codec_config->nPcmMode = 1; // Auto Mode
#endif

	return 0;
}

static gboolean media_stream_start_with_dsp(LinphoneCore *lc, guint32 ssid)
{
	rtp_config_t rtp_config;
	rtcp_config_t rtcp_config;
	t38udp_config_t t38udp_config;
	payloadtype_config_t codec_config;
	t38_payloadtype_config_t t38_config;

	if (GetRtpCfgFromSession(&rtp_config, &rtcp_config, &t38udp_config, 
							&codec_config, &t38_config, lc, ssid) != 0)
	{
		g_warning("%s: GetRtpCfgFromSession failed\n", __FUNCTION__);
		return FALSE;
	}

#ifdef CHECK_RESOURCE
	if (VOIP_RESOURCE_UNAVAILABLE ==
		rcm_tapi_VoIP_resource_check(lc->chid, codec_config.uLocalPktFormat))
	{
	#ifdef CONFIG_RTK_VOIP_IVR
		linphone_no_resource_handling(lc, ssid);
	#endif
		return FALSE;
	}
#endif

#ifdef CONFIG_RTK_VOIP_IP_PHONE
	lc->call[ssid]->rfc2833_payload_type_remote =
		rtp_config.rfc2833_payload_type_remote;
#endif

	// rock: add fax/modem handling
	if (!lc->call[ssid]->t38_params.initialized) 
	{
		if (lc->call[ssid]->faxflag == FMS_MODEM_RUN ||
			lc->call[ssid]->faxflag == FMS_BEMODEM_RUN)
		{
			codec_config.nJitterFactor = 13; // fixed delay
			if (g_SystemParam.modem_lec == 0)
				rtk_Set_G168_LEC(lc->chid, 0); // turn off LEC if MODEM (or High Speed FAX)
		}
		else if (lc->call[ssid]->faxflag == FMS_FAX_RUN ||
			lc->call[ssid]->faxflag == FMS_BEFAX_RUN)
		{
			codec_config.nJitterFactor = 13; // fixed delay
			rtk_Set_G168_LEC(lc->chid, g_pVoIPCfg->ports[lc->chid].lec); // default LEC if low speed fax
		}
		else
		{
			rtk_Set_G168_LEC(lc->chid, g_pVoIPCfg->ports[lc->chid].lec); // default LEC
		}
	}
	else
	{
		rtk_Set_G168_LEC(lc->chid, g_pVoIPCfg->ports[lc->chid].lec); // default LEC
	}
	
	if( !lc->call[ssid]->t38_params.initialized )
	{
		rcm_tapi_SetRtpConfig(&rtp_config);
		if( rtcp_config.txInterval )
			rcm_tapi_SetRtcpConfig(&rtcp_config) ;//thlin+ for RTCP
		rcm_tapi_SetRtpPayloadType(&codec_config) ;
		rcm_tapi_SetRtpSessionState(lc->chid, ssid, rtp_session_sendrecv);
	} else {
		rtk_SetT38UdpConfig(&t38udp_config);
		rtk_SetT38PayloadType(&t38_config);
	}
	return TRUE;
}

#ifdef CHECK_RESOURCE
int linphone_core_start_media_streams(LinphoneCore *lc, guint32 ssid, LinphoneCall *call)
#else
void linphone_core_start_media_streams(LinphoneCore *lc, guint32 ssid, LinphoneCall *call)
#endif
{
	if (lc->audiostream[ssid])
		linphone_core_stop_media_streams(lc, ssid);
#ifdef RCM_VOIP_INTERFACE_ROUTE
	if(g_pVoIPCfg->rfc_flags & SIP_RTP_ROUTING)
		rcm_add_voip_interface_route(lc , ssid);
#endif
	lc->audiostream[ssid] = media_stream_start_with_dsp(lc, ssid);


	if (lc->audiostream[ssid]) {
		//lc->call[ssid]->state = LCStateAVRunning;
		SetCallState(lc->chid,ssid,&(lc->call[ssid]->state),LCStateAVRunning);
	}

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
		rcm_tapi_SetRtpSessionState(lc->chid, ssid, FALSE);
#endif

#ifdef RCM_VOIP_INTERFACE_ROUTE
		if(g_pVoIPCfg->rfc_flags & SIP_RTP_ROUTING)
			rcm_del_voip_interface_route(lc , ssid);
#endif

	}
}

int linphone_core_accept_dialog(LinphoneCore *lc, guint32 ssid, const char *url)
{
	char * sdpmesg ;
	voipCfgProxy_t *proxy_cfg = &g_pVoIPCfg->ports[lc->chid].proxies[lc->default_proxy_index];
	sdp_context_t * ctx = NULL ;

	if (lc->call[ssid] == NULL)
	{
		g_warning("Call null[%d][%d]\n", lc->chid, ssid);
		return -1;
	}

	rcm_timerCancel(lc->no_Answer_timer_id);
	
	/* stop ringing */
	if (lc->ringstream[ssid])
	{
		lc->ringstream[ssid] = FALSE;
#ifndef __FEDORA_COMPLIER
		rcm_tapi_SetRingFXS(lc->chid, FALSE);
		rcm_tapi_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_RINGING, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
#endif
	}

	/* sends a 200 OK */
    /* Mandy add for stun support */
#if 0//eric: mark STUN	, not upport now
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

#endif
/* delay offer, 200ok must with sdp first */
	if(lc->call[ssid]->delay_offer_enable){ 	
		ctx = lc->call[ssid]->sdpctx ;		
		sdpmesg = sdp_context_get_offer( ctx ) ;	
		
	}else{
	sdpmesg = lc->call[ssid]->sdpctx->answerstr ;	/* takes the sdp already computed */
	}

	eXosip_lock() ;
	//[SD6, bohungwu, exosip 3.5 integration 
	//eXosip_answer_call_with_body( lc->call[ssid]->did , 200 , "application/sdp" , sdpmesg ) ;
	{
		osip_message_t *answer=NULL;
		int i;
		
		i = eXosip_call_build_answer(lc->call[ssid]->tid, 200, &answer);

		if(lc->call[ssid]->proxy!=NULL){
		_osip_list_set_empty(&answer->contacts,(void (*)(void*))osip_contact_free);
		osip_message_set_contact(answer,lc->call[ssid]->proxy->contact);
		}
		//eric.chung ,if support session timer 
		if(proxy_cfg->SessionUpdateTimer!=0){
		osip_message_set_supported(answer, "timer");
		}

		
		if(i != 0)
		{
			eXosip_call_send_answer(lc->call[ssid]->tid, 400, NULL);
		}
		else
		{
			i = osip_message_set_body(answer, sdpmesg, strlen(sdpmesg));
			if(i == 0)
			{
				i = osip_message_set_content_type(answer, "application/sdp");
				if(i == 0)
				{
					i = rcm_osip_message_set_content_length(answer, strlen(sdpmesg));
					if(i == 0)
					{
						eXosip_call_send_answer(lc->call[ssid]->tid, 200, answer) ;
					}
					else
					{
						eXosip_call_send_answer(lc->call[ssid]->tid, 400, NULL);
					}
				}
				else
				{
					eXosip_call_send_answer(lc->call[ssid]->tid, 400, NULL);
				}
			}
			else
			{
				eXosip_call_send_answer(lc->call[ssid]->tid, 400, NULL);
			}
		}
	}
	//]
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
	//lc->call_terminated[ssid]->state = LCStateTerminated;
	SetCallState(lc->chid,ssid,&(lc->call_terminated[ssid]->state),LCStateTerminated);

}

int linphone_core_terminate_dialog(LinphoneCore *lc, uint32 ssid, const char *url)
{
		LinphoneCall *call;

		call = lc->call[ssid];
		if( call != NULL )
		{
			eXosip_lock() ;
			//[SD6, bohungwu, exosip 3.5 integration
			//eXosip_terminate_call( call->cid , call->did ) ;
			eXosip_call_terminate( call->cid , call->did ) ;
			//]
			eXosip_unlock() ;

			/* stop ringing */
			if( lc->ringstream[ssid] )
			{
				lc->ringstream[ssid] = FALSE ;
				rcm_tapi_SetRingFXS(lc->chid, FALSE);
//				rcm_tapi_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
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
		rcm_tapi_SetRTPRFC2833(lc->chid, ssid, dtmf, 100);	// keyin is digit not char
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
		case SIGN_FLASHHOOK:
					return;
				break;
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

		rcm_tapi_SetPlayTone( lc ->chid, ssid, nTone, TRUE, DSPCODEC_TONEDIRECTION_REMOTE );

label_ignore_dtmf_in_band_tone:
#endif /* CONFIG_RTK_VOIP_IP_PHONE */
		return;
  	}

	if (lc->sip_conf.dtmf_type==1)		// DTMF SIP INFO (out-of-band)
  	{
		char dtmf_body[128];
		//osip_message_t *msg=NULL;
		//char clen[10];
		char dtmf_char;

	    /* Out of Band DTMF (use INFO method) */
	    LinphoneCall *call=lc->call[ssid];
	    if (call==NULL)
	    {
			return;
		}

		if(dtmf!=SIGN_FLASHHOOK){
			dtmf_char = (dtmf == 10) ? '0' :
					(dtmf == 11) ? '*' :
					(dtmf == 12) ? '#' :
					(dtmf + '0');
			snprintf(dtmf_body, sizeof(dtmf_body), "Signal=%c\r\nDuration=%d\r\n", dtmf_char, g_pVoIPCfg->ports[lc->chid].sip_info_duration);
		}else{
			//add  hook flash via SIP 
			snprintf(dtmf_body, sizeof(dtmf_body), "Signal=16\r\nDuration=%d\r\n", g_pVoIPCfg->ports[lc->chid].sip_info_duration);

		}


		//[SD6, bohungwu, exosip 3.5 integration
		//eXosip_lock();
		//eXosip_info_call(call->did, "application/dtmf-relay", dtmf_body);
		rcm_eXosip_info_call(call->did, "application/dtmf-relay", dtmf_body);
		//eXosip_unlock();
		//]

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
#ifdef OLD_API	 //eric temp mark , note need 
		//[SD6, bohungwu, exosip 3.5 integration
		//eXosip_set_firewallip( addr,lc->chid ) ;
		rcm_eXosip_set_firewallip( addr,lc->chid ) ;
		//]
		sprintf(natport, "%i", port);
		//[SD6, bohungwu, exosip 3.5 integration
		//eXosip_set_firewallport(lc->chid, natport);
		rcm_eXosip_set_firewallport(lc->chid, natport);
#endif
		//]
	}
	else
	{
		lc->net_conf.use_nat = FALSE ;
		lc->net_conf.nat_address = NULL ;
		lc->net_conf.nat_port = 0;
#ifdef OLD_API	 //eric temp mark , note need 
		//[SD6, bohungwu, exosip 3.5 integration
		//eXosip_set_firewallip( "",lc->chid) ;		
		//eXosip_set_firewallport(lc->chid, NULL);
		rcm_eXosip_set_firewallip( "",lc->chid) ;		
		rcm_eXosip_set_firewallport(lc->chid, NULL);
		//]
#endif
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
		linphone_proxy_send_unregister(cfg);	/* to unregister */
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
		linphone_proxy_send_unregister(cfg);
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

/* build inital subscribes message */
void rcm_linphone_core_send_initial_subscribes(LinphoneCore *lc , LinphoneProxyConfig *cfg)
{
//	LinphoneProxyConfig * cfg=NULL ;
	osip_message_t *subscribes_msg;
	gchar * from = NULL ;
	osip_list_t * routes = NULL ;
	char *proxy= NULL;
	//printf("Send SUBSCRIBE to SIP server \n");
	//linphone_core_get_default_proxy(lc,&cfg);

	if( cfg != NULL )
	{
		//routes = linphone_proxy_config_get_routes( cfg ) ;
		from = osip_strdup(linphone_proxy_config_get_identity(cfg));
		proxy=osip_strdup(linphone_proxy_config_get_addr(cfg));
	}else{

		//printf("proxy not configure , no subscribe should send \n");
		return;
	}

//	osip_message_set_accept(subscribes_msg,"application/simple-message-summary");

	eXosip_lock();
	/*from == to */

	eXosip_subscribe_build_initial_request(&subscribes_msg,from,from,proxy,SUBSCIRBE_EVENT_CONTENT,600);

	osip_message_set_outboundproxy(	subscribes_msg,cfg->outbound_proxy,cfg->outbound_port);

	_osip_list_set_empty(&subscribes_msg->contacts,(void (*)(void*))osip_contact_free);
	osip_message_set_contact(subscribes_msg,cfg->contact);

	lc->sid=eXosip_subscribe_send_initial_request(subscribes_msg);
	eXosip_unlock();

    if(from)	osip_free(from);	//Alex, Valgrind, 20111213
}

/* build inital options message */
void rcm_linphone_core_send_options(LinphoneCore *lc,LinphoneProxyConfig * cfg)
{
//	LinphoneProxyConfig * cfg=NULL ;
	osip_message_t *options_msg;
	gchar * from = NULL ;
	osip_list_t * routes = NULL ;
	char *proxy= NULL;
	
	//printf("Send OPTIONS to SIP server \n");
	///linphone_core_get_default_proxy(lc,&cfg);

	if( cfg != NULL )
	{
//		routes = linphone_proxy_config_get_routes( cfg ) ;
		from = osip_strdup(linphone_proxy_config_get_identity(cfg));
//		from=osip_strdup(linphone_core_get_primary_contact(lc));
		proxy=osip_strdup(linphone_proxy_config_get_addr(cfg));
	}else{

		//printf("proxy not configure , no subscribe should send \n");
		return;
	}


	eXosip_options_build_request (&options_msg, from,from,proxy);
	osip_message_set_outboundproxy( options_msg,cfg->outbound_proxy,cfg->outbound_port);
	osip_message_set_header(options_msg, "Session-expires", "200");
	osip_message_set_supported(options_msg, "timer");
			
	eXosip_lock();
	lc->options_tid=eXosip_options_send_request(options_msg);
	eXosip_unlock();

    if(from)	osip_free(from);	//Alex, Valgrind, 20111213
}


void rcm_linphone_message_to_proxy(LinphoneCore *lc,LinphoneProxyConfig * cfg)
{
	voipCfgPortParam_t *voip_ptr = &g_pVoIPCfg->ports[lc->chid];
	voipCfgProxy_t *proxy_cfg;
	
//	sip SUBSCRIBE use proxy0 for check

	proxy_cfg = &voip_ptr->proxies[cfg->index];
	//printf("\r\n rcm_linphone_message_to_proxy, proxy_cfg->enable is %d\n",proxy_cfg->enable);
	if (proxy_cfg->enable & PROXY_SUBSCRIBE){
		if(lc->initial_subscribes_sent==FALSE){
			rcm_linphone_core_send_initial_subscribes(lc,cfg);
			lc->initial_subscribes_sent=TRUE;
		}
	}

	/* if enable sip redundancy */
	/* type 1, send option to registered proxy for keep live and ping */

	
	if(g_pVoIPCfg->rfc_flags & SIP_SERVER_REDUNDANCY){
		if(proxy_cfg->enable & PROXY_OPTIONS_PING || g_pVoIPCfg->rfc_flags & SIP_OPTION){
			lc->OptionsPingStatus=Option_WaitACK;
			rcm_linphone_core_send_options(lc,cfg);		
			rcm_timerLaunch(lc->options_ping_timer,proxy_cfg->HeartbeatCount*4000);
		}

	}else{

		/* normal send option to proxy server for keep live */
		if(proxy_cfg->enable & PROXY_OPTIONS_PING || g_pVoIPCfg->rfc_flags & SIP_OPTION){
			lc->OptionsPingStatus=Option_WaitACK;
			rcm_linphone_core_send_options(lc,cfg); 	
			rcm_timerLaunch(lc->options_ping_timer,proxy_cfg->HeartbeatCount*4000);			
		}
	}
}


/* build onhold re-invite */

int rcm_linphone_eXosip_on_hold(LinphoneCore *lc, guint32 ssid)
{

	eXosip_dialog_t *jd = NULL;
	eXosip_call_t *jc = NULL;
	//sdp_message_t *sdp_msg=NULL;
	//char *sdp_body_str=NULL;

	LinphoneCall *call;
	uint32 chid;
	int err;
	//int i;
	osip_message_t *reinvite=NULL;
	chid = lc->chid;
	call = lc->call[ssid];
	sdp_connection_t *conn;

#ifdef CONFIG_RTK_VOIP_SIP_TLS
	osip_uri_t *uri=NULL;
	char *host;
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
	voipCfgProxy_t *proxy_cfg = &g_pVoIPCfg->ports[lc->chid].proxies[lc->default_proxy_index];
	char szbuf[LINE_MAX_LEN];



	//call->did => diallog id
	if (call->did > 0)
		eXosip_call_dialog_find(call->did, &jc, &jd);

	if (jd == NULL || jd->d_dialog==NULL)
	{
		g_warning("on_update_call: No call here? jid = %d\n", call->did );
		return -1;
	}


	if(call->sdpctx->offer!=NULL)
		sdp_message_free(call->sdpctx->offer);

	
	//sdp_msg=eXosip_get_local_sdp(call->did);
	//call->sdpctx->offer=eXosip_get_local_sdp(call->did);
	call->sdpctx->offer=eXosip_get_local_sdp_from_tid(call->tid);

	if(call->sdpctx->offer==NULL){
		g_warning("\r\n rcm_linphone_eXosip_on_hold sdp null by tid \n");
		call->sdpctx->offer=eXosip_get_local_sdp(call->did);
		if(call->sdpctx->offer==NULL){
			g_warning("\r\n rcm_linphone_eXosip_on_hold sdp null by did\n");
		}
	}

	// mix 2543 & 3264 hold
	conn = sdp_message_connection_get(call->sdpctx->offer, -1, 0);
	if (conn && conn->c_addr)
	{
		osip_free(conn->c_addr);
		conn->c_addr = osip_strdup("0.0.0.0");
	}

	err= osip_negotiation_sdp_message_put_media_mode(call->sdpctx->offer, MEDIA_SENDONLY);
	
	if( err!= 0 )
	{
		g_warning( "on_hold_call: osip_negotiation_sdp_message_put_on_hold Fail\n" ) ;
		sdp_message_free( call->sdpctx->offer ) ;
		return -2 ;
	}

	// implement increment version in the o= field
	sprintf(call->sdpctx->offer->o_sess_version, "%.10d", call->sdpctx->version);

	eXosip_lock();
	err=eXosip_call_build_request(call->did,"INVITE",&reinvite);
	if((err != 0) || (reinvite==NULL)){
		eXosip_unlock();
		g_warning("\r\nrcm_eXosip_on_update_call  build re-invite fail ,err is %d \n",err);
		return err;
	}
	eXosip_unlock();
	
	osip_message_set_allow(reinvite, "INVITE, UPDATE, ACK, CANCEL, OPTIONS, BYE, REFER, NOTIFY, MESSAGE, SUBSCRIBE, INFO");
	if(lc->call[ssid]->proxy!=NULL){
	_osip_list_set_empty(&reinvite->contacts,(void (*)(void*))osip_contact_free);
	osip_message_set_contact(reinvite,lc->call[ssid]->proxy->contact);
	}

	/* invite option */
	//osip_message_set_header(reinvite, "Session-expires", "180");
	//osip_message_set_supported(reinvite, "timer");

	
	if(proxy_cfg->SessionUpdateTimer!=0){
		sprintf(szbuf, "%d", proxy_cfg->SessionUpdateTimer);
		osip_message_set_header(reinvite, "Session-expires", szbuf);
	osip_message_set_supported(reinvite, "timer");
	}
		


	rcm_set_sdp(reinvite,call->sdpctx->offer);

	eXosip_lock();
	err = eXosip_call_send_request(call->did, reinvite);
	eXosip_unlock();

	return err;
}


int rcm_linphone_eXosip_off_hold(LinphoneCore *lc, guint32 ssid)
{
	eXosip_dialog_t *jd = NULL;
	eXosip_call_t *jc = NULL;
	//sdp_message_t *sdp_msg=NULL;
	//char *sdp_body_str=NULL;

	LinphoneCall *call;
	uint32 chid;
	int err;
	//int i;
	osip_message_t *reinvite=NULL;
	chid = lc->chid;
	call = lc->call[ssid];
	sdp_connection_t *conn;

#ifdef CONFIG_RTK_VOIP_SIP_TLS
	osip_uri_t *uri=NULL;
	char *host;
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
	voipCfgProxy_t *proxy_cfg = &g_pVoIPCfg->ports[lc->chid].proxies[lc->default_proxy_index];
	char szbuf[LINE_MAX_LEN];
	


	//call->did => diallog id
	if (call->did > 0)
		eXosip_call_dialog_find(call->did, &jc, &jd);

	if (jd == NULL || jd->d_dialog==NULL)
	{
		g_warning("on_update_call: No call here? jid = %d\n", call->did );
		return -1;
	}


	if(call->sdpctx->offer!=NULL)
		sdp_message_free(call->sdpctx->offer);

	
	//sdp_msg=eXosip_get_local_sdp(call->did);
	//call->sdpctx->offer=eXosip_get_local_sdp(call->did);
	call->sdpctx->offer=eXosip_get_local_sdp_from_tid(call->tid);


	if(call->sdpctx->offer==NULL){
		g_warning("\r\n rcm_linphone_eXosip_on_hold sdp null ,by tid\n");
		call->sdpctx->offer=eXosip_get_local_sdp(call->did);
		if(call->sdpctx->offer==NULL){
			g_warning("\r\n rcm_linphone_eXosip_on_hold sdp null ,by did\n");
		}
	}

	// mix 2543 & 3264 hold
	conn = sdp_message_connection_get(call->sdpctx->offer, -1, 0);
	if (conn && conn->c_addr)
	{
		osip_free(conn->c_addr);
		conn->c_addr = osip_strdup(call->sdpctx->offer->o_addr);
	}

	err= osip_negotiation_sdp_message_put_media_mode(call->sdpctx->offer, MEDIA_SENDRECV);
	if( err!= 0 )
	{
		g_warning( "on_hold_call: osip_negotiation_sdp_message_put_on_hold Fail\n" ) ;
		sdp_message_free( call->sdpctx->offer ) ;
		return -2 ;
	}

	// implement increment version in the o= field
	sprintf(call->sdpctx->offer->o_sess_version, "%.10d", call->sdpctx->version);

/* eric todo:need check SIP TLS*/

	/* if ip, port change */
	if (lc->net_conf.nat_address!=NULL && lc->net_conf.use_nat) {
		/* modify the connection address of host */
		sdp_connection_t *conn;
		sdp_media_t *med;
		int pos_media = 0;
		sdp_message_t *offersdp_msg=call->sdpctx->offer;
		conn = sdp_message_connection_get(offersdp_msg, -1, 0);
		if (conn!=NULL && conn->c_addr!=NULL)
		{
			osip_free(conn->c_addr);
			conn->c_addr = osip_strdup(lc->net_conf.nat_address);
	  	}
		med = (sdp_media_t *) osip_list_get (&offersdp_msg->m_medias, pos_media);
		while (med != NULL)
		{
			if (med->m_media!=NULL && 0==osip_strcasecmp(med->m_media, "audio"))
			{
				osip_free(med->m_port);
				med->m_port=(char *)osip_malloc(15);
				snprintf(med->m_port, 14, "%i", lc->net_conf.nat_voice_port[ssid]);
				break;
			}
			pos_media++;
			med = (sdp_media_t *) osip_list_get (&offersdp_msg->m_medias, pos_media);
		}
	}

	eXosip_lock();
	if(eXosip_call_build_request(call->did,"INVITE",&reinvite) != 0 || reinvite==NULL){
		eXosip_unlock();
		g_warning("\r\nrcm_eXosip_on_update_call  build re-invite fail \n");
	}
	eXosip_unlock();
	
	osip_message_set_allow(reinvite, "INVITE, UPDATE, ACK, CANCEL, OPTIONS, BYE, REFER, NOTIFY, MESSAGE, SUBSCRIBE, INFO");
	if(lc->call[ssid]->proxy!=NULL){
	_osip_list_set_empty(&reinvite->contacts,(void (*)(void*))osip_contact_free);
	osip_message_set_contact(reinvite,lc->call[ssid]->proxy->contact);
	}

	/* invite option */
//	osip_message_set_header(reinvite, "Session-expires", "180");
	//osip_message_set_supported(reinvite, "timer");


/* invite option */
	if(proxy_cfg->SessionUpdateTimer!=0){
		sprintf(szbuf, "%d", proxy_cfg->SessionUpdateTimer);
		osip_message_set_header(reinvite, "Session-expires", szbuf);
		osip_message_set_header(reinvite, "Min-SE", "90");
		osip_message_set_supported(reinvite, "timer");
	}

	//sdp_message_to_str(sdp_msg,&sdp_body_str);
	//sdp_message_free(sdp_msg);	

	//osip_message_set_body(reinvite,sdp_body_str,strlen(sdp_body_str));
	//osip_message_set_content_type(reinvite,"application/sdp");
	//osip_message_set_content_length(sip,clen);
	//osip_free(sdp);

	rcm_set_sdp(reinvite,call->sdpctx->offer);

	eXosip_lock();
	err = eXosip_call_send_request(call->did, reinvite);
	eXosip_unlock();

	return err;
}



int rcm_linphone_eXosip_on_fax(LinphoneCore *lc, guint32 ssid,int port, FAX_MODEM_STATE fms_state)
{

	eXosip_dialog_t *jd = NULL;
	eXosip_call_t *jc = NULL;
	sdp_message_t *sdp_msg=NULL;
	char *sdp_body_str=NULL;
	LinphoneProxyConfig *proxy=NULL;
	LinphoneCall *call;
	uint32 chid;
	int err;
	int i;
	osip_message_t *reinvite=NULL;
	chid = lc->chid;
	call = lc->call[ssid];
	sdp_connection_t *conn;

#ifdef CONFIG_RTK_VOIP_SIP_TLS
	osip_uri_t *uri=NULL;
	char *host;
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
	voipCfgProxy_t *proxy_cfg = &g_pVoIPCfg->ports[lc->chid].proxies[lc->default_proxy_index];
	char szbuf[LINE_MAX_LEN];
	linphone_core_get_default_proxy( lc , &proxy ) ;	

	//call->did => diallog id
	if (call->did > 0)
		eXosip_call_dialog_find(call->did, &jc, &jd);

	if (jd == NULL || jd->d_dialog==NULL)
	{
		g_warning("on_update_call: No call here? jid = %d\n", call->did );
		return -1;
	}




	if(fms_state!=FMS_IDLE)
	{
		if(call->sdpctx->offer!=NULL)
			sdp_message_free(call->sdpctx->offer);

	
		//sdp_msg=eXosip_get_local_sdp(call->did);
		//call->sdpctx->offer=eXosip_get_local_sdp(call->did);
		call->sdpctx->offer=eXosip_get_local_sdp_from_tid(call->tid);


		if(call->sdpctx->offer==NULL){
			printf("\r\n rcm_linphone_eXosip_on_fax sdp null \n");
		}
	
		i = osip_negotiation_sdp_message_put_on_fax( call->sdpctx->offer, port , FAX_G711_CODEC) ;
		if( i != 0 )
		{
			g_warning( "rcm_linphone_eXosip_on_fax: osip_negotiation_sdp_message_put_on_fax Fail\n" ) ;
			sdp_message_free( call->sdpctx->offer ) ;
			return -2 ;
		}
	}
	else
	{
		sdp_context_t *ctx;
		gchar * sdpmesg = NULL ;
		ctx = lc->call[ssid]->sdpctx;
		if(ctx->offer!=NULL)
			sdp_message_free(call->sdpctx->offer);
		sdpmesg = sdp_context_get_offer( ctx ) ;
	}


	// implement increment version in the o= field
	sprintf(call->sdpctx->offer->o_sess_version, "%.10d", call->sdpctx->version);

	eXosip_lock();
	if(eXosip_call_build_request(call->did,"INVITE",&reinvite) != 0 || reinvite==NULL){
		eXosip_unlock();
		printf("\r\nrcm_linphone_eXosip_on_fax  build re-invite fail \n");
	}
	eXosip_unlock();
	
	osip_message_set_outboundproxy(	reinvite,proxy->outbound_proxy,proxy->outbound_port);
	
	osip_message_set_allow(reinvite, "INVITE, UPDATE, ACK, CANCEL, OPTIONS, BYE, REFER, NOTIFY, MESSAGE, SUBSCRIBE, INFO");
	if(lc->call[ssid]->proxy!=NULL){
	_osip_list_set_empty(&reinvite->contacts,(void (*)(void*))osip_contact_free);
	osip_message_set_contact(reinvite,lc->call[ssid]->proxy->contact);
	}

	/* invite option */
	//osip_message_set_header(reinvite, "Session-expires", "180");
//	osip_message_set_supported(reinvite, "timer");


	if(proxy_cfg->SessionUpdateTimer!=0){
		sprintf(szbuf, "%d", proxy_cfg->SessionUpdateTimer);
		osip_message_set_header(reinvite, "Session-expires", szbuf);
	osip_message_set_supported(reinvite, "timer");
	}


	rcm_set_sdp(reinvite,call->sdpctx->offer);

	eXosip_lock();
	err = eXosip_call_send_request(call->did, reinvite);
	eXosip_unlock();

	return err;


}


int rcm_linphone_eXosip_mcid_reinvite(LinphoneCore *lc, guint32 ssid)
{

	eXosip_dialog_t *jd = NULL;
	eXosip_call_t *jc = NULL;
	//sdp_message_t *sdp_msg=NULL;
	//char *sdp_body_str=NULL;

	LinphoneCall *call;
	uint32 chid;
	int err;
	//int i;
	osip_message_t *reinvite=NULL;
	chid = lc->chid;
	call = lc->call[ssid];
	//sdp_connection_t *conn;
	char clen[10];

	char mcid_xml_body[] =
		"<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
		 "<mcid>"
		 "	<request>"
		 "	  <MicdRequestIndicator>1</MicdRequestIndicator>"
		 "		<HoldingIndicator/>"
		 "	  </request>"
		 "	</mcid>";

	

#ifdef CONFIG_RTK_VOIP_SIP_TLS
	osip_uri_t *uri=NULL;
	char *host;
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/


	//call->did => diallog id
	if (call->did > 0)
		eXosip_call_dialog_find(call->did, &jc, &jd);

	if (jd == NULL || jd->d_dialog==NULL)
	{
		g_warning("on_update_call: No call here? jid = %d\n", call->did );
		return -1;
	}

	g_message("\r\n eXosip_call_build_request micd reinvite\n");
	eXosip_lock();
	if(eXosip_call_build_request(call->did,"INVITE",&reinvite) != 0 || reinvite==NULL){
		eXosip_unlock();
		g_message("\r\nrcm_eXosip_on_update_call  build re-invite fail \n");
	}
	eXosip_unlock();

	osip_message_set_allow(reinvite, "INVITE, UPDATE, ACK, CANCEL, OPTIONS, BYE, REFER, NOTIFY, MESSAGE, SUBSCRIBE, INFO");
	if(lc->call[ssid]->proxy!=NULL){
		_osip_list_set_empty(&reinvite->contacts,(void (*)(void*))osip_contact_free);
		osip_message_set_contact(reinvite,lc->call[ssid]->proxy->contact);
	}


	osip_message_set_content_type(reinvite,"application/vnd.etsi.mcid+xml");

	osip_message_set_body(reinvite,mcid_xml_body,strlen(mcid_xml_body));
	snprintf(clen,sizeof(clen),"%lu",(unsigned long)strlen(mcid_xml_body));
	osip_message_set_content_length(reinvite,clen);

	eXosip_lock();
	err = eXosip_call_send_request(call->did, reinvite);
	eXosip_unlock();
	return err;

}

int linphone_core_onhood_event(LinphoneCore *lc, guint32 ssid)
{
	LinphoneCall *call;
	uint32 chid;
	int err;

	chid = lc->chid;
	call = lc->call[ssid];
	g_message("linphone_core_onhood_event ssid %d\n",ssid);
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
//				rcm_tapi_SetPlayTone(chid, ssid == 0 ? 1 : 0, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
			// send hold
			//call->state = LCStateHold; // start hold
			SetCallState(chid,ssid,&call->state,LCStateHold);
			g_message("\r\nlinphone-state chid[%d],ssid[%d], status %d\n",chid,ssid,call->state );
			//eXosip_lock();
			call->sdpctx->version++;
			err=rcm_linphone_eXosip_on_hold(lc,ssid);
			if (err != 0) return -1;
			break;
		case LCStateHold: // not ACK, hold again
			call->sdpctx->version++;
			err=rcm_linphone_eXosip_on_hold(lc,ssid);
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
				//call->state = LCStateAVRunning; // start hold
				SetCallState(chid,ssid,&call->state,LCStateAVRunning);
				
				g_message("\r\nlinphone-state linphone_core_onhood_event chid[%d],ssid[ssid], status %d\n",chid,ssid,call->state );
				if (GetSessionState(chid, ssid == 0 ? 1 : 0) == SS_STATE_IDLE)
					rcm_tapi_SetPlayTone(chid, ssid == 0 ? 1 : 0, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
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
				rcm_tapi_SetPlayTone(chid, ssid == 0 ? 1 : 0, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
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
	g_message("linphone_core_offhood_event ssid %d\n",ssid);
	if (call != NULL)
	{
		switch (call->state)
		{
		case LCStateHold:
			// waiting hold ack
			return -1;
		case LCStateHoldACK:
		case LCStateBeResume: //to be unheld. by eric
		//add eric for netcologn sip server
		case LCStateBeHoldACK:
//			SetActiveSession(chid, ssid, TRUE);
//			rcm_tapi_SetPlayTone(chid, ssid, DSPCODEC_TONE_HOLD, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
			// send resume
			//call->state = LCStateResume;
			SetCallState(chid,ssid,&call->state,LCStateResume);
			/* Mandy: Use new RTP port in INVITE message when we resume the call. */
			linphone_core_set_use_info_for_stun(lc, ssid, STUN_UPDATE_RTP);
			call->sdpctx->version++;
			err=rcm_linphone_eXosip_off_hold(lc,ssid);
			if (err != 0) return -1;
			break;
		case LCStateResume: // not ACK, send again
			g_message("......LCStateResume.........\n");
			/* Mandy: Use new RTP port in INVITE message when we resume the call. */
			linphone_core_set_use_info_for_stun(lc, ssid, STUN_UPDATE_RTP);
			call->sdpctx->version++;
			/* Mandy: Use new RTP port in INVITE message when we resume the call. */
			err=rcm_linphone_eXosip_off_hold(lc,ssid);

			return -1; 	// still wait ACK
		case LCStateBeHold:
	//	case LCStateBeHoldACK:
			SetActiveSession(chid , ssid, TRUE);
			rcm_tapi_SetPlayTone(chid , ssid, DSPCODEC_TONE_HOLD, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
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
	g_message("linphone_call_fax_reinvite\n");
	if( call != NULL && call->state == LCStateAVRunning)
	{
		/*++ T38 add by Jack Chan 24/01/07++ */
		int T38_port = 0;
		if( TRUE == lc->sip_conf.T38_enable && FMS_FAX_RUN == call->faxflag ){
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
		//[SD6, ericchung, exosip 3.5 integration
		//call->state = LCStateFax ;
		SetCallState(lc->chid,ssid,&call->state,LCStateFax);
		call->sdpctx->version++;
		rcm_linphone_eXosip_on_fax(lc,ssid,T38_port,call->faxflag);
	}
	else
	{
		g_warning("\nfax on state failed\n");
	}
}

void rcm_linphone_call_off_fax_reinvite(LinphoneCore *lc, guint32 ssid)
{
	LinphoneCall *call;
	
	call = lc->call[ssid];
	g_message("linphone_call_off_fax_reinvite\n");
	if( call != NULL && call->state == LCStateAVRunning)
	{
		call->t38_params.initialized=0;
#if 1
//		linphone_core_stop_media_streams(lc, ssid); 
#else
		sleep(1) ;
#endif
		//[SD6, ericchung, exosip 3.5 integration
//		call->state = LCStateFax ;
		SetCallState(lc->chid,ssid,&call->state,LCStateFax);
		call->sdpctx->version++;
		rcm_linphone_eXosip_on_fax(lc,ssid,0,call->faxflag);
	}
	else
	{
		g_warning("\nfax off state failed\n");
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

		if (&jd->d_dialog->remote_contact_uri->url &&
			&jd->d_dialog->remote_contact_uri->url.host)
		{
			if (jd->d_dialog->remote_contact_uri->url.username[0]!='\0')
				sprintf(contact_url, "sip:%s@%s",osip_uri_get_username(&jd->d_dialog->remote_contact_uri->url),
					osip_uri_get_host(&jd->d_dialog->remote_contact_uri->url));
			else
				sprintf(contact_url, "sip:%s", osip_uri_get_host(&jd->d_dialog->remote_contact_uri->url));
		}
		else
		{
			return -1;
		}

		if (osip_uri_get_port(&jd->d_dialog->remote_contact_uri->url))
		{
			sprintf(port,":%d",osip_uri_get_port(&jd->d_dialog->remote_contact_uri->url));
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
		//eXosip_lock() ;//not need
		//[SD6, bohungwu, exosip 3.5 integration
		//eXosip_transfer_call(call->did, refer);
		rcm_eXosip_transfer_call(call->did, refer);
		//]
		//eXosip_unlock() ;
	}
}

void linphone_core_forward_dialog(LinphoneCore *lc, guint32 ssid, const char *forward)
{
	LinphoneCall *call;
	osip_message_t *msg;

	call = lc->call[ssid];
	if (call != NULL)
	{
		eXosip_lock();
		//[SD6, bohungwu, exosip 3.5 integration
		//c_redirections no longer exists in eXosip_call_t in 3.5
		//eXosip_set_redirection_address(call->did, forward);
		//eXosip_answer_call(call->did, 302, NULL);
		

		eXosip_call_build_answer(lc->call[ssid]->tid,302,&msg);
		osip_message_set_contact(msg,forward);
		eXosip_call_send_answer(lc->call[ssid]->tid,302,msg);
		eXosip_unlock();
		//eXosip_set_redirection_address(call->did, NULL);
		//]

	}
}


int rcm_linphone_dial_local(LinphoneCore *lc, guint32 ssid, int remote_chid,const char *url){
	gint err = 0 ;
	gchar * sdpmesg = NULL ;
	osip_list_t * routes = NULL ;
	gchar * from = NULL ;
	osip_message_t * invite = NULL ;
	sdp_context_t * ctx = NULL ;
	LinphoneProxyConfig * proxy = NULL ;
	osip_from_t * parsed_url2 = NULL ;
	osip_to_t * real_parsed_url = NULL ;
	char * to_url = NULL ;
	char szbuf[LINE_MAX_LEN];
	char to_buf[LINE_MAX_LEN];
	char local_ip[MAXHOSTNAMELEN];
	voipCfgProxy_t *proxy_cfg = &g_pVoIPCfg->ports[lc->chid].proxies[lc->default_proxy_index];

	g_message("rcm_linphone_dial_local , remote chid is %d\n",remote_chid);

	if(strlen(solar_ptr->bound_voip_ipaddress)!=0){
			strcpy(local_ip,solar_ptr->bound_voip_ipaddress);
	}else{
		strcpy(local_ip,"127.0.0.1");		
	}
	/* check source chid is fxs or fxo */
	if( RTK_VOIP_IS_DAA_CH( lc->chid, g_VoIP_Feature ) ){
		//fxo make call...
		/* check fxo incoming caller id */
		if (lc->caller_id[0])
		{
			if (lc->caller_id_name[0])
				sprintf(szbuf, "\"%s\" <sip:%s@%s>", lc->caller_id_name, lc->caller_id,solar_ptr->bound_voip_ipaddress);
			else
				sprintf(szbuf, "sip:%s@%s", lc->caller_id,local_ip);
			
		}
		else
		{
			//currently only one fxo,
			//sprintf(szbuf,"sip:fxs%d@%s",lc->chid+1,solar_ptr->bound_voip_ipaddress);
			sprintf(szbuf,"sip:fxo@%s",local_ip);
			
		}

		g_message("fxo: from_url is %s\n",szbuf);

	}else{

		//fxs make call
		g_message("lc->chid is %d\n",lc->chid);
		if (proxy_cfg->number[0]){
			sprintf(szbuf,"sip:%s@%s",proxy_cfg->number,local_ip);
		}else{
			sprintf(szbuf,"sip:fxs%d@%s",lc->chid+1,local_ip);
		}
		g_message("url is %s\n",szbuf);
	
	}


	//check destination chid 
	if( RTK_VOIP_IS_DAA_CH( remote_chid, g_VoIP_Feature ) ){
		/* dial to fxo */
		if (strcmp(g_pVoIPCfg->funckey_pstn,url ) == 0){ // dial to pstn funckey_pstn: default is *0
			sprintf(to_buf, "sip:fxo@%s", local_ip);
			g_message("\r\n to_url	is %s\n",to_buf);
		}else{
			sprintf(to_buf, "%s", url);
			g_message("\r\n to_url	is %s\n",to_buf);
		}
		/* modify from */
		sprintf(szbuf,"sip:fxstofxo@%s",local_ip);
		g_message("from url is %s\n",szbuf);
		

	}else{	
		sprintf(to_buf, "sip:fxs%d@%s", remote_chid+1, local_ip);
		g_message("\r\n to_url  is %s\n",to_buf);	
	}
	
	from = osip_strdup(szbuf);
	to_url = osip_strdup(to_buf);

	err = eXosip_call_build_initial_invite(&invite, (gchar *) to_url, (gchar *) from, routes, "Phone call");
	if( err < 0 )
	{
		g_warning( "Could not build initial invite\n" ) ;
		goto end ;
	}

	
	/* from */
	osip_from_init( &parsed_url2 ) ;
	osip_from_parse( parsed_url2 , from ) ;
	/* to */
	osip_from_init( &real_parsed_url ) ;
	osip_from_parse( real_parsed_url , to_url ) ;
	

	/* make sdp message */


	lc->call[ssid] = linphone_call_new_outgoing(lc, parsed_url2, real_parsed_url);
	ctx = lc->call[ssid]->sdpctx;
	sdpmesg = sdp_context_get_offer( ctx ) ;
	
	/* invite option */
	if(!(g_pVoIPCfg->rfc_flags & SIP_DISABLE_PRACK)){
	osip_message_set_supported(invite, "100rel");
	}
	
	osip_message_set_allow(invite, "INVITE, UPDATE, ACK, CANCEL, OPTIONS, BYE, REFER, NOTIFY, MESSAGE, SUBSCRIBE, INFO");

	osip_message_set_body(invite, sdpmesg, strlen(sdpmesg));
	osip_message_set_content_type(invite, "application/sdp");
	rcm_osip_message_set_content_length(invite, strlen(sdpmesg));

	eXosip_lock() ;
	
	err = eXosip_call_send_initial_invite(invite);

/* eric for test change sequence if() movoe up !! */
	if( err < 0 )
	{
		g_warning( "Could not initiate call.\n" ) ;
		linphone_call_destroy(lc->call[ssid]);
		lc->call[ssid] = NULL;
		eXosip_unlock();
		goto end ;
	}
	lc->call[ssid]->cid = err;
	eXosip_unlock() ;

end :
	if (from != NULL)
		osip_free(from);
    if (to_url != NULL)
        osip_free(to_url);	//Alex, 20111111, valgrind error


	if( real_parsed_url != NULL )
		osip_to_free( real_parsed_url ) ;
	if( parsed_url2 != NULL )
		osip_from_free( parsed_url2 ) ;

	return err ;


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

#ifdef RCM_VOIP_INTERFACE_ROUTE

#define VOIP_ITF_NONE     ""
#define VOIP_ITF_LAN      "br0"

#define ROUTE             "/bin/route"

#define ARG_ADD           "add"
#define ARG_DEL           "del"

#define ARG_NET           "-net"
#define ARG_HOST          "-host"

#define ARG_NETMASK       "netmask"

#define ARG_MASK_HOST     "255.255.255.255"
#define ARG_MASK_CLASS_C  "255.255.255.0"

#define ARG_DEV           "dev"

//return 0:OK, other:fail
int do_cmd(const char *filename, char *argv [], int dowait)
{
	pid_t pid, wpid;
	int stat=0, st;
	sigset_t  tmpset, origset;

	sigfillset(&tmpset);
	sigprocmask(SIG_BLOCK,&tmpset,&origset);
	pid = vfork();
	sigprocmask(SIG_SETMASK,&origset,0);
	if(pid == 0) {
		/* the child */
		char *env[3];

		signal(SIGINT, SIG_IGN);
		argv[0] = (char *)filename;
		env[0] = "PATH=/bin:/usr/bin:/etc:/sbin:/usr/sbin";
		env[1] = NULL;

		execve(filename, argv, env);

		printf("exec %s failed\n", filename);
		_exit(2);
	} else if(pid > 0) {
		if (!dowait)
			stat = 0;
		else {
			/* parent, wait till rc process dies before spawning */
			while ((wpid = wait(&st)) != pid)
			{
				if (wpid == -1 && errno == EINTR)
				{
				     wpid=waitpid(pid, &st, WNOHANG);
				}
				if (wpid == -1 && errno == ECHILD) { /* see wait(2) manpage */
					stat = 0;
					break;
				}
			}
		}
	} else if(pid < 0) {
		printf("fork of %s failed\n", filename);
		stat = -1;
	}
	st = WEXITSTATUS(stat);
	return st;
}

//return 0:OK, other:fail
int va_cmd(const char *cmd, int num, int dowait, ...)
{
    va_list ap;
    int k;
    char *s;
    char *argv[24];
    int status;

    va_start(ap, dowait);

    for (k=0; k<num; k++)
    {
        s = va_arg(ap, char *);
        argv[k+1] = s;
    }
    
    argv[k+1] = NULL;
    status = do_cmd(cmd, argv, dowait);
    va_end(ap);

    return status;
}


int  rcm_add_voip_interface_route(LinphoneCore *lc, guint32 ssid)
{
	rtp_config_t rtp_config;
	rtcp_config_t rtcp_config;
	t38udp_config_t t38udp_config;
	payloadtype_config_t codec_config;
	t38_payloadtype_config_t t38_config;

	char remoteip[16];
	struct in_addr ip_addr;

// check if voip_interface is valid
	if( !strcmp(solar_ptr->current_voip_interface, VOIP_ITF_NONE) || 
		!strcmp(solar_ptr->current_voip_interface, VOIP_ITF_LAN) ){
		printf("no voip interface setting. it use %s.\n" , solar_ptr->current_voip_interface);
		//return FALSE;   //remove it for init test
	}

// set routing for voip_interface
	//TstVoipMgrRtpCfg rtp_cfg;
	//GetRtpCfg(&rtp_cfg, lc, ssid);

	if (GetRtpCfgFromSession(&rtp_config, &rtcp_config, &t38udp_config, 
							&codec_config, &t38_config, lc, ssid) != 0)
	{
		g_warning("%s: GetRtpCfgFromSession failed\n", __FUNCTION__);
		return FALSE;
	}

	memcpy(&ip_addr , &rtp_config.remIp , sizeof(struct in_addr));
	strncpy(remoteip, inet_ntoa(ip_addr) , 16);
	
#if 1 //Debug message
	printf("ssid=%d\n" , ssid);
	
	printf("rtp chid=%d rtp sid=%d rtp remIp=%08x rtp remPort=%d rtp extIp=%08x rtp extPort=%d" ,  
			rtp_config.chid , rtp_config.sid , rtp_config.remIp , rtp_config.remPort , 
			rtp_config.extIp , rtp_config.extPort);

	printf("voip_interface %s\n",solar_ptr->current_voip_interface);
    
	printf("%s %s\n" , remoteip , inet_ntoa(ip_addr)); 
	
	printf("%s %s %s %s %s %s %s\n",
           ARG_ADD , 
           ARG_NET , 
           remoteip, 
           ARG_NETMASK  , 
           ARG_MASK_HOST , 
           ARG_DEV , 
           solar_ptr->current_voip_interface);
#endif

	va_cmd(ROUTE , 
           7 , 
           1 , 
           ARG_ADD , 
           ARG_NET , 
           remoteip, 
           ARG_NETMASK  , 
           ARG_MASK_HOST , 
           ARG_DEV , 
           solar_ptr->current_voip_interface);
	

	return TRUE;
}

int  rcm_del_voip_interface_route(LinphoneCore *lc, guint32 ssid)
{
    rtp_config_t rtp_config;
    rtcp_config_t rtcp_config;
    t38udp_config_t t38udp_config;
    payloadtype_config_t codec_config;
    t38_payloadtype_config_t t38_config;

	extern TAPP solar;
	TAPP *tApp = &solar;
	LinphoneCore *tlc;

    char remoteip[16];
    struct in_addr ip_addr;
	
	int i = 0 , j = 0;

	for(i=0 ; i<g_MaxVoIPPorts; i++){
		tlc = &tApp->ports[i];
		printf("check chid = %d current chid = %d\n", tlc->chid , lc->chid);

		if(lc->chid != tlc->chid){
	        for(j=0 ; j<MAX_SS ; j++){
				if(tlc->audiostream[j]==TRUE){
			        printf("channel %d session %d still in use\n", tlc->chid , j);	
					return FALSE;
				}
			}
		}
	}

    if (GetRtpCfgFromSession(&rtp_config, &rtcp_config, &t38udp_config,
                            &codec_config, &t38_config, lc, ssid) != 0)
    {
        g_warning("%s: GetRtpCfgFromSession failed\n", __FUNCTION__);
        return FALSE;
    }

	memcpy(&ip_addr , &rtp_config.remIp , sizeof(struct in_addr));
	strncpy(remoteip, inet_ntoa(ip_addr) , 16);

#if 1	//Debug message
    printf("%s %s\n" , remoteip , inet_ntoa(ip_addr)); 

	printf("%s %s %s %s %s %s %s\n",
           ARG_DEL , 
           ARG_NET , 
           remoteip, 
           ARG_NETMASK  , 
           ARG_MASK_HOST , 
           ARG_DEV , 
           solar_ptr->current_voip_interface);
#endif
	
	va_cmd(ROUTE , 
           7 , 
           1 , 
           ARG_DEL , 
           ARG_NET , 
           remoteip, 
           ARG_NETMASK  , 
           ARG_MASK_HOST , 
           ARG_DEV , 
           solar_ptr->current_voip_interface);
	

	return TRUE;
}
#endif

