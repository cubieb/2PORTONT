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
 * File:		 		 sc1445x_ortp.c
 * Purpose:		 		 
 * Created:		 		 Dec 2007
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
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "sc1445x_ortp.h"
#include "sc1445x_media.h"
    
#include <ortp/ortp.h>
#include <ortp/rtp.h>
#include <ortp/telephonyevents.h>
#include <ortp/rtpsession.h>
#include "sc1445x_mcu_debug.h"
 

int sc1445x_ortp_init(void)
{
	ortp_init();
	
	ortp_scheduler_init();  
	//ortp_set_debug_file("oRTP", NULL);
	//ortp_set_log_level_mask(ORTP_DEBUG|ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR);
   	ortp_set_log_level_mask(0);  	 
 	return 0;
}
void ssrc_cb(RtpSession *session)
{
	;//printf("hey, the ssrc has changed !\n");
}
  
void sc1445x_ortp_stream_close(RtpSession *rtps)
{
 	if (rtps==NULL) return ;
		sc1445x_ortp_remove_callback_routines(rtps);
		sc1445x_ortp_close_session(rtps);
}
extern PayloadType	telephone_event101;
extern PayloadType	pcmu8000;
extern PayloadType ilbc;
extern PayloadType ilbc_20;
RtpSession *sc1445x_ortp_stream_open (unsigned char *lAddress,unsigned char *rAddress, unsigned int lport, unsigned int rport,int dynamicPayloadType,int payloadType, int jitter, int tos )
{
  	RtpSession *pRtpSession;
	int ret;
  	
	pRtpSession= sc1445x_ortp_create_duplex_session(lport, 0);
  
   	if (pRtpSession==NULL)
	{
	   DPRINT("[%s] Unable to create a RTP Session \n ", __FUNCTION__);
	   return NULL;
	}
 
	rtp_session_set_local_addr(pRtpSession,(char*)lAddress , lport);
  	rtp_session_set_remote_addr(pRtpSession,(char*)rAddress, rport);

	if (payloadType>96)
	{
		sc1445x_media_init();
		if (payloadType==98){
 			rtp_profile_set_payload(sc1445x_media_get_av_profile(),dynamicPayloadType, &ilbc);
			rtp_profile_set_payload(sc1445x_media_get_av_profile(),payloadType, &ilbc);
		}
		if (payloadType==99)
		{
			rtp_profile_set_payload(sc1445x_media_get_av_profile(),dynamicPayloadType, &ilbc_20);
			rtp_profile_set_payload(sc1445x_media_get_av_profile(),payloadType, &ilbc_20);
		}
		ret = rtp_session_set_payload_type(pRtpSession,dynamicPayloadType);   

	}else  	ret = rtp_session_set_payload_type(pRtpSession,payloadType);   

   	rtp_session_set_profile(pRtpSession,sc1445x_media_get_av_profile());

  

	if (ret<0) {
		DPRINT("[%s] FAILED TO SET PAYLOAD TYPE \n ", __FUNCTION__);
		return NULL;
	}
  	rtp_session_set_jitter_compensation(pRtpSession,jitter);  
 	rtp_session_enable_adaptive_jitter_compensation(pRtpSession,TRUE);
  	rtp_session_set_dscp(pRtpSession,tos);
 	//rtp_session_enable_adaptive_jitter_compensation(pRtpSession,FALSE);
	//rtp_session_set_jitter_compensation(pRtpSession,jitter);  
   	//  set callback routines
//	rtp_session_set_ssrc(pRtpSession,0);
   	sc1445x_ortp_set_callback_routines(pRtpSession);
	return pRtpSession;	
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////// INITIATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RtpSession * sc1445x_ortp_create_duplex_session( int locport, int ipv6)
{
 	RtpSession *rtps;
 	rtps=rtp_session_new(RTP_SESSION_SENDRECV);

	rtp_session_set_profile(rtps,sc1445x_media_get_av_profile());
  	rtp_session_set_recv_buf_size(rtps,MAX_RTP_SIZE);
  	rtp_session_set_scheduling_mode(rtps,0);
  	rtp_session_set_blocking_mode(rtps,0);
// 	rtp_session_enable_adaptive_jitter_compensation(rtps,TRUE);
// 	rtp_session_set_connected_mode(rtps,TRUE);
	rtp_session_set_symmetric_rtp(rtps,FALSE);
 	return rtps;
}
// Destroys a rtp session.
void sc1445x_ortp_close_session(RtpSession * rtps)
{
	if (rtps==NULL) return;
	rtp_session_destroy (rtps);
}
 ////// TRANSMISSION PATH
int sc1445x_ortp_send(RtpSession * rtps, unsigned char * buffer, int len, unsigned int rtp_ts)
{
  if (rtps==NULL) return -101;
	//rtp_ts+=(unsigned int )160;
	return rtp_session_send_with_ts(rtps, buffer, len, (unsigned int)rtp_ts); 
}

int sc1445x_ortp_send_dtmf_event(RtpSession * rtps, char dtmf, int duration, int volume,unsigned int rtp_ts )
{	
	int ret;
	if (rtps==NULL) return -1;
 	ret = rtp_session_telephone_events_supported(rtps);
	if (ret==-1) {	
		DPRINT("[%s] Rtp Telephone Events not supported \n ", __FUNCTION__ );
 		return -1;
	}
  	ret=rtp_session_send_dtmf(rtps, dtmf, rtp_ts);
	if (ret){
		DPRINT("[%s] Unable to send DTMF Event \n", __FUNCTION__);
		return -1;
	}
	return 0;
 }
/*
void sc1445x_ortp_set_marker(RtpSession * rtps)
{

}
 
void sc1445x_ortp_check_marker(RtpSession * rtps)
{

}
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////RECEIPTION PATH
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int sc1445x_ortp_receive(RtpSession * rtps,unsigned int rtp_rxts, unsigned char * buffer, int len, int *have_more)
{
//	mblk_t *mp;
	int ret;
 	if (rtps==NULL) return -1;
 
    ret = rtp_session_recv_with_ts (rtps, buffer,len, rtp_rxts,have_more); 
 	return ret;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////CALLBACK ROUTINES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	This function provides the way for an application to be informed of various events that
 *	may occur during a rtp session. @signal is a string identifying the event, and @cb is 
 *	a user supplied function in charge of processing it. The application can register
 *	several callbacks for the same signal, in the limit of #RTP_CALLBACK_TABLE_MAX_ENTRIES.
 *	Here are name and meaning of supported signals types:
 *
 *	"ssrc_changed" : the SSRC of the incoming stream has changed.
 *
 *	"payload_type_changed" : the payload type of the incoming stream has changed.
 *
 *	"telephone-event_packet" : a telephone-event rtp packet (RFC2833) is received.
 *
 *	"telephone-event" : a telephone event has occured. This is a high-level shortcut for "telephone-event_packet".
 *
 *	"network_error" : a network error happened on a socket. Arguments of the callback functions are
 *						a const char * explaining the error, an int errno error code and the user_data as usual.
 *
 *	"timestamp_jump" : we have received a packet with timestamp in far future compared to last timestamp received.
 *						The farness of far future is set by rtp_sesssion_set_time_jump_limit()
*/

int sc1445x_ortp_set_callback_routines(RtpSession *rtps)
{
	if (rtps==NULL) return -1;
 
	rtp_session_signal_connect(rtps,"telephone-event",(RtpCallback)sc1445x_ortp_on_dtmf_received, (unsigned long)0);//(unsigned long)stream->dtmfgen);
	rtp_session_signal_connect(rtps,"payload_type_changed",(RtpCallback)sc1445x_ortp_on_payload_type_changed,(unsigned long)rtps);//(unsigned long)stream);
	rtp_session_signal_connect(rtps,"network_error",(RtpCallback)sc1445x_ortp_on_network_error,(unsigned long)0);//(unsigned long)stream);

	// TBD VAD
	 rtp_session_signal_connect(rtps,"timestamp_jump",(RtpCallback)sc1445x_ortp_on_timestamp_jump,(unsigned long)(unsigned long)rtps);//(unsigned long)stream);
    return 0;
}

int sc1445x_ortp_remove_callback_routines(RtpSession *rtps)
{
	if (rtps==NULL) return -1;
  
	rtp_session_signal_disconnect_by_callback(rtps,"telephone-event",(RtpCallback)sc1445x_ortp_on_dtmf_received);
	rtp_session_signal_disconnect_by_callback(rtps,"payload_type_changed",(RtpCallback)sc1445x_ortp_on_payload_type_changed);
	rtp_session_signal_disconnect_by_callback(rtps,"network_error",(RtpCallback)sc1445x_ortp_on_network_error);
	rtp_session_signal_disconnect_by_callback(rtps,"timestamp_jump",(RtpCallback)sc1445x_ortp_on_timestamp_jump);

    return 0;
}
 
int dtmf_tab[16]={'0','1','2','3','4','5','6','7','8','9','*','#','A','B','C','D'};
 void sc1445x_ortp_on_dtmf_received(RtpSession *rtps, int dtmf, void * user_data)
{
//	MSFilter *dtmfgen=(MSFilter*)user_data;
	if (rtps==NULL) return ;

	if (dtmf>15){
		DPRINT("[%s] Unsupported telephone-event type.", __FUNCTION__);
		return;
	}
 }

void sc1445x_ortp_on_payload_type_changed(RtpSession *rtps, unsigned long data)
{  
	int pt;//=(int)rtp_session_get_recv_payload_type((RtpSession *)data);
  	pt=(int)rtp_session_get_recv_payload_type(rtps);
 
 //	AudioStream *stream=(AudioStream*)data;
//	int pt=rtp_session_get_recv_payload_type(stream->session);
//	audio_stream_change_decoder(stream,pt);
}

void sc1445x_ortp_on_timestamp_jump(RtpSession *rtps, unsigned long data, unsigned long userdata)
{
 	if ((RtpSession *)userdata==NULL) return ; 

  	 sc1445x_mcu_scheduler_resync((RtpSession *)userdata);
 
}
void sc1445x_ortp_on_network_error(RtpSession *rtps, unsigned long data)
{
	int pt; 
 	if (rtps==NULL) return ; 
 	pt=(int)rtp_session_get_recv_payload_type(rtps);
}

void sc1445x_ortp_get_statistics(void)
{
	ortp_global_stats_display();
}

 
