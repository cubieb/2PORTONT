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
 * File:		si_call_log_api.c
 * Purpose:		
 * Created:	23/10/2007
 * By:			KF
 * Country:	Greece
 *
 *-----------------------------------------------------------------------------
 */

/*========================== Include files ==================================*/
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <si_print_api.h>

#include "si_call_log_api.h"

/*========================== Local macro definitions ========================*/


/*========================== Global definitions =============================*/


/*========================== Local function prototypes ======================*/


/*========================== Local data definitions =========================*/

call missed_call[MAX_NUM_OF_MISSED_CALLS];
call received_call[MAX_NUM_OF_RECEIVED_CALLS];
call dialed_call[MAX_NUM_OF_DIALED_CALLS];

int num_of_missed_calls = 0;
int num_of_received_calls = 0;
int num_of_dialed_calls = 0;

int MissedCallsCurrentPosition = -1;
int ReceivedCallsCurrentPosition = -1;
int DialedCallsCurrentPosition = -1;

int CallLogMajorVersion = 0;
int CallLogMinorVersion = 1;

char FileMISSED_CALLS[] = MISSED_CALLS_FILEPATH;
char FileRECEIVED_CALLS[] = RECEIVED_CALLS_FILEPATH;
char FileDIALED_CALLS[] = DIALED_CALLS_FILEPATH;

CallLogErrorCode CallLogErrorCodes[] = {
	{ERROR_CallLog_MissedCallsFileNotFound, "Missed Calls file not found"},
	{ERROR_CallLog_ReceivedCallsFileNotFound, "Received Calls file not found"},
	{ERROR_CallLog_DialedCallsFileNotFound, "Dialed Calls file not found"},
	{ERROR_CallLog_NoMissedCall, "No Missed Call"},
	{ERROR_CallLog_NoReceivedCall, "No Received Call"},
	{ERROR_CallLog_NoDialedCall, "No Dialed Call"},
	{ERROR_CallLog_MissedCallNotFound, "Missed Call not found"},
	{ERROR_CallLog_ReceivedCallNotFound, "Received Call not found"},
	{ERROR_CallLog_DialedCallNotFound, "Dialed Call not found"},
};

/*========================== Function definitions ===========================*/
int calllog_LoadMissedCalls()
{
	FILE *fp;

	fp = fopen(FileMISSED_CALLS, "rb");
	if(fp == NULL) {
		num_of_missed_calls = 0;
	  	return ERROR_CallLog_MissedCallsFileNotFound;
	}

	fread(&CallLogMajorVersion, sizeof(int), 1, fp);
	fread(&CallLogMinorVersion, sizeof(int), 1, fp);
	fread(&num_of_missed_calls, sizeof(int), 1, fp);
	if(num_of_missed_calls > MAX_NUM_OF_RECEIVED_CALLS)
	{
		 
		num_of_missed_calls=0;
	}else
		fread(missed_call, sizeof(call), num_of_missed_calls, fp);

	fclose(fp);
	  
	return 0;
}


int calllog_LoadReceivedCalls()
{
	FILE *fp;
  
  	fp = fopen(FileRECEIVED_CALLS, "rb");
  	if(fp == NULL) {
		  num_of_received_calls = 0;
    	return ERROR_CallLog_ReceivedCallsFileNotFound;
  	}

	fread(&CallLogMajorVersion, sizeof(int), 1, fp);
	fread(&CallLogMinorVersion, sizeof(int), 1, fp);
  fread(&num_of_received_calls, sizeof(int), 1, fp);
 	fread(received_call, sizeof(call), num_of_received_calls, fp);
  
 	fclose(fp);
  
  return 0;
}


int calllog_LoadDialedCalls()
{
	FILE *fp;
  
  fp = fopen(FileDIALED_CALLS, "rb");
  if(fp == NULL) {
  	num_of_dialed_calls = 0;
   	return ERROR_CallLog_DialedCallsFileNotFound;
  }

	fread(&CallLogMajorVersion, sizeof(int), 1, fp);
	fread(&CallLogMinorVersion, sizeof(int), 1, fp);
  fread(&num_of_dialed_calls, sizeof(int), 1, fp);
  fread(dialed_call, sizeof(call), num_of_dialed_calls, fp);
  
  fclose(fp);

  return 0;
}


