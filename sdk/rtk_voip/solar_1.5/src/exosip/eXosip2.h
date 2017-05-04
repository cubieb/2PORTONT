/*
  eXosip - This is the eXtended osip library.
  Copyright (C) 2002, 2003  Aymeric MOIZARD  - jack@atosc.org
  
  eXosip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  eXosip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef __EXOSIP2_H__
#define __EXOSIP2_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include "../osip/osip.h"
#include "../osip/osip_dialog.h"
#include "../osip/osip_negotiation.h"
#include "eXosip_cfg.h"
#include "eXosip.h"
#include "jpipe.h"

#include "../linphone/linphonecore.h"

#ifndef JD_EMPTY

#define JD_EMPTY          0
#define JD_INITIALIZED    1
#define JD_TRYING         2
#define JD_QUEUED         3
#define JD_RINGING        4
#define JD_ESTABLISHED    5
#define JD_REDIRECTED     6
#define JD_AUTH_REQUIRED  7
#define JD_CLIENTERROR    8
#define JD_SERVERERROR    9
#define JD_GLOBALFAILURE  10
#define JD_TERMINATED     11

#define JD_MAX            11

#endif

#define EXOSIP_VERSION	"0.1"

#ifdef __cplusplus
extern "C"
{
#endif

void  eXosip_update(void);
void  __eXosip_wakeup(void);

typedef struct eXosip_dialog_t {

  int              d_id;
  int              d_STATE;
  osip_dialog_t   *d_dialog;      /* active dialog */

  int              d_timer;
  osip_message_t  *d_200Ok;
  osip_message_t  *d_ack;
  osip_list_t     *media_lines;

  osip_list_t     *d_inc_trs;
  osip_list_t     *d_out_trs;

  /* struct _BodyHandler     *d_bh; */

  struct eXosip_dialog_t *next;
  struct eXosip_dialog_t *parent;
} eXosip_dialog_t ;

typedef struct eXosip_subscribe_t {
  int                 nPort;
  
  int                 s_id;
  char                s_uri[255];
  int                 s_online_status;
  int                 s_ss_status;
  int                 s_ss_reason;
  int                 s_ss_expires;
  eXosip_dialog_t    *s_dialogs;

  osip_transaction_t *s_inc_tr;
  osip_transaction_t *s_out_tr;

  struct eXosip_subscribe_t *next;
  struct eXosip_subscribe_t *parent;
} eXosip_subscribe_t ;

typedef struct eXosip_notify_t {
  int                 nPort;

  int                 n_id;
  char                n_uri[255];
  int                 n_online_status;
  char                *n_contact_info;

  int                 n_ss_status;
  int                 n_ss_reason;
  int                 n_ss_expires;
  eXosip_dialog_t    *n_dialogs;

  osip_transaction_t *n_inc_tr;
  osip_transaction_t *n_out_tr;

  struct eXosip_notify_t    *next;
  struct eXosip_notify_t    *parent;
} eXosip_notify_t ;

typedef struct eXosip_call_t {
  int                      nPort;

  int                      c_id;
  char                     c_subject[100];
  int                      c_ack_sdp; /* flag for alternative SDP offer-response model */
  eXosip_dialog_t         *c_dialogs;
  osip_transaction_t      *c_inc_tr;
  osip_transaction_t      *c_out_tr;
  osip_transaction_t      *c_inc_options_tr;
  osip_transaction_t      *c_out_options_tr;
  void                    *external_reference;

  osip_negotiation_ctx_t  *c_ctx;
  char                     c_sdp_port[10];

  char                     c_redirection[1024]; /* contact for 3xx answers */

  struct eXosip_call_t           *next;
  struct eXosip_call_t           *parent;
} eXosip_call_t ;


#if 0
typedef struct eXosip_realm_t eXosip_realm_t;

struct eXosip_realm_t {

  int             r_id;

  char           *r_realm;
  char           *r_username;
  char           *r_passwd;

  eXosip_realm_t *next;
  eXosip_realm_t *parent;
};
#endif

