/*************************************************************************************************************
*** ListsApp
*** An implementation of lists API over SQL database
**
**************************************************************************************************************/
#ifndef __LISTS_APP_H__
#define __LISTS_APP_H__


/*******************************************
Includes
********************************************/
#include "cmbs_api.h"
#include "SQLiteWrapper.h"

/*******************************************
Defines
********************************************/
#define LIST_NAME_MAX_LEN       30
#define LIST_NUMBER_MAX_LEN     30


/*******************************************
Types
********************************************/
typedef enum
{
    MATCH_EXACT,
    MATCH_RETURN_NEXT_ON_FAIL,
    MATCH_RETURN_PREV_ON_FAIL
}eMatchOption;

typedef enum
{
    LIST_RC_OK = 0,
    LIST_RC_FAIL,
    LIST_RC_UNSUPPORTED_LIST,
    LIST_RC_ARRAY_TOO_SMALL,
    LIST_RC_UNKNOWN_FIELD,
    LIST_RC_NO_SORT,
    LIST_RC_UNSUPPORTED_MATCH_OPTION,
    LIST_RC_INVALID_START_INDEX,
    LIST_RC_ENTRY_NOT_AVAILABLE

}LIST_RC;

typedef enum
{
    LIST_TYPE_CONTACT_LIST,
    LIST_TYPE_LINE_SETTINGS_LIST,
    LIST_TYPE_MISSED_CALLS,       
    LIST_TYPE_OUTGOING_CALLS,     
    LIST_TYPE_INCOMING_ACCEPTED_CALLS,       
    LIST_TYPE_ALL_CALLS,          
    LIST_TYPE_ALL_INCOMING_CALLS,

    LIST_TYPE_MAX

}LIST_TYPE;

typedef enum
{
    FIELD_TYPE_CHAR,
    FIELD_TYPE_INT,
    FIELD_TYPE_TEXT,

    FIELD_TYPE_MAX

} FIELD_TYPE;

typedef enum
{
    FIELD_ID_INVALID = -1,
    FIELD_ID_ENTRY_ID,
    FIELD_ID_LAST_NAME,
    FIELD_ID_FIRST_NAME,
    FIELD_ID_CONTACT_NUM_1,
    FIELD_ID_CONTACT_NUM_1_TYPE,
    FIELD_ID_CONTACT_NUM_1_OWN,
    FIELD_ID_CONTACT_NUM_1_DEFAULT,
    FIELD_ID_CONTACT_NUM_2,
    FIELD_ID_CONTACT_NUM_2_TYPE,
    FIELD_ID_CONTACT_NUM_2_OWN,
    FIELD_ID_CONTACT_NUM_2_DEFAULT,
    FIELD_ID_ASSOCIATED_MELODY,
    FIELD_ID_LINE_ID,
    FIELD_ID_NUMBER,
    FIELD_ID_DATE_AND_TIME,
    FIELD_ID_READ_STATUS,
    FIELD_ID_LINE_NAME,
    FIELD_ID_NUM_OF_CALLS,
    FIELD_ID_CALL_TYPE,
    FIELD_ID_ATTACHED_HANDSETS,
    FIELD_ID_DIALING_PREFIX,
    FIELD_ID_FP_MELODY,
    FIELD_ID_FP_VOLUME,
    FIELD_ID_BLOCKED_NUMBER,
    FIELD_ID_MULTIPLE_CALLS_MODE,
    FIELD_ID_INTRUSION_CALL,
    FIELD_ID_PERMANENT_CLIR,
    FIELD_ID_PERMANENT_CLIR_ACTIVATION_CODE,
    FIELD_ID_PERMANENT_CLIR_DEACTIVATION_CODE,
    FIELD_ID_CALL_FWD_UNCOND,
    FIELD_ID_CALL_FWD_UNCOND_ACTIVATION_CODE,
    FIELD_ID_CALL_FWD_UNCOND_DEACTIVATION_CODE,
    FIELD_ID_CALL_FWD_UNCOND_TARGET_NUMBER,
    FIELD_ID_CALL_FWD_NO_ANSWER,
    FIELD_ID_CALL_FWD_NO_ANSWER_ACTIVATION_CODE,
    FIELD_ID_CALL_FWD_NO_ANSWER_DEACTIVATION_CODE,
    FIELD_ID_CALL_FWD_NO_ANSWER_TARGET_NUMBER,
    FIELD_ID_CALL_FWD_NO_ANSWER_TIMEOUT,
    FIELD_ID_CALL_FWD_BUSY,
    FIELD_ID_CALL_FWD_BUSY_ACTIVATION_CODE,
    FIELD_ID_CALL_FWD_BUSY_DEACTIVATION_CODE,
    FIELD_ID_CALL_FWD_BUSY_TARGET_NUMBER,

    FIELD_ID_MAX

} FIELD_ID;

