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
 * Purpose : Definition of OMCI internal APIs
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1)
 */
#ifndef __MCAST_WRAPPER_H__
#define __MCAST_WRAPPER_H__

#ifdef  __cplusplus
extern "C" {
#endif


#include "gos_linux.h"
#include "omci_util.h"
#include "mib_table_defs.h"


typedef struct omci_mcast_wrapper_s
{
	/* reset */
	GOS_ERROR_CODE   (*omci_config_init)(void);
	/* set */
    GOS_ERROR_CODE   (*omci_max_simultaneous_groups_set)(UINT16, UINT16);
    GOS_ERROR_CODE   (*omci_immediate_leave_set)(UINT16, UINT8);
    GOS_ERROR_CODE   (*omci_last_member_query_interval_set)(UINT16, UINT32);
    GOS_ERROR_CODE   (*omci_dynamic_acl_table_entry_set)(UINT32, UINT32, UINT32, UINT16, UINT16, MIB_TABLE_MCASTOPERPROF_T *, MIB_TABLE_EXTMCASTOPERPROF_T *);
    GOS_ERROR_CODE   (*omci_unauthorized_join_behaviour_set)(UINT16, UINT8);
    GOS_ERROR_CODE   (*omci_mop_profile_add)(UINT16);
    GOS_ERROR_CODE   (*omci_mop_profile_del)(UINT16);
	GOS_ERROR_CODE   (*omci_mop_profile_per_port_set)(UINT32, UINT16, UINT16); //RG
	GOS_ERROR_CODE   (*omci_acl_per_port_set)(UINT32, UINT32, UINT16, unsigned int); //RTK
    GOS_ERROR_CODE   (*omci_us_igmp_rate_set)(UINT16, UINT32);
    GOS_ERROR_CODE   (*omci_ds_igmp_multicast_tci_set)(UINT16, UINT8 *);
	GOS_ERROR_CODE   (*omci_us_igmp_tag_info_set)(MIB_TABLE_MCASTOPERPROF_T *);//RG
	GOS_ERROR_CODE   (*omci_igmp_function_set)(UINT16, UINT8);
    GOS_ERROR_CODE   (*omci_ctrl_pkt_behaviour_set)(UINT16, UINT8);
    GOS_ERROR_CODE   (*omci_robustness_set)(UINT16, UINT8);
	GOS_ERROR_CODE   (*omci_querier_ip_addr_set)(UINT16, void *, UINT32);
	GOS_ERROR_CODE   (*omci_query_interval_set)(UINT16, UINT32);
	GOS_ERROR_CODE   (*omci_query_max_response_time_set)(UINT16, UINT32);
	GOS_ERROR_CODE   (*omci_connection_rule_set)(UINT32, OMCI_MCAST_PORT_INFO_t *);//RTK
	GOS_ERROR_CODE   (*omci_allowed_preview_groups_table_entry_set)(UINT32, UINT16, UINT16, MIB_TABLE_MCASTSUBCONFINFO_T*);
	GOS_ERROR_CODE   (*omci_max_multicast_bandwidth_set)(UINT16, UINT32);
	GOS_ERROR_CODE   (*omci_bandwidth_enforcement_set)(UINT16, UINT8);
	/* get */
	GOS_ERROR_CODE   (*omci_current_multicast_bandwidth_get)(UINT16, UINT32 *);
	GOS_ERROR_CODE   (*omci_join_message_counter_get)(UINT16, UINT32 *);
	GOS_ERROR_CODE   (*omci_bandwidth_exceeded_counter_get)(UINT16, UINT32 *);
	GOS_ERROR_CODE   (*omci_active_group_list_table_get)(MIB_ATTR_INDEX, UINT16, MIB_TABLE_MCASTSUBMONITOR_T *, UINT32 *);
} omci_mcast_wrapper_t;

#ifdef  __cplusplus
}
#endif

#endif

