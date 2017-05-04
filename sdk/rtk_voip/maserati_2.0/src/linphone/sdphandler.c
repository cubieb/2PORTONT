/*
  * Linphone is sip (RFC3261) compatible internet phone.
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Lesser General Public
  * License as published by the Free Software Foundation; either
  * version 2.1 of the License, or (at your option) any later version.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Lesser General Public License for more details.
  *
  * You should have received a copy of the GNU Lesser General Public
  * License along with this library; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  */

#include "sdphandler.h"
//[SD6, bohungwu, exosip 3.5 integration
//#include "../osip/osip_port.h"
//#include "../osip/sdp_message.h"
//#include "../exosip/eXosip.h"
#include "osipparser2/osip_port.h"
#include "osipparser2/sdp_message.h"
#include "eXosip2/eXosip.h"
#include "../oRTP/payloadtype.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

//]

#define keywordcmp(key,str)  strncmp(key,str,strlen(key))

#include "uglib.h"

#define sstrdup_sprintf g_strdup_printf

#define eXosip_trace(loglevel,args)  do        \
{                       \
	char *__strmsg;  \
	__strmsg=g_strdup_printf args ;    \
	OSIP_TRACE(osip_trace(__FILE__,__LINE__,(loglevel),NULL,"%s\n",__strmsg)); \
	osip_free (__strmsg);        \
}while (0);

static int _sdp_message_get_mline_dir(sdp_message_t *sdp, int mline);

char * int_2char(int a){
	char *p=osip_malloc(16);
	snprintf(p,16,"%i",a);
	return p;
}

/* return the value of attr "field" for payload pt at line pos (field=rtpmap,fmtp...)*/
char *sdp_message_a_attr_value_get_with_pt(sdp_message_t *sdp,int pos,int pt,char *field)
{
	int i,tmppt=0,scanned=0;
	char *tmp;
	sdp_attribute_t *attr;
	for (i=0;(attr=sdp_message_attribute_get(sdp,pos,i))!=NULL;i++){
		if (keywordcmp(field,attr->a_att_field)==0){
			sscanf(attr->a_att_value,"%i %n",&tmppt,&scanned);
			if (pt==tmppt){
				tmp=attr->a_att_value+scanned;
				if (strlen(tmp)>0)
					return tmp;
				else {
					eXosip_trace(OSIP_WARNING,("sdp has a strange a= line."));
					continue;
				}
			}
		}
	}
	return NULL;
}



int
sdp_payload_init (sdp_payload_t * payload)
{
	memset (payload, 0, sizeof (sdp_payload_t));
	return 0;
}

sdp_context_t *sdp_handler_create_context(sdp_handler_t *handler, const char *localip, const char *username){
	sdp_context_t *ctx=osip_malloc(sizeof(sdp_context_t));

	if( ctx == NULL)
	{
		g_warning( "create_context is NULL !!\n" ) ;
		return NULL ;
	}
	memset(ctx,0,sizeof(sdp_context_t));
	if (localip!=NULL) ctx->localip=osip_strdup(localip);
	if (username && username[0])
		ctx->username=osip_strdup(username);
	else
		ctx->username=osip_strdup("-");
	ctx->handler=handler;
	return ctx;
}

void sdp_context_set_user_pointer(sdp_context_t * ctx, void* up){
	ctx->reference=up;
}

void *sdp_context_get_user_pointer(sdp_context_t * ctx){
	return ctx->reference;
}

int sdp_context_get_status(sdp_context_t* ctx){
	return ctx->negoc_status;
}

sdp_message_t hold_message_str ;
char local_ip_str[20] ;

/* generate a template sdp */
sdp_message_t *
sdp_context_generate_template (sdp_context_t * ctx)
{
	sdp_message_t *local;
	int inet6;
	
	sdp_message_init (&local);
	if (strchr(ctx->localip,':')!=NULL){
		inet6=1;
	}else inet6=0;
	if (!inet6){
		sdp_message_v_version_set (local, osip_strdup ("0"));
		sdp_message_o_origin_set (local, osip_strdup (ctx->username),
				  osip_strdup ("0000000001"), osip_strdup ("0000000001"),
				  osip_strdup ("IN"), osip_strdup ("IP4"),
				  osip_strdup (ctx->localip));
		strcpy( local_ip_str , ctx->localip ) ;
		sdp_message_s_name_set (local, osip_strdup ("A conversation"));
		sdp_message_c_connection_add (local, -1,
				      osip_strdup ("IN"), osip_strdup ("IP4"),
				      osip_strdup (ctx->localip), NULL, NULL);
		sdp_message_t_time_descr_add (local, osip_strdup ("0"), osip_strdup ("0"));
/* eric test */
#if 0
		sdp_message_v_version_set (&hold_message_str, osip_strdup ("0"));
		sdp_message_o_origin_set (&hold_message_str, osip_strdup (ctx->username),
				  osip_strdup ("00000001"), osip_strdup ("00000001"),
				  osip_strdup ("IN"), osip_strdup ("IP4"),
				  osip_strdup (ctx->localip));
		sdp_message_s_name_set (&hold_message_str, osip_strdup ("A conversation"));
		sdp_message_c_connection_add (&hold_message_str, -1,
				      osip_strdup ("IN"), osip_strdup ("IP4"),
				      osip_strdup ("0.0.0.0"), NULL, NULL);
		sdp_message_t_time_descr_add (&hold_message_str, osip_strdup ("0"), osip_strdup ("0"));
#endif
	}else{
		sdp_message_v_version_set (local, osip_strdup ("0"));
		sdp_message_o_origin_set (local, osip_strdup (ctx->username),
				  osip_strdup ("123456"), osip_strdup ("654321"),
				  osip_strdup ("IN"), osip_strdup ("IP6"),
				  osip_strdup (ctx->localip));
		sdp_message_s_name_set (local, osip_strdup ("A conversation"));
		sdp_message_c_connection_add (local, -1,
				      osip_strdup ("IN"), osip_strdup ("IP6"),
				      osip_strdup (ctx->localip), NULL, NULL);
		sdp_message_t_time_descr_add (local, osip_strdup ("0"), osip_strdup ("0"));
	}

	return local;
}

/* to add payloads to the offer, must be called inside the write_offer callback */
void
sdp_context_add_payload (sdp_context_t * ctx, sdp_payload_t * payload, char *media)
{
	sdp_message_t *offer = ctx->offer;
	char *attr_field;
	if (!ctx->incb)
	{
		eXosip_trace (OSIP_ERROR,
			    ("You must not call sdp_context_add_*_payload outside the write_offer callback\n"));
		exit(0);
	}
	if (payload->proto == NULL)
		payload->proto = "RTP/AVP";
	if (sdp_message_m_media_get (offer, payload->line) == NULL)
	{
		/* need a new line */
		sdp_message_m_media_add (offer, osip_strdup (media),
				 int_2char (payload->localport), NULL,
				 osip_strdup (payload->proto));
	}
	sdp_message_m_payload_add (offer, payload->line, int_2char (payload->pt));
	if (payload->a_rtpmap != NULL)
	{
		attr_field =
			sstrdup_sprintf ("%i %s", payload->pt,
					 payload->a_rtpmap);
		sdp_message_a_attribute_add (offer, payload->line,
				     osip_strdup ("rtpmap"), attr_field);
	}
	if (payload->a_fmtp != NULL)
	{
		attr_field =
			sstrdup_sprintf ("%i %s", payload->pt,
					 payload->a_fmtp);
		sdp_message_a_attribute_add (offer, payload->line, osip_strdup ("fmtp"),
				     attr_field);
	}
#ifdef SUPPORT_V152_VBD
	if (payload->a_gpmd != NULL)
	{
		attr_field =
			sstrdup_sprintf ("%i %s", payload->pt,
					 payload->a_gpmd);
		sdp_message_a_attribute_add (offer, payload->line, osip_strdup ("gpmd"),
				     attr_field);
	}
#endif
	if (payload->b_as_bandwidth != 0)
	{	
		if (sdp_message_bandwidth_get(offer,payload->line,0)==NULL){
			attr_field =
			sstrdup_sprintf ("%i", payload->b_as_bandwidth);
			sdp_message_b_bandwidth_add (offer, payload->line, osip_strdup ("AS"),
				     attr_field);
		}
	}
}

void
sdp_context_add_audio_payload (sdp_context_t * ctx, sdp_payload_t * payload)
{
	sdp_context_add_payload (ctx, payload, "audio");
}

char *
sdp_context_get_offer ( sdp_context_t * ctx)
{
	sdp_message_t *offer;
	sdp_handler_t *sdph=ctx->handler;
	char *tmp;

	offer = sdp_context_generate_template (ctx);

	// implement version change in the o= field
	ctx->version++;
	sprintf(offer->o_sess_version, "%.10d", ctx->version);

	/* add audio codecs */
	ctx->offer = offer;
	ctx->incb = 1;
	if (sdph->set_audio_codecs != NULL)
		sdph->set_audio_codecs (ctx);
	ctx->incb = 0;
	sdp_message_to_str(offer,&tmp);
	ctx->offerstr=tmp;
	return tmp;
}


/* refuse the line */
static void refuse_mline(sdp_message_t *answer,char *mtype,char *proto, int mline)
{
	sdp_message_m_media_add (answer,
							 osip_strdup (mtype),
							 int_2char (0), NULL,
							 osip_strdup (proto));
	/* add a payload just to comply with sdp RFC.*/
	sdp_message_m_payload_add(answer,mline,int_2char(0));
}

#ifdef DYNAMIC_PAYLOAD
int linphone_core_get_rfc2833_pt(sdp_message_t *sdp)
{
	sdp_attribute_t *attr;
	int pos_media, pos;
	int pt, scanned;
	char *pt_value;

	//[SD6, ericchung, exosip 3.5 integration
	if(sdp==NULL){
		g_error("linphone_core_get_rfc2833_pt sdp is null\n");
		return 0;
	}
	//]

	for (pos_media=-1; !sdp_message_endof_media(sdp, pos_media); pos_media++)
	{
		for (pos=0; (attr=sdp_message_attribute_get(sdp, pos_media, pos))!=NULL; pos++)
	    {
			if (strcmp(attr->a_att_field, "rtpmap") == 0)
			{
				sscanf(attr->a_att_value, "%d %n", &pt, &scanned);
				pt_value = attr->a_att_value + scanned;
				if (osip_strcasecmp(pt_value, "telephone-event/8000") == 0)
					return pt;
			}
		}
	}

	return 0;
}
#endif // DYNAMIC_PAYLOAD

