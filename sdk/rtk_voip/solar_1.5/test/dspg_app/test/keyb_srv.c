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
#include <stdlib.h>

#include "cmbs_platf.h"

#include "cmbs_api.h"
#include "cfr_ie.h"
#include "cfr_mssg.h"
#include "appcmbs.h"
#include "appsrv.h"
#include "appcall.h"
#include "cmbs_str.h"
#include "tcx_keyb.h"

/*****************************************************************************
*
*        Service Keyboard loop
*
******************************************************************************/
//		========== keyb_RFPISet  ===========
/*!
		\brief				 Set the RFPI of the CMBS Module

		\param[in,out]		 <none>

		\return				<none>

*/

void     keyb_ParamFlexGet( void )
{
   char  buffer[12];
   u16   u16_Location;

   printf( "Enter Location        (dec): " );
   tcx_gets( buffer, sizeof(buffer) );
   u16_Location = atoi( buffer );

   printf( "Enter Length (dec. max 100): " );
   tcx_gets( buffer, sizeof(buffer) );

   app_SrvParamGet( CMBS_PARAM_FLEX, u16_Location, (u16)atoi(buffer), 1 );

}
void     keyb_RFPISet( void )
{
   char  buffer[12];
   u8    u8_RFPI[5];
   int   i,j=0;

   printf( "Enter RFPI: " );
   tcx_gets( buffer, sizeof(buffer) );

   for( i=0; i< 10; i+=2 )
   {
       u8_RFPI[j] = app_ASC2HEX( buffer + i );
       j++;
   }

   app_SrvParamSet( CMBS_PARAM_RFPI , 0, sizeof(u8_RFPI), u8_RFPI, 1 );
}
//		========== keyb_ChipSettingsSet ===========
/*!
		\brief				 set the tuning parameter of the chipset

		\param[in,out]		 <none>

		\return				<none>

*/

void     keyb_ChipSettingsSet( void )
{
   char  buffer[4];
   u8    u8_Value;
   ST_APPCMBS_CONTAINER st_Container;

   printf( "Enter RVBG: " );
   tcx_gets( buffer, sizeof(buffer) );
   if( strlen(buffer) )
   {
      u8_Value = app_ASC2HEX(buffer);
      app_SrvParamSet( CMBS_PARAM_RVBG , 0, sizeof(u8_Value), &u8_Value, 1 );

      appcmbs_WaitForContainer ( CMBS_EV_DSR_PARAM_SET_RES, &st_Container );
   }
   printf( "\nEnter RVREF: " );
   tcx_gets( buffer, sizeof(buffer) );
   if( strlen(buffer) )
   {
      u8_Value = app_ASC2HEX(buffer);
      app_SrvParamSet( CMBS_PARAM_RVREF , 0, sizeof(u8_Value), &u8_Value, 1 );

      appcmbs_WaitForContainer ( CMBS_EV_DSR_PARAM_SET_RES, &st_Container );
   }

   printf( "\nEnter RVTUN: " );
   tcx_gets( buffer, sizeof(buffer) );
   {
      u8_Value = app_ASC2HEX( buffer );
      app_SrvParamSet( CMBS_PARAM_RXTUN , 0, sizeof(u8_Value), &u8_Value, 1 );

      appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_SET_RES, &st_Container );
   }
}
//		========== keyb_ChipSettingsGet ===========
/*!
		\brief				 Shows the tuning parameter of the CMBS module

		\param[in,out]		 <none>

		\return				<none>

*/

void     keyb_ChipSettingsGet( void )
{
   u8    u8_Value[3];
   ST_APPCMBS_CONTAINER st_Container;

   app_SrvParamGet( CMBS_PARAM_RVBG, 0, 0, 1 );
   appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_GET_RES, &st_Container );
   u8_Value[0] = st_Container.ch_Info[0];

   app_SrvParamGet( CMBS_PARAM_RVREF, 0, 0, 1 );
   appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_GET_RES, &st_Container );
   u8_Value[1] = st_Container.ch_Info[0];

   app_SrvParamGet( CMBS_PARAM_RXTUN, 0, 0, 1 );
   appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_GET_RES, &st_Container );
   u8_Value[2] = st_Container.ch_Info[0];

   tcx_appClearScreen();
   printf( "Chipset settings\n" );
   printf( "RVBG: %02x\n", u8_Value[0] );
   printf( "RVREF: %02x\n", u8_Value[1] );
   printf( "RVTUN: %02x\n", u8_Value[2] );
   printf( "Press Any Key!\n" );
   tcx_getch();
}
//		========== keypb_EEPromParamGet ===========
/*!
		\brief         Handle EEProm Settings get
      \param[in,ou]  <none>
		\return

*/

