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

#ifndef LINPHONECORE_H
#define LINPHONECORE_H

//#include "voip_manager.h"
#include "rcm_tapi.h"
#ifdef CONFIG_RTK_VOIP_TR104
#include "cwmpevt.h"
#endif /*CONFIG_RTK_VOIP_TR104*/

#undef PACKAGE
#undef VERSION

#ifdef HAVE_GETTEXT
#include <libintl.h>
#ifndef _
#define _(String) gettext(String)
#endif
#else
#define _(something)	(something)
#endif

#include "uglib.h"
#include "osipparser2/osip_message.h"

       #include <sys/socket.h>
       #include <netinet/in.h>
       #include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_RTK_VOIP_TR104
#include "cwmpevt.h"
#endif /*CONFIG_RTK_VOIP_TR104*/
#ifndef PayloadType_defined
#define PayloadType_defined
typedef struct _PayloadType PayloadType;
#endif


#define _PATH_TMP_STATUS "/tmp/voipstatus"

/*add call log output file, 
fxs 0 is /tmp/call_log_p0
fxs 1 is /tmp/call_log_p1
*/
#define _PATH_CALL_LOGS	"/tmp/call_log"


gboolean payload_type_enabled(struct _PayloadType *pt);
gboolean payload_type_usable(struct _PayloadType *pt);
void payload_type_set_enable(struct _PayloadType *pt,gint value);
void payload_type_set_usable(struct _PayloadType *pt,gint value);

/* define SIP header/event contact , E8C */

//need check e8c use which event ?
//#define SUBSCIRBE_EVENT_CONTENT	"message-summary"

#define SUBSCIRBE_EVENT_CONTENT	"ua-profile;profile-type=user;"

#define OPTIONS_PING_EXPIRE_TIME 30	/*30sec*/
#define OPTIONS_PING_CYCLE_TIME	60 /*60sec*/
#define REGISTER_RETRY_TIMEOUT	90

/* The definition for the linephoneCore>fxsfeatureFlags */

#define FXS_FLEASH_KEY				(1<<0)		/* for fxs second/3way call function key */
#define FXS_ONE_SHOT_CID_HIDDEN		(1<<1)		/* for one shot caller id hidden*/
#define FXS_ONE_SHOT_CID_DISPLAY	(1<<2)		/* for one shot caller id display */
#define FXS_SPECIAL_DIAL_TONE		(1<<3)		/* CFNR/CFNB..play special dial tone*/
#define FXS_CONFERENCE_ACTIVATE		(1<<4)		/* enable conference */
#define FXS_CALL_HOLD_ACTIVATE		(1<<5)		/* enable call hold */
#define FXS_MCID_ACTIVATE			(1<<6)		/* enable call hold */
#define FXS_VOICEMAIL_TONE			(1<<7)		/* Voice mail, play VOICEMAIL tone */

#define CONNECTION_TYPE_28KMODEM 0
#define CONNECTION_TYPE_56KMODEM 1
#define CONNECTION_TYPE_NUMERIS 2
#define CONNECTION_TYPE_ADSL 3
#define CONNECTION_TYPE_ETHERNET 4

//[SD6, ericchung, exosip 3.5 integration
#define CONTACT_LENGTH_MAX 64
//]


#define MAX_DIAL_LENGTH			63	/* Maximum Dial length */

/*internal call , only support 2-FXS now , 
 DIAL **1 for fxs 1,  and **2 for fxs2 */
#define LC_FUNCKEY_FXS_0 	"..1"
#define LC_FUNCKEY_FXS_1 	"..2"


extern const double bandwidths[5];

struct _LpConfig;

#ifdef CONFIG_RTK_VOIP_SRTP
#define	SDP_KEY_LEN	40
#endif

typedef struct sip_config
{
//	gchar display_name[DNS_LEN];
//	gchar username[DNS_LEN] ;
//	gchar login_id[DNS_LEN] ;
//	gchar password[DNS_LEN] ;
//	gchar *contact;
//	gchar *guessed_contact;
	gint sip_port;
	GList *proxies;
	GList *deleted_proxies;
	gint dtmf_type ;
	guint16 rfc2833_payload_type;
#ifdef SUPPORT_V152_VBD
	guint16 v152_payload_type;	// 0: off, 96~127 are valid
	RtpPayloadType v152_codec_type;	// a-law or u-law
#endif
#ifdef SUPPORT_RTP_REDUNDANT_APP
	guint16 rtp_redundant_payload_type;	// 0: off, 96~127 are valid
	RtpPayloadType rtp_redundant_codec_type;	// a-law, u-law or G.729
#endif
//	gboolean guess_hostname;
	gboolean loopback_only;
	gboolean ipv6_enabled;
#ifndef CONFIG_RTK_VOIP_DIALPLAN
	gchar uc_forward_enable;
	gchar uc_forward[FW_LEN];
	gchar busy_forward_enable;
	gchar busy_forward[FW_LEN];
	gchar na_forward_enable;
	gchar na_forward[FW_LEN];
	gint na_forward_time;
#endif /* !CONFIG_RTK_VOIP_DIALPLAN */
	gboolean call_waiting_enable;
	/*++T.38 add by Jack Chan 24/01/07 for Voip++*/
	gboolean T38_enable;
	gint T38_prot;
	//T.38 parameters
	gboolean T38ParamEnable;
	guint16 T38MaxBuffer;
	guchar T38RateMgt;
	guchar T38MaxRate;
	gboolean T38EnableECM;
	guchar T38ECCSignal;
	guchar T38ECCData;
	gboolean T38EnableSpoof;
	guchar T38DuplicateNum;
	/*--end--*/
/* +++++Add by Jack for VoIP security 240108+++++ */
#ifdef CONFIG_RTK_VOIP_SRTP
	gboolean security_enable;
	gchar key_exchange_mode;
#endif
/*-----end-----*/
} sip_config_t;