char *
sdp_context_get_answer ( sdp_context_t *ctx,const char *remote_offer)
{
	sdp_message_t *answer=NULL,*remote=NULL;
	char *mtype=NULL, *tmp=NULL, *bw=NULL;
	char *proto=NULL, *port=NULL, *pt=NULL;
	int i, j, ncodec, mcodec, m_lines_accepted = 0;
	int err;
	sdp_payload_t payload;
	sdp_handler_t *sdph=ctx->handler;
#if CONFIG_RTK_VOIP_SRTP
	int mcrypt=0;
	int bReInvite=0;
#endif

	LinphoneCall *call=(LinphoneCall*)sdp_context_get_user_pointer(ctx);
	LinphoneCore *lc=call->core;
	
	sdp_message_init(&remote);
	err=sdp_message_parse(remote,remote_offer);

	{
		char *strSDP;
		sdp_message_to_str(remote, &strSDP);
		g_message("+++++aa DEBUG:SDP Message:%s+++++\n",strSDP);
		osip_free(strSDP);
	}
	
	if (err<0) {
		eXosip_trace(OSIP_WARNING,("Could not parse remote offer."));
		return NULL;
	}

#if 1
	// rock: for reinvite issue
	{
		//LinphoneCall * call = ( LinphoneCall * )sdp_context_get_user_pointer( ctx ) ;

		if (ctx->remote)
		{
			// release old remote
#if CONFIG_RTK_VOIP_SRTP
			//when holder is callee
			bReInvite=1;
#endif
			sdp_message_free(ctx->remote);
		}

		call->audio_params.initialized = 0;
		call->audio_params.initialized_vbd = 0;
#ifdef SUPPORT_RTP_REDUNDANT_APP
		call->audio_params.initialized_rtp_redundant = 0;
#endif
		/*++added by Jack Chan 05/02/07 for T.38++*/
		call->t38_params.initialized = 0;
		/*--end--*/
	}
#endif

	ctx->remote=remote;

	tmp = sdp_message_c_addr_get (remote, 0, 0);
	if (tmp == NULL)
	  tmp = sdp_message_c_addr_get (remote, -1, 0);
	if (ctx->localip==NULL) {
		/* NULL means guess, otherwise we use the address given as localip */
//		eXosip_get_localip_for(tmp, &ctx->localip);
		rcm_get_localip_for_voip(solar_ptr->current_voip_interface,&ctx->localip);
	}
	else eXosip_trace(OSIP_INFO1,("Using firewall address in sdp."));

//[SD6, ericchung, exosip 3.5 integration
//#ifdef CHECK_LOOPBACK	
#if 0
//]
	// check loopback mode
	ctx->loopback = strcmp(tmp, ctx->localip) == 0;
#endif

	answer = sdp_context_generate_template (ctx);

	// implement version change in the o= field
	ctx->version++;
	sprintf(answer->o_sess_version, "%.10d", ctx->version);
	
	/* for each m= line */
	for (i = 0; !sdp_message_endof_media (remote, i); i++)
	{
		sdp_payload_init(&payload);
		mtype = sdp_message_m_media_get (remote, i);
		proto = sdp_message_m_proto_get (remote, i);
		port = sdp_message_m_port_get (remote, i);
		payload.remoteport = osip_atoi (port);
		payload.proto = proto;
		payload.line = i;
		payload.c_addr = sdp_message_c_addr_get (remote, i, 0);
		if (payload.c_addr == NULL)
			payload.c_addr = sdp_message_c_addr_get (remote, -1, 0);
		/* get application specific bandwidth, if any */
		bw=sdp_message_b_bandwidth_get(remote, i, 0);
		if (bw!=NULL) payload.b_as_bandwidth=atoi(bw);
		if (keywordcmp ("audio", mtype) == 0)
		{
			if (sdph->accept_audio_codecs != NULL)
			{
				ncodec = 0;
				mcodec = 0;	// media codec, not include telephone-event
				/*+++++ Added by Jack for SRTP 140108+++++*/
				/* ask if support RTP/SAVP */
#ifdef CONFIG_RTK_VOIP_SRTP
				if(keywordcmp("RTP/SAVP", proto) == 0 && lc->sip_conf.security_enable == 1){
					sdp_attribute_t *attr;
					char *localKey=NULL;
					char *remoteKey=NULL;
					int localCryptAlg;
					int remoteCryptAlg;
					char *tmpKey=NULL;
					char *cryptoStr=NULL;
					int pos;
					
					//handle the reinvite case
					// when holder is caller
					if(ctx->answer)
						bReInvite=1;
					if(bReInvite){
						sdp_message_t *reInviteSDP;
						
						if(ctx->answer)
							reInviteSDP=ctx->answer;
						else 
							reInviteSDP=ctx->offer;
						for (pos=0; (attr=sdp_message_attribute_get(reInviteSDP, i, pos))!=NULL; pos++){
							if (strcmp(attr->a_att_field, "crypto") == 0){
								if(strstr(attr->a_att_value,"AES_CM_128_HMAC_SHA1_80")){
									payload.localCryptAlg=HMAC_SHA1;
								}else{
									payload.localCryptAlg=-1;
								}
								tmpKey = strstr(attr->a_att_value,"inline:");
								if(tmpKey)
									localKey=tmpKey + strlen("inline:");
								memcpy(&payload.localSDPKey, localKey, SDP_KEY_LEN);
								break;
							}
						}
					}
					
					for (pos=0; (attr=sdp_message_attribute_get(remote, i, pos))!=NULL; pos++){
						if (strcmp(attr->a_att_field, "crypto") == 0){
							cryptoStr = attr->a_att_value;
							/*extract the remote key and crypt. algorithms*/
							if(strstr(attr->a_att_value,"AES_CM_128_HMAC_SHA1_80")){
								/*libsrtp just implement counter mod for SDES*/
								payload.remoteCryptAlg=HMAC_SHA1;
							}else{ 
								payload.remoteCryptAlg=-1;
							}
							/*negoriation with crypt. algorithm*/
							payload.localCryptAlg=payload.remoteCryptAlg;
							tmpKey = strstr(attr->a_att_value,"inline:");
							if(tmpKey)
								remoteKey = tmpKey + strlen("inline:");
							if(strlen(remoteKey) >= SDP_KEY_LEN && payload.remoteCryptAlg != -1){
								unsigned char key[30];	//local srtp key	
								
								memcpy(&payload.remoteSDPKey, remoteKey, SDP_KEY_LEN);
								/* not reinvite case */
								if(!bReInvite){
									if(!sdp_generate_sdes_key(key, 30)){
										g_warning("Create sdes key fail!\n");
										continue;
									}else{
										base64Encode(key, payload.localSDPKey);
									}
								}
								g_message("+++++remote sdp key:%s+++++\n",payload.remoteSDPKey);
								g_message("+++++local sdp key:%s+++++\n",payload.localSDPKey);
								break;	
							}
						}	
					}
				}
#endif /*CONFIG_RTK_VOIP_SRTP*/
				/*-----end-----*/
				/* for each payload type */
				for (j = 0;
				     ((pt =
				       sdp_message_m_payload_get (remote, i,
							  j)) != NULL); j++)
				{
					payload.pt = osip_atoi (pt);
					/* get the rtpmap associated to this codec, if any */
					payload.a_rtpmap =
						sdp_message_a_attr_value_get_with_pt
						(remote, i, payload.pt,
						 "rtpmap");
					/* get the fmtp, if any */
					payload.a_fmtp =
						sdp_message_a_attr_value_get_with_pt
						(remote, i, payload.pt,
						 "fmtp");
#ifdef SUPPORT_V152_VBD
					/* get the gpmd, if any */
					payload.a_gpmd =
						sdp_message_a_attr_value_get_with_pt
						(remote, i, payload.pt,
						 "gpmd");
#endif
					
					/* ask the application if this codec is supported */
					err = sdph->accept_audio_codecs (ctx,
									 &payload);
					if (err != Unsupported && payload.localport > 0)
					{
						ncodec++;
						if (err == SupportedAndValid)
							mcodec++;

						/* codec accepted */
						if (ncodec == 1)
						{
							/* first codec accepted, setup the line  */
							sdp_message_m_media_add
								(answer,
								 osip_strdup
								 (mtype),
								 int_2char
								 (payload.
								  localport),
								 NULL,
								 osip_strdup
								 (proto));
						}
						/* add the payload, rtpmap, fmtp */
						sdp_message_m_payload_add (answer, i,
								   int_2char
								   (payload.
								    pt));
						if (payload.a_rtpmap != NULL)
						{
							sdp_message_a_attribute_add
								(answer, i,
								 osip_strdup
								 ("rtpmap"),
								 sstrdup_sprintf
								 ("%i %s",
								  payload.pt,
								  payload.
								  a_rtpmap));
						}
						if (payload.a_fmtp != NULL)
						{
							sdp_message_a_attribute_add
								(answer, i,
								 osip_strdup
								 ("fmtp"),
								 sstrdup_sprintf
								 ("%i %s",
								  payload.pt,
								  payload.
								  a_fmtp));
						}
#ifdef SUPPORT_V152_VBD
						if (payload.a_gpmd != NULL)
						{
							sdp_message_a_attribute_add
								(answer, i,
								 osip_strdup
								 ("gpmd"),
								 sstrdup_sprintf
								 ("%i %s",
								  payload.pt,
								  payload.
								  a_gpmd));
						}
#endif
						if (payload.b_as_bandwidth !=
						    0)
						{
							if (sdp_message_bandwidth_get(answer,i,0)==NULL)
								sdp_message_b_bandwidth_add
								(answer, i,
								 osip_strdup
								 ("AS"),
								 sstrdup_sprintf
								 ("%i",
								  payload.
								  b_as_bandwidth));
						}

					#if 0
						// support one codec only on response
						break;
					#endif
					}
				}
/*+++++ Added by Jack for SRTP 140108+++++*/
#if CONFIG_RTK_VOIP_SRTP
						if( lc->sip_conf.security_enable == 1 && payload.remoteCryptAlg == HMAC_SHA1){
							sdp_message_a_attribute_add
								(answer, i, osip_strdup("crypto"),
								sstrdup_sprintf("1 AES_CM_128_HMAC_SHA1_80 inline:%s",payload.localSDPKey));
							mcrypt++;
						}
#endif
/*-----end-----*/
				if (ncodec == 0)
				{
					/* refuse the line */
					refuse_mline(answer,mtype,proto,i);
					
				}
#if CONFIG_RTK_VOIP_SRTP
				/* if security is enable only security call is accepted */
				else if((lc->sip_conf.security_enable == 1 && mcrypt > 0) || (lc->sip_conf.security_enable == 0 && keywordcmp("RTP/SAVP", proto) != 0))
					if (mcodec > 0)
						m_lines_accepted++;
#else
				else if (mcodec > 0)
					m_lines_accepted++;
#endif
			}
			else
			{
				/* refuse this line (leave port to 0) */
				refuse_mline(answer,mtype,proto,i);
			}

		}/*++T.38 added by Jack Chan 25/01/07 for VoIP++*/
		else if(keywordcmp ("image", mtype) == 0){
			sdp_attribute_t *sdpAtb;
			char *mediaPlayload = NULL;
			StreamParams *params;
			
			//t.38 setting not enable
			if(FALSE == lc->sip_conf.T38_enable){
				ctx->negoc_status = 488;
			#if 0
				sdp_message_free(ctx->answer);
				if (ctx->answerstr)
						osip_free(ctx->answerstr);
			#endif
				sdp_message_free(answer);
				//sdp_message_free(remote);
				g_message("+++++debug:t.38 setting not enable+++++\n");
				return NULL;
			}
			mediaPlayload = sdp_message_m_payload_get(remote, i, 0);
			if(0 != osip_strcasecmp(mediaPlayload, "t38"))
			{
				ctx->negoc_status = 488;
			#if 0
				sdp_message_free(ctx->answer);
				if (ctx->answerstr)
						osip_free(ctx->answerstr);
			#endif
				sdp_message_free(answer);
				//sdp_message_free(remote);
				g_warning("+++++debug: unknown image type %s +++++\n", mediaPlayload);
				return NULL;
			}

			params = &call->t38_params ;
			if(lc->sip_conf.T38_prot > 0)
				params->localport = lc->sip_conf.T38_prot;

			params->remoteport = payload.remoteport ;
			params->line = payload.line ;
		#ifdef DYNAMIC_PAYLOAD
			params->local_pt = rtpPayloadT38_Virtual;
			params->remote_pt = rtpPayloadT38_Virtual;
			params->static_pt = rtpPayloadT38_Virtual;

		#ifdef AUTO_RESOURCE
			if (VOIP_RESOURCE_UNAVAILABLE == rcm_tapi_VoIP_resource_check(lc->chid, params->static_pt))
			{
				ctx->negoc_status = 488;
				sdp_message_free(answer);
				g_warning("no resource for T38\n");
				return NULL;
			}
		#endif

		#else
			params->pt = rtpPayloadT38_Virtual; 
		#endif // DYNAMIC_PAYLOAD
			//params->remoteaddr = payload.c_addr ;
			bzero(&params->remote_addr, sizeof(params->remote_addr));
			strcpy(params->remote_addr,payload.c_addr);			
			params->remote_ip=inet_addr(payload.c_addr);		
			params->initialized = 1 ;
			
			sdp_message_m_media_add(answer,osip_strdup(mtype),int_2char(params->localport),NULL,osip_strdup(proto));
			if(NULL != mediaPlayload)
				sdp_message_m_payload_add(answer, i, osip_strdup(mediaPlayload));
			for(j = 0; (sdpAtb = sdp_message_attribute_get(remote, i, j)) != NULL; j++){
				sdp_message_a_attribute_add(answer, i, osip_strdup(sdpAtb->a_att_field), osip_strdup(sdpAtb->a_att_value));				
			}
			m_lines_accepted++;
		}
		/*--end--*/
	}
#if 1
	// rock: for reinvite issue
	if (ctx->answer)
	{
		// release old answer
		sdp_message_free(ctx->answer);
	}
#endif

	ctx->answer = answer;
	if (m_lines_accepted > 0)
	{
#ifdef DYNAMIC_PAYLOAD
		StreamParams *audio_params = &call->audio_params;
		audio_params->rfc2833_local_pt = linphone_core_get_rfc2833_pt(ctx->answer);
		audio_params->rfc2833_remote_pt = linphone_core_get_rfc2833_pt(ctx->remote);
#else
		if (lc->sip_conf.dtmf_type != DTMF_INBAND)		// No need telephone-event if DTMF In Band 
		{
			/* add telephone-event payload*/
			char payload_buf[100];
			
			sprintf(payload_buf, "%d", lc->sip_conf.rfc2833_payload_type);			
		  	sdp_message_m_payload_add(answer, 0, osip_strdup(payload_buf));
			sprintf(payload_buf, "%d telephone-event/8000", lc->sip_conf.rfc2833_payload_type);			
		  	sdp_message_a_attribute_add(answer, 0, osip_strdup("rtpmap"), osip_strdup(payload_buf));
			sprintf(payload_buf, "%d 0-15", lc->sip_conf.rfc2833_payload_type);			
		  	sdp_message_a_attribute_add(answer, 0, osip_strdup("fmtp"), osip_strdup(payload_buf));
		}
#endif // DYNAMIC_PAYLOAD

#if 0
		// add sendrecv attribute
		if( 0 != keywordcmp( "udptl", proto ) ){	/* Not a T.38 re-INVITE */
			int k;
			for(i=-1; !sdp_message_endof_media(remote, i); i++){
				char *sendRecv = NULL;
				for(k=0; NULL!=(sendRecv = sdp_message_a_att_field_get(remote, i, k)); k++){
					if( 0 == osip_strcasecmp(sendRecv, "sendonly"))   /*for hold message*/
					{
						sdp_message_a_attribute_add(answer, 0, osip_strdup ("recvonly"), NULL);
						// mix 2543 & 3264 hold
						if (answer->c_connection)
						{
							osip_free(answer->c_connection->c_addr);
							answer->c_connection->c_addr = osip_strdup("0.0.0.0");
						}
					}
					else if( 0 == osip_strcasecmp(sendRecv, "sendrecv"))
						sdp_message_a_attribute_add(answer, 0, osip_strdup ("sendrecv"), NULL);			
				}
			}
		}
#endif

		ctx->negoc_status = 200;
		sdp_message_to_str(answer,&tmp);
#ifdef FIX_MEMORY_LEAK
		// for reinvite issue
		if (ctx->answerstr)
			osip_free(ctx->answerstr);
#endif
		ctx->answerstr=tmp;
		return tmp;
	}else{
		ctx->negoc_status = 415;
		return NULL;
	}
}



