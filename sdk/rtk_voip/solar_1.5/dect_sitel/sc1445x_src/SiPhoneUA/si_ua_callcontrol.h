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
 * File:		si_ua_callcontrol.h
 * Purpose:		
 * Created:		Nov 2007
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_UA_CALLCONTROL_H
#define SI_UA_CALLCONTROL_H

#include "si_ua_init.h"
#include "../CCFSM/ccfsm_api.h"

//				REGISTER SECTION
siregistration_t *si_ua_get_reg_account(int rid);
int si_ua_register(SICORE* pCore,  siregistration_t *pRegisterInfo, int regEnable);
int si_ua_unregister(SICORE* pCore, siregistration_t *pRegisterInfo);
int checkToReRegister(SICORE* pCore);
 
int si_ua_registrationended(int rid, eXosip_event_t *pEvent, int flag);
//				SUBSCRIBE SECTION
int si_ua_subscribe(SICORE* pCore);
//				INCOMING CALL SECTION
int gerenateCallIdentification(void);
 
void si_ua_incomingCall(eXosip_event_t *pEvent);
int si_ua_setRinging(sicall_t* pCall, osip_message_t*); 
int si_ua_acceptCall( sicall_t* pCall);
int	openRTPSocketOnCallAccept(sicall_t* pCall,eXosip_event_t *pEvent);

 //MSG
int si_ua_recv_msgAck(sicall_t *pCall, eXosip_event_t *pEvent);

int si_ua_redirect(sicall_t *pCall, eXosip_event_t *pEvent, char *redirectionid, char* reason);
int si_ua_rejectCall(sicall_t* pCall);

//				OUTGOINING CALL SECTION
int  si_ua_recv_callProceeding(sicall_t *pCall, eXosip_event_t *pEvent);
int si_ua_callEstablish( int accountID, int portId,int line, int codec, char *fromURI, char*toURI, int IPCall, sicall_t* rCall);
int si_ua_Invite_w_Replace( int accountID, int clientId, int vline, int codec,char *fromURI, char*toURI, int IPCall, sicall_t *rCall,char *referredBy, char *replaces);

int si_ua_recv_ringing(sicall_t *pCall, eXosip_event_t *pEvent);
osip_message_t * si_ua_build_reply_on_ACK200(sicall_t *pCall);

osip_message_t * si_ua_recv_callAnswered(sicall_t *pCall , eXosip_event_t *pEvent);

//				MANAGEMENT CALL SECTION
int si_ua_blindTransfer(sicall_t *pCall, char *remote);
int si_ua_attendedTransfer(sicall_t *pCallTargetTranfer, sicall_t *pCall) ; 

//int si_ua_blindTransfer(int clientId, int line, char *refer_to, char*, char*);
int si_ua_transmit_Xrefer(sicall_t *pCall);
int si_ua_recv_ack(sicall_t* pCall, eXosip_event_t *pEvent);
int si_ua_recv_callClosed(sicall_t* pCall, eXosip_event_t *pEvent);
int si_ua_recv_callReleased(sicall_t* pCall, eXosip_event_t *pEvent);

int si_ua_callTerminated(eXosip_event_t *pEvent);
int si_ua_terminateCall(sicall_t* pCall);

//				BASIC CALL FUNCTIONs
sicall_t* allocateCallId(eXosip_event_t* pEvent);
void releaseCallId(sicall_t* pCall);
int isBusy(int);
int isWaiting(int);
int isAnonymousBlocked(int accountID, eXosip_event_t *pEvent);

int checkDECTBroadcastState(int accountID);

int isRedirected(int);
int isAutoAnswer(int);
sicall_t*  getFirstActiveCall(void);
sicall_t*  getOtherActiveCall(sicall_t* curCall );
int findAccount(char *userName );
ccfsm_attached_entity findEntity(int accountID);

void InitiateCall(sicall_t *pCall);
int  incallCodecChecking(int codecType);
char* getRedirectionUri(int);
char* getRedirectionReason(int);
sicall_t*  findCallByLine(int line);
sicall_t*  findCallByCid(int cid);
sicall_t*  findCallByTid(int tid);
sicall_t*  findCallByIds( eXosip_event_t* pEvent);

int si_stop_tone_dial(unsigned short line);
int si_alsa_stop_tone(sicall_t *pCall);
int si_start_tone_busy(sicall_t *pCall);
int si_start_tone_congestion(sicall_t *pCall);
int si_start_tone_dial(unsigned short line);
int si_start_tone_disconnect1(sicall_t *pCall);
int si_start_tone_disconnect2(sicall_t *pCall);
int si_start_tone_ringing(sicall_t *pCall);
int si_start_tone_speeddial(sicall_t *pCall);
int si_start_tone_waiting(sicall_t *pCall);
void si_play_ringtone(sicall_t *pCall, char* filename );
void si_stop_ringtone(sicall_t *pCall);
void si_ua_setconference(sicall_t* pCall1, sicall_t* pCall2);
int checkRequiredHeader(osip_message_t *pPacket);
int sipua_blindtransfer_indication(sicall_t *pCall, int reason);
int create_cback_transfer(sicall_t* pCall, int flag, char *from, char *to, char *replaces);
void create_updatestatus_callback(int id, int accountid, int callid, int parameter, int reason, char*info, int state);
int  getNumOfCalls(sicall_t* curCall );
int findPort(int accountID, int entity);
int isSRTPEnabled(sicall_t *pCall);

#endif //SI_UA_CALLCONTROL_H
