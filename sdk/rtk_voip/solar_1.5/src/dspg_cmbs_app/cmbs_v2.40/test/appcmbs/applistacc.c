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

#include "appla2.h"

void app_LaDataPacketSend ( void * pv_AppRef, u16 SessionId, u8 u8_IsLast );
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

#define APP_LA_MAX_MISSED_CALL_LIST_ENTRIES  20
typedef struct
{
   u8           u8_EntryFilled;
   u8           u8_NumberAttribute;  // 3 less significant bits are used only (own bit is less significant, then internal, then editable bit
   u8           pu8_Number[128];
   u8           pu8_Name[128];
   ST_DATE_TIME st_DateTime;
   u8           u8_New; // only less significant should be used.
   u8           pu8_LineName[128];
   u8           u8_LineId;
   u8           u8_NumberOfCalls;
} st_APP_LA_MissedCallListEntry, *pst_APP_LA_MissedCallListEntry;

st_APP_LA_MissedCallListEntry g_st_APP_LA_MissedCallListEntry[APP_LA_MAX_MISSED_CALL_LIST_ENTRIES + 1]; // Index from 1 to APP_LA_MAX_MISSED_CALL_LIST_ENTRIES


#define APP_LA_MAX_OUTGOING_CALL_LIST_ENTRIES  20
typedef struct
{
   u8           u8_EntryFilled;
   u8           u8_NumberAttribute;
   u8           pu8_Number[128];
   u8           pu8_Name[128];
   ST_DATE_TIME st_DateTime;
   u8           pu8_LineName[128];
   u8           u8_LineId;
} st_APP_LA_OutgoingCallListEntry, *pst_APP_LA_OutgoingCallListEntry;

st_APP_LA_OutgoingCallListEntry g_st_APP_LA_OutgoingCallListEntry[APP_LA_MAX_OUTGOING_CALL_LIST_ENTRIES + 1]; // Index from 1 to APP_LA_MAX_OUTGOING_CALL_LIST_ENTRIES


#define APP_LA_MAX_INCOMING_ACCEPTED_CALL_LIST_ENTRIES  20
typedef struct
{
   u8           u8_EntryFilled;
   u8           u8_NumberAttribute;
   u8           pu8_Number[128];
   u8           pu8_Name[128];
   ST_DATE_TIME st_DateTime;
   u8           pu8_LineName[128];
   u8           u8_LineId;
} st_APP_LA_IncomingAcceptedCallListEntry, *pst_APP_LA_IncomingAcceptedCallListEntry;

st_APP_LA_IncomingAcceptedCallListEntry g_st_APP_LA_IncomingAcceptedCallListEntry[APP_LA_MAX_INCOMING_ACCEPTED_CALL_LIST_ENTRIES + 1]; // Index from 1 to APP_LA_MAX_INCOMING_ACCEPTED_CALL_LIST_ENTRIES


#define APP_LA_MAX_CONTACT_LIST_ENTRIES  20
typedef struct
{
   u8           u8_EntryFilled;
   u8           u8_NumberAttribute;   
   u8           pu8_Name[128];
   u8           pu8_FirstName[128];
   u8           pu8_Number[128];
   u8           u8_AssociatedMelody;
   u8           u8_LineId;
} st_APP_LA_ContactListEntry, *pst_APP_LA_ContactListEntry;

st_APP_LA_ContactListEntry g_st_APP_LA_ContactListEntry[APP_LA_MAX_CONTACT_LIST_ENTRIES + 1]; // Index from 1 to APP_LA_MAX_CONTACT_LIST_ENTRIES


#define APP_LA_MAX_ALL_CALL_LIST_ENTRIES  20
typedef struct
{
   u8           u8_EntryFilled;
   u8           u8_NumberAttribute;
   u8           u8_CallType;  // 3 less significant bits are used only (outgoing call bit is less significant, then accepted, then missed call bit
   u8           pu8_Number[128];
   u8           pu8_Name[128];
   ST_DATE_TIME st_DateTime;
   u8           pu8_LineName[128];
   u8           u8_LineId;
} st_APP_LA_AllCallListEntry, *pst_APP_LA_AllCallListEntry;

st_APP_LA_AllCallListEntry g_st_APP_LA_AllCallListEntry[APP_LA_MAX_ALL_CALL_LIST_ENTRIES + 1]; // Index from 1 to APP_LA_MAX_ALL_CALL_LIST_ENTRIES



#define APP_LA_MAX_LINE_SETTING_LIST_ENTRIES  32
typedef struct
{
   u8           u8_EntryFilled;
   u8           pu8_LineName[128];
   u8           u8_LineId;
   u8           u8_NumberHandsets;
   u8           pu8_HandsetBitfield[32]; // 1rst byte is the bitfield for HS1 to HS8, LSB is HS1, following byte is for HS9 to HS16, etc etc until HS255
   u8           pu8_DialingPrefixe[128];
   u8           pu8_BlockedNumber[128];
   u8           u8_FPMelody;
   u8           u8_FPVolume;
   u8           u8_MultiCallMode;
   u8           u8_IntrusionCall;
   u8           u8_CLIRValue;
   u8           u8_CLIRActDataLen; 
   u8           pu8_CLIRActData[8];
   u8           u8_CLIRDesactDataLen; 
   u8           pu8_CLIRDesactData[8];
   u8           u8_CFUValue;
   u8           u8_CFUNotSetByUser;
   u8           u8_CFUActDataLen; 
   u8           pu8_CFUActData;
   u8           u8_CFUDesactDataLen; 
   u8           pu8_CFUDesactData;
   u8           pu8_CFUNumber[128];
   u8           u8_CFNANotSetByUser;
   u8           u8_CFNAValue;
   u8           u8_CFNANbSecond;
   u8           u8_CFNAActDataLen; 
   u8           pu8_CFNAActData;
   u8           u8_CFNADesactDataLen; 
   u8           pu8_CFNADesactData;
   u8           pu8_CFNANumber[128];
   u8           u8_CFBNotSetByUser;
   u8           u8_CFBValue;
   u8           u8_CFBActDataLen; 
   u8           pu8_CFBActData;
   u8           u8_CFBDesactDataLen; 
   u8           pu8_CFBDesactData;
   u8           pu8_CFBNumber[128];
} st_APP_LA_LineSettingListEntry, *pst_APP_LA_LineSettingListEntry;

