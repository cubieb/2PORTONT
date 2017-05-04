/*!
*	\file		appservice.c
*	\brief		handles the service relevant interfaces
*	\Author		kelbch
*
*	@(#)	%filespec: -1 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                          \n
*	----------------------------------------------------------------------------\n
*                   Kelbch      1        Initialize \n
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
#include <fcntl.h>
#include <termios.h>
#include <sys/time.h> //we need <sys/select.h>; should be included in <sys/types.h> ???
#include <signal.h>
#endif

#include "cmbs_api.h"
#include "cfr_ie.h"
#include "cfr_mssg.h"

#include "appcmbs.h"

char           _app_HexString2Byte( char * psz_HexString );

//		========== app_ASC2HEX  ===========
/*!
		\brief				 convert a two digits asc ii string to a hex value

		\param[in,out]		 pu8_Digits		 pointer to two digits string

		\return				u8              return value of convertion

*/

u8             app_ASC2HEX ( char * psz_Digits )
{
   u8 u8_Value = 0;
   int i,j=1;

   for ( i=0; i<2; i++ )
   {
      if ( psz_Digits[i] >= '0' && psz_Digits[i] <= '9' )
      {
         u8_Value |= ( psz_Digits[i] - '0' ) << 4 *j;
      }
      else if ( psz_Digits[i] >= 'a' && psz_Digits[i] <= 'f' )
      {
         u8_Value |= ( 0x0a + (psz_Digits[i] - 'a') ) << 4 *j;
      }
      else if ( psz_Digits[i] >= 'A' && psz_Digits[i] <= 'F' )
      {
         u8_Value |= ( 0x0a + (psz_Digits[i] - 'A') ) << 4 *j;
      }
      j = 0;
   }

   return u8_Value;
}

//		========== app_OnHandsetRegistered ===========
/*!
		\brief			a new handset is registered, analyze IPEI, Type, Model, etc.

		\param[in,out]		 *pv_List		 IE list

		\return			<none>

*/
void           app_OnHandsetRegistered( void *pv_List )
{
   PST_CFR_IE_LIST pst_IEList = (PST_CFR_IE_LIST)pv_List;
   ST_IE_HANDSETINFO st_HsInfo;

   memset( &st_HsInfo, 0, sizeof(ST_IE_HANDSETINFO) );
                                 // CMBS target send only one IE to Hoast
   cmbs_api_ie_HandsetInfoGet( cfr_ie_ItemGet(pst_IEList), &st_HsInfo );

   APPCMBS_INFO(( "APPSRV-INFO: Handset:%d IPUI:%02x%02x%02x%02x%02x registered\n",
                  st_HsInfo.u8_Hs,
                  st_HsInfo.u8_IPEI[0], st_HsInfo.u8_IPEI[1], st_HsInfo.u8_IPEI[2],
                  st_HsInfo.u8_IPEI[3], st_HsInfo.u8_IPEI[4] ));

                                 // if token is send signal to upper application
   if ( g_cmbsappl.n_Token )
   {
      appcmbs_ObjectSignal( (void*)&st_HsInfo , sizeof(st_HsInfo), CMBS_IE_HANDSETINFO, CMBS_EV_DSR_HS_REGISTERED );
   }
}

//		========== app_OnRegistrationOpenRSP ===========
/*!
		\brief			response of open registration event

		\param[in]     *pv_List		 IE list

		\return			<none>

*/
void           app_OnRegistrationOpenRSP( void * pv_List )
{
   int i = FALSE;

   if (! app_ResponseCheck( pv_List ) )
   {
      i = TRUE;

      APPCMBS_INFO(( "APPSRV-INFO: Registration Open successful\n")) ;
   }
   else
   {
      APPCMBS_ERROR(( "APPSRV-ERROR: !!! Registration Open failed\n")) ;
   }

   if ( g_cmbsappl.n_Token )
   {
      appcmbs_ObjectSignal( NULL , 0, i, CMBS_EV_DSR_CORD_OPENREG_RES );
   }
}

