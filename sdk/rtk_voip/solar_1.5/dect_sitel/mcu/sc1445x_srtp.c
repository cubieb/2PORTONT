
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
 * (C) Copyright SiTel Semiconductor BV, unpublished work.file: 

 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:		 		 sc1445x_srtp.c
 * Purpose:		 		 
 * Created:		 		 Feb 2009
 * By:		 		 	 YP
 * Country:		 		 Greece
 *
 *-----------------------------------------------------------------------------
 */

/*========================== Include files ==================================*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include "sc1445x_mcu_debug.h"
 
#include <assert.h>    
#include <ortp/rtpsession.h>
#include <sys/socket.h>
//RFC 4568
#include "sc1445x_srtp.h"
#include <srtp/crypto_types.h>

#define _SRTP_STDERR_DEBUG_

#ifdef _SRTP_FILE_DEBUG_
  #define SVOIP_PERROR(err_code, more_info) srtp_perror(err_code, more_info)
  int SRTP_FILE_LOG = 1;
#elif defined _SRTP_STDERR_DEBUG_
  #define SRTP_PERROR(err_code, more_info) srtp_perror(err_code, more_info)
  int SRTP_FILE_LOG = 0;
#elif defined _SRTP_SYSLOG_DEBUG_
  #define SRTP_PERROR(err_code, more_info) syslog_srtp(err_code, more_info); 
  int SRTP_FILE_LOG = 0;
#else
  #define SRTP_PERROR(a,b)
  int SRTP_FILE_LOG = 0;
#endif

 char SRTP_INFO_LOG[] = "/mnt/flash/srtp_info.log";
  
typedef struct crypto_suite
{
    char				*name;
    int					cipher_type;
    unsigned int		cipher_key_len;
    int					auth_type;
    unsigned int		auth_key_len;
    unsigned int		srtp_auth_tag_len;
    unsigned int		srtcp_auth_tag_len;
    sec_serv_t			service;
} crypto_suite;

static crypto_suite crypto_suites[] = {
    /* plain RTP/RTCP (no cipher & no auth) */
    {"NULL", NULL_CIPHER, 0, NULL_AUTH, 0, 0, 0, sec_serv_none},
     /* cipher AES_CM, auth HMAC_SHA1, auth tag len = 10 octets */
    {"AES_CM_128_HMAC_SHA1_80", AES_128_ICM, 30, HMAC_SHA1, 20, 10, 10, 
	sec_serv_conf_and_auth},
     /* cipher AES_CM, auth HMAC_SHA1, auth tag len = 4 octets */
    {"AES_CM_128_HMAC_SHA1_32", AES_128_ICM, 30, HMAC_SHA1, 20, 4, 10,
	sec_serv_conf_and_auth},
     /* 
     * F8_128_HMAC_SHA1_8 not supported by libsrtp?
     * {"F8_128_HMAC_SHA1_8", NULL_CIPHER, 0, NULL_AUTH, 0, 0, 0, sec_serv_none}
     */
};

static int  srtp_lib_initialized = 0; 

/* SSRC collisions and key expirations handling */
void srtp_event_cb(srtp_event_data_t *data)
{
	switch (data->event) {
	case event_ssrc_collision:
		srtp_debug_info("event_ssrc_collision", __FUNCTION__);
		break;
	case event_key_soft_limit:
		srtp_debug_info("event_key_soft_limit", __FUNCTION__);
		break;
	case event_key_hard_limit:
		srtp_debug_info("event_key_hard_limit", __FUNCTION__);
		break;
	case event_packet_index_limit:
		srtp_debug_info("event_packet_index_limit", __FUNCTION__);
		break;
	}
}
 
int srtp_initiate_lib(void)
{
 	if (!srtp_lib_initialized)
	{
	 	err_status_t err;
		err = srtp_init();
		if (err != err_status_ok) { 
			srtp_perror(err, __FUNCTION__);
  			return -1;
		}

		srtp_install_event_handler(srtp_event_cb);
	}
	
  srtp_lib_initialized=1;

	return 0;
}

