/*************************************************************************************************************
*** List Access Session Manager
**
**************************************************************************************************************/

/*******************************************
Includes
********************************************/
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "LASessionMgr.h"
#include "ListsApp.h"
#include "appla2.h"
#include "ListChangeNotif.h"

/*******************************************
Defines
********************************************/
#define LA_SESSION_MGR_MAX_SESSIONS 5

#define LA_SESSION_MGR_MAX_FIELDS   32

#define LA_SESSION_MGR_MAX_ENTRIES_PER_READ  10

#define LA_SESSION_UNDEFINED_SESSION_ID 0xFFFF

#define CHECK_SESSION_ID(u16_SessionId)             \
    s32_Index = GetIndexBySessionId(u16_SessionId); \
                                                    \
    if (s32_Index == -1)                            \
    {                                               \
        return LA_SESSION_MGR_RC_UNKNOWN_SESSION_ID;\
    }

/*******************************************
Types
********************************************/

/*******************************************
Globals
********************************************/
/* Sessions */
static stLASession g_pst_LASessions[LA_SESSION_MGR_MAX_SESSIONS];

/*******************************************
Auxiliary
********************************************/
static s32 GetIndexBySessionId(IN u32 u32_SessionId)
{
    u32 u32_Index;
    for ( u32_Index = 0; u32_Index < LA_SESSION_MGR_MAX_SESSIONS; ++u32_Index )
    {
        if ( g_pst_LASessions[u32_Index].u32_SessionId == u32_SessionId )
        {
            return u32_Index;
        }
    }

    /* not found */
    return -1;
}

static s32 CATiqListId_2_AppListId(IN u32 u32_CATiqListId)
{
    switch ( u32_CATiqListId )
    {
    case  CMBS_LA_LIST_MISSED_CALLS:      
        return LIST_TYPE_MISSED_CALLS;

    case  CMBS_LA_LIST_OUTGOING_CALLS:     
        return LIST_TYPE_OUTGOING_CALLS;

    case  CMBS_LA_LIST_INCOMING_ACC:       
        return LIST_TYPE_INCOMING_ACCEPTED_CALLS;

    case  CMBS_LA_LIST_ALL_CALLS:          
        return LIST_TYPE_ALL_CALLS;

    case  CMBS_LA_LIST_CONTACT:            
        return LIST_TYPE_CONTACT_LIST;

    case  CMBS_LA_LIST_LINE_SETTINGS:      
        return LIST_TYPE_LINE_SETTINGS_LIST;

    case  CMBS_LA_LIST_ALL_INCOMING_CALLS: 
        return LIST_TYPE_ALL_INCOMING_CALLS;

    default:
        return -1;
    }
}

static s32 AppListId_2_CATiqListId(IN u32 u32_AppListId)
{
    switch ( u32_AppListId )
    {
    case  LIST_TYPE_MISSED_CALLS:      
        return CMBS_LA_LIST_MISSED_CALLS;

    case  LIST_TYPE_OUTGOING_CALLS:     
        return CMBS_LA_LIST_OUTGOING_CALLS;

    case  LIST_TYPE_INCOMING_ACCEPTED_CALLS:       
        return CMBS_LA_LIST_INCOMING_ACC;

    case  LIST_TYPE_ALL_CALLS:          
        return CMBS_LA_LIST_ALL_CALLS;

    case  LIST_TYPE_CONTACT_LIST:            
        return CMBS_LA_LIST_CONTACT;

    case  LIST_TYPE_LINE_SETTINGS_LIST:      
        return CMBS_LA_LIST_LINE_SETTINGS;

    case  LIST_TYPE_ALL_INCOMING_CALLS: 
        return CMBS_LA_LIST_ALL_INCOMING_CALLS;

    default:
        return -1;
    }
}

static s32 CATiqFieldId_2_AppFieldId(IN u32 u32_ListId, IN u32 u32_FieldId)
{
    u32_ListId = (u32)AppListId_2_CATiqListId(u32_ListId);
    switch ( u32_ListId )
    {
    case  CMBS_LA_LIST_MISSED_CALLS:  
        switch ( u32_FieldId )
        {
        case CMBS_MCL_FLD_NUMBER:           return FIELD_ID_NUMBER;
        case CMBS_MCL_FLD_NAME:             return FIELD_ID_LAST_NAME;
        case CMBS_MCL_FLD_DATETIME:         return FIELD_ID_DATE_AND_TIME;
        case CMBS_MCL_FLD_NEW:              return FIELD_ID_READ_STATUS;
        case CMBS_MCL_FLD_LINE_NAME:        return FIELD_ID_LINE_NAME; 
        case CMBS_MCL_FLD_LINE_ID:          return FIELD_ID_LINE_ID;
        case CMBS_MCL_FLD_NR_OF_CALLS:      return FIELD_ID_NUM_OF_CALLS;
        default:                            return FIELD_ID_INVALID;
        }
        break;

    case  CMBS_LA_LIST_OUTGOING_CALLS:     
        switch ( u32_FieldId )
        {
        case CMBS_OCL_FLD_NUMBER:           return FIELD_ID_NUMBER;
        case CMBS_OCL_FLD_NAME:             return FIELD_ID_LAST_NAME;
        case CMBS_OCL_FLD_DATETIME:         return FIELD_ID_DATE_AND_TIME;
        case CMBS_OCL_FLD_LINE_NAME:        return FIELD_ID_LINE_NAME;
        case CMBS_OCL_FLD_LINE_ID:          return FIELD_ID_LINE_ID; 
        default:                            return FIELD_ID_INVALID;
        }
        break;

    case  CMBS_LA_LIST_INCOMING_ACC: 
        switch ( u32_FieldId )
        {
        case CMBS_IACL_FLD_NUMBER:           return FIELD_ID_NUMBER;
        case CMBS_IACL_FLD_NAME:             return FIELD_ID_LAST_NAME;
        case CMBS_IACL_FLD_DATETIME:         return FIELD_ID_DATE_AND_TIME;
        case CMBS_IACL_FLD_LINE_NAME:        return FIELD_ID_LINE_NAME;
        case CMBS_IACL_FLD_LINE_ID:          return FIELD_ID_LINE_ID; 
        default:                             return FIELD_ID_INVALID;
        }      
        break;

    case  CMBS_LA_LIST_ALL_CALLS: 
        switch ( u32_FieldId )
        {
        case CMBS_ACL_FLD_CALL_TYPE:    return FIELD_ID_CALL_TYPE;
        case CMBS_ACL_FLD_NUMBER:       return FIELD_ID_NUMBER;
        case CMBS_ACL_FLD_NAME:         return FIELD_ID_LAST_NAME;
        case CMBS_ACL_FLD_DATETIME:     return FIELD_ID_DATE_AND_TIME;
        case CMBS_ACL_FLD_LINE_NAME:    return FIELD_ID_LINE_NAME; 
        case CMBS_ACL_FLD_LINE_ID:      return FIELD_ID_LINE_ID;
        default:                        return FIELD_ID_INVALID;
        }      
        break;

    case  CMBS_LA_LIST_CONTACT: 
        switch ( u32_FieldId )
        {
        case CMBS_CL_FLD_NAME:          return FIELD_ID_LAST_NAME;
        case CMBS_CL_FLD_FIRST_NAME:    return FIELD_ID_FIRST_NAME;
        case CMBS_CL_FLD_CONTACT_NR:    return FIELD_ID_CONTACT_NUM_1;
        case CMBS_CL_FLD_ASSOC_MDY:     return FIELD_ID_ASSOCIATED_MELODY;
        case CMBS_CL_FLD_LINE_ID:       return FIELD_ID_LINE_ID; 
        default:                        return FIELD_ID_INVALID;
        }
        break;

    case  CMBS_LA_LIST_LINE_SETTINGS:
        switch ( u32_FieldId )
        {
        case CMBS_LSL_FLD_LINE_NAME:            return FIELD_ID_LINE_NAME;
        case CMBS_LSL_FLD_LINE_ID:              return FIELD_ID_LINE_ID;
        case CMBS_LSL_FLD_ATTACHED_HANDSETS:    return FIELD_ID_ATTACHED_HANDSETS;
        case CMBS_LSL_FLD_DIALING_PREFIX:       return FIELD_ID_DIALING_PREFIX;
        case CMBS_LSL_FLD_FP_MELODY:            return FIELD_ID_FP_MELODY; 
        case CMBS_LSL_FLD_FP_VOLUME:            return FIELD_ID_FP_VOLUME;
        case CMBS_LSL_FLD_BLOCKED_NUMBER:       return FIELD_ID_BLOCKED_NUMBER;
        case CMBS_LSL_FLD_MULTI_CALL_MODE:      return FIELD_ID_MULTIPLE_CALLS_MODE;
        case CMBS_LSL_FLD_INTRUSION_CALL:       return FIELD_ID_INTRUSION_CALL;
        case CMBS_LSL_FLD_PERMANENT_CLIR:       return FIELD_ID_PERMANENT_CLIR; 
        case CMBS_LSL_FLD_CALL_FORWARDING:      return FIELD_ID_CALL_FWD_UNCOND;
        case CMBS_LSL_FLD_CFNA:                 return FIELD_ID_CALL_FWD_NO_ANSWER;
        case CMBS_LSL_FLD_CFB:                  return FIELD_ID_CALL_FWD_BUSY;
        default:                                return FIELD_ID_INVALID;
        }
        break;

    case  CMBS_LA_LIST_ALL_INCOMING_CALLS: 
        switch ( u32_FieldId )
        {
        case CMBS_AICL_FLD_NUMBER:           return FIELD_ID_NUMBER;
        case CMBS_AICL_FLD_NAME:             return FIELD_ID_LAST_NAME;
        case CMBS_AICL_FLD_DATETIME:         return FIELD_ID_DATE_AND_TIME;
        case CMBS_AICL_FLD_NEW:              return FIELD_ID_READ_STATUS;
        case CMBS_AICL_FLD_LINE_NAME:        return FIELD_ID_LINE_NAME; 
        case CMBS_AICL_FLD_LINE_ID:          return FIELD_ID_LINE_ID;
        case CMBS_AICL_FLD_NR_OF_CALLS:      return FIELD_ID_NUM_OF_CALLS;
        default:                             return FIELD_ID_INVALID;
        }
        break;

    default:
        return FIELD_ID_INVALID;
    }
}

static s32 AppFieldId_2_CATiqFieldId(IN u32 u32_ListId, IN u32 u32_FieldId)
{
    u32_ListId = AppListId_2_CATiqListId(u32_ListId);

    switch ( u32_ListId )
    {
    case  CMBS_LA_LIST_MISSED_CALLS:  
        switch ( u32_FieldId )
        {
        case FIELD_ID_NUMBER:           return  CMBS_MCL_FLD_NUMBER;
        case FIELD_ID_LAST_NAME:        return  CMBS_MCL_FLD_NAME;     
        case FIELD_ID_DATE_AND_TIME:    return  CMBS_MCL_FLD_DATETIME;
        case FIELD_ID_READ_STATUS:      return  CMBS_MCL_FLD_NEW;   
        case FIELD_ID_LINE_NAME:        return  CMBS_MCL_FLD_LINE_NAME;
        case FIELD_ID_LINE_ID:          return  CMBS_MCL_FLD_LINE_ID;
        case FIELD_ID_NUM_OF_CALLS:     return  CMBS_MCL_FLD_NR_OF_CALLS;
        default:                        return  CMBS_FLD_UNDEFINED;
        }
        break;

    case  CMBS_LA_LIST_OUTGOING_CALLS:     
        switch ( u32_FieldId )
        {
        case FIELD_ID_NUMBER:               return CMBS_OCL_FLD_NUMBER;   
        case FIELD_ID_LAST_NAME:            return CMBS_OCL_FLD_NAME;     
        case FIELD_ID_DATE_AND_TIME:        return CMBS_OCL_FLD_DATETIME; 
        case FIELD_ID_LINE_NAME:            return CMBS_OCL_FLD_LINE_NAME;
        case FIELD_ID_LINE_ID:              return CMBS_OCL_FLD_LINE_ID;  
        default:                            return CMBS_FLD_UNDEFINED;
        }
        break;

    case  CMBS_LA_LIST_INCOMING_ACC: 
        switch ( u32_FieldId )
        {
        case FIELD_ID_NUMBER:                return CMBS_IACL_FLD_NUMBER;   
        case FIELD_ID_LAST_NAME:             return CMBS_IACL_FLD_NAME;     
        case FIELD_ID_DATE_AND_TIME:         return CMBS_IACL_FLD_DATETIME; 
        case FIELD_ID_LINE_NAME:             return CMBS_IACL_FLD_LINE_NAME;
        case FIELD_ID_LINE_ID:               return CMBS_IACL_FLD_LINE_ID;  
        default:                             return CMBS_FLD_UNDEFINED;
        }      
        break;

    case  CMBS_LA_LIST_ALL_CALLS: 
        switch ( u32_FieldId )
        {
        case FIELD_ID_CALL_TYPE:        return CMBS_ACL_FLD_CALL_TYPE;    
        case FIELD_ID_NUMBER:           return CMBS_ACL_FLD_NUMBER;       
        case FIELD_ID_LAST_NAME:        return CMBS_ACL_FLD_NAME;         
        case FIELD_ID_DATE_AND_TIME:    return CMBS_ACL_FLD_DATETIME;     
        case FIELD_ID_LINE_NAME:        return CMBS_ACL_FLD_LINE_NAME;    
        case FIELD_ID_LINE_ID:          return CMBS_ACL_FLD_LINE_ID;      
        default:                        return CMBS_FLD_UNDEFINED;
        }      
        break;

    case  CMBS_LA_LIST_CONTACT: 
        switch ( u32_FieldId )
        {
        case FIELD_ID_LAST_NAME:         return CMBS_CL_FLD_NAME;          
        case FIELD_ID_FIRST_NAME:        return CMBS_CL_FLD_FIRST_NAME;    
        case FIELD_ID_CONTACT_NUM_1:     return CMBS_CL_FLD_CONTACT_NR;    
        case FIELD_ID_CONTACT_NUM_2:     return CMBS_CL_FLD_CONTACT_NR;    
        case FIELD_ID_ASSOCIATED_MELODY: return CMBS_CL_FLD_ASSOC_MDY;     
        case FIELD_ID_LINE_ID:           return CMBS_CL_FLD_LINE_ID;       
        default:                         return CMBS_FLD_UNDEFINED;
        }
        break;

    case  CMBS_LA_LIST_LINE_SETTINGS:
        switch ( u32_FieldId )
        {
        case FIELD_ID_LINE_NAME:                return CMBS_LSL_FLD_LINE_NAME;            
        case FIELD_ID_LINE_ID:                  return CMBS_LSL_FLD_LINE_ID;              
        case FIELD_ID_ATTACHED_HANDSETS:        return CMBS_LSL_FLD_ATTACHED_HANDSETS;    
        case FIELD_ID_DIALING_PREFIX:           return CMBS_LSL_FLD_DIALING_PREFIX;       
        case FIELD_ID_FP_MELODY:                return CMBS_LSL_FLD_FP_MELODY;            
        case FIELD_ID_FP_VOLUME:                return CMBS_LSL_FLD_FP_VOLUME;            
        case FIELD_ID_BLOCKED_NUMBER:           return CMBS_LSL_FLD_BLOCKED_NUMBER;       
        case FIELD_ID_MULTIPLE_CALLS_MODE:      return CMBS_LSL_FLD_MULTI_CALL_MODE;      
        case FIELD_ID_INTRUSION_CALL:           return CMBS_LSL_FLD_INTRUSION_CALL;       
        case FIELD_ID_PERMANENT_CLIR:           return CMBS_LSL_FLD_PERMANENT_CLIR;       
        case FIELD_ID_CALL_FWD_UNCOND:          return CMBS_LSL_FLD_CALL_FORWARDING;      
        case FIELD_ID_CALL_FWD_NO_ANSWER:       return CMBS_LSL_FLD_CFNA;                 
        case FIELD_ID_CALL_FWD_BUSY:            return CMBS_LSL_FLD_CFB;                  
        default:                                return CMBS_FLD_UNDEFINED;
        }
        break;

    case  CMBS_LA_LIST_ALL_INCOMING_CALLS: 
        switch ( u32_FieldId )
        {
        case FIELD_ID_NUMBER:                return CMBS_AICL_FLD_NUMBER;     
        case FIELD_ID_LAST_NAME:             return CMBS_AICL_FLD_NAME;       
        case FIELD_ID_DATE_AND_TIME:         return CMBS_AICL_FLD_DATETIME;   
        case FIELD_ID_READ_STATUS:           return CMBS_AICL_FLD_NEW;        
        case FIELD_ID_LINE_NAME:             return CMBS_AICL_FLD_LINE_NAME;  
        case FIELD_ID_LINE_ID:               return CMBS_AICL_FLD_LINE_ID;    
        case FIELD_ID_NUM_OF_CALLS:          return CMBS_AICL_FLD_NR_OF_CALLS;
        default:                             return CMBS_FLD_UNDEFINED;
        }
        break;

    default:
        return CMBS_FLD_UNDEFINED;
    }
}