typedef struct rtp_config
{
	guint16 audio_rtp_port;
	gint audio_jitt_comp;  /*jitter compensation*/
	guint8 g7231_rate;
	guint8 iLBC_mode;
	guint8 speex_nb_rate;
	guint8 bVAD;
	guint8 nVAD_thr;
	guint8 bCNG;
	guint8 nCNG_thr;
	guint8 nSID_mode;
	guint8 nSID_level;
	gint16 nSID_gain;
	guint8 bPLC;
	guint8 nRtcpInterval;
	guint8 bRtcpXR;
	guint32 nJitterDelay;
	guint32 nMaxDelay;
	guint32 nMaxStrictDelay;
	guint32 nJitterFactor;
	guint32 nG726Packing;
#ifdef SUPPORT_G7111
	guint8 g7111_precedence[G7111_MODES];
#endif
#ifdef SUPPORT_VOICE_QOS
	guint8 tos;
#endif
	guint8 *pFrameSize;

} user_rtp_config_t;

typedef struct net_config
{
	double bandwidth; 		/* the bandwidth in bit/s of the selected interface*/ /*unused*/
	gchar *nat_address;
	/* Mandy add for stun support */
	gint nat_port;
	gint nat_voice_port[MAX_SS];
	gint nat_t38_port;
	/* Mandy: Keep NAT entry alive */
	int udp_keepAlive_frequency;
	int natEntry_build_time;
	gboolean use_nat;
	gint con_type;			/* type of connection 28k 56k 128k adsl ...*/
}net_config_t;

typedef struct codecs_config
{
	GList *audio_codecs;  /* list of audio codecs in order of preference*/
}codecs_config_t;

typedef struct ui_config
{
	int is_daemon;
	int is_applet;
	guint timer_id;  /* the timer id for registration */
}ui_config_t;

struct _LinphoneCore;
struct _sdp_context;

typedef struct _StreamParams
{
	int initialized;
	int line;
	int localport;
	int remoteport;
//	char *remoteaddr;
	unsigned long remote_ip;
	char remote_addr[16]; //xxx.xxx.xxx.xxx
#ifdef DYNAMIC_PAYLOAD
	int local_pt;
	int remote_pt;
	int static_pt;	// static pt record codec in rtk dsp
	int rfc2833_local_pt;
	int rfc2833_remote_pt;
#else
	int pt;
#endif
#ifdef SUPPORT_V152_VBD
	int initialized_vbd;
	int local_pt_vbd;
	int remote_pt_vbd;
	int static_pt_vbd;	// static pt record codec in rtk dsp
#endif
#ifdef SUPPORT_RTP_REDUNDANT_APP
	int initialized_rtp_redundant;
	int local_pt_rtp_redundant;
	int remote_pt_rtp_redundant;
#endif
#ifdef CONFIG_RTK_VOIP_SRTP
	char remoteSDPKey[SDP_KEY_LEN];
	char localSDPKey[SDP_KEY_LEN];
	int remoteCryptAlg;
	int localCryptAlg;
#endif /*CONFIG_RTK_VOIP_SRTP*/
#ifdef SUPPORT_G7111
	guint8 g7111_precedence[G7111_MODES];
#endif
} StreamParams;

typedef enum _LCState{
	LCStateInit,
	LCStateRinging,
	LCStateAVRunning,
	LCStateFax,
	LCStateBeFax,
	LCStateHold,
	LCStateHoldACK,
	LCStateResume,
	LCStateResumeACK,
	LCStateBeHold,
	LCStateBeHoldACK,
	LCStateBeResume,
	LCStateUpdate,
	LCStateTerminated
}LCState;

#ifdef CONFIG_RTK_VOIP_SIP_TLS
typedef enum _TRANSPORT_TYPE{
	TRANSPORT_UDP,
	TRANSPORT_TCP,
	TRANSPORT_TLS,
	TRANSPORT_ERROR
}TRANSPORT_TYPE;
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

typedef enum _LinphoneCallDir {LinphoneCallOutgoing, LinphoneCallIncoming} LinphoneCallDir;

typedef enum _ProxyRegStatus {
	REGSTATUS_INIT,
	REGSTATUS_REG_ING,				/* registering */
	REGSTATUS_REG_SUCCESS,			/* register success */
	REGSTATUS_REG_SUCCESS_EXPIRED,	/* register success but expired, so it is registering */
	REGSTATUS_REG_FAIL,				/* register failure (don't retry) */
	REGSTATUS_REG_FAIL_RETRY_S,		/* register failure (short delay retry) */
	REGSTATUS_REG_FAIL_RETRY_L,		/* register failure (long delay retry) */
	REGSTATUS_RESTART				/* restart call manager */
} ProxyRegStatus;

