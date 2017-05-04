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
 * Purpose : Definition of ME attribute: VLAN tagging operation configuration data (78)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: VLAN tagging operation configuration data (78)
 */

#ifndef __MIB_VLANTAGOPCFGDATA_TABLE_H__
#define __MIB_VLANTAGOPCFGDATA_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table VlanTagOpCfgData attribute index */
#define MIB_TABLE_VLANTAGOPCFGDATA_ATTR_NUM (6)
#define MIB_TABLE_VLANTAGOPCFGDATA_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_VLANTAGOPCFGDATA_USTAGOPMODE_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_VLANTAGOPCFGDATA_USTAGTCI_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_VLANTAGOPCFGDATA_DSTAGOPMODE_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_VLANTAGOPCFGDATA_TYPE_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_VLANTAGOPCFGDATA_POINTER_INDEX ((MIB_ATTR_INDEX)6)


/* Table VlanTagOpCfgData attribute len, only string attrubutes have length definition */
typedef enum {
	VTOCD_US_VLAN_TAG_OP_MODE_AS_IS		= 0,
	VTOCD_US_VLAN_TAG_OP_MODE_MODIFY		= 1,
	VTOCD_US_VLAN_TAG_OP_MODE_INSERT		= 2,
} vtocd_attr_us_vlan_tag_op_mode_t;

typedef enum {
	VTOCD_DS_VLAN_TAG_OP_MODE_AS_IS		= 0,
	VTOCD_DS_VLAN_TAG_OP_MODE_REMOVE		= 1,
} vtocd_attr_ds_vlan_tag_op_mode_t;

typedef enum {
	VTOCD_ASSOC_TYPE_DEFAULT			= 0,
	VTOCD_ASSOC_TYPE_IP_HOST_IPV6_HOST	= 1,
	VTOCD_ASSOC_TYPE_IEEE_8021P_MAPPER	= 2,
	VTOCD_ASSOC_TYPE_MAC_BRIDGE_PORT	= 3,
	VTOCD_ASSOC_TYPE_PPTP_XDSL_UNI		= 4,
	VTOCD_ASSOC_TYPE_GEM_IWTP			= 5,
	VTOCD_ASSOC_TYPE_MCAST_GEM_IWTP		= 6,
	VTOCD_ASSOC_TYPE_PPTP_MOCA_UNI		= 7,
	VTOCD_ASSOC_TYPE_ETH_FLOW_TP		= 9,
	VTOCD_ASSOC_TYPE_PPTP_ETH_UNI		= 10,
	VTOCD_ASSOC_TYPE_VEIP				= 11,
	VTOCD_ASSOC_TYPE_MPLS_PSEUDO_TP		= 12,
} vtocd_attr_assoc_type_t;

// Table VlanTagOpCfgData entry stucture
typedef struct {
    UINT16 EntityID; // index 1
    UINT8  UsTagOpMode;
    UINT16 UsTagTci;
    UINT8  DsTagOpMode;
    UINT8  Type;
    UINT16 Pointer;
} __attribute__((aligned)) MIB_TABLE_VLANTAGOPCFGDATA_T;


#ifdef __cplusplus
}
#endif

#endif