//[SD6, ericchung, rcm integration
void rcm_set_sdp(osip_message_t *sip,sdp_message_t *msg){
	int sdplen;
	char clen[10];
	char *sdp=NULL;
	sdp_message_to_str(msg,&sdp);
	sdplen=strlen(sdp);
	snprintf(clen,sizeof(clen),"%i",sdplen);
	osip_message_set_body(sip,sdp,sdplen);
	osip_message_set_content_type(sip,"application/sdp");
	osip_message_set_content_length(sip,clen);
	osip_free(sdp);
}
//]


char *
rcm_sdp_context_get_answer_by_ev ( sdp_context_t *ctx,eXosip_event_t *ev)
{
	sdp_message_t *answer=NULL,*remote=NULL;
	char *mtype=NULL, *tmp=NULL, *bw=NULL;
	char *proto=NULL, *port=NULL, *pt=NULL;
    char tmpbuf[100];
	int i, j, ncodec, mcodec, m_lines_accepted = 0;
	int err;
	sdp_payload_t payload;
	sdp_handler_t *sdph=ctx->handler;

	PayloadType *lc_codec;
	GList *elem;
	sdp_payload_t lc_payload,peer_payload;
	int chose_code=-1;
	
#if CONFIG_RTK_VOIP_SRTP
	int mcrypt=0;
	int bReInvite=0;
#endif

	LinphoneCall *call=(LinphoneCall*)sdp_context_get_user_pointer(ctx);
	LinphoneCore *lc=call->core;


	g_message("rcm_sdp_context_get_answer_by_ev\n");	
	remote = eXosip_get_sdp_info(ev->request);
	
	if(remote == NULL){
		g_message("no SDP message  \n");
	}


#if 0	
	sdp_message_init(&remote);
	err=sdp_message_parse(remote,remote_offer);

	{
		char *strSDP;
		sdp_message_to_str(remote, &strSDP);
		g_message("+++++DEBUG:SDP Message:%s+++++\n",strSDP);
		osip_free(strSDP);
	}
	if (err<0) {
		eXosip_trace(OSIP_WARNING,("Could not parse remote offer."));
		return NULL;
	}	//Alex, 20111021, Valgrind
#endif	
#if 1
	// rock: for reinvite issue
	{
		//LinphoneCall * call = ( LinphoneCall * )sdp_context_get_user_pointer( ctx ) ;

		if (ctx->remote)
		{
			// release old remote
#if CONFIG_RTK_VOIP_SRTP
			//when holder is callee
			bReInvite=1;
#endif
			sdp_message_free(ctx->remote);
		}

		call->audio_params.initialized = 0;
		/*++added by Jack Chan 05/02/07 for T.38++*/
		call->t38_params.initialized = 0;
		/*--end--*/
	}
#endif

	ctx->remote=remote;



	tmp = sdp_message_c_addr_get (remote, 0, 0);
	if (tmp == NULL)
	  tmp = sdp_message_c_addr_get (remote, -1, 0);




	if (ctx->localip==NULL) {
		/* NULL means guess, otherwise we use the address given as localip */
//#ifdef OLD_API		
//		eXosip_get_localip_for(tmp, &ctx->localip);
		rcm_get_localip_for_voip(solar_ptr->current_voip_interface,&ctx->localip);
//#endif
	}
	else eXosip_trace(OSIP_INFO1,("Using firewall address in sdp."));


	

//#ifdef CHECK_LOOPBACK	
#if 0//eric
	// check loopback mode
	ctx->loopback = strcmp(tmp, ctx->localip) == 0;
#endif

	answer = sdp_context_generate_template (ctx);

	// implement version change in the o= field
	ctx->version++;
	sprintf(answer->o_sess_version, "%.10d", ctx->version);
	
	/* for each m= line */
	for (i = 0; !sdp_message_endof_media (remote, i); i++)
	{
		sdp_payload_init(&payload);
		mtype = sdp_message_m_media_get (remote, i);
		proto = sdp_message_m_proto_get (remote, i);
		port = sdp_message_m_port_get (remote, i);
		payload.remoteport = osip_atoi (port);
		payload.proto = proto;
		payload.line = i;
		payload.c_addr = sdp_message_c_addr_get (remote, i, 0);
		if (payload.c_addr == NULL)
			payload.c_addr = sdp_message_c_addr_get (remote, -1, 0);
		/* get application specific bandwidth, if any */
		bw=sdp_message_b_bandwidth_get(remote, i, 0);
		if (bw!=NULL) payload.b_as_bandwidth=atoi(bw);


		g_message("\r\n check _sdp_message_get_mline_dir \n");
		ctx->remote_dir=_sdp_message_get_mline_dir(remote,i);

		if (keywordcmp ("audio", mtype) == 0)
		{
			if (sdph->accept_audio_codecs != NULL)
			{
				ncodec = 0;
				mcodec = 0;	// media codec, not include telephone-event
				/*+++++ Added by Jack for SRTP 140108+++++*/
				/* ask if support RTP/SAVP */
#ifdef CONFIG_RTK_VOIP_SRTP
				if(keywordcmp("RTP/SAVP", proto) == 0 && lc->sip_conf.security_enable == 1){
					sdp_attribute_t *attr;
					char *localKey=NULL;
					char *remoteKey=NULL;
					int localCryptAlg;
					int remoteCryptAlg;
					char *tmpKey=NULL;
					char *cryptoStr=NULL;
					int pos;
					
					//handle the reinvite case
					// when holder is caller
					if(ctx->answer)
						bReInvite=1;
					if(bReInvite){
						sdp_message_t *reInviteSDP;
						
						if(ctx->answer)
							reInviteSDP=ctx->answer;
						else 
							reInviteSDP=ctx->offer;
						for (pos=0; (attr=sdp_message_attribute_get(reInviteSDP, i, pos))!=NULL; pos++){
							if (strcmp(attr->a_att_field, "crypto") == 0){
								if(strstr(attr->a_att_value,"AES_CM_128_HMAC_SHA1_80")){
									payload.localCryptAlg=HMAC_SHA1;
								}else{
									payload.localCryptAlg=-1;
								}
								tmpKey = strstr(attr->a_att_value,"inline:");
								if(tmpKey)
									localKey=tmpKey + strlen("inline:");
								memcpy(&payload.localSDPKey, localKey, SDP_KEY_LEN);
								break;
							}
						}
					}
					
					for (pos=0; (attr=sdp_message_attribute_get(remote, i, pos))!=NULL; pos++){
						if (strcmp(attr->a_att_field, "crypto") == 0){
							cryptoStr = attr->a_att_value;
							/*extract the remote key and crypt. algorithms*/
							if(strstr(attr->a_att_value,"AES_CM_128_HMAC_SHA1_80")){
								/*libsrtp just implement counter mod for SDES*/
								payload.remoteCryptAlg=HMAC_SHA1;
							}else{ 
								payload.remoteCryptAlg=-1;
							}
							/*negoriation with crypt. algorithm*/
							payload.localCryptAlg=payload.remoteCryptAlg;
							tmpKey = strstr(attr->a_att_value,"inline:");
							if(tmpKey)
								remoteKey = tmpKey + strlen("inline:");
							if(strlen(remoteKey) >= SDP_KEY_LEN && payload.remoteCryptAlg != -1){
								unsigned char key[30];	//local srtp key	
								
								memcpy(&payload.remoteSDPKey, remoteKey, SDP_KEY_LEN);
								/* not reinvite case */
								if(!bReInvite){
									if(!sdp_generate_sdes_key(key, 30)){
										g_warning("Create sdes key fail!\n");
										continue;
									}else{
										base64Encode(key, payload.localSDPKey);
									}
								}
								g_message("+++++remote sdp key:%s+++++\n",payload.remoteSDPKey);
								g_message("+++++local sdp key:%s+++++\n",payload.localSDPKey);
								break;	
							}
						}	
					}
				}
#endif /*CONFIG_RTK_VOIP_SRTP*/
				/*-----end-----*/
				/*ericchung: add for check callee first codec ,E8C spec*/
				elem=lc->codecs_conf.audio_codecs;
				/* find first codec */
				printf("find first codec\n");
				while(elem!=NULL){
					lc_codec=(PayloadType*) elem->data;
					if (payload_type_usable(lc_codec) && payload_type_enabled(lc_codec)){
							sdp_payload_init(&lc_payload);
//						printf("%s %d  lc_codec->mime_type : %s \n",__FUNCTION__,__LINE__,lc_codec->mime_type);
						lc_payload.a_rtpmap=g_strdup_printf("%s/%i",lc_codec->mime_type,lc_codec->clock_rate);  //check my codec 
						//check incoming call , caller codec 

						for (j = 0; ((pt = sdp_message_m_payload_get (remote, i, j)) != NULL); j++){
				
							payload.pt = osip_atoi (pt);

							/* get the rtpmap associated to this codec, if any */
							payload.a_rtpmap =	sdp_message_a_attr_value_get_with_pt(remote, i, payload.pt,"rtpmap");
							if (payload.a_rtpmap != NULL){
	//							printf("Compare index %d peer_payload.a_rtpmap is %s payload.pt is %d\n",i,payload.a_rtpmap, payload.pt);
//								if (osip_strcasecmp(payload.a_rtpmap, lc_payload.a_rtpmap) == 0){
								if(strstr(payload.a_rtpmap, lc_payload.a_rtpmap)){
		//							printf("ya !! match my codec %s \n",payload.a_rtpmap);

									/* get the fmtp, if any */
									payload.a_fmtp =
									sdp_message_a_attr_value_get_with_pt
									(remote, i, payload.pt,
									 "fmtp");
#ifdef SUPPORT_V152_VBD
									/* get the gpmd, if any */
									payload.a_gpmd =
									sdp_message_a_attr_value_get_with_pt
									(remote, i, payload.pt,
									 "gpmd");
#endif
									printf("payload %d %s \n",payload.pt,payload.a_rtpmap);
									/* ask the application if this codec is supported */
									err = sdph->accept_audio_codecs (ctx, &payload);

									if (err != Unsupported && payload.localport > 0){				
										chose_code=j;
										break;
									}
								}
							}
                            /* Added by Alex, for INVITE without detailed rtpmap in SDP*/
                            else{
                                PayloadType *payLoad = rtp_profile_get_payload(lc->local_profile, payload.pt);
                                if(payLoad != NULL && lc_codec->type==payLoad->type){
                                    sprintf(tmpbuf,"%s/%i",payLoad->mime_type,payLoad->clock_rate);
                                    if (osip_strcasecmp(tmpbuf, lc_payload.a_rtpmap) == 0){
	                                    /* ask the application if this codec is supported */
										err = sdph->accept_audio_codecs (ctx, &payload);

										if (err != Unsupported && payload.localport > 0){				
											chose_code=j;
											break;
										}
                                    }
                                }
                            }
                            /* End of Added*/
						}
                        free(lc_payload.a_rtpmap);	//Alex, 20111021, Valgrind
					}
					if(chose_code==-1)
						elem=g_list_next(elem);
					else
						break;

				}


				if(chose_code!=-1){

					//printf("\r\n match codec is chose_code %d\n",chose_code);

					ncodec++;
					if (err == SupportedAndValid)
						mcodec++;

					/* codec accepted */
					if (ncodec == 1){
					/* first codec accepted, setup the line  */
						sdp_message_m_media_add
							(answer,
							 osip_strdup
							 (mtype),
							 int_2char
							 (payload.
							  localport),
							 NULL,
							 osip_strdup
							 (proto));
					}
						
					/* add the payload, rtpmap, fmtp */
					sdp_message_m_payload_add (answer, i,
							   int_2char
							   (payload.
							    pt));
						
					if (payload.a_rtpmap != NULL)
					{
						sdp_message_a_attribute_add
							(answer, i,
							 osip_strdup
							 ("rtpmap"),
							 sstrdup_sprintf
							 ("%i %s",
							  payload.pt,
							  payload.
							  a_rtpmap));
					}
					if (payload.a_fmtp != NULL)
					{
						sdp_message_a_attribute_add
							(answer, i,
							 osip_strdup
							 ("fmtp"),
							 sstrdup_sprintf
							 ("%i %s",
							  payload.pt,
							  payload.
							  a_fmtp));
					}
#ifdef SUPPORT_V152_VBD
					if (payload.a_gpmd != NULL)
					{
						sdp_message_a_attribute_add
							(answer, i,
							 osip_strdup
							 ("gpmd"),
							 sstrdup_sprintf
							 ("%i %s",
							  payload.pt,
							  payload.
							  a_gpmd));
					}
#endif
					if (payload.b_as_bandwidth !=
					    0)
					{
						if (sdp_message_bandwidth_get(answer,i,0)==NULL)
							sdp_message_b_bandwidth_add
							(answer, i,
							 osip_strdup
							 ("AS"),
							 sstrdup_sprintf
							 ("%i",
							  payload.
							  b_as_bandwidth));
					}
				}

#if 0			

				/*+++++ Added by Jack for SRTP 140108+++++*/
				/* ask if support RTP/SAVP */
				/* for each payload type */
				for (j = 0;
				     ((pt =
				       sdp_message_m_payload_get (remote, i,
							  j)) != NULL); j++)
				{
					payload.pt = osip_atoi (pt);
					/* get the rtpmap associated to this codec, if any */
					payload.a_rtpmap =
						sdp_message_a_attr_value_get_with_pt
						(remote, i, payload.pt,
						 "rtpmap");
					/* get the fmtp, if any */
					payload.a_fmtp =
						sdp_message_a_attr_value_get_with_pt
						(remote, i, payload.pt,
						 "fmtp");
#ifdef SUPPORT_V152_VBD
					/* get the gpmd, if any */
					payload.a_gpmd =
						sdp_message_a_attr_value_get_with_pt
						(remote, i, payload.pt,
						 "gpmd");
#endif
					
					/* ask the application if this codec is supported */
					err = sdph->accept_audio_codecs (ctx,
									 &payload);
					if (err != Unsupported && payload.localport > 0)
					{
						ncodec++;
						if (err == SupportedAndValid)
							mcodec++;

						/* codec accepted */
						if (ncodec == 1)
						{
							/* first codec accepted, setup the line  */
							sdp_message_m_media_add
								(answer,
								 osip_strdup
								 (mtype),
								 int_2char
								 (payload.
								  localport),
								 NULL,
								 osip_strdup
								 (proto));
						}
						/* add the payload, rtpmap, fmtp */
						sdp_message_m_payload_add (answer, i,
								   int_2char
								   (payload.
								    pt));
						if (payload.a_rtpmap != NULL)
						{
							sdp_message_a_attribute_add
								(answer, i,
								 osip_strdup
								 ("rtpmap"),
								 sstrdup_sprintf
								 ("%i %s",
								  payload.pt,
								  payload.
								  a_rtpmap));
						}
						if (payload.a_fmtp != NULL)
						{
							sdp_message_a_attribute_add
								(answer, i,
								 osip_strdup
								 ("fmtp"),
								 sstrdup_sprintf
								 ("%i %s",
								  payload.pt,
								  payload.
								  a_fmtp));
						}
#ifdef SUPPORT_V152_VBD
						if (payload.a_gpmd != NULL)
						{
							sdp_message_a_attribute_add
								(answer, i,
								 osip_strdup
								 ("gpmd"),
								 sstrdup_sprintf
								 ("%i %s",
								  payload.pt,
								  payload.
								  a_gpmd));
						}
#endif
						if (payload.b_as_bandwidth !=
						    0)
						{
							if (sdp_message_bandwidth_get(answer,i,0)==NULL)
								sdp_message_b_bandwidth_add
								(answer, i,
								 osip_strdup
								 ("AS"),
								 sstrdup_sprintf
								 ("%i",
								  payload.
								  b_as_bandwidth));
						}

					#if 0
						// support one codec only on response
						break;
					#endif
					}
				}
#endif					 
			
/*+++++ Added by Jack for SRTP 140108+++++*/
#if CONFIG_RTK_VOIP_SRTP
						if( lc->sip_conf.security_enable == 1 && payload.remoteCryptAlg == HMAC_SHA1){
							sdp_message_a_attribute_add
								(answer, i, osip_strdup("crypto"),
								sstrdup_sprintf("1 AES_CM_128_HMAC_SHA1_80 inline:%s",payload.localSDPKey));
							mcrypt++;
						}
#endif
/*-----end-----*/
				if (ncodec == 0)
				{
					/* refuse the line */
					refuse_mline(answer,mtype,proto,i);
					
				}
#if CONFIG_RTK_VOIP_SRTP
				/* if security is enable only security call is accepted */
				else if((lc->sip_conf.security_enable == 1 && mcrypt > 0) || (lc->sip_conf.security_enable == 0 && keywordcmp("RTP/SAVP", proto) != 0))
					if (mcodec > 0)
						m_lines_accepted++;
#else
				else if (mcodec > 0)
					m_lines_accepted++;
#endif
			}
			else
			{
				/* refuse this line (leave port to 0) */
				refuse_mline(answer,mtype,proto,i);
			}

		}/*++T.38 added by Jack Chan 25/01/07 for VoIP++*/
		else if(keywordcmp ("image", mtype) == 0){
			sdp_attribute_t *sdpAtb;
			char *mediaPlayload = NULL;
			StreamParams *params;
			
			//t.38 setting not enable
			if(FALSE == lc->sip_conf.T38_enable){
				ctx->negoc_status = 488;
			#if 0
				sdp_message_free(ctx->answer);
				if (ctx->answerstr)
						osip_free(ctx->answerstr);
			#endif
				sdp_message_free(answer);
				//sdp_message_free(remote);
				g_message("+++++debug:t.38 setting not enable+++++\n");
				return NULL;
			}
			mediaPlayload = sdp_message_m_payload_get(remote, i, 0);
			if(0 != osip_strcasecmp(mediaPlayload, "t38"))
			{
				ctx->negoc_status = 488;
			#if 0
				sdp_message_free(ctx->answer);
				if (ctx->answerstr)
						osip_free(ctx->answerstr);
			#endif
				sdp_message_free(answer);
				//sdp_message_free(remote);
				g_warning("+++++debug: unknown image type %s +++++\n", mediaPlayload);
				return NULL;
			}

			params = &call->t38_params ;
			if(lc->sip_conf.T38_prot > 0)
				params->localport = lc->sip_conf.T38_prot;

			params->remoteport = payload.remoteport ;
			params->line = payload.line ;
		#ifdef DYNAMIC_PAYLOAD
			params->local_pt = rtpPayloadT38_Virtual;
			params->remote_pt = rtpPayloadT38_Virtual;
			params->static_pt = rtpPayloadT38_Virtual;

		#ifdef AUTO_RESOURCE
			if (VOIP_RESOURCE_UNAVAILABLE == rcm_tapi_VoIP_resource_check(lc->chid, params->static_pt))
			{
				ctx->negoc_status = 488;
				sdp_message_free(answer);
				g_warning("no resource for T38\n");
				return NULL;
			}
		#endif

		#else
			params->pt = rtpPayloadT38_Virtual; 
		#endif // DYNAMIC_PAYLOAD
			//params->remoteaddr = payload.c_addr ;
			bzero(&params->remote_addr, sizeof(params->remote_addr));
			strcpy(params->remote_addr,payload.c_addr);
			params->remote_ip=inet_addr(payload.c_addr);
			params->initialized = 1 ;

			sdp_message_m_media_add(answer,osip_strdup(mtype),int_2char(params->localport),NULL,osip_strdup(proto));
			if(NULL != mediaPlayload)
				sdp_message_m_payload_add(answer, i, osip_strdup(mediaPlayload));
			for(j = 0; (sdpAtb = sdp_message_attribute_get(remote, i, j)) != NULL; j++){
				sdp_message_a_attribute_add(answer, i, osip_strdup(sdpAtb->a_att_field), osip_strdup(sdpAtb->a_att_value));				
			}
			m_lines_accepted++;
		}
		/*--end--*/
	}
#if 1
	// rock: for reinvite issue
	if (ctx->answer)
	{
		// release old answer
		sdp_message_free(ctx->answer);
	}
#endif

	ctx->answer = answer;
	if (m_lines_accepted > 0)
	{
#ifdef DYNAMIC_PAYLOAD
		StreamParams *audio_params = &call->audio_params;
//printf("\r\n dynamic_payload check \n");

#if 1
		if (lc->sip_conf.dtmf_type != DTMF_INBAND)		// No need telephone-event if DTMF In Band 
		{
			/* add telephone-event payload*/
			char payload_buf[100];
			audio_params->rfc2833_remote_pt = linphone_core_get_rfc2833_pt(ctx->remote);
			if(audio_params->rfc2833_remote_pt!=0){
			sprintf(payload_buf, "%d", audio_params->rfc2833_remote_pt);			
		  	sdp_message_m_payload_add(answer, 0, osip_strdup(payload_buf));
			sprintf(payload_buf, "%d telephone-event/8000", audio_params->rfc2833_remote_pt);			
		  	sdp_message_a_attribute_add(answer, 0, osip_strdup("rtpmap"), osip_strdup(payload_buf));
			sprintf(payload_buf, "%d 0-15", audio_params->rfc2833_remote_pt);			
		  	sdp_message_a_attribute_add(answer, 0, osip_strdup("fmtp"), osip_strdup(payload_buf));
		}
		}
#endif
//		audio_params->rfc2833_local_pt = linphone_core_get_rfc2833_pt(ctx->answer);
//		printf("\r\n audio_params rfc2833_local_pt is  %d\n",audio_params->rfc2833_local_pt );
		audio_params->rfc2833_local_pt=linphone_core_get_rfc2833_pt(ctx->remote);

		audio_params->rfc2833_remote_pt = linphone_core_get_rfc2833_pt(ctx->remote);
		//printf("\r\n audio_params rfc2833_remote_pt is  %d\n",audio_params->rfc2833_remote_pt );
#else
		if (lc->sip_conf.dtmf_type != DTMF_INBAND)		// No need telephone-event if DTMF In Band 
		{
			/* add telephone-event payload*/
			char payload_buf[100];
			
			sprintf(payload_buf, "%d", lc->sip_conf.rfc2833_payload_type);			
		  	sdp_message_m_payload_add(answer, 0, osip_strdup(payload_buf));
			sprintf(payload_buf, "%d telephone-event/8000", lc->sip_conf.rfc2833_payload_type);			
		  	sdp_message_a_attribute_add(answer, 0, osip_strdup("rtpmap"), osip_strdup(payload_buf));
			sprintf(payload_buf, "%d 0-15", lc->sip_conf.rfc2833_payload_type);			
		  	sdp_message_a_attribute_add(answer, 0, osip_strdup("fmtp"), osip_strdup(payload_buf));
		}
#endif // DYNAMIC_PAYLOAD

#if 0
		// add sendrecv attribute
		if( 0 != keywordcmp( "udptl", proto ) ){	/* Not a T.38 re-INVITE */
			int k;
			for(i=-1; !sdp_message_endof_media(remote, i); i++){
				char *sendRecv = NULL;
				for(k=0; NULL!=(sendRecv = sdp_message_a_att_field_get(remote, i, k)); k++){
					if( 0 == osip_strcasecmp(sendRecv, "sendonly"))   /*for hold message*/
					{
						sdp_message_a_attribute_add(answer, 0, osip_strdup ("recvonly"), NULL);
						// mix 2543 & 3264 hold
						if (answer->c_connection)
						{
							osip_free(answer->c_connection->c_addr);
							answer->c_connection->c_addr = osip_strdup("0.0.0.0");
						}
					}
					else if( 0 == osip_strcasecmp(sendRecv, "sendrecv"))
						sdp_message_a_attribute_add(answer, 0, osip_strdup ("sendrecv"), NULL);			
				}
			}
		}
#endif

		ctx->negoc_status = 200;

		/* add send,receive on sdp*/
		osip_negotiation_sdp_message_put_media_mode(answer, MEDIA_SENDRECV);

		sdp_message_to_str(answer,&tmp);
#ifdef FIX_MEMORY_LEAK
		// for reinvite issue
		if (ctx->answerstr)
			osip_free(ctx->answerstr);
#endif
		ctx->answerstr=tmp;
		return tmp;
	}else{
		ctx->negoc_status = 415;
		return NULL;
	}
}

