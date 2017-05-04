/*!
*	\file	  	cmbs_str.h		
*	\brief		headerfile for CMBS enumeration string converter
*	\Author		kelbch 
*
*	@(#)	%filespec: cmbs_str.h %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                          \n
*	----------------------------------------------------------------------------\n
*	16-apr-09		kelbch		1.0		 initialize
*******************************************************************************
*	COPYRIGHT DOSCH & AMAND RESEARCH GMBH & CO.KG
*	DOSCH & AMAND RESEARCH GMBH & CO.KG Confidential
*
*******************************************************************************/

#if	!defined( CMBS_STR_H )
#define	CMBS_STR_H


#if defined( __cplusplus )
extern "C"
{
#endif
char *               getstr_E_CMBS_PARAM( E_CMBS_PARAM e_Param );
E_CMBS_PARAM         getidx_E_CMBS_PARAM( char * psz_Value );
char *               getstr_E_CMBS_TONE( E_CMBS_TONE e_Tone );
E_CMBS_TONE          getidx_E_CMBS_TONE( char * psz_Value );
char *               getstr_CMBS_CALL_PROGR( E_CMBS_CALL_PROGRESS e_Prog );
E_CMBS_CALL_PROGRESS getidx_CMBS_CALL_PROGR( char * psz_Value );

#if defined( __cplusplus )
}
#endif

#endif	//	CMBS_STR_H
//*/
