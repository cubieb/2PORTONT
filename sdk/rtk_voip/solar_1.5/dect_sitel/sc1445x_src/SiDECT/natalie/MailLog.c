/****************************************************************************
*  Program/file: MailLog.cpp
*   
*  Copyright (C) by RTX TELECOM A/S, Denmark.
*  These computer program listings and specifications, are the property of 
*  RTX TELECOM A/S, Denmark and shall not be reproduced or copied or used in 
*  whole or in part without written permission from RTX TELECOM A/S, Denmark.
*
*  Programmer: LKA   
*
*  MODULE:
*  CONTROLLING DOCUMENT:
*  SYSTEM DEPENDENCIES:
*   
*   
*  DESCRIPTION:
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
$Modtime:   19 Sep 2008 14:02:40  $
$Archive:   J:/sw/Projects/450SIP/Fp/DectDemoApp/vcs/MailLog.c_v  $

*/


/****************************************************************************
*                               Include files
****************************************************************************/



#include <stdarg.h>
#include <string.h>

#include <stdio.h>

// typedef unsigned char   uint8;     // byte 0..255
// typedef   signed char   int8;      // byte -128..127
// typedef unsigned short  uint16;    // word 0..65535
// typedef   signed short  int16;     // 16-bit -32768..32767
// typedef unsigned long   uint32;    // 32-bit 0..4294967295
// typedef   signed long   int32;     // 32-bit -2147483648..2147483647
// typedef unsigned char   boolean;   // 0 is logical FALSE, !0 is logical TRUE

#define OFFSETOF(type, field) ((size_t)(&((type*)0)->field))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#include <sys/types.h>

#if 1
// #include <time.h>
//#include <conio.h>
// #include <stdlib.h>
// #include <sys/signal.h>
//vm
//#include <windows.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include<stdbool.h>
#include "std-def.h"
#endif
// #include "primitiv.h"	
// #include "api-intf.h"


#include "ISip450Api.h"
/****************************************************************************
*                              Macro definitions
****************************************************************************/
#define WS &str[CharCount]

/****************************************************************************
*                     Enumerations/Type definitions/Structs
****************************************************************************/


/****************************************************************************
*                            Global variables/const
****************************************************************************/


/****************************************************************************
*                            Local variables/const
****************************************************************************/
static char str[0xFFFF];
static uint16 IndentPos;
static uint16 CharCount;


const char *ApiStatusStr[20] =
{
  "AS_SUCCESS",
  "AS_NOT_CONNECTED",
  "AS_NOT_SUPPORTED",
  "AS_BAD_ARGUMENTS",
  "AS_BAD_ADDRESS",
  "AS_BAD_HANDLE",
  "AS_BAD_DATA",
  "AS_BAD_LENGTH",
  "AS_NO_MEMORY",
  "AS_NO_DEVICE",
  "AS_NO_DATA",
  "AS_NOT_READY",
  "AS_CANCELLED",
  "AS_BUSY",
  "AS_TIMEOUT",
  "AS_NOT_FOUND",
  "AS_DENIED",
  "AS_REJECTED",
  "AS_NO_RESOURCE",
  "AS_CALL_DROPPED",
};


	


/****************************************************************************
*                          Local Function prototypes
****************************************************************************/


/****************************************************************************
*                                Implementation
****************************************************************************/

void LogPrintf(const char* format, ...)
{
  int c;
  va_list argptr;
  va_start(argptr, format);
  c = vsprintf(WS, format, argptr);
  va_end(argptr);
  if (c > 0)
  {
    CharCount = (uint16)(CharCount + c);
  }
}

/***************************************************************************/
void LogTimestamp(void)
{
//   SYSTEMTIME Time;
//   GetLocalTime(&Time);
//   CharCount = 0;
//   LogPrintf("%02u:%02u:%02u:%03u ", Time.wHour, Time.wMinute, Time.wSecond, Time.wMilliseconds);
//   IndentPos = CharCount;
}

