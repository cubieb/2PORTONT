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
 * File:		si_ua_base64.h
 * Purpose:		
 * Created:		Feb 2009
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_UA_BASE64_H
#define SI_UA_BASE64_H
 
#define SI_BASE256_TO_BASE64_LEN(len)	(len * 4 / 3 + 3)

#define SI_BASE64_TO_BASE256_LEN(len)	(len * 3 / 4)

int si_base64_encode(const unsigned char *input, int in_len,char *output, int *out_len);

int si_base64_decode(const char* input, int in_len, unsigned char *out, int *out_len);

#endif //SI_UA_BASE64_H






