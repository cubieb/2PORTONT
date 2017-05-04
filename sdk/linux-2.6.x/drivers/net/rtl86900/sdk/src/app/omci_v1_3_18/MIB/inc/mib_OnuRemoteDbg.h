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
 * Purpose : Definition of ME attribute: ONU remote debug (158)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: ONU remote debug (158)
 */

#ifndef __MIB_ONU_REMOTE_DEBUG_H__
#define __MIB_ONU_REMOTE_DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_ONU_REMOTE_DBG_ATTR_NUM (4)
#define MIB_TABLE_ONU_REMOTE_DBG_ENTITY_ID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_ONU_REMOTE_DBG_CMD_FMT_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_ONU_REMOTE_DBG_CMD_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_ONU_REMOTE_DBG_REPLY_TBL_INDEX ((MIB_ATTR_INDEX)4)

#define MIB_TABLE_ORD_CMD_LEN (25)
#define MIB_TABLE_ORD_REPLY_TBL_LEN (25)


typedef enum {
    ORD_CMT_FMT_ASCII_STRING,
    ORD_CMT_FMT_FREE_FMT,
} ord_attr_cmt_fmt_t;

typedef struct {
    UINT8    ReplyTbl[MIB_TABLE_ORD_REPLY_TBL_LEN];
} __attribute__((packed)) ord_attr_reply_tbl_t;

typedef struct ord_attr_reply_tbl_entry_s {
    ord_attr_reply_tbl_t					tableEntry;
    LIST_ENTRY(ord_attr_reply_tbl_entry_s)	entries;
} __attribute__((aligned)) ord_attr_reply_tbl_entry_t;

typedef struct {
	UINT16   EntityId;
	UINT8    CmdFmt;
	CHAR     Cmd[MIB_TABLE_ORD_CMD_LEN+1];
	UINT8    ReplyTbl[MIB_TABLE_ORD_REPLY_TBL_LEN];
	LIST_HEAD(ord_attr_reply_tbl_head_s,
		ord_attr_reply_tbl_entry_s) ReplyTbl_head;
} __attribute__((aligned)) MIB_TABLE_ONU_REMOTE_DBG_T;


#ifdef __cplusplus
}
#endif

#endif