int srtp_get_crypto_suite_id(char *cr_name) 
{
	int i ;
    int num = sizeof(crypto_suites)/sizeof(crypto_suites[0]);
	for (i=1;i<num;i++)
	{
		if (crypto_suites[i].name==NULL) continue;
  		if (!memcmp( crypto_suites[i].name, cr_name, strlen(crypto_suites[i].name)))
		{
			return i;
		}
	}
	return 0;
}
  
int  srtp_sendto (struct _RtpTransport *t, const  void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen); 
int  srtp_recvfrom(struct _RtpTransport *t, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen) ;

unsigned char buf_tx[1024];
int  srtp_sendto (struct _RtpTransport *t, const  void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) 
{
	int error;
	RtpSession *session = (RtpSession *)t->data;
	ortp_socket_t sockfd=  session->rtp.socket; 
	srtp_session_t *pSrtpSession = rtp_session_get_data(session);
  	memcpy(buf_tx, (unsigned char*)buf,len );

     error =srtp_protect(pSrtpSession->srtp_tx_policy, buf_tx, &len);  
	if (!error)
	{
		error = sendto (sockfd,buf_tx, (int)len, 0,to,tolen);
	}
   	return error;
}

int srtp_recvfrom(struct _RtpTransport *t, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen) 
{ 
	int error=0;
	int slen=len; 
	int ret;
 	RtpSession *session = (RtpSession *)t->data;
	ortp_socket_t sockfd=  session->rtp.socket; 
	srtp_session_t *pSrtpSession = rtp_session_get_data(session);   
 	error = recvfrom(sockfd, buf,len, 0, (struct sockaddr *) from,fromlen);
	if (error>0)
	{
		//assert( (((long)buf) & 0x03)==0 );
 		slen=error;
  		ret = srtp_unprotect(pSrtpSession->srtp_rx_policy, (unsigned char*)buf, &slen);
  		return  slen;
  	}else return 0;
}
int srtcp_sendto (struct _RtpTransport *t, const  void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen); 
int srtcp_recvfrom(struct _RtpTransport *t, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen) ;
int srtcp_sendto (struct _RtpTransport *t, const  void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen)
{
	int error;;
	RtpSession *session = (RtpSession *)t->data;
	srtp_session_t *pSrtpSession = rtp_session_get_data(session);
	ortp_socket_t sockfd=  pSrtpSession->rtcp_sockfd; 
//	ortp_socket_t sockfd=  session->rtcp.socket; 

  	memcpy(buf_tx, (unsigned char*)buf,len );
	if (sockfd<=0) {
   	  DPRINT( "[%s] Invalid srtp socket  \n", __FUNCTION__);	 
 	  return  -1;
	}

	error =srtp_protect_rtcp(pSrtpSession->srtp_tx_policy, buf_tx, &len);  
 	if (!error)
	{
		error = sendto (sockfd,buf_tx, (int)len, 0,to,tolen);
 	}
   	return error;

}
int srtcp_recvfrom(struct _RtpTransport *t, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen) 
{
	int error=0;
	int slen=len; 
	int ret;
 	RtpSession *session = (RtpSession *)t->data;
	srtp_session_t *pSrtpSession = rtp_session_get_data(session);   
	ortp_socket_t sockfd=  pSrtpSession->rtcp_sockfd; 
	//ortp_socket_t sockfd=  session->rtp.socket; 

 	error = recvfrom(sockfd, buf,len, 0, (struct sockaddr *) from,fromlen);
	if (error>0)
	{
		//assert( (((long)buf) & 0x03)==0 );
 		slen=error;
  		ret = srtp_unprotect_rtcp(pSrtpSession->srtp_rx_policy, (unsigned char*)buf, &slen);
    		return  slen;
  	}else return 0;
}
   