typedef struct _LinphoneProxyConfig
{
	struct _LinphoneCore *lc;
	gchar *reg_proxy; // register proxy address
	gchar *reg_identity; //phonenumber , url , 1234@aa.bb.cc.dd
//	gchar *reg_route;
    osip_list_t *reg_routes;              /**< route set */
#if 0
	gchar *realm;
#endif
	int expires;
	int reg_time;
	int rid;
	gboolean frozen;
	gboolean reg_sendregister; //enable send register to server 
	gboolean auth_failed;	// auth failed will stop quick retry (2 sec)
	gboolean registered;
	gboolean publish;
	
	int index; // index of proxies "in flash"  ,RTK add
	ProxyRegStatus reg_status;
	osip_list_t  service_routes;
	char contact[CONTACT_LENGTH_MAX];
	char outbound_proxy[128];
	int outbound_port;
	
} LinphoneProxyConfig;

typedef struct _LinphoneCall
{
	struct _LinphoneCore *core;
	StreamParams audio_params;
	/*++added by Jack Chan 02/01/07++*/
	StreamParams t38_params;
	/**--end--*/
	LinphoneCallDir dir;
//	struct _RtpProfile *profile;	/*points to the local_profile or to the remote "guessed" profile*/
	struct _LinphoneCallLog *log;
	int cid; /*call id */
	int did; /*dialog id */
	int tid;/* transation id*/
	struct _sdp_context *sdpctx;
	time_t start_time;
	LCState	state;
	gboolean auth_failed;
	struct timeval no_answer_start;
#ifdef CONFIG_RTK_VOIP_IP_PHONE
	int rfc2833_payload_type_remote;	/* 0: not 2833 */
#endif
	FAX_MODEM_STATE faxflag;
	// one stage dial
	char one_stage_dial_number[FW_LEN];
#ifdef MULTI_PROXY
	LinphoneProxyConfig *proxy;
#endif
	int delay_offer_enable;
#ifdef VOIP_CALL_LOG
	char *from;
	char *to;
	unsigned int seq_num;
	time_t end_time;
	time_t duration;
#endif
} LinphoneCall;

LinphoneCall * linphone_call_new_outgoing(struct _LinphoneCore *lc, const osip_from_t *from, const osip_to_t *to);
LinphoneCall * linphone_call_new_incoming(struct _LinphoneCore *lc, guint32 ssid, const char * from,
	const char *to,const osip_from_t * fromurl , const osip_to_t * tourl, int cid, int did);

#define linphone_call_set_state(lcall,st)	(lcall)->state=(st)
void linphone_call_destroy(struct _LinphoneCall *obj);


typedef enum _LinphoneCallStatus {
	LinphoneCallSuccess,
	LinphoneCallAborted,
	LinphoneCallMissed
} LinphoneCallStatus;

typedef struct _LinphoneCallLog{
	struct _LinphoneCore *lc;
	LinphoneCallDir dir;
	LinphoneCallStatus status;
	char *from;
	char *to;
	char *start_date;
	int duration;
} LinphoneCallLog;

typedef enum _LinphoneOptionsPingStatus { 
	Option_WaitACK,
	Option_Next_Ping
} LinphoneOptionsPingStatus;


/* private: */
LinphoneCallLog * linphone_call_log_new(LinphoneCall *call, char *local, char * remote);
void linphone_call_log_completed(LinphoneCallLog *calllog, LinphoneCall *call);
void linphone_call_log_destroy(LinphoneCallLog *cl);

/*public: */
gchar * linphone_call_log_to_str(LinphoneCallLog *cl);

#if 0

typedef enum{
	LinphoneSPWait,
	LinphoneSPDeny,
	LinphoneSPAccept
}LinphoneSubscribePolicy;

typedef struct _LinphoneFriend{
	osip_from_t *url;
	char *contact;	/* not stored, use for immediate outgoing subscribe after receiving incoming subscribe*/
	int sid;
	int nid;
	int last_outsubsc;
	LinphoneSubscribePolicy pol;
	struct _LinphoneProxyConfig *proxy;
	struct _LinphoneCore *lc;
	gboolean subscribe;
	gboolean inc_subscribe_pending;
}LinphoneFriend;

LinphoneFriend * linphone_friend_new(void);
LinphoneFriend *linphone_friend_new_with_addr(const gchar *addr);a
gint linphone_friend_set_sip_addr(LinphoneFriend *fr, const gchar *name);
gint linphone_friend_send_subscribe(LinphoneFriend *fr, gboolean val);
gint linphone_friend_set_inc_subscribe_policy(LinphoneFriend *fr, LinphoneSubscribePolicy pol);
gint linphone_friend_set_proxy(LinphoneFriend *fr, struct _LinphoneProxyConfig *cfg);
void linphone_friend_edit(LinphoneFriend *fr);
void linphone_friend_done(LinphoneFriend *fr);
void linphone_friend_destroy(LinphoneFriend *lf);
/* memory returned by those 3 functions must be freed */
gchar *linphone_friend_get_name(LinphoneFriend *lf);
gchar *linphone_friend_get_addr(LinphoneFriend *lf);
gchar *linphone_friend_get_url(LinphoneFriend *lf);	/* name <sip address> */
#define linphone_friend_url(lf) ((lf)->url)

