/*
 * Copyright (C) 2012 Realtek Semiconductor Corp. 
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated, 
 * modified or distributed under the authorized license from Realtek. 
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER 
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED. 
 *
 */

#ifndef __MIB_LOIDAUTH_TABLE_H__
#define __MIB_LOIDAUTH_TABLE_H__

/* Table LoIdAuth attribute len, only string attrubutes have length definition */
#define MIB_TABLE_OPERATIONID_LEN (4)
#define MIB_TABLE_LOIDAUTH_LOID_LEN (24)
#define MIB_TABLE_LOIDAUTH_PASSWORD_LEN (12)

/* Table LoIdAuth attribute index */
#define MIB_TABLE_LOIDAUTH_ATTR_NUM (5)
#define MIB_TABLE_LOIDAUTH_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_LOIDAUTH_OPERATIONID_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_LOIDAUTH_LOID_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_LOIDAUTH_PASSWORD_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_LOIDAUTH_AUTHSTATUS_INDEX ((MIB_ATTR_INDEX)5)

/* Table LoIdAuth attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
	CHAR     OperationId[MIB_TABLE_OPERATIONID_LEN+1];
	CHAR     LoID[MIB_TABLE_LOIDAUTH_LOID_LEN+1];
	CHAR     Password[MIB_TABLE_LOIDAUTH_PASSWORD_LEN+1];
	UINT8    AuthStatus;
} MIB_TABLE_LOIDAUTH_T;

#endif /* __MIB_LOIDAUTH_TABLE_H__ */
