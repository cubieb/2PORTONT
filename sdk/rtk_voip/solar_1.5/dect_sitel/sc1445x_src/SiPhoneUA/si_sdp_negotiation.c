#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <si_print_api.h>

#include "internal.h"
//#include <netinet.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "si_ua_callcontrol.h"
#include "si_ua_stun.h"
#include "sdp_message.h"
#include "si_sdp_negotiation.h"
#include "si_ua_fax.h"
#include "si_ua_srtp_layer.h"
//#include "../exosip2/src/eXosip2.h"

  struct __eXosip_sockaddr
  {
    u_char ss_len;
    u_char ss_family;
    u_char padding[128 - 2];
  };
   
extern ata_t38_sdp_params_type *fax_sdp_params; 
void t38_add_a_attribute_200_ok(int, osip_negotiation_t *sdpInfo, sdp_message_t * remote, sdp_message_t *dest);
int t38_check_a_attribute_200_ok(int media_line, sdp_message_t *sdp);

osip_negotiation_t sdpNegotiationData[MAX_SUPPORTED_ACCOUNTS];
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//NEGOTIATION PARSER
/////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////// 
 
FmtpInformation_t FmtpInformation[]={
 	{101,"101 0-16"}, 
 	{98,"98 mode=30"}, 
 	{99,"99 mode=20"}, 
 	{-1,NULL},
	};
 
CodecInformation_t CodecInformation[]={
	{9,"9","G722", "8000"},
	{0,"0","PCMU", "8000"},
	{8,"8","PCMA", "8000"},
	{18,"18","G729", "8000"},
	{2,"2","G726-32", "8000"},
	{98,"98","iLBC", "8000"},
	{99,"99","iLBC", "8000"},
	{101,"101","telephone-event", "8000"},
	{-1,NULL, NULL},
	};
audio_profile profileTelephoneEvent;
int crypto_init(int accountId, SICORE* pCore);

int crypto_init(int accountId, SICORE* pCore)
{
	int cryptoId = 0;
	memset(&pCore->crypto_suites[accountId][0] ,0, sizeof(crypto_profile)*MAX_ENCRYPTION_SUITES);
 	// set crypto level
//	pCore->crypto_level[accountId] = ID_CRYPTO_MANDATORY;
		//ID_CRYPTO_MANDATORY;
	//ID_CRYPTO_DISABLE
	//ID_CRYPTO_OPTIONAL

 	// set crypto suite

	pCore->crypto_suites[accountId][cryptoId].enabled = 1;
	pCore->crypto_suites[accountId][cryptoId].flag = 1;

 	strcpy(pCore->crypto_suites[accountId][cryptoId ].suite,"AES_CM_128_HMAC_SHA1_32");
	pCore->crypto_suites[accountId][cryptoId ].suite_len = strlen("AES_CM_128_HMAC_SHA1_32");
 	// set crypto key
	pCore->crypto_suites[accountId][cryptoId].key_len=30;
	memset(pCore->crypto_suites[accountId][cryptoId ].key, 0xa2, pCore->crypto_suites[accountId][cryptoId ].key_len );
 
	cryptoId =1;
  	// set crypto suite
	pCore->crypto_suites[accountId][cryptoId ].enabled = 1;
	pCore->crypto_suites[accountId][cryptoId].flag = 1;
	strcpy(pCore->crypto_suites[accountId][cryptoId ].suite,"AES_CM_128_HMAC_SHA1_80");
	pCore->crypto_suites[accountId][cryptoId].suite_len = strlen("AES_CM_128_HMAC_SHA1_80");
 	// set crypto key
	pCore->crypto_suites[accountId][cryptoId ].key_len=30;
	memset(pCore->crypto_suites[accountId][cryptoId ].key, 0xa2, pCore->crypto_suites[accountId][cryptoId ].key_len );

	cryptoId =2;
  	// set crypto suite
	pCore->crypto_suites[accountId][cryptoId ].enabled = 0;
	pCore->crypto_suites[accountId][cryptoId].flag = 1;
 	strcpy(pCore->crypto_suites[accountId][cryptoId ].suite,"AES_CM_128_HMAC_SHA1_80");
	pCore->crypto_suites[accountId][cryptoId ].suite_len = strlen("AES_CM_128_HMAC_SHA1_80");
 	// set crypto key
	pCore->crypto_suites[accountId][cryptoId ].key_len=30;
	memset(pCore->crypto_suites[accountId][cryptoId ].key, 0xa2, pCore->crypto_suites[accountId][cryptoId ].key_len );

	cryptoId =3;
  	// set crypto suite
	pCore->crypto_suites[accountId][cryptoId ].enabled = 0;
	pCore->crypto_suites[accountId][cryptoId].flag = 1;
 	strcpy(pCore->crypto_suites[accountId][cryptoId ].suite,"AES_CM_128_HMAC_SHA1_32");
	pCore->crypto_suites[accountId][cryptoId ].suite_len = strlen("AES_CM_128_HMAC_SHA1_32");
 	// set crypto key
	pCore->crypto_suites[accountId][cryptoId ].key_len =30;
//	memset(pCore->crypto_suites[accountId][cryptoId ].key, 0xa2, pCore->crypto_suites[accountId][cryptoId ].key_len );
	if (pCore->crypto_level[accountId]==ID_CRYPTO_DISABLE)
	{
		pCore->crypto_suites[accountId][0].enabled = 0;
		pCore->crypto_suites[accountId][1].enabled = 0;
		pCore->crypto_suites[accountId][2].enabled = 0;
		pCore->crypto_suites[accountId][3].enabled = 0;
 	}
	return 0;
} 
//FIX OCTOBER
int eXosip_sdp_negotiation_init(int accountId, osip_negotiation_t *sdpInfo, SICORE* pCore)
{
    int i;
	crypto_init(accountId, pCore);
	memcpy(&sdpInfo->tx_encryption_suites, &pCore->crypto_suites[accountId][0], sizeof(crypto_profile)*MAX_ENCRYPTION_SUITES);
	sdpInfo->crypto_level = pCore->crypto_level[accountId];
 
	strcpy(sdpInfo->o_username, "-");               /**< username */
	strcpy(sdpInfo->o_session_name,"SIPCall");        /**< session name */
	strcpy(sdpInfo->o_nettype,"IN");                /**< Network Type */
  if (pCore->IPv6)
		strcpy(sdpInfo->o_addrtype,"IP6");               /**< Address type */
	else
		strcpy(sdpInfo->o_addrtype,"IP4"); 
	strcpy((char*)sdpInfo->o_addr, (char*)pCore->IPAddress);                   /**< Address */
	strcpy(sdpInfo->c_nettype,"IN");                /**< Network Type */
	strcpy(sdpInfo->c_nettype,"IN");                /**< Network Type */
	if (pCore->IPv6)
		strcpy(sdpInfo->c_addrtype,"IP6");               /**< Address type */
	else 
		strcpy(sdpInfo->c_addrtype,"IP4");               /**< Address type */
	strcpy((char*)sdpInfo->c_addr, (char*)pCore->IPAddress);                   /**< Address */
 
	sdpInfo->c_addr_multicast_ttl=NULL;     /**< TTL value for multicast address  */
	sdpInfo->c_addr_multicast_int=NULL;     /**< Nb of address for multicast */

	//YPAPA
	sdpInfo-> fcn_set_info=NULL;
    sdpInfo-> fcn_set_info=NULL;
    sdpInfo->fcn_set_uri=NULL;

    sdpInfo->fcn_set_emails =NULL;
    sdpInfo->fcn_set_phones =NULL;
    sdpInfo->fcn_set_attributes =NULL;
   
	// set ptime 
	for (i=0;i<MAX_SUPPORTED_CODECS;i++)
    {
 	   if (pCore->Codecs[accountId][i]>0) 
	   {
		   if (pCore->Codecs[accountId][i]==98){
			sprintf(sdpInfo->ptime,"ptime:%d",30);
			break;
		   }else if (pCore->Codecs[accountId][i]==99){
			   sprintf(sdpInfo->ptime,"ptime:%d",20);
				break;
		   }else
			   sprintf(sdpInfo->ptime,"ptime:%d",pCore->ptime[accountId]);
	   			break;
 	   }
  	}
	
	strcpy(profileTelephoneEvent.a_fmtp, GetFmtpAttr(101));
	sprintf(profileTelephoneEvent.a_rtpmap, "%d %s/%s",101, GetCodecName(101),GetCodecRate(101));
 	profileTelephoneEvent.isfmtp =1;
	strcpy(profileTelephoneEvent.payload ,"101");
	profileTelephoneEvent.valid =1; 

     for (i=0;i<MAX_SUPPORTED_CODECS;i++)
    {
	   char  tmp[32];
	   char  num[8];
	   char  *pfmtp;
	   if (pCore->Codecs[accountId][i]<0) {
	     sdpInfo->audio_codec[i].valid=0;
		}else{
		  sprintf(num, "%d", pCore->Codecs[accountId][i]);
		  sprintf(tmp, "%d %s/%s", pCore->Codecs[accountId][i], GetCodecName(pCore->Codecs[accountId][i]),GetCodecRate(pCore->Codecs[accountId][i]));
 		  strcpy(sdpInfo->audio_codec[i].payload, num);/**< supported audio codec */
 		  if (sdpInfo->crypto_level==ID_CRYPTO_DISABLE)
			  strcpy(sdpInfo->audio_codec[i].proto, "RTP/AVP");					 
		  else
			strcpy(sdpInfo->audio_codec[i].proto, "RTP/SAVP");					 

		  strcpy(sdpInfo->audio_codec[i].a_rtpmap, tmp);
		  sdpInfo->audio_codec[i].valid=1;
		  pfmtp = GetFmtpAttr(pCore->Codecs[accountId][i]);
		  if (pfmtp){
			strcpy(sdpInfo->audio_codec[i].a_fmtp,pfmtp);
			sdpInfo->audio_codec[i].isfmtp=1;
 		 }else sdpInfo->audio_codec[i].isfmtp=0;
 	   }
   }
 	return 0;
}
char* create_sdp_answer_no_remote(sicall_t* pCall, osip_negotiation_t *sdpInfo, SICORE* pCore, char *LocalAudioBuffer, char *PublicAudioBuffer)
{
//YPAPA SOS
	 
   	return eXosip_create_sdpbody_offer(pCall, NULL, sdpInfo, LocalAudioBuffer,PublicAudioBuffer, 0);
}

