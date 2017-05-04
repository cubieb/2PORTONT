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
 * File:		 	si_ua_server_api.h
 * Purpose:		 		 
 * Created:		06/12/2007
 * By:		 	YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_UA_SERVER_API_H
#define SI_UA_SERVER_API_H

/*========================== Include files ==================================*/
#include "../exosip2/src/eXosip.h"
#include "si_ua_init.h"
 
#include "../common/si_phone_api.h"
#include "../CCFSM/ccfsm_api.h"

/*========================== Local macro definitions & typedefs =============*/
 /*========================== Global variables ===============================*/


/*========================== Global function prototypes =====================*/
void si_ua_configure(int iface, void* params, int size);


int sip_answer_call(int callid, int codectype, int portid, int accountid);
int sip_hold_call(int callid, int flag);
int sip_resume_call(int callid, int flag);
int sip_blindtransfer_call(int callid, char *number);
int sip_attendedtransfer_call(int fromid, int toid);

int sip_terminate_call(int callid);
int sip_invite(ccfsm_outgoing_call_w_replaces_req_data *pCallData);
void sip_reset(void);
void sip_dtmf_start(char key);
int sip_send_instant_message(char  *pIMtext, int accountid,char *dialled_num, int callType );
void sip_fax_start(  int callid, int faxevent); 

void sip_incall_codec_change(int callid, int codec) ;
int sip_terminate_call(int callid);
 
//sicall_t *FindCallPerClient(int clientID, int vline, int extension );
int GetUserAgentStatus(int clientID);
  
sicall_t* sip_find_callid(int index); 
int sip_presence_subscribe (ccfsm_attached_entity attachedentity, int accountID, char *subAccount, char *subRealm, int expires);
int sip_presence_publish (int accountID, pr_status_basic pres, pr_note note, int expires);
int sip_registration(int account, int status);

#endif /* SI_UA_SERVER_API_H */

