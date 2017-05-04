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
 * File:		 		 si_ua_srtp_layer.c
 * Purpose:		 		 
 * Created:		 		 Feb 2009
 * By:		 		 	 YP
 * Country:		 		 Greece
 *
 *-----------------------------------------------------------------------------
 */
/*========================== Include files ==================================*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>  
#include "si_ua_init.h"
#include "si_sdp_negotiation.h"
#include "si_ua_srtp_layer.h"
#include "si_ua_base64.h"
 
static const si_ua_str_t ID_CRYPTO   = { "crypto", 6 };

int srtp_get_crypto_suites(osip_negotiation_t *sdpInfo, sdp_message_t * sdp, int media_pos, crypto_parameters_t* pcrypto_params)
{
	crypto_profile rx_crypto_suite;
	int a_attr_pos =0;
	int encryption =0;
	int ret;
	char *a_att_name, *a_att_value;
  
	a_att_name = sdp_message_a_att_field_get (sdp, media_pos, a_attr_pos);
  	if (a_att_name == NULL){
		a_attr_pos++;
  		a_att_name = sdp_message_a_att_field_get (sdp, media_pos, a_attr_pos);
 	}

	while (a_att_name != NULL)
	{
 
		if (!strncmp(a_att_name, "crypto", 6))
  			encryption = 1;
  		if (!strncmp(a_att_name, "encryption", 10))
			encryption = 1;
 		if (!strncmp(a_att_name, ID_CRYPTO.data, ID_CRYPTO.slen ))
			encryption = 1;
 		if (encryption)
		{
 			a_att_value = sdp_message_a_att_value_get (sdp, media_pos, a_attr_pos);
			ret = srtp_parse_crypto_attributes(&rx_crypto_suite, sdpInfo, a_att_value);
			if (ret>0) {
	  			 if (strp_match_suite(sdpInfo,&rx_crypto_suite, pcrypto_params, ret))
				 {
   					return 0;
				 } 
			} 
		}

		a_attr_pos++;
		a_att_name = sdp_message_a_att_field_get (sdp, media_pos, a_attr_pos);
 	}
 	return -1;
}
int srtp_parse_crypto_attributes(crypto_profile *rx_suite, osip_negotiation_t *sdpInfo, char* crypto_data)
{
	int tag;
	int ret;
	char *token;
   
	//extract tag info
    token = strtok(crypto_data, " ");
    if (!token) {
 		return -1;
    }
    tag = atoi(token);
	if (tag == 0) {
 		return -1;
	}
     //extract crypto suite info
    token = strtok(NULL, " "); 
    if (!token) {
 		return -1;
    }
	if (strlen(token)<MAX_CRYPTO_SUITE_SIZE)
	{
		  strcpy(rx_suite->suite, token) ;
		 rx_suite->suite_len = strlen(rx_suite->suite);
	}
 	// Key method  
    token = strtok(NULL, ":");
    if (!token) {
 		return -1;
    }
 	if (strncmp(token, "inline", 6)) {
  		return -1;
    }
     // Key 
    token = strtok(NULL, "| ");
    if (!token) {
 		return -1;
    }
	if (strlen(token)<MAX_CRYPTO_KEY_64_SIZE)
	{
		rx_suite->key_len=MAX_CRYPTO_KEY_SIZE;
		ret=si_base64_decode((char*)token, strlen(token), (unsigned char*)rx_suite->key,(int*) &rx_suite->key_len);
 		return tag;
	} 
 	return -1;
}

void strp_zero_suite(crypto_profile *rx_crypto_suite) 
{
	memset(rx_crypto_suite, 0, sizeof (crypto_profile) ); 
 
}
/*
void strp_put_suite(osip_negotiation_t *sdpInfo, crypto_profile *rx_suite)
{
	int i;
	for (i=0;i<MAX_ENCRYPTION_SUITES;i++)
	{
		if (!sdpInfo->rx_encryption_suite[i].enabled)
		{
			memcpy(&sdpInfo->rx_encryption_suite[i], rx_suite,sizeof (crypto_profile));
		}
 	}
}
*/
 
