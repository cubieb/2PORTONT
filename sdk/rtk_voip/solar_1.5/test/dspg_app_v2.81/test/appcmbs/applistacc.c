/*************************** [Includes] *******************************************************/
#ifdef __linux
    #include <pthread.h>
#endif

#include <stdio.h>
#include <string.h>
#include "cmbs_api.h"
#include "cmbs_int.h"
#include "cmbs_str.h"
#include "LASessionMgr.h"

/*************************** [Globals] ********************************************************/
u8 u8_APP_LA_Initialized = 0;


/*************************** [List access protocol implementation] ****************************/
void app_LaInitialize( void )
{
    LASessionMgr_Init();
}

int app_LaIsValidList ( u16 u16_ListId )
{
    if ( ( u16_ListId == CMBS_LA_LIST_MISSED_CALLS )    ||   
         ( u16_ListId == CMBS_LA_LIST_OUTGOING_CALLS )  ||
         ( u16_ListId == CMBS_LA_LIST_INCOMING_ACC)     ||
         ( u16_ListId == CMBS_LA_LIST_ALL_CALLS )       ||      
         ( u16_ListId == CMBS_LA_LIST_CONTACT )         ||       
         ( u16_ListId == CMBS_LA_LIST_LINE_SETTINGS )   ||
         ( u16_ListId == CMBS_LA_LIST_ALL_INCOMING_CALLS )
       )
    {
        return 1;
    }

    return 0;
}

void app_SendDataPackets(void* pv_AppRefHandle, u16 u16_SessionId, u8 pu8_Data[], u16* pu16_Marker, u16* pu16_Len)
{
    bool IsLast;
    u16 u16_DataPacketSize;
    u16 u16_BytesLeft = (*pu16_Len - *pu16_Marker);

    if ( u16_BytesLeft < 57 )
    {
        u16_DataPacketSize = u16_BytesLeft;
        IsLast = TRUE;
    }
    else
    {
        u16_DataPacketSize = 56;
        IsLast = FALSE;
    }

    cmbs_dsr_la_DataPacketSend( pv_AppRefHandle,
                                u16_SessionId,
                                IsLast,
                                pu8_Data + *pu16_Marker,
                                u16_DataPacketSize);

    *pu16_Marker += u16_DataPacketSize;
}

int app_OnLaSessionStart(void * pv_AppRef, void * pv_EventData)
{
    void *          pv_IE = NULL;
    u16             u16_IE, u16_SessionId, u16_ListId, u16_HandsetId, u16_NrOfEntries, i = 0;
    LA_SESSION_MGR_RC session_rc;
    ST_IE_LA_FIELDS st_LaFields;
    E_CMBS_RESPONSE e_Response = CMBS_RESPONSE_OK;
    E_CMBS_LA_START_SESSION_ERRORS
    e_RejectReason = CMBS_LA_START_SESSION_ERROR_MAX;

    cmbs_api_ie_GetFirst( pv_EventData, &pv_IE, &u16_IE );
    while ( pv_IE != NULL )
    {
        printf( "%s: ", getstr_E_CMBS_IE( u16_IE ) );

        switch ( u16_IE )
        {
        case CMBS_IE_LA_SESSION_ID:     /*!< Id of the List Access Session */

            cmbs_api_ie_LASessionIdGet( pv_IE, &u16_SessionId );
            printf( "%d\n", u16_SessionId );
            break;

        case CMBS_IE_HANDSETS:          /*!< Bit mask of one or more handsets */

            cmbs_api_ie_HandsetsGet( pv_IE, &u16_HandsetId );
            for ( i = 0; i < 16; i++ )
            {
                if ( u16_HandsetId & (1<<i ) )
                    break;
            }

            u16_HandsetId = i;
            printf( "%d\n", i+1 );
            break;

        case CMBS_IE_LA_LIST_ID:        /*!< Id of the list being accessed via Access List Service */

            cmbs_api_ie_LAListIdGet( pv_IE, &u16_ListId );
            printf( "%d\n", u16_ListId );
            break;

        case CMBS_IE_LA_SORT_FIELDS:    /*!< Holds sort fields of a list. */
            printf("LA_xxx_FIELDS:\n");

            cmbs_api_ie_LAFieldsGet( pv_IE, &st_LaFields );
            printf( "Length:%d\n", st_LaFields.u16_Length );
            printf( "Sort fields:" );
            if ( st_LaFields.u16_Length == 0 )
            {
                printf(" (none)\n");
            }
            else
            {
                for ( i = 0; i < st_LaFields.u16_Length; i++ )
                {
                    printf( " %2d", st_LaFields.pu16_FieldId[i] );
                }
                printf( "\n" );
            }
            break;

        default:
            break;
        }
        cmbs_api_ie_GetNext( pv_EventData, &pv_IE, &u16_IE );
    }

    printf( "\nStart Session SessionId=%x\n", u16_SessionId );

    session_rc = LASessionMgr_StartSession(u16_SessionId, u16_ListId, u16_HandsetId, &st_LaFields, &u16_NrOfEntries);

    if ( session_rc == LA_SESSION_MGR_RC_NO_FREE_SESSIONS )
    {
        e_Response     = CMBS_RESPONSE_ERROR;
        e_RejectReason = CMBS_LA_START_SESSION_ERROR_MAX_SESSIONS_REACHED;
    }
    else if ( session_rc == LA_SESSION_MGR_RC_UNSUPPORTED_LIST )
    {
        e_Response     = CMBS_RESPONSE_ERROR;
        e_RejectReason = CMBS_LA_START_SESSION_ERROR_LIST_NOT_SUPPORTED;
    }

    if ( !app_LaIsValidList ( u16_ListId ) )
    {
        printf("ERROR: No valid list selected\n");
        e_Response     = CMBS_RESPONSE_ERROR;
        e_RejectReason = CMBS_LA_START_SESSION_ERROR_LIST_NOT_SUPPORTED;
    }

    if ( e_Response != CMBS_RESPONSE_OK )
    {
        u16_NrOfEntries = 0;
        st_LaFields.u16_Length = 0;
    }

    return cmbs_dsr_la_SessionStartRes( pv_AppRef, u16_SessionId, u16_NrOfEntries, &st_LaFields, e_Response, e_RejectReason );
}

