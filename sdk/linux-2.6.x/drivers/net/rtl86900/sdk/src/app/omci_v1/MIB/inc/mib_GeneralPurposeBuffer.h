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
 * Purpose : Definition of ME attribute: General purpose buffer (308)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: General purpose buffer (308)
 */

#ifndef __MIB_GENERAL_PURPOSE_BUFFER_H__
#define __MIB_GENERAL_PURPOSE_BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_GENERAL_PURPOSE_BUFFER_ATTR_NUM (3)
#define MIB_TABLE_GENERAL_PURPOSE_BUFFER_ENTITY_ID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_GENERAL_PURPOSE_BUFFER_MAX_SIZE_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_GENERAL_PURPOSE_BUFFER_BUFFER_TBL_INDEX ((MIB_ATTR_INDEX)3)

#define MIB_TABLE_GPB_BUFFER_TBL_LEN (25)


typedef struct {
	UINT8	BufferTbl[MIB_TABLE_GPB_BUFFER_TBL_LEN];
} __attribute__((packed)) gpb_attr_buffer_tbl_t;

typedef struct gpb_attr_buffer_tbl_entry_s {
    gpb_attr_buffer_tbl_t					tableEntry;
    LIST_ENTRY(gpb_attr_buffer_tbl_entry_s)	entries;
} __attribute__((aligned)) gpb_attr_buffer_tbl_entry_t;

typedef struct {
	UINT16	EntityId;
    UINT32	MaxSize;
    UINT8	BufferTbl[MIB_TABLE_GPB_BUFFER_TBL_LEN];
    UINT32	BufferTbl_size;
    LIST_HEAD(gpb_attr_buffer_tbl_head_s,
		gpb_attr_buffer_tbl_entry_s) BufferTbl_head;
} __attribute__((aligned)) MIB_TABLE_GENERAL_PURPOSE_BUFFER_T;


GOS_ERROR_CODE general_purpose_buffer_attr_buffer_tbl_append(
        omci_me_instance_t entityId, UINT8 *pData, UINT32 dataSize);
GOS_ERROR_CODE general_purpose_buffer_attr_buffer_tbl_append_over(
        omci_me_instance_t entityId);


#ifdef __cplusplus
}
#endif

#endif
