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
 * Purpose : Definition of ME attribute: ONU data (2)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: ONU data (2)
 */

#ifndef __MIB_ONTDATA_TABLE_H__
#define __MIB_ONTDATA_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table OntData attribute index */
#define MIB_TABLE_ONTDATA_ATTR_NUM (2)
#define MIB_TABLE_ONTDATA_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_ONTDATA_MIBDATASYNC_INDEX ((MIB_ATTR_INDEX)2)


/* Table OntData attribute len, only string attrubutes have length definition */

// Table OntData entry stucture
typedef struct {
    UINT16 EntityID; // index 1
    UINT8  MIBDataSync;
} __attribute__((aligned)) MIB_TABLE_ONTDATA_T;


#ifdef __cplusplus
}
#endif

#endif
