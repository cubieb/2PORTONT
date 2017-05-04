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
*   14-dec-09		sergiym		 ?		 Add start/stop log menu \n
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

   printf( "Enter Length (dec. max 512): " );
   tcx_gets( buffer, sizeof(buffer) );

   app_SrvParamAreaGet( CMBS_PARAM_AREA_TYPE_EEPROM, u16_Location, (u16)atoi(buffer), 1 );

}
void     keyb_RFPISet( void )
{
   ST_APPCMBS_CONTAINER st_Container;
   char  buffer[12];
   u8    u8_RFPI[5];
   int   i,j=0;

   printf("\n");
   printf( "Enter RFPI: " );
   tcx_gets( buffer, sizeof(buffer) );

   for( i=0; i< 10; i+=2 )
   {
       u8_RFPI[j] = app_ASC2HEX( buffer + i );
       j++;
   }

   printf("\n");
   app_SrvParamSet( CMBS_PARAM_RFPI, u8_RFPI, sizeof(u8_RFPI), 1 );

   appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_SET_RES, &st_Container );

   printf("\n");
   printf( "Press Any Key!\n" );
   tcx_getch();

}

void     keyb_ECOModeSet( void )
{
   ST_APPCMBS_CONTAINER st_Container;
   u8					u8_Value;

   printf( "\nEnter 0 - to disable ECO mode, 1 - to enable ECO mode: " );
   
   switch( tcx_getch() )
   {
        case '0':
			u8_Value = 0;
            break;

        case '1':
			u8_Value = 1;
            break;

        default:
            return;
   }

   app_ProductionParamSet( CMBS_PARAM_ECO_MODE, &u8_Value, sizeof(u8), 1 );

   appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_SET_RES, &st_Container );

   printf( "\nPress Any Key!\n" );
   tcx_getch();

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
      app_SrvParamSet( CMBS_PARAM_RVBG, &u8_Value, sizeof(u8_Value), 1 );

      appcmbs_WaitForContainer ( CMBS_EV_DSR_PARAM_SET_RES, &st_Container );
   }
   printf( "\nEnter RVREF: " );
   tcx_gets( buffer, sizeof(buffer) );
   if( strlen(buffer) )
   {
      u8_Value = app_ASC2HEX(buffer);
      app_SrvParamSet( CMBS_PARAM_RVREF, &u8_Value, sizeof(u8_Value), 1 );

      appcmbs_WaitForContainer ( CMBS_EV_DSR_PARAM_SET_RES, &st_Container );
   }

   printf( "\nEnter RVTUN: " );
   tcx_gets( buffer, sizeof(buffer) );
   {
      u8_Value = app_ASC2HEX( buffer );
      app_SrvParamSet( CMBS_PARAM_RXTUN, &u8_Value, sizeof(u8_Value), 1 );

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

   app_SrvParamGet( CMBS_PARAM_RVBG, 1 );
   appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_GET_RES, &st_Container );
   u8_Value[0] = st_Container.ch_Info[0];

   app_SrvParamGet( CMBS_PARAM_RVREF, 1 );
   appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_GET_RES, &st_Container );
   u8_Value[1] = st_Container.ch_Info[0];

   app_SrvParamGet( CMBS_PARAM_RXTUN, 1 );
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
   int   nEvent;
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
         app_SrvParamGet( CMBS_PARAM_RFPI, 1 );
         nEvent = CMBS_EV_DSR_PARAM_GET_RES;
      break;

      case '2':
         app_SrvParamGet( CMBS_PARAM_AUTH_PIN, 1 );
         nEvent = CMBS_EV_DSR_PARAM_GET_RES;
      break;

      case '3':
         keyb_ChipSettingsGet();
         nEvent = CMBS_EV_DSR_PARAM_GET_RES;
      return;

      case '4':
         app_SrvParamGet( CMBS_PARAM_RVBG, 1 );
         nEvent = CMBS_EV_DSR_PARAM_GET_RES;
      break;

      case '5':
         app_SrvParamGet( CMBS_PARAM_RVREF, 1 );
         nEvent = CMBS_EV_DSR_PARAM_GET_RES;
      break;

      case '6':
         app_SrvParamGet( CMBS_PARAM_RXTUN, 1 );
         nEvent = CMBS_EV_DSR_PARAM_GET_RES;
      break;

      case '7':
         app_SrvParamGet( CMBS_PARAM_TEST_MODE, 1 );
         nEvent = CMBS_EV_DSR_PARAM_GET_RES;
      break;

      case '8':
         app_SrvParamGet( CMBS_PARAM_MASTER_PIN, 1 );
         nEvent = CMBS_EV_DSR_PARAM_GET_RES;
      break;

      case '9':
         keyb_ParamFlexGet();
         nEvent = CMBS_EV_DSR_PARAM_AREA_GET_RES;
      break;

      default:
         return;
   }
                                 // wait for CMBS target message
   appcmbs_WaitForContainer( nEvent, &st_Container );

   printf( "Press Any Key!\n" );
   tcx_getch();
}

