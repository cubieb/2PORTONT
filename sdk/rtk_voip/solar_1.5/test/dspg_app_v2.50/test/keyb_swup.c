/*!
*	\file		keyb_swup.c
*	\brief		firmware update test
*	\Author		stein 
*
*	@(#)	%filespec: keyb_swup.c-2 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                \n
*	----------------------------------------------------------------------------\n
*******************************************************************************
*	COPYRIGHT DOSCH & AMAND RESEARCH GMBH & CO.KG
*	DOSCH & AMAND RESEARCH GMBH & CO.KG Confidential
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if ! defined ( WIN32 )
#include <unistd.h>
#include <termios.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/msg.h>
#else
#include <conio.h>
#include <io.h>
#endif

#include <fcntl.h>
#include <errno.h>

#include "cmbs_platf.h"
#include "cmbs_api.h"
#include "cfr_ie.h"
#include "cfr_mssg.h"
#include "appcmbs.h"
#include "appswup.h"
#include "cmbs_str.h"
#include "tcx_keyb.h"


//		========== keyb_SwupStart ===========
/*!
		\brief         starts firmware update
      \param[in,ou]  <none>
		\return		

*/
#if defined( WIN32 )
void        keyb_SwupStart( void )
{
   HANDLE   fd;
   char     szFileName[128] = "";

   memset( szFileName, 0, sizeof(szFileName) );

   printf( "\nEnter firmware binary file name:\n" );
   tcx_gets(szFileName, sizeof(szFileName));

   fd = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

	if( fd == INVALID_HANDLE_VALUE )
	{
		printf( "WIN32 Unable to open file %s --> %d", szFileName, GetLastError() );
        tcx_getch();
		return;
	}

   printf( "keyb_SwupStart Binary file: %s\n", szFileName );

   app_FwUpdStart( fd );
}
#else
void        keyb_SwupStart( void )
{
   int      fd;
   char     szFileName[64];

   memset( szFileName, 0, sizeof(szFileName) );

   printf( "\nEnter firmware binary file name:\n" );
   tcx_gets(szFileName, sizeof(szFileName));

	fd = open(szFileName, O_RDONLY);

	if( fd == -1 )
	{
		printf( "Unable to open file %s", szFileName );
		return;
	}

   printf( "keyb_SwupStart Binary file: %s\n", szFileName );

   app_FwUpdStart( fd );
}
#endif

void        keyb_SwupPrep( void )
{
   app_FwUpdPrepare();
}

//		========== keyb_SRVLoop ===========
/*!
		\brief         keyboard loop of test application
      \param[in,ou]  <none>
		\return		

*/
void        keyb_SwupLoop( void )
{
   int n_Keep = TRUE;
   
   while ( n_Keep )
   {
//      tcx_appClearScreen();
      printf ( "-----------------------------\n" );
      printf ( "1 => Start   firmware update (from bootloader)\n");
      printf ( "2 => Prepare firmware update (from application)\n");
      printf ( "- - - - - - - - - - - - - - - \n");
      printf ( "q => Return to Interface Menu\n" );
      
      switch ( tcx_getch() )
      {
         case ' ':
            tcx_appClearScreen();
         break;
            
         case '1':
               keyb_SwupStart();
            break;
         case '2':
               keyb_SwupPrep();
            break;
         case 'q':
            n_Keep = FALSE;
            break;          
      }
   }
}

//*/