st_APP_LA_LineSettingListEntry g_st_APP_LA_LineSettingListEntry[APP_LA_MAX_LINE_SETTING_LIST_ENTRIES + 1]; // Index from 1 to APP_LA_MAX_LINE_SETTING_LIST_ENTRIES



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
   ST_IE_DATA st_DataGet ;
   u8             u8_IsLast;

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
          else if (u16_IE == CMBS_IE_LA_IS_LAST)
	  	{
                     cmbs_api_ie_LAIsLastGet(pv_IE, &u8_IsLast );
                     printf( "IsLAst:%d\n", u8_IsLast );
	  	}
			
      else if (u16_IE == CMBS_IE_DATA)
	  	{
                     printf("IE_DATA:\n");
                     cmbs_api_ie_DataGet(pv_IE, &st_DataGet);
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

            app_LaDataPacketSend (pv_AppRef,u16_SessionId, u8_IsLast );
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

            app_LaDataPacketSend ( pv_AppRef,u16_SessionId, u8_IsLast );
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

            app_LaDataPacketSend ( pv_AppRef,u16_SessionId, u8_IsLast );
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
      u16 u16_Index = 0;
	u8 u8_EntryFieldNumber = 0;
	u16 u16_EntryIdentifier;
	u16 u16_EntryLength;
	u8 *pu8_FieldId;
	E_CMBS_RESPONSE e_Response = CMBS_RESPONSE_OK;
		

cmbs_util_LA_EntryLenght_EntryIdentifierGet(st_DataGet.pu8_Data, &u16_EntryIdentifier, &u16_EntryLength, &u16_Index); // index will be updated here to point on fieldId 1
	
     i_SessionIndex = app_LaGetSessionIndex( u16_SessionId );
      
      switch(g_st_APP_LA_Session[i_SessionIndex].u16_ListId)
	  	{
		case CMBS_LA_LIST_MISSED_CALLS:
			while (u16_Index < u16_EntryLength) // Check if it was the last field
				{
				pu8_FieldId = st_DataGet.pu8_Data + u16_Index; // get the pointer on fieldid
				switch(*pu8_FieldId) // check the fieldid content
					{
					case CMBS_MCL_FLD_NUMBER:
						{
							ST_UTIL_LA_FLD_NUMBER st_LA_Fld_Number;
							cmbs_util_LA_Fld_NumberGet(pu8_FieldId, &st_LA_Fld_Number);
						}
					break;
					case CMBS_MCL_FLD_NAME:
						{
							ST_UTIL_LA_FLD_NAME st_LA_Fld_Name;
							cmbs_util_LA_Fld_NameGet( pu8_FieldId, &st_LA_Fld_Name);
						}
					break;
					case CMBS_MCL_FLD_DATETIME:
						{
							ST_UTIL_LA_FLD_DATETIME st_LA_Fld_DateTime;
							cmbs_util_LA_Fld_DateTimeGet( pu8_FieldId, &st_LA_Fld_DateTime);
						}
					break;
					case CMBS_MCL_FLD_NEW:
						{
							ST_UTIL_LA_FLD_NEW st_LA_Fld_New;
							cmbs_util_LA_Fld_NewGet( pu8_FieldId, &st_LA_Fld_New);
						}
					break;
					case CMBS_MCL_FLD_LINE_NAME:
						{
							ST_UTIL_LA_FLD_LINE_NAME st_LA_Fld_LineName;
							cmbs_util_LA_Fld_LineNameGet( pu8_FieldId, &st_LA_Fld_LineName);
						}
					break;
					case CMBS_MCL_FLD_LINE_ID:
						{
							ST_UTIL_LA_FLD_LINE_ID st_LA_Fld_LineId;
							cmbs_util_LA_Fld_LineIdGet( pu8_FieldId, &st_LA_Fld_LineId);
						}
					break;
					case CMBS_MCL_FLD_NR_OF_CALLS:
						{
							ST_UTIL_LA_FLD_NR_OF_CALLS st_LA_Fld_NumberCalls;
							cmbs_util_LA_Fld_NrOfCallsGet(pu8_FieldId, &st_LA_Fld_NumberCalls);
						}
					break;
					default:
					break;
					}
				cmbs_util_la_ShiftIndexToNextFieldID(pu8_FieldId, &u16_Index); // calculate index for next fieldid = current index + Lenght of current  field + 2: fieldid/field length
				}
			break;

				case CMBS_LA_LIST_OUTGOING_CALLS:
					
			       while (u16_Index < u16_EntryLength) // Check if it was the last field
				{
				pu8_FieldId = st_DataGet.pu8_Data + u16_Index; // get the pointer on fieldid
				switch(*pu8_FieldId) // check the fieldid content
					{
					case CMBS_OCL_FLD_NUMBER:
						{
							ST_UTIL_LA_FLD_NUMBER st_LA_Fld_Number;
							cmbs_util_LA_Fld_NumberGet(pu8_FieldId, &st_LA_Fld_Number);
						}
					break;
					case CMBS_OCL_FLD_NAME:
						{
							ST_UTIL_LA_FLD_NAME st_LA_Fld_Name;
							cmbs_util_LA_Fld_NameGet( pu8_FieldId, &st_LA_Fld_Name);
						}
					break;
					case CMBS_OCL_FLD_DATETIME:
						{
							ST_UTIL_LA_FLD_DATETIME st_LA_Fld_DateTime;
							cmbs_util_LA_Fld_DateTimeGet( pu8_FieldId, &st_LA_Fld_DateTime);
						}
					break;
					case CMBS_OCL_FLD_LINE_NAME:
						{
							ST_UTIL_LA_FLD_LINE_NAME st_LA_Fld_LineName;
							cmbs_util_LA_Fld_LineNameGet( pu8_FieldId, &st_LA_Fld_LineName);
						}
					break;
					case CMBS_OCL_FLD_LINE_ID:
						{
							ST_UTIL_LA_FLD_LINE_ID st_LA_Fld_LineId;
							cmbs_util_LA_Fld_LineIdGet( pu8_FieldId, &st_LA_Fld_LineId);
						}
					break;
					default:
					break;
					}
				cmbs_util_la_ShiftIndexToNextFieldID(pu8_FieldId, &u16_Index); // calculate index for next fieldid = current index + Lenght of current  field + 2: fieldid/field length
				}
					break;
					
				case CMBS_LA_LIST_INCOMING_ACC:
			       while (u16_Index < u16_EntryLength) // Check if it was the last field
				{
				pu8_FieldId = st_DataGet.pu8_Data + u16_Index; // get the pointer on fieldid
				switch(*pu8_FieldId) // check the fieldid content
					{
					case CMBS_IACL_FLD_NUMBER:
						{
							ST_UTIL_LA_FLD_NUMBER st_LA_Fld_Number;
							cmbs_util_LA_Fld_NumberGet(pu8_FieldId, &st_LA_Fld_Number);
						}
					break;
					case CMBS_IACL_FLD_NAME:
						{
							ST_UTIL_LA_FLD_NAME st_LA_Fld_Name;
							cmbs_util_LA_Fld_NameGet( pu8_FieldId, &st_LA_Fld_Name);
						}
					break;
					case CMBS_IACL_FLD_DATETIME:
						{
							ST_UTIL_LA_FLD_DATETIME st_LA_Fld_DateTime;
							cmbs_util_LA_Fld_DateTimeGet( pu8_FieldId, &st_LA_Fld_DateTime);
						}
					break;
					case CMBS_IACL_FLD_LINE_NAME:
						{
							ST_UTIL_LA_FLD_LINE_NAME st_LA_Fld_LineName;
							cmbs_util_LA_Fld_LineNameGet( pu8_FieldId, &st_LA_Fld_LineName);
						}
					break;
					case CMBS_IACL_FLD_LINE_ID:
						{
							ST_UTIL_LA_FLD_LINE_ID st_LA_Fld_LineId;
							cmbs_util_LA_Fld_LineIdGet( pu8_FieldId, &st_LA_Fld_LineId);
						}
					break;
					default:
					break;
					}
				cmbs_util_la_ShiftIndexToNextFieldID(pu8_FieldId, &u16_Index); // calculate index for next fieldid = current index + Lenght of current  field + 2: fieldid/field length
				}
					break;

					
				case CMBS_LA_LIST_CONTACT:
			       while (u16_Index < u16_EntryLength) // Check if it was the last field
				{
				pu8_FieldId = st_DataGet.pu8_Data + u16_Index; // get the pointer on fieldid
				switch(*pu8_FieldId) // check the fieldid content
					{
					case CMBS_CL_FLD_NAME:
						{
							ST_UTIL_LA_FLD_NAME st_LA_Fld_Name;
							cmbs_util_LA_Fld_NameGet( pu8_FieldId, &st_LA_Fld_Name);
						}
					break;
					case CMBS_CL_FLD_FIRST_NAME:
						{
							ST_UTIL_LA_FLD_FIRST_NAME st_LA_Fld_FirstName;
							cmbs_util_LA_Fld_FirstNameGet( pu8_FieldId, &st_LA_Fld_FirstName);
						}
					break;
					case CMBS_CL_FLD_CONTACT_NR:
						{
							ST_UTIL_LA_FLD_CONTACT_NR st_LA_Fld_ContactNumber;
							cmbs_util_LA_Fld_ContactNrGet( pu8_FieldId, &st_LA_Fld_ContactNumber);
						}
					break;
					case CMBS_CL_FLD_ASSOC_MDY:
						{
                                                  ST_UTIL_LA_FLD_ASSOC_MDY  st_LA_Fld_Associated_melody;
							cmbs_util_LA_Fld_AssocMdyGet( pu8_FieldId, &st_LA_Fld_Associated_melody);
						}
					break;
					case CMBS_CL_FLD_LINE_ID:
						{
							ST_UTIL_LA_FLD_LINE_ID st_LA_Fld_LineId;
							cmbs_util_LA_Fld_LineIdGet( pu8_FieldId, &st_LA_Fld_LineId);
						}
					break;
					
					default:
					break;
					}
				cmbs_util_la_ShiftIndexToNextFieldID(pu8_FieldId, &u16_Index);// calculate index for next fieldid = current index + Lenght of current  field + 2: fieldid/field length
				}
					break;

					
				case CMBS_LA_LIST_ALL_CALLS:
			       while (u16_Index < u16_EntryLength) // Check if it was the last field
				{
				pu8_FieldId = st_DataGet.pu8_Data + u16_Index; // get the pointer on fieldid
				switch(*pu8_FieldId) // check the fieldid content
					{
					case CMBS_ACL_FLD_CALL_TYPE:
						{
							ST_UTIL_LA_FLD_CALL_TYPE st_LA_Fld_CallType;
							cmbs_util_LA_Fld_CallTypeGet( pu8_FieldId, &st_LA_Fld_CallType);
						}
					break;
					case CMBS_ACL_FLD_NUMBER:
						{
							ST_UTIL_LA_FLD_NUMBER st_LA_Fld_Number;
							cmbs_util_LA_Fld_NumberGet(pu8_FieldId, &st_LA_Fld_Number);
						}
					break;
					case CMBS_ACL_FLD_NAME:
						{
							ST_UTIL_LA_FLD_NAME st_LA_Fld_Name;
							cmbs_util_LA_Fld_NameGet( pu8_FieldId, &st_LA_Fld_Name);
						}
					break;
					case CMBS_ACL_FLD_DATETIME:
						{
							ST_UTIL_LA_FLD_DATETIME st_LA_Fld_DateTime;
							cmbs_util_LA_Fld_DateTimeGet( pu8_FieldId, &st_LA_Fld_DateTime);
						}
					break;
					case CMBS_ACL_FLD_LINE_NAME:
						{
							ST_UTIL_LA_FLD_LINE_NAME st_LA_Fld_LineName;
							cmbs_util_LA_Fld_LineNameGet( pu8_FieldId, &st_LA_Fld_LineName);
						}
					break;
					case CMBS_ACL_FLD_LINE_ID:
						{
							ST_UTIL_LA_FLD_LINE_ID st_LA_Fld_LineId;
							cmbs_util_LA_Fld_LineIdGet( pu8_FieldId, &st_LA_Fld_LineId);
						}
					break;
					default:
					break;
					}
				cmbs_util_la_ShiftIndexToNextFieldID(pu8_FieldId, &u16_Index);// calculate index for next fieldid = current index + Lenght of current  field + 2: fieldid/field length
				}
					break;


	
				case CMBS_LA_LIST_LINE_SETTINGS:
			       while (u16_Index < u16_EntryLength) // Check if it was the last field
				{
				pu8_FieldId = st_DataGet.pu8_Data + u16_Index; // get the pointer on fieldid
				switch(*pu8_FieldId) // check the fieldid content
					{
					case CMBS_LSL_FLD_LINE_NAME:
						{
							ST_UTIL_LA_FLD_LINE_NAME st_LA_Fld_LineName;
							cmbs_util_LA_Fld_LineNameGet( pu8_FieldId, &st_LA_Fld_LineName);
						}
					break;
					case CMBS_LSL_FLD_LINE_ID:
						{
							ST_UTIL_LA_FLD_LINE_ID st_LA_Fld_LineId;
							cmbs_util_LA_Fld_LineIdGet( pu8_FieldId, &st_LA_Fld_LineId);
						}
					
					case CMBS_LSL_FLD_ATTACHED_HANDSETS:
						{
							ST_UTIL_LA_FLD_ATTACHED_HANDSETS st_LA_Fld_AttachedHandset;
							cmbs_util_LA_Fld_AttachedHandsetsGet(pu8_FieldId, &st_LA_Fld_AttachedHandset);
						}
					break;
					case CMBS_LSL_FLD_DIALING_PREFIX:
						{
							ST_UTIL_LA_FLD_DIALING_PREFIX st_LA_Fld_DialingPrefixe;
							cmbs_util_LA_Fld_DialPrefixeGet( pu8_FieldId, &st_LA_Fld_DialingPrefixe);
						}
					break;
					case CMBS_LSL_FLD_FP_MELODY:
						{
							ST_UTIL_LA_FLD_FP_MELODY st_LA_Fld_FPmelody;
							cmbs_util_LA_Fld_AssocMdyGet( pu8_FieldId, &st_LA_Fld_FPmelody);
						}
					break;
					case CMBS_LSL_FLD_FP_VOLUME:
						{
							ST_UTIL_LA_FLD_FP_VOLUME st_LA_Fld_FPvolume;
							cmbs_util_LA_Fld_FPvolumeGet( pu8_FieldId, &st_LA_Fld_FPvolume);
						}
					break;
					case CMBS_LSL_FLD_BLOCKED_NUMBER:
						{
							ST_UTIL_LA_FLD_BLOCKED_NUMBER st_LA_Fld_BlockedNumber;
							cmbs_util_LA_Fld_BlockedNumberGet( pu8_FieldId, &st_LA_Fld_BlockedNumber);
						}
					
					case CMBS_LSL_FLD_MULTI_CALL_MODE:
						{
							ST_UTIL_LA_FLD_MULTI_CALL_MODE st_LA_Fld_MultiCallMode;
							cmbs_util_LA_Fld_MultiCallModeGet(pu8_FieldId, &st_LA_Fld_MultiCallMode);
						}
					break;
					case CMBS_LSL_FLD_INTRUSION_CALL:
						{
							ST_UTIL_LA_FLD_INTRUSION_CALL st_LA_Fld_IntrusionCall;
							cmbs_util_LA_Fld_IntrusionCallGet( pu8_FieldId, &st_LA_Fld_IntrusionCall);
						}
					break;
					case CMBS_LSL_FLD_PERMANENT_CLIR:
						{
							ST_UTIL_LA_FLD_PERMANENT_CLIR st_LA_Fld_CLIR;
							cmbs_util_LA_Fld_PermanentCLIRGet( pu8_FieldId, &st_LA_Fld_CLIR);
						}
					break;
					case CMBS_LSL_FLD_CALL_FORWARDING:
						{
							ST_UTIL_LA_FLD_CALL_FORWARDING st_LA_Fld_FW;
							cmbs_util_LA_Fld_CallForwardingGet( pu8_FieldId, &st_LA_Fld_FW);
						}
					break;
					case CMBS_LSL_FLD_CFNA:
						{
							ST_UTIL_LA_FLD_CFNA st_LA_Fld_CFNA;
							cmbs_util_LA_Fld_CFNAGet( pu8_FieldId, &st_LA_Fld_CFNA);
						}
					break;
					case CMBS_LSL_FLD_CFB:
						{
							ST_UTIL_LA_FLD_CFB st_LA_Fld_CFB;
							cmbs_util_LA_Fld_CFBGet( pu8_FieldId, &st_LA_Fld_CFB);
						}
					break;
					default:
					break;
					}
				cmbs_util_la_ShiftIndexToNextFieldID(pu8_FieldId, &u16_Index);
				//	u16_Index += (*(pu8_FieldId+1)  + 2); // calculate index for next fieldid = current index + Lenght of current  field + 2: fieldid/field length
				}
					break;

					

		default:
		break;
					
      	}

	  
        return cmbs_dsr_la_DataPacketReceivedRes( pv_AppRef, u16_SessionId, e_Response );


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

            app_LaDataPacketSend ( pv_AppRef,u16_SessionId, u8_IsLast );
         }
      }

      return TRUE;
   }
   else
      return FALSE;

}


