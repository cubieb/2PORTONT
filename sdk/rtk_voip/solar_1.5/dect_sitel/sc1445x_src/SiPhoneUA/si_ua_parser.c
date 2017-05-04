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
 * File:		 		 si_ua_parser.c
 * Purpose:		 		 
 * Created:		 		 Nov 2007
 * By:		 		 	 YP
 * Country:		 		 Greece
 *
 *-----------------------------------------------------------------------------
 */
/*========================== Include files ==================================*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <si_print_api.h>
 
#include "si_sdp_negotiation.h"
#include "si_ua_parser.h"

int getRemoteSDPInfo(sdp_message_t * sdp, unsigned char* remoteaddress, int *remoteport)
{
	int pos_media=-1;
	int pos = 0;
	char *sndrcv = NULL;
 	unsigned char *ipaddr = NULL;
 	char *remoteportchar = NULL;
	
	while (!sdp_message_endof_media(sdp, pos_media))
	{
			ipaddr = (unsigned char *)sdp_message_c_addr_get(sdp, pos_media, pos);
			remoteportchar = sdp_message_m_port_get (sdp, pos);
			if (remoteportchar && ipaddr && remoteaddress && remoteport)
			{
				*remoteport = atoi(remoteportchar);
				strcpy((char *)remoteaddress, (char *)ipaddr);
 			}
			while (ipaddr!=NULL){ /* only one is allowed here? */			
				if (pos==1 && pos_media==-1){
					break;
				}
				if (0==osip_strcasecmp("0.0.0.0", (char *)ipaddr)){
					//si_print(PRINT_LEVEL_INFO, " ADDRESS %s ", ipaddr);
					break;
				}
				pos++;
				ipaddr = (unsigned char *)sdp_message_c_addr_get(sdp, pos_media, pos);
			}
 			if (pos==1 && pos_media==-1){
				ipaddr=NULL;
			}
		if (ipaddr!=NULL) break;
 		pos = 0;
		pos_media++;
  }

 //	if (ipaddr==NULL)
	{
		sndrcv = NULL;
		pos_media=-1;
		pos = 0;
		while (!sdp_message_endof_media(sdp, pos_media)){
 			const char *mtype = sdp_message_m_media_get(sdp, pos_media);
			/* take only audio streams in account */
			if ((mtype ) && !osip_strcasecmp(mtype, "image")){
				return SI_MESSAGE_INVITE_IMAGE;
			}else if ((mtype ) && !osip_strcasecmp(mtype, "audio")){
				sndrcv = sdp_message_a_att_field_get(sdp, pos_media, pos);
  				while (sndrcv!=NULL)
				{
 					if (0==osip_strcasecmp("inactive", sndrcv) || 0==osip_strcasecmp("sendonly", sndrcv) || 0==osip_strcasecmp("recvonly", sndrcv) || 0==osip_strcasecmp("sendrecv", sndrcv)){
						break;
					}
					pos++;
					sndrcv = sdp_message_a_att_field_get(sdp, pos_media, pos);
				}
			}
//add
 			sndrcv = sdp_message_a_att_field_get(sdp, pos_media, pos);
 			if (0==osip_strcasecmp("inactive", sndrcv) || 0==osip_strcasecmp("sendonly", sndrcv) || 0==osip_strcasecmp("recvonly", sndrcv) || 0==osip_strcasecmp("sendrecv", sndrcv)){
 				break;
			}
//add
 			if (sndrcv!=NULL){
				break;
			}
			pos = 0;
			pos_media++;
		}
	}	 
	if (sndrcv!=NULL)
		si_print(PRINT_LEVEL_INFO, "SNDRCV : %s \n",sndrcv ) ;
	else
		si_print(PRINT_LEVEL_INFO, "SNDRCV =NULL \n");

	if (ipaddr!=NULL)
		si_print(PRINT_LEVEL_INFO, "ipaddr : %s \n",ipaddr ) ;
	else si_print(PRINT_LEVEL_INFO, "ipaddr=NULL \n ");

	if (ipaddr!=NULL || (sndrcv!=NULL && (0==osip_strcasecmp("inactive", sndrcv) || 0==osip_strcasecmp("sendonly", sndrcv))))
	{
		if (sndrcv!=NULL && (0==osip_strcasecmp("recvonly", sndrcv))) 
			return SI_MESSAGE_INVITE_OFF_HOLD;
		else
		//  We received an INVITE to put on hold the other party. 
		return SI_MESSAGE_INVITE_ON_HOLD;
	}else{
		// This is a call modification, probably for taking it of hold 
		return SI_MESSAGE_INVITE_OFF_HOLD;
	}
		/*else{

			 if (pos!=0){
				if(body != NULL && body->body != NULL && osip_strcasecmp(body->body, "holdon") == 0){
					//  We received an INVITE to put on hold the other party.  
					return SI_MESSAGE_INVITE_ON_HOLD;
				}else if(body != NULL && body->body != NULL && osip_strcasecmp(body->body, "holdoff") == 0){
					// This is a call modification, probably for taking it of hold  
					return SI_MESSAGE_INVITE_OFF_HOLD;
					return;
				}
			} 
		}*/
 	return 0;
}

 
int get_c_address_port(sdp_message_t * sdp, unsigned char* remoteaddress,int *remoteport)
{
	int pos_media=0;
  unsigned char *c_address = NULL;
 	char *remoteportchar=NULL;
	char *media_type;
	if ((sdp==NULL) || (remoteaddress==NULL) || (remoteport==NULL)) return -1;
 	c_address = (unsigned char *)sdp_message_c_addr_get(sdp,-1, 0);
	while (!sdp_message_endof_media(sdp, pos_media))
	{
 		media_type= sdp_message_m_media_get(sdp, pos_media);
  	 	if (!osip_strcasecmp(media_type, "audio"))
		{
 			remoteportchar = sdp_message_m_port_get (sdp, pos_media);
 			if (remoteportchar && c_address )
			{
				*remoteport = atoi(remoteportchar);
				strcpy((char *)remoteaddress, (char *)c_address);
				return 0;
			}
		}
		pos_media++;
   }
   return -1;
}
//FIX BROADWORKS - TEST PLAN 7
 
int check_incoming_sdp(	sicall_t* pCall, sdp_message_t * sdp,int flag)
{
	char *pInSessionId  = sdp_message_o_sess_id_get(sdp);
	char *pInSessionVer = sdp_message_o_sess_version_get(sdp);
	  	if (pInSessionId && pInSessionVer)
	{
		if (!strcmp(pCall->in_session_id, pInSessionId))
		{
			if (!strcmp(pCall->in_session_version,pInSessionVer))	
			{
				 
				return 1;
			}
			else  strcpy(pCall->in_session_version ,pInSessionVer);
 		}
	} 
 	strcpy(pCall->in_session_version, pInSessionVer);
	strcpy(pCall->in_session_id, pInSessionId);
 
	return 0;
}
