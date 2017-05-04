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
 * Purpose : Definition of ME attribute: Ethernet PMHD 2 (89)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Ethernet PMHD 2 (89)
 */

#ifndef __MIB_ETHPMDATA2_TABLE_H__
#define __MIB_ETHPMDATA2_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table EthPmData2 attribute index */
#define MIB_TABLE_ETHPMDATA2_ATTR_NUM (4)
#define MIB_TABLE_ETHPMDATA2_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_ETHPMDATA2_INTENDTIME_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_ETHPMDATA2_THRESHOLDID_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_ETHPMDATA2_PPPOEFILFRM_INDEX ((MIB_ATTR_INDEX)4)

/* Table EthPmData2 attribute len, only string attrubutes have length definition */

// Table EthPmData2 entry stucture
typedef struct {
	UINT16   EntityId;
	UINT8    IntEndTime;
	UINT16   ThresholdID;
	UINT32   PppoeFilFrm;
} __attribute__((aligned)) MIB_TABLE_ETHPMDATA2_T;


#ifdef __cplusplus
}
#endif

#endif