void linphone_friend_write_to_config_file(struct _LpConfig *config, LinphoneFriend *lf, int index);
LinphoneFriend * linphone_friend_new_from_config_file(struct _LinphoneCore *lc, int index);

#endif

LinphoneProxyConfig *linphone_proxy_config_new(const gchar *server_addr);
gint linphone_proxy_config_set_server_addr(LinphoneProxyConfig *obj, const gchar *server_addr);
void linphone_proxy_config_set_identity(LinphoneProxyConfig *obj, const gchar *identity);
//void linphone_proxy_config_set_route(LinphoneProxyConfig *obj, const gchar *route);
void linphone_proxy_config_expires(LinphoneProxyConfig *obj, const gint expires);
void linphone_proxy_config_enableregister(LinphoneProxyConfig *obj, gboolean val);
void linphone_proxy_config_register(LinphoneProxyConfig *obj);

void linphone_proxy_send_unregister(LinphoneProxyConfig *obj);
void linphone_proxy_send_register_again(LinphoneProxyConfig *obj);

void linphone_proxy_config_done(LinphoneProxyConfig *obj);
void linphone_proxy_config_enable_publish(LinphoneProxyConfig *obj, gboolean val);
#define linphone_proxy_config_get_routes(obj)  ((obj)->reg_routes)
#define linphone_proxy_config_get_identity(obj)	((obj)->reg_identity)
#define linphone_proxy_config_publish_enabled(obj) ((obj)->publish)
#define linphone_proxy_config_get_addr(obj) ((obj)->reg_proxy)

/* destruction is called automatically when removing the proxy config */
void linphone_proxy_config_destroy(LinphoneProxyConfig *cfg);
LinphoneProxyConfig *linphone_proxy_config_new_from_config_file(struct _LpConfig *config, int index);
void linphone_proxy_config_write_to_config_file(struct _LpConfig* config,LinphoneProxyConfig *obj, int index);

typedef struct _LinphoneAuthInfo
{
	gchar *username;
	gchar *realm;
	gchar *userid;
	gchar *passwd;
	gchar *ha1;
	gboolean works;
}LinphoneAuthInfo;

LinphoneAuthInfo *linphone_auth_info_new(const gchar *username, const gchar *userid,
				   										const gchar *passwd, const gchar *ha1,const gchar *realm);
void linphone_auth_info_set_passwd(LinphoneAuthInfo *info, const gchar *passwd);
/* you don't need those function*/
void linphone_auth_info_destroy(LinphoneAuthInfo *info);
LinphoneAuthInfo * linphone_auth_info_new_from_config_file(struct _LpConfig *config, int pos);

#if 0

typedef struct _LinphoneChatRoom{
	struct _LinphoneCore *lc;
	char  *peer;
	osip_from_t *peer_url;
	gpointer user_data;
} LinphoneChatRoom ;

LinphoneChatRoom * linphone_core_create_chat_room(struct _LinphoneCore *lc, const char *to);
void linphone_chat_room_send_message(LinphoneChatRoom *cr, const char *msg);
void linphone_chat_room_destroy(LinphoneChatRoom *cr);
void linphone_chat_room_set_user_data(LinphoneChatRoom *cr, gpointer ud);
gpointer linphone_chat_room_get_user_data(LinphoneChatRoom *cr);

#endif

#if 0

typedef void (*ShowInterfaceCb)(struct _LinphoneCore *lc);
typedef void (*InviteReceivedCb)(struct _LinphoneCore *lc, char *from);
typedef void (*ByeReceivedCb)(struct _LinphoneCore *lc, const char *from);
typedef void (*DisplayStatusCb)(struct _LinphoneCore *lc, char *message);
typedef void (*DisplayMessageCb)(struct _LinphoneCore *lc, char *message);
typedef void (*DisplayUrlCb)(struct _LinphoneCore *lc, char *message,char *url);
typedef void (*DisplayQuestionCb)(struct _LinphoneCore *lc, char *message);
typedef void (*LinphoneCoreCbFunc)(struct _LinphoneCore *lc,gpointer user_data);
typedef void (*NotifyReceivedCb)(struct _LinphoneCore *lc, LinphoneFriend * fid, const char *url, const char *status, const char *img);
typedef void (*NewUnknownSubscriberCb)(struct _LinphoneCore *lc, LinphoneFriend *lf, const char *url);
typedef void (*AuthInfoRequested)(struct _LinphoneCore *lc, const char *realm, const char *username);
typedef void (*CallLogUpdated)(struct _LinphoneCore *lc, struct _LinphoneCallLog *newcl);
typedef void (*TextMessageReceived)(struct _LinphoneCore *lc, LinphoneChatRoom *room, const gchar *from, const gchar *message);
typedef struct _LinphoneVTable
{
	ShowInterfaceCb show;
	InviteReceivedCb inv_recv;
	ByeReceivedCb bye_recv;
	NotifyReceivedCb notify_recv;
	NewUnknownSubscriberCb new_unknown_subscriber;
	AuthInfoRequested auth_info_requested;
	DisplayStatusCb display_status;
	DisplayMessageCb display_message;
	DisplayMessageCb display_warning;
	DisplayUrlCb display_url;
	DisplayQuestionCb display_question;
	CallLogUpdated call_log_updated;
	TextMessageReceived text_received;
} LinphoneCoreVTable;

