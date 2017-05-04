
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
 * File:		sc1445x_alsa.h
 * Purpose:		
 * Created:		Dec 2008
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef sc1445x_SRTP_H
#define sc1445x_SRTP_H
#include "../sc1445x_src/common/si_phone_api.h"
#include <srtp/srtp.h>
#include "sc1445x_mcu_api_defs.h"

 typedef struct _srtp_session_t
{
	srtp_t	srtp_tx_policy;
	srtp_t	srtp_rx_policy;
	void	*srtp_transport;
	void	*srtcp_transport;
	ortp_socket_t rtp_sockfd;
	ortp_socket_t rtcp_sockfd;
  }srtp_session_t;

int srtp_initiate_lib(void);
void srtp_event_cb(srtp_event_data_t *data);
void* srtp_alloc_session(int size);
void  srtp_free_session(void*);
int srtp_get_crypto_suite_id(char *cr_name) ;
void srtp_destroy_session(void* pSrtpSession);  

int srtp_create_session( RtpSession *pSession, void** pSSession, sc1445x_mcu_srtp_params_t *pStrpParams)  ;

int srtp_syslog_(err_status_t err_code,  char* more_info);
int srtp_debug_info(char *custom_info, char *info);
int srtp_perror(err_status_t err_code,  char* more_info);
int srtp_set_transport_function(srtp_session_t *pSecSession, RtpSession *session,void **rtp_tr, void **rtcp_tr);

#endif //sc1445x_SRTP_H
