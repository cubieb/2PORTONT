/*************************************************************************************************************
*** List Change Notification
**
**************************************************************************************************************/

/*******************************************
Includes
********************************************/
#include "ListChangeNotif.h"
#include "cfr_mssg.h"
#include "appcmbs.h"
#include "ListsApp.h"

/*******************************************
Defines
********************************************/

/*******************************************
Types
********************************************/

/*******************************************
Globals
********************************************/

/*******************************************
Auxiliary
********************************************/

/* ***************** Auxiliary end ***************** */

/*******************************************
List Change Notification API
********************************************/
void ListChangeNotif_MissedCallListChanged( IN u32 u32_LineId, IN bool bNewEntryAdded )
{
    u32 u32_HandsetMask, u32_NumOfRead, u32_NumOfUnread;

    List_GetAttachedHs(u32_LineId, &u32_HandsetMask);

    List_GetMissedCallsNumOfEntries(u32_LineId, &u32_NumOfRead, &u32_NumOfUnread );

    cmbs_dsr_gen_SendMissedCalls( g_cmbsappl.pv_CMBSRef, 0, u32_LineId, u32_HandsetMask, u32_NumOfUnread, bNewEntryAdded, u32_NumOfUnread + u32_NumOfRead );
}

void ListChangeNotif_ListChanged( IN u32 u32_LineId, IN eLineType tLineType, IN u32 u32_HandsetMask,
                                  IN u32 u32_TotalNumOfEntries, IN eLIST_CHANGE_NOTIF_LIST_TYPE tListId )
{
    cmbs_dsr_gen_SendListChanged( g_cmbsappl.pv_CMBSRef, 0, u32_HandsetMask, tListId, u32_TotalNumOfEntries, u32_LineId, tLineType );
}

/* End Of File *****************************************************************************************************************************/