//		========== keypb_ProductionParamGet ===========
/*!
		\brief         Handle Production Settings get
        \param[in,ou]  <none>
		\return

*/
    
void  keypb_ProductionParamGet( void )
{
   ST_APPCMBS_CONTAINER st_Container;
   int   nEvent;

   memset( &st_Container,0,sizeof(st_Container) );

   printf( "\nSelect Production Param:\n" );
   printf( "1 => ECO mode\n" );

   switch( tcx_getch() )
   {
      case '1':
         app_ProductionParamGet( CMBS_PARAM_ECO_MODE, 1 );
         nEvent = CMBS_EV_DSR_PARAM_GET_RES;
      break;

      default:
         return;
   }
                                 // wait for CMBS target message
   appcmbs_WaitForContainer( nEvent, &st_Container );

   printf( "\nPress Any Key!\n" );
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

   printf( "Enter Length (dec. max 512): " );
   tcx_gets( buffer, sizeof(buffer) );
   u16_Len = atoi( buffer );

   printf( "Enter Data    (hexadecimal): " );
   tcx_gets( buffer, u16_Len * 2 );

   for( i=0, x=0; i < u16_Len; i++ )
   {
      u8_Data[i] = app_ASC2HEX( buffer+x );
      x += 2;
   }

   app_SrvParamAreaSet( CMBS_PARAM_AREA_TYPE_EEPROM, u16_Location, u16_Len, u8_Data, 1 );
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
//   printf( "0 => Reset EEprom\n" );


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
         printf("\n");
         printf( "New PIN Code (default:ffff1590): " );
         tcx_gets( buffer, sizeof(buffer) );

         app_SrvPINCodeSet( buffer );
         printf( "Press Any Key!\n" );
         tcx_getch();
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
            app_SrvParamSet( CMBS_PARAM_TEST_MODE, &u8_Value, sizeof(u8_Value), 1 );

            appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_SET_RES, &st_Container );
         }
      }
      break;

      case '9':
         keyb_ParamFlexSet();
         break;
/*         
      case '0':
      {
         ST_APPCMBS_CONTAINER st_Container;
          u8 u8_Value = 0;
          
          printf( "Reset EEprom\n" );
          app_SrvParamSet( CMBS_PARAM_RESET_ALL, &u8_Value, sizeof(u8_Value), 1 );
          
           appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_SET_RES, &st_Container );
      }
          break;
*/
   }
}

