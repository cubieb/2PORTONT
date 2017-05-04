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
 * File:		siUAFunctions.h
 * Purpose:		
 * Created:		Jul 2008
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */
#ifndef SIUAFUNCTIONS_H
#define SIUAFUNCTIONS_H

#include "si_ua_api.h"
#include "../common/si_config_file.h"
#include "../common/operation_mode_defs.h"

#ifndef ATA_ENABLED
  int libUASetPhoneSettings(PhoneSettings *pPhoneSettings, SICORE *pCore);
#endif

#ifdef SA_CVM
#define libUASetNonCriticalCoreValues( x )
#else
int libUASetNonCriticalCoreValues(ConfigSettings *pConfigSettings);
#endif // SA_CVM
int libUASetCriticalCoreValues(ConfigSettings *pConfigSettings);
int libUASetSIPSettings(SIPSettings *pSIPSettings, SICORE *pCore);
int libUASetNetApplicationSettings(NetApplications *pNetApplications, SICORE *pCore);
int libUASetSIPAccounts(SIPAccount *pAccount, int AccNum, SICORE *pCore);
int libUASetCallSettings(CallSettings *pCallSettings, int AccNum,SICORE *pCore);
int libUASetAudioSettings(AudioSettings *pAudioSettings, int AccNum, SICORE *pCore);
int libUASetCore(unsigned char* IPAddress, ConfigSettings *pConfigSettings, SICORE *pCore);
int libReadConfigFiles(ConfigSettings *pConfigSettings);
int libUASetSetCodecSupported(ConfigSettings *pConfigSettings);
int libUASetNetworkSettings(NetworkSettings *pNetwork, SICORE *pCore);
int getRandomPort(void);

#endif // SIUAFUNCTIONS_H