//		========== app_OnRegistrationCloseRSP ===========
/*!
		\brief			response of close registration event

		\param[in]     *pv_List		 IE list

		\return			<none>

*/
void           app_OnRegistrationCloseRSP( void * pv_List )
{
   int i = FALSE;

   if (! app_ResponseCheck( pv_List ) )
   {
      i = TRUE;

      APPCMBS_INFO(( "APPSRV-INFO: Registration Close successful\n")) ;
   }
   else
   {
      APPCMBS_ERROR(( "APPSRV-ERROR: !!! Registration Close failed\n")) ;
   }

   if ( g_cmbsappl.n_Token )
   {
      appcmbs_ObjectSignal( NULL , 0, i, CMBS_EV_DSR_CORD_CLOSEREG_RES );
   }
}

//		========== app_OnParamGetRSP ===========
/*!
		\brief			response of param get event

		\param[in]     *pv_List		 IE list

		\return			<none>

*/
void           app_OnParamGetRSP ( void * pv_List )
{
   void *   pv_IE = NULL;
   u16      u16_IE;

   if( pv_List )
   {
                                 // collect information elements.
                                 // we expect: CMBS_IE_PARAMETER + CMBS_IE_RESPONSE
      cmbs_api_ie_GetFirst( pv_List, &pv_IE, &u16_IE );

      while( pv_IE != NULL )
      {
         if ( CMBS_IE_PARAMETER == u16_IE )
         {
            ST_IE_PARAMETER st_Param;

            cmbs_api_ie_ParameterGet( pv_IE, &st_Param );
                                 // signal parameter setting to upper application
            if ( g_cmbsappl.n_Token )
            {
               appcmbs_ObjectSignal( (void*)st_Param.pu8_Data, st_Param.u16_DataLen, st_Param.e_Param, CMBS_EV_DSR_PARAM_GET_RES );
            }
         }

         app_IEToString( pv_IE, u16_IE );

         cmbs_api_ie_GetNext( pv_List, &pv_IE, &u16_IE );
      }
   }
}

//		========== app_OnParamSetRSP ===========
/*!
		\brief			response of param set event

		\param[in]     *pv_List		 IE list

		\return			<none>

*/
void           app_OnParamSetRSP( void * pv_List )
{
   void *      pv_IE;
   u16         u16_IE;
   int         i = 0;

   if( pv_List )
   {
                                 // collect information elements.
                                 // we expect: CMBS_IE_PARAMETER + CMBS_IE_RESPONSE
      cmbs_api_ie_GetFirst( pv_List, &pv_IE, &u16_IE );

      while( pv_IE != NULL )
      {
         if( u16_IE == CMBS_IE_PARAMETER )
         {
            APPCMBS_INFO(( "APPSRV-INFO: Param \"" ));
            app_IEToString( pv_IE, u16_IE );
         }

         cmbs_api_ie_GetNext( pv_List, &pv_IE, &u16_IE );
      }
                                 // signal to upper application only IE repsone info
      if ( app_ResponseCheck( pv_List ) )
      {
         i = TRUE;

         APPCMBS_INFO(( "\" set successful\n" )) ;
      }
      else
      {
         APPCMBS_ERROR(( "\" set failed !!!\n" )) ;
      }

      if ( g_cmbsappl.n_Token )
      {
         appcmbs_ObjectSignal( NULL , 0, i, CMBS_EV_DSR_PARAM_SET_RES );
      }

   }
}