int srtp_set_transport_function(srtp_session_t *pSecSession, RtpSession *session,void **rtp_tr, void **rtcp_tr)
{
	RtpTransport *rtpt;  
	RtpTransport *rtcpt;  

 	rtpt=ortp_new(RtpTransport,1);
	rtpt->data=session;
	rtpt->t_getsocket=NULL;
	rtpt->t_sendto=srtp_sendto;
	rtpt->t_recvfrom=srtp_recvfrom;

	rtcpt=ortp_new(RtpTransport,1);
	rtcpt->data=session;
	rtcpt->t_getsocket=NULL;
	rtcpt->t_sendto=srtcp_sendto;
	rtcpt->t_recvfrom=srtcp_recvfrom;

	*rtp_tr =(void*) rtpt;
	*rtcp_tr = (void*)rtcpt;

	pSecSession->rtp_sockfd = rtp_session_get_rtp_socket(session);
	pSecSession->rtcp_sockfd = rtp_session_get_rtcp_socket(session);

    rtp_session_set_transports(session, rtpt, rtcpt);
  	return 0;
}
 
 void* srtp_alloc_session(int size)
 {
	return malloc(size);
 }
 void srtp_free_session(void* session)
 {
		free(session);
 }
void srtp_destroy_session(void* pSrtpSession)  
{
	int ret;
	srtp_session_t *pSrtp =(srtp_session_t *)pSrtpSession ;
	RtpTransport *t=(RtpTransport *)pSrtp->srtp_transport ;
   	RtpSession *session = (RtpSession *)t->data;
 
	if ( pSrtp)
	{
		if (session)
			rtp_session_set_transports(session, NULL, NULL);

		ret= srtp_dealloc(pSrtp->srtp_rx_policy);
  		ret = srtp_dealloc(pSrtp->srtp_tx_policy);
	 
		ortp_free(pSrtp->srtp_transport);
		ortp_free(pSrtp->srtcp_transport);

		srtp_free_session((void*)pSrtp);
  	}  
}

