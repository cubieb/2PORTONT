#ifndef RTK_RG_DEBUG_H
#define RTK_RG_DEBUG_H


/*romedriver Boyce 2014-07-18*/
#if defined (CONFIG_APOLLO)
#include <common/rt_type.h>
#include <rtk/port.h>
#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) &&  defined(CONFIG_XDSL_ROMEDRIVER)
//rtl865x

#endif


#define RG_INIT_DEFAULT_BC_rate_limit				-1
#define RG_INIT_DEFAULT_BC_rate_limit_portMask			0x0
#define RG_INIT_DEFAULT_IPv4_MC_rate_limit			-1
#define RG_INIT_DEFAULT_IPv4_MC_rate_limit_portMask		0x0
#define RG_INIT_DEFAULT_IPv6_MC_rate_limit			-1
#define RG_INIT_DEFAULT_IPv6_MC_rate_limit_portMask		0x0
#define RG_INIT_DEFAULT_acl_drop_ip_range_rule_handle_by_sw	0
#define RG_INIT_DEFAULT_acl_permit_ip_range_rule_handle_by_sw	0
#define RG_INIT_DEFAULT_acl_reserved_arrange
#define RG_INIT_DEFAULT_alg_user_defined_port_num
#define RG_INIT_DEFAULT_alg_user_defined_time_out
#define RG_INIT_DEFAULT_arp_max_request_count			0
#define RG_INIT_DEFAULT_arp_request_interval_sec		RTK_RG_DEFAULT_ARP_REQUEST_INTERVAL_SECOND
#define RG_INIT_DEFAULT_arp_timeout				RTK_RG_DEFAULT_ARP_TIMEOUT
#define RG_INIT_DEFAULT_assign_ack_priority_and_disable_svlan	DISABLED
#define RG_INIT_DEFAULT_bridgeWan_drop_by_protocal
#define RG_INIT_DEFAULT_callback
#define RG_INIT_DEFAULT_congestion_ctrl_inbound_ack_to_high_queue	0
#define RG_INIT_DEFAULT_congestion_ctrl_interval_usec			0
#define RG_INIT_DEFAULT_congestion_ctrl_port_mask			//0
#define RG_INIT_DEFAULT_congestion_ctrl_send_byte_per_sec		//12500000
#define RG_INIT_DEFAULT_congestion_ctrl_send_byte_per_sec_for_wan	//12500000
#define RG_INIT_DEFAULT_congestion_ctrl_send_remainder_in_next_gap	//0
#define RG_INIT_DEFAULT_congestion_ctrl_send_times_per_port		//0
#define RG_INIT_DEFAULT_control_hw_TTL_minus	1
#define RG_INIT_DEFAULT_debug_level
#define RG_INIT_DEFAULT_dump_ps_rx_pkt
#define RG_INIT_DEFAULT_dump_ps_tx_pkt
#define RG_INIT_DEFAULT_filter_level
#define RG_INIT_DEFAULT_fwd_statistic
#define RG_INIT_DEFAULT_gponDsBCModuleEnable			DISABLED
#define RG_INIT_DEFAULT_house_keep_sec				RTK_RG_DEFAULT_HOUSE_KEEP_SECOND
#define RG_INIT_DEFAULT_hwnat					RG_HWNAT_ENABLE
#define RG_INIT_DEFAULT_igmpSnooping				ENABLED
#define RG_INIT_DEFAULT_igmp_dvmrpRouterAgingTime		DEFAULT_DVMRP_AGING_TIME
#define RG_INIT_DEFAULT_igmp_fastLeave				0
#define RG_INIT_DEFAULT_igmp_groupList_memDump
#define RG_INIT_DEFAULT_igmp_groupMemberAgingTime		DEFAULT_GROUP_MEMBER_INTERVAL
#define RG_INIT_DEFAULT_igmp_lastMemberAgingTime		10
#define RG_INIT_DEFAULT_igmp_max_simultaneous_group_size	RTK_RG_DEFAULT_IGMP_SYS_MAX_SIMULTANEOUS_GROUP_SIZE_UNLIMIT
#define RG_INIT_DEFAULT_igmp_mld_query_filter_portmask		0xffff
#define RG_INIT_DEFAULT_igmp_mospfRouterAgingTime		DEFAULT_MOSPF_AGING_TIME
#define RG_INIT_DEFAULT_igmp_pimRouterAgingTime			DEFAULT_PIM_AGING_TIME
#define RG_INIT_DEFAULT_igmp_querierPresentInterval		DEFAULT_QUERIER_PRESENT_TIMEOUT
#define RG_INIT_DEFAULT_igmp_report_filter_portmask		0xffff
#define RG_INIT_DEFAULT_igmp_report_ingress_filter_portmask	0xffff
#define RG_INIT_DEFAULT_igmp_sys_timer_sec			RTK_RG_DEFAULT_IGMP_SYS_TIMER_INTERVAL
#define RG_INIT_DEFAULT_igmp_trap_to_PS				RTK_RG_DISABLED
#define RG_INIT_DEFAULT_ipsec_passthru				PASS_OLD
#define RG_INIT_DEFAULT_ipv6MC_tranlate_ingressVid		RTK_RG_ENABLED
#define RG_INIT_DEFAULT_ivlMulticastSupport
#define RG_INIT_DEFAULT_keep_protocol_stack_packets_orig_cvlan	0
#define RG_INIT_DEFAULT_l4_choice_hw_in				0
#define RG_INIT_DEFAULT_l4ways					0
#define RG_INIT_DEFAULT_l4ways_list				0
#define RG_INIT_DEFAULT_mcast_force_report_sec			0
#define RG_INIT_DEFAULT_mcast_protocol				RG_MC_BOTH_IGMP_MLD
#define RG_INIT_DEFAULT_mcast_query_sec				RTK_RG_DEFAULT_MCAST_QUERY_INTERVAL
#define RG_INIT_DEFAULT_mld_trap_to_PS				RTK_RG_DISABLED
#define RG_INIT_DEFAULT_neighbor_timeout			RTK_RG_DEFAULT_NEIGHBOR_TIMEOUT
#define RG_INIT_DEFAULT_pon_port_unmatch_cf_drop		0
#define RG_INIT_DEFAULT_portBindingByProtocal				0
#define RG_INIT_DEFAULT_portBindingByProtocal_filter_downstream_vid	0
#define RG_INIT_DEFAULT_pppoe_bc_passthrought_to_bindingWan		RTK_RG_DISABLED
#define RG_INIT_DEFAULT_pppoe_mc_routing_trap				RTK_RG_DISABLED
#define RG_INIT_DEFAULT_pppoe_proxy_only_for_binding_packet		0
#define RG_INIT_DEFAULT_proc_to_pipe					0
#define RG_INIT_DEFAULT_ps_rx_mirror_to_port0				0
#define RG_INIT_DEFAULT_qosInternalAndRemark
#define RG_INIT_DEFAULT_qos_type
#define RG_INIT_DEFAULT_redirect_first_http_req_by_mac
#define RG_INIT_DEFAULT_redirect_first_http_req_set_url
#define RG_INIT_DEFAULT_remove_l34_tag_for_same_mac		RG_HWNAT_DISABLE
#define RG_INIT_DEFAULT_send_from_cpu
#define RG_INIT_DEFAULT_stag_enable				RTK_RG_DISABLED
#define RG_INIT_DEFAULT_strange_packet_drop			RG_HWNAT_DISABLE
#define RG_INIT_DEFAULT_tcp_disable_stateful_tracking	0
#define RG_INIT_DEFAULT_tcp_do_not_del_when_rst_fin		0
#define RG_INIT_DEFAULT_tcp_hw_learning_at_syn			0
#define RG_INIT_DEFAULT_tcp_in_shortcut_learning_at_syn		0
#define RG_INIT_DEFAULT_tcp_long_timeout			RTK_RG_DEFAULT_TCP_LONG_TIMEOUT
#define RG_INIT_DEFAULT_tcp_short_timeout			RTK_RG_DEFAULT_TCP_SHORT_TIMEOUT
#define RG_INIT_DEFAULT_tcp_short_timeout_housekeep_jiffies	200
#define RG_INIT_DEFAULT_trace_filter
#define RG_INIT_DEFAULT_trap_lan_add_host
#define RG_INIT_DEFAULT_trap_lan_del_host
#define RG_INIT_DEFAULT_trap_lan_enable				DISABLED
#define RG_INIT_DEFAULT_trap_syn_and_disable_svlan		DISABLED
#define RG_INIT_DEFAULT_turn_off_arp_hw_traffic_info		0
#define RG_INIT_DEFAULT_turn_off_ipv4_shortcut			0
#define RG_INIT_DEFAULT_turn_off_ipv6_shortcut			0
#define RG_INIT_DEFAULT_udp_long_timeout			RTK_RG_DEFAULT_UDP_LONG_TIMEOUT
#define RG_INIT_DEFAULT_udp_short_timeout			RTK_RG_DEFAULT_UDP_SHORT_TIMEOUT
#define RG_INIT_DEFAULT_unknownDA_rate_limit			-1
#define RG_INIT_DEFAULT_unknownDA_rate_limit_portMask		0x0
#define RG_INIT_DEFAULT_unknownDA_trap_to_PS			RTK_RG_DISABLED
#define RG_INIT_DEFAULT_urlFilter_mode				RG_FILTER_BLACK
#define RG_INIT_DEFAULT_virtualMAC_with_PON			0
#define RG_INIT_DEFAULT_wifi_tx_redirect_to_port0		0
#define RG_INIT_DEFAULT_wmux_add
#define RG_INIT_DEFAULT_wmux_del
#define RG_INIT_DEFAULT_wmux_flag
#define RG_INIT_DEFAULT_wmux_info
#define RG_INIT_DEFAULT_wmux_init

