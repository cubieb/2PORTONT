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
 * Purpose : Definition of ME attribute: GAL Ethernet profile (272)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: GAL Ethernet profile (272)
 */

#ifndef __MIB_GALETHPROF_TABLE_H__
#define __MIB_GALETHPROF_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table GalEthProf attribute index */
#define MIB_TABLE_GALETHPROF_ATTR_NUM (2)
#define MIB_TABLE_GALETHPROF_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_GALETHPROF_MAXGEMPAYLOADSIZE_INDEX ((MIB_ATTR_INDEX)2)

/* Table GalEthProf attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
	UINT32   MaxGemPayloadSize;
} __attribute__((aligned)) MIB_TABLE_GALETHPROF_T;


#ifdef __cplusplus
}
#endif

#endif
