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
 * File:		si_phone_api.h
 * Purpose:		
 * Created:		29/11/2007
 * By:			KF
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_PHONE_API_H
#define SI_PHONE_API_H

/*========================== Include files ==================================*/

/*========================== Local macro definitions & typedefs =============*/
////v1.0.1.1 : 20- June 2008 : release date 
////v2.0.2.1 : Dec 2008  (MCU, CCFSM)
#include "si_config_file.h"

#define UA_VERSION				      2.0.1.1
#define UA_SERVER_PORT					17170
#define MAX_SUPPORTED_CALLS_API			8

 // Registration Status Definitions
 
#define STATUS_REG_INIT			    0
#define STATUS_REG_STARTUP			1
#define STATUS_REG_IN_PROGRESS		2
#define STATUS_REG_SUCCESS			3
#define STATUS_REG_FAILED		    4
#define STATUS_REG_TERMINATED		5
#define STATUS_UNREG_IN_PROGRESS	6


typedef struct _incomingInfo
{
	 char displayname[64];
	 char username[64];
	 char host[64];
}incomingInfo_t;

typedef struct _CallBackParams {
 int  callbackID;
 int  vline;
 int  accountId;
 int reason;
 int codecType;
 char info[64];
 incomingInfo_t infoData;
} CallBackParams;
 
typedef struct _UserAgentStatus {
	int  isregistered[MAX_SUPPORTED_ACCOUNTS];
	unsigned char IPAddress[16];
	char CallStates[MAX_SUPPORTED_CALLS_API];
} UserAgentStatus;

#define REG_ALLOW_SUBSCRIBE 0x0001

 struct _siregistration {
	char regusername[64];
	char reglogin[64];
	char regpassword[16];
	int  useRegistration;
 	int  registrationID;
	int  transactionID;
	int  expiry;
	int  state;
	int  allow;
	int  authentication;
	int  retries;
};

typedef struct _siregistration siregistration_t;
#define MAX_ENCRYPTION_SUITES		4
#define MAX_CRYPTO_KEY_SIZE			32
#define MAX_CRYPTO_KEY_64_SIZE		64
#define MAX_CRYPTO_SUITE_SIZE		64

typedef struct _crypto_profile
{
	char			key[MAX_CRYPTO_KEY_64_SIZE];
	unsigned int	key_len;
	char			suite[MAX_CRYPTO_SUITE_SIZE];
	unsigned int	suite_len;
	unsigned int	flag;
	int				enabled;
}crypto_profile;
#define STANDARD_SIP_SERVER 0
#define BROADWORKS_SIP_SERVER 1

typedef struct _SICORE
{
  // critical variables
  int SipServerid;
  int Codecs[MAX_SUPPORTED_ACCOUNTS][16]; 
  int CodecsSupported[MAX_SUPPORTED_ACCOUNTS][16];
  int localSIPPort;
  char StunAddress[64];
  int StunPort;
  int NATMode;
  char RegistrarIPAddress[64];
  int SipServerPort;
  int remoteRegistrarPort;
  char regusername[MAX_SUPPORTED_ACCOUNTS][64];
  char reglogin[MAX_SUPPORTED_ACCOUNTS][64];
  char regpassword[MAX_SUPPORTED_ACCOUNTS][16];
  int  useRegistration[MAX_SUPPORTED_ACCOUNTS];
  char regrealm [64];
  int VAD[MAX_SUPPORTED_ACCOUNTS];
  int jitter[MAX_SUPPORTED_ACCOUNTS];
  int transport;   
  int SessionTimerEnable;
  int SessionExpireTimer;
  int MinSessionExpireTimer;
  int Naptr;
  int Prack;
  int Update;
  int Presence;
  int localAudioRtpPort;	
 // non critical variables
  int busyFlag[MAX_SUPPORTED_ACCOUNTS];
  int autoanswer[MAX_SUPPORTED_ACCOUNTS];
  int callwaiting[MAX_SUPPORTED_ACCOUNTS];
  int hideid[MAX_SUPPORTED_ACCOUNTS];
  int rejectanonymous[MAX_SUPPORTED_ACCOUNTS];
  int redirectFlag[MAX_SUPPORTED_ACCOUNTS];
  char redirectURI[MAX_SUPPORTED_ACCOUNTS][64];
  char redirectReason[MAX_SUPPORTED_ACCOUNTS][16];
  int redirectTime[MAX_SUPPORTED_ACCOUNTS] ;
  int DtmfMode[MAX_SUPPORTED_ACCOUNTS];
  int tos[MAX_SUPPORTED_ACCOUNTS];
  char ringToneSelection[12];   
  int keypadToneSelection;

  // crypro settings
  int crypto_level[MAX_SUPPORTED_ACCOUNTS];
  crypto_profile crypto_suites[MAX_SUPPORTED_ACCOUNTS][MAX_ENCRYPTION_SUITES]; 
 // runtime variables
  int session_version; 

  int IPv6;
  unsigned char IPAddress[64];
  char Gateway[64];
  char firewallIP[64];
  int regExpTime;
  int sipStatus;
  int ptime[MAX_SUPPORTED_ACCOUNTS];
  siregistration_t siregistration[MAX_SUPPORTED_ACCOUNTS];
}SICORE;
/*========================== Global variables ===============================*/

/*========================== Global function prototypes =====================*/

#endif /* SI_PHONE_API_H */

