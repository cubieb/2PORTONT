/*
 * Copyright (C) 2014 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : Definition of OMCI MIB table related define
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI MIB table related define
 */

#ifndef __MIB_TABLE_H__
#define __MIB_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "mib_tree.h"
#include "omci_alarm.h"


typedef struct omciMibTables_s{
	int tablesCount;
	LIST_HEAD(listhead,mib_table_s) head;
}omciMibTables_t;



/*mib attribute information structure*/
typedef struct {
    const CHAR* Name;
    const CHAR* Desc;
    MIB_ATTR_TYPE DataType;
    UINT32 Len;
    BOOL IsIndex;
    BOOL MibSave;
    MIB_ATTR_OUT_STYLE OutStyle;
    UINT32 OltAcc;
    BOOL AvcFlag;
    UINT32 OptionType;
} MIB_ATTR_INFO_T;


/*mib table information structure*/
typedef struct {
    const CHAR* Name;
    const CHAR* ShortName;
    const CHAR* Desc;
    UINT32 ClassId;
    UINT32 InitType;
    UINT32 StdType;
    UINT32 ActionType;
	UINT32 attrNum;
	UINT32 entrySize;
	void*  pDefaultRow;
    MIB_ATTR_INFO_T* pAttributes;
} MIB_TABLE_INFO_T;

typedef struct {
    UINT8 aBitMask[OMCI_ALARM_NUMBER_BITMASK_IN_BYTE];
} mib_alarm_table_t;

/*mib table entry structure for save low data and linking to tree and table*/
typedef struct mib_entry_s{
	void *pData;
    void *pPmCurrentBin;
    void *pAlarmTable;
	LIST_ENTRY(mib_entry_s) entries;
}MIB_ENTRY_T;

typedef enum {
    OMCI_PM_OPER_UPDATE,
    OMCI_PM_OPER_SWAP,
    OMCI_PM_OPER_UPDATE_AND_SWAP,
    OMCI_PM_OPER_RESET,
} omci_pm_oper_type_t;

/*mib table callback function prototype*/
typedef GOS_ERROR_CODE (* MIB_CALLBACK_FUNC)(MIB_TABLE_INDEX	tableIndex,
                                             void*           	pOldRow,
                                             void*           	pNewRow,
                                             MIB_ATTRS_SET*  	pAttrsSet,
                                             MIB_OPERA_TYPE  	operationType);


typedef GOS_ERROR_CODE (* MIB_CFG_FUNC)(  void*           		pOldRow,
                                             void*           	pNewRow,
                                         MIB_OPERA_TYPE  	operationType,
                                         MIB_ATTRS_SET 		attrSet,
                                         UINT32 			pri);


typedef GOS_ERROR_CODE (*MIB_CONN_CHECK_FUNC)(MIB_TREE_T            *pTree,
												MIB_TREE_CONN_T     *pConn,
												omci_me_instance_t  entity,
												int                 parm);

typedef GOS_ERROR_CODE (*MIB_DUMP_FUNC)(void *pData);

typedef GOS_ERROR_CODE (*MIB_TREE_ADD_FUNC)(MIB_TREE_T          *pTree,
											omci_me_instance_t   entity);

typedef GOS_ERROR_CODE (*MIB_PM_FUNC)(MIB_TABLE_INDEX           tableIndex,
                                        omci_me_instance_t      instanceID,
                                        omci_pm_oper_type_t     operType,
                                        BOOL                    *pIsTcaRaised);

typedef GOS_ERROR_CODE (*MIB_ALARM_FUNC)(MIB_TABLE_INDEX        tableIndex,
                                            omci_alm_data_t     alarmData,
                                            omci_me_instance_t  *pInstanceID,
                                            BOOL                *pIsUpdated);

typedef GOS_ERROR_CODE (*MIB_TEST_FUNC)(void    *pData);

typedef struct mib_table_oper{
	MIB_CFG_FUNC	  	meOperDrvCfg;
	MIB_CFG_FUNC		meOperConnCfg;
	MIB_CONN_CHECK_FUNC meOperConnCheck;
	MIB_DUMP_FUNC		meOperDump;
	MIB_TREE_ADD_FUNC 	meOperAvlTreeAdd;
    MIB_TREE_ADD_FUNC   meOperAvlTreeDel;
    MIB_PM_FUNC         meOperPmHandler;
    MIB_ALARM_FUNC      meOperAlarmHandler;
    MIB_TEST_FUNC       meOperTestHandler;
}MIB_TABLE_OPER_T;

