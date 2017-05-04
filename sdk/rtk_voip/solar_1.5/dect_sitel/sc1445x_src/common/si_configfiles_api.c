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
 * (C) Copyright SiTel Semiconductor BV, unpublished work

 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:    si_mmi_configfiles.c
 * Purpose:		 		 
 * Created: June 2008
 * By: 	 	  EF
 * Country:	Greece
 *
 *-----------------------------------------------------------------------------
 */
/*========================== Include files ==================================*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
/* atoi */
#include <stdlib.h> 

#include <si_print_api.h>
 
#include "si_configfiles_api.h"
#include "si_call_log_api.h"
#if (!((defined ATA_ENABLED) || (defined DECT_ENABLED)))
	#include "si_phbook_api.h"
#endif

/*========================== Local function prototypes ======================*/


/*========================== Local data definitions =========================*/

#ifndef RECALL_ORG
#undef USE_FLASH
#endif

//#define DEGUG_PRINT_CONFIGURATIONFILE 1

char m_Ringtone[MAX_NUM_OF_RINGTONES][12];
int num_of_ringtones;

#ifdef USE_FLASH
char FileSYSLOG_SETTINGS[] = "/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/print_log_settings.cfg";  
char FileSIP_SETTINGS[] = "/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/SIP_settings.cfg";
char FileNETWORK_SETTINGS[] = "/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/network_settings.cfg";
char FileNET_APPLICATIONS[] = "/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/net_applications.cfg";
char FileHARDWARE_SETTINGS[] = "/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/hardware_settings.cfg";
char FileNETWORK_SETTINGS_INFO[] = "/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/network_settings_info.cfg";
char FilePHONE_SETTINGS[] = "/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/phone_settings.cfg";
char FileSUBSCRIBE_SETTINGS[] = "/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/subscribe_settings.cfg";
char FileDIALPLAN_SETTINGS[] = "/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/dial_plans.cfg";

char FileCALL_SETTINGS[7][256] = {
	"/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/call_settings_1.cfg",
	"/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/call_settings_2.cfg",
	"/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/call_settings_3.cfg",
	"/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/call_settings_4.cfg",
	"/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/call_settings_5.cfg",
	"/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/call_settings_6.cfg",
	"/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/call_settings_7.cfg"
};

char FileAUDIO_SETTINGS[7][256] = {
	"/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/audio_settings_1.cfg",
	"/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/audio_settings_2.cfg",
	"/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/audio_settings_3.cfg",
	"/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/audio_settings_4.cfg",
	"/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/audio_settings_5.cfg",
	"/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/audio_settings_6.cfg",
	"/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/audio_settings_7.cfg"
};

char FileSIP_ACCOUNTS[7][256] = {
	"/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/SIP_account_1.cfg",
  "/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/SIP_account_2.cfg",
  "/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/SIP_account_3.cfg",
	"/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/SIP_account_4.cfg",
	"/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/SIP_account_5.cfg",
	"/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/SIP_account_6.cfg",
	"/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/SIP_account_7.cfg",
};

char FileNETWORK_SETTINGS_INFO_mmi[] = "/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/network_settings_info.cfg";
char FileRINGTONES_LIST[] = "/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/ringtones_list.cfg";
char FileSYSTEM_NET_CONFIG[] = "/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/net_config";
char FileINTERNET_RADIO_SETTINGS[] = "/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/default.pls";

#else
char FileSYSLOG_SETTINGS[] = "/ApplicationData/storage/sc14450_fs/configfile/print_log_settings.cfg";
char FileSIP_SETTINGS[] = "/ApplicationData/storage/sc14450_fs/configfile/SIP_settings.cfg";
char FileNETWORK_SETTINGS[] = "/ApplicationData/storage/sc14450_fs/configfile/network_settings.cfg";
char FileHARDWARE_SETTINGS[] = "/ApplicationData/storage/sc14450_fs/configfile/hardware_settings.cfg";
char FileNET_APPLICATIONS[] = "/ApplicationData/storage/sc14450_fs/configfile/net_applications.cfg";
char FileNETWORK_SETTINGS_INFO[] = "/ApplicationData/storage/sc14450_fs/configfile/network_settings_info.cfg";
char FilePHONE_SETTINGS[] = "/ApplicationData/storage/sc14450_fs/configfile/phone_settings.cfg";
char FileSUBSCRIBE_SETTINGS[] = "/ApplicationData/storage/sc14450_fs/configfile/subscribe_settings.cfg";
char FileDIALPLAN_SETTINGS[] = "/ApplicationData/storage/sc14450_fs/configfile/dial_plans.cfg";

const char * const FileCALL_SETTINGS[] = {
	"ApplicationData/storage/sc14450_fs/configfile/call_settings_1.cfg",
	"ApplicationData/storage/sc14450_fs/configfile/call_settings_2.cfg",
	"ApplicationData/storage/sc14450_fs/configfile/call_settings_3.cfg",
	"ApplicationData/storage/sc14450_fs/configfile/call_settings_4.cfg",
	"ApplicationData/storage/sc14450_fs/configfile/call_settings_5.cfg",
	"ApplicationData/storage/sc14450_fs/configfile/call_settings_6.cfg",
	"ApplicationData/storage/sc14450_fs/configfile/call_settings_7.cfg"
};

const char * const FileAUDIO_SETTINGS[] = {
	"/ApplicationData/storage/sc14450_fs/configfile/audio_settings_1.cfg",
	"/ApplicationData/storage/sc14450_fs/configfile/audio_settings_2.cfg",
	"/ApplicationData/storage/sc14450_fs/configfile/audio_settings_3.cfg",
	"/ApplicationData/storage/sc14450_fs/configfile/audio_settings_4.cfg",
	"/ApplicationData/storage/sc14450_fs/configfile/audio_settings_5.cfg",
	"/ApplicationData/storage/sc14450_fs/configfile/audio_settings_6.cfg",
	"/ApplicationData/storage/sc14450_fs/configfile/audio_settings_7.cfg"
};

const char * const FileSIP_ACCOUNTS[] = {
	"ApplicationData/storage/sc14450_fs/configfile/SIP_account_1.cfg",
  "ApplicationData/storage/sc14450_fs/configfile/SIP_account_2.cfg",
  "ApplicationData/storage/sc14450_fs/configfile/SIP_account_3.cfg",
	"ApplicationData/storage/sc14450_fs/configfile/SIP_account_4.cfg",
	"ApplicationData/storage/sc14450_fs/configfile/SIP_account_5.cfg",
	"ApplicationData/storage/sc14450_fs/configfile/SIP_account_6.cfg",
	"ApplicationData/storage/sc14450_fs/configfile/SIP_account_7.cfg",
};
char FileNETWORK_SETTINGS_INFO_mmi[] = "/ApplicationData/storage/sc14450_fs/configfile/network_settings_info.cfg";
char FileRINGTONES_LIST[] = "/ApplicationData/storage/sc14450_fs/configfile/ringtones_list.cfg";
char FileSYSTEM_NET_CONFIG[] = "/ApplicationData/storage/sc14450_fs/configfile/net_config";
char FileINTERNET_RADIO_SETTINGS[] = "/ApplicationData/storage/sc14450_fs/configfile/default.pls";

#endif

#if ((defined ATA_ENABLED) || (defined DECT_ENABLED))
#ifdef USE_FLASH
char FileSTAR_CODES[] = "/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/star_codes.cfg";
#else
char FileSTAR_CODES[] = "/ApplicationData/storage/sc14450_fs/configfile/star_codes.cfg";
#endif

char *StarCodeName[] = {
	"add_to_conf", 
	"blind_transfer", 
	"callerID_block", 
	"callerID_send", 
	"call_waiting_off", 
	"call_waiting_on", 
	"call_forward_uncond_on", 
	"call_forward_uncond_off", 
	"call_forward_busy_on", 
	"call_forward_busy_off", 
	"call_forward_delayed_on", 
	"call_forward_delayed_off", 
	"internal_call", 
	"unpark_call", 
	"park_call",
	"dnd_off",
	"dnd_on"
};

#endif

