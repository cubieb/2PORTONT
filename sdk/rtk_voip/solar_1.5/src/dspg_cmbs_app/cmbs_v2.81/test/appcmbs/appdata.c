/*!
*  \file       appdata.c
*	\brief		handles CAT-iq data functioality
*	\Author		stein
*
*	@(#)	%filespec: appdata.c~DMZD53#3 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                          \n
*	----------------------------------------------------------------------------\n
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if ! defined ( WIN32 )
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/time.h>
#include <signal.h>
#endif

#include "cmbs_api.h"
#include "cfr_ie.h"
#include "cfr_mssg.h"
#include "cmbs_str.h"

#include "appcmbs.h"

extern u16        app_HandsetMap( char * psz_Handsets );


E_CMBS_RC         app_DataSessionOpen( char * psz_Handset )
{
   ST_DATA_SESSION_TYPE
                  st_DataSessionType;
   u16            u16_Handset = app_HandsetMap( psz_Handset );

   st_DataSessionType.e_ChannelType = CMBS_DATA_CHANNEL_IWU;   
   st_DataSessionType.e_ServiceType = CMBS_DATA_SERVICE_TRANSPARENT;
   
   return cmbs_dsr_hs_DataSessionOpen( g_cmbsappl.pv_CMBSRef,
                                       &st_DataSessionType,
                                       u16_Handset );
}


E_CMBS_RC         app_DataSend( u16 u16_SessionId, ST_IE_DATA * pst_Data )
{   
   return cmbs_dsr_hs_DataSend( g_cmbsappl.pv_CMBSRef,
                                u16_SessionId, pst_Data->pu8_Data, pst_Data->u16_DataLen );
}


E_CMBS_RC         app_DataSessionClose( u16 u16_SessionId )
{
   return cmbs_dsr_hs_DataSessionClose( g_cmbsappl.pv_CMBSRef,
                                        u16_SessionId );
}

                                        
//		========== app_DataEntity ===========
/*!
		\brief		 CMBS entity to handle response information from target side
		\param[in]	 pv_AppRef		 application reference
		\param[in]	 e_EventID		 received CMBS event
		\param[in]	 pv_EventData	 pointer to IE list
		\return	 	 <int>

*/
int               app_DataEntity( void * pv_AppRef, E_CMBS_EVENT_ID e_EventID, void * pv_EventData )
{
   void *         pv_IE = NULL;
   u16            u16_IE;
   ST_IE_DATA_SESSION_TYPE
                  st_SessionType;
   u16            u16_SessionId, u16_Handsets;
   ST_IE_DATA     st_Data;
   int            i;

   if( pv_AppRef ){}; // eliminate compiler warning

   if( e_EventID == CMBS_EV_DSR_HS_DATA_SESSION_OPEN      ||
       e_EventID == CMBS_EV_DSR_HS_DATA_SESSION_OPEN_RES  ||
       e_EventID == CMBS_EV_DSR_HS_DATA_SESSION_CLOSE     ||
       e_EventID == CMBS_EV_DSR_HS_DATA_SESSION_CLOSE_RES ||
       e_EventID == CMBS_EV_DSR_HS_DATA_RECEIVE           ||
       e_EventID == CMBS_EV_DSR_HS_DATA_RECEIVE_RES )
   {
      cmbs_api_ie_GetFirst( pv_EventData, &pv_IE, &u16_IE );
      while( pv_IE != NULL )
      {
         printf( "%s: ", getstr_E_CMBS_IE( u16_IE ) );

         if( u16_IE == CMBS_IE_DATA_SESSION_ID )
         {
            cmbs_api_ie_DataSessionIdGet( pv_IE, &u16_SessionId );
            printf( "%d", u16_SessionId );
         }

         if( u16_IE == CMBS_IE_DATA_SESSION_TYPE )
         {
            cmbs_api_ie_DataSessionTypeGet( pv_IE, &st_SessionType );
            printf( "ChannelType:%d ServiceType:%d",
                     st_SessionType.e_ChannelType,
                     st_SessionType.e_ServiceType );
         }

         if( u16_IE == CMBS_IE_HANDSETS )
         {
            cmbs_api_ie_HandsetsGet( pv_IE, &u16_Handsets );
            
            for( i = 0; i < 16; i++ )
            {
               if( u16_Handsets & (1<<i ) )   
                  break;
            }
            printf( "%d", i+1 );
         }

         if( u16_IE == CMBS_IE_DATA )
         {
            memset( &st_Data, 0, sizeof(st_Data)); 
            cmbs_api_ie_DataGet( pv_IE, &st_Data );
            
            if( st_Data.pu8_Data )
            {
               printf( "Length:%d\n", st_Data.u16_DataLen );
               for( i=0; i<st_Data.u16_DataLen; i++ )
               {
                  printf( " %02X", st_Data.pu8_Data[i] );
               }
               printf( "\n" );
            }
         }

         if( u16_IE == CMBS_IE_RESPONSE )
         {
            ST_IE_RESPONSE st_Response;
            cmbs_api_ie_ResponseGet( pv_IE, &st_Response );
            printf( "%s", st_Response.e_Response == CMBS_RESPONSE_OK ? "Ok":"Error" );
         }

         printf( "\n" );
      
         cmbs_api_ie_GetNext( pv_EventData, &pv_IE, &u16_IE );
      }

      return TRUE;
   }
   else
      return FALSE;
   
}
//*/
