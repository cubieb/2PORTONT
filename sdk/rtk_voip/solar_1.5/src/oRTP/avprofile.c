/*
  The oRTP library is an RTP (Realtime Transport Protocol - rfc1889) stack.
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

#include "voip_flash.h"
#include "payloadtype.h"

char offset127=127; 
char offset0xD5=0xD5; 
char offset0=0;

#define TYPE(val) type: (val)
#define CLOCK_RATE(val) clock_rate: (val)
#define NORMAL_BITRATE(val) normal_bitrate: (val)
#define MIME_TYPE(val) mime_type: (val)
#define FMTP(val) FMTP : (val)

PayloadType pcmu8000={
	TYPE( PAYLOAD_AUDIO_CONTINUOUS),
	CLOCK_RATE( 8000),
	NORMAL_BITRATE( 64000),
	MIME_TYPE ("PCMU")
};

PayloadType pcma8000={
	TYPE( PAYLOAD_AUDIO_CONTINUOUS),
	CLOCK_RATE(8000),
	NORMAL_BITRATE( 64000),
	MIME_TYPE ("PCMA")
};

PayloadType payload_type_g7231=
{
	TYPE( PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	NORMAL_BITRATE( 6300),
	MIME_TYPE ("G723")
};

PayloadType payload_type_g729={
	TYPE( PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	NORMAL_BITRATE( 8000),
	MIME_TYPE ("G729")
};

PayloadType payload_type_g726_16={
	TYPE( PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	NORMAL_BITRATE( 16000),
	MIME_TYPE ("G726-16")
};

PayloadType payload_type_g726_24={
	TYPE( PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	NORMAL_BITRATE( 24000),
	MIME_TYPE ("G726-24")
};

PayloadType payload_type_g726_32={
	TYPE( PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	NORMAL_BITRATE( 32000),
	MIME_TYPE ("G726-32")
};

PayloadType payload_type_g726_40={
	TYPE( PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	NORMAL_BITRATE( 40000),
	MIME_TYPE ("G726-40")
};

PayloadType payload_type_gsm_fr={
	TYPE( PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	NORMAL_BITRATE( 26400),
	MIME_TYPE ("GSM")
};

PayloadType telephone_event={
    TYPE(PAYLOAD_AUDIO_PACKETIZED),
    CLOCK_RATE(8000),
	NORMAL_BITRATE(8000),
    MIME_TYPE("telephone-event")
};

PayloadType payload_type_rtp_redundant={
    TYPE(PAYLOAD_AUDIO_PACKETIZED),
    CLOCK_RATE(8000),
	NORMAL_BITRATE(8000),
    MIME_TYPE("red")
};

PayloadType payload_type_iLBC={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	NORMAL_BITRATE(15200),
	MIME_TYPE("iLBC")
};

PayloadType payload_type_g722={
	TYPE(PAYLOAD_AUDIO_CONTINUOUS),
	CLOCK_RATE(8000),
	NORMAL_BITRATE(64000),
	MIME_TYPE("G722")
};

PayloadType payload_type_SPEEX_NB={
	TYPE(PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	NORMAL_BITRATE(8000),
	MIME_TYPE("SPEEX")
};

#ifdef DYNAMIC_PAYLOAD

const profile_list_t profileList[] = {
	{	/* G711 u-law */
		_CODEC_G711U,
		rtpPayloadPCMU,
		&pcmu8000,
		1
	},
	{	/* G711 a-law */
		_CODEC_G711A,
		rtpPayloadPCMA,
		&pcma8000,
		1
	},
	{	/* G729 */
		_CODEC_G729,
		rtpPayloadG729,
		&payload_type_g729,
		1
	},
	{	/* G723 */
		_CODEC_G723,
		rtpPayloadG723,
		&payload_type_g7231,
		3
	},
	{	/* G726 16k */
		_CODEC_G726_16,
		rtpPayloadG726_16,
		&payload_type_g726_16,
		1
	},
	{	/* G726 24k */
		_CODEC_G726_24,
		rtpPayloadG726_24,
		&payload_type_g726_24,
		1
	},
	{	/* G726 32k */
		_CODEC_G726_32,
		rtpPayloadG726_32,
		&payload_type_g726_32,
		1
	},
	{	/* G726 40k */
		_CODEC_G726_40,
		rtpPayloadG726_40,
		&payload_type_g726_40,
		1
	},
	{	/* GSM-FR */
		_CODEC_GSMFR,
		rtpPayloadGSM,
		&payload_type_gsm_fr,
		1
	},
	{	/* iLBC */
		_CODEC_ILBC,
		rtpPayload_iLBC,
		&payload_type_iLBC,
		1
	},
	{	/* G722 */
		_CODEC_G722,
		rtpPayloadG722,
		&payload_type_g722,
		1
	},
	{	/* SPEEX_NB */
		_CODEC_SPEEX_NB,
		rtpPayload_SPEEX_NB_RATE8,
		&payload_type_SPEEX_NB,
		1
	},
	{	/* telephone-event */
		0,
		rtpPayloadDTMF_RFC2833,
		&telephone_event,
		0
	},
	{	/* RTP redundant */
		0,
		rtpPayloadRtpRedundant,
		&payload_type_rtp_redundant,
		0
	},
	{
		/* end of codec */
		0,
		0,
		NULL,
		0
	}
};

