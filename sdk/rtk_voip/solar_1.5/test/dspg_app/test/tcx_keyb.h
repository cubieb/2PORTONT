/*!
*	\file			tcx_keyb.h
*	\brief			
*	\Author		kelbch 
*
*	@(#)	%filespec: tcx_keyb.h-1 %
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

#if	!defined( TCX_KEYB_H )
#define	TCX_KEYB_H


#if defined( __cplusplus )
extern "C"
{
#endif

void  keyboard_loop( void );

int   tcx_getch( void );
void  tcx_appClearScreen( void );
int   tcx_gets( char * buffer, int n_Length );

void  keyb_SRVLoop( void );
void  keyb_CallLoop( void );
void  keyb_SwupLoop( void );

#if defined( __cplusplus )
}
#endif

#endif	//	TCX_KEYB_H
//*/
