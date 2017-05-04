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
 * File:		call_log_api.h
 * Purpose:		
 * Created:	23/10/2007
 * By:			KF
 * Country:	Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_CALL_LOG_API_H
#define SI_CALL_LOG_API_H

/*========================== Include files ==================================*/
#include <time.h>


/*========================== Local macro definitions & typedefs =============*/
#ifndef RECALL_ORG
#undef USE_FLASH
#endif

#ifdef USE_FLASH
#define MISSED_CALLS_FILEPATH  "/mnt/flash/ApplicationData/storage/sc14450_fs/call_log/missed_calls.log"
#define RECEIVED_CALLS_FILEPATH "/mnt/flash/ApplicationData/storage/sc14450_fs/call_log/received_calls.log"
#define DIALED_CALLS_FILEPATH  "/mnt/flash/ApplicationData/storage/sc14450_fs/call_log/dialed_calls.log"
#else
#define MISSED_CALLS_FILEPATH  "/ApplicationData/storage/sc14450_fs/call_log/missed_calls.log"
#define RECEIVED_CALLS_FILEPATH "/ApplicationData/storage/sc14450_fs/call_log/received_calls.log"
#define DIALED_CALLS_FILEPATH  "/ApplicationData/storage/sc14450_fs/call_log/dialed_calls.log"
#endif

#define MAX_NUM_OF_MISSED_CALLS   	10
#define MAX_NUM_OF_RECEIVED_CALLS 	10
#define MAX_NUM_OF_DIALED_CALLS   	10

#define MISSED   	0
#define RECEIVED	1
#define DIALED		2
#define ALL    		3

#define ERROR_CallLog_MissedCallsFileNotFound		(-201)
#define ERROR_CallLog_ReceivedCallsFileNotFound	(-202)
#define ERROR_CallLog_DialedCallsFileNotFound		(-203)
#define ERROR_CallLog_NoMissedCall					    (-204)
#define ERROR_CallLog_NoReceivedCall				    (-205)
#define ERROR_CallLog_NoDialedCall					    (-206)
#define ERROR_CallLog_MissedCallNotFound			  (-207)
#define ERROR_CallLog_ReceivedCallNotFound			(-208)
#define ERROR_CallLog_DialedCallNotFound		  	(-209)

typedef struct _call {
	int index;
	char number[64];
	char year; // number of years after 1970
	char month;
	char day;
	char hour;
	char minute;
	char second;
} call;

typedef struct _CallLogErrorCode {
   int ErrorId;
   char ErrorString[35];
} CallLogErrorCode;

/*========================== Global variables ===============================*/


/*========================== Global function prototypes =====================*/
int calllog_LoadMissedCalls();
int calllog_LoadReceivedCalls();
int calllog_LoadDialedCalls();
int calllog_StoreMissedCalls();
int calllog_StoreReceivedCalls();
int calllog_StoreDialedCalls();
int cmd_calllog_add_record(char *number,  struct tm *now, char calllogtype);
int calllog_AddMissedCall(call *call_to_add);
int calllog_AddReceivedCall(call *call_to_add);
int calllog_AddDialedCall(call *call_to_add);
int calllog_DeleteMissedCall(int index);
int calllog_DeleteReceivedCall(int index);
int calllog_DeleteDialedCall(int index);
int calllog_DeleteAllMissedCalls();
int calllog_DeleteAllReceivedCalls();
int calllog_DeleteAllDialedCalls();
int calllog_DeleteAllCalls();
call *calllog_GotoFirst(char call_type);
call *calllog_GotoLast(char call_type);
call *calllog_GotoNext(char call_type);
call *calllog_GotoPrevious(char char_type);
call *calllog_ReturnCurrent(char call_type);
int calllog_GetListLength(char call_list);
char* calllog_GetErrorString(int errCode);
int cmd_calllog_add (call *call_info, char calllogtype);

#endif /* SI_CALL_LOG_API_H */
