/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*/

#ifndef __RTK_RG_ASIC_APOLLOPROFB_H__
#define __RTK_RG_ASIC_APOLLOPROFB_H__


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <rtk_rg_struct.h>

/*
 * Symbol Definition
 */

//#define RTL9602C_L34_HSB_WORD 10


/*
 * Data Declaration
 */
 
/* flow-based global state */
typedef enum rtk_rg_asic_globalStateType_s
{
    	FB_GLOBAL_STATE_START = 0,
	FB_GLOBAL_TTL_1 = FB_GLOBAL_STATE_START,
	FB_GLOBAL_TRAP_TCP_SYN_FIN_REST,
	FB_GLOBAL_TRAP_TCP_SYC_ACK,
	FB_GLOBAL_TRAP_FRAGMENT,
	FB_GLOBAL_L3_CS_CHK,
	FB_GLOBAL_L4_CS_CHK,
	FB_GLOBAL_PATH12_SKIP_CVID,
	FB_GLOBAL_PATH34_UCBC_SKIP_CVID,
	FB_GLOBAL_PATH34_MC_SKIP_CVID,
	FB_GLOBAL_PATH5_SKIP_CVID,
	FB_GLOBAL_PATH6_SKIP_CVID,
	FB_GLOBAL_PATH12_SKIP_SVID,
	FB_GLOBAL_PATH34_UCBC_SKIP_SVID,
	FB_GLOBAL_PATH34_MC_SKIP_SVID,
	FB_GLOBAL_PATH5_SKIP_SVID,
	FB_GLOBAL_PATH6_SKIP_SVID,
	FB_GLOBAL_L2_FLOW_LOOKUP_BY_MAC,
	FB_GLOBAL_STATE_END
}rtk_rg_asic_globalStateType_t;

/* flow-based forwarding action */
typedef enum rtk_rg_asic_forwardAction_s
{
   	FB_ACTION_FORWARD = 0,
	FB_ACTION_TRAP2CPU,
	FB_ACTION_DROP,
	FB_ACTION_END
}rtk_rg_asic_forwardAction_t;

/* flow-based table type for reset */
typedef enum rtk_rg_asic_resetTableType_s
{
	FB_RST_FLOW_TBL = 0,
    	FB_RST_TCAM_TBL,
	FB_RST_IF_TBL,
	FB_RST_ETHER_TYPE,
	FB_RST_EXT_PMASK_TYPE,
	FB_RST_MAC_INDEX_TBL,
	FB_RST_ALL,
	FB_RST_END
}rtk_rg_asic_resetTableType_t;



/* flow-based table - for path 1,2  */
typedef struct rtk_rg_asic_path1To2_entry_s
{
   	uint8 valid;
	uint8 out_dmac_trans;
	uint8 in_path;
	uint32 in_intf_idx;
	union
	{
		uint8 in_stream_idx_check;
		uint8 out_stream_idx_act;
	};	
	uint8 in_multiple_act;
	uint8 out_share_meter_act;
	uint32 out_share_meter_idx;
	uint8 in_spa_check;
	uint8 in_ctagif;
	uint8 in_stagif;
	uint8 in_pppoeif;
	uint32 out_intf_idx;
	uint8 in_dscp_check;
	uint16 in_dscp;
	uint8 in_protocol;
	uint32 in_smac_lut_idx;
	uint32 in_dmac_lut_idx;
	uint16 in_svlan_id;
	uint16 in_cvlan_id;
	uint8 in_spa;
	uint8 in_ext_spa;
	uint16 in_pppoe_sid;
	uint32 out_dmac_idx;
	uint8 out_svid_format_act;
	uint16 out_user_priority;
	uint8 out_cvid_format_act;
	uint32 out_extra_tag_index;
	uint8 out_uc_lut_lookup;
	rtk_portmask_t out_portmask;
	uint8 out_ext_portmask_idx;
	uint8 out_spri_format_act;
	uint16 out_spri;
	uint16 out_svlan_id;
	uint8 out_cpri_format_act;
	uint16 out_cpri;
	uint16 out_cvlan_id;
	uint16 in_pppoe_sid_check;
	union
	{
		uint8 in_stream_idx;
		uint8 out_stream_idx;
	};
	uint8 out_multiple_act;
	uint8 out_dscp_act;
	uint16 out_dscp;
	uint8 out_drop;
	uint8 out_egress_cvid_act;
	uint8 out_egress_svid_act;
	uint8 out_ctag_format_act;
	uint8 out_stag_format_act;
	uint8 out_user_pri_act;
	uint8 out_egress_port_to_vid_act;
	uint8 out_smac_trans;
	uint8 lock;
}rtk_rg_asic_path1To2_entry_t;