typedef struct eXosip_reg_t {
  int             nPort;

  int             r_id;

  int             r_reg_period;     /* delay between registration */
  char           *r_aor;            /* sip identity */
  char           *r_registrar;      /* registrar */
#if 0
  eXosip_realm_t *r_realms;         /* list of realms */
#endif
  char           *r_contact;        /* list of contacts string */

  osip_transaction_t  *r_last_tr;
 
  osip_list_t	 *r_routes;

  struct eXosip_reg_t   *next;
  struct eXosip_reg_t   *parent;
} eXosip_reg_t ;


typedef struct eXosip_pub_t {
  int             nPort;

  int             p_id;

  time_t          p_expires;        /* expiration date (started+period) */
  int             p_period;         /* delay between registration */
  char            p_aor[256];       /* sip identity */
  char            p_sip_etag[64];   /* sip_etag from 200ok */

  osip_transaction_t  *p_last_tr;

  struct eXosip_pub_t   *next;
  struct eXosip_pub_t   *parent;
} eXosip_pub_t ;

int _eXosip_pub_update(eXosip_pub_t **pub, osip_transaction_t *tr, osip_message_t *answer);
int _eXosip_pub_find_by_aor(eXosip_pub_t **pub, const char *aor);
int _eXosip_pub_init(int nPort, eXosip_pub_t **pub, const char *aor, const char *exp);
void _eXosip_pub_free(eXosip_pub_t *pub);

typedef struct jauthinfo_t {
  char username[50];
  char userid[50];
  char passwd[50];
  char ha1[50];
  char realm[50];
  struct jauthinfo_t *parent;
  struct jauthinfo_t *next;
} jauthinfo_t ;

int
__eXosip_create_authorization_header(osip_message_t *previous_answer,
				     const char *rquri, const char *username,
				     const char *passwd,
				     osip_authorization_t **auth);
int
__eXosip_create_proxy_authorization_header(osip_message_t *previous_answer,
					   const char *rquri,
					   const char *username,
					   const char *passwd,
					   osip_proxy_authorization_t **auth);


eXosip_event_t *eXosip_event_init_for_call(int type, eXosip_call_t *jc,
					      eXosip_dialog_t *jd);
int eXosip_event_add_sdp_info(eXosip_event_t *je, osip_message_t *message);

int eXosip_event_add_status(eXosip_event_t *je, osip_message_t *response);
eXosip_event_t *eXosip_event_init_for_subscribe(int type,
						   eXosip_subscribe_t *js,
						   eXosip_dialog_t *jd);
eXosip_event_t *eXosip_event_init_for_notify(int type, eXosip_notify_t *jn,
						eXosip_dialog_t *jd);
eXosip_event_t *eXosip_event_init_for_reg(int type, eXosip_reg_t *jr);
eXosip_event_t *eXosip_event_init_for_message(int type, osip_transaction_t
					      *tr, osip_message_t *sip);
int eXosip_event_init(int nPort, eXosip_event_t **je, int type);
eXosip_call_t *eXosip_event_get_callinfo(eXosip_event_t *je);
eXosip_dialog_t *eXosip_event_get_dialoginfo(eXosip_event_t *je);
eXosip_reg_t *eXosip_event_get_reginfo(eXosip_event_t *je);
eXosip_notify_t *eXosip_event_get_notifyinfo(eXosip_event_t *je);
eXosip_subscribe_t *eXosip_event_get_subscribeinfo(eXosip_event_t *je);
int eXosip_event_add(eXosip_event_t *je);
eXosip_event_t *eXosip_event_wait(int tv_s, int tv_ms);
eXosip_event_t *eXosip_event_get(void);

#ifdef FIX_RESPONSE_TIME
typedef void (*eXosip_callback_t)(void *user, int type, eXosip_event_t *);
void eXosip_register_callback(eXosip_callback_t *cb);
#else
typedef void (*eXosip_callback_t)(int type, eXosip_event_t *);
#endif

#ifdef MALLOC_DEBUG
#define strdup_printf(f,...) __strdup_printf(__FILE__, __LINE__, (f), __VA_ARGS__)
char *__strdup_printf(char *file, int line, const char *fmt, ...);
#else
char *strdup_printf(const char *fmt, ...);
#endif