int app_OnLaSessionEnd(void * pv_AppRef, void * pv_EventData)
{
    void *                  pv_IE = NULL;
    u16                     u16_IE;
    u16                     u16_SessionId;
    LA_SESSION_MGR_RC       session_rc;
    E_CMBS_RESPONSE         e_Response = CMBS_RESPONSE_OK;
    E_CMBS_LA_NACK_ERRORS   e_RejectReason = CMBS_LA_NACK_MAX;

    cmbs_api_ie_GetFirst( pv_EventData, &pv_IE, &u16_IE );
    while ( pv_IE != NULL )
    {
        printf( "%s: ", getstr_E_CMBS_IE( u16_IE ) );       
        switch ( u16_IE )
        {
        case CMBS_IE_LA_SESSION_ID:     /*!< Id of the List Access Session */

            cmbs_api_ie_LASessionIdGet( pv_IE, &u16_SessionId );
            printf( "%d\n", u16_SessionId );
            break;
        default:
            break;
        }
        cmbs_api_ie_GetNext( pv_EventData, &pv_IE, &u16_IE );
    }

    printf("End Session SessionId=%x\n", u16_SessionId);

    session_rc = LASessionMgr_EndSession(u16_SessionId);

    if ( session_rc == LA_SESSION_MGR_RC_UNKNOWN_SESSION_ID )
    {
        printf("ERROR: u16_SessionId=%x not known\n", u16_SessionId );
        e_Response     = CMBS_RESPONSE_ERROR;
        e_RejectReason = CMBS_LA_NACK_INVALID_SESSION;
    }

    return cmbs_dsr_la_SessionEndRes( pv_AppRef, u16_SessionId, e_Response, e_RejectReason );
}

int app_OnLaSessionEndRes(void * pv_AppRef, void * pv_EventData)
{
    UNUSED_PARAMETER(pv_AppRef);
    UNUSED_PARAMETER(pv_EventData);

    return TRUE;
}

int app_OnLaQuerySuppEntryFields(void * pv_AppRef, void * pv_EventData)
{
    E_CMBS_RESPONSE         e_Response = CMBS_RESPONSE_OK;
    E_CMBS_LA_NACK_ERRORS   e_RejectReason = CMBS_LA_NACK_MAX;
    ST_LA_FIELDS            st_LANonEditFields;
    ST_LA_FIELDS            st_LAEditFields;
    void *                  pv_IE = NULL;
    u16                     u16_IE;
    u16                     u16_SessionId;
    LA_SESSION_MGR_RC       session_rc;

    memset(&st_LANonEditFields, 0, sizeof(st_LANonEditFields));
    memset(&st_LAEditFields, 0, sizeof(st_LAEditFields));

    cmbs_api_ie_GetFirst( pv_EventData, &pv_IE, &u16_IE );
    while ( pv_IE != NULL )
    {
        printf( "%s: ", getstr_E_CMBS_IE( u16_IE ) );       
        switch ( u16_IE )
        {
        case CMBS_IE_LA_SESSION_ID:     /*!< Id of the List Access Session */

            cmbs_api_ie_LASessionIdGet( pv_IE, &u16_SessionId );
            printf( "%d\n", u16_SessionId );
            break;
        default:
            break;
        }
        cmbs_api_ie_GetNext( pv_EventData, &pv_IE, &u16_IE );
    }

    printf("Query Supported Fields\n");

    session_rc = LASessionMgr_GetSupportedFields(u16_SessionId, &st_LAEditFields, &st_LANonEditFields);

    if ( session_rc == LA_SESSION_MGR_RC_UNKNOWN_SESSION_ID )
    {
        printf("ERROR: Wrong SessionId=%x\n", u16_SessionId );
        e_Response     = CMBS_RESPONSE_ERROR;
        e_RejectReason = CMBS_LA_NACK_INVALID_SESSION;
    }

    return cmbs_dsr_la_QuerySupEntryFieldsRes( pv_AppRef, u16_SessionId,
                                               &st_LANonEditFields, &st_LAEditFields, e_Response, e_RejectReason );
}

