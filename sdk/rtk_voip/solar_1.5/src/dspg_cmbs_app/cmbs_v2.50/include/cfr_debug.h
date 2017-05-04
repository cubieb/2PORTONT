/*!
*	\file			cfr_debug.h
*	\brief
*	\Author		kelbch
*
*	@(#)	%filespec: cfr_debug.h-1 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                          \n
*	----------------------------------------------------------------------------\n
* 14-feb-09		kelbch		1		 Initialize \n
*******************************************************************************
*	COPYRIGHT DOSCH & AMAND RESEARCH GMBH & CO.KG
*	DOSCH & AMAND RESEARCH GMBH & CO.KG Confidential
*
*******************************************************************************/

#if	!defined( CFR_DEBUG_H )
#define	CFR_DEBUG_H

extern int   printf(const char *format, ...);
extern int   prj_printf(const char *format, ...);

#define	DEBUG		0xF

#if defined ( CMBS_API_TARGET )

#define	_CMBS_PRINTF	tapp_log_printf
#define	CFR_DBG_OUT		printf
#define	USB_DBG_OUT		printf

#else

#if defined ( CMBS_SYS_LOG )
#define	_CMBS_PRINTF	cmbs_intDebugOUT
#define	CFR_DBG_OUT		cmbs_intDebugOUT
#define	USB_DBG_OUT		cmbs_intDebugOUT
#else
#define	_CMBS_PRINTF	printf
#define	CFR_DBG_OUT		printf
#define	USB_DBG_OUT		printf
#endif

#endif // defined ( CMBS_API_TARGET )

#define caseretstr(x) case x: return #x

#define	CFR_DBG_MASK_INFO    0x00000001L
#define	CFR_DBG_MASK_WARN    0x00000002L
#define	CFR_DBG_MASK_ERROR   0x00000004L


#if	defined( DEBUG ) && (DEBUG & CFR_DBG_MASK_INFO )
#define	CFR_DBG_INFO				_CMBS_PRINTF
#else
#define	CFR_DBG_INFO(x)
#endif

#if	defined( DEBUG ) && (DEBUG & CFR_DBG_MASK_ERROR )
#define	CFR_DBG_ERROR				_CMBS_PRINTF
#else
#define	CFR_DBG_ERROR(x)
#endif

#if	defined( DEBUG ) && (DEBUG & CFR_DBG_MASK_WARN )
#define	CFR_DBG_WARN				_CMBS_PRINTF
#else
#define	CFR_DBG_WARN(x)
#endif

#if defined( __cplusplus )
extern "C"
{
#endif

void     cmbs_intDebugOUT ( const char *pszFormat, ... );

#if defined( __cplusplus )
}
#endif

#endif	//	CFR_DEBUG_H
//*/