#if 0
char *
sdp_context_set_answer ( sdp_context_t *ctx,const char *remote_offer)
{
	sdp_message_t *remote=NULL;
	char *mtype=NULL, *tmp=NULL, *bw=NULL;
	char *proto=NULL, *port=NULL, *pt=NULL;
	int i, j;
	int err;
	sdp_payload_t payload;
	sdp_handler_t *sdph=ctx->handler;

	LinphoneCall *call=(LinphoneCall*)sdp_context_get_user_pointer(ctx);
	LinphoneCore *lc=call->core;
	
//printf( "\n oooaudio_params-> localport: %d, remoteport: %d.", call->audio_params.localport , call->audio_params.remoteport ) ;
	sdp_message_init(&remote);
	err=sdp_message_parse(remote,remote_offer);
	if (err<0) {
		eXosip_trace(OSIP_WARNING,("Could not parse remote offer."));
		sdp_message_free(remote);
		return NULL;
	}

	{
		if (ctx->answer)
		{
			// release old remote
			sdp_message_free(ctx->answer);
		}
		//call->audio_params.initialized = 0;
	}	
	ctx->answer=remote;

	/* for each m= line */
	for( i = 0 ; !sdp_message_endof_media( remote , i ) ; i++ )
	{
		sdp_payload_init( &payload ) ;
		mtype = sdp_message_m_media_get( remote , i ) ;
		proto = sdp_message_m_proto_get( remote , i ) ;
		port = sdp_message_m_port_get( remote , i ) ;
		payload.remoteport = osip_atoi( port ) ;

		payload.localport = osip_atoi( sdp_message_m_port_get( ctx->offer , i ) ) ;
		payload.proto = proto ;
		payload.line = i ;
		payload.c_addr = sdp_message_c_addr_get( remote , i , 0 ) ;
		if( payload.c_addr == NULL )
			payload.c_addr = sdp_message_c_addr_get( remote , -1 , 0 ) ;
		bw = sdp_message_b_bandwidth_get( remote , i , 0 ) ;
		if( bw != NULL )
			payload.b_as_bandwidth = atoi( bw ) ;

		if( keywordcmp( "audio" , mtype ) == 0 )
		{
			if( sdph->get_audio_codecs != NULL )
			{
				/* for each payload type */
				for( j = 0 ; ( ( pt = sdp_message_m_payload_get( remote , i , j) ) != NULL ) ; j++ )
				{
					payload.pt = osip_atoi( pt ) ;
					/* get the rtpmap associated to this codec, if any */
					payload.a_rtpmap = sdp_message_a_attr_value_get_with_pt( remote , i , payload.pt , "rtpmap" ) ;
					/* get the fmtp, if any */
					payload.a_fmtp = sdp_message_a_attr_value_get_with_pt( remote , i , payload.pt , "fmtp" ) ;
					/* ask the application if this codec is supported */
					err = sdph->get_audio_codecs( ctx , &payload ) ;
				}
			}
		}
		else if(keywordcmp ("image", mtype) == 0)
		{
			if( sdph->get_t38_config != NULL )
			{
				/* for each payload type */
				for( j = 0 ; ( ( pt = sdp_message_m_payload_get( remote , i , j) ) != NULL ) ; j++ )
				{
					payload.pt = osip_atoi( pt ) ;
					err = sdph->get_t38_config( ctx , &payload ) ;
				}
			}
		}
	}
#if 1
	/* Mandy add for stun support */
	if (lc->net_conf.nat_address!=NULL && lc->net_conf.use_nat){
		port = (char*)osip_malloc(10);
		if (call->t38_params.initialized)
		{
			snprintf(port, 9, "%i", lc->net_conf.nat_t38_port);
			g_message("The nat_t38_port is %d.\n\r", lc->net_conf.nat_t38_port);	
		}
		else
		{
			snprintf(port, 9, "%i", lc->net_conf.nat_voice_port);
			g_message("The nat_voice_port is %d.\n\r", lc->net_conf.nat_voice_port);	
		}
		sdp_message_m_port_set(remote, 0, port);
	}
#endif
	

	sdp_message_to_str(ctx->answer,&tmp);
#ifdef FIX_MEMORY_LEAK
	if (ctx->answerstr)
		osip_free(ctx->answerstr);

      //sdp_message_free(remote);
#endif

	ctx->answerstr=tmp;
	return tmp;
}
#endif