int app_OnLaReadEntries(void * pv_AppRef, void * pv_EventData)
{
    E_CMBS_RESPONSE         e_Response = CMBS_RESPONSE_OK;
    E_CMBS_LA_NACK_ERRORS   e_RejectReason = CMBS_LA_NACK_MAX;
    void *                  pv_IE = NULL;
    u16                     u16_IE;
    u16                     u16_SessionId;
    ST_IE_LA_FIELDS         st_LaFields;
    u16                     u16_EntryIndex;
    u16                     u16_EntryCountRequested;
    int                     i = 0;
    LA_SESSION_MGR_RC       session_rc;
    ST_IE_READ_DIRECTION    st_ReadDirection;
    ST_IE_MARK_REQUEST      st_Mark;
    u8*                     pu8_Data;
    u16                     *pu16_Marker, *pu16_TotalLen;
    bool                    bForward;

    cmbs_api_ie_GetFirst( pv_EventData, &pv_IE, &u16_IE );
    while ( pv_IE != NULL )
    {
        printf( "%s: ", getstr_E_CMBS_IE( u16_IE ) );
        switch ( u16_IE )
        {
        case CMBS_IE_LA_SESSION_ID:     /*!< Id of the List Access Session */

            cmbs_api_ie_LASessionIdGet( pv_IE, &u16_SessionId );
            printf( "%d\n", u16_SessionId );
            break;

        case CMBS_IE_LA_FIELDS:
            printf("LA_FIELDS:\n");
            cmbs_api_ie_LAFieldsGet( pv_IE, &st_LaFields );
            printf( "Length:%d\n", st_LaFields.u16_Length );
            printf( "FieldIds:" );

            if ( st_LaFields.u16_Length == 0 )
            {
                printf(" (none)\n");
            }
            else
            {
                for ( i = 0; i < st_LaFields.u16_Length; i++ )
                {
                    printf( " %2d", st_LaFields.pu16_FieldId[i] );
                }
                printf( "\n" );
            }
            break;

        case CMBS_IE_LA_ENTRY_INDEX:
            cmbs_api_ie_LAEntryIndexGet( pv_IE, &u16_EntryIndex );
            printf( "u16_EntryIndex=%d\n", u16_EntryIndex );
            break;

        case CMBS_IE_LA_ENTRY_COUNT:
            cmbs_api_ie_LAEntryCountGet( pv_IE, &u16_EntryCountRequested );
            printf( "u16_EntryCountRequested=%d\n", u16_EntryCountRequested );
            break;

        case CMBS_IE_LA_READ_DIRECTION:
            cmbs_api_ie_ReadDirectionGet( pv_IE, &st_ReadDirection );
            printf( "Read Direction=%d\n", st_ReadDirection.e_ReadDirection );
            if ( st_ReadDirection.e_ReadDirection == CMBS_READ_DIRECTION_FORWARD )
            {
                bForward = TRUE;
            }
            else
            {
                bForward = FALSE;
            }
            break;

        case CMBS_IE_LA_MARK_REQUEST:
            cmbs_api_ie_MarkRequestGet( pv_IE, &st_Mark );
            printf( "Mark=%d\n", st_Mark.e_MarkRequest );
            break;

        default:
            break;
        }
        cmbs_api_ie_GetNext( pv_EventData, &pv_IE, &u16_IE );
    }

    LASessionMgr_GetDataBuffer(u16_SessionId, &pu8_Data, &pu16_Marker, &pu16_TotalLen);

    session_rc = LASessionMgr_ReadEntries(u16_SessionId, u16_EntryIndex, bForward, st_Mark.e_MarkRequest,
                                          &st_LaFields, &u16_EntryCountRequested, pu8_Data, pu16_TotalLen);

    if ( session_rc == LA_SESSION_MGR_RC_UNKNOWN_SESSION_ID )
    {
        e_Response = CMBS_RESPONSE_ERROR;
        e_RejectReason = CMBS_LA_NACK_INVALID_SESSION;
    }
    else if ( session_rc == LA_SESSION_MGR_RC_NOT_ENOUGH_MEMORY )
    {
        e_Response = CMBS_RESPONSE_ERROR;
        e_RejectReason = CMBS_LA_NACK_TEMP_NOT_POSSIBLE;
    }
    else if ( session_rc == LA_SESSION_MGR_RC_INVALID_START_INDEX )
    {
        e_Response = CMBS_RESPONSE_ERROR;
        e_RejectReason = CMBS_LA_NACK_INVALID_START_INDEX;
    }

    cmbs_dsr_la_ReadEntriesRes(pv_AppRef, u16_SessionId, u16_EntryIndex, u16_EntryCountRequested, e_Response, e_RejectReason);

    /* Possibly need to send Missed call notification */
    LASessionMgr_SendMissedCallNotification(u16_SessionId, st_Mark.e_MarkRequest);

    if ( e_Response == CMBS_RESPONSE_OK )
    {
        app_SendDataPackets(pv_AppRef, u16_SessionId, pu8_Data, pu16_Marker, pu16_TotalLen);
    }

    return TRUE;
}