typedef struct
{
    const char* s_Name;
    FIELD_TYPE  e_Type;
    bool        b_AutoInc;
    bool        b_PrimaryKey;
    bool        b_Mandatory;
    bool        b_Editable;
    FIELD_ID    e_FieldId;
}stListField;

typedef enum 
{
    NUM_TYPE_FIXED,
    NUM_TYPE_MOBILE,
    NUM_TYPE_WORK
}eNumberType;

typedef struct
{
    u32     u32_EntryId;
    char    sLastName[LIST_NAME_MAX_LEN];
    char    sFirstName[LIST_NAME_MAX_LEN];
    char    sNumber1[LIST_NUMBER_MAX_LEN];
    char    cNumber1Type;
    bool    bNumber1Default;
    bool    bNumber1Own;
    char    sNumber2[LIST_NUMBER_MAX_LEN];
    char    cNumber2Type;
    bool    bNumber2Default;
    bool    bNumber2Own;
    u32     u32_AssociatedMelody;
    u32     u32_LineId;

}stContactListEntry;


typedef enum
{
    CALL_TYPE_MISSED,
    CALL_TYPE_OUTGOING,
    CALL_TYPE_INCOMING

}eCallType;

typedef struct
{
    u32     u32_EntryId;
    char    sNumber[LIST_NUMBER_MAX_LEN];
    time_t  t_DateAndTime;
    bool    bRead;
    char    sLineName[LIST_NAME_MAX_LEN];   /* Stored in LineSettingsList */
    u32     u32_LineId;
    u32     u32_NumOfCalls;
    char    cCallType;
    char    sLastName[LIST_NAME_MAX_LEN];   /* stored in ContactList */
    char    sFirstName[LIST_NAME_MAX_LEN];  /* stored in ContactList */

}stCallsListEntry;

typedef struct
{
    u32     u32_EntryId;
    char    sLineName[LIST_NAME_MAX_LEN];
    u32     u32_LineId;
    u32     u32_AttachedHsMask;
    char    sDialPrefix[LIST_NUMBER_MAX_LEN];
    u32     u32_FPMelody;
    u32     u32_FPVolume;
    char    sBlockedNumber[LIST_NUMBER_MAX_LEN];
    bool    bMultiCalls;
    bool    bIntrusionCall;
    bool    bPermanentCLIR;
    char    sPermanentCLIRActCode[LIST_NUMBER_MAX_LEN];
    char    sPermanentCLIRDeactCode[LIST_NUMBER_MAX_LEN];
    bool    bCallFwdUncond;
    char    sCallFwdUncondActCode[LIST_NUMBER_MAX_LEN];
    char    sCallFwdUncondDeactCode[LIST_NUMBER_MAX_LEN];
    char    sCallFwdUncondNum[LIST_NUMBER_MAX_LEN];
    bool    bCallFwdNoAns;
    char    sCallFwdNoAnsActCode[LIST_NUMBER_MAX_LEN];
    char    sCallFwdNoAnsDeactCode[LIST_NUMBER_MAX_LEN];
    char    sCallFwdNoAnsNum[LIST_NUMBER_MAX_LEN];
    u32     u32_CallFwdNoAnsTimeout;
    bool    bCallFwdBusy;
    char    sCallFwdBusyActCode[LIST_NUMBER_MAX_LEN];
    char    sCallFwdBusyDeactCode[LIST_NUMBER_MAX_LEN];
    char    sCallFwdBusyNum[LIST_NUMBER_MAX_LEN];

}stLineSettingsListEntry;

typedef enum
{
    MARK_LEAVE_UNCHANGED,
    MARK_CLEAR,
    MARK_SET
} eMarkRequest;

/* Entry Max Size */
typedef union
{
    stContactListEntry          ContactListEntry;
    stCallsListEntry            CallsListEntry;
    stLineSettingsListEntry     LineSettingsListEntry;
}uEntryTypes;

#define LIST_ENTRY_MAX_SIZE sizeof(uEntryTypes)

/*******************************************
Globals
********************************************/


/*******************************************
List API
********************************************/
/* Initialize Lists Application */
LIST_RC List_Init(void);

