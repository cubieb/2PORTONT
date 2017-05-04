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
 * Purpose : Definition of ME attribute: BBF TR-069 management server (340)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: BBF TR-069 management server (340)
 */

#ifndef __MIB_TR069MANAGESERVER_TABLE_H__
#define __MIB_TR069MANAGESERVER_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_TR069MANAGESERVER_ATTR_NUM (4)
#define MIB_TABLE_TR069MANAGESERVER_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_TR069MANAGESERVER_ADMINSTATE_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_TR069MANAGESERVER_ACSADDRESS_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_TR069MANAGESERVER_ASSOCIATETAG_INDEX ((MIB_ATTR_INDEX)4)

typedef struct {
    UINT16 EntityID; 
    UINT8  AdminState;
    UINT16 AcsAddress;
    UINT16 AssociateTag;
} __attribute__((aligned)) MIB_TABLE_TR069MANAGESERVER_T;


#ifdef __cplusplus
}
#endif

#endif
