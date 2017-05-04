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
 * File:		  si_configfiles_api.h
 * Purpose:		
 * Created:		June 2008
 * By:			  YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_CONFIGFILES_API_H
#define SI_CONFIGFILES_API_H

/*========================== Include files ==================================*/
#include "si_config_file.h"
#include "operation_mode_defs.h"

/*========================== Local macro definitions & typedefs =============*/

#define ERROR_FileNotFound	    -1
#define ERROR_SyntaxError	    -2
#define ERROR_FileNotUpdated	 1

#define MAX_NUM_OF_RINGTONES	10
#ifdef USE_FLASH
#define RINGTONE_STORAGE_DIRECTORY "/ApplicationData/storage/sc14450_fs/ringfiles/"
#else
#define RINGTONE_STORAGE_DIRECTORY "/ApplicationData/storage/sc14450_fs/ringfiles/"
#endif
/*========================== Global variables ===============================*/


/*========================== Global function prototypes =====================*/

typedef enum _SIP_Account_files{
	FileSIP_ACCOUNT_1,
	FileSIP_ACCOUNT_2,
	FileSIP_ACCOUNT_3,
	FileSIP_ACCOUNT_4,
	FileSIP_ACCOUNT_5,
	FileSIP_ACCOUNT_6,
	FileSIP_ACCOUNT_7,
}SIP_Account_Files;

int LoadAllConfigFiles(ConfigSettings* pConfigFile);
int si_ua_readconfig(ConfigSettings* pConfigFile);

#if (!((defined ATA_ENABLED) || (defined DECT_ENABLED)))
  int sc_fileapi_LoadPhoneSettings(PhoneSettings *pPhoneSettings);
  int sc_fileapi_StorePhoneSettings(PhoneSettings *pPhoneSettings);
#else
  int sc_fileapi_StoreATASettings(ATA_Settings *pATA_Settings);
  int sc_fileapi_LoadATASettings(ATA_Settings *pATA_Settings);
  int sc_fileapi_LoadStarCodes(star_code_t *pStarCodes);
  int sc_fileapi_StoreStarCodes(star_code_t *pStarCodes);
#endif

#ifdef SA_CVM
#define si_ua_read_nonCriticalsettings( x )
#else
int si_ua_read_nonCriticalsettings(ConfigSettings* pConfigFile);
#endif

int sc_fileapi_LoadSubscribeSettings(SubscribeSettings *pSubscribeSettings);
int sc_fileapi_StoreSubscribeSettings(SubscribeSettings *pSubscribeSettings);
int Subscribe_SyncTo_Phbook();

#ifdef SA_CVM
#define sc_fileapi_LoadNetApplications( x )
#define sc_fileapi_StoreNetApplications( x )
#define sc_fileapi_LoadNetworkSettings( x )
#define sc_fileapi_LoadAudioSettings( x, y )
#define sc_fileapi_StoreAudioSettings( x, y )
#define sc_fileapi_LoadCallSettings( x, y )
#define sc_fileapi_StoreCallSettings( x, y )
#define sc_fileapi_LoadHardwareSettings( x )
#define sc_fileapi_StoreHardwareSettings( x )
#define sc_fileapi_StoreNetworkSettings( x )
#define sc_fileapi_LoadSIPSettings( x )
#define sc_fileapi_StoreSIPSettings( x )
#define sc_fileapi_LoadSIPAccount( x, y )
#define sc_fileapi_StoreSIPAccount( x, y )
#define sc_fileapi_StorePrintLogSetting( x )
#define sc_fileapi_LoadPrintLogSetting( x )
#else
int sc_fileapi_LoadNetApplications(NetApplications *pNetApplications);
int sc_fileapi_StoreNetApplications(NetApplications *pNetApplications);
int sc_fileapi_LoadNetworkSettings(NetworkSettings *pNetworkSettings);
int sc_fileapi_LoadAudioSettings(AudioSettings *pAudioSettings, int account);
int sc_fileapi_StoreAudioSettings(AudioSettings *pAudioSettings, int account);
int sc_fileapi_LoadCallSettings(CallSettings *pCallSettings, int account);
int sc_fileapi_StoreCallSettings(CallSettings *pCallSettings, int account);
int sc_fileapi_LoadHardwareSettings(HardwareSettings *pHardwareSettings);
int sc_fileapi_StoreHardwareSettings(HardwareSettings *pHardwareSettings);
int sc_fileapi_StoreNetworkSettings(NetworkSettings *pNetworkSettings);
int sc_fileapi_LoadSIPSettings(SIPSettings *pSIPSettings);
int sc_fileapi_StoreSIPSettings(SIPSettings *pSIPSettings);
int sc_fileapi_LoadSIPAccount(SIPAccount *pSIPAccount, int account);
int sc_fileapi_StoreSIPAccount(SIPAccount *pSIPAccount, int account);
int sc_fileapi_StorePrintLogSetting(PrintLogSettings *pPrintLogSettings);
int sc_fileapi_LoadPrintLogSetting(PrintLogSettings *pPrintLogSettings);
#endif // SA_CVM

int Ringtones_Load(char *pRingtone, int *num_of_ringtones_ptr);
int NetworkSettingsInfo_Load(NetworkSettingsInfo *pNetworkSettingsInfo);
int si_system_configuration_api(NetworkSettings *pNetConfig); 
int si_ua_writeNetworkSettingsInfo(ConfigSettings* pConfigFile);
int si_ua_SetNetworkSettingsInfo(unsigned char* IPAddress);
int sc_fileapi_StoreDialPlanSettings(DialPlan_settings *pDialPlan_settings);
int sc_fileapi_LoadDialPlanSettings(DialPlan_settings *pDialPlan_settings);

int extract_parameter_value(FILE* fp, char *parameter_name, char *parameter_value);

int getStrValue(FILE* fp, char *parameter_name, char *parameter_value);
int getCharValue(FILE* fp, char *parameter_name, char *parameter);
int getUCharValue(FILE* fp, char *parameter_name, unsigned char *parameter_value);
int getIntValue(FILE* fp, char *parameter_name, int *parameter);
int getUIntValue(FILE* fp, char *parameter_name, unsigned int *parameter_value);
int getShortValue(FILE* fp, char *parameter_name, short *parameter_value);
int getUShortValue(FILE* fp, char *parameter_name, unsigned short *parameter_value);

 
#endif //SI_CONFIGFILES_API_H
