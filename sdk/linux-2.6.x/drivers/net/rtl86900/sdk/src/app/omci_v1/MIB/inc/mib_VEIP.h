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
 * Purpose : Definition of ME attribute: Virtual Ethernet interface point (329)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Virtual Ethernet interface point (329)
 */

#ifndef __MIB_VEIP_TABLE_H__
#define __MIB_VEIP_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_INTERDOMAINNAME_LEN (25)

/* Table VEIP attribute index */
#define MIB_TABLE_VEIP_ATTR_NUM (6)
#define MIB_TABLE_VEIP_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_VEIP_ADMINSTATE_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_VEIP_OPERSTATE_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_VEIP_INTERDOMAINNAME_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_VEIP_TCPUDPPTR_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_VEIP_IANSASSIGNPORT_INDEX ((MIB_ATTR_INDEX)6)

/* Table VEIP attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
	UINT8    AdminState;
	UINT8    OperState;
	CHAR     InterDomainName[MIB_TABLE_INTERDOMAINNAME_LEN+1];
	UINT16   TcpUdpPtr;
	UINT16   IansAssignPort;
} __attribute__((aligned)) MIB_TABLE_VEIP_T;


#ifdef __cplusplus
}
#endif

#endif
