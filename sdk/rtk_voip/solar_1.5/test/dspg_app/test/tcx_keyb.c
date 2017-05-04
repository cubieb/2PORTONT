/*!
*	\file		tcx_keyb.c
*	\brief		test command line generator for manual testing
*	\Author		kelbch 
*
*	@(#)	%filespec: -1 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                \n
*	----------------------------------------------------------------------------\n
*   16-apr-09		kelbch		1.0		 Initialize \n
*******************************************************************************
*	COPYRIGHT DOSCH & AMAND RESEARCH GMBH & CO.KG
*	DOSCH & AMAND RESEARCH GMBH & CO.KG Confidential
*
*******************************************************************************/
#include <stdio.h>
#include "cmbs_platf.h"

#include "cmbs_api.h"
#include "cfr_ie.h"
#include "cfr_mssg.h"
#include "appcmbs.h"
#include "appsrv.h"
#include "appcall.h"
#include "cmbs_str.h"
#include "tcx_keyb.h"

#if defined ( __linux__ )

//		========== tcx_getch  ===========
/*!
		\brief				 re-implementation of getch function to get the right
                         behaviour under linux
		\param[in,out]		 <none>
		\return				 <int> return value of stdin entering

*/

int tcx_getch (void) 
{
   int ch;
   struct termios old_t, tmp_t;

                                 // save old terminal settings
   if (tcgetattr (STDIN_FILENO, &old_t)) 
   {
                                 // return error if no stdin 
                                 // terminal is available
    return -1;
   }
   
                                 // take old terminal settings 
                                 // and adapt to needed onces
   memcpy (&tmp_t, &old_t, sizeof (old_t));
                                 // de-activate echo and switch of 
                                 // line mode
   tmp_t.c_lflag &= ~ICANON & ~ECHO;
   
   if (tcsetattr (STDIN_FILENO, TCSANOW, (const struct termios *)&tmp_t)) 
   {
      return -1;
   }
                                 // read character and hope that
                                 // STDIN_FILENO handles blocked condition
   ch = getchar ();
                                 // restore old terminal settings
   tcsetattr (STDIN_FILENO, TCSANOW, (const struct termios *)&old_t);
                              
   return ch;
}

int   tcx_gets( char * buffer, int n_Length )
{
   fgets( buffer,n_Length,stdin);
                                 // remove also carrige return
   buffer[strlen(buffer)-1]=0;

   return 0;
}
#else

int tcx_getch (void)
{
   
   return getch();
}

int   tcx_gets( char * buffer, int n_Length )
{
   gets( buffer );
   
   return 0;
}
#endif
//		========== tcx_appClearScreen  ===========
/*!
		\brief			 clear command line screen due to asc ii escape sequence
		\param[in,out]	 <none>
		\return			 <none>

*/

void  tcx_appClearScreen (void)
{
#if defined ( __linux__ )
   char esc = 27;
   printf("%c%s",esc,"[2J");
   printf("%c%s",esc,"[1;1H");
#elif defined ( WIN32 )
    system("cls");
#endif   
}
//		========== keyboard_loop  ===========
/*!
		\brief				 line command keyboard loop of test application

		\param[in,out]		 <none>

		\return				 <none>

*/

void  keyboard_loop ( void )
{
   
   int    n_Run = TRUE;
   while (n_Run )
   {
      tcx_appClearScreen();
      
      printf ( "#######################\n" );
      printf ( "Choose IF\n");
      printf ( "s => Service, system\n" );
      printf ( "c => Call management\n" );
      printf ( "f => Firmware update\n" );
      printf ( "q => Quit\n" );
      printf ( "Choose:");
      switch (tcx_getch())
      {
         case 's':
            keyb_SRVLoop();
         break;
         case 'c':
            keyb_CallLoop();
         break;
         case 'f':
            keyb_SwupStart();
//            keyb_SwupLoop();
         break;
         case 'q':
            n_Run = FALSE;
         break;
      }
   }
}

//*/
