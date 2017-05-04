#ifndef DECT_TEST_SCENARIO_H
#define	DECT_TEST_SCENARIO_H

#define CMBS_TRUE	1
#define CMBS_FALSE	0

/* Function Prototype */
void  CMBS_Api_ReleaseNotify( u16 u16_CallId );
void CMBS_Api_FwVersionGet( u16* pVersion, u16* pVersionBuild);
void CMBS_Api_FwVersionGet2();
E_CMBS_RC CMBS_Api_RegistrationOpen();
E_CMBS_RC CMBS_Api_RegistrationClose();
E_CMBS_RC CMBS_Api_HandsetPage(char* hs_mask);
//E_CMBS_RC CMBS_Api_HandsetPage(char all_flag, char hsid)
E_CMBS_RC CMBS_Api_HandsetDelet(char* hs_mask);
E_CMBS_RC CMBS_Api_GetRegHandset( void );
void CMBS_Api_InBound_2HS_Ring( void );
int CMBS_Api_InBound_Ring_CallerID( char lineId, char* clid_num, char* clid_name );
void CMBS_Api_OutboundCallProgress(int n_Line, char * psz_Value );
void CMBS_Api_OutboundCall(PST_CALL_OBJ pst_call);
void CMBS_Api_CallRelease (int n_Line, char reason);
E_APPCMBS_CALL CMBS_Api_LineStateGet(int line);
#endif