jfriend_t *jfriend_get(void);
jsubscriber_t *jsubscriber_get(void);
jidentity_t *jidentity_get(void);
int jfriend_get_and_set_next_token (char **dest, char *buf,
				    char **next);

#ifdef MALLOC_DEBUG
#define eXosip_trace(loglevel,...)  do        \
{                       \
	char *__strmsg;  \
	__strmsg=__strdup_printf(__FILE__, __LINE__,__VA_ARGS__);    \
	OSIP_TRACE(osip_trace(__FILE__,__LINE__,(loglevel),NULL,"%s\n",__strmsg)); \
	osip_free (__strmsg);        \
}while (0);
#else
#define eXosip_trace(loglevel,args)  do        \
{                       \
	char *__strmsg;  \
	__strmsg=strdup_printf args ;    \
	OSIP_TRACE(osip_trace(__FILE__,__LINE__,(loglevel),NULL,"%s\n",__strmsg)); \
	osip_free (__strmsg);        \
}while (0);
#endif

typedef struct eXosip_t {
  int forced_localip; /* set to 1 when we must always use the default local ip */
  char *localip;	/* default local ip */
  char *localports[MAX_VOIP_PORTS];
  char *media_ports[MAX_VOIP_PORTS];

  char *user_agent;

  FILE               *j_input;
  FILE               *j_output;
  eXosip_call_t      *j_calls;        /* my calls        */
  eXosip_subscribe_t *j_subscribes;   /* my friends      */
  eXosip_notify_t    *j_notifies;     /* my susbscribers */
  osip_list_t        *j_transactions;

  eXosip_reg_t       *j_reg;          /* my registrations */
  eXosip_pub_t       *j_pub;          /* my publications  */

#ifndef DISABLE_THREAD
  void               *j_cond;
  void               *j_mutexlock;
#endif

  osip_t             *j_osip;
  int                 j_sockets[MAX_VOIP_PORTS];
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	int		j_tls_sockets[MAX_VOIP_PORTS][MAX_PROXY];
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
  int                 j_stop_ua;
#ifndef DISABLE_THREAD
  void               *j_thread;
#endif
  jpipe_t            *j_socketctl;
  jpipe_t            *j_socketctl_event;

  jsubscriber_t      *j_subscribers;
  jfriend_t          *j_friends;
  jidentity_t        *j_identitys;

  int                 j_runtime_mode;
#ifdef FIX_RESPONSE_TIME
  eXosip_callback_t   *j_call_callbacks;
  void                *user;
#else
  eXosip_callback_t   j_call_callbacks[EXOSIP_CALLBACK_COUNT];
#endif
  osip_fifo_t        *j_events;

  osip_negotiation_t *osip_negotiation;
  char                j_firewall_ip[MAX_VOIP_PORTS][50];
  /* Mandy add for stun support */
  char		    j_firewall_ports[MAX_VOIP_PORTS][9];
  char		    j_firewall_media_ports[MAX_VOIP_PORTS * MAX_SS][9];
  /*++added by Jack Chan 05/03/07 for T.38++*/
  char			j_firewall_fax_ports[MAX_VOIP_PORTS][9];
  char			fax_ports[MAX_VOIP_PORTS][9];
 /*--end--*/

  jauthinfo_t        *authinfos;

  int                 ip_family; /* AF_INET6 or AF_INET */
  char                nat_type[16];
  char                answer_contact[256]; /* conatact header value to use in answers */

#ifdef NEW_STUN
  int j_media_sockets[MAX_VOIP_PORTS * MAX_SS];
  int j_t38_sockets[MAX_VOIP_PORTS];
  struct timeval j_firewall_ports_expire[MAX_VOIP_PORTS];
  struct timeval j_firewall_fax_ports_expire[MAX_VOIP_PORTS];
  struct timeval j_firewall_media_ports_expire[MAX_VOIP_PORTS * MAX_SS];
#endif

#ifdef SIP_PING
  osip_transaction_t *pings[MAX_VOIP_PORTS];
  char pinged_ip[MAX_VOIP_PORTS][50];
  char pinged_ports[MAX_VOIP_PORTS][9];
  struct timeval pinged_ports_expire[MAX_VOIP_PORTS];
#endif
#ifdef PLANET_TRANSFER
	unsigned char bIVR_transfer;
	char blind_transfer_code[20];
	char attended_transfer_code[20];
#endif
} eXosip_t ;