// PROC_PRINTF: please add "(char *buf, char **start, off_t offset,int count, int *eof, void *data)" in function header of caller.
//              must add "int len=0" at start of function and add "return len" at end of function.

char *proc_printf(struct seq_file *s, char *fmt, ...);
//#define PROC_PRINTF( comment ,arg...)	do { if((s==NULL)||(rg_db.systemGlobal.proc_to_pipe==0)) {printk(comment,##arg);} else {seq_printf(s,comment,##arg);} }while(0)
#define PROC_PRINTF( comment ,arg...)	do { proc_printf(s,comment,##arg); }while(0)


void common_dump(rtk_rg_debug_level_t level, const char *funcs,int line, char *comment,...);

int32 dump_tcpudp(struct seq_file *s, void *v);
int32 dump_napt(struct seq_file *s, void *v);
int32 dump_netif(struct seq_file *s, void *v);
int32 dump_pppoe(struct seq_file *s, void *v);
int32 dump_nexthop(struct seq_file *s, void *v);
int32 dump_arp(struct seq_file *s, void *v);
int32 dump_ip(struct seq_file *s, void *v);
int32 dump_l3(struct seq_file *s, void *v);

int32 dump_l4hs(struct seq_file *s, void *v);
int32 dump_hs(struct seq_file *s, void *v);
int32 dump_lut(int idx,struct seq_file *s);
int32 dump_lut_table(struct seq_file *s, void *v);
int32 dump_ipmc_group(struct seq_file *s, void *v);
int32 dump_ipmc_routing(struct seq_file *s, void *v);
int32 dump_vlan_table(struct seq_file *s, void *v);
int32 dump_vlan_mbr(struct seq_file *s, void *v);
int32 dump_svlan_mbr(struct seq_file *s, void *v);
int32 dump_svlan_c2s(struct seq_file *s, void *v);
int32 dump_svlan_mc2s(struct seq_file *s, void *v);
int32 dump_svlan_sp2c(struct seq_file *s, void *v);
int32 dump_acl(struct seq_file *s, void *v);
int32 dump_acl_template(struct seq_file *s, void *v);
int32 dump_acl_vidRangeTable(struct seq_file *s, void *v);
int32 dump_acl_ipRangeTable(struct seq_file *s, void *v);
int32 dump_acl_portRangeTable(struct seq_file *s, void *v);
int32 dump_acl_pktlenRangeTable(struct seq_file *s, void *v);
int32 dump_cf(struct seq_file *s, void *v);
int32 dump_reg(struct seq_file *s, void *v);
int32 dump_l34_bind_table(struct seq_file *s, void *v);
int32 dump_l34_wantype_table(struct seq_file *s, void *v);
int32 dump_ipv6_route_table(struct seq_file *s, void *v);
int32 dump_ipv6_neighbor_table(struct seq_file *s, void *v);
int32 dump_piso(struct seq_file *s, void *v);

void dump_packet(unsigned char *pkt,unsigned int size,char *memo);
void dump_compare_packet(uint8 *output,uint8 *expect,int out_size,int expect_size);


void mibdump_lut(int idx,struct seq_file *s);
int32 mibdump_lut_table(struct seq_file *s, void *v);
int32 mibdump_ipv6_neighbor_table(struct seq_file *s, void *v);
int32 mibdump_frag(struct seq_file *s, void *v);
int _rtk_rg_trace_filter_compare(struct sk_buff *skb,rtk_rg_pktHdr_t *pPktHdr);
void _rtk_rg_error_id_mapping(int id, char *mappingName);
rtk_rg_err_code_t return_err_mapping(int err_id, char *err_name,int line);

void _rtk_rg_dump_stack(void);


#endif