int create_sdp_hold_on(sdp_message_t * sdp, int type)
{
   int pos;
  int pos_media = -1;
 
  char *rcvsnd;
  int recv_send = -1;
  int found =0;	
  pos = 0;
 
  pos_media = 0;
  while (!sdp_message_endof_media (sdp, pos_media))
   {
		pos = 0;
 		rcvsnd = sdp_message_a_att_field_get (sdp, pos_media, pos);
 		 
		while (rcvsnd != NULL)
		{
   			if (rcvsnd != NULL && 0 == strcmp (rcvsnd, "sendonly"))
			{
   				sdp_message_a_attribute_del (sdp, pos_media,"sendonly");
 				sdp_message_a_attribute_add (sdp, pos_media, osip_strdup ("inactive"), NULL);

 				recv_send = 0;
				found =1;
				break;
			}
			else if (rcvsnd != NULL && (0 == strcmp (rcvsnd, "recvonly")|| 0 == strcmp (rcvsnd, "sendrecv")))
			{
  				recv_send = 0;
  				sdp_message_a_attribute_del (sdp, pos_media,rcvsnd);
 				if (!type) sdp_message_a_attribute_add (sdp, pos_media, osip_strdup ("sendonly"), NULL);
				else sdp_message_a_attribute_add (sdp, pos_media, osip_strdup ("inactive"), NULL);
				found =1;

				break;
 			}
			pos++;
 			rcvsnd = sdp_message_a_att_field_get (sdp, pos_media, pos);
		}

		if (!found )
		{
 			if (type){
  				sdp_message_a_attribute_add (sdp, pos_media, osip_strdup ("inactive"), NULL);
			}
			else{
 				sdp_message_a_attribute_add (sdp, pos_media, osip_strdup ("sendonly"), NULL);
			}
		}

		pos_media++;
	}


//	if (recv_send == -1)
//    {
      // we need to add a global attribute with a field set to "sendonly"  
//		sdp_message_a_attribute_del (sdp, pos, char *att_field)

  //    sdp_message_a_attribute_add (sdp, -1, osip_strdup ("sendonly"), NULL);
  //  }
   return 0;
}

int create_sdp_hold_off(sdp_message_t * sdp, int type)
{
  int pos;
  int pos_media = -1;
  char *rcvsnd;
 
  pos = 0;
/*  rcvsnd = sdp_message_a_att_field_get (sdp, pos_media, pos);
  while (rcvsnd != NULL)
    {
		if (rcvsnd != NULL && 0 == strcmp (rcvsnd, "sendonly"))
		{
 			recv_send = 0;
		}
		else if (rcvsnd != NULL && (0 == strcmp (rcvsnd, "recvonly")|| 0 == strcmp (rcvsnd, "sendrecv")))
		{
			recv_send = 0;
			sprintf (rcvsnd, "sendonly");
		}

		pos++;
		rcvsnd = sdp_message_a_att_field_get (sdp, pos_media, pos);

  }
 */
 
  pos_media = 0;
  while (!sdp_message_endof_media (sdp, pos_media))
   {
		pos = 0;
 		rcvsnd = sdp_message_a_att_field_get (sdp, pos_media, pos);
 		
		while (rcvsnd != NULL)
		{
  			if (rcvsnd != NULL && 0 == strcmp (rcvsnd, "inactive"))
			{
  				sdp_message_a_attribute_del (sdp, pos_media,"inactive");
 				sdp_message_a_attribute_add (sdp, pos_media, osip_strdup ("recvonly"), NULL);

			}else if (rcvsnd != NULL && 0 == strcmp (rcvsnd, "sendonly"))
			{
  				sdp_message_a_attribute_del (sdp, pos_media,"sendonly");
 				sdp_message_a_attribute_add (sdp, pos_media, osip_strdup ("sendrecv"), NULL);
				//sdp_message_a_attribute_add (sdp, pos_media, osip_strdup ("setup:active"), NULL);

 				 
			}
			else if (rcvsnd != NULL && (0 == strcmp (rcvsnd, "recvonly")|| 0 == strcmp (rcvsnd, "sendrecv")))
			{
   				sdp_message_a_attribute_del (sdp, pos_media,rcvsnd);
 				sdp_message_a_attribute_add (sdp, pos_media, osip_strdup ("sendrecv"), NULL);
 			}
			pos++;
 			rcvsnd = sdp_message_a_att_field_get (sdp, pos_media, pos);
		}
		pos_media++;
	}

    return 0;
}

int validatePtimeValue(int ptimevalue)
{
	switch (ptimevalue)
	{
		case 10:
		case 20:
		case 30:
		case 40:
		case 50:
		case 60:
			return 1;
		default:
			return 0;
	}
}