void app_LaDataPacketSend ( void * pv_AppRef, u16 u16_SessionId, u8 u8_IsLast )
{

   u16             i_SessionIndex;
   u16             u16_EntryID;

   u16 u16_index = 0;
u8 p_entry[256];
u8 *pu8_finalbuff;

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

   u16_EntryID = (u8) g_st_APP_LA_Session[i_SessionIndex].u16_CMBS_LAEntryNr;


// Start to fill Data content structure in the "Data packet" command as per CATIQ 2.0 protocol for 1 entry

// reserve max 4 bytes for EntryID and Length, they will be handled at the end of the function
u16_index+=4;
   
switch(g_st_APP_LA_Session[i_SessionIndex].u16_ListId)
{
case CMBS_LA_LIST_MISSED_CALLS:
   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_MCL_FLD_NUMBER) )
   {
    ST_UTIL_LA_FLD_NUMBER st_LA_Fld_Number;
      st_LA_Fld_Number.u8_Attribute = (g_st_APP_LA_MissedCallListEntry[u16_EntryID].u8_NumberAttribute & 7) << 4;
      st_LA_Fld_Number.u8_FieldId = CMBS_MCL_FLD_NUMBER;
      st_LA_Fld_Number.u16_DataLen   = strlen(g_st_APP_LA_MissedCallListEntry[u16_EntryID].pu8_Number);
      st_LA_Fld_Number.pu8_Data     = (void *)g_st_APP_LA_MissedCallListEntry[u16_EntryID].pu8_Number;
      cmbs_util_LA_Fld_NumberAdd( p_entry, &st_LA_Fld_Number,  &u16_index); // index will be updated inside the function
   }

   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_MCL_FLD_NAME) )
   {
    ST_UTIL_LA_FLD_NAME st_LA_Fld_Name;
      st_LA_Fld_Name.u8_Attribute   = 0x00;
      st_LA_Fld_Name.u8_FieldId = CMBS_MCL_FLD_NAME;
      st_LA_Fld_Name.u16_DataLen     = strlen(g_st_APP_LA_MissedCallListEntry[u16_EntryID].pu8_Name);
      st_LA_Fld_Name.pu8_Data       = (void *)g_st_APP_LA_MissedCallListEntry[u16_EntryID].pu8_Name;
      cmbs_util_LA_Fld_NameAdd( p_entry, &st_LA_Fld_Name,  &u16_index );
   }

   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_MCL_FLD_DATETIME) )
   {
   ST_UTIL_LA_FLD_DATETIME st_LA_Fld_DateTime;
      st_LA_Fld_DateTime.u8_Attribute = 0x00;
      st_LA_Fld_DateTime.u8_FieldId = CMBS_MCL_FLD_DATETIME;
      st_LA_Fld_DateTime.u8_Year      = g_st_APP_LA_MissedCallListEntry[u16_EntryID].st_DateTime.u8_Year;
      st_LA_Fld_DateTime.u8_Month     = g_st_APP_LA_MissedCallListEntry[u16_EntryID].st_DateTime.u8_Month;
      st_LA_Fld_DateTime.u8_Day       = g_st_APP_LA_MissedCallListEntry[u16_EntryID].st_DateTime.u8_Day;
      st_LA_Fld_DateTime.u8_Hours     = g_st_APP_LA_MissedCallListEntry[u16_EntryID].st_DateTime.u8_Hours;
      st_LA_Fld_DateTime.u8_Mins      = g_st_APP_LA_MissedCallListEntry[u16_EntryID].st_DateTime.u8_Mins;
      st_LA_Fld_DateTime.u8_Secs      = g_st_APP_LA_MissedCallListEntry[u16_EntryID].st_DateTime.u8_Secs;
      cmbs_util_LA_Fld_DateTimeAdd( p_entry, &st_LA_Fld_DateTime,  &u16_index );
   }

   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_MCL_FLD_NEW) )
   {
   
   ST_UTIL_LA_FLD_NEW st_LA_Fld_New;
      st_LA_Fld_New.u8_Attribute = (g_st_APP_LA_MissedCallListEntry[u16_EntryID].u8_New & 1) << 5;
      st_LA_Fld_New.u8_FieldId = CMBS_MCL_FLD_NEW;
      cmbs_util_LA_Fld_NewAdd( p_entry, &st_LA_Fld_New,  &u16_index );
   }

   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_MCL_FLD_LINE_NAME) )
   {
   ST_UTIL_LA_FLD_LINE_NAME  st_LA_Fld_LineName;
      st_LA_Fld_LineName.u8_Attribute = 0x00;
      st_LA_Fld_LineName.u8_FieldId = CMBS_MCL_FLD_LINE_NAME;
      st_LA_Fld_LineName.u16_DataLen   = strlen(g_st_APP_LA_MissedCallListEntry[u16_EntryID].pu8_LineName);
      st_LA_Fld_LineName.pu8_Data     = (void *)g_st_APP_LA_MissedCallListEntry[u16_EntryID].pu8_LineName;
      cmbs_util_LA_Fld_LineNameAdd( p_entry, &st_LA_Fld_LineName,  &u16_index );
   }

   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_MCL_FLD_LINE_ID) )
   {
   ST_UTIL_LA_FLD_LINE_ID  st_LA_Fld_LineId;
      st_LA_Fld_LineId.u8_Attribute = 0x00;
      st_LA_Fld_LineId.u8_FieldId = CMBS_MCL_FLD_LINE_ID;
      st_LA_Fld_LineId.u8_Value     = g_st_APP_LA_MissedCallListEntry[u16_EntryID].u8_LineId;
      cmbs_util_LA_Fld_LineIdAdd( p_entry, &st_LA_Fld_LineId,  &u16_index );
   }

   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_MCL_FLD_NR_OF_CALLS) )
   {
   ST_UTIL_LA_FLD_NR_OF_CALLS  st_LA_Fld_NrOfCalls;
      st_LA_Fld_NrOfCalls.u8_Attribute = 0x00;
      st_LA_Fld_NrOfCalls.u8_FieldId = CMBS_MCL_FLD_NR_OF_CALLS;
      st_LA_Fld_NrOfCalls.u8_Value     = g_st_APP_LA_MissedCallListEntry[u16_EntryID].u8_NumberOfCalls;
      cmbs_util_LA_Fld_NrOfCallsAdd( p_entry, &st_LA_Fld_NrOfCalls,  &u16_index );
   }
   break;