/* Create List in database */
LIST_RC List_CreateList(IN LIST_TYPE ListType);

/* Get number of entries */
LIST_RC List_GetCount(IN LIST_TYPE ListType, OUT u32* pu32_Count);

/* Get Supported fields */
LIST_RC List_GetSupportedFields(IN LIST_TYPE ListType,
                                OUT u32 pu32_EditableFieldsIds[],  INOUT u32* pu32_EditableSize,       /* Editable Fields */
                                OUT u32 pu32_NonEditableFields[],  INOUT u32* pu32_NonEditableSize     /* Non-Editable Fields */);

/* Insert *NEW* Entry
 
   *NOTE: For calls list, the data base automatically sets the 'call type' field, according to list type*
 
   Input Arguments:
   ------------------
   ListType         - List to insert entry
   pv_Entry         - pointer to actual entry
   pu32Fields       - Fields used in provided entry (array of Ids)
   u32_FieldsSize   - size of array
 
   Output Arguments:
   ------------------
   pu32_EntryId     - ID of inserted entry
*/
LIST_RC List_InsertEntry(IN LIST_TYPE ListType, IN void* pv_Entry, IN u32 pu32Fields[], IN u32 u32_FieldsSize, OUT u32* pu32_EntryId);

/* Delete a single entry from the database */
LIST_RC List_DeleteEntry(IN LIST_TYPE ListType, IN u32 u32_EntryId);

/* Delete all entries from the database */
LIST_RC List_DeleteAllEntries(IN LIST_TYPE ListType);

/* Update (existing) Entry */
LIST_RC List_UpdateEntry(IN LIST_TYPE ListType, IN void* pv_Entry, IN u32 pu32Fields[], IN u32 u32_FieldsSize, IN u32 u32_EntryId);

/* Read entries
 
   Input Arguments:
   ------------------------
   ListType             - requested list
   u32_StartIndex       - start index in respect to sorting fields  
   bForward             - Direction of read
   eMark                - Flag for requesting resetting (or setting) of the 'Read status' field for all read entries
   pu32Fields           - Array of requested fields
   u32_FieldsSize       - size of pu32Fields
   u32_SortField1       - most significant sort field
   u32_SortField2       - least significant sort field (use invalid_id if not needed)
   u32_EntriesSize      - number of requested entries
 
   Output Arguments:
   ------------------------
    pv_Entries          - array of read entries
    pu32_EntriesSize    - size of pv_Entries
*/
LIST_RC List_ReadEntries(IN LIST_TYPE ListType, IN u32 u32_StartIndex, IN bool bForward, IN eMarkRequest eMark, IN u32 pu32Fields[], IN u32 u32_FieldsSize,
                         IN u32 u32_SortField1, IN u32 u32_SortField2, OUT void* pv_Entries, INOUT u32* pu32_EntriesSize);

/* Read entry by ID
 
   Input Arguments:
   ------------------------
   ListType             - requested list
   u32_EntryId          - Id Of requested entry
   pu32Fields           - Array of requested fields
   u32_FieldsSize       - size of pu32Fields
 
   Output Arguments:
   ------------------------
    pv_Entry          - pointer to read entry
*/
LIST_RC List_ReadEntryById(IN LIST_TYPE ListType, IN u32 u32_EntryId, IN u32 pu32Fields[], IN u32 u32_FieldsSize, OUT void* pv_Entry);

/* Search entries
 
   Input Arguments:
   ------------------------
   ListType             - requested list
   eMatchOption         - what to return in case of no exact match (return nothing, return first greater than, return first lesser than)
   bCaseSensitive       - case sensitivity of the search
   sSearchedVal         - the value to search for
   bForward             - Direction of read
   eMark                - Flag for requesting resetting (or setting) of the 'Read status' field for all read entries
   pu32Fields           - Array of requested fields
   u32_FieldsSize       - size of pu32Fields
   u32_SortField1       - most significant sort field
   u32_SortField2       - least significant sort field (use invalid_id if not needed)
   u32_EntriesSize      - number of requested entries
 
   Output Arguments:
   ------------------------
    pv_Entries          - array of read entries
    pu32_EntriesSize    - size of pv_Entries
    pu32_StartIndex     - index of first returned entry
*/
LIST_RC List_SearchEntries(IN LIST_TYPE ListType, IN eMatchOption eMatching, IN bool bCaseSensitive, IN const char* sSearchedVal, 
                           IN bool bForward, IN eMarkRequest eMark, IN u32 pu32Fields[], IN u32 u32_FieldsSize,
                           IN u32 u32_SortField1, IN u32 u32_SortField2, OUT void* pv_Entries, INOUT u32* pu32_EntriesSize,
                           OUT u32* pu32_StartIndex); 


