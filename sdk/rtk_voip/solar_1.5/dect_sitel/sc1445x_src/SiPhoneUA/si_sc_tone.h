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
 * File:		si_sc_tone.h
 * Purpose:		
 * Created:		Feb 2008
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_SC_TONE_H
#define SI_SC_TONE_H

 
#include "si_ua_init.h"
#include "si_sdp_negotiation.h"

typedef enum SI_TONE_ID_T
{
    SI_TONE_DTMF_0              = '0',   /**< DMTF 0 */
    SI_TONE_DTMF_1              = '1',   /**< DMTF 1 */
    SI_TONE_DTMF_2              = '2',   /**< DMTF 2 */
    SI_TONE_DTMF_3              = '3',   /**< DMTF 3 */
    SI_TONE_DTMF_4              = '4',   /**< DMTF 4 */
    SI_TONE_DTMF_5              = '5',   /**< DMTF 5 */
    SI_TONE_DTMF_6              = '6',   /**< DMTF 6 */
    SI_TONE_DTMF_7              = '7',   /**< DMTF 7 */
    SI_TONE_DTMF_8              = '8',   /**< DMTF 8 */
    SI_TONE_DTMF_9              = '9',   /**< DMTF 9 */
    SI_TONE_DTMF_STAR           = '*',   /**< DMTF * */
    SI_TONE_DTMF_POUND          = '#',   /**< DMTF # */
    SI_TONE_DTMF_FLASH          = '!',   /**< DTMF Flash */
    SI_TONE_TONE_DIALTONE  = 512,        /**< Dialtone */
    SI_TONE_TONE_BUSY,                   /**< Call-busy tone */
    SI_TONE_TONE_RINGBACK,               /**< Remote party is ringing feedback tone */
    SI_TONE_TONE_RINGTONE,               /**< Default ring/alert tone */
    SI_TONE_TONE_CALLFAILED,             /**< Fasy Busy / call failed tone */
    SI_TONE_TONE_SILENCE,                /**< Silence */
    SI_TONE_TONE_BACKSPACE,              /**< Backspace tone */
    SI_TONE_TONE_CALLWAITING,            /**< Call waiting alert tone*/
    SI_TONE_TONE_CALLHELD,               /**< Call held feedback tone */
    SI_TONE_TONE_LOUD_FAST_BUSY          /**< Off hook / fast busy tone */
} SI_TONE_ID ;

  
#endif //SI_SC_TONE_H
