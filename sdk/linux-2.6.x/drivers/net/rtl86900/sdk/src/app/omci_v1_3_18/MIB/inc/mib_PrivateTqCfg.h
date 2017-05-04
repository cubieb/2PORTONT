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
 * Purpose : Definition of ME attribute: Private Tcont Queue Configuration (65534)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Private Tcont Queue Configuration (65534)
 */

#ifndef __MIB_PRIVATE_TQCFG_TABLE_H__
#define __MIB_PRIVATE_TQCFG_TABLE_H__


/* Table private pon queue number per tcont configuration attribute index */
#define MIB_TABLE_PRIVATE_TQCFG_ATTR_NUM            (4)
#define MIB_TABLE_PRIVATE_TQCFG_ENTITYID_INDEX      ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_PRIVATE_TQCFG_TYPE                ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_PRIVATE_TQCFG_PON_SLOT_ID         ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_PRIVATE_TQCFG_QUEUE_NUM_PER_TCONT ((MIB_ATTR_INDEX)4)

#define MIB_TABLE_PON_QUEUE_MAX_NUM                 (256)

typedef enum
{
    PRIVATE_TQCFG_TYPE_RTK_DEFAULT_DEFINITION   = 0,
    PRIVATE_TQCFG_TYPE_CUSTOMIZED_DEFINITION    = 1
} private_tq_cfg_type_t;

/* Table private tcont queue cfg me stucture */
typedef struct
{
    UINT16  EntityID;
    UINT8   Type;
    UINT8   PonSlotId;
    UINT8   QueueNumPerTcont[MIB_TABLE_PON_QUEUE_MAX_NUM];
} __attribute__((aligned)) MIB_TABLE_PRIVATE_TQCFG_T;


#endif