int app_OnLaEditEntry(void * pv_AppRef, void * pv_EventData)
{
    E_CMBS_RESPONSE         e_Response = CMBS_RESPONSE_OK;
    E_CMBS_LA_NACK_ERRORS   e_RejectReason = CMBS_LA_NACK_MAX;
    ST_IE_LA_FIELDS         st_LaFields;
    u16                     u16_EntryID;
    void *                  pv_IE = NULL;
    u16                     u16_IE;
    u16                     u16_SessionId;
    int                     i = 0;
    LA_SESSION_MGR_RC       session_rc;
    u8*                     pu8_Data;
    u16                     *pu16_Marker, *pu16_TotalLen;

    cmbs_api_ie_GetFirst( pv_EventData, &pv_IE, &u16_IE );
    while ( pv_IE != NULL )
    {
        printf( "%s: ", getstr_E_CMBS_IE( u16_IE ) );
        switch ( u16_IE )
        {
        case CMBS_IE_LA_SESSION_ID:     /*!< Id of the List Access Session */
            cmbs_api_ie_LASessionIdGet( pv_IE, &u16_SessionId );
            printf( "%d\n", u16_SessionId );
            break;

        case CMBS_IE_LA_EDIT_FIELDS:
            printf("LA_FIELDS:\n");
            cmbs_api_ie_LAFieldsGet( pv_IE, &st_LaFields );
            printf( "Length:%d\n", st_LaFields.u16_Length );
            printf( "FieldIds:" );

            if ( st_LaFields.u16_Length == 0 )
            {
                printf(" (none)\n");
            }
            else
            {
                for ( i = 0; i < st_LaFields.u16_Length; i++ )
                {
                    printf( " %2d", st_LaFields.pu16_FieldId[i] );
                }
                printf( "\n" );
            }
            break;

        case CMBS_IE_LA_ENTRY_ID:
            cmbs_api_ie_LAEntryIdGet( pv_IE, &u16_EntryID );
            break;

        default:
            break;
        }
        cmbs_api_ie_GetNext( pv_EventData, &pv_IE, &u16_IE );
    }
    printf("Edit EntryID %d\n", u16_EntryID);

    LASessionMgr_GetDataBuffer(u16_SessionId, &pu8_Data, &pu16_Marker, &pu16_TotalLen);

    session_rc = LASessionMgr_EditEntry(u16_SessionId, u16_EntryID, &st_LaFields, pu8_Data, pu16_TotalLen);

    if ( session_rc == LA_SESSION_MGR_RC_UNKNOWN_SESSION_ID )
    {
        printf("ERROR: Wrong SessionId=%x\n", u16_SessionId );
        e_Response     = CMBS_RESPONSE_ERROR;
        e_RejectReason = CMBS_LA_NACK_INVALID_SESSION;
    }
    else if ( session_rc == LA_SESSION_MGR_RC_ENTRY_NOT_AVAILABLE )
    {
        printf("ERROR: Entry Not Available=%x\n", u16_EntryID );
        e_Response     = CMBS_RESPONSE_ERROR;
        e_RejectReason = CMBS_LA_NACK_ENTRY_NOT_AVAILABLE;
    }

    cmbs_dsr_la_EditEntryRes( pv_AppRef, u16_SessionId, e_Response, e_RejectReason );

    if ( e_Response == CMBS_RESPONSE_OK )
    {
        app_SendDataPackets(pv_AppRef, u16_SessionId, pu8_Data, pu16_Marker, pu16_TotalLen);
    }

    return TRUE;
}