int strp_match_suite(osip_negotiation_t *sdpInfo, crypto_profile *rx_suite, crypto_parameters_t* crypto_params, int tag)
{
	int i;
	strp_zero_suite(&crypto_params->rx_encryption_suite);
 
 
	for (i=0;i<MAX_ENCRYPTION_SUITES;i++)
	{
 		if (sdpInfo->tx_encryption_suites[i].enabled)
		{
    		if (!memcmp( sdpInfo->tx_encryption_suites[i].suite , rx_suite->suite , rx_suite->suite_len ))
			{
  				memcpy(&crypto_params->rx_encryption_suite, rx_suite,sizeof (crypto_profile));
			 	memcpy(crypto_params->tx_encryption_suite[i].suite,  rx_suite->suite  ,rx_suite->suite_len);
  
				crypto_params->tx_encryption_suite_id =i;
				crypto_params->tx_encryption_suite[i].flag = sdpInfo->tx_encryption_suites[i].flag ;
				crypto_params->tag = tag; 				
				crypto_params->tx_encryption_suite[i].suite_len = rx_suite->suite_len;
				crypto_params->rx_encryption_suite.suite_len = rx_suite->suite_len;

				crypto_params->tx_encryption_suite[i].key_len =sdpInfo->tx_encryption_suites[i].key_len;
				crypto_params->rx_encryption_suite.key_len = sdpInfo->tx_encryption_suites[i].key_len;


				crypto_params->tx_encryption_suite[i].enabled  =1;
				crypto_params->rx_encryption_suite.enabled  =1;
  				return 1;
			}
		}
 	}
 	crypto_params->tx_encryption_suite[i].enabled  =0;
	crypto_params->rx_encryption_suite.enabled  =0;
	return 0;
}
 
int srtp_add_crypto_suite_into_sdp_answer(crypto_parameters_t *pcrypto_params,sdp_message_t * sdp, int media_pos)
{
    int ret;
	char tmp[128];
	char tmpkey[64];//SI_BASE256_TO_BASE64_LEN (MAX_CRYPTO_KEY_SIZE)+1];
	int keysize=SI_BASE256_TO_BASE64_LEN (MAX_CRYPTO_KEY_SIZE);
  	memset(tmpkey,0,SI_BASE256_TO_BASE64_LEN (MAX_CRYPTO_KEY_SIZE)+1);

   	if (!pcrypto_params->tx_encryption_suite[pcrypto_params->tx_encryption_suite_id].enabled || !pcrypto_params->rx_encryption_suite.enabled) 
		return 0 ; 

	if (pcrypto_params->keyGerenated) 
	{
 			memset(tmp,0,128);
			memset(tmpkey,0,64);//SI_BASE256_TO_BASE64_LEN (MAX_CRYPTO_KEY_SIZE)+1);
   			keysize=SI_BASE256_TO_BASE64_LEN (MAX_CRYPTO_KEY_SIZE);
  			ret = si_base64_encode(
							(unsigned char*)pcrypto_params->tx_encryption_suite[pcrypto_params->tx_encryption_suite_id].key,
							pcrypto_params->tx_encryption_suite[pcrypto_params->tx_encryption_suite_id].key_len,
							tmpkey,
							&keysize);

			sprintf(tmp, "%d %s inline:%s", pcrypto_params->tag, pcrypto_params->tx_encryption_suite[pcrypto_params->tx_encryption_suite_id].suite,tmpkey); 
 		sdp_message_a_attribute_add (sdp, media_pos,osip_strdup ("crypto"), osip_strdup(tmp));
		return 0;
	}

  	//generate key 
   	srtp_generate_key(pcrypto_params->tx_encryption_suite[pcrypto_params->tx_encryption_suite_id].key, 
		pcrypto_params->tx_encryption_suite[pcrypto_params->tx_encryption_suite_id].key_len);

    pcrypto_params->rx_encryption_suite.key_len = pcrypto_params->tx_encryption_suite[pcrypto_params->tx_encryption_suite_id].key_len;
	ret = si_base64_encode(
		(unsigned char*)pcrypto_params->tx_encryption_suite[pcrypto_params->tx_encryption_suite_id].key,
		 pcrypto_params->tx_encryption_suite[pcrypto_params->tx_encryption_suite_id].key_len,  
		tmpkey,
		&keysize);

 
	if (ret!=0) return -1;
	pcrypto_params->keyGerenated=1;
 	sprintf(tmp, "%d %s inline:%s", pcrypto_params->tag, pcrypto_params->tx_encryption_suite[pcrypto_params->tx_encryption_suite_id].suite,tmpkey); 
    sdp_message_a_attribute_add (sdp, media_pos,osip_strdup ("crypto"), osip_strdup(tmp));
 	 

	return 0;
}

