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

#include "payloadtype.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#define snprintf _snprintf
#define strcasecmp stricmp
#endif

PayloadType *payload_type_new()
{
	PayloadType *newpayload=g_new0(PayloadType,1);
	newpayload->flags|=PAYLOAD_TYPE_ALLOCATED;
	return newpayload;
}


PayloadType *payload_type_clone(PayloadType *payload)
{
	PayloadType *newpayload=g_new0(PayloadType,1);
	memcpy(newpayload,payload,sizeof(PayloadType));
	newpayload->mime_type=g_strdup(payload->mime_type);
	if (payload->fmtp!=NULL) newpayload->fmtp=g_strdup(payload->fmtp);
	newpayload->flags|=PAYLOAD_TYPE_ALLOCATED;
	return newpayload;
}

void payload_type_destroy(PayloadType *pt)
{
	g_free(pt->mime_type);
	g_free(pt->fmtp);
	g_free(pt);
}

gint rtp_profile_get_payload_number_from_mime(RtpProfile *profile,const char *mime)
{
	PayloadType *pt;
	gint i;
	for (i=0;i<RTP_PROFILE_MAX_PAYLOADS;i++)
	{
		pt=rtp_profile_get_payload(profile,i);
		if (pt!=NULL)
		{
			if (strcasecmp(pt->mime_type,mime)==0){
				return i;
			}
		}
	}
	return -1;
}

gint rtp_profile_find_payload_number(RtpProfile*profile,const gchar *mime,int rate)
{
	int i;
	PayloadType *pt;
	for (i=0;i<RTP_PROFILE_MAX_PAYLOADS;i++)
	{
		pt=rtp_profile_get_payload(profile,i);
		if (pt!=NULL)
		{
			if (strcasecmp(pt->mime_type,mime)==0 && pt->clock_rate==rate){
			
				return i;
			}
		}
	}
	return -1;
}

gint rtp_profile_get_payload_number_from_rtpmap(RtpProfile *profile,const char *rtpmap)
{
	gint clock_rate,ret;
	char *p,*mime,*tmp,*c;
	
	/* parse the rtpmap */
	tmp=g_strdup(rtpmap);
	p=strchr(tmp,'/');
	if (p!=NULL){
		mime=tmp;
		*p='\0';
		c=p+1;
		p=strchr(c,'/');
		if (p!=NULL) *p='\0';
		clock_rate=atoi(c);
	}else
	{
		return -1;
	}
	
	ret=rtp_profile_find_payload_number(profile,mime,clock_rate);
	g_free(tmp);
	return ret;
}

PayloadType * rtp_profile_find_payload(RtpProfile *prof,const gchar *mime,int rate)
{
	int i;
	i=rtp_profile_find_payload_number(prof,mime,rate);
	if (i>=0) return rtp_profile_get_payload(prof,i);
	return NULL;
}

RtpProfile * rtp_profile_new(const char *name)
{
	RtpProfile *prof=g_new0(RtpProfile,1);
	rtp_profile_set_name(prof,name);
	rtp_profile_clear_all(prof);
	return prof;
}

void rtp_profile_set_name(RtpProfile *obj, const char *name){
	if (obj->name!=NULL) g_free(obj->name);
	obj->name=g_strdup(name);
}

void rtp_profile_destroy(RtpProfile *prof)
{
	int i;
	PayloadType *payload;
	for (i=0;i<RTP_PROFILE_MAX_PAYLOADS;i++)
	{
		payload=rtp_profile_get_payload(prof,i);
		if (payload!=NULL && (payload->flags & PAYLOAD_TYPE_ALLOCATED))
			payload_type_destroy(payload);
	}
	g_free(prof);
}