char* create_sdp_answer_from_remote(sicall_t* pCall, osip_negotiation_t *sdpInfo, SICORE* pCore, sdp_message_t * remote)
{
	sdp_message_t * local;
	char *local_body;
  	int i, ret;
	int pos=0;
	int m_lines_that_match = 0;
 	
	if (0!=strncmp(remote->v_version,"0",1))
	{
 	  return NULL;//406;//Not Acceptable
 	} 
 
	ret = sdp_message_init(&local);
	if (ret != 0) return NULL;//YPAPA -1;
 

	sdp_partial_clone(pCall, sdpInfo, remote, &local);
	ret = sdp_confirm_media (pCall, sdpInfo, remote, &local);
	if (ret != 0)
    {
      sdp_message_free (local);
      return NULL; 
    }
	i =0;
  

    while (!sdp_message_endof_media (local, i))
    {
      // this is to refuse each line with no codec that matches! 
      if (NULL == sdp_message_m_payload_get (local, i, 0))
	  {
		  /*si_print(PRINT_LEVEL_DEBUG, "sdp_confirm_media : Image support 4: \n");
 		  sdp_media_t *med = osip_list_get (&local->m_medias, i);
		  char *str = sdp_message_m_payload_get (remote, i, 0);

		  sdp_message_m_payload_add (local, i, osip_strdup (str));
		  osip_free (med->m_port);
		  med->m_port = osip_strdup ("0");	// refuse this line 
		  */
		  ;

	  }
      else
	  {			//number of "m" lines that match  
		sdp_media_t *med = osip_list_get (&local->m_medias, i);
		m_lines_that_match++;
		osip_free (med->m_port);
  		if (0 == strcmp (med->m_media, "audio"))
	    {
 			med->m_port = eXosip_sdp_get_audio_port (pCall);

  	    }else if (0 == strcmp (med->m_media, "image"))
		{
 			med->m_port = eXosip_sdp_get_image_port (pCall);
    		}
  		#ifdef VIDEO_SUPPORT
			  else if (0 == strcmp (med->m_media, "video"))
				{
				  if (config->fcn_get_video_port != NULL)
				med->m_port = config->fcn_get_video_port (context, i);
				  else
				med->m_port = osip_strdup ("0");	// should never happen  
			  }
		#endif	    
 	   else
	    {
	 		med->m_port = eXosip_sdp_get_other_port(i);
 	    }
	}
      i++;
    }
  
	
  {
    char *rcvsnd;
    rcvsnd = sdp_message_a_att_field_get (remote, 0, pos);
    while (rcvsnd != NULL)
    {
 	
	if (rcvsnd != NULL && 0 == strcmp (rcvsnd, "sendonly"))
	    sdp_message_a_attribute_add (local, 0, osip_strdup ("inactive"), NULL);
 	else if (rcvsnd != NULL && (0 == strcmp (rcvsnd, "recvonly")))
	    sdp_message_a_attribute_add (local, 0, osip_strdup ("sendonly"), NULL);
 	else if (rcvsnd != NULL &&  (0 == strcmp (rcvsnd, "sendrecv")))
	    sdp_message_a_attribute_add (local, 0, osip_strdup ("sendrecv"), NULL);

 	pos++;
	rcvsnd = sdp_message_a_att_field_get (remote, 0, pos);
    }
  }
   {
    char *ptimeinfo;
	char *ptimevalue;
	char ptimevalue_ret[16];
	pos=0;
    ptimeinfo = sdp_message_a_att_field_get (remote, 0, pos);
    while (ptimeinfo != NULL)
    {
  		if (ptimeinfo != NULL && 0 == strcmp (ptimeinfo, "ptime"))
		{	
			ptimevalue = sdp_message_a_att_value_get(remote, 0, pos);
 			
			if (validatePtimeValue(atoi(ptimevalue)))
			{
 
				sprintf(ptimevalue_ret,"ptime:%d",atoi(ptimevalue));
				sdp_message_a_attribute_add (local, 0, osip_strdup (ptimevalue_ret), NULL);
			}
			else {
 
				sdp_message_a_attribute_add (local, 0, osip_strdup (sdpInfo->ptime), NULL);
			}
 		}
 		pos++;
		ptimeinfo = sdp_message_a_att_field_get (remote, 0, pos);
	}
   }


   if (1)//m_lines_that_match > 0)
   {
 
  	  if (pCore->firewallIP[0]!='\0')
	  {
		  char *c_address = NULL;
		  int pos=0;
		  /* If remote message contains a Public IP, we have to replace the SDP
			connection address */
		  c_address = sdp_message_c_addr_get(remote, -1, 0);
		  while (c_address==NULL)
		  {
			  c_address = sdp_message_c_addr_get(remote, pos, 0);
			  pos++;
			  if (pos>10)
				  break;
		  }
		  if (c_address!=NULL) /* found a connection address: check if it is public */
		  {
			  struct addrinfo *addrinfo;
			  struct __eXosip_sockaddr addr;
			   
			  i = eXosip_get_addrinfo(&addrinfo, c_address, pCore->SipServerPort);
			  if (i==0)
				{
				  memcpy ((char*)&addr,  (char*)(addrinfo->ai_addr),  addrinfo->ai_addrlen);
				  freeaddrinfo (addrinfo);
				  c_address = (char*)inet_ntoa (((struct sockaddr_in *) &addr)->sin_addr);
 				}

			  if (eXosip_is_public_address(c_address))
			    {
				  /* replace the IP with our firewall ip */
				  sdp_connection_t *conn;
				  pos=-1;
				  conn = sdp_message_connection_get(local, pos, 0);
				  while (conn!=NULL)
				  {
					  if (conn->c_addr!=NULL )
					  {
						  osip_free(conn->c_addr);
						  conn->c_addr = osip_strdup(pCore->firewallIP);
						  }
						  pos++;
					  conn = sdp_message_connection_get(local, pos, 0);
				  }
			    }
		  }
	  }
 		ret = sdp_message_to_str(local, &local_body);
		// fix: Memory Leak (06052008)      
		if (remote) sdp_message_free (remote);
		remote = NULL;
		if (local)	sdp_message_free (local);
		local=NULL;
 		return local_body;
	}
	else
    {
 
	// fix: Memory Leak (06052008)      
		if (remote)		sdp_message_free (remote);
		remote = NULL;
        if (local) sdp_message_free (local);
 		local=NULL;
	  return NULL;//YPAPA415;
    }
}
sdp_message_t * create_sdp_answer_4invite_on_hold(sicall_t *pCall, osip_negotiation_t *sdpInfo, SICORE* pCore, sdp_message_t * remote)
{
	sdp_message_t * local;
   	int i, ret;
	int m_lines_that_match = 0;
 
	if (0!=strncmp(remote->v_version,"0",1))
	{
 	  //YPAPA
	  return NULL;//406;//Not Acceptable
 	} 

	ret = sdp_message_init(&local);
	if (ret != 0) return NULL;//YPAPA -1;
	sdp_partial_clone(pCall, sdpInfo, remote, &local);
	ret = sdp_confirm_media (pCall, sdpInfo, remote, &local);
	if (ret != 0)
    {
      sdp_message_free (local);
      return NULL; 
    }
	i =0;
 
    while (!sdp_message_endof_media (local, i))
    {
           /* this is to refuse each line with no codec that matches! */
      /*if (NULL == sdp_message_m_payload_get (local, i, 0))
	  {
		  sdp_media_t *med = osip_list_get (&local->m_medias, i);
		  char *str = sdp_message_m_payload_get (remote, i, 0);

		  sdp_message_m_payload_add (local, i, osip_strdup (str));
		  osip_free (med->m_port);
		  si_print(PRINT_LEVEL_DEBUG, "===============   REMOVE PORT \n");
		  med->m_port = osip_strdup ("0");	 // refuse this line  
	  }
      else
 */
	  {			/* number of "m" lines that match */
		sdp_media_t *med = osip_list_get (&local->m_medias, i);
		m_lines_that_match++;
 		osip_free (med->m_port);
		// AMD: use the correct fcn_get_xxx_port method: 
		if (0 == strcmp (med->m_media, "audio"))
    	{
 			med->m_port = eXosip_sdp_get_audio_port ( pCall);
    	}else if (0 == strcmp (med->m_media, "image"))
    	{
	  		med->m_port = eXosip_sdp_get_image_port ( pCall);
			 
     	}
 
		#ifdef VIDEO_SUPPORT
		else if (0 == strcmp (med->m_media, "video"))
			{
			  if (config->fcn_get_video_port != NULL)
			med->m_port = config->fcn_get_video_port (context, i);
			  else
			med->m_port = osip_strdup ("0");	/* should never happen */
		}
		#endif	    
	  	else
	    	{
	 		med->m_port = eXosip_sdp_get_other_port(i);
 	    	}
	 }
      i++;
    }

  {
    char *rcvsnd;
    int pos=0;	
    rcvsnd = sdp_message_a_att_field_get (remote, 0, pos);
    while (rcvsnd != NULL)
    {
 	
	if (rcvsnd != NULL && (0 == strcmp (rcvsnd, "inactive")))
	    sdp_message_a_attribute_add (local, 0, osip_strdup ("inactive"), NULL);
	else if (rcvsnd != NULL && (0 == strcmp (rcvsnd, "sendonly"))){
 	    sdp_message_a_attribute_add (local, 0, osip_strdup ("recvonly"), NULL);
	}
 	else if (rcvsnd != NULL && (0 == strcmp (rcvsnd, "recvonly")))
	    sdp_message_a_attribute_add (local, 0, osip_strdup ("sendonly"), NULL);
 	else if (rcvsnd != NULL &&  (0 == strcmp (rcvsnd, "sendrecv")))
	    sdp_message_a_attribute_add (local, 0, osip_strdup ("sendrecv"), NULL);

 	pos++;
	rcvsnd = sdp_message_a_att_field_get (remote, 0, pos);
    }
  }
     if (1)//m_lines_that_match > 0) 
    { 	
  	  if (pCore->firewallIP[0]!='\0')
	  {
		  char *c_address = NULL;
		  int pos=0;
		  /* If remote message contains a Public IP, we have to replace the SDP
			connection address */
		  c_address = sdp_message_c_addr_get(remote, -1, 0);
		  while (c_address==NULL)
		  {
			  c_address = sdp_message_c_addr_get(remote, pos, 0);
			  pos++;
			  if (pos>10)
				  break;
		  }
		  if (c_address!=NULL) /* found a connection address: check if it is public */
		  {
 
			  struct addrinfo *addrinfo;
			  struct __eXosip_sockaddr addr;
			  			   

			  i = eXosip_get_addrinfo(&addrinfo, c_address, pCore->SipServerPort );
			  if (i==0)
				{
				  memcpy ((char*)&addr,  (char*)(addrinfo->ai_addr),  addrinfo->ai_addrlen);
				  freeaddrinfo (addrinfo);
				  c_address = (char*)inet_ntoa (((struct sockaddr_in *) &addr)->sin_addr);
 				}

			  if (eXosip_is_public_address(c_address))
			    {
				  /* replace the IP with our firewall ip */
				  sdp_connection_t *conn;
				  pos=-1;
				  conn = sdp_message_connection_get(local, pos, 0);
				  while (conn!=NULL)
				  {
					  if (conn->c_addr!=NULL )
					  {
						  osip_free(conn->c_addr);
						  conn->c_addr = osip_strdup(pCore->firewallIP);
					  }
					  pos++;
					  conn = sdp_message_connection_get(local, pos, 0);
				  }
			    }
		  }
	  }
	   sdp_message_free (remote);
  	   return local;
	}
	else
    	{
      	  sdp_message_free (local);
		  return NULL;//YPAPA415;
        }
}

char* eXosip_create_sdpbody_offer(sicall_t* pCall, osip_message_t *invite, osip_negotiation_t * config, char *local_audio_sdp_port, char *public_sdp_port, int inviteflag)
{
	sdp_message_t *sdp;
	char *body;
	int ret;
	 

	sdp= eXosip_create_sdp_offer( pCall, invite,  config, local_audio_sdp_port, public_sdp_port,  inviteflag);
	if (sdp==NULL) return NULL;
	ret = sdp_message_to_str(sdp, &body);
	// fix: Memory Leak (06052008)
	sdp_message_free(sdp);
 	if (ret!=0) return NULL;
 	return body;
}