void  keypb_EEPromParamGet( void )
{
   ST_APPCMBS_CONTAINER st_Container;
//   int   i;

   memset( &st_Container,0,sizeof(st_Container) );

   printf( "Select EEProm Param:\n" );
   printf( "1 => RFPI\n" );
   printf( "2 => PIN\n" );
   printf( "3 => Chipset settings\n" );
/*
   printf( "4 => VBG\n" );
   printf( "5 => VREF\n" );
   printf( "6 => RXTUN\n" );
*/
   printf( "7 => TEST Mode\n" );
   printf( "8 => Master PIN\n" );
   printf( "9 => Flex EEprom get\n" );

   switch( tcx_getch() )
   {
      case '1':
         app_SrvParamGet( CMBS_PARAM_RFPI, 0, 0, 1 );
      break;

      case '2':
         app_SrvParamGet( CMBS_PARAM_AUTH_PIN, 0, 0, 1 );
      break;

      case '3':
         keyb_ChipSettingsGet();
      return;

      case '4':
         app_SrvParamGet( CMBS_PARAM_RVBG, 0, 0, 1 );
      break;

      case '5':
         app_SrvParamGet( CMBS_PARAM_RVREF, 0, 0, 1 );
      break;

      case '6':
         app_SrvParamGet( CMBS_PARAM_RXTUN, 0, 0, 1 );
      break;

      case '7':
         app_SrvParamGet( CMBS_PARAM_TEST_MODE, 0, 0, 1 );
      break;

      case '8':
         app_SrvParamGet( CMBS_PARAM_MASTER_PIN, 0, 0, 1 );
      break;

      case '9':
         keyb_ParamFlexGet();
      break;

      default:
         return;
   }
                                 // wait for CMBS target message
   appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_GET_RES, &st_Container );

/* result already shown in app_IEToString() function
                                 // show result
   switch ( st_Container.n_Info )
   {
      case CMBS_PARAM_RFPI:
         printf( "RFPI: ");
      break;
      case CMBS_PARAM_RVBG:
         printf( "RVBG: ");
      break;
      case CMBS_PARAM_RVREF:
         printf( "RVREF: ");
      break;
      case CMBS_PARAM_RXTUN:
         printf( "RXTUN: ");
      break;
      case CMBS_PARAM_MASTER_PIN:
         printf( "MASTERPIN: ");
      break;
      case CMBS_PARAM_AUTH_PIN:
         printf( "PIN: ");
      break;
      case CMBS_PARAM_TEST_MODE:
         printf( "TEST MODE: ");
      break;
      default:
         printf( "\n-------\n" );
   }

   for ( i=0; i < st_Container.n_InfoLen;i++ )
   {
      printf(" %x", (u8)st_Container.ch_Info[i]);
   }

   printf ("\n");
*/

   printf( "Press Any Key!\n" );
   tcx_getch();
}

void  keyb_ParamFlexSet( void )
{
   char buffer[256];
   u8   u8_Data[128];
   u16  u16_Len = 0;
   u16  u16_Location, i, x;

   printf( "Enter Location        (dec): " );
   tcx_gets( buffer, sizeof(buffer) );
   u16_Location = atoi( buffer );

   printf( "Enter Length (dec. max 100): " );
   tcx_gets( buffer, sizeof(buffer) );
   u16_Len = atoi( buffer );

   printf( "Enter Data    (hexadecimal): " );
   tcx_gets( buffer, u16_Len * 2 );

   for( i=0, x=0; i < u16_Len; i++ )
   {
      u8_Data[i] = app_ASC2HEX( buffer+x );
      x += 2;
   }

   app_SrvParamSet( CMBS_PARAM_FLEX , u16_Location, u16_Len, u8_Data, 1 );
}