int app_OnLaSaveEntry(void * pv_AppRef, void * pv_EventData)
{
    u16                     u16_EntryID;
    void *                  pv_IE = NULL;
    u16                     u16_IE;
    u16                     u16_SessionId;

    UNUSED_PARAMETER(pv_AppRef);

    cmbs_api_ie_GetFirst( pv_EventData, &pv_IE, &u16_IE );
    while ( pv_IE != NULL )
    {
        printf( "%s: ", getstr_E_CMBS_IE( u16_IE ) );
        switch ( u16_IE )
        {
        case CMBS_IE_LA_SESSION_ID:     /*!< Id of the List Access Session */

            cmbs_api_ie_LASessionIdGet( pv_IE, &u16_SessionId );
            printf( "%d\n", u16_SessionId );
            break;

        case CMBS_IE_LA_ENTRY_ID:
            cmbs_api_ie_LAEntryIdGet( pv_IE, &u16_EntryID );
            break;

        default:
            break;
        }
        cmbs_api_ie_GetNext( pv_EventData, &pv_IE, &u16_IE );
    }

    printf("Save Entry %d\n", u16_EntryID);

    LASessionMgr_SetSaveEntryID(u16_SessionId, u16_EntryID);

    return TRUE;
}

int app_OnLaDeleteEntry(void * pv_AppRef, void * pv_EventData)
{
    E_CMBS_RESPONSE         e_Response = CMBS_RESPONSE_OK;
    E_CMBS_LA_NACK_ERRORS   e_RejectReason = CMBS_LA_NACK_MAX;
    u16                     u16_TotalNoOfEntries;
    u16                     u16_EntryID;
    void *                  pv_IE = NULL;
    u16                     u16_IE;
    u16                     u16_SessionId;
    LA_SESSION_MGR_RC       session_rc;

    cmbs_api_ie_GetFirst( pv_EventData, &pv_IE, &u16_IE );
    while ( pv_IE != NULL )
    {
        printf( "%s: ", getstr_E_CMBS_IE( u16_IE ) );
        switch ( u16_IE )
        {
        case CMBS_IE_LA_SESSION_ID:     /*!< Id of the List Access Session */
            cmbs_api_ie_LASessionIdGet( pv_IE, &u16_SessionId );
            printf( "%d\n", u16_SessionId );
            break;

        case CMBS_IE_LA_ENTRY_ID:
            cmbs_api_ie_LAEntryIdGet( pv_IE, &u16_EntryID );
            printf( "u16_EntryID=%d\n", u16_EntryID );
            break;

        default:
            break;
        }
        cmbs_api_ie_GetNext( pv_EventData, &pv_IE, &u16_IE );
    }

    printf("Delete EntryID %d\n", u16_EntryID);

    session_rc = LASessionMgr_DeleteEntry(u16_SessionId, u16_EntryID, &u16_TotalNoOfEntries);
    if ( session_rc == LA_SESSION_MGR_RC_UNKNOWN_SESSION_ID )
    {
        printf("ERROR: Wrong SessionId=%x\n", u16_SessionId );
        e_Response     = CMBS_RESPONSE_ERROR;
        e_RejectReason = CMBS_LA_NACK_INVALID_SESSION;
    }
    else if ( session_rc == LA_SESSION_MGR_RC_ENTRY_NOT_AVAILABLE )
    {
        e_Response     = CMBS_RESPONSE_ERROR;
        e_RejectReason = CMBS_LA_NACK_ENTRY_NOT_AVAILABLE;
    }

    cmbs_dsr_la_DeleteEntryRes( pv_AppRef, u16_SessionId, u16_TotalNoOfEntries, e_Response, e_RejectReason );

    /* Possibly need to send Missed call notification / List Change notification */
    LASessionMgr_SendListChangedNotificationOnDelete(u16_SessionId);

    return TRUE;
}

int app_OnLaDeleteList(void * pv_AppRef, void * pv_EventData)
{
    E_CMBS_RESPONSE         e_Response = CMBS_RESPONSE_OK;
    E_CMBS_LA_NACK_ERRORS   e_RejectReason = CMBS_LA_NACK_MAX;
    void *                  pv_IE = NULL;
    u16                     u16_IE;
    u16                     u16_SessionId;
    LA_SESSION_MGR_RC       session_rc;

    cmbs_api_ie_GetFirst( pv_EventData, &pv_IE, &u16_IE );
    while ( pv_IE != NULL )
    {
        printf( "%s: ", getstr_E_CMBS_IE( u16_IE ) );
        switch ( u16_IE )
        {
        case CMBS_IE_LA_SESSION_ID:     /*!< Id of the List Access Session */
            cmbs_api_ie_LASessionIdGet( pv_IE, &u16_SessionId );
            printf( "%d\n", u16_SessionId );
            break;

        default:
            break;
        }
        cmbs_api_ie_GetNext( pv_EventData, &pv_IE, &u16_IE );
    }

    printf("Delete List\n");

    session_rc = LASessionMgr_DeleteAllEntries(u16_SessionId);

    if ( session_rc == LA_SESSION_MGR_RC_UNKNOWN_SESSION_ID )
    {
        printf("ERROR: Wrong SessionId=%x\n", u16_SessionId );
        e_Response     = CMBS_RESPONSE_ERROR;
        e_RejectReason = CMBS_LA_NACK_INVALID_SESSION;
    }
    else if ( session_rc == LA_SESSION_MGR_RC_NOT_ALLOWED )
    {
        printf("ERROR: Cannot delete list\n");
        e_Response     = CMBS_RESPONSE_ERROR;
        e_RejectReason = CMBS_LA_NACK_PROC_NOT_ALLOWED;
    }

    cmbs_dsr_la_DeleteListRes( pv_AppRef, u16_SessionId, e_Response, e_RejectReason );

    /* Possibly need to send Missed call notification */
    LASessionMgr_SendListChangedNotificationOnDelete(u16_SessionId);

    return TRUE;
}