profile_dynamic_pt_t profile_dynamic_pt[] = {
	{
		rtpPayloadDTMF_RFC2833,
		96
	},
	{
		rtpPayload_iLBC,
		97
	},
	{
		rtpPayload_iLBC_20ms,
		98
	},
#ifdef SUPPORT_G7111
	{
		rtpPayloadPCMU_WB,
		99
	},
	{
		rtpPayloadPCMA_WB,
		100
	},
#endif
	{
		rtpPayloadV152_Virtual,
		102
	},
	{
		rtpPayloadG726_16,
		103
	},
	{
		rtpPayloadG726_24,
		104
	},
	{
		rtpPayloadG726_32,
		105
	},
	{
		rtpPayloadG726_40,
		106
	},
	{
		rtpPayloadRtpRedundant,
		121
	},
	{
		/* end of dynamic_pt */
		rtpPayloadUndefined,
		0
	}
};

#else // DYNAMIC_PAYLOAD

#ifdef CONFIG_RTK_VOIP_ILBC
#error "Use iLBC need dynamic payload support!"
#endif

#ifdef CONFIG_RTK_VOIP_SPEEX_NB
#error "Use SPEEX NB need dynamic payload support!"
#endif

#endif // DYNAMIC_PAYLOAD

#ifdef SUPPORT_CODEC_DESCRIPTOR

typedef struct profile_list_s {
	PayloadType *pPayloadType;		/* payload info */
	unsigned int pt;				/* payload type */
} profile_list_t;

const profile_list_t profileList[_CODEC_MAX] = {
	{	/* G711 u-law */
		&pcmu8000,
		rtpPayloadPCMU,
	},
	{	/* G711 a-law */
		&pcma8000,
		rtpPayloadPCMA,
	},
	{	/* G723 */
		&payload_type_g7231,
		rtpPayloadG723,
	},
	{	/* G729 */
		&payload_type_g729,
		rtpPayloadG729,
	},
	{	/* G726 16k */
		&payload_type_g726_16,
		rtpPayloadG726_16,
	},
	{	/* G726 24k */
		&payload_type_g726_24,
		rtpPayloadG726_24,
	},
	{	/* G726 32k */
		&payload_type_g726_32,
		rtpPayloadG726_32,
	},
	{	/* G726 40k */
		&payload_type_g726_40,
		rtpPayloadG726_40,
	},
	{	/* GSM-FR */
		&payload_type_gsm_fr,
		rtpPayloadGSM,
	},
	{	/* iLBC */
		&payload_type_iLBC,
		rtpPayload_iLBC,
	},
	{	/* SPEEX NB */
		&payload_type_SPEEX_NB,
		rtpPayload_SPEEX_NB_RATE8,
	},
};

#define NUM_OF_PROFILE_LIST		( sizeof( profileList ) / sizeof( profileList[ 0 ] ) )

#endif /* SUPPORT_CODEC_DESCRIPTOR */

#ifdef DYNAMIC_PAYLOAD
void AddRtpProfilePayload( RtpProfile *profile )
{
	unsigned int i;
	
	for( i = 0; profileList[i].pPayloadType ; i ++ ) {
		rtp_profile_set_payload( profile, 
								 profileList[ i ].pt, 
								 profileList[ i ].pPayloadType );
	}
}

int dynamic_pt_get(RtpPayloadType pt)
{
	int i;

	for (i=0; profile_dynamic_pt[i].pt != rtpPayloadUndefined; i++)
	{
		if (profile_dynamic_pt[i].pt == pt)
			return profile_dynamic_pt[i].pt_value;
	}

	return -1;
}

int dynamic_pt_set(RtpPayloadType pt, char pt_value)
{
	int i;

	for (i=0; profile_dynamic_pt[i].pt != rtpPayloadUndefined; i++)
	{
		if (profile_dynamic_pt[i].pt == pt)
		{
			if (pt_value < 96 || pt_value > 127)
				return -1;

			profile_dynamic_pt[i].pt_value = pt_value;
			return 0;
		}
	}

	return -1;
}

int dynamic_pt_check(void)
{
	int i, j;

	for (i=0; profile_dynamic_pt[i].pt != rtpPayloadUndefined; i++)
	{
		for (j=i+1; profile_dynamic_pt[j].pt != rtpPayloadUndefined; j++)
		{
			if (profile_dynamic_pt[i].pt_value ==
				profile_dynamic_pt[j].pt_value)
			{
				g_warning("%s: pt%d and pt%d are the same %d\n", __FUNCTION__,
					i, j, profile_dynamic_pt[i].pt_value);
				return -1;
			}
		}
	}

	return 0;
}

#endif

#ifdef SUPPORT_CODEC_DESCRIPTOR
void AddRtpProfilePayload( RtpProfile *profile )
{
	unsigned int i;
	
	for( i = 0; i < NUM_OF_PROFILE_LIST; i ++ ) {
		rtp_profile_set_payload( profile, 
								 profileList[ i ].pt, 
								 profileList[ i ].pPayloadType );
	}
}
#endif /* SUPPORT_CODEC_DESCRIPTOR */

RtpProfile av_profile;

void av_profile_init( RtpProfile *profile )
{
	rtp_profile_clear_all(profile);
	rtp_profile_set_name(profile,"AV profile");

#ifdef DYNAMIC_PAYLOAD
	AddRtpProfilePayload( profile );
#endif

#ifdef SUPPORT_CODEC_DESCRIPTOR
	AddRtpProfilePayload( profile );
#endif
}

void av_profile_destroy( RtpProfile *profile )
{
	g_free(profile->name);
	rtp_profile_clear_all(profile);
}
