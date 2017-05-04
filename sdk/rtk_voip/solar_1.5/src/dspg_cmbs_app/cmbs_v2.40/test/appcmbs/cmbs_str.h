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
char *               getstr_E_CMBS_EVENT( E_CMBS_EVENT_ID e_Event );
char *               getstr_E_CMBS_IE( E_CMBS_IE_TYPE e_IE );
char *               getstr_E_CMBS_PARAM( E_CMBS_PARAM e_Param );
char *               getstr_E_CMBS_HSTYPE( E_CMBS_HS_TYPE e_HsType );
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