//make outgoing call,  peer answer the call , read sdp from remote 200ok  

void
rcm_sdp_context_read_answer_by_ev (sdp_context_t *ctx, eXosip_event_t *ev)
{
	sdp_message_t *remote;
	char *mtype;
	char *proto, *port, *pt, *bw;
	int i, j,err;
	sdp_payload_t payload;
	sdp_handler_t *sdph=ctx->handler;
	
	g_message("\r\n in rcm_sdp_context_read_answer_by_ev \n");


	if(sdph==NULL)
		g_message("\r\n sdph is null!\n");
	remote = eXosip_get_sdp_info(ev->response);

	if(remote==NULL){
		g_message("\r\n this response no include SDP ,return \n");
		return ;
	}

#if 0
	{
		char *strSDP;
		sdp_message_to_str(remote, &strSDP);
		g_message("+++++bb DEBUG:SDP Message:%s+++++\n",strSDP);
		osip_free(strSDP);
	}

#endif

g_message("\r\n in rm_sdp_context_read_answer_by_ev aa \n");

#if 1
	// rock: for reinvite issue
	{
		LinphoneCall * call = ( LinphoneCall * )sdp_context_get_user_pointer( ctx ) ;

		if (ctx->remote)
		{
			// release old remote
			sdp_message_free(ctx->remote);
		}

		call->audio_params.initialized = 0;
		call->audio_params.initialized_vbd = 0;
#ifdef SUPPORT_RTP_REDUNDANT_APP
		call->audio_params.initialized_rtp_redundant = 0;
#endif
		/*++added by Jack Chan 02/01/07 for t.38++*/
		call->t38_params.initialized = 0;
		/**--end--*/
	}	
#endif
	g_message("\r\n in rm_sdp_context_read_answer_by_ev  003 \n");

	ctx->remote=remote;
	/* for each m= line */
	for( i = 0 ; !sdp_message_endof_media( remote , i ) ; i++ )
	{

	
		g_message("\r\n in rm_sdp_context_read_answer_by_ev  004 run %d\n",i);
		sdp_payload_init( &payload ) ;
		mtype = sdp_message_m_media_get( remote , i ) ;
		proto = sdp_message_m_proto_get( remote , i ) ;
		port = sdp_message_m_port_get( remote , i ) ;
		payload.remoteport = osip_atoi( port ) ;

		if(ctx->offer == NULL)
			g_message("\n offer NULL");

		payload.localport = osip_atoi( sdp_message_m_port_get( ctx->offer , i ) ) ;
		payload.proto = proto ;
		payload.line = i ;
		payload.c_addr = sdp_message_c_addr_get( remote , i , 0 ) ;
		g_message("\r\n in rm_sdp_context_read_answer_by_ev  005 \n");
		if( payload.c_addr == NULL )
			payload.c_addr = sdp_message_c_addr_get( remote , -1 , 0 ) ;




//	#ifdef CHECK_LOOPBACK	
#if 0//eric
		// check loopback mode
		if(	ctx->rtp_localip == NULL){
			printf("	ctx->rtp_localip is null ");
			ctx->loopback=0;
		}else{
			ctx->loopback = strcmp(payload.c_addr, ctx->rtp_localip) == 0;
		}
	#endif

		bw = sdp_message_b_bandwidth_get( remote , i , 0 ) ;
	g_message("\r\n in rm_sdp_context_read_answer_by_ev  006 \n");
		if( bw != NULL )
			payload.b_as_bandwidth = atoi( bw ) ;
		if( keywordcmp( "audio" , mtype ) == 0 )
		{
		g_message("\r\n in rm_sdp_context_read_answer_by_ev  007 \n");
			if( sdph->get_audio_codecs != NULL )
			{

			g_message("\r\n in rm_sdp_context_read_answer_by_ev  008 \n");
#ifdef CONFIG_RTK_VOIP_SRTP
				if(keywordcmp("RTP/SAVP", proto) == 0){
					sdp_attribute_t *attr;
					char *localKey=NULL;
					char *remoteKey=NULL;
					int localCryptAlg;
					int remoteCryptAlg;
					char *tmpKey=NULL;
					char *cryptoStr=NULL;
					int pos;

					/* extract the remote SDP key*/
					for (pos=0; (attr=sdp_message_attribute_get(remote, i, pos))!=NULL; pos++){
						if (strcmp(attr->a_att_field, "crypto") == 0){
							cryptoStr = attr->a_att_value;
							/*extract the remote key and crypt. algorithms*/
							if(strstr(attr->a_att_value,"AES_CM_128_HMAC_SHA1_80")){
								/*libsrtp just implement counter mod for SDES*/
								payload.remoteCryptAlg=HMAC_SHA1;
							}else{
								payload.remoteCryptAlg=-1;
							}
							/*negoriation with crypt. algorithm*/
							//payload.localCryptAlg=payload.remoteCryptAlg;
							tmpKey = strstr(attr->a_att_value,"inline:");
							if(tmpKey)
								remoteKey = tmpKey + strlen("inline:");
							memcpy(&payload.remoteSDPKey, remoteKey, SDP_KEY_LEN);
							g_message("+++++remote sdp key:%s+++++\n",payload.remoteSDPKey);
							break;
						}
					}

					g_message("\r\n in rm_sdp_context_read_answer_by_ev  009 \n");
					/* extract the local SDPKey */

					if(ctx->offer == NULL)
						printf("ctx->offer is null \n");
					
					for (pos=0; (attr=sdp_message_attribute_get(ctx->offer, i, pos))!=NULL; pos++){
						if (strcmp(attr->a_att_field, "crypto") == 0){
							/*extract the remote key and crypt. algorithms*/
							if(strstr(attr->a_att_value,"AES_CM_128_HMAC_SHA1_80")){
								/*libsrtp just implement counter mod for SDES*/
								payload.localCryptAlg=HMAC_SHA1;
							}else{
								payload.localCryptAlg=-1;
							}
							if(payload.remoteCryptAlg != payload.localCryptAlg)
								g_warning("crypt. algorithm does not match!\n");
							tmpKey = strstr(attr->a_att_value,"inline:");
							if(tmpKey)
								localKey = tmpKey + strlen("inline:");
							memcpy(&payload.localSDPKey, localKey, SDP_KEY_LEN);
							g_message("+++++local sdp key:%s+++++\n",payload.localSDPKey);
							break;
						}
					}
				}
				g_message("\r\n in rm_sdp_context_read_answer_by_ev  010 \n");
#endif /* CONFIG_RTK_VOIP_SRTP */
				/* for each payload type */
				g_message("\r\n in rm_sdp_context_read_answer_by_ev  011 \n");

				for( j = 0 ; ( ( pt = sdp_message_m_payload_get( remote , i , j) ) != NULL ) ; j++ )
				{
					payload.pt = osip_atoi( pt ) ;
					/* get the rtpmap associated to this codec, if any */
					payload.a_rtpmap = sdp_message_a_attr_value_get_with_pt( remote , i , payload.pt , "rtpmap" ) ;
					/* get the fmtp, if any */
					payload.a_fmtp = sdp_message_a_attr_value_get_with_pt( remote , i , payload.pt , "fmtp" ) ;
#ifdef SUPPORT_V152_VBD
					/* get the gpmd, if any */
					payload.a_gpmd = sdp_message_a_attr_value_get_with_pt( remote , i , payload.pt , "gpmd" ) ;
#endif
					/* ask the application if this codec is supported */
					err = sdph->get_audio_codecs( ctx , &payload ) ;
				}
			}
		}/*++added by Jack Chan 02/02/07 for t38++*/
		else if( keywordcmp( "image" , mtype ) == 0 ){
			g_message("\r\n in rm_sdp_context_read_answer_by_ev  012 \n");
			if( sdph->get_t38_config!= NULL )
			{
				/* for each payload type */
				for( j = 0 ; ( ( pt = sdp_message_m_payload_get( remote , i , j) ) != NULL ) ; j++ )
				{
					if(0 != osip_strcasecmp("t38", pt))
					{
						g_warning("+++++debug: unknown image type %s +++++\n", pt);
						continue;
					}
					payload.pt = rtpPayloadT38_Virtual;
					/* get the rtpmap associated to this codec, if any */
					payload.a_rtpmap = sdp_message_a_attr_value_get_with_pt( remote , i , payload.pt , "rtpmap" ) ;
					/* get the fmtp, if any */
					payload.a_fmtp = sdp_message_a_attr_value_get_with_pt( remote , i , payload.pt , "fmtp" ) ;
					/* ask the application if this codec is supported */
					err = sdph->get_t38_config( ctx , &payload ) ;
				}
			}
			/*--end--*/
		}
	}
	g_message("\r\n in rm_sdp_context_read_answer_by_ev  005\n");

	
#ifdef DYNAMIC_PAYLOAD
{

	LinphoneCall *call = (LinphoneCall *) sdp_context_get_user_pointer(ctx);
	StreamParams *audio_params = &call->audio_params;
	audio_params->rfc2833_local_pt = linphone_core_get_rfc2833_pt(ctx->offer);
	audio_params->rfc2833_remote_pt = linphone_core_get_rfc2833_pt(ctx->remote);
}
#endif // DYNAMIC_PAYLOAD
}