int calllog_StoreMissedCalls()
{
	FILE *fp;
  
	fp = fopen(FileMISSED_CALLS, "wb");
	if(fp == NULL)
		return ERROR_CallLog_MissedCallsFileNotFound;

  fwrite(&CallLogMajorVersion, sizeof(int), 1, fp);
  fwrite(&CallLogMinorVersion, sizeof(int), 1, fp);
	fwrite(&num_of_missed_calls, sizeof(int), 1, fp);
	fwrite(missed_call, sizeof(call), num_of_missed_calls, fp);
  
	fclose(fp);
  
  return 0;
}


int calllog_StoreReceivedCalls()
{
	FILE *fp;
  
	fp = fopen(FileRECEIVED_CALLS, "wb");
	if(fp == NULL)
		return ERROR_CallLog_ReceivedCallsFileNotFound;

  fwrite(&CallLogMajorVersion, sizeof(int), 1, fp);
  fwrite(&CallLogMinorVersion, sizeof(int), 1, fp);
	fwrite(&num_of_received_calls, sizeof(int), 1, fp);
	fwrite(received_call, sizeof(call), num_of_received_calls, fp);
  
	fclose(fp);
  
  return 0;
}


int calllog_StoreDialedCalls()
{
	FILE *fp;
  
	fp = fopen(FileDIALED_CALLS, "wb");
	if(fp == NULL)
		return ERROR_CallLog_DialedCallsFileNotFound;

  fwrite(&CallLogMajorVersion, sizeof(int), 1, fp);
  fwrite(&CallLogMinorVersion, sizeof(int), 1, fp);
	fwrite(&num_of_dialed_calls, sizeof(int), 1, fp);
	fwrite(dialed_call, sizeof(call), num_of_dialed_calls, fp);
  
	fclose(fp);
  
  return 0;
}


int calllog_AddMissedCall(call *call_to_add)
{
	int i;

	 

	for(i = num_of_dialed_calls - 1 ; i >= 0 ; i--) {
		if(i == MAX_NUM_OF_MISSED_CALLS - 1)
			continue;
	  

		memcpy(&missed_call[i + 1], &missed_call[i], sizeof(call));
		missed_call[i + 1].index ++;
	}
   
	memcpy(&missed_call[0], call_to_add, sizeof(call));
	missed_call[0].index = 0;
 	num_of_missed_calls++;
	if(num_of_missed_calls > MAX_NUM_OF_MISSED_CALLS)
		num_of_missed_calls = MAX_NUM_OF_MISSED_CALLS;
		  
 	return 0;
}


int calllog_AddReceivedCall(call *call_to_add)
{
	int i;

  for(i = num_of_received_calls - 1 ; i >= 0 ; i--) {
		if(i >= (MAX_NUM_OF_RECEIVED_CALLS - 1))
			break;
		memcpy(&received_call[i + 1], &received_call[i], sizeof(call));
		received_call[i + 1].index ++;
	}

  memcpy(&received_call[0], call_to_add, sizeof(call));
	received_call[0].index = 0;

	num_of_received_calls++;
	if(num_of_received_calls > MAX_NUM_OF_RECEIVED_CALLS)
		num_of_received_calls = MAX_NUM_OF_RECEIVED_CALLS;
		  
 	return 0;
}


int calllog_AddDialedCall(call *call_to_add)
{
	int i;

  for(i = num_of_dialed_calls - 1 ; i >= 0 ; i--) {
		if(i == MAX_NUM_OF_DIALED_CALLS - 1)
			continue;

    memcpy(&dialed_call[i + 1], &dialed_call[i], sizeof(call));
		dialed_call[i + 1].index ++;
	}

  memcpy(&dialed_call[0], call_to_add, sizeof(call));
	dialed_call[0].index = 0;

	num_of_dialed_calls++;
	if(num_of_dialed_calls > MAX_NUM_OF_DIALED_CALLS)
		num_of_dialed_calls = MAX_NUM_OF_DIALED_CALLS;
		  
 	return 0;
}


int calllog_DeleteMissedCall(int index)
{
	int i;
  
  if(num_of_missed_calls == 0)
		return ERROR_CallLog_NoMissedCall;
  
  if((index < 0) || (index > num_of_missed_calls - 1))
		return ERROR_CallLog_MissedCallNotFound;
  
  for(i = index ; i < num_of_missed_calls - 1 ; i++) {
   	memcpy(&missed_call[i], &missed_call[i + 1], sizeof(call));
		missed_call[i].index --;
	}
    
  num_of_missed_calls--; 

  return 0;
}


