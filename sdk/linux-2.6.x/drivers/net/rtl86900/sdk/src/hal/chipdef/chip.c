/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 * Purpose : chip symbol and data type definition in the SDK.
 *
 * Feature : chip symbol and data type definition
 *
 */

/*
 * Include Files
 */
#include <common/debug/rt_log.h>
#include <osal/lib.h>
#include <ioal/mem32.h>
#include <hal/chipdef/chip.h>
#include <hal/common/halctrl.h>

#if defined(CONFIG_SDK_APOLLO)
#include <hal/chipdef/apollo/apollo_def.h>
#endif

#if defined(CONFIG_SDK_APOLLOMP)
#include <hal/chipdef/apollomp/apollomp_def.h>
#endif

#if defined(CONFIG_SDK_RTL9601B)
#include <hal/chipdef/rtl9601b/rtl9601b_def.h>
#endif


#if defined(CONFIG_SDK_RTL9602C)
#include <hal/chipdef/rtl9602c/rtl9602c_def.h>
#endif

#if defined(CONFIG_SDK_RTL9607B)
#include <hal/chipdef/rtl9607b/rtl9607b_def.h>
#endif

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
#if defined(CONFIG_SDK_APOLLO)
static rt_portinfo_t apollo_port_info =
{
    /* Normal APOLLO Chip Port Information */
    {
        RT_GE_PORT    /*P0 */, RT_GE_PORT    /*P1 */, RT_GE_PORT    /*P2 */, RT_GE_PORT    /*P3 */, RT_GE_PORT     /*P4 */,
        RT_GE_PORT    /*P5 */, RT_CPU_PORT    /*P6 */, RT_PORT_NONE  /*P7 */, RT_PORT_NONE  /*P8 */, RT_PORT_NONE   /*P9 */,
        RT_PORT_NONE  /*P10*/, RT_PORT_NONE  /*P11*/, RT_PORT_NONE /*P12*/, RT_PORT_NONE  /*P13*/, RT_PORT_NONE   /*P14*/,
        RT_PORT_NONE  /*P15*/, RT_PORT_NONE  /*P16*/, RT_PORT_NONE /*P17*/, RT_PORT_NONE  /*P18*/, RT_PORT_NONE   /*P19*/,
        RT_PORT_NONE  /*P20*/, RT_PORT_NONE  /*P21*/, RT_PORT_NONE /*P22*/, RT_PORT_NONE  /*P23*/, RT_PORT_NONE   /*P24*/,
        RT_PORT_NONE  /*P25*/, RT_PORT_NONE  /*P26*/, RT_PORT_NONE /*P27*/, RT_PORT_NONE  /*P28*/, RT_PORT_NONE   /*P29*/,
        RT_PORT_NONE  /*P30*/, RT_PORT_NONE  /*P31*/
     },
     /*DSL port member*/
     {
     	16, /*port number*/
     	15,	/*max*/
     	0,  /*min*/
     	{{0xFFFF}}/*port mask*/
     },
     /*extension port member*/
     {
     	6, /*port number*/
     	5,	/*max*/
     	0,  /*min*/
     	{{0x3F}}/*port mask*/
     },
     .ponPort   = 3,
     .rgmiiPort = 2
}; /* end of apollo_port_info */

/*
Apollo port mapping

UTP0               0
UTP1               1
RGMII              2
PON/fiber/UTP4     3
UTP2               4
UTP3               5
CPU port           6

*/



/* Normal apollo Chip Port Information */
static rt_register_capacity_t apollo_capacityInfo =
{
    .max_num_of_mirror                  = APOLLO_MAX_NUM_OF_MIRROR               ,
    .max_num_of_trunk                   = APOLLO_MAX_NUM_OF_TRUNK                ,
    .max_num_of_trunkMember             = APOLLO_MAX_NUM_OF_TRUNKMEMBER          ,
    .max_num_of_dumb_trunkMember        = APOLLO_MAX_NUM_OF_DUMB_TRUNKMEMBER     ,
    .max_num_of_trunkHashVal            = APOLLO_MAX_NUM_OF_TRUNKHASHVAL         ,
    .max_num_of_msti                    = APOLLO_MAX_NUM_OF_MSTI                 ,
    .max_num_of_metering                = APOLLO_MAX_NUM_OF_METERING             ,
    .max_num_of_field_selector          = APOLLO_MAX_NUM_OF_FIELD_SELECTOR       ,
    .max_num_of_range_check_srcPort     = APOLLO_MAX_NUM_OF_RANGE_CHECK_SRCPORT  ,
    .max_num_of_range_check_ip          = APOLLO_MAX_NUM_OF_RANGE_CHECK_IP       ,
    .max_num_of_range_check_vid         = APOLLO_MAX_NUM_OF_RANGE_CHECK_VID      ,
    .max_num_of_range_check_pktLen      = APOLLO_MAX_NUM_OF_RANGE_CHECK_PKTLEN   ,
    .max_num_of_range_check_l4Port      = APOLLO_MAX_NUM_OF_RANGE_CHECK_L4PORT   ,
    .max_num_of_pattern_match_data      = APOLLO_MAX_NUM_OF_PATTERN_MATCH_DATA   ,
    .pattern_match_port_max             = APOLLO_PATTERN_MATCH_PORT_MAX          ,
    .pattern_match_port_min             = APOLLO_PATTERN_MATCH_PORT_MIN          ,
    .max_num_of_l2_hashdepth            = APOLLO_MAX_NUM_OF_L2_HASHDEPTH         ,
    .max_num_of_queue                   = APOLLO_MAX_NUM_OF_QUEUE                ,
    .min_num_of_queue                   = APOLLO_MIN_NUM_OF_QUEUE                ,
    .max_num_of_pon_queue               = APOLLO_MAX_NUM_OF_PON_QUEUE            ,
    .min_num_of_pon_queue               = APOLLO_MIN_NUM_OF_PON_QUEUE            ,
    .max_num_of_cvlan_tpid              = APOLLO_MAX_NUM_OF_CVLAN_TPID           ,
    .max_num_of_svlan_tpid              = APOLLO_MAX_NUM_OF_SVLAN_TPID           ,
    .tpid_entry_idx_max                 = APOLLO_TPID_ENTRY_IDX_MAX              ,
    .tpid_entry_mask_max                = APOLLO_TPID_ENTRY_MASK_MAX             ,
    .protocol_vlan_idx_max              = APOLLO_PROTOCOL_VLAN_IDX_MAX           ,
    .max_num_vlan_mbrcfg                = APOLLO_MAX_NUM_VLAN_MBRCFG             ,
    .vlan_fid_max                       = APOLLO_VLAN_FID_MAX                    ,
    .flowctrl_thresh_max                = APOLLO_FLOWCTRL_THRESH_MAX             ,
    .flowctrl_pauseOn_page_packet_max   = APOLLO_FLOWCTRL_PAUSEON_PAGE_PACKET_MAX,
    .pri_of_selection_max               = APOLLO_PRI_OF_SELECTION_MAX            ,
    .pri_of_selection_min               = APOLLO_PRI_OF_SELECTION_MIN            ,
    .pri_sel_group_index_max            = APOLLO_PRI_SEL_GROUP_INDEX_MAX         ,
    .pri_sel_weight_max                 = APOLLO_PRI_SEL_WEIGHT_MAX              ,
    .queue_weight_max                   = APOLLO_QUEUE_WEIGHT_MAX                ,
    .qid0_weight_max                    = APOLLO_QID0_WEIGHT_MAX                 ,
    .rate_of_bandwidth_max              = APOLLO_RATE_OF_BANDWIDTH_MAX           ,
    .thresh_of_igr_bw_flowctrl_max      = APOLLO_THRESH_OF_IGR_BW_FLOWCTRL_MAX   ,
    .max_num_of_fastPath_of_rate        = APOLLO_MAX_NUM_OF_FASTPATH_OF_RATE     ,
    .rate_of_storm_control_max          = APOLLO_RATE_OF_STORM_CONTROL_MAX       ,
    .burst_rate_of_storm_control_max    = APOLLO_BURST_RATE_OF_STORM_CONTROL_MAX ,
    .internal_priority_max              = APOLLO_INTERNAL_PRIORITY_MAX           ,
    .drop_precedence_max                = APOLLO_DROP_PRECEDENCE_MAX             ,
    .priority_remap_group_idx_max       = APOLLO_PRIORITY_REMAP_GROUP_IDX_MAX    ,
    .priority_remark_group_idx_max      = APOLLO_PRIORITY_REMARK_GROUP_IDX_MAX   ,
    .priority_to_queue_group_idx_max    = APOLLO_PRIORITY_TO_QUEUE_GROUP_IDX_MAX ,
    .wred_weight_max                    = APOLLO_WRED_WEIGHT_MAX                 ,
    .wred_mpd_max                       = APOLLO_WRED_MPD_MAX                    ,
    .acl_rate_max                       = APOLLO_ACL_RATE_MAX                    ,
    .l2_learn_limit_cnt_max             = APOLLO_L2_LEARN_LIMIT_CNT_MAX          ,
    .l2_learn_lut_4way_no               = APOLLO_L2_LEARN_4WAY_NO                ,
    .l2_aging_time_max                  = APOLLO_L2_AGING_TIME_MAX               ,
    .l2_entry_aging_max                 = APOLLO_L2_ENTRY_AGING_MAX              ,
    .eee_queue_thresh_max               = APOLLO_EEE_QUEUE_THRESH_MAX            ,
    .sec_minIpv6FragLen_max             = APOLLO_SEC_MINIPV6FRAGLEN_MAX          ,
    .sec_maxPingLen_max                 = APOLLO_SEC_MAXPINGLEN_MAX              ,
    .sec_smurfNetmaskLen_max            = APOLLO_SEC_SMURFNETMASKLEN_MAX         ,
    .sflow_rate_max                     = APOLLO_SFLOW_RATE_MAX                  ,
    .max_num_of_mcast_fwd               = APOLLO_MAX_NUM_OF_MCAST_FWD            ,
    .miim_page_id_min                   = APOLLO_MIIM_PAGE_ID_MIN                ,
    .miim_page_id_max                   = APOLLO_MIIM_PAGE_ID_MAX                ,
    .miim_reg_id_max                    = APOLLO_MIIM_REG_ID_MAX                 ,
    .miim_data_max                      = APOLLO_MIIM_DATA_MAX                   ,
    .l34_netif_table_max                = APOLLO_L34_NETIF_TABLE_MAX             ,
    .l34_arp_table_max                  = APOLLO_L34_ARP_TABLE_MAX               ,
    .l34_extip_table_max                = APOLLO_L34_EXTIP_TABLE_MAX             ,
    .l34_routing_table_max              = APOLLO_L34_ROUTING_TABLE_MAX           ,
    .l34_napt_table_max                 = APOLLO_L34_NAPT_TABLE_MAX              ,
    .l34_naptr_table_max                = APOLLO_L34_NAPTR_TABLE_MAX             ,
    .l34_nh_table_max                   = APOLLO_L34_NH_TABLE_MAX                ,
    .l34_pppoe_table_max                = APOLLO_L34_PPPOE_TABLE_MAX             ,
    .gpon_tcont_max                     = APOLLO_GPON_TCONT_MAX                  ,
    .gpon_flow_max                      = APOLLO_GPON_FLOW_MAX                   ,
    .classify_entry_max                 = APOLLO_CLASSIFY_ENTRY_MAX              ,
    .classify_sid_max                   = APOLLO_CLASSIFY_SID_MAX                ,
    .classify_l4port_range_mum          = APOLLO_CLASSIFY_L4PORT_RANG_NUM        ,
    .classify_ip_range_mum              = APOLLO_CLASSIFY_IP_RANG_NUM            ,
    .max_num_of_acl_template            = APOLLO_MAX_NUM_OF_ACL_TEMPLATE         ,
    .max_num_of_acl_rule_field          = APOLLO_MAX_NUM_OF_ACL_RULE_FIELD       ,
    .max_num_of_acl_action              = APOLLO_MAX_NUM_OF_ACL_ACTION           ,
    .max_num_of_acl_rule_entry          = APOLLO_MAX_NUM_OF_ACL_RULE_ENTRY       ,
    .max_special_congest_second         = APOLLO_MAX_SPECIAL_CONGEST_SEC         ,
    .max_enhanced_fid                   = APOLLO_MAX_ENHANCED_FID                ,
    .max_num_of_log_mib                 = APOLLO_MAX_NUM_OF_LOG_MIB              ,
    .ext_cpu_port_id                    = APOLLO_EXT_CPU_PORT_ID                 ,
    .ponmac_tcont_queue_max             = APOLLO_PONMAC_TCONT_QUEUE_MAX          ,
    .ponmac_pir_cir_rate_max            = APOLLO_PONMAC_PIR_CIR_RATE_MAX         ,
    .max_mib_latch_timer                = APOLLO_MAX_MIB_LATCH_TIMER             ,
    .max_rgmii_tx_delay                 = APOLLO_MAX_RGMII_TX_DELAY              ,
    .max_rgmii_rx_delay                 = APOLLO_MAX_RGMII_RX_DELAY              ,
};