//		========== app_OnFwVersionGetRSP ===========
/*!
		\brief			response of FW version get event

		\param[in]     *pv_List		 IE list

		\return			<none>

*/
void           app_OnFwVersionGetRSP( void * pv_List )
{
   ST_APPCMBS_IEINFO
            st_IEInfo;
   void *   pv_IE;
   u16      u16_IE;

   if( pv_List )
   {
                              // collect information elements.
                              // we expect: CMBS_IE_FW_VERSION + CMBS_IE_RESPONSE
      cmbs_api_ie_GetFirst( pv_List, &pv_IE, &u16_IE );
      while( pv_IE != NULL )
      {
         app_IEToString( pv_IE, u16_IE );

         appcmbs_IEInfoGet ( pv_IE, u16_IE, &st_IEInfo );
         if ( u16_IE == CMBS_IE_FW_VERSION )
         {
            printf ( "CMBS_TARGET Version %d %04x\n",st_IEInfo.Info.st_FwVersion.e_SwModule, st_IEInfo.Info.st_FwVersion.u16_FwVersion );

            if ( g_cmbsappl.n_Token )
            {
               appcmbs_ObjectSignal( (void*)&st_IEInfo.Info.st_FwVersion,
                                     sizeof(st_IEInfo.Info.st_FwVersion),
                                     CMBS_IE_FW_VERSION,
                                     CMBS_EV_DSR_FW_VERSION_GET_RES );
            }

         }
         cmbs_api_ie_GetNext( pv_List, &pv_IE, &u16_IE );
      }
   }
}

//		========== app_ServiceEntity ===========
/*!
		\brief		 CMBS entity to handle response information from target side
		\param[in]	 pv_AppRef		 application reference
		\param[in]	 e_EventID		 received CMBS event
		\param[in]	 pv_EventData	 pointer to IE list
		\return	 	 <int>

*/

int            app_ServiceEntity( void * pv_AppRef, E_CMBS_EVENT_ID e_EventID, void * pv_EventData )
{
                                 // ensure that the compiler does not print out a warning
   if (pv_AppRef){};

   switch ( e_EventID )
   {
      case  CMBS_EV_DSR_HS_REGISTERED:
      {
         app_OnHandsetRegistered(pv_EventData);
         break;
      }

      case CMBS_EV_DSR_CORD_OPENREG_RES:
         app_OnRegistrationOpenRSP( pv_EventData );
         break;

      case CMBS_EV_DSR_CORD_CLOSEREG_RES:
         app_OnRegistrationCloseRSP( pv_EventData );
         break;

      case CMBS_EV_DSR_PARAM_GET_RES:
         app_OnParamGetRSP( pv_EventData );
         break;

      case CMBS_EV_DSR_PARAM_SET_RES:
         app_OnParamSetRSP( pv_EventData );
         break;

      case CMBS_EV_DSR_FW_VERSION_GET_RES:
         app_OnFwVersionGetRSP( pv_EventData );
         break;

      case CMBS_EV_DSR_HS_PAGE_RES:
      case CMBS_EV_DSR_HS_DELETE_RES:
      {
         u8 u8_Resp = app_ResponseCheck(pv_EventData);
         if( u8_Resp == CMBS_RESPONSE_OK )
         {
            APPCMBS_INFO(( "APPCMBS-SRV: INFO Response: OK\n"));
         }
         else
         {
            APPCMBS_ERROR(("APPCMBS-SRV: ERROR !!! Response ERROR\n"));
         }
         break;
      }

      default:
         return FALSE;
   }

  return TRUE;
}

//		==========  app_HandsetMap ===========
/*!
		\brief			   convert handset string to used handset bit mask
                        does also support "all" and "none" as input

		\param[in,out]		psz_Handsets	pointer to handset string

		\return				<u16>    return handset bit mask

*/

u16            app_HandsetMap ( char * psz_Handsets )
{
   u16   u16_Handset = 0;
   unsigned int   i;

   if ( !strcmp ( psz_Handsets, "all\0" ) )
      return 0xFFFF;

   if ( !strcmp ( psz_Handsets, "none\0") )
   {
      return 0;
   }

   for (i=0; i < strlen(psz_Handsets); i++ )
   {
      if ( psz_Handsets[i]>= '1' && psz_Handsets[i]<= '9' )
      {
         u16_Handset |= (1<<(psz_Handsets[i]-'1'));
      }
   }

   APPCMBS_INFO (( "APPCMBS-SRV: INFO u16_Handset %04x\n", u16_Handset ));

   return u16_Handset;
}