typedef struct _LCCallbackObj
{
  LinphoneCoreCbFunc _func;
  gpointer _user_data;
}LCCallbackObj;

#endif

#if 0

typedef enum _LinphoneOnlineStatus{
	LINPHONE_STATUS_UNKNOWN,
	LINPHONE_STATUS_ONLINE,
	LINPHONE_STATUS_BUSY,
	LINPHONE_STATUS_BERIGHTBACK,
	LINPHONE_STATUS_AWAY,
	LINPHONE_STATUS_ONTHEPHONE,
	LINPHONE_STATUS_OUTTOLUNCH,
	LINPHONE_STATUS_NOT_DISTURB,
	LINPHONE_STATUS_MOVED,
	LINPHONE_STATUS_ALT_SERVICE,
	LINPHONE_STATUS_OFFLINE,
	LINPHONE_STATUS_END
}LinphoneOnlineStatus;

#endif

typedef struct _TAPP TAPP;

typedef struct _LinphoneCore
{
	TAPP *parent;
	int chid;
//	LinphoneCoreVTable vtable;
//	struct _LpConfig *config;
	net_config_t net_conf;
	sip_config_t sip_conf;
	user_rtp_config_t rtp_conf;
	codecs_config_t codecs_conf;
	ui_config_t ui_conf;
#ifdef MULTI_PROXY
	LinphoneProxyConfig *proxies[MAX_PROXY];
	int default_proxy_index; // default index in flash
#endif
	LinphoneProxyConfig *default_proxy;
//	GList *friends;
//	GList *auth_info;
	gboolean ringstream[MAX_SS] ;
//	LCCallbackObj preview_finished_cb;
//	gboolean preview_finished;
	LinphoneCall *call[MAX_SS] ;   /* the current call, in the future it will be a list of calls (conferencing)*/
	LinphoneCall *call_terminated[MAX_SS] ;
	int rid; /*registration id*/
	GList *call_logs;
	GList *chatrooms;
	int max_call_logs;
	int missed_calls;
	gboolean audiostream[MAX_SS] ;
	struct _RtpProfile *local_profile;
	GList *subscribers;	/* unknown subscribers */
	gint minutes_away;
//	LinphoneOnlineStatus presence_mode;
	gchar *alt_contact;
	void *data;
	gboolean caller_id_mode;
	char cid_dtmf_mode;	/* for the caller id dtmf statr/end digit */
	// call management
	int callingstate;
	char dial_code[LINE_MAX_LEN]; //record dial number 
	int	digit_index;
#ifdef CONFIG_RTK_VOIP_IVR
	char dial_initial_hash;
	int bPlayIVR;
#endif
#ifdef CONFIG_RTK_VOIP_DIALPLAN
	unsigned int dial_data_index;
	char dial_data[LINE_MAX_LEN]; //for dialplan
#endif
	// off-hook alarm and auto-dial
	struct timeval off_hook_start;//record off_hook_start time
	struct timeval auto_dial_start;//everytime receive digit key , re-get current time to auto_dial_start
	//[SD6, ericchung, rcm integration
	uint32	fxsfeatureFlags;
	int offhook_timer; //timeout play WARNING (alert) tone
	int busytone_timer; //timeout play busy tone
	int dialtimeout_timer; // dial interval timeout 
	int flashtimeout_timer; //press flash timeout 
	int stop_alltone_timer;
	int register_retry_timer; //retry register server timer
	int e8c_tr104_timer;	//only for e8c test 
	gchar *lc_input;
	// caller id/name from fxo
	char caller_id[21];
	char caller_id_name[51];
	gboolean caller_id_hidden; // 0: display callerid ,  1: anonymous call 
	gboolean initial_subscribes_sent; /* 0: not send subscript,  1: already send subsciprt */
	int sid; /*subscribes id*/
	int options_tid;	/* record options transation id */
	int options_ping_timer;	/* E8C , use OPTIONS to ping sip server */
	int hotline_timer;	/* E8C , delay sometime to dial hotline */
	LinphoneOptionsPingStatus OptionsPingStatus;
	int no_Answer_timer_id;
	int no_Answer_timeout;
} LinphoneCore;

struct _TAPP
{
	LinphoneCore ports[MAX_VOIP_PORTS];
	int in_main_thread;
	int bAutoAnswer;
	unsigned long port_link_status;
	int rcm_linphone_reset_timer;	//for configure change , need wait all voip call disconnect.
	int rcm_linphone_watchdog_timer; /*for watch dog timer */
	char bound_voip_ipaddress[MAXHOSTNAMELEN];
	char current_voip_interface[MAXHOSTNAMELEN];
	int use_br0;
};

void linphonec_display_something(LinphoneCore *lc, char *something);
void linphonec_display_warning(LinphoneCore *lc, char *something);
void linphonec_display_url(LinphoneCore *lc, char *something, char *url);