//		========== keypb_EEPromParamSet ===========
/*!
		\brief         Handle EEProm Settings set
      \param[in,ou]  <none>
		\return

*/
void  keypb_EEPromParamSet( void )
{
   char buffer[16];

   printf( "Select EEProm Param:\n" );
   printf( "1 => RFPI\n" );
   printf( "2 => PIN\n" );
   printf( "3 => Chipset Tunes\n" );
/*
   printf( "4 => VBG\n" );
   printf( "5 => VREF\n" );
   printf( "6 => RXTUN\n" );
*/
   printf( "7 => Test mode\n" );
   printf( "9 => Flex EEprom set\n" );


   switch( tcx_getch() )
   {
      case '1':
         keyb_RFPISet();
/*
         printf("New RFPI value (hex): ");

         memset( buffer, 0, sizeof(buffer) );
         tcx_gets( buffer, sizeof(buffer) );
         app_SrvParamSet( CMBS_PARAM_RFPI, buffer, CMBS_PARAM_RFPI_LENGTH * 2 );
*/
      break;

      case '2':
         printf( "New PIN Code (default:ffff1590): " );
         tcx_gets( buffer, sizeof(buffer) );

         app_SrvPINCodeSet( buffer );
      break;

      case '3':
         keyb_ChipSettingsSet();
      break;
/*
      case '4':
         printf("New VBG value (hex): ");

         memset( buffer, 0, sizeof(buffer) );
         tcx_gets( buffer, sizeof(buffer) );
         app_SrvParamSet( CMBS_PARAM_RVBG, buffer, CMBS_PARAM_RVBG_LENGTH * 2 );
      break;

      case '5':
         printf("New VREF value (hex): ");

         memset( buffer, 0, sizeof(buffer) );
         tcx_gets( buffer, sizeof(buffer) );
         app_SrvParamSet( CMBS_PARAM_RVREF, buffer, CMBS_PARAM_RVREF_LENGTH * 2 );
      break;

      case '6':
         printf("New RXTUN value (hex): ");

         memset( buffer, 0, sizeof(buffer) );
         tcx_gets( buffer, sizeof(buffer) );
         app_SrvParamSet( CMBS_PARAM_RXTUN, buffer, CMBS_PARAM_RXTUN_LENGTH * 2 );
      break;
*/
      case '7':
      {
         ST_APPCMBS_CONTAINER st_Container;
         u8 u8_Value;

         printf( "New Test mode value (hex,00=>disabled,81=>TBR6): " );

         memset( buffer, 0, sizeof(buffer) );
         tcx_gets( buffer, sizeof(buffer) );

         if( strlen(buffer) )
         {
            u8_Value = app_ASC2HEX(buffer);
            app_SrvParamSet( CMBS_PARAM_TEST_MODE , 0, sizeof(u8_Value), &u8_Value, 1 );

            appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_SET_RES, &st_Container );
         }
      }
      break;

      case '9':
         keyb_ParamFlexSet();
         break;
   }
}

//		========== keypb_HandsetPage ===========
/*!
		\brief         Page a handset or all handsets
      \param[in,ou]  <none>
		\return

*/
void  keypb_HandsetPage( void )
{
   char buffer[20];

   printf( "Enter handset mask ( e.g. 1,2,3,4 or none or all):\n" );
   tcx_gets( buffer, sizeof(buffer) );

   app_SrvHandsetPage( buffer);

}

//		========== keypb_HandsetDelete ===========
/*!
		\brief         Delete a handset or all handsets
      \param[in,ou]  <none>
		\return

*/
void  keypb_HandsetDelete( void )
{
   char buffer[20];

   printf( "Enter handset mask ( e.g. 1,2,3,4 or none or all):\n" );
   tcx_gets( buffer, sizeof(buffer) );

   app_SrvHandsetDelete( buffer );

}

//		========== keypb_SYSRegistrationMode ===========
/*!
		\brief         Subscription on/off
      \param[in,ou]  <none>
		\return

*/
void  keypb_SYSRegistrationMode( void )
{
   printf( "1 => Registration open\n" );
   printf( "2 => Registration close\n" );

   switch( tcx_getch() )
   {
      case '1':
         app_SrvSubscriptionOpen();
         break;
      case '2':
         app_SrvSubscriptionClose();
         break;
   }
}

//		========== keypb_SYSRegistrationRestart ===========
/*!
		\brief         Reboot CMBS Target
      \param[in,ou]  <none>
		\return

*/
void  keypb_SYSRegistrationRestart( void )
{
   app_SrvSystemReboot();
   printf( "Exit application\n" );
   //\todo cleanup
}

