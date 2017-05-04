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
 * Purpose : Definition of ME attribute: Authentication security method (148)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Authentication security method (148)
 */

#ifndef __MIB_AUTHENTICATION_SECURITY_METHOD_H__
#define __MIB_AUTHENTICATION_SECURITY_METHOD_H__


#define MIB_TABLE_AUTH_SEC_METHOD_ATTR_NUM (6)
#define MIB_TABLE_AUTH_SEC_METHOD_ENTITY_ID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_AUTH_SEC_METHOD_VALID_SCHEM_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_AUTH_SEC_METHOD_USERNAME1_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_AUTH_SEC_METHOD_PASSWORD_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_AUTH_SEC_METHOD_REALM_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_AUTH_SEC_METHOD_USERNAME2_INDEX ((MIB_ATTR_INDEX)6)

#define MIB_TABLE_AUTH_SEC_METHOD_USERNAME1_LEN (25)
#define MIB_TABLE_AUTH_SEC_METHOD_PASSWORD_LEN (25)
#define MIB_TABLE_AUTH_SEC_METHOD_REALM_LEN (25)
#define MIB_TABLE_AUTH_SEC_METHOD_USERNAME2_LEN (25)


typedef enum {
    AUTH_SEC_METHOD_VALIDATION_SCHEME_DISABLED			= 0,
    AUTH_SEC_METHOD_VALIDATION_SCHEME_RFC_2617_MD5		= 1,
    AUTH_SEC_METHOD_VALIDATION_SCHEME_RFC_2617_BASIC	= 3,
} auth_sec_method_attr_validation_scheme_t;

typedef struct {
	UINT16	EntityId;
    UINT8	ValidSchem;
    CHAR	Username1[MIB_TABLE_AUTH_SEC_METHOD_USERNAME1_LEN + 1];
    CHAR	Password[MIB_TABLE_AUTH_SEC_METHOD_PASSWORD_LEN + 1];
    CHAR	Realm[MIB_TABLE_AUTH_SEC_METHOD_REALM_LEN + 1];
    CHAR	Username2[MIB_TABLE_AUTH_SEC_METHOD_USERNAME2_LEN + 1];
} __attribute__((aligned)) MIB_TABLE_AUTH_SEC_METHOD_T;


#endif