/* Normal APOLLO Chip PER_PORT block information */
static rt_macPpInfo_t apollo_macPpInfo =
{
    0x20000, /* lowerbound_addr */
    0x203FF, /* upperbound_addr */
    0x400,  /* interval */
};
#endif /* End of #if defined(CONFIG_SDK_APOLLO) */



#if defined(CONFIG_SDK_APOLLOMP)
static rt_portinfo_t apollomp_port_info =
{
    /* Normal APOLLO Chip Port Information */
    {
        RT_GE_PORT    /*P0 */, RT_GE_PORT    /*P1 */, RT_GE_PORT    /*P2 */, RT_GE_PORT    /*P3 */, RT_GE_PORT     /*P4 */,
        RT_GE_PORT    /*P5 */, RT_CPU_PORT    /*P6 */, RT_PORT_NONE  /*P7 */, RT_PORT_NONE  /*P8 */, RT_PORT_NONE   /*P9 */,
        RT_PORT_NONE  /*P10*/, RT_PORT_NONE  /*P11*/, RT_PORT_NONE /*P12*/, RT_PORT_NONE  /*P13*/, RT_PORT_NONE   /*P14*/,
        RT_PORT_NONE  /*P15*/, RT_PORT_NONE  /*P16*/, RT_PORT_NONE /*P17*/, RT_PORT_NONE  /*P18*/, RT_PORT_NONE   /*P19*/,
        RT_PORT_NONE  /*P20*/, RT_PORT_NONE  /*P21*/, RT_PORT_NONE /*P22*/, RT_PORT_NONE  /*P23*/, RT_PORT_NONE   /*P24*/,
        RT_PORT_NONE  /*P25*/, RT_PORT_NONE  /*P26*/, RT_PORT_NONE /*P27*/, RT_PORT_NONE  /*P28*/, RT_PORT_NONE   /*P29*/,
        RT_PORT_NONE  /*P30*/, RT_PORT_NONE  /*P31*/
     },
     /*DSL port member*/
     {
     	16, /*port number*/
     	15,	/*max*/
     	0,  /*min*/
     	{{0xFFFF}}/*port mask*/
     },
     /*extension port member*/
     {
     	6, /*port number*/
     	5,	/*max*/
     	0,  /*min*/
     	{{0x3F}}/*port mask*/
     },
     .ponPort   = 4,
     .rgmiiPort = 5
}; /* end of apollomp_port_info */



/*
Apollo MP port mapping

UTP0               0
UTP1               1
UTP2               2
UTP3               3
PON/fiber/UTP4     4
RGMII              5
CPU port           6

*/



