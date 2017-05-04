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
 * Purpose : Definition of ME attribute: Threshold data 2 (274)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Threshold data 2 (274)
 */

#ifndef __MIB_THRESHOLDDATA2_TABLE_H__
#define __MIB_THRESHOLDDATA2_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table ThresholdData2 attribute index */
#define MIB_TABLE_THRESHOLDDATA2_ATTR_NUM (8)
#define MIB_TABLE_THRESHOLDDATA2_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_THRESHOLDDATA2_THRESHOLDVALUE8_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_THRESHOLDDATA2_THRESHOLDVALUE9_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_THRESHOLDDATA2_THRESHOLDVALUE10_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_THRESHOLDDATA2_THRESHOLDVALUE11_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_THRESHOLDDATA2_THRESHOLDVALUE12_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_THRESHOLDDATA2_THRESHOLDVALUE13_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_THRESHOLDDATA2_THRESHOLDVALUE14_INDEX ((MIB_ATTR_INDEX)8)

/* Table ThresholdData2 attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
	UINT32   ThresholdValue8;
	UINT32   ThresholdValue9;
	UINT32   ThresholdValue10;
	UINT32   ThresholdValue11;
	UINT32   ThresholdValue12;
	UINT32   ThresholdValue13;
	UINT32   ThresholdValue14;
} __attribute__((aligned)) MIB_TABLE_THRESHOLDDATA2_T;


#ifdef __cplusplus
}
#endif

#endif