/* flow-based table - tcp/udp l2, for path 3,4  */
typedef struct rtk_rg_asic_path3To4_entry_s
{
	uint8 valid;
	uint8 out_dmac_trans;
	uint8 in_path;
	uint32 in_intf_idx;
	union
	{
		uint8 in_stream_idx_check;
		uint8 out_stream_idx_act;
	};
	uint8 in_multiple_act;
	uint8 out_share_meter_act;
	uint32 out_share_meter_idx;
	uint8 in_ipv4_or_ipv6;
	uint8 in_ctagif;
	uint8 in_stagif;
	uint8 in_pppoeif;
	uint32 out_intf_idx;
	uint8 in_l4proto;
	uint8 in_dscp_check;
	uint16 in_dscp;
	union
	{
		uint32 in_src_ipv4_addr;
		uint32 in_src_ipv6_addr_hash;
	};
	union
	{
		uint32 in_dst_ipv4_addr;
		uint32 in_dst_ipv6_addr_hash;
	};
	uint16 in_l4_src_port;
	uint16 in_l4_dst_port;
	uint32 out_dmac_idx;
	uint8 out_svid_format_act;
	uint16 out_user_priority;
	uint8 out_cvid_format_act;
	uint32 out_extra_tag_index;
	uint8 out_smac_trans;
	uint8 out_uc_lut_lookup;
	rtk_portmask_t out_portmask;
	uint8 out_ext_portmask_idx;
	uint8 out_spri_format_act;
	uint16 out_spri;
	uint16 out_svlan_id;
	uint8 out_cpri_format_act;
	uint16 out_cpri;
	uint16 out_cvlan_id;
	uint8 in_pppoe_sid_check;
	union
	{
		uint8 in_stream_idx;
		uint8 out_stream_idx;
	};
	uint8 out_multiple_act;
	uint8 out_dscp_act;
	uint16 out_dscp;
	uint8 out_drop;
	uint8 out_egress_cvid_act;
	uint8 out_egress_svid_act;
	uint8 out_ctag_format_act;
	uint8 out_stag_format_act;
	uint8 out_user_pri_act;
	uint8 out_egress_port_to_vid_act;
	uint8 lock;
}rtk_rg_asic_path3To4_entry_t;

/* flow-based table - unicast l34 routing/napt, for path 5  */
typedef struct rtk_rg_asic_path5_entry_s
{
	uint8 valid;
	uint8 in_path;
	uint32 in_intf_idx;
	union
	{
		uint8 in_stream_idx_check;
		uint8 out_stream_idx_act;
	};
	uint8 out_share_meter_act;
	uint32 out_share_meter_idx;
	uint8 in_ipv4_or_ipv6;
	uint8 in_ctagif;
	uint8 in_stagif;
	uint8 in_pppoeif;
	uint32 out_intf_idx;
	uint8 in_l4proto;
	uint8 in_dscp_check;
	uint16 in_dscp;
	union
	{
		uint32 in_src_ipv4_addr;
		uint32 in_src_ipv6_addr_hash;
	};
	union
	{
		uint32 in_dst_ipv4_addr;	//routing/napt mode
		uint32 in_dst_ipv6_addr_hash; //routing/napt mode
		uint32 out_dst_ipv4_addr;	//napt-r mode
	};
	uint16 in_l4_src_port;
	uint16 in_l4_dst_port;
	uint32 out_dmac_idx;
	uint8 out_svid_format_act;
	uint16 out_user_priority;
	uint8 out_cvid_format_act;
	uint32 out_extra_tag_index;
	uint16 out_l4_port;
	uint8 out_spri_format_act;
	uint16 out_spri;
	uint16 out_svlan_id;
	uint8 out_cpri_format_act;
	uint16 out_cpri;
	uint16 out_cvlan_id;
	uint8 out_l4_direction;
	union
	{
		uint8 in_stream_idx;
		uint8 out_stream_idx;
	};
	uint8 out_l4_act;
	uint8 out_dscp_act;
	uint16 out_dscp;
	uint8 out_drop;
	uint8 out_egress_cvid_act;
	uint8 out_egress_svid_act;
	uint8 out_ctag_format_act;
	uint8 out_stag_format_act;
	uint8 out_user_pri_act;
	uint8 out_egress_port_to_vid_act;
	uint8 lock;
}rtk_rg_asic_path5_entry_t;

