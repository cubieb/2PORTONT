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
 * Purpose : Definition of ME attribute: UNI-G (264)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: UNI-G (264)
 */

#ifndef __MIB_UNIG_TABLE_H__
#define __MIB_UNIG_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_UNIG_ATTR_NUM (6)
#define MIB_TABLE_UNIG_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_UNIG_CFGOPTIONSTATUS_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_UNIG_ADMINSTATE_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_UNIG_MANAGECAPABILITY_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_UNIG_NONOMCIPOINTER_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_UNIG_RELAY_AGENT_OPTIONS_INDEX ((MIB_ATTR_INDEX)6)


typedef struct {
    UINT16 EntityID; 
    UINT16 CfgOptionStatus;
    UINT8  AdminState;
    UINT8  ManageCapability;
    UINT16 NonOmciPointer;
    UINT16 RelayAgentOptions;
} __attribute__((aligned)) MIB_TABLE_UNIG_T;

typedef enum 
{
    UNIG_ADMIN_STATE_UNLOCK,
    UNIG_ADMIN_STATE_LOCK,
} UNIG_ADMIN_STATE;

typedef enum {
    UNIG_MGMT_CAPABILITY_OMCI_ONLY,
    UNIG_MGMT_CAPABILITY_NON_OMCI_ONLY,
    UNIG_MGMT_CAPABILITY_BOTH_OMCI_NON_OMCI,
} unig_attr_mgmt_capability_t;


#ifdef __cplusplus
}
#endif

#endif