void linphonec_prompt_for_auth(LinphoneCore *lc, const char *realm, const char *username);

/* The legacy API */

void linphone_core_enable_logs(FILE *file);
void linphone_core_disable_logs(void);

void linphone_core_init(TAPP *pApp, LinphoneCore *lc, int chid);

int linphone_core_set_primary_contact( LinphoneCore * lc , const gchar * contact ) ;

const gchar *linphone_core_get_primary_contact(LinphoneCore *lc);

void linphone_core_set_guess_hostname( LinphoneCore * lc , gboolean val ) ;
gboolean linphone_core_get_guess_hostname(LinphoneCore *lc);

gboolean linphone_core_ipv6_enabled(LinphoneCore *lc);
void linphone_core_enable_ipv6( LinphoneCore * lc , gboolean val ) ;

#ifdef USE_CURRENT_CONTACT
void linphone_core_get_current_contact(LinphoneCore *lc, const char *to, char **contact);
osip_from_t *linphone_core_get_current_contact_parsed(LinphoneCore *lc, const char *to);
#else
osip_from_t *linphone_core_get_primary_contact_parsed(LinphoneCore *lc);
#endif


void linphone_core_set_connection_type(LinphoneCore *lc, int type);

#define STUN_UPDATE_SIP 0x01
#define STUN_UPDATE_RTP 0x02
#define STUN_UPDATE_T38 0x04
int linphone_core_set_use_info_for_stun(LinphoneCore *lc, int ssid, int flag);

int linphone_core_invite(LinphoneCore *lc, uint32 ssid, const gchar *url);

int linphone_core_accept_dialog(LinphoneCore *lc, guint32 ssid, const char *url);

int linphone_core_terminate_dialog(LinphoneCore *lc, uint32 ssid, const char *url);
void linphone_core_close_terminate_call(LinphoneCore *lc, uint32 ssid);

void linphone_core_send_dtmf(LinphoneCore *lc, uint32 ssid, gchar dtmf);

/* returns a GList of MSCodecInfo */
GList *linphone_core_get_audio_codecs(LinphoneCore *lc);

int linphone_core_set_audio_codecs( LinphoneCore * lc , GList * codecs ) ;

void linphonec_proxy_add(LinphoneCore *lc);

void linphone_core_add_proxy_config(LinphoneCore *lc, LinphoneProxyConfig *config);

void linphone_core_remove_proxy_config(LinphoneCore *lc, LinphoneProxyConfig *config);

GList *linphone_core_get_proxy_config_list(LinphoneCore *lc);

void linphone_core_set_default_proxy(LinphoneCore *lc, LinphoneProxyConfig *config);
void linphone_core_set_default_proxy_index( LinphoneCore * lc , int index ) ;

int linphone_core_get_default_proxy(LinphoneCore *lc, LinphoneProxyConfig **config);

void linphone_core_add_auth_info(LinphoneCore *lc, LinphoneAuthInfo *info);

void linphone_core_remove_auth_info(LinphoneCore *lc, LinphoneAuthInfo *info);

void linphone_core_clear_all_auth_info(LinphoneCore *lc);

void linphone_core_set_use_info_for_dtmf( LinphoneCore * lc , gint dtmf_type ) ;

void linphone_core_set_sip_port( LinphoneCore * lc , int port ) ;

void linphone_core_set_nat_address( LinphoneCore * lc , const gchar * addr , const gint port, gboolean use ) ;

const gchar *linphone_core_get_nat_address(LinphoneCore *lc, gboolean *use);

const gchar *linphone_core_get_nat_address_if_used(LinphoneCore *lc);

int rcm_linphone_dial_local(LinphoneCore *lc, guint32 ssid, int remote_chid,const char *url);

#if 0
void linphone_core_force_ip_address(LinphoneCore *lc, const gchar *ipaddr);
#endif
/* function to be periodically called in a main loop */
void linphone_core_iterate(LinphoneCore *lc);

#if 0

void linphone_core_reject_subscriber(LinphoneCore *lc, LinphoneFriend *lf);
/* a list of LinphoneFriend */
GList * linphone_core_get_friend_list(LinphoneCore *lc);
/* notify all friends that have subscribed */
void linphone_core_notify_all_friends(LinphoneCore *lc, LinphoneOnlineStatus os);

#endif

/* returns a list of LinphoneCallLog */
GList * linphone_core_get_call_logs(LinphoneCore *lc);

gboolean linphone_core_is_in_main_thread(LinphoneCore *lc);

void linphone_core_uninit(LinphoneCore *lc);

void linphone_update_and_display_registration_status( LinphoneCore *lc, LinphoneProxyConfig *cfg, ProxyRegStatus newStatus );

void linphone_core_update_proxy_register_unconditionally(LinphoneCore *lc);

/* end of lecacy api */

