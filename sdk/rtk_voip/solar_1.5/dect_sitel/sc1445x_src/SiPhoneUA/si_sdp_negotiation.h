/*
 *-----------------------------------------------------------------------------
 *                                                                             
 *               @@@@@@@      *   @@@@@@@@@           *                                     
 *              @       @             @               *                            
 *              @             *       @      ****     *                                 
 *               @@@@@@@      *       @     *    *    *                              
 *        ___---        @     *       @     ******    *                                 
 *  ___---      @       @     *       @     *         *                             
 *   -_          @@@@@@@  _   *       @      ****     *                               
 *     -_                 _ -                                                     
 *       -_          _ -                                                       
 *         -_   _ -        s   e   m   i   c   o   n   d   u   c   t   o   r 
 *           -                                                                    
 *                                                                              
 * (C) Copyright SiTel Semiconductor BV, unpublished work.
 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:			si_sdp_negotiation.h
 * Purpose:		
 * Created:		07/11/2007
 * By:				YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */
#ifndef SI_SDP_NEGOTIATION_H
#define SI_SDP_NEGOTIATION_H
#include "si_ua_api.h"
#include "si_ua_init.h"
#include "si_ua_fax.h"
 
#define SI_MEDIA_DTMF_PAYLOAD		101
#define SI_MEDIA_SPEEXNB_PAYLOAD	105
#define SI_MEDIA_ILBC_PAYLOAD		111
#define SI_MEDIA_G72632_PAYLOAD		2
#define SI_MEDIA_G729_PAYLOAD		18
 
#define SI_MEDIA_G726WB_PAYLOAD		102
#define SI_MEDIA_SPEEXWB_PAYLOAD	103
#define SI_MEDIA_AMR_PAYLOAD		123
#define SI_MEDIA_AMR_WB_PAYLOAD		124
#define SI_MEDIA_H263_PAYLOAD		34
#define SI_MEDIA_MPEG4_PAYLOAD		98
#define SI_MEDIA_H264_PAYLOAD		40
#define SI_MEDIA_CN_16000_PAYLOAD	125
#define SI_MEDIA_CN_PAYLOAD 13
#define SI_MEDIA_CN_PT_STR "13"


/**
 * Structure for storing the global configuration management.
 * @struct osip_negotiation
 */
  typedef struct _audio_profile
  {
	char payload[8];
	char proto[10];
  	char a_rtpmap[32];
    int valid;
	char a_fmtp[32];
	int isfmtp;
  }audio_profile;

 
typedef struct osip_negotiation osip_negotiation_t;
struct osip_negotiation
  {
    char o_username[16];               /**< username */
    char o_session_id[16];             /**< session identifier */
    char o_session_version[16];        /**< session version */
    char o_session_name[16];			/**< session name */
    char o_nettype[8];                /**< Network Type */
    char o_addrtype[8];               /**< Address type */
    char o_addr[64];                   /**< Address */	// FIX_IPv6

    char c_nettype[8];                /**< Network Type */
    char c_addrtype[8];               /**< Address Type */
    char c_addr[64];                   /**< Address */// FIX_IPv6
    char *c_addr_multicast_ttl;     /**< TTL value for multicast address  */
    char *c_addr_multicast_int;     /**< Nb of address for multicast */

    audio_profile audio_codec[MAX_SUPPORTED_CODECS];       /**< supported audio codec */

	crypto_options crypto_level ;
	crypto_profile tx_encryption_suites[MAX_ENCRYPTION_SUITES];
//	crypto_profile rx_encryption_suite ;

//	int tx_selected_crypto_suite;
 
	char ptime[12];
  //  other_profile *other_codec;       /**< supported application */

    int (*fcn_set_info) (  sdp_message_t *);   /**< callback for info */
    int (*fcn_set_uri) (  sdp_message_t *);    /**< callback for uri */

    int (*fcn_set_emails) (  sdp_message_t *); /**< callback for email */
    int (*fcn_set_phones) (  sdp_message_t *); /**< callback for phones */
    int (*fcn_set_attributes) (  sdp_message_t *, int); /**< callback for attr */
 };
  
typedef struct _CodecInformation_t
{
	int num;
	char *snum;
	char *name;
	char *rate;
}CodecInformation_t;

typedef struct _FmtpInformation_t
{
	int num;
 	char *attr;
}FmtpInformation_t;


extern osip_negotiation_t sdpNegotiationData[];


