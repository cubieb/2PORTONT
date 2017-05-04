/*!
*	\file		tcx_keyb.c
*	\brief		test command line generator for manual testing
*	\Author		kelbch
*
*	@(#)	%filespec: tcx_keyb.c~DMZD53#5 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                \n
*	----------------------------------------------------------------------------\n
*   16-apr-09		kelbch		1.0		     Initialize \n
*   18-sep-09		kelbch		pj1029-479	 add quick demonstration menu \n
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

/***************************************************************
*
* external defined function for a quick demonstration
*
****************************************************************/

extern void keyb_SwupStart( void );
extern void keyb_CatIqLoop( void );
extern void keyb_DataLoop( void );
extern void keyb_IncCallWB( void );
extern void keyb_IncCallNB( void );
extern void keyb_IncCallRelease( void );
extern void keyb_CallInfo( void );


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
   do
   {
   gets( buffer );
   }
   while (buffer[0] == '\0');

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


extern u16   tcx_ApplVersionGet(void);
extern int   tcx_ApplVersionBuildGet(void);
extern void     appcmbs_VersionGet( char * pc_Version );

//		========== keyboard_loop  ===========
/*!
		\brief				 line command keyboard loop of test application

		\param[in,out]		 <none>

		\return				 <none>

*/

void  keyboard_loop ( void )
{
   char ch_Version[80];
   int    n_Run = TRUE;
   
   appcmbs_VersionGet(ch_Version);

   while (n_Run )
   {
//      tcx_appClearScreen();

      printf ( "#############################################################\n" );
      printf ( "#\tVersion:\n");
      printf ( "#\tApplication\t: Version %02x.%02x - Build %d\n", (tcx_ApplVersionGet()>>8),(tcx_ApplVersionGet()&0xFF), tcx_ApplVersionBuildGet());
      printf ( "#\tTarget     \t: %s\n", ch_Version);
      printf ( "#\n");
      printf ( "#############################################################\n" );
      printf ( "#\tChoose IF\n");
      // printf ( "#\tg => Start Cordless Stack\n");
      printf ( "#\ts => Service, system\n" );
      printf ( "#\tc => Call management\n" );
      printf ( "#\tf => Firmware update\n" );
      printf ( "#\ti => Facility requests not supported,yet\n" );
      printf ( "#\tj => Data handling not supported, yet\n" );
      printf ( "#\t----------------------------------------------------------\n");
      printf ( "#\tw => Incoming wideband CLI: 1234, CNAME: Call WB\n" );
      printf ( "#\t     Active Call >>> Codec change to wideband \n" );
      printf ( "#\tn => Incoming narrow band CLI: 6789, CNAME: Call NB\n" );
      printf ( "#\t     Active Call >>> Codec change to narrow band \n" );
      printf ( "#\tr => Release call\n" );
      printf ( "#\t----------------------------------------------------------\n");
      printf ( "#\tq => Quit\n\n" );
      printf ( "#############################################################\n" );
      keyb_CallInfo();
      printf ( "\nChoose:");

      switch (tcx_getch())
      {
         // case 'g':
			// appcmbs_CordlessStart( NULL,0 );
			// break;

         case ' ':
            tcx_appClearScreen();
         break;
            
         case 's':
            keyb_SRVLoop();
         break;

         case 'c':
            keyb_CallLoop();
         break;

         case 'f':
            keyb_SwupStart();
         break;

         case 'i':
            keyb_CatIqLoop();
         break;

         case 'j':
            keyb_DataLoop();
         break;

		   case 'w':
         	 keyb_IncCallWB();
           break;

         case 'n':
         	 keyb_IncCallNB();
			 break;

         case 'r':
         	 keyb_IncCallRelease();
           break;

         case 'q':
            n_Run = FALSE;
         break;
      }
   }
}

//*/