int srtp_add_crypto_suite_into_sdp_offer(crypto_parameters_t *pcrypto_params, osip_negotiation_t *sdpInfo,sdp_message_t * sdp, int media_pos)
{
    int ret;
	int suite_count;
	char tmp[128];
	char tmpkey[64];////[SI_BASE256_TO_BASE64_LEN (MAX_CRYPTO_KEY_SIZE)+1];
	int keysize=SI_BASE256_TO_BASE64_LEN (MAX_CRYPTO_KEY_SIZE);

 	if (!((sdpInfo->crypto_level==ID_CRYPTO_MANDATORY) || (sdpInfo->crypto_level==ID_CRYPTO_OPTIONAL)))
	return 0;

	if (pcrypto_params->keyGerenated) 
	{
 			memset(tmp,0,128);
			memset(tmpkey,0,64);//SI_BASE256_TO_BASE64_LEN (MAX_CRYPTO_KEY_SIZE)+1);
 			keysize=SI_BASE256_TO_BASE64_LEN (MAX_CRYPTO_KEY_SIZE);
			ret = si_base64_encode(
						(unsigned char*)pcrypto_params->tx_encryption_suite[pcrypto_params->tx_encryption_suite_id].key,
						pcrypto_params->tx_encryption_suite[pcrypto_params->tx_encryption_suite_id].key_len,
						tmpkey,
						&keysize);

			sprintf(tmp, "1 %s inline:%s", pcrypto_params->tx_encryption_suite[pcrypto_params->tx_encryption_suite_id].suite,tmpkey); 
     		sdp_message_a_attribute_add (sdp, media_pos,osip_strdup ("crypto"), osip_strdup(tmp));
		return 0;
	}

   	for (suite_count=0;suite_count<MAX_ENCRYPTION_SUITES;suite_count++)
	{
 		if (sdpInfo->tx_encryption_suites[suite_count].enabled)
		{
 			memset(tmp,0,128);
			memset(tmpkey,0,64);//SI_BASE256_TO_BASE64_LEN (MAX_CRYPTO_KEY_SIZE)+1);
   			keysize=SI_BASE256_TO_BASE64_LEN (MAX_CRYPTO_KEY_SIZE);
  			//generate key
 			memcpy(&pcrypto_params->tx_encryption_suite[suite_count], &sdpInfo->tx_encryption_suites[suite_count], sizeof(crypto_profile));
   			srtp_generate_key(pcrypto_params->tx_encryption_suite[suite_count].key,
				pcrypto_params->tx_encryption_suite[suite_count].key_len);
  		    pcrypto_params->rx_encryption_suite.key_len = pcrypto_params->tx_encryption_suite[suite_count ].key_len;
  			ret = si_base64_encode(
							(unsigned char*)pcrypto_params->tx_encryption_suite[suite_count].key,
							pcrypto_params->tx_encryption_suite[suite_count].key_len,
							tmpkey,
							&keysize);
   			if (ret!=0) return -1;
			pcrypto_params->keyGerenated=1;
   			sprintf(tmp, "%d %s inline:%s", suite_count+1,pcrypto_params->tx_encryption_suite[suite_count].suite,tmpkey); 
     		sdp_message_a_attribute_add (sdp, media_pos,osip_strdup ("crypto"), osip_strdup(tmp));
  		//	break;
 		}
 	}
 	return 0;
}

int srtp_generate_key(char* key, int keylen) 
{
	int i ; 
//	ret = crypto_get_random(key, keylen);
 	for (i=0;i<keylen;i++)
  		key[i] = (unsigned char)random();		
 
   	return 0;
}