case CMBS_LA_LIST_OUTGOING_CALLS:
   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_OCL_FLD_NUMBER) )
   {
    ST_UTIL_LA_FLD_NUMBER st_LA_Fld_Number;
      st_LA_Fld_Number.u8_Attribute = (g_st_APP_LA_OutgoingCallListEntry[u16_EntryID].u8_NumberAttribute & 7) << 4;
      st_LA_Fld_Number.u8_FieldId = CMBS_OCL_FLD_NUMBER;
      st_LA_Fld_Number.u16_DataLen   = strlen(g_st_APP_LA_OutgoingCallListEntry[u16_EntryID].pu8_Number);
      st_LA_Fld_Number.pu8_Data     = (void *)g_st_APP_LA_OutgoingCallListEntry[u16_EntryID].pu8_Number;
      cmbs_util_LA_Fld_NumberAdd( p_entry, &st_LA_Fld_Number,  &u16_index); // index will be updated inside the function
   }

   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_OCL_FLD_NAME) )
   {
    ST_UTIL_LA_FLD_NAME st_LA_Fld_Name;
      st_LA_Fld_Name.u8_Attribute   = 0x00;
      st_LA_Fld_Name.u8_FieldId = CMBS_OCL_FLD_NAME;
      st_LA_Fld_Name.u16_DataLen     = strlen(g_st_APP_LA_OutgoingCallListEntry[u16_EntryID].pu8_Name);
      st_LA_Fld_Name.pu8_Data       = (void *)g_st_APP_LA_OutgoingCallListEntry[u16_EntryID].pu8_Name;
      cmbs_util_LA_Fld_NameAdd( p_entry, &st_LA_Fld_Name,  &u16_index );
   }

   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_OCL_FLD_DATETIME) )
   {
   ST_UTIL_LA_FLD_DATETIME st_LA_Fld_DateTime;
      st_LA_Fld_DateTime.u8_Attribute = 0x00;
      st_LA_Fld_DateTime.u8_FieldId = CMBS_OCL_FLD_DATETIME;
      st_LA_Fld_DateTime.u8_Year      = g_st_APP_LA_OutgoingCallListEntry[u16_EntryID].st_DateTime.u8_Year;
      st_LA_Fld_DateTime.u8_Month     = g_st_APP_LA_OutgoingCallListEntry[u16_EntryID].st_DateTime.u8_Month;
      st_LA_Fld_DateTime.u8_Day       = g_st_APP_LA_OutgoingCallListEntry[u16_EntryID].st_DateTime.u8_Day;
      st_LA_Fld_DateTime.u8_Hours     = g_st_APP_LA_OutgoingCallListEntry[u16_EntryID].st_DateTime.u8_Hours;
      st_LA_Fld_DateTime.u8_Mins      = g_st_APP_LA_OutgoingCallListEntry[u16_EntryID].st_DateTime.u8_Mins;
      st_LA_Fld_DateTime.u8_Secs      = g_st_APP_LA_OutgoingCallListEntry[u16_EntryID].st_DateTime.u8_Secs;
      cmbs_util_LA_Fld_DateTimeAdd( p_entry, &st_LA_Fld_DateTime,  &u16_index );
   }

   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_OCL_FLD_LINE_NAME) )
   {
   ST_UTIL_LA_FLD_LINE_NAME  st_LA_Fld_LineName;
      st_LA_Fld_LineName.u8_Attribute = 0x00;
      st_LA_Fld_LineName.u8_FieldId = CMBS_OCL_FLD_LINE_NAME;
      st_LA_Fld_LineName.u16_DataLen   = strlen(g_st_APP_LA_OutgoingCallListEntry[u16_EntryID].pu8_LineName);
      st_LA_Fld_LineName.pu8_Data     = (void *)g_st_APP_LA_OutgoingCallListEntry[u16_EntryID].pu8_LineName;
      cmbs_util_LA_Fld_LineNameAdd( p_entry, &st_LA_Fld_LineName,  &u16_index );
   }

   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_OCL_FLD_LINE_ID) )
   {
   ST_UTIL_LA_FLD_LINE_ID  st_LA_Fld_LineId;
      st_LA_Fld_LineId.u8_Attribute = 0x00;
      st_LA_Fld_LineId.u8_FieldId = CMBS_OCL_FLD_LINE_ID;
      st_LA_Fld_LineId.u8_Value     = g_st_APP_LA_OutgoingCallListEntry[u16_EntryID].u8_LineId;
      cmbs_util_LA_Fld_LineIdAdd( p_entry, &st_LA_Fld_LineId,  &u16_index );
   }
   break;