//make outgoing call,  peer answer the call , read sdp from remote 200ok  
void
rcm_sdp_context_read_answer_by_sdp (sdp_context_t *ctx, osip_message_t *sip_message)
{
	sdp_message_t *remote;
	char *mtype;
	char *proto, *port, *pt, *bw;
	int i, j,err;
	sdp_payload_t payload;
	sdp_handler_t *sdph=ctx->handler;
	
	g_message("\r\n in rcm_sdp_context_read_answer_by_ev_ack \n");


	if(sdph==NULL)
		g_message("\r\n sdph is null!\n");
	
	remote = eXosip_get_sdp_info(sip_message);

	if(remote==NULL){
		g_message("\r\n this ack no include SDP ,return \n");
		return ;
	}

#if 0
	{
		char *strSDP;
		sdp_message_to_str(remote, &strSDP);
		g_message("+++++bb DEBUG:SDP Message:%s+++++\n",strSDP);
		osip_free(strSDP);
	}

#endif

g_message("\r\n in rm_sdp_context_read_answer_by_ev aa \n");

#if 1
	// rock: for reinvite issue
	{
		LinphoneCall * call = ( LinphoneCall * )sdp_context_get_user_pointer( ctx ) ;

		if (ctx->remote)
		{
			// release old remote
			sdp_message_free(ctx->remote);
		}

		call->audio_params.initialized = 0;
		call->audio_params.initialized_vbd = 0;
#ifdef SUPPORT_RTP_REDUNDANT_APP
		call->audio_params.initialized_rtp_redundant = 0;
#endif
		/*++added by Jack Chan 02/01/07 for t.38++*/
		call->t38_params.initialized = 0;
		/**--end--*/
	}	
#endif
	g_message("\r\n in rm_sdp_context_read_answer_by_ev  003 \n");

	ctx->remote=remote;
	/* for each m= line */
	for( i = 0 ; !sdp_message_endof_media( remote , i ) ; i++ )
	{

	
		g_message("\r\n in rm_sdp_context_read_answer_by_ev  004 run %d\n",i);
		sdp_payload_init( &payload ) ;
		mtype = sdp_message_m_media_get( remote , i ) ;
		proto = sdp_message_m_proto_get( remote , i ) ;
		port = sdp_message_m_port_get( remote , i ) ;
		payload.remoteport = osip_atoi( port ) ;

		if(ctx->offer == NULL)
			g_message("\n offer NULL");

		payload.localport = osip_atoi( sdp_message_m_port_get( ctx->offer , i ) ) ;
		payload.proto = proto ;
		payload.line = i ;
		payload.c_addr = sdp_message_c_addr_get( remote , i , 0 ) ;
		g_message("\r\n in rm_sdp_context_read_answer_by_ev  005 \n");
		if( payload.c_addr == NULL )
			payload.c_addr = sdp_message_c_addr_get( remote , -1 , 0 ) ;




//	#ifdef CHECK_LOOPBACK	
#if 0//eric
		// check loopback mode
		if(	ctx->rtp_localip == NULL){
			printf("	ctx->rtp_localip is null ");
			ctx->loopback=0;
		}else{
			ctx->loopback = strcmp(payload.c_addr, ctx->rtp_localip) == 0;
		}
	#endif

		bw = sdp_message_b_bandwidth_get( remote , i , 0 ) ;
	g_message("\r\n in rm_sdp_context_read_answer_by_ev  006 \n");
		if( bw != NULL )
			payload.b_as_bandwidth = atoi( bw ) ;
		if( keywordcmp( "audio" , mtype ) == 0 )
		{
		g_message("\r\n in rm_sdp_context_read_answer_by_ev  007 \n");
			if( sdph->get_audio_codecs != NULL )
			{

			g_message("\r\n in rm_sdp_context_read_answer_by_ev  008 \n");
#ifdef CONFIG_RTK_VOIP_SRTP
				if(keywordcmp("RTP/SAVP", proto) == 0){
					sdp_attribute_t *attr;
					char *localKey=NULL;
					char *remoteKey=NULL;
					int localCryptAlg;
					int remoteCryptAlg;
					char *tmpKey=NULL;
					char *cryptoStr=NULL;
					int pos;

					/* extract the remote SDP key*/
					for (pos=0; (attr=sdp_message_attribute_get(remote, i, pos))!=NULL; pos++){
						if (strcmp(attr->a_att_field, "crypto") == 0){
							cryptoStr = attr->a_att_value;
							/*extract the remote key and crypt. algorithms*/
							if(strstr(attr->a_att_value,"AES_CM_128_HMAC_SHA1_80")){
								/*libsrtp just implement counter mod for SDES*/
								payload.remoteCryptAlg=HMAC_SHA1;
							}else{
								payload.remoteCryptAlg=-1;
							}
							/*negoriation with crypt. algorithm*/
							//payload.localCryptAlg=payload.remoteCryptAlg;
							tmpKey = strstr(attr->a_att_value,"inline:");
							if(tmpKey)
								remoteKey = tmpKey + strlen("inline:");
							memcpy(&payload.remoteSDPKey, remoteKey, SDP_KEY_LEN);
							g_message("+++++remote sdp key:%s+++++\n",payload.remoteSDPKey);
							break;
						}
					}

					g_message("\r\n in rm_sdp_context_read_answer_by_ev  009 \n");
					/* extract the local SDPKey */

					if(ctx->offer == NULL)
						printf("ctx->offer is null \n");
					
					for (pos=0; (attr=sdp_message_attribute_get(ctx->offer, i, pos))!=NULL; pos++){
						if (strcmp(attr->a_att_field, "crypto") == 0){
							/*extract the remote key and crypt. algorithms*/
							if(strstr(attr->a_att_value,"AES_CM_128_HMAC_SHA1_80")){
								/*libsrtp just implement counter mod for SDES*/
								payload.localCryptAlg=HMAC_SHA1;
							}else{
								payload.localCryptAlg=-1;
							}
							if(payload.remoteCryptAlg != payload.localCryptAlg)
								g_warning("crypt. algorithm does not match!\n");
							tmpKey = strstr(attr->a_att_value,"inline:");
							if(tmpKey)
								localKey = tmpKey + strlen("inline:");
							memcpy(&payload.localSDPKey, localKey, SDP_KEY_LEN);
							g_message("+++++local sdp key:%s+++++\n",payload.localSDPKey);
							break;
						}
					}
				}
				g_message("\r\n in rm_sdp_context_read_answer_by_ev  010 \n");
#endif /* CONFIG_RTK_VOIP_SRTP */
				/* for each payload type */
				g_message("\r\n in rm_sdp_context_read_answer_by_ev  011 \n");

				for( j = 0 ; ( ( pt = sdp_message_m_payload_get( remote , i , j) ) != NULL ) ; j++ )
				{
					payload.pt = osip_atoi( pt ) ;
					/* get the rtpmap associated to this codec, if any */
					payload.a_rtpmap = sdp_message_a_attr_value_get_with_pt( remote , i , payload.pt , "rtpmap" ) ;
					/* get the fmtp, if any */
					payload.a_fmtp = sdp_message_a_attr_value_get_with_pt( remote , i , payload.pt , "fmtp" ) ;
#ifdef SUPPORT_V152_VBD
					/* get the gpmd, if any */
					payload.a_gpmd = sdp_message_a_attr_value_get_with_pt( remote , i , payload.pt , "gpmd" ) ;
#endif
					/* ask the application if this codec is supported */
					err = sdph->get_audio_codecs( ctx , &payload ) ;
				}
			}
		}/*++added by Jack Chan 02/02/07 for t38++*/
		else if( keywordcmp( "image" , mtype ) == 0 ){
			g_message("\r\n in rm_sdp_context_read_answer_by_ev  012 \n");
			if( sdph->get_t38_config!= NULL )
			{
				/* for each payload type */
				for( j = 0 ; ( ( pt = sdp_message_m_payload_get( remote , i , j) ) != NULL ) ; j++ )
				{
					if(0 != osip_strcasecmp("t38", pt))
					{
						g_warning("+++++debug: unknown image type %s +++++\n", pt);
						continue;
					}
					payload.pt = rtpPayloadT38_Virtual;
					/* get the rtpmap associated to this codec, if any */
					payload.a_rtpmap = sdp_message_a_attr_value_get_with_pt( remote , i , payload.pt , "rtpmap" ) ;
					/* get the fmtp, if any */
					payload.a_fmtp = sdp_message_a_attr_value_get_with_pt( remote , i , payload.pt , "fmtp" ) ;
					/* ask the application if this codec is supported */
					err = sdph->get_t38_config( ctx , &payload ) ;
				}
			}
			/*--end--*/
		}
	}
	g_message("\r\n in rm_sdp_context_read_answer_by_ev  005\n");

	
#ifdef DYNAMIC_PAYLOAD
{

	LinphoneCall *call = (LinphoneCall *) sdp_context_get_user_pointer(ctx);
	StreamParams *audio_params = &call->audio_params;
	audio_params->rfc2833_local_pt = linphone_core_get_rfc2833_pt(ctx->offer);
	audio_params->rfc2833_remote_pt = linphone_core_get_rfc2833_pt(ctx->remote);
}
#endif // DYNAMIC_PAYLOAD
} 