int srtp_create_session( RtpSession *pSession, void** pSSession, sc1445x_mcu_srtp_params_t *pStrpParams)  
{
	int cr_id, au_id ;
 	err_status_t err;
	srtp_session_t *pSrtpSession;//=(srtp_session_t *)pSSession;
 	srtp_policy_t    tx_policy;
	srtp_policy_t    rx_policy;

	if (!pStrpParams->cr_au_flag)
		return 0;
  
	pSrtpSession = (srtp_session_t *)srtp_alloc_session(sizeof(srtp_session_t));
	if (pStrpParams==NULL)
 		return -1;
   
	if (srtp_initiate_lib())
	{
		srtp_free_session((void*)pSrtpSession);
		return -1;
	}
   
	// find crypto suite index 
	cr_id = au_id =  srtp_get_crypto_suite_id(pStrpParams->tx_suite);
  if (pStrpParams->cr_au_flag & MCU_SRTP_NO_ENCRYPTION)
  	cr_id = 0;
	if (pStrpParams->cr_au_flag  & MCU_SRTP_NO_AUTHENTICATION)
 	 	au_id = 0;
	if (pStrpParams->tx_key_len != crypto_suites[cr_id].cipher_key_len ||
		pStrpParams->rx_key_len != crypto_suites[cr_id].cipher_key_len){
 	 	goto _error;
	}

	// Init transmit direction 
  memset(&tx_policy, 0, sizeof(srtp_policy_t));
 	if (cr_id && au_id)
		tx_policy.rtp.sec_serv    = sec_serv_conf_and_auth;
    else if (cr_id)
		tx_policy.rtp.sec_serv    = sec_serv_conf;
    else if (au_id)
		tx_policy.rtp.sec_serv    = sec_serv_auth;
    else
		tx_policy.rtp.sec_serv    = sec_serv_none;

    tx_policy.key		    = (unsigned char*)pStrpParams->tx_key   ;
    tx_policy.ssrc.type	    = ssrc_any_outbound;//ssrc_specific //uncomment rtp_session_set_ssrc(pRtpSession,0); on sc1445x_ortp_stream_open
    tx_policy.ssrc.value	= 0;

	tx_policy.rtp.cipher_type	    = crypto_suites[cr_id].cipher_type;
    tx_policy.rtp.cipher_key_len  = crypto_suites[cr_id].cipher_key_len;
    tx_policy.rtp.auth_type	    = crypto_suites[au_id].auth_type;
    tx_policy.rtp.auth_key_len    = crypto_suites[au_id].auth_key_len;
    tx_policy.rtp.auth_tag_len    = crypto_suites[au_id].srtp_auth_tag_len;
    tx_policy.rtcp		    = tx_policy.rtp;
    tx_policy.rtcp.auth_tag_len   = crypto_suites[au_id].srtcp_auth_tag_len;
    tx_policy.next		    = NULL;

    err = srtp_create(&pSrtpSession->srtp_tx_policy, &tx_policy);
    if (err != err_status_ok) {
  		goto _error;
    }
	// Init receive direction 
     memset(&rx_policy, 0, sizeof(srtp_policy_t));
	
	cr_id = au_id = srtp_get_crypto_suite_id(pStrpParams->rx_suite);
	if (!cr_id)  
 		while (1);
  
	if (cr_id && au_id)
		rx_policy.rtp.sec_serv    = sec_serv_conf_and_auth;
    else if (cr_id)
		rx_policy.rtp.sec_serv    = sec_serv_conf;
    else if (au_id)
		rx_policy.rtp.sec_serv    = sec_serv_auth;
    else
		rx_policy.rtp.sec_serv    = sec_serv_none;
 
	rx_policy.key		    = (unsigned char*)pStrpParams->rx_key  ;
    rx_policy.ssrc.type	    = ssrc_any_inbound;
    rx_policy.ssrc.value	= 0;

	rx_policy.rtp.cipher_type	    = crypto_suites[cr_id].cipher_type;
    rx_policy.rtp.cipher_key_len  = crypto_suites[cr_id].cipher_key_len;
    rx_policy.rtp.auth_type	    = crypto_suites[au_id].auth_type;
    rx_policy.rtp.auth_key_len    = crypto_suites[au_id].auth_key_len;
    rx_policy.rtp.auth_tag_len    = crypto_suites[au_id].srtp_auth_tag_len;

    rx_policy.rtcp					=rx_policy.rtp;
    rx_policy.rtcp.auth_tag_len   = crypto_suites[au_id].srtcp_auth_tag_len;
    rx_policy.next		    = NULL;

    err = srtp_create(&pSrtpSession->srtp_rx_policy, &rx_policy);
    if (err != err_status_ok) {
 		goto _error;
    }
	 srtp_set_transport_function(pSrtpSession, pSession, &pSrtpSession->srtp_transport,&pSrtpSession->srtcp_transport);
 	rtp_session_set_data(pSession, (void*)pSrtpSession);
 	*pSSession= pSrtpSession;
  	return 0;
_error:
 
	ortp_free(((srtp_session_t *)pSrtpSession)->srtp_transport);		
	ortp_free(((srtp_session_t *)pSrtpSession)->srtcp_transport);		
	srtp_free_session((void*)pSrtpSession);
 	return -1;
}

/* Syslog messages */
#include "../syslog/syslog.h"


/* Syslog Function for srtp error/info messages */

