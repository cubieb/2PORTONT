/*************************************************************************************************************
*** List Access Session Manager
**
**************************************************************************************************************/
#ifndef __LA_SESSION_MGR_H__
#define __LA_SESSION_MGR_H__


/*******************************************
Includes
********************************************/
#include "cmbs_api.h"
#include "appcall.h"

/*******************************************
Defines
********************************************/
#define LA_SESSION_MGR_MAX_DATA_LEN 2000

/*******************************************
Types
********************************************/
typedef struct
{
    u32 u32_LineId;
    u32 u32_NumOfCallsBefore;
    u32 u32_NumOfCallsAfter;
}stMissedCallNotifAux;

typedef struct
{
    u32 u32_HsId;
    u32 u32_SessionId;
    u32 u32_SaveEntryId;

    /* CAT-iq numbering */
    u32 u32_CATiqListId;
    u32 u32_CATiqSortField1;
    u32 u32_CATiqSortField2;

    /* CMBS Application numbering */
    u32 u32_ListId;
    u32 u32_SortField1;
    u32 u32_SortField2;

    /* Buffer for send / receive */
    u8 pu8_DataBuffer[LA_SESSION_MGR_MAX_DATA_LEN];
    u16 u16_DataBytesMarker;
    u16 u16_DataTotalLen;

    /* Auxiliary for Missed Call Notification */
    stMissedCallNotifAux pst_MissedCallNotifAux[APPCALL_LINEOBJ_MAX];

    /* Auxiliary for deletion of entries - List change notification */
    u32 pu32_LineIdsOfDeletedEntries[APPCALL_LINEOBJ_MAX];
    u32 pu32_AttachedHsOfDeletedEntries[APPCALL_LINEOBJ_MAX];
    u32 u32_LineIdsSize;

}stLASession;

typedef enum
{
    LA_SESSION_MGR_RC_OK,
    LA_SESSION_MGR_RC_FAIL,
    LA_SESSION_MGR_RC_NO_FREE_SESSIONS,
    LA_SESSION_MGR_RC_UNSUPPORTED_LIST,
    LA_SESSION_MGR_RC_UNKNOWN_SESSION_ID,
    LA_SESSION_MGR_RC_NOT_ENOUGH_MEMORY,
    LA_SESSION_MGR_RC_NOT_ALLOWED,
    LA_SESSION_MGR_RC_INVALID_START_INDEX,
    LA_SESSION_MGR_RC_ENTRY_NOT_AVAILABLE

}LA_SESSION_MGR_RC;

/*******************************************
Globals
********************************************/

/*******************************************
Session Mgr API
********************************************/
/* Init */
LA_SESSION_MGR_RC LASessionMgr_Init(void);

/* Get Data Buffer */
LA_SESSION_MGR_RC LASessionMgr_GetDataBuffer(IN u16 u16_SessionId, OUT u8** ppu8_DataBuffer, OUT u16** ppu16_Marker, OUT u16** ppu16_Len);

/* Start session */
LA_SESSION_MGR_RC LASessionMgr_StartSession(IN u16 u16_SessionId, IN u16 u16_ListId, IN u16 u16_HsId, IN PST_IE_LA_FIELDS pst_SortFields,
                                            OUT u16* pu16_CurrNumOfEntries);

/* End Session */
LA_SESSION_MGR_RC LASessionMgr_EndSession(IN u16 u16_SessionId);

/* Get Supported Fields */
LA_SESSION_MGR_RC LASessionMgr_GetSupportedFields(IN u16 u16_SessionId,
                                                  OUT PST_IE_LA_FIELDS pst_EditableFields, OUT PST_IE_LA_FIELDS pst_NonEditableFields);

/* Read Entries */
LA_SESSION_MGR_RC LASessionMgr_ReadEntries(IN u16 u16_SessionId, IN u16 u16_StartIdx, IN bool bForward, IN E_CMBS_MARK_REQUEST eMark,
                                           IN PST_IE_LA_FIELDS pst_RequestedFields, INOUT u16* pu16_NumOfReqEntries,
                                           OUT u8 pu8_Data[], INOUT u16* pu16_DataLen);
/* Edit Entry */
LA_SESSION_MGR_RC LASessionMgr_EditEntry(IN u16 u16_SessionId, IN u32 u32_EntryId,
                                         IN PST_IE_LA_FIELDS pst_RequestedFields, OUT u8 pu8_Data[], INOUT u16* pu16_DataLen);

/* Search Entries */
LA_SESSION_MGR_RC LASessionMgr_SearchEntries(IN u16 u16_SessionId, IN E_CMBS_LA_SEARCH_MATCHING eMatch, IN bool bCaseSensitive,
                                             IN const char* s_SearchedValue, IN bool bForward, IN E_CMBS_MARK_REQUEST eMark,
                                             IN PST_IE_LA_FIELDS pst_RequestedFields, INOUT u16* pu16_NumOfReqEntries,
                                             OUT u8 pu8_Data[], INOUT u16* pu16_DataLen, OUT u32* pu32_StartIdx);

/* Save Entry */
LA_SESSION_MGR_RC LASessionMgr_SetSaveEntryID(IN u16 u16_SessionId, IN u32 u32_EntryId);
LA_SESSION_MGR_RC LASessionMgr_SaveEntry(IN u16 u16_SessionId, OUT u32* pu32_EntryId, OUT u32* pu32_PositionIdx, OUT u32* pu32_TotalNumEntries);

/* Delete Entry */
LA_SESSION_MGR_RC LASessionMgr_DeleteEntry(IN u16 u16_SessionId, IN u16 u16_EntryId, OUT u16* pu16_NumOfEntries);

/* Delete All Entries */
LA_SESSION_MGR_RC LASessionMgr_DeleteAllEntries(IN u16 u16_SessionId);

/* Send Missed call notification if needed */
LA_SESSION_MGR_RC LASessionMgr_SendMissedCallNotification(IN u16 u16_SessionId, IN E_CMBS_MARK_REQUEST eMark);

/* Send list changed notification if needed */
LA_SESSION_MGR_RC LASessionMgr_SendListChangedNotification(IN u16 u16_SessionId, IN u16 u16_EntryId);
LA_SESSION_MGR_RC LASessionMgr_SendListChangedNotificationOnDelete(IN u16 u16_SessionId);

#endif /* __LA_SESSION_MGR_H__ */

/* End Of File *****************************************************************************************************************************/