//make outgoing call,  peer answer the call , read sdp from remote 200ok  
void
sdp_context_read_answer (sdp_context_t *ctx, const char *remote_answer)
{
	sdp_message_t *remote;
	char *mtype;
	char *proto, *port, *pt, *bw;
	int i, j,err;
	sdp_payload_t payload;
	sdp_handler_t *sdph=ctx->handler;

	sdp_message_init(&remote);
	err=sdp_message_parse(remote,remote_answer);
	if (err<0) {
		eXosip_trace(OSIP_WARNING,("Could not parse remote offer."));
#if 1
		// rock: handle memory leak
		sdp_message_free(remote);
#endif
		return ;
	}

#if 1
	// rock: for reinvite issue
	{
		LinphoneCall * call = ( LinphoneCall * )sdp_context_get_user_pointer( ctx ) ;

		if (ctx->remote)
		{
			// release old remote
			sdp_message_free(ctx->remote);
		}

		call->audio_params.initialized = 0;
		call->audio_params.initialized_vbd = 0;
#ifdef SUPPORT_RTP_REDUNDANT_APP
		call->audio_params.initialized_rtp_redundant = 0;
#endif
		/*++added by Jack Chan 02/01/07 for t.38++*/
		call->t38_params.initialized = 0;
		/**--end--*/
	}	
#endif

	ctx->remote=remote;
	/* for each m= line */
	for( i = 0 ; !sdp_message_endof_media( remote , i ) ; i++ )
	{
		sdp_payload_init( &payload ) ;
		mtype = sdp_message_m_media_get( remote , i ) ;
		proto = sdp_message_m_proto_get( remote , i ) ;
		port = sdp_message_m_port_get( remote , i ) ;
		payload.remoteport = osip_atoi( port ) ;

if(ctx->offer == NULL)
printf("\n offer NULL");

		payload.localport = osip_atoi( sdp_message_m_port_get( ctx->offer , i ) ) ;
		payload.proto = proto ;
		payload.line = i ;
		payload.c_addr = sdp_message_c_addr_get( remote , i , 0 ) ;
		if( payload.c_addr == NULL )
			payload.c_addr = sdp_message_c_addr_get( remote , -1 , 0 ) ;

	#ifdef CHECK_LOOPBACK	
		// check loopback mode
		ctx->loopback = strcmp(payload.c_addr, ctx->rtp_localip) == 0;
	#endif

		bw = sdp_message_b_bandwidth_get( remote , i , 0 ) ;
		if( bw != NULL )
			payload.b_as_bandwidth = atoi( bw ) ;
		if( keywordcmp( "audio" , mtype ) == 0 )
		{
			if( sdph->get_audio_codecs != NULL )
			{
#ifdef CONFIG_RTK_VOIP_SRTP
				if(keywordcmp("RTP/SAVP", proto) == 0){
					sdp_attribute_t *attr;
					char *localKey=NULL;
					char *remoteKey=NULL;
					int localCryptAlg;
					int remoteCryptAlg;
					char *tmpKey=NULL;
					char *cryptoStr=NULL;
					int pos;

					/* extract the remote SDP key*/
					for (pos=0; (attr=sdp_message_attribute_get(remote, i, pos))!=NULL; pos++){
						if (strcmp(attr->a_att_field, "crypto") == 0){
							cryptoStr = attr->a_att_value;
							/*extract the remote key and crypt. algorithms*/
							if(strstr(attr->a_att_value,"AES_CM_128_HMAC_SHA1_80")){
								/*libsrtp just implement counter mod for SDES*/
								payload.remoteCryptAlg=HMAC_SHA1;
							}else{
								payload.remoteCryptAlg=-1;
							}
							/*negoriation with crypt. algorithm*/
							//payload.localCryptAlg=payload.remoteCryptAlg;
							tmpKey = strstr(attr->a_att_value,"inline:");
							if(tmpKey)
								remoteKey = tmpKey + strlen("inline:");
							memcpy(&payload.remoteSDPKey, remoteKey, SDP_KEY_LEN);
							g_message("+++++remote sdp key:%s+++++\n",payload.remoteSDPKey);
							break;
						}
					}
					/* extract the local SDPKey */
					for (pos=0; (attr=sdp_message_attribute_get(ctx->offer, i, pos))!=NULL; pos++){
						if (strcmp(attr->a_att_field, "crypto") == 0){
							/*extract the remote key and crypt. algorithms*/
							if(strstr(attr->a_att_value,"AES_CM_128_HMAC_SHA1_80")){
								/*libsrtp just implement counter mod for SDES*/
								payload.localCryptAlg=HMAC_SHA1;
							}else{
								payload.localCryptAlg=-1;
							}
							if(payload.remoteCryptAlg != payload.localCryptAlg)
								g_warning("crypt. algorithm does not match!\n");
							tmpKey = strstr(attr->a_att_value,"inline:");
							if(tmpKey)
								localKey = tmpKey + strlen("inline:");
							memcpy(&payload.localSDPKey, localKey, SDP_KEY_LEN);
							g_message("+++++local sdp key:%s+++++\n",payload.localSDPKey);
							break;
						}
					}
				}
#endif /* CONFIG_RTK_VOIP_SRTP */
				/* for each payload type */
				for( j = 0 ; ( ( pt = sdp_message_m_payload_get( remote , i , j) ) != NULL ) ; j++ )
				{
					payload.pt = osip_atoi( pt ) ;
					/* get the rtpmap associated to this codec, if any */
					payload.a_rtpmap = sdp_message_a_attr_value_get_with_pt( remote , i , payload.pt , "rtpmap" ) ;
					/* get the fmtp, if any */
					payload.a_fmtp = sdp_message_a_attr_value_get_with_pt( remote , i , payload.pt , "fmtp" ) ;
#ifdef SUPPORT_V152_VBD
					/* get the gpmd, if any */
					payload.a_gpmd = sdp_message_a_attr_value_get_with_pt( remote , i , payload.pt , "gpmd" ) ;
#endif
					/* ask the application if this codec is supported */
					err = sdph->get_audio_codecs( ctx , &payload ) ;
				}
			}
		}/*++added by Jack Chan 02/02/07 for t38++*/
		else if( keywordcmp( "image" , mtype ) == 0 ){
			if( sdph->get_t38_config!= NULL )
			{
				/* for each payload type */
				for( j = 0 ; ( ( pt = sdp_message_m_payload_get( remote , i , j) ) != NULL ) ; j++ )
				{
					if(0 != osip_strcasecmp("t38", pt))
					{
						g_warning("+++++debug: unknown image type %s +++++\n", pt);
						continue;
					}
					payload.pt = rtpPayloadT38_Virtual;
					/* get the rtpmap associated to this codec, if any */
					payload.a_rtpmap = sdp_message_a_attr_value_get_with_pt( remote , i , payload.pt , "rtpmap" ) ;
					/* get the fmtp, if any */
					payload.a_fmtp = sdp_message_a_attr_value_get_with_pt( remote , i , payload.pt , "fmtp" ) ;
					/* ask the application if this codec is supported */
					err = sdph->get_t38_config( ctx , &payload ) ;
				}
			}
			/*--end--*/
		}
	}
#ifdef DYNAMIC_PAYLOAD
{
	LinphoneCall *call = (LinphoneCall *) sdp_context_get_user_pointer(ctx);
	StreamParams *audio_params = &call->audio_params;
	audio_params->rfc2833_local_pt = linphone_core_get_rfc2833_pt(ctx->offer);
	audio_params->rfc2833_remote_pt = linphone_core_get_rfc2833_pt(ctx->remote);
}
#endif // DYNAMIC_PAYLOAD
}
void sdp_context_free(sdp_context_t *ctx){
	osip_free(ctx->localip);
	osip_free(ctx->username);
	if (ctx->offer!=NULL) sdp_message_free(ctx->offer);
	if (ctx->remote!=NULL) sdp_message_free(ctx->remote);
	if (ctx->answer!=NULL) sdp_message_free(ctx->answer);
	if (ctx->offerstr!=NULL) osip_free(ctx->offerstr);
	if (ctx->answerstr!=NULL) osip_free(ctx->answerstr);
	if (ctx->rtp_localip != NULL) osip_free(ctx->rtp_localip);
	osip_free(ctx);
}