static void ConvertTimeToCatiqTime(INOUT ST_UTIL_LA_FLD_DATETIME* pst_DateTime)
{
    ++pst_DateTime->u8_Month;       /* Convert from [0..11] to [1..12] */
    pst_DateTime->u8_Year -= 100;   /* 2-digit in the 21st century */
}

static void PrepareFieldsForRead(IN u32 u32_ListId, IN PST_IE_LA_FIELDS pst_CatiqFields, OUT u32 pu32_Fields[], OUT u32* pu32_FieldsNum)
{
    bool bContactListNumber = FALSE;
    u32 u32_i;

    /* Always read Entry ID */
    *pu32_FieldsNum = 0;
    pu32_Fields[(*pu32_FieldsNum)++] = FIELD_ID_ENTRY_ID;

    for ( u32_i = 0; u32_i < pst_CatiqFields->u16_Length; ++u32_i )
    {
        pu32_Fields[(*pu32_FieldsNum)++] = CATiqFieldId_2_AppFieldId(u32_ListId, pst_CatiqFields->pu16_FieldId[u32_i]);

        /* in case of contact list, if field number is requested twice, we should add num2 field */
        if ( (u32_ListId == LIST_TYPE_CONTACT_LIST) && (pst_CatiqFields->pu16_FieldId[u32_i] == CMBS_CL_FLD_CONTACT_NR) )
        {
            if ( bContactListNumber )
            {
                /* this is second field */
                pu32_Fields[*pu32_FieldsNum - 1] = FIELD_ID_CONTACT_NUM_2;
            }

            bContactListNumber = TRUE;
        }
    }
}

static void PrepareMissedCallEntry(IN stCallsListEntry* pst_AppEntry, IN PST_IE_LA_FIELDS pst_Fields,
                                   IN u8 pu8_Data[], OUT u16* pu16_EntryLen)
{
    u16 u16_index = 0, u16_FieldIdx;
    u8 pu8_Buffer[LIST_ENTRY_MAX_SIZE];
    u8* pu8_Entry;

    memset(pu8_Buffer, 0, LIST_ENTRY_MAX_SIZE);

    /* reserve max 4 bytes for EntryID and Length (each can consume up to 2 bytes), they will be handled at the end of the function */
    u16_index += 4;

    for ( u16_FieldIdx = 0; u16_FieldIdx < pst_Fields->u16_Length; ++u16_FieldIdx )
    {
        switch ( pst_Fields->pu16_FieldId[u16_FieldIdx] )
        {
        case CMBS_MCL_FLD_NUMBER:
            {
                ST_UTIL_LA_FLD_NUMBER st_LA_Fld_Number;
                st_LA_Fld_Number.u8_Attribute   = 0;
                st_LA_Fld_Number.u8_FieldId     = CMBS_MCL_FLD_NUMBER;
                st_LA_Fld_Number.u16_DataLen    = (u16) strlen((char*)pst_AppEntry->sNumber);
                st_LA_Fld_Number.pu8_Data       = (void*)pst_AppEntry->sNumber;
                cmbs_util_LA_Fld_NumberAdd(pu8_Buffer, &st_LA_Fld_Number, &u16_index);
            }
            break;

        case CMBS_MCL_FLD_NAME:
            {
                ST_UTIL_LA_FLD_NAME st_LA_Fld_Name;
                st_LA_Fld_Name.u8_Attribute = 0;
                st_LA_Fld_Name.u8_FieldId   = CMBS_MCL_FLD_NAME;
                st_LA_Fld_Name.u16_DataLen  = (u16) strlen((char*)pst_AppEntry->sLastName);
                st_LA_Fld_Name.pu8_Data     = (void*)pst_AppEntry->sLastName;
                cmbs_util_LA_Fld_NameAdd(pu8_Buffer, &st_LA_Fld_Name, &u16_index);
            }
            break;

        case CMBS_MCL_FLD_DATETIME:
            {
                ST_UTIL_LA_FLD_DATETIME st_LA_Fld_DateTime;
                struct tm* pst_Time;
                pst_Time = gmtime(&(pst_AppEntry->t_DateAndTime));

                st_LA_Fld_DateTime.u8_Attribute         = 0;
                st_LA_Fld_DateTime.u8_FieldId           = CMBS_MCL_FLD_DATETIME;
                st_LA_Fld_DateTime.u8_Coding            = 3;
                st_LA_Fld_DateTime.u8_interpretation    = 0;
                st_LA_Fld_DateTime.u8_Year              = pst_Time->tm_year;
                st_LA_Fld_DateTime.u8_Month             = pst_Time->tm_mon;
                st_LA_Fld_DateTime.u8_Day               = pst_Time->tm_mday;
                st_LA_Fld_DateTime.u8_Hours             = pst_Time->tm_hour;
                st_LA_Fld_DateTime.u8_Mins              = pst_Time->tm_min;
                st_LA_Fld_DateTime.u8_Secs              = pst_Time->tm_sec;
                st_LA_Fld_DateTime.u8_TimeZone          = 0;

                ConvertTimeToCatiqTime(&st_LA_Fld_DateTime);
                cmbs_util_LA_Fld_DateTimeAdd(pu8_Buffer, &st_LA_Fld_DateTime,  &u16_index);
            }
            break;

        case CMBS_MCL_FLD_NEW:
            {
                ST_UTIL_LA_FLD_NEW st_LA_Fld_New;
                st_LA_Fld_New.u8_Attribute  = pst_AppEntry->bRead ? 0x40 : 0x60;
                st_LA_Fld_New.u8_FieldId    = CMBS_MCL_FLD_NEW;
                cmbs_util_LA_Fld_NewAdd(pu8_Buffer, &st_LA_Fld_New, &u16_index);
            }
            break;

        case CMBS_MCL_FLD_LINE_NAME:
            {
                ST_UTIL_LA_FLD_LINE_NAME st_LA_Fld_LineName;
                st_LA_Fld_LineName.u8_Attribute = 0;
                st_LA_Fld_LineName.u8_FieldId   = CMBS_MCL_FLD_LINE_NAME;
                st_LA_Fld_LineName.u16_DataLen  = (u16) strlen((char*)pst_AppEntry->sLineName);
                st_LA_Fld_LineName.pu8_Data     = (void*)pst_AppEntry->sLineName;
                cmbs_util_LA_Fld_LineNameAdd(pu8_Buffer, &st_LA_Fld_LineName, &u16_index);
            }
            break;

        case CMBS_MCL_FLD_LINE_ID:
            {
                ST_UTIL_LA_FLD_LINE_ID st_LA_Fld_LineId;
                st_LA_Fld_LineId.u8_Attribute   = 0;
                st_LA_Fld_LineId.u8_FieldId     = CMBS_MCL_FLD_LINE_ID;
                st_LA_Fld_LineId.Subtype        = 0;
                st_LA_Fld_LineId.u16_LineNumber = pst_AppEntry->u32_LineId;
                cmbs_util_LA_Fld_LineIdAdd(pu8_Buffer, &st_LA_Fld_LineId, &u16_index);
            }
            break;

        case CMBS_MCL_FLD_NR_OF_CALLS:
            {
                ST_UTIL_LA_FLD_NR_OF_CALLS st_LA_Fld_NrOfCalls;
                st_LA_Fld_NrOfCalls.u8_Attribute = 0;
                st_LA_Fld_NrOfCalls.u8_FieldId   = CMBS_MCL_FLD_NR_OF_CALLS;
                st_LA_Fld_NrOfCalls.u8_Value     = pst_AppEntry->u32_NumOfCalls;
                cmbs_util_LA_Fld_NrOfCallsAdd(pu8_Buffer, &st_LA_Fld_NrOfCalls, &u16_index);
            }
            break;
        default:
            break;
        }
    }

    /* Add EntryId, Entry length */
    pu8_Entry = cmbs_util_LA_EntryLenght_EntryIdentifierAdd(pu8_Buffer, u16_index, pst_AppEntry->u32_EntryId);

    *pu16_EntryLen = u16_index - (u16)(pu8_Entry - pu8_Buffer);

    /* copy to buffer */
    memcpy(pu8_Data, pu8_Entry, *pu16_EntryLen);
}

static void PrepareOutgoingCallEntry(IN stCallsListEntry* pst_AppEntry, IN PST_IE_LA_FIELDS pst_Fields,
                                     IN u8 pu8_Data[], OUT u16* pu16_EntryLen)
{
    u16 u16_index = 0, u16_FieldIdx;
    u8 pu8_Buffer[LIST_ENTRY_MAX_SIZE];
    u8* pu8_Entry;

    memset(pu8_Buffer, 0, LIST_ENTRY_MAX_SIZE);

    /* reserve max 4 bytes for EntryID and Length (each can consume up to 2 bytes), they will be handled at the end of the function */
    u16_index += 4;

    for ( u16_FieldIdx = 0; u16_FieldIdx < pst_Fields->u16_Length; ++u16_FieldIdx )
    {
        switch ( pst_Fields->pu16_FieldId[u16_FieldIdx] )
        {
        case CMBS_OCL_FLD_NUMBER:
            {
                ST_UTIL_LA_FLD_NUMBER st_LA_Fld_Number;
                st_LA_Fld_Number.u8_Attribute   = 0;
                st_LA_Fld_Number.u8_FieldId     = CMBS_OCL_FLD_NUMBER;
                st_LA_Fld_Number.u16_DataLen    = (u16) strlen((char*)pst_AppEntry->sNumber);
                st_LA_Fld_Number.pu8_Data       = (void*)pst_AppEntry->sNumber;
                cmbs_util_LA_Fld_NumberAdd(pu8_Buffer, &st_LA_Fld_Number, &u16_index);
            }
            break;

        case CMBS_OCL_FLD_NAME:
            {
                ST_UTIL_LA_FLD_NAME st_LA_Fld_Name;
                st_LA_Fld_Name.u8_Attribute = 0;
                st_LA_Fld_Name.u8_FieldId   = CMBS_OCL_FLD_NAME;
                st_LA_Fld_Name.u16_DataLen  = (u16) strlen((char*)pst_AppEntry->sLastName);
                st_LA_Fld_Name.pu8_Data     = (void*)pst_AppEntry->sLastName;
                cmbs_util_LA_Fld_NameAdd(pu8_Buffer, &st_LA_Fld_Name, &u16_index);
            }
            break;

        case CMBS_OCL_FLD_DATETIME:
            {
                ST_UTIL_LA_FLD_DATETIME st_LA_Fld_DateTime;
                struct tm* pst_Time;
                pst_Time = gmtime(&(pst_AppEntry->t_DateAndTime));

                st_LA_Fld_DateTime.u8_Attribute         = 0;
                st_LA_Fld_DateTime.u8_FieldId           = CMBS_OCL_FLD_DATETIME;
                st_LA_Fld_DateTime.u8_Coding            = 3;
                st_LA_Fld_DateTime.u8_interpretation    = 0;
                st_LA_Fld_DateTime.u8_Year              = pst_Time->tm_year;
                st_LA_Fld_DateTime.u8_Month             = pst_Time->tm_mon;
                st_LA_Fld_DateTime.u8_Day               = pst_Time->tm_mday;
                st_LA_Fld_DateTime.u8_Hours             = pst_Time->tm_hour;
                st_LA_Fld_DateTime.u8_Mins              = pst_Time->tm_min;
                st_LA_Fld_DateTime.u8_Secs              = pst_Time->tm_sec;
                st_LA_Fld_DateTime.u8_TimeZone          = 0;

                ConvertTimeToCatiqTime(&st_LA_Fld_DateTime);
                cmbs_util_LA_Fld_DateTimeAdd(pu8_Buffer, &st_LA_Fld_DateTime,  &u16_index);
            }
            break;

        case CMBS_OCL_FLD_LINE_NAME:
            {
                ST_UTIL_LA_FLD_LINE_NAME st_LA_Fld_LineName;
                st_LA_Fld_LineName.u8_Attribute = 0;
                st_LA_Fld_LineName.u8_FieldId   = CMBS_OCL_FLD_LINE_NAME;
                st_LA_Fld_LineName.u16_DataLen  = (u16) strlen((char*)pst_AppEntry->sLineName);
                st_LA_Fld_LineName.pu8_Data     = (void*)pst_AppEntry->sLineName;
                cmbs_util_LA_Fld_LineNameAdd(pu8_Buffer, &st_LA_Fld_LineName, &u16_index);
            }
            break;

        case CMBS_OCL_FLD_LINE_ID:
            {
                ST_UTIL_LA_FLD_LINE_ID st_LA_Fld_LineId;
                st_LA_Fld_LineId.u8_Attribute   = 0;
                st_LA_Fld_LineId.u8_FieldId     = CMBS_OCL_FLD_LINE_ID;
                st_LA_Fld_LineId.Subtype        = 0;
                st_LA_Fld_LineId.u16_LineNumber = pst_AppEntry->u32_LineId;
                cmbs_util_LA_Fld_LineIdAdd(pu8_Buffer, &st_LA_Fld_LineId, &u16_index);
            }
            break;

        default:
            break;
        }
    }

    /* Add EntryId, Entry length */
    pu8_Entry = cmbs_util_LA_EntryLenght_EntryIdentifierAdd(pu8_Buffer, u16_index, pst_AppEntry->u32_EntryId);

    *pu16_EntryLen = (u16)(u16_index - (pu8_Entry - pu8_Buffer));

    /* copy to buffer */
    memcpy(pu8_Data, pu8_Entry, *pu16_EntryLen);
}

