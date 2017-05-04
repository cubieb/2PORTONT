/*!
*  \file       applistacc.c
*	\brief		handles CAT-iq list access functioality
*	\Author		stein
*
*	@(#)	%filespec: applistacc.c-4 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                          \n
*	----------------------------------------------------------------------------\n
*
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
#include <sys/time.h>
#include <signal.h>
#endif

#include "cmbs_api.h"
#include "cmbs_int.h"
#include "cfr_ie.h"
#include "cfr_mssg.h"
#include "cmbs_str.h"

#include "appcmbs.h"


void app_LaDataPacketSend ( u16 SessionId, u8 u8_IsLast );
u16 app_LaNrOfListEntries ( u16 u16_SessionId );
int app_LaGetSessionIndex ( u16 u16_SessionId );
int app_LaGetNextFreeSessionIndex ( u16 u16_SessionId );
void app_LaInitExampleLists( void );
int app_LaIsValidList ( u16 u16_ListId );

#define APP_LA_MAX_NR_OF_SESSIONS            10
#define APP_LA_MAX_MISSED_CALL_LIST_ENTRIES  20

typedef struct
{
   u8 u8_EntryFilled;
   u16 u16_SessionId;
   u16 u16_CMBS_LAEntryNr;
   u16 u16_CMBS_LAEndIndex;
   u16 u16_ListId;
   u16 u16_requestedFields;
} st_APP_LA_Session, *pst_APP_LA_Session;

st_APP_LA_Session g_st_APP_LA_Session[APP_LA_MAX_NR_OF_SESSIONS];

u8 u8_APP_LA_Initialized = 0;

typedef struct
{
   u8           u8_EntryFilled;
   u8           pu8_Number[128];
   u8           pu8_Name[128];
   ST_DATE_TIME st_DateTime;
   u8           u8_New;
   u8           pu8_LineName[128];
   u8           u8_LineId;
   u8           u8_NumberOfCalls;
} st_APP_LA_MissedCallListEntry, *pst_APP_LA_MissedCallListEntry;

st_APP_LA_MissedCallListEntry g_st_APP_LA_MissedCallListEntry[APP_LA_MAX_MISSED_CALL_LIST_ENTRIES + 1]; // Index from 1 to APP_LA_MAX_MISSED_CALL_LIST_ENTRIES


//		========== app_LaEntity ===========
/*!
		\brief		 CMBS entity to handle response information from target side
		\param[in]	 pv_AppRef		 application reference
		\param[in]	 e_EventID		 received CMBS event
		\param[in]	 pv_EventData	 pointer to IE list
		\return	 	 <int>

*/
int               app_LaEntity( void * pv_AppRef, E_CMBS_EVENT_ID e_EventID, void * pv_EventData )
{
   void *          pv_IE = NULL;
   u16             u16_IE;
   u16             u16_SessionId, u16_Handsets, u16_ListId;
   ST_IE_LA_FIELDS st_LaFields;
   int             i;
   u16             u16_EntryIndex;
   u16             u16_EntryCount;
   int             i_SessionIndex;


   if ( !u8_APP_LA_Initialized )
   {
      memset(g_st_APP_LA_Session, 0, sizeof(g_st_APP_LA_Session));
      memset(g_st_APP_LA_MissedCallListEntry, 0, sizeof(g_st_APP_LA_MissedCallListEntry));
      app_LaInitExampleLists();
      u8_APP_LA_Initialized = 1;
   }

   if( pv_AppRef ){}; // eliminate compiler warning

   if( e_EventID == CMBS_EV_DSR_LA_SESSION_START           ||
       e_EventID == CMBS_EV_DSR_LA_SESSION_END             ||
       e_EventID == CMBS_EV_DSR_LA_QUERY_SUPP_ENTRY_FIELDS ||
       e_EventID == CMBS_EV_DSR_LA_READ_ENTRIES            ||
       e_EventID == CMBS_EV_DSR_LA_SEARCH_ENTRIES          ||
       e_EventID == CMBS_EV_DSR_LA_EDIT_ENTRY              ||
       e_EventID == CMBS_EV_DSR_LA_SAVE_ENTRY              ||
       e_EventID == CMBS_EV_DSR_LA_DELETE_ENTRY            ||
       e_EventID == CMBS_EV_DSR_LA_DELETE_LIST             ||
       e_EventID == CMBS_EV_DSR_LA_DATA_PACKET_RECEIVE     ||
       e_EventID == CMBS_EV_DSR_LA_DATA_PACKET_SEND_RES )
   {
      cmbs_api_ie_GetFirst( pv_EventData, &pv_IE, &u16_IE );
      while( pv_IE != NULL )
      {
         printf( "%s: ", getstr_E_CMBS_IE( u16_IE ) );

         if( u16_IE == CMBS_IE_LA_SESSION_ID )
         {
            cmbs_api_ie_LASessionIdGet( pv_IE, &u16_SessionId );
            printf( "%d", u16_SessionId );
         }
         else if( u16_IE == CMBS_IE_LA_LIST_ID )
         {
            cmbs_api_ie_LAListIdGet( pv_IE, &u16_ListId );
            printf( "%d", u16_ListId );
         }
         else if( u16_IE == CMBS_IE_LA_ENTRY_INDEX )
         {
            cmbs_api_ie_LAEntryIndexGet( pv_IE, &u16_EntryIndex );
            printf( "u16_EntryIndex=%d", u16_EntryIndex );
         }
         else if( u16_IE == CMBS_IE_LA_ENTRY_COUNT )
         {
            cmbs_api_ie_LAEntryCountGet( pv_IE, &u16_EntryCount );
            printf( "u16_EntryCount=%d", u16_EntryCount );
         }
         else if( u16_IE == CMBS_IE_HANDSETS )
         {
            cmbs_api_ie_HandsetsGet( pv_IE, &u16_Handsets );

            for( i = 0; i < 16; i++ )
            {
               if( u16_Handsets & (1<<i ) )
                  break;
            }
            printf( "%d", i+1 );
         }
         else if( u16_IE == CMBS_IE_LA_FIELDS      ||
                  u16_IE == CMBS_IE_LA_SORT_FIELDS ||
                  u16_IE == CMBS_IE_LA_EDIT_FIELDS ||
                  u16_IE == CMBS_IE_LA_CONST_FIELDS )
         {
            printf("LA_xxx_FIELDS:\n");

            cmbs_api_ie_LAFieldsGet( pv_IE, &st_LaFields );

            printf( "Length:%d\n", st_LaFields.u16_Length );

            printf( "FieldIds:" );

            if ( st_LaFields.u16_Length == 0 )
            {
               printf(" (none)\n");
            }
            else
            {
               for( i=0; i < st_LaFields.u16_Length; i++ )
               {
                  printf( " %2d", st_LaFields.pu16_FieldId[i] );
               }
               printf( "\n" );
            }
         }
         else if( u16_IE == CMBS_IE_RESPONSE )
         {
            ST_IE_RESPONSE st_Response;
            cmbs_api_ie_ResponseGet( pv_IE, &st_Response );
            printf( "%s", st_Response.e_Response == CMBS_RESPONSE_OK ? "Ok":"Error" );
         }
         printf( "\n" );

         cmbs_api_ie_GetNext( pv_EventData, &pv_IE, &u16_IE );
      }

      if( e_EventID == CMBS_EV_DSR_LA_SESSION_START )
      {
         E_CMBS_RESPONSE e_Response = CMBS_RESPONSE_OK;
         E_CMBS_LA_START_SESSION_ERRORS
                         e_RejectReason = CMBS_LA_START_SESSION_ERROR_MAX;
         u16             u16_NrOfEntries;
         int             i_SessionIndex;

         printf( "Start Session SessionId=%x\n", u16_SessionId );

         i_SessionIndex = app_LaGetSessionIndex ( u16_SessionId );
         if ( i_SessionIndex < APP_LA_MAX_NR_OF_SESSIONS )
         {
            printf( "ERROR: u16_SessionId=%x already in use\n", u16_SessionId );
            e_Response     = CMBS_RESPONSE_ERROR;
            e_RejectReason = CMBS_LA_START_SESSION_ERROR_NOT_ENOUGH_RESOURCES;
         }
         else
         {
            i_SessionIndex = app_LaGetNextFreeSessionIndex( u16_SessionId );
            if ( i_SessionIndex == APP_LA_MAX_NR_OF_SESSIONS )
            {
               printf("ERROR: No free Index in Session Table on HOST side\n");
               e_Response     = CMBS_RESPONSE_ERROR;
               e_RejectReason = CMBS_LA_START_SESSION_ERROR_NOT_ENOUGH_RESOURCES;
            }
            else
            {
               g_st_APP_LA_Session[i_SessionIndex].u16_ListId = u16_ListId;
            }
         }

         if ( !app_LaIsValidList ( u16_ListId ) )
         {
            printf("ERROR: No valid list selected\n");
            e_Response     = CMBS_RESPONSE_ERROR;
            e_RejectReason = CMBS_LA_START_SESSION_ERROR_LIST_NOT_SUPPORTED;

         }

         if ( e_Response == CMBS_RESPONSE_OK )
         {
            u16_NrOfEntries = app_LaNrOfListEntries ( u16_SessionId );

            printf("List contains %d entries\n", u16_NrOfEntries);

            st_LaFields.u16_Length = 2;
            st_LaFields.pu16_FieldId[0] = 1;
            st_LaFields.pu16_FieldId[1] = 2;
         }
         else
         {
            u16_NrOfEntries = 0;
            st_LaFields.u16_Length = 0;
         }

         return cmbs_dsr_la_SessionStartRes( g_cmbsappl.pv_CMBSRef, u16_SessionId,
                                             u16_NrOfEntries, &st_LaFields, e_Response, e_RejectReason );

      }
      else if( e_EventID == CMBS_EV_DSR_LA_SESSION_END )
      {
         E_CMBS_RESPONSE e_Response = CMBS_RESPONSE_OK;
         E_CMBS_LA_NACK_ERRORS
                         e_RejectReason = CMBS_LA_NACK_MAX;

         printf("End Session SessionId=%x\n", u16_SessionId);

         i_SessionIndex = app_LaGetSessionIndex ( u16_SessionId );
         if ( i_SessionIndex == APP_LA_MAX_NR_OF_SESSIONS )
         {
            printf("ERROR: u16_SessionId=%x not known\n", u16_SessionId );
            e_Response     = CMBS_RESPONSE_ERROR;
            e_RejectReason = CMBS_LA_NACK_INVALID_SESSION;
         }
         else
         {
            // Mark entry in Session Table as unused
            g_st_APP_LA_Session[i_SessionIndex].u8_EntryFilled = 0;
         }

         return cmbs_dsr_la_SessionEndRes( g_cmbsappl.pv_CMBSRef, u16_SessionId, e_Response, e_RejectReason );
      }
      else if( e_EventID == CMBS_EV_DSR_LA_QUERY_SUPP_ENTRY_FIELDS )
      {
         E_CMBS_RESPONSE e_Response = CMBS_RESPONSE_OK;
         E_CMBS_LA_NACK_ERRORS
                         e_RejectReason = CMBS_LA_NACK_MAX;
         ST_LA_FIELDS    st_LANonEditFields;
         ST_LA_FIELDS    st_LAEditFields;

         printf("Query Supported Fields\n");

         i_SessionIndex = app_LaGetSessionIndex( u16_SessionId );
         if ( i_SessionIndex == APP_LA_MAX_NR_OF_SESSIONS )
         {
            printf("ERROR: Wrong SessionId=%x\n", u16_SessionId );
            e_Response     = CMBS_RESPONSE_ERROR;
            e_RejectReason = CMBS_LA_NACK_INVALID_SESSION;
         }
         else
         {
            printf("Fields from list %d\n", g_st_APP_LA_Session[i_SessionIndex].u16_ListId);
         }

         st_LANonEditFields.u16_Length = 5;
         st_LANonEditFields.pu16_FieldId[0] = 3;
         st_LANonEditFields.pu16_FieldId[1] = 4;
         st_LANonEditFields.pu16_FieldId[2] = 5;
         st_LANonEditFields.pu16_FieldId[3] = 6;
         st_LANonEditFields.pu16_FieldId[4] = 7;

         st_LAEditFields.u16_Length = 2;
         st_LAEditFields.pu16_FieldId[0] = 1; // Number
         st_LAEditFields.pu16_FieldId[1] = 2; // Name

         return cmbs_dsr_la_QuerySupEntryFieldsRes( g_cmbsappl.pv_CMBSRef, u16_SessionId,
                                                    &st_LANonEditFields, &st_LAEditFields, e_Response, e_RejectReason );
      }
      else if( e_EventID == CMBS_EV_DSR_LA_READ_ENTRIES )
      {
         E_CMBS_RESPONSE e_Response = CMBS_RESPONSE_OK;
         E_CMBS_LA_NACK_ERRORS
                         e_RejectReason = CMBS_LA_NACK_MAX;
         u8              u8_IsLast;

         printf("Read entries from index %d to %d\n", u16_EntryIndex, u16_EntryIndex + u16_EntryCount - 1);

         i_SessionIndex = app_LaGetSessionIndex( u16_SessionId );
         if ( i_SessionIndex == APP_LA_MAX_NR_OF_SESSIONS )
         {
            printf("ERROR: Wrong SessionId=%x\n", u16_SessionId );
            e_Response     = CMBS_RESPONSE_ERROR;
            e_RejectReason = CMBS_LA_NACK_INVALID_SESSION;
         }
         else
         {
            printf("Reading from list %d\n", g_st_APP_LA_Session[i_SessionIndex].u16_ListId);
         }

         g_st_APP_LA_Session[i_SessionIndex].u16_CMBS_LAEntryNr  = u16_EntryIndex;
         g_st_APP_LA_Session[i_SessionIndex].u16_CMBS_LAEndIndex = u16_EntryIndex + u16_EntryCount - 1;

         g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields = 0;

         for(i = 0; i < st_LaFields.u16_Length; i++)
         {
            g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields |= (1 << st_LaFields.pu16_FieldId[i]);
         }

         printf("g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields=%x\n", g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields);

         cmbs_dsr_la_ReadEntriesRes( g_cmbsappl.pv_CMBSRef, u16_SessionId,
                                     u16_EntryIndex, u16_EntryCount, e_Response, e_RejectReason );

         if ( e_Response == CMBS_RESPONSE_OK )
         {
            u8_IsLast = (g_st_APP_LA_Session[i_SessionIndex].u16_CMBS_LAEntryNr == g_st_APP_LA_Session[i_SessionIndex].u16_CMBS_LAEndIndex) ? 1 : 0;

            app_LaDataPacketSend ( u16_SessionId, u8_IsLast );
         }
      }
      else if( e_EventID == CMBS_EV_DSR_LA_SEARCH_ENTRIES )
      {
         u16             u16_EntryStartIndex = 0;
         u16             u16_NumOfEntries = 1;
         u8              u8_IsLast;
         E_CMBS_RESPONSE e_Response = CMBS_RESPONSE_OK;
         E_CMBS_LA_NACK_ERRORS
                         e_RejectReason = CMBS_LA_NACK_MAX;

         printf("Search for entry\n");

         i_SessionIndex = app_LaGetSessionIndex( u16_SessionId );
         if ( i_SessionIndex == APP_LA_MAX_NR_OF_SESSIONS )
         {
            printf("ERROR: Wrong SessionId=%x\n", u16_SessionId );
            e_Response     = CMBS_RESPONSE_ERROR;
            e_RejectReason = CMBS_LA_NACK_INVALID_SESSION;
         }
         else
         {
            printf("Searching in list %d\n", g_st_APP_LA_Session[i_SessionIndex].u16_ListId);
         }

         g_st_APP_LA_Session[i_SessionIndex].u16_CMBS_LAEntryNr  = 1;
         g_st_APP_LA_Session[i_SessionIndex].u16_CMBS_LAEndIndex = 1;

         g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields = 0;

         // ALTDV
         cmbs_api_ie_GetFirst( pv_EventData, &pv_IE, &u16_IE );
         cmbs_api_ie_LAFieldsGet( pv_IE, &st_LaFields );

         for(i = 0; i < st_LaFields.u16_Length; i++)
         {
            g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields |= (1 << st_LaFields.pu16_FieldId[i]);
         }

         printf("g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields=%x\n", g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields);

         cmbs_dsr_la_SearchEntriesRes( g_cmbsappl.pv_CMBSRef, u16_SessionId,
                                       u16_EntryStartIndex, u16_NumOfEntries, e_Response, e_RejectReason );

         if ( e_Response == CMBS_RESPONSE_OK )
         {
            u8_IsLast = (g_st_APP_LA_Session[i_SessionIndex].u16_CMBS_LAEntryNr == g_st_APP_LA_Session[i_SessionIndex].u16_CMBS_LAEndIndex) ? 1 : 0;

            app_LaDataPacketSend ( u16_SessionId, u8_IsLast );
         }
      }
      else if( e_EventID == CMBS_EV_DSR_LA_EDIT_ENTRY )
      {
         u8              u8_IsLast;
         E_CMBS_RESPONSE e_Response = CMBS_RESPONSE_OK;
         E_CMBS_LA_NACK_ERRORS
                         e_RejectReason = CMBS_LA_NACK_MAX;

         printf("Edit Entry %d\n", u16_EntryIndex);

         i_SessionIndex = app_LaGetSessionIndex( u16_SessionId );
         if ( i_SessionIndex == APP_LA_MAX_NR_OF_SESSIONS )
         {
            printf("ERROR: Wrong SessionId=%x\n", u16_SessionId );
            e_Response     = CMBS_RESPONSE_ERROR;
            e_RejectReason = CMBS_LA_NACK_INVALID_SESSION;
         }
         else
         {
            printf("Edit entry in list %d\n", g_st_APP_LA_Session[i_SessionIndex].u16_ListId);
         }

         g_st_APP_LA_Session[i_SessionIndex].u16_CMBS_LAEntryNr  = u16_EntryIndex;
         g_st_APP_LA_Session[i_SessionIndex].u16_CMBS_LAEndIndex = u16_EntryIndex;

         g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields = 0;

         for(i = 0; i < st_LaFields.u16_Length; i++)
         {
            g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields |= (1 << st_LaFields.pu16_FieldId[i]);
         }

         printf("g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields=%x\n", g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields);

         cmbs_dsr_la_EditEntryRes( g_cmbsappl.pv_CMBSRef, u16_SessionId, e_Response, e_RejectReason );

         if ( e_Response == CMBS_RESPONSE_OK )
         {
            u8_IsLast = (g_st_APP_LA_Session[i_SessionIndex].u16_CMBS_LAEntryNr == g_st_APP_LA_Session[i_SessionIndex].u16_CMBS_LAEndIndex) ? 1 : 0;

            app_LaDataPacketSend ( u16_SessionId, u8_IsLast );
         }
      }
      else if( e_EventID == CMBS_EV_DSR_LA_SAVE_ENTRY )
      {
         u16             u16_EntryId = 1;
         u16             u16_EntryIndex = 2;
         u16             u16_NumOfEntries = 3;
         E_CMBS_RESPONSE e_Response = CMBS_RESPONSE_OK;
         E_CMBS_LA_NACK_ERRORS
                         e_RejectReason = CMBS_LA_NACK_MAX;

         printf("Save Entry %d\n", u16_EntryIndex);

         return cmbs_dsr_la_SaveEntryRes( g_cmbsappl.pv_CMBSRef, u16_SessionId,
                                          u16_EntryId, u16_EntryIndex, u16_NumOfEntries, e_Response, e_RejectReason );
      }
      else if( e_EventID == CMBS_EV_DSR_LA_DELETE_ENTRY )
      {
         E_CMBS_RESPONSE e_Response = CMBS_RESPONSE_OK;
         E_CMBS_LA_NACK_ERRORS
                         e_RejectReason = CMBS_LA_NACK_MAX;
         u16             u16_TotalNoOfEntries;

         printf("Delete Entry %d\n", u16_EntryIndex);

         i_SessionIndex = app_LaGetSessionIndex( u16_SessionId );
         if ( i_SessionIndex == APP_LA_MAX_NR_OF_SESSIONS )
         {
            printf("ERROR: Wrong SessionId=%x\n", u16_SessionId );
            e_Response     = CMBS_RESPONSE_ERROR;
            e_RejectReason = CMBS_LA_NACK_INVALID_SESSION;
         }
         else
         {
            printf("Delete Entry in list %d\n", g_st_APP_LA_Session[i_SessionIndex].u16_ListId);

            if ( u16_EntryIndex > app_LaNrOfListEntries ( u16_SessionId ) )
            {
               e_Response     = CMBS_RESPONSE_ERROR;
               e_RejectReason = CMBS_LA_NACK_ENTRY_NOT_AVAILABLE;
            }
         }

         if ( e_Response == CMBS_RESPONSE_OK )
         {
            // Here the application should delete the entry
            // Here we simulate a deleted entry
            u16_TotalNoOfEntries = app_LaNrOfListEntries ( u16_SessionId ) - 1;
         }
         else
         {
            u16_TotalNoOfEntries = app_LaNrOfListEntries ( u16_SessionId );
         }

         return cmbs_dsr_la_DeleteEntryRes( g_cmbsappl.pv_CMBSRef, u16_SessionId, u16_TotalNoOfEntries, e_Response, e_RejectReason );
      }
      else if( e_EventID == CMBS_EV_DSR_LA_DELETE_LIST )
      {
         E_CMBS_RESPONSE e_Response = CMBS_RESPONSE_OK;
         E_CMBS_LA_NACK_ERRORS
                         e_RejectReason = CMBS_LA_NACK_MAX;

         printf("Delete List\n");

         i_SessionIndex = app_LaGetSessionIndex( u16_SessionId );
         if ( i_SessionIndex == APP_LA_MAX_NR_OF_SESSIONS )
         {
            printf("ERROR: Wrong SessionId=%x\n", u16_SessionId );
            e_Response     = CMBS_RESPONSE_ERROR;
            e_RejectReason = CMBS_LA_NACK_INVALID_SESSION;
         }
         else
         {
            printf("Delete list %d\n", g_st_APP_LA_Session[i_SessionIndex].u16_ListId);
         }

         return cmbs_dsr_la_DeleteListRes( g_cmbsappl.pv_CMBSRef, u16_SessionId, e_Response, e_RejectReason );
      }
      else if( e_EventID == CMBS_EV_DSR_LA_DATA_PACKET_RECEIVE )
      {
         printf("What to do here ?\n");

         // return cmbs_dsr_la_DataPacketReceivedRes( g_cmbsappl.pv_CMBSRef, u16_SessionId, u8_Response );
      }
      else if( e_EventID == CMBS_EV_DSR_LA_DATA_PACKET_SEND_RES )
      {
         i_SessionIndex = app_LaGetSessionIndex( u16_SessionId );
         if ( i_SessionIndex == APP_LA_MAX_NR_OF_SESSIONS )
         {
            printf("ERROR: Wrong SessionId=%x\n", u16_SessionId );
            return FALSE;
         }

         g_st_APP_LA_Session[i_SessionIndex].u16_CMBS_LAEntryNr++;

         if ( g_st_APP_LA_Session[i_SessionIndex].u16_CMBS_LAEntryNr <= g_st_APP_LA_Session[i_SessionIndex].u16_CMBS_LAEndIndex )
         {
            u8              u8_IsLast;

            u8_IsLast = (g_st_APP_LA_Session[i_SessionIndex].u16_CMBS_LAEntryNr == g_st_APP_LA_Session[i_SessionIndex].u16_CMBS_LAEndIndex) ? 1 : 0;

            app_LaDataPacketSend ( u16_SessionId, u8_IsLast );
         }
      }

      return TRUE;
   }
   else
      return FALSE;

}


