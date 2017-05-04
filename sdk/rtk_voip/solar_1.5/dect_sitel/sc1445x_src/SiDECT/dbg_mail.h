/****************************************************************************
*  Program/file: DBG_MAIL.H
*
*  Copyright (C) by RTX TELECOM A/S, Denmark.
*  These computer program listings and specifications, are the property of
*  RTX TELECOM A/S, Denmark and shall not be reproduced or copied or used in
*  whole or in part without written permission from RTX TELECOM A/S, Denmark.
*
*  Programmer: LHJ
*
*  MODULE: WIN32SIM
*  CONTROLLING DOCUMENT:
*  SYSTEM DEPENDENCIES:
*
*
*  DESCRIPTION: Mail definitions.
*
*
*
*
*
****************************************************************************/

/****************************************************************************
*                                  PVCS info
*****************************************************************************

$Author: krammer $
$Date: 2012/08/17 08:26:37 $
$Revision: 1.1 $
$Modtime:   21 Feb 2003 15:11:46  $

*/

#ifndef __DBG_MAIL_H
#define __DBG_MAIL_H

typedef enum
{
   DUMMY_DATA=0,
   KEY_MESSAGE,

   EE_WRITE_DEFAULT_req = 0x3A,

   //&&&&&&&&&&&&& LINE TASK - LINE DRIVER &&&&&&&&&&&&&
   LINE_METERING_PULSE_REQ=0x12A0,
   LINE_METERING_PULSE_CFM,
   LINE_POLARITY_REVERSAL_REQ,
   LINE_POLARITY_REVERSAL_CFM,
   LINE_ALERT_REQ,
   LINE_ALERT_CFM,
   LINE_STOP_ALERT_REQ,
   LINE_STOP_ALERT_CFM,
   LINE_KEYPAD_IND,
   LINE_SINGLE_KEY_IND,
   LINE_OFF_HOOK_IND,
   LINE_ON_HOOK_IND,
   LINE_HOOK_FLASH_IND,
   LINE_MODEM_IND,
   LINE_POLARITY_REVERSAL_TIMEOUT,
   LINE_KEY_POLL_TIMEOUT,
   LINE_CMD,
   LINE_SETUP_REQ,
   LINE_SETUP_IND,
   LINE_IWUTOIWU_IND,
   LINE_PROGRESS_IND,
   LINE_TEST_TIMEOUT,
   LINE_LINK_ESTABLISH_IND,
   LINE_LINK_SHUT_DOWN_IND,
   LINE_SYSTEM_LOCKED,
   LINE_SYSTEM_UNLOCKED,
   ATSA_LINE_DRIVER_NEW_PARAMATERS_IND,
   LINE_TEST_PHONE_PRESENT_REQ,
   LINE_TEST_PHONE_PRESENT_CFM,
   LINE_PHONE_PRESENT_IND,
   LINE_PHONE_NOT_PRESENT_IND,
   LINE_RING_RING_TRIP_TEST_REQ,
   LINE_RING_RING_TRIP_TEST_CFM,
   LINE_RING_RING_TRIP_FAIL_IND, 
   LINE_RING_RING_TRIP_OK_IND,
   
   //&&&&&&&&&&&&& DSP TASK &&&&&&&&&&&&&
   DSP_OPEN_AUDIO_REQ=0x1400,
   DSP_OPEN_AUDIO_CFM,
   DSP_CLOSE_AUDIO_REQ,
   DSP_CLOSE_AUDIO_CFM,
   DSP_DISCONNECTION_TONE_REQ,
   DSP_DISCONNECTION_TONE_CFM,
   DSP_USER_ISOLATION_TONE_REQ,
   DSP_USER_ISOLATION_TONE_CFM,
   DSP_BUSY_TONE_REQ,
   DSP_BUSY_TONE_CFM,
   DSP_TONE_OFF_REQ,
   DSP_TONE_OFF_CFM,
   DSP_SWITCH_TO_MODEM_REQ,
   DSP_SWITCH_TO_MODEM_CFM,
   DSP_SWITCH_TO_VOICE_REQ,
   DSP_SWITCH_TO_VOICE_CFM,
   DSP_RINGBACK_TONE_REQ,
   DSP_RINGBACK_TONE_CFM,
   DSP_DAILTONE_REQ,
   DSP_DAILTONE_CFM,
   DSP_CONTROL_TIMEOUT_1,
   DSP_CONTROL_TIMEOUT_2,
   
   DSS1_DL_EST_REQ=0x2180,
   DSS1_DL_REL_REQ,
   DSS1_DL_DATA_REQ,
   DSS1_DL_UDATA_REQ,

   // ##########################################################################
   //                            TEST PRIMITIVES
   //  Please do not edit values or change formating without notifying CM
   //
   // ##########################################################################
   
   TEST_MAILTRACE_START_REQ                   = 0xF000,
   TEST_MAILTRACE_START_CFM                           ,
   TEST_MAILTRACE_STOP_REQ                            ,
   TEST_MAILTRACE_STOP_CFM                            ,
   TEST_MAILTRACE_DOWNLOAD_OPTIONS_REQ                ,
   TEST_MAILTRACE_DOWNLOAD_OPTIONS_CFM                ,
   TEST_MAILTRACE_DOWNLOAD_START_REQ                  ,
   TEST_MAILTRACE_TRACE_START_TIMER_IND               ,
   TEST_MAILTRACE_TRACE_STOP_TIMER_IND                ,
   TEST_MAILTRACE_TRACE_OUTPUT_MAIL_IND               ,
   TEST_MAILTRACE_TRACE_INPUT_MAIL_IND                ,
   TEST_MAILTRACE_TRACE_COMMENT_IND                   ,
   TEST_MAILTRACE_STOP_IND                            ,
   TEST_MAILTRACE_MAILQUEUE_DUMP_IND                  ,
   TEST_MAILTRACE_TRACE_INPUT_DDU_SIGNAL_IND          ,
   TEST_MAILTRACE_TRACE_OUTPUT_DDU_SIGNAL_IND         ,

   TEST_AFIELD_START_REQ                      = 0xF020,
   TEST_AFIELD_START_CFM                              ,
   TEST_AFIELD_STOP_REQ                               ,
   TEST_AFIELD_STOP_CFM                               ,
   TEST_AFIELD_DATA_FP_RX_IND                         ,
   TEST_AFIELD_DATA_FP_TX_IND                         ,
   TEST_AFIELD_DATA_PP_RX_IND                         ,
   TEST_AFIELD_DATA_PP_TX_IND                         ,
   TEST_A_B_FULL_SLOT_DATA_FP_RX_IND                  ,
   TEST_A_B_FULL_SLOT_DATA_FP_TX_IND                  ,
   TEST_A_B_FULL_SLOT_DATA_PP_RX_IND                  ,
   TEST_A_B_FULL_SLOT_DATA_PP_TX_IND                  ,
   TEST_A_B_DOUBLE_SLOT_DATA_FP_RX_IND                ,
   TEST_A_B_DOUBLE_SLOT_DATA_FP_TX_IND                ,
   TEST_A_B_DOUBLE_SLOT_DATA_PP_RX_IND                ,
   TEST_A_B_DOUBLE_SLOT_DATA_PP_TX_IND                ,

   TEST_MEMORY_WRITE_REQ=0xF030                       ,
   TEST_MEMORY_READ_REQ                               ,
   TEST_MEMORY_READ_CFM                               ,
   TEST_MEMORY_WRITE_CFM                              ,
   TEST_MEMORY_MEMSET_REQ                             ,
   TEST_MEMORY_MEMSET_CFM                             ,

   // Interface to Infineon DECT chipset DSPRAM
   TEST_READ_DSPRAM_REQ                               ,
   TEST_READ_DSPRAM_CFM                               ,
   TEST_WRITE_DSPRAM_REQ                              ,
   TEST_WRITE_DSPRAM_CFM                              ,

   TEST_BMC_TRACE_START_REQ=0xF040                    ,
   TEST_BMC_TRACE_START_CFM                           ,
   TEST_BMC_TRACE_STOP_REQ                            ,
   TEST_BMC_TRACE_STOP_CFM                            ,
   TEST_BMC_TRACE_MSG_IND                             ,
   
   TEST_MODULE_TEST_START=0xF060                      ,
   TEST_MODULE_TEST_STOP                              ,

   TEST_RDBG_PRINTF_START_REQ                 = 0xF070,
   TEST_RDBG_PRINTF_START_REQ_NO_CFM                  ,
   TEST_RDBG_PRINTF_START_CFM                         ,
   TEST_RDBG_PRINTF_STOP_REQ                          ,
   TEST_RDBG_PRINTF_STOP_CFM                          ,
   TEST_RDBG_PRINTF_IND                               ,

   TEST_LOAD_MONITOR_START_REQ                = 0xF080,
   TEST_LOAD_MONITOR_START_CFM                        ,
   TEST_LOAD_MONITOR_STOP_REQ                         ,
   TEST_LOAD_MONITOR_STOP_CFM                         ,
   TEST_LOAD_MONITOR_LOAD_IND                         ,
   TEST_DPRS_BANDWIDTH_CHANGE_REQ                     ,
   TEST_DPRS_BANDWIDTH_CHANGE_CFM                     ,
   TEST_DPRS_BANDWIDTH_MANUAL_REQ                     ,
   TEST_DPRS_BANDWIDTH_MANUAL_CFM                     ,
   TEST_DPRS_BANDWIDTH_AUTO_REQ                       ,
   TEST_DPRS_BANDWIDTH_AUTO_CFM                       ,
   TEST_DPRS_START_C_REQ                              ,
   TEST_DPRS_START_C_CFM                              ,
   TEST_DPRS_STOP_C_REQ                               ,
   TEST_DPRS_STOP_C_CFM                               ,

   // ##########################################################################
   //                          PTA TEST PRIMITIVES
   // ##########################################################################

   // GENERAL TESTS
   TEST_HELLO_WORLD_REQ                        =0xf100,
   TEST_HELLO_WORLD_CFM                               ,

   TEST_ENTER_TESTMODE_REQ                            ,
   TEST_EXIT_TESTMODE_REQ                             ,
   TEST_ENTER_TESTMODE_CFM                            ,
   TEST_EXIT_TESTMODE_CFM                             ,

   TEST_FLASH_SHIFT_REQ                               ,
   TEST_FLASH_SHIFT_CFM                               ,
   TEST_FLASH_CHECK_REQ                               ,
   TEST_FLASH_A_IND                                   ,
   TEST_FLASH_B_IND                                   ,
   
   TEST_RAM_REQ                                       ,
   TEST_RAM_CFM                                       ,
   TEST_RAM_OK_IND                                    ,
   TEST_RAM_ERROR_IND                                 ,
   
   TEST_ROM_REQ                                       ,
   TEST_ROM_CFM                                       ,
   TEST_ROM_OK_IND                                    ,
   TEST_ROM_ERROR_IND                                 ,

   TEST_SW_VERSION_REQ                                ,
   TEST_SW_VERSION_CFM                                ,
   
   TEST_EEPROM_TEST_REQ                               ,
   TEST_EEPROM_TEST_CFM                               ,
   TEST_EEPROM_TEST_OK_IND                            ,
   TEST_EEPROM_TEST_ERROR_IND                         ,
   
   TEST_EEPROM_SET_DEFAULT_REQ                        ,
   TEST_EEPROM_SET_DEFAULT_CFM                        ,
   
   TEST_EEPROM_READ_REQ                               ,
   TEST_EEPROM_READ_CFM                               ,
   TEST_EEPROM_WRITE_REQ                              ,
   TEST_EEPROM_WRITE_CFM                              ,
   
   TEST_FACTORY_SETTING_READ_REQ                      , 
   TEST_FACTORY_SETTING_READ_CFM                      ,
   TEST_FACTORY_SETTING_WRITE_REQ                     , 
   TEST_FACTORY_SETTING_WRITE_CFM                     , 
   
   TEST_READ_ADC0_REQ                                 ,
   TEST_READ_ADC1_REQ                                 ,
   TEST_READ_ADC0_CFM                                 ,
   TEST_READ_ADC1_CFM                                 ,
   
   TEST_DIGITAL_LINK_LOOPBACK_ON_REQ                  ,
   TEST_DIGITAL_LINK_LOOPBACK_ON_CFM                  ,
   TEST_DIGITAL_LINK_LOOPBACK_OK_IND                  ,
   TEST_DIGITAL_LINK_LOOPBACK_ERROR_IND               ,
   
   TEST_SET_MIN_ADC0_REQ                              ,
   TEST_SET_MIN_ADC0_CFM                              ,
  
   // ATS TESTS
   TEST_LED_ON_REQ                             =0xf200,
   TEST_LED_OFF_REQ                                   ,
   TEST_LED_ON_CFM                                    ,
   TEST_LED_OFF_CFM                                   ,
   
   TEST_CHARGER_ON_REQ                                ,
   TEST_CHARGER_OFF_REQ                               ,
   TEST_CHARGER_ON_CFM                                ,
   TEST_CHARGER_OFF_CFM                               ,
   
   TEST_BATTERY_ADC_READ_REQ                          ,
   TEST_BATTERY_ADC_READ_CFM                          ,
   TEST_BATTERY_MAX_SET_REQ                           ,
   TEST_BATTERY_MAX_SET_CFM                           ,
   TEST_BATTERY_MIN_SET_REQ                           ,
   TEST_BATTERY_MIN_SET_CFM                           ,
  
   TEST_ATS_CTU_LINK_REQ                              ,
   TEST_ATS_CTU_LINK_CFM                              ,
   TEST_ATS_CTU_LINK_OK_IND                           ,
   TEST_ATS_CTU_LINK_ERROR_IND                        ,

   // CTU TESTS
  TEST_FREQUENCY_WRITE_REQ=0xf300,
  TEST_FREQUENCY_WRITE_CFM,
  TEST_FREQUENCY_READ_REQ,
  TEST_FREQUENCY_READ_CFM,
  TEST_QUAD_WRITE_REQ,
  TEST_QUAD_WRITE_CFM,
  TEST_QUAD_READ_REQ,
  TEST_QUAD_READ_CFM,
  TEST_QUAD_TUNE_ON_REQ,
  TEST_QUAD_TUNE_ON_CFM,
  TEST_QUAD_TUNE_COMPLETE_IND,
  TEST_MODULATION_GAIN_WRITE_REQ,
  TEST_MODULATION_GAIN_WRITE_CFM,
  TEST_MODULATION_GAIN_READ_REQ,
  TEST_MODULATION_GAIN_READ_CFM,
  TEST_RF_POWER_WRITE_REQ,
  TEST_RF_POWER_WRITE_CFM,
  TEST_RF_POWER_READ_REQ,
  TEST_RF_POWER_READ_CFM,
  TEST_RSSI_MEASUREMENT_REQ,
  TEST_RSSI_MEASUREMENT_CFM,
  TEST_RSSI_WRITE_PARAMETERS_REQ,
  TEST_RSSI_WRITE_PARAMETERS_CFM,
  TEST_MAC_INFO_REQ,
  TEST_MAC_INFO_CFM,
  TEST_MAC_CONT_TX_REQ,
  TEST_MAC_CONT_TX_CFM,
  TEST_MAC_CONT_RX_REQ,
  TEST_MAC_CONT_RX_CFM,
  TEST_PP_LOCK_REQ,
  TEST_PP_LOCK_CFM,
  TEST_ANTENNA_SELECT_REQ,
  TEST_ANTENNA_SELECT_CFM,
  TEST_TX_POWER_HIGH_REQ,
  TEST_TX_POWER_HIGH_CFM,
  TEST_TX_POWER_LOW_REQ,
  TEST_TX_POWER_LOW_CFM,
  TEST_TX_POWER_ENABLE_ALGORITM_REQ,
  TEST_TX_POWER_ENABLE_ALGORITM_CFM,
  TEST_MAC_SET_FIXED_FREQ_REQ,
  TEST_MAC_SET_FIXED_FREQ_CFM,

   // ATS-I TESTS
   TEST_ISAC_AUTO_FREQUENCY_ON_REQ             =0xf400,
   TEST_ISAC_AUTO_FREQUENCY_OFF_REQ                   ,
   TEST_ISAC_AUTO_FREQUENCY_ON_CFM                    ,
   TEST_ISAC_AUTO_FREQUENCY_OFF_CFM                   ,
   TEST_ISAC_POWER_SOURCE_ON_REQ                      ,
   TEST_ISAC_POWER_SOURCE_OFF_REQ                     ,
   TEST_ISAC_POWER_SOURCE_ON_CFM                      ,
   TEST_ISAC_POWER_SOURCE_OFF_CFM                     ,
   TEST_ISAC_LINE_ACTIVATE_REQ                        ,
   TEST_ISAC_LINE_DEACTIVATE_REQ                      ,
   TEST_ISAC_LINE_ACTIVATE_CFM                        ,
   TEST_ISAC_LINE_DEACTIVATE_CFM                      ,
   TEST_ISAC_LOOPBACK_ON_REQ                          ,
   TEST_ISAC_LOOPBACK_OFF_REQ                         ,
   TEST_ISAC_LOOPBACK_ON_CFM                          ,
   TEST_ISAC_LOOPBACK_OFF_CFM                         ,

   // ATS-A TEST
   TEST_RING_REQ                               =0xf500,
   TEST_STOP_RING_REQ                                 ,
   TEST_RING_CFM                                      ,
   TEST_STOP_RING_CFM                                 ,
   TEST_LINE_CURRENT_SET_REQ                          ,
   TEST_LINE_CURRENT_SET_CFM                          ,
   TEST_LINE_CURRENT_READ_REQ                         ,
   TEST_LINE_CURRENT_READ_CFM                         , 
   TEST_LINE_IMPEDANCE_SET_REQ                        ,
   TEST_LINE_IMPEDANCE_SET_CFM                        ,
   TEST_LINE_IMPEDANCE_READ_REQ                       ,
   TEST_LINE_IMPEDANCE_READ_CFM                       ,
   TEST_METERING_REQ                                  ,
   TEST_STOP_METERING_REQ                             ,
   TEST_METERING_CFM                                  ,
   TEST_STOP_METERING_CFM                             ,
   TEST_LINE_HOOK_REQ                                 ,
   TEST_LINE_HOOK_CFM                                 ,
   TEST_LINE_POLARITY_REQ                             ,
   TEST_LINE_POLARITY_CFM                             ,
   TEST_LINE_TX_LEVEL_REQ                             ,
   TEST_LINE_TX_LEVEL_CFM                             ,
   TEST_LINE_RX_LEVEL_REQ                             ,
   TEST_LINE_RX_LEVEL_CFM                             ,
   TEST_PCM_TO_IOM_ENABLE_REQ                         ,
   TEST_PCM_TO_IOM_ENABLE_CFM                         ,
   TEST_PCM_TO_IOM_DISABLE_REQ                        ,
   TEST_PCM_TO_IOM_DISABLE_CFM                        ,

   // ATS-D TESTS ( Reserved Base=0xf600

   // ##########################################################################
   //                       END PTA TEST PRIMITIVES
   // ##########################################################################

   // Test primitiver
  // Test primitiver, not part of the official interface.
  TEST_MAC_BURST_FIG31_REQ=0xf700,
  TEST_MAC_BURST_FIG31_CFM,
  TEST_MAC_TOGGLE_REQ,
  TEST_MAC_TOGGLE_CFM,
  TEST_MAC_BUILD_IDLE_SLOT_REQ,
  TEST_MAC_BUILD_IDLE_SLOT_CFM,
  TEST_MAC_ACTIVATE_REQ,
  TEST_MAC_ACTIVATE_CFM,
  TEST_MAC_DEACTIVATE_REQ,
  TEST_MAC_DEACTIVATE_CFM,

  TEST_GET_COMPILATION_TIME_REQ,
  TEST_GET_COMPILATION_TIME_CFM,

   TEST_SET_ENROLL_BIT_REQ,
   TEST_SET_ENROLL_BIT_CFM,
   TEST_CLEAR_ENROLL_BIT_REQ,
   TEST_CLEAR_ENROLL_BIT_CFM,

   TEST_ISAC_TEST_REQ=0xf800,
   TEST_ISAC_TEST_CFM=0xf801,

   TEST_CMD_REQ=0xf8fe,
   TEST_CMD_CFM=0xf8ff,

   TEST_COMMUNICATION_PING_PONG=0xff00,

   NUMBER_OF_PRIMITIVES
} ENUM16(DebugPrimitiveType);


typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
} recSendMailP0Type;

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bParm1;
} recSendMailP1Type;

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bParm1;
   uint8               bParm2;
} recSendMailP2Type;

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bParm1;
   uint8               bParm2;
   uint8               bParm3;
} recSendMailP3Type;

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bParm1;
   uint8               bParm2;
   uint8               bParm3;
   uint8               bParm4;
} recSendMailP4Type;

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bParm1;
   uint8               bParm2;
   uint8               bParm3;
   uint8               bParm4;
   uint8               bParm5;
} recSendMailP5Type;

typedef struct
{
  DebugPrimitiveType   PrimitiveIdentifier;
  uint8                bData[20];
} recDataType;

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bProgramId;
   uint8               bTaskId;
   uint16              wAdress;
   uint8               bNumberOfBytes;
   uint8               bData[50];
} recEepromWriteDataReqType;

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bProgramId;
   uint8               bTaskId;
   uint16              wAdress;
   uint8               bNumberOfBytes;
} recEepromReadDataReqType;

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint16              wAdress;
   uint8               bNumberOfBytes;
   uint8               bData[50];
} recEepromReadDataCfmType;

/**********************************************************************/

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bProgramId;
   uint8               bTaskId;
   uint8               bDownloadData[256];
} recMailTraceDownloadOptionsReqType;

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bTraceCounter;
   uint8               bSender;
   uint8               bReciever;
   uint16              wLength;
   uint8               bData[1];
} recMailTraceMailIndType;

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bTraceCounter;
   uint8               bTaskId;
   uint16              bTimerId;
   uint16              wTimer;
} recMailTraceStartTimerIndType;

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bTraceCounter;
   uint8               bTaskId;
   uint16              bTimerId;
} recMailTraceStopTimerIndType;

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8 bTaskId;
   uint8 bLength;
   uint8 bData[1];
} recMailTraceCommentIndType;

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bTraceCounter;
   uint8               bChannel;
   uint8               bFrameNumber;
   uint8               bMultiFrameNumber[3];
   uint8               bCrc;
   uint8               bRssi;
   uint8               bData[6];
} recATraceMailIndType;

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bTraceCounter;
   uint8               bChannel;
   uint8               bFrameNumber;
   uint8               bMultiFrameNumber[3];
   uint8               bCrc;
   uint8               bRssi;
   uint8               bAfieldData[6];
   uint8               bBfieldData[32];
} recATraceBfieldMailIndType;

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bTraceCounter;
   uint8               bChannel;
   uint8               bFrameNumber;
   uint8               bMultiFrameNumber[3];
   uint8               bCrc;
   uint8               bRssi;
   uint8               bAfieldData[6];
   uint8               bBfieldData[80];
   uint8               bCrc2;
} recATraceBfieldDoubleMailIndType;