sdp_message_t* eXosip_create_sdp_offer(sicall_t* pCall, osip_message_t *invite, osip_negotiation_t * config, char *local_audio_sdp_port, char *public_sdp_port, int inviteflag)
{
	SICORE* pCore = &siphoneCore;
	int i;
	int ret;
	sdp_message_t *sdp;

  if (local_audio_sdp_port!=NULL)
  {
    if (emptystr(public_sdp_port) || invalidport((int)atoi(public_sdp_port)) )//SIPIT 25
		public_sdp_port = local_audio_sdp_port;
    
		ret = osip_negotiation_sdp_build_offer(pCall, config, &sdp, public_sdp_port);
		if (ret!=0) return NULL;

		if (sdp!=NULL)
		{
			int pos=0;
			while (!sdp_message_endof_media (sdp, pos))
			{
				int k = 0;
				char *tmp = sdp_message_m_media_get (sdp, pos);
				if (0 == osip_strncasecmp (tmp, "audio", 5))
				{
					char *payload = NULL;
					do {
						payload = sdp_message_m_payload_get (sdp, pos, k);
						/*if (payload == NULL)
							{
							} //YPAPA to add fmtp information, ptime and sendrecv
						else if (0==strcmp("101",payload))
							{
							sdp_message_a_attribute_add (sdp, pos,osip_strdup("fmtp"),osip_strdup("101 0-16")); 
									// YPAPA osip_strdup("fmtp"),osip_strdup(my_payload->a_fmtp));
								}*/
						k++;
						} while (payload != NULL);
				}
				pos++;
				}
			}
 
		// YPAPA3
		if (inviteflag)
		//  if (eXosip.j_firewall_ip[0]!='\0')
		if (pCore->firewallIP[0]!='\0')
		{
			char *c_address = invite->req_uri->host;
			int pos=0;
			struct addrinfo *addrinfo;
			struct __eXosip_sockaddr addr;
		  			   

			i = eXosip_get_addrinfo(&addrinfo, invite->req_uri->host, pCore->SipServerPort);
 			if (i==0)
			{
				memcpy (&addr, addrinfo->ai_addr, addrinfo->ai_addrlen);
				freeaddrinfo (addrinfo);
				c_address = inet_ntoa (((struct sockaddr_in *) &addr)->sin_addr);
			}else {
			}
 			//If remote message contains a Public IP, we have to replace the SDP    connection address 
			if (eXosip_is_public_address(c_address))
			{
				// replace the IP with our firewall ip 
				sdp_connection_t *conn = sdp_message_connection_get(sdp, -1, 0);
				if (conn!=NULL && conn->c_addr!=NULL )
				{
	  				//FIX - BROADWORKS -- TOCKECK
						 osip_free(conn->c_addr);
						 conn->c_addr = osip_strdup(pCore->firewallIP);
				}
				pos=0;
				conn = sdp_message_connection_get(sdp, pos, 0);
				while (conn!=NULL)
				{

					if (conn!=NULL && conn->c_addr!=NULL )
					{
						 osip_free(conn->c_addr);
						// edo sos
						//FIX - BROADWORKS TOCHECK
					 conn->c_addr = osip_strdup(pCore->firewallIP);
					}
					pos++;
					conn = sdp_message_connection_get(sdp, pos, 0);
				}
			}
		}
 		return sdp;
	}
 	return 0; 
}

int osip_negotiation_sdp_build_offer (sicall_t* pCall,osip_negotiation_t * config, sdp_message_t ** sdp, char *service_port)
{
	int i;
	int media_line = 0;
 	int ret, first=1;
 	i = sdp_message_init (sdp);
 	if (i != 0) return -1;
 	sdp_message_v_version_set (*sdp, osip_strdup ("0"));
 	/* those fields MUST be set */
	sprintf(config->o_session_id, "%d", pCall->session_id);
	sprintf(config->o_session_version, "%d", pCall->session_version);
	pCall->session_version++;

     sdp_message_o_origin_set (*sdp,
			    osip_strdup (config->o_username),
			    osip_strdup (config->o_session_id),
			    osip_strdup (config->o_session_version),
			    osip_strdup (config->o_nettype),
			    osip_strdup (config->o_addrtype),
			    osip_strdup (config->o_addr));
 
   sdp_message_s_name_set (*sdp, osip_strdup ("SiCall"));

	if (config->fcn_set_info != NULL)
	config->fcn_set_info ( *sdp);
	if (config->fcn_set_uri != NULL)
	config->fcn_set_uri ( *sdp);
	if (config->fcn_set_emails != NULL)
	config->fcn_set_emails ( *sdp);
	if (config->fcn_set_phones != NULL)
	config->fcn_set_phones ( *sdp);
 
	if (config->c_nettype != NULL)
	{
		sdp_message_c_connection_add (*sdp, -1,
				  osip_strdup (config->c_nettype),
				  osip_strdup (config->c_addrtype),
				  osip_strdup (config->c_addr),
				  osip_strdup (config->c_addr_multicast_ttl),
				  osip_strdup (config->c_addr_multicast_int));
			 

	} 

  {				/* offer-answer draft says we must copy the "t=" line */
    int now = 0;// YPAPA time (NULL);
	// MEMORY LEAKS????
    char *tmp = osip_malloc (15);
    char *tmp2 = osip_malloc (15);
    sprintf (tmp, "%i", now);
    sprintf (tmp2, "%i", now);
 // sprintf (tmp2, "%i", now + 3600);


    i = sdp_message_t_time_descr_add (*sdp, tmp, tmp2);
	if (i != 0) {  
        //YPAPA
		sdp_message_free (*sdp);
  		return -1;
	}
  }

  if (config->fcn_set_attributes != NULL)
    config->fcn_set_attributes ( *sdp, -1);
  
   /* add all audio codec */
  if (!pCall->faxsupport)
  {
 	if (osip_negotiation_get_audiocodec(config, 0))
    {
      int pos = 0;
  
      audio_profile *my = osip_negotiation_get_audiocodec(config, pos);
       /* all media MUST have the same PROTO, PORT. */
      while (osip_negotiation_get_audiocodec(config, pos))
     {
		my = (audio_profile *) osip_negotiation_get_audiocodec(config, pos);
 	 	if (first) 
		 {
			 first =0;
 			 sdp_message_m_media_add (*sdp, osip_strdup ("audio"),
					   osip_strdup (service_port),
					  NULL,
  					   osip_strdup (my->proto));
		 } 
 		 ret = incallCodecChecking(atoi(my->payload));
		  
  		 if (ret)
 		 {
 			sdp_message_m_payload_add (*sdp, media_line,osip_strdup (my->payload));
			if (my->a_rtpmap != NULL){
				sdp_message_a_attribute_add (*sdp, media_line, osip_strdup ("rtpmap"),osip_strdup (my->a_rtpmap));
				if (GetCodecType(my->payload)>95)
				{
				   char *sfmtp = GetFmtpAttr(GetCodecType(my->payload));
 					if (sfmtp)
					{
						sdp_message_a_attribute_add (*sdp, media_line,osip_strdup("fmtp"),osip_strdup(sfmtp)); 
   					} 
 			   } 
			}
		}else{
				 
				sdp_message_free (*sdp); 
 				return -1;
 			}
		pos++;
      }  
      sdp_message_a_attribute_add (*sdp, media_line,osip_strdup ("sendrecv"), NULL);
  	  sdp_message_a_attribute_add (*sdp, media_line,osip_strdup (config->ptime), NULL);
 	  srtp_add_crypto_suite_into_sdp_offer(&pCall->crypto_parameters, config, *sdp, media_line);
      media_line++;
   }
  }else{
 
 	  sdp_message_m_media_add (*sdp, osip_strdup ("image"),osip_strdup (service_port),NULL, osip_strdup ("udptl t38"));
 
	  sdp_message_a_attribute_add (*sdp, media_line,osip_strdup ("T38FaxVersion"), osip_strdup(fax_sdp_params->T38FaxVersion));
	  sdp_message_a_attribute_add (*sdp, media_line,osip_strdup ("T38MaxBitRate"), osip_strdup(fax_sdp_params->T38MaxBitRate ));
      sdp_message_a_attribute_add (*sdp, media_line,osip_strdup ("T38FaxFillBitRemoval"), osip_strdup(fax_sdp_params->T38FaxFillBitRemoval ));
      sdp_message_a_attribute_add (*sdp, media_line,osip_strdup ("T38FaxTranscodingMMR"), osip_strdup(fax_sdp_params->T38FaxTranscodingMMR ));
      sdp_message_a_attribute_add (*sdp, media_line,osip_strdup ("T38FaxTranscodingJBIG"), osip_strdup(fax_sdp_params->T38FaxTranscodingJBIG ));
      sdp_message_a_attribute_add (*sdp, media_line,osip_strdup ("T38FaxRateManagement"), osip_strdup(fax_sdp_params->T38FaxRateManagement ));
      sdp_message_a_attribute_add (*sdp, media_line,osip_strdup ("T38FaxMaxBuffer"), osip_strdup(fax_sdp_params->T38FaxMaxBuffer ));
      sdp_message_a_attribute_add (*sdp, media_line,osip_strdup ("T38FaxMaxDatagram"), osip_strdup(fax_sdp_params->T38FaxMaxDatagram ));
	  sdp_message_a_attribute_add (*sdp, media_line,osip_strdup ("T38FaxUdpEC"), osip_strdup(fax_sdp_params->T38FaxUdpEC ));
   
  }
 
 


 #ifdef VIDEO_SUPPORT
   if (!video_port) return 0;
	
  /* add all video codec */
  if (!osip_list_eol (config->video_codec, 0))
    {
      int pos = 0;
      __payload_t *my =
	(__payload_t *) osip_list_get (config->video_codec, pos);

      /* all media MUST have the same PROTO, PORT. */
      sdp_message_m_media_add (*sdp, osip_strdup ("video"),
			       osip_strdup (video_port),
			       osip_strdup (my->number_of_port),
			       osip_strdup (my->proto));

      while (!osip_list_eol (config->video_codec, pos))
	{
	  my = (__payload_t *) osip_list_get (config->video_codec, pos);
	  sdp_message_m_payload_add (*sdp, media_line,
				     osip_strdup (my->payload));
	  if (my->a_rtpmap != NULL)
	    sdp_message_a_attribute_add (*sdp, media_line,
					 osip_strdup ("rtpmap"),
					 osip_strdup (my->a_rtpmap));
	  pos++;
	}
      media_line++;
    }
	#endif

 	return 0;
}

  
int sdp_partial_clone (sicall_t* pCall,  osip_negotiation_t * config,sdp_message_t * remote,sdp_message_t ** dest)
{
	int i;

	sprintf(config->o_session_id, "%d", pCall->session_id);
	sprintf(config->o_session_version, "%d", pCall->session_version);
	pCall->session_version++;

    sdp_message_v_version_set (*dest, osip_strdup ("0"));
   /* those fields MUST be set */
	sdp_message_o_origin_set (*dest,
	osip_strdup (config->o_username),
	osip_strdup (config->o_session_id),
	osip_strdup (config->o_session_version),
	osip_strdup (config->o_nettype),
	osip_strdup (config->o_addrtype),
	osip_strdup (config->o_addr));

  //sdp_message_s_name_set (*dest, osip_strdup (remote->s_name));
	sdp_message_s_name_set (*dest, osip_strdup ("SiCall"));

  if (config->fcn_set_info != NULL)
    config->fcn_set_info ( *dest);
  if (config->fcn_set_uri != NULL)
    config->fcn_set_uri ( *dest);
  
  if (config->fcn_set_emails != NULL)
    config->fcn_set_emails ( *dest);

  if (config->fcn_set_phones != NULL)
    config->fcn_set_phones ( *dest);

  if (config->c_nettype != NULL)
    sdp_message_c_connection_add (*dest, -1,
				  osip_strdup (config->c_nettype),
				  osip_strdup (config->c_addrtype),
				  osip_strdup (config->c_addr),
				  osip_strdup (config->c_addr_multicast_ttl),
				  osip_strdup (config->c_addr_multicast_int));

  {				/* offer-answer draft says we must copy the "t=" line */
    char *tmp = sdp_message_t_start_time_get (remote, 0);
    char *tmp2 = sdp_message_t_stop_time_get (remote, 0);

    if (tmp == NULL || tmp2 == NULL)
      return -1;		/* no t line?? */
    i =
      sdp_message_t_time_descr_add (*dest, osip_strdup (tmp),
				    osip_strdup (tmp2));
    if (i != 0)
      return -1;
  }
    if (config->fcn_set_attributes != NULL)
    config->fcn_set_attributes ( *dest, -1);
  return 0;
}