void app_LaDataPacketSend ( u16 u16_SessionId, u8 u8_IsLast )
{
   PST_CFR_IE_LIST p_List;
   int             i_SessionIndex;
   u16             u16_EntryNr;

   i_SessionIndex = app_LaGetSessionIndex( u16_SessionId );

   if ( i_SessionIndex == APP_LA_MAX_NR_OF_SESSIONS )
   {
      printf("ERROR: Wrong SessionId=%x\n", u16_SessionId );
      return;
   }
   else
   {
      printf("Fields from list %d\n", g_st_APP_LA_Session[i_SessionIndex].u16_ListId);
   }

   u16_EntryNr = g_st_APP_LA_Session[i_SessionIndex].u16_CMBS_LAEntryNr;

   p_List = (PST_CFR_IE_LIST)cmbs_api_ie_GetList();


}


u16 app_LaNrOfListEntries ( u16 u16_SessionId )
{
   u16 u16_Cnt = 0;
   u16 u16_Index;

   for(u16_Index = 1; u16_Index <= APP_LA_MAX_MISSED_CALL_LIST_ENTRIES; u16_Index++)
   {
      if ( g_st_APP_LA_MissedCallListEntry[u16_Index].u8_EntryFilled )
      {
         u16_Cnt++;
      }
   }

   return u16_Cnt;
}