#ifdef ATA_ENABLED
#ifdef USE_FLASH
char FileATA_SETTINGS[] = "/mnt/flash/ApplicationData/storage/sc14450_fs/configfile/ata_settings.cfg";
#else
char FileATA_SETTINGS[] = "/ApplicationData/storage/sc14450_fs/configfile/ata_settings.cfg";
#endif

/*
	char *StarCodeLocal[] = {"AddToConferenceLocal", "BlindTransferLocal", 
	"BlockCallerIDLocal", "SendCallerIDLocal", "DisableCallWaitingLocal", 
	"EnableCallWaitingLocal", "EnableUnconditionalCallForwardLocal", 
	"DisableUnconditionalCallForwardLocal", "EnableBusyCallForwardLocal", 
	"DisableBusyCallForwardLocal", "EnableDelayedCallForwardLocal", 
	"DisableDelayedCallForwardLocal", "InternalCallLocal", "UnparkLocal", 
	"parkLocal"};
	*/
#endif

/*========================== Function definitions ===========================*/

int LoadAllConfigFiles(ConfigSettings* pConfigFile)
{
	int ret;

	ret = si_ua_readconfig(pConfigFile);
#if 0
	if (ret<0) {
		si_print(PRINT_LEVEL_CRIT, "\n FATAL ERROR trying to open files %d \n ", ret);
		return -1;
	} 
#else
	if (ret!=0) {
		printf("\n FATAL ERROR trying to open files %X \n ", ret);
		return -1;
	} 
#endif

	return 0;
}

int si_ua_read_nonCriticalsettings(ConfigSettings* pConfigFile)
{
	int i;
 	sc_fileapi_LoadSIPSettings(&pConfigFile->m_SIPSettings);

 	for (i=0;i<MAX_SUPPORTED_ACCOUNTS;i++){
 		if (sc_fileapi_LoadAudioSettings(&pConfigFile->m_AudioSettings[i], i+1)) return -1;
		if (sc_fileapi_LoadCallSettings(&pConfigFile->m_CallSettings[i], i+1)) return -2;
		if (sc_fileapi_LoadSIPAccount(&pConfigFile->m_SIPAccount[i], i+1)) return -3;
	}
	return 0;
}

int si_ua_readconfig(ConfigSettings* pConfigFile) 
{
	int ret = 0;
	int i = 0;
 	for (i=0;i<MAX_SUPPORTED_ACCOUNTS;i++){
 		if (sc_fileapi_LoadAudioSettings(&pConfigFile->m_AudioSettings[i], i+1)) ret |= 1 << 1; //return -1;
		if (sc_fileapi_LoadCallSettings(&pConfigFile->m_CallSettings[i], i+1)) ret |= 1 << 2; //return -2;
		if (sc_fileapi_LoadSIPAccount(&pConfigFile->m_SIPAccount[i], i+1)) ret |= 1 << 3; //return -3;
	}

	if (0>sc_fileapi_LoadSIPSettings(&pConfigFile->m_SIPSettings)) ret |= 1 << 4; //return -4;

  if (0>sc_fileapi_LoadNetworkSettings(&pConfigFile->m_NetworkSettings)) ret |= 1 << 5; //return -5;

 	if (0>sc_fileapi_LoadHardwareSettings(&pConfigFile->m_HardwareSettings)) ret |= 1 << 6; //return -6;

	if (0>Ringtones_Load(&m_Ringtone[0][0], &num_of_ringtones)) ret |= 1 << 7; //return -7;

	if (0>NetworkSettingsInfo_Load(&pConfigFile->m_NetworkSettings_info)) ret |= 1 << 8; //return -8;

	if (0>sc_fileapi_LoadPrintLogSetting(&pConfigFile->m_PrintLogSettings)) ret |= 1 << 9; //return -9;
 
  if (0>si_system_configuration_api(&pConfigFile->m_NetworkSettings)) ret |= 1 << 10; //return -10;

#if (!((defined ATA_ENABLED) || (defined DECT_ENABLED)))
  if (0>sc_fileapi_LoadPhoneSettings(&pConfigFile->m_PhoneSettings)) ret |= 1 << 11; //return -11;

	if (phbookLoadContacts()) ret |= 1 << 12; //return -12;			

  if (calllog_LoadMissedCalls()) ret |= 1 << 13; //return -13;

  if (calllog_LoadReceivedCalls()) ret |= 1 << 14; //return -14;

  if (calllog_LoadDialedCalls()) ret |= 1 << 15; //return -15;

	if (0>sc_fileapi_LoadSubscribeSettings(&pConfigFile->m_SubscribeSettings)) ret |= 1 << 16; //return -16;

	if (0>Subscribe_SyncTo_Phbook()) ret |= 1 << 17; //return -17;

	if (0>sc_fileapi_LoadDialPlanSettings(&pConfigFile->m_DialPlan_settings)) ret |= 1 << 18; //return -18;

#elif (defined ATA_ENABLED)
  if (sc_fileapi_LoadATASettings(&pConfigFile->m_ATA_Settings)) ret |= 1 << 19; //return -19;
#elif ((defined ATA_ENABLED) || (defined DECT_ENABLED))
  if (sc_fileapi_LoadStarCodes (pConfigFile->StarCode)) ret |= 1 << 20; //return -20;
#endif

  if (sc_fileapi_LoadNetApplications(&pConfigFile->m_NetApplications)) ret |= 1 << 21; //return -21;

  return ret;
}

int Ringtones_Load(char *pRingtone, int *num_of_ringtones_ptr)
{
	FILE *fp;
	int ret, i;

	fp = fopen(FileRINGTONES_LIST, "rb");
  if(fp == NULL)
		return -1;

	strcpy(pRingtone, "system");

	for(i = 1 ; i < MAX_NUM_OF_RINGTONES ; i++) {
		ret = fread(pRingtone + 12 * i, 12, 1, fp);
		if(ret <= 0)
			break;
	}

	*num_of_ringtones_ptr = i;
  
	fclose(fp);

	return 0;
}

int NetworkSettingsInfo_Load(NetworkSettingsInfo *pNetworkSettingsInfo)
{
	FILE *fp;
	int MajorVersion,MinorVersion;

	fp = fopen(FileNETWORK_SETTINGS_INFO, "rb");
  	if(fp == NULL){	
		si_print(PRINT_LEVEL_CRIT, "\n failed to open file : %s\n", FileNETWORK_SETTINGS_INFO);
		return -1;
	}
  
	fread(&MajorVersion, sizeof(int), 1, fp);
	fread(&MinorVersion, sizeof(int), 1, fp);
	fread(pNetworkSettingsInfo, sizeof(NetworkSettingsInfo), 1, fp);

	fclose(fp);
  
	return 0;
}

int si_ua_writeNetworkSettingsInfo(ConfigSettings* pConfigFile) 
{
 	FILE *fp;
	int MajorVersion,MinorVersion;
 	fp = fopen(FileNETWORK_SETTINGS_INFO, "wb");
	if(fp == NULL){	
		si_print(PRINT_LEVEL_CRIT, "\n failed to open file : %s\n", FileNETWORK_SETTINGS_INFO);
		return -1;
	}
 	 	fwrite(&MajorVersion, sizeof(int), 1, fp);
		fwrite(&MinorVersion, sizeof(int), 1, fp);
		fwrite(&pConfigFile->m_NetworkSettings_info, sizeof(NetworkSettingsInfo), 1, fp);
		fclose(fp);
	return 0;
}

int si_ua_SetNetworkSettingsInfo(unsigned char* IPAddress)
{
  ConfigSettings p_Config_Settings;

 	strcpy((char*)p_Config_Settings.m_NetworkSettings_info.IP_address_info,(char*)IPAddress );
 	p_Config_Settings.m_NetworkSettings_info.DHCP_info = 	(char) p_Config_Settings.m_NetworkSettings.DHCP;
 	si_ua_writeNetworkSettingsInfo(&p_Config_Settings);
 	return 0;
}