/*mib table structure*/
typedef struct mib_table_s {

	MIB_TABLE_INDEX	  tableIndex;
    MIB_TABLE_INFO_T* pTableInfo;
    MIB_CALLBACK_FUNC preCheckCB[MIB_TABLE_CB_MAX_NUM];
    MIB_CALLBACK_FUNC operationCB[MIB_TABLE_CB_MAX_NUM];
    UINT32            curEntryCount;
    UINT32            snapshotEntryCount;
    UINT32            almSnapShotCnt;
	MIB_TABLE_OPER_T  *meOper;
    void*             pSnapshotData;
    void*             pAlmSnapShot;
	LIST_ENTRY(mib_table_s) entries;
    LIST_HEAD(entryhead,mib_entry_s) entryHead;
} MIB_TABLE_T;


const CHAR* MIB_GetTableName(MIB_TABLE_INDEX tableIndex);
const CHAR* MIB_GetTableShortName(MIB_TABLE_INDEX tableIndex);
const CHAR* MIB_GetTableDesc(MIB_TABLE_INDEX tableIndex);
UINT32 MIB_GetTableClassId(MIB_TABLE_INDEX tableIndex);
UINT32 MIB_GetTableInitType(MIB_TABLE_INDEX tableIndex);
UINT32 MIB_GetTableStdType(MIB_TABLE_INDEX tableIndex);
UINT32 MIB_GetTableActionType(MIB_TABLE_INDEX tableIndex);
const CHAR* MIB_GetAttrName(MIB_TABLE_INDEX tableIndex, MIB_ATTR_INDEX attrIndex);
const CHAR* MIB_GetAttrDesc(MIB_TABLE_INDEX tableIndex, MIB_ATTR_INDEX attrIndex);
MIB_ATTR_TYPE MIB_GetAttrDataType(MIB_TABLE_INDEX tableIndex, MIB_ATTR_INDEX attrIndex);
UINT32 MIB_GetAttrLen(MIB_TABLE_INDEX tableIndex, MIB_ATTR_INDEX attrIndex);
BOOL MIB_GetAttrIsIndex(MIB_TABLE_INDEX tableIndex, MIB_ATTR_INDEX attrIndex);
BOOL MIB_GetAttrMibSave(MIB_TABLE_INDEX tableIndex, MIB_ATTR_INDEX attrIndex);
MIB_ATTR_OUT_STYLE MIB_GetAttrOutStyle(MIB_TABLE_INDEX tableIndex, MIB_ATTR_INDEX attrIndex);
UINT32 MIB_GetAttrOltAcc(MIB_TABLE_INDEX tableIndex, MIB_ATTR_INDEX attrIndex);
BOOL MIB_GetAttrAvcFlag(MIB_TABLE_INDEX tableIndex, MIB_ATTR_INDEX attrIndex);
UINT32 MIB_GetAttrOptionType(MIB_TABLE_INDEX tableIndex, MIB_ATTR_INDEX attrIndex);
MIB_ENTRY_T* MIB_GetTableEntry(MIB_TABLE_INDEX tableIndex, void* pObjectRow);
MIB_TABLE_INFO_T* MIB_GetTableInfoPtr(MIB_TABLE_INDEX tableIndex);
void* MIB_GetDefaultRow(MIB_TABLE_INDEX tableIndex);



GOS_ERROR_CODE MIB_InfoRegister(MIB_TABLE_INDEX tableIdx,MIB_TABLE_INFO_T *pTableInfo, MIB_TABLE_OPER_T *oper);
GOS_ERROR_CODE MIB_UnRegister(MIB_TABLE_INDEX tableIndex);
MIB_TABLE_INDEX MIB_Register(void);