/* flow-based table - dual header, for path 6  */
typedef struct rtk_rg_asic_path6_entry_s
{
	uint8 valid;
	uint8 in_path;
	uint32 in_intf_idx;
	uint8 in_l4_src_port_check;
	uint8 in_l4_dst_port_check;
	uint8 in_l2tp_tunnel_id_check;
	uint8 in_l2tp_session_id_check;
	uint8 in_src_mac_check;
	uint8 in_dst_mac_check;
	uint8 in_src_ip_check;
	uint8 in_dst_ip_check;
	uint8 in_ipv4_or_ipv6;
	uint8 in_ctagif;
	uint8 in_stagif;
	uint8 in_pppoeif;
	uint8 in_gre_call_id_check;
	uint8 in_dscp_check;
	uint16 in_dscp;	
    union
	{
		uint32 in_src_ipv4_addr;
		uint32 in_src_ipv6_addr_hash;
	};
	union
	{
		uint32 in_dst_ipv4_addr;
		uint32 in_dst_ipv6_addr_hash;
	};
	uint16 in_l4_src_port;
	uint16 in_l4_dst_port;
	uint8 in_protocol;
	uint32 in_smac_lut_idx;
	uint32 in_dmac_lut_idx;
	uint16 in_l2tp_tunnel_id;
	union
	{
		uint16 in_gre_call_id;	   //pptp mode
		uint16 in_l2tp_session_id; //l2tp mode
	};
	uint8 out_drop;
	uint8 lock;
}rtk_rg_asic_path6_entry_t;

/* interface table */
typedef struct rtk_rg_asic_netif_entry_s
{
    uint8 valid;
    rtk_mac_t gateway_mac_addr;
	ipaddr_t gateway_ipv4_addr;
	uint16 intf_mtu;
	uint8 intf_mtu_check;
	uint16 out_pppoe_sid;
	uint8 out_pppoe_act;
	uint8 deny_ipv4;
	uint8 deny_ipv6;
	rtk_rg_asic_forwardAction_t ingress_action;
	rtk_rg_asic_forwardAction_t egress_action;
	rtk_portmask_t allow_ingress_portmask;
	rtk_portmask_t allow_ingress_ext_portmask;
}rtk_rg_asic_netif_entry_t;

/* interface table */
typedef struct rtk_rg_asic_netifMib_entry_s
{
   uint32 in_intf_uc_packet_cnt;
   uint64 in_intf_uc_byte_cnt;
   uint32 in_intf_mc_packet_cnt;
   uint64 in_intf_mc_byte_cnt;
   uint32 in_intf_bc_packet_cnt;
   uint64 in_intf_bc_byte_cnt;
   uint32 out_intf_uc_packet_cnt;
   uint64 out_intf_uc_byte_cnt;
   uint32 out_intf_mc_packet_cnt;
   uint64 out_intf_mc_byte_cnt;
   uint32 out_intf_bc_packet_cnt;
   uint64 out_intf_bc_byte_cnt;
}rtk_rg_asic_netifMib_entry_t;


/* mac address indirect access table */
typedef struct rtk_rg_asic_indirectMac_entry_s
{
	uint32 l2_idx;
}rtk_rg_asic_indirectMac_entry_t;

/* ether-type table */
typedef struct rtk_rg_asic_etherType_entry_s
{
    uint16 ethertype;
}rtk_rg_asic_etherType_entry_t;