/* Get Entry Index */ 
LIST_RC List_GetEntryIdex(IN LIST_TYPE ListType, IN u32 u32_SortField1, IN u32 u32_SortField2, IN u32 u32_EntryId, OUT u32* pu32_Index);

/*******************************************
Auxiliary
********************************************/
/* prepare strings of fields and values from the given Entry */
LIST_RC List_PrepareContactListEntryForInsert(IN stContactListEntry* pst_Entry, IN u32 pu32Fields[], IN u32 u32_FieldsSize,
                                              OUT char sFields[], INOUT u32* pu32_FieldsSize,
                                              OUT char sValues[], INOUT u32* pu32_ValuesSize);
LIST_RC List_PrepareCallsListEntryForInsert(IN LIST_TYPE ListType, IN stCallsListEntry* pst_Entry, IN u32 pu32Fields[], IN u32 u32_FieldsSize,
                                            OUT char sFields[], INOUT u32* pu32_FieldsSize,
                                            OUT char sValues[], INOUT u32* pu32_ValuesSize);
LIST_RC List_PrepareLineSettingsListEntryForInsert(IN stLineSettingsListEntry* pst_Entry, IN u32 pu32Fields[], IN u32 u32_FieldsSize,
                                                   OUT char sFields[], INOUT u32* pu32_FieldsSize,
                                                   OUT char sValues[], INOUT u32* pu32_ValuesSize);

LIST_RC List_PrepareContactListEntryForUpdate(IN stContactListEntry* pst_Entry, IN u32 pu32Fields[], IN u32 u32_FieldsSize,
                                              OUT char sFields[], INOUT u32* pu32_FieldsSize);
LIST_RC List_PrepareCallsListEntryForUpdate(IN stCallsListEntry* pst_Entry, IN u32 pu32Fields[], IN u32 u32_FieldsSize,
                                            OUT char sFields[], INOUT u32* pu32_FieldsSize);
LIST_RC List_PrepareLineSettingsListEntryForUpdate(IN stLineSettingsListEntry* pst_Entry, IN u32 pu32Fields[], IN u32 u32_FieldsSize,
                                                   OUT char sFields[], INOUT u32* pu32_FieldsSize);

/* prepare string with fields names from the given field Ids */
LIST_RC List_PrepareFieldsNamesForRead(IN LIST_TYPE ListType, IN u32 pu32Fields[], IN u32 u32_FieldsSize,
                                       OUT char sFields[], INOUT u32* pu32_FieldsSize,
                                       OUT bool* pb_JoinContactList, OUT bool* pb_JoinLineSettingsList);

/* Fill Entry */
LIST_RC List_FillEntry(IN LIST_TYPE ListType, IN u32 pu32Fields[], IN u32 u32_FieldsSize, OUT void* pv_Entries, IN u32 u32_EntriesOffset,
                       IN void* pv_SelectHandle, IN PST_COLUMN_MAP pst_Map);
LIST_RC List_FillContactListEntry(IN u32 pu32Fields[], IN u32 u32_FieldsSize, OUT void* pv_Entries, IN u32 u32_EntriesOffset,
                                  IN void* pv_SelectHandle, IN PST_COLUMN_MAP pst_Map);
LIST_RC List_FillCallsListEntry(IN u32 pu32Fields[], IN u32 u32_FieldsSize, OUT void* pv_Entries, IN u32 u32_EntriesOffset,
                                IN void* pv_SelectHandle, IN PST_COLUMN_MAP pst_Map);
LIST_RC List_FillLineSettingsEntry(IN u32 pu32Fields[], IN u32 u32_FieldsSize, OUT void* pv_Entries, IN u32 u32_EntriesOffset,
                                   IN void* pv_SelectHandle, IN PST_COLUMN_MAP pst_Map);

/* Attached Handsets */
LIST_RC List_GetAttachedHs(IN u32 u32_LineId, OUT u32* pu32_AttachedHsMask);

/* Missed Calls Num Of Read / Unread Entries */
LIST_RC List_GetMissedCallsNumOfEntries(IN u32 u32_LineId, OUT u32* pu32_NumOfUnread, OUT u32* pu32_NumOfRead);




#endif /* __LISTS_APP_H__ */

/* End Of File *****************************************************************************************************************************/