int sc_fileapi_LoadPrintLogSetting(PrintLogSettings *pPrintLogSettings)
{
	FILE *fp;
	char update_flag = 0;

	fp = fopen(FileSYSLOG_SETTINGS, "r");
	if(fp == NULL) {
		printf( "open %s fail\n", FileSYSLOG_SETTINGS );
		return ERROR_FileNotFound;
	}

 	update_flag = getCharValue(fp, "print_enable", &pPrintLogSettings->print_enable);
	if (update_flag<0) {fclose(fp);return update_flag;} 

	update_flag = getCharValue(fp, "print_level", &pPrintLogSettings->print_level); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	update_flag = getCharValue(fp, "print_level_option", &pPrintLogSettings->print_level_option); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	update_flag = getStrValue(fp, "print_to", pPrintLogSettings->print_to); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	update_flag = getStrValue(fp, "syslog_server_ip", pPrintLogSettings->syslog_server_ip);
	if (update_flag<0) {fclose(fp);return update_flag;} 

	update_flag = getStrValue(fp, "print_file_path", pPrintLogSettings->print_file_path);
	if (update_flag<0) {fclose(fp);return update_flag;} 
  
	fclose(fp);
  if(update_flag == 1)
		return ERROR_FileNotUpdated;
	else
		return 0;
}

int sc_fileapi_StorePrintLogSetting(PrintLogSettings *pPrintLogSettings)
{
	FILE *fp;

	fp = fopen(FileSYSLOG_SETTINGS, "w");
	if(fp == NULL)
		return ERROR_FileNotFound;

	fprintf(fp, "print_enable=\"%d\"\n", pPrintLogSettings->print_enable);
	fprintf(fp, "print_level=\"%d\"\n", pPrintLogSettings->print_level);
	fprintf(fp, "print_level_option=\"%d\"\n", pPrintLogSettings->print_level_option);
 	fprintf(fp, "print_to=\"%s\"\n", pPrintLogSettings->print_to);
	fprintf(fp, "syslog_server_ip=\"%s\"\n", pPrintLogSettings->syslog_server_ip);
	fprintf(fp, "print_file_path=\"%s\"\n", pPrintLogSettings->print_file_path);

	fclose(fp);

	return 0;
}


#ifdef ATA_ENABLED
int sc_fileapi_LoadATASettings(ATA_Settings *pATA_Settings)
{
  FILE *fp;
	char update_flag = 0;

	fp = fopen(FileATA_SETTINGS, "r");
	if(fp == NULL)
  	return ERROR_FileNotFound;

  //bell_style
  update_flag = getCharValue(fp, "bell_style", &pATA_Settings->bell_style); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
	//fax_mode
  update_flag = getCharValue(fp, "fax_mode", &pATA_Settings->fax_mode); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	//dial_plan
  update_flag = getStrValue(fp, "dial_plan", pATA_Settings->dial_plan); 
	if (update_flag == 0) {
		if(!(IsDialPlanValid(pATA_Settings->dial_plan)))
			pATA_Settings->dial_plan[0] = '\0';
	}
	else{
		fclose(fp);return update_flag;
	}

	fclose(fp);
  if(update_flag == 1)
	  return ERROR_FileNotUpdated;
	else
		return 0;
}

int sc_fileapi_StoreATASettings(ATA_Settings *pATA_Settings)
{ 
	FILE *fp;

	fp = fopen(FileATA_SETTINGS, "w");
	
	if(fp == NULL)
 	return ERROR_FileNotFound;

  if((pATA_Settings->bell_style==1)||(pATA_Settings->bell_style==0))
	  fprintf(fp, "bell_style=\"%c\"\n", pATA_Settings->bell_style);
  else fprintf(fp, "bell_style=\"%c\"\n", 1);

	if((pATA_Settings->fax_mode==PASS_THROUGH)||(pATA_Settings->fax_mode==AUTO_DETECT))
	  fprintf(fp, "fax_mode=\"%c\"\n", pATA_Settings->fax_mode);
  else fprintf(fp, "fax_mode=\"%c\"\n", 1);

	if(!(IsDialPlanValid(pATA_Settings->dial_plan))) 
		si_print(PRINT_LEVEL_ERR, "[%s] Invalid Dial Plan", __FUNCTION__);
	else
		fprintf(fp, "dial_plan=\"%s\"\n", pATA_Settings->dial_plan);
	
	fclose(fp);

	return 0;
}

#endif

int IsDialPlanValid(char *dial_plan)
{
	int i;
	int bracket_open = 0, square_bracket_open = 0;

	if(strlen(dial_plan) >= 200)
		return 0;

	for(i = 0 ; dial_plan[i] != 0 ; i++) {
		if(!isdigit(dial_plan[i]) && (dial_plan[i] != 't') && (dial_plan[i] != 'T') && (dial_plan[i] != '*') && (dial_plan[i] != 'x') && (dial_plan[i] != 'X') && (dial_plan[i] != '.') && (dial_plan[i] != '[') && (dial_plan[i] != ']') && (dial_plan[i] != '(') && (dial_plan[i] != ')') && (dial_plan[i] != '-') && (dial_plan[i] != '|'))
			return 0;

		if(dial_plan[i] == '(') {
			if(i != 0)
				return 0;
			else
				bracket_open = 1;
		}

		if(dial_plan[i] == ')') {
			if((dial_plan[i + 1] != 0) || (bracket_open == 0))
				return 0;
			else
				bracket_open = 0;
		}

		if(dial_plan[i] == '[') {
			if(square_bracket_open == 1)
				return 0;
			else
				square_bracket_open = 1;
		}

		if(dial_plan[i] == ']') {
			if(square_bracket_open == 0)
				return 0;
			else
				square_bracket_open = 0;
		}

		if(dial_plan[i + 1] == 0) {
			if((bracket_open == 1) || (square_bracket_open == 1))
				return 0;
		}

		if(dial_plan[i] == '|') {
			if((dial_plan[i + 1] == 0) || (square_bracket_open == 1) || (dial_plan[i + 1] == ')'))
				return 0;
		}

		if((dial_plan[i] == 't') || (dial_plan[i] == 'T')) {
			if((dial_plan[i + 1] != '|') && (dial_plan[i + 1] != ')') && (dial_plan[i + 1] != 0))
				return 0;
		}

		if(dial_plan[i] == '-') {
			if((square_bracket_open == 0) || !isdigit(dial_plan[i - 1]) || !isdigit(dial_plan[i + 1]))
				return 0;
		}
	}
	return 1;
}

#if ((defined DECT_ENABLED) || (defined ATA_ENABLED))
int sc_fileapi_LoadStarCodes(star_code_t *pStarCodes)
{
	FILE *fp;
	char parameter_value[256];
	char parameter_name[30];
	char update_flag = 0;
	int index=0;
	int ret, i, j;
	char temp;

	fp = fopen(FileSTAR_CODES, "r");
  	if(fp == NULL)
    	return ERROR_FileNotFound;

	for(i = 0 ; i < STAR_CODES_NUMBER ; i++) {
		sprintf(parameter_name, "name.%d", i);
		rewind(fp);
		ret = extract_parameter_value(fp, parameter_name, parameter_value);
		if(ret == 0) {
			for(j = 0 ; j < STAR_CODES_NUMBER ; j++) {
				if(!strcmp(parameter_value, StarCodeName[j])) {
					index = j;
					strcpy(pStarCodes[index].name, parameter_value);
					#ifdef DEGUG_PRINT_CONFIGURATIONFILE
						si_print(PRINT_LEVEL_INFO, "Configuration parameter: %s \n ",pStarCodes[index].name);	
					#endif
 					break;
				}
			}
 		}else if(ret == ERROR_SyntaxError) {
			fclose(fp);
 			return ret;
		}else if(ret == ERROR_FileNotUpdated)
			update_flag = 1;

		sprintf(parameter_name, "local.%d", i);
		rewind(fp);

		ret = extract_parameter_value(fp, parameter_name, parameter_value);
		if(ret == 0) {
			temp = (char)atoi(parameter_value);

      if((temp == 0) || (temp == 1)){
				pStarCodes[index].local = temp;
#ifdef DEGUG_PRINT_CONFIGURATIONFILE
        si_print(PRINT_LEVEL_INFO, "Configuration parameter: %s %d \n ", parameter_name, temp);
#endif
      }else{
				fclose(fp);
  				return ERROR_SyntaxError;
			}
		}else if(ret == ERROR_SyntaxError) {
			fclose(fp);
			return ret;
		}else if(ret == ERROR_FileNotUpdated)
			update_flag = 1;

		sprintf(parameter_name, "code.%d", i);
		rewind(fp);

		ret = extract_parameter_value(fp, parameter_name, parameter_value);
		if(ret == 0) {
			if(strlen(parameter_value) != 2) {
				fclose(fp);
				return ERROR_SyntaxError;
			}

			if((parameter_value[0] < '0') || (parameter_value[0] > '9') || (parameter_value[1] < '0') || (parameter_value[1] > '9')) {
				fclose(fp);
  				return ERROR_SyntaxError;
			}
			strcpy(pStarCodes[index].code, parameter_value);
#ifdef DEGUG_PRINT_CONFIGURATIONFILE
      si_print(PRINT_LEVEL_INFO, "Configuration parameter: %s %s \n ", parameter_name, pStarCodes[index].code);
#endif
		}else if(ret == ERROR_SyntaxError) {
			fclose(fp);
 			return ret;
		}else if(ret == ERROR_FileNotUpdated)
			update_flag = 1;
	}

	fclose(fp);
  	
	if(update_flag == 1){
  		return ERROR_FileNotUpdated;
	}
	else{
  		return 0;
	}
}

