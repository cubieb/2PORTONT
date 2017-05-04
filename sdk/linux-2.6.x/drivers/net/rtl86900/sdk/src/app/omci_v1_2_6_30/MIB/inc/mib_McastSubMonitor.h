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
 * Purpose : Definition of ME attribute: Multicast subscriber monitor (311)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Multicast subscriber monitor (311)
 */

#ifndef __MIB_MCASTSUBMONITOR_TABLE_H__
#define __MIB_MCASTSUBMONITOR_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table SWImage attribute len, only string attrubutes have length definition */
#define MIB_TABLE_MSM_IPV4_ACTIVEGROUPLISTTABLE_LEN (24)
#define MIB_TABLE_MSM_IPV6_ACTIVEGROUPLISTTABLE_LEN (58)
#define MIB_TABLE_MSM_IPV6_ACTIVEGROUPLISTTABLE_SRC_IP_ADDR_LEN (16)
#define MIB_TABLE_MSM_IPV6_ACTIVEGROUPLISTTABLE_MCAST_DST_IP_ADDR_LEN (16)
#define MIB_TABLE_MSM_IPV6_ACTIVEGROUPLISTTABLE_CLIENT_IP_ADDR_LEN (16)

/* Table McastSubMonitor attribute index */
#define MIB_TABLE_MCASTSUBMONITOR_ATTR_NUM (7)
#define MIB_TABLE_MCASTSUBMONITOR_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_MCASTSUBMONITOR_METYPE_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_MCASTSUBMONITOR_CURRMCASTBANDWIDTH_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_MCASTSUBMONITOR_JOINMSGCOUNTER_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_MCASTSUBMONITOR_BANDWIDTHEXCCOUNTER_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_MCASTSUBMONITOR_IPV4_ACTIVEGROUPLISTTABLE_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_MCASTSUBMONITOR_IPV6_ACTIVEGROUPLISTTABLE_INDEX ((MIB_ATTR_INDEX)7)

typedef enum {
    MSM_ME_TYPE_MBPCD,
    MSM_ME_TYPE_IEEE_8021P_MAPPER,
} msm_attr_me_type_t;


/* Table McastSubMonitor attribute len, only string attrubutes have length definition */
typedef struct {
    UINT16	VlanID;
    UINT32	SrcIpAddr;
    UINT32	McastDstIpAddr;
    UINT32	BeActualBandwidthEst;
    UINT32	ClientIpAddr;
    UINT32	ElapseTime;
    UINT16	reserved;
} __attribute__((packed)) msm_attr_ipv4_active_group_list_table_t;

typedef struct msm_attr_ipv4_active_group_list_table_entry_s {
    msm_attr_ipv4_active_group_list_table_t                    tableEntry;
    LIST_ENTRY(msm_attr_ipv4_active_group_list_table_entry_s)  entries;
} __attribute__((aligned)) msm_attr_ipv4_active_group_list_table_entry_t;

typedef struct {
    UINT16	VlanID;
    UINT8	SrcIpAddr[MIB_TABLE_MSM_IPV6_ACTIVEGROUPLISTTABLE_SRC_IP_ADDR_LEN];
    UINT8	McastDstIpAddr[MIB_TABLE_MSM_IPV6_ACTIVEGROUPLISTTABLE_MCAST_DST_IP_ADDR_LEN];
    UINT32	BeActualBandwidthEst;
    UINT8	ClientIpAddr[MIB_TABLE_MSM_IPV6_ACTIVEGROUPLISTTABLE_CLIENT_IP_ADDR_LEN];
    UINT32	ElapseTime;
} __attribute__((packed)) msm_attr_ipv6_active_group_list_table_t;

typedef struct msm_attr_ipv6_active_group_list_table_entry_s {
    msm_attr_ipv6_active_group_list_table_t                    tableEntry;
    LIST_ENTRY(msm_attr_ipv6_active_group_list_table_entry_s)  entries;
} __attribute__((aligned)) msm_attr_ipv6_active_group_list_table_entry_t;

typedef struct {
	UINT16   EntityId;
	UINT8    MeType;
	UINT32   CurrMcastBandwidth;
	UINT32   JoinMsgCounter;
	UINT32   BandwidthExcCounter;
	UINT8    IPv4ActiveGroupListTable[MIB_TABLE_MSM_IPV4_ACTIVEGROUPLISTTABLE_LEN];
	UINT8    IPv6ActiveGroupListTable[MIB_TABLE_MSM_IPV6_ACTIVEGROUPLISTTABLE_LEN];
	LIST_HEAD(msm_attr_ipv4_active_group_list_table_head_s,
		msm_attr_ipv4_active_group_list_table_entry_s) IPv4ActiveGroupListTable_head;
	LIST_HEAD(msm_attr_ipv6_active_group_list_table_head_s,
		msm_attr_ipv6_active_group_list_table_entry_s) IPv6ActiveGroupListTable_head;
} __attribute__((aligned)) MIB_TABLE_MCASTSUBMONITOR_T;


#ifdef __cplusplus
}
#endif

#endif