/* Normal apollo MP Chip Port Information */
static rt_register_capacity_t apollomp_capacityInfo =
{
    .max_num_of_mirror                  = APOLLOMP_MAX_NUM_OF_MIRROR               ,
    .max_num_of_trunk                   = APOLLOMP_MAX_NUM_OF_TRUNK                ,
    .max_num_of_trunkMember             = APOLLOMP_MAX_NUM_OF_TRUNKMEMBER          ,
    .max_num_of_dumb_trunkMember        = APOLLOMP_MAX_NUM_OF_DUMB_TRUNKMEMBER     ,
    .max_num_of_trunkHashVal            = APOLLOMP_MAX_NUM_OF_TRUNKHASHVAL         ,
    .max_num_of_msti                    = APOLLOMP_MAX_NUM_OF_MSTI                 ,
    .max_num_of_metering                = APOLLOMP_MAX_NUM_OF_METERING             ,
    .max_num_of_field_selector          = APOLLOMP_MAX_NUM_OF_FIELD_SELECTOR       ,
    .max_num_of_range_check_srcPort     = APOLLOMP_MAX_NUM_OF_RANGE_CHECK_SRCPORT  ,
    .max_num_of_range_check_ip          = APOLLOMP_MAX_NUM_OF_RANGE_CHECK_IP       ,
    .max_num_of_range_check_vid         = APOLLOMP_MAX_NUM_OF_RANGE_CHECK_VID      ,
    .max_num_of_range_check_pktLen      = APOLLOMP_MAX_NUM_OF_RANGE_CHECK_PKTLEN   ,
    .max_num_of_range_check_l4Port      = APOLLOMP_MAX_NUM_OF_RANGE_CHECK_L4PORT   ,
    .max_num_of_pattern_match_data      = APOLLOMP_MAX_NUM_OF_PATTERN_MATCH_DATA   ,
    .pattern_match_port_max             = APOLLOMP_PATTERN_MATCH_PORT_MAX          ,
    .pattern_match_port_min             = APOLLOMP_PATTERN_MATCH_PORT_MIN          ,
    .max_num_of_l2_hashdepth            = APOLLOMP_MAX_NUM_OF_L2_HASHDEPTH         ,
    .max_num_of_queue                   = APOLLOMP_MAX_NUM_OF_QUEUE                ,
    .min_num_of_queue                   = APOLLOMP_MIN_NUM_OF_QUEUE                ,
    .max_num_of_pon_queue               = APOLLOMP_MAX_NUM_OF_PON_QUEUE            ,
    .min_num_of_pon_queue               = APOLLOMP_MIN_NUM_OF_PON_QUEUE            ,
    .max_num_of_cvlan_tpid              = APOLLOMP_MAX_NUM_OF_CVLAN_TPID           ,
    .max_num_of_svlan_tpid              = APOLLOMP_MAX_NUM_OF_SVLAN_TPID           ,
    .tpid_entry_idx_max                 = APOLLOMP_TPID_ENTRY_IDX_MAX              ,
    .tpid_entry_mask_max                = APOLLOMP_TPID_ENTRY_MASK_MAX             ,
    .protocol_vlan_idx_max              = APOLLOMP_PROTOCOL_VLAN_IDX_MAX           ,
    .max_num_vlan_mbrcfg                = APOLLOMP_MAX_NUM_VLAN_MBRCFG             ,
    .vlan_fid_max                       = APOLLOMP_VLAN_FID_MAX                    ,
    .flowctrl_thresh_max                = APOLLOMP_FLOWCTRL_THRESH_MAX             ,
    .flowctrl_pauseOn_page_packet_max   = APOLLOMP_FLOWCTRL_PAUSEON_PAGE_PACKET_MAX,
    .pri_of_selection_max               = APOLLOMP_PRI_OF_SELECTION_MAX            ,
    .pri_of_selection_min               = APOLLOMP_PRI_OF_SELECTION_MIN            ,
    .pri_sel_group_index_max            = APOLLOMP_PRI_SEL_GROUP_INDEX_MAX         ,
    .pri_sel_weight_max                 = APOLLOMP_PRI_SEL_WEIGHT_MAX              ,
    .queue_weight_max                   = APOLLOMP_QUEUE_WEIGHT_MAX                ,
    .qid0_weight_max                    = APOLLOMP_QID0_WEIGHT_MAX                 ,
    .rate_of_bandwidth_max              = APOLLOMP_RATE_OF_BANDWIDTH_MAX           ,
    .thresh_of_igr_bw_flowctrl_max      = APOLLOMP_THRESH_OF_IGR_BW_FLOWCTRL_MAX   ,
    .max_num_of_fastPath_of_rate        = APOLLOMP_MAX_NUM_OF_FASTPATH_OF_RATE     ,
    .rate_of_storm_control_max          = APOLLOMP_RATE_OF_STORM_CONTROL_MAX       ,
    .burst_rate_of_storm_control_max    = APOLLOMP_BURST_RATE_OF_STORM_CONTROL_MAX ,
    .internal_priority_max              = APOLLOMP_INTERNAL_PRIORITY_MAX           ,
    .drop_precedence_max                = APOLLOMP_DROP_PRECEDENCE_MAX             ,
    .priority_remap_group_idx_max       = APOLLOMP_PRIORITY_REMAP_GROUP_IDX_MAX    ,
    .priority_remark_group_idx_max      = APOLLOMP_PRIORITY_REMARK_GROUP_IDX_MAX   ,
    .priority_to_queue_group_idx_max    = APOLLOMP_PRIORITY_TO_QUEUE_GROUP_IDX_MAX ,
    .wred_weight_max                    = APOLLOMP_WRED_WEIGHT_MAX                 ,
    .wred_mpd_max                       = APOLLOMP_WRED_MPD_MAX                    ,
    .acl_rate_max                       = APOLLOMP_ACL_RATE_MAX                    ,
    .l2_learn_limit_cnt_max             = APOLLOMP_L2_LEARN_LIMIT_CNT_MAX          ,
    .l2_learn_lut_4way_no               = APOLLOMP_L2_LEARN_4WAY_NO                ,
    .l2_aging_time_max                  = APOLLOMP_L2_AGING_TIME_MAX               ,
    .l2_entry_aging_max                 = APOLLOMP_L2_ENTRY_AGING_MAX              ,
    .eee_queue_thresh_max               = APOLLOMP_EEE_QUEUE_THRESH_MAX            ,
    .sec_minIpv6FragLen_max             = APOLLOMP_SEC_MINIPV6FRAGLEN_MAX          ,
    .sec_maxPingLen_max                 = APOLLOMP_SEC_MAXPINGLEN_MAX              ,
    .sec_smurfNetmaskLen_max            = APOLLOMP_SEC_SMURFNETMASKLEN_MAX         ,
    .sflow_rate_max                     = APOLLOMP_SFLOW_RATE_MAX                  ,
    .max_num_of_mcast_fwd               = APOLLOMP_MAX_NUM_OF_MCAST_FWD            ,
    .miim_page_id_min                   = APOLLOMP_MIIM_PAGE_ID_MIN                ,
    .miim_page_id_max                   = APOLLOMP_MIIM_PAGE_ID_MAX                ,
    .miim_reg_id_max                    = APOLLOMP_MIIM_REG_ID_MAX                 ,
    .miim_data_max                      = APOLLOMP_MIIM_DATA_MAX                   ,
    .l34_netif_table_max                = APOLLOMP_L34_NETIF_TABLE_MAX             ,
    .l34_arp_table_max                  = APOLLOMP_L34_ARP_TABLE_MAX               ,
    .l34_extip_table_max                = APOLLOMP_L34_EXTIP_TABLE_MAX             ,
    .l34_routing_table_max              = APOLLOMP_L34_ROUTING_TABLE_MAX           ,
    .l34_napt_table_max                 = APOLLOMP_L34_NAPT_TABLE_MAX              ,
    .l34_naptr_table_max                = APOLLOMP_L34_NAPTR_TABLE_MAX             ,
    .l34_nh_table_max                   = APOLLOMP_L34_NH_TABLE_MAX                ,
    .l34_pppoe_table_max                = APOLLOMP_L34_PPPOE_TABLE_MAX             ,
    .l34_ipv6_routing_table_max       = APOLLOMP_L34_IPV6_ROUTING_TABLE_MAX,
    .l34_ipv6_neighbor_table_max    =  APOLLOMP_L34_IPV6_NBR_TABLE_MAX,
    .l34_binding_table_max	         =  APOLLOMP_L34_BINDING_TABLE_MAX,
    .l34_wan_type_table_max	  =  APOLLOMP_L34_WAN_TYPE_TABLE_MAX,
    .l34_ipmc_tran_table_max            = APOLLOMP_L34_IPMC_TRAN_TABLE_MAX         ,
    .gpon_tcont_max                     =   APOLLOMP_GPON_TCONT_MAX                  ,
    .gpon_flow_max                      = APOLLOMP_GPON_FLOW_MAX                   ,
    .classify_entry_max                 = APOLLOMP_CLASSIFY_ENTRY_MAX              ,
    .classify_sid_max                   = APOLLOMP_CLASSIFY_SID_MAX                ,
    .classify_l4port_range_mum          = APOLLOMP_CLASSIFY_L4PORT_RANGE_NUM       ,
    .classify_ip_range_mum              = APOLLOMP_CLASSIFY_IP_RANGE_NUM            ,
    .max_num_of_acl_template            = APOLLOMP_MAX_NUM_OF_ACL_TEMPLATE         ,
    .max_num_of_acl_rule_field          = APOLLOMP_MAX_NUM_OF_ACL_RULE_FIELD       ,
    .max_num_of_acl_action              = APOLLOMP_MAX_NUM_OF_ACL_ACTION           ,
    .max_num_of_acl_rule_entry          = APOLLOMP_MAX_NUM_OF_ACL_RULE_ENTRY       ,
    .max_special_congest_second         = APOLLOMP_MAX_SPECIAL_CONGEST_SEC         ,
    .max_enhanced_fid                   = APOLLOMP_MAX_ENHANCED_FID                ,
    .max_num_of_log_mib                 = APOLLOMP_MAX_NUM_OF_LOG_MIB              ,
    .ext_cpu_port_id                    = APOLLOMP_EXT_CPU_PORT_ID                 ,
    .ponmac_tcont_queue_max             = APOLLOMP_PONMAC_TCONT_QUEUE_MAX            ,
    .ponmac_pir_cir_rate_max            = APOLLOMP_PONMAC_PIR_CIR_RATE_MAX           ,
    .max_mib_latch_timer                = APOLLOMP_MAX_MIB_LATCH_TIMER             ,
    .max_num_of_nexthop               	= APOLLOMP_MAX_NUM_OF_NEXTHOP			   ,
    .max_rgmii_tx_delay                 = APOLLOMP_MAX_RGMII_TX_DELAY              ,
    .max_rgmii_rx_delay                 = APOLLOMP_MAX_RGMII_RX_DELAY              ,
    .max_llid_entry                     = APOLLOMP_MAX_LLID_ENTRY              ,

};


/* Normal APOLLO MP Chip PER_PORT block information */
static rt_macPpInfo_t apollomp_macPpInfo =
{
    0x20000, /* lowerbound_addr */
    0x203FF, /* upperbound_addr */
    0x400,  /* interval */
};
#endif /* End of #if defined(CONFIG_SDK_APOLLOMP) */