int sc_fileapi_StoreStarCodes(star_code_t *pStarCodes)
{
	FILE *fp;
	int i;

	fp = fopen(FileSTAR_CODES, "w");
	if(fp == NULL)
 	return ERROR_FileNotFound;
	
	for(i = 0 ; i < STAR_CODES_NUMBER ; i++) {
		fprintf(fp, "name.%d=\"%s\"\n", i, pStarCodes[i].name);
		fprintf(fp, "local.%d=%d\n", i, pStarCodes[i].local);
		fprintf(fp, "code.%d=\"%s\"\n", i, pStarCodes[i].code);
	}

	fclose(fp);

	return 0;
}
#endif

int sc_fileapi_LoadAudioSettings(AudioSettings *pAudioSettings, int account)
{
	FILE *fp;
	char parameter_name[30];
	char update_flag = 0;
	int i;

	fp = fopen(FileAUDIO_SETTINGS[account-1], "r");

	if(fp == NULL) {
	 	return ERROR_FileNotFound;
	}

 	// Load Codec Parameters
 	for(i = 0 ; i < MAX_SUPPORTED_CODECS ; i++) {
		sprintf(parameter_name, "codec.%d", i);
		update_flag = getIntValue(fp, parameter_name,  &pAudioSettings->codec[i]); 
		if (update_flag<0) {fclose(fp);return update_flag;} 
	}

	//Load PTIME Parameter
	update_flag = getIntValue(fp, "ptime",  &pAudioSettings->ptime); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	update_flag = getIntValue(fp, "jitter",  &pAudioSettings->jitter); 
 	if (update_flag<0) {fclose(fp);return update_flag;} 
  
	// Load VAD Parameter
	update_flag = getIntValue(fp, "VAD",  &pAudioSettings->VAD); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
  
	// Load DTMF mode Parameter
	update_flag = getIntValue(fp, "DTMF_mode",  &pAudioSettings->DTMF_mode); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
  
	// Load SRTP mode Parameter
	update_flag = getIntValue(fp, "SRTP_mode",  &pAudioSettings->SRTP_mode); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

  // Load TOS mode Parameter
	update_flag = getIntValue(fp, "TOS",  &pAudioSettings->TOS); 
 	if (update_flag<0) {fclose(fp);return update_flag;} 

	fclose(fp);
	return 0;

 	if(update_flag == 1) return ERROR_FileNotUpdated;
	else return 0;
}

int sc_fileapi_StoreAudioSettings(AudioSettings *pAudioSettings, int account)
{
	FILE *fp;
	int i;

	fp = fopen(FileAUDIO_SETTINGS[account-1], "w");
   
	if(fp == NULL)
   	return ERROR_FileNotFound;

 	for(i = 0 ; i < MAX_SUPPORTED_CODECS ; i++)
		fprintf(fp, "codec.%d=%d\n", i, pAudioSettings->codec[i]);

	fprintf(fp, "ptime=%d\n", pAudioSettings->ptime);
	fprintf(fp, "jitter=%d\n", pAudioSettings->jitter);
	fprintf(fp, "VAD=%d\n", pAudioSettings->VAD);
	fprintf(fp, "DTMF_mode=%d\n", pAudioSettings->DTMF_mode);
	fprintf(fp, "SRTP_mode=%d\n", pAudioSettings->SRTP_mode);
	fprintf(fp, "TOS=%d\n", pAudioSettings->TOS);

	fclose(fp);
 	return 0;
}

/////////// CALLL SETTINGS 
int sc_fileapi_LoadCallSettings(CallSettings *pCallSettings, int account)
{
	FILE *fp;
	char update_flag = 0;
	
	fp = fopen(FileCALL_SETTINGS[account-1], "r");

  if(fp == NULL)
    return ERROR_FileNotFound;

	// Call Forward Number
	update_flag = getStrValue(fp, "call_forward_number", pCallSettings->call_forward_number); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	// Call Forward Mode
	update_flag = getCharValue(fp, "call_forward_mode", &pCallSettings->call_forward_mode); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
  
	// Call Foward Start Time
	update_flag = getCharValue(fp, "call_forward_start_time", &pCallSettings->call_forward_start_time); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
  
	// Call Waiting
	update_flag = getCharValue(fp, "call_waiting", &pCallSettings->call_waiting); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
  
	//DND
	update_flag = getCharValue(fp, "DND", &pCallSettings->DND); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

 	// AutoAnswer
	update_flag = getCharValue(fp, "auto_answer", &pCallSettings->auto_answer); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 	
	// Hide ID
 	update_flag= getCharValue(fp, "hide_id", &pCallSettings->hide_id);
	if (update_flag<0){fclose(fp); return update_flag;}

	// Reject Anonyous
 	update_flag= getCharValue(fp, "reject_anonymous", &pCallSettings->reject_anonymous);
 	if (update_flag<0){fclose(fp); return update_flag;}
 
 
	fclose(fp);
  	if(update_flag == 1)
		return ERROR_FileNotUpdated;
	else
		return 0;
}

int sc_fileapi_StoreCallSettings(CallSettings *pCallSettings, int account)
{
	FILE *fp;

	fp = fopen(FileCALL_SETTINGS[account-1], "w");

  if(fp == NULL)
		return ERROR_FileNotFound;

	fprintf(fp, "call_forward_number=\"%s\"\n", pCallSettings->call_forward_number);
	fprintf(fp, "call_forward_mode=%d\n", pCallSettings->call_forward_mode);
	fprintf(fp, "call_forward_start_time=%d\n", pCallSettings->call_forward_start_time);
	fprintf(fp, "call_waiting=%d\n", pCallSettings->call_waiting);
	fprintf(fp, "DND=%d\n", pCallSettings->DND);
	fprintf(fp, "auto_answer=%d\n", pCallSettings->auto_answer);
	fprintf(fp, "hide_id=%d\n", pCallSettings->hide_id);
	fprintf(fp, "reject_anonymous=%d\n", pCallSettings->reject_anonymous);

	fclose(fp);
 	return 0;
}

int sc_fileapi_LoadHardwareSettings(HardwareSettings *pHardwareSettings)
{
	FILE *fp;
	char parameter_name[30];
	char update_flag = 0;
	int i;

	fp = fopen(FileHARDWARE_SETTINGS, "r");
  	if(fp == NULL)
    	return ERROR_FileNotFound;
	// MAC Address
	for(i = 0 ; i < 6 ; i++) {
		sprintf(parameter_name, "MAC_address_1.%d", i);
 		update_flag = getUCharValue(fp, parameter_name, &pHardwareSettings->MAC_address_1[i]); 
		if (update_flag<0) {fclose(fp);return update_flag;} 
  	}
	for(i = 0 ; i < 6 ; i++) {
		sprintf(parameter_name, "MAC_address_2.%d", i);
 		update_flag = getUCharValue(fp, parameter_name, &pHardwareSettings->MAC_address_2[i]); 
		if (update_flag<0) {fclose(fp);return update_flag;} 
  	}
	// Product Model
	update_flag = getStrValue(fp, "product_model", pHardwareSettings->product_model); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	// User name
 	update_flag = getStrValue(fp, "user_name", pHardwareSettings->user_name); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	// password 
 	update_flag = getStrValue(fp, "user_password", pHardwareSettings->user_password); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

 
	fclose(fp);
 	if(update_flag == 1)
		return ERROR_FileNotUpdated;
	else
		return 0;
}
 
