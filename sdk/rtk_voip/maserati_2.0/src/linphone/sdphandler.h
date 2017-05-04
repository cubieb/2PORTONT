 /*
  * Linphone is sip (RFC3261) compatible internet phone.
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Lesser General Public
  * License as published by the Free Software Foundation; either
  * version 2.1 of the License, or (at your option) any later version.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Lesser General Public License for more details.
  *
  * You should have received a copy of the GNU Lesser General Public
  * License along with this library; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  */

#ifndef SDP_HANDLER_H
#define SDP_HANDLER_H
#include "eXosip2.h"

#include "osipparser2/sdp_message.h"
#include "linphonecore.h"

#ifdef CONFIG_RTK_VOIP_SRTP
#include "crypto_types.h"
#endif

typedef enum{
	SalStreamSendRecv,
	SalStreamSendOnly,
	SalStreamRecvOnly,
	SalStreamInactive
}SalStreamDir;


typedef struct _sdp_payload
{
	int line;	/* the index of the m= line */
	int pt;		/*payload type */
	int localport;
	int remoteport;
	int b_as_bandwidth;	/* application specific bandwidth */
	char *proto;
	char *c_nettype;
	char *c_addrtype;
	char *c_addr;
	char *c_addr_multicast_ttl;
	char *c_addr_multicast_int;
	char *a_rtpmap;
	char *a_fmtp;
#ifdef SUPPORT_V152_VBD
	char *a_gpmd;
#endif
#ifdef SUPPORT_RTP_REDUNDANT_APP
	int pt_red;	/* real payload type in RTP redundant (linphone_payload_is_supported() fills it) */
#endif
	/*+++++ Added by Jack for SRTP 140108+++++*/
#ifdef CONFIG_RTK_VOIP_SRTP
	char remoteSDPKey[SDP_KEY_LEN];
	char localSDPKey[SDP_KEY_LEN];
	int remoteCryptAlg;
	int localCryptAlg;
#endif /*CONFIG_RTK_VOIP_SRTP*/
	/*-----end-----*/
} sdp_payload_t;

typedef enum _sdp_context_state
{
	SDP_CONTEXT_STATE_INIT,
	SDP_CONTEXT_STATE_NEGOCIATION_OPENED,
	SDP_CONTEXT_STATE_NEGOCIATION_CLOSED
} sdp_context_state_t;

typedef struct _sdp_context
{
	struct _sdp_handler *handler;
	char *localip;
	char *rtp_localip;
	char *username;
	void *reference;
	sdp_message_t *offer;		/* the local sdp to be used for outgoing request */
	char *offerstr;
	sdp_message_t *answer;		/* the local sdp generated from an inc request */
	char *answerstr;
	sdp_message_t *remote;
	int negoc_status;	/* in sip code */
	int incb;
	sdp_context_state_t state;
	int version;
	// loopback mode
	int loopback;
	SalStreamDir remote_dir; //media direction
} sdp_context_t ;

typedef int (*sdp_handler_read_codec_func_t) (struct _sdp_context *,
											sdp_payload_t *);
typedef int (*sdp_handler_write_codec_func_t) (struct _sdp_context *);
/*++added by Jack Chan xx/xx/07for++*/
typedef int (*sdp_handler_read_t38_func_t) (struct _sdp_context *, 
											sdp_payload_t *);
/*--end--*/

typedef struct _sdp_handler
{
	sdp_handler_read_codec_func_t accept_audio_codecs ;	/* from remote sdp */		// linphone_accept_audio_offer
	sdp_handler_write_codec_func_t set_audio_codecs ;	/* to local sdp */			// linphone_set_audio_offer
	sdp_handler_read_codec_func_t get_audio_codecs ;	/* from incoming answer */	// linphone_read_audio_answer
	/*++added by Jack Chan xx/xx/07for++*/
	sdp_handler_read_t38_func_t get_t38_config;			/*from incoming t38 fax */	//linphone_read_t38_answer
	/*--end--*/
} sdp_handler_t ;

char *sdp_message_a_attr_value_get_with_pt(sdp_message_t *sdp,int pos,int pt,char *field);

/* create a context for a sdp negociation: localip is the ip address to be used in the sdp message, can
be a firewall address.
It can be null when negociating for an incoming offer; In that case it will be guessed. */
sdp_context_t *sdp_handler_create_context(sdp_handler_t *handler, const char *localip, const char *username);
void sdp_context_set_user_pointer(sdp_context_t * ctx, void* up);
void *sdp_context_get_user_pointer(sdp_context_t * ctx);
void sdp_context_add_audio_payload( sdp_context_t * ctx, sdp_payload_t * payload);
char * sdp_context_get_offer(sdp_context_t *ctx);
char * sdp_context_get_answer(sdp_context_t* ctx, const char *remote_offer);
char * rcm_sdp_context_get_answer_by_ev(sdp_context_t* ctx, eXosip_event_t *ev);

/* Mandy add for stun support */
char * sdp_context_set_answer ( sdp_context_t *ctx,const char *remote_offer);
int sdp_context_get_status(sdp_context_t* ctx);
void sdp_context_read_answer(sdp_context_t *ctx, const char *remote_answer);
void rcm_sdp_context_read_answer_by_ev(sdp_context_t *ctx, 	eXosip_event_t *ev);
void rcm_sdp_context_read_answer_by_sdp(sdp_context_t *ctx, 	osip_message_t *sip_message);

void sdp_context_free(sdp_context_t *ctx);

int sdp_payload_init (sdp_payload_t * payload);

sdp_message_t *sdp_context_generate_template (sdp_context_t * ctx);
void rcm_set_sdp(osip_message_t *sip,sdp_message_t *msg);


#ifdef CONFIG_RTK_VOIP_SRTP
int sdp_generate_sdes_key(unsigned char* key, int key_len);
#endif /*CONFIG_RTK_VOIP_SRTP*/

#endif
