/*!
*	\file		appmain.h
*	\brief	object and utility declaration
*	\Author		kelbch 
*
*	@(#)	%filespec: -1 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                          \n
*	----------------------------------------------------------------------------\n
*******************************************************************************
*	COPYRIGHT DOSCH & AMAND RESEARCH GMBH & CO.KG
*	DOSCH & AMAND RESEARCH GMBH & CO.KG Confidential
*
*******************************************************************************/

#if	!defined( APPCMBS_H )
#define	APPCMBS_H

#include "cmbs_api.h"

#ifndef caseretstr
   #define caseretstr(x) case x: return #x
#endif

#define     APPCMBS_ERROR(x)        printf x
#define     APPCMBS_WARN(x)         printf x
#define     APPCMBS_INFO(x)         printf x

#define     APPCMBS_IE(x)      printf x

typedef struct
{
   E_CMBS_IE_TYPE    e_IE;
   union
   {
      u32                     u32_CallInstance;
      ST_IE_CALLEDPARTY       st_CalledParty;
      ST_IE_CALLERPARTY       st_CallerParty;
      ST_IE_CALLERNAME        st_CallerName;
      ST_IE_CALLPROGRESS      st_CallProgress;
      ST_IE_CALLINFO          st_CallInfo;
      ST_IE_DISPLAY_STRING    st_DisplayString;
      ST_IE_RELEASE_REASON    st_Reason;
      ST_IE_MEDIA_DESCRIPTOR  st_MediaDesc;
      ST_IE_MEDIA_CHANNEL     st_MediaChannel;
      ST_IE_TONE              st_Tone;
      ST_IE_TIMEOFDAY         st_Time;
      ST_IE_HANDSETINFO       st_HandsetInfo;
      ST_IE_PARAMETER         st_Param;
      ST_IE_FW_VERSION        st_FwVersion;
      ST_IE_SYS_LOG           st_SysLog;
      ST_IE_RESPONSE          st_Resp;
//      ST_IE_STATUS            st_Status;
      u32                     u32_IntegerValue;
   }Info;
} ST_APPCMBS_IEINFO, * PST_APPCMBS_IEINFO;

typedef struct
{
   void *   pv_ApplRef;
   void *   pv_CMBSRef;
   int      n_MssgAppID;
   int      n_Token;
#ifdef WIN32
   PST_ICOM_ENTRY pst_AppSyncEntry;          
#endif
}ST_CMBS_APPL, *PST_CMBS_APPL;

typedef struct
{
   char     ch_Info[255];
   int      n_InfoLen;
   int      n_Info;
   int      n_Event;
} ST_APPCMBS_CONTAINER, * PST_APPCMBS_CONTAINER;

typedef struct
{
   long     mType;
   ST_APPCMBS_CONTAINER
            Content;
} ST_APPCMBS_LINUX_CONTAINER, * PST_APPCMBS_LINUX_CONTAINER;

extern ST_CMBS_APPL  g_cmbsappl;

#if defined( __cplusplus )
extern "C"
{
#endif

s8             app_ResponseCheck( void * pv_List );
void           app_IEToString( void * pv_IE, u16 u16_IE );
u8             app_ASC2HEX ( char * psz_Digits );

	// callback for CMBS responses according specification
int            app_ServiceEntity( void * pv_AppRef, E_CMBS_EVENT_ID e_EventID, void * pv_EventData );
int            app_CallEntity( void * pv_AppRef, E_CMBS_EVENT_ID e_EventID, void * pv_EventData );
int            app_SwupEntity( void * pv_AppRef, E_CMBS_EVENT_ID e_EventID, void * pv_EventData );
	// initialize the CMBS application
int            appcmbs_Initialize( void * pv_AppReference, PST_CMBS_DEV pst_DevCtl, PST_CMBS_DEV pst_DevMedia );
void           appcmbs_Cleanup(void);
void           appcmbs_PrepareRecvAdd ( u32 u32_Token );
   // synchronize upper application with async CMBS received Data
void           appcmbs_ObjectSignal( char * psz_Info, int n_InfoLen, int n_Info, int n_Event );
int            appcmbs_WaitForContainer ( int n_Event, PST_APPCMBS_CONTAINER pst_Container );

void           appcmbs_IEInfoGet ( void * pv_IE, u16 u16_IE, PST_APPCMBS_IEINFO p_Info );
void     appcmbs_VersionGet( char * pc_Version );

#if defined( __cplusplus )
}
#endif

#endif	//	APPCMBS_H
//*/