int sc_fileapi_StoreHardwareSettings(HardwareSettings *pHardwareSettings)
{
	FILE *fp;
	int i;

	fp = fopen(FileHARDWARE_SETTINGS, "w");
  	if(fp == NULL)
   	return ERROR_FileNotFound;

 	for(i = 0 ; i < 6 ; i++)
		fprintf(fp, "MAC_address_1.%d=%d\n", i, pHardwareSettings->MAC_address_1[i]);

	for(i = 0 ; i < 6 ; i++)
		fprintf(fp, "MAC_address_2.%d=%d\n", i, pHardwareSettings->MAC_address_2[i]);

	fprintf(fp, "product_model=\"%s\"\n", pHardwareSettings->product_model);
	fprintf(fp, "user_name=\"%s\"\n", pHardwareSettings->user_name);
	fprintf(fp, "user_password=\"%s\"\n", pHardwareSettings->user_password);

	fclose(fp);

	return 0;
}

int sc_fileapi_LoadInternetRadioSettings(InternetRadioSettings *pInternetRadioSettings)
{
	FILE *fp;
	char parameter_name[16];
	char update_flag = 0;
	int i = 0;
	
	fp = fopen(FileINTERNET_RADIO_SETTINGS, "r");
  if(fp == NULL)
    return ERROR_FileNotFound;

	// Load Net Radio Parameters
 	for(i = 0 ; i < MAX_NUM_OF_STATIONS ; i++) {
		sprintf(parameter_name, "Title%d", i + 1);
		update_flag = getStrValue(fp, parameter_name, pInternetRadioSettings[i].station_name); 
		if (update_flag<0) {fclose(fp);return update_flag;} 

		sprintf(parameter_name, "File%d", i + 1);
		update_flag = getStrValue(fp, parameter_name, pInternetRadioSettings[i].URL); 
		if (update_flag<0) {fclose(fp);return update_flag;} 
	}

	fclose(fp);
  	
	return 0;
}


int InternetRadioSettings_Store(InternetRadioSettings *pInternetRadioSettings, int num_of_records)
{
	FILE *fp;
	int i;

	fp = fopen(FileINTERNET_RADIO_SETTINGS, "w");
  if(fp == NULL)
		return ERROR_FileNotFound;

	fputs("[playlist]\n", fp);
	fprintf(fp, "NumberOfEntries=%d\n", num_of_records);

	for(i = 0 ; i < num_of_records ; i++) {
		fprintf(fp, "File%d=%s\n", i + 1, pInternetRadioSettings[i].URL);
		fprintf(fp, "Title%d=%s\n", i + 1, pInternetRadioSettings[i].station_name);
		fprintf(fp, "Length%d=-1\n", i + 1);
	}

	fclose(fp);

	return 0;
}


int si_system_configuration_api(NetworkSettings *pNetConfig) 
{
  FILE *fp;
  fp = fopen(FileSYSTEM_NET_CONFIG, "w");
  if(fp == NULL)
   return ERROR_FileNotFound;

  if (pNetConfig->DHCP){
    fprintf(fp, "#STATIC_NET_CONFIG=y \n");
  }else{
    fprintf(fp, "STATIC_NET_CONFIG=y \n");
  }

  fprintf(fp, "export interface=eth0 \n");
  fprintf(fp, "export ip=%d.%d.%d.%d \n", pNetConfig->IP_address[0],pNetConfig->IP_address[1],pNetConfig->IP_address[2],pNetConfig->IP_address[3]);
  fprintf(fp, "export router=%d.%d.%d.%d \n", pNetConfig->default_gateway[0],pNetConfig->default_gateway[1],pNetConfig->default_gateway[2],pNetConfig->default_gateway[3]);
  fprintf(fp, "export dns=\"%d.%d.%d.%d %d.%d.%d.%d\" \n", pNetConfig->primary_DNS[0],pNetConfig->primary_DNS[1],pNetConfig->primary_DNS[2],pNetConfig->primary_DNS[3],pNetConfig->secondary_DNS[0],pNetConfig->secondary_DNS[1],pNetConfig->secondary_DNS[2],pNetConfig->secondary_DNS[3]);
  fprintf(fp, "export broadcast=%d.%d.%d.255 \n", pNetConfig->IP_address[0],pNetConfig->IP_address[1],pNetConfig->IP_address[2]);
  fprintf(fp, "export subnet=%d.%d.%d.%d \n", pNetConfig->subnet_mask[0],pNetConfig->subnet_mask[1],pNetConfig->subnet_mask[2],pNetConfig->subnet_mask[3]);
  // fprintf(fp, "export domain=");

  fclose(fp);
  return 0;
}


int sc_fileapi_LoadNetworkSettings(NetworkSettings *pNetworkSettings)
{
	FILE *fp;
	char parameter_name[30];
	char update_flag = 0;
	int i;

	fp = fopen(FileNETWORK_SETTINGS, "r");
  	if(fp == NULL)
    	return ERROR_FileNotFound;

	for(i = 0 ; i < 4 ; i++) {
 		sprintf(parameter_name, "IP_address.%d", i);
	 	update_flag = getUCharValue(fp, parameter_name, &pNetworkSettings->IP_address[i]); 
		if (update_flag<0) {fclose(fp);return update_flag;} 
  	}

	for(i = 0 ; i < 4 ; i++) {
		sprintf(parameter_name, "subnet_mask.%d", i);
 	 	update_flag = getUCharValue(fp, parameter_name, &pNetworkSettings->subnet_mask[i]); 
		if (update_flag<0) {fclose(fp);return update_flag;} 
 	}

	for(i = 0 ; i < 4 ; i++) {
		sprintf(parameter_name, "default_gateway.%d", i);
	 	update_flag = getUCharValue(fp, parameter_name, &pNetworkSettings->default_gateway[i]); 
		if (update_flag<0) {fclose(fp);return update_flag;} 
 	}

	for(i = 0 ; i < 4 ; i++) {
		sprintf(parameter_name, "primary_DNS.%d", i);
	 	update_flag = getUCharValue(fp, parameter_name, &pNetworkSettings->primary_DNS[i]); 
		if (update_flag<0) {fclose(fp);return update_flag;} 
 	}

	for(i = 0 ; i < 4 ; i++) {
		sprintf(parameter_name, "secondary_DNS.%d", i);
	 	update_flag = getUCharValue(fp, parameter_name, &pNetworkSettings->secondary_DNS[i]); 
		if (update_flag<0) {fclose(fp);return update_flag;} 
  	}

  update_flag = getCharValue(fp, "DHCP", &pNetworkSettings->DHCP); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

  update_flag = getCharValue(fp, "IPMode", &pNetworkSettings->IPMode); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
   
	fclose(fp);
  	if(update_flag == 1)
		return ERROR_FileNotUpdated;
	else
		return 0;
}

int sc_fileapi_StoreNetworkSettings(NetworkSettings *pNetworkSettings)
{
	FILE *fp;
	int i;

	fp = fopen(FileNETWORK_SETTINGS, "w");
  if(fp == NULL)
		return ERROR_FileNotFound;

 	for(i = 0 ; i < 4 ; i++)
		fprintf(fp, "IP_address.%d=%d\n", i, pNetworkSettings->IP_address[i]);

	for(i = 0 ; i < 4 ; i++)
		fprintf(fp, "subnet_mask.%d=%d\n", i, pNetworkSettings->subnet_mask[i]);

	for(i = 0 ; i < 4 ; i++)
		fprintf(fp, "default_gateway.%d=%d\n", i, pNetworkSettings->default_gateway[i]);

	for(i = 0 ; i < 4 ; i++)
		fprintf(fp, "primary_DNS.%d=%d\n", i, pNetworkSettings->primary_DNS[i]);

	for(i = 0 ; i < 4 ; i++)
		fprintf(fp, "secondary_DNS.%d=%d\n", i, pNetworkSettings->secondary_DNS[i]);

	fprintf(fp, "DHCP=%d\n", pNetworkSettings->DHCP);
	fprintf(fp, "IPMode=%d\n", pNetworkSettings->IPMode);

	if (0>si_system_configuration_api(pNetworkSettings)) return -1;

  fclose(fp);

 	return 0;
}

