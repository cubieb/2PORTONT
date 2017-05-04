#ifndef DECT_TEST_SCENARIO_H
#define	DECT_TEST_SCENARIO_H

#define CMBS_TRUE	1
#define CMBS_FALSE	0

typedef struct
{
   u16	u16_LineId;
   u16	bCallId_NeedAnswer;
   u16	u16_CallId;
   u16	u16_HsId;
} ST_CALLID_ANSWER;

/* Function Prototype */
void CMBS_Api_RTK_init(void);
void  CMBS_Api_ReleaseNotify( u16 u16_CallId );
void CMBS_Api_FwVersionGet( u16* pVersion, u16* pVersionBuild);
void CMBS_Api_FwVersionGet2(void);
E_CMBS_RC CMBS_Api_RegistrationOpen(void);
E_CMBS_RC CMBS_Api_RegistrationClose(void);
E_CMBS_RC CMBS_Api_HandsetPage(char* hs_mask);
//E_CMBS_RC CMBS_Api_HandsetPage(char all_flag, char hsid)
E_CMBS_RC CMBS_Api_HandsetDelet(char* hs_mask);
E_CMBS_RC CMBS_Api_GetRegHandset( void );
void CMBS_Api_InBound_2HS_Ring( void );
int CMBS_Api_InBound_Ring_CallerID( char lineId, char* clid_num, char* clid_name );
void CMBS_Api_OutboundCallProgress(int n_Line, char * psz_Value );
void CMBS_Api_OutboundEstablishCall(PST_CALL_OBJ pst_call);
void CMBS_Api_OutboundAnswerCall(char lineId);
void CMBS_Api_CallRelease (int n_Line, char reason);
void CMBS_Api_CallRelease_Busy (u16 u16_CallId);
E_APPCMBS_CALL CMBS_Api_LineStateGet(int line);
void CMBS_Api_BusyPlay ( u16 u16_CallId, int no_On );
#endif

