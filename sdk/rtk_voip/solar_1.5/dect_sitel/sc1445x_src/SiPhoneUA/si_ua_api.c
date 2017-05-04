

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
 * File:		 		 si_ua_api.c
 * Purpose:		 		 
 * Created:		 		 Nov 2007
 * By:		 		 	 YP
 * Country:		 		 Greece
 *
 *-----------------------------------------------------------------------------
 */
/*========================== Include files ==================================*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <si_print_api.h>

#include "si_sdp_negotiation.h"
#include "si_ua_api.h"
extern  SICORE siphoneCore;

int apiUAInitRequest(SICORE* pCore)
{

	memcpy(&siphoneCore, pCore, sizeof(SICORE));
	siphoneCore.session_version =0x329387;
 	// Initiate SCORE
 	si_ua_init(&siphoneCore);
 	return 0;
}

 
int api_ua_setBusyFlag(int flag, int AccNum)
{
	siphoneCore.busyFlag[AccNum]=flag;
	 return 0;
}

int api_ua_setAutoAnswerFlag(int flag, int AccNum)
{
	siphoneCore.autoanswer[AccNum]=flag;
	return 0;
}

int api_ua_setCallWaitingFlag(int flag, int AccNum)
{
 	siphoneCore.callwaiting[AccNum]=flag;
	return 0;
}
int api_ua_setHideIdFlag(int flag, int AccNum)
{
 	siphoneCore.hideid[AccNum]=flag;
	return 0;
}
int api_ua_setRejectAnonymousFlag(int flag, int AccNum)
{
	siphoneCore.rejectanonymous[AccNum]=flag;
	return 0;
}

int api_ua_setRedirectFlag(int flag, int AccNum)
{
	siphoneCore.redirectFlag[AccNum]=flag;
 	return 0;
}
int api_ua_setRedirectURIFlag(char* dst, int AccNum)
{
	strcpy(&siphoneCore.redirectURI[AccNum][0],dst);
	return 0;
}
int api_ua_setRedirectReasonFlag(char* reason, int AccNum)
{
	strcpy(&siphoneCore.redirectReason[AccNum][0],reason);
 	return 0;
}

int api_ua_setRedirectTimeFlag(int redTime, int AccNum)
{
	siphoneCore.redirectTime[AccNum]=redTime;
 	return 0;
}
int api_ua_setDtmfModeFlag(int flag, int AccNum)
{
	siphoneCore.DtmfMode[AccNum]=flag;
	return 0;
}  
int api_ua_setTosFlag(int flag, int AccNum)
{
	siphoneCore.tos[AccNum]=flag;
	return 0;
}  

int api_ua_setRegInfo(SIPAccount *p_SIP_Account, char *registrar, int AccNum)
{
 	strcpy(siphoneCore.RegistrarIPAddress, registrar); 
    strcpy(&siphoneCore.regusername[AccNum][0], p_SIP_Account->user_name); 
   	strcpy(&siphoneCore.reglogin[AccNum][0], p_SIP_Account->user_id); 
   	strcpy(&siphoneCore.regpassword[AccNum][0], p_SIP_Account->user_password); 
  	siphoneCore.useRegistration[AccNum] = p_SIP_Account->use_account;
 	return 0;
}  

int api_ua_setCodecList(int *Codecs, int AccNum)
{
 	int i, j;
    for (i=0;i<MAX_SUPPORTED_CODECS;i++) 
		siphoneCore.CodecsSupported [AccNum][i]=-1;

    j=0;
 	for (i=0;i<MAX_SUPPORTED_CODECS;i++)
	{
 		if (Codecs[i]>-1)
		{
  			siphoneCore.CodecsSupported[AccNum][j]=(int)Codecs[i];
 			j++;
		}else break;
	}
 
	siphoneCore.CodecsSupported[AccNum][j]=(int)101;
	eXosip_sdp_negotiation_init(AccNum,  &sdpNegotiationData[AccNum], &siphoneCore); 

	return 0;
}

int api_ua_setCodecSupportedList(int *Codecs, int AccNum)
{
  int i, j;
    for (i=0;i<MAX_SUPPORTED_CODECS;i++) 
	  siphoneCore.CodecsSupported[AccNum][i]=-1;

    j=0;
  for (i=0;i<MAX_SUPPORTED_CODECS;i++)
 {
   if (Codecs[i]>-1)
  {
    siphoneCore.CodecsSupported[AccNum][j]=(int)Codecs[i];
     j++;
  }else break;
 }
  siphoneCore.CodecsSupported[AccNum][j]=(int)101;
   return 0;
}

int api_ua_setAudioParams(int pTime, int Vad, int jitter, int srtpmode, int AccNum)
{
	siphoneCore.VAD[AccNum]=Vad;
	siphoneCore.ptime[AccNum]=pTime;
	siphoneCore.jitter[AccNum] =jitter;
	siphoneCore.crypto_level[AccNum] =srtpmode;

	return 0;
}
int api_ua_Call(char *params) 
{
    	return 0;
}

int api_ua_Answer(char *params) 
{
  	return 0;
}

int api_ua_Reject(char *params) 
{

	return 0;
}

int api_ua_Terminate(char *params) 
{
  
 	return 0;
}

int api_ua_Holdon(char *params) 
{
/*	sicall_t* pCall;
	int line =0;

	pCall = findCallByLine(line);
	if (pCall->localHold==0) 	si_ua_holdOn(line, 0); 
	if (pCall->localHold==1) 	si_ua_holdOff(line); 
 */
 	return 0;
}
int api_ua_Holdoff(char *params) 
{
//	int line =0;
//	si_ua_holdOff(line);
 	return 0;
}

int api_ua_transfer(char *params) 
{
//	int line =0;
//	si_ua_blindTransfer(line,"3001@192.168.0.1", "sip:3000@192.168.0.18","sip:3003@192.168.0.18");

 	return 0;
}
int api_ua_initconf(char *params) 
{
 	return 0;
}

int api_ua_addcall(char *params) 
{
 	return 0;
}
int api_ua_removecall(char *params) 
{
 	return 0;
}
int api_ua_terminateconf(char *params) 
{
 	return 0;
}
int api_ua_SendDtfm(char *params)//int line, int mode, char digit, char duration) 
{
/*	int line =0;
	int mode=DTMF_MODE_INFO;

	switch (mode)
	{
	case DTMF_MODE_INFO:
 		 si_ua_sendDtmfInfo(SiCall[line].did,(unsigned char) 5, (unsigned char)250);
		break;
	case DTMF_MODE_INBAND:
		break;
	case DTMF_MODE_RTPPAYLOAD:
		break;
  	}*/
	return 0;
}