/* extra tag action */
typedef struct rtk_rg_asic_extraTagAction_s
{
    uint32 act_bit: 3;
	union{
		struct{							// Update Ethertype & MTU
			uint32 ethertype: 16;
			uint32 mtu_diff_value: 9;
			uint32 ethertype_trans: 1;
			uint32 mtu_operation: 1;
			uint32 reserve0: 2;
		}type1;
		struct{							// Inser extra tag to specific location
			uint32 reserve0: 8;
			uint32 length: 8;
			uint32 src_addr_offset: 9;
			uint32 reserve1: 2;
		}type2;
		struct{							// Update Outer IP header length
			uint32 pkt_buff_offset: 8;		// replaced offset
			uint32 length: 8;
			uint32 value: 9;
			uint32 operation: 2;			// 0:+; 1:-; 2:*; 3:/;
		}type3;
		struct{							// Update GRE seg or IPv4 ID
			uint32 pkt_buff_offset: 8;
			uint32 seq_ack_reg_idx: 4;
			uint32 reserve0: 3;
			uint32 data_src_type: 1;		// 0: GRE seq/ack 1:IP Identification
			uint32 reserve1: 11;
		}type4;
		struct{							// Caculate outer L3 header checksum (L3 checksum offload)
			uint32 pkt_buff_offset: 8;
			uint32 reserve0: 19;
		}type5;
		struct{							// Caculate outer L4 header checksum (L4 checksum offload)
			uint32 pkt_buff_offset: 8;
			uint32 reserve0: 19;
		}type6;	
	}act;	
}rtk_rg_asic_extraTagAction_t;

/*
 * Symbol Definition
 */


/*
 * Function Declaration
 */

extern rtk_rg_err_code_t rtk_rg_asic_path1To2_add(uint32 *idx, rtk_rg_asic_path1To2_entry_t *pP1P2Data, uint16 igrSVID, uint16 igrCVID);

extern rtk_rg_err_code_t rtk_rg_asic_path1To2_set(uint32 idx, rtk_rg_asic_path1To2_entry_t *pP1P2Data);

extern rtk_rg_err_code_t rtk_rg_asic_path1To2_get(uint32 idx, rtk_rg_asic_path1To2_entry_t *pP1P2Data);

extern rtk_rg_err_code_t rtk_rg_asic_path1To2_del(uint32 idx);
	
extern rtk_rg_err_code_t rtk_rg_asic_path3To4_add(uint32 *idx, rtk_rg_asic_path3To4_entry_t *pP3P4Data, uint16 igrSVID, uint16 igrCVID);

extern rtk_rg_err_code_t rtk_rg_asic_path3To4_set(uint32 idx, rtk_rg_asic_path3To4_entry_t *pP3P4Data);

extern rtk_rg_err_code_t rtk_rg_asic_path3To4_get(uint32 idx, rtk_rg_asic_path3To4_entry_t *pP3P4Data);

extern rtk_rg_err_code_t rtk_rg_asic_path3To4_del(uint32 idx);

extern rtk_rg_err_code_t rtk_rg_asic_path5_add(uint32 *idx, rtk_rg_asic_path5_entry_t *pP5Data, uint16 igrSVID, uint16 igrCVID);

extern rtk_rg_err_code_t rtk_rg_asic_path5_set(uint32 idx, rtk_rg_asic_path5_entry_t *pP5Data);

extern rtk_rg_err_code_t rtk_rg_asic_path5_get(uint32 idx, rtk_rg_asic_path5_entry_t *pP5Data);

extern rtk_rg_err_code_t rtk_rg_asic_path5_del(uint32 idx);

extern rtk_rg_err_code_t rtk_rg_asic_path6_add(uint32 *idx, rtk_rg_asic_path6_entry_t *pP6Data, uint16 igrSVID, uint16 igrCVID);

extern rtk_rg_err_code_t rtk_rg_asic_path6_set(uint32 idx, rtk_rg_asic_path6_entry_t *pP6Data);

extern rtk_rg_err_code_t rtk_rg_asic_path6_get(uint32 idx, rtk_rg_asic_path6_entry_t *pP6Data);

extern rtk_rg_err_code_t rtk_rg_asic_path6_del(uint32 idx);

extern rtk_rg_err_code_t rtk_rg_asic_netifTable_add(uint32 idx, rtk_rg_asic_netif_entry_t *pNetifEntry);

extern rtk_rg_err_code_t rtk_rg_asic_netifTable_del(uint32 idx);