typedef struct jinfo_t {
  int nPort;
  eXosip_dialog_t     *jd;
  eXosip_call_t       *jc;
  eXosip_subscribe_t  *js;
  eXosip_notify_t     *jn;
} jinfo_t ;

#ifdef CONFIG_RTK_VOIP_SIP_TLS
typedef struct proxy_info_t{
	TRANSPORT_TYPE transportType;
	int proxyIndex;
}proxy_info;
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/

int eXosip_guess_ip_for_via (int family, char *address, int size);

int  eXosip_sdp_negotiation_init(osip_negotiation_t **sn);
void eXosip_sdp_negotiation_free(osip_negotiation_t *sn);
int eXosip_retrieve_sdp_negotiation_result(osip_negotiation_ctx_t *ctx, char *payload_name,  int pnsize);


sdp_message_t * eXosip_get_local_sdp_info(osip_transaction_t *invite_tr);
sdp_message_t *eXosip_get_remote_sdp_info(osip_transaction_t *invite_tr);
sdp_message_t *eXosip_get_local_sdp(osip_transaction_t *transaction);
sdp_message_t *eXosip_get_remote_sdp(osip_transaction_t *transaction);



int    eXosip_set_callbacks(osip_t *osip);
char  *osip_call_id_new_random(void);
char  *osip_to_tag_new_random(void);
char  *osip_from_tag_new_random(void);
unsigned int via_branch_new_random(void);
void __eXosip_delete_jinfo(osip_transaction_t *transaction);
#ifdef MALLOC_DEBUG
#define __eXosip_new_jinfo(p, c, d, s, n) ____eXosip_new_jinfo(p, c, d, s, n, __FILE__, __LINE__)
jinfo_t *____eXosip_new_jinfo(int nPort, eXosip_call_t *jc, eXosip_dialog_t *jd,
			    eXosip_subscribe_t *js, eXosip_notify_t *jn, char *file, int line);
#else
jinfo_t *__eXosip_new_jinfo(int nPort, eXosip_call_t *jc, eXosip_dialog_t *jd,
			    eXosip_subscribe_t *js, eXosip_notify_t *jn);
#endif

int  eXosip_dialog_init_as_uac(eXosip_dialog_t **jd, osip_message_t *_200Ok);
int  eXosip_dialog_init_as_uas(eXosip_dialog_t **jd, osip_message_t *_invite, osip_message_t *_200Ok);
void eXosip_dialog_free(eXosip_dialog_t *jd);
void eXosip_dialog_set_state(eXosip_dialog_t *jd, int state);
void eXosip_delete_early_dialog(eXosip_dialog_t *jd);


struct __eXosip_sockaddr {
  u_char ss_len;
  u_char ss_family;
  u_char padding[128 - 2];
};

int eXosip_get_addrinfo (struct addrinfo **addrinfo,
				char *hostname, int service);
int eXosip_get_ip_by_name(unsigned long *ip, const char *hostname, int service);
int isrfc1918(char *ipaddr);
void eXosip_get_localip_from_via(osip_message_t *,char**localip);
int  generating_initial_subscribe(int nPort, osip_message_t **message, char *to,
				 char *from, osip_list_t *routes);
int  generating_message(int nPort, osip_message_t **message, char *to, char *from,
		       osip_list_t *routes, char *buff);
#ifdef SIP_PING
int generating_ping(int nPort, osip_message_t **ping, char *from, char *to, osip_list_t *routes);
#endif
int  generating_publish(int nPort, osip_message_t **message, char *to, char *from,
			osip_list_t *routes);