case CMBS_LA_LIST_INCOMING_ACC:
   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_IACL_FLD_NUMBER) )
   {
    ST_UTIL_LA_FLD_NUMBER st_LA_Fld_Number;
      st_LA_Fld_Number.u8_Attribute = (g_st_APP_LA_IncomingAcceptedCallListEntry[u16_EntryID].u8_NumberAttribute & 7) << 4;
      st_LA_Fld_Number.u8_FieldId = CMBS_IACL_FLD_NUMBER;
      st_LA_Fld_Number.u16_DataLen   = strlen(g_st_APP_LA_IncomingAcceptedCallListEntry[u16_EntryID].pu8_Number);
      st_LA_Fld_Number.pu8_Data     = (void *)g_st_APP_LA_IncomingAcceptedCallListEntry[u16_EntryID].pu8_Number;
      cmbs_util_LA_Fld_NumberAdd( p_entry, &st_LA_Fld_Number,  &u16_index); // index will be updated inside the function
   }

   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_IACL_FLD_NAME) )
   {
    ST_UTIL_LA_FLD_NAME st_LA_Fld_Name;
      st_LA_Fld_Name.u8_Attribute   = 0x00;
      st_LA_Fld_Name.u8_FieldId = CMBS_IACL_FLD_NAME;
      st_LA_Fld_Name.u16_DataLen     = strlen(g_st_APP_LA_IncomingAcceptedCallListEntry[u16_EntryID].pu8_Name);
      st_LA_Fld_Name.pu8_Data       = (void *)g_st_APP_LA_IncomingAcceptedCallListEntry[u16_EntryID].pu8_Name;
      cmbs_util_LA_Fld_NameAdd( p_entry, &st_LA_Fld_Name,  &u16_index );
   }

   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_IACL_FLD_DATETIME) )
   {
   ST_UTIL_LA_FLD_DATETIME st_LA_Fld_DateTime;
      st_LA_Fld_DateTime.u8_Attribute = 0x00;
      st_LA_Fld_DateTime.u8_FieldId = CMBS_IACL_FLD_DATETIME;
      st_LA_Fld_DateTime.u8_Year      = g_st_APP_LA_IncomingAcceptedCallListEntry[u16_EntryID].st_DateTime.u8_Year;
      st_LA_Fld_DateTime.u8_Month     = g_st_APP_LA_IncomingAcceptedCallListEntry[u16_EntryID].st_DateTime.u8_Month;
      st_LA_Fld_DateTime.u8_Day       = g_st_APP_LA_IncomingAcceptedCallListEntry[u16_EntryID].st_DateTime.u8_Day;
      st_LA_Fld_DateTime.u8_Hours     = g_st_APP_LA_IncomingAcceptedCallListEntry[u16_EntryID].st_DateTime.u8_Hours;
      st_LA_Fld_DateTime.u8_Mins      = g_st_APP_LA_IncomingAcceptedCallListEntry[u16_EntryID].st_DateTime.u8_Mins;
      st_LA_Fld_DateTime.u8_Secs      = g_st_APP_LA_IncomingAcceptedCallListEntry[u16_EntryID].st_DateTime.u8_Secs;
      cmbs_util_LA_Fld_DateTimeAdd( p_entry, &st_LA_Fld_DateTime,  &u16_index );
   }

   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_IACL_FLD_LINE_NAME) )
   {
   ST_UTIL_LA_FLD_LINE_NAME  st_LA_Fld_LineName;
      st_LA_Fld_LineName.u8_Attribute = 0x00;
      st_LA_Fld_LineName.u8_FieldId = CMBS_IACL_FLD_LINE_NAME;
      st_LA_Fld_LineName.u16_DataLen   = strlen(g_st_APP_LA_IncomingAcceptedCallListEntry[u16_EntryID].pu8_LineName);
      st_LA_Fld_LineName.pu8_Data     = (void *)g_st_APP_LA_IncomingAcceptedCallListEntry[u16_EntryID].pu8_LineName;
      cmbs_util_LA_Fld_LineNameAdd( p_entry, &st_LA_Fld_LineName,  &u16_index );
   }

   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_IACL_FLD_LINE_ID) )
   {
   ST_UTIL_LA_FLD_LINE_ID  st_LA_Fld_LineId;
      st_LA_Fld_LineId.u8_Attribute = 0x00;
      st_LA_Fld_LineId.u8_FieldId = CMBS_IACL_FLD_LINE_ID;
      st_LA_Fld_LineId.u8_Value     = g_st_APP_LA_IncomingAcceptedCallListEntry[u16_EntryID].u8_LineId;
      cmbs_util_LA_Fld_LineIdAdd( p_entry, &st_LA_Fld_LineId,  &u16_index );
   }
   break;