#if defined(CONFIG_SDK_RTL9601B)
static rt_portinfo_t rtl9601b_port_info =
{
    /* Normal RTL9601B Chip Port Information */
    {
        RT_GE_PORT    /*P0 */, RT_GE_PORT    /*P1 */, RT_CPU_PORT   /*P2 */, RT_PORT_NONE  /*P3 */, RT_PORT_NONE     /*P4 */,
        RT_PORT_NONE  /*P5 */, RT_PORT_NONE  /*P6 */, RT_PORT_NONE  /*P7 */, RT_PORT_NONE  /*P8 */, RT_PORT_NONE   /*P9 */,
        RT_PORT_NONE  /*P10*/, RT_PORT_NONE  /*P11*/, RT_PORT_NONE  /*P12*/, RT_PORT_NONE  /*P13*/, RT_PORT_NONE   /*P14*/,
        RT_PORT_NONE  /*P15*/, RT_PORT_NONE  /*P16*/, RT_PORT_NONE  /*P17*/, RT_PORT_NONE  /*P18*/, RT_PORT_NONE   /*P19*/,
        RT_PORT_NONE  /*P20*/, RT_PORT_NONE  /*P21*/, RT_PORT_NONE  /*P22*/, RT_PORT_NONE  /*P23*/, RT_PORT_NONE   /*P24*/,
        RT_PORT_NONE  /*P25*/, RT_PORT_NONE  /*P26*/, RT_PORT_NONE  /*P27*/, RT_PORT_NONE  /*P28*/, RT_PORT_NONE   /*P29*/,
        RT_PORT_NONE  /*P30*/, RT_PORT_NONE  /*P31*/
     },
     /*DSL port member*/
     {
     	16, /*port number*/
     	15,	/*max*/
     	0,  /*min*/
     	{{0xFFFF}}/*port mask*/
     },
     /*extension port member*/
     {
     	6, /*port number*/
     	5,	/*max*/
     	0,  /*min*/
     	{{0x3F}}/*port mask*/
     },
     .ponPort   = 1,
     .rgmiiPort = -1
}; /* end of rtl9601b_port_info */

/*
RTL9601B port mapping

UTP0               0
PON/fiber/UTP1     1
CPU port           2

*/



/* Normal RTL9601B Chip Port Information */
static rt_register_capacity_t rtl9601b_capacityInfo =
{
    .max_num_of_mirror                  = RTL9601B_MAX_NUM_OF_MIRROR               ,
    .max_num_of_trunk                   = RTL9601B_MAX_NUM_OF_TRUNK                ,
    .max_num_of_trunkMember             = RTL9601B_MAX_NUM_OF_TRUNKMEMBER          ,
    .max_num_of_dumb_trunkMember        = RTL9601B_MAX_NUM_OF_DUMB_TRUNKMEMBER     ,
    .max_num_of_trunkHashVal            = RTL9601B_MAX_NUM_OF_TRUNKHASHVAL         ,
    .max_num_of_msti                    = RTL9601B_MAX_NUM_OF_MSTI                 ,
    .max_num_of_metering                = RTL9601B_MAX_NUM_OF_METERING             ,
    .max_num_of_field_selector          = RTL9601B_MAX_NUM_OF_FIELD_SELECTOR       ,
    .max_num_of_range_check_srcPort     = RTL9601B_MAX_NUM_OF_RANGE_CHECK_SRCPORT  ,
    .max_num_of_range_check_ip          = RTL9601B_MAX_NUM_OF_RANGE_CHECK_IP       ,
    .max_num_of_range_check_vid         = RTL9601B_MAX_NUM_OF_RANGE_CHECK_VID      ,
    .max_num_of_range_check_pktLen      = RTL9601B_MAX_NUM_OF_RANGE_CHECK_PKTLEN   ,
    .max_num_of_range_check_l4Port      = RTL9601B_MAX_NUM_OF_RANGE_CHECK_L4PORT   ,
    .max_num_of_pattern_match_data      = RTL9601B_MAX_NUM_OF_PATTERN_MATCH_DATA   ,
    .pattern_match_port_max             = RTL9601B_PATTERN_MATCH_PORT_MAX          ,
    .pattern_match_port_min             = RTL9601B_PATTERN_MATCH_PORT_MIN          ,
    .max_num_of_l2_hashdepth            = RTL9601B_MAX_NUM_OF_L2_HASHDEPTH         ,
    .max_num_of_queue                   = RTL9601B_MAX_NUM_OF_QUEUE                ,
    .min_num_of_queue                   = RTL9601B_MIN_NUM_OF_QUEUE                ,
    .max_num_of_pon_queue               = RTL9601B_MAX_NUM_OF_PON_QUEUE            ,
    .min_num_of_pon_queue               = RTL9601B_MIN_NUM_OF_PON_QUEUE            ,
    .max_num_of_cvlan_tpid              = RTL9601B_MAX_NUM_OF_CVLAN_TPID           ,
    .max_num_of_svlan_tpid              = RTL9601B_MAX_NUM_OF_SVLAN_TPID           ,
    .tpid_entry_idx_max                 = RTL9601B_TPID_ENTRY_IDX_MAX              ,
    .tpid_entry_mask_max                = RTL9601B_TPID_ENTRY_MASK_MAX             ,
    .protocol_vlan_idx_max              = RTL9601B_PROTOCOL_VLAN_IDX_MAX           ,
    .max_num_vlan_mbrcfg                = RTL9601B_MAX_NUM_VLAN_MBRCFG             ,
    .vlan_fid_max                       = RTL9601B_VLAN_FID_MAX                    ,
    .flowctrl_thresh_max                = RTL9601B_FLOWCTRL_THRESH_MAX             ,
    .flowctrl_pauseOn_page_packet_max   = RTL9601B_FLOWCTRL_PAUSEON_PAGE_PACKET_MAX,
    .pri_of_selection_max               = RTL9601B_PRI_OF_SELECTION_MAX            ,
    .pri_of_selection_min               = RTL9601B_PRI_OF_SELECTION_MIN            ,
    .pri_sel_group_index_max            = RTL9601B_PRI_SEL_GROUP_INDEX_MAX         ,
    .pri_sel_weight_max                 = RTL9601B_PRI_SEL_WEIGHT_MAX              ,
    .queue_weight_max                   = RTL9601B_QUEUE_WEIGHT_MAX                ,
    .qid0_weight_max                    = RTL9601B_QID0_WEIGHT_MAX                 ,
    .rate_of_bandwidth_max              = RTL9601B_RATE_OF_BANDWIDTH_MAX           ,
    .thresh_of_igr_bw_flowctrl_max      = RTL9601B_THRESH_OF_IGR_BW_FLOWCTRL_MAX   ,
    .max_num_of_fastPath_of_rate        = RTL9601B_MAX_NUM_OF_FASTPATH_OF_RATE     ,
    .rate_of_storm_control_max          = RTL9601B_RATE_OF_STORM_CONTROL_MAX       ,
    .burst_rate_of_storm_control_max    = RTL9601B_BURST_RATE_OF_STORM_CONTROL_MAX ,
    .internal_priority_max              = RTL9601B_INTERNAL_PRIORITY_MAX           ,
    .drop_precedence_max                = RTL9601B_DROP_PRECEDENCE_MAX             ,
    .priority_remap_group_idx_max       = RTL9601B_PRIORITY_REMAP_GROUP_IDX_MAX    ,
    .priority_remark_group_idx_max      = RTL9601B_PRIORITY_REMARK_GROUP_IDX_MAX   ,
    .priority_to_queue_group_idx_max    = RTL9601B_PRIORITY_TO_QUEUE_GROUP_IDX_MAX ,
    .wred_weight_max                    = RTL9601B_WRED_WEIGHT_MAX                 ,
    .wred_mpd_max                       = RTL9601B_WRED_MPD_MAX                    ,
    .acl_rate_max                       = RTL9601B_ACL_RATE_MAX                    ,
    .l2_learn_limit_cnt_max             = RTL9601B_L2_LEARN_LIMIT_CNT_MAX          ,
    .l2_learn_lut_4way_no               = RTL9601B_L2_LEARN_4WAY_NO                ,
    .l2_aging_time_max                  = RTL9601B_L2_AGING_TIME_MAX               ,
    .l2_entry_aging_max                 = RTL9601B_L2_ENTRY_AGING_MAX              ,
    .eee_queue_thresh_max               = RTL9601B_EEE_QUEUE_THRESH_MAX            ,
    .sec_minIpv6FragLen_max             = RTL9601B_SEC_MINIPV6FRAGLEN_MAX          ,
    .sec_maxPingLen_max                 = RTL9601B_SEC_MAXPINGLEN_MAX              ,
    .sec_smurfNetmaskLen_max            = RTL9601B_SEC_SMURFNETMASKLEN_MAX         ,
    .sflow_rate_max                     = RTL9601B_SFLOW_RATE_MAX                  ,
    .max_num_of_mcast_fwd               = RTL9601B_MAX_NUM_OF_MCAST_FWD            ,
    .miim_page_id_min                   = RTL9601B_MIIM_PAGE_ID_MIN                ,
    .miim_page_id_max                   = RTL9601B_MIIM_PAGE_ID_MAX                ,
    .miim_reg_id_max                    = RTL9601B_MIIM_REG_ID_MAX                 ,
    .miim_data_max                      = RTL9601B_MIIM_DATA_MAX                   ,
    .l34_netif_table_max                = RTL9601B_L34_NETIF_TABLE_MAX             ,
    .l34_arp_table_max                  = RTL9601B_L34_ARP_TABLE_MAX               ,
    .l34_extip_table_max                = RTL9601B_L34_EXTIP_TABLE_MAX             ,
    .l34_routing_table_max              = RTL9601B_L34_ROUTING_TABLE_MAX           ,
    .l34_napt_table_max                 = RTL9601B_L34_NAPT_TABLE_MAX              ,
    .l34_naptr_table_max                = RTL9601B_L34_NAPTR_TABLE_MAX             ,
    .l34_nh_table_max                   = RTL9601B_L34_NH_TABLE_MAX                ,
    .l34_pppoe_table_max                = RTL9601B_L34_PPPOE_TABLE_MAX             ,
    .l34_ipv6_routing_table_max         = RTL9601B_L34_IPV6_ROUTING_TABLE_MAX,
    .l34_ipv6_neighbor_table_max        = RTL9601B_L34_IPV6_NBR_TABLE_MAX,
    .l34_binding_table_max	            = RTL9601B_L34_BINDING_TABLE_MAX,
    .l34_wan_type_table_max	            = RTL9601B_L34_WAN_TYPE_TABLE_MAX,
    .gpon_tcont_max                     = RTL9601B_GPON_TCONT_MAX                  ,
    .gpon_flow_max                      = RTL9601B_GPON_FLOW_MAX                   ,
    .classify_entry_max                 = RTL9601B_CLASSIFY_ENTRY_MAX              ,
    .classify_sid_max                   = RTL9601B_CLASSIFY_SID_MAX                ,
    .classify_l4port_range_mum          = RTL9601B_CLASSIFY_L4PORT_RANGE_NUM       ,
    .classify_ip_range_mum              = RTL9601B_CLASSIFY_IP_RANGE_NUM            ,
    .max_num_of_acl_template            = RTL9601B_MAX_NUM_OF_ACL_TEMPLATE         ,
    .max_num_of_acl_rule_field          = RTL9601B_MAX_NUM_OF_ACL_RULE_FIELD       ,
    .max_num_of_acl_action              = RTL9601B_MAX_NUM_OF_ACL_ACTION           ,
    .max_num_of_acl_rule_entry          = RTL9601B_MAX_NUM_OF_ACL_RULE_ENTRY       ,
    .max_special_congest_second         = RTL9601B_MAX_SPECIAL_CONGEST_SEC         ,
    .max_enhanced_fid                   = RTL9601B_MAX_ENHANCED_FID                ,
    .max_num_of_log_mib                 = RTL9601B_MAX_NUM_OF_LOG_MIB              ,
    .ext_cpu_port_id                    = RTL9601B_EXT_CPU_PORT_ID                 ,
    .ponmac_tcont_queue_max             = RTL9601B_PONMAC_TCONT_QUEUE_MAX            ,
    .ponmac_pir_cir_rate_max            = RTL9601B_PONMAC_PIR_CIR_RATE_MAX           ,
    .max_mib_latch_timer                = RTL9601B_MAX_MIB_LATCH_TIMER             ,
    .max_num_of_nexthop               	= RTL9601B_MAX_NUM_OF_NEXTHOP			   ,
    .max_rgmii_tx_delay                 = RTL9601B_MAX_RGMII_TX_DELAY              ,
    .max_rgmii_rx_delay                 = RTL9601B_MAX_RGMII_RX_DELAY              ,
    .max_llid_entry                     = RTL9601B_MAX_LLID_ENTRY              ,

};



