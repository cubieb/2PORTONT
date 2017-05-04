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
 * Purpose : Definition of ME attribute: Generic status portal (330)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Generic status portal (330)
 */

#ifndef __MIB_GENERIC_STATUS_PORTAL_H__
#define __MIB_GENERIC_STATUS_PORTAL_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_GENERIC_STATUS_PORTAL_ATTR_NUM (4)
#define MIB_TABLE_GENERIC_STATUS_PORTAL_ENTITY_ID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_GENERIC_STATUS_PORTAL_STATUS_DOC_TBL_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_GENERIC_STATUS_PORTAL_CFG_DOC_TBL_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_GENERIC_STATUS_PORTAL_AVC_REPORT_RATE_INDEX ((MIB_ATTR_INDEX)4)

#define MIB_TABLE_GSP_STATUS_DOC_TBL_LEN (25)
#define MIB_TABLE_GSP_CFG_DOC_TBL_LEN (25)


typedef enum {
    GENERIC_STATUS_PORTAL_AVC_REPORT_RATE_DISABLE	= 0,
    GENERIC_STATUS_PORTAL_AVC_REPORT_RATE_TEN_MINS	= 1,
    GENERIC_STATUS_PORTAL_AVC_REPORT_RATE_PER_SECS	= 2,
    GENERIC_STATUS_PORTAL_AVC_REPORT_RATE_NO_LIMIT	= 3,
} generic_status_portal_attr_avc_report_rate_t;

typedef struct {
    UINT8    StatusDocTbl[MIB_TABLE_GSP_STATUS_DOC_TBL_LEN];
} __attribute__((packed)) gsp_attr_status_doc_tbl_t;

typedef struct gsp_attr_status_doc_tbl_entry_s {
    gsp_attr_status_doc_tbl_t					tableEntry;
    LIST_ENTRY(gsp_attr_status_doc_tbl_entry_s)	entries;
} __attribute__((aligned)) gsp_attr_status_doc_tbl_entry_t;

typedef struct {
    UINT8    CfgDocTbl[MIB_TABLE_GSP_CFG_DOC_TBL_LEN];
} __attribute__((packed)) gsp_attr_cfg_doc_tbl_t;

typedef struct gsp_attr_cfg_doc_tbl_entry_s {
    gsp_attr_cfg_doc_tbl_t						tableEntry;
    LIST_ENTRY(gsp_attr_cfg_doc_tbl_entry_s)	entries;
} __attribute__((aligned)) gsp_attr_cfg_doc_tbl_entry_t;

typedef struct {
    UINT16  EntityId;
    UINT8   StatusDocTbl[MIB_TABLE_GSP_STATUS_DOC_TBL_LEN];
    UINT8   CfgDocTbl[MIB_TABLE_GSP_CFG_DOC_TBL_LEN];
    UINT8   AvcReportRate;
    UINT32  StatusDocTbl_size;
    UINT32  CfgDocTbl_size;
    LIST_HEAD(gsp_attr_status_doc_tbl_head_s,
		gsp_attr_status_doc_tbl_entry_s) StatusDocTbl_head;
    LIST_HEAD(gsp_attr_cfg_doc_tbl_head_s,
		gsp_attr_cfg_doc_tbl_entry_s) CfgDocTbl_head;
} __attribute__((aligned)) MIB_TABLE_GENERIC_STATUS_PORTAL_T;


GOS_ERROR_CODE generic_status_portal_attr_status_doc_tbl_append(
        omci_me_instance_t entityId, UINT8 *pData, UINT32 dataSize);
GOS_ERROR_CODE generic_status_portal_attr_status_doc_tbl_append_over(
        omci_me_instance_t entityId);
GOS_ERROR_CODE generic_status_portal_attr_cfg_doc_tbl_append(
        omci_me_instance_t entityId, UINT8 *pData, UINT32 dataSize);
GOS_ERROR_CODE generic_status_portal_attr_cfg_doc_tbl_append_over(
        omci_me_instance_t entityId);


#ifdef __cplusplus
}
#endif

#endif