/*internal use only */
#define linphone_core_thread_enter(lc) (lc)->in_main_thread=0
#define linphone_core_thread_leave(lc) (lc)->in_main_thread=1
#define linphone_core_lock(lc)	g_mutex_lock((lc)->lock)
#define linphone_core_unlock(lc)	g_mutex_unlock((lc)->lock)
#ifdef CHECK_RESOURCE
int linphone_core_start_media_streams(LinphoneCore *lc, guint32 ssid, LinphoneCall *call);
#else
void linphone_core_start_media_streams(LinphoneCore *lc, guint32 ssid, LinphoneCall *call);
#endif
void linphone_core_stop_media_streams(LinphoneCore *lc, guint32 ssid);
const char * linphone_core_get_identity(LinphoneCore *lc);
const osip_list_t * linphone_core_get_routes(LinphoneCore *lc);

int linphone_core_onhood_event(LinphoneCore *lc, guint32 ssid);
int linphone_core_offhood_event(LinphoneCore *lc, guint32 ssid);
void linphone_call_fax_reinvite(LinphoneCore *lc, guint32 ssid);
void rcm_linphone_call_off_fax_reinvite(LinphoneCore *lc, guint32 ssid);

void linphone_core_transfer_call(LinphoneCore *lc, guint32 ssid, const char *refer);
void linphone_core_forward_dialog(LinphoneCore * lc, guint32 ssid, const char *forward);

//[SD6, ericchung, rcm integration
int rcm_linphone_eXosip_on_hold(LinphoneCore *lc, guint32 ssid);
int rcm_linphone_eXosip_off_hold(LinphoneCore *lc, guint32 ssid);	
int rcm_linphone_eXosip_on_fax(LinphoneCore *lc, guint32 ssid,int port,FAX_MODEM_STATE fms_state);
int rcm_linphone_eXosip_mcid_reinvite(LinphoneCore *lc, guint32 ssid);
void rcm_linphone_core_send_initial_subscribes(LinphoneCore *lc,LinphoneProxyConfig * cfg);
void rcm_linphone_core_send_options(LinphoneCore *lc,LinphoneProxyConfig * cfg);
void rcm_linphone_message_to_proxy(LinphoneCore *lc,LinphoneProxyConfig * cfg);

void rcm_linphone_core_iterate_exosip(void);

//]

#ifdef __cplusplus
}
#endif

typedef enum
{
	SYS_STATE_IDLE, //0
#ifdef CONFIG_RTK_VOIP_IVR
	SYS_STATE_AUTH,//1
	SYS_STATE_AUTH_ERROR,//2
#endif
	SYS_STATE_AUTOBYPASS_WARNING,//3
	SYS_STATE_EDIT,//4
	SYS_STATE_EDIT_ERROR,//5
	SYS_STATE_RING,//6
	SYS_STATE_CONNECT,//7
	SYS_STATE_CONNECT_DAA_RING,//8
	SYS_STATE_CONNECT_EDIT,//9
	SYS_STATE_CONNECT_EDIT_ERROR, //10
	SYS_STATE_DAA_RING,//11
	SYS_STATE_DAA_CONNECT,//12
	SYS_STATE_TRANSFER_EDIT,//13
	SYS_STATE_TRANSFER_EDIT_ERROR,//14
	SYS_STATE_IN2CALLS_OUT,//15
	SYS_STATE_IN2CALLS_IN, //second call incoming
	SYS_STATE_IN3WAY,
	SYS_STATE_TRANSFERRING,
	SYS_STATE_VOICE_IVR,
	SYS_STATE_VOICE_IVR_DONE,
#ifdef ATTENDED_TRANSFER
	SYS_STATE_TRANSFER_CONNECT,		// 2 call here, wait attended transfer
	SYS_STATE_TRANSFER_ONHOOK,
	SYS_STATE_TRANSFER_FINISH,
#endif
/*+++added by Jack Chan for Planet transfer+++*/
#ifdef PLANET_TRANSFER
	SYS_STATE_TRANSFER_BLIND,
	SYS_STATE_TRANSFER_ATTENDED
#endif /*PLANET_TRANSFER*/
/*---end---*/
} SYSTEMSTSTE;

typedef enum
{
	SS_STATE_IDLE ,
	SS_STATE_CALLOUT , //start outgoing call 
	SS_STATE_CALLER ,  //outgoing call success, I am Caller
	SS_STATE_CALLIN , //Receive incoming call
	SS_STATE_CALLEE   //incoming call success, I am Callee
} SESSIONSTATE ;

typedef enum
{
	REINVITE_INIT = 0 ,
	REINVITE_CALLER = 1 ,
	REINVITE_CALLEE = 2 ,
	REINVITE_ERR = 0xF
} REINVITE_EVENT ;

typedef struct precedence_config_s
{
	uint32 chid;
	uint32 uCodecMax;
	RtpPayloadType *pCodecPred;
	int32 *pFramePerPacket;
} precedence_config_t;

typedef enum {
	Unsupported = -1,
	Supported = 0,
	SupportedAndValid = 1 /* valid= the presence of this codec is enough to make a call */
} SupportLevel;

extern SYSTEMSTSTE	g_sys_state[] ;
extern SESSIONSTATE c_session_state[MAX_VOIP_PORTS][MAX_SS];

void SetSysState( guint32 chid , SYSTEMSTSTE state ) ;
SYSTEMSTSTE GetSysState( guint32 chid ) ;

void SetSessionState( guint32 chid , guint32 ssid , SESSIONSTATE value ) ;
//[SD6, ericchung, exosip 3.5 integration
void SetCallState( guint32 chid , guint32 ssid ,LCState *callstate, LCState value ) ;
//]