/* Normal RTL9601B Chip PER_PORT block information */
static rt_macPpInfo_t rtl9601b_macPpInfo =
{
    0x20000, /* lowerbound_addr */
    0x203FF, /* upperbound_addr */
    0x400,  /* interval */
};
#endif /* End of #if defined(CONFIG_SDK_RTL9601B) */




#if defined(CONFIG_SDK_RTL9602C)
static rt_portinfo_t rtl9602c_port_info =
{
    /* Normal RTL9602C Chip Port Information */
    {
        RT_FE_PORT    /*P0 */, RT_GE_PORT    /*P1 */, RT_GE_PORT   /*P2 */, RT_CPU_PORT   /*P3 */, RT_PORT_NONE   /*P4 */,
        RT_PORT_NONE  /*P5 */, RT_PORT_NONE  /*P6 */, RT_PORT_NONE /*P7 */, RT_PORT_NONE  /*P8 */, RT_PORT_NONE   /*P9 */,
        RT_PORT_NONE  /*P10*/, RT_PORT_NONE  /*P11*/, RT_PORT_NONE /*P12*/, RT_PORT_NONE  /*P13*/, RT_PORT_NONE   /*P14*/,
        RT_PORT_NONE  /*P15*/, RT_PORT_NONE  /*P16*/, RT_PORT_NONE /*P17*/, RT_PORT_NONE  /*P18*/, RT_PORT_NONE   /*P19*/,
        RT_PORT_NONE  /*P20*/, RT_PORT_NONE  /*P21*/, RT_PORT_NONE /*P22*/, RT_PORT_NONE  /*P23*/, RT_PORT_NONE   /*P24*/,
        RT_PORT_NONE  /*P25*/, RT_PORT_NONE  /*P26*/, RT_PORT_NONE /*P27*/, RT_PORT_NONE  /*P28*/, RT_PORT_NONE   /*P29*/,
        RT_PORT_NONE  /*P30*/, RT_PORT_NONE  /*P31*/
     },
     /*DSL port member*/
     {
     	0, /*port number*/
     	0,	/*max*/
     	0,  /*min*/
     	{{0x0000}}/*port mask*/
     },
     /*extension port member*/
     {
     	7, /*port number*/
     	6,	/*max*/
     	0,  /*min*/
     	{{0x7f}}/*port mask*/
     },
     .ponPort   = 2,
     .rgmiiPort = -1
}; /* end of rtl9602c_port_info */

/*
RTL9602C port mapping

UTP0               0
PON/fiber/UTP1     1
CPU port           2

*/



