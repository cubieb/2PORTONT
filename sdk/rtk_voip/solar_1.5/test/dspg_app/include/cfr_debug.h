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

extern int   prj_printf(const char *format, ...);

#define	DEBUG		0xF

#define	CFR_DBG_OUT		printf

#define	USB_DBG_OUT		printf

#define caseretstr(x) case x: return #x

#define	CFR_DBG_MASK_INFO		0x00000001L
#define	CFR_DBG_MASK_WARN		0x00000002L
#define	CFR_DBG_MASK_ERROR		0x00000004L


#if	defined( DEBUG ) && (DEBUG & CFR_DBG_MASK_INFO )
#define	CFR_DBG_INFO		printf
#else
#define	CFR_DBG_INFO(x)
#endif

#if	defined( DEBUG ) && (DEBUG & CFR_DBG_MASK_ERROR )
#define	CFR_DBG_ERROR				printf
#else
#define	CFR_DBG_ERROR(x)
#endif

#if	defined( DEBUG ) && (DEBUG & CFR_DBG_MASK_WARN )
#define	CFR_DBG_WARN				printf
#else
#define	CFR_DBG_WARN(x)
#endif

#if defined( __cplusplus )
extern "C"
{
#endif

#if defined( __cplusplus )
}
#endif

#endif	//	CFR_DEBUG_H
//*/