//		========== keypb_SYSFWVersionGet ===========
/*!
		\brief         Get current CMBS Target version
      \param[in,ou]  <none>
		\return

*/
void  keypb_SYSFWVersionGet( void )
{
   ST_APPCMBS_CONTAINER st_Container;
   PST_IE_FW_VERSION  p_IE;

   app_SrvFWVersionGet( TRUE );

   appcmbs_WaitForContainer( CMBS_EV_DSR_FW_VERSION_GET_RES, &st_Container );

   p_IE = (PST_IE_FW_VERSION)st_Container.ch_Info;

   printf( "FIRMWARE Version:\n " );
   switch( p_IE->e_SwModule )
   {
      case CMBS_MODULE_CMBS:        printf( "CMBS Module " );     break;
      case CMBS_MODULE_DECT:        printf( "DECT Module " );     break;
      case CMBS_MODULE_DSP:         printf( "DSP Module " );      break;
      case CMBS_MODULE_EEPROM:      printf( "EEPROM Module " );   break;
      case CMBS_MODULE_USB:         printf( "USB Module " );      break;
      default:                      printf( "UNKNOWN Module " );
   }
   printf( "VER_%04x\n", p_IE->u16_FwVersion );

   printf( "Press Any Key!\n" );
   tcx_getch();
}


//		========== keypb_LogBufferRead ===========
/*!
		\brief         Get current content of log buffer
      \param[in,ou]  <none>
		\return

*/
void  keypb_LogBufferRead( void )
{
   ST_APPCMBS_CONTAINER st_Container;
   PST_IE_SYS_LOG       p_IE;
   u16                  u16_DataLen;            /*!< Length of data */
   u8 *                 pu8_Data;               /*!< Data pointer */
   u16                  i;
   u8                   c;

   app_SrvLogBufferRead( TRUE );

   appcmbs_WaitForContainer( CMBS_EV_DSR_SYS_LOG, &st_Container );

   p_IE = (PST_IE_SYS_LOG)st_Container.ch_Info;

   u16_DataLen = p_IE->u16_DataLen;
   pu8_Data    = p_IE->pu8_Data;

   printf("LogBuffer:");
   for(i = 0; i < u16_DataLen; i++)
   {
     c = pu8_Data[i];
     if(c >= 0x20 && c <= 0x7e) printf("%c", c);
     else                       printf("(%02X)", c);
   }
   printf("\n");

   printf( "Press Any Key!\n" );
   tcx_getch();
}


//		========== keypb_SYSTestModeGet ===========
/*!
		\brief         Get current CMBS Target test mode state
      \param[in,ou]  <none>
		\return

*/
void  keypb_SYSTestModeGet( void )
{
   app_SrvTestModeGet( TRUE );
}

//		========== keypb_SYSTestModeSet ===========
/*!
		\brief         enable TBR 6 mode
      \param[in,ou]  <none>
		\return

*/
void  keypb_SYSTestModeSet( void )
{
   app_SrvTestModeSet();
}

//		========== keyb_SRVLoop ===========
/*!
		\brief         keyboard loop of test application
      \param[in,ou]  <none>
		\return

*/
void  keyb_SRVLoop( void )
{
   int n_Keep = TRUE;

   while( n_Keep )
   {
      tcx_appClearScreen();
      printf( "-----------------------------\n" );
      printf( "Choose service:\n" );
      printf( "1 => EEProm  Param Get\n" );
      printf( "2 => EEProm  Param Set\n" );
      printf( "3 => Handset Page\n" );
      printf( "4 => Handset Delete\n" );
      printf( "5 => System  Registration Mode\n" );
      printf( "6 => System  Restart\n" );
      printf( "7 => System  FW Version Get\n" );
      printf( "8 => System  Testmode Get\n" );
      printf( "9 => System  Testmode Set\n" );
      printf( "L => Read    Logbuffer\n" );
      printf( "- - - - - - - - - - - - - - - \n" );
      printf( "q => Return to Interface Menu\n" );

      switch( tcx_getch() )
      {
         case '1':
               keypb_EEPromParamGet();
            break;
         case '2':
               keypb_EEPromParamSet();
            break;
         case '3':
               keypb_HandsetPage();
            break;
         case '4':
               keypb_HandsetDelete();
            break;
         case '5':
               keypb_SYSRegistrationMode();
            break;
         case '6':
               keypb_SYSRegistrationRestart();
            break;
         case '7':
               keypb_SYSFWVersionGet();
            break;
         case '8':
               keypb_SYSTestModeGet();
            break;
         case '9':
               keypb_SYSTestModeSet();
            break;
         case 'l':
         case 'L':
               keypb_LogBufferRead();
            break;
         case 'q':
            n_Keep = FALSE;
            break;

      }
   }
}

//*/