int  generating_cancel(int nPort, osip_message_t **dest, osip_message_t *request_cancelled);
int  generating_options_within_dialog(int nPort, osip_message_t **info, osip_dialog_t *dialog);
int  generating_info_within_dialog(int nPort, osip_message_t **info, osip_dialog_t *dialog);
int  generating_bye(int nPort, osip_message_t **bye, osip_dialog_t *dialog);
int  generating_refer(int nPort, osip_message_t **refer, osip_dialog_t *dialog, const char *refer_to);
int  generating_refer_outside_dialog(int nPort, osip_message_t **refer, char *refer_to, char *from, char *to, 
	osip_list_t *routes);
int  generating_invite_on_hold(int nPort, osip_message_t **invite, osip_dialog_t *dialog,
				char *subject, char *sdp);
int  generating_invite_off_hold(int nPort, osip_message_t **invite, osip_dialog_t *dialog,
				char *subject, char *sdp);
int  generating_options(int nPort, osip_message_t **options, char *from, char *to, osip_list_t *routes);
int  generating_ack_for_2xx(int nPort, osip_message_t **ack, osip_dialog_t *dialog);
int  generating_info(int nPort, osip_message_t **info, char *from, char *to, osip_list_t *routes);

int  _eXosip_reg_find(eXosip_reg_t **reg, osip_transaction_t *tr);
int  eXosip_reg_init(int nPort, eXosip_reg_t **jr, char *from, char *proxy, char *contact, osip_list_t *routes);
void eXosip_reg_free(eXosip_reg_t *jreg);
int generating_register(int nPort, osip_message_t **reg, char *from, osip_list_t *routes, char *proxy, char *contact, 
	int expires);
char *generating_sdp_answer(int nPort, osip_message_t *request, osip_negotiation_ctx_t *context);

int eXosip_call_dialog_find(int jid, eXosip_call_t **jc, eXosip_dialog_t **jd);
int eXosip_notify_dialog_find(int nid, eXosip_notify_t **jn, eXosip_dialog_t **jd);
int eXosip_subscribe_dialog_find(int nid, eXosip_subscribe_t **js, eXosip_dialog_t **jd);
int eXosip_call_find(int cid, eXosip_call_t **jc);
#ifdef MALLOC_DEBUG
int __eXosip_dialog_set_200ok(eXosip_dialog_t *_jd, osip_message_t *_200Ok, char *file, int line);
#define eXosip_dialog_set_200ok(d,k) __eXosip_dialog_set_200ok(d,k,__FILE__,__LINE__)
#else
int eXosip_dialog_set_200ok(eXosip_dialog_t *_jd, osip_message_t *_200Ok);
#endif

int _eXosip2_answer_invite_3456xx(eXosip_call_t *jc, eXosip_dialog_t *jd, int code, osip_message_t **answer);
int _eXosip2_answer_invite_2xx(eXosip_call_t *jc, eXosip_dialog_t *jd, int code, osip_message_t **answer);
int _eXosip2_answer_invite_1xx(eXosip_call_t *jc, eXosip_dialog_t *jd, int code, osip_message_t **answer);
int eXosip_answer_invite_1xx(eXosip_call_t *jc, eXosip_dialog_t *jd, int code);
int eXosip_answer_invite_2xx(eXosip_call_t *jc, eXosip_dialog_t *jd, int code, char *local_sdp_port);
int eXosip_answer_invite_2xx_with_body(eXosip_call_t *jc, eXosip_dialog_t *jd, int code,const char*, const char*);
int eXosip_answer_invite_3456xx(eXosip_call_t *jc, eXosip_dialog_t *jd, int code);
int eXosip_answer_options_1xx(eXosip_call_t *jc, eXosip_dialog_t *jd, int code);
int eXosip_answer_options_2xx(eXosip_call_t *jc, eXosip_dialog_t *jd, int code);
int eXosip_answer_options_3456xx(eXosip_call_t *jc, eXosip_dialog_t *jd, int code);
void eXosip_notify_answer_subscribe_1xx(eXosip_notify_t *jc,
					eXosip_dialog_t *jd, int code);
void eXosip_notify_answer_subscribe_2xx(eXosip_notify_t *jn,
					eXosip_dialog_t *jd, int code);
void eXosip_notify_answer_subscribe_3456xx(eXosip_notify_t *jn,
					   eXosip_dialog_t *jd, int code);