static void PrepareIncomingAcceptedCallEntry(IN stCallsListEntry* pst_AppEntry, IN PST_IE_LA_FIELDS pst_Fields,
                                             IN u8 pu8_Data[], OUT u16* pu16_EntryLen)
{
    u16 u16_index = 0, u16_FieldIdx;
    u8 pu8_Buffer[LIST_ENTRY_MAX_SIZE];
    u8* pu8_Entry;

    memset(pu8_Buffer, 0, LIST_ENTRY_MAX_SIZE);

    /* reserve max 4 bytes for EntryID and Length (each can consume up to 2 bytes), they will be handled at the end of the function */
    u16_index += 4;

    for ( u16_FieldIdx = 0; u16_FieldIdx < pst_Fields->u16_Length; ++u16_FieldIdx )
    {
        switch ( pst_Fields->pu16_FieldId[u16_FieldIdx] )
        {
        case CMBS_IACL_FLD_NUMBER:
            {
                ST_UTIL_LA_FLD_NUMBER st_LA_Fld_Number;
                st_LA_Fld_Number.u8_Attribute   = 0;
                st_LA_Fld_Number.u8_FieldId     = CMBS_IACL_FLD_NUMBER;
                st_LA_Fld_Number.u16_DataLen    = (u16) strlen((char*)pst_AppEntry->sNumber);
                st_LA_Fld_Number.pu8_Data       = (void*)pst_AppEntry->sNumber;
                cmbs_util_LA_Fld_NumberAdd(pu8_Buffer, &st_LA_Fld_Number, &u16_index);
            }
            break;

        case CMBS_IACL_FLD_NAME:
            {
                ST_UTIL_LA_FLD_NAME st_LA_Fld_Name;
                st_LA_Fld_Name.u8_Attribute = 0;
                st_LA_Fld_Name.u8_FieldId   = CMBS_IACL_FLD_NAME;
                st_LA_Fld_Name.u16_DataLen  = (u16) strlen((char*)pst_AppEntry->sLastName);
                st_LA_Fld_Name.pu8_Data     = (void*)pst_AppEntry->sLastName;
                cmbs_util_LA_Fld_NameAdd(pu8_Buffer, &st_LA_Fld_Name, &u16_index);
            }
            break;

        case CMBS_IACL_FLD_DATETIME:
            {
                ST_UTIL_LA_FLD_DATETIME st_LA_Fld_DateTime;
                struct tm* pst_Time;
                pst_Time = gmtime(&(pst_AppEntry->t_DateAndTime));

                st_LA_Fld_DateTime.u8_Attribute         = 0;
                st_LA_Fld_DateTime.u8_FieldId           = CMBS_IACL_FLD_DATETIME;
                st_LA_Fld_DateTime.u8_Coding            = 3;
                st_LA_Fld_DateTime.u8_interpretation    = 0;
                st_LA_Fld_DateTime.u8_Year              = pst_Time->tm_year;
                st_LA_Fld_DateTime.u8_Month             = pst_Time->tm_mon;
                st_LA_Fld_DateTime.u8_Day               = pst_Time->tm_mday;
                st_LA_Fld_DateTime.u8_Hours             = pst_Time->tm_hour;
                st_LA_Fld_DateTime.u8_Mins              = pst_Time->tm_min;
                st_LA_Fld_DateTime.u8_Secs              = pst_Time->tm_sec;
                st_LA_Fld_DateTime.u8_TimeZone          = 0;

                ConvertTimeToCatiqTime(&st_LA_Fld_DateTime);
                cmbs_util_LA_Fld_DateTimeAdd(pu8_Buffer, &st_LA_Fld_DateTime,  &u16_index);
            }
            break;

        case CMBS_IACL_FLD_LINE_NAME:
            {
                ST_UTIL_LA_FLD_LINE_NAME st_LA_Fld_LineName;
                st_LA_Fld_LineName.u8_Attribute = 0;
                st_LA_Fld_LineName.u8_FieldId   = CMBS_IACL_FLD_LINE_NAME;
                st_LA_Fld_LineName.u16_DataLen  = (u16) strlen((char*)pst_AppEntry->sLineName);
                st_LA_Fld_LineName.pu8_Data     = (void*)pst_AppEntry->sLineName;
                cmbs_util_LA_Fld_LineNameAdd(pu8_Buffer, &st_LA_Fld_LineName, &u16_index);
            }
            break;

        case CMBS_IACL_FLD_LINE_ID:
            {
                ST_UTIL_LA_FLD_LINE_ID st_LA_Fld_LineId;
                st_LA_Fld_LineId.u8_Attribute   = 0;
                st_LA_Fld_LineId.u8_FieldId     = CMBS_IACL_FLD_LINE_ID;
                st_LA_Fld_LineId.Subtype        = 0;
                st_LA_Fld_LineId.u16_LineNumber = pst_AppEntry->u32_LineId;
                cmbs_util_LA_Fld_LineIdAdd(pu8_Buffer, &st_LA_Fld_LineId, &u16_index);
            }
            break;

        default:
            break;
        }
    }

    /* Add EntryId, Entry length */
    pu8_Entry = cmbs_util_LA_EntryLenght_EntryIdentifierAdd(pu8_Buffer, u16_index, pst_AppEntry->u32_EntryId);

    *pu16_EntryLen = (u16)(u16_index - (pu8_Entry - pu8_Buffer));

    /* copy to buffer */
    memcpy(pu8_Data, pu8_Entry, *pu16_EntryLen);
}

static void PrepareAllCallEntry(IN stCallsListEntry* pst_AppEntry, IN PST_IE_LA_FIELDS pst_Fields,
                                IN u8 pu8_Data[], OUT u16* pu16_EntryLen)
{
    u16 u16_index = 0, u16_FieldIdx;
    u8 pu8_Buffer[LIST_ENTRY_MAX_SIZE];
    u8* pu8_Entry;

    memset(pu8_Buffer, 0, LIST_ENTRY_MAX_SIZE);

    /* reserve max 4 bytes for EntryID and Length (each can consume up to 2 bytes), they will be handled at the end of the function */
    u16_index += 4;

    for ( u16_FieldIdx = 0; u16_FieldIdx < pst_Fields->u16_Length; ++u16_FieldIdx )
    {
        switch ( pst_Fields->pu16_FieldId[u16_FieldIdx] )
        {
        case CMBS_ACL_FLD_CALL_TYPE:
            {
                ST_UTIL_LA_FLD_CALL_TYPE st_LA_Fld_CallType;
                switch ( pst_AppEntry->cCallType )
                {
                case CALL_TYPE_MISSED:      st_LA_Fld_CallType.u8_Attribute = 0x20;     break;
                case CALL_TYPE_OUTGOING:    st_LA_Fld_CallType.u8_Attribute = 0x08;     break;
                case CALL_TYPE_INCOMING:    st_LA_Fld_CallType.u8_Attribute = 0x10;     break;
                default:                                                                break;
                }

                st_LA_Fld_CallType.u8_FieldId = CMBS_ACL_FLD_CALL_TYPE;
                cmbs_util_LA_Fld_CallTypeAdd(pu8_Buffer, &st_LA_Fld_CallType, &u16_index);
            }
            break;

        case CMBS_ACL_FLD_NUMBER:
            {
                ST_UTIL_LA_FLD_NUMBER st_LA_Fld_Number;
                st_LA_Fld_Number.u8_Attribute   = 0;
                st_LA_Fld_Number.u8_FieldId     = CMBS_ACL_FLD_NUMBER;
                st_LA_Fld_Number.u16_DataLen    = (u16) strlen((char*)pst_AppEntry->sNumber);
                st_LA_Fld_Number.pu8_Data       = (void*)pst_AppEntry->sNumber;
                cmbs_util_LA_Fld_NumberAdd(pu8_Buffer, &st_LA_Fld_Number, &u16_index);
            }
            break;

        case CMBS_ACL_FLD_NAME:
            {
                ST_UTIL_LA_FLD_NAME st_LA_Fld_Name;
                st_LA_Fld_Name.u8_Attribute = 0;
                st_LA_Fld_Name.u8_FieldId   = CMBS_ACL_FLD_NAME;
                st_LA_Fld_Name.u16_DataLen  = (u16) strlen((char*)pst_AppEntry->sLastName);
                st_LA_Fld_Name.pu8_Data     = (void*)pst_AppEntry->sLastName;
                cmbs_util_LA_Fld_NameAdd(pu8_Buffer, &st_LA_Fld_Name, &u16_index);
            }
            break;

        case CMBS_ACL_FLD_DATETIME:
            {
                ST_UTIL_LA_FLD_DATETIME st_LA_Fld_DateTime;
                struct tm* pst_Time;
                pst_Time = gmtime(&(pst_AppEntry->t_DateAndTime));

                st_LA_Fld_DateTime.u8_Attribute         = 0;
                st_LA_Fld_DateTime.u8_FieldId           = CMBS_ACL_FLD_DATETIME;
                st_LA_Fld_DateTime.u8_Coding            = 3;
                st_LA_Fld_DateTime.u8_interpretation    = 0;
                st_LA_Fld_DateTime.u8_Year              = pst_Time->tm_year;
                st_LA_Fld_DateTime.u8_Month             = pst_Time->tm_mon;
                st_LA_Fld_DateTime.u8_Day               = pst_Time->tm_mday;
                st_LA_Fld_DateTime.u8_Hours             = pst_Time->tm_hour;
                st_LA_Fld_DateTime.u8_Mins              = pst_Time->tm_min;
                st_LA_Fld_DateTime.u8_Secs              = pst_Time->tm_sec;
                st_LA_Fld_DateTime.u8_TimeZone          = 0;

                ConvertTimeToCatiqTime(&st_LA_Fld_DateTime);
                cmbs_util_LA_Fld_DateTimeAdd(pu8_Buffer, &st_LA_Fld_DateTime,  &u16_index);
            }
            break;

        case CMBS_ACL_FLD_LINE_NAME:
            {
                ST_UTIL_LA_FLD_LINE_NAME st_LA_Fld_LineName;
                st_LA_Fld_LineName.u8_Attribute = 0;
                st_LA_Fld_LineName.u8_FieldId   = CMBS_ACL_FLD_LINE_NAME;
                st_LA_Fld_LineName.u16_DataLen  = (u16) strlen((char*)pst_AppEntry->sLineName);
                st_LA_Fld_LineName.pu8_Data     = (void*)pst_AppEntry->sLineName;
                cmbs_util_LA_Fld_LineNameAdd(pu8_Buffer, &st_LA_Fld_LineName, &u16_index);
            }
            break;

        case CMBS_ACL_FLD_LINE_ID:
            {
                ST_UTIL_LA_FLD_LINE_ID st_LA_Fld_LineId;
                st_LA_Fld_LineId.u8_Attribute   = 0;
                st_LA_Fld_LineId.u8_FieldId     = CMBS_ACL_FLD_LINE_ID;
                st_LA_Fld_LineId.Subtype        = 0;
                st_LA_Fld_LineId.u16_LineNumber = pst_AppEntry->u32_LineId;
                cmbs_util_LA_Fld_LineIdAdd(pu8_Buffer, &st_LA_Fld_LineId, &u16_index);
            }
            break;

        default:
            break;
        }
    }

    /* Add EntryId, Entry length */
    pu8_Entry = cmbs_util_LA_EntryLenght_EntryIdentifierAdd(pu8_Buffer, u16_index, pst_AppEntry->u32_EntryId);

    *pu16_EntryLen = (u16)(u16_index - (pu8_Entry - pu8_Buffer));

    /* copy to buffer */
    memcpy(pu8_Data, pu8_Entry, *pu16_EntryLen);
}

static void PrepareIncomingCallEntry(IN stCallsListEntry* pst_AppEntry, IN PST_IE_LA_FIELDS pst_Fields,
                                     IN u8 pu8_Data[], OUT u16* pu16_EntryLen)
{
    u16 u16_index = 0, u16_FieldIdx;
    u8 pu8_Buffer[LIST_ENTRY_MAX_SIZE];
    u8* pu8_Entry;

    memset(pu8_Buffer, 0, LIST_ENTRY_MAX_SIZE);

    /* reserve max 4 bytes for EntryID and Length (each can consume up to 2 bytes), they will be handled at the end of the function */
    u16_index += 4;

    for ( u16_FieldIdx = 0; u16_FieldIdx < pst_Fields->u16_Length; ++u16_FieldIdx )
    {
        switch ( pst_Fields->pu16_FieldId[u16_FieldIdx] )
        {
        case CMBS_AICL_FLD_NUMBER:
            {
                ST_UTIL_LA_FLD_NUMBER st_LA_Fld_Number;
                st_LA_Fld_Number.u8_Attribute   = 0;
                st_LA_Fld_Number.u8_FieldId     = CMBS_AICL_FLD_NUMBER;
                st_LA_Fld_Number.u16_DataLen    = (u16) strlen((char*)pst_AppEntry->sNumber);
                st_LA_Fld_Number.pu8_Data       = (void*)pst_AppEntry->sNumber;
                cmbs_util_LA_Fld_NumberAdd(pu8_Buffer, &st_LA_Fld_Number, &u16_index);
            }
            break;

        case CMBS_AICL_FLD_NAME:
            {
                ST_UTIL_LA_FLD_NAME st_LA_Fld_Name;
                st_LA_Fld_Name.u8_Attribute = 0;
                st_LA_Fld_Name.u8_FieldId   = CMBS_AICL_FLD_NAME;
                st_LA_Fld_Name.u16_DataLen  = (u16) strlen((char*)pst_AppEntry->sLastName);
                st_LA_Fld_Name.pu8_Data     = (void*)pst_AppEntry->sLastName;
                cmbs_util_LA_Fld_NameAdd(pu8_Buffer, &st_LA_Fld_Name, &u16_index);
            }
            break;

        case CMBS_AICL_FLD_DATETIME:
            {
                ST_UTIL_LA_FLD_DATETIME st_LA_Fld_DateTime;
                struct tm* pst_Time;
                pst_Time = gmtime(&(pst_AppEntry->t_DateAndTime));

                st_LA_Fld_DateTime.u8_Attribute         = 0;
                st_LA_Fld_DateTime.u8_FieldId           = CMBS_AICL_FLD_DATETIME;
                st_LA_Fld_DateTime.u8_Coding            = 3;
                st_LA_Fld_DateTime.u8_interpretation    = 0;
                st_LA_Fld_DateTime.u8_Year              = pst_Time->tm_year;
                st_LA_Fld_DateTime.u8_Month             = pst_Time->tm_mon;
                st_LA_Fld_DateTime.u8_Day               = pst_Time->tm_mday;
                st_LA_Fld_DateTime.u8_Hours             = pst_Time->tm_hour;
                st_LA_Fld_DateTime.u8_Mins              = pst_Time->tm_min;
                st_LA_Fld_DateTime.u8_Secs              = pst_Time->tm_sec;
                st_LA_Fld_DateTime.u8_TimeZone          = 0;

                ConvertTimeToCatiqTime(&st_LA_Fld_DateTime);
                cmbs_util_LA_Fld_DateTimeAdd(pu8_Buffer, &st_LA_Fld_DateTime,  &u16_index);
            }
            break;

        case CMBS_AICL_FLD_NEW:
            {
                ST_UTIL_LA_FLD_NEW st_LA_Fld_New;
                if ( pst_AppEntry->cCallType ==  CALL_TYPE_INCOMING )
                {
                    st_LA_Fld_New.u8_Attribute = 0x40;
                }
                else
                {
                    st_LA_Fld_New.u8_Attribute  = pst_AppEntry->bRead ? 0x40 : 0x60;
                }
                st_LA_Fld_New.u8_FieldId    = CMBS_AICL_FLD_NEW;
                cmbs_util_LA_Fld_NewAdd(pu8_Buffer, &st_LA_Fld_New, &u16_index);
            }
            break;

        case CMBS_AICL_FLD_LINE_NAME:
            {
                ST_UTIL_LA_FLD_LINE_NAME st_LA_Fld_LineName;
                st_LA_Fld_LineName.u8_Attribute = 0;
                st_LA_Fld_LineName.u8_FieldId   = CMBS_AICL_FLD_LINE_NAME;
                st_LA_Fld_LineName.u16_DataLen  = (u16) strlen((char*)pst_AppEntry->sLineName);
                st_LA_Fld_LineName.pu8_Data     = (void*)pst_AppEntry->sLineName;
                cmbs_util_LA_Fld_LineNameAdd(pu8_Buffer, &st_LA_Fld_LineName, &u16_index);
            }
            break;

        case CMBS_AICL_FLD_LINE_ID:
            {
                ST_UTIL_LA_FLD_LINE_ID st_LA_Fld_LineId;
                st_LA_Fld_LineId.u8_Attribute   = 0;
                st_LA_Fld_LineId.u8_FieldId     = CMBS_AICL_FLD_LINE_ID;
                st_LA_Fld_LineId.Subtype        = 0;
                st_LA_Fld_LineId.u16_LineNumber = pst_AppEntry->u32_LineId;
                cmbs_util_LA_Fld_LineIdAdd(pu8_Buffer, &st_LA_Fld_LineId, &u16_index);
            }
            break;

        case CMBS_AICL_FLD_NR_OF_CALLS:
            {
                ST_UTIL_LA_FLD_NR_OF_CALLS st_LA_Fld_NrOfCalls;
                st_LA_Fld_NrOfCalls.u8_Attribute = 0;
                st_LA_Fld_NrOfCalls.u8_FieldId   = CMBS_AICL_FLD_NR_OF_CALLS;
                if ( pst_AppEntry->cCallType ==  CALL_TYPE_INCOMING )
                {
                    st_LA_Fld_NrOfCalls.u8_Value     = 0;
                }
                else
                {
                    st_LA_Fld_NrOfCalls.u8_Value     = pst_AppEntry->u32_NumOfCalls;
                }
                cmbs_util_LA_Fld_NrOfCallsAdd(pu8_Buffer, &st_LA_Fld_NrOfCalls, &u16_index);
            }
            break;
        default:
            break;
        }
    }

    /* Add EntryId, Entry length */
    pu8_Entry = cmbs_util_LA_EntryLenght_EntryIdentifierAdd(pu8_Buffer, u16_index, pst_AppEntry->u32_EntryId);

    *pu16_EntryLen = (u16)(u16_index - (pu8_Entry - pu8_Buffer));

    /* copy to buffer */
    memcpy(pu8_Data, pu8_Entry, *pu16_EntryLen);
}

static void PrepareLineSettingsEntry(IN stLineSettingsListEntry* pst_AppEntry, IN PST_IE_LA_FIELDS pst_Fields,
                                     IN u8 pu8_Data[], OUT u16* pu16_EntryLen)
{
    u16 u16_index = 0, u16_FieldIdx;
    u8 pu8_Buffer[LIST_ENTRY_MAX_SIZE];
    u8* pu8_Entry;

    memset(pu8_Buffer, 0, LIST_ENTRY_MAX_SIZE);

    /* reserve max 4 bytes for EntryID and Length (each can consume up to 2 bytes), they will be handled at the end of the function */
    u16_index += 4;

    for ( u16_FieldIdx = 0; u16_FieldIdx < pst_Fields->u16_Length; ++u16_FieldIdx )
    {
        switch ( pst_Fields->pu16_FieldId[u16_FieldIdx] )
        {
        case CMBS_LSL_FLD_LINE_NAME:
            {
                ST_UTIL_LA_FLD_LINE_NAME st_LA_Fld_LineName;
                st_LA_Fld_LineName.u8_Attribute     = 0x40;
                st_LA_Fld_LineName.u8_FieldId       = CMBS_LSL_FLD_LINE_NAME;
                st_LA_Fld_LineName.u16_DataLen      = (u16) strlen((char*)pst_AppEntry->sLineName);
                st_LA_Fld_LineName.pu8_Data         = (void*)pst_AppEntry->sLineName;
                cmbs_util_LA_Fld_LineNameAdd(pu8_Buffer, &st_LA_Fld_LineName, &u16_index);
            }
            break;

        case CMBS_LSL_FLD_LINE_ID:
            {
                ST_UTIL_LA_FLD_LINE_ID st_LA_Fld_LineId;
                st_LA_Fld_LineId.u8_Attribute   = 0;
                st_LA_Fld_LineId.u8_FieldId     = CMBS_LSL_FLD_LINE_ID;
                st_LA_Fld_LineId.Subtype        = 3;
                st_LA_Fld_LineId.u16_LineNumber = pst_AppEntry->u32_LineId;
                cmbs_util_LA_Fld_LineIdAdd(pu8_Buffer, &st_LA_Fld_LineId, &u16_index);
            }
            break;

        case CMBS_LSL_FLD_ATTACHED_HANDSETS:
            {
                ST_UTIL_LA_FLD_ATTACHED_HANDSETS st_LA_Fld_AttachedHandsets;
                st_LA_Fld_AttachedHandsets.u8_Attribute = 0x40;
                st_LA_Fld_AttachedHandsets.u8_FieldId   = CMBS_LSL_FLD_ATTACHED_HANDSETS;
                st_LA_Fld_AttachedHandsets.pu8_Data     = (void*)&pst_AppEntry->u32_AttachedHsMask; 
                cmbs_util_LA_Fld_AttachedHandsetsAdd(pu8_Buffer, &st_LA_Fld_AttachedHandsets, &u16_index);
            }
            break;

        case CMBS_LSL_FLD_DIALING_PREFIX:
            {
                ST_UTIL_LA_FLD_DIALING_PREFIX st_LA_Fld_DialingPrefixe;
                st_LA_Fld_DialingPrefixe.u8_Attribute   = 0x40;
                st_LA_Fld_DialingPrefixe.u8_FieldId     = CMBS_LSL_FLD_DIALING_PREFIX;
                st_LA_Fld_DialingPrefixe.u16_DataLen    = (u16) strlen((char*)pst_AppEntry->sDialPrefix);
                st_LA_Fld_DialingPrefixe.pu8_Data       = (void*)pst_AppEntry->sDialPrefix;
                cmbs_util_LA_Fld_DialPrefixeAdd(pu8_Buffer, &st_LA_Fld_DialingPrefixe,  &u16_index);
            }
            break;

        case CMBS_LSL_FLD_FP_MELODY:
            {
                ST_UTIL_LA_FLD_FP_MELODY st_LA_Fld_FPmelody;
                st_LA_Fld_FPmelody.u8_Attribute = 0x40;
                st_LA_Fld_FPmelody.u8_FieldId   = CMBS_LSL_FLD_FP_MELODY;
                st_LA_Fld_FPmelody.u8_Value     = pst_AppEntry->u32_FPMelody;
                cmbs_util_LA_Fld_AssocMdyAdd(pu8_Buffer, &st_LA_Fld_FPmelody, &u16_index);
            }
            break;

        case CMBS_LSL_FLD_FP_VOLUME:
            {
                ST_UTIL_LA_FLD_FP_VOLUME st_LA_Fld_FPvolume;
                st_LA_Fld_FPvolume.u8_Attribute = 0x40;
                st_LA_Fld_FPvolume.u8_FieldId   = CMBS_LSL_FLD_FP_VOLUME;
                st_LA_Fld_FPvolume.u8_Value     = pst_AppEntry->u32_FPVolume;
                cmbs_util_LA_Fld_FPvolumeAdd(pu8_Buffer, &st_LA_Fld_FPvolume, &u16_index);
            }
            break;

        case CMBS_LSL_FLD_BLOCKED_NUMBER:
            {
                ST_UTIL_LA_FLD_BLOCKED_NUMBER st_LA_Fld_BlockedNumber;
                st_LA_Fld_BlockedNumber.u8_Attribute    = 0x40;
                st_LA_Fld_BlockedNumber.u8_FieldId      = CMBS_LSL_FLD_BLOCKED_NUMBER;
                st_LA_Fld_BlockedNumber.u16_DataLen     = (u16) strlen((char*)pst_AppEntry->sBlockedNumber);
                st_LA_Fld_BlockedNumber.pu8_Data        = (void*)pst_AppEntry->sBlockedNumber;
                cmbs_util_LA_Fld_BlockedNumberAdd(pu8_Buffer, &st_LA_Fld_BlockedNumber, &u16_index);
            }
            break;

        case CMBS_LSL_FLD_MULTI_CALL_MODE:
            {
                ST_UTIL_LA_FLD_MULTI_CALL_MODE st_LA_Fld_MultiCallMode;
                st_LA_Fld_MultiCallMode.u8_Attribute    = 0x40;
                st_LA_Fld_MultiCallMode.u8_FieldId      = CMBS_LSL_FLD_MULTI_CALL_MODE;
                st_LA_Fld_MultiCallMode.u8_Value        = pst_AppEntry->bMultiCalls ? 0x31 : 0x30;
                cmbs_util_LA_Fld_MultiCallModeAdd(pu8_Buffer, &st_LA_Fld_MultiCallMode, &u16_index);
            }
            break;

        case CMBS_LSL_FLD_INTRUSION_CALL:
            {
                ST_UTIL_LA_FLD_INTRUSION_CALL st_LA_Fld_IntrusionCall;
                st_LA_Fld_IntrusionCall.u8_Attribute    = 0x40;
                st_LA_Fld_IntrusionCall.u8_FieldId      = CMBS_LSL_FLD_INTRUSION_CALL;
                st_LA_Fld_IntrusionCall.u8_Value        = pst_AppEntry->bIntrusionCall ? 0x31 : 0x30;
                cmbs_util_LA_Fld_IntrusionCallAdd(pu8_Buffer, &st_LA_Fld_IntrusionCall, &u16_index);
            }
            break;

        case CMBS_LSL_FLD_PERMANENT_CLIR:
            {
                ST_UTIL_LA_FLD_PERMANENT_CLIR st_LA_Fld_PermanentCLIR;
                st_LA_Fld_PermanentCLIR.u8_Attribute        = 0x40;
                st_LA_Fld_PermanentCLIR.u8_FieldId          = CMBS_LSL_FLD_PERMANENT_CLIR;
                st_LA_Fld_PermanentCLIR.u8_Value            = pst_AppEntry->bPermanentCLIR ? 0x31 : 0x30;
                st_LA_Fld_PermanentCLIR.u8_ActDataLen       = (u8) strlen((char*)pst_AppEntry->sPermanentCLIRActCode);
                st_LA_Fld_PermanentCLIR.pu8_ActData         = (void*)pst_AppEntry->sPermanentCLIRActCode;
                st_LA_Fld_PermanentCLIR.u8_DesactDataLen    = (u8) strlen((char*)pst_AppEntry->sPermanentCLIRDeactCode);  
                st_LA_Fld_PermanentCLIR.pu8_DesactData      = (void*)pst_AppEntry->sPermanentCLIRDeactCode;
                cmbs_util_LA_Fld_PermanentCLIRAdd(pu8_Buffer, &st_LA_Fld_PermanentCLIR, &u16_index);
            }
            break;

        case CMBS_LSL_FLD_CALL_FORWARDING:
            {
                ST_UTIL_LA_FLD_CALL_FORWARDING st_LA_Fld_CallForwarding;
                st_LA_Fld_CallForwarding.u8_Attribute       = 0x40;
                st_LA_Fld_CallForwarding.u8_FieldId         = CMBS_LSL_FLD_CALL_FORWARDING;
                st_LA_Fld_CallForwarding.u8_NotSetByUSer    = 0;
                st_LA_Fld_CallForwarding.u8_Value           = pst_AppEntry->bCallFwdUncond ? 0x31 : 0x30;
                st_LA_Fld_CallForwarding.u8_ActDataLen      = (u8) strlen((char*)pst_AppEntry->sCallFwdUncondActCode); 
                st_LA_Fld_CallForwarding.pu8_ActData        = (void*)pst_AppEntry->sCallFwdUncondActCode;
                st_LA_Fld_CallForwarding.u8_DesactDataLen   = (u8) strlen((char*)pst_AppEntry->sCallFwdUncondDeactCode); 
                st_LA_Fld_CallForwarding.pu8_DesactData     = (void*)pst_AppEntry->sCallFwdUncondDeactCode;
                st_LA_Fld_CallForwarding.u8_NumberLen       = (u8) strlen((char*)pst_AppEntry->sCallFwdUncondNum);
                st_LA_Fld_CallForwarding.pu8_Number         = (void*)pst_AppEntry->sCallFwdUncondNum;
                cmbs_util_LA_Fld_CallForwardingAdd(pu8_Buffer, &st_LA_Fld_CallForwarding, &u16_index);
            }
            break;

        case CMBS_LSL_FLD_CFNA:
            {
                ST_UTIL_LA_FLD_CFNA st_LA_Fld_CFNA;
                st_LA_Fld_CFNA.u8_Attribute         = 0x40;
                st_LA_Fld_CFNA.u8_FieldId           = CMBS_LSL_FLD_CFNA;
                st_LA_Fld_CFNA.u8_NotSetByUSer      = 0;
                st_LA_Fld_CFNA.u8_Value             = pst_AppEntry->bCallFwdNoAns ? 0x31 : 0x30;
                st_LA_Fld_CFNA.u8_Timeout           = (u8) pst_AppEntry->u32_CallFwdNoAnsTimeout;
                st_LA_Fld_CFNA.u8_ActDataLen        = (u8) strlen((char*)pst_AppEntry->sCallFwdNoAnsActCode);   
                st_LA_Fld_CFNA.pu8_ActData          = (void*)pst_AppEntry->sCallFwdNoAnsActCode;                 
                st_LA_Fld_CFNA.u8_DesactDataLen     = (u8) strlen((char*)pst_AppEntry->sCallFwdNoAnsDeactCode); 
                st_LA_Fld_CFNA.pu8_DesactData       = (void*)pst_AppEntry->sCallFwdNoAnsDeactCode;               
                st_LA_Fld_CFNA.u8_NumberLen         = (u8) strlen((char*)pst_AppEntry->sCallFwdNoAnsNum);       
                st_LA_Fld_CFNA.pu8_Number           = (void*)pst_AppEntry->sCallFwdNoAnsNum;                     
                cmbs_util_LA_Fld_CFNAAdd(pu8_Buffer, &st_LA_Fld_CFNA, &u16_index);
            }
            break;

        case CMBS_LSL_FLD_CFB:
            {
                ST_UTIL_LA_FLD_CFB st_LA_Fld_CFB;
                st_LA_Fld_CFB.u8_Attribute      = 0x40;
                st_LA_Fld_CFB.u8_FieldId        = CMBS_LSL_FLD_CFB;
                st_LA_Fld_CFB.u8_NotSetByUSer   = 0;
                st_LA_Fld_CFB.u8_Value          = pst_AppEntry->bCallFwdBusy ? 0x31 : 0x30;                
                st_LA_Fld_CFB.u8_ActDataLen     = (u8) strlen((char*)pst_AppEntry->sCallFwdBusyActCode);  
                st_LA_Fld_CFB.pu8_ActData       = (void*)pst_AppEntry->sCallFwdBusyActCode;                
                st_LA_Fld_CFB.u8_DesactDataLen  = (u8) strlen((char*)pst_AppEntry->sCallFwdBusyDeactCode);
                st_LA_Fld_CFB.pu8_DesactData    = (void*)pst_AppEntry->sCallFwdBusyDeactCode;              
                st_LA_Fld_CFB.u8_NumberLen      = (u8) strlen((char*)pst_AppEntry->sCallFwdBusyNum);      
                st_LA_Fld_CFB.pu8_Number        = (void*)pst_AppEntry->sCallFwdBusyNum;                    
                cmbs_util_LA_Fld_CFBAdd(pu8_Buffer, &st_LA_Fld_CFB, &u16_index);
            }
            break;

        default:
            break;
        }
    }

    /* Add EntryId, Entry length */
    pu8_Entry = cmbs_util_LA_EntryLenght_EntryIdentifierAdd(pu8_Buffer, u16_index, pst_AppEntry->u32_EntryId);

    *pu16_EntryLen = (u16)(u16_index - (pu8_Entry - pu8_Buffer));

    /* copy to buffer */
    memcpy(pu8_Data, pu8_Entry, *pu16_EntryLen);
}

static void PrepareContactListEntry(IN stContactListEntry* pst_AppEntry, IN PST_IE_LA_FIELDS pst_Fields,
                                    IN u8 pu8_Data[], OUT u16* pu16_EntryLen)
{
    u16 u16_index = 0, u16_FieldIdx;
    u8 pu8_Buffer[LIST_ENTRY_MAX_SIZE];
    u8* pu8_Entry;
    bool bContactNumber = FALSE;

    memset(pu8_Buffer, 0, LIST_ENTRY_MAX_SIZE);

    /* reserve max 4 bytes for EntryID and Length (each can consume up to 2 bytes), they will be handled at the end of the function */
    u16_index += 4;

    for ( u16_FieldIdx = 0; u16_FieldIdx < pst_Fields->u16_Length; ++u16_FieldIdx )
    {
        switch ( pst_Fields->pu16_FieldId[u16_FieldIdx] )
        {
        case CMBS_CL_FLD_NAME:
            {
                ST_UTIL_LA_FLD_NAME st_LA_Fld_Name;
                st_LA_Fld_Name.u8_Attribute     = 0x40;
                st_LA_Fld_Name.u8_FieldId       = CMBS_CL_FLD_NAME;
                st_LA_Fld_Name.u16_DataLen      = (u16) strlen((char*)pst_AppEntry->sLastName);
                st_LA_Fld_Name.pu8_Data         = (void*)pst_AppEntry->sLastName;
                cmbs_util_LA_Fld_NameAdd(pu8_Buffer, &st_LA_Fld_Name, &u16_index);
            }
            break;

        case CMBS_CL_FLD_FIRST_NAME:
            {
                ST_UTIL_LA_FLD_FIRST_NAME st_LA_Fld_FirstName;
                st_LA_Fld_FirstName.u8_Attribute    = 0x40;
                st_LA_Fld_FirstName.u8_FieldId      = CMBS_CL_FLD_FIRST_NAME;
                st_LA_Fld_FirstName.u16_DataLen     = (u16) strlen((char*)pst_AppEntry->sFirstName);
                st_LA_Fld_FirstName.pu8_Data        = (void*)pst_AppEntry->sFirstName;
                cmbs_util_LA_Fld_FirstNameAdd(pu8_Buffer, &st_LA_Fld_FirstName, &u16_index);
            }
            break;

        case CMBS_CL_FLD_CONTACT_NR:
            {
                ST_UTIL_LA_FLD_CONTACT_NR st_LA_Fld_ContactNumber;
                st_LA_Fld_ContactNumber.u8_FieldId = CMBS_CL_FLD_CONTACT_NR;


                if ( bContactNumber == FALSE )
                {
                    st_LA_Fld_ContactNumber.u8_Attribute = 0x40;
                    if ( pst_AppEntry->bNumber1Default )
                    {
                        st_LA_Fld_ContactNumber.u8_Attribute |= 0x20;
                    }
                    if ( pst_AppEntry->bNumber1Own )
                    {
                        st_LA_Fld_ContactNumber.u8_Attribute |= 0x10;
                    }
                    switch ( pst_AppEntry->cNumber1Type )
                    {
                    case NUM_TYPE_FIXED:    st_LA_Fld_ContactNumber.u8_Attribute |= 0x08;   break;
                    case NUM_TYPE_MOBILE:   st_LA_Fld_ContactNumber.u8_Attribute |= 0x04;   break;
                    case NUM_TYPE_WORK:     st_LA_Fld_ContactNumber.u8_Attribute |= 0x02;   break;
                    default:                                                                break;
                    }
                    st_LA_Fld_ContactNumber.u16_DataLen  = (u16) strlen((char*)pst_AppEntry->sNumber1);
                    st_LA_Fld_ContactNumber.pu8_Data     = (void*)pst_AppEntry->sNumber1;
                }
                else
                {
                    st_LA_Fld_ContactNumber.u8_Attribute = 0x40;
                    if ( pst_AppEntry->bNumber2Default )
                    {
                        st_LA_Fld_ContactNumber.u8_Attribute |= 0x20;
                    }
                    if ( pst_AppEntry->bNumber2Own )
                    {
                        st_LA_Fld_ContactNumber.u8_Attribute |= 0x10;
                    }
                    switch ( pst_AppEntry->cNumber2Type )
                    {
                    case NUM_TYPE_FIXED:    st_LA_Fld_ContactNumber.u8_Attribute |= 0x08;   break;
                    case NUM_TYPE_MOBILE:   st_LA_Fld_ContactNumber.u8_Attribute |= 0x04;   break;
                    case NUM_TYPE_WORK:     st_LA_Fld_ContactNumber.u8_Attribute |= 0x02;   break;
                    default:                                                                break;
                    }
                    st_LA_Fld_ContactNumber.u16_DataLen  = (u16) strlen((char*)pst_AppEntry->sNumber2);
                    st_LA_Fld_ContactNumber.pu8_Data     = (void*)pst_AppEntry->sNumber2;
                }

                cmbs_util_LA_Fld_ContactNrAdd(pu8_Buffer, &st_LA_Fld_ContactNumber, &u16_index);

                bContactNumber = TRUE;
            }
            break;

        case CMBS_CL_FLD_ASSOC_MDY:
            {
                ST_UTIL_LA_FLD_ASSOC_MDY st_LA_Fld_Associated_melody;
                st_LA_Fld_Associated_melody.u8_Attribute    = 0x40;
                st_LA_Fld_Associated_melody.u8_FieldId      = CMBS_CL_FLD_ASSOC_MDY;
                st_LA_Fld_Associated_melody.u8_Value        = pst_AppEntry->u32_AssociatedMelody;
                cmbs_util_LA_Fld_AssocMdyAdd(pu8_Buffer, &st_LA_Fld_Associated_melody, &u16_index);
            }
            break;

        case CMBS_CL_FLD_LINE_ID:
            {
                ST_UTIL_LA_FLD_LINE_ID st_LA_Fld_LineId;
                st_LA_Fld_LineId.u8_Attribute   = 0x40;
                st_LA_Fld_LineId.u8_FieldId     = CMBS_CL_FLD_LINE_ID;
                st_LA_Fld_LineId.Subtype        = 3;
                st_LA_Fld_LineId.u16_LineNumber = pst_AppEntry->u32_LineId;
                cmbs_util_LA_Fld_LineIdAdd(pu8_Buffer, &st_LA_Fld_LineId, &u16_index);
            }
            break;

        default:
            break;
        }
    }

    /* Add EntryId, Entry length */
    pu8_Entry = cmbs_util_LA_EntryLenght_EntryIdentifierAdd(pu8_Buffer, u16_index, pst_AppEntry->u32_EntryId);

    *pu16_EntryLen = (u16)(u16_index - (pu8_Entry - pu8_Buffer));

    /* copy to buffer */
    memcpy(pu8_Data, pu8_Entry, *pu16_EntryLen);
}



static void ConvertEntriesToCatiqDataBuffer(IN u32 u32_ListId, IN PST_IE_LA_FIELDS pst_Fields,
                                            IN u8 pu8_Entries[], IN u16 u16_NumOfEntries,
                                            OUT u8 pu8_Data[], INOUT u16* pu16_DataLen)
{
    u16 u16_Index, u16_DataOffset = 0, u16_EntryLen;

    for ( u16_Index = 0; u16_Index < u16_NumOfEntries; ++u16_Index )
    {
        switch ( u32_ListId )
        {
        case LIST_TYPE_MISSED_CALLS:
            PrepareMissedCallEntry((stCallsListEntry*)pu8_Entries + u16_Index, pst_Fields, pu8_Data + u16_DataOffset, &u16_EntryLen);
            break;
        case LIST_TYPE_OUTGOING_CALLS:
            PrepareOutgoingCallEntry((stCallsListEntry*)pu8_Entries + u16_Index, pst_Fields, pu8_Data + u16_DataOffset, &u16_EntryLen);
            break;
        case LIST_TYPE_INCOMING_ACCEPTED_CALLS:
            PrepareIncomingAcceptedCallEntry((stCallsListEntry*)pu8_Entries + u16_Index, pst_Fields, pu8_Data + u16_DataOffset, &u16_EntryLen);
            break;
        case LIST_TYPE_ALL_CALLS:
            PrepareAllCallEntry((stCallsListEntry*)pu8_Entries + u16_Index, pst_Fields, pu8_Data + u16_DataOffset, &u16_EntryLen);
            break;
        case LIST_TYPE_ALL_INCOMING_CALLS:
            PrepareIncomingCallEntry((stCallsListEntry*)pu8_Entries + u16_Index, pst_Fields, pu8_Data + u16_DataOffset, &u16_EntryLen);
            break;
        case LIST_TYPE_LINE_SETTINGS_LIST:
            PrepareLineSettingsEntry((stLineSettingsListEntry*)pu8_Entries + u16_Index, pst_Fields, pu8_Data + u16_DataOffset, &u16_EntryLen);
            break;
        case LIST_TYPE_CONTACT_LIST:
            PrepareContactListEntry((stContactListEntry*)pu8_Entries + u16_Index, pst_Fields, pu8_Data + u16_DataOffset, &u16_EntryLen);
            break;
        default:
            break;
        }

        u16_DataOffset += u16_EntryLen;
    }

    *pu16_DataLen = u16_DataOffset;
}

static void ParseCallListEntry(IN u16 u16_SessionIdx, OUT u8 pu8_ParsedEntry[], OUT u32 pu32_Fields[], OUT u32* pu32_FieldsNum)
{
    stCallsListEntry* pEntry = (stCallsListEntry*)pu8_ParsedEntry;
    u16 u16_Index = 0;
    u16 u16_EntryIdentifier;
    u16 u16_EntryLength;
    u8* pu8_FieldId;
    u8* pu8_BufferToParse;

    pu8_BufferToParse = g_pst_LASessions[u16_SessionIdx].pu8_DataBuffer;

    *pu32_FieldsNum = 0;

    /* Get EntryId, Entry Length */
    cmbs_util_LA_EntryLenght_EntryIdentifierGet(pu8_BufferToParse, &u16_EntryIdentifier, &u16_EntryLength, &u16_Index);

    while ( u16_Index < u16_EntryLength )
    {
        pu8_FieldId = pu8_BufferToParse + u16_Index;

        switch ( *pu8_FieldId )
        {
        /* Read only fields are ignored */
        case CMBS_MCL_FLD_NUMBER:
        case CMBS_MCL_FLD_NAME:
        case CMBS_MCL_FLD_DATETIME:
        case CMBS_MCL_FLD_LINE_NAME:
        case CMBS_MCL_FLD_LINE_ID:
        case CMBS_MCL_FLD_NR_OF_CALLS:

        case CMBS_MCL_FLD_NEW:
            {
                ST_UTIL_LA_FLD_NEW st_LA_Fld_New;
                cmbs_util_LA_Fld_NewGet( pu8_FieldId, &st_LA_Fld_New);

                pEntry->bRead = (st_LA_Fld_New.u8_Attribute & 0x20) ? FALSE : TRUE;

                pu32_Fields[(*pu32_FieldsNum)++] = CATiqFieldId_2_AppFieldId(g_pst_LASessions[u16_SessionIdx].u32_ListId, *pu8_FieldId);
            }
            break;

        default:
            break;
        }

        cmbs_util_la_ShiftIndexToNextFieldID(pu8_FieldId, &u16_Index);
    }
}

static void ParseLineSettingsEntry(IN u16 u16_SessionIdx, OUT u8 pu8_ParsedEntry[], OUT u32 pu32_Fields[], OUT u32* pu32_FieldsNum)
{
    stLineSettingsListEntry* pEntry = (stLineSettingsListEntry*)pu8_ParsedEntry;
    u16 u16_Index = 0;
    u16 u16_EntryIdentifier;
    u16 u16_EntryLength;
    u8* pu8_FieldId;
    u8* pu8_BufferToParse;

    pu8_BufferToParse = g_pst_LASessions[u16_SessionIdx].pu8_DataBuffer;

    *pu32_FieldsNum = 0;

    /* Get EntryId, Entry Length */
    cmbs_util_LA_EntryLenght_EntryIdentifierGet(pu8_BufferToParse, &u16_EntryIdentifier, &u16_EntryLength, &u16_Index);

    while ( u16_Index < u16_EntryLength )
    {
        pu8_FieldId = pu8_BufferToParse + u16_Index;

        pu32_Fields[(*pu32_FieldsNum)++] = CATiqFieldId_2_AppFieldId(g_pst_LASessions[u16_SessionIdx].u32_ListId, *pu8_FieldId);

        switch ( *pu8_FieldId )
        {
        case CMBS_LSL_FLD_LINE_NAME:
            {
                ST_UTIL_LA_FLD_LINE_NAME st_LA_Fld_LineName;
                cmbs_util_LA_Fld_LineNameGet( pu8_FieldId, &st_LA_Fld_LineName );

                memcpy(pEntry->sLineName, st_LA_Fld_LineName.pu8_Data, st_LA_Fld_LineName.u16_DataLen);
                pEntry->sLineName[st_LA_Fld_LineName.u16_DataLen] = 0;
            }
            break;

        case CMBS_LSL_FLD_LINE_ID:
            {
                ST_UTIL_LA_FLD_LINE_ID st_LA_Fld_LineId;
                cmbs_util_LA_Fld_LineIdGet( pu8_FieldId, &st_LA_Fld_LineId );

                pEntry->u32_LineId = st_LA_Fld_LineId.u16_LineNumber;
            }
            break;

        case CMBS_LSL_FLD_ATTACHED_HANDSETS:
            {
                ST_UTIL_LA_FLD_ATTACHED_HANDSETS st_LA_Fld_AttachedHandset;
                cmbs_util_LA_Fld_AttachedHandsetsGet( pu8_FieldId, &st_LA_Fld_AttachedHandset );

                pEntry->u32_AttachedHsMask = st_LA_Fld_AttachedHandset.pu8_Data[0];
            }
            break;

        case CMBS_LSL_FLD_DIALING_PREFIX:
            {
                ST_UTIL_LA_FLD_DIALING_PREFIX st_LA_Fld_DialingPrefixe;
                cmbs_util_LA_Fld_DialPrefixeGet( pu8_FieldId, &st_LA_Fld_DialingPrefixe );

                memcpy(pEntry->sDialPrefix, st_LA_Fld_DialingPrefixe.pu8_Data, st_LA_Fld_DialingPrefixe.u16_DataLen);
                pEntry->sDialPrefix[st_LA_Fld_DialingPrefixe.u16_DataLen] = 0;
            }
            break;

        case CMBS_LSL_FLD_FP_MELODY:
            {
                ST_UTIL_LA_FLD_FP_MELODY st_LA_Fld_FPmelody;
                cmbs_util_LA_Fld_AssocMdyGet( pu8_FieldId, &st_LA_Fld_FPmelody );

                pEntry->u32_FPMelody = st_LA_Fld_FPmelody.u8_Value;
            }
            break;

        case CMBS_LSL_FLD_FP_VOLUME:
            {
                ST_UTIL_LA_FLD_FP_VOLUME st_LA_Fld_FPvolume;
                cmbs_util_LA_Fld_FPvolumeGet( pu8_FieldId, &st_LA_Fld_FPvolume );

                pEntry->u32_FPVolume = st_LA_Fld_FPvolume.u8_Value;
            }
            break;

        case CMBS_LSL_FLD_BLOCKED_NUMBER:
            {
                ST_UTIL_LA_FLD_BLOCKED_NUMBER st_LA_Fld_BlockedNumber;
                cmbs_util_LA_Fld_BlockedNumberGet( pu8_FieldId, &st_LA_Fld_BlockedNumber );

                memcpy(pEntry->sBlockedNumber, st_LA_Fld_BlockedNumber.pu8_Data, st_LA_Fld_BlockedNumber.u16_DataLen);
                pEntry->sBlockedNumber[st_LA_Fld_BlockedNumber.u16_DataLen] = 0;
            }

        case CMBS_LSL_FLD_MULTI_CALL_MODE:
            {
                ST_UTIL_LA_FLD_MULTI_CALL_MODE st_LA_Fld_MultiCallMode;
                cmbs_util_LA_Fld_MultiCallModeGet( pu8_FieldId, &st_LA_Fld_MultiCallMode );

                pEntry->bMultiCalls = st_LA_Fld_MultiCallMode.u8_Value == 0x31 ? TRUE : FALSE;
            }
            break;

        case CMBS_LSL_FLD_INTRUSION_CALL:
            {
                ST_UTIL_LA_FLD_INTRUSION_CALL st_LA_Fld_IntrusionCall;
                cmbs_util_LA_Fld_IntrusionCallGet( pu8_FieldId, &st_LA_Fld_IntrusionCall );

                pEntry->bIntrusionCall = st_LA_Fld_IntrusionCall.u8_Value == 0x31 ? TRUE : FALSE;
            }
            break;

        case CMBS_LSL_FLD_PERMANENT_CLIR:
            {
                ST_UTIL_LA_FLD_PERMANENT_CLIR st_LA_Fld_CLIR;
                cmbs_util_LA_Fld_PermanentCLIRGet( pu8_FieldId, &st_LA_Fld_CLIR );

                pEntry->bPermanentCLIR = st_LA_Fld_CLIR.u8_Value == 0x31 ? TRUE : FALSE;

                memcpy(pEntry->sPermanentCLIRActCode, st_LA_Fld_CLIR.pu8_ActData, st_LA_Fld_CLIR.u8_ActDataLen);
                pEntry->sPermanentCLIRActCode[st_LA_Fld_CLIR.u8_ActDataLen] = 0;

                memcpy(pEntry->sPermanentCLIRDeactCode, st_LA_Fld_CLIR.pu8_DesactData, st_LA_Fld_CLIR.u8_DesactDataLen);
                pEntry->sPermanentCLIRDeactCode[st_LA_Fld_CLIR.u8_DesactDataLen] = 0;
            }
            break;

        case CMBS_LSL_FLD_CALL_FORWARDING:
            {
                ST_UTIL_LA_FLD_CALL_FORWARDING st_LA_Fld_FW;
                cmbs_util_LA_Fld_CallForwardingGet( pu8_FieldId, &st_LA_Fld_FW );

                pEntry->bCallFwdUncond = st_LA_Fld_FW.u8_Value == 0x31 ? TRUE : FALSE;

                memcpy(pEntry->sCallFwdUncondActCode, st_LA_Fld_FW.pu8_ActData, st_LA_Fld_FW.u8_ActDataLen);
                pEntry->sCallFwdUncondActCode[st_LA_Fld_FW.u8_ActDataLen] = 0;

                memcpy(pEntry->sCallFwdUncondDeactCode, st_LA_Fld_FW.pu8_DesactData, st_LA_Fld_FW.u8_DesactDataLen);
                pEntry->sCallFwdUncondDeactCode[st_LA_Fld_FW.u8_DesactDataLen] = 0;

                memcpy(pEntry->sCallFwdUncondNum, st_LA_Fld_FW.pu8_Number, st_LA_Fld_FW.u8_NumberLen);
                pEntry->sCallFwdUncondNum[st_LA_Fld_FW.u8_NumberLen] = 0;
            }
            break;

        case CMBS_LSL_FLD_CFNA:
            {
                ST_UTIL_LA_FLD_CFNA st_LA_Fld_CFNA;
                cmbs_util_LA_Fld_CFNAGet( pu8_FieldId, &st_LA_Fld_CFNA);

                pEntry->bCallFwdNoAns = st_LA_Fld_CFNA.u8_Value == 0x31 ? TRUE : FALSE;

                memcpy(pEntry->sCallFwdNoAnsActCode, st_LA_Fld_CFNA.pu8_ActData, st_LA_Fld_CFNA.u8_ActDataLen);
                pEntry->sCallFwdNoAnsActCode[st_LA_Fld_CFNA.u8_ActDataLen] = 0;

                memcpy(pEntry->sCallFwdNoAnsDeactCode, st_LA_Fld_CFNA.pu8_DesactData, st_LA_Fld_CFNA.u8_DesactDataLen);
                pEntry->sCallFwdNoAnsDeactCode[st_LA_Fld_CFNA.u8_DesactDataLen] = 0;

                memcpy(pEntry->sCallFwdNoAnsNum, st_LA_Fld_CFNA.pu8_Number, st_LA_Fld_CFNA.u8_NumberLen);
                pEntry->sCallFwdNoAnsNum[st_LA_Fld_CFNA.u8_NumberLen] = 0;

                pEntry->u32_CallFwdNoAnsTimeout = st_LA_Fld_CFNA.u8_Timeout;
            }
            break;

        case CMBS_LSL_FLD_CFB:
            {
                ST_UTIL_LA_FLD_CFB st_LA_Fld_CFB;
                cmbs_util_LA_Fld_CFBGet( pu8_FieldId, &st_LA_Fld_CFB);

                pEntry->bCallFwdBusy = st_LA_Fld_CFB.u8_Value == 0x31 ? TRUE : FALSE;

                memcpy(pEntry->sCallFwdBusyActCode, st_LA_Fld_CFB.pu8_ActData, st_LA_Fld_CFB.u8_ActDataLen);
                pEntry->sCallFwdBusyActCode[st_LA_Fld_CFB.u8_ActDataLen] = 0;

                memcpy(pEntry->sCallFwdBusyDeactCode, st_LA_Fld_CFB.pu8_DesactData, st_LA_Fld_CFB.u8_DesactDataLen);
                pEntry->sCallFwdBusyDeactCode[st_LA_Fld_CFB.u8_DesactDataLen] = 0;

                memcpy(pEntry->sCallFwdBusyNum, st_LA_Fld_CFB.pu8_Number, st_LA_Fld_CFB.u8_NumberLen);
                pEntry->sCallFwdBusyNum[st_LA_Fld_CFB.u8_NumberLen] = 0;
            }
            break;

        default:
            break;
        }

        cmbs_util_la_ShiftIndexToNextFieldID(pu8_FieldId, &u16_Index);
    }
}

static void ParseContactListEntry(IN u16 u16_SessionIdx, OUT u8 pu8_ParsedEntry[], OUT u32 pu32_Fields[], OUT u32* pu32_FieldsNum)
{
    stContactListEntry* pEntry = (stContactListEntry*)pu8_ParsedEntry;
    u16 u16_Index = 0;
    u16 u16_EntryIdentifier;
    u16 u16_EntryLength;
    u8* pu8_FieldId;
    u8* pu8_BufferToParse;
    bool bContactNumber = FALSE;

    pu8_BufferToParse = g_pst_LASessions[u16_SessionIdx].pu8_DataBuffer;

    *pu32_FieldsNum = 0;

    /* Get EntryId, Entry Length */
    cmbs_util_LA_EntryLenght_EntryIdentifierGet(pu8_BufferToParse, &u16_EntryIdentifier, &u16_EntryLength, &u16_Index);

    while ( u16_Index < u16_EntryLength )
    {
        pu8_FieldId = pu8_BufferToParse + u16_Index;

        pu32_Fields[(*pu32_FieldsNum)++] = CATiqFieldId_2_AppFieldId(g_pst_LASessions[u16_SessionIdx].u32_ListId, *pu8_FieldId);

        switch ( *pu8_FieldId )
        {
        case CMBS_CL_FLD_NAME:
            {
                ST_UTIL_LA_FLD_NAME st_LA_Fld_Name;
                cmbs_util_LA_Fld_NameGet( pu8_FieldId, &st_LA_Fld_Name );

                memcpy(pEntry->sLastName, st_LA_Fld_Name.pu8_Data, st_LA_Fld_Name.u16_DataLen);
                pEntry->sLastName[st_LA_Fld_Name.u16_DataLen] = 0;
            }
            break;

        case CMBS_CL_FLD_FIRST_NAME:
            {
                ST_UTIL_LA_FLD_FIRST_NAME st_LA_Fld_FirstName;
                cmbs_util_LA_Fld_FirstNameGet( pu8_FieldId, &st_LA_Fld_FirstName );

                memcpy(pEntry->sFirstName, st_LA_Fld_FirstName.pu8_Data, st_LA_Fld_FirstName.u16_DataLen);
                pEntry->sFirstName[st_LA_Fld_FirstName.u16_DataLen] = 0;
            }
            break;

        case CMBS_CL_FLD_CONTACT_NR:
            {
                ST_UTIL_LA_FLD_CONTACT_NR st_LA_Fld_ContactNumber;
                cmbs_util_LA_Fld_ContactNrGet( pu8_FieldId, &st_LA_Fld_ContactNumber);

                if ( bContactNumber == FALSE )
                {
                    /* First Number */
                    memcpy(pEntry->sNumber1, st_LA_Fld_ContactNumber.pu8_Data, st_LA_Fld_ContactNumber.u16_DataLen);
                    pEntry->sNumber1[st_LA_Fld_ContactNumber.u16_DataLen] = 0;

                    pEntry->bNumber1Default = st_LA_Fld_ContactNumber.u8_Attribute & 0x20;
                    pEntry->bNumber1Own     = st_LA_Fld_ContactNumber.u8_Attribute & 0x10;

                    if ( st_LA_Fld_ContactNumber.u8_Attribute & 0x08 )
                    {
                        pEntry->cNumber1Type = NUM_TYPE_FIXED;
                    }
                    else if ( st_LA_Fld_ContactNumber.u8_Attribute & 0x04 )
                    {
                        pEntry->cNumber1Type = NUM_TYPE_MOBILE;
                    }
                    else
                    {
                        pEntry->cNumber1Type = NUM_TYPE_WORK;
                    }
                }
                else
                {
                    /* Second number */
                    memcpy(pEntry->sNumber2, st_LA_Fld_ContactNumber.pu8_Data, st_LA_Fld_ContactNumber.u16_DataLen);
                    pEntry->sNumber2[st_LA_Fld_ContactNumber.u16_DataLen] = 0;

                    pEntry->bNumber2Default = st_LA_Fld_ContactNumber.u8_Attribute & 0x20;
                    pEntry->bNumber2Own     = st_LA_Fld_ContactNumber.u8_Attribute & 0x10;

                    if ( st_LA_Fld_ContactNumber.u8_Attribute & 0x08 )
                    {
                        pEntry->cNumber2Type = NUM_TYPE_FIXED;
                    }
                    else if ( st_LA_Fld_ContactNumber.u8_Attribute & 0x04 )
                    {
                        pEntry->cNumber2Type = NUM_TYPE_MOBILE;
                    }
                    else
                    {
                        pEntry->cNumber2Type = NUM_TYPE_WORK;
                    }

                    /* overwrite num1 field id with num2 field id */
                    pu32_Fields[*pu32_FieldsNum - 1] = FIELD_ID_CONTACT_NUM_2;
                }

                bContactNumber = TRUE;
            }
            break;

        case CMBS_CL_FLD_ASSOC_MDY:
            {
                ST_UTIL_LA_FLD_ASSOC_MDY st_LA_Fld_Associated_melody;
                cmbs_util_LA_Fld_AssocMdyGet( pu8_FieldId, &st_LA_Fld_Associated_melody );

                pEntry->u32_AssociatedMelody = st_LA_Fld_Associated_melody.u8_Value;
            }
            break;

        case CMBS_CL_FLD_LINE_ID:
            {
                ST_UTIL_LA_FLD_LINE_ID st_LA_Fld_LineId;
                cmbs_util_LA_Fld_LineIdGet( pu8_FieldId, &st_LA_Fld_LineId );

                pEntry->u32_LineId = st_LA_Fld_LineId.u16_LineNumber;
            }
            break;

        default:
            break;
        }

        cmbs_util_la_ShiftIndexToNextFieldID(pu8_FieldId, &u16_Index);
    }
}

static void UseDefaultSort(IN u16 u16_SessionIdx, OUT PST_IE_LA_FIELDS pst_SortFields)
{
    u32 u32_ListId = g_pst_LASessions[u16_SessionIdx].u32_ListId;

    pst_SortFields->u16_Length = 1;

    switch ( u32_ListId )
    {
    case LIST_TYPE_CONTACT_LIST:
        g_pst_LASessions[u16_SessionIdx].u32_CATiqSortField1 = pst_SortFields->pu16_FieldId[0] = CMBS_CL_FLD_NAME;
        g_pst_LASessions[u16_SessionIdx].u32_SortField1 = FIELD_ID_LAST_NAME;
        break;

    case LIST_TYPE_LINE_SETTINGS_LIST:
        g_pst_LASessions[u16_SessionIdx].u32_CATiqSortField1 = pst_SortFields->pu16_FieldId[0] = CMBS_LSL_FLD_LINE_ID;
        g_pst_LASessions[u16_SessionIdx].u32_SortField1 = FIELD_ID_LINE_ID;
        break;

    case LIST_TYPE_MISSED_CALLS:
        g_pst_LASessions[u16_SessionIdx].u32_CATiqSortField1 = pst_SortFields->pu16_FieldId[0] = CMBS_MCL_FLD_DATETIME;
        g_pst_LASessions[u16_SessionIdx].u32_SortField1 = FIELD_ID_DATE_AND_TIME;
        break;

    case LIST_TYPE_OUTGOING_CALLS:
        g_pst_LASessions[u16_SessionIdx].u32_CATiqSortField1 = pst_SortFields->pu16_FieldId[0] = CMBS_OCL_FLD_DATETIME;
        g_pst_LASessions[u16_SessionIdx].u32_SortField1 = FIELD_ID_DATE_AND_TIME;
        break;
    case LIST_TYPE_INCOMING_ACCEPTED_CALLS:
        g_pst_LASessions[u16_SessionIdx].u32_CATiqSortField1 = pst_SortFields->pu16_FieldId[0] = CMBS_IACL_FLD_DATETIME;
        g_pst_LASessions[u16_SessionIdx].u32_SortField1 = FIELD_ID_DATE_AND_TIME;
        break;
    case LIST_TYPE_ALL_CALLS:
        g_pst_LASessions[u16_SessionIdx].u32_CATiqSortField1 = pst_SortFields->pu16_FieldId[0] = CMBS_ACL_FLD_DATETIME;
        g_pst_LASessions[u16_SessionIdx].u32_SortField1 = FIELD_ID_DATE_AND_TIME;
        break;
    case LIST_TYPE_ALL_INCOMING_CALLS:
        g_pst_LASessions[u16_SessionIdx].u32_CATiqSortField1 = pst_SortFields->pu16_FieldId[0] = CMBS_AICL_FLD_DATETIME;
        g_pst_LASessions[u16_SessionIdx].u32_SortField1 = FIELD_ID_DATE_AND_TIME;
        break;
    default:
        break;
    }
}

static void SortArray(u16 u16_Array[], u16 u16_Length)
{
    s16 u16_i, u16_j, u16_temp;

    for ( u16_i = u16_Length - 1; u16_i >= 0; --u16_i )
    {
        for ( u16_j = 1; u16_j <= u16_i; ++u16_j )
        {
            if ( u16_Array[u16_j - 1] > u16_Array[u16_j] )
            {
                u16_temp = u16_Array[u16_j - 1];
                u16_Array[u16_j - 1] = u16_Array[u16_j];
                u16_Array[u16_j] = u16_temp;
            }
        }
    }
}

static void InitSession(u32 u32_SessionIdx)
{
    memset(&g_pst_LASessions[u32_SessionIdx], 0, sizeof(g_pst_LASessions[u32_SessionIdx]));
    g_pst_LASessions[u32_SessionIdx].u32_SessionId = LA_SESSION_UNDEFINED_SESSION_ID;
    g_pst_LASessions[u32_SessionIdx].u32_CATiqSortField1 = CMBS_FLD_UNDEFINED;
    g_pst_LASessions[u32_SessionIdx].u32_CATiqSortField2 = CMBS_FLD_UNDEFINED;
    g_pst_LASessions[u32_SessionIdx].u32_SortField1 = FIELD_ID_INVALID;
    g_pst_LASessions[u32_SessionIdx].u32_SortField2 = FIELD_ID_INVALID;
    g_pst_LASessions[u32_SessionIdx].u16_DataBytesMarker = 0;
    g_pst_LASessions[u32_SessionIdx].u16_DataTotalLen = 0;
}

static void StoreNumOfUnreadMissedCalls(IN u32 u32_SessionIdx, IN bool bBefore)
{
    u32 u32_NumOfLines, u32_Index, u32_NumOfMissedCalls;

    List_GetCount(LIST_TYPE_LINE_SETTINGS_LIST, &u32_NumOfLines);
    List_GetCount(LIST_TYPE_MISSED_CALLS, &u32_NumOfMissedCalls);

    if ( u32_NumOfLines > APPCALL_LINEOBJ_MAX )
    {
        printf("ERROR: Too many lines!\n");
        return;
    }

    if ( bBefore )
    {
        /* Prepare list of Line IDs */
        for ( u32_Index = 1; u32_Index <= u32_NumOfLines; ++u32_Index )
        {
            u32 u32_FieldId = FIELD_ID_LINE_ID, u32_EntrySize = 1;
            stLineSettingsListEntry st_Entry;

            List_ReadEntries(LIST_TYPE_LINE_SETTINGS_LIST, u32_Index, TRUE, MARK_LEAVE_UNCHANGED, &u32_FieldId, 1,
                             FIELD_ID_LINE_ID, FIELD_ID_INVALID, &st_Entry, &u32_EntrySize);

            g_pst_LASessions[u32_SessionIdx].pst_MissedCallNotifAux[u32_Index - 1].u32_LineId = st_Entry.u32_LineId;
            g_pst_LASessions[u32_SessionIdx].pst_MissedCallNotifAux[u32_Index - 1].u32_NumOfCallsAfter = 0;
            g_pst_LASessions[u32_SessionIdx].pst_MissedCallNotifAux[u32_Index - 1].u32_NumOfCallsBefore = 0;
        }
    }

    /* Run on Missed call list and count unread messages per line */
    for ( u32_Index = 1; u32_Index <= u32_NumOfMissedCalls; ++u32_Index )
    {
        u32 pu32_FieldsIds[2] = { FIELD_ID_LINE_ID, FIELD_ID_READ_STATUS };
        u32 u32_EntrySize = 1, u32_LineIdx;
        stCallsListEntry st_Entry;

        List_ReadEntries(LIST_TYPE_MISSED_CALLS, u32_Index, TRUE, MARK_LEAVE_UNCHANGED,  pu32_FieldsIds, 2,
                         FIELD_ID_LINE_ID, FIELD_ID_INVALID, &st_Entry, &u32_EntrySize);

        if ( st_Entry.bRead == FALSE )
        {
            /* Increase ammount of unread entries for the line */
            for ( u32_LineIdx = 0; u32_LineIdx < u32_NumOfLines; ++u32_LineIdx )
            {
                if ( g_pst_LASessions[u32_SessionIdx].pst_MissedCallNotifAux[u32_LineIdx].u32_LineId == st_Entry.u32_LineId )
                {
                    if ( bBefore )
                    {
                        ++(g_pst_LASessions[u32_SessionIdx].pst_MissedCallNotifAux[u32_LineIdx].u32_NumOfCallsBefore);
                    }
                    else
                    {
                        ++(g_pst_LASessions[u32_SessionIdx].pst_MissedCallNotifAux[u32_LineIdx].u32_NumOfCallsAfter);
                    }
                }
            }
        }
    }
}

static void SendLineSettingsListChangeNotification(IN u32 u32_LineID)
{
    u32 u32_HandsetMask;

    List_GetAttachedHs(u32_LineID, &u32_HandsetMask);

    ListChangeNotif_ListChanged( u32_LineID, LINE_TYPE_RELATING_TO, u32_HandsetMask, 1, LIST_CHANGE_NOTIF_LIST_TYPE_LINE_SETTINGS_LIST );
}

static u32 LineSettingsGetLineIdByEntryId(IN u32 u32_EntryId)
{
    u32 u32_FieldId = FIELD_ID_LINE_ID;
    stLineSettingsListEntry st_Entry;
    List_ReadEntryById(LIST_TYPE_LINE_SETTINGS_LIST, u32_EntryId, &u32_FieldId, 1, &st_Entry);

    return st_Entry.u32_LineId;
}

static u32 MissedCallGetLineIdByEntryId(IN u32 u32_EntryId)
{
    u32 u32_FieldId = FIELD_ID_LINE_ID;
    stCallsListEntry st_Entry;
    List_ReadEntryById(LIST_TYPE_MISSED_CALLS, u32_EntryId, &u32_FieldId, 1, &st_Entry);

    return st_Entry.u32_LineId;
}

/* ***************** Auxiliary end ***************** */


/*******************************************
Session Mgr API
********************************************/
LA_SESSION_MGR_RC LASessionMgr_Init(void)
{
    LIST_RC list_rc;
    u32 u32_Index;

    /* Init lists */
    list_rc = List_Init();
    if ( list_rc != LIST_RC_OK )
    {
        return LA_SESSION_MGR_RC_FAIL;
    }

    /* create lists */
    for ( u32_Index = LIST_TYPE_CONTACT_LIST; u32_Index < LIST_TYPE_MAX; ++u32_Index )
    {
        list_rc = List_CreateList(u32_Index);
        if ( list_rc == LIST_RC_UNSUPPORTED_LIST )
        {
            return LA_SESSION_MGR_RC_UNSUPPORTED_LIST;
        }
    }

    /* Init sessions */
    for ( u32_Index = 0; u32_Index < LA_SESSION_MGR_MAX_SESSIONS; ++u32_Index )
    {
        InitSession(u32_Index);
    }

    return LA_SESSION_MGR_RC_OK;
}

LA_SESSION_MGR_RC LASessionMgr_GetDataBuffer(IN u16 u16_SessionId, OUT u8** ppu8_DataBuffer, OUT u16** ppu16_Marker, OUT u16** ppu16_Len)
{
    s32 s32_Index;
    CHECK_SESSION_ID(u16_SessionId);

    *ppu8_DataBuffer = g_pst_LASessions[s32_Index].pu8_DataBuffer;
    *ppu16_Marker    = &(g_pst_LASessions[s32_Index].u16_DataBytesMarker);
    *ppu16_Len       = &(g_pst_LASessions[s32_Index].u16_DataTotalLen);

    return LA_SESSION_MGR_RC_OK;
}

LA_SESSION_MGR_RC LASessionMgr_StartSession(IN u16 u16_SessionId, IN u16 u16_ListId, IN u16 u16_HsId, IN PST_IE_LA_FIELDS pst_SortFields,
                                            OUT u16* pu16_CurrNumOfEntries)
{
    u32 u32_NumEntries;

    /* find free session object */
    s32 s32_Index = GetIndexBySessionId(LA_SESSION_UNDEFINED_SESSION_ID);
    if ( s32_Index == -1 )
    {
        printf("LASessionMgr_StartSession: no more free sessions!\n");
        return LA_SESSION_MGR_RC_NO_FREE_SESSIONS;
    }

    /* Fill in session parameters */
    g_pst_LASessions[s32_Index].u32_SessionId = u16_SessionId;
    g_pst_LASessions[s32_Index].u32_CATiqListId = u16_ListId;
    g_pst_LASessions[s32_Index].u32_ListId = CATiqListId_2_AppListId(u16_ListId);
    g_pst_LASessions[s32_Index].u32_HsId = u16_HsId;

    if ( pst_SortFields->u16_Length > 0 )
    {
        g_pst_LASessions[s32_Index].u32_CATiqSortField1 = pst_SortFields->pu16_FieldId[0];
        g_pst_LASessions[s32_Index].u32_SortField1 = CATiqFieldId_2_AppFieldId(g_pst_LASessions[s32_Index].u32_ListId, pst_SortFields->pu16_FieldId[0]);
    }
    if ( pst_SortFields->u16_Length > 1 )
    {
        g_pst_LASessions[s32_Index].u32_CATiqSortField2 = pst_SortFields->pu16_FieldId[1];
        g_pst_LASessions[s32_Index].u32_SortField2 = CATiqFieldId_2_AppFieldId(g_pst_LASessions[s32_Index].u32_ListId, pst_SortFields->pu16_FieldId[1]);
    }
    if ( pst_SortFields->u16_Length == 0 )
    {
        UseDefaultSort(s32_Index, pst_SortFields);
    }

    List_GetCount(g_pst_LASessions[s32_Index].u32_ListId, &u32_NumEntries);
    *pu16_CurrNumOfEntries = (u16)u32_NumEntries;

    return LA_SESSION_MGR_RC_OK;
}

LA_SESSION_MGR_RC LASessionMgr_EndSession(IN u16 u16_SessionId)
{
    s32 s32_Index;
    CHECK_SESSION_ID(u16_SessionId);

    InitSession(s32_Index);

    return LA_SESSION_MGR_RC_OK;
}

LA_SESSION_MGR_RC LASessionMgr_GetSupportedFields(IN u16 u16_SessionId,
                                                  OUT PST_IE_LA_FIELDS pst_EditableFields,
                                                  OUT PST_IE_LA_FIELDS pst_NonEditableFields)
{
    u32 pu32_EditableFields[LA_SESSION_MGR_MAX_FIELDS], pu32_NonEditableFields[LA_SESSION_MGR_MAX_FIELDS],
    u32_EditableNum = LA_SESSION_MGR_MAX_FIELDS, u32_NonEditableNum = LA_SESSION_MGR_MAX_FIELDS;

    u32 u32_i, u32_ListId;

    s32 s32_FieldId;

    s32 s32_Index;
    CHECK_SESSION_ID(u16_SessionId);

    u32_ListId = g_pst_LASessions[s32_Index].u32_ListId;

    List_GetSupportedFields(u32_ListId, pu32_EditableFields, &u32_EditableNum, pu32_NonEditableFields, &u32_NonEditableNum);

    pst_EditableFields->u16_Length = pst_NonEditableFields->u16_Length = 0;
    for ( u32_i = 0; u32_i < u32_EditableNum; ++u32_i )
    {
        if ( (s32_FieldId = AppFieldId_2_CATiqFieldId(u32_ListId, pu32_EditableFields[u32_i])) != CMBS_FLD_UNDEFINED )
        {
            pst_EditableFields->pu16_FieldId[pst_EditableFields->u16_Length++] = s32_FieldId;
        }
    }
    for ( u32_i = 0; u32_i < u32_NonEditableNum; ++u32_i )
    {
        if ( (s32_FieldId = AppFieldId_2_CATiqFieldId(u32_ListId, pu32_NonEditableFields[u32_i])) != CMBS_FLD_UNDEFINED )
        {
            pst_NonEditableFields->pu16_FieldId[pst_NonEditableFields->u16_Length++] = s32_FieldId;
        }
    }

    SortArray(pst_EditableFields->pu16_FieldId,     pst_EditableFields->u16_Length);
    SortArray(pst_NonEditableFields->pu16_FieldId,  pst_NonEditableFields->u16_Length);

    return LA_SESSION_MGR_RC_OK;
}

LA_SESSION_MGR_RC LASessionMgr_ReadEntries(IN u16 u16_SessionId, IN u16 u16_StartIdx, IN bool bForward, IN E_CMBS_MARK_REQUEST eMark,
                                           IN PST_IE_LA_FIELDS pst_RequestedFields, INOUT u16* pu16_NumOfReqEntries,
                                           OUT u8 pu8_Data[], INOUT u16* pu16_DataLen)
{
    u32 u32_ListId, pu32_Fields[LA_SESSION_MGR_MAX_FIELDS], u32_FieldsNum, u32_NumReqEntries = *pu16_NumOfReqEntries, u32_Count;
    u8 pu8_Entries[LA_SESSION_MGR_MAX_ENTRIES_PER_READ * LIST_ENTRY_MAX_SIZE];
    eMarkRequest eReadMark;
    LIST_RC list_rc;

    s32 s32_Index;
    CHECK_SESSION_ID(u16_SessionId);

    if ( *pu16_NumOfReqEntries > LA_SESSION_MGR_MAX_ENTRIES_PER_READ )
    {
        return LA_SESSION_MGR_RC_NOT_ENOUGH_MEMORY;
    }

    u32_ListId = g_pst_LASessions[s32_Index].u32_ListId;

    List_GetCount(u32_ListId, &u32_Count);
    if ( u16_StartIdx < 1 || u16_StartIdx > u32_Count )
    {
        return LA_SESSION_MGR_RC_INVALID_START_INDEX;
    }

    PrepareFieldsForRead(u32_ListId, pst_RequestedFields, pu32_Fields, &u32_FieldsNum);

    switch ( eMark )
    {
    case CMBS_MARK_REQUEST_SET_READ_STATUS:     eReadMark = MARK_SET;               break;
    case CMBS_MARK_REQUEST_RESET_READ_STATUS:   eReadMark = MARK_CLEAR;             break;
    default:                                    eReadMark = MARK_LEAVE_UNCHANGED;   break;
    }

    /* Monitor read entries for Missed Call Notification */
    if ( (eReadMark != MARK_LEAVE_UNCHANGED) && (u32_ListId == LIST_TYPE_MISSED_CALLS) )
    {
        StoreNumOfUnreadMissedCalls(s32_Index, TRUE);
    }

    list_rc = List_ReadEntries(u32_ListId, u16_StartIdx, bForward, eReadMark, pu32_Fields, u32_FieldsNum,
                               g_pst_LASessions[s32_Index].u32_SortField1, g_pst_LASessions[s32_Index].u32_SortField2,
                               pu8_Entries, &u32_NumReqEntries);

    if ( (eReadMark != MARK_LEAVE_UNCHANGED) && (u32_ListId == LIST_TYPE_MISSED_CALLS) )
    {
        StoreNumOfUnreadMissedCalls(s32_Index, FALSE);
    }

    *pu16_NumOfReqEntries = (u16)u32_NumReqEntries;

    if ( list_rc == LIST_RC_FAIL )
    {
        return LA_SESSION_MGR_RC_FAIL;
    }

    ConvertEntriesToCatiqDataBuffer(u32_ListId, pst_RequestedFields, pu8_Entries, *pu16_NumOfReqEntries, pu8_Data, pu16_DataLen);

    return LA_SESSION_MGR_RC_OK;
}

LA_SESSION_MGR_RC LASessionMgr_SearchEntries(IN u16 u16_SessionId, IN E_CMBS_LA_SEARCH_MATCHING eMatch, IN bool bCaseSensitive,
                                             IN const char* s_SearchedValue, IN bool bForward, IN E_CMBS_MARK_REQUEST eMark,
                                             IN PST_IE_LA_FIELDS pst_RequestedFields, INOUT u16* pu16_NumOfReqEntries,
                                             OUT u8 pu8_Data[], INOUT u16* pu16_DataLen, OUT u32* pu32_StartIdx)
{
    u32 u32_ListId, pu32_Fields[LA_SESSION_MGR_MAX_FIELDS], u32_FieldsNum, u32_NumOfReqEntries = *pu16_NumOfReqEntries;
    u8 pu8_Entries[LA_SESSION_MGR_MAX_ENTRIES_PER_READ * LIST_ENTRY_MAX_SIZE];
    eMarkRequest eReadMark;
    eMatchOption eSearchMatch;
    LIST_RC list_rc;

    s32 s32_Index;
    CHECK_SESSION_ID(u16_SessionId);

    if ( *pu16_NumOfReqEntries > LA_SESSION_MGR_MAX_ENTRIES_PER_READ )
    {
        return LA_SESSION_MGR_RC_NOT_ENOUGH_MEMORY;
    }

    u32_ListId = g_pst_LASessions[s32_Index].u32_ListId;

    PrepareFieldsForRead(u32_ListId, pst_RequestedFields, pu32_Fields, &u32_FieldsNum);

    switch ( eMark )
    {
    case CMBS_MARK_REQUEST_SET_READ_STATUS:     eReadMark = MARK_SET;               break;
    case CMBS_MARK_REQUEST_RESET_READ_STATUS:   eReadMark = MARK_CLEAR;             break;
    default:                                    eReadMark = MARK_LEAVE_UNCHANGED;   break;
    }

    switch ( eMatch )
    {
    case CMBS_LA_SEARCH_MATCHING_EXACT_NEXT:    eSearchMatch = MATCH_RETURN_NEXT_ON_FAIL;   break;
    case CMBS_LA_SEARCH_MATCHING_EXACT_PREV:    eSearchMatch = MATCH_RETURN_PREV_ON_FAIL;   break;
    default:                                    eSearchMatch = MATCH_EXACT;                 break;
    }

    /* Monitor read entries for Missed Call Notification */
    if ( (eReadMark != MARK_LEAVE_UNCHANGED) && (u32_ListId == LIST_TYPE_MISSED_CALLS) )
    {
        StoreNumOfUnreadMissedCalls(s32_Index, TRUE);
    }

    list_rc = List_SearchEntries(u32_ListId, eSearchMatch, bCaseSensitive, s_SearchedValue, bForward, eReadMark, pu32_Fields, u32_FieldsNum, 
                                 g_pst_LASessions[s32_Index].u32_SortField1, g_pst_LASessions[s32_Index].u32_SortField2,
                                 pu8_Entries, &u32_NumOfReqEntries, pu32_StartIdx);

    if ( (eReadMark != MARK_LEAVE_UNCHANGED) && (u32_ListId == LIST_TYPE_MISSED_CALLS) )
    {
        StoreNumOfUnreadMissedCalls(s32_Index, FALSE);
    }

    if ( list_rc == LIST_RC_FAIL )
    {
        return LA_SESSION_MGR_RC_FAIL;
    }

    *pu16_NumOfReqEntries = u32_NumOfReqEntries;

    ConvertEntriesToCatiqDataBuffer(u32_ListId, pst_RequestedFields, pu8_Entries, *pu16_NumOfReqEntries, pu8_Data, pu16_DataLen);

    return LA_SESSION_MGR_RC_OK;
}

LA_SESSION_MGR_RC LASessionMgr_SetSaveEntryID(IN u16 u16_SessionId, IN u32 u32_EntryId)
{
    s32 s32_Index;
    CHECK_SESSION_ID(u16_SessionId);

    g_pst_LASessions[s32_Index].u32_SaveEntryId = u32_EntryId;

    g_pst_LASessions[s32_Index].u16_DataBytesMarker = g_pst_LASessions[s32_Index].u16_DataTotalLen = 0;

    return LA_SESSION_MGR_RC_OK;
}

LA_SESSION_MGR_RC LASessionMgr_SaveEntry(IN u16 u16_SessionId, OUT u32* pu32_EntryId, OUT u32* pu32_PositionIdx, OUT u32* pu32_TotalNumEntries)
{
    u8 pu8_Entry[LIST_ENTRY_MAX_SIZE];
    u32 u32_listId, pu32_Fields[LA_SESSION_MGR_MAX_FIELDS], u32_FieldsNum;
    LIST_RC list_rc;

    s32 s32_Index;
    CHECK_SESSION_ID(u16_SessionId);

    u32_listId = g_pst_LASessions[s32_Index].u32_ListId;

    switch ( u32_listId )
    {
    case LIST_TYPE_MISSED_CALLS:
    case LIST_TYPE_ALL_INCOMING_CALLS:
        ParseCallListEntry(s32_Index, pu8_Entry, pu32_Fields, &u32_FieldsNum);
        break;

    case LIST_TYPE_OUTGOING_CALLS:
    case LIST_TYPE_INCOMING_ACCEPTED_CALLS:
    case LIST_TYPE_ALL_CALLS:
        /* No Writeable fields in this list */
        return LA_SESSION_MGR_RC_NOT_ALLOWED;

    case LIST_TYPE_LINE_SETTINGS_LIST:
        ParseLineSettingsEntry(s32_Index, pu8_Entry, pu32_Fields, &u32_FieldsNum);
        break;

    case LIST_TYPE_CONTACT_LIST:
        ParseContactListEntry(s32_Index, pu8_Entry, pu32_Fields, &u32_FieldsNum);
        break;

    default:
        break;
    }

    if ( g_pst_LASessions[s32_Index].u32_SaveEntryId == 0 )
    {
        if ( u32_listId == LIST_TYPE_LINE_SETTINGS_LIST || u32_listId == LIST_TYPE_CONTACT_LIST )
        {
            /* New Entry */
            list_rc = List_InsertEntry(u32_listId, pu8_Entry, pu32_Fields, u32_FieldsNum, pu32_EntryId);
            if ( list_rc != LIST_RC_OK )
            {
                return LA_SESSION_MGR_RC_FAIL;
            }
        }
        else
        {
            return LA_SESSION_MGR_RC_NOT_ALLOWED;
        }
    }
    else
    {
        list_rc = List_UpdateEntry(u32_listId, pu8_Entry, pu32_Fields, u32_FieldsNum, g_pst_LASessions[s32_Index].u32_SaveEntryId);
        *pu32_EntryId = g_pst_LASessions[s32_Index].u32_SaveEntryId;
        if ( list_rc != LIST_RC_OK )
        {
            return LA_SESSION_MGR_RC_ENTRY_NOT_AVAILABLE;
        }
    }

    /* Position Index */
    List_GetEntryIdex(u32_listId, g_pst_LASessions[s32_Index].u32_SortField1, g_pst_LASessions[s32_Index].u32_SortField2,
                      *pu32_EntryId, pu32_PositionIdx);

    /* Entry Num */
    List_GetCount(u32_listId, pu32_TotalNumEntries);

    return LA_SESSION_MGR_RC_OK;
}


LA_SESSION_MGR_RC LASessionMgr_EditEntry(IN u16 u16_SessionId, IN u32 u32_EntryId,
                                         IN PST_IE_LA_FIELDS pst_RequestedFields, OUT u8 pu8_Data[], INOUT u16* pu16_DataLen)
{
    u32 u32_ListId, pu32_Fields[LA_SESSION_MGR_MAX_FIELDS], u32_FieldsNum;
    u8 pu8_Entry[LIST_ENTRY_MAX_SIZE];
    LIST_RC list_rc;

    s32 s32_Index;
    CHECK_SESSION_ID(u16_SessionId);

    u32_ListId = g_pst_LASessions[s32_Index].u32_ListId;

    PrepareFieldsForRead(u32_ListId, pst_RequestedFields, pu32_Fields, &u32_FieldsNum);

    list_rc = List_ReadEntryById(u32_ListId, u32_EntryId, pu32_Fields, u32_FieldsNum, pu8_Entry);

    if ( list_rc == LIST_RC_FAIL )
    {
        return LA_SESSION_MGR_RC_FAIL;
    }
    else if ( list_rc == LIST_RC_ENTRY_NOT_AVAILABLE )
    {
        return LA_SESSION_MGR_RC_ENTRY_NOT_AVAILABLE;
    }

    ConvertEntriesToCatiqDataBuffer(u32_ListId, pst_RequestedFields, pu8_Entry, 1, pu8_Data, pu16_DataLen);

    return LA_SESSION_MGR_RC_OK;
}

LA_SESSION_MGR_RC LASessionMgr_DeleteEntry(IN u16 u16_SessionId, IN u16 u16_EntryId, OUT u16* pu16_NumOfEntries)
{
    u32 u32_ListId, u32_EntryCount;
    LIST_RC list_rc;

    s32 s32_Index;
    CHECK_SESSION_ID(u16_SessionId);

    u32_ListId = g_pst_LASessions[s32_Index].u32_ListId;

    /* For list change notification */
    if ( u32_ListId == LIST_TYPE_MISSED_CALLS || u32_ListId == LIST_TYPE_LINE_SETTINGS_LIST )
    {
        if ( u32_ListId == LIST_TYPE_MISSED_CALLS )
        {
            g_pst_LASessions[s32_Index].pu32_LineIdsOfDeletedEntries[0] = MissedCallGetLineIdByEntryId(u16_EntryId);
        }
        else
        {
            g_pst_LASessions[s32_Index].pu32_LineIdsOfDeletedEntries[0] = LineSettingsGetLineIdByEntryId(u16_EntryId);
        }

        List_GetAttachedHs(g_pst_LASessions[s32_Index].pu32_LineIdsOfDeletedEntries[0],
                           &g_pst_LASessions[s32_Index].pu32_AttachedHsOfDeletedEntries[0]);

        g_pst_LASessions[s32_Index].u32_LineIdsSize = 1;
    }  

    /* Delete Entry */
    list_rc = List_DeleteEntry(u32_ListId, u16_EntryId);

    if ( list_rc == LIST_RC_FAIL )
    {
        return LA_SESSION_MGR_RC_ENTRY_NOT_AVAILABLE;
    }

    List_GetCount(u32_ListId, &u32_EntryCount);
    *pu16_NumOfEntries = (u16)u32_EntryCount;

    return LA_SESSION_MGR_RC_OK;
}

LA_SESSION_MGR_RC LASessionMgr_DeleteAllEntries(IN u16 u16_SessionId)
{
    u32 u32_ListId;

    s32 s32_Index;
    CHECK_SESSION_ID(u16_SessionId);

    u32_ListId = g_pst_LASessions[s32_Index].u32_ListId;

    if ( u32_ListId == LIST_TYPE_LINE_SETTINGS_LIST )
    {
        return LA_SESSION_MGR_RC_NOT_ALLOWED;
    }

    /* For missed call notification */
    if ( u32_ListId == LIST_TYPE_MISSED_CALLS )
    {
        u32 u32_EntryCount, u32_Index, u32_FieldId = FIELD_ID_LINE_ID, u32_EntrySize = 1, u32_DifferentLineIds = 0;

        List_GetCount(u32_ListId, &u32_EntryCount);

        for ( u32_Index = 1; u32_Index <= u32_EntryCount; ++u32_Index )
        {
            stCallsListEntry st_Entry;

            List_ReadEntries(u32_ListId, u32_Index, TRUE, MARK_LEAVE_UNCHANGED, &u32_FieldId, 1, FIELD_ID_LINE_ID, FIELD_ID_INVALID,
                             &st_Entry, &u32_EntrySize);

            if ( (u32_DifferentLineIds == 0) || 
                 ((u32_DifferentLineIds > 0) && (g_pst_LASessions[s32_Index].pu32_LineIdsOfDeletedEntries[u32_DifferentLineIds - 1] != st_Entry.u32_LineId)) )
            {
                g_pst_LASessions[s32_Index].pu32_LineIdsOfDeletedEntries[u32_DifferentLineIds] = st_Entry.u32_LineId;
    
                List_GetAttachedHs(st_Entry.u32_LineId, &g_pst_LASessions[s32_Index].pu32_AttachedHsOfDeletedEntries[u32_DifferentLineIds++]);
            }
        }

        g_pst_LASessions[s32_Index].u32_LineIdsSize = u32_DifferentLineIds;
    }  

    List_DeleteAllEntries(u32_ListId);

    return LA_SESSION_MGR_RC_OK;
}

LA_SESSION_MGR_RC LASessionMgr_SendMissedCallNotification(IN u16 u16_SessionId, IN E_CMBS_MARK_REQUEST eMark)
{
    u32 u32_NumOfLines, u32_Index;

    s32 s32_Index;
    CHECK_SESSION_ID(u16_SessionId);

    if ( g_pst_LASessions[s32_Index].u32_ListId == LIST_TYPE_MISSED_CALLS &&
         eMark != CMBS_MARK_REQUEST_LEAVE_UNCHANGED )
    {
    
        List_GetCount(LIST_TYPE_LINE_SETTINGS_LIST, &u32_NumOfLines);
    
        for ( u32_Index = 0; u32_Index < u32_NumOfLines; ++u32_Index )
        {
            if ( g_pst_LASessions[s32_Index].pst_MissedCallNotifAux[u32_Index].u32_NumOfCallsBefore != 
                 g_pst_LASessions[s32_Index].pst_MissedCallNotifAux[u32_Index].u32_NumOfCallsAfter )
            {
                /* Need to send missed call notification */
                ListChangeNotif_MissedCallListChanged(g_pst_LASessions[s32_Index].pst_MissedCallNotifAux[u32_Index].u32_LineId, FALSE);
            }
        }
    }

    return LA_SESSION_MGR_RC_OK;
}

LA_SESSION_MGR_RC LASessionMgr_SendListChangedNotification(IN u16 u16_SessionId, IN u16 u16_EntryId)
{
    s32 s32_Index;
    CHECK_SESSION_ID(u16_SessionId);

    if ( g_pst_LASessions[s32_Index].u32_ListId == LIST_TYPE_LINE_SETTINGS_LIST )
    {
        u32 u32_LineId = LineSettingsGetLineIdByEntryId(u16_EntryId);
        SendLineSettingsListChangeNotification(u32_LineId);
    }
    else if ( g_pst_LASessions[s32_Index].u32_ListId == LIST_TYPE_MISSED_CALLS )
    {
        u32 u32_LineId = MissedCallGetLineIdByEntryId(u16_EntryId);
        ListChangeNotif_MissedCallListChanged(u32_LineId, FALSE);
    }

    return LA_SESSION_MGR_RC_OK;
}

LA_SESSION_MGR_RC LASessionMgr_SendListChangedNotificationOnDelete(IN u16 u16_SessionId)
{
    s32 s32_Index;
    CHECK_SESSION_ID(u16_SessionId);

    if ( g_pst_LASessions[s32_Index].u32_ListId == LIST_TYPE_LINE_SETTINGS_LIST )
    {
        ListChangeNotif_ListChanged( g_pst_LASessions[s32_Index].pu32_LineIdsOfDeletedEntries[0],
                                     LINE_TYPE_RELATING_TO,
                                     g_pst_LASessions[s32_Index].pu32_AttachedHsOfDeletedEntries[0],
                                     0,
                                     LIST_CHANGE_NOTIF_LIST_TYPE_LINE_SETTINGS_LIST );

    }
    else if ( g_pst_LASessions[s32_Index].u32_ListId == LIST_TYPE_MISSED_CALLS )
    {
        u32 u32_index;

        for ( u32_index = 0; u32_index < g_pst_LASessions[s32_Index].u32_LineIdsSize; ++u32_index )
        {
            u32 u32_NumOfRead, u32_NumOfUnread;

            List_GetMissedCallsNumOfEntries(g_pst_LASessions[s32_Index].pu32_LineIdsOfDeletedEntries[u32_index],
                                            &u32_NumOfUnread, &u32_NumOfRead);

            ListChangeNotif_MissedCallListChanged(g_pst_LASessions[s32_Index].pu32_LineIdsOfDeletedEntries[u32_index], FALSE);
        }
    }

    return LA_SESSION_MGR_RC_OK;
}

/* End Of File *****************************************************************************************************************************/