static const si_ua_str_t ID_RTP_AVP  = { "RTP/AVP", 7 };
static const si_ua_str_t ID_RTP_SAVP = { "RTP/SAVP", 8 };
static const si_ua_str_t ID_INACTIVE = { "inactive", 8 };

accept_options sdp_get_crypto_options(osip_negotiation_t *sdpInfo, sdp_message_t * remote, int media_pos,char *in_protocol,si_ua_str_t **out_protocol, crypto_parameters_t* pcrypto_params)
{
	int ret;
	int crypto_req_option =0;
	//int crypto_user_option = 0;
	//crypto_profile *rx_crypto_suite;
 	if (in_protocol==NULL) return ID_ACCEPT_NO;
  	 
 //	if (!strncmp(in_protocol,ID_RTP_SAVP.data ,ID_RTP_SAVP.slen) ) 
	{
   	ret = srtp_get_crypto_suites(sdpInfo,remote, media_pos, pcrypto_params);
		if (ret!=0)
			crypto_req_option=0;
		else crypto_req_option=1;
 	}
 	if (!crypto_req_option)
	{
		switch((int)sdpInfo->crypto_level)
		{
			case ID_CRYPTO_MANDATORY:
				*out_protocol=NULL;	 
				return ID_ACCEPT_NO;
 			case ID_CRYPTO_OPTIONAL:
				*out_protocol=&ID_RTP_AVP;	 
				return ID_ACCEPT_YES;
 			case ID_CRYPTO_DISABLE:
				*out_protocol=&ID_RTP_AVP;	 
				return ID_ACCEPT_YES;
		}
	}else{
		switch((int)sdpInfo->crypto_level)
		{
			case ID_CRYPTO_MANDATORY:
				*out_protocol=&ID_RTP_SAVP;	 
				return ID_ACCEPT_YES;
 			case ID_CRYPTO_OPTIONAL:
				*out_protocol=&ID_RTP_SAVP;	 
				return ID_ACCEPT_YES;
			case ID_CRYPTO_DISABLE:
				*out_protocol=&ID_RTP_AVP;	 
				return ID_ACCEPT_YES;
		}
	}
	*out_protocol=&ID_RTP_AVP;	 
 	return ID_ACCEPT_YES;
}