int eXosip_build_response_default(int jid, int status);
int _eXosip_build_response_default(osip_message_t **dest, osip_dialog_t *dialog,
				  int status, osip_message_t *request);
int complete_answer_that_establish_a_dialog(int nPort, osip_message_t *response, osip_message_t *request);
int _eXosip_build_request_within_dialog(int nPort, osip_message_t **dest, char *method_name,
				       osip_dialog_t *dialog, char *transport);
int eXosip_build_initial_options(int nPort, osip_message_t **options, char *to, char *from,
				 osip_list_t *route);

void eXosip_kill_transaction(osip_list_t * transactions);
int eXosip_remove_transaction_from_call(osip_transaction_t *tr, eXosip_call_t *jc);
osip_transaction_t *eXosip_find_last_inc_notify(eXosip_subscribe_t *jn, eXosip_dialog_t *jd);
osip_transaction_t *eXosip_find_last_out_notify(eXosip_notify_t *jn, eXosip_dialog_t *jd);
osip_transaction_t *eXosip_find_last_inc_subscribe(eXosip_notify_t *jn, eXosip_dialog_t *jd);
osip_transaction_t *eXosip_find_last_out_subscribe(eXosip_subscribe_t *js, eXosip_dialog_t *jd );

osip_transaction_t *eXosip_find_last_out_options(eXosip_call_t *jc, eXosip_dialog_t *jd );
osip_transaction_t *eXosip_find_last_inc_options(eXosip_call_t *jc, eXosip_dialog_t *jd );
osip_transaction_t *eXosip_find_last_options(eXosip_call_t *jc, eXosip_dialog_t *jd );
osip_transaction_t *eXosip_find_last_out_info(eXosip_call_t *jc, eXosip_dialog_t *jd );
osip_transaction_t *eXosip_find_last_inc_info(eXosip_call_t *jc, eXosip_dialog_t *jd );
osip_transaction_t *eXosip_find_last_info(eXosip_call_t *jc, eXosip_dialog_t *jd );
osip_transaction_t *eXosip_find_last_invite(eXosip_call_t *jc, eXosip_dialog_t *jd );
osip_transaction_t *eXosip_find_last_inc_invite(eXosip_call_t *jc, eXosip_dialog_t *jd);
osip_transaction_t *eXosip_find_last_out_invite(eXosip_call_t *jc, eXosip_dialog_t *jd);
osip_transaction_t *eXosip_find_last_refer(eXosip_call_t *jc, eXosip_dialog_t *jd );
osip_transaction_t *eXosip_find_last_inc_notify_for_refer(eXosip_call_t *jc, eXosip_dialog_t *jd);
osip_transaction_t *eXosip_find_last_out_notify_for_refer(eXosip_call_t *jc, eXosip_dialog_t *jd);
osip_transaction_t *eXosip_find_last_inc_bye(eXosip_call_t *jc, eXosip_dialog_t *jd);
osip_transaction_t *eXosip_find_last_out_bye(eXosip_call_t *jc, eXosip_dialog_t *jd);
osip_transaction_t *eXosip_find_last_inc_refer(eXosip_call_t *jc, eXosip_dialog_t *jd);
osip_transaction_t *eXosip_find_last_out_refer(eXosip_call_t *jc, eXosip_dialog_t *jd);
osip_transaction_t *eXosip_find_last_out_tr(eXosip_call_t *jc, eXosip_dialog_t *jd);


int  eXosip_call_init(int nPort, eXosip_call_t **jc);
void eXosip_call_free(eXosip_call_t *jc);
void __eXosip_call_remove_dialog_reference_in_call(eXosip_call_t *jc, eXosip_dialog_t *jd);
void eXosip_call_set_subject(eXosip_call_t *jc, char *subject);
#ifdef DISABLE_THREAD
#ifdef NEW_STUN
#ifdef CONFIG_RTK_VOIP_SIP_TLS
int eXosip_read_message(LinphoneCore *lc, char **buf, int *buf_len, int socket_type, int proxyIndex);
#else /*CONFIG_RTK_VOIP_SIP_TLS*/
int eXosip_read_message(LinphoneCore *lc, char **buf, int *buf_len);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
#else
int  eXosip_read_message(LinphoneCore *lc);
#endif
#else
int  eXosip_read_message(int max_message_nb, int sec_max, int usec_max);
#endif
void eXosip_release_terminated_calls ( void );

