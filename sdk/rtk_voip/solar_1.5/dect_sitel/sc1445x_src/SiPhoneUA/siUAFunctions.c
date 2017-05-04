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
 * File:		 		 siUAFunctions.c
 * Purpose:		 		 
 * Created:		 		 Jul 2008
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
#include "siUAFunctions.h"
#include <ortp/stun.h>


int libUASetCore(unsigned char* IPAddress, ConfigSettings *pConfigSettings, SICORE *pCore)
{
 int AccountNumber;

 for (AccountNumber=0;AccountNumber<MAX_SUPPORTED_ACCOUNTS;AccountNumber++)
 {
   libUASetSIPAccounts(&pConfigSettings->m_SIPAccount[AccountNumber], AccountNumber, pCore);
   libUASetCallSettings(&pConfigSettings->m_CallSettings[AccountNumber], AccountNumber, pCore);
   libUASetAudioSettings(&pConfigSettings->m_AudioSettings[AccountNumber], AccountNumber, pCore);
 }   
 
 libUASetSIPSettings(&pConfigSettings->m_SIPSettings,pCore);
 libUASetNetApplicationSettings(&pConfigSettings->m_NetApplications, pCore);
 libUASetNetworkSettings(&pConfigSettings->m_NetworkSettings, pCore);

#if (!((defined ATA_ENABLED) || (defined DECT_ENABLED)))
  libUASetPhoneSettings(&pConfigSettings->m_PhoneSettings,pCore);
#endif
  memcpy(pCore->IPAddress, IPAddress, 64); 

 return 0;
}

int libUASetSetCodecSupported(ConfigSettings *pConfigSettings)
{
 int AccNum;
 for (AccNum=0;AccNum<MAX_SUPPORTED_ACCOUNTS; AccNum++)
 {
	api_ua_setCodecSupportedList(pConfigSettings->m_AudioSettings[AccNum].codec,  AccNum);
 }
 return 0;
}

int libUASetNonCriticalCoreValues(ConfigSettings *pConfigSettings)
{
	int AccNum;
 	for (AccNum=0;AccNum<MAX_SUPPORTED_ACCOUNTS; AccNum++)
	{
		api_ua_setBusyFlag(pConfigSettings->m_CallSettings[AccNum].DND, AccNum);
 		api_ua_setAutoAnswerFlag(pConfigSettings->m_CallSettings[AccNum].auto_answer , AccNum);
 		api_ua_setCallWaitingFlag(pConfigSettings->m_CallSettings[AccNum].call_waiting, AccNum);
 		api_ua_setHideIdFlag(pConfigSettings->m_CallSettings[AccNum].hide_id, AccNum);
 		api_ua_setRejectAnonymousFlag(pConfigSettings->m_CallSettings[AccNum].reject_anonymous, AccNum);
 		api_ua_setRedirectFlag(pConfigSettings->m_CallSettings[AccNum].call_forward_mode, AccNum);
 		api_ua_setRedirectURIFlag(pConfigSettings->m_CallSettings[AccNum].call_forward_number, AccNum);

  	// api_ua_setRedirectReasonFlag(pConfigSettings->m_CallSettings[AccNum].call_forward_start_time,  AccNum);	
		// api_ua_setRedirectTimeFlag(int redTime, int AccNum)
 		api_ua_setDtmfModeFlag(pConfigSettings->m_AudioSettings[AccNum].DTMF_mode, AccNum);
 		api_ua_setTosFlag(pConfigSettings->m_AudioSettings[AccNum].TOS, AccNum);
	
 		api_ua_setCodecList(pConfigSettings->m_AudioSettings[AccNum].codec,  AccNum);
 		api_ua_setAudioParams(pConfigSettings->m_AudioSettings[AccNum].ptime, pConfigSettings->m_AudioSettings[AccNum].VAD, pConfigSettings->m_AudioSettings[AccNum].jitter ,pConfigSettings->m_AudioSettings[AccNum].SRTP_mode , AccNum);	
 	}  
  return 0;
}

int libUASetCriticalCoreValues(ConfigSettings *pConfigSettings)
{
	int AccNum;
 	for (AccNum=0;AccNum<MAX_SUPPORTED_ACCOUNTS; AccNum++)
	{
	   api_ua_setRegInfo(&pConfigSettings->m_SIPAccount[AccNum], pConfigSettings->m_SIPSettings.registrar, AccNum);
	}
 	return 0;
}

int libUASetNetApplicationSettings(NetApplications *pNetApplications, SICORE *pCore)
{
	pCore->Presence = pNetApplications->presence;
	
	return 0;
}

int libUASetNetworkSettings(NetworkSettings *pNetwork, SICORE *pCore)//FIX VAG
{
 pCore->IPv6 = pNetwork->IPMode;
  return 0;
}