int app_OnLaSearchEntries(void * pv_AppRef, void * pv_EventData)
{
    u32                         u32_EntryStartIndex;
    u16                         u16_EntryCount = 0;
    ST_IE_LA_SEARCH_CRITERIA    st_LASearchCriteria;
    ST_IE_LA_FIELDS             st_LaFields;
    E_CMBS_RESPONSE             e_Response = CMBS_RESPONSE_OK;
    E_CMBS_LA_NACK_ERRORS       e_RejectReason = CMBS_LA_NACK_MAX;
    int                         i = 0;
    void *                      pv_IE = NULL;
    u16                         u16_IE;
    u16                         u16_SessionId;
    u8*                         pu8_Data;
    u16                         *pu16_Marker, *pu16_TotalLen;
    LA_SESSION_MGR_RC           session_rc;

    memset ( &st_LASearchCriteria, 0, sizeof(st_LASearchCriteria) );
    memset ( &st_LaFields, 0, sizeof(st_LaFields) );

    cmbs_api_ie_GetFirst( pv_EventData, &pv_IE, &u16_IE );
    while ( pv_IE != NULL )
    {
        printf( "%s: ", getstr_E_CMBS_IE( u16_IE ) );
        switch ( u16_IE )
        {
        case CMBS_IE_LA_SESSION_ID:     /*!< Id of the List Access Session */
            cmbs_api_ie_LASessionIdGet( pv_IE, &u16_SessionId );
            printf( "%d\n", u16_SessionId );
            break;

        case CMBS_IE_LA_SEARCH_CRITERIA:
            cmbs_api_ie_LASearchCriteriaGet( pv_IE, &st_LASearchCriteria );
            printf( "MatchingType:%d\n", st_LASearchCriteria.e_MatchingType );
            printf( "CaseSensitive:%d\n", st_LASearchCriteria.u8_CaseSensitive );
            printf( "Direction:%d\n", st_LASearchCriteria.u8_Direction );
            printf( "MarkEntriesReq:%d\n", st_LASearchCriteria.u8_MarkEntriesReq );
            printf( "Pattern:%s\n", st_LASearchCriteria.pu8_Pattern );
            break;

        case CMBS_IE_LA_ENTRY_COUNT:
            cmbs_api_ie_LAEntryCountGet( pv_IE, &u16_EntryCount );
            printf( "Entry counter:%d\n", u16_EntryCount );
            break;

        case CMBS_IE_LA_FIELDS:
            cmbs_api_ie_LAFieldsGet( pv_IE, &st_LaFields );
            printf( "LA_FIELDS:\n");
            printf( "Length:%d\n", st_LaFields.u16_Length );
            printf( "FieldIds:" );
            if ( st_LaFields.u16_Length == 0 )
            {
                printf(" (none)\n");
            }
            else
            {
                for ( i = 0; i < st_LaFields.u16_Length; i++ )
                {
                    printf( " %2d", st_LaFields.pu16_FieldId[i] );
                }
                printf( "\n" );
            }
            break;

        default:
            break;
        }
        cmbs_api_ie_GetNext( pv_EventData, &pv_IE, &u16_IE );
    }

    printf("Search for entry\n");

    LASessionMgr_GetDataBuffer(u16_SessionId, &pu8_Data, &pu16_Marker, &pu16_TotalLen);

    session_rc = LASessionMgr_SearchEntries(u16_SessionId, st_LASearchCriteria.e_MatchingType, st_LASearchCriteria.u8_CaseSensitive,
                                            (const char*)(st_LASearchCriteria.pu8_Pattern), !(st_LASearchCriteria.u8_Direction),
                                            st_LASearchCriteria.u8_MarkEntriesReq, &st_LaFields, &u16_EntryCount, 
                                            pu8_Data, pu16_TotalLen, &u32_EntryStartIndex);

    if ( session_rc == LA_SESSION_MGR_RC_UNKNOWN_SESSION_ID )
    {
        e_Response = CMBS_RESPONSE_ERROR;
        e_RejectReason = CMBS_LA_NACK_INVALID_SESSION;
    }
    else if ( session_rc == LA_SESSION_MGR_RC_NOT_ENOUGH_MEMORY )
    {
        e_Response = CMBS_RESPONSE_ERROR;
        e_RejectReason = CMBS_LA_NACK_TEMP_NOT_POSSIBLE;
    }

    cmbs_dsr_la_SearchEntriesRes(pv_AppRef, u16_SessionId, u32_EntryStartIndex, u16_EntryCount, e_Response, e_RejectReason );

    /* Possibly need to send Missed call notification */
    LASessionMgr_SendMissedCallNotification(u16_SessionId, st_LASearchCriteria.u8_MarkEntriesReq);

    if ( (e_Response == CMBS_RESPONSE_OK) && (u16_EntryCount > 0))
    {
        app_SendDataPackets(pv_AppRef, u16_SessionId, pu8_Data, pu16_Marker, pu16_TotalLen);
    }

    return TRUE;
}