char* srtp_error_message[] =
{
  "nothing to report",                        /**= 0   < err_status_ok           */
  "unspecified failure",                      /**= 1   < err_status_fail         */
  "unsupported parameter",                    /**= 2,  < err_status_bad_param    */
  "couldn't allocate memory",                 /**= 3,  < err_status_alloc_fail   */
  "couldn't deallocate properly",             /**= 4,  < err_status_dealloc_fail */
  "couldn't initialize",                      /**= 5,  < err_status_init_fail    */
  "can't process as much data as requested",  /**= 6,  < err_status_terminus     */
  "authentication failure",                   /**= 7,  < err_status_auth_fail    */
  "cipher failure",                           /**= 8,  < err_status_cipher_fail  */
  "replay check failed (bad index)",          /**= 9,  < err_status_replay_fail  */
  "replay check failed (index too old)",      /**= 10, < err_status_replay_old   */
  "algorithm failed test routine",            /**= 11, < err_status_algo_fail    */
  "unsupported operation",                    /**= 12, < err_status_no_such_op   */
  "no appropriate context found",             /**= 13, < err_status_no_ctx       */
  "unable to perform desired validation",     /**= 14, < err_status_cant_check   */
  "can't use key any more",                   /**= 15, < err_status_key_expired  */
  "error in use of socket",                   /**= 16, < err_status_socket_err   */
  "error in use POSIX signals",               /**= 17, < err_status_signal_err   */
  "nonce check failed",                       /**= 18, < err_status_nonce_bad    */
  "couldn't read data",                       /**= 19, < err_status_read_fail    */
  "couldn't write data",                      /**= 20, < err_status_write_fail   */
  "error pasring data",                       /**= 21, < err_status_parse_err    */
  "error encoding data",                      /**= 22, < err_status_encode_err   */
  "error while using semaphores",             /**= 23, < err_status_semaphore_err*/
  "error while using pfkey ",                 /**= 24, < err_status_pfkey_err    */

};

int srtp_syslog_(err_status_t err_code,  char* more_info)
{
  if (err_code > err_status_pfkey_err)
    syslog(LOG_ERR, "SRTP error: Unknown Error Code");
  else 
  {
    syslog(LOG_ERR, "SRTP error: ");
    syslog(LOG_ERR, srtp_error_message[err_code]);
  }

  if(!(more_info == ""))
    syslog(LOG_ERR, more_info);

  syslog(LOG_ERR, "\n");

  return 0;
}

/* Function that prints the srtp error to either stderr or file */
/* File is given by SRTP_FILE_LOG */

int srtp_perror(err_status_t err_code,  char* more_info)
{
	FILE *fp = stderr;
 
	if (err_code<0) return 0;
 	if (SRTP_FILE_LOG)
	{
		fp = fopen(SRTP_INFO_LOG, "w");
		if (fp == NULL){
			fp = stderr;
			fprintf(fp, "SRTP error: Log File was not found \n");
			fprintf(fp, "SRTP error: Redirecting errors to stderr \n");
		} 
		fprintf(fp, "%s", "SRTP info: Start logging \n\n");
 	}
 
  /* err_status_pfkey_err is the last error message */ 
	if (err_code > err_status_pfkey_err)
		fprintf(fp, "SRTP Error: Unknown Error Code\n");
	if (more_info == NULL)
		fprintf(fp, "SRTP Error: %s\n", srtp_error_message[err_code]);
	else
		fprintf(fp, "SRTP Error: %s. [%s]\n", srtp_error_message[err_code], more_info);

  if (fp != stderr) 
    fclose(fp);
 
  return 0;
}


/* Function that prints the srtp error to either stderr or file */
/* File is given by SRTP_FILE_LOG */
int srtp_debug_info(char *custom_info, char *info)
{
	FILE *fp = stderr;

	if (SRTP_FILE_LOG)
	{
		fp = fopen(SRTP_INFO_LOG, "w");

		if (fp == NULL){
			fp = stderr;
			fprintf(fp, "SRTP info: Log File was not found \n");
			fprintf(fp, "SRTP info: Redirecting info to stdout \n");
		} 
		fprintf(fp, "%s", "SRTP info: Start logging \n\n");
	}
 
	fprintf(fp, "SRTP Info: %s [%s] \n", custom_info, info);

   if (fp!=stderr) 
    fclose(fp);
  
  return 0;
}

