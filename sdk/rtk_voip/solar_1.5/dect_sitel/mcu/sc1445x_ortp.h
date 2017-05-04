
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
 * File:		sc1445x_ortp.h
 * Purpose:		
 * Created:		Dec 2007
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef sc1445x_ORTP_H
#define sc1445x_ORTP_H
  
#include "sc1445x_mcu_block.h"
 
#include <ortp/ortp.h>

#define MAX_RTP_SIZE	1500
int sc1445x_ortp_init(void); 
RtpSession * sc1445x_ortp_stream_open (unsigned char *lAddress,unsigned char *rAddress, unsigned int lport, unsigned int rport,int dynamicPayloadType,int payloadType, int jitter, int tos );
void sc1445x_ortp_stream_close(RtpSession *rtps);
int sc1445x_ortp_send(RtpSession * rtps, unsigned char * buffer, int len, unsigned int timestamp);
int sc1445x_ortp_receive(RtpSession * rtps, unsigned int rtp_rxts, unsigned char * buffer, int len, int *have_more);
int sc1445x_ortp_send_dtmf_event(RtpSession * rtps,char dtmf, int duration, int volume, unsigned int );


RtpSession * sc1445x_ortp_create_duplex_session( int locport, int ipv6);
void sc1445x_ortp_close_session(RtpSession * rtps);
void ssrc_cb(RtpSession *session);
   
///////RECEIPTION PATH
///////CALLBACK ROUTINES
int sc1445x_ortp_set_callback_routines(RtpSession *rtps);
int sc1445x_ortp_remove_callback_routines(RtpSession *rtps);

void sc1445x_ortp_on_dtmf_received(RtpSession *s, int dtmf, void * user_data);
void sc1445x_ortp_on_payload_type_changed(RtpSession *session, unsigned long data);
void sc1445x_ortp_on_network_error(RtpSession *session, unsigned long data);
void sc1445x_ortp_on_timestamp_jump(RtpSession *rtps, unsigned long data,unsigned long userdata);
void sc1445x_ortp_get_statistics(void);


#endif //sc1445x_ORTP_H