//		========== keypb_ProductionParamSet ===========
/*!
		\brief         Handle Production Settings set
        \param[in,ou]  <none>
		\return

*/
void  keypb_ProductionParamSet( void )
{
   printf( "Select EEProm Param:\n" );
   printf( "1 => ECO mode\n" );

   switch( tcx_getch() )
   {
      case '1':
         keyb_ECOModeSet();
      break;

      default:
         return;
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

//      ========== keypb_HandsetStopPaging ===========
/*!
        \brief         Stop page handsets
      \param[in,ou]  <none>
        \return

*/
void  keypb_HandsetStopPaging( void )
{

   app_SrvHandsetStopPaging();

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


//		========== keypb_SysLogStart ===========
/*!
		\brief         Get current content of log buffer
      \param[in,ou]  <none>
		\return

*/
void  keypb_SysLogStart( void )
{
   app_SrvLogBufferStart();

   printf( "SysLog started\n" );
}

//		========== keypb_SysLogStop ===========
/*!
		\brief         Get current content of log buffer
      \param[in,ou]  <none>
		\return

*/
void  keypb_SysLogStop( void )
{
   app_SrvLogBufferStop();

   printf( "SysLog stopped\n" );
}

//		========== keypb_SysLogRead ===========
/*!
		\brief         Get current content of log buffer
      \param[in,ou]  <none>
		\return

*/
void  keypb_SysLogRead( void )
{
   ST_APPCMBS_CONTAINER st_Container;

   app_SrvLogBufferRead( TRUE );

   appcmbs_WaitForContainer( CMBS_EV_DSR_SYS_LOG, &st_Container );

   printf( "Press Any Key!\n" );
   tcx_getch();
}

/* == ALTDV == */

//      ========== keyb_SYSPowerOff ===========
/*!
        \brief         Power Off CMBS Target
      \param[in,ou]  <none>
        \return

*/
void  keypb_SYSPowerOff( void )
{
   app_SrvSystemPowerOff();
   printf("\n");
   printf( "Power Off CMBS Target\n" );
}

//      ========== keyb_RF_Control ===========
/*!
        \brief         Suspend/Resume RF on CMBS Target
        \param[in,ou]  <none>
        \return

*/
void keypb_RF_Control( void )
{
    printf("\n");
    printf( "1 => RF Suspend\n" );
    printf( "2 => RF Resume\n" );

    switch( tcx_getch() )
    {
       case '1':
          app_SrvRFSuspend();
          break;
       case '2':
          app_SrvRFResume();
          break;
    }
}


//      ========== keypb_TurnOn_NEMo_mode ===========
/*!
        \brief         Turn On/Off NEMo mode on CMBS Target
        \param[in,ou]  <none>
        \return

*/
void keypb_Turn_On_Off_NEMo_mode(void)
{
   printf("\n");
   printf( "1 => Turn On NEMo mode\n" );
   printf( "2 => Turn Off NEMo mode\n" );

   switch( tcx_getch() )
   {
      case '1':
         app_SrvTurnOnNEMo();
         break;
      case '2':
         app_SrvTurnOffNEMo();
         break;
   }
}

//      ========== keyb_RegisteredHandsets ===========
/*!
        \brief         Get List of subscribed handsets
        \param[in,ou]  <none>
        \return

*/
void  keypb_RegisteredHandsets( void )
{
    ST_APPCMBS_CONTAINER        st_Container;
    ST_IE_RESPONSE              st_Resp;
    int                         i;
    u16                         u16_HsMask = 0;

    memset( &st_Container,0,sizeof(st_Container) );

    printf("\nEnter Mask of handsets in hex (FF for all): ");
    scanf("%hX", &u16_HsMask);

    app_SrvRegisteredHandsets( u16_HsMask, 1 );

    // wait for CMBS target message
    appcmbs_WaitForContainer( CMBS_EV_DSR_HS_SUBSCRIBED_LIST_GET_RES, &st_Container );

    memcpy( &st_Resp, &st_Container.ch_Info, st_Container.n_InfoLen );

    printf ( "\nSubscribed list get %s\n", st_Resp.e_Response == CMBS_RESPONSE_OK ? "Success": "Fail" );
    printf( "Press Any Key!\n" );
    tcx_getch();
}


//      ========== keyb_SetNewHandsetName ===========
/*!
        \brief         Set List of subscribed handsets
        \param[in,ou]  <none>
        \return

*/
void  keyb_SetNewHandsetName( void )
{
    u16     u16_HsID = 0;
    u8      u8_HsName[10];
    u16     u16_Name[10];
    u8      u8_Index = 0;
    u16     u16_NameSize = 0;

    printf("\nEnter handset ID : ");
    scanf("%hX", &u16_HsID);
    printf("\nEnter name (max 8 symbols): ");
    scanf("%s", u8_HsName);

    u16_NameSize = (u16)strlen(u8_HsName) * 2;
    for ( u8_Index = 0; u8_Index < 10; u8_Index++ )
    {
        u16_Name[u8_Index] = (u16)u8_HsName[u8_Index];
    }

    app_SrvSetNewHandsetName( u16_HsID, u16_Name, u16_NameSize, 1 );
}


//      ========== keypb_LineSettingsGet ===========
/*!
        \brief         Get List of Lines settings
        \param[in,ou]  <none>
        \return

*/
void  keypb_LineSettingsGet( void )
{
    ST_APPCMBS_CONTAINER        st_Container;
    ST_IE_RESPONSE              st_Resp;
    int                         i;
    u16                         u16_LinesMask = 0;

    memset( &st_Container,0,sizeof(st_Container) );

    printf("\nEnter Mask of lines in hex (FF for all): ");
    scanf("%hX", &u16_LinesMask);

    app_SrvLineSettingsGet( u16_LinesMask, 1 );

    // wait for CMBS target message
    appcmbs_WaitForContainer( CMBS_EV_DSR_LINE_SETTINGS_LIST_GET_RES, &st_Container );

    memcpy( &st_Resp, &st_Container.ch_Info, st_Container.n_InfoLen );

    printf ( "\nLines settings list get %s\n", st_Resp.e_Response == CMBS_RESPONSE_OK ? "Success": "Fail" );
    printf( "Press Any Key!\n" );
    tcx_getch();
}


//      ========== keypb_LineSettingsSet ===========
/*!
        \brief         Set List of Lines settings
        \param[in,ou]  <none>
        \return

*/
void  keypb_LineSettingsSet( void )
{
    u16 u16_Line_Id = 0;
    u8  u8_LineName[6];
    u8  u8_LineNameLen = 0;
    u8  u8_Index = 0;
    ST_IE_LINE_SETTINGS_LIST    st_LineSettingsList;
    ST_IE_LINE_SETTINGS_TYPE    st_LineSettingsType;

    memset( &st_LineSettingsList, 0, sizeof(st_LineSettingsList) );
    memset( &st_LineSettingsType, 0, sizeof(st_LineSettingsType) );

    printf("\nEnter line ID in hex: ");
    scanf("%hX", &u16_Line_Id);

    st_LineSettingsList.u8_Line_Id = (u8)u16_Line_Id;

    printf("\nEnter type of line setting: \n");
    printf( "1 => Set Line name\n" );
    printf( "2 => Set Handsets mask \n" );
    printf( "3 => Set Call Intrusion\n" );
    printf( "4 => Set Multiline Calls\n" );
    
    switch (tcx_getch())
    {
        case '1':
            st_LineSettingsType.e_LineSettingsType = CMBS_LINE_SETTINGS_TYPE_LINE_NAME;
            printf("\nEnter line name (max 5 symbols): ");
            scanf("%s", u8_LineName);
            u8_LineNameLen = (u8)strlen( (char*)u8_LineName );
            for ( u8_Index = 0; u8_Index < u8_LineNameLen; u8_Index++ )
            {
                st_LineSettingsList.u16_LineName[u8_Index] = (u16)u8_LineName[u8_Index];
            }
            st_LineSettingsList.u8_Line_NameLen = u8_LineNameLen * 2;
            break;

        case '2':
            st_LineSettingsType.e_LineSettingsType = CMBS_LINE_SETTINGS_TYPE_HS_MASK;
            printf("\nEnter Hs mask in hex:  ");
            scanf("%hX", &st_LineSettingsList.u16_Attached_HS);
            break;

        case '3':
            st_LineSettingsType.e_LineSettingsType = CMBS_LINE_SETTINGS_TYPE_CALL_INTRUSION;
            printf("\nEnter 0 - disable, 1 - enable: ");
            scanf("%hd", &st_LineSettingsList.u8_Call_Intrusion);
            break;

        case '4':
            st_LineSettingsType.e_LineSettingsType = CMBS_LINE_SETTINGS_TYPE_MULTIPLE_CALLS;
            printf("\nEnter 0 - disable, 1 - enable: ");
            scanf("%hd", &st_LineSettingsList.u8_Multiple_Calls);
            break;
        
        default:
            return;
    }

    app_SrvLineSettingsSet( &st_LineSettingsType, &st_LineSettingsList, 1 );
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
      printf( "3 => Production  Param Get\n" );
      printf( "4 => Production  Param Set\n" );
      printf( "5 => Handset Page\n" );
      printf( "6 => Stop Paging\n" );
      printf( "7 => Handset Delete\n" );
      printf( "8 => System  Registration Mode\n" );
      printf( "9 => System  Restart\n" );
      printf( "A => System  FW Version Get\n" );
      printf( "B => System  Testmode Get\n" );
      printf( "C => System  Testmode Set\n" );
      printf( "D => SysLog  Start\n" );
      printf( "E => SysLog  Stop\n" );
      printf( "F => SysLog  Read\n" );
      printf( "G => List Of Registered Handsets\n" );
      printf( "H => Set new handset name\n" );
      printf( "I => RF Control\n" );
      printf( "J => Turn On/Off NEMo mode\n" );
      printf( "K => System  Power Off\n" );
      printf( "L => Get Line Settings\n" );
      printf( "M => Set Line Settings\n" );
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
               keypb_ProductionParamGet();
            break;
         case '4':
               keypb_ProductionParamSet();
            break;
         case '5':
               keypb_HandsetPage();
            break;
         case '6':
               keypb_HandsetStopPaging();
            break;
         case '7':
               keypb_HandsetDelete();
            break;
         case '8':
               keypb_SYSRegistrationMode();
            break;
         case '9':
               keypb_SYSRegistrationRestart();
            break;
         case 'a':
         case 'A':
               keypb_SYSFWVersionGet();
            break;
         case 'b':
         case 'B':
               keypb_SYSTestModeGet();
            break;
         case 'c':
         case 'C':
               keypb_SYSTestModeSet();
            break;
         case 'd':
         case 'D':
               keypb_SysLogStart();
            break;
         case 'e':
         case 'E':
               keypb_SysLogStop();
            break;
         case 'f':
         case 'F':
               keypb_SysLogRead();
            break;
         case 'g':
         case 'G':
               keypb_RegisteredHandsets();
            break;
         case 'h':
         case 'H':
               keyb_SetNewHandsetName();
            break;
         case 'i':
         case 'I':
             keypb_RF_Control();
            break;
         case 'j':
         case 'J':
               keypb_Turn_On_Off_NEMo_mode();
            break;
         case 'k':
         case 'K':
             keypb_SYSPowerOff();
            break;
         case 'l':
         case 'L':
             keypb_LineSettingsGet();
            break;
         case 'm':
         case 'M':
             keypb_LineSettingsSet();
            break;



         case 'q':
            n_Keep = FALSE;
            break;

      }
   }
}

//*/