int sc_fileapi_LoadNetApplications(NetApplications *pNetApplications)
{
	FILE *fp;
	char update_flag = 0;

	fp = fopen(FileNET_APPLICATIONS, "r");
  if(fp == NULL)
		return ERROR_FileNotFound;

	update_flag = getStrValue(fp, "tftp_server_ip", pNetApplications->tftp_server_ip); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	update_flag = getCharValue(fp, "RSSFeedEnable", &pNetApplications->RSSFeedEnable); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

  update_flag = getCharValue(fp, "RSSFeedTopics", &pNetApplications->RSSFeedTopics); 
	if (update_flag<0) {fclose(fp);return update_flag;}

	update_flag = getCharValue(fp, "InternetRadio", &pNetApplications->InternetRadio); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	update_flag = getCharValue(fp, "NTP_enable", &pNetApplications->NTP_enable); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	update_flag = getStrValue(fp, "NTP_server_address", pNetApplications->NTP_server_address); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

  update_flag = getCharValue(fp, "presence", &pNetApplications->presence); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	fclose(fp);
  	if(update_flag == 1)
		return ERROR_FileNotUpdated;
	else
		return 0;
}

int sc_fileapi_StoreNetApplications(NetApplications *pNetApplications)
{
	FILE *fp;

	fp = fopen(FileNET_APPLICATIONS, "w");
  if(fp == NULL)
		return ERROR_FileNotFound;

	fprintf(fp, "tftp_server_ip=\"%s\"\n", pNetApplications->tftp_server_ip);
	fprintf(fp, "RSSFeedEnable=%d\n", pNetApplications->RSSFeedEnable);
	fprintf(fp, "RSSFeedTopics=%d\n", pNetApplications->RSSFeedTopics);
	fprintf(fp, "InternetRadio=%d\n", pNetApplications->InternetRadio);
	fprintf(fp, "NTP_enable=%d\n", pNetApplications->NTP_enable);
	fprintf(fp, "NTP_server_address=\"%s\"\n", pNetApplications->NTP_server_address);
	fprintf(fp, "presence=%d\n", pNetApplications->presence);

	fclose(fp);

	return 0;
}

#if (!((defined ATA_ENABLED) || (defined DECT_ENABLED)))
int sc_fileapi_LoadDialPlanSettings(DialPlan_settings *pDialPlan_settings)
{
  FILE *fp;
	char update_flag = 0;

	fp = fopen(FileDIALPLAN_SETTINGS, "r");
	if(fp == NULL)
  	return ERROR_FileNotFound;

  //dial plan
  update_flag = getStrValue(fp, "dial_plan", pDialPlan_settings->dial_plan); 
	if (update_flag == 0) {
		if(!(IsDialPlanValid(pDialPlan_settings->dial_plan)))
			pDialPlan_settings->dial_plan[0] = '\0';

	}else{
		fclose(fp);return update_flag;
	}

	//dp_enable
  update_flag = getCharValue(fp, "dp_enable", &pDialPlan_settings->dp_enable); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	fclose(fp);
  if(update_flag == 1)
	  return ERROR_FileNotUpdated;
	else
		return 0;
}

int sc_fileapi_StoreDialPlanSettings(DialPlan_settings *pDialPlan_settings)
{ 
	FILE *fp;

	fp = fopen(FileDIALPLAN_SETTINGS, "w");
	if(fp == NULL)
 	return ERROR_FileNotFound;

	if(!(IsDialPlanValid(pDialPlan_settings->dial_plan))) 
		si_print(PRINT_LEVEL_ERR, "[%s] Invalid Dial Plan", __FUNCTION__);
	else
		fprintf(fp, "dial_plan=\"%s\"\n", pDialPlan_settings->dial_plan);

	fprintf(fp, "dp_enable=%d\n", pDialPlan_settings->dp_enable);
	
	fclose(fp);

	return 0;
}

int sc_fileapi_LoadSubscribeSettings(SubscribeSettings *pSubscribeSettings)
{
	FILE *fp;
	char update_flag = 0;
	char parameter_name[64];
	int i = 0, contacts_no = 0;

	fp = fopen(FileSUBSCRIBE_SETTINGS, "r");
	if(fp == NULL) 
		return ERROR_FileNotFound;

	contacts_no = phbookMemUsage();

 	// Load subscribe entries
	for (i = 0; i < contacts_no; i++) {
		sprintf(parameter_name, "subscribe_enable.%d", i);
		update_flag = getCharValue(fp, parameter_name, &pSubscribeSettings->subscribe_enable[i]); 
		if (update_flag<0) {fclose(fp);return update_flag;} 

		sprintf(parameter_name, "subscribe_number.%d", i);
		update_flag = getStrValue(fp, parameter_name, pSubscribeSettings->subscribe_number[i]); 
		if (update_flag<0) {fclose(fp);return update_flag;} 

		sprintf(parameter_name, "subscribe_name.%d", i);
		update_flag = getStrValue(fp, parameter_name, pSubscribeSettings->subscribe_name[i]); 
		if (update_flag<0) {fclose(fp);return update_flag;} 

		sprintf(parameter_name, "subscribe_status.%d", i);
		update_flag = getCharValue(fp, parameter_name, &pSubscribeSettings->subscribe_status[i]); 
		if (update_flag<0) {fclose(fp);return update_flag;} 
	}

	fclose(fp);
  if(update_flag == 1)
		return ERROR_FileNotUpdated;
	else
		return 0;
}

int sc_fileapi_StoreSubscribeSettings(SubscribeSettings *pSubscribeSettings)
{
	FILE *fp;
	int i = 0, contacts_no = 0;

	fp = fopen(FileSUBSCRIBE_SETTINGS, "w");
	if(fp == NULL)
		return ERROR_FileNotFound;

	contacts_no = phbookMemUsage();

	for(i = 0 ; i < contacts_no ; i++) {
		fprintf(fp, "subscribe_enable.%d=%d\n", i, pSubscribeSettings->subscribe_enable[i]);
		fprintf(fp, "subscribe_name.%d=%s\n", i, pSubscribeSettings->subscribe_name[i]);
		fprintf(fp, "subscribe_number.%d=%s\n", i, pSubscribeSettings->subscribe_number[i]);
		fprintf(fp, "subscribe_status.%d=%d\n", i, pSubscribeSettings->subscribe_status[i]);
	}

	fclose(fp);

	return 0;
}

int Subscribe_SyncTo_Phbook(void)
{
	int i = 0, k = 0, contacts_no = 0;
	phbookContact *t_phbookContact;
	SubscribeSettings pSubscribeSettings;
	SubscribeSettings tmp_SubscribeSettings;

	sc_fileapi_LoadSubscribeSettings(&pSubscribeSettings);

	memcpy(&tmp_SubscribeSettings, &pSubscribeSettings, sizeof(SubscribeSettings));
	memset(&pSubscribeSettings, 0, sizeof(SubscribeSettings));

	contacts_no = phbookMemUsage();
	t_phbookContact = phbookGotoFirst();

	// copy all phbook entries to the subscribe list
	for (i = 0; i < contacts_no; i++)
	{
		if (t_phbookContact == NULL) break;

		strcpy(pSubscribeSettings.subscribe_number[i], t_phbookContact->home_number);
		strcpy(pSubscribeSettings.subscribe_name[i], t_phbookContact->name);
		pSubscribeSettings.subscribe_enable[i] = SUBSCRIBE_ON;
		pSubscribeSettings.subscribe_status[i] = SUBSCRIBE_STATUS_OFF;

		t_phbookContact = phbookGotoNext();
	}

	for (i = 0; i < MAX_NUM_OF_CONTACTS; i++){
		for (k = 0; k < MAX_NUM_OF_CONTACTS; k++){
			// find previous subscribe entries that match the new entries
			if (!(strcmp(tmp_SubscribeSettings.subscribe_name[k], pSubscribeSettings.subscribe_name[i]))){
				pSubscribeSettings.subscribe_enable[i] = tmp_SubscribeSettings.subscribe_enable[k];
				pSubscribeSettings.subscribe_status[i] = tmp_SubscribeSettings.subscribe_status[k];
			}
		}
	}

	sc_fileapi_StoreSubscribeSettings(&pSubscribeSettings);

	return 0;
}