/**********************************************************************/

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bProgramId;
   uint8               bTaskId;
   uint8               bLength;
   uint8               bMask;
} recBmcTraceStartReqType;

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bTraceCounter;
   uint8               bTraceData[1];
} recBmcTraceMailIndType;

/**********************************************************************/

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bTestedTask;
   uint8               bModulTstProgramId;
   uint8               bNumberOfInterfaces;
   uint8               bInterfaces[1];
} recModulTstStartType;

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bLength;
   DebugPrimitiveType  LogPrimitive;
   uint8               bData[50];
} recModulTstMailType;


/**********************************************************************/

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint32              lAdress;
   uint8               bNumberOfBytes;
   uint8               bData[1];
} recMemoryWriteReqType;

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bProgramId;
   uint8               bTaskId;
   uint32              lAdress;
   uint8               bNumberOfBytes;
} recMemoryReadReqType;

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint32              lAdress;
   uint8               bNumberOfBytes;
   uint8               bData[50];
} recMemoryReadCfmType;

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bProgramId;
   uint8               bTaskId;
   uint32              lAdress;
   uint32              lLength;
   uint8               bValue;
} recMemoryMemsetReqType;



typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8 bTaskId;
   uint8 bLength;
   uint8 bData[1];
} recPrintfIndType;