/*MIB Tree related export API*/
MIB_NODE_T*     MIB_BalanceAvlTree(MIB_NODE_T **node) ;
MIB_NODE_T*     MIB_AvlTreeNodeAdd(MIB_NODE_T **root,MIB_AVL_KEY_T key, MIB_TABLE_INDEX tableId,void *pData) ;
MIB_TREE_NODE_ENTRY_T*    MIB_AvlTreeEntrySearch(MIB_NODE_T *node, MIB_AVL_KEY_T key,omci_me_instance_t entityId) ;
MIB_NODE_T*     MIB_AvlTreeSearch(MIB_NODE_T *node, MIB_AVL_KEY_T key) ;
MIB_TREE_T*     MIB_AvlTreeSearchByKey(MIB_TREE_T* pTreePrev, omci_me_instance_t entityId,MIB_AVL_KEY_T key) ;
GOS_ERROR_CODE  MIB_AvlTreeEntryDump(MIB_NODE_T *pNode);
GOS_ERROR_CODE  MIB_AvlTreeDump(void);
GOS_ERROR_CODE  MIB_AvlTreeDumpByKey(MIB_AVL_KEY_T key);
GOS_ERROR_CODE  MIB_TreeConnDump(void);
GOS_ERROR_CODE  MIB_TreeConnDelAll(void);
MIB_TREE_T*     MIB_AvlTreeCreate(MIB_TABLE_MACBRISERVPROF_T *pBridge);
GOS_ERROR_CODE MIB_AvlTreeNodeEntryRemoveByKey(MIB_NODE_T *pNode, MIB_AVL_KEY_T key, omci_me_instance_t entityId);
GOS_ERROR_CODE  MIB_AvlTreeNodeRemove(MIB_NODE_T *pNode);
GOS_ERROR_CODE  MIB_AvlTreeRemove(MIB_TREE_T *pTree);
GOS_ERROR_CODE  MIB_AvlTreeRemoveAll(void);
GOS_ERROR_CODE  MIB_TreeConnUpdate(MIB_TREE_T *pTree);
MIB_NODE_T* MIB_AvlTreeNodeAdd2Tail(MIB_NODE_T **root,MIB_AVL_KEY_T key, MIB_TABLE_INDEX tableId,void *pData);



GOS_ERROR_CODE omci_GenTraffRule(MIB_TREE_CONN_T *pConn,int index);
GOS_ERROR_CODE OMCI_MeConnCheck(MIB_TABLE_INDEX tableId,MIB_TREE_T *pTree,MIB_TREE_CONN_T *pConn,omci_me_instance_t entityId, void *opt);

/*MIB Table Base*/

GOS_ERROR_CODE  MIB_Init(MIB_TABLE_INDEX* pIncludeTbl, MIB_TABLE_INDEX* pExcludeTbl);
MIB_TABLE_INDEX MIB_GetTableIndexByName(const CHAR* name);
UINT32          MIB_GetTableAttrNum(MIB_TABLE_INDEX tableIndex);
UINT32          MIB_GetTableCurEntryCount(MIB_TABLE_INDEX tableIndex);
UINT32          MIB_GetTableEntrySize(MIB_TABLE_INDEX tableIndex);
UINT32          MIB_GetTableEntrySizeBeforePadded(MIB_TABLE_INDEX tableIndex);

MIB_ATTR_INDEX MIB_GetAttrIndexByName(MIB_TABLE_INDEX tableIndex, const CHAR* name);
UINT32         MIB_GetAttrSize(MIB_TABLE_INDEX tableIndex, MIB_ATTR_INDEX attrIndex);
UINT32         MIB_GetAttrDefaultValue(MIB_TABLE_INDEX tableIndex, MIB_ATTR_INDEX attrIndex);

GOS_ERROR_CODE MIB_RegisterCallback(MIB_TABLE_INDEX tableIndex, MIB_CALLBACK_FUNC pfnPreCheck, MIB_CALLBACK_FUNC pfnCallback);
GOS_ERROR_CODE MIB_RegisterCallbackToAll(MIB_CALLBACK_FUNC pfnPreCheck, MIB_CALLBACK_FUNC pfnCallback);
GOS_ERROR_CODE MIB_ClearAttrSet(MIB_ATTRS_SET* pAttrSet);
GOS_ERROR_CODE MIB_UnSetAttrSet(MIB_ATTRS_SET* pAttrSet, MIB_ATTR_INDEX attrIndex);
GOS_ERROR_CODE MIB_SetAttrSet(MIB_ATTRS_SET* pAttrSet, MIB_ATTR_INDEX attrIndex);
BOOL           MIB_IsInAttrSet(MIB_ATTRS_SET* pAttrSet, MIB_ATTR_INDEX attrIndex);
GOS_ERROR_CODE MIB_FullAttrSet(MIB_ATTRS_SET* pAttrSet);
UINT32         MIB_GetAttrNumOfSet(MIB_ATTRS_SET* pAttrSet);

