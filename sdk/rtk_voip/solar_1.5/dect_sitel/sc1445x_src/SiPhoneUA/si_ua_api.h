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
 * File:		si_ua_api.h
 * Purpose:		
 * Created:		Nov 2007
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_UA_API_H1
#define SI_UA_API_H1
#include "../common/si_config_file.h"
#include "../common/si_phone_api.h"
  
 ////////////////////////////////////////////////////////////////////////////////////////
int apiUAInitRequest(SICORE* pCore);
 
int api_ua_setBusyFlag(int flag, int AccNum);
int api_ua_setAutoAnswerFlag(int flag, int AccNum);
int api_ua_setCallWaitingFlag(int flag, int AccNum);
int api_ua_setHideIdFlag(int flag, int AccNum);
int api_ua_setRejectAnonymousFlag(int flag, int AccNum);
int api_ua_setRedirectFlag(int flag, int AccNum);
int api_ua_setRedirectURIFlag(char* dst, int AccNum);
int api_ua_setRedirectReasonFlag(char* reason, int AccNum);
int api_ua_setRedirectTimeFlag(int redTime, int AccNum);

int api_ua_setDtmfModeFlag(int flag, int AccNum);
int api_ua_setTosFlag(int flag, int AccNum);

int api_ua_setCodecList(int *Codecs, int AccNum);

int api_ua_Call(char *params); 
int api_ua_Answer(char *params); 
int api_ua_Reject(char *params); 
int api_ua_Terminate(char *params); 
int api_ua_Holdon(char *params); 
int api_ua_Holdoff(char *params); 
int api_ua_transfer(char *params); 
int api_ua_initconf(char *params);
int api_ua_addcall(char *params); 
int api_ua_removecall(char *params); 
int api_ua_terminateconf(char *params); 
int api_ua_SendDtfm(char *params);
int api_ua_setCodecSupportedList(int *Codecs, int AccNum);
int api_ua_setAudioParams(int pTime, int Vad, int jitter, int srtpmode, int AccNum);

int api_ua_setRegInfo(SIPAccount *p_SIP_Account, char *registrar, int AccNum);

#endif //SI_UA_API_H1