/* Normal RTL9602C Chip Port Information */
static rt_register_capacity_t rtl9602c_capacityInfo =
{
    .max_num_of_mirror                  = RTL9602C_MAX_NUM_OF_MIRROR               ,
    .max_num_of_trunk                   = RTL9602C_MAX_NUM_OF_TRUNK                ,
    .max_num_of_trunkMember             = RTL9602C_MAX_NUM_OF_TRUNKMEMBER          ,
    .max_num_of_dumb_trunkMember        = RTL9602C_MAX_NUM_OF_DUMB_TRUNKMEMBER     ,
    .max_num_of_trunkHashVal            = RTL9602C_MAX_NUM_OF_TRUNKHASHVAL         ,
    .max_num_of_msti                    = RTL9602C_MAX_NUM_OF_MSTI                 ,
    .max_num_of_metering                = RTL9602C_MAX_NUM_OF_METERING             ,
    .max_num_of_field_selector          = RTL9602C_MAX_NUM_OF_FIELD_SELECTOR       ,
    .max_num_of_range_check_srcPort     = RTL9602C_MAX_NUM_OF_RANGE_CHECK_SRCPORT  ,
    .max_num_of_range_check_ip          = RTL9602C_MAX_NUM_OF_RANGE_CHECK_IP       ,
    .max_num_of_range_check_vid         = RTL9602C_MAX_NUM_OF_RANGE_CHECK_VID      ,
    .max_num_of_range_check_pktLen      = RTL9602C_MAX_NUM_OF_RANGE_CHECK_PKTLEN   ,
    .max_num_of_range_check_l4Port      = RTL9602C_MAX_NUM_OF_RANGE_CHECK_L4PORT   ,
    .max_num_of_pattern_match_data      = RTL9602C_MAX_NUM_OF_PATTERN_MATCH_DATA   ,
    .pattern_match_port_max             = RTL9602C_PATTERN_MATCH_PORT_MAX          ,
    .pattern_match_port_min             = RTL9602C_PATTERN_MATCH_PORT_MIN          ,
    .max_num_of_l2_hashdepth            = RTL9602C_MAX_NUM_OF_L2_HASHDEPTH         ,
    .max_num_of_queue                   = RTL9602C_MAX_NUM_OF_QUEUE                ,
    .min_num_of_queue                   = RTL9602C_MIN_NUM_OF_QUEUE                ,
    .max_num_of_pon_queue               = RTL9602C_MAX_NUM_OF_PON_QUEUE            ,
    .min_num_of_pon_queue               = RTL9602C_MIN_NUM_OF_PON_QUEUE            ,
    .max_num_of_cvlan_tpid              = RTL9602C_MAX_NUM_OF_CVLAN_TPID           ,
    .max_num_of_svlan_tpid              = RTL9602C_MAX_NUM_OF_SVLAN_TPID           ,
    .tpid_entry_idx_max                 = RTL9602C_TPID_ENTRY_IDX_MAX              ,
    .tpid_entry_mask_max                = RTL9602C_TPID_ENTRY_MASK_MAX             ,
    .protocol_vlan_idx_max              = RTL9602C_PROTOCOL_VLAN_IDX_MAX           ,
    .max_num_vlan_mbrcfg                = RTL9602C_MAX_NUM_VLAN_MBRCFG             ,
    .vlan_fid_max                       = RTL9602C_VLAN_FID_MAX                    ,
    .flowctrl_thresh_max                = RTL9602C_FLOWCTRL_THRESH_MAX             ,
    .flowctrl_pauseOn_page_packet_max   = RTL9602C_FLOWCTRL_PAUSEON_PAGE_PACKET_MAX,
    .pri_of_selection_max               = RTL9602C_PRI_OF_SELECTION_MAX            ,
    .pri_of_selection_min               = RTL9602C_PRI_OF_SELECTION_MIN            ,
    .pri_sel_group_index_max            = RTL9602C_PRI_SEL_GROUP_INDEX_MAX         ,
    .pri_sel_weight_max                 = RTL9602C_PRI_SEL_WEIGHT_MAX              ,
    .queue_weight_max                   = RTL9602C_QUEUE_WEIGHT_MAX                ,
    .qid0_weight_max                    = RTL9602C_QID0_WEIGHT_MAX                 ,
    .rate_of_bandwidth_max              = RTL9602C_RATE_OF_BANDWIDTH_MAX           ,
    .thresh_of_igr_bw_flowctrl_max      = RTL9602C_THRESH_OF_IGR_BW_FLOWCTRL_MAX   ,
    .max_num_of_fastPath_of_rate        = RTL9602C_MAX_NUM_OF_FASTPATH_OF_RATE     ,
    .rate_of_storm_control_max          = RTL9602C_RATE_OF_STORM_CONTROL_MAX       ,
    .burst_rate_of_storm_control_max    = RTL9602C_BURST_RATE_OF_STORM_CONTROL_MAX ,
    .internal_priority_max              = RTL9602C_INTERNAL_PRIORITY_MAX           ,
    .drop_precedence_max                = RTL9602C_DROP_PRECEDENCE_MAX             ,
    .priority_remap_group_idx_max       = RTL9602C_PRIORITY_REMAP_GROUP_IDX_MAX    ,
    .priority_remark_group_idx_max      = RTL9602C_PRIORITY_REMARK_GROUP_IDX_MAX   ,
    .priority_to_queue_group_idx_max    = RTL9602C_PRIORITY_TO_QUEUE_GROUP_IDX_MAX ,
    .wred_weight_max                    = RTL9602C_WRED_WEIGHT_MAX                 ,
    .wred_mpd_max                       = RTL9602C_WRED_MPD_MAX                    ,
    .acl_rate_max                       = RTL9602C_ACL_RATE_MAX                    ,
    .l2_learn_limit_cnt_max             = RTL9602C_L2_LEARN_LIMIT_CNT_MAX          ,
    .l2_learn_lut_4way_no               = RTL9602C_L2_LEARN_4WAY_NO                ,
    .l2_aging_time_max                  = RTL9602C_L2_AGING_TIME_MAX               ,
    .l2_entry_aging_max                 = RTL9602C_L2_ENTRY_AGING_MAX              ,
    .eee_queue_thresh_max               = RTL9602C_EEE_QUEUE_THRESH_MAX            ,
    .sec_minIpv6FragLen_max             = RTL9602C_SEC_MINIPV6FRAGLEN_MAX          ,
    .sec_maxPingLen_max                 = RTL9602C_SEC_MAXPINGLEN_MAX              ,
    .sec_smurfNetmaskLen_max            = RTL9602C_SEC_SMURFNETMASKLEN_MAX         ,
    .sflow_rate_max                     = RTL9602C_SFLOW_RATE_MAX                  ,
    .max_num_of_mcast_fwd               = RTL9602C_MAX_NUM_OF_MCAST_FWD            ,
    .miim_page_id_min                   = RTL9602C_MIIM_PAGE_ID_MIN                ,
    .miim_page_id_max                   = RTL9602C_MIIM_PAGE_ID_MAX                ,
    .miim_reg_id_max                    = RTL9602C_MIIM_REG_ID_MAX                 ,
    .miim_data_max                      = RTL9602C_MIIM_DATA_MAX                   ,
    .l34_netif_table_max                = RTL9602C_L34_NETIF_TABLE_MAX             ,
    .l34_arp_table_max                  = RTL9602C_L34_ARP_TABLE_MAX               ,
    .l34_extip_table_max                = RTL9602C_L34_EXTIP_TABLE_MAX             ,
    .l34_routing_table_max              = RTL9602C_L34_ROUTING_TABLE_MAX           ,
    .l34_napt_table_max                 = RTL9602C_L34_NAPT_TABLE_MAX              ,
    .l34_naptr_table_max                = RTL9602C_L34_NAPTR_TABLE_MAX             ,
    .l34_nh_table_max                   = RTL9602C_L34_NH_TABLE_MAX                ,
    .l34_pppoe_table_max                = RTL9602C_L34_PPPOE_TABLE_MAX             ,
    .l34_ipv6_routing_table_max         = RTL9602C_L34_IPV6_ROUTING_TABLE_MAX      ,
    .l34_ipv6_neighbor_table_max        = RTL9602C_L34_IPV6_NBR_TABLE_MAX          ,
    .l34_binding_table_max	            = RTL9602C_L34_BINDING_TABLE_MAX           ,
    .l34_wan_type_table_max	            = RTL9602C_L34_WAN_TYPE_TABLE_MAX          ,
    .l34_flow_route_table_max	        = RTL9602C_L34_FLOW_ROUTE_TABLE_MAX        ,
    .l34_dslite_table_max               = RTL9602C_L34_DSLITE_INF_TABLE_MAX        ,
    .l34_dslite_mc_table_max            = RTL9602C_L34_DSLITE_MCAST_TABLE_MAX      ,
    .l34_ipmc_tran_table_max            = RTL9602C_L34_IPMC_TRAN_TABLE_MAX         ,
    .gpon_tcont_max                     = RTL9602C_GPON_TCONT_MAX                  ,
    .gpon_flow_max                      = RTL9602C_GPON_FLOW_MAX                   ,
    .classify_entry_max                 = RTL9602C_CLASSIFY_ENTRY_MAX              ,
    .classify_sid_max                   = RTL9602C_CLASSIFY_SID_MAX                ,
    .classify_l4port_range_mum          = RTL9602C_CLASSIFY_L4PORT_RANGE_NUM       ,
    .classify_ip_range_mum              = RTL9602C_CLASSIFY_IP_RANGE_NUM           ,
    .max_num_of_acl_template            = RTL9602C_MAX_NUM_OF_ACL_TEMPLATE         ,
    .max_num_of_acl_rule_field          = RTL9602C_MAX_NUM_OF_ACL_RULE_FIELD       ,
    .max_num_of_acl_action              = RTL9602C_MAX_NUM_OF_ACL_ACTION           ,
    .max_num_of_acl_rule_entry          = RTL9602C_MAX_NUM_OF_ACL_RULE_ENTRY       ,
    .max_special_congest_second         = RTL9602C_MAX_SPECIAL_CONGEST_SEC         ,
    .max_enhanced_fid                   = RTL9602C_MAX_ENHANCED_FID                ,
    .max_num_of_log_mib                 = RTL9602C_MAX_NUM_OF_LOG_MIB              ,
	.max_num_of_host 					= RTL9602C_MAX_NUM_OF_HOST			   ,
    .ext_cpu_port_id                    = RTL9602C_EXT_CPU_PORT_ID                 ,
    .ponmac_tcont_queue_max             = RTL9602C_PONMAC_TCONT_QUEUE_MAX            ,
    .ponmac_pir_cir_rate_max            = RTL9602C_PONMAC_PIR_CIR_RATE_MAX           ,
    .max_mib_latch_timer                = RTL9602C_MAX_MIB_LATCH_TIMER             ,
    .max_num_of_nexthop               	= RTL9602C_MAX_NUM_OF_NEXTHOP			   ,
    .max_rgmii_tx_delay                 = RTL9602C_MAX_RGMII_TX_DELAY              ,
    .max_rgmii_rx_delay                 = RTL9602C_MAX_RGMII_RX_DELAY              ,
    .max_llid_entry                     = RTL9602C_MAX_LLID_ENTRY              ,

};



/* Normal RTL9602C Chip PER_PORT block information */
static rt_macPpInfo_t rtl9602c_macPpInfo =
{
    0x20000, /* lowerbound_addr */
    0x203FF, /* upperbound_addr */
    0x400,  /* interval */
};
#endif /* End of #if defined(CONFIG_SDK_RTL9602C) */



