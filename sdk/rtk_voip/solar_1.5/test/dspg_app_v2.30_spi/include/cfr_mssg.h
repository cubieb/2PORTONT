/*!
*	\file			cfr_mssg.h
*	\brief		framework for messaging system
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

#if	!defined( CFR_MSSG_H )
#define	CFR_MSSG_H
#ifdef WIN32
#define        CFR_CMBS_START          (WM_USER + 1000)
#define        CFR_CMBS_MSSG_DESTROY   (WM_USER + 1001)
#define        CFR_CMBS_MSSG_APPSYNC   (WM_USER + 1002)
#define        CFR_CMBS_END            (WM_USER + 2000)
#endif

/* Definition for critical sections, only available for PC host applications (WIN32, Linux) */

#if defined(CMBS_API_TARGET)
#define        CFR_CMBS_CRITICALSECTION                   void *
#define        CFR_CMBS_INIT_CRITICALSECTION( Section )
#define        CFR_CMBS_ENTER_CRITICALSECTION( Section )  OS00_DISABLE_SWITCH
#define        CFR_CMBS_LEAVE_CRITICALSECTION( Section )  OS00_ENABLE_SWITCH
#define        CFR_CMBS_DELETE_CRITICALSECTION( Section )
#else
#if defined(WIN32)
#define        CFR_CMBS_CRITICALSECTION                   CRITICAL_SECTION
#define        CFR_CMBS_INIT_CRITICALSECTION( Section )   InitializeCriticalSection( &(Section) )
#define        CFR_CMBS_ENTER_CRITICALSECTION( Section )  EnterCriticalSection( &(Section) )
#define        CFR_CMBS_LEAVE_CRITICALSECTION( Section )  LeaveCriticalSection( &(Section) )
#define        CFR_CMBS_DELETE_CRITICALSECTION( Section ) DeleteCriticalSection( &(Section) )
#endif // defined(WIN32)
#if defined( __linux__ )
#define        CFR_CMBS_CRITICALSECTION                   pthread_mutex_t
#define        CFR_CMBS_INIT_CRITICALSECTION( Section )   pthread_mutex_init( &(Section), NULL )
#define        CFR_CMBS_ENTER_CRITICALSECTION( Section )  pthread_mutex_lock( &(Section) )
#define        CFR_CMBS_LEAVE_CRITICALSECTION( Section )  pthread_mutex_unlock( &(Section) )
#define        CFR_CMBS_DELETE_CRITICALSECTION( Section ) pthread_mutex_destroy( &(Section) )
#endif // defined(__linux__)
#endif // defined(CMBS_API_TARGET)


typedef enum
{
   E_CFR_INTERPROCESS_FAILT,       /*!< error occured during interprocess communication */
   E_CFR_INTERPROCESS_MSSG,        /*!< message communication done */
   E_CFR_INTERPROCESS_TIMEOUT,     /*!< timeout of interprocess communication */
   E_CFR_INTERPROCESS_EVENT,       /*!< event communication done */
   E_CFR_INTERPROCESS_MAX
} E_CFR_INTERPROCESS;

typedef struct
{
   u32      u32_ThreadID;
   u32      u32_Timeout;
	 u32	  u32_Cleanup;
	 u32	  u32_MssgWait;
} ST_ICOM_ENTRY, * PST_ICOM_ENTRY;

#if defined( __cplusplus )
extern "C"
{
#endif
PST_ICOM_ENTRY          cfr_MQueueCreate( u32 u32_ThreadID, u32 u32_Timeout );
void                    cfr_MQueueDestroy( PST_ICOM_ENTRY pThis );
void                    cfr_MQueueMssgFree( PST_ICOM_ENTRY pEntry, void * pv_Param );
void                    cfr_MQueueSend( PST_ICOM_ENTRY pThis, u32 u32_MssgID, void * pv_Param, u16 u16_ParamSize );
E_CFR_INTERPROCESS      cfr_MQueueGet( PST_ICOM_ENTRY pThis, u32 * pu32_MssgID, void **ppv_Param, u16 * pu16_ParamSize );
void                    cfr_MQueueThreadIDUpdate( PST_ICOM_ENTRY pThis, u32 u32_ThreadID );

#if defined( __cplusplus )
}
#endif

#endif	//	CFR_MSSG_H
//*/