int app_LaGetSessionIndex ( u16 u16_SessionId )
{
   u16 u16_Index;

   for(u16_Index = 0; u16_Index < APP_LA_MAX_NR_OF_SESSIONS; u16_Index++)
   {
      if(u16_Index == 0)
      {
         printf("g_st_APP_LA_Session[u16_Index].u8_EntryFilled=%d\n", g_st_APP_LA_Session[u16_Index].u8_EntryFilled);
         printf("g_st_APP_LA_Session[u16_Index].u16_SessionId=%x\n",  g_st_APP_LA_Session[u16_Index].u16_SessionId);
      }

      if ( g_st_APP_LA_Session[u16_Index].u8_EntryFilled && g_st_APP_LA_Session[u16_Index].u16_SessionId == u16_SessionId )
      {
         return u16_Index;
      }
   }

   return APP_LA_MAX_NR_OF_SESSIONS;

}


int app_LaGetNextFreeSessionIndex ( u16 u16_SessionId )
{
   u16 u16_Index;

   for(u16_Index = 0; u16_Index < APP_LA_MAX_NR_OF_SESSIONS; u16_Index++)
   {
      if ( !g_st_APP_LA_Session[u16_Index].u8_EntryFilled )
      {
         printf("app_LaGetNextFreeSessionIndex: New Index = %d\n", u16_Index);
         memset(&g_st_APP_LA_Session[u16_Index], 0, sizeof(g_st_APP_LA_Session[u16_Index]));
         g_st_APP_LA_Session[u16_Index].u8_EntryFilled = 1;
         g_st_APP_LA_Session[u16_Index].u16_SessionId  = u16_SessionId;
         return u16_Index;
      }
   }

   return APP_LA_MAX_NR_OF_SESSIONS;

}