int calllog_DeleteReceivedCall(int index)
{
	int i;
  
  if(num_of_received_calls == 0)
		return ERROR_CallLog_NoReceivedCall;
  
  if((index < 0) || (index > num_of_received_calls - 1))
		return ERROR_CallLog_ReceivedCallNotFound;
  
  for(i = index ; i < num_of_received_calls - 1 ; i++) {
   	memcpy(&received_call[i], &received_call[i + 1], sizeof(call));
		received_call[i].index --;
	}
    
  num_of_received_calls--; 

  return 0;
}


int calllog_DeleteDialedCall(int index)
{
	int i;
  
  if(num_of_dialed_calls == 0)
	  return ERROR_CallLog_NoDialedCall;
  
  if((index < 0) || (index > num_of_dialed_calls - 1))
		return ERROR_CallLog_DialedCallNotFound;
  
  for(i = index ; i < num_of_dialed_calls - 1 ; i++) {
   	memcpy(&dialed_call[i], &dialed_call[i + 1], sizeof(call));
		dialed_call[i].index --;
	}
    
  num_of_dialed_calls--; 

  return 0;
}


int calllog_DeleteAllMissedCalls()
{
  num_of_missed_calls = 0; 

	return 0;
}


int calllog_DeleteAllReceivedCalls()
{
  num_of_received_calls = 0; 

  return 0;
}


int calllog_DeleteAllDialedCalls()
{
  num_of_dialed_calls = 0; 

  return 0;
}


int calllog_DeleteAllCalls()
{
  num_of_missed_calls = num_of_received_calls = num_of_dialed_calls = 0; 

  return 0;
}


call *calllog_GotoFirst(char call_type)
{
	if(call_type == MISSED) {
		if (!num_of_missed_calls)
			return (call *)NULL;

		MissedCallsCurrentPosition = 0;

	  return (call *)&missed_call[MissedCallsCurrentPosition];
	}else if(call_type == RECEIVED) {
		if (!num_of_received_calls)
			return (call *)NULL;

		ReceivedCallsCurrentPosition = 0;

		return (call *)&received_call[ReceivedCallsCurrentPosition];
	}else if(call_type == DIALED) {
		if (!num_of_dialed_calls)
			return (call *)NULL;

		DialedCallsCurrentPosition = 0;

		return (call *)&dialed_call[DialedCallsCurrentPosition];
	}else
		return(call *)NULL;
}

call *calllog_ReturnCurrent(char call_type)
{
	if(call_type == MISSED) {
		if (!num_of_missed_calls)
			return (call *)NULL;

		return (call *)&missed_call[MissedCallsCurrentPosition];
	}else if(call_type == RECEIVED) {
		if (!num_of_received_calls)
			return (call *)NULL;

		return (call *)&received_call[ReceivedCallsCurrentPosition];
	}else if(call_type == DIALED) {
		if (!num_of_dialed_calls)
			return (call *)NULL;

		return (call *)&dialed_call[DialedCallsCurrentPosition];
	}else
		return(call *)NULL;
}


call *calllog_GotoLast(char call_type)
{
	if(call_type == MISSED) {
		if (!num_of_missed_calls)
			return (call *)NULL;

		MissedCallsCurrentPosition = num_of_missed_calls - 1;

		return (call *)&missed_call[MissedCallsCurrentPosition];
	}else if(call_type == RECEIVED) {
		if (!num_of_received_calls)
			return (call *)NULL;

		ReceivedCallsCurrentPosition = num_of_received_calls - 1;

		return (call *)&received_call[ReceivedCallsCurrentPosition];
	}else if(call_type == DIALED) {
		if (!num_of_dialed_calls)
			return (call *)NULL;

		DialedCallsCurrentPosition = num_of_dialed_calls - 1;

		return (call *)&dialed_call[DialedCallsCurrentPosition];
	}else
		return(call *)NULL;
}