int sdp_confirm_media (sicall_t* pCall, osip_negotiation_t *sdpInfo, sdp_message_t * remote, sdp_message_t ** dest)
{
  char *payload;
  char *tmp, *tmp2, *tmp3;
  char *in_protocol;
  si_ua_str_t *out_protocol;
  accept_options acc_option=ID_ACCEPT_NO; 
 
#ifdef VIDEO_SUPPORT
  char* local_video_port;
  int video_qty = 0;
#endif
  int ret;
  int i;
  int k;
  int acceptedCodec=0;
  char mime[128] = {0};
  char fmtp[128] = {0};
  int audio_qty = 0;		/* accepted audio line: do not accept more than one */

  i = 0;
   while (!sdp_message_endof_media (remote, i))
    {
      tmp = sdp_message_m_media_get (remote, i);
	  if (tmp == NULL){
 
 		  return -1;
	  }
      tmp2 = sdp_message_m_port_get (remote, i);
      tmp3 = sdp_message_m_number_of_port_get (remote, i);
      in_protocol = sdp_message_m_proto_get (remote, i);
	  if (acc_option!=ID_ACCEPT_YES)
	  	  acc_option=sdp_get_crypto_options(sdpInfo,remote, i, in_protocol, &out_protocol, &pCall->crypto_parameters);

	  if (acc_option==ID_ACCEPT_NO)	{
   		  return -2;
	  }

      k = 0;
     if (0== strncmp (tmp, "audio", 5))
	 {
			sdp_message_m_media_add (*dest, osip_strdup (tmp), osip_strdup ("0"),
	 		       NULL, osip_strdup (out_protocol->data));
 		do
	    {
			payload = sdp_message_m_payload_get (remote, i, k);
			
	 		if (payload != NULL)
			{
				audio_profile m_AudioCodec;//my_payload ;//= NULL;
				int pnum = atoi(payload);
				int my_payload=0;
	 			memset(&m_AudioCodec,0,sizeof(audio_profile));
				memset(mime,0,128);
				memset(fmtp,0,128);
			 
				if (pnum >= 96)  /* for dynamic payload codes we prefer to match mime types */
				{
					if (sdp_message_m_payload_mime_get(remote, i, atoi(payload), mime, fmtp, sizeof(mime) - 1) == 0){
 					    my_payload = osip_negotiation_find_audio_payload_by_mime (&m_AudioCodec, sdpInfo, mime, fmtp);
						if (my_payload)
						{
							sprintf(m_AudioCodec.a_rtpmap,"%s %s",payload, mime);
							strcpy(m_AudioCodec.payload,payload);
						}
					}
  			    }
 
				if (my_payload == 0){
					my_payload = osip_negotiation_find_audio_payload (&m_AudioCodec, sdpInfo, payload);    
					if (pnum==101 && my_payload==0)
 						memcpy(&m_AudioCodec, &profileTelephoneEvent, sizeof(audio_profile*));
 				}
 				if ((my_payload != 0) && (incallCodecChecking(atoi(payload))))/* payload is supported */
				{
					ret = -1;	/* somtimes, codec can be refused even if supported */
					ret = eXosip_sdp_accept_audio_codec( tmp2,tmp3, audio_qty,payload);

		//			printf("  m_AudioCodec.a_rtpmap=%s payload=%s m_AudioCodec.a_fmtp=%s\n\n",m_AudioCodec.a_rtpmap,payload,m_AudioCodec.a_fmtp);

					if (0 == ret)
					{
					  if (atoi(payload)!=101) 
						  acceptedCodec=1;
					  sdp_message_m_payload_add (*dest, i,osip_strdup (payload));

					  if (m_AudioCodec.a_rtpmap != NULL)
					  {
						sdp_message_a_attribute_add (*dest, i,osip_strdup("rtpmap"),osip_strdup(m_AudioCodec.a_rtpmap));
					  }  
					  if (m_AudioCodec.isfmtp)
						sdp_message_a_attribute_add (*dest, i,
									 osip_strdup("fmtp"),osip_strdup(m_AudioCodec.a_fmtp));
/*
  					  sdp_message_m_payload_add (*dest, i,osip_strdup (my_payload->payload));
					  if (my_payload->a_rtpmap != NULL)
					  {
						sdp_message_a_attribute_add (*dest, i,osip_strdup("rtpmap"),osip_strdup(my_payload->a_rtpmap));
					  }
 					  if (my_payload->isfmtp)
						sdp_message_a_attribute_add (*dest, i,
									 osip_strdup("fmtp"),osip_strdup(my_payload->a_fmtp));
*/

					}
				}
			}		  
			k++;
		} while (payload != NULL);

		if (!acceptedCodec) 
		{
  			si_print(PRINT_LEVEL_DEBUG, "Payload not supported \n ");
			return -1;
					}
		if (NULL != sdp_message_m_payload_get (*dest, i, 0))
			audio_qty = 1;
 
		// find out encryption options
		if (!strncmp(out_protocol->data,ID_RTP_SAVP.data, ID_RTP_SAVP.slen))
		{
 		//	crypto_profile *rx_crypto_suite;
  		//	rx_crypto_suite = srtp_get_crypto_suites(sdpInfo,remote, i);
		//	if (rx_crypto_suite)
			{		 
 				srtp_add_crypto_suite_into_sdp_answer(&pCall->crypto_parameters, *dest, i);
 			}
 		} 
 	
	} else if ( 0 == strncmp (tmp, "image", 5))
	{
    		    sdp_message_m_media_add (*dest, osip_strdup ("image"),osip_strdup ("20000"),NULL, osip_strdup ("udptl t38"));
 		    t38_add_a_attribute_200_ok(i, sdpInfo, remote, *dest);
	} 
 
	 
#ifdef VIDEO_SUPPORT
      else if (0 == strncmp (tmp, "video", 5))
	{
        /* JWA: added 4 lines + if/else to reject video when it is not accepted locally */
        local_video_port = sdpInfo->fcn_get_video_port (  i);
        if (local_video_port[0]=='\0')
        { free(local_video_port);
        }else{
      sdp_message_m_media_add (*dest, osip_strdup (tmp), osip_strdup ("0"),
			       NULL, osip_strdup (in_protocol));

	  do
	    {
	      payload = sdp_message_m_payload_get (remote, i, k);
	      if (payload != NULL)
		{
		  __payload_t *my_payload = NULL;

		  int pnum = atoi(payload);

		  if (pnum >= 96)  /* for dynamic payload codes we prefer to match mime types */
 		    {
		      if (sdp_message_m_payload_mime_get(remote, i, atoi(payload), mime, fmtp, sizeof(mime) - 1) == 0)
				my_payload = osip_negotiation_find_video_payload_by_mime (sdpInfo, mime);
		    }


		  if (my_payload == NULL)
		      my_payload = osip_negotiation_find_video_payload (sdpInfo, payload);



		  if (my_payload != NULL)	/* payload is supported */
		    {
		      ret = -1;
		      if (sdpInfo->fcn_accept_video_codec != NULL)
			ret =
			  sdpInfo->fcn_accept_video_codec (  tmp2, tmp3,
							  video_qty, payload);
		      if (0 == ret)
			{
			  sdp_message_m_payload_add (*dest, i,
						     osip_strdup (my_payload->payload));
			  /* TODO  set the attribute list (rtpmap..) */
 			  if (my_payload->a_rtpmap != NULL)
			    sdp_message_a_attribute_add (*dest, i,
							 osip_strdup
							 ("rtpmap"),
							 osip_strdup
							 (my_payload->
							  a_rtpmap));


			  if (my_payload->c_nettype != NULL)
			    {
			      sdp_media_t *med =
				osip_list_get (&(*dest)->m_medias, i);

			      if (osip_list_eol (&med->c_connections, 0))
				sdp_message_c_connection_add (*dest, i,
							      osip_strdup
							      (my_payload->
							       c_nettype),
							      osip_strdup
							      (my_payload->
							       c_addrtype),
							      osip_strdup
							      (my_payload->
							       c_addr),
							      osip_strdup
							      (my_payload->
							       c_addr_multicast_ttl),
							      osip_strdup
							      (my_payload->
							       c_addr_multicast_int));
			    }
			}
		    }
		}
	      k++;
	    }
	  while (payload != NULL);
	  if (NULL != sdp_message_m_payload_get (*dest, i, 0))
	    video_qty = 1;
  }
	}
#endif
      else
	{
               sdp_message_m_media_add (*dest, osip_strdup (tmp), osip_strdup ("20000"),
			       NULL, osip_strdup (in_protocol));

	  do
	    {
	      payload = sdp_message_m_payload_get (remote, i, k);
	      if (payload != NULL)
		{
		  audio_profile  *my_payload =
		    osip_negotiation_find_other_payload (sdpInfo, payload);

		  if (my_payload != NULL)	/* payload is supported */
		    {
		      ret = -1;
 			ret =
			  eXosip_sdp_accept_other_codec (tmp, tmp2,
							  tmp3, payload);
		      if (0 == ret)
			{
			  sdp_message_m_payload_add (*dest, i,
						     osip_strdup (payload));
			  /* rtpmap has no meaning here! */
			  /*YPAPA TODO
			  if (my_payload->c_nettype != NULL)
			    {
			      sdp_media_t *med =
				osip_list_get (&(*dest)->m_medias, i);

			      if (osip_list_eol (&med->c_connections, 0))
				sdp_message_c_connection_add (*dest, i,osip_strdup(my_payload->c_nettype),
							      osip_strdup(my_payload->c_addrtype),
							      osip_strdup(my_payload->c_addr),
							      osip_strdup(my_payload->c_addr_multicast_ttl),
							      osip_strdup(my_payload->c_addr_multicast_int));
			    }*/
			}
		    }
		}
	      k++;
	    }
	  while (payload != NULL);
	}
      i++;
   }
 
  return 0;
}
char* build_sdp_answer(SICORE* pCore, sicall_t* pCall)
{
	char LocalAudioBuffer[16];
	char PublicAudioBuffer[16];
   	sdp_message_t *sdpRemote=NULL;
	int tid = pCall->tid ;
   
 	sdpRemote= eXosip_get_remote_sdp_from_tid(tid);
 	select_audio_port(pCore, pCall, LocalAudioBuffer, PublicAudioBuffer);
 

	if (sdpRemote!=NULL){
 
 // 		sdp_get_first_supported(&sdpNegotiationData[pCall->accountID],sdpRemote, &pCall->sStreamParameters);
  		return create_sdp_answer_from_remote(pCall, &sdpNegotiationData[pCall->accountID], &siphoneCore, sdpRemote );
	}
 	else// return NULL; 
	{
  		return create_sdp_answer_no_remote(  pCall, &sdpNegotiationData[pCall->accountID], &siphoneCore, LocalAudioBuffer, PublicAudioBuffer);
	}

 } 

// it returns the first payload type supported by the remote side (as it is in 200OK message)
int sdp_get_first_supported(osip_negotiation_t *sdpInfo, sdp_message_t * remote, audio_stream_parameters *pStreamParameters)
{
  char *payload;
  char *tmp, *tmp2, *tmp3, *tmp4;
  char *tmp5=NULL;
  int i;
  int k;
  int ret=-1;
  char mime[128] = {0};
  char fmtp[128] = {0};//FIX ILBC 

//  int audio_qty = 0;		/* accepted audio line: do not accept more than one */
  i = 0;
  tmp5 = sdp_message_c_addr_get (remote, -1, 0);
   if (tmp5==NULL) 
	  tmp5 = sdp_message_c_addr_get (remote, i, 0);
  // reInvite bug fix
 
 if (remote==NULL) 
	 si_print(PRINT_LEVEL_ERR, "INVALID SDP................\n");

 while (!sdp_message_endof_media (remote, i))
  {
      tmp = sdp_message_m_media_get (remote, i);
      tmp2 = sdp_message_m_port_get (remote, i);
      tmp3 = sdp_message_m_number_of_port_get (remote, i);
      tmp4 = sdp_message_m_proto_get (remote, i);

   	  if (tmp5==NULL) 
		     tmp5 = sdp_message_c_addr_get (remote, i, 0);
 
	   if (tmp == NULL) {
  		   return -1;
	   }
 
      k = 0;
      if (0 == strncmp (tmp, "audio", 5))
	  {
		  do{

			 payload = sdp_message_m_payload_get (remote, i, k);
			 if (payload == NULL)
			 {
				k++;
				payload = sdp_message_m_payload_get (remote, i, k);
			 }

			 if (payload != NULL)
			 {
				 audio_profile m_AudioCodec;//my_payload ;//= NULL;
				 int my_payload = 0;
				  int pnum = atoi(payload);
				  int pnum1;
				  if (pnum >= 96)
				  {  /* for dynamic payload codes we prefer to match mime types */
					  if (sdp_message_m_payload_mime_get( remote, i, atoi(payload), mime,fmtp, sizeof(mime) - 1) == 0){
 		 				my_payload = osip_negotiation_find_audio_payload_by_mime1(&m_AudioCodec,sdpInfo, mime, fmtp);
					  }
		 		  }
//ekei1


				  if (my_payload == 0)
						my_payload = osip_negotiation_find_audio_payload (&m_AudioCodec,sdpInfo, payload);

		  			  if ((my_payload != 0) && (incallCodecChecking(atoi(payload))))/* payload is supported */
					  {
 						 if (m_AudioCodec.a_rtpmap != NULL)
						 {
// 							 printf("[%s]=========    MIME GOT = %s payload=%d  return to appls = %d \n", __FUNCTION__, mime,pnum,atoi(my_payload->payload) );
 							 pStreamParameters->payload =  atoi(m_AudioCodec.payload);
							 pStreamParameters->d_payload =  pnum; 
						 
 							 pStreamParameters->remote_port = atoi(tmp2);
 							 if (tmp5)
								 strcpy(pStreamParameters->remote_address, tmp5);		 
							 else {
  								 return -1;
 							 }

 							 ret = 0;
     						 break;
						 }
 
 				  }
 			  }
				k++;
		   }while (payload!=NULL);

  	  }else if (0 == strncmp (tmp, "image", 5))
		{
 			pStreamParameters->remote_port = atoi(tmp2);
 			 if (tmp5)
				 strcpy(pStreamParameters->remote_address, tmp5);		 

  			return -2;
		}
 
	  i++;
  }
    
   return ret;
}
 
