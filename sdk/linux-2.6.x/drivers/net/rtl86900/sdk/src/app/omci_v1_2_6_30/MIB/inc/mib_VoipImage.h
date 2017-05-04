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
 * Purpose : Definition of ME attribute: VoIP image (353)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: VoIP image (353)
 */

#ifndef __MIB_VOIP_IMAGE_TABLE_H__
#define __MIB_VOIP_IMAGE_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_VOIP_IMAGE_ATTR_NUM (6)
#define MIB_TABLE_VOIP_IMAGE_ENTITY_ID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_VOIP_IMAGE_VERSION_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_VOIP_IMAGE_IS_COMMITTED_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_VOIP_IMAGE_IS_ACTIVE_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_VOIP_IMAGE_IS_VALID_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_VOIP_IMAGE_CRC32_INDEX ((MIB_ATTR_INDEX)6)

#define MIB_TABLE_VOIP_IMAGE_VERSION_LEN (14)


typedef struct {
    UINT16	EntityId;
    CHAR	Version[MIB_TABLE_VOIP_IMAGE_VERSION_LEN+1];
    UINT8	IsCommitted;
    UINT8	IsActive;
    UINT8	IsValid;
    UINT32	Crc32;
} __attribute__((aligned)) MIB_TABLE_VOIP_IMAGE_T;


#ifdef __cplusplus
}
#endif

#endif