/***************************************************************************/
void LogData(uint8* Data, uint16 Size)
{
  while (Size--)
  {
    if (Size)
    {
      LogPrintf("%02X.", *Data++);
    }
    else
    {
      LogPrintf("%02X", *Data++);
    }
  }
}


/***************************************************************************/
void LogNewLine(void)
{
  *WS = '\n';
  CharCount++;
  memset(WS, ' ', IndentPos);
  CharCount = (uint16)(CharCount + IndentPos);
  *WS = '\0';
}

/***************************************************************************/
void LogComplete(void)
{
  if (CharCount)
  {
    *WS = '\0';
    IndentPos = 0;
    CharCount = 0;
  }
}

void LogStatus(ApiStatusType status)
{
  if (status < sizeof(ApiStatusStr)/sizeof(char*))
  {
    LogPrintf("Status=%s", ApiStatusStr[status]);
  }
  else
  {
    LogPrintf("Status=%02X", status);
  }
}


char *MailLog(uint8 *MailPtr, uint16 MailLength)
{
  Sip450ApiMailType *mp = (Sip450ApiMailType*)MailPtr;

  LogTimestamp();
  switch(mp->Primitive)
  {
      case API_FP_INIT_REQ:
         LogPrintf("API_FP_INIT_REQ ");
         #define M ((ApiFpInitReqType*)MailPtr)
         LogData(M->NvsData, 16);
         LogPrintf(" ...");
         #undef M
         break;

      case API_FP_INIT_CFM:
         LogPrintf("API_FP_INIT_CFM");
         break;
     case API_FP_NVS_UPDATE_IND:
         LogPrintf("API_FP_NVS_UPDATE_IND ");
         #define M ((ApiFpNvsUpdateIndType*)MailPtr)
         LogPrintf("SeqNo=0x%hx NvsOffset=0x%hx Length=0x%hx NvsData=",
                   M->SeqNo, M->NvsOffset, M->Length);
         LogData(M->NvsData, MIN(16, M->Length));
         if (M->Length > 16) LogPrintf(" ...");
         #undef M
         break;
         
     case API_FP_NVS_UPDATE_RES:
        LogPrintf("API_FP_NVS_UPDATE_RES ");
         #define M ((ApiFpNvsUpdateResType*)MailPtr)
         LogPrintf("SeqNo=0x%hx", M->SeqNo);
         #undef M
         break;
         
     case API_FP_RESET_REQ:
      LogPrintf("API_FP_RESET_REQ ");
      LogPrintf("No parameters");
      break;
    case API_FP_RESET_IND:
      LogPrintf("API_FP_RESET_IND ");
      #define M ((ApiFpResetIndType*)MailPtr)
      LogStatus(M->Status);
      #undef M
      break;
    case API_FP_GET_FW_VERSION_REQ:
      LogPrintf("API_FP_GET_FW_VERSION_REQ ");
      LogPrintf("No parameters");
      break;
    case API_FP_GET_FW_VERSION_CFM:
      LogPrintf("API_FP_GET_FW_VERSION_CFM ");
      #define M ((ApiFpGetFwVersionCfmType*)MailPtr)
      LogStatus(M->Status);
      LogPrintf(", ");
      LogPrintf("VersionHex = %d(0x%x), ",M->VersionHex,M->VersionHex);
      LogPrintf("LinkDate = ");
      LogData(M->LinkDate,5*sizeof(uint8));
      LogPrintf(", ");
      LogPrintf("DectType = 0x%x",M->DectType);
      #undef M
      break;
    case API_FP_GET_ID_REQ:
      LogPrintf("API_FP_GET_ID_REQ ");
      LogPrintf("No parameters");
      break;
    case API_FP_GET_ID_CFM:
      LogPrintf("API_FP_GET_ID_CFM ");
      #define M ((ApiFpGetIdCfmType*)MailPtr)
      LogStatus(M->Status);
      LogPrintf(", ");
      LogPrintf("Id = ");
      LogData(M->Id,5*sizeof(uint8));
      #undef M
      break;
    case API_FP_SET_ACCESS_CODE_REQ:
      LogPrintf("API_FP_SET_ACCESS_CODE_REQ ");
      #define M ((ApiFpSetAccessCodeReqType*)MailPtr)
      LogPrintf("Ac = ");
      LogData(M->Ac,2*sizeof(uint8));
      #undef M
      break;
    case API_FP_SET_ACCESS_CODE_CFM:
      LogPrintf("API_FP_SET_ACCESS_CODE_CFM ");
      #define M ((ApiFpSetAccessCodeCfmType*)MailPtr)
      LogStatus(M->Status);
      #undef M
      break;
    case API_FP_GET_ACCESS_CODE_REQ:
      LogPrintf("API_FP_GET_ACCESS_CODE_REQ ");
      LogPrintf("No parameters");
      break;
    case API_FP_GET_ACCESS_CODE_CFM:
      LogPrintf("API_FP_GET_ACCESS_CODE_CFM ");
      #define M ((ApiFpGetAccessCodeCfmType*)MailPtr)
      LogStatus(M->Status);
      LogPrintf(", ");
      LogPrintf("Ac = ");
      LogData(M->Ac,2*sizeof(uint8));
      #undef M
      break;
    case API_FP_GET_REGISTRATION_COUNT_REQ:
      LogPrintf("API_FP_GET_REGISTRATION_COUNT_REQ ");
      LogPrintf("No parameters");
      break;
    case API_FP_GET_REGISTRATION_COUNT_CFM:
      LogPrintf("API_FP_GET_REGISTRATION_COUNT_CFM ");
      #define M ((ApiFpGetRegistrationCountCfmType*)MailPtr)
      LogStatus(M->Status);
      LogPrintf(", ");
      LogPrintf("MaxNoHandsets = %d(0x%x), ",M->MaxNoHandsets,M->MaxNoHandsets);
      LogPrintf("HandsetIdLength = %d(0x%x), ",M->HandsetIdLength,M->HandsetIdLength);
      LogPrintf("HandsetId = ");
      LogData(M->HandsetId,M->HandsetIdLength);
      #undef M
      break;
    case API_FP_DELETE_REGISTRATION_REQ:
      LogPrintf("API_FP_DELETE_REGISTRATION_REQ ");
      #define M ((ApiFpDeleteRegistrationReqType*)MailPtr)
      LogPrintf("HandsetId = %d(0x%x)",M->HandsetId,M->HandsetId);
      #undef M
      break;
    case API_FP_DELETE_REGISTRATION_CFM:
      LogPrintf("API_FP_DELETE_REGISTRATION_CFM ");
      #define M ((ApiFpDeleteRegistrationCfmType*)MailPtr)
      LogStatus(M->Status);
      LogPrintf(", ");
      LogPrintf("HandsetId = %d(0x%x)",M->HandsetId,M->HandsetId);
      #undef M
      break;
    case API_FP_REGISTRATION_DELETED_IND:
      LogPrintf("API_FP_REGISTRATION_DELETED_IND ");
      #define M ((ApiFpRegistrationDeletedIndType*)MailPtr)
      LogPrintf("HandsetId = %d(0x%x)",M->HandsetId,M->HandsetId);
      #undef M
      break;
    case API_FP_SET_REGISTRATION_MODE_REQ:
      LogPrintf("API_FP_SET_REGISTRATION_MODE_REQ ");
      #define M ((ApiFpSetRegistrationModeReqType*)MailPtr)
      LogPrintf("RegistrationEnabled = %d(0x%x), ",M->RegistrationEnabled,M->RegistrationEnabled);
      LogPrintf("DeleteLastHandset = %d(0x%x)",M->DeleteLastHandset,M->DeleteLastHandset);
      #undef M
      break;
    case API_FP_SET_REGISTRATION_MODE_CFM:
      LogPrintf("API_FP_SET_REGISTRATION_MODE_CFM ");
      #define M ((ApiFpSetRegistrationModeCfmType*)MailPtr)
      LogStatus(M->Status);
      #undef M
      break;
    case API_FP_REGISTRATION_COMPLETE_IND:
      LogPrintf("API_FP_REGISTRATION_COMPLETE_IND ");
      #define M ((ApiFpRegistrationCompleteIndType*)MailPtr)
      LogStatus(M->Status);
      LogPrintf(", ");
      LogPrintf("HandsetId = %d(0x%x), ",M->HandsetId,M->HandsetId);
      LogPrintf("ModelId.MANIC = 0x%04x, ",M->ModelId.MANIC);
      LogPrintf("ModelId.MODIC = 0x%02x, ",M->ModelId.MODIC);
      LogPrintf("CodecListLength = %d(0x%x), ",M->CodecListLength,M->CodecListLength);
      LogPrintf("Data = ");
      LogData(M->Data,M->CodecListLength);
      #undef M
      break;
    case API_FP_HANDSET_PRESENT_IND:
      LogPrintf("API_FP_HANDSET_PRESENT_IND ");
      #define M ((ApiFpHandsetPresentIndType*)MailPtr)
      LogPrintf("HandsetId = %d(0x%x), ",M->HandsetId,M->HandsetId);
      LogPrintf("ModelId.MANIC = 0x%04x, ",M->ModelId.MANIC);
      LogPrintf("ModelId.MODIC = 0x%02x, ",M->ModelId.MODIC);
      LogPrintf("CodecListLength = %d(0x%x), ",M->CodecListLength,M->CodecListLength);
      LogPrintf("Data = ");
      LogData(M->Data,M->CodecListLength);
      #undef M
      break;
    case API_FP_SETUP_IND:
      LogPrintf("API_FP_SETUP_IND ");
      #define M ((ApiFpSetupIndType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);

      LogPrintf("DestinationId = %d(0x%x), ",M->DestinationId,M->DestinationId);
      LogPrintf("BasicService = 0x%x, ",M->BasicService);
      LogPrintf("CallClass = 0x%x, ",M->CallClass);
      LogPrintf("KeypadLength = %d(0x%x), ",M->KeypadLength,M->KeypadLength);
      LogPrintf("CalledNumberLength = %d(0x%x), ",M->CalledNumberLength,M->CalledNumberLength);
      LogPrintf("CallingNumberLength = %d(0x%x), ",M->CallingNumberLength,M->CallingNumberLength);
      LogPrintf("CodecListLength = %d(0x%x), ",M->CodecListLength,M->CodecListLength);
      LogPrintf("IwuLength = %d(0x%x), ",M->IwuLength,M->IwuLength);
      LogPrintf("ProprietaryLength = %d(0x%x), ",M->ProprietaryLength,M->ProprietaryLength);
      LogPrintf("Data = ");
      if (MailLength >= OFFSETOF(ApiFpSetupIndType, Data))
      {
        LogData(M->Data,MailLength - OFFSETOF(ApiFpSetupIndType, Data));
      }
      #undef M
      break;
    case API_FP_SETUP_RES:
      LogPrintf("API_FP_SETUP_RES ");
      #define M ((ApiFpSetupResType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogStatus(M->Status);
      LogPrintf(", ");
      LogPrintf("DestinationId = %d(0x%x)",M->DestinationId,M->DestinationId);
      #undef M
      break;
    case API_FP_SETUP_REQ:
      LogPrintf("API_FP_SETUP_REQ ");
      #define M ((ApiFpSetupReqType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogPrintf("BasicService = 0x%x, ",M->BasicService);
      LogPrintf("CallClass = 0x%x, ",M->CallClass);
      LogPrintf("SourceId = %d(0x%x), ",M->SourceId,M->SourceId);
      LogPrintf("Signal = 0x%x, ",M->Signal);
      LogPrintf("ClipLength = %d(0x%x), ",M->ClipLength,M->ClipLength);
      LogPrintf("CallingNumberLength = %d(0x%x), ",M->CallingNumberLength,M->CallingNumberLength);
      LogPrintf("CallingNamelength = %d(0x%x), ",M->CallingNameLength,M->CallingNameLength);
      LogPrintf("CodecListLength = %d(0x%x), ",M->CodecListLength,M->CodecListLength);
      LogPrintf("IwuLength = %d(0x%x), ",M->IwuLength,M->IwuLength);
      LogPrintf("ProprietaryLength = %d(0x%x), ",M->ProprietaryLength,M->ProprietaryLength);
      LogPrintf("Data = ");
      if (MailLength >= OFFSETOF(ApiFpSetupReqType, Data))
      {
        LogData(M->Data,MailLength - OFFSETOF(ApiFpSetupReqType, Data));
      }
      #undef M
      break;
    case API_FP_SETUP_CFM:
      LogPrintf("API_FP_SETUP_CFM ");
      #define M ((ApiFpSetupCfmType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogStatus(M->Status);
      LogPrintf(", ");
      LogPrintf("BasicService = 0x%x, ",M->BasicService);
      LogPrintf("CallClass = 0x%x, ",M->CallClass);
      LogPrintf("SourceId = %d(0x%x)",M->SourceId,M->SourceId);
      #undef M
      break;
    case API_FP_SETUP_ACK_REQ:
      LogPrintf("API_FP_SETUP_ACK_REQ ");
      #define M ((ApiFpSetupAckReqType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogPrintf("Signal = 0x%x, ",M->Signal);
      LogPrintf("ProgressInd = 0x%x, ",M->ProgressInd);
      LogPrintf("CodecListLength = %d(0x%x), ",M->CodecListLength,M->CodecListLength);
      LogPrintf("IwuLength = %d(0x%x), ",M->IwuLength,M->IwuLength);
      LogPrintf("ProprietaryLength = %d(0x%x), ",M->ProprietaryLength,M->ProprietaryLength);
      LogPrintf("Data = ");
      if (MailLength >= OFFSETOF(ApiFpSetupAckReqType, Data))
      {
        LogData(M->Data,MailLength - OFFSETOF(ApiFpSetupAckReqType, Data));
      }
      #undef M
      break;
    case API_FP_SETUP_ACK_CFM:
      LogPrintf("API_FP_SETUP_ACK_CFM ");
      #define M ((ApiFpSetupAckCfmType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogStatus(M->Status);
      #undef M
      break;
    case API_FP_ALERT_IND:
      LogPrintf("API_FP_ALERT_IND ");
      #define M ((ApiFpAlertIndType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogPrintf("CodecListLength = %d(0x%x), ",M->CodecListLength,M->CodecListLength);
      LogPrintf("Data = ");
      if (MailLength >= OFFSETOF(ApiFpAlertIndType, Data))
      {
        LogData(M->Data,MailLength - OFFSETOF(ApiFpAlertIndType, Data));
      }
      #undef M
      break;
    case API_FP_ALERT_REQ:
      LogPrintf("API_FP_ALERT_REQ ");
      #define M ((ApiFpAlertReqType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogPrintf("ProgressInd = 0x%x, ",M->ProgressInd);
      LogPrintf("Signal = 0x%x, ",M->Signal);
      LogPrintf("CodecListLength = %d(0x%x), ",M->CodecListLength,M->CodecListLength);
      LogPrintf("Data = ");
      if (MailLength >= OFFSETOF(ApiFpAlertReqType, Data))
      {
        LogData(M->Data,MailLength - OFFSETOF(ApiFpAlertReqType, Data));
      }
      #undef M
      break;
    case API_FP_ALERT_CFM:
      LogPrintf("API_FP_ALERT_CFM ");
      #define M ((ApiFpAlertCfmType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogStatus(M->Status);
      #undef M
      break;
    case API_FP_CONNECT_IND:
      LogPrintf("API_FP_CONNECT_IND ");
      #define M ((ApiFpConnectIndType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogPrintf("AdpcmChannel = %x, ",M->AdpcmChannel);
      LogPrintf("CodecListLength = %d(0x%x), ",M->CodecListLength,M->CodecListLength);
      LogPrintf("Data = ");
      if (MailLength >= OFFSETOF(ApiFpConnectIndType, Data))
      {
        LogData(M->Data,MailLength - OFFSETOF(ApiFpConnectIndType, Data));
      }
      #undef M
      break;
    case API_FP_CONNECT_RES:
      LogPrintf("API_FP_CONNECT_RES ");
      #define M ((ApiFpConnectResType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogStatus(M->Status);
      #undef M
      break;
    case API_FP_CONNECT_REQ:
      LogPrintf("API_FP_CONNECT_REQ ");
      #define M ((ApiFpConnectReqType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogPrintf("CodecListLength = %d(0x%x), ",M->CodecListLength,M->CodecListLength);
      LogPrintf("Data = ");
      if (MailLength >= OFFSETOF(ApiFpConnectReqType, Data))
      {
        LogData(M->Data,MailLength - OFFSETOF(ApiFpConnectReqType, Data));
      }
      #undef M
      break;
    case API_FP_CONNECT_CFM:
      LogPrintf("API_FP_CONNECT_CFM ");
      #define M ((ApiFpConnectCfmType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogPrintf("AdpcmChannel = %x, ",M->AdpcmChannel);
      LogStatus(M->Status);
      #undef M
      break;
    case API_FP_SIGNAL_REQ:
      LogPrintf("API_FP_SIGNAL_REQ ");
      #define M ((ApiFpSignalReqType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogPrintf("Signal = 0x%x",M->Signal);
      #undef M
      break;
    case API_FP_SIGNAL_CFM:
      LogPrintf("API_FP_SIGNAL_CFM ");
      #define M ((ApiFpSignalCfmType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogStatus(M->Status);
      #undef M
      break;
    case API_FP_SEND_CLIP_REQ:
      LogPrintf("API_FP_SEND_CLIP_REQ ");
      #define M ((ApiFpSendClipReqType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogPrintf("ClipLength = %d(0x%x), ",M->ClipLength,M->ClipLength);
      LogPrintf("CallingNumberLength = %d(0x%x), ",M->CallingNumberLength,M->CallingNumberLength);
      LogPrintf("CallingNamelength = %d(0x%x), ",M->CallingNameLength,M->CallingNameLength);
      LogPrintf("Data = ");
      if (MailLength >= OFFSETOF(ApiFpSendClipReqType, Data))
      {
        LogData(M->Data,MailLength - OFFSETOF(ApiFpSendClipReqType, Data));
      }
      #undef M
      break;
    case API_FP_SEND_CLIP_CFM:
      LogPrintf("API_FP_SEND_CLIP_CFM ");
      #define M ((ApiFpSendClipCfmType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogStatus(M->Status);
      #undef M
      break;
    case API_FP_RELEASE_IND:
      LogPrintf("API_FP_RELEASE_IND ");
      #define M ((ApiFpReleaseIndType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogPrintf("Reason = 0x%x",M->Reason);
      #undef M
      break;
    case API_FP_RELEASE_RES:
      LogPrintf("API_FP_RELEASE_RES ");
      #define M ((ApiFpReleaseResType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogStatus(M->Status);
      #undef M
      break;
    case API_FP_RELEASE_REQ:
      LogPrintf("API_FP_RELEASE_REQ ");
      #define M ((ApiFpReleaseReqType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogPrintf("Reason = 0x%x",M->Reason);
      #undef M
      break;
    case API_FP_RELEASE_CFM:
      LogPrintf("API_FP_RELEASE_CFM ");
      #define M ((ApiFpReleaseCfmType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogStatus(M->Status);
      #undef M
      break;
    case API_FP_REJECT_IND:
      LogPrintf("API_FP_REJECT_IND ");
      #define M ((ApiFpRejectIndType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogPrintf("Reason = 0x%x",M->Reason);
      #undef M
      break;
    case API_FP_MULTIKEYPAD_IND:
      LogPrintf("API_FP_MULTIKEYPAD_IND ");
      #define M ((ApiFpMultikeypadIndType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogPrintf("KeypadLength = %d(0x%x), ",M->KeypadLength,M->KeypadLength);
      LogPrintf("Data = ");
      if (MailLength >= OFFSETOF(ApiFpMultikeypadIndType, Data))
      {
        LogData(M->Data,MailLength - OFFSETOF(ApiFpMultikeypadIndType, Data));
      }
      #undef M
      break;
    case API_FP_CALL_PROC_REQ:
      LogPrintf("API_FP_CALL_PROC_REQ ");
      #define M ((ApiFpCallProcReqType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogPrintf("ProgressInd = 0x%x, ",M->ProgressInd);
      LogPrintf("Signal = 0x%x, ",M->Signal);
      LogPrintf("CodecListLength = %d(0x%x), ",M->CodecListLength,M->CodecListLength);
      LogPrintf("Data = ");
      if (MailLength >= OFFSETOF(ApiFpCallProcReqType, Data))
      {
        LogData(M->Data,MailLength - OFFSETOF(ApiFpCallProcReqType, Data));
      }
      #undef M
      break;
    case API_FP_CALL_PROC_CFM:
      LogPrintf("API_FP_CALL_PROC_CFM ");
      #define M ((ApiFpCallProcCfmType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogStatus(M->Status);
      #undef M
      break;
    case API_FP_IWU_TO_IWU_REQ:
      LogPrintf("API_FP_IWU_TO_IWU_REQ ");
      #define M ((ApiFpIwuToIwuReqType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogPrintf("IwuLength = %d(0x%x), ",M->IwuLength,M->IwuLength);
      LogPrintf("Data = ");
      if (MailLength >= OFFSETOF(ApiFpIwuToIwuReqType, Data))
      {
        LogData(M->Data,MailLength - OFFSETOF(ApiFpIwuToIwuReqType, Data));
      }
      #undef M
      break;
    case API_FP_IWU_TO_IWU_CFM:
      LogPrintf("API_FP_IWU_TO_IWU_CFM ");
      #define M ((ApiFpIwuToIwuCfmType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogStatus(M->Status);
      #undef M
      break;
    case API_FP_IWU_TO_IWU_IND:
      LogPrintf("API_FP_IWU_TO_IWU_IND ");
      #define M ((ApiFpIwuToIwuIndType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogPrintf("IwuLength = %d(0x%x), ",M->IwuLength,M->IwuLength);
      LogPrintf("Data = ");
      if (MailLength >= OFFSETOF(ApiFpIwuToIwuIndType, Data))
      {
        LogData(M->Data,MailLength - OFFSETOF(ApiFpIwuToIwuIndType, Data));
      }
      #undef M
      break;
    case API_FP_PROPRIETARY_REQ:
      LogPrintf("API_FP_PROPRIETARY_REQ ");
      #define M ((ApiFpProprietaryReqType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogPrintf("ProprietaryLength = %d(0x%x), ",M->ProprietaryLength,M->ProprietaryLength);
      LogPrintf("Data = ");
      if (MailLength >= OFFSETOF(ApiFpProprietaryReqType, Data))
      {
        LogData(M->Data,MailLength - OFFSETOF(ApiFpProprietaryReqType, Data));
      }
      #undef M
      break;
    case API_FP_PROPRIETARY_CFM:
      LogPrintf("API_FP_PROPRIETARY_CFM ");
      #define M ((ApiFpProprietaryCfmType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogStatus(M->Status);
      #undef M
      break;
    case API_FP_PROPRIETARY_IND:
      LogPrintf("API_FP_PROPRIETARY_IND ");
      #define M ((ApiFpProprietaryIndType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogPrintf("ProprietaryLength = %d(0x%x), ",M->ProprietaryLength,M->ProprietaryLength);
      LogPrintf("Data = ");
      if (MailLength >= OFFSETOF(ApiFpProprietaryIndType, Data))
      {
        LogData(M->Data,MailLength - OFFSETOF(ApiFpProprietaryIndType, Data));
      }
      #undef M
      break;
    case API_FP_MODIFY_CODEC_REQ:
      LogPrintf("API_FP_MODIFY_CODEC_REQ ");
      #define M ((ApiFpModifyCodecReqType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogPrintf("CodecListLength = %d(0x%x), ",M->CodecListLength,M->CodecListLength);
      LogPrintf("Data = ");
      if (MailLength >= OFFSETOF(ApiFpModifyCodecReqType, Data))
      {
        LogData(M->Data,MailLength - OFFSETOF(ApiFpModifyCodecReqType, Data));
      }
      #undef M
      break;
    case API_FP_MODIFY_CODEC_CFM:
      LogPrintf("API_FP_MODIFY_CODEC_CFM ");
      #define M ((ApiFpModifyCodecCfmType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogStatus(M->Status);
      #undef M
      break;
    case API_FP_MODIFY_CODEC_IND:
      LogPrintf("API_FP_MODIFY_CODEC_IND ");
      #define M ((ApiFpModifyCodecIndType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogPrintf("CodecListLength = %d(0x%x), ",M->CodecListLength,M->CodecListLength);
      LogPrintf("Data = ");
      if (MailLength >= OFFSETOF(ApiFpModifyCodecIndType, Data))
      {
        LogData(M->Data,MailLength - OFFSETOF(ApiFpModifyCodecIndType, Data));
      }
      #undef M
      break;
    case API_FP_MODIFY_CODEC_RES:
      LogPrintf("API_FP_MODIFY_CODEC_RES ");
      #define M ((ApiFpModifyCodecResType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogStatus(M->Status);
      #undef M
      break;
    case API_FP_CODEC_INFO_IND:
      LogPrintf("API_FP_CODEC_INFO_IND ");
      #define M ((ApiFpCodecInfoIndType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogPrintf("CodecListLength = %d(0x%x), ",M->CodecListLength,M->CodecListLength);
      LogPrintf("Data = ");
      if (MailLength >= OFFSETOF(ApiFpCodecInfoIndType, Data))
      {
        LogData(M->Data,MailLength - OFFSETOF(ApiFpCodecInfoIndType, Data));
      }
      #undef M
      break;
    case API_FP_CALLED_NUMBER_IND:
      LogPrintf("API_FP_CALLED_NUMBER_IND ");
      #define M ((ApiFpCalledNumberIndType*)MailPtr)
      LogPrintf("HandsetId = 0x%x, ",M->HandsetId);
      LogPrintf("CalledNumberLength = %d(0x%x), ",M->CalledNumberLength,M->CalledNumberLength);
      LogPrintf("Data = ");
      if (MailLength >= OFFSETOF(ApiFpCalledNumberIndType, Data))
      {
        LogData(M->Data,MailLength - OFFSETOF(ApiFpCalledNumberIndType, Data));
      }
      #undef M
      break;

    default:
      LogPrintf("Unknown: ");
      LogData(MailPtr, MailLength);
      LogComplete();
      break;
  }
  LogComplete();
  return str;
}