int sdp_get_first_crypto_option(osip_negotiation_t *sdpInfo, sdp_message_t * remSDP, crypto_parameters_t *pcrypto_params)
{
  char *media_type, *in_protocol;
  si_ua_str_t *out_protocol;
  int media_pos;
  accept_options acc_option; 
  int ret=-1;
   	 

  media_pos = 0;
  while (!sdp_message_endof_media (remSDP, media_pos))
  {
       media_type = sdp_message_m_media_get(remSDP, media_pos);
      in_protocol = sdp_message_m_proto_get(remSDP, media_pos);
 
	  if (!strncmp (media_type, "audio", 5)  || !strncmp (media_type, "image", 5))
	  {
 		  acc_option=sdp_get_crypto_options(sdpInfo,remSDP, media_pos, in_protocol, &out_protocol, pcrypto_params);
  		  if (acc_option==ID_ACCEPT_NO)	return -1;

	  }
	 media_pos++;
  }
  
  return ret;
}
 
int sdp_get_fax_params(ata_t38_sdp_params_type *fax_sdp_params, sdp_message_t * sdp, audio_stream_parameters *pStreamParameters)
{
	char *tmp, *tmp2, *tmp3, *tmp4;
	char *tmp5=NULL;
	int i;
	int k;
  
	i = 0;
	tmp5 = sdp_message_c_addr_get (sdp, -1, 0);
	if (tmp5==NULL) 
		tmp5 = sdp_message_c_addr_get (sdp, i, 0);
 
	while (!sdp_message_endof_media (sdp, i))
    {
      tmp = sdp_message_m_media_get (sdp, i);
      tmp2 = sdp_message_m_port_get (sdp, i);
      tmp3 = sdp_message_m_number_of_port_get (sdp, i);
      tmp4 = sdp_message_m_proto_get (sdp, i);
 
	   if (tmp5==NULL) 
		     tmp5 = sdp_message_c_addr_get (sdp, i, 0);
  
	   if (tmp == NULL) {
		   return -1;
	   }
       k = 0;
		if (0 == strncmp (tmp, "image", 5))
		{
 			pStreamParameters->remote_port = atoi(tmp2);
 			 if (tmp5)
 				 strcpy(pStreamParameters->remote_address, tmp5);		 
			 else return -2;

			if (!t38_check_a_attribute_200_ok(i, sdp))
				return 0;
			else return -2;
		}
  	  i++;
	}
	return -1;
}

// it returns the first payload type supported by the remote side (as it is in 200OK message)
int sdp_get_first_requested(osip_negotiation_t *sdpInfo, sdp_message_t * remote)
{
  char *payload;
  char *tmp, *tmp2, *tmp3, *tmp4;
  char *tmp5=NULL;
  int i;
  int k;
//int audio_qty = 0;		/* accepted audio line: do not accept more than one */

  i = 0;
  tmp5 = sdp_message_c_addr_get (remote, -1, 0);
  if (tmp5==NULL) 
	  tmp5 = sdp_message_c_addr_get (remote, i, 0);
  // reInvite bug fix
	
  while (!sdp_message_endof_media (remote, i))
    {
      tmp = sdp_message_m_media_get (remote, i);
      tmp2 = sdp_message_m_port_get (remote, i);
      tmp3 = sdp_message_m_number_of_port_get (remote, i);
      tmp4 = sdp_message_m_proto_get (remote, i);
 
	   if (tmp5==NULL) 
		     tmp5 = sdp_message_c_addr_get (remote, i, 0);
  
      if (tmp == NULL) return -1;
      k = 0;
      if (0 == strncmp (tmp, "audio", 5))
	  {
		  do{

			 payload = sdp_message_m_payload_get (remote, i, k);
			 if (payload != NULL){
   				return atoi(payload);
  			  }
				k++;
		   }while (payload!=NULL);
  	  }
	  i++;
  }
   return -1;
}

static int mimeparse(const char *mime, int *rate, int *chans)
{
  const char *rp;
  const char *cp;
  int len;

  rp = strchr(mime, '/');
  if (!rp)
    return strlen(mime);

  len = rp - mime;

  rp++;
  if (!*rp)
    return len;
 
  *rate = atol(rp);

  cp = strchr(rp, '/');
  if (!cp)
    return len;

  cp++;
  if (!*cp)
    return len;

  *chans = atol(cp);
  return len;
}

static int 
mimecmp(const char *m1, const char *m2, int rate, int chans)
{
  int r1,c1,l1;
  int r2,c2,l2;

  r2 = r1 = rate;
  c2 = c1 = chans;

  l1 = mimeparse(m1, &r1, &c1);
  l2 = mimeparse(m2, &r2, &c2);
 
  if ((l1 == l2) && (r1 == r2) ){
    return osip_strncasecmp(m1, m2, l1);
  }
  if ((l1 == l2) && (r1 == r2) && (c1 == c2))
    return osip_strncasecmp(m1, m2, l1);

  return 1;
}
 int osip_negotiation_find_audio_payload_by_mime1(audio_profile  *pAudioCodec, osip_negotiation_t * sdpInfo, char *mime, char *fmtp)
{
  int i;
  char remotemime[128];
  if (mime == NULL) return NULL;
     for (i=0;i<MAX_SUPPORTED_CODECS;i++)
    {
 		if (sdpInfo->audio_codec[i].valid) {
			if (sdpInfo->audio_codec[i].a_rtpmap)
		    {
				char *mymime;
				mymime = strchr(sdpInfo->audio_codec[i].a_rtpmap,' ');
				mymime+=strspn(mymime," ");
 				if (mymime)	//FIX ILBC
				{
					if (!mimecmp(mymime,mime,8000,1))
					{
 						if (!fmtpcompare(sdpInfo->audio_codec[i].payload, sdpInfo->audio_codec[i].a_fmtp, fmtp))  
						{
							 
							memcpy(pAudioCodec, &sdpInfo->audio_codec[i], sizeof(audio_profile));
  								return 1;// (audio_profile*)&sdpInfo->audio_codec[i];		
						}
  					}
				}
			}	

 		}
     }
 
  return 0;
}
int osip_negotiation_find_audio_payload_by_mime(audio_profile  *pAudioCodec, osip_negotiation_t * sdpInfo, char *mime, char *fmtp)
{
  int i;
  char remotemime[128];
  if (mime == NULL) return NULL;
     for (i=0;i<MAX_SUPPORTED_CODECS;i++)
    {
 		if (sdpInfo->audio_codec[i].valid) {
			if (sdpInfo->audio_codec[i].a_rtpmap)
		    {
				char *mymime;
				mymime = strchr(sdpInfo->audio_codec[i].a_rtpmap,' ');
				mymime+=strspn(mymime," ");
 				if (mymime)	//FIX ILBC
				{
					if (!mimecmp(mymime,mime,8000,1))
					{
 						if (!fmtpcompare(sdpInfo->audio_codec[i].payload, sdpInfo->audio_codec[i].a_fmtp, fmtp))  
						{
							 
							strcpy(pAudioCodec->a_fmtp, fmtp); 
 							if (strlen(fmtp)>1) pAudioCodec->isfmtp = 1;
							else  pAudioCodec->isfmtp = 0;
							 
							//memcpy(pAudioCodec, &sdpInfo->audio_codec[i], sizeof(audio_profile));
  								return 1;// (audio_profile*)&sdpInfo->audio_codec[i];		
						}
  					}
				}
			}	

 		}
     }
 
  return 0;
}