//[SD6, ericchung, rcm integration
static int _sdp_message_get_mline_dir(sdp_message_t *sdp, int mline){
	int i;
	sdp_attribute_t *attr;
	for (i=0;(attr=sdp_message_attribute_get(sdp,mline,i))!=NULL;i++){
		if (keywordcmp("sendrecv",attr->a_att_field)==0){
			g_message("\r\n sdp is sendrecv !! \n");
			return SalStreamSendRecv;
			//return 0;
		}else if (keywordcmp("sendonly",attr->a_att_field)==0){
			g_message("\r\n sdp is sendonly !! \n");
			return SalStreamSendOnly;
			//return 0;
		}else if (keywordcmp("recvonly",attr->a_att_field)==0){
			g_message("\r\n sdp is sendonly !! \n");
			return SalStreamSendOnly;
			//return 0;
		}else if (keywordcmp("inactive",attr->a_att_field)==0){
			g_message("\r\n sdp is inactive !! \n");
			return SalStreamInactive;
			//return 0;
		}
	}
	return SalStreamSendRecv;
	return 1;
}
//]


#ifdef CONFIG_RTK_VOIP_SRTP
int sdp_generate_sdes_key(unsigned char* key, int key_len)
{
	int i;
	unsigned int random;

	if( key_len > SDP_KEY_LEN )
		return -1;
	for( i = 0; i < key_len; i++){
		random = osip_build_random_number();
		key[i] = (random % 256);
		g_message("key:%d\n",key[i]);
	}
	return 1;
}
#endif