//		========== app_SrvHandsetDelete  ===========
/*!
		\brief				 call CMBS function of handset delete

		\param[in,out]		 psz_Handsets     pointer to parameter string,e.g."1234" or "all"

		\return				 <E_CMBS_RC>

*/
E_CMBS_RC      app_SrvHandsetDelete ( char * psz_Handsets )
{
   return cmbs_dsr_hs_Delete( g_cmbsappl.pv_CMBSRef, app_HandsetMap(psz_Handsets) );
}

//		========== app_SrvHandsetPage  ===========
/*!
		\brief				 call CMBS function of handset page
		\param[in,out]		 psz_Handsets     pointer to parameter string,e.g."1234" or "all" or "none"
		\return				 <E_CMBS_RC>
      \note              stopp pagine, re-call this function with "none"

*/
E_CMBS_RC      app_SrvHandsetPage( char * psz_Handsets )
{
   return cmbs_dsr_hs_Page( g_cmbsappl.pv_CMBSRef, app_HandsetMap(psz_Handsets) );
}

//		========== app_SrvSubscriptionOpen  ===========
/*!
		\brief				 call CMBS function of open registration
		\param[in,out]		 <none>
		\return				 <E_CMBS_RC>

*/
E_CMBS_RC      app_SrvSubscriptionOpen( void )
{
   return cmbs_dsr_cord_OpenRegistration( g_cmbsappl.pv_CMBSRef );
}

//		========== app_SrvSubscriptionClose  ===========
/*!
		\brief				 call CMBS function of open registration
		\param[in,out]		 <none>
		\return				 <E_CMBS_RC>

*/
E_CMBS_RC      app_SrvSubscriptionClose( void )
{
   return cmbs_dsr_cord_CloseRegistration( g_cmbsappl.pv_CMBSRef);
}

//		========== app_SrvPINCodeGet  ===========
/*!
		\brief				 get current registration code
		\param[in,out]		 u32_Token     TRUE, if upper application waits for answer
		\return				 <E_CMBS_RC>

*/
/*
E_CMBS_RC      app_SrvPINCodeGet( u32 u32_Token )
{
   appcmbs_PrepareRecvAdd ( u32_Token );

   return cmbs_dsr_param_Get( g_cmbsappl.pv_CMBSRef, CMBS_PARAM_AUTH_PIN );
}
*/
//		========== app_SrvPINCodeSet  ===========
/*!
		\brief				 set authentication PIN code, registration code
		\param[in,out]		 psz_PIN    pointer to PIN string, e.g. "ffff1590" for PIN 1590
		\return				 <E_CMBS_RC>

*/
E_CMBS_RC      app_SrvPINCodeSet( char * psz_PIN )
{
   unsigned int      i;

   u8       szPinCode[CMBS_PARAM_PIN_CODE_LENGTH] = {0, 0, 0, 0};

   if ( strlen ( psz_PIN ) ==(CMBS_PARAM_PIN_CODE_LENGTH*2) )
   {
      printf ( "PIN %s\n",psz_PIN );
      for (i=0; i < (CMBS_PARAM_PIN_CODE_LENGTH*2); i++ )
      {
         printf( "%c => %d BCD %02x\n",psz_PIN[i],(i/2) , szPinCode[i/2]);

         if ( psz_PIN[i] == 'f' )
         {
            if ( (i&0x01) )
            {
               szPinCode[i/2] |= 0x0F;
            }
            else
            {
               szPinCode[i/2] |= 0xF0;
            }

         }
         else if ( psz_PIN[i] >= '0' && psz_PIN[i]<='9' )
         {
            if ( (i&0x01) )
            {
               szPinCode[i/2] |= (psz_PIN[i] -'0');
            }
            else
            {
               szPinCode[i/2] |= (psz_PIN[i] -'0') << 4;
            }
         }
         else
         {
            if ( (i&0x01) )
            {
               szPinCode[i/2] |= 1;
            }
            else
            {
               szPinCode[i/2] |= 1 << 4;
            }

         }
      }
   }
   printf( "%02x %02x %02x\n", szPinCode[0], szPinCode[1], szPinCode[2]);
   return cmbs_dsr_param_Set( g_cmbsappl.pv_CMBSRef, CMBS_PARAM_AUTH_PIN, szPinCode, CMBS_PARAM_PIN_CODE_LENGTH );
}

