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
 * Purpose : Definition of ME attribute: MAC bridge port bridge table data (50)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: MAC bridge port configuration data (50)
 */

#ifndef __MIB_MACBRIPORTBRITBLDATA_TABLE_H__
#define __MIB_MACBRIPORTBRITBLDATA_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table MacBriPortBriTblData attribute index */
#define MIB_TABLE_MACBRIPORTBRITBLDATA_ATTR_NUM (2)
#define MIB_TABLE_MACBRIPORTBRITBLDATA_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_MACBRIPORTBRITBLDATA_BRIDGETABLE_INDEX ((MIB_ATTR_INDEX)2)


/* Bridge Table attribute len, only string attrubutes have length definition */
#define MIB_TABLE_BRIDGETABLE_ETNRYLEN  (8)
#define MIB_TABLE_BRIDGETABLE_MACLEN    (6)


typedef struct
{
    union
    {
        struct
        {
            UINT16 age          : 12;
            UINT16 rsd2         : 1;
            UINT16 is_dynamic   : 1;
            UINT16 rsd1         : 1;
            UINT16 is_filter    : 1;
        } bit;
        UINT16 val;
    } information;
	UINT8 macAddr[MIB_TABLE_BRIDGETABLE_MACLEN];
}omci_bridge_table_entry_t;

// Table MacBriPortBriTblData entry stucture
typedef struct {
    UINT16 EntityID; // index 1
    UINT8  BridgeTable[MIB_TABLE_BRIDGETABLE_ETNRYLEN];
    UINT16	 curBriTblEntryCnt;
} __attribute__((aligned)) MIB_TABLE_MACBRIPORTBRITBLDATA_T;


#ifdef __cplusplus
}
#endif

#endif
