
/*
linphone
Copyright (C) 2000  Simon MORLAT (simon.morlat@free.fr)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "misc.h"
//#include "../mediastreamer/mediastream.h"
//#include "../mediastreamer/mscodec.h"
#include "../oRTP/payloadtype.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "voip_manager.h"

#define UDP_HDR_SZ 8
#define RTP_HDR_SZ 12
#define IP4_HDR_SZ 20   /*20 is the minimum, but there may be some options*/

void payload_type_set_enable(PayloadType *pt,gint value) 
{
	if ((value)!=0) payload_type_set_flag(pt,PAYLOAD_TYPE_ENABLED); \
	else payload_type_unset_flag(pt,PAYLOAD_TYPE_ENABLED); 
}

void payload_type_set_usable(PayloadType *pt,gint value) 
{
	if ((value)!=0) payload_type_set_flag(pt,PAYLOAD_TYPE_USABLE); \
	else payload_type_unset_flag(pt,PAYLOAD_TYPE_USABLE); 
}

gboolean payload_type_enabled(PayloadType *pt) {
	return (((pt)->flags & PAYLOAD_TYPE_ENABLED)!=0);
}
gboolean payload_type_usable(PayloadType *pt) {
	return ((pt)->flags & PAYLOAD_TYPE_USABLE);
}

/* return TRUE if codec can be used with bandwidth, FALSE else*/
gboolean payload_type_check_usable(PayloadType *pt,double bandwidth)
{
	gboolean ret=0;
	double codec_band;
	double npacket;
	double packet_size;
	
	switch (pt->type){
		case PAYLOAD_AUDIO_CONTINUOUS:
		case PAYLOAD_AUDIO_PACKETIZED:
			/* very approximative estimation... revisit*/
			npacket=50;
			packet_size=(double)(pt->normal_bitrate/(50*8))+UDP_HDR_SZ+RTP_HDR_SZ+IP4_HDR_SZ;
			codec_band=packet_size*8.0*npacket;
			ret=(codec_band<bandwidth);
			//g_message("Payload %s: %g",pt->mime_type,codec_band);
			break;
	}
	/*if (!ret) g_warning("Payload %s is not usable with your internet connection.",pt->mime_type);*/
	
	return ret;
}

void linphone_core_check_codecs_for_bandwidth(LinphoneCore *lc)
{
	GList *elem;
	PayloadType *pt;
	
	for(elem=lc->codecs_conf.audio_codecs;elem!=NULL;elem=g_list_next(elem))
	{
		pt=(PayloadType*)elem->data;
		payload_type_set_usable(pt,payload_type_check_usable(pt,lc->net_conf.bandwidth));	
	}
}

int32 GetCodecPrecedence(int chid, precedence_config_t *cfg)
{
	voipCfgPortParam_t *voip_cfg;
	RtpPayloadType szCodecSeq[_CODEC_MAX];
	int32 szFramePerPacket[_CODEC_MAX];
	int i, j, idx;

	if (cfg->uCodecMax != _CODEC_MAX)
	{
		printf("cfg not match\n");
		return -1;
	}
    
	voip_cfg = &g_pVoIPCfg->ports[chid];
	for (i=0; i<_CODEC_MAX; i++)
	{
		szCodecSeq[i] = rtpPayloadUndefined;
		szFramePerPacket[i] = 1;
	}

	for (i=0, idx=0; i<_CODEC_MAX; i++)
	{
		if (g_mapSupportedCodec[i] < 0 ||
			g_mapSupportedCodec[i] >= _CODEC_MAX)
			continue; // not supported
		
		j = voip_cfg->precedence[idx++];
		if (j >= _CODEC_MAX)
			continue; // skip error

#ifdef DYNAMIC_PAYLOAD
		if (profileList[i].nSupportedCodec != i)
			g_error("invalid payload index = %d\n", i);

		szCodecSeq[j] = profileList[i].pt;
		szFramePerPacket[j] = (voip_cfg->frame_size[i] + 1) / (profileList[i].nFramePeriod);
#endif // DYNAMIC_PAYLOAD
	}

	memcpy(cfg->pCodecPred, szCodecSeq, sizeof(szCodecSeq));
	memcpy(cfg->pFramePerPacket, szFramePerPacket, sizeof(szFramePerPacket));
    return 0;
}

