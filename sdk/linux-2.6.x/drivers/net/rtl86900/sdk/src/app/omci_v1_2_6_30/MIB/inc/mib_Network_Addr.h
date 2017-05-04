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
 * Purpose : Definition of ME attribute: Network address (137)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Network address (137)
 */

#ifndef __MIB_NETWORK_ADDRESS_H__
#define __MIB_NETWORK_ADDRESS_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_NETWORK_ADDR_ATTR_NUM (3)
#define MIB_TABLE_NETWORK_ADDR_ENTITY_ID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_NETWORK_ADDR_SECURITY_PTR_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_NETWORK_ADDR_ADDR_PTR_INDEX ((MIB_ATTR_INDEX)3)


typedef struct {
	UINT16	EntityId;
    UINT16	SecurityPtr;
    UINT16	AddrPtr;
} __attribute__((aligned)) MIB_TABLE_NETWORK_ADDR_T;


#ifdef __cplusplus
}
#endif

#endif
