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
 * File:			call_log_api.h
 * Purpose:		
 * Created:		29/11/2007
 * By:			KF
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef CALL_LOG_API_H
#define CALL_LOG_API_H

/*========================== Include files ==================================*/


/*========================== Local macro definitions & typedefs =============*/
#define DIAL_ACTION			1
#define ACCEPT_ACTION		2
#define TRANSFER_ACTION		3
#define CONFERENCE_ACTION	4
#define REJECT_ACTION		5
#define TERMINATE_ACTION	6
#define HOLD_ACTION 			7
#define MUTE_ACTION 			8
#define SPEAKER_ACTION 		9
#define HOOK_ACTION			10
#define DTMF_ACTION 			11
#define VIEW_ACTION 			12

typedef struct _CallAction {
	char action;
	int line;
	char number[16];
	char state;
	char digit;
} CallAction;

/*========================== Global variables ===============================*/


/*========================== Global function prototypes =====================*/

#endif /* CALL_LOG_API_H */