#if defined(CONFIG_SDK_RTL9607B)
static rt_portinfo_t rtl9607b_port_info =
{
    /* Normal RTL9607B Chip Port Information */
    {
        RT_GE_PORT    /*P0 */, RT_GE_PORT    /*P1 */, RT_GE_PORT   /*P2 */, RT_GE_PORT   /*P3 */, RT_GE_PORT   /*P4 */,
        RT_GE_PORT  /*P5 */, RT_GE_SERDES_PORT  /*P6 */, RT_GE_SERDES_PORT /*P7 */, RT_GE_PORT  /*P8 */, RT_CPU_PORT   /*P9 */,
        RT_CPU_PORT  /*P10*/, RT_PORT_NONE  /*P11*/, RT_PORT_NONE /*P12*/, RT_PORT_NONE  /*P13*/, RT_PORT_NONE   /*P14*/,
        RT_PORT_NONE  /*P15*/, RT_PORT_NONE  /*P16*/, RT_PORT_NONE /*P17*/, RT_PORT_NONE  /*P18*/, RT_PORT_NONE   /*P19*/,
        RT_PORT_NONE  /*P20*/, RT_PORT_NONE  /*P21*/, RT_PORT_NONE /*P22*/, RT_PORT_NONE  /*P23*/, RT_PORT_NONE   /*P24*/,
        RT_PORT_NONE  /*P25*/, RT_PORT_NONE  /*P26*/, RT_PORT_NONE /*P27*/, RT_PORT_NONE  /*P28*/, RT_PORT_NONE   /*P29*/,
        RT_PORT_NONE  /*P30*/, RT_PORT_NONE  /*P31*/
     },
     /*DSL port member*/
     {
     	0, /*port number*/
     	0,	/*max*/
     	0,  /*min*/
     	{{0x0000}}/*port mask*/
     },
     /*extension port member*/
     {
     	18, /*port number*/
     	17,	/*max*/
     	0,  /*min*/
     	{{0x3ffff}}/*port mask*/
     },
     .ponPort   = 5,
     .rgmiiPort = 8
}; /* end of rtl9602c_port_info */

/*
RTL9607B port mapping

UTP0               0
PON/fiber/UTP1     5
CPU port           9

*/



/* Normal RTL9607B Chip Port Information */
static rt_register_capacity_t rtl9607b_capacityInfo =
{
    .max_num_of_mirror                  = RTL9607B_MAX_NUM_OF_MIRROR               ,
    .max_num_of_trunk                   = RTL9607B_MAX_NUM_OF_TRUNK                ,
    .max_num_of_trunkMember             = RTL9607B_MAX_NUM_OF_TRUNKMEMBER          ,
    .max_num_of_dumb_trunkMember        = RTL9607B_MAX_NUM_OF_DUMB_TRUNKMEMBER     ,
    .max_num_of_trunkHashVal            = RTL9607B_MAX_NUM_OF_TRUNKHASHVAL         ,
    .max_num_of_msti                    = RTL9607B_MAX_NUM_OF_MSTI                 ,
    .max_num_of_metering                = RTL9607B_MAX_NUM_OF_METERING             ,
    .max_num_of_field_selector          = RTL9607B_MAX_NUM_OF_FIELD_SELECTOR       ,
    .max_num_of_range_check_srcPort     = RTL9607B_MAX_NUM_OF_RANGE_CHECK_SRCPORT  ,
    .max_num_of_range_check_ip          = RTL9607B_MAX_NUM_OF_RANGE_CHECK_IP       ,
    .max_num_of_range_check_vid         = RTL9607B_MAX_NUM_OF_RANGE_CHECK_VID      ,
    .max_num_of_range_check_pktLen      = RTL9607B_MAX_NUM_OF_RANGE_CHECK_PKTLEN   ,
    .max_num_of_range_check_l4Port      = RTL9607B_MAX_NUM_OF_RANGE_CHECK_L4PORT   ,
    .max_num_of_pattern_match_data      = RTL9607B_MAX_NUM_OF_PATTERN_MATCH_DATA   ,
    .pattern_match_port_max             = RTL9607B_PATTERN_MATCH_PORT_MAX          ,
    .pattern_match_port_min             = RTL9607B_PATTERN_MATCH_PORT_MIN          ,
    .max_num_of_l2_hashdepth            = RTL9607B_MAX_NUM_OF_L2_HASHDEPTH         ,
    .max_num_of_queue                   = RTL9607B_MAX_NUM_OF_QUEUE                ,
    .min_num_of_queue                   = RTL9607B_MIN_NUM_OF_QUEUE                ,
    .max_num_of_pon_queue               = RTL9607B_MAX_NUM_OF_PON_QUEUE            ,
    .min_num_of_pon_queue               = RTL9607B_MIN_NUM_OF_PON_QUEUE            ,
    .max_num_of_cvlan_tpid              = RTL9607B_MAX_NUM_OF_CVLAN_TPID           ,
    .max_num_of_svlan_tpid              = RTL9607B_MAX_NUM_OF_SVLAN_TPID           ,
    .tpid_entry_idx_max                 = RTL9607B_TPID_ENTRY_IDX_MAX              ,
    .tpid_entry_mask_max                = RTL9607B_TPID_ENTRY_MASK_MAX             ,
    .protocol_vlan_idx_max              = RTL9607B_PROTOCOL_VLAN_IDX_MAX           ,
    .max_num_vlan_mbrcfg                = RTL9607B_MAX_NUM_VLAN_MBRCFG             ,
    .vlan_fid_max                       = RTL9607B_VLAN_FID_MAX                    ,
    .flowctrl_thresh_max                = RTL9607B_FLOWCTRL_THRESH_MAX             ,
    .flowctrl_pauseOn_page_packet_max   = RTL9607B_FLOWCTRL_PAUSEON_PAGE_PACKET_MAX,
    .pri_of_selection_max               = RTL9607B_PRI_OF_SELECTION_MAX            ,
    .pri_of_selection_min               = RTL9607B_PRI_OF_SELECTION_MIN            ,
    .pri_sel_group_index_max            = RTL9607B_PRI_SEL_GROUP_INDEX_MAX         ,
    .pri_sel_weight_max                 = RTL9607B_PRI_SEL_WEIGHT_MAX              ,
    .queue_weight_max                   = RTL9607B_QUEUE_WEIGHT_MAX                ,
    .qid0_weight_max                    = RTL9607B_QID0_WEIGHT_MAX                 ,
    .rate_of_bandwidth_max              = RTL9607B_RATE_OF_BANDWIDTH_MAX           ,
    .thresh_of_igr_bw_flowctrl_max      = RTL9607B_THRESH_OF_IGR_BW_FLOWCTRL_MAX   ,
    .max_num_of_fastPath_of_rate        = RTL9607B_MAX_NUM_OF_FASTPATH_OF_RATE     ,
    .rate_of_storm_control_max          = RTL9607B_RATE_OF_STORM_CONTROL_MAX       ,
    .burst_rate_of_storm_control_max    = RTL9607B_BURST_RATE_OF_STORM_CONTROL_MAX ,
    .internal_priority_max              = RTL9607B_INTERNAL_PRIORITY_MAX           ,
    .drop_precedence_max                = RTL9607B_DROP_PRECEDENCE_MAX             ,
    .priority_remap_group_idx_max       = RTL9607B_PRIORITY_REMAP_GROUP_IDX_MAX    ,
    .priority_remark_group_idx_max      = RTL9607B_PRIORITY_REMARK_GROUP_IDX_MAX   ,
    .priority_to_queue_group_idx_max    = RTL9607B_PRIORITY_TO_QUEUE_GROUP_IDX_MAX ,
    .wred_weight_max                    = RTL9607B_WRED_WEIGHT_MAX                 ,
    .wred_mpd_max                       = RTL9607B_WRED_MPD_MAX                    ,
    .acl_rate_max                       = RTL9607B_ACL_RATE_MAX                    ,
    .l2_learn_limit_cnt_max             = RTL9607B_L2_LEARN_LIMIT_CNT_MAX          ,
    .l2_learn_lut_4way_no               = RTL9607B_L2_LEARN_4WAY_NO                ,
    .l2_aging_time_max                  = RTL9607B_L2_AGING_TIME_MAX               ,
    .l2_entry_aging_max                 = RTL9607B_L2_ENTRY_AGING_MAX              ,
    .eee_queue_thresh_max               = RTL9607B_EEE_QUEUE_THRESH_MAX            ,
    .sec_minIpv6FragLen_max             = RTL9607B_SEC_MINIPV6FRAGLEN_MAX          ,
    .sec_maxPingLen_max                 = RTL9607B_SEC_MAXPINGLEN_MAX              ,
    .sec_smurfNetmaskLen_max            = RTL9607B_SEC_SMURFNETMASKLEN_MAX         ,
    .sflow_rate_max                     = RTL9607B_SFLOW_RATE_MAX                  ,
    .max_num_of_mcast_fwd               = RTL9607B_MAX_NUM_OF_MCAST_FWD            ,
    .miim_page_id_min                   = RTL9607B_MIIM_PAGE_ID_MIN                ,
    .miim_page_id_max                   = RTL9607B_MIIM_PAGE_ID_MAX                ,
    .miim_reg_id_max                    = RTL9607B_MIIM_REG_ID_MAX                 ,
    .miim_data_max                      = RTL9607B_MIIM_DATA_MAX                   ,
    .l34_netif_table_max                = RTL9607B_L34_NETIF_TABLE_MAX             ,
    .l34_arp_table_max                  = RTL9607B_L34_ARP_TABLE_MAX               ,
    .l34_extip_table_max                = RTL9607B_L34_EXTIP_TABLE_MAX             ,
    .l34_routing_table_max              = RTL9607B_L34_ROUTING_TABLE_MAX           ,
    .l34_napt_table_max                 = RTL9607B_L34_NAPT_TABLE_MAX              ,
    .l34_naptr_table_max                = RTL9607B_L34_NAPTR_TABLE_MAX             ,
    .l34_nh_table_max                   = RTL9607B_L34_NH_TABLE_MAX                ,
    .l34_pppoe_table_max                = RTL9607B_L34_PPPOE_TABLE_MAX             ,
    .l34_ipv6_routing_table_max         = RTL9607B_L34_IPV6_ROUTING_TABLE_MAX      ,
    .l34_ipv6_neighbor_table_max        = RTL9607B_L34_IPV6_NBR_TABLE_MAX          ,
    .l34_binding_table_max	            = RTL9607B_L34_BINDING_TABLE_MAX           ,
    .l34_wan_type_table_max	            = RTL9607B_L34_WAN_TYPE_TABLE_MAX          ,
    .l34_flow_route_table_max	        = RTL9607B_L34_FLOW_ROUTE_TABLE_MAX        ,
    .l34_dslite_table_max               = RTL9607B_L34_DSLITE_INF_TABLE_MAX        ,
    .l34_dslite_mc_table_max            = RTL9607B_L34_DSLITE_MCAST_TABLE_MAX      ,
    .l34_ipmc_tran_table_max            = RTL9607B_L34_IPMC_TRAN_TABLE_MAX         ,
    .gpon_tcont_max                     = RTL9607B_GPON_TCONT_MAX                  ,
    .gpon_flow_max                      = RTL9607B_GPON_FLOW_MAX                   ,
    .classify_entry_max                 = RTL9607B_CLASSIFY_ENTRY_MAX              ,
    .classify_sid_max                   = RTL9607B_CLASSIFY_SID_MAX                ,
    .classify_l4port_range_mum          = RTL9607B_CLASSIFY_L4PORT_RANGE_NUM       ,
    .classify_ip_range_mum              = RTL9607B_CLASSIFY_IP_RANGE_NUM           ,
    .max_num_of_acl_template            = RTL9607B_MAX_NUM_OF_ACL_TEMPLATE         ,
    .max_num_of_acl_rule_field          = RTL9607B_MAX_NUM_OF_ACL_RULE_FIELD       ,
    .max_num_of_acl_action              = RTL9607B_MAX_NUM_OF_ACL_ACTION           ,
    .max_num_of_acl_rule_entry          = RTL9607B_MAX_NUM_OF_ACL_RULE_ENTRY       ,
    .max_special_congest_second         = RTL9607B_MAX_SPECIAL_CONGEST_SEC         ,
    .max_enhanced_fid                   = RTL9607B_MAX_ENHANCED_FID                ,
    .max_num_of_log_mib                 = RTL9607B_MAX_NUM_OF_LOG_MIB              ,
	.max_num_of_host 					= RTL9607B_MAX_NUM_OF_HOST			   ,
    .ext_cpu_port_id                    = RTL9607B_EXT_CPU_PORT_ID                 ,
    .ponmac_tcont_queue_max             = RTL9607B_PONMAC_TCONT_QUEUE_MAX            ,
    .ponmac_pir_cir_rate_max            = RTL9607B_PONMAC_PIR_CIR_RATE_MAX           ,
    .max_mib_latch_timer                = RTL9607B_MAX_MIB_LATCH_TIMER             ,
    .max_num_of_nexthop               	= RTL9607B_MAX_NUM_OF_NEXTHOP			   ,
    .max_rgmii_tx_delay                 = RTL9607B_MAX_RGMII_TX_DELAY              ,
    .max_rgmii_rx_delay                 = RTL9607B_MAX_RGMII_RX_DELAY              ,
    .max_llid_entry                     = RTL9607B_MAX_LLID_ENTRY              ,

};