int eXosip_process_icmp_message( unsigned long ip, unsigned short port );

int  eXosip_subscribe_init(int nPort, eXosip_subscribe_t **js, char *uri);
void eXosip_subscribe_free(eXosip_subscribe_t *js);
int  _eXosip_subscribe_set_refresh_interval(eXosip_subscribe_t *js, osip_message_t *inc_subscribe);
int  eXosip_subscribe_need_refresh(eXosip_subscribe_t *js, int now);
int eXosip_subscribe_send_subscribe(eXosip_subscribe_t *js,
				    eXosip_dialog_t *jd, const char *expires);

int  eXosip_notify_init(int nPort, eXosip_notify_t **jn, osip_message_t *inc_subscribe);
void eXosip_notify_free(eXosip_notify_t *jn);
int  _eXosip_notify_set_contact_info(eXosip_notify_t *jn, char *uri);
int  _eXosip_notify_set_refresh_interval(eXosip_notify_t *jn,
					 osip_message_t *inc_subscribe);
void _eXosip_notify_add_expires_in_2XX_for_subscribe(eXosip_notify_t *jn,
						     osip_message_t *answer);
int  _eXosip_notify_add_body(eXosip_notify_t *jn, osip_message_t *notify);
int  eXosip_notify_add_allowed_subscriber(char *sip_url);
int  _eXosip_notify_is_a_known_subscriber(osip_message_t *sip);
int  eXosip_notify_send_notify(eXosip_notify_t *jn, eXosip_dialog_t *jd,
			       int subsciption_status,
			       int online_status);
int  _eXosip_transfer_send_notify(eXosip_call_t *jc,
				  eXosip_dialog_t *jd,
				  int subscription_status,
				  char *body);


int eXosip_is_public_address(const char *addr);

extern eXosip_t eXosip;

#ifdef DISABLE_THREAD
void eXosip_process_newrequest(int nPort, osip_event_t *evt);
void eXosip_process_response_out_of_transaction(osip_event_t *evt);
#endif

// for ack retransmission in UA core
int send_ack_directly(int nPort, osip_message_t *ack);
osip_message_t *build_ack_for_receiving_2xx_response(eXosip_call_t *jc,
	eXosip_dialog_t *jd, osip_message_t *msg_2xx);

#ifdef NEW_STUN
int rtk_stun_init(int chid);
int rtk_stun_update(int chid, int ssid, int type);
int rtk_stun_check(int chid, int ssid, int type);
int rtk_stun_send(int chid, int ssid, int flag);
int rtk_stun_keep_alive(int chid, struct __eXosip_sockaddr *addr);
int eXosip_bind_socket(int *pfd, int port);
#ifdef CONFIG_RTK_VOIP_SIP_TLS
int eXosip_create_TCP_socket(int *pfd, int port, int nPort, int proxyIndex);
int eXosip_check_tls_proxy(int nPort, char *proxy);
TRANSPORT_TYPE eXosip_get_transport_type(osip_message_t *sip);
int eXosip_get_TCP_status(int fd);
int eXosip_find_TCP_socket_form_proxy(int nPort, char *proxy);
proxy_info exosip_get_proxy_info(int nPort, char *proxy, int port);
#endif /*CONFIG_RTK_VOIP_SIP_TLS*/
#endif

#ifdef SIP_PING
int rtk_ping_init(int chid, char *from, char *to, osip_list_t *routes);
int rtk_ping_close(int chid);
int rtk_ping_send(int chid);
int rtk_ping_update(int chid, const char *pinged_ip, const char *pinged_port);
int rtk_ping_check(int chid);
#endif


eXosip_reg_t *eXosip_reg_find(int rid);

jauthinfo_t *eXosip_find_authentication_info(const char *username, const char *realm);

#ifdef __cplusplus
}
#endif

#endif