extern rtk_rg_err_code_t rtk_rg_asic_netifTable_get(uint32 idx, rtk_rg_asic_netif_entry_t *pNetifEntry);

extern rtk_rg_err_code_t rtk_rg_asic_netifMib_get(uint32 idx, rtk_rg_asic_netifMib_entry_t *pNetifMibEntry);

extern rtk_rg_err_code_t rtk_rg_asic_netifMib_reset(uint32 idx);

extern rtk_rg_err_code_t rtk_rg_asic_indirectMacTable_add(uint32 idx, rtk_rg_asic_indirectMac_entry_t *pIndirectMacEntry);

extern rtk_rg_err_code_t rtk_rg_asic_indirectMacTable_del(uint32 idx);

extern rtk_rg_err_code_t rtk_rg_asic_indirectMacTable_get(uint32 idx, rtk_rg_asic_indirectMac_entry_t *pIndirectMacEntry);

extern rtk_rg_err_code_t rtk_rg_asic_etherTypeTable_add(uint32 idx, rtk_rg_asic_etherType_entry_t *pEtherTypeEntry);

extern rtk_rg_err_code_t rtk_rg_asic_etherTypeTable_del(uint32 idx, rtk_rg_asic_etherType_entry_t *pEtherTypeEntry);

extern rtk_rg_err_code_t rtk_rg_asic_etherTypeTable_get(uint32 idx, rtk_rg_asic_etherType_entry_t *pEtherTypeEntry);

extern rtk_rg_err_code_t rtk_rg_asic_extraTagAction_add(uint32 actionList, uint32 actionIdx, rtk_rg_asic_extraTagAction_t *pExtraTagAction);

extern rtk_rg_err_code_t rtk_rg_asic_extraTagAction_del(uint32 actionList, uint32 actionIdx);

extern rtk_rg_err_code_t rtk_rg_asic_extraTagAction_get(uint32 actionList, uint32 actionIdx, rtk_rg_asic_extraTagAction_t *pExtraTagAction);

extern rtk_rg_err_code_t rtk_rg_asic_extraTagContentBuffer_set(uint32 offset, uint32 len, char *pContentBuffer);

extern rtk_rg_err_code_t rtk_rg_asic_extraTagContentBuffer_get(uint32 offset, uint32 len, char *pContentBuffer);

extern rtk_rg_err_code_t rtk_rg_asic_extraTagInsertHdrLen_set(uint32 actionList, uint32 len);

extern rtk_rg_err_code_t rtk_rg_asic_extraTagInsertHdrLen_get(uint32 actionList, uint32 *len);

extern rtk_rg_err_code_t rtk_rg_asic_flowTrfIndicator_get(uint32 idx, rtk_enable_t *pFlowIndicator);

extern rtk_rg_err_code_t rtk_rg_asic_flowTraffic_get(uint32 setNum, uint32 *flowTrafficSet);

extern rtk_rg_err_code_t rtk_rg_asic_netifTrfIndicator_get(uint32 idx, rtk_enable_t *pNetifIndicator);

extern rtk_rg_err_code_t rtk_rg_asic_netifTraffic_get(uint32 idx, uint32 *value);

extern rtk_rg_err_code_t rtk_rg_asic_trapCpuPriority_set(rtk_enable_t state, uint32 priority);

extern rtk_rg_err_code_t rtk_rg_asic_trapCpuPriority_get(rtk_enable_t *pState, uint32 *pPriority);

extern rtk_rg_err_code_t rtk_rg_asic_spaUnmatchAction_set(rtk_rg_asic_forwardAction_t action);

extern rtk_rg_err_code_t rtk_rg_asic_spaUnmatchAction_get(rtk_rg_asic_forwardAction_t *pAction);

extern rtk_rg_err_code_t rtk_rg_asic_globalState_set(rtk_rg_asic_globalStateType_t stateType, rtk_enable_t state);

extern rtk_rg_err_code_t rtk_rg_asic_globalState_get(rtk_rg_asic_globalStateType_t stateType, rtk_enable_t *pState);

extern rtk_rg_err_code_t rtk_rg_asic_table_reset(rtk_rg_asic_resetTableType_t type);

extern rtk_rg_err_code_t rtk_rg_asic_init(void);


#endif /* __RTK_RG_ASIC_APOLLOPROFB_H__ */