int libUASetSIPSettings(SIPSettings *pSIPSettings, SICORE *pCore)
{
 	strcpy(pCore->RegistrarIPAddress, pSIPSettings->registrar); 
	strcpy(pCore->StunAddress, pSIPSettings->STUN_address );//"194.221.62.209");//80.239.235.209");//stun.voipbuster.com");
 	pCore->transport =(int) pSIPSettings->transport;

	if ((pCore->transport < TRANSPORT_UDP) || (pCore->transport > TRANSPORT_TLS))
		pCore->transport = TRANSPORT_UDP;

	pCore->NATMode= (int)pSIPSettings->NAT;
	 
	if (pCore->NATMode<0 || pCore->NATMode>3) 
		pCore->NATMode=0;
	pCore->StunPort=(int)atoi(pSIPSettings->STUN_port);
    if (!pCore->StunPort || (pCore->StunPort>0xffff))

	pCore->StunPort=3478;
	pCore->SessionTimerEnable = (int)pSIPSettings->SessionTimer ; 
	pCore->SessionExpireTimer=300;
	pCore->MinSessionExpireTimer=90;
	pCore->Naptr = (int)pSIPSettings->NAPTR; 
	pCore->Prack = (int)pSIPSettings->PRACK; 
	pCore->Update = (int)pSIPSettings->update; 

  pCore->localAudioRtpPort = 18000;
	pCore->SipServerPort=atoi(pSIPSettings->LocalSipPort);
	if (pCore->SipServerPort==0)
		pCore->SipServerPort = getRandomPort();
 	else if (pCore->SipServerPort<1024 || pCore->SipServerPort>65536)
		pCore->SipServerPort = 5060;
	
	pCore->SipServerid=STANDARD_SIP_SERVER;
	if (pSIPSettings->registrarPort!="")
		pCore->remoteRegistrarPort = atoi(pSIPSettings->registrarPort); // Tue 14 Oct (sipit23)
	else
		pCore->remoteRegistrarPort = 5060;
 
	pCore->regExpTime= 3600;
	pCore->localSIPPort=5060;
 	return 0;
} 

int getRandomPort(void)
{
	int min=0x4000;
	int max=0x7FFF;
	int ret, ret1;
 	ret1 = stunGetSystemTimeSecs();
	srand( ret1);
  ret = random();
	 
	ret = ret|min;
	ret = ret&max;
	return ret; 
}

int libUASetSIPAccounts(SIPAccount *pAccount, int AccNum, SICORE *pCore)
{
	strcpy(&pCore->regusername[AccNum][0], pAccount->user_name); 
	strcpy(&pCore->reglogin[AccNum][0], pAccount->user_id); 
	strcpy(&pCore->regpassword[AccNum][0], pAccount->user_password); 
	pCore->useRegistration[AccNum] = (int)pAccount->use_account;
	return 0;
 } 

#ifndef ATA_ENABLED
int libUASetPhoneSettings(PhoneSettings *pPhoneSettings, SICORE *pCore)
{
 	memcpy(pCore->ringToneSelection, pPhoneSettings->ring_tone, strlen(pPhoneSettings->ring_tone));
	pCore->keypadToneSelection = (int)pPhoneSettings->keypad_tone;
  
  return 0;
}
#endif

int libUASetCallSettings(CallSettings *pCallSettings, int AccNum, SICORE *pCore)
{
  pCore->busyFlag[AccNum] = (int)pCallSettings->DND;
 	pCore->autoanswer[AccNum] =(int)  pCallSettings->auto_answer; 
	pCore->callwaiting[AccNum]= (int)  pCallSettings->call_waiting; 
	pCore->hideid[AccNum] =  (int)pCallSettings->hide_id;
	// siphoneCore.rejectanonymous = pConfig->m_CallSettings.rejectanonymous; //TO ADD CONFIGFILE
  pCore->redirectFlag[AccNum] = (int) pCallSettings->call_forward_mode;
 	strcpy(pCore->redirectURI[AccNum],  pCallSettings->call_forward_number); 
	strcpy(pCore->redirectReason[AccNum], "no-answer"); //TO ADD CONFIGFILE
  
  return 0;
}

int libUASetAudioSettings(AudioSettings *pAudioSettings, int AccNum, SICORE *pCore)
{
	int i, j=0; 

	for (i=0;i<MAX_SUPPORTED_CODECS;i++){
	  pCore->Codecs[AccNum][i] = -1;
	  pCore->CodecsSupported[AccNum][i] = (int)-1;
	}
  j=0;
  for (i=0;i<MAX_SUPPORTED_CODECS;i++)
	{
		if (pAudioSettings->codec[i]>-1)
		{
     // if (GetCodecName(pAudioSettings->codec[i]))
			{
				pCore->Codecs[AccNum][j]=(int)pAudioSettings->codec[i];
				pCore->CodecsSupported[AccNum][j]=(int)pAudioSettings->codec[i];
				j++;
			}
		}
	}
	pCore->Codecs[AccNum][j]=(int)101;
	pCore->CodecsSupported[AccNum][j]=(int)101;

	pCore->ptime[AccNum] = (int)(pAudioSettings->ptime); 
	pCore->VAD[AccNum] = (int)(pAudioSettings->VAD); 
	pCore->DtmfMode[AccNum] = (int)(pAudioSettings->DTMF_mode);
	pCore->tos[AccNum] = (int)(pAudioSettings->TOS );
	pCore->crypto_level[AccNum] = (int)(pAudioSettings->SRTP_mode );
	pCore->jitter [AccNum] = (int)(pAudioSettings->jitter);

  return 0;
}





 