/* Normal RTL9607B Chip PER_PORT block information */
static rt_macPpInfo_t rtl9607b_macPpInfo =
{
    0x20000, /* lowerbound_addr */
    0x203FF, /* upperbound_addr */
    0x400,  /* interval */
};
#endif /* End of #if defined(CONFIG_SDK_RTL9607B) */


/* Supported mac chip lists */
static rt_device_t supported_devices[] =
{
#if defined(CONFIG_SDK_APOLLO)
    /* RT_DEVICE_APOLLO */
    {
            APOLLO_CHIP_ID,
            CHIP_REV_ID_0,
            APOLLO_CHIP_ID,
            CHIP_REV_ID_0,
            CHIP_AFLAG_LEXRA,
            &apollo_port_info,
            &apollo_capacityInfo,
            &apollo_macPpInfo
    },
    /* RT_DEVICE_APOLLO_REV_B */
    {
            APOLLO_CHIP_ID,
            CHIP_REV_ID_A,
            APOLLO_CHIP_ID,
            CHIP_REV_ID_A,
            CHIP_AFLAG_LEXRA,
            &apollo_port_info,
            &apollo_capacityInfo,
            &apollo_macPpInfo
    },
#endif
#if defined(CONFIG_SDK_APOLLOMP)
    /* RT_DEVICE_APOLLOMP */
    {
            APOLLOMP_CHIP_ID,
            CHIP_REV_ID_A,
            APOLLOMP_CHIP_ID,
            CHIP_REV_ID_A,
            CHIP_AFLAG_LEXRA,
            &apollomp_port_info,
            &apollomp_capacityInfo,
            &apollomp_macPpInfo
    },
#endif
#if defined(CONFIG_SDK_RTL9601B)
    /* RT_DEVICE_RTL9601B */
    {
            RTL9601B_CHIP_ID,
            CHIP_REV_ID_0,
            RTL9601B_CHIP_ID,
            CHIP_REV_ID_0,
            CHIP_AFLAG_LEXRA,
            &rtl9601b_port_info,
            &rtl9601b_capacityInfo,
            &rtl9601b_macPpInfo
    },
    /* RT_DEVICE_RTL9601B_REV_B */
    {
            RTL9601B_CHIP_ID,
            CHIP_REV_ID_A,
            RTL9601B_CHIP_ID,
            CHIP_REV_ID_A,
            CHIP_AFLAG_LEXRA,
            &rtl9601b_port_info,
            &rtl9601b_capacityInfo,
            &rtl9601b_macPpInfo
    },
#endif
#if defined(CONFIG_SDK_RTL9602C)
    /* RT_DEVICE_RTL9602C */
    {
            RTL9602C_CHIP_ID,
            CHIP_REV_ID_A,
            RTL9602C_CHIP_ID,
            CHIP_REV_ID_A,
            CHIP_AFLAG_LEXRA,
            &rtl9602c_port_info,
            &rtl9602c_capacityInfo,
            &rtl9602c_macPpInfo
    },
#endif
#if defined(CONFIG_SDK_RTL9607B)
    /* RT_DEVICE_RTL9607B */
    {
            RTL9607B_CHIP_ID,
            CHIP_REV_ID_A,
            RTL9607B_CHIP_ID,
            CHIP_REV_ID_A,
            CHIP_AFLAG_LEXRA,
            &rtl9607b_port_info,
            &rtl9607b_capacityInfo,
            &rtl9607b_macPpInfo
    },
#endif
};


/*
 * Macro Definition
 */



/* Function Name:
 *      hal_isPpBlock_check
 * Description:
 *      Check the register is PER_PORT block or not?
 * Input:
 *      addr       - register address
 * Output:
 *      pIsPpBlock - pointer buffer of chip is PER_PORT block?
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - failed
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32
hal_isPpBlock_check(uint32 addr, uint32 *pIsPpBlock)
{
    /* parameter check */
    RT_PARAM_CHK((NULL == pIsPpBlock), RT_ERR_NULL_POINTER);

#if 1
    if (addr < HAL_GET_MACPP_MIN_ADDR() || addr > HAL_GET_MACPP_MAX_ADDR())
        *pIsPpBlock = FALSE;
    else
        *pIsPpBlock = TRUE;
#else
    *pIsPpBlock = FALSE;
#endif


    return RT_ERR_OK;
} /* end of hal_isPpBlock_check */



/* Function Name:
 *      hal_find_device
 * Description:
 *      Find the mac chip from SDK supported mac device lists.
 * Input:
 *      chip_id     - chip id
 *      chip_rev_id - chip revision id
 * Output:
 *      None
 * Return:
 *      NULL        - Not found
 *      Otherwise   - Pointer of mac chip structure that found
 * Note:
 *      The function have take care the forward compatible in revision.
 *      Return one recently revision if no extra match revision.
 */
rt_device_t *
hal_find_device(uint32 chip_id, uint32 chip_rev_id)
{
    uint32  dev_idx;
    uint32  most_rev_id = 0;
    rt_device_t *pMatchDevice = NULL;


    RT_PARAM_CHK((chip_rev_id > CHIP_REV_ID_MAX), NULL);

    /* find out appropriate supported revision from supported_devices lists
     */
    for (dev_idx = 0; dev_idx < RT_DEVICE_END; dev_idx++)
    {
        if (supported_devices[dev_idx].chip_id == chip_id)
        {
            if (supported_devices[dev_idx].chip_rev_id == chip_rev_id)
            {
                /* Match and return this MAC device */
                return (&supported_devices[dev_idx]);
            }
            else if ((supported_devices[dev_idx].chip_rev_id < chip_rev_id) &&
                     (supported_devices[dev_idx].chip_rev_id >= most_rev_id))
            {
                /* Match better candidate of MAC device */
                most_rev_id = supported_devices[dev_idx].chip_rev_id;
                pMatchDevice = &supported_devices[dev_idx];
            }
        }
    }

    return (pMatchDevice);
} /* end of hal_find_device */
