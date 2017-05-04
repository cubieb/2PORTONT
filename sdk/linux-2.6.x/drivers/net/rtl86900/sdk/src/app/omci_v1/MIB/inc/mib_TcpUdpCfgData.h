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
 * Purpose : Definition of ME attribute: TCP/UDP config data (136)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: TCP/UDP config data (136)
 */

#ifndef __MIB_TCP_UDP_CFG_DATA_TABLE_H__
#define __MIB_TCP_UDP_CFG_DATA_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_TCP_UDP_CFG_DATA_ATTR_NUM (5)
#define MIB_TABLE_TCP_UDP_CFG_DATA_ENTITY_ID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_TCP_UDP_CFG_DATA_PORT_ID_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_TCP_UDP_CFG_DATA_PROTOCOL_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_TCP_UDP_CFG_DATA_TOS_DIFFSERV_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_TCP_UDP_CFG_DATA_IP_HOST_POINTER_INDEX ((MIB_ATTR_INDEX)5)


typedef struct {
	UINT16   EntityId;
	UINT16   PortId;
	UINT8    Protocol;
	UINT8    TosDiffserv;
	UINT16   IpHostPointer;
} __attribute__((aligned)) MIB_TABLE_TCP_UDP_CFG_DATA_T;


#ifdef __cplusplus
}
#endif

#endif
