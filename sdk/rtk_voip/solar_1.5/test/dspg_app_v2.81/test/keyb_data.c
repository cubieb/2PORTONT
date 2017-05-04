/*!
*	\file		keyb_data.c
*	\brief		cat-iq 2.0 data services tests
*	\Author		stein 
*
*	@(#)	%filespec: keyb_data.c~DMZD53#3 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                \n
*	----------------------------------------------------------------------------\n
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
#include "appdata.h"
#include "cmbs_str.h"
#include "tcx_keyb.h"


u8 u8_DataBuffer[]=
{	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
	0x60,0x61,0x62,0x63,0x64,0x65,0x56,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F
};

void              keyb_DataSessionOpen( void )
{
   char           InputBuffer[20];

   printf( "Enter handset number :\n" );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );

   app_DataSessionOpen( InputBuffer );
}


void              keyb_DataSend( void )
{
   u16            u16_SessionId;
   ST_IE_DATA     st_Data;
   char           InputBuffer[20];
//   char           psz_TestBuffer[64];

   printf( "Enter data session Id:\n" );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );
   u16_SessionId = atoi( InputBuffer );   
   
   st_Data.pu8_Data    = u8_DataBuffer;
   st_Data.u16_DataLen = sizeof( u8_DataBuffer );
   
/* sending short data
   memset( psz_TestBuffer, 0, sizeof(psz_TestBuffer) );
   sprintf( psz_TestBuffer, "%s", "012345678901234" );

   st_Data.pu8_Data    = (u8 *)psz_TestBuffer;
   st_Data.u16_DataLen = (u16)strlen( psz_TestBuffer );
*/
   app_DataSend( u16_SessionId, &st_Data );
}


void              keyb_DataSessionClose( void )
{
   u16            u16_SessionId;
   char           InputBuffer[20];

   printf( "Enter data session Id:\n" );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );
   u16_SessionId = atoi( InputBuffer );

   app_DataSessionClose( u16_SessionId );
}


//		========== keyb_DataLoop ===========
/*!
		\brief         keyboard loop for CAT-iq data menu
      \param[in,ou]  <none>
		\return		

*/
void              keyb_DataLoop( void )
{
   int            n_Keep = TRUE;
   
   while( n_Keep )
   {
//      tcx_appClearScreen();
      printf ( "-----------------------------\n" );
      printf ( "1 => Open data session\n");
      printf ( "2 => Send 128 bytes through IFU-INFO channel\n");
      printf ( "3 => Close data session\n");
      printf ( "- - - - - - - - - - - - - - - \n");
      printf ( "q => Return to Interface Menu\n" );
      
      switch ( tcx_getch() )
      {
         case ' ':
            tcx_appClearScreen();
         break;
            
         case '1':
               keyb_DataSessionOpen();
            break;

         case '2':
               keyb_DataSend();
            break;

         case '3':
               keyb_DataSessionClose();
            break;

         case 'q':
            n_Keep = FALSE;
            break;          
      }
   }
}

//*/