case CMBS_LA_LIST_CONTACT:
   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_CL_FLD_NAME) )
   {
    ST_UTIL_LA_FLD_NAME st_LA_Fld_Name;
      st_LA_Fld_Name.u8_Attribute   = 0x00;
      st_LA_Fld_Name.u8_FieldId = CMBS_CL_FLD_NAME;
      st_LA_Fld_Name.u16_DataLen     = strlen(g_st_APP_LA_ContactListEntry[u16_EntryID].pu8_Name);
      st_LA_Fld_Name.pu8_Data       = (void *)g_st_APP_LA_ContactListEntry[u16_EntryID].pu8_Name;
      cmbs_util_LA_Fld_NameAdd( p_entry, &st_LA_Fld_Name,  &u16_index );
   }
   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_CL_FLD_FIRST_NAME) )
   {
    ST_UTIL_LA_FLD_FIRST_NAME st_LA_Fld_FirstName;
      st_LA_Fld_FirstName.u8_Attribute   = 0x00;
      st_LA_Fld_FirstName.u8_FieldId = CMBS_CL_FLD_FIRST_NAME;
      st_LA_Fld_FirstName.u16_DataLen     = strlen(g_st_APP_LA_ContactListEntry[u16_EntryID].pu8_FirstName);
      st_LA_Fld_FirstName.pu8_Data       = (void *)g_st_APP_LA_ContactListEntry[u16_EntryID].pu8_FirstName;
      cmbs_util_LA_Fld_FirstNameAdd( p_entry, &st_LA_Fld_FirstName,  &u16_index );
   }

   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_CL_FLD_CONTACT_NR) )
   {
    ST_UTIL_LA_FLD_CONTACT_NR st_LA_Fld_ContactNumber;
      st_LA_Fld_ContactNumber.u8_Attribute = (g_st_APP_LA_ContactListEntry[u16_EntryID].u8_NumberAttribute << 1);  // LSB to MSB: work, mobile, fixed, own, default
      st_LA_Fld_ContactNumber.u8_FieldId = CMBS_CL_FLD_CONTACT_NR;
      st_LA_Fld_ContactNumber.u16_DataLen   = strlen(g_st_APP_LA_ContactListEntry[u16_EntryID].pu8_Number);
      st_LA_Fld_ContactNumber.pu8_Data     = (void *)g_st_APP_LA_ContactListEntry[u16_EntryID].pu8_Number;
      cmbs_util_LA_Fld_ContactNrAdd( p_entry, &st_LA_Fld_ContactNumber,  &u16_index); // index will be updated inside the function
   }

   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_CL_FLD_ASSOC_MDY) )
   {
   ST_UTIL_LA_FLD_ASSOC_MDY  st_LA_Fld_Associated_melody;
      st_LA_Fld_Associated_melody.u8_Attribute = 0x00;
      st_LA_Fld_Associated_melody.u8_FieldId = CMBS_CL_FLD_ASSOC_MDY;
      st_LA_Fld_Associated_melody.u8_Value     = g_st_APP_LA_ContactListEntry[u16_EntryID].u8_AssociatedMelody;
      cmbs_util_LA_Fld_AssocMdyAdd( p_entry, &st_LA_Fld_Associated_melody,  &u16_index );
   }

   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_CL_FLD_LINE_ID) )
   {
   ST_UTIL_LA_FLD_LINE_ID  st_LA_Fld_LineId;
      st_LA_Fld_LineId.u8_Attribute = 0x00;
      st_LA_Fld_LineId.u8_FieldId = CMBS_CL_FLD_LINE_ID;
      st_LA_Fld_LineId.u8_Value     = g_st_APP_LA_ContactListEntry[u16_EntryID].u8_LineId;
      cmbs_util_LA_Fld_LineIdAdd( p_entry, &st_LA_Fld_LineId,  &u16_index );
   }
   break;

case CMBS_LA_LIST_ALL_CALLS:
   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_ACL_FLD_CALL_TYPE) )
   {
    ST_UTIL_LA_FLD_CALL_TYPE st_LA_Fld_CallType;
      st_LA_Fld_CallType.u8_Attribute = (g_st_APP_LA_AllCallListEntry[u16_EntryID].u8_CallType & 7) << 5;
      st_LA_Fld_CallType.u8_FieldId = CMBS_ACL_FLD_CALL_TYPE;
      cmbs_util_LA_Fld_CallTypeAdd( p_entry, &st_LA_Fld_CallType,  &u16_index );
   }
   
   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_ACL_FLD_NUMBER) )
   {
    ST_UTIL_LA_FLD_NUMBER st_LA_Fld_Number;
      st_LA_Fld_Number.u8_Attribute = (g_st_APP_LA_AllCallListEntry[u16_EntryID].u8_NumberAttribute & 7) << 4;
      st_LA_Fld_Number.u8_FieldId = CMBS_ACL_FLD_NUMBER;
      st_LA_Fld_Number.u16_DataLen   = strlen(g_st_APP_LA_AllCallListEntry[u16_EntryID].pu8_Number);
      st_LA_Fld_Number.pu8_Data     = (void *)g_st_APP_LA_AllCallListEntry[u16_EntryID].pu8_Number;
      cmbs_util_LA_Fld_NumberAdd( p_entry, &st_LA_Fld_Number,  &u16_index); // index will be updated inside the function
   }

   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_ACL_FLD_NAME) )
   {
    ST_UTIL_LA_FLD_NAME st_LA_Fld_Name;
      st_LA_Fld_Name.u8_Attribute   = 0x00;
      st_LA_Fld_Name.u8_FieldId = CMBS_ACL_FLD_NAME;
      st_LA_Fld_Name.u16_DataLen     = strlen(g_st_APP_LA_AllCallListEntry[u16_EntryID].pu8_Name);
      st_LA_Fld_Name.pu8_Data       = (void *)g_st_APP_LA_AllCallListEntry[u16_EntryID].pu8_Name;
      cmbs_util_LA_Fld_NameAdd( p_entry, &st_LA_Fld_Name,  &u16_index );
   }

   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_ACL_FLD_DATETIME) )
   {
   ST_UTIL_LA_FLD_DATETIME st_LA_Fld_DateTime;
      st_LA_Fld_DateTime.u8_Attribute = 0x00;
      st_LA_Fld_DateTime.u8_FieldId = CMBS_ACL_FLD_DATETIME;
      st_LA_Fld_DateTime.u8_Year      = g_st_APP_LA_AllCallListEntry[u16_EntryID].st_DateTime.u8_Year;
      st_LA_Fld_DateTime.u8_Month     = g_st_APP_LA_AllCallListEntry[u16_EntryID].st_DateTime.u8_Month;
      st_LA_Fld_DateTime.u8_Day       = g_st_APP_LA_AllCallListEntry[u16_EntryID].st_DateTime.u8_Day;
      st_LA_Fld_DateTime.u8_Hours     = g_st_APP_LA_AllCallListEntry[u16_EntryID].st_DateTime.u8_Hours;
      st_LA_Fld_DateTime.u8_Mins      = g_st_APP_LA_AllCallListEntry[u16_EntryID].st_DateTime.u8_Mins;
      st_LA_Fld_DateTime.u8_Secs      = g_st_APP_LA_AllCallListEntry[u16_EntryID].st_DateTime.u8_Secs;
      cmbs_util_LA_Fld_DateTimeAdd( p_entry, &st_LA_Fld_DateTime,  &u16_index );
}

   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_ACL_FLD_LINE_NAME) )
   {
   ST_UTIL_LA_FLD_LINE_NAME  st_LA_Fld_LineName;
      st_LA_Fld_LineName.u8_Attribute = 0x00;
      st_LA_Fld_LineName.u8_FieldId = CMBS_ACL_FLD_LINE_NAME;
      st_LA_Fld_LineName.u16_DataLen   = strlen(g_st_APP_LA_AllCallListEntry[u16_EntryID].pu8_LineName);
      st_LA_Fld_LineName.pu8_Data     = (void *)g_st_APP_LA_AllCallListEntry[u16_EntryID].pu8_LineName;
      cmbs_util_LA_Fld_LineNameAdd( p_entry, &st_LA_Fld_LineName,  &u16_index );
   }

   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_ACL_FLD_LINE_ID) )
   {
   ST_UTIL_LA_FLD_LINE_ID  st_LA_Fld_LineId;
      st_LA_Fld_LineId.u8_Attribute = 0x00;
      st_LA_Fld_LineId.u8_FieldId = CMBS_ACL_FLD_LINE_ID;
      st_LA_Fld_LineId.u8_Value     = g_st_APP_LA_AllCallListEntry[u16_EntryID].u8_LineId;
      cmbs_util_LA_Fld_LineIdAdd( p_entry, &st_LA_Fld_LineId,  &u16_index );
}
   break;

