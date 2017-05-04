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
 * Purpose : Definition of ME attribute: Private Vlan Configuration (65535)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Private Vlan Configuration (65535)
 */

#ifndef __MIB_PRIVATE_VLANCFG_TABLE_H__
#define __MIB_PRIVATE_VLANCFG_TABLE_H__


/* Table Private Vlan Configuration attribute index */
#define MIB_TABLE_PRIVATE_VLANCFG_ATTR_NUM (5)
#define MIB_TABLE_PRIVATE_VLANCFG_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_PRIVATE_VLANCFG_TYPE ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_PRIVATE_VLANCFG_MANUAL_MODE ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_PRIVATE_VLANCFG_MANUAL_TAG_VID ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_PRIVATE_VLANCFG_MANUAL_TAG_PRI ((MIB_ATTR_INDEX)5)


/* Table private vlan cfg attribute len, only string attrubutes have length definition */

typedef enum {
    PRIVATE_VLANCFG_TYPE_AUTO_DETECTION    = 0,
    PRIVATE_VLANCFG_TYPE_MANUAL_SETTING   = 1
} private_vlancfg_type_t;

typedef enum {
    PRIVATE_VLANCFG_MANUAL_MODE_TRANSPAREN                  = 0,
    PRIVATE_VLANCFG_MANUAL_MODE_TAGGING                     = 1,
    PRIVATE_VLANCFG_MANUAL_MODE_REMOTE_ACCESS               = 2,
    PRIVATE_VLANCFG_MANUAL_MODE_US_REMOVE_ACCESS_DS_UNTAG   = 3
} private_vlancfg_manual_mode_t;

// Table private vlan cfg entry stucture
typedef struct {
    UINT16	EntityID;
    UINT8	Type;
    UINT8	ManualMode;
    UINT16  ManualTagVid;
    UINT16  ManualTagPri;
} __attribute__((aligned)) MIB_TABLE_PRIVATE_VLANCFG_T;


#endif