void linphone_core_setup_local_rtp_profile(LinphoneCore *lc)
{
	int i;
	GList *audiopt;
	PayloadType *payload;
	precedence_config_t CodecPredCfg;
	int codec_seq[_CODEC_MAX];
	uint32 FramePerPacket[_CODEC_MAX];

	CodecPredCfg.chid = lc->chid;
	CodecPredCfg.uCodecMax = _CODEC_MAX;
	CodecPredCfg.pCodecPred = codec_seq; 
	CodecPredCfg.pFramePerPacket = FramePerPacket;
	GetCodecPrecedence(lc->chid, &CodecPredCfg);

	lc->local_profile=&av_profile;
	
	// add supported codec (not include telephone-event
	audiopt=NULL;

	/* supported codec can be set in menuconfig */
	for (i=0; i<g_nMaxCodec; i++)
	{
	

		if (i==0&&(lc ->sip_conf.rtp_redundant_payload_type )){//add rtp redundant in first codec list
			payload=rtp_profile_get_payload(lc->local_profile, rtpPayloadRtpRedundant);
			if (payload!=NULL){
				/* find a mediastreamer codec for this payload type */
				switch (payload->type){
					case PAYLOAD_AUDIO_CONTINUOUS:
					case PAYLOAD_AUDIO_PACKETIZED:
						g_message("Adding new codec %s/%i\n",payload->mime_type,payload->clock_rate);
						payload_type_set_enable(payload,1);
						audiopt=g_list_append(audiopt,(gpointer)payload);
						break;
					default:
						g_message("Unsupported rtp media type:%d\n",payload->type);
				}
			}else{
				g_message("payload = NULL\n");
			}
		}

		payload=rtp_profile_get_payload(lc->local_profile, codec_seq[i] );
		if (payload!=NULL){
			/* find a mediastreamer codec for this payload type */
			switch (payload->type){
				case PAYLOAD_AUDIO_CONTINUOUS:
				case PAYLOAD_AUDIO_PACKETIZED:
					g_message("Adding new codec %s/%i\n",payload->mime_type,payload->clock_rate);
					payload_type_set_enable(payload,1);
					audiopt=g_list_append(audiopt,(gpointer)payload);
					break;
				default:
					g_message("Unsupported rtp media type:%d\n",payload->type);
			}
		}
	}
	g_list_foreach(lc->codecs_conf.audio_codecs,(GFunc)payload_type_destroy,NULL);
	g_list_free(lc->codecs_conf.audio_codecs);
	lc->codecs_conf.audio_codecs=audiopt;
	linphone_core_check_codecs_for_bandwidth(lc);
}

int from_2char_without_params(osip_from_t *from,char **str)
{
	osip_from_t *tmpfrom;
	osip_from_clone(from,&tmpfrom);
	//[SD6, bohungwu, exosip 3.5 integration
	//while(!osip_list_eol(tmpfrom->gen_params,0)){
	while(!osip_list_eol(&(tmpfrom->gen_params),0)){
		//osip_generic_param_t *param=(osip_generic_param_t*)osip_list_get(tmpfrom->gen_params,0);
		osip_generic_param_t *param=(osip_generic_param_t*)osip_list_get(&(tmpfrom->gen_params),0);
		//]
		osip_generic_param_free(param);
		//[SD6, bohungwu, exosip 3.5 integration
		//osip_list_remove(tmpfrom->gen_params,0);
		osip_list_remove(&(tmpfrom->gen_params),0);
		//]
	}
	osip_from_to_str(tmpfrom,str);
	osip_from_free(tmpfrom);
	return 0;
}

gboolean host_has_ipv6_network()
{
	return FALSE;
}
