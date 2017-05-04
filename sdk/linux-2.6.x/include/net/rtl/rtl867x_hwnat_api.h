#ifndef RTL8676_API_H
#define RTL8676_API_H


#ifdef CONFIG_RTL_HARDWARE_MULTICAST
/********************************************************         
	RTL8676 API  - Multicast hardware acceleration
***********************************************************/
#include <net/rtl/rtl865x_multicast.h>

int rtl8676_set_Mutlicast_acc(unsigned int group_ip, unsigned int source_ip, unsigned short source_vid, unsigned short source_port,unsigned int gourp_member);
int rtl8676_reset_Mutlicast_acc(unsigned int source_ip,unsigned int group_ip,unsigned int gourp_member);
int rtl8676_cancel_Mutlicast_acc(unsigned int source_ip,unsigned int group_ip);
#endif

#ifdef CONFIG_RTL_MULTI_ETH_WAN
/********************************************************         
	RTL8676 API  - Multiwan
***********************************************************/
#include <net/rtl/rtl_nic.h>

int rtl8676_register_Multiwan_dev(char *ifname, int proto, int vid, int napt);
int rtl8676_update_portmapping_Multiwan_dev(char *ifname,unsigned int member);
int rtl8676_unregister_Multiwan_dev(char *ifname);
int rtl8676_set_Multiwan_NetifMacAddr(char *ifname, char *addr);
#endif

#if defined( CONFIG_RTL_PPPOE_HWACC) && defined(CONFIG_RTL_LAYERED_DRIVER) && defined (CONFIG_RTL_LAYERED_DRIVER_L3)
/********************************************************         
	RTL8676 API  - ppp
***********************************************************/

#include <net/sock.h>
#include <linux/if_pppox.h>
#include <net/rtl/rtl865x_ppp.h>
void rtl8676_ppp_connect_channel(char* name, struct sock* sk, int SE_TYPE);
void rtl8676_ppp_disconnect_channel(char* ppp_dev_name);
#endif

#if defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL_LAYERED_DRIVER_L2)
/********************************************************         
	RTL8676 API  - fdb
***********************************************************/
#include <net/rtl/rtl865x_fdb_api.h>

void rtl8676_fdb_delete(unsigned short l2Type,  const unsigned char *addr);
void rtl8676_fdb_new(unsigned char is_static, const unsigned char *addr);
void rtl8676_update_L2_check(void *orig_br_port, void *updated_br_port, const char *name, const unsigned char *addr);
#endif



#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER_L3)
/********************************************************         
	RTL8676 API  - L3 table
***********************************************************/
#include <net/rtl/rtl865x_route_api.h>
int32 rtl8676_add_route(int ifindex, char *ifname, unsigned int IpDst, unsigned int IpMask, unsigned IpGw);
int32 rtl8676_del_route(unsigned int IpDst, unsigned int IpMask);
#endif


#ifdef CONFIG_RTL_HARDWARE_NAT
#include <net/netfilter/nf_nat_core.h>
#ifdef CONFIG_RTL8676_Static_ACL
/********************************************************         
	RTL8676 API  - TCP/IP
***********************************************************/
int rtl8676_add_napt(struct nf_conn *ct);
int rtl8676_del_napt(struct nf_conn *ct);
int rtl8676_query_napt(struct nf_conn *ct);
#else /* CONFIG_RTL8676_Dynamic_ACL */
/********************************************************         
	RTL8676 API  - Unicast hardware acceleration
***********************************************************/
int rtl8676_add_L2Unicast_hwacc(unsigned char* src_mac, unsigned char* dst_mac, char *in_ifname, char *out_ifname);
int rtl8676_del_L2Unicast_hwacc(unsigned char* src_mac, unsigned char* dst_mac);
int rtl8676_del_L2Unicast_hwacc_agingout_mac(unsigned char* del_mac);
int rtl8676_clean_L2Unicast_hwacc(void);
int rtl8676_query_L2Unicast_hwacc(unsigned char* query_mac);
int rtl8676_add_L34Unicast_hwacc_upstream
	(uint32 scr_ip,uint16 scr_port,uint32 dst_ip,uint16 dst_port,uint32 src_naptIp ,uint16 src_naptPort,uint8 protocol
	,uint32 nexthop_ip,char* in_netif,char* out_netif,uint8 priority,uint8 pri_flag);