void app_LaInitExampleLists( void )
{
   strcpy((char *)g_st_APP_LA_MissedCallListEntry[1].pu8_Number, "497312456897");
   strcpy((char *)g_st_APP_LA_MissedCallListEntry[1].pu8_Name,   "JENDREZEJZAK");
   g_st_APP_LA_MissedCallListEntry[1].st_DateTime.u8_Year = 9;
   g_st_APP_LA_MissedCallListEntry[1].st_DateTime.u8_Month = 9;
   g_st_APP_LA_MissedCallListEntry[1].st_DateTime.u8_Day = 9;
   g_st_APP_LA_MissedCallListEntry[1].st_DateTime.u8_Hours = 6;
   g_st_APP_LA_MissedCallListEntry[1].st_DateTime.u8_Mins = 45;
   g_st_APP_LA_MissedCallListEntry[1].st_DateTime.u8_Secs = 0;
   g_st_APP_LA_MissedCallListEntry[1].u8_New = 1;
   strcpy((char *)g_st_APP_LA_MissedCallListEntry[1].pu8_LineName, "Provider 1");
   g_st_APP_LA_MissedCallListEntry[1].u8_LineId = 0;
   g_st_APP_LA_MissedCallListEntry[1].u8_NumberOfCalls = 2;
   g_st_APP_LA_MissedCallListEntry[1].u8_EntryFilled = 1;

   strcpy((char *)g_st_APP_LA_MissedCallListEntry[2].pu8_Number, "0145567897");
   strcpy((char *)g_st_APP_LA_MissedCallListEntry[2].pu8_Name,   "");
   g_st_APP_LA_MissedCallListEntry[2].st_DateTime.u8_Year = 9;
   g_st_APP_LA_MissedCallListEntry[2].st_DateTime.u8_Month = 9;
   g_st_APP_LA_MissedCallListEntry[2].st_DateTime.u8_Day = 6;
   g_st_APP_LA_MissedCallListEntry[2].st_DateTime.u8_Hours = 18;
   g_st_APP_LA_MissedCallListEntry[2].st_DateTime.u8_Mins = 48;
   g_st_APP_LA_MissedCallListEntry[2].st_DateTime.u8_Secs = 0;
   g_st_APP_LA_MissedCallListEntry[2].u8_New = 1;
   strcpy((char *)g_st_APP_LA_MissedCallListEntry[2].pu8_LineName, "Provider 1");
   g_st_APP_LA_MissedCallListEntry[2].u8_LineId = 0;
   g_st_APP_LA_MissedCallListEntry[2].u8_NumberOfCalls = 3;
   g_st_APP_LA_MissedCallListEntry[2].u8_EntryFilled = 1;

   strcpy((char *)g_st_APP_LA_MissedCallListEntry[3].pu8_Number, "00441324778824");
   strcpy((char *)g_st_APP_LA_MissedCallListEntry[3].pu8_Name,   "C.Alexander");
   g_st_APP_LA_MissedCallListEntry[3].st_DateTime.u8_Year = 9;
   g_st_APP_LA_MissedCallListEntry[3].st_DateTime.u8_Month = 9;
   g_st_APP_LA_MissedCallListEntry[3].st_DateTime.u8_Day = 6;
   g_st_APP_LA_MissedCallListEntry[3].st_DateTime.u8_Hours = 15;
   g_st_APP_LA_MissedCallListEntry[3].st_DateTime.u8_Mins = 36;
   g_st_APP_LA_MissedCallListEntry[3].st_DateTime.u8_Secs = 36;
   g_st_APP_LA_MissedCallListEntry[3].u8_New = 0;
   strcpy((char *)g_st_APP_LA_MissedCallListEntry[3].pu8_LineName, "Provider 1");
   g_st_APP_LA_MissedCallListEntry[3].u8_LineId = 0;
   g_st_APP_LA_MissedCallListEntry[3].u8_NumberOfCalls = 1;
   g_st_APP_LA_MissedCallListEntry[3].u8_EntryFilled = 1;

   strcpy((char *)g_st_APP_LA_MissedCallListEntry[4].pu8_Number, "1234567890");
   strcpy((char *)g_st_APP_LA_MissedCallListEntry[4].pu8_Name,   "RIGHT");
   g_st_APP_LA_MissedCallListEntry[4].st_DateTime.u8_Year = 9;
   g_st_APP_LA_MissedCallListEntry[4].st_DateTime.u8_Month = 9;
   g_st_APP_LA_MissedCallListEntry[4].st_DateTime.u8_Day = 6;
   g_st_APP_LA_MissedCallListEntry[4].st_DateTime.u8_Hours = 15;
   g_st_APP_LA_MissedCallListEntry[4].st_DateTime.u8_Mins = 36;
   g_st_APP_LA_MissedCallListEntry[4].st_DateTime.u8_Secs = 36;
   g_st_APP_LA_MissedCallListEntry[4].u8_New = 0;
   strcpy((char *)g_st_APP_LA_MissedCallListEntry[4].pu8_LineName, "Provider 1");
   g_st_APP_LA_MissedCallListEntry[4].u8_LineId = 0;
   g_st_APP_LA_MissedCallListEntry[4].u8_NumberOfCalls = 1;
   g_st_APP_LA_MissedCallListEntry[4].u8_EntryFilled = 1;

   strcpy((char *)g_st_APP_LA_MissedCallListEntry[5].pu8_Number, "1234567890");
   strcpy((char *)g_st_APP_LA_MissedCallListEntry[5].pu8_Name,   "xWxRxOxNxG");
   g_st_APP_LA_MissedCallListEntry[5].st_DateTime.u8_Year = 9;
   g_st_APP_LA_MissedCallListEntry[5].st_DateTime.u8_Month = 9;
   g_st_APP_LA_MissedCallListEntry[5].st_DateTime.u8_Day = 6;
   g_st_APP_LA_MissedCallListEntry[5].st_DateTime.u8_Hours = 15;
   g_st_APP_LA_MissedCallListEntry[5].st_DateTime.u8_Mins = 36;
   g_st_APP_LA_MissedCallListEntry[5].st_DateTime.u8_Secs = 36;
   g_st_APP_LA_MissedCallListEntry[5].u8_New = 0;
   strcpy((char *)g_st_APP_LA_MissedCallListEntry[5].pu8_LineName, "Provider 1");
   g_st_APP_LA_MissedCallListEntry[5].u8_LineId = 0;
   g_st_APP_LA_MissedCallListEntry[5].u8_NumberOfCalls = 1;
   g_st_APP_LA_MissedCallListEntry[5].u8_EntryFilled = 1;
}


int app_LaIsValidList ( u16 u16_ListId )
{
   // At the moment only Missed Call List is valid, other not implemented
   if ( u16_ListId == CMBS_LA_LIST_MISSED_CALLS )
   {
      return 1;
   }

   return 0;
}


//*/
