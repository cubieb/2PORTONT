/*
  The mediastreamer library aims at providing modular media processing and I/O
	for linphone, but also for any telephony application.
  Copyright (C) 2001  Simon MORLAT simon.morlat@linphone.org
  										
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#if 0

#include "../config.h"
#include "mediastream.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include "eXosip.h"

#ifndef __FEDORA_COMPLIER
#include "sip_interface.h"
#	ifdef PLATFORM_865X
#	include <board.h>
#	endif
#endif

#ifndef __FEDORA_COMPLIER
#ifdef PLATFORM_865X
extern romeCfgParam_t * pRomeCfgParam ;
#endif
extern void get_wan_ip_address( char * ip_ptr ) ;
#endif

static gboolean audio_stream_start_full( guint32 chid , guint32 ssid , RtpProfile * profile , int locport , char * remip ,
								int remport , int payload , int jitt_comp )
{
	PayloadType * pt ;
	payloadtype_config_t codec_config ;
	rtp_config_t gdsprtp_config ;
	unsigned short src_port , dst_port ;

	/* creates the couple of encoder/decoder */
	pt = rtp_profile_get_payload( profile , payload ) ;
	if( pt == NULL )
		return FALSE ;

#ifndef __FEDORA_COMPLIER

#ifdef USE_CURRENT_CONTACT
	{
		char *local_ip = NULL;

		eXosip_get_localip_for(remip, &local_ip);
		src_ip = inet_addr(local_ip);
		g_free(local_ip);
	}
#else
	if( src_ip == 0 )
	{
		char ip_str[20] ;
		get_wan_ip_address( ip_str ) ;
		g_message( "ip_str = %x" , ip_str ) ;
		src_ip = inet_addr( ip_str ) ;
	}
#endif
	dst_ip = inet_addr( remip ) ;

	g_message( "rtp[%d][%d] : src_ip = %x , dst_ip = %x" , chid , ssid , src_ip , dst_ip ) ;
//	rtk_SetRtpSessionState( chid , ssid , rtp_session_sendrecv ) ;

	codec_config.chid = chid ;
	codec_config.sid = ssid ;
	if( strcmp( pt->mime_type , "PCMA" ) == 0 )
		codec_config.uPktFormat = rtpPayloadPCMA ;
	else if( strcmp( pt->mime_type , "PCMU" ) == 0 )
		codec_config.uPktFormat = rtpPayloadPCMU ;
	else if( strcmp( pt->mime_type , "G723" ) == 0 ) 
		codec_config.uPktFormat = rtpPayloadG723 ;
	else if( strcmp( pt->mime_type , "G729" ) == 0 ) 
		codec_config.uPktFormat = rtpPayloadG729 ;

	codec_config.nG723Type = 0 ;
	codec_config.nFramePerPacket = 1 ;
	codec_config.bVAD = FALSE ;
//	rtk_SetRtpPayloadType( &codec_config ) ;

	gdsprtp_config.isTcp = FALSE ;
	memcpy( &(gdsprtp_config.remIp) , &dst_ip , 4 ) ;
	dst_port = ( unsigned short )remport ;
	memcpy( &(gdsprtp_config.remPort) , &dst_port , 2 ) ;
	memcpy( &(gdsprtp_config.extIp) , &src_ip , 4 ) ;
	src_port = ( unsigned short )locport ;
	memcpy( &(gdsprtp_config.extPort) , &src_port , 2 ) ;
	gdsprtp_config.chid = chid ;
	gdsprtp_config.sid = ssid ;

	rtk_SetRtpConfig( &gdsprtp_config ) ;
	rtk_SetRtpPayloadType( &codec_config ) ;
	rtk_SetRtpSessionState( chid , ssid , rtp_session_sendrecv ) ;
#endif
	return TRUE ;
}

gboolean audio_stream_start_with_dsp( guint32 chid , guint32 ssid , RtpProfile * prof , int locport ,
									char * remip , int remport , int profile , int jitt_comp )
{
	return audio_stream_start_full( chid , ssid , prof , locport , remip , remport , profile , jitt_comp ) ;
}

gint audio_stream_send_dtmf(gboolean stream, gchar dtmf)
{
	return 0;
}

#endif