int fmtpcompare( char *payload,char *myfmtp, char* fmtp)
{
	char* myvar=NULL;
 	char* var=NULL;
	if (myfmtp && (strlen(myfmtp)>1))   myvar= strchr(myfmtp,' ');
	if (fmtp && (strlen(fmtp)>1))		var= strchr(fmtp,' ');
	if (atoi(payload) == 101) return 0;//Accept it

	if (atoi(payload) == 98) 
	{
		if (var==NULL) return 0;//Accept it
		if (!strcmp(myvar,var)) return 0;//Accept it
	}

	if (atoi(payload) == 99) 
	{
		if (var==NULL) return 1;//Reject it
		if (!strcmp(myvar,var)) return 0;//Accept it
 	}
	
	return 1;
}
audio_profile  * osip_negotiation_get_audiocodec(osip_negotiation_t * sdpInfo, int i)
{
 	if (i>=MAX_SUPPORTED_CODECS)return NULL;
	if (sdpInfo->audio_codec[i].valid) 
	{
 	   if (sdpInfo->audio_codec[i].a_rtpmap)
	   {
 			return (audio_profile*)&sdpInfo->audio_codec[i];		
	   }
	}
    return NULL;
}

int
osip_negotiation_find_audio_payload (audio_profile  *pAudioCodec, osip_negotiation_t * sdpInfo,
				     char *payload)
{
   int i;
  
  if (payload == NULL) return 0;

    for (i=0;i<MAX_SUPPORTED_CODECS;i++)
     {
	 	if (sdpInfo->audio_codec[i].valid) {
	 	 	 if (!strcmp (sdpInfo->audio_codec[i].payload, payload))
 			{
				memcpy(pAudioCodec, (audio_profile*)&sdpInfo->audio_codec[i], sizeof(audio_profile));
				return 1;
			}
		}
    }
   return 0;
}
audio_profile  * osip_negotiation_find_other_payload(osip_negotiation_t * config, char *mime)
{

return NULL;
}


  
/**
 * Get the mime payload string of a media in a SDP message
 * that corresponds to the specified payload number
 * 
 * @param sdp			The sdp message.
 * @param pos_media		The index of media in the SDP message
 * @param payloadnumber	The payload number
 * @param result(out)	The output buffer to contain result
 * @param result		The output buffer size 
 * @return				0 on success. -1 on failure
 */
int sdp_message_m_payload_mime_get(sdp_message_t * sdp, int pos_media, int payloadnumber, char *result,char *fmtp, const size_t result_size)
{
	sdp_media_t *med = osip_list_get (&sdp->m_medias, pos_media);
	sdp_attribute_t *attr,*attrfmtp;
	int i;
	
	if (med == NULL)	return -1;
	
	for( i = 0; ; i++)
    {
		attr = sdp_message_attribute_get (sdp, pos_media, i);
		
		if (!attr) return -1;
		
		if (!strcmp(attr->a_att_field, "rtpmap") && (atoi(attr->a_att_value) == payloadnumber))
		{
			char *p = attr->a_att_value;
 			attrfmtp = sdp_message_attribute_get (sdp, pos_media, i+1);
			if (attrfmtp && !strcmp(attrfmtp->a_att_field, "fmtp"))
			{
				if (strlen(attrfmtp->a_att_value)<result_size)
				{
					strcpy(fmtp, attrfmtp->a_att_value);
 				}
			} 
			p = strchr(p, ' ');
			if (!p) return -1;
				// YPAPA TODO ;
				//return NULL;
			
			p += strspn(p, " ");
			
			if (*p) {
				if (strlen(p) >= result_size) {
						return -1;
				}
				strcpy(result, p);
				return 0;
			}
												
			return -1;
		}
    }
	return -1;
}
//v1.0.1.1

int sdp_message_get_ptime(sdp_message_t * sdp)
{
	sdp_media_t *med = osip_list_get (&sdp->m_medias,0);
	sdp_attribute_t *attr;
	int pos_media = 0;
	int i;
 	if (med == NULL) return -1;
	for( i = 0; ; i++)
    {
		attr = sdp_message_attribute_get (sdp, pos_media, i);
 		if (!attr) 	return -1;
		
		if (!strcmp(attr->a_att_field, "ptime")) {
			
			return (atoi(attr->a_att_value)) ;
		}
     }
	return -1;
}

int eXosip_sdp_accept_audio_codec( char *port, char *number_of_port,
				  int audio_qty, char *payload)
{
	if (incallCodecChecking(atoi(payload)))
		return 0;
	else return -1;
}

int eXosip_sdp_accept_other_codec( char *type, char *port,
				  char *number_of_port, char *payload)
{
  /* ... */
  return -1;
}

char *eXosip_sdp_get_audio_port(sicall_t* pCall)
{
    char SdpPort[16];
	char *portstr=NULL;
	sprintf(SdpPort, "%d", pCall->public_sdp_audio_port);
 	portstr= osip_strdup(SdpPort); 
 	return portstr;
 }
int ImageUdpPortRange  = 31191;
char *eXosip_sdp_get_image_port(sicall_t* pCall)
{
   	char SdpPort[16];
	pCall->public_sdp_audio_port = ImageUdpPortRange;
	sprintf(SdpPort, "%d", pCall->public_sdp_audio_port);
 	return osip_strdup(SdpPort); 
} 

char *eXosip_sdp_get_other_port(int pos_media)
{
  	return 0;   
}
char *GetCodecName(int cnum)
{
	int i;
	for (i=0;CodecInformation[i].name;i++)
	{
		if (CodecInformation[i].num==cnum)
		{
			return CodecInformation[i].name;
		}
	}
	return NULL;
}
int GetCodecType(char *scodec)
{
	int i;
	for (i=0;CodecInformation[i].name;i++)
	{
 		if (!strcmp(CodecInformation[i].snum,scodec))
		{
			return CodecInformation[i].num;
		}
	}
	return -1;
}
char *GetCodecRate(int cnum)
{
	int i;
	for (i=0;CodecInformation[i].name;i++)
	{
		if (CodecInformation[i].num==cnum)
		{
			return CodecInformation[i].rate;
		}
	}
	return NULL;
}
char *GetFmtpAttr(int cnum)
{
	int i;
	for (i=0;FmtpInformation[i].num>0;i++)
	{
		if (FmtpInformation[i].num==cnum)
		{
			return FmtpInformation[i].attr;
		}
	}
	return NULL;
}
 
void t38_add_a_attribute_200_ok(int media_line, osip_negotiation_t *sdpInfo, sdp_message_t * remote, sdp_message_t *sdp)
{
 	sdp_attribute_t *pInAttribute;
	int pos =0;
  
	pInAttribute = sdp_message_attribute_get (remote,media_line ,pos);
	while (pInAttribute)
	{
 		// ret = t38Confirm(pInAttribute->a_att_field, pInAttribute->a_att_value)
		// if (!ret) reject();
		//   if( sdp_params.T38FaxUdpEnabled )
			// UDPTL support 
		pos++;
		pInAttribute = sdp_message_attribute_get (remote,media_line, pos);
	}
  
	// Read supported values from T38 algorithm 
   	sdp_message_a_attribute_add (sdp, media_line,osip_strdup ("T38FaxVersion"), osip_strdup(fax_sdp_params->T38FaxVersion));
	sdp_message_a_attribute_add (sdp, media_line,osip_strdup ("T38MaxBitRate"), osip_strdup(fax_sdp_params->T38MaxBitRate));
	sdp_message_a_attribute_add (sdp, media_line,osip_strdup ("T38FaxFillBitRemoval"), osip_strdup(fax_sdp_params->T38FaxFillBitRemoval));
	sdp_message_a_attribute_add (sdp, media_line,osip_strdup ("T38FaxTranscodingMMR"), osip_strdup(fax_sdp_params->T38FaxTranscodingMMR));
	sdp_message_a_attribute_add (sdp, media_line,osip_strdup ("T38FaxTranscodingJBIG"), osip_strdup(fax_sdp_params->T38FaxTranscodingJBIG));
	sdp_message_a_attribute_add (sdp, media_line,osip_strdup ("T38FaxRateManagement"), osip_strdup(fax_sdp_params->T38FaxRateManagement));
	sdp_message_a_attribute_add (sdp, media_line,osip_strdup ("T38FaxMaxBuffer"), osip_strdup(fax_sdp_params->T38FaxMaxBuffer));
	sdp_message_a_attribute_add (sdp, media_line,osip_strdup ("T38FaxMaxDatagram"), osip_strdup(fax_sdp_params->T38FaxMaxDatagram));
	sdp_message_a_attribute_add (sdp, media_line,osip_strdup ("T38FaxUdpEC"), osip_strdup(fax_sdp_params->T38FaxUdpEC));
}
 
int t38_check_a_attribute_200_ok(int media_line, sdp_message_t *sdp)
{
	sdp_attribute_t *pInAttribute;
	int pos =0;
  
	pInAttribute = sdp_message_attribute_get (sdp,media_line ,pos);
	while (pInAttribute)
	{
 		// ret = t38Confirm(pInAttribute->a_att_field, pInAttribute->a_att_value)
		// if (!ret) reject();
		//   if( sdp_params.T38FaxUdpEnabled )
			// UDPTL support 
		pos++;
		pInAttribute = sdp_message_attribute_get (sdp,media_line, pos);
	}
	return 0;
}