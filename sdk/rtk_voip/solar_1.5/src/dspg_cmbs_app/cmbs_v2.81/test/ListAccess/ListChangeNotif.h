/*************************************************************************************************************
*** List Change Notification
**
**************************************************************************************************************/
#ifndef __LA_LIST_CHANGE_NOTIF_H__
#define __LA_LIST_CHANGE_NOTIF_H__


/*******************************************
Includes
********************************************/
#include "cmbs_api.h"

/*******************************************
Defines
********************************************/

/*******************************************
Types
********************************************/
typedef enum 
{
    LINE_TYPE_EXTERNAL,
    LINE_TYPE_RELATING_TO,
    LINE_TYPE_ALL_LINES
} eLineType;

typedef enum
{
    LIST_CHANGE_NOTIF_LIST_TYPE_CONTACT_LIST,
    LIST_CHANGE_NOTIF_LIST_TYPE_LINE_SETTINGS_LIST,
    LIST_CHANGE_NOTIF_LIST_TYPE_MISSED_CALLS,       
    LIST_CHANGE_NOTIF_LIST_TYPE_OUTGOING_CALLS,     
    LIST_CHANGE_NOTIF_LIST_TYPE_INCOMING_ACCEPTED_CALLS,       
    LIST_CHANGE_NOTIF_LIST_TYPE_ALL_CALLS,          
    LIST_CHANGE_NOTIF_LIST_TYPE_ALL_INCOMING_CALLS,

    LIST_CHANGE_NOTIF_LIST_TYPE_MAX

}eLIST_CHANGE_NOTIF_LIST_TYPE;

/*******************************************
Globals
********************************************/

/*******************************************
List Change Notification API
********************************************/
void ListChangeNotif_MissedCallListChanged( IN u32 u32_LineId, IN bool bNewEntryAdded );

void ListChangeNotif_ListChanged( IN u32 u32_LineId, IN eLineType tLineType, IN u32 u32_HandsetMask,
                                  IN u32 u32_TotalNumOfEntries, IN eLIST_CHANGE_NOTIF_LIST_TYPE tListId );


#endif /* __LA_LIST_CHANGE_NOTIF_H__ */

/* End Of File *****************************************************************************************************************************/

