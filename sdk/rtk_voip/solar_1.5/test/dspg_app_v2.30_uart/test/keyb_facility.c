/*!
*	\file		keyb_catiq.c
*	\brief		firmware update test
*	\Author		stein 
*
*	@(#)	%filespec: -1 %
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
#include "appfacility.h"
#include "cmbs_str.h"
#include "tcx_keyb.h"



void              keyb_CatIqVMWI( void )
{
   u16            u16_RequestId;
   u8             u8_LineId, u8_Messages;
   char           InputBuffer[20];

   printf( "Enter Request ID         (dec): " );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );
   u16_RequestId = atoi( InputBuffer );

   printf( "Enter Line ID            (dec): " );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );
   u8_LineId = atoi( InputBuffer );

   printf( "Enter number of messages (dec): " );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );
   u8_Messages = atoi( InputBuffer );

   printf( "Enter handset mask ( e.g. 1,2,3,4 or none or all):\n" );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );

   app_FacilityVMWI( u16_RequestId, u8_LineId, u8_Messages, InputBuffer );
}


void              keyb_CatIqMissedCalls( void )
{
   u16            u16_RequestId;
   u8             u8_LineId, u8_Messages;
   char           InputBuffer[20];

   printf( "Enter Request ID         (dec): " );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );
   u16_RequestId = atoi( InputBuffer );

   printf( "Enter Line ID            (dec): " );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );
   u8_LineId = atoi( InputBuffer );

   printf( "Enter number of calls    (dec): " );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );
   u8_Messages = atoi( InputBuffer );

   printf( "Enter handset mask ( e.g. 1,2,3,4 or none or all):\n" );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );

   app_FacilityMissedCalls( u16_RequestId, u8_LineId, u8_Messages, InputBuffer );
}


void              keyb_CatIqListChanged( void )
{
   u16            u16_RequestId;
   u8             u8_ListId, u8_ListEntries;
   char           InputBuffer[20];

   printf( "Enter Request ID         (dec): " );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );
   u16_RequestId = atoi( InputBuffer );

   printf( "Enter List ID            (dec): " );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );
   u8_ListId = atoi( InputBuffer );

   printf( "Enter number of list entries  : " );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );
   u8_ListEntries = atoi( InputBuffer );

   printf( "Enter handset mask ( e.g. 1,2,3,4 or none or all):\n" );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );

   app_FacilityListChanged( u16_RequestId, u8_ListId, u8_ListEntries, InputBuffer );
}


void              keyb_CatIqWebContent( void )
{
   u16            u16_RequestId;
//   u8             u8_LineId;
   u8             u8_WebContents;
   char           InputBuffer[20];

   printf( "Enter Request ID         (dec): " );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );
   u16_RequestId = atoi( InputBuffer );

/*
   printf( "Enter Line ID            (dec): " );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );
   u8_LineId = atoi( InputBuffer );
*/
   printf( "Enter number of web contents  : " );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );
   u8_WebContents = atoi( InputBuffer );

   printf( "Enter handset mask ( e.g. 1,2,3,4 or none or all):\n" );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );

   app_FacilityWebContent( u16_RequestId, u8_WebContents, InputBuffer );
}


void              keyb_CatIqPropEvent( void )
{
   u16            u16_RequestId, u16_PropEvent=0;
   u8             u8_DataLen, u8_Data[20];
   char           InputBuffer[64];
   int            i, j;

   printf( "Enter Request ID          (dec): " );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );
   u16_RequestId = atoi( InputBuffer );
/*
   printf( "Enter Proprietary Event   (hex): " );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   memset( u8_Data, 0, sizeof(u8_Data) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );
   for( i=0, j=0; i < 4; i++ )
   {
      u8_Data[i] = app_ASC2HEX( InputBuffer+j );
      j += 2;
   }
   u16_PropEvent = (u8_Data[0] << 8) | u8_Data[1];
*/   
   printf( "Enter Data Length (dec. max 20): " );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );
   u8_DataLen = atoi( InputBuffer );

   printf( "Enter Data ( currently this raw data will be transmitted) (hex): " );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   memset( u8_Data, 0, sizeof(u8_Data) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );

   for( i=0, j=0; i < u8_DataLen; i++ )
   {
      u8_Data[i] = app_ASC2HEX( InputBuffer+j );
      j += 2;
   }

   printf( "Enter handset mask ( e.g. 1,2,3,4 or none or all):\n" );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );

   app_FacilityPropEvent( u16_RequestId, u16_PropEvent, u8_Data, u8_DataLen, InputBuffer );
}


void              keyb_CatIqTimeUpdate( void )
{
   u16            u16_RequestId;
   char           InputBuffer[32];
   ST_DATE_TIME   st_DateAndTime;

   st_DateAndTime.e_Coding = CMBS_DATE_TIME;
   st_DateAndTime.e_Interpretation = CMBS_CURRENT_TIME;

   st_DateAndTime.u8_Year  = 9;
   st_DateAndTime.u8_Month = 10;
   st_DateAndTime.u8_Day   = 03;
   
   st_DateAndTime.u8_Hours = 11;
   st_DateAndTime.u8_Mins  = 55;
   st_DateAndTime.u8_Secs  = 1;

   st_DateAndTime.u8_Zone  = 8;

   printf( "Sending Date:20%02d-%02d-%02d Time:%02d:%02d:%02d Zone:0x%02X\n",
            st_DateAndTime.u8_Year, st_DateAndTime.u8_Month, st_DateAndTime.u8_Day,
            st_DateAndTime.u8_Hours, st_DateAndTime.u8_Mins, st_DateAndTime.u8_Secs,
            st_DateAndTime.u8_Zone );

   printf( "Enter Request ID          (dec): " );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );
   u16_RequestId = atoi( InputBuffer );

   printf( "Enter handset mask ( e.g. 1,2,3,4 or none or all):\n" );
   memset( InputBuffer, 0, sizeof(InputBuffer) );
   tcx_gets( InputBuffer, sizeof(InputBuffer) );

   app_FacilityDateTime( u16_RequestId, &st_DateAndTime, InputBuffer );
}


//		========== keyb_CatIqLoop ===========
/*!
		\brief         keyboard loop for CAT-iq menu
      \param[in,ou]  <none>
		\return		

*/
void              keyb_CatIqLoop( void )
{
   int            n_Keep = TRUE;
   
   while( n_Keep )
   {
      tcx_appClearScreen();
      printf ( "-----------------------------\n" );
      printf ( "1 => Send VMWI Voice Message Waiting Indication\n");
      printf ( "2 => Send Missed Calls Notification\n");
      printf ( "3 => Send List Changed Notification\n");
      printf ( "4 => Send Web Content Notification\n");
      printf ( "- - - - - - - - - - - - - - - \n");
      printf ( "5 => Send Proprietary Event\n");
      printf ( "6 => Send Date_Time Update Event\n");
      printf ( "- - - - - - - - - - - - - - - \n");
      printf ( "q => Return to Interface Menu\n" );
      
      switch ( tcx_getch() )
      {
         case '1':
               keyb_CatIqVMWI();
            break;

         case '2':
               keyb_CatIqMissedCalls();
            break;

         case '3':
               keyb_CatIqListChanged();
            break;

         case '4':
               keyb_CatIqWebContent();
            break;

         case '5':
               keyb_CatIqPropEvent();
            break;

         case '6':
               keyb_CatIqTimeUpdate();
            break;

         case 'q':
            n_Keep = FALSE;
            break;          
      }
   }
}

//*/