int sc_fileapi_LoadPhoneSettings(PhoneSettings *pPhoneSettings)
{
	FILE *fp;
	char update_flag = 0;

	fp = fopen(FilePHONE_SETTINGS, "r");
  if(fp == NULL)
		return ERROR_FileNotFound;
	
	//display_brightness
  update_flag = getCharValue(fp, "display_brightness", &pPhoneSettings->display_brightness); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
  
	//backlight_timeout
  update_flag = getCharValue(fp, "backlight_timeout", &pPhoneSettings->backlight_timeout); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	//ring_tone
  update_flag = getStrValue(fp, "ring_tone", pPhoneSettings->ring_tone); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	//ring_volume
  update_flag = getCharValue(fp, "ring_volume", &pPhoneSettings->ring_volume); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
	//speaker_volume
  update_flag = getUShortValue(fp, "speaker_volume", &pPhoneSettings->speaker_volume); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	//handset_volume
  update_flag = getUShortValue(fp, "handset_volume", &pPhoneSettings->handset_volume); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
	//headset_volume
  update_flag = getUShortValue(fp, "headset_volume", &pPhoneSettings->headset_volume); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

  //keypad_tone
  update_flag = getCharValue(fp, "keypad_tone", &pPhoneSettings->keypad_tone); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
  //date_year
  update_flag = getCharValue(fp, "date_year", &pPhoneSettings->date_year); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

  //date_month
  update_flag = getCharValue(fp, "date_month", &pPhoneSettings->date_month); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
 	//date_day
  update_flag = getCharValue(fp, "date_day", &pPhoneSettings->date_day); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
 	//time_hour
  update_flag = getCharValue(fp, "time_hour", &pPhoneSettings->time_hour); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
 	//time_minute
  update_flag = getCharValue(fp, "time_minute", &pPhoneSettings->time_minute); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
	//time_second
  update_flag = getCharValue(fp, "time_second", &pPhoneSettings->time_second); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
	//time_format
  update_flag = getCharValue(fp, "time_format", &pPhoneSettings->time_format); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	//time_zone
  update_flag = getCharValue(fp, "time_zone", &pPhoneSettings->time_zone); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
	//language
  update_flag = getCharValue(fp, "language", &pPhoneSettings->language); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
	fclose(fp);
  	
	if(update_flag == 1)
		return ERROR_FileNotUpdated;
	else
		return 0;
}


int sc_fileapi_StorePhoneSettings(PhoneSettings *pPhoneSettings)
{
	FILE *fp;

	fp = fopen(FilePHONE_SETTINGS, "w");
  if(fp == NULL)
   	return ERROR_FileNotFound;
 
	fprintf(fp, "display_brightness=%d\n", pPhoneSettings->display_brightness);
	fprintf(fp, "backlight_timeout=%d\n", pPhoneSettings->backlight_timeout);
	fprintf(fp, "ring_tone=\"%s\"\n", pPhoneSettings->ring_tone);
	fprintf(fp, "ring_volume=%d\n", pPhoneSettings->ring_volume);
	fprintf(fp, "speaker_volume=%d\n", pPhoneSettings->speaker_volume);
	fprintf(fp, "handset_volume=%d\n", pPhoneSettings->handset_volume);
	fprintf(fp, "headset_volume=%d\n", pPhoneSettings->headset_volume);
	fprintf(fp, "keypad_tone=%d\n", pPhoneSettings->keypad_tone);
	fprintf(fp, "date_year=%d\n", pPhoneSettings->date_year);
	fprintf(fp, "date_month=%d\n", pPhoneSettings->date_month);
	fprintf(fp, "date_day=%d\n", pPhoneSettings->date_day);
	fprintf(fp, "time_hour=%d\n", pPhoneSettings->time_hour);
	fprintf(fp, "time_minute=%d\n", pPhoneSettings->time_minute);
	fprintf(fp, "time_second=%d\n", pPhoneSettings->time_second);
	fprintf(fp, "time_format=%d\n", pPhoneSettings->time_format);
	fprintf(fp, "time_zone=%d\n", pPhoneSettings->time_zone);
	fprintf(fp, "language=%d\n", pPhoneSettings->language);

	fclose(fp);

	return 0;
}
#endif

int sc_fileapi_LoadSIPSettings(SIPSettings *pSIPSettings)
{
	FILE *fp;
	char update_flag = 0;

	fp = fopen(FileSIP_SETTINGS, "r");
  	if(fp == NULL)
    	return ERROR_FileNotFound;
 
	//domain
  update_flag = getStrValue(fp, "domain", pSIPSettings->domain); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	//registrar
  update_flag = getStrValue(fp, "registrar", pSIPSettings->registrar); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
	//proxy_address
  update_flag = getStrValue(fp, "proxy_address", pSIPSettings->proxy_address); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
	//registrarPort
  update_flag = getStrValue(fp, "registrarPort", pSIPSettings->registrarPort); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
	//IP_address
  update_flag = getStrValue(fp, "IP_address", pSIPSettings->IP_address); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
	//STUN_address
  update_flag = getStrValue(fp, "STUN_address", pSIPSettings->STUN_address); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
	//STUN_port
  update_flag = getStrValue(fp, "STUN_port", pSIPSettings->STUN_port); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	//RTP_port_start
  update_flag = getStrValue(fp, "RTP_port_start", pSIPSettings->RTP_port_start); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	//RTP_port_stop
  update_flag = getStrValue(fp, "RTP_port_stop", pSIPSettings->RTP_port_stop); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
	//expiry
  update_flag = getIntValue(fp, "expiry", &pSIPSettings->expiry); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
  
	//publicport
  update_flag = getStrValue(fp, "publicport", pSIPSettings->publicport); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
	//NAT

  update_flag = getCharValue(fp, "NAT", &pSIPSettings->NAT); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
 	//use_out_bound_proxy
  update_flag = getCharValue(fp, "use_out_bound_proxy", &pSIPSettings->use_out_bound_proxy); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
 	//transport
  update_flag = getCharValue(fp, "transport", &pSIPSettings->transport); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	//Session timer
  update_flag = getCharValue(fp, "SessionTimer", &pSIPSettings->SessionTimer); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	//Naptr
  update_flag = getCharValue(fp, "NAPTR", &pSIPSettings->NAPTR); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	//Prack
  update_flag = getCharValue(fp, "PRACK", &pSIPSettings->PRACK); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	//Update
    update_flag = getCharValue(fp, "update", &pSIPSettings->update); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	//Loacl Sip Port
	update_flag = getStrValue(fp, "LocalSipPort", pSIPSettings->LocalSipPort); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
	fclose(fp);
  if(update_flag == 1)
		return ERROR_FileNotUpdated;
	else
		return 0;
}


