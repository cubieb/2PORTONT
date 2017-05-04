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
 * File:		si_ua_client_api.h
 * Purpose:		 		 
 * Created:		06/12/2007
 * By:		 	KF
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_UA_CLIENT_API_H
#define SI_UA_CLIENT_API_H

/*========================== Include files ==================================*/
#include "../../common/si_phone_api.h"

/*========================== Local macro definitions & typedefs =============*/
#define CONSOLE_SERVER_PORT		17173
#define UA_SERVER_IP_ADDRESS	"192.168.0.16"

/*========================== Global variables ===============================*/


/*========================== Global function prototypes =====================*/
void ParsePacket(CallBackParams  *pCallBack);
int si_api_ua_client_init(void);
int client_init(void);
int server_init(void);
int receive_loop(void * arg);
int ConnectToUAServer(void);
int SendCmd(char *buffer, int buf_len);


#endif /* SI_UA_CLIENT_API_H */
