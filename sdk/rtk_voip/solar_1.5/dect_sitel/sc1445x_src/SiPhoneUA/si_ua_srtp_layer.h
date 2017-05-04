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
 * (C) Copyright SiTel Semiconductor BV, unpublished work.
 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:		si_ua_srtp_layer.h
 * Purpose:		
 * Created:		Feb 2009
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_UA_SRTP_LAYER_H
#define SI_UA_SRTP_LAYER_H
#include   "si_ua_init.h"
/*typedef enum _ciphers_tipher_types
{
	NULL_CIPHER,  
 	AES_128_ICM, 
 	AES_128_CBC
}ciphers_tipher_types;
 
typedef enum _authentication_function_types
{
	NULL_AUTH,
	UST_AES_128_XMAC,
	HMAC_SHA1
}authentication_function_types;
 
 typedef enum {
  sec_serv_none          = 0, //< no services                        
  sec_serv_conf          = 1, //< confidentiality                    
  sec_serv_auth          = 2, //< authentication                     
  sec_serv_conf_and_auth = 3  //< confidentiality and authentication 
} sec_serv_t;
*/
int srtp_get_crypto_suites(osip_negotiation_t *sdpInfo, sdp_message_t * sdp, int media_pos, crypto_parameters_t* crypto_params);
int srtp_parse_crypto_attributes(crypto_profile *rx_suite, osip_negotiation_t *sdpInfo, char* crypto_data);
void strp_zero_suite(crypto_profile *rx_crypto_suite) ;
//void strp_add_suite(osip_negotiation_t *sdpInfo, crypto_profile *rx_suite);
int strp_match_suite(osip_negotiation_t *sdpInfo, crypto_profile *rx_suite, crypto_parameters_t* crypto_params, int tag);

int srtp_add_crypto_suite_into_sdp_answer(crypto_parameters_t *pcrypto_params,sdp_message_t * sdp, int media_pos);

int srtp_add_crypto_suite_into_sdp_offer(crypto_parameters_t *pcrypto_params,osip_negotiation_t *sdpInfo,sdp_message_t * sdp, int media_pos);
int srtp_generate_key(char* key, int keylen) ;


#endif //SI_UA_SRTP_LAYER_H