void UpdateLedDisplayMode( LinphoneCore * lc );
SESSIONSTATE GetSessionState( guint32 chid , guint32 ssid ) ;
int GetIdleSession( guint32 chid ) ;
void SetActiveSession( guint32 chid , guint32 ssid , int value ) ;
int GetActiveSession( guint32 chid ) ;
int session_2_session_event(LinphoneCore *lc, int s_case);
void digitcode_init(LinphoneCore *lc);

int linphone_find_chid_ssid(TAPP *pApp, int cid, int *p_chid, int *p_ssid);
#ifdef CHECK_RESOURCE
void linphone_no_resource_handling(LinphoneCore *lc, int ssid);
#endif

#ifdef ATTENDED_TRANSFER
int atdfr_construct_referto(LinphoneCore *lc, char *refer_to);
#endif

int32 GetCodecPrecedence(int chid, precedence_config_t *cfg);
int GetRtpCfgFromSession(rtp_config_t *rtp_config,
	rtcp_config_t *rtcp_config,
	t38udp_config_t *t38udp_config, 
	payloadtype_config_t *dsp_config, 
	t38_payloadtype_config_t *t38_config,
	LinphoneCore *lc, int ssid);

int qos2tos(unsigned char index);
int qos2dscp(unsigned char index);
int linphone_core_get_remote_rfc2833_pt(LinphoneCall *call);
int GetRtpCfg(TstVoipMgrRtpCfg *rtp_cfg, LinphoneCore *lc, int ssid);
//[SD6, bohungwu. exosip 3.5 integration
//int eXosip_on_update_call(int jid, int session_version);
int CheckDND(int chid);
int rtk_check_fxo_call_fxs(osip_to_t *to);
int rtk_call_check_fxo_call_fxs(LinphoneCall *call);

#ifdef CONFIG_RTK_VOIP_TR104
void solarSendResponseToCwmp(cwmpEvtMsg *evtMsg);
void linphone_call_log_statistics(LinphoneCore *lc, voiceProfileLineStatus *VPLineStatus);
void rcm_linphone_reset_call_log_statistics(LinphoneCore *lc);

#endif /*CONFIG_RTK_VOIP_TR104*/
/*+++++ Added by Jack for SRTP 150108+++++*/
#ifdef CONFIG_RTK_VOIP_SRTP
void base64Encode(unsigned char *s, char *result);
void base64Decode (char *s, unsigned char *result);
#endif /*CONFIG_RTK_VOIP_SRTP*/
/*-----end-----*/
extern TAPP *solar_ptr;
extern voipCfgParam_t *g_pVoIPCfg;
extern int g_MaxVoIPPorts;
extern voip_flash_share_t *g_pVoIPShare;
extern int g_nMaxCodec;
extern int g_mapSupportedCodec[_CODEC_MAX];

typedef struct SystemParam_s {
	// 3 way conference command
	unsigned char conference_code[ 20 ];
	unsigned char stutter_tone;
	unsigned char fxo_redial;
	int proxy_timeout_count_limit;
	int proxy_timeout_retry_sec;
#ifdef SUPPORT_G7111
	int g711u_wb_pt;
	int g711a_wb_pt;
#endif
	int g726_16_pt;
	int g726_24_pt;
	int g726_32_pt;
	int g726_40_pt;
	int rtp_init;
	unsigned long rtp_init_seqno;
	unsigned long rtp_init_ssrc;
	unsigned long rtp_init_timestamp;
	int red_demo;
	int red_audio;
	int red_rfc2833;
	int fpp_demo;
	int fpp;
	int fppvbd;
	unsigned long silence_demo;
	unsigned long silence_dbov;
	unsigned long silence_period;
	unsigned char modem_lec;
	unsigned long ivr_delay;
	unsigned long max_strict_delay;
	int qos_dscp;
} SystemParam_t;

extern SystemParam_t g_SystemParam;

#endif

//[SD6, ericchung, rcm integration
//ericchung test, timeout handle function
//change time out from 3sec to 10sec
#define TIMEOUT_FXS_FLASHTKEY			10000

void offhook_dialtimeout(void *arg);
void fxs_flashkeytimeout(void *arg);
void dial_interval_timeout(void *arg);
void timeout_playbusytone(void *arg);
void stop_alltone(void *arg);
void rcm_active_BusyWrongTone_timer(LinphoneCore *lc);
void rerty_register(void *arg);
void rcm_configure_restart(void *arg);
void e8c_dtmf_test(void *arg);
void rcm_options_ping_timeout(void *arg);
void dial_to_hotline(void *arg);
void no_answer_timeout(void *arg);
//]
void rcm_IvrPolling_proc( LinphoneCore *lc );
void rcm_linphone_NoAnswerCheck( LinphoneCore *lc ,int ActiveLine);
void rcm_linphone_watchdog_hander(void *arg);

#ifdef RCM_VOIP_INTERFACE_ROUTE
int  rcm_add_voip_interface_route(LinphoneCore *lc, guint32 ssid);
int  rcm_del_voip_interface_route(LinphoneCore *lc, guint32 ssid);
#endif
