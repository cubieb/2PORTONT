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
 * Purpose : Definition of ME attribute: Software image (7)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Software image (7)
 */

#ifndef __MIB_SWIMAGE_TABLE_H__
#define __MIB_SWIMAGE_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table SWImage attribute index */
#define MIB_TABLE_SWIMAGE_ATTR_NUM (7)
#define MIB_TABLE_SWIMAGE_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_SWIMAGE_VERSION_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_SWIMAGE_COMMITTED_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_SWIMAGE_ACTIVE_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_SWIMAGE_VALID_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_SWIMAGE_PRODUCT_CODE_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_SWIMAGE_IMAGE_HASH_INDEX ((MIB_ATTR_INDEX)7)


/* Table SWImage attribute len, only string attrubutes have length definition */
#define MIB_TABLE_SWIMAGE_VERSION_LEN (14)
#define MIB_TABLE_SWIMAGE_PRODUCT_CODE_LEN (25)
#define MIB_TABLE_SWIMAGE_IMAGE_HASH_LEN (16)

// Table SWImage entry stucture
typedef struct {
    UINT16 EntityID; // index 1
    CHAR   Version[MIB_TABLE_SWIMAGE_VERSION_LEN+1];
    UINT8  Committed;
    UINT8  Active;
    UINT8  Valid;
    CHAR   ProductCode[MIB_TABLE_SWIMAGE_PRODUCT_CODE_LEN+1];
    UINT8  ImageHash[MIB_TABLE_SWIMAGE_IMAGE_HASH_LEN];
} __attribute__((aligned)) MIB_TABLE_SWIMAGE_T;


#ifdef __cplusplus
}
#endif

#endif
