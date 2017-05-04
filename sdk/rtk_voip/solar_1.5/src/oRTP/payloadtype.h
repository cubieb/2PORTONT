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

#ifndef PAYLOADTYPE_H
#define PAYLOADTYPE_H
#include "rtpport.h"
#include "../linphone/uglib.h"

typedef enum{
	PAYLOAD_TYPE_ALLOCATED = 1
}PayloadTypeFlags;

struct _PayloadType
{
	gint type;
	#define PAYLOAD_AUDIO_CONTINUOUS 0
	#define PAYLOAD_AUDIO_PACKETIZED 1
	#define PAYLOAD_OTHER 3  /* ?? */
	gint clock_rate;
	/* other usefull information */
	gint normal_bitrate;	/*in bit/s */
	char *mime_type;
	char *fmtp;	/*various parameters, as a string */
	PayloadTypeFlags flags;
//	void *user_data;
};

#ifndef PayloadType_defined
#define PayloadType_defined
typedef struct _PayloadType PayloadType;
#endif

#ifdef __cplusplus
extern "C"{
#endif
PayloadType *payload_type_new(void);
PayloadType *payload_type_clone(PayloadType *payload);
void payload_type_destroy(PayloadType *pt);
#ifdef __cplusplus
}
#endif

#define payload_type_set_flag(pt,flag) (pt)->flags|=(flag)
#define payload_type_unset_flag(pt,flag) (pt)->flags&=(~flag)

#define RTP_PROFILE_MAX_PAYLOADS 128

typedef struct _RtpProfile
{
	char *name;
	PayloadType *payload[RTP_PROFILE_MAX_PAYLOADS];
} RtpProfile ;

extern RtpProfile av_profile;

#define payload_type_set_user_data(pt,p)	(pt)->user_data=(p)
#define payload_type_get_user_data(pt)		((pt)->user_data)



#define rtp_profile_get_name(profile) 	(const char*)((profile)->name)
#define rtp_profile_set_payload(profile,index,pt)  (profile)->payload[(index)]=(pt)
#define rtp_profile_clear_payload(profile,index)	(profile)->payload[(index)]=NULL
#define rtp_profile_clear_all(profile)	memset((void*)(profile),0,sizeof(RtpProfile))
#define rtp_profile_get_payload(profile,index)	((profile)->payload[(index)])
#ifdef __cplusplus
extern "C"{
#endif
void rtp_profile_set_name(RtpProfile *prof, const char *name);
gint rtp_profile_get_payload_number_from_mime(RtpProfile *profile,const char *mime);
gint rtp_profile_get_payload_number_from_rtpmap(RtpProfile *profile, const char *rtpmap);
gint rtp_profile_find_payload_number(RtpProfile *prof,const gchar *mime,int rate);
PayloadType * rtp_profile_find_payload(RtpProfile *prof,const gchar *mime,int rate);

RtpProfile * rtp_profile_new(const char *name);

/* frees the profile and all its PayloadTypes*/
void rtp_profile_destroy(RtpProfile *prof);
#ifdef __cplusplus
}
#endif

/* some payload types */
/* audio */
#ifdef DYNAMIC_PAYLOAD

#include "voip_params.h"

typedef struct profile_list_s {
	unsigned int nSupportedCodec;	/* internal codec index */
	RtpPayloadType pt;				/* payload type */
	PayloadType *pPayloadType;		/* payload info */
	unsigned int nFramePeriod;		/* in unit of 10 ms */
} profile_list_t;

extern const profile_list_t profileList[];

typedef struct profile_dynamic_pt_s {
	RtpPayloadType pt;				/* payload in DSP */
	char pt_value;					/* payload in SDP/RTP */
} profile_dynamic_pt_t;

extern profile_dynamic_pt_t profile_dynamic_pt[];

#else
extern PayloadType pcmu8000;
extern PayloadType pcma8000;
extern PayloadType payload_type_g7231;
extern PayloadType payload_type_g729;
extern PayloadType payload_type_g726_16;
extern PayloadType payload_type_g726_24;
extern PayloadType payload_type_g726_32;
extern PayloadType payload_type_g726_40;
#endif

#endif