#define TEST_SW_VERSION_SIZE 42

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               swVerBankA[TEST_SW_VERSION_SIZE];
   uint8               swVerBankB[TEST_SW_VERSION_SIZE];
} TestSwVersionCfmType;

typedef struct                             //Mailstruct for MAC_SEARCH_REQ
{                                          //APP -> MAC
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bProgramId;
   uint8               bTaskId;
   uint8               bWantedParkArr[5];
   uint8               bWantedPli;
}recTestPpMacStartReqType;

typedef struct
{
   DebugPrimitiveType PrimitiveIdentifier;
   uint8              bFreqSlot;
   uint8              bRfpiArr[5];
} recTestPpMacStartCfmType;


typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bProgramId;
   uint8               bTaskId;
} recGetCompilationTimeReqType;

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8 bTaskId;
   uint8 bDateArr[11];
   uint8 bTimeArr[8];
} recGetCompilationTimeCfmType;

typedef struct
{
   DebugPrimitiveType  PrimitiveIdentifier;
   uint8               bProgramId;
   uint8               bTaskId;
   uint8               bAntenna;   // 0 = PP, 1 = FP1, 2 = FP2
} recSetAntennaReqType;


typedef union
{
   DebugPrimitiveType  PrimitiveIdentifier;

   recSendMailP0Type              recSendMailP0;
   recSendMailP1Type              recSendMailP1;
   recSendMailP2Type              recSendMailP2;
   recSendMailP3Type              recSendMailP3;

   recDataType                    recData;

   // debug.
   recEepromWriteDataReqType          recEepromWriteDataReq;
   recEepromReadDataReqType           recEepromReadReqData;
   recEepromReadDataCfmType           recEepromReadDataCfm;
   recMailTraceDownloadOptionsReqType recMailTraceDownloadOptionsReq;
   recMailTraceMailIndType            recMailTraceMailInd;
   recMailTraceStartTimerIndType      recMailTraceStartTimerInd;
   recMailTraceStopTimerIndType       recMailTraceStopTimerInd;
   recMailTraceCommentIndType         recMailTraceCommentInd;
   recModulTstStartType               recModulTstStart;
   recModulTstMailType                recModulTstMail;
   recMemoryWriteReqType              recMemoryWriteReq;
   recMemoryReadReqType               recMemoryReadReq;
   recMemoryReadCfmType               recMemoryReadCfm;
   recATraceMailIndType               recATraceMailInd;
   recATraceBfieldMailIndType         recATraceBfieldMailInd;
   recATraceBfieldDoubleMailIndType   recATraceBfieldDoubleMailInd;
   recBmcTraceStartReqType            recBmcTraceStartReq;
   recBmcTraceMailIndType             recBmcTraceMailInd;
   recPrintfIndType                   recPrintfInd;
   TestSwVersionCfmType               TestSwVersionCfm;
   recTestPpMacStartReqType           recTestPpMacStartReq;
   recTestPpMacStartCfmType           recTestPpMacStartCfm;
   recGetCompilationTimeReqType       recGetCompilationTimeReq;
   recGetCompilationTimeCfmType       recGetCompilationTimeCfm;
   recSetAntennaReqType               recSetAntennaReq;
} DebugMailType;




#endif