int eXosip_sdp_negotiation_init(int accountId, osip_negotiation_t *sdpInfo, SICORE* pCore);
char* build_sdp_answer(SICORE *pCore, sicall_t* pCall);
 
int osip_negotiation_sdp_build_offer(sicall_t* pCall, osip_negotiation_t* config, sdp_message_t ** sdp, char *audio_port);
char* eXosip_create_sdpbody_offer(sicall_t* pCall, osip_message_t *invite, osip_negotiation_t * config, char *local_audio_sdp_port, char *public_sdp_port, int inviteflag);
sdp_message_t* eXosip_create_sdp_offer(sicall_t* pCall, osip_message_t *invite, osip_negotiation_t * config, char *local_audio_sdp_port, char *public_sdp_port, int inviteflag);
 
sdp_message_t * create_sdp_answer_4invite_on_hold(sicall_t *pCall,osip_negotiation_t *sdpInfo, SICORE* pCore, sdp_message_t * remote);

char* create_sdp_answer_from_remote(sicall_t* pCall, osip_negotiation_t *sdpInfo, SICORE* pCore, sdp_message_t * remote );
 
char* create_sdp_answer_no_remote(sicall_t* pCall, osip_negotiation_t *sdpInfo, SICORE* pCore, char *LocalAudioBuffer, char *PublicAudioBuffer);

int create_sdp_hold_on(sdp_message_t * sdp, int type);
int create_sdp_hold_off(sdp_message_t * sdp, int type);

int sdp_partial_clone (sicall_t* pCall,   osip_negotiation_t * sdpInfo, sdp_message_t * remote,   sdp_message_t ** dest);
 
int osip_negotiation_find_audio_payload_by_mime1(audio_profile  *pAudioCodec, osip_negotiation_t * sdpInfo, char *mime, char *fmtp);
int osip_negotiation_find_audio_payload_by_mime(audio_profile  *pAudioCodec, osip_negotiation_t * sdpInfo, char *mime, char *fmtp);
//audio_profile  * osip_negotiation_find_audio_payload_by_mime(osip_negotiation_t * sdpInfo, char *mime, char* fmtp);
audio_profile  * osip_negotiation_find_other_payload(osip_negotiation_t * config, char *mime);
audio_profile  * osip_negotiation_get_audiocodec(osip_negotiation_t * sdpInfo, int i);

int
osip_negotiation_find_audio_payload (audio_profile  *pAudioCodec, osip_negotiation_t * sdpInfo,char *payload);
//audio_profile *osip_negotiation_find_audio_payload (osip_negotiation_t * config,char *payload);

accept_options sdp_get_crypto_options(osip_negotiation_t *sdpInfo, sdp_message_t * remote, int media_pos,char *in_protocol,si_ua_str_t **out_protocol, crypto_parameters_t* pcrypto_params);

int sdp_confirm_media (sicall_t* pCall, osip_negotiation_t * config,  sdp_message_t * remote, sdp_message_t ** dest);
int sdp_get_first_supported(osip_negotiation_t *sdpInfo, sdp_message_t * remote,audio_stream_parameters *pStreamParameters);
int sdp_get_first_crypto_option(osip_negotiation_t *sdpInfo, sdp_message_t * remSDP, crypto_parameters_t *pCryptoParameters);

int sdp_get_fax_params(ata_t38_sdp_params_type *fax_sdp_params, sdp_message_t * sdp, audio_stream_parameters *pStreamParameters);
int sdp_get_first_requested(osip_negotiation_t *sdpInfo, sdp_message_t * remote);
int sdp_message_m_payload_mime_get(sdp_message_t * sdp, int pos_media, int payloadnumber, char *result,char*fmtp, const size_t result_size);
int eXosip_sdp_accept_audio_codec(char *port, char *number_of_port, int audio_qty, char *payload);
int eXosip_sdp_accept_other_codec(char *type, char *port, char *number_of_port, char *payload);
char *eXosip_sdp_get_audio_port( sicall_t* pCall);
char *eXosip_sdp_get_image_port(sicall_t* pCall);

char *eXosip_sdp_get_other_port(int pos_media);
int sdp_message_get_ptime(sdp_message_t * sdp);
int validatePtimeValue(int ptimevalue);
char *GetCodecName(int cnum);
char *GetCodecRate(int cnum);
char *GetFmtpAttr(int cnum);
int GetCodecType(char *snum);
int fmtpcompare( char *codec,char *myfmtp, char* fmtp); 

 #endif //SI_SDP_NEGOTIATION_H