//		========== app_SrvRFPIGet  ===========
/*!
		\brief				 call CMBS function of parameter get
		\param[in,out]		 u32_Token     TRUE, if upper application waits for answer
		\return				 <E_CMBS_RC>

*/
/*
E_CMBS_RC      app_SrvRFPIGet( u32 u32_Token )
{
   appcmbs_PrepareRecvAdd ( u32_Token );

   return cmbs_dsr_param_Get( g_cmbsappl.pv_CMBSRef, CMBS_PARAM_RFPI );
}
*/
//		========== app_SrvTestModeGet  ===========
/*!
		\brief				 get test mode state
		\param[in,out]		 u32_Token     TRUE, if upper application waits for answer
		\return				 <E_CMBS_RC>

*/
E_CMBS_RC      app_SrvTestModeGet( u32 u32_Token )
{
   appcmbs_PrepareRecvAdd ( u32_Token );

   return cmbs_dsr_param_Get( g_cmbsappl.pv_CMBSRef, CMBS_PARAM_TEST_MODE );
}

//		========== app_SrvTestModeGet  ===========
/*!
		\brief				 set TBR 6 on
		\param[in,out]		 <none>
		\return				 <E_CMBS_RC>

*/
E_CMBS_RC      app_SrvTestModeSet( void )
{
   u8 u8_TestMode = 0x81;

   return cmbs_dsr_param_Set( g_cmbsappl.pv_CMBSRef, CMBS_PARAM_TEST_MODE, &u8_TestMode, CMBS_PARAM_TEST_MODE_LENGTH );
}


//		========== _app_HexString2Byte  ===========
/*!
      \brief            convert a 2 bytes hex character string to binary
      \param[in]        psz_HexString     2 bytes hex character string
      \return           1 byte
*/
char           _app_HexString2Byte( char * psz_HexString )
{
   char        c;

                                    // first character
   c = (psz_HexString[0] >= 'A' ? ((psz_HexString[0] & 0xdf) - 'A')+10 : (psz_HexString[0] - '0'));

   c <<= 4;
                                    // second character
   c += (psz_HexString[1] >= 'A' ? ((psz_HexString[1] & 0xdf) - 'A')+10 : (psz_HexString[1] - '0'));

   return c;
}


//		========== app_SrvParamSet  ===========
/*!
      \brief            set parameter value
      \param[in]        e_Param           pre-defined parameter settings
      \param[in]        psz_Data          user input string with hex characters
      \param[in]        u16_szDataLength  length of user input string
      \return           <E_CMBS_RC>
*/
/*
E_CMBS_RC      app_SrvParamSet( E_CMBS_PARAM e_Param, char * psz_Data, u16 u16_szDataLength )
{
   u8          u8_Data[10];
   int         s, x;

   for( s = 0, x = 0; s < u16_szDataLength && psz_Data[s]; x++, s += 2 )
   {
      u8_Data[x] = _app_HexString2Byte( &psz_Data[s] );
   }

   return cmbs_dsr_param_Set( g_cmbsappl.pv_CMBSRef, e_Param, u8_Data, u16_szDataLength / 2 );
}
*/