int app_OnLaDataPacketReceive(void * pv_AppRef, void * pv_EventData)
{
    u32                     u32_NumOfEntries, u32_EntryId, u32_PositionIdx;
    E_CMBS_RESPONSE         e_Response = CMBS_RESPONSE_OK;
    E_CMBS_LA_NACK_ERRORS   e_RejectReason = CMBS_LA_NACK_MAX;
    E_CMBS_RC               r_res = CMBS_RC_OK;
    ST_IE_DATA              st_DataGet;
    void *                  pv_IE = NULL;
    u16                     u16_IE;
    u16                     u16_SessionId;
    u8                      u8_IsLast;
    u8*                     pu8_DataBuffer;
    u16                     *pu16_Marker, *pu16_TotalLen;
    LA_SESSION_MGR_RC       session_rc;

    cmbs_api_ie_GetFirst( pv_EventData, &pv_IE, &u16_IE );
    while ( pv_IE != NULL )
    {
        printf( "%s: ", getstr_E_CMBS_IE( u16_IE ) );
        switch ( u16_IE )
        {
        case CMBS_IE_LA_SESSION_ID:     /*!< Id of the List Access Session */
            cmbs_api_ie_LASessionIdGet( pv_IE, &u16_SessionId );
            printf( "%d\n", u16_SessionId );
            break;

        case CMBS_IE_LA_IS_LAST:
            cmbs_api_ie_LAIsLastGet(pv_IE, &u8_IsLast );
            printf( "IsLAst:%d\n", u8_IsLast );
            break;

        case CMBS_IE_DATA:
            printf("IE_DATA:\n");
            cmbs_api_ie_DataGet(pv_IE, &st_DataGet);
            break;

        default:
            break;
        }
        cmbs_api_ie_GetNext( pv_EventData, &pv_IE, &u16_IE );
    }

    /* get the packet into the buffer related to the sessionID */
    session_rc = LASessionMgr_GetDataBuffer(u16_SessionId, &pu8_DataBuffer, &pu16_Marker, &pu16_TotalLen);
    if ( session_rc != LA_SESSION_MGR_RC_OK )
    {
        r_res = CMBS_RC_ERROR_GENERAL;
    }

    /* append data */
    memcpy(pu8_DataBuffer + *pu16_TotalLen, st_DataGet.pu8_Data, st_DataGet.u16_DataLen);
    *pu16_TotalLen += st_DataGet.u16_DataLen;

    /* send response */
    cmbs_dsr_la_DataPacketReceivedRes( pv_AppRef, u16_SessionId, r_res );

    /* if it was the last then start the parsing */
    if ( u8_IsLast && (session_rc == LA_SESSION_MGR_RC_OK) )
    {
        session_rc = LASessionMgr_SaveEntry(u16_SessionId, &u32_EntryId, &u32_PositionIdx, &u32_NumOfEntries);

        if ( session_rc == LA_SESSION_MGR_RC_ENTRY_NOT_AVAILABLE )
        {
            printf("Error: ENTRY NOT AVAILABLE\n");

            e_Response     = CMBS_RESPONSE_ERROR;
            e_RejectReason = CMBS_LA_NACK_ENTRY_NOT_AVAILABLE;
        }
        else if ( session_rc == LA_SESSION_MGR_RC_NOT_ALLOWED )
        {
            printf("Error: PROC NOT ALLOWED\n");

            e_Response     = CMBS_RESPONSE_ERROR;
            e_RejectReason = CMBS_LA_NACK_PROC_NOT_ALLOWED;
        }
        else
        {
            printf("Sending SaveEntry CFM: Session:%d, EntryId: %d, PositionIndex: %d, TotalEntriesNum: %d\n",
                   u16_SessionId, u32_EntryId, u32_PositionIdx, u32_NumOfEntries);
        }

        cmbs_dsr_la_SaveEntryRes( pv_AppRef, u16_SessionId, u32_EntryId, u32_PositionIdx, u32_NumOfEntries, e_Response, e_RejectReason );

        /* Possibly need to send List Change notification */
        LASessionMgr_SendListChangedNotification(u16_SessionId, u32_EntryId);
    }

    return TRUE;
}

