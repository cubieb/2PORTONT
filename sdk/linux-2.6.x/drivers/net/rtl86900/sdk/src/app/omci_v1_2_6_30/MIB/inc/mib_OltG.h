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
 * Purpose : Definition of ME attribute: OLG-G (131)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: OLG-G (131)
 */

#ifndef __MIB_OLTG_TABLE_H__
#define __MIB_OLTG_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table SWImage attribute len, only string attrubutes have length definition */
#define MIB_TABLE_EQUIPID_LEN (20)
#define MIB_TABLE_VERSION_LEN (14)
#define MIB_TABLE_TODINFO_LEN (14)

/* Table OltG attribute index */
#define MIB_TABLE_OLTG_ATTR_NUM (5)
#define MIB_TABLE_OLTG_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_OLTG_OLTVENDORID_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_OLTG_EQUIPID_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_OLTG_VERSION_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_OLTG_TODINFO_INDEX ((MIB_ATTR_INDEX)5)

typedef struct {
	unsigned int uSeqNumOfGemSuperframe;
    unsigned long long uTimeStampSecs:48;
    unsigned int uTimeStampNanosecs;
} __attribute__((packed)) oltg_attr_tod_info_t;

/* Table OltG attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
	UINT32   OltVendorId;
	CHAR     EquipId[MIB_TABLE_EQUIPID_LEN+1];
	CHAR     Version[MIB_TABLE_VERSION_LEN+1];
	UINT8    ToDInfo[MIB_TABLE_TODINFO_LEN];	
} __attribute__((aligned)) MIB_TABLE_OLTG_T;


#ifdef __cplusplus
}
#endif

#endif
