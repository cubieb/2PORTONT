
 /*-----------------------------------------------------------------------------
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
 * File:		 	si_cc_server_api.h
 * Purpose:		 		 
 * Created:		20/10/2007
 * By:		 	  EF
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_CC_SERVER_API_H
#define SI_CC_SERVER_API_H

/*========================== Include files ==================================*/
#include "../CCFSM/ccfsm_api.h"

/*========================== Local macro definitions & typedefs =============*/
  
#define MAX_NUM_OF_CLIENTS	2

typedef struct _ClientInfo {
	char ClientName[15];
	unsigned short PortToSendAnswer;
	unsigned short ClientPort;
	unsigned long ClientIPAddress;
	char ConnectStatus;
} ClientInfo;

/*========================== Global variables ===============================*/
int si_cc_api_process(void);
void sc1445x_CONSOLE_RETRANSMIT(void *data, int size);
void parse_iface_request(ccfsm_req_type *pReq);

/*========================== Global function prototypes =====================*/


#endif /* SI_CC_SERVER_API_H */