//		========== app_SrvParamGet  ===========
/*!
		\brief				 get parameter information
		\param[in,out]		 e_Param       pre-defined parameter settings
		\param[in,out]		 u16_Pos       position of EEprom offset
		\param[in,out]		 u16_Length    length of to read area
		\param[in,out]		 u32_Token     TRUE, if upper application waits for answer
		\return				 <E_CMBS_RC>

*/
E_CMBS_RC      app_SrvParamGet( E_CMBS_PARAM e_Param, u16 u16_Pos, u16 u16_Length, u32 u32_Token )
{
                                 // ensure that the compiler does not print out a warning
   if ( u16_Pos ){};
   if (u16_Length){};

   if ( e_Param != CMBS_PARAM_UNKNOWN)
   {
      appcmbs_PrepareRecvAdd ( u32_Token );

      if ( e_Param != CMBS_PARAM_FLEX )
      {
         return cmbs_dsr_param_Get( g_cmbsappl.pv_CMBSRef,e_Param);
      }
      else
      {
         return cmbs_dsr_param_area_Get( g_cmbsappl.pv_CMBSRef, CMBS_MEM_EEPROM, (u32)u16_Pos, u16_Length );
      }
   }
   else
   {
      APPCMBS_ERROR(( "ERROR: app_SrvParamGet offset is not implemented, yet \n"));
   }

   return CMBS_RC_ERROR_GENERAL;
}

//		========== app_SrvParamSet  ===========
/*!
      \brief            set parameter value
      \param[in]        e_Param           pre-defined parameter settings
		\param[in]        u16_Pos           position of EEprom offset
		\param[in]        u16_Length        length of data
      \param[in]        pu8_Data          user input data
		\param[in]        u32_Token         TRUE tells the upper application to wait for an answer
      \return           <E_CMBS_RC>
*/
E_CMBS_RC      app_SrvParamSet( E_CMBS_PARAM e_Param, u16 u16_Pos, u16 u16_Length, u8* pu8_Data, u32 u32_Token )
{
   if ( e_Param != CMBS_PARAM_UNKNOWN)
   {
      appcmbs_PrepareRecvAdd ( u32_Token );

      if( e_Param != CMBS_PARAM_FLEX )
      {
         return cmbs_dsr_param_Set( g_cmbsappl.pv_CMBSRef, e_Param, pu8_Data, u16_Length );
      }
      else
      {
         return cmbs_dsr_param_area_Set( g_cmbsappl.pv_CMBSRef, CMBS_MEM_EEPROM,
                                             (u32)u16_Pos, pu8_Data, u16_Length );
      }
   }
   else
   {
      APPCMBS_ERROR(( "ERROR: app_SrvParamSet offset is not implemented, yet \n"));
   }

   return CMBS_RC_ERROR_GENERAL;

}

//		========== app_SrvFWVersionGet  ===========
/*!
		\brief				 get firmware version
		\param[in,out]		 u32_Token     TRUE, if upper application waits for answer
		\return				 <E_CMBS_RC>

*/
E_CMBS_RC      app_SrvFWVersionGet( u32 u32_Token )
{
   appcmbs_PrepareRecvAdd ( u32_Token );

   return cmbs_dsr_fw_VersionGet( g_cmbsappl.pv_CMBSRef, CMBS_MODULE_CMBS );
}


//		========== app_SrvLogBufferRead  ===========
/*!
		\brief				 read log buffer
		\param[in,out]		 u32_Token     TRUE, if upper application waits for answer
		\return				 <E_CMBS_RC>

*/
E_CMBS_RC      app_SrvLogBufferRead( u32 u32_Token )
{
   appcmbs_PrepareRecvAdd ( u32_Token );

   return cmbs_dsr_LogBufferRead( g_cmbsappl.pv_CMBSRef, CMBS_MODULE_CMBS );
}


//		========== app_SrvSystemReboot  ===========
/*!
		\brief				 reboot CMBS Target
		\param[in,out]		 <none>
		\return				 <E_CMBS_RC>
      \note              also re-register CMBS-API to system!

*/
E_CMBS_RC      app_SrvSystemReboot( void )
{
   return cmbs_dsr_sys_Reset( g_cmbsappl.pv_CMBSRef );
}

//*/