GOS_ERROR_CODE MIB_Get(MIB_TABLE_INDEX tableIndex, void *pRow, UINT32 rowLen);
GOS_ERROR_CODE MIB_GetPmCurrentBin(MIB_TABLE_INDEX tableIndex, void *pRow, UINT32 rowLen);
GOS_ERROR_CODE MIB_GetNext(MIB_TABLE_INDEX tableIndex, void *pRow, UINT32 rowLen);
GOS_ERROR_CODE MIB_GetFirst(MIB_TABLE_INDEX tableIndex, void *pRow, UINT32 rowLen);
GOS_ERROR_CODE MIB_Set(MIB_TABLE_INDEX tableIndex, void *pRow, UINT32 rowLen);
GOS_ERROR_CODE MIB_SetPmCurrentBin(MIB_TABLE_INDEX tableIndex, void *pRow, UINT32 rowLen);

GOS_ERROR_CODE MIB_SetAttributes(MIB_TABLE_INDEX tableIndex, void* pRow, UINT32 rowLen, MIB_ATTRS_SET* pAttrSet);

GOS_ERROR_CODE MIB_Delete(MIB_TABLE_INDEX tableIndex, void* pRow, UINT32 rowLen);

GOS_ERROR_CODE MIB_Clear(MIB_TABLE_INDEX tableIndex);
GOS_ERROR_CODE MIB_ClearAll(void);
GOS_ERROR_CODE MIB_Default(MIB_TABLE_INDEX tableIndex, void* pRow, UINT32 rowLen);

GOS_ERROR_CODE MIB_SetAttrToBuf(MIB_TABLE_INDEX tableIndex, MIB_ATTR_INDEX attrIndex, void *pAttr, void *pBuf, UINT32 size);
GOS_ERROR_CODE MIB_GetAttrFromBuf(MIB_TABLE_INDEX tableIndex, MIB_ATTR_INDEX attrIndex, void *pAttr, void *pBuf, UINT32 size);

GOS_ERROR_CODE MIB_CreateSnapshot(MIB_TABLE_INDEX tableIndex);
GOS_ERROR_CODE MIB_DeleteSnapshot(MIB_TABLE_INDEX tableIndex);
GOS_ERROR_CODE MIB_CreateAllTableSnapshots(void);
GOS_ERROR_CODE MIB_DeleteAllTableSnapshots(void);
UINT32         MIB_GetSnapshotEntryCount(MIB_TABLE_INDEX tableIndex);
GOS_ERROR_CODE MIB_GetSnapshot(MIB_TABLE_INDEX tableIndex, void* pRow, UINT32 rowLen);
GOS_ERROR_CODE MIB_GetSnapshotNext(MIB_TABLE_INDEX tableIndex, void* pRow, UINT32 rowLen);
GOS_ERROR_CODE MIB_GetSnapshotFirst(MIB_TABLE_INDEX tableIndex, void* pRow, UINT32 rowLen);

GOS_ERROR_CODE MIB_DumpText(UINT32 fd);
GOS_ERROR_CODE MIB_DumpAll(void);
GOS_ERROR_CODE MIB_DumpTable(MIB_TABLE_INDEX tableIndex, void* pData);

int MIB_GetTableNum(void);
MIB_TABLE_T* mib_GetTablePtr(MIB_TABLE_INDEX tableIndex);
MIB_ATTR_INFO_T* mib_GetAttrInfo(MIB_TABLE_INDEX tableIndex, MIB_ATTR_INDEX attrIndex);
INT32 mib_CompareEntry(MIB_TABLE_INDEX tableIndex, void * pRow1, void * pRow2);
BOOL mib_FindEntry(MIB_TABLE_INDEX tableIndex, void* pObjectRow, void* ppRetRow);

MIB_TABLE_INDEX MIB_GetTableIndexByClassId(omci_me_class_t classId);
BOOL            MIB_TableSupportAction(MIB_TABLE_INDEX tableIndex, omci_me_action_mask_t action);
GOS_ERROR_CODE  MIB_CreatePublicTblSnapshot(void);
GOS_ERROR_CODE  MIB_DeletePublicTblSnapshot(void);
GOS_ERROR_CODE  MIB_ClearPublic(void);


#ifdef __cplusplus
}
#endif

#endif