int rtl8676_add_L34Unicast_hwacc_downstream
	(uint32 scr_ip,uint16 scr_port,uint32 dst_ip,uint16 dst_port,uint32 dst_naptIp, uint16 dst_naptPort,uint8 protocol
	,char* in_netif,char* out_netif);
#ifdef CONFIG_RTL_HW_QOS_SUPPORT
int rtl8676_add_L34Unicast_hwacc_ct(struct nf_conn *ct,uint32 upstream_nexthop_ip,char* upstream_out_netif
												,int32 ipqos_enable,int32 ipqos_swQid,int32 ipqos_8021p_remark,int32 ipqos_dscp_remark);
#else
int rtl8676_add_L34Unicast_hwacc_ct(struct nf_conn *ct,uint32 upstream_nexthop_ip,char* upstream_out_netif);
#endif
int rtl8676_del_L34Unicast_hwacc_upstream
	(uint32 scr_ip,uint16 scr_port,uint32 dst_ip,uint16 dst_port,uint32 src_naptIp ,uint16 src_naptPort,uint8 protocol);
int rtl8676_del_L34Unicast_hwacc_downstream
	(uint32 scr_ip,uint16 scr_port,uint32 dst_ip,uint16 dst_port,uint32 dst_naptIp, uint16 dst_naptPort,uint8 protocol);
int rtl8676_del_L34Unicast_hwacc_ct(struct nf_conn *ct);
int rtl8676_query_L34Unicast_hwacc_upstream
	(uint32 scr_ip,uint16 scr_port,uint32 dst_ip,uint16 dst_port,uint32 src_naptIp ,uint16 src_naptPort,uint8 protocol);
int rtl8676_query_L34Unicast_hwacc_downstream
	(uint32 scr_ip,uint16 scr_port,uint32 dst_ip,uint16 dst_port,uint32 dst_naptIp, uint16 dst_naptPort,uint8 protocol);
int rtl8676_query_L34Unicast_hwacc_ct(struct nf_conn *ct);
int rtl8676_clean_L34Unicast_hwacc(void);
#endif
#endif



#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER)  && defined(CONFIG_RTL_LAYERED_DRIVER_L3)
/********************************************************         
	RTL8676 API  - arp
***********************************************************/
#include <net/neighbour.h>
#include <net/rtl/rtl865x_arp_api.h>
#include <net/rtl/rtl865x_nat.h>

enum arp_action_type {
	ARP_DEL = 0,
	ARP_CHECK,
};
int rtl8676_setArp(struct neighbour *n, int cmd);
#endif


#ifdef CONFIG_RTL_HW_QOS_SUPPORT
/********************************************************         
	RTL8676 API  - IPQoS
***********************************************************/
#include <net/rtl/rtl867x_hwnat_qosrule.h>


int rtl8676_IPQos_Enable(int sp_queue_num, int wrr_queue_num ,int ceil[],int rate[], int default_qid);
int rtl8676_IPQos_Disable(void);
int rtl8676_IPQos_Remarking(int enable_8021p,int enable_dscp);
int rtl8676_IPQos_RateLimit_Enable(int total_bandwidth);
int rtl8676_IPQos_RateLimit_Disable(void);
int rtl8676_IPQos_AddRule(rtl867x_hwnat_qos_rule_t* input_rule,uint32 q_index,uint32 remark_1p,uint32 remark_dscp);
int rtl8676_IPQos_AddRule_Classification(rtl867x_hwnat_qos_rule_t* input_rule,uint32 q_index);
int rtl8676_IPQos_AddRule_Remarking(rtl867x_hwnat_qos_rule_t* input_rule,uint32 remark_1p,uint32 remark_dscp);
void	rtl8676_IPQos_CleanRule(void);
#endif

#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER_L3)
/********************************************************         
	RTL8676 API  - ext. IP
***********************************************************/
#include <net/rtl/rtl865x_ip_api.h>
int rtl8676_add_napt_extIp(char *name, unsigned int ip);
int rtl8676_del_napt_extIp(char *name, unsigned int ip);
int rtl8676_napt_extip_handle(char *name, int dev_event, unsigned int addr);
#endif

#endif