case CMBS_LA_LIST_LINE_SETTINGS:
   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_LSL_FLD_LINE_NAME) )
   {
   ST_UTIL_LA_FLD_LINE_NAME  st_LA_Fld_LineName;
      st_LA_Fld_LineName.u8_Attribute = 0x00;
      st_LA_Fld_LineName.u8_FieldId = CMBS_LSL_FLD_LINE_NAME;
      st_LA_Fld_LineName.u16_DataLen   = strlen(g_st_APP_LA_LineSettingListEntry[u16_EntryID].pu8_LineName);
      st_LA_Fld_LineName.pu8_Data     = (void *)g_st_APP_LA_LineSettingListEntry[u16_EntryID].pu8_LineName;
      cmbs_util_LA_Fld_LineNameAdd( p_entry, &st_LA_Fld_LineName,  &u16_index );
   }
   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_LSL_FLD_LINE_ID) )
   {
   ST_UTIL_LA_FLD_LINE_ID  st_LA_Fld_LineId;
      st_LA_Fld_LineId.u8_Attribute = 0x00;
      st_LA_Fld_LineId.u8_FieldId = CMBS_LSL_FLD_LINE_ID;
      st_LA_Fld_LineId.u8_Value     = g_st_APP_LA_LineSettingListEntry[u16_EntryID].u8_LineId;
      cmbs_util_LA_Fld_LineIdAdd( p_entry, &st_LA_Fld_LineId,  &u16_index );
   }
   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_LSL_FLD_ATTACHED_HANDSETS) )
   {
    ST_UTIL_LA_FLD_ATTACHED_HANDSETS st_LA_Fld_AttachedHandsets;
      st_LA_Fld_AttachedHandsets.u8_Attribute = 0x00;
      st_LA_Fld_AttachedHandsets.u8_FieldId = CMBS_LSL_FLD_ATTACHED_HANDSETS;
      st_LA_Fld_AttachedHandsets.u16_DataLen   = g_st_APP_LA_LineSettingListEntry[u16_EntryID].u8_NumberHandsets;   
      st_LA_Fld_AttachedHandsets.pu8_Data     = (void *)g_st_APP_LA_LineSettingListEntry[u16_EntryID].pu8_HandsetBitfield; 
      cmbs_util_LA_Fld_AttachedHandsetsAdd( p_entry, &st_LA_Fld_AttachedHandsets,  &u16_index); // index will be updated inside the function
   }
   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_LSL_FLD_DIALING_PREFIX) )  // see number
   {
    ST_UTIL_LA_FLD_DIALING_PREFIX st_LA_Fld_DialingPrefixe;
      st_LA_Fld_DialingPrefixe.u8_Attribute   = 0x00;
      st_LA_Fld_DialingPrefixe.u8_FieldId = CMBS_LSL_FLD_DIALING_PREFIX;
      st_LA_Fld_DialingPrefixe.u16_DataLen     = strlen(g_st_APP_LA_LineSettingListEntry[u16_EntryID].pu8_DialingPrefixe);
      st_LA_Fld_DialingPrefixe.pu8_Data       = (void *)g_st_APP_LA_LineSettingListEntry[u16_EntryID].pu8_DialingPrefixe;
      cmbs_util_LA_Fld_DialPrefixeAdd( p_entry, &st_LA_Fld_DialingPrefixe,  &u16_index );
   }
   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_LSL_FLD_FP_MELODY) ) // see melody
   {
   ST_UTIL_LA_FLD_FP_MELODY  st_LA_Fld_FPmelody;
      st_LA_Fld_FPmelody.u8_Attribute = 0x00;
      st_LA_Fld_FPmelody.u8_FieldId = CMBS_LSL_FLD_FP_MELODY;
      st_LA_Fld_FPmelody.u8_Value     = g_st_APP_LA_LineSettingListEntry[u16_EntryID].u8_FPMelody;
      cmbs_util_LA_Fld_AssocMdyAdd( p_entry, &st_LA_Fld_FPmelody,  &u16_index );
   }
   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_LSL_FLD_FP_VOLUME) )
   {
   ST_UTIL_LA_FLD_FP_VOLUME  st_LA_Fld_FPvolume;
      st_LA_Fld_FPvolume.u8_Attribute = 0x00;
      st_LA_Fld_FPvolume.u8_FieldId = CMBS_LSL_FLD_FP_VOLUME;
      st_LA_Fld_FPvolume.u8_Value     = g_st_APP_LA_LineSettingListEntry[u16_EntryID].u8_FPVolume;
      cmbs_util_LA_Fld_FPvolumeAdd( p_entry, &st_LA_Fld_FPvolume,  &u16_index );
   }
   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_LSL_FLD_BLOCKED_NUMBER) )  // see number
   {
    ST_UTIL_LA_FLD_BLOCKED_NUMBER st_LA_Fld_BlockedNumber;
      st_LA_Fld_BlockedNumber.u8_Attribute   = 0x00;
      st_LA_Fld_BlockedNumber.u8_FieldId = CMBS_LSL_FLD_BLOCKED_NUMBER;
      st_LA_Fld_BlockedNumber.u16_DataLen     = strlen(g_st_APP_LA_LineSettingListEntry[u16_EntryID].pu8_BlockedNumber);
      st_LA_Fld_BlockedNumber.pu8_Data       = (void *)g_st_APP_LA_LineSettingListEntry[u16_EntryID].pu8_BlockedNumber;
      cmbs_util_LA_Fld_BlockedNumberAdd( p_entry, &st_LA_Fld_BlockedNumber,  &u16_index );
   }
   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_LSL_FLD_MULTI_CALL_MODE) )
   {
   ST_UTIL_LA_FLD_MULTI_CALL_MODE  st_LA_Fld_MultiCallMode;
      st_LA_Fld_MultiCallMode.u8_Attribute = 0x00;
      st_LA_Fld_MultiCallMode.u8_FieldId = CMBS_LSL_FLD_MULTI_CALL_MODE;
      st_LA_Fld_MultiCallMode.u8_Value     = g_st_APP_LA_LineSettingListEntry[u16_EntryID].u8_MultiCallMode;
      cmbs_util_LA_Fld_MultiCallModeAdd( p_entry, &st_LA_Fld_MultiCallMode,  &u16_index );
   	}
   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_LSL_FLD_INTRUSION_CALL) )
   {
   ST_UTIL_LA_FLD_INTRUSION_CALL  st_LA_Fld_IntrusionCall;
      st_LA_Fld_IntrusionCall.u8_Attribute = 0x00;
      st_LA_Fld_IntrusionCall.u8_FieldId = CMBS_LSL_FLD_INTRUSION_CALL;
      st_LA_Fld_IntrusionCall.u8_Value     = g_st_APP_LA_LineSettingListEntry[u16_EntryID].u8_IntrusionCall;
      cmbs_util_LA_Fld_IntrusionCallAdd( p_entry, &st_LA_Fld_IntrusionCall,  &u16_index );
   }
   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_LSL_FLD_PERMANENT_CLIR) ) 
   {
    ST_UTIL_LA_FLD_PERMANENT_CLIR st_LA_Fld_PermanentCLIR;
      st_LA_Fld_PermanentCLIR.u8_Attribute   = 0x00;
      st_LA_Fld_PermanentCLIR.u8_FieldId = CMBS_LSL_FLD_PERMANENT_CLIR;
      st_LA_Fld_PermanentCLIR.u8_Value                 = g_st_APP_LA_LineSettingListEntry[u16_EntryID].u8_CLIRValue;
      st_LA_Fld_PermanentCLIR.u8_ActDataLen         = g_st_APP_LA_LineSettingListEntry[u16_EntryID].u8_CLIRActDataLen;
      st_LA_Fld_PermanentCLIR.pu8_ActData            = (void *)g_st_APP_LA_LineSettingListEntry[u16_EntryID].pu8_CLIRActData;
      st_LA_Fld_PermanentCLIR.u8_DesactDataLen    = g_st_APP_LA_LineSettingListEntry[u16_EntryID].u8_CLIRDesactDataLen;  
      st_LA_Fld_PermanentCLIR.pu8_DesactData       = (void *)g_st_APP_LA_LineSettingListEntry[u16_EntryID].pu8_CLIRDesactData;
      cmbs_util_LA_Fld_PermanentCLIRAdd( p_entry, &st_LA_Fld_PermanentCLIR,  &u16_index );
   }
   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_LSL_FLD_CALL_FORWARDING) ) 
   {
    ST_UTIL_LA_FLD_CALL_FORWARDING st_LA_Fld_CallForwarding;
      st_LA_Fld_CallForwarding.u8_Attribute   = 0x00;
      st_LA_Fld_CallForwarding.u8_FieldId = CMBS_LSL_FLD_CALL_FORWARDING;
      st_LA_Fld_CallForwarding.u8_NotSetByUSer     = g_st_APP_LA_LineSettingListEntry[u16_EntryID].u8_CFUNotSetByUser; // 1 if not set by user, 0 otherwise
      st_LA_Fld_CallForwarding.u8_Value                 = g_st_APP_LA_LineSettingListEntry[u16_EntryID].u8_CFUValue;
      st_LA_Fld_CallForwarding.u8_ActDataLen         = g_st_APP_LA_LineSettingListEntry[u16_EntryID].u8_CFUActDataLen; 
      st_LA_Fld_CallForwarding.pu8_ActData            = (void *)g_st_APP_LA_LineSettingListEntry[u16_EntryID].pu8_CFUActData;
      st_LA_Fld_CallForwarding.u8_DesactDataLen    = g_st_APP_LA_LineSettingListEntry[u16_EntryID].u8_CFUDesactDataLen; 
      st_LA_Fld_CallForwarding.pu8_DesactData       = (void *)g_st_APP_LA_LineSettingListEntry[u16_EntryID].pu8_CFUDesactData;
      st_LA_Fld_CallForwarding.u8_NumberLen   = strlen(g_st_APP_LA_LineSettingListEntry[u16_EntryID].pu8_CFUNumber);
      st_LA_Fld_CallForwarding.pu8_Number     = (void *)g_st_APP_LA_LineSettingListEntry[u16_EntryID].pu8_CFUNumber;
      cmbs_util_LA_Fld_CallForwardingAdd( p_entry, &st_LA_Fld_CallForwarding,  &u16_index );
   }
   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_LSL_FLD_CFNA) ) 
   {
    ST_UTIL_LA_FLD_CFNA st_LA_Fld_CFNA;
      st_LA_Fld_CFNA.u8_Attribute   = 0x00;
      st_LA_Fld_CFNA.u8_FieldId = CMBS_LSL_FLD_CFNA;
      st_LA_Fld_CFNA.u8_NotSetByUSer     = g_st_APP_LA_LineSettingListEntry[u16_EntryID].u8_CFNANotSetByUser; // 1 if not set by user, 0 otherwise
      st_LA_Fld_CFNA.u8_Value                 = g_st_APP_LA_LineSettingListEntry[u16_EntryID].u8_CFNAValue;
      st_LA_Fld_CFNA.u8_NbSecond                 = g_st_APP_LA_LineSettingListEntry[u16_EntryID].u8_CFNANbSecond;
      st_LA_Fld_CFNA.u8_ActDataLen         = g_st_APP_LA_LineSettingListEntry[u16_EntryID].u8_CFNAActDataLen; 
      st_LA_Fld_CFNA.pu8_ActData            = (void *)g_st_APP_LA_LineSettingListEntry[u16_EntryID].pu8_CFNAActData;
      st_LA_Fld_CFNA.u8_DesactDataLen    = g_st_APP_LA_LineSettingListEntry[u16_EntryID].u8_CFNADesactDataLen; 
      st_LA_Fld_CFNA.pu8_DesactData       = (void *)g_st_APP_LA_LineSettingListEntry[u16_EntryID].pu8_CFNADesactData;
      st_LA_Fld_CFNA.u8_NumberLen   = strlen(g_st_APP_LA_LineSettingListEntry[u16_EntryID].pu8_CFNANumber);
      st_LA_Fld_CFNA.pu8_Number     = (void *)g_st_APP_LA_LineSettingListEntry[u16_EntryID].pu8_CFNANumber;
      cmbs_util_LA_Fld_CFNAAdd( p_entry, &st_LA_Fld_CFNA,  &u16_index );
   }
   if ( g_st_APP_LA_Session[i_SessionIndex].u16_requestedFields & (1 << CMBS_LSL_FLD_CFB) ) 
   {
    ST_UTIL_LA_FLD_CFB st_LA_Fld_CFB;
      st_LA_Fld_CFB.u8_Attribute   = 0x00;
      st_LA_Fld_CFB.u8_FieldId = CMBS_LSL_FLD_CFB;
      st_LA_Fld_CFB.u8_NotSetByUSer     = g_st_APP_LA_LineSettingListEntry[u16_EntryID].u8_CFBNotSetByUser; // 1 if not set by user, 0 otherwise
      st_LA_Fld_CFB.u8_Value                 = g_st_APP_LA_LineSettingListEntry[u16_EntryID].u8_CFBValue;
      st_LA_Fld_CFB.u8_ActDataLen         = g_st_APP_LA_LineSettingListEntry[u16_EntryID].u8_CFBActDataLen; 
      st_LA_Fld_CFB.pu8_ActData            = (void *)g_st_APP_LA_LineSettingListEntry[u16_EntryID].pu8_CFBActData;
      st_LA_Fld_CFB.u8_DesactDataLen    = g_st_APP_LA_LineSettingListEntry[u16_EntryID].u8_CFBDesactDataLen; 
      st_LA_Fld_CFB.pu8_DesactData       = (void *)g_st_APP_LA_LineSettingListEntry[u16_EntryID].pu8_CFBDesactData;
      st_LA_Fld_CFB.u8_NumberLen   = strlen(g_st_APP_LA_LineSettingListEntry[u16_EntryID].pu8_CFBNumber);
      st_LA_Fld_CFB.pu8_Number     = (void *)g_st_APP_LA_LineSettingListEntry[u16_EntryID].pu8_CFBNumber;
      cmbs_util_LA_Fld_CFBAdd( p_entry, &st_LA_Fld_CFB,  &u16_index );
   }
   break;


default:
break;
}
 // end to fill Data content structure in the "Data packet" command as per CATIQ 2.0 protocol   


   pu8_finalbuff =    cmbs_util_LA_EntryLenght_EntryIdentifierAdd( p_entry, u16_index, u16_EntryID);  

cmbs_dsr_la_DataPacketSend( pv_AppRef,
                                              u16_SessionId,
                                              u8_IsLast,
                                              p_entry,
                                              u16_index );

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
   g_st_APP_LA_MissedCallListEntry[1].u8_NumberAttribute= 0;
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

   g_st_APP_LA_MissedCallListEntry[2].u8_NumberAttribute= 0;
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

   g_st_APP_LA_MissedCallListEntry[3].u8_NumberAttribute= 0;
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

   g_st_APP_LA_MissedCallListEntry[4].u8_NumberAttribute= 0;
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

   g_st_APP_LA_MissedCallListEntry[5].u8_NumberAttribute= 0;
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
