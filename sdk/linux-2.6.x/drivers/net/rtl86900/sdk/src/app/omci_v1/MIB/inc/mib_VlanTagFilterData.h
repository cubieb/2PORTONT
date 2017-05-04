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
 * Purpose : Definition of ME attribute: VLAN tagging filter data (84)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: VLAN tagging filter data (84)
 */

#ifndef __MIB_VLANTAGFILTERDATA_TABLE_H__
#define __MIB_VLANTAGFILTERDATA_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table VlanTagFilterData attribute index */
#define MIB_TABLE_VLANTAGFILTERDATA_ATTR_NUM (4)
#define MIB_TABLE_VLANTAGFILTERDATA_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_VLANTAGFILTERDATA_FILTERTBL_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_VLANTAGFILTERDATA_FWDOP_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_VLANTAGFILTERDATA_NUMOFENTRIES_INDEX ((MIB_ATTR_INDEX)4)


/* Table VlanTagFilterData attribute len, only string attrubutes have length definition */
#define MIB_TABLE_VLANTAGFILTERDATA_FILTERTBL_LEN (24)

typedef enum {
    VTFD_FWD_OP_TAG_A_UNTAG_A	= 0x00,
    VTFD_FWD_OP_TAG_C_UNTAG_A	= 0x01,
    VTFD_FWD_OP_TAG_A_UNTAG_C	= 0x02,
    VTFD_FWD_OP_VID_F_UNTAG_A	= 0x03,
    VTFD_FWD_OP_VID_F_UNTAG_C	= 0x04,
    VTFD_FWD_OP_VID_G_UNTAG_A	= 0x05,
    VTFD_FWD_OP_VID_G_UNTAG_C	= 0x06,
    VTFD_FWD_OP_PRI_F_UNTAG_A	= 0x07,
    VTFD_FWD_OP_PRI_F_UNTAG_C	= 0x08,
    VTFD_FWD_OP_PRI_G_UNTAG_A	= 0x09,
    VTFD_FWD_OP_PRI_G_UNTAG_C	= 0x0A,
    VTFD_FWD_OP_TCI_F_UNTAG_A	= 0x0B,
    VTFD_FWD_OP_TCI_F_UNTAG_C	= 0x0C,
    VTFD_FWD_OP_TCI_G_UNTAG_A	= 0x0D,
    VTFD_FWD_OP_TCI_G_UNTAG_C	= 0x0E,
    VTFD_FWD_OP_VID_H_UNTAG_A	= 0x0F,
    VTFD_FWD_OP_VID_H_UNTAG_C	= 0x10,
    VTFD_FWD_OP_PRI_H_UNTAG_A	= 0x11,
    VTFD_FWD_OP_PRI_H_UNTAG_C	= 0x12,
    VTFD_FWD_OP_TCI_H_UNTAG_A	= 0x13,
    VTFD_FWD_OP_TCI_H_UNTAG_C	= 0x14,
    VTFD_FWD_OP_TAG_B_UNTAG_C	= 0x15,
    VTFD_FWD_OP_VID_J_UNTAG_A	= 0x16,
    VTFD_FWD_OP_VID_J_UNTAG_C	= 0x17,
    VTFD_FWD_OP_PRI_J_UNTAG_A	= 0x18,
    VTFD_FWD_OP_PRI_J_UNTAG_C	= 0x19,
    VTFD_FWD_OP_TCI_J_UNTAG_A	= 0x1A,
    VTFD_FWD_OP_TCI_J_UNTAG_C	= 0x1B,
    VTFD_FWD_OP_VID_K_UNTAG_A	= 0x1C,
    VTFD_FWD_OP_VID_K_UNTAG_C	= 0x1D,
    VTFD_FWD_OP_PRI_K_UNTAG_A	= 0x1E,
    VTFD_FWD_OP_PRI_K_UNTAG_C	= 0x1F,
    VTFD_FWD_OP_TCI_K_UNTAG_A	= 0x20,
    VTFD_FWD_OP_TCI_K_UNTAG_C	= 0x21
} vlan_tagging_filter_data_fwd_op_t;

// Table VlanTagFilterData entry stucture
typedef struct {
    UINT16 EntityID; // index 1
    UINT8  FilterTbl[MIB_TABLE_VLANTAGFILTERDATA_FILTERTBL_LEN];
    UINT8  FwdOp;
    UINT8  NumOfEntries;
} __attribute__((aligned)) MIB_TABLE_VLANTAGFILTERDATA_T;


#ifdef __cplusplus
}
#endif

#endif