int sc_fileapi_StoreSIPSettings(SIPSettings *pSIPSettings)
{
	FILE *fp;

	fp = fopen(FileSIP_SETTINGS, "w");
  if(fp == NULL)
   	return ERROR_FileNotFound;

	fprintf(fp, "domain=\"%s\"\n", pSIPSettings->domain);
	fprintf(fp, "registrar=\"%s\"\n", pSIPSettings->registrar);
	fprintf(fp, "proxy_address=\"%s\"\n", pSIPSettings->proxy_address);
	fprintf(fp, "registrarPort=\"%s\"\n", pSIPSettings->registrarPort);
	fprintf(fp, "IP_address=\"%s\"\n", pSIPSettings->IP_address);
	fprintf(fp, "STUN_address=\"%s\"\n", pSIPSettings->STUN_address);
	fprintf(fp, "STUN_port=\"%s\"\n", pSIPSettings->STUN_port);
	fprintf(fp, "RTP_port_start=\"%s\"\n", pSIPSettings->RTP_port_start);
	fprintf(fp, "RTP_port_stop=\"%s\"\n", pSIPSettings->RTP_port_stop);
	fprintf(fp, "expiry=%d\n", pSIPSettings->expiry);
	fprintf(fp, "publicport=\"%s\"\n", pSIPSettings->publicport);
	fprintf(fp, "NAT=%d\n", pSIPSettings->NAT);
	fprintf(fp, "use_out_bound_proxy=%d\n", pSIPSettings->use_out_bound_proxy);
	fprintf(fp, "transport=%d\n", pSIPSettings->transport);
	fprintf(fp, "SessionTimer=%d\n", pSIPSettings->SessionTimer);
	fprintf(fp, "NAPTR=%d\n", pSIPSettings->NAPTR);
	fprintf(fp, "PRACK=%d\n", pSIPSettings->PRACK);
	fprintf(fp, "update=%d\n", pSIPSettings->update);
	fprintf(fp, "LocalSipPort=\"%s\"\n", pSIPSettings->LocalSipPort);

	fclose(fp);

	return 0;
}


int sc_fileapi_LoadSIPAccount(SIPAccount *pSIPAccount, int account)
{
	FILE *fp;
	char update_flag = 0;

	fp = fopen(FileSIP_ACCOUNTS[account-1], "r");
	
	if(fp == NULL)
    return ERROR_FileNotFound;

	//user_id
  update_flag = getStrValue(fp, "user_id", pSIPAccount->user_id); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
	//user_name
  update_flag = getStrValue(fp, "user_name", pSIPAccount->user_name); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

	//user_password
  update_flag = getStrValue(fp, "user_password", pSIPAccount->user_password); 
	if (update_flag<0) {fclose(fp);return update_flag;} 
 
  //use_account
  update_flag = getCharValue(fp, "use_account", &pSIPAccount->use_account); 
	if (update_flag<0) {fclose(fp);return update_flag;} 

 	fclose(fp);
  if(update_flag == 1)
	  return ERROR_FileNotUpdated;
	else
		return 0;
}


int sc_fileapi_StoreSIPAccount(SIPAccount *pSIPAccount, int account)
{
	FILE *fp;

	fp = fopen(FileSIP_ACCOUNTS[account-1], "w");

  if(fp == NULL)
   	return ERROR_FileNotFound;

	fprintf(fp, "user_id=\"%s\"\n", pSIPAccount->user_id);
	fprintf(fp, "user_name=\"%s\"\n", pSIPAccount->user_name);
	fprintf(fp, "user_password=\"%s\"\n", pSIPAccount->user_password);
  fprintf(fp, "use_account=\"%d\"\n", pSIPAccount->use_account);

	fclose(fp);

	return 0;
}


int extract_parameter_value(FILE* fp, char *parameter_name, char *parameter_value)
{
	char buffer[256];
	char *ptr;

	while(1) {
	 	if(fgets(buffer, sizeof(buffer), fp) != NULL) {
			if(buffer[strlen(buffer) - 1] != '\n')
				return ERROR_SyntaxError;

			buffer[strlen(buffer) - 1] = 0;

			for(ptr = buffer ; *ptr != 0 ; ptr++) {
				if(*ptr == '#') {
					*ptr = 0;
					break;
				}
			}
				
			ptr = buffer;
			while(isspace(*ptr))
				ptr++;

			if(!strncmp(ptr, parameter_name, strlen(parameter_name))) {
				ptr += strlen(parameter_name);

				if(*ptr == 0)
					return ERROR_SyntaxError;

				if(!isspace(*ptr) && (*ptr != '='))
					continue;

				while(isspace(*ptr) || (*ptr == '='))
					ptr++;

				if(*ptr == 0)
					return ERROR_SyntaxError;
					
				if(*ptr == '"') {
					ptr++;
					if(*ptr == 0)
						return ERROR_SyntaxError;
	
					strcpy(parameter_value, ptr);
					for(ptr = parameter_value ; *ptr != 0 ; ptr++) {
						if(*ptr == '"') {
							*ptr = 0;
							break;
						}
					}
				}else{
					strcpy(parameter_value, ptr);
					for(ptr = parameter_value ; *ptr != 0 ; ptr++) {
						if(isspace(*ptr)) {
							*ptr = 0;
							break;
						}
					}
				}

				return 0;
			}
		}else{
			return ERROR_FileNotUpdated;
		}
	}
 	return 0;
}


int getStrValue(FILE* fp, char *parameter_name, char *parameter_value)
{
	char parameter[256];
	int ret;
	rewind(fp);
 	ret = extract_parameter_value(fp, parameter_name, parameter);
	
	if(ret == 0) strcpy(parameter_value, parameter);
#ifdef DEGUG_PRINT_CONFIGURATIONFILE
	si_print(PRINT_LEVEL_INFO, "Configuration parameter: %s %s \n ",parameter_name,  parameter);	
#endif

 	return ret;
}

int getCharValue(FILE* fp, char *parameter_name, char *parameter_value)
{
	char parameter[256];
	int ret;
	rewind(fp);
 	ret = extract_parameter_value(fp, parameter_name, parameter);
	if(ret == 0) parameter_value[0] = (char)atoi(parameter);
#ifdef DEGUG_PRINT_CONFIGURATIONFILE
	si_print(PRINT_LEVEL_INFO,"Configuration parameter: %s %d \n ",parameter_name,  parameter_value[0]);	
#endif
 	return ret;
 }
int getUCharValue(FILE* fp, char *parameter_name, unsigned char *parameter_value)
{
	char parameter[256];
	int ret;
	rewind(fp);
 	ret = extract_parameter_value(fp, parameter_name, parameter);
	if(ret == 0) parameter_value[0] = (unsigned char)atoi(parameter);
#ifdef DEGUG_PRINT_CONFIGURATIONFILE
	si_print(PRINT_LEVEL_INFO, "Configuration parameter: %s %d \n ",parameter_name,  parameter_value[0]);	
#endif
	return ret;
 }

int getIntValue(FILE* fp, char *parameter_name, int *parameter_value)
{
	char parameter[256];
	int ret;
	rewind(fp);
 	ret = extract_parameter_value(fp, parameter_name, parameter);
	if(ret == 0) parameter_value[0] = (int)atoi(parameter);
#ifdef DEGUG_PRINT_CONFIGURATIONFILE
	si_print(PRINT_LEVEL_INFO, "Configuration parameter: %s %d \n ",parameter_name,  parameter_value[0]);	
#endif
	return ret;
}

int getUIntValue(FILE* fp, char *parameter_name,unsigned  int *parameter_value)
{
	char parameter[256];
	int ret;
	rewind(fp);
 	ret = extract_parameter_value(fp, parameter_name, parameter);
	if(ret == 0) parameter_value[0] = (unsigned int)atoi(parameter);
#ifdef DEGUG_PRINT_CONFIGURATIONFILE
	si_print(PRINT_LEVEL_INFO, "Configuration parameter: %s %d \n ",parameter_name,  parameter_value[0]);	
#endif
	return ret;
}

int getShortValue(FILE* fp, char *parameter_name, short *parameter_value)
{
	char parameter[256];
	int ret;
	rewind(fp);
 	ret = extract_parameter_value(fp, parameter_name, parameter);
	if(ret == 0) parameter_value[0] = (short)atoi(parameter);
#ifdef DEGUG_PRINT_CONFIGURATIONFILE
	si_print(PRINT_LEVEL_INFO, "Configuration parameter: %s %d \n ",parameter_name,  parameter_value[0]);	
#endif
	return ret;
}

int getUShortValue(FILE* fp, char *parameter_name, unsigned short *parameter_value)
{
	char parameter[256];
	int ret;
	rewind(fp);
 	ret = extract_parameter_value(fp, parameter_name, parameter);
	if(ret == 0) parameter_value[0] = (unsigned short)atoi(parameter);
#ifdef DEGUG_PRINT_CONFIGURATIONFILE
	si_print(PRINT_LEVEL_INFO, "Configuration parameter: %s %d \n ",parameter_name,  parameter_value[0]);	
#endif
	return ret;
}


