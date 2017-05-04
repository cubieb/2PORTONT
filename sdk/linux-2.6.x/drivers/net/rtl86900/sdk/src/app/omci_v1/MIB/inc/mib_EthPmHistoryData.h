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
 * Purpose : Definition of ME attribute: Ethernet PMHD (24)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Ethernet PMHD (24)
 */

#ifndef __MIB_ETHPMHISTORYDATA_TABLE_H__
#define __MIB_ETHPMHISTORYDATA_TABLE_H__


/* Table EthPmHistoryData attribute index */
#define MIB_TABLE_ETHPMHISTORYDATA_ATTR_NUM (17)
#define MIB_TABLE_ETHPMHISTORYDATA_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_ETHPMHISTORYDATA_INTENDTIME_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_ETHPMHISTORYDATA_THRESHOLDID_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_ETHPMHISTORYDATA_FCSERRORS_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_ETHPMHISTORYDATA_EXCCOLLISION_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_ETHPMHISTORYDATA_LATECOLLISION_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_ETHPMHISTORYDATA_FRAMETOOLONGS_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_ETHPMHISTORYDATA_RXBUFOVERFLOW_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_ETHPMHISTORYDATA_TXBUFOVERFLOW_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_ETHPMHISTORYDATA_SINGLECOLLISION_INDEX ((MIB_ATTR_INDEX)10)
#define MIB_TABLE_ETHPMHISTORYDATA_MULTICOLLISION_INDEX ((MIB_ATTR_INDEX)11)
#define MIB_TABLE_ETHPMHISTORYDATA_SQECOUNTER_INDEX ((MIB_ATTR_INDEX)12)
#define MIB_TABLE_ETHPMHISTORYDATA_TXDEFERRED_INDEX ((MIB_ATTR_INDEX)13)
#define MIB_TABLE_ETHPMHISTORYDATA_TXINTERMACERR_INDEX ((MIB_ATTR_INDEX)14)
#define MIB_TABLE_ETHPMHISTORYDATA_CARRIERSENSEERR_INDEX ((MIB_ATTR_INDEX)15)
#define MIB_TABLE_ETHPMHISTORYDATA_ALIGNMENTERR_INDEX ((MIB_ATTR_INDEX)16)
#define MIB_TABLE_ETHPMHISTORYDATA_RXINTERMACERR_INDEX ((MIB_ATTR_INDEX)17)

/* Table EthPmHistoryData attribute len, only string attrubutes have length definition */

// Table EthPmHistoryData entry stucture
typedef struct {
	UINT16   EntityId;
	UINT8    IntEndTime;
	UINT16   ThresholdID;
	UINT32   FCSErrors;
	UINT32   ExcCollision;
	UINT32   LateCollision;
	UINT32   FrameTooLongs;
	UINT32   RxBufOverflow;
	UINT32   TxBufOverflow;
	UINT32   SingleCollision;
	UINT32   MultiCollision;
	UINT32   SQECounter;
	UINT32   TxDeferred;
	UINT32   TxInterMacErr;
	UINT32   CarrierSenseErr;
	UINT32   AlignmentErr;
	UINT32   RxInterMacErr;
} __attribute__((aligned)) MIB_TABLE_ETHPMHISTORYDATA_T;


#endif