call *calllog_GotoNext(char call_type)
{
	if(call_type == MISSED) {
		if (!num_of_missed_calls)
			return (call *)NULL;

		MissedCallsCurrentPosition ++;
		if((MissedCallsCurrentPosition < 0) || (MissedCallsCurrentPosition > num_of_missed_calls - 1))
			MissedCallsCurrentPosition = 0;

		return (call *)&missed_call[MissedCallsCurrentPosition];
	}else if(call_type == RECEIVED) {
		if (!num_of_received_calls)
			return (call *)NULL;

		ReceivedCallsCurrentPosition ++;
		if((ReceivedCallsCurrentPosition < 0) || (ReceivedCallsCurrentPosition > num_of_received_calls - 1))
			ReceivedCallsCurrentPosition = 0;

		return (call *)&received_call[ReceivedCallsCurrentPosition];
	}else if(call_type == DIALED) {
		if (!num_of_dialed_calls)
			return (call *)NULL;

		DialedCallsCurrentPosition ++;
		if((DialedCallsCurrentPosition < 0) || (DialedCallsCurrentPosition > num_of_dialed_calls - 1))
			DialedCallsCurrentPosition = 0;

		return (call *)&dialed_call[DialedCallsCurrentPosition];
	}else
		return(call *)NULL;
}


call *calllog_GotoPrevious(char call_type)
{
	if(call_type == MISSED) {
		if (!num_of_missed_calls)
			return (call *)NULL;

		MissedCallsCurrentPosition --;
		if((MissedCallsCurrentPosition < 0) || (MissedCallsCurrentPosition > num_of_missed_calls - 1))
			MissedCallsCurrentPosition = num_of_missed_calls - 1;

		return (call *)&missed_call[MissedCallsCurrentPosition];
	}else if(call_type == RECEIVED) {
		if (!num_of_received_calls)
			return (call *)NULL;

		ReceivedCallsCurrentPosition --;
		if((ReceivedCallsCurrentPosition < 0) || (ReceivedCallsCurrentPosition > num_of_received_calls - 1))
			ReceivedCallsCurrentPosition = num_of_received_calls - 1;

		return (call *)&received_call[ReceivedCallsCurrentPosition];
	}else if(call_type == DIALED) {
		if (!num_of_dialed_calls)
			return (call *)NULL;

		DialedCallsCurrentPosition --;
		if((DialedCallsCurrentPosition < 0) || (DialedCallsCurrentPosition > num_of_dialed_calls - 1))
			DialedCallsCurrentPosition = num_of_dialed_calls - 1;

		return (call *)&dialed_call[DialedCallsCurrentPosition];
	}else
		return(call *)NULL;
}


int calllog_GetListLength(char call_list)
{
	if(call_list == MISSED)
		return num_of_missed_calls;
	else if(call_list == RECEIVED)
		return num_of_received_calls;
	else if(call_list == DIALED)
		return num_of_dialed_calls;
	else
		return -1;
}


char* calllog_GetErrorString(int errCode)
{
	int i;

	for(i = 0 ; CallLogErrorCodes[i].ErrorId ; i++) {
		if(CallLogErrorCodes[i].ErrorId == errCode)
      	return CallLogErrorCodes[i].ErrorString;
	}
    return (char*)NULL;
}
 
int cmd_calllog_add_record(char *number,  struct tm *now, char calllogtype)
{
	call mCallRecord;
	if (!number || !now) return -1;

	if (strlen(number)<=sizeof(mCallRecord.number))
		strcpy(mCallRecord.number, number);
	else strncpy(mCallRecord.number, number,sizeof(mCallRecord.number)-1);

	 

	mCallRecord.number[sizeof(mCallRecord.number)]='\0';

 	mCallRecord.year = (char)(now->tm_year-70);
	mCallRecord.month = (char)now->tm_mon+1;
 	mCallRecord.day = (char)now->tm_mday;
	mCallRecord.hour = now->tm_hour;
	mCallRecord.minute = now->tm_min;
	mCallRecord.second = 0;
  
	return cmd_calllog_add (&mCallRecord, calllogtype);
}
int cmd_calllog_add (call *call_info, char calllogtype)
{
	int ret;	
//	strncpy(call_info.number, number, 64);
//	call_info->number[63] = '\0'
	if(calllogtype == MISSED) {
		ret = calllog_AddMissedCall(call_info);
		calllog_StoreMissedCalls();
	}else if(calllogtype == RECEIVED) {
		ret = calllog_AddReceivedCall(call_info);
 		calllog_StoreReceivedCalls();
	}else if(calllogtype == DIALED) {
		ret = calllog_AddDialedCall(call_info);
		calllog_StoreDialedCalls();
	}

  return 0;
}