int app_OnLaDataPacketSendRes(void * pv_AppRef, void * pv_EventData)
{
    ST_IE_RESPONSE          st_Response;
    void *                  pv_IE = NULL;
    u16                     u16_IE;
    u16                     u16_SessionId;
    u8*                     pu8_DataBuffer;
    u16                     *pu16_Marker, *pu16_TotalLen;

    cmbs_api_ie_GetFirst( pv_EventData, &pv_IE, &u16_IE );
    while ( pv_IE != NULL )
    {
        printf( "%s: ", getstr_E_CMBS_IE( u16_IE ) );
        switch ( u16_IE )
        {
        case CMBS_IE_LA_SESSION_ID:     /*!< Id of the List Access Session */
            cmbs_api_ie_LASessionIdGet( pv_IE, &u16_SessionId );
            printf( "%d\n", u16_SessionId );
            break;

        case CMBS_IE_RESPONSE:
            cmbs_api_ie_ResponseGet( pv_IE, &st_Response );
            printf( "%s\n", st_Response.e_Response == CMBS_RESPONSE_OK ? "Ok":"Error" );
            break;

        default:
            break;
        }
        cmbs_api_ie_GetNext( pv_EventData, &pv_IE, &u16_IE );
    }

    LASessionMgr_GetDataBuffer(u16_SessionId, &pu8_DataBuffer, &pu16_Marker, &pu16_TotalLen);

    if ( *pu16_Marker == *pu16_TotalLen )
    {
        printf("Last packet, No need to send more\n");

        *pu16_Marker = *pu16_TotalLen = 0;
    }
    else
    {
        printf("It was not last packet, need to send more\n");

        app_SendDataPackets(pv_AppRef, u16_SessionId, pu8_DataBuffer, pu16_Marker, pu16_TotalLen);
    }

    return TRUE;
}

/*		========== app_LaEntity ===========
\brief		 CMBS entity to handle response information from target side
\param[in]	 pv_AppRef		 application reference
\param[in]	 e_EventID		 received CMBS event
\param[in]	 pv_EventData	 pointer to IE list
\return	 	 <int>

*/
int app_LaEntity( void * pv_AppRef, E_CMBS_EVENT_ID e_EventID, void * pv_EventData )
{
    int res = FALSE;

    UNUSED_PARAMETER(pv_AppRef);

    switch ( e_EventID )
    {
    case CMBS_EV_DSR_LA_SESSION_START:
        res = app_OnLaSessionStart(pv_AppRef, pv_EventData);
        break;

    case CMBS_EV_DSR_LA_SESSION_END:
        res = app_OnLaSessionEnd(pv_AppRef, pv_EventData);
        break;

    case CMBS_EV_DSR_LA_SESSION_END_RES:
        res = app_OnLaSessionEndRes(pv_AppRef, pv_EventData);
        break;

    case CMBS_EV_DSR_LA_QUERY_SUPP_ENTRY_FIELDS:
        res = app_OnLaQuerySuppEntryFields(pv_AppRef, pv_EventData);
        break;

    case CMBS_EV_DSR_LA_READ_ENTRIES:
        res = app_OnLaReadEntries(pv_AppRef, pv_EventData);
        break;

    case CMBS_EV_DSR_LA_EDIT_ENTRY:
        res = app_OnLaEditEntry(pv_AppRef, pv_EventData);
        break;

    case CMBS_EV_DSR_LA_SAVE_ENTRY:
        res = app_OnLaSaveEntry(pv_AppRef, pv_EventData);
        break;

    case CMBS_EV_DSR_LA_DELETE_ENTRY:
        res = app_OnLaDeleteEntry(pv_AppRef, pv_EventData);
        break;

    case CMBS_EV_DSR_LA_DELETE_LIST:
        res = app_OnLaDeleteList(pv_AppRef, pv_EventData);
        break;

    case CMBS_EV_DSR_LA_SEARCH_ENTRIES:
        res = app_OnLaSearchEntries(pv_AppRef, pv_EventData);
        break;

    case CMBS_EV_DSR_LA_DATA_PACKET_RECEIVE:
        res = app_OnLaDataPacketReceive(pv_AppRef, pv_EventData);
        break;

    case CMBS_EV_DSR_LA_DATA_PACKET_SEND_RES:
        res = app_OnLaDataPacketSendRes(pv_AppRef, pv_EventData);
        break;

    default:
        return FALSE;

    }
    return res;
}
/************************************* [End Of File] ******************************************************************************/
