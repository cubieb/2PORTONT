#ifndef RTK_RG_STRUCT_H
#define RTK_RG_STRUCT_H

#ifdef __KERNEL__
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/semaphore.h>		//used to lock WAN interface L3 APIs
#include <linux/list.h>		//list_head and routines
#include <linux/fs.h>

#ifdef CONFIG_DEFAULTS_KERNEL_3_18
#else
#include <linux/config.h>
#endif

#ifdef CONFIG_RTL8686NIC
#if defined(CONFIG_RTL9602C_SERIES)
	#include <re8686_rtl9602c.h>
#else
	#include <re8686.h>
#endif
#endif
#else //model

#include <re8686_sim.h>
#endif

#ifdef CONFIG_APOLLO

#include <rtk/acl.h>
#include <rtk/classify.h>
#include <rtk/l2.h>
#include <rtk/l34.h>
#ifdef CONFIG_GPON_FEATURE
#include <module/gpon/gpon.h>
#endif
#ifdef CONFIG_EPON_FEATURE
#include <rtk/epon.h>
#endif
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
#include <rtk/ponmac.h>
#endif

#endif

#include <rtk_rg_define.h>

#if defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) && defined(CONFIG_XDSL_ROMEDRIVER)
#include <rtk_rg_xdsl_struct.h>
#endif

#if defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_RTL9601B_SERIES) //CF action register is different between Apollo adn ApolloFE 
#include <rtk_rg_acl_apollo.h>
#elif defined(CONFIG_RTL9602C_SERIES)
#include <rtk_rg_acl_apolloFE.h>
#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#include <rtk_rg_acl_xdsl.h>
#endif


#ifdef CONFIG_SMP

typedef enum rtk_rg_tasklet_queue_type_e
{
	RG_TASKLET_TYPE_FROM_NIC=0,
	RG_TASKLET_TYPE_FROM_WIFI,
	RG_TASKLET_TYPE_FROM_TIMER
}rtk_rg_tasklet_queue_type_t;

struct rg_private 
{
	atomic_t start_index,end_index;
	struct re_private *nic_data;	
	struct sk_buff *skb[MAX_RG_TASKLET_QUEUE_SIZE];
	struct rx_info rxInfo[MAX_RG_TASKLET_QUEUE_SIZE];
	rtk_rg_tasklet_queue_type_t tasklet_type[MAX_RG_TASKLET_QUEUE_SIZE];	//0:from NIC, 1:from WIFI, 2:from timer
};
#endif

/* ERROR NO =============================================================== */
typedef enum rtk_rg_err_code_e
{
	RT_ERR_RG_OK=0,

	RT_ERR_RG_FAILED = (RT_ERR_COMMON_END+2), //0x10001
	RT_ERR_RG_NOT_SUPPORT_TESTCHIP,		//the API enable some feature not support by test chip			//unused
	RT_ERR_RG_BUF_OVERFLOW,	
	RT_ERR_RG_NULL_POINTER,				//input pointer is null
	RT_ERR_RG_INITPM_UNINIT,			//initparm's pointers is uninit					//0x10005

	RT_ERR_RG_PON_INVALID,				//PON port can not be set as WAN port in RLE0371
	RT_ERR_RG_INVALID_PARAM,			//parameters is invalid
	RT_ERR_RG_ENTRY_FULL,				//the entry table is full
	RT_ERR_RG_NOT_INIT,					//the module is not init
	RT_ERR_RG_VLAN_BASED_OVERLAP_SUBNET,//diff VLAN-based interfaces can not have overlap subnet
	RT_ERR_RG_DEF_ROUTE_EXIST,			//the internet connection is exist				//0x1000b
	
	RT_ERR_RG_STATIC_NOT_FOUND,			//DHCP server index did not find				
	RT_ERR_RG_ARP_NOT_FOUND,			//ARP did not find
	RT_ERR_RG_ARP_FULL,					//ARP table overflow
	RT_ERR_RG_VLAN_BIND_UNINIT,			//vlan-binding is uninit
	RT_ERR_RG_PPPOE_UNINIT,				//pppoe_before is not called before				//0x10010
	
	RT_ERR_RG_CHIP_NOT_SUPPORT,			//the function is not supported in this chip version
	RT_ERR_RG_ENTRY_NOT_EXIST,		
	RT_ERR_RG_INDEX_OUT_OF_RANGE,											
	RT_ERR_RG_NO_MORE_ENTRY_FOUND,		
	RT_ERR_RG_MODIFY_LAN_AT_WAN_EXIST,	//deprecated, LAN intf can not add or delete when WAN exist		//0x10015
	
	RT_ERR_RG_LAN_NOT_EXIST,			//LAN intf did not created before binding
	RT_ERR_RG_GW_MAC_NOT_SET,			//for lite romeDriver, set WAN with default gw must has MAC address input
	RT_ERR_RG_VLAN_SET_FAIL,			//vlan set failed
	RT_ERR_RG_VLAN_GET_FAIL,			//vlan get failed
	RT_ERR_RG_INTF_SET_FAIL,			//interface set failed
	RT_ERR_RG_INTF_GET_FAIL,			//interface get failed							//0x1001b
	
	RT_ERR_RG_ROUTE_SET_FAIL,				//routing table set failed
	RT_ERR_RG_ROUTE_GET_FAIL,				//routing table get failed
	RT_ERR_RG_EXTIP_SET_FAIL,			//internal external IP table set failed
	RT_ERR_RG_EXTIP_GET_FAIL,			//internal external IP table get failed
	RT_ERR_RG_NXP_SET_FAIL,				//nexthop table set failed						//0x10020
	
	RT_ERR_RG_NXP_GET_FAIL,				//nexthop table get failed
	RT_ERR_RG_PPPOE_SET_FAIL,			//pppoe table set failed
	RT_ERR_RG_PPPOE_GET_FAIL,			//pppoe table get failed
	RT_ERR_RG_PORT_BIND_SET_FAIL,		//Port-binding set failed
	RT_ERR_RG_PORT_BIND_GET_FAIL,		//Port-binding get failed						//0x10025
	
	RT_ERR_RG_EXTPORT_BIND_SET_FAIL,	//extension Port-binding set failed
	RT_ERR_RG_EXTPORT_BIND_GET_FAIL,	//extension Port-binding get failed
	RT_ERR_RG_WANTYPE_SET_FAIL,			//WAN type table set failed
	RT_ERR_RG_WANTYPE_GET_FAIL,			//WAN type table get failed
	RT_ERR_RG_VLAN_BIND_SET_FAIL,		//VLAN-Port binding set failed					//0x1002a
	
	RT_ERR_RG_VLAN_BIND_GET_FAIL,		//VLAN-Port binding get failed
	RT_ERR_RG_ACL_CF_FIELD_CONFLICT,	//aclFilter assigned some conflict field(ex:ipv4 ipv6 at the same time, DMAC != ingress_INTF.gmac, CVID!= ingress_INTF.vid, can be reference to _rtk_rg_conflictField_and_flowDirection_check())
	RT_ERR_RG_ACL_CF_FLOW_DIRECTION_ERROR, //return this error while in lan_to_lan /wan_to_wan and assignde CF pattern (CF ASIC just support in upstream/dowmstream)
	RT_ERR_RG_ACL_ENTRY_FULL,			//ACL ASIC entry is full
	RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED,	//set/get ACL ASIC entry failed					//0x1002f
	
	RT_ERR_RG_ACL_IPTABLE_FULL,			//ACL IP range table ASIC full
	RT_ERR_RG_ACL_IPTABLE_ACCESS_FAILED,//set/get ACL IP range table ASIC failed 
	RT_ERR_RG_ACL_PORTTABLE_FULL,		//ACL port range table ASIC full
	RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED,//set/get ACL port range table ASIC failed
	RT_ERR_RG_CF_ENTRY_FULL,			//CF ASIC entry is full						//0x10034
	
	RT_ERR_RG_CF_ENTRY_ACCESS_FAILED,	//set/get CF ASIC entry failed
	RT_ERR_RG_CF_IPTABLE_FULL,			//CF IP range table ASIC full
	RT_ERR_RG_CF_IPTABLE_ACCESS_FAILED,	//set/get CF IP range table ASIC failed 
	RT_ERR_RG_CF_PORTTABLE_FULL,		//CF port range table ASIC full
	RT_ERR_RG_CF_PORTTABLE_ACCESS_FAILED,//set/get ACL port range table ASIC failed			//0x10039
	
	RT_ERR_RG_CF_DSCPTABLE_FULL,		//CF dscp remarking table ASIC full
	RT_ERR_RG_CF_DSCPTABLE_ACCESS_FAILED,//set/get ACL dscp remarking table ASIC failed
	RT_ERR_RG_ACL_SW_ENTRY_FULL,		//software aclFilterEntry full
	RT_ERR_RG_ACL_SW_ENTRY_ACCESS_FAILED,//set/get ACL aclFilterEntry failed
	RT_ERR_RG_ACL_SW_ENTRY_NOT_FOUND,	//not found valid aclFilterEntry					//0x1003e
	
	RT_ERR_RG_ACL_SW_ENTRY_USED,		//set an aclFilterEntry which is in used(not used aclFilterEntry should clean to zero)
	RT_ERR_RG_L2_ENTRY_ACCESS_FAILED,		//set/get L2 ASIC entry failed
	RT_ERR_RG_L2_MACFILTER_ENTRY_ACCESS_FAILED,//access software macFulterEntry failed 
	RT_ERR_RG_L2_MACFILTER_ENTRY_FULL,		//software macFulterEntry is full (size defined by MAX_MAC_FILTER_ENTRY_SIZE)
	RT_ERR_RG_L2_MACFILTER_ENTRY_NOT_FOUND,	//not found valid macFilterEntry				//0x10043

	RT_ERR_RG_URLFILTER_ENTRY_ACCESS_FAILED, //access software urlFilterEntry failed 
	RT_ERR_RG_URLFILTER_ENTRY_FULL,		//software urlFulterEntry is full (size defined by MAX_URL_FILTER_ENTRY_SIZE)
	RT_ERR_RG_URLFILTER_ENTRY_NOT_FOUND,//not found valid software urlFilterEntry	
	RT_ERR_RG_SVRPORT_SW_ENTRY_NOT_FOUND,
	RT_ERR_RG_UPNP_SW_ENTRY_NOT_FOUND,											//0x10048
	
	RT_ERR_RG_NAPT_SW_ENTRY_NOT_FOUND,	//0x10049
	RT_ERR_RG_EXTIP_TYPE_MISMATCH,
	RT_ERR_RG_NAPT_SET_FAIL,				
	RT_ERR_RG_NAPT_GET_FAIL,
	RT_ERR_RG_NAPT_OVERFLOW,
	
	RT_ERR_RG_NAPTR_SET_FAIL,		//0x1004e
	RT_ERR_RG_NAPTR_GET_FAIL,
	RT_ERR_RG_NAPTR_OVERFLOW,
	RT_ERR_RG_NAPT_FLOW_DUPLICATE,
	RT_ERR_RG_NAPT_FLOW_OVERFLOW,
	
	RT_ERR_RG_ARP_MAPPING_OVERFLOW,	//0x10053
	RT_ERR_RG_ARP_ENTRY_STATIC,			//add ARP failed because ARP entry is exist and STATIC
	RT_ERR_RG_ARP_OVERFLOW,
	RT_ERR_RG_L2_ENTRY_NOT_FOUND,
	RT_ERR_RG_PPB_SET_FAILED,			//set port-and-protocol based vlan failed
	
	RT_ERR_RG_UNTAG_BRIDGEWAN_TWICE,	//set untag bridge WAN at same port twice
	RT_ERR_RG_NEIGHBOR_NOT_FOUND,				//Neighbor did not find
	RT_ERR_RG_NEIGHBOR_FULL,					//Neighbor table overflow				//0x1005a
	RT_ERR_RG_SUBNET_INTERFACE_ASYMMETRIC,		//new routing entry has same IP-range with other interface, but has different VLANID or MAC address or type
	RT_ERR_RG_VLAN_USED_BY_INTERFACE,			//the vlan entered is overlaped with interface setting

	RT_ERR_RG_VLAN_USED_BY_VLANBINDING,			//the vlan entered is overlaped with vlanBinding setting
	RT_ERR_RG_VLAN_USED_BY_CVLAN,				//the vlan entered is overlaped with customer vlan setting
	RT_ERR_RG_VLAN_NOT_CREATED_BY_CVLAN,				//the vlan entered is not created as customer vlan setting
	RT_ERR_RG_UNBIND_BDWAN_SHOULD_EQUAL_LAN_VLAN,		//un-bind bridge wan should equals to LAN's VLAN ID		//0x10060
	RT_ERR_RG_BIND_WITH_UNBIND_WAN,				//binding can not set to un-bind WAN

	RT_ERR_RG_CVLAN_CREATED,					//the customer vlan id had been created before
	RT_ERR_RG_CVLAN_RESERVED,					//the customer vlan id had been reserved by system
	RT_ERR_RG_ALG_SRV_IN_LAN_EXIST,				//the ALG Service In Lan service had been assigned
	RT_ERR_RG_ALG_SRV_IN_LAN_NO_IP,				//the ALG Service In Lan service did not assign server ip				//0x10065
	RT_ERR_RG_CREATE_GATEWAY_LUT_FAIL,			//get error when add gateway Mac entry in Interface_add

	RT_ERR_RG_DELETE_GATEWAY_LUT_FAIL,			//get error when del gateway Mac entry in Interface_del
	RT_ERR_RG_STORMCONTROL_TYPE_FULL,			//at most support 4 types
	RT_ERR_RG_STORMCONTROL_ENTRY_FULL,			//at most 16 entries(limit by sharemeter)
	RT_ERR_RG_STORMCONTROL_ENTRY_HAS_BEEN_SET,  //0x1006a
	RT_ERR_RG_STORMCONTROL_ENTRY_NOT_FOUND,

	RT_ERR_RG_SHAREMETER_SET_FAILED,
	RT_ERR_RG_SHAREMETER_GET_FAILED,
	RT_ERR_RG_SHAREMETER_INVALID_METER_INDEX,
	RT_ERR_RG_SHAREMETER_INVALID_RATE,	
	RT_ERR_RG_SHAREMETER_INVALID_INPUT,			//0x10070

	RT_ERR_RG_ADD_ARP_TO_SW_TABLE,				//the routing will direct go to fwdEngine for sw arp table
	RT_ERR_RG_ADD_ARP_MAC_FAILED,				//return FAIL when call _rtk_rg_arpAndMacEntryAdd
	RT_ERR_RG_PPPOEPASSTHROUGHT_NOT_SUPPORTED,
	RT_ERR_RG_IPMC_IP_LIST_OUT_OF_RANGE,		//IPMC include or exclude IP list out of range.
	RT_ERR_RG_IPMC_EXCLUDE_MODE_NOT_SUPPORT_EXTPORT,

	RT_ERR_RG_ALG_SRV_IN_LAN_ENABLED,				//the ALG Service In Lan service had been enabled, turn off it before deleting or changing IP
	RT_ERR_RG_INTF_OVERLAP_AND_SAME_SUBNET,		//two interfaces can not have the same ip subnet at all
	RT_ERR_RG_GPON_TWO_UNTAG_BRIDGE_WAN,		//PON port can not support two untag bridge WAN at same time
	RT_ERR_RG_GPON_MODULE_NOT_INIT,
	RT_ERR_RG_GPON_SET_INFO_FAILED,				//0x1007a

	RT_ERR_RG_GPON_GET_INFO_FAILED,
	RT_ERR_RG_GPON_ALREADY_ACTIVATED,
	RT_ERR_RG_GPON_ACTIVATED_FAILED,
	RT_ERR_RG_GPON_DEACTIVATED_FAILED,
	RT_ERR_RG_GPON_GET_STATUS_FAILED,			

	RT_ERR_RG_EPON_MODULE_NOT_INIT,				//0x10080
	RT_ERR_RG_EPON_SET_INFO_FAILED,
	RT_ERR_RG_EPON_GET_INFO_FAILED,
	RT_ERR_RG_PON_GET_INFO_FAILED,
	RT_ERR_RG_ACL_NOT_SUPPORT,

	RT_ERR_RG_CF_NOT_SUPPORT,					//0x10085
	RT_ERR_RG_VLAN_TWO_UNTAG_CPU_LAN,		//since vconfig need tag to separate each interface, we can only have one untag LAN at same time
	RT_ERR_RG_VLAN_PRI_CONFLICT_WIFI,		//when set up dualband wifi, the VLAN priority can't be set as CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI
	RT_ERR_RG_VLAN_USED_BY_SYSTEM,
	RT_ERR_RG_ACCESSWAN_DISABLE,			//if the activeFunction is not category, set_mac_act should return this value

	RT_ERR_RG_ACCESSWAN_NOT_LAN,			//0x1008a		//if the MAC address is not LAN host, we can't change it's category
	RT_ERR_RG_VLAN_OVER_RANGE,				//the VLAN is beyond the resonable range
	RT_ERR_RG_CPU_TAG_DIFF_BRIDGE_WAN,		//when add bridge WAN, VLAN's CPU port should follow bridge WAN's tag or untag
	RT_ERR_RG_ENTRY_EXIST,					//for any table if the adding entry has existed in tables
	RT_ERR_RG_ENTRY_NOT_FOUND,				//for any table if the search/delete entry can not found in tables

	RT_ERR_RG_IPV6_LAN_MORE_SUBNET_FAIL,	//IPv6 can't be added as more subnet at same interface.	
	RT_ERR_RG_WLAN_BINDING_CONFLICT,		//0x10090	//the WLAN device binding is enabled by wlan_dev_binding_mask, port_binding_mask should not enable ext0 port
	RT_ERR_RG_WLAN_BINDING_OVERLAP,			//two WAN are settings the same WLAN device
	RT_ERR_RG_WLAN_BINDING_ABSENCE,			//the WLAN device non-existence
	RT_ERR_RG_ACCESSWAN_WLAN_CONFLICT,		//if ext0 had been set limit, wlan0_dev_mask should not be set

	RT_ERR_RG_ACCESSWAN_WLAN_NOT_ZERO,		//if wan access type is category, wlan0_dev_mask should keep zero
	RT_ERR_RG_NAPTFILTERANDQOS_SW_ENTRY_NOT_FOUND,//0x10095
	RT_ERR_RG_NAPTFILTERANDQOS_SW_ENTRY_FULL,
	RT_ERR_RG_DHCP_LEASED_INVALID_IP,		//when set DHCP info with status leased, IP address can't be zero
	RT_ERR_RG_STP_BLOCKING_ENABLED,			//when STP blocking mode is enabled, port-isolation should not be used

	RT_ERR_RG_B4_IP_NOT_SET,				//set DSLITE WAN must has B4 address input
	RT_ERR_RG_AFTR_IP_NOT_SET,				//set DSLITE WAN must has AFTR address input		//0x1009a
	RT_ERR_RG_AFTR_MAC_NOT_SET,				//set DSLITE WAN without autolearn must has MAC address input
	RT_ERR_RG_PPTP_MAC_NOT_SET,				//set PPTP WAN without autolearn must has MAC address input
	RT_ERR_RG_L2TP_MAC_NOT_SET,				//set L2TP WAN without autolearn must has MAC address input

	RT_ERR_RG_POLICYROUTE_NO_NH,			//set policy router should set nexthop or no nexthop idx could be approached
	RT_ERR_RG_WWAN_SAME_VXD,				//for one vxd can only setup one wwan
	RT_ERR_RG_WWAN_NOT_EXIST,				//WWAN device is not exist or initialized		//0x100a0
	RT_ERR_RG_GPON_NOT_SUPPORT,				//returned by APIs that should't be called under GPON mode
	RT_ERR_RG_ACL_EGRESS_WAN_MIX,			//mixed egress WAN pattern rules with non-egress-WAN pattern rules at same weight
	RT_ERR_RG_DSLITE_UNMATCH,				//intfIdx is not match the adding WAN
	RT_ERR_RG_DSLITE_UNINIT,				//the dslite table is not init before set WAN
	RT_ERR_RG_DRIVER_NOT_SUPPORT,			//RG API didn't support for specific chip model	//0x100a5
}rtk_rg_err_code_t; 
/* End of ERROR NO ======================================================== */


/* DEBUG ================================================================= */

typedef enum rtk_rg_debug_level_e
{
	RTK_RG_DEBUG_LEVEL_DEBUG=0x1,
	RTK_RG_DEBUG_LEVEL_FIXME=0x2,
	RTK_RG_DEBUG_LEVEL_CALLBACK=0x4,
	RTK_RG_DEBUG_LEVEL_TRACE=0x8,
	RTK_RG_DEBUG_LEVEL_ACL=0x10,
	RTK_RG_DEBUG_LEVEL_WARN=0x20,
	RTK_RG_DEBUG_LEVEL_TRACE_DUMP=0x40,
	RTK_RG_DEBUG_LEVEL_WMUX=0x80,
	RTK_RG_DEBUG_LEVEL_MACLEARN=0x100,
	RTK_RG_DEBUG_LEVEL_TABLE=0x200,
	RTK_RG_DEBUG_LEVEL_ALG=0x400,
	RTK_RG_DEBUG_LEVEL_IGMP=0x800,
	RTK_RG_DEBUG_LEVEL_ACL_RRESERVED=0x1000,
	RTK_RG_DEBUG_LEVEL_RG_API=0x2000,
	RTK_RG_DEBUG_LEVEL_ALL=0xffffffff,
} rtk_rg_debug_level_t;

#define TRACFILTER_MAX 4


typedef enum rtk_rg_debug_trace_filter_bitmask_e
{
	RTK_RG_DEBUG_TRACE_FILTER_SPA=0x1, //source port 0~5 phyiscal Port, 6:CPU, 7:EXT0, 8:EXT1
	RTK_RG_DEBUG_TRACE_FILTER_DA=0x2,	//DNAC
	RTK_RG_DEBUG_TRACE_FILTER_SA=0x4,	//SMAC
	RTK_RG_DEBUG_TRACE_FILTER_ETH=0x8, //ethertype
	RTK_RG_DEBUG_TRACE_FILTER_SIP=0x10, //src IP
	RTK_RG_DEBUG_TRACE_FILTER_DIP=0x20, //dest IP	
	RTK_RG_DEBUG_TRACE_FILTER_IP=0x40, //src IP or dest IP
	RTK_RG_DEBUG_TRACE_FILTER_L4PROTO=0x80, // Layer4 protocol
	RTK_RG_DEBUG_TRACE_FILTER_SPORT=0x100, // L4 Src Port
	RTK_RG_DEBUG_TRACE_FILTER_DPORT=0x200, // L4 Dst Port
	RTK_RG_DEBUG_TRACE_FILTER_REASON=0x400, // Trap reason
} rtk_rg_debug_trace_filter_bitmask_t;


typedef struct rtk_rgDebugTraceFilter_s
{
	uint16		spa;	
	rtk_mac_t	dmac;
	rtk_mac_t	dmac_mask;
	rtk_mac_t	smac;
	rtk_mac_t	smac_mask;
	uint16		ethertype;
	uint32		sip;
	uint32		dip;
	uint32		ip;
	uint16		l4proto;
	uint16		sport;
	uint16		dport;
	uint8		reason;
}rtk_rgDebugTraceFilter_t;

typedef enum rtk_rg_hwnatState_e
{
	RG_HWNAT_DISABLE=0,
	RG_HWNAT_ENABLE,
	RG_HWNAT_PROTOCOL_STACK,
	RG_HWNAT_UC_FORCE_HW_FWD,
}rtk_rg_hwnatState_t;

/* End of DEBUG ========================================================== */


/* RTK RG API ============================================================= */

typedef enum rtk_rg_mac_port_idx_e
{
#if defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_RTL9601B_SERIES)
	RTK_RG_MAC_PORT0=0,
	RTK_RG_MAC_PORT1,
	RTK_RG_MAC_PORT2,
	RTK_RG_MAC_PORT3,
	RTK_RG_MAC_PORT_PON,
	RTK_RG_MAC_PORT_RGMII,
	RTK_RG_MAC_PORT_CPU,
	RTK_RG_MAC_PORT_MAX

#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)

	RTK_RG_MAC_PORT0=0,
	RTK_RG_MAC_PORT1=1,
	RTK_RG_MAC_PORT2=2,
	RTK_RG_MAC_PORT3=3,
	RTK_RG_MAC_PORT_PON=4,
	RTK_RG_MAC_PORT_RGMII=5,
	RTK_RG_MAC_PORT_CPU=6,
	RTK_RG_MAC_PORT_MAX

#elif defined(CONFIG_RTL9602C_SERIES)

	RTK_RG_MAC_PORT0=0,
	RTK_RG_MAC_PORT1,
	RTK_RG_MAC_PORT_PON,
	RTK_RG_MAC_PORT_CPU,
	RTK_RG_MAC_PORT_MAX

#endif

} rtk_rg_mac_port_idx_t;



typedef struct rtk_rg_mac_portmask_s
{
	uint32 portmask;	//the portmask bit should be defined by rtk_rg_mac_port_idx_t.
}rtk_rg_mac_portmask_t;


typedef enum rtk_rg_port_idx_e
{
#if defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_RTL9601B_SERIES)

	RTK_RG_PORT0=0,
	RTK_RG_PORT1,
	RTK_RG_PORT2,
	RTK_RG_PORT3,
	RTK_RG_PORT_PON,
	RTK_RG_PORT_RGMII,
	RTK_RG_PORT_CPU,
	RTK_RG_EXT_PORT0=7,
	RTK_RG_EXT_PORT1,
	RTK_RG_EXT_PORT2,
	RTK_RG_EXT_PORT3,
	RTK_RG_EXT_PORT4,
	RTK_RG_PORT_MAX,

#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)

	RTK_RG_PORT0=0,
	RTK_RG_PORT1=1,
	RTK_RG_PORT2=2,
	RTK_RG_PORT3=3,
	RTK_RG_PORT_PON=4, 		//for xdsl RTK_RG_PORT_PON == WAN port
	RTK_RG_PORT_RGMII=5,
	RTK_RG_PORT_CPU=6,
	RTK_RG_EXT_PORT0=7,
	RTK_RG_EXT_PORT1=8,
	RTK_RG_EXT_PORT2=9,
	RTK_RG_PORT_MAX=10,
#elif defined(CONFIG_RTL9602C_SERIES)

	RTK_RG_PORT0=0,
	RTK_RG_PORT1,
	RTK_RG_PORT_PON,
	RTK_RG_PORT_CPU,
	RTK_RG_EXT_PORT0=4,
	RTK_RG_EXT_PORT1,
	RTK_RG_EXT_PORT2,
	RTK_RG_EXT_PORT3,
	RTK_RG_EXT_PORT4,
	RTK_RG_EXT_PORT5,
	RTK_RG_PORT_MAX,

#endif
} rtk_rg_port_idx_t;

#if defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#define RTK_RG_EXT_PORT3 9	/*xdsl useless port*/
#define RTK_RG_EXT_PORT4 9	/*xdsl useless port*/
#endif
typedef struct rtk_rg_portmask_e
{
	uint32 portmask;	//the portmask bit should be defined by rtk_rg_port_idx_t.
}rtk_rg_portmask_t;

typedef enum rtk_rg_binding_idx_e
{
	RTK_RG_BD_EXT_PORT0=0,
	RTK_RG_BD_EXT_PORT1=1,
	RTK_RG_BD_EXT_PORT2=2,
	RTK_RG_BD_EXT_PORT3=3,
	RTK_RG_BD_EXT_PORT4=4,
	RTK_RG_BD_PORT_MAX,
} rtk_rg_binding_idx_t;

typedef struct rtk_rg_port_isolation_s
{
	rtk_rg_port_idx_t port;
	rtk_rg_portmask_t portmask;
}rtk_rg_port_isolation_t;

typedef struct rtk_rg_cached_port_isolation_s
{
	unsigned char valid;	//1st flag
	rtk_rg_portmask_t mask;	//record portmask need to modified: 0-0x7f
	rtk_rg_portmask_t portmask[7];	//CPU LAN+WAN port only
}rtk_rg_cached_port_isolation_t;

//System
typedef enum rtk_rg_ip_version_e
{
	IPVER_V4ONLY=0,
	IPVER_V6ONLY=1,
	IPVER_V4V6=2,
}rtk_rg_ip_version_t;

typedef enum rtk_rg_ip_updated_e
{
	ONLY_IPV4_UPDATED=0,
	ONLY_IPV6_UPDATED,
	IPV4_IPV6_UPDATED,
	NO_IP_UPDATED,
}rtk_rg_ip_updated_t;

typedef struct rtk_rg_VersionString_s
{
	char version_string[128];
} rtk_rg_VersionString_t;

typedef struct rtk_rg_macEntry_s
{
	rtk_mac_t mac;
	int isIVL; //0:SVL, 1:IVL
	int fid; //only used in SVL
	int vlan_id; //egress to this MAC, add a CVLAN tag. (vlan_id=0, untag)	
	rtk_rg_port_idx_t port_idx;
	int arp_used;
	int static_entry;
	int fix_l34_vlan;
	int auth;	
#if defined(CONFIG_RTL9602C_SERIES)	
	int ctag_if;	//0:untag, 1:tagged
#endif
}rtk_rg_macEntry_t;

typedef struct rtk_rg_arpEntry_s
{
	int macEntryIdx;
	ipaddr_t ipv4Addr;
	int staticEntry;
}rtk_rg_arpEntry_t;

//LANNefInfo
extern const char* rg_lanNet_phone_type[];
extern const char* rg_lanNet_computer_type[];
extern const char* rg_lanNet_brand[][MAX_LANNET_SUB_BRAND_SIZE];
extern const char* rg_lanNet_os[][MAX_LANNET_SUB_OS_SIZE];

typedef enum rtk_rg_lanNet_device_type_e
{
	RG_LANNET_TYPE_OTHER,
	RG_LANNET_TYPE_PHONE,
	RG_LANNET_TYPE_COMPUTER,
}rtk_rg_lanNet_device_type_t;

typedef enum rtk_rg_lanNet_brand_e
{
	RG_BRAND_OTHER=0,
	RG_BRAND_HUAWEI,
	RG_BRAND_XIAOMI,
	RG_BRAND_MEIZU,
	RG_BRAND_IPHONE,
	RG_BRAND_NOKIA,
	RG_BRAND_SAMSUNG,
	RG_BRAND_SONY,
	RG_BRAND_ERICSSON,
	RG_BRAND_MOT,
	RG_BRAND_HTC,
	RG_BRAND_SGH,		
	RG_BRAND_LG,
	RG_BRAND_SHARP,
	RG_BRAND_PHILIPS,
	RG_BRAND_PANASONIC,
	RG_BRAND_ALCATEL,
	RG_BARND_LENOVO,
	RG_BRAND_END,
}rtk_rg_lanNet_brand_t;

typedef enum rtk_rg_lanNet_os_e
{
	RG_OS_OTHER=0,
	RG_OS_WINDOWS_NT,
	RG_OS_MACINTOSH,
	RG_OS_IOS,
	RG_OS_ANDROID,
	RG_OS_WINDOWS_PHONE,
	RG_OS_LINUX,
	RG_OS_END,
}rtk_rg_lanNet_os_t;

typedef enum rtk_rg_lanNet_connect_type_e
{
	RG_CONN_MAC_PORT=0,
	RG_CONN_WIFI,
}rtk_rg_lanNet_connect_type_t;

typedef struct rtk_rg_lanNetInfo_s
{
	//connection type, Dev Name,  brand, OS, device type
	char dev_name[MAX_LANNET_DEV_NAME_LENGTH];	//for LANNetInfo
	rtk_rg_lanNet_device_type_t dev_type;
	rtk_rg_lanNet_brand_t brand;
	rtk_rg_lanNet_os_t os;
	rtk_rg_lanNet_connect_type_t conn_type;		//for LANNetInfo
}rtk_rg_lanNetInfo_t;

typedef struct rtk_rg_arpInfo_s
{
	rtk_rg_arpEntry_t arpEntry;
	int valid;
	int idleSecs;
	//lan device info
	rtk_rg_lanNetInfo_t lanNetInfo;
	char brandStr[MAX_LANNET_BRAND_NAME_LENGTH];
	char osStr[MAX_LANNET_OS_NAME_LENGTH];
}rtk_rg_arpInfo_t;

typedef struct rtk_rg_neighborEntry_s
{
	unsigned int l2Idx;
	unsigned char matchRouteIdx;
	unsigned char interfaceId[8];
	unsigned char valid;
	unsigned char staticEntry;
}rtk_rg_neighborEntry_t;

typedef struct rtk_rg_neighborInfo_s
{
	rtk_rg_neighborEntry_t neighborEntry;
	int idleSecs;
}rtk_rg_neighborInfo_t;


typedef enum rtk_rg_portMirrorInfo_direction_e
{
	RTK_RG_MIRROR_TX_RX_BOTH=0,
	RTK_RG_MIRROR_RX_ONLY,
	RTK_RG_MIRROR_TX_ONLY,
	RTK_RG_MIRROR_END
} rtk_rg_portMirrorInfo_direction_t;

typedef struct rtk_rg_portMirrorInfo_s
{
	uint32 monitorPort;
	rtk_rg_mac_portmask_t enabledPortMask;
	rtk_rg_portMirrorInfo_direction_t direct;
}rtk_rg_portMirrorInfo_t;

typedef enum rtk_rg_enable_e
{
    RTK_RG_DISABLED = 0,
    RTK_RG_ENABLED,
    RTK_RG_ENABLE_END
} rtk_rg_enable_t;
typedef enum rtk_rg_port_speed_e
{
    RTK_RG_PORT_SPEED_10M = 0,
    RTK_RG_PORT_SPEED_100M,
    RTK_RG_PORT_SPEED_1000M,
    RTK_RG_PORT_SPEED_END,
} rtk_rg_port_speed_t;
typedef enum rtk_rg_port_duplex_e
{
    RTK_RG_PORT_HALF_DUPLEX = 0,
    RTK_RG_PORT_FULL_DUPLEX,
    RTK_RG_PORT_DUPLEX_END
} rtk_rg_port_duplex_t;
typedef enum rtk_rg_port_linkStatus_e
{
    RTK_RG_PORT_LINKDOWN = 0,
    RTK_RG_PORT_LINKUP,
    RTK_RG_PORT_LINKSTATUS_END
} rtk_rg_port_linkStatus_t;

typedef struct rtk_rg_phyPortAbilityInfo_s
{
	rtk_rg_enable_t			force_disable_phy;
	rtk_rg_enable_t 		valid;
	rtk_rg_port_speed_t		speed;
	rtk_rg_port_duplex_t	duplex;
	rtk_rg_enable_t			flowCtrl;
	rtk_rg_enable_t			fc; //valid when flowCtrl is enabled
	rtk_rg_enable_t			asym_fc;// valid when flowCtrl is enabled
}rtk_rg_phyPortAbilityInfo_t;

typedef struct rtk_rg_portStatusInfo_s
{
	rtk_rg_port_linkStatus_t linkStatus;
	rtk_rg_port_speed_t		linkSpeed;
	rtk_rg_port_duplex_t	linkDuplex;
}rtk_rg_portStatusInfo_t;


typedef enum rtk_rg_storm_type_e
{
    RTK_RG_STORM_TYPE_UNKNOWN_UNICAST = 0,
    RTK_RG_STORM_TYPE_UNKNOWN_MULTICAST,
    RTK_RG_STORM_TYPE_MULTICAST,
    RTK_RG_STORM_TYPE_BROADCAST,
    RTK_RG_STORM_TYPE_DHCP,
    RTK_RG_STORM_TYPE_ARP,
    RTK_RG_STORM_TYPE_IGMP_MLD,
    RTK_RG_STORM_TYPE_END
} rtk_rg_storm_type_t;

typedef struct rtk_rg_stormControlInfo_s
{
	rtk_rg_enable_t		valid;
	rtk_rg_port_idx_t	port;
	rtk_rg_storm_type_t stormType;
	uint32 				meterIdx;
}rtk_rg_stormControlInfo_t;

typedef struct rtk_rg_qos_queue_weights_s
{
    uint32 weights[RTK_RG_MAX_NUM_OF_QUEUE];
} rtk_rg_qos_queue_weights_t;


typedef struct rtk_rg_ipv4RoutingEntry_s
{
	ipaddr_t dest_ip;
	ipaddr_t ip_mask;
	ipaddr_t nexthop; //0:for Interface route
	int wan_intf_idx;
} rtk_rg_ipv4RoutingEntry_t;

typedef struct rtk_rg_ipv6RoutingEntry_s
{
	rtk_ipv6_addr_t dest_ip;
	int prefix_len;
	int NhOrIntfIdx;
//#ifdef	CONFIG_APOLLO
	rtk_l34_ipv6RouteType_t type;
//#endif
} rtk_rg_ipv6RoutingEntry_t;

typedef enum rtk_rg_binding_type_e
{
	BIND_TYPE_PORT,
	BIND_TYPE_VLAN,
} rtk_rg_binding_type_t;

typedef struct rtk_rg_vlanBindingEntry_e
{
	rtk_rg_port_idx_t vlan_bind_port_idx;
	int vlan_bind_vlan_id;
} rtk_rg_vlanBindingEntry_t;


typedef struct rtk_rg_bindingEntry_s
{
	rtk_rg_binding_type_t type;
	union
	{
		rtk_rg_portmask_t port_bind_pmask;
		rtk_rg_vlanBindingEntry_t vlan;
	};	
	int wan_intf_idx;
} rtk_rg_bindingEntry_t;

//NAPT Flow

typedef enum rtk_rg_naptState_e
{
	INVALID		=0,
	SYN_RECV	=1,
	UDP_FIRST	=2,
	SYN_ACK_RECV =3,
	UDP_SECOND	=4,
	TCP_CONNECTED	=5,
	UDP_CONNECTED	=6,	
	FIRST_FIN	=7,
	RST_RECV	=8,	

	FIN_SEND_AND_RECV	=9,
	LAST_ACK	=10
} rtk_rg_naptState_t;


typedef enum rtk_rg_naptInType_e
{
	NAPT_IN_TYPE_SYMMETRIC_NAPT=0,
	NAPT_IN_TYPE_RESTRICTED_CONE=1,
	NAPT_IN_TYPE_FULL_CONE=2,
} rtk_rg_naptInType_t;

typedef enum rtk_rg_asicNaptInType_e
{
	ASIC_NAPT_IN_TYPE_INVALID=0,
	ASIC_NAPT_IN_TYPE_FULL_CONE=1,
	ASIC_NAPT_IN_TYPE_PORT_RESTRICTED_CONE=2,
	ASIC_NAPT_IN_TYPE_RESTRICTED_CONE=3
} rtk_rg_asicNaptInType_t;


typedef struct rtk_rg_naptEntry_s
{
	int is_tcp;
	ipaddr_t local_ip;
	ipaddr_t remote_ip;
	int wan_intf_idx;
	unsigned short int local_port;
	unsigned short int remote_port;
	unsigned short int external_port;
	unsigned char outbound_pri_valid;
	unsigned char outbound_priority;
	unsigned char inbound_pri_valid;
	unsigned char inbound_priority;
	rtk_rg_naptInType_t coneType;	//default 0 as symmetric cone type
} rtk_rg_naptEntry_t;


typedef enum  rtk_rg_naptDirection_e
{
	NAPT_DIRECTION_OUTBOUND	=0,
	NAPT_DIRECTION_INBOUND	=1,
	NAPT_DIRECTION_ROUTING	=2, //bi-direction
	IPV6_ROUTE_OUTBOUND =3,	//support for IPV6 stateful
	IPV6_ROUTE_INBOUND =4,	//support for IPv6 stateful
}rtk_rg_naptDirection_t;

typedef struct rtk_rg_naptInfo_s
{
	rtk_rg_naptEntry_t naptTuples;
	uint32	idleSecs;
	rtk_rg_naptState_t	state;
} rtk_rg_naptInfo_t;


typedef enum  rtk_rg_virtualServerMappingType_e
{
	VS_MAPPING_N_TO_N	=0,
	VS_MAPPING_N_TO_1	=1,
}rtk_rg_virtualServerMappingType_t;


#if defined(RTK_RG_INGRESS_QOS_TEST_PATCH) && defined(CONFIG_RTL9600_SERIES)
typedef enum rtk_rg_ingress_qos_testing_type_s
{
	RTK_RG_INGRESS_QOS_ORIGINAL = 0,
	RTK_RG_INGRESS_QOS_ALL_HIGH_QUEUE,
} rtk_rg_ingress_qos_testing_type_t;
#endif

#if 1
typedef enum rtk_rg_lease_time_type_e
{
	LEASE_TIME_TYPE_1WEEK=0,
	LEASE_TIME_TYPE_1DAY=1,
	LEASE_TIME_TYPE_1HOUR=2,
	LEASE_TIME_TYPE_1MIN=3,
} rtk_rg_lease_time_type_t;
#endif

typedef enum rtk_rg_house_keep_select_e
{
#if defined(CONFIG_ROME_NAPT_SHORTCUT) || defined(CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT)
	RTK_RG_SHORTCUT_HOUSE_KEEP_SELECT,
#endif
#if defined(CONFIG_RG_NAPT_AUTO_AGEOUT)
	RTK_RG_NAPT_HOUSE_KEEP_SELECT,
#endif
#if defined(CONFIG_RG_LAYER2_SOFTWARE_LEARN)
	RTK_RG_LAYER2_HOUSE_KEEP_SELECT,
#endif
#if defined(CONFIG_RG_ARP_AUTO_AGEOUT)
	RTK_RG_ARP_HOUSE_KEEP_SELECT,
	RTK_RG_NEIGHBOR_HOUSE_KEEP_SELECT,
#endif
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	RTK_RG_IPV6_STATEFUL_HOUSE_KEEP_SELECT,
	RTK_RG_IPV6_FRAGMENT_HOUSE_KEEP_SELECT,
#endif
	RTK_RG_IPV4_FRAGMENT_HOUSE_KEEP_SELECT,
	RTK_RG_ALG_DYNAMIC_PORT_HOUSE_KEEP_SELECT,
	RTK_RG_MAX_HOUSE_KEEP_SELECT,
} rtk_rg_house_keep_select_t;


// LAN Interface
typedef struct rtk_rg_lanIntfConf_s
{	
	rtk_rg_ip_version_t ip_version;		//0: ipv4, 1: ipv6, 2:both v4 & v6
	rtk_mac_t gmac;
	ipaddr_t ip_addr;
	ipaddr_t ip_network_mask;		
	rtk_ipv6_addr_t ipv6_addr;
	int ipv6_network_mask_length;
	rtk_rg_portmask_t port_mask;
	rtk_rg_mac_portmask_t untag_mask;
	//rtk_portmask_t port_mask;
	//rtk_portmask_t extport_mask;
	int intf_vlan_id;
	rtk_rg_enable_t vlan_based_pri_enable; //xdsl not support vlan pri, should set DISABLED
	int vlan_based_pri;
#if 0	
	int dhcp_server_enable; 
	rtk_rg_lease_time_type_t lease_time;
	ipaddr_t dhcp_start_ip_addr;
	ipaddr_t dhcp_end_ip_addr;
	rtk_rg_portmask_t dhcp_port_binding_mask;
	//rtk_portmask_t dhcp_port_binding_mask;
	//rtk_portmask_t dhcp_extport_binding_mask;
#endif		
	int mtu;
	//int pppoe_passThrough;		//1:turn on, 0:turn off
	int isIVL;		//0: SVL, 1:IVL
	int replace_subnet;		//0: add one more subnet, 1:delete old, add new one
} rtk_rg_lanIntfConf_t;



//WAN Interface
typedef enum rtk_rg_wan_type_e
{
	RTK_RG_STATIC=0,
	RTK_RG_DHCP,
	RTK_RG_PPPoE,
	RTK_RG_BRIDGE,
	RTK_RG_PPTP,
	RTK_RG_L2TP,		//5
	RTK_RG_DSLITE,
	RTK_RG_PPPoE_DSLITE,
} rtk_rg_wan_type_t;

typedef enum rtk_rg_wirelessWan_e
{
	RG_WWAN_WIRED=0,
	RG_WWAN_WLAN0_VXD=13,		//wlan0dev-idx
	RG_WWAN_WLAN1_VXD=27,		//wlan0dev-idx
}rtk_rg_wirelessWan_t;

typedef struct rtk_rg_wanIntfConf_s
{	
	rtk_rg_wan_type_t wan_type;
	rtk_mac_t gmac;
	//rtk_portmask_t wan_port_mask;	//PON or RGMII
	rtk_rg_port_idx_t wan_port_idx;
	rtk_rg_portmask_t port_binding_mask;
	rtk_rg_portmask_t vlan_binding_mask;	//update by vlan-binding api
	//rtk_portmask_t port_binding_mask; //LAN port
	//rtk_portmask_t extport_binding_mask; //WLAN port
	unsigned int egress_vlan_tag_on:1;
	int egress_vlan_id;
	rtk_rg_enable_t vlan_based_pri_enable;
	int vlan_based_pri;
	unsigned int isIVL:1;				//0: SVL, 1:IVL
	unsigned int none_internet:1;		//0: internet, 1:other
	unsigned int forcedAddNewIntf:1;	//0:normal, 1:forced create new interface
	unsigned int wlan0_dev_binding_mask;
} rtk_rg_wanIntfConf_t;

typedef struct rtk_rg_ipStaticInfo_s
{
	rtk_rg_ip_version_t ip_version;		//0: ipv4, 1: ipv6, 2:both v4 & v6
	int napt_enable; // L3 or L4
	int ipv6_napt_enable;//support for ipv6 napt
	ipaddr_t ip_addr;
	ipaddr_t ip_network_mask;
	int ipv4_default_gateway_on;				//1:should set default route, 0:otherwise
	ipaddr_t gateway_ipv4_addr;
	rtk_ipv6_addr_t ipv6_addr;
	int ipv6_mask_length;
	int ipv6_default_gateway_on;			//1:should set default route, 0:otherwise
	rtk_ipv6_addr_t gateway_ipv6_addr;
	int mtu;
	int gw_mac_auto_learn_for_ipv4;
	int gw_mac_auto_learn_for_ipv6;
	rtk_mac_t gateway_mac_addr_for_ipv4;
	rtk_mac_t gateway_mac_addr_for_ipv6;
	unsigned int static_route_with_arp:1;	//routing by ARP, policy route by NX
} rtk_rg_ipStaticInfo_t;

typedef struct rtk_rg_ipDsliteStaticInfo_s
{
	rtk_rg_ipStaticInfo_t static_info;
	rtk_l34_dsliteInf_entry_t rtk_dslite;
	int aftr_mac_auto_learn;
	rtk_mac_t aftr_mac_addr;
} rtk_rg_ipDslitStaticInfo_t;

typedef enum rtk_rg_dhcp_status_e
{
	DHCP_STATUS_LEASED=0,
	DHCP_STATUS_RELEASED=1
} rtk_rg_dhcp_status_t;

typedef struct rtk_rg_ipDhcpClientInfo_s
{
	rtk_rg_ipStaticInfo_t hw_info;
	rtk_rg_dhcp_status_t stauts; //leased, released
} rtk_rg_ipDhcpClientInfo_t;

typedef enum rtk_rg_ppp_auth_type_e
{
	PPP_AUTH_TYPE_PAP=0,
	PPP_AUTH_TYPE_CHAP=1
} rtk_rg_ppp_auth_type_t;

typedef int (*p_dialOnDemondCallBack)(unsigned int);
typedef int (*p_idleTimeOutCallBack)(unsigned int);

typedef enum rtk_rg_ppp_status_e
{
	PPP_STATUS_DISCONNECT=0,
	PPP_STATUS_CONNECT=1
} rtk_rg_ppp_status_t;

typedef struct rtk_rg_pppoeClientInfoBeforeDial_s
{
	char username[32];
	char password[32]; 
	rtk_rg_ppp_auth_type_t auth_type;
	int pppoe_proxy_enable;
	int max_pppoe_proxy_num;
	int auto_reconnect;
	int dial_on_demond;
	int idle_timeout_secs;
	rtk_rg_ppp_status_t stauts; //connect/disconnect?
	p_dialOnDemondCallBack dialOnDemondCallBack;
	p_idleTimeOutCallBack idleTimeOutCallBack;
} rtk_rg_pppoeClientInfoBeforeDial_t;

typedef struct rtk_rg_pppoeClientInfoAfterDial_s
{
	rtk_rg_ipStaticInfo_t hw_info;
	unsigned short int sessionId;
} rtk_rg_pppoeClientInfoAfterDial_t;

typedef struct rtk_rg_pppoeDsliteInfoAfterDial_s
{
	rtk_rg_ipDslitStaticInfo_t dslite_hw_info;
	unsigned short int sessionId;
} rtk_rg_pppoeDsliteInfoAfterDial_t;

typedef struct rtk_rg_ipPppoeClientInfo_s
{
	rtk_rg_pppoeClientInfoBeforeDial_t before_dial;
	rtk_rg_pppoeClientInfoAfterDial_t after_dial;
} rtk_rg_ipPppoeClientInfo_t;

typedef struct rtk_rg_pptpClientInfoBeforeDial_s
{
	char username[32];
	char password[32]; 
	ipaddr_t pptp_ipv4_addr;
} rtk_rg_pptpClientInfoBeforeDial_t;

typedef struct rtk_rg_pptpClientInfoAfterDial_s
{
	rtk_rg_ipStaticInfo_t hw_info;
	
	unsigned short callId;
	unsigned short gateway_callId;
} rtk_rg_pptpClientInfoAfterDial_t;

typedef struct rtk_rg_ipPPTPClientInfo_s
{
	rtk_rg_pptpClientInfoBeforeDial_t before_dial;
	rtk_rg_pptpClientInfoAfterDial_t after_dial;

	unsigned int gre_header_sequence;
	unsigned int gre_header_acknowledgment;
	unsigned short ipv4_header_identifier;
	unsigned int sw_gre_header_sequence;	//from protocol stack output sequence
	unsigned int sw_gre_header_server_sequence;	//from server input sequence
	unsigned int sw_gre_header_server_sequence_started:1;
} rtk_rg_ipPPTPClientInfo_t;

typedef struct rtk_rg_l2tpClientInfoBeforeDial_s
{
	char username[32];
	char password[32]; 
	ipaddr_t l2tp_ipv4_addr;
} rtk_rg_l2tpClientInfoBeforeDial_t;

typedef struct rtk_rg_l2tpClientInfoAfterDial_s
{
	rtk_rg_ipStaticInfo_t hw_info;
	
	unsigned short outer_port;				//used in outer UDP encapsulation
	unsigned short gateway_outer_port;		//used in outer UDP encapsulation
	unsigned short tunnelId;
	unsigned short gateway_tunnelId;
	unsigned short sessionId;
	unsigned short gateway_sessionId;
} rtk_rg_l2tpClientInfoAfterDial_t;

typedef struct rtk_rg_ipL2TPClientInfo_s
{
	rtk_rg_l2tpClientInfoBeforeDial_t before_dial;
	rtk_rg_l2tpClientInfoAfterDial_t after_dial;

	unsigned short ipv4_header_identifier;
} rtk_rg_ipL2TPClientInfo_t;

typedef struct rtk_rg_ipPppoeDsliteInfo_s
{
	rtk_rg_pppoeClientInfoBeforeDial_t before_dial;
	rtk_rg_pppoeDsliteInfoAfterDial_t after_dial;
} rtk_rg_ipPppoeDsliteInfo_t;

typedef struct rtk_rg_wanIntfInfo_s
{
	rtk_rg_wanIntfConf_t wan_intf_conf;
	union{
		rtk_rg_ipStaticInfo_t static_info;
		rtk_rg_ipDhcpClientInfo_t	dhcp_client_info;
		rtk_rg_ipPppoeClientInfo_t pppoe_info;
		rtk_rg_ipPPTPClientInfo_t pptp_info;
		rtk_rg_ipL2TPClientInfo_t l2tp_info;
		rtk_rg_ipDslitStaticInfo_t dslite_info;
		rtk_rg_ipPppoeDsliteInfo_t pppoe_dslite_info;
	};
	int bind_wan_type_ipv4;
	int bind_wan_type_ipv6;
	rtk_rg_wirelessWan_t wirelessWan;			//0:wire WAN, 13:wireless WAN(wlan0-vxd), 27:wireless WAN(wlan1-vxd)
	int nexthop_ipv4;
	int nexthop_ipv6;
	int pppoe_idx;	//idx for pppoe table
	int extip_idx;	//idx for ext ip table
	int baseIntf_idx;	//for pptp/l2tp
#if defined(CONFIG_RTL9602C_SERIES)
	int dslite_idx;	//for dslite/pppoe_dslite
#endif
} rtk_rg_wanIntfInfo_t;

typedef struct rtk_rg_intfInfo_s
{
	char intf_name[32];
	int is_wan;
	union 
	{
		rtk_rg_lanIntfConf_t lan_intf;
		rtk_rg_wanIntfInfo_t wan_intf;
	};
} rtk_rg_intfInfo_t;

//IPv6 server in Lan: upnp, virtual server, dmz
typedef enum rtk_rg_ipv6_connLookup_hitIf_s
{
	RTK_RG_IPV6_LOOKUP_NONE_HIT=0,
	RTK_RG_IPV6_LOOKUP_UPNP_HIT,
	RTK_RG_IPV6_LOOKUP_VIRTUALSERVER_HIT,
	RTK_RG_IPV6_LOOKUP_DMZ_HIT,

}rtk_rg_ipv6_connLookup_hitIf_t;

typedef struct rtk_rg_ipv6_connLookup_s
{
	rtk_rg_ipv6_connLookup_hitIf_t serverInLanHit;
	int hitIndex;	//valid while serverInLanHit is 1~3, represent the hit upnp or virtual_server or dmz rule index.
	rtk_ipv6_addr_t transIP;
	int16 transPort;
}rtk_rg_ipv6_connLookup_t;


//VLAN function
typedef struct rtk_rg_cvlan_info_s
{
	int vlanId;
	int isIVL;		//0: SVL, 1:IVL
	rtk_rg_portmask_t memberPortMask;
	rtk_rg_mac_portmask_t untagPortMask;
	rtk_rg_enable_t vlan_based_pri_enable;
	int vlan_based_pri;
}rtk_rg_cvlan_info_t;

//VLAN Binding
typedef struct rtk_rg_vlanBinding_s
{
	rtk_rg_port_idx_t port_idx;
	//rtk_portmask_t port_mask;
	//rtk_portmask_t ext_port_mask;
	int ingress_vid;
	int wan_intf_idx;
}rtk_rg_vlanBinding_t;

//DMZ
typedef struct rtk_rg_dmzInfo_s
{
	int enabled;
	int mac_mapping_enabled;
	int ipversion; //0:v4_only 1:v6v4_only 2:both_v4_v6
	union
	{
		ipaddr_t private_ip; //only used when mac_mapping_enabled=0 && (ipversion==0 || ipversion==2)
		rtk_mac_t mac; //only used when mac_mapping_enabled=1
	};
	rtk_ipv6_addr_t	private_ipv6;//only used when mac_mapping_enabled=0 && (ipversion==1 || ipversion==2)
	
} rtk_rg_dmzInfo_t;

typedef enum rtk_rg_ipv4MulticastFlowMode_e
{
	RTK_RG_IPV4MC_DONT_CARE_SRC=0,
	RTK_RG_IPV4MC_INCLUDE,
	RTK_RG_IPV4MC_EXCLUDE,
} rtk_rg_ipv4MulticastFlowMode_t;

//Multicast flow
typedef struct rtk_rg_multicastFlow_s
{
	ipaddr_t 	multicast_ipv4_addr;
	ipaddr_t	multicast_ipv6_addr[4];
	int		isIPv6;	
	rtk_rg_ipv4MulticastFlowMode_t srcFilterMode;
	union{
		rtk_rg_portmask_t port_mask;
		rtk_rg_portmask_t excludeModePortmask;
		rtk_rg_portmask_t includeModePortmaskList[MAX_IPMC_IP_LIST];
	};
	ipaddr_t includeOrExcludeIpList[MAX_IPMC_IP_LIST]; //v4

	int maxIpListNums;
	unsigned int isIVL:1;
	unsigned short vlanID;	//used for IVL only
} rtk_rg_multicastFlow_t;


typedef struct rtk_rg_l2MulticastFlow_s
{
	rtk_mac_t mac;
	rtk_rg_portmask_t port_mask;
	unsigned int isIVL:1;
	unsigned short vlanID;	//used for IVL only
} rtk_rg_l2MulticastFlow_t;

typedef struct rtk_rg_ipv4MulticastFlow_s
{
	ipaddr_t groupIp;	
	rtk_rg_ipv4MulticastFlowMode_t srcFilterMode;
	int maxIpListNums;
	ipaddr_t includeOrExcludeIpList[MAX_IPMC_IP_LIST];
	union{
		rtk_rg_portmask_t includeModePortmaskList[MAX_IPMC_IP_LIST];
		rtk_rg_portmask_t excludeModePortmask;
		rtk_rg_portmask_t dontCareSipModePortmask;
	};	
} rtk_rg_ipv4MulticastFlow_t;

#if defined(CONFIG_RTL9602C_SERIES)
typedef struct rtk_rg_ipv6MulticastFlow_s
{
    rtk_ipv6_addr_t groupIp6;
	rtk_rg_portmask_t portMaskIPMC6;

} rtk_rg_ipv6MulticastFlow_t;

#endif

typedef enum rtk_rg_multicastProtocol_e
{
	RG_MC_BOTH_IGMP_MLD=0,
	RG_MC_IGMP_ONLY,
	RG_MC_MLD_ONLY,
	RG_MC_PROTOCOL_END,
}rtk_rg_multicastProtocol_t;

//ALG
typedef enum rtk_rg_alg_function_sequence_e
{
	//Server in WAN
	RTK_RG_ALG_SIP_TCP=0,		
	RTK_RG_ALG_SIP_UDP,
	RTK_RG_ALG_H323_TCP,
	RTK_RG_ALG_H323_UDP,
	RTK_RG_ALG_RTSP_TCP,
	RTK_RG_ALG_RTSP_UDP,
	RTK_RG_ALG_FTP_TCP,
	RTK_RG_ALG_FTP_UDP,			//7

	//Server in LAN
	RTK_RG_ALG_SIP_TCP_SRV_IN_LAN,
	RTK_RG_ALG_SIP_UDP_SRV_IN_LAN,
	RTK_RG_ALG_H323_TCP_SRV_IN_LAN,
	RTK_RG_ALG_H323_UDP_SRV_IN_LAN,
	RTK_RG_ALG_RTSP_TCP_SRV_IN_LAN,
	RTK_RG_ALG_RTSP_UDP_SRV_IN_LAN,
	RTK_RG_ALG_FTP_TCP_SRV_IN_LAN,
	RTK_RG_ALG_FTP_UDP_SRV_IN_LAN,		//15

	//Pass through
	RTK_RG_ALG_PPTP_TCP_PASSTHROUGH,
	RTK_RG_ALG_PPTP_UDP_PASSTHROUGH,
	RTK_RG_ALG_L2TP_TCP_PASSTHROUGH,
	RTK_RG_ALG_L2TP_UDP_PASSTHROUGH,
	RTK_RG_ALG_IPSEC_TCP_PASSTHROUGH,
	RTK_RG_ALG_IPSEC_UDP_PASSTHROUGH,		//21
	
	RTK_RG_ALG_PPPOE_PASSTHROUGH,
#ifdef CONFIG_RG_ROMEDRIVER_ALG_BATTLENET_SUPPORT
	RTK_RG_ALG_BATTLENET_TCP,
#endif
	RTK_RG_ALG_FUNCTION_END,
}rtk_rg_alg_function_sequence_t;

typedef enum rtk_rg_alg_type_e
{
	//Server in WAN
	RTK_RG_ALG_SIP_TCP_BIT					=0x1,
	RTK_RG_ALG_SIP_UDP_BIT					=0x2,
	RTK_RG_ALG_H323_TCP_BIT					=0x4,
	RTK_RG_ALG_H323_UDP_BIT					=0x8,
	RTK_RG_ALG_RTSP_TCP_BIT					=0x10,
	RTK_RG_ALG_RTSP_UDP_BIT					=0x20,
	RTK_RG_ALG_FTP_TCP_BIT					=0x40,
	RTK_RG_ALG_FTP_UDP_BIT					=0x80,			//8

	//Server in LAN
	RTK_RG_ALG_SIP_TCP_SRV_IN_LAN_BIT		=0x100,
	RTK_RG_ALG_SIP_UDP_SRV_IN_LAN_BIT		=0x200,
	RTK_RG_ALG_H323_TCP_SRV_IN_LAN_BIT		=0x400,
	RTK_RG_ALG_H323_UDP_SRV_IN_LAN_BIT		=0x800,
	RTK_RG_ALG_RTSP_TCP_SRV_IN_LAN_BIT		=0x1000,
	RTK_RG_ALG_RTSP_UDP_SRV_IN_LAN_BIT		=0x2000,
	RTK_RG_ALG_FTP_TCP_SRV_IN_LAN_BIT		=0x4000,
	RTK_RG_ALG_FTP_UDP_SRV_IN_LAN_BIT		=0x8000,		//16

	//Pass through
	RTK_RG_ALG_PPTP_TCP_PASSTHROUGH_BIT		=0x10000,
	RTK_RG_ALG_PPTP_UDP_PASSTHROUGH_BIT		=0x20000,
	RTK_RG_ALG_L2TP_TCP_PASSTHROUGH_BIT		=0x40000,
	RTK_RG_ALG_L2TP_UDP_PASSTHROUGH_BIT		=0x80000,
	RTK_RG_ALG_IPSEC_TCP_PASSTHROUGH_BIT	=0x100000,
	RTK_RG_ALG_IPSEC_UDP_PASSTHROUGH_BIT	=0x200000,		//22
	
	RTK_RG_ALG_PPPOE_PASSTHROUGH_BIT		=0x400000,

	//BattleNet
#ifdef CONFIG_RG_ROMEDRIVER_ALG_BATTLENET_SUPPORT
	RTK_RG_ALG_BATTLENET_TCP_BIT 			=0x800000,
#endif
	
}rtk_rg_alg_type_t;

typedef enum rtk_rg_alg_tcpKnownPort_e
{
	RTK_RG_ALG_SIP_TCP_PORT=5060,
	RTK_RG_ALG_H323_TCP_PORT=1720,
	RTK_RG_ALG_RTSP_TCP_PORT=554,
	RTK_RG_ALG_PPTP_PASSTHROUGH_TCP_PORT=1723,
	RTK_RG_ALG_L2TP_PASSTHROUGH_TCP_PORT=1701,
	RTK_RG_ALG_IPSEC_PASSTHROUGH_TCP_PORT=500,		//used by ISAKMP
	RTK_RG_ALG_FTP_TCP_PORT=21,
	RTK_RG_ALG_FTP_DATA_TCP_PORT=20,
#ifdef CONFIG_RG_ROMEDRIVER_ALG_BATTLENET_SUPPORT
	RTK_RG_ALG_BATTLENET_TCP_PORT=6112,
#endif
}rtk_rg_alg_tcpKnownPort_t;

typedef enum rtk_rg_alg_udpKnownPort_e
{
	RTK_RG_ALG_SIP_UDP_PORT=5060,
	RTK_RG_ALG_H323_UDP_PORT=1720,
	RTK_RG_ALG_RTSP_UDP_PORT=554,
	RTK_RG_ALG_PPTP_PASSTHROUGH_UDP_PORT=1723,
	RTK_RG_ALG_L2TP_PASSTHROUGH_UDP_PORT=1701,
	RTK_RG_ALG_IPSEC_PASSTHROUGH_UDP_PORT=500,		//used by ISAKMP
	RTK_RG_ALG_FTP_UDP_PORT=21,
	RTK_RG_ALG_FTP_DATA_UDP_PORT=20,
}rtk_rg_alg_udpKnownPort_t;

typedef int (*p_algRegisterFunction)(int,int,unsigned char*,unsigned char*);	//the third is sk_buff* ,the fourth is type rtk_rg_pktHdr_t*

typedef struct rtk_rg_alg_funcMapping_s
{
	unsigned short int portNum;
	p_algRegisterFunction registerFunction;
	unsigned char keepExtPort;
}rtk_rg_alg_funcMapping_t;

typedef struct rtk_rg_alg_serverIpMapping_s
{
	rtk_rg_alg_type_t algType;
	ipaddr_t serverAddress;
}rtk_rg_alg_serverIpMapping_t;

typedef struct rtk_rg_alg_dynamicPort_s
{
	p_algRegisterFunction algFun;
	unsigned short int portNum;
	int timeout;
	int isTCP;
	int serverInLan;	//0:Server In Wan, 1:Server In Lan
	ipaddr_t intIP;		//for serverInLan's internal server ip

	struct list_head alg_list;
}rtk_rg_alg_dynamicPort_t;

#ifdef __KERNEL__
#if defined(CONFIG_DEFAULTS_KERNEL_3_18)
//copy from fs/proc/internal.h
struct proc_dir_entry {
	unsigned int low_ino;
	umode_t mode;
	nlink_t nlink;
	kuid_t uid;
	kgid_t gid;
	loff_t size;
	const struct inode_operations *proc_iops;
	const struct file_operations *proc_fops;
	struct proc_dir_entry *next, *parent, *subdir;
	void *data;
	atomic_t count;		/* use count */
	atomic_t in_use;	/* number of callers into module in progress; */
			/* negative -> it's going away RSN */
	struct completion *pde_unload_completion;
	struct list_head pde_openers;	/* who did ->open, but not ->release */
	spinlock_t pde_unload_lock; /* proc_fops checks and pde_users bumps */
	u8 namelen;
	char name[];
};
#endif
typedef struct rtk_rg_proc_s
{
	char *name;
	int (*get) (struct seq_file *s, void *v);
	int (*set) ( struct file *, const char *,unsigned long, void *);
	unsigned int inode_id;
	struct file_operations proc_fops;
}rtk_rg_proc_t;
#endif

/* FTP */
//LUKE20130816: move to separate ALG file
#if 0
//entry for each FTP session
typedef struct rtk_rg_ftpCtrlFlowEntry_s {
	unsigned int remoteIpAddr;		//remote host ip
	unsigned int internalIpAddr;			//NPI internal ip

	unsigned short int remotePort;	//remote port
	unsigned short int internalPort;		//internal port

	//if outbound enlarge packet, Delta is positive value;
	//if outbound shrink packet, Delta is negative value
	int Delta;								//used to sync acknowledgement
		
	struct rtk_rg_ftpCtrlFlowEntry_s *pNext,*pPrev;
}rtk_rg_ftpCtrlFlowEntry_t;
#endif

/* DHCP */
//bootp message type
typedef struct rtk_rg_dhcpMsgType_s
{
	uint8 op;
	uint8 htype;
	uint8 hlen;
	uint8 hops;
	uint32 xid;
	uint16 secs;
	uint16 flags;
	uint32 ciaddr;
	uint32 yiaddr;
	uint32 siaddr;
	uint32 giaddr;
	uint8 chaddr[16];
	uint8 sname[64];
	uint8 file[128];
	uint32 cookie;
	//uint8 options[COOKIE_LEN]; /* 312 - cookie */ 
}rtk_rg_dhcpMsgType_t;


/* PPTP */
//PPTP message type
typedef enum rtk_rg_pptpCtrlMsgType_e
{
	PPTP_StartCtrlConnRequest 	= 1,
	PPTP_StartCtrlConnReply 	= 2,
	PPTP_StopCtrlConnRequest 	= 3,
	PPTP_StopCtrlConnReply 		= 4,
	PPTP_EchoRequest 			= 5,
	PPTP_EchoReply 				= 6,
	PPTP_OutCallRequest 		= 7,
	PPTP_OutCallReply 			= 8,
	PPTP_InCallRequest 			= 9,
	PPTP_InCallReply 			= 10,
	PPTP_InCallConn 			= 11,
	PPTP_CallClearRequest 		= 12,
	PPTP_CallDiscNotify 		= 13,
	PPTP_WanErrorNotify 		= 14,
	PPTP_SetLinkInfo 			= 15
}rtk_rg_pptpCtrlMsgType_t;

//Message structures 
typedef struct rtk_rg_pptpMsgHead_s
{
	unsigned short int    length;			/* total length */
	unsigned short int    msgType;			/* PPTP message type */
	unsigned int      	  magic;			/* magic cookie */
	unsigned short int    type;				/* control message type */
	unsigned short int    resv0;			/* reserved */
}rtk_rg_pptpMsgHead_t;

typedef struct rtk_rg_pptpCallIds_s
{
	unsigned short int    cid1;				/* Call ID field #1 */
	unsigned short int    cid2;				/* Call ID field #2 */
}rtk_rg_pptpCallIds_t;

typedef struct rtk_rg_pptpCodes_s
{
	unsigned char     resCode;				/* Result Code */
	unsigned char     errCode;				/* Error Code */
}rtk_rg_pptpCodes_t;

//20130821LUKE:close it because implemented by separate module
#if 0
//GRE entry for each PPTP session
typedef struct rtk_rg_pptpGreEntry_s 
{
	unsigned int remoteIpAddr;
	rtk_mac_t remoteMacAddr;
	unsigned int internalIpAddr;
	rtk_mac_t internalMacAddr;

	unsigned short int remoteCallID;
	unsigned short int externalCallID;
	unsigned short int internalCallID;

	unsigned char valid;
}rtk_rg_pptpGreEntry_t;

typedef struct rtk_rg_pptpGreLinkList_s 
{
	rtk_rg_pptpGreEntry_t greEntry;
	struct rtk_rg_pptpGreLinkList_s *pPrev, *pNext;
}rtk_rg_pptpGreLinkList_t;
#endif

/* L2TP */
typedef struct rtk_rg_alg_l2tp_ctrlHeader_s
{
	uint16 flag;		//must be 0xc802
	uint16 length;
	uint16 tunnel_id;
	uint16 session_id;
	uint16 numberSent;
	uint16 numberReceived;
}rtk_rg_alg_l2tp_ctrlHeader_t;

typedef struct rtk_rg_alg_l2tp_avpHeader_s
{
	uint8 MH;		
	uint8 length;
	uint16 vendor_id;	//normal are zero
	uint16 attType;
}rtk_rg_alg_l2tp_avpHeader_t;

typedef struct rtk_rg_alg_l2tp_flow_s
{
	uint8 valid;
	ipaddr_t internalIP;
	ipaddr_t remoteIP;
	uint16 IntTulID;	//internal tunnel ID
	uint16 ExtTulID;	//external tunnel ID
}rtk_rg_alg_l2tp_flow_t;

typedef struct rtk_rg_alg_l2tp_linkList_s
{
	rtk_rg_alg_l2tp_flow_t l2tpFlow;
	struct rtk_rg_alg_l2tp_linkList_s *pPrev, *pNext;
}rtk_rg_alg_l2tp_linkList_t;

//VirtualServer(PortForward)
typedef struct rtk_rg_virtualServer_s
{
	int ipversion; //0:v4_only 1:v6v4_only 2:both_v4_v6
	int is_tcp;
	int wan_intf_idx; //for gateway ip
	int gateway_port_start;
	ipaddr_t local_ip;//only used when (ipversion==0 || ipversion==2)
	rtk_ipv6_addr_t	local_ipv6;//only used when (ipversion==1 || ipversion==2)
	unsigned short int local_port_start;
	int mappingPortRangeCnt;
	rtk_rg_virtualServerMappingType_t mappingType;
	int valid;
	rtk_rg_alg_type_t hookAlgType;	//only for server-in-lan
	unsigned int disable_wan_check;	//0: turn-on wan interface check, 1: disable wan interface check
} rtk_rg_virtualServer_t;


//GPON downstream BC remarking

typedef enum rtk_rg_gpon_ds_bc_filter_fields_e
{
	GPON_DS_BC_FILTER_INGRESS_STREAMID_BIT=(1<<0),
	GPON_DS_BC_FILTER_INGRESS_STAGIf_BIT=(1<<1),
	GPON_DS_BC_FILTER_INGRESS_CTAGIf_BIT=(1<<2),
	GPON_DS_BC_FILTER_INGRESS_SVID_BIT=(1<<3),
	GPON_DS_BC_FILTER_INGRESS_CVID_BIT=(1<<4),
	GPON_DS_BC_FILTER_EGRESS_PORT_BIT=(1<<5),
}rtk_rg_gpon_ds_bc_filter_fields_t;

typedef enum rtk_rg_gpon_ds_bc_tag_decision_e
{
	RTK_RG_GPON_BC_FORCE_UNATG,
	RTK_RG_GPON_BC_FORCE_TAGGIN_WITH_CVID,
	RTK_RG_GPON_BC_FORCE_TAGGIN_WITH_CVID_CPRI,
}rtk_rg_gpon_ds_bc_tag_decision_t;


typedef struct rtk_rg_gpon_ds_bc_action_s
{
	rtk_rg_gpon_ds_bc_tag_decision_t ctag_decision;
	uint16 assigned_ctag_cvid;
	uint16 assigned_ctag_cpri;//valid only when ctag_decision is RTK_RG_GPON_BC_FORCE_TAGGIN_WITH_CVID_CPRI
}rtk_rg_gpon_ds_bc_action_t;


typedef struct rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_s
{
	//care patterns
	unsigned int filter_fields; // please refer to rtk_rg_gpon_ds_bc_filter_fields_t typedef.

	//assign value for care patterns
	uint16 ingress_stream_id;
	uint8 ingress_stagIf;
	uint8 ingress_ctagIf;
	uint16 ingress_stag_svid;
	uint16 ingress_ctag_cvid;
	rtk_rg_portmask_t egress_portmask;

	//action: force assign ctag
	rtk_rg_gpon_ds_bc_action_t ctag_action;
}rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t;


typedef struct rtk_rg_sw_gpon_ds_bc_vlanfilterAndRemarking_s
{
	uint32 valid;
	rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t filterRule;
}rtk_rg_sw_gpon_ds_bc_vlanfilterAndRemarking_t;


//ACL
#if 1
typedef enum rtk_rg_cfpri_decision_e{
	ACL_CFPRI_ASSIGN,
	ACL_CFPRI_NOP,
}rtk_rg_cfpri_decision_t;

typedef struct rtk_rg_cfpri_action_s{
	rtk_rg_cfpri_decision_t cfPriDecision;
	uint8 assignedCfPri;
}rtk_rg_cfpri_action_t;

typedef enum rtk_rg_sid_llid_decision_e{
	ACL_SID_LLID_ASSIGN,
	ACL_SID_LLID_NOP,
}rtk_rg_sid_llid_decision_t;

typedef struct rtk_rg_sid_llid_action_s{
	rtk_rg_sid_llid_decision_t sidDecision;
	uint32 assignedSid_or_llid;
}rtk_rg_sid_llid_action_t;

typedef enum rtk_rg_dscp_decision_e{
	ACL_DSCP_ASSIGN,
	ACL_DSCP_NOP,
}rtk_rg_dscp_decision_t;

typedef struct rtk_rg_dscp_action_s{
	rtk_rg_dscp_decision_t dscpDecision;
	uint8 assignedDscp;
}rtk_rg_dscp_action_t;


typedef struct rtk_rg_log_action_s{
	uint8 assignedCounterIdx;
}rtk_rg_log_action_t;

typedef enum rtk_rg_acl_uni_decision_e{
	ACL_UNI_FWD_TO_PORTMASK_ONLY,
	ACL_UNI_FORCE_BY_MASK,
	ACL_UNI_TRAP_TO_CPU,
	AL_UNI_NOP,
}rtk_rg_acl_uni_decision_t;

typedef struct rtk_rg_uni_action_s{
	rtk_rg_acl_uni_decision_t	uniActionDecision;
	uint32 assignedUniPortMask;
}rtk_rg_uni_action_t;

typedef enum rtk_rg_acl_fwd_decision_e{
	ACL_FWD_NOP,
	ACL_FWD_DROP,
	ACL_FWD_TRAP_TO_CPU,
	ACL_FWD_DROP_TO_PON,
}rtk_rg_acl_fwd_decision_t;

typedef struct rtk_rg_fwd_action_s{
	rtk_rg_acl_fwd_decision_t	fwdDecision;
}rtk_rg_fwd_action_t;


typedef enum rtk_rg_acl_cvlan_tagif_decision_e{
	//The new action should alway put on later
	ACL_CVLAN_TAGIF_NOP,
	ACL_CVLAN_TAGIF_TAGGING,
	ACL_CVLAN_TAGIF_TAGGING_WITH_C2S, //apollo only 
	ACL_CVLAN_TAGIF_TAGGING_WITH_SP2C, //apollo only
	ACL_CVLAN_TAGIF_UNTAG,
	ACL_CVLAN_TAGIF_TRANSPARENT,
	ACL_CVLAN_TAGIF_END,
}rtk_rg_acl_cvlan_tagif_decision_t;

typedef enum rtk_rg_acl_cvlan_cvid_decision_e{
	//The new action should alway put on later
	ACL_CVLAN_CVID_ASSIGN,
	ACL_CVLAN_CVID_COPY_FROM_1ST_TAG,
	ACL_CVLAN_CVID_COPY_FROM_2ND_TAG,
	ACL_CVLAN_CVID_COPY_FROM_INTERNAL_VID, //(upstream only)
	ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID, //(downstream only)
	ACL_CVLAN_CVID_NOP, //apolloFE  (downstream only)
	ACL_CVLAN_CVID_CPOY_FROM_SP2C, //apolloFE 
	
	ACL_CVLAN_CVID_END,
}rtk_rg_acl_cvlan_cvid_decision_t;

typedef enum rtk_rg_acl_cvlan_cpri_decision_e{
	//The new action should alway put on later
	ACL_CVLAN_CPRI_ASSIGN,
	ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG,
	ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG,
	ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI,
	ACL_CVLAN_CPRI_NOP,//apolloFE
	ACL_CVLAN_CPRI_COPY_FROM_DSCP_REMAP,//apolloFE 
	ACL_CVLAN_CPRI_COPY_FROM_SP2C, //apolloFE (downstream only)	
	ACL_CVLAN_CPRI_END,
}rtk_rg_acl_cvlan_cpri_decision_t;

typedef enum rtk_rg_acl_svlan_tagif_decision_e{
	//The new action should alway put on later
	ACL_SVLAN_TAGIF_NOP,
	ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID,
	ACL_SVLAN_TAGIF_TAGGING_WITH_8100, //apollo
	ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C, //apollo (downstream only)
	ACL_SVLAN_TAGIF_UNTAG,
	ACL_SVLAN_TAGIF_TRANSPARENT,
	ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2, //apolloFE 
	ACL_SVLAN_TAGIF_TAGGING_WITH_ORIGINAL_STAG_TPID, //apolloFE 
	ACL_SVLAN_TAGIF_END,
}rtk_rg_acl_svlan_tagif_decision_t;

typedef enum rtk_rg_acl_svlan_svid_decision_e{
	//The new action should alway put on later
	ACL_SVLAN_SVID_ASSIGN,
	ACL_SVLAN_SVID_COPY_FROM_1ST_TAG,
	ACL_SVLAN_SVID_COPY_FROM_2ND_TAG,
	ACL_SVLAN_SVID_NOP, //apolloFE
	ACL_SVLAN_SVID_SP2C, //apolloFE
	ACL_SVLAN_SVID_END,
}rtk_rg_acl_svlan_svid_decision_t;

typedef enum rtk_rg_acl_svlan_spri_decision_e{
	//The new action should alway put on later
	ACL_SVLAN_SPRI_ASSIGN,
	ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG,
	ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG,
	ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI,
	ACL_SVLAN_SPRI_NOP,//apolloFE
	ACL_SVLAN_SPRI_COPY_FROM_DSCP_REMAP, //apolloFE (downstream only)
	ACL_SVLAN_SPRI_COPY_FROM_SP2C, //apolloFE (downstream only)	
	ACL_SVLAN_SPRI_END,
}rtk_rg_acl_svlan_spri_decision_t;
#endif

typedef enum rtk_rg_acl_action_type_e
{
	ACL_ACTION_TYPE_DROP=0,
	ACL_ACTION_TYPE_PERMIT,
	ACL_ACTION_TYPE_TRAP,
	ACL_ACTION_TYPE_QOS,	//action of stream_id, CVLAN, SVLAN also belong to this type
	ACL_ACTION_TYPE_TRAP_TO_PS,
	ACL_ACTION_TYPE_POLICY_ROUTE,		//sw only
	ACL_ACTION_TYPE_END
} rtk_rg_acl_action_type_t;

typedef enum rtk_rg_acl_filter_and_qos_action_e
{
	ACL_ACTION_NOP_BIT=(1<<0),
	ACL_ACTION_1P_REMARKING_BIT=(1<<1),
	ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT=(1<<2),
	ACL_ACTION_DSCP_REMARKING_BIT=(1<<3),
	ACL_ACTION_QUEUE_ID_BIT=(1<<4),
	ACL_ACTION_SHARE_METER_BIT=(1<<5),	
	ACL_ACTION_STREAM_ID_OR_LLID_BIT=(1<<6),	
	ACL_ACTION_ACL_PRIORITY_BIT=(1<<7),
	ACL_ACTION_ACL_CVLANTAG_BIT=(1<<8),	
	ACL_ACTION_ACL_SVLANTAG_BIT=(1<<9),	
	ACL_ACTION_ACL_INGRESS_VID_BIT=(1<<10),
	ACL_ACTION_DS_UNIMASK_BIT	= (1<<11),		//only support in ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN(type 4)
	ACL_ACTION_REDIRECT_BIT = (1<<12), //only support in ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET(type 0)
	ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT = (1<<13), //only support in ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_XXXX(type1~4)
	ACL_ACTION_QOS_END=(1<<14), 
} rtk_rg_acl_qos_action_t;


typedef enum rtk_rg_acl_filter_fields_e
{
	INGRESS_PORT_BIT=0x1,
	INGRESS_INTF_BIT=0x2,
	EGRESS_INTF_BIT=0x4,		//not support in 0371 testchip
	INGRESS_ETHERTYPE_BIT=0x8,
	INGRESS_CTAG_PRI_BIT=0x10,
	INGRESS_CTAG_VID_BIT=0x20,
	INGRESS_SMAC_BIT=0x40,
	INGRESS_DMAC_BIT=0x80,
	INGRESS_DSCP_BIT=0x100,
	INGRESS_L4_TCP_BIT=0x200,	
	INGRESS_L4_UDP_BIT=0x400,	
	INGRESS_IPV6_SIP_RANGE_BIT=0x800,
	INGRESS_IPV6_DIP_RANGE_BIT=0x1000,
	INGRESS_IPV4_SIP_RANGE_BIT=0x2000,	
	INGRESS_IPV4_DIP_RANGE_BIT=0x4000,
	INGRESS_L4_SPORT_RANGE_BIT=0x8000,
	INGRESS_L4_DPORT_RANGE_BIT=0x10000,
	EGRESS_IPV4_SIP_RANGE_BIT=0x20000,	//not support in 0371 testchip
	EGRESS_IPV4_DIP_RANGE_BIT=0x40000,	//not support in 0371 testchip
	EGRESS_L4_SPORT_RANGE_BIT=0x80000,	//not support in 0371 testchip
	EGRESS_L4_DPORT_RANGE_BIT=0x100000,	//not support in 0371 testchip
	INGRESS_L4_ICMP_BIT=0x200000,	//not support in 0371 testchip,  IPv4 ICMP only
	EGRESS_CTAG_PRI_BIT=0x400000,
	EGRESS_CTAG_VID_BIT=0x800000,
	INGRESS_IPV6_DSCP_BIT=0x1000000,			//Only support while PPPoE Passthrought disabled. 
	INGRESS_STREAM_ID_BIT=0x2000000,
	INGRESS_STAG_PRI_BIT=0x4000000,
	INGRESS_STAG_VID_BIT=0x8000000,
	INGRESS_STAGIF_BIT=0x10000000,
	INGRESS_CTAGIF_BIT=0x20000000,
	INGRESS_EGRESS_PORTIDX_BIT=0x40000000,	//cf [2:0]uni pattern: ingress_port_idx for US, egress_port_idx for DS
	INTERNAL_PRI_BIT=0x80000000,	//cf [7:5]IntPri pattern
	INGRESS_L4_POROTCAL_VALUE_BIT=(1ULL<<32),
	INGRESS_TOS_BIT=(1ULL<<33),
	INGRESS_IPV6_TC_BIT=(1ULL<<34),
	INGRESS_IPV6_SIP_BIT =(1ULL<<35),
	INGRESS_IPV6_DIP_BIT =(1ULL<<36),
	INGRESS_WLANDEV_BIT =(1ULL<<37),	//only supported in fwdEngine, and limit ingress_port_mask is (1<<RTK_RG_EXT_PORT0)
	INGRESS_IPV4_TAGIF_BIT =(1ULL<<38),
	INGRESS_IPV6_TAGIF_BIT =(1ULL<<39),
	INGRESS_L4_ICMPV6_BIT = (1ULL<<40),	//IPv6 ICMPv6 only
	INGRESS_CTAG_CFI_BIT = (1ULL<<41),
	INGRESS_STAG_DEI_BIT = (1ULL<<42),
	EGRESS_IP4MC_IF =  (1ULL<<43),
	EGRESS_IP6MC_IF =  (1ULL<<44),
} rtk_rg_acl_filter_fields_t;


typedef enum rtk_rg_acl_fwding_type_direction_e{
	ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET=0,
	ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP,
	ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP,
	ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN,
	ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN,
	ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP, //apolloFE
	ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP, //apolloFE
	ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT, //apolloFE
	ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_PERMIT, //apolloFE
}rtk_rg_acl_fwding_type_direction_t;


typedef struct rtk_rg_cvlan_tag_action_s{
	rtk_rg_acl_cvlan_tagif_decision_t 	cvlanTagIfDecision;
	rtk_rg_acl_cvlan_cvid_decision_t	cvlanCvidDecision;
	rtk_rg_acl_cvlan_cpri_decision_t	cvlanCpriDecision;
	uint32 assignedCvid;
	uint8 assignedCpri;
}rtk_rg_cvlan_tag_action_t;

typedef struct rtk_rg_svlan_tag_action_s{
	rtk_rg_acl_svlan_tagif_decision_t 	svlanTagIfDecision;
	rtk_rg_acl_svlan_svid_decision_t	svlanSvidDecision;
	rtk_rg_acl_svlan_spri_decision_t	svlanSpriDecision;
	uint32 assignedSvid;
	uint8 assignedSpri;
}rtk_rg_svlan_tag_action_t;

typedef struct rtk_rg_aclFilterAndQos_s
{
	//unsigned int filter_fields;
	unsigned long long int filter_fields; // please refer to rtk_rg_acl_filter_fields_t typedef.
	rtk_rg_acl_fwding_type_direction_t fwding_type_and_direction;
	int acl_weight;	//the larger number, the higher priority.

	//pattern
	rtk_rg_portmask_t ingress_port_mask;
	int ingress_port_idx;	//only support in upstream type(CF uni pattern, fwding_type_and_direction is 1 or 3)
	int egress_port_idx;	//only support in downstream type(CF uni pattern, fwding_type_and_direction is 2 or 4); in L34 mode is CPU port

	int ingress_dscp;
	int ingress_tos;
	int ingress_ipv6_dscp;	//Only support while PPPoE Passthrought disabled. 
	int ingress_ipv6_tc;	//Only support while PPPoE Passthrought disabled.
	int ingress_intf_idx;
	int egress_intf_idx;
	int ingress_ethertype;
	int ingress_ctag_vid;
	int ingress_ctag_pri;
	int ingress_ctag_cfi;
	int ingress_stag_vid;
	int ingress_stag_pri;
	int ingress_stag_dei;
	int ingress_stream_id;
	int egress_ctag_vid;
	int egress_ctag_pri;
	int ingress_ctagIf;		//if  INGRESS_CTAGIF_BIT valid => 0:must not  have ctag   1:must have ctag
	int ingress_stagIf;		//if  INGRESS_STAGIF_BIT valid => 0:must not have stag   1:must have stag
	rtk_mac_t ingress_smac;
	rtk_mac_t ingress_dmac;
	ipaddr_t ingress_src_ipv4_addr_start;
	ipaddr_t ingress_src_ipv4_addr_end;
	ipaddr_t ingress_dest_ipv4_addr_start;
	ipaddr_t ingress_dest_ipv4_addr_end;
	uint8 ingress_src_ipv6_addr_start[16];
	uint8 ingress_src_ipv6_addr_end[16];
	uint8 ingress_dest_ipv6_addr_start[16];
	uint8 ingress_dest_ipv6_addr_end[16];
	unsigned short int ingress_src_l4_port_start;
	unsigned short int ingress_src_l4_port_end;
	unsigned short int ingress_dest_l4_port_start;
	unsigned short int ingress_dest_l4_port_end;
	ipaddr_t egress_src_ipv4_addr_start;
	ipaddr_t egress_src_ipv4_addr_end;
	ipaddr_t egress_dest_ipv4_addr_start;
	ipaddr_t egress_dest_ipv4_addr_end;
	unsigned short int egress_src_l4_port_start;
	unsigned short int egress_src_l4_port_end;
	unsigned short int egress_dest_l4_port_start;
	unsigned short int egress_dest_l4_port_end;
	int internal_pri;
	uint16 ingress_l4_protocal;
	uint8 ingress_src_ipv6_addr[16]; //using with ingress_src_ipv6_addr_mask
	uint8 ingress_dest_ipv6_addr[16]; //using with ingress_dest_ipv6_addr_mask
	uint32 ingress_wlanDevMask; //bit[0]:EXT0 root, bit[1~4]:EXT0 vap, bit[5~12]:EXT0 wps,  bit[13]:EXT1 root, bit[14~17]:EXT1 vap, bit[18~25]:EXT1 wps.
	uint16 ingress_ipv4_tagif;	//0:must not be ipv4,  1:must be ipv4, 
	uint16 ingress_ipv6_tagif;	//0:must not be ipv6,  1:must be ipv6,
	uint16 egress_ip4mc_if; //0:must not be ipv4 multicast, not include IGMP
	uint16 egress_ip6mc_if; //0:must not be ipv6 multicast, not include MLD

	//patter mask
	rtk_mac_t ingress_smac_mask;
	rtk_mac_t ingress_dmac_mask;
	int ingress_stream_id_mask;
	int ingress_ethertype_mask;
	//int egress_uni_mask;
	int ingress_port_idx_mask;	//only support in upstream type(CF uni pattern, fwding_type_and_direction is 1 or 3) , relate to INGRESS_EGRESS_PORTIDX_BIT
	int egress_port_idx_mask;	//only support in downstream type(CF uni pattern, fwding_type_and_direction is 2 or 4), relate to INGRESS_EGRESS_PORTIDX_BIT; in L34 mode is CPU port
	uint8 ingress_src_ipv6_addr_mask[16];
	uint8 ingress_dest_ipv6_addr_mask[16];
	uint16 egress_ctag_vid_mask; 


	//action
	rtk_rg_acl_action_type_t action_type;
	rtk_rg_acl_qos_action_t qos_actions; /* only used for action_type=ACL_ACTION_TYPE_QOS */

	unsigned char action_dot1p_remarking_pri;
	unsigned char action_ip_precedence_remarking_pri;
	unsigned char action_dscp_remarking_pri;
	unsigned char action_queue_id;
	unsigned char action_share_meter;
	unsigned char action_stream_id_or_llid;
	unsigned char action_acl_priority;
	rtk_rg_cvlan_tag_action_t action_acl_cvlan;
	rtk_rg_svlan_tag_action_t action_acl_svlan;
	unsigned char action_policy_route_wan;
	int action_acl_ingress_vid;
	uint32 downstream_uni_portmask;
	uint32 redirect_portmask;
	uint32 egress_internal_priority;
	
} rtk_rg_aclFilterAndQos_t;


#if defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_RTL9601B_SERIES) || defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)

typedef struct rtk_rg_aclFilterEntry_s
{
	uint32 hw_aclEntry_start;
	uint32 hw_aclEntry_size; 
	uint32 hw_cfEntry_start;
	uint32 hw_cfEntry_size;
	rtk_rg_aclSWEntry_used_tables_field_t hw_used_table;//record which range tables are used
	uint8 hw_used_table_index[USED_TABLE_END];//record  used range tables index
	rtk_rg_aclSWEntry_type_t type;
	rtk_rg_aclFilterAndQos_t acl_filter;
	rtk_rg_enable_t valid;
}rtk_rg_aclFilterEntry_t;

#elif defined(CONFIG_RTL9602C_SERIES)

typedef struct rtk_rg_aclFilterEntry_s
{
	uint32 hw_aclEntry_start;
	uint32 hw_aclEntry_size; 
	uint32 hw_cfEntry_start;
	uint32 hw_cfEntry_size;
	rtk_rg_aclSWEntry_used_tables_field_t hw_used_table;//record which range tables are used
	uint8 hw_used_table_index[USED_TABLE_END];//record  used range tables index
	rtk_rg_aclSWEntry_type_t type;
	rtk_rg_aclFilterAndQos_t acl_filter;
	rtk_rg_enable_t valid;
}rtk_rg_aclFilterEntry_t;

#endif

typedef enum rtk_rg_napt_filter_direction_check_e
{
	CHECK_OUTBOUND_PKT_WITH_OUTBOUND_RULE = 0,
	CHECK_OUTBOUND_PKT_WITH_INBOUND_RULE,
	CHECK_INBOUND_PKT_WITH_OUTBOUND_RULE,
	CHECK_INBOUND_PKT_WITH_INBOUND_RULE,
	CHECK_BOUND_PKT_WITH_BOUND_RULE_END,
}rtk_rg_napt_filter_direction_check_t;

typedef enum rtk_rg_napt_filter_fields_e
{
	INGRESS_SIP=(1<<0),
	EGRESS_SIP=(1<<1),
	INGRESS_DIP=(1<<2),
	EGRESS_DIP=(1<<3),
	INGRESS_SPORT=(1<<4),
	EGRESS_SPORT=(1<<5),
	INGRESS_DPORT=(1<<6),
	EGRESS_DPORT=(1<<7),
	L4_PROTOCAL=(1<<8),
	INGRESS_SIP_RANGE=(1<<9),
	INGRESS_DIP_RANGE=(1<<10),
	INGRESS_SPORT_RANGE=(1<<11),
	INGRESS_DPORT_RANGE=(1<<12),
	EGRESS_SIP_RANGE=(1<<13),
	EGRESS_DIP_RANGE=(1<<14),
	EGRESS_SPORT_RANGE=(1<<15),
	EGRESS_DPORT_RANGE=(1<<16),
}rtk_rg_napt_filter_fields_t;

typedef enum rtk_rg_napt_action_fields_e
{
	ASSIGN_NAPT_PRIORITY_BIT=(1<<0),
	NAPT_DROP_BIT=(1<<1),
	NAPT_PERMIT_BIT=(1<<2),
}rtk_rg_napt_action_fields_t;

typedef enum rtk_rg_napt_fwding_direction_e{
	RTK_RG_NAPT_FILTER_OUTBOUND=0,
	RTK_RG_NAPT_FILTER_INBOUND,
	RTK_RG_NAPT_FILTER_DIRECTION_END,
}rtk_rg_napt_fwding_direction_t;

typedef enum rtk_rg_napt_fwding_ruleType_e{
	RTK_RG_NAPT_FILTER_PERSIST=0,
	RTK_RG_NAPT_FILTER_ONE_SHOT,
}rtk_rg_napt_fwding_ruleType_t;


typedef struct rtk_rg_naptFilterAndQos_s
{
	rtk_rg_napt_fwding_direction_t direction;
	//compare patterns
	uint32 weight;
	rtk_rg_napt_filter_fields_t filter_fields;
	ipaddr_t ingress_src_ipv4_addr;
	ipaddr_t egress_src_ipv4_addr;
	ipaddr_t ingress_dest_ipv4_addr;
	ipaddr_t egress_dest_ipv4_addr;
	unsigned short int ingress_src_l4_port;
	unsigned short int egress_src_l4_port;
	unsigned short int ingress_dest_l4_port;
	unsigned short int egress_dest_l4_port;
	uint32 ingress_l4_protocal;
	
	ipaddr_t ingress_src_ipv4_addr_range_start;
	ipaddr_t ingress_src_ipv4_addr_range_end;
	ipaddr_t ingress_dest_ipv4_addr_range_start;
	ipaddr_t ingress_dest_ipv4_addr_range_end;
	unsigned short int ingress_src_l4_port_range_start;
	unsigned short int ingress_src_l4_port_range_end;
	unsigned short int ingress_dest_l4_port_range_start;
	unsigned short int ingress_dest_l4_port_range_end;

	ipaddr_t egress_src_ipv4_addr_range_start;
	ipaddr_t egress_src_ipv4_addr_range_end;
	ipaddr_t egress_dest_ipv4_addr_range_start;
	ipaddr_t egress_dest_ipv4_addr_range_end;
	unsigned short int egress_src_l4_port_range_start;
	unsigned short int egress_src_l4_port_range_end;
	unsigned short int egress_dest_l4_port_range_start;
	unsigned short int egress_dest_l4_port_range_end;

	rtk_rg_napt_action_fields_t action_fields;
	//assigned priority
	uint32 assign_priority;

	rtk_rg_napt_fwding_ruleType_t ruleType; //0:persist, 1:one shot
	
}rtk_rg_naptFilterAndQos_t;

typedef struct rtk_rg_sw_naptFilterAndQos_s
{
	uint32 valid;
	uint32 sw_index;
	rtk_rg_naptFilterAndQos_t naptFilter;
	struct rtk_rg_sw_naptFilterAndQos_s *pNextValid;
}rtk_rg_sw_naptFilterAndQos_t;


#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
typedef enum rtk_rg_acl_transform_type_e
{
	RG_ACL_TRANS_NONE=0,
	RG_ACL_TRANS_v6_OTHER,
	RG_ACL_TRANS_v4_OTHER,
	RG_ACL_TRANS_v6_SUBNET,
	RG_ACL_TRANS_v4_SUBNET,
	RG_ACL_TRANS_v4_OTHER_v6_OTHER,		//5
	RG_ACL_TRANS_v4_SUBNET_v6_OTHER,
	RG_ACL_TRANS_v4_OTHER_v6_SUBNET,
	RG_ACL_TRANS_v4_SUBNET_v6_SUBNET,
	RG_ACL_TRANS_L2_PORT_BIND,
	RG_ACL_TRANS_L2_VLAN_BIND,			//10
	RG_ACL_TRANS_L34_PORT_BIND,
	RG_ACL_TRANS_L34_VLAN_BIND,
}rtk_rg_acl_transform_type_t;
#endif

//L2 classify

typedef enum rtk_rg_cf_direction_type_e
{
	RTK_RG_CLASSIFY_DIRECTION_UPSTREAM,
	RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM,	
	RTK_RG_CLASSIFY_DIRECTION_END,
} rtk_rg_cf_direction_type_t;

typedef enum rtk_rg_cf_filter_fields_e
{
	EGRESS_ETHERTYPR_BIT=(1<<0),
	EGRESS_GEMIDX_BIT=(1<<1),
	EGRESS_LLID_BIT=(1<<2),
	EGRESS_TAGVID_BIT=(1<<3),
	EGRESS_TAGPRI_BIT=(1<<4),
	EGRESS_INTERNALPRI_BIT=(1<<5),
	EGRESS_STAGIF_BIT=(1<<6),
	EGRESS_CTAGIF_BIT=(1<<7),
	EGRESS_UNI_BIT=(1<<8),
} rtk_rg_cf_filter_fields_t;

typedef enum rtk_rg_cf_us_action_type_e
{
	CF_US_ACTION_STAG_BIT = (1ULL<<0),
	CF_US_ACTION_CTAG_BIT = (1ULL<<1),
	CF_US_ACTION_CFPRI_BIT=(1ULL<<2),
	CF_US_ACTION_DSCP_BIT=(1ULL<<3),//DSCP:apollo 9600Series have side effect!
	CF_US_ACTION_SID_BIT=(1ULL<<4),
	CF_US_ACTION_FWD_BIT=(1ULL<<5),//apolloFE change drop action to fwd action
	CF_US_ACTION_DROP_BIT=(1ULL<<6),
	CF_US_ACTION_LOG_BIT=(1ULL<<7),//LOG:apollo 9600Series have side effect!
	CF_US_ACTION_END_BIT=(1ULL<<8),
} rtk_rg_cf_us_action_type_t;


typedef enum rtk_rg_cf_ds_action_type_e
{
	CF_DS_ACTION_STAG_BIT = (1ULL<<0),
	CF_DS_ACTION_CTAG_BIT = (1ULL<<1),
	CF_DS_ACTION_CFPRI_BIT=(1ULL<<2),
	CF_DS_ACTION_DSCP_BIT=(1ULL<<3),//DSCP:apollo 9600Series have side effect!
	CF_DS_ACTION_UNI_MASK_BIT=(1ULL<<4),
	CF_DS_ACTION_DROP_BIT=(1ULL<<5),//DROP: using uni to achieve this action!
	CF_DS_ACTION_END_BIT=(1ULL<<6),
} rtk_rg_cf_ds_action_type_t;



typedef enum rtk_rg_cf_pattern_type_e{
	CF_PATTERN_ETHERTYPE=0,
	CF_PATTERN_GEMIDX_OR_LLID,
	CF_PATTERN_OUTTERTAGVID,
	CF_PATTERN_OUTTERTAGPRI,
	CF_PATTERN_INTERNALPRI,
	CF_PATTERN_STAGFLAG,
	CF_PATTERN_CTAGFLAG,
	CF_PATTERN_UNI,
	CF_PATTERN_END,
}rtk_rg_cf_pattern_type_t;


typedef struct rtk_rg_classifyEntry_s
{
	uint32 index; //limit in 64-511,  0:invalid   else:should be the asic index
	rtk_rg_cf_direction_type_t direction;

	//patterns
	rtk_rg_cf_filter_fields_t filter_fields;
	
	uint32 etherType;
	uint32 gemidx;
	uint32 llid;
	uint32 outterTagVid;
	uint32 outterTagPri;
	uint32 internalPri;
	uint32 stagIf;
	uint32 ctagIf;
	uint32 uni;

	//pattern mask
	uint32 etherType_mask;
	uint32 gemidx_mask;
	uint32 uni_mask;
	
	//US actions mask
	rtk_rg_cf_us_action_type_t us_action_field;
	//DS actions  mask
	rtk_rg_cf_ds_action_type_t ds_action_field;
	
	rtk_rg_cvlan_tag_action_t action_cvlan;
	rtk_rg_svlan_tag_action_t action_svlan;
	rtk_rg_cfpri_action_t	action_cfpri;
	rtk_rg_sid_llid_action_t action_sid_or_llid;
	rtk_rg_dscp_action_t action_dscp;
	rtk_rg_fwd_action_t action_fwd;
	rtk_rg_log_action_t action_log;
	rtk_rg_uni_action_t action_uni;
}rtk_rg_classifyEntry_t;

//Force Portal
typedef struct rtk_rg_forcePortalURL_s
{
	unsigned char url_string[MAX_URL_FILTER_STR_LENGTH];
	unsigned int valid:1;
	unsigned int attach_orig_url:1;		//0: not attached, 1: attache original URL after the url_string
}rtk_rg_forcePortalURL_t;

//URL Filter
typedef struct rtk_rg_urlFilterString_s
{
	unsigned char url_filter_string[MAX_URL_FILTER_STR_LENGTH];
	unsigned char path_filter_string[MAX_URL_FILTER_PATH_LENGTH];
	int path_exactly_match;
	int wan_intf;
} rtk_rg_urlFilterString_t;

//UPnP
typedef enum rtk_rg_upnp_type_e
{
	UPNP_TYPE_ONESHOT=0,
	UPNP_TYPE_PERSIST=1
} rtk_rg_upnp_type_t;

typedef struct rtk_rg_upnpConnection_s
{
	int is_tcp;
	int valid;
	int wan_intf_idx; //for gateway ip
	int gateway_port;
	ipaddr_t local_ip;
	unsigned short int local_port;
	int limit_remote_ip;
	int limit_remote_port;
	ipaddr_t remote_ip;
	unsigned short int remote_port;
	rtk_rg_upnp_type_t type; //one shot?
	int timeout; //auto-delete after timeout, 0:disable auto-delete
	int idle;	//idle time
} rtk_rg_upnpConnection_t;

//IPv6 Stateful routing
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
typedef struct rtk_rg_ipv6_layer4_linkList_s
{
	//5 tuple
	rtk_ipv6_addr_t		srcIP;
	rtk_ipv6_addr_t		destIP;
	uint16				srcPort;
	uint16				destPort;
	uint8				isTCP;		//0:UDP, 1:TCP

	//additional info for IPv6 NAPT
	rtk_ipv6_addr_t		internalIP;		//internal IP: supported by ipv6 NAPTR
	uint16				externalPort;	//l4 external port: supported by ipv6 NAPT
	uint16				internalPort;	//l4 internal port: supported by ipv6 NAPTR
	uint8				extipIdx;	//supported by ipv6 napt

	rtk_rg_naptState_t 	state;
	struct rtk_rg_ipv6_layer4_linkList_s *pPair_list;
	uint32 				idleSecs;

	uint8 				netifIdx;
	uint16 				dmacL2Idx;
#if defined(CONFIG_RTL9600_SERIES)		
#else	//support lut traffic bit
	int smacL2Idx; //for updating idle time
#endif
#if defined(CONFIG_RTL9602C_SERIES)
	//mib counter and cf decision
	int mibNetifIdx; //for update mib counter
	rtk_rg_cf_direction_type_t mibDirect;
	char mibTagDelta;	//add or minus for NIC offload vlan tag
#endif
	rtk_l34_nexthop_type_t	wanType;
	rtk_rg_naptDirection_t	direction;
#ifdef CONFIG_GPON_FEATURE	
	uint8 streamID;	//for GPON WAN
#endif
	uint8 priority;
	uint8 servicePriority;
	uint16 internalVlanID;	//internal vlan for filtering
	uint8 internalCFPri;	//internal priority after CF module
	int16 dscp;
	uint16 vlanID;
	uint8 vlanTagif;
	uint16 serviceVlanID;
	uint8 serviceVlanTagif;
	int16 dmac2cvlanID;		//-1 means not apply
	uint8 dmac2cvlanTagif;
	int8 macPort;
	int8 extPort;	//hw extention port
	int neighborIdx;	//for updating idle time, -1 for non-neighbor
	uint32 uniPortmask;
	uint8 tagAccType;

	//fragment data
	uint8 isFrag;
	uint32 fragAction;
	unsigned long beginIdleTime;
	uint16 receivedLength;	//already received packet length
	uint16 totalLength;	//calculated by last fragment
	uint8 queueCount;

	uint8 valid;
	//indicate that this shortcut entry is not ready
	uint8 notFinishUpdated;
	
	struct list_head layer4_list;
}rtk_rg_ipv6_layer4_linkList_t;
#endif


/* Init */
typedef int (*p_initByHwCallBack)(void);
typedef int (*p_arpAddByHwCallBack)(rtk_rg_arpInfo_t*);
typedef int (*p_arpDelByHwCallBack)(rtk_rg_arpInfo_t*);
typedef int (*p_macAddByHwCallBack)(rtk_rg_macEntry_t*);
typedef int (*p_macDelByHwCallBack)(rtk_rg_macEntry_t*);
typedef int (*p_routingAddByHwCallBack)(rtk_rg_ipv4RoutingEntry_t*);
typedef int (*p_routingDelByHwCallBack)(rtk_rg_ipv4RoutingEntry_t*);
typedef int (*p_naptAddByHwCallBack)(rtk_rg_naptInfo_t*);
typedef int (*p_naptDelByHwCallBack)(rtk_rg_naptInfo_t*);
typedef int (*P_bindAddByHwCallBack)(rtk_rg_bindingEntry_t*);
typedef int (*P_bindDelByHwCallBack)(rtk_rg_bindingEntry_t*);
typedef int (*p_interfaceAddByHwCallBack)(rtk_rg_intfInfo_t*,int*);
typedef int (*p_interfaceDelByHwCallBack)(rtk_rg_intfInfo_t*,int*);
typedef int (*p_neighborAddByHwCallBack)(rtk_rg_neighborInfo_t*);
typedef int (*p_neighborDelByHwCallBack)(rtk_rg_neighborInfo_t*);
typedef int (*p_v6RoutingAddByHwCallBack)(rtk_rg_ipv6RoutingEntry_t*);
typedef int (*p_v6RoutingDelByHwCallBack)(rtk_rg_ipv6RoutingEntry_t*);
typedef int (*p_pppoeBeforeDiagByHwCallBack)(rtk_rg_pppoeClientInfoBeforeDial_t*,int*);
typedef int (*p_dhcpRequestByHwCallBack)(int*);
typedef int (*p_naptInboundConnLookupFirstCallBack)(void*,ipaddr_t*,uint16*);
typedef int (*p_ipv6NaptInboundConnLookupFirstCallBack)(void*,rtk_ipv6_addr_t*,uint16*);
typedef int (*p_naptInboundConnLookupSecondCallBack)(void*,ipaddr_t*,uint16*);
typedef int (*p_ipv6NaptInboundConnLookupSecondCallBack)(void*,rtk_ipv6_addr_t*,uint16*);
typedef int (*p_naptInboundConnLookupThirdCallBack)(void*,ipaddr_t*,uint16*);
typedef int (*p_ipv6NaptInboundConnLookupThirdCallBack)(void*,rtk_ipv6_addr_t*,uint16*);
typedef int (*p_pptpBeforeDialByHwCallBack)(rtk_rg_pptpClientInfoBeforeDial_t*,int*);
typedef int (*p_l2tpBeforeDialByHwCallBack)(rtk_rg_l2tpClientInfoBeforeDial_t*,int*);
typedef int (*p_pppoeDsliteBeforeDialByHwCallBack)(rtk_rg_pppoeClientInfoBeforeDial_t*,int*);
typedef int (*p_softwareNaptInfoAddCallBack)(rtk_rg_naptInfo_t*);
typedef int (*p_softwareNaptInfoDeleteCallBack)(rtk_rg_naptInfo_t*);
typedef int (*p_naptPreRouteDPICallBack)(void*,rtk_rg_naptDirection_t);
typedef int (*p_naptForwardDPICallBack)(void*,rtk_rg_naptDirection_t);

#ifdef __KERNEL__
typedef struct rtk_rg_wq_initByHwCallBack_s
{
	struct work_struct work;
}rtk_rg_wq_initByHwCallBack_t;

typedef struct rtk_rg_wq_arpAddByHwCallBack_s
{
	rtk_rg_arpInfo_t arpInfo;
	struct work_struct work;
}rtk_rg_wq_arpAddByHwCallBack_t;

typedef struct rtk_rg_wq_arpDelByHwCallBack_s
{
	rtk_rg_arpInfo_t arpInfo;
	struct work_struct work;
}rtk_rg_wq_arpDelByHwCallBack_t;

typedef struct rtk_rg_wq_macAddByHwCallBack_s
{
	rtk_rg_macEntry_t macInfo;
	struct work_struct work;
}rtk_rg_wq_macAddByHwCallBack_t;

typedef struct rtk_rg_wq_macDelByHwCallBack_s
{
	rtk_rg_macEntry_t macInfo;
	struct work_struct work;
}rtk_rg_wq_macDelByHwCallBack_t;

typedef struct rtk_rg_wq_routingAddByHwCallBack_s
{
	rtk_rg_ipv4RoutingEntry_t v4RoutingInfo;
	struct work_struct work;
}rtk_rg_wq_routingAddByHwCallBack_t;

typedef struct rtk_rg_wq_routingDelByHwCallBack_s
{
	rtk_rg_ipv4RoutingEntry_t v4RoutingInfo;
	struct work_struct work;
}rtk_rg_wq_routingDelByHwCallBack_t;

typedef struct rtk_rg_wq_naptAddByHwCallBack_s
{
	rtk_rg_naptInfo_t naptInfo;
	struct work_struct work;
}rtk_rg_wq_naptAddByHwCallBack_t;

typedef struct rtk_rg_wq_naptDelByHwCallBack_s
{
	rtk_rg_naptInfo_t naptInfo;
	struct work_struct work;
}rtk_rg_wq_naptDelByHwCallBack_t;

typedef struct rtk_rg_wq_bindingAddByHwCallBack_s
{
	rtk_rg_bindingEntry_t bindInfo;
	struct work_struct work;
}rtk_rg_wq_bindingAddByHwCallBack_t;

typedef struct rtk_rg_wq_bindingDelByHwCallBack_s
{
	rtk_rg_bindingEntry_t bindInfo;
	struct work_struct work;
}rtk_rg_wq_bindingDelByHwCallBack_t;

typedef struct rtk_rg_wq_interfaceAddByHwCallBack_s
{
	rtk_rg_intfInfo_t intfInfo;
	int intfIdx;
	struct work_struct work;
}rtk_rg_wq_interfaceAddByHwCallBack_t;

typedef struct rtk_rg_wq_interfaceDelByHwCallBack_s
{
	rtk_rg_intfInfo_t intfInfo;
	int intfIdx;
	struct work_struct work;
}rtk_rg_wq_interfaceDelByHwCallBack_t;

typedef struct rtk_rg_wq_neighborAddByHwCallBack_s
{
	rtk_rg_neighborInfo_t neighborInfo;
	struct work_struct work;
}rtk_rg_wq_neighborAddByHwCallBack_t;

typedef struct rtk_rg_wq_neighborDelByHwCallBack_s
{
	rtk_rg_neighborInfo_t neighborInfo;
	struct work_struct work;
}rtk_rg_wq_neighborDelByHwCallBack_t;

typedef struct rtk_rg_wq_v6RoutingAddByHwCallBack_s
{
	rtk_rg_ipv6RoutingEntry_t v6RoutingInfo;
	struct work_struct work;
}rtk_rg_wq_v6RoutingAddByHwCallBack_t;

typedef struct rtk_rg_wq_v6RoutingDelByHwCallBack_s
{
	rtk_rg_ipv6RoutingEntry_t v6RoutingInfo;
	struct work_struct work;
}rtk_rg_wq_v6RoutingDelByHwCallBack_t;

typedef struct rtk_rg_wq_pppoeBeforeDiagByHwCallBack_s
{
	rtk_rg_pppoeClientInfoBeforeDial_t pppoeBeforeInfo;
	int wanIdx;
	struct work_struct work;
}rtk_rg_wq_pppoeBeforeDiagByHwCallBack_t;

typedef struct rtk_rg_wq_dhcpRequestByHwCallBack_s
{
	int wanIdx;
	struct work_struct work;
}rtk_rg_wq_dhcpRequestByHwCallBack_t;

typedef struct rtk_rg_wq_pptpBeforeDialByHwCallBack_s
{
	rtk_rg_pptpClientInfoBeforeDial_t pptpBeforeInfo;
	int wanIdx;
	struct work_struct work;
}rtk_rg_wq_pptpBeforeDialByHwCallBack_t;

typedef struct rtk_rg_wq_l2tpBeforeDialByHwCallBack_s
{
	rtk_rg_l2tpClientInfoBeforeDial_t l2tpBeforeInfo;
	int wanIdx;
	struct work_struct work;
}rtk_rg_wq_l2tpBeforeDialByHwCallBack_t;

typedef struct rtk_rg_wq_pppoeDsliteBeforeDialByHwCallBack_s
{
	rtk_rg_pppoeClientInfoBeforeDial_t pppoeDsliteBeforeInfo;
	int wanIdx;
	struct work_struct work;
}rtk_rg_wq_pppoeDsliteBeforeDialByHwCallBack_t;

#if 0
typedef struct rtk_rg_wq_softwarNaptAddCallBack_s
{
	rtk_rg_naptInfo_t naptInfo;
	struct work_struct work;
}rtk_rg_wq_softwareNaptAddCallBack_t;

typedef struct rtk_rg_wq_softwareNaptDelCallBack_s
{
	rtk_rg_naptInfo_t naptInfo;
	struct work_struct work;
}rtk_rg_wq_softwareNaptDelCallBack_t;
#endif
#endif

typedef struct rtk_rg_initParams_s
{
	uint32 igmpSnoopingEnable:1;
	uint32 macBasedTagDecision:1;		//control DMAC2CVID per-port and forced state registers
	uint32 wanPortGponMode:1;		//control wan port is GPON mode or EPON/UTP mode
	uint32 ivlMulticastSupport:1;	//control l2 multicast support IVL mode(hash by VLAN)

	uint32 fwdVLAN_CPU;
	uint32 fwdVLAN_CPU_SVLAN;
	uint32 fwdVLAN_Proto_Block;		//used for IP_version only_mode, as PVID to block traffic to other port
	uint32 fwdVLAN_BIND_INTERNET;	//used for LAN to transparent with internet WAN
	uint32 fwdVLAN_BIND_OTHER;		//used for other WAN separate with un-binding ports or internet WAN
	
	p_initByHwCallBack initByHwCallBack;
	p_arpAddByHwCallBack arpAddByHwCallBack;
	p_arpDelByHwCallBack arpDelByHwCallBack;
	p_macAddByHwCallBack macAddByHwCallBack;
	p_macDelByHwCallBack macDelByHwCallBack;
	p_routingAddByHwCallBack routingAddByHwCallBack;
	p_routingDelByHwCallBack routingDelByHwCallBack;
	p_naptAddByHwCallBack naptAddByHwCallBack;
	p_naptDelByHwCallBack naptDelByHwCallBack;
	P_bindAddByHwCallBack bindingAddByHwCallBack;
	P_bindDelByHwCallBack bindingDelByHwCallBack;
	p_interfaceAddByHwCallBack interfaceAddByHwCallBack;
	p_interfaceDelByHwCallBack interfaceDelByHwCallBack;
	p_neighborAddByHwCallBack neighborAddByHwCallBack;
	p_neighborDelByHwCallBack neighborDelByHwCallBack;
	p_v6RoutingAddByHwCallBack v6RoutingAddByHwCallBack;
	p_v6RoutingDelByHwCallBack v6RoutingDelByHwCallBack;
	p_pppoeBeforeDiagByHwCallBack pppoeBeforeDiagByHwCallBack;
	p_naptInboundConnLookupFirstCallBack naptInboundConnLookupFirstCallBack;
	p_naptInboundConnLookupSecondCallBack naptInboundConnLookupSecondCallBack;
	p_naptInboundConnLookupThirdCallBack naptInboundConnLookupThirdCallBack;
	p_dhcpRequestByHwCallBack dhcpRequestByHwCallBack;
	p_pptpBeforeDialByHwCallBack pptpBeforeDialByHwCallBack;
	p_l2tpBeforeDialByHwCallBack l2tpBeforeDialByHwCallBack;
	p_pppoeDsliteBeforeDialByHwCallBack pppoeDsliteBeforeDialByHwCallBack;

	p_ipv6NaptInboundConnLookupFirstCallBack ipv6NaptInboundConnLookupFirstCallBack;
	p_ipv6NaptInboundConnLookupSecondCallBack ipv6NaptInboundConnLookupSecondCallBack;
	p_ipv6NaptInboundConnLookupThirdCallBack ipv6NaptInboundConnLookupThirdCallBack;

	p_softwareNaptInfoAddCallBack softwareNaptInfoAddCallBack;
	p_softwareNaptInfoDeleteCallBack softwareNaptInfoDeleteCallBack;
	p_naptPreRouteDPICallBack naptPreRouteDPICallBack;
	p_naptForwardDPICallBack naptForwardDPICallBack;
} rtk_rg_initParams_t;


/* internal */

typedef enum rtk_rg_flow_direction_s
{
	FLOW_DIRECTION_CAN_NOT_DECIDE=0,
	FLOW_DIRECTION_UPSTREAM, 	//LAN_TO_WAN
	FLOW_DIRECTION_DOWNSTREAM, //WAN_TO_LAN
	FLOW_DIRECTION_LAN_TO_LAN,
	FLOW_DIRECTION_WAN_TO_WAN,
}rtk_rg_flow_direction_t;


/*URL Module*/
typedef struct rtk_rg_urlFilterEntry_s
{
	int valid;
	rtk_rg_urlFilterString_t urlFilter;
}rtk_rg_urlFilterEntry_t;

typedef enum rtk_rg_filterControlType_e
{
	RG_FILTER_BLACK=0,
	RG_FILTER_WHITE,
}rtk_rg_filterControlType_t;

/*LUT Module*/
typedef enum rtk_rg_macfilter_interface_e
{
	INTERFACE_FOR_LAN=0,
	INTERFACE_FOR_WAN,
	INTERFACE_END
}rtk_rg_macfilter_interface_t;

typedef enum rtk_rg_macFilterEntry_direction_s
{
	RTK_RG_MACFILTER_FILTER_SRC_DEST_MAC_BOTH,
	RTK_RG_MACFILTER_FILTER_SRC_MAC_ONLY,
	RTK_RG_MACFILTER_FILTER_DEST_MAC_ONLY,
	RTK_RG_MACFILTER_FILTER_END
} rtk_rg_macFilterEntry_direction_t;

typedef struct rtk_rg_macFilterEntry_s
{
	rtk_mac_t mac;
	int isIVL; //0:SVL, 1:IVL
	int vlan_id;
	rtk_rg_macFilterEntry_direction_t direct;
}rtk_rg_macFilterEntry_t;

typedef struct rtk_rg_macFilterSWEntry_s
{
	int valid;
	int l2_table_entry_index;
	int l2_table_entry_index_for_lan;
	int l2_table_entry_index_for_wan;
	rtk_rg_macFilterEntry_t macFilterEntry;
}rtk_rg_macFilterSWEntry_t;

/* MIB counter */
typedef struct rtk_rg_port_mib_info_s
{
    uint64 ifInOctets;
    uint32 ifInUcastPkts;
    uint32 ifInMulticastPkts;
    uint32 ifInBroadcastPkts;
    uint32 ifInDiscards;
    uint64 ifOutOctets;
    uint32 ifOutDiscards;
    uint32 ifOutUcastPkts;
    uint32 ifOutMulticastPkts;
    uint32 ifOutBrocastPkts;
    uint32 dot1dBasePortDelayExceededDiscards;
    uint32 dot1dTpPortInDiscards;
    uint32 dot1dTpHcPortInDiscards;
    uint32 dot3InPauseFrames;
    uint32 dot3OutPauseFrames;
    //uint32 dot3OutPauseOnFrames;
    uint32 dot3StatsAligmentErrors;
    uint32 dot3StatsFCSErrors;
    uint32 dot3StatsSingleCollisionFrames;
    uint32 dot3StatsMultipleCollisionFrames;
    uint32 dot3StatsDeferredTransmissions;
    uint32 dot3StatsLateCollisions;
    uint32 dot3StatsExcessiveCollisions;
    uint32 dot3StatsFrameTooLongs;
    uint32 dot3StatsSymbolErrors;
    uint32 dot3ControlInUnknownOpcodes;
    uint32 etherStatsDropEvents;
    uint64 etherStatsOctets;
    uint32 etherStatsBcastPkts;
    uint32 etherStatsMcastPkts;
    uint32 etherStatsUndersizePkts;
    uint32 etherStatsOversizePkts;
    uint32 etherStatsFragments;
    uint32 etherStatsJabbers;
    uint32 etherStatsCollisions;
    uint32 etherStatsCRCAlignErrors;
    uint32 etherStatsPkts64Octets;
    uint32 etherStatsPkts65to127Octets;
    uint32 etherStatsPkts128to255Octets;
    uint32 etherStatsPkts256to511Octets;
    uint32 etherStatsPkts512to1023Octets;
    uint32 etherStatsPkts1024to1518Octets;
    uint64 etherStatsTxOctets;
    uint32 etherStatsTxUndersizePkts;
    uint32 etherStatsTxOversizePkts;
    uint32 etherStatsTxPkts64Octets;
    uint32 etherStatsTxPkts65to127Octets;
    uint32 etherStatsTxPkts128to255Octets;
    uint32 etherStatsTxPkts256to511Octets;
    uint32 etherStatsTxPkts512to1023Octets;
    uint32 etherStatsTxPkts1024to1518Octets;
    uint32 etherStatsTxPkts1519toMaxOctets;
    uint32 etherStatsTxBcastPkts;
    uint32 etherStatsTxMcastPkts;
    uint32 etherStatsTxFragments;
    uint32 etherStatsTxJabbers;
    uint32 etherStatsTxCRCAlignErrors;
    uint32 etherStatsRxUndersizePkts;
    uint32 etherStatsRxUndersizeDropPkts;
    uint32 etherStatsRxOversizePkts;
    uint32 etherStatsRxPkts64Octets;
    uint32 etherStatsRxPkts65to127Octets;
    uint32 etherStatsRxPkts128to255Octets;
    uint32 etherStatsRxPkts256to511Octets;
    uint32 etherStatsRxPkts512to1023Octets;
    uint32 etherStatsRxPkts1024to1518Octets;
    uint32 etherStatsRxPkts1519toMaxOctets;
    uint32 inOampduPkts;
    uint32 outOampduPkts;
}rtk_rg_port_mib_info_t;

/* Priority to queue mapping */
typedef struct rtk_rg_qos_pri2queue_s
{
	 uint32 pri2queue[RTK_MAX_NUM_OF_PRIORITY];
}rtk_rg_qos_pri2queue_t;

/* Weight of each priority source */
typedef struct rtk_rg_qos_priSelWeight_s
{
    uint32 weight_of_portBased;
    uint32 weight_of_dot1q;
    uint32 weight_of_dscp;
    uint32 weight_of_acl;
    uint32 weight_of_lutFwd;
    uint32 weight_of_saBaed;
    uint32 weight_of_vlanBased;
    uint32 weight_of_svlanBased;
    uint32 weight_of_l4Based;
}rtk_rg_qos_priSelWeight_t;

/* Types of DSCP remarking source */
typedef enum rtk_rg_qos_dscpRmkSrc_e
{
    RTK_RG_DSCP_RMK_SRC_INT_PRI,
    RTK_RG_DSCP_RMK_SRC_DSCP,
    RTK_RG_DSCP_RMK_SRC_END
} rtk_rg_qos_dscpRmkSrc_t;

/* DOS Port Security*/
typedef enum rtk_rg_dos_type_e
{
    RTK_RG_DOS_DAEQSA_DENY = 0,
    RTK_RG_DOS_LAND_DENY,
    RTK_RG_DOS_BLAT_DENY,
    RTK_RG_DOS_SYNFIN_DENY,
    RTK_RG_DOS_XMA_DENY,
    RTK_RG_DOS_NULLSCAN_DENY,
    RTK_RG_DOS_SYN_SPORTL1024_DENY,
    RTK_RG_DOS_TCPHDR_MIN_CHECK,
    RTK_RG_DOS_TCP_FRAG_OFF_MIN_CHECK,
    RTK_RG_DOS_ICMP_FRAG_PKTS_DENY,
    RTK_RG_DOS_POD_DENY,
    RTK_RG_DOS_UDPDOMB_DENY,
    RTK_RG_DOS_SYNWITHDATA_DENY,
    RTK_RG_DOS_SYNFLOOD_DENY,
    RTK_RG_DOS_FINFLOOD_DENY,
    RTK_RG_DOS_ICMPFLOOD_DENY,
    RTK_RG_DOS_TYPE_MAX
}rtk_rg_dos_type_t;

typedef enum rtk_rg_dos_action_e
{
    RTK_RG_DOS_ACTION_DROP = 0,
    RTK_RG_DOS_ACTION_TRAP,
}rtk_rg_dos_action_t;



/* End of RTK RG API ====================================================== */

/* NIC ==================================================================== */
typedef struct rtk_rg_rxdesc_s{
	union{
		struct{
			uint32 own:1;//31
			uint32 eor:1;//30
			uint32 fs:1;//29
			uint32 ls:1;//28
			uint32 crcerr:1;//27
			uint32 ipv4csf:1;//26
			uint32 l4csf:1;//25
			uint32 rcdf:1;//24
			uint32 ipfrag:1;//23
			uint32 pppoetag:1;//22
			uint32 rwt:1;//21
			uint32 pkttype:4;//17~20
			uint32 l3routing:1;//16
			uint32 origformat:1;//15
			uint32 pctrl:1;//14
		#if defined(CONFIG_RTL9602C_SERIES)
			uint32 issb:1;//13
			uint32 rsvd:1;//12
		#else
			uint32 rsvd:2;//12~13
		#endif
			uint32 data_length:12;//0~11
		}bit;
		uint32 dw;//double word
	}opts1;
	uint32 addr;
	union{
		struct{
			uint32 cputag:1;//31
			uint32 ptp_in_cpu_tag_exist:1;//30
			uint32 svlan_tag_exist:1;//29
			uint32 rsvd_2:2;//27~28 ----> used for software: 0:Hit ACL trap to Protocol-Stack rule, 1:Other
			uint32 pon_stream_id:7;//20~26
			uint32 rsvd_1:3;//17~19
			uint32 ctagva:1;//16
			uint32 cvlan_tag:16;//0~15
		}bit;
		uint32 dw;//double word
	}opts2;
	union{
		struct{
#if defined(CONFIG_RTL9602C_SERIES)
			uint32 src_port_num:4;//28~31
			uint32 dst_port_mask:7;//21~27
#else
			uint32 src_port_num:5;//27~31			
			uint32 dst_port_mask:6;//21~26
#endif			
			uint32 reason:8;//13~20
			uint32 internal_priority:3;//10~12
			uint32 ext_port_ttl_1:5;//5~9
			uint32 rsvd:5;//0~4
		}bit;
		uint32 dw;//double word
	}opts3;
}rtk_rg_rxdesc_t;

typedef struct rtk_rg_txdesc_s{
	union{
		struct{
			uint32 own:1;//31
			uint32 eor:1;//30
			uint32 fs:1;//29
			uint32 ls:1;//28
			uint32 ipcs:1;//27
			uint32 l4cs:1;//26
			uint32 keep:1;//25
			uint32 blu:1;//24
			uint32 crc:1;//23
			uint32 vsel:1;//22
			uint32 dislrn:1;//21
			uint32 cputag_ipcs:1;//20
			uint32 cputag_l4cs:1;//19
			uint32 cputag_psel:1;//18
			uint32 rsvd:1;//17
			uint32 data_length:17;//0~16
		}bit;
		uint32 dw;//double word
	}opts1;
	uint32 addr;
	union{
		struct{
			uint32 cputag:1;//31
			uint32 aspri:1;//30
			uint32 cputag_pri:3;//27~29
			uint32 tx_vlan_action:2;//25~26
			uint32 tx_pppoe_action:2;//23~24
			uint32 tx_pppoe_idx:3;//20~22
			uint32 efid:1;//19
			uint32 enhance_fid:3;//16~18
			uint32 vidl:8;//8~15
			uint32 prio:3;//5~7
			uint32 cfi:1;// 4
			uint32 vidh:4;//0~3
		}bit;
		uint32 dw;//double word
	}opts2;
	union{
		struct{
			uint32 extspa:3;//29~31
			uint32 tx_portmask:6;//23~28
			uint32 tx_dst_stream_id:7;//16~22
#if defined(CONFIG_RTL9602C_SERIES)
			uint32 rsvd:12;// 4~15
			uint32 rsv1:1;// 3
			uint32 rsv0:1;// 2
			uint32 l34_keep:1;// 1
			uint32 ptp:1;//0
#else
			uint32 reserved:14; // 2~15
			uint32 l34_keep:1;// 1
			uint32 PTP:1;//0
#endif			
		}bit;
		uint32 dw;//double word
	}opts3;
	union{
		uint32 dw;
	}opts4;
}rtk_rg_txdesc_t;
/* End of NIC ============================================================= */


/* Forward Engine ========================================================= */
typedef enum rtk_rg_fwdEngineReturn_e
{

	//No Free skb [return state]
	RG_FWDENGINE_RET_DIRECT_TX			=0x0,
	RG_FWDENGINE_RET_UN_INIT			,
	RG_FWDENGINE_RET_QUEUE_FRAG			,
	RG_FWDENGINE_RET_FRAGMENT_ONE		,
	RG_FWDENGINE_RET_FRAGMENT			,
	RG_FWDENGINE_RET_NAPT_OK			,	//5
	RG_FWDENGINE_RET_L2FORWARDED		,
	RG_FWDENGINE_RET_CONTINUE			,
	RG_FWDENGINE_RET_BROADCAST			,
	RG_FWDENGINE_RET_FRAG_ONE_PS		,
	RG_FWDENGINE_RET_FRAG_ONE_DROP		,	//10
	RG_FWDENGINE_RET_HWLOOKUP			,
	RG_FWDENGINE_RET_SEND_TO_WIFI		,
	RG_FWDENGINE_RET_HIT_BINDING		,
	RG_FWDENGINE_RET_DIRECT_TX_SLOW		,
	RG_FWDENGINE_RET_ERROR				,	//15
	RG_FWDENGINE_RET_ROUTING_TRAP		,
	RG_FWDENGINE_RET_ROUTING_DROP		,
	RG_FWDENGINE_RET_SLOWPATH           ,
	RG_FWDENGINE_RET_RATE_LIMIT_DROP	,

	//To PS (Free by PS) [return Action]
	RG_FWDENGINE_RET_TO_PS				, // Do ACL action in fwdEngine_rx_skb 
	RG_FWDENGINE_RET_ACL_TO_PS			, // Don't do ACL action in fwdEngine_rx_skb

	//Drop [return Action]
	RG_FWDENGINE_RET_DROP				, // Free skb in fwdEngine_rx_skb
}rtk_rg_fwdEngineReturn_t;

typedef enum rtk_rg_fwdEngineAclAccTypeReturn_e
{
	RG_FWDENGINE_ACL_ACC_TYPE_TX_PKTBUFF		=0x0, //more efficiency
	RG_FWDENGINE_ACL_ACC_TYPE_TX_DESC			=0x1,
}rtk_rg_fwdEngineAclAccTypeReturn_t;

typedef enum rtk_rg_fwdEngineAlgReturn_e
{
	RG_FWDENGINE_ALG_RET_FAIL		=-1,
	RG_FWDENGINE_ALG_RET_SUCCESS	=0,
	RG_FWDENGINE_ALG_RET_DROP		=1,
}rtk_rg_fwdEngineAlgReturn_t;

typedef enum rtk_rg_successFailReturn_e
{
	RG_RET_FAIL		=-1,
	RG_RET_SUCCESS	=0,

}rtk_rg_successFailReturn_t;

typedef enum rtk_rg_naptPreRouteCallBackReturn_e
{
	RG_FWDENGINE_PREROUTECB_CONTINUE	=0,
	RG_FWDENGINE_PREROUTECB_DROP		=1,
	RG_FWDENGINE_PREROUTECB_TRAP		=2,
	RG_FWDENGINE_PREROUTECB_END			=3,
}rtk_rg_naptPreRouteCallBackReturn_t;

typedef enum rtk_rg_naptForwardCallBackReturn_e
{
	RG_FWDENGINE_FORWARDCB_FINISH_DPI	=0,		//the flow no need check anymore, add to hw
	RG_FWDENGINE_FORWARDCB_CONTINUE_DPI	=1,		//the flow has to be checked next time
	RG_FWDENGINE_FORWARDCB_DROP			=2,
	RG_FWDENGINE_FORWARDCB_END			=3,
}rtk_rg_naptForwardCallBackReturn_t;

typedef enum rtk_rg_lookupIdxReturn_e
{
	RG_RET_LOOKUPIDX_NOT_FOUND		=-1,
	RG_RET_LOOKUPIDX_ONLY_INBOUND_FOUND  =-2,
}rtk_rg_lookupIdxReturn_t;

typedef enum rtk_rg_extPortGetReturn_e
{
	RG_RET_EXTPORT_NOT_GET		=-1,
}rtk_rg_extPortGetReturn_t;

typedef enum rtk_rg_entryGetReturn_e
{
	RG_RET_ENTRY_NOT_GET		=-1,
}rtk_rg_entryGetReturn_t;

typedef enum rtk_rg_mbssidDev_e
{	
	RG_RET_MBSSID_NOT_FOUND		=-1,
	RG_RET_MBSSID_MASTER_ROOT_INTF	=0,
	RG_RET_MBSSID_MASTER_VAP0_INTF	=1,
	RG_RET_MBSSID_MASTER_VAP1_INTF	=2,
	RG_RET_MBSSID_MASTER_VAP2_INTF	=3,
	RG_RET_MBSSID_MASTER_VAP3_INTF	=4,
	RG_RET_MBSSID_MASTER_WDS0_INTF	=5,
	RG_RET_MBSSID_MASTER_WDS1_INTF	=6,
	RG_RET_MBSSID_MASTER_WDS2_INTF	=7,
	RG_RET_MBSSID_MASTER_WDS3_INTF	=8,
	RG_RET_MBSSID_MASTER_WDS4_INTF	=9,
	RG_RET_MBSSID_MASTER_WDS5_INTF	=10,
	RG_RET_MBSSID_MASTER_WDS6_INTF	=11,
	RG_RET_MBSSID_MASTER_WDS7_INTF	=12,
	RG_RET_MBSSID_MASTER_CLIENT_INTF =13,
	RG_RET_MBSSID_SLAVE_ROOT_INTF	=WLAN_DEVICE_NUM,
	RG_RET_MBSSID_SLAVE_VAP0_INTF	=WLAN_DEVICE_NUM+1,
	RG_RET_MBSSID_SLAVE_VAP1_INTF	=WLAN_DEVICE_NUM+2,
	RG_RET_MBSSID_SLAVE_VAP2_INTF	=WLAN_DEVICE_NUM+3,
	RG_RET_MBSSID_SLAVE_VAP3_INTF	=WLAN_DEVICE_NUM+4,
	RG_RET_MBSSID_SLAVE_WDS0_INTF	=WLAN_DEVICE_NUM+5,
	RG_RET_MBSSID_SLAVE_WDS1_INTF	=WLAN_DEVICE_NUM+6,
	RG_RET_MBSSID_SLAVE_WDS2_INTF	=WLAN_DEVICE_NUM+7,
	RG_RET_MBSSID_SLAVE_WDS3_INTF	=WLAN_DEVICE_NUM+8,
	RG_RET_MBSSID_SLAVE_WDS4_INTF	=WLAN_DEVICE_NUM+9,
	RG_RET_MBSSID_SLAVE_WDS5_INTF	=WLAN_DEVICE_NUM+10,
	RG_RET_MBSSID_SLAVE_WDS6_INTF	=WLAN_DEVICE_NUM+11,
	RG_RET_MBSSID_SLAVE_WDS7_INTF	=WLAN_DEVICE_NUM+12,
	RG_RET_MBSSID_SLAVE_CLIENT_INTF =WLAN_DEVICE_NUM+13,
	RG_RET_MBSSID_FLOOD_ALL_INTF	=100,
	
		
}rtk_rg_mbssidDev_t;




#if 0
/* Protocol */
struct	rg_proto_ether_header {
	unsigned char	ether_dhost[6];
	unsigned char	ether_shost[6];
	unsigned short int	ether_type;
};

struct rg_proto_iphdr {
	unsigned char	ihl_protocol;
	unsigned char	tos;
	unsigned short int	tot_len;
	unsigned short int	id;
	unsigned short int	frag_off;
	unsigned char	ttl;
	unsigned char	protocol;
	unsigned short int	check;
	unsigned int	saddr;
	unsigned int	daddr;
};

struct rg_proto_tcphdr {
	unsigned short int	source;
	unsigned short int	dest;
	unsigned int	seq;
	unsigned int	ack_seq;
	unsigned char	doff;
	unsigned char	flag;
	unsigned short int	window;
	unsigned short int	check;
	unsigned short int	urg_ptr;
};

struct rg_proto_udphdr {
	unsigned short int	source;
	unsigned short int	dest;
	unsigned short int	len;
	unsigned short int	check;
};
#endif

/* End of Forward Engine ================================================== */



/* Parser ================================================================= */
typedef enum rtk_rg_pkthdr_tagif_e
{
	SVLAN_TAGIF=(1<<0),
	CVLAN_TAGIF=(1<<1),
	PPPOE_TAGIF=(1<<2),
	IPV4_TAGIF=(1<<3),
	IPV6_TAGIF=(1<<4),
	TCP_TAGIF=(1<<5),
	UDP_TAGIF=(1<<6),
	IGMP_TAGIF=(1<<7),
	PPTP_TAGIF=(1<<8),
	GRE_TAGIF=(1<<9),
	ICMPV6_TAGIF=(1<<10),
	ICMP_TAGIF=(1<<11),
	ESP_TAGIF=(1<<12),  /*siyuan add for alg IPsec passthrough*/
	MSS_TAGIF=(1<<13),
	ARP_TAGIF=(1<<14),
	V6FRAG_TAGIF=(1<<15),
	IPV6_MLD_TAGIF=(1<<16),
	GRE_SEQ_TAGIF=(1<<17),
	GRE_ACK_TAGIF=(1<<18),
	PPTP_INNER_TAGIF=(1<<19),
	L2TP_INNER_TAGIF=(1<<20),
	L2TP_TAGIF=(1<<21),
	DSLITE_TAGIF=(1<<22),
	DSLITE_INNER_TAGIF=(1<<23),
	V6TRAP_TAGIF=(1<<24),
	DVMRP_TAGIF=(1<<25),
	MOSPF_TAGIF=(1<<26),
	PIM_TAGIF=(1<<27),
	DSLITEMC_INNER_TAGIF=(1<<28),
} rtk_rg_pkthdr_tagif_t;


typedef struct rtk_rg_tcpFlags_s{

#ifdef _LITTLE_ENDIAN

	uint8	fin:1;
	uint8	syn:1;
	uint8	reset:1;
	uint8	push:1;
	uint8	ack:1;
	uint8	urg:1;
	uint8	reserved:2;
	
#else
	uint8	reserved:2;
	uint8	urg:1;
	uint8	ack:1;
	uint8	push:1;
	uint8	reset:1;
	uint8	syn:1;
	uint8	fin:1;

#endif	
}rtk_rg_tcpFlags_t;





#if defined(CONFIG_RTL9602C_SERIES)

typedef enum rtk_rg_igrAction_controlBit_e
{
	RG_IGR_SVLAN_ACT_DONE_BIT = 	(1<<0),
	RG_IGR_CVLAN_ACT_DONE_BIT = 	(1<<1),
	RG_IGR_PRIORITY_ACT_DONE_BIT = 	(1<<2),
	RG_IGR_POLICE_ACT_DONE_BIT = 	(1<<3),
	RG_IGR_FORWARD_ACT_DONE_BIT = 	(1<<4),
	RG_IGR_INTCF_ACT_DONE_BIT = 	(1<<5), 
	RG_IGR_PROUTE_ACT_DONE_BIT = 	(1<<6), 
}rtk_rg_igrAction_controlBit_t;

typedef enum rtk_rg_egrAction_controlBit_e
{
	RG_EGR_SVLAN_ACT_DONE_BIT = (1<<0),
	RG_EGR_CVLAN_ACT_DONE_BIT = (1<<1),
	RG_EGR_CFPRI_ACT_DONE_BIT = (1<<2),
	RG_EGR_DSCP_ACT_DONE_BIT = (1<<3),
	RG_EGR_FWD_ACT_DONE_BIT = (1<<4), //UNI act or Drop act
	RG_EGR_SID_ACT_DONE_BIT = (1<<5), 
}rtk_rg_egrAction_controlBit_t;

typedef struct rtk_rg_aclHitAndAction_s
{
	uint32 aclIgrHitMask[(MAX_ACL_SW_ENTRY_SIZE/32)+1]; //check RG_ACL  ingressPart hit or not:  1ULL & ruleNo is 0: not hit  1: hit
	uint32 aclEgrHitMask[(MAX_ACL_SW_ENTRY_SIZE/32)+1]; //check RG_ACL  egressPart hit or not:   1ULL & ruleNo is 0: not hit  1: hit
	uint32 aclEgrPattern1HitMask[(TOTAL_CF_ENTRY_SIZE/32)+1]; //check CF pattern1 hit or not:  1ULL & ruleNo is 0: not hit  1: hit   PS:aclEgrL2HitMask[0] is reserved now
	int aclEgrHaveToCheckRuleIdx[MAX_ACL_SW_ENTRY_SIZE];//record ingress part hit rule index, for speeding up egress module pattern check. (-1 means invalid index, and rest array doen't need to check)


	int aclIgrRuleChecked; //record the  _rtk_rg_ingressACLPatternCheck has called! 0:not called before.  1:it has called, it cab be pass if called twice.


	//***********[PER PACKET CLEAR DECISION PART]*********//
	uint8 ACL_DECISION_EGR_PART_CLEAR; //[NOTE]used to clean acl egress module decision for each BC/MC data path. each decision need to clrear should be put later then this parameter!!!

	int aclIgrDoneAction; //refer to rtk_rg_igrAction_controlBit_t
	int aclEgrDoneAction; //refert to rtk_rg_egrAction_controlBit_t. 

	/*RG ACL final decided actions*/
	rtk_rg_acl_action_type_t action_type;
	rtk_rg_acl_qos_action_t qos_actions; /* only used for action_type=ACL_ACTION_TYPE_QOS */

	unsigned char action_dot1p_remarking_pri;
	unsigned char action_ip_precedence_remarking_pri;
	unsigned char action_dscp_remarking_pri;
	unsigned char action_queue_id;
	unsigned char action_share_meter;
	unsigned char action_stream_id_or_llid;
	unsigned char action_acl_priority;
	unsigned char action_redirect_portmask;
	unsigned char  action_acl_egress_internal_priority;
	rtk_rg_cvlan_tag_action_t action_acl_cvlan;
	rtk_rg_svlan_tag_action_t action_acl_svlan;


	/*CF64~511 final decided action*/
	rtk_rg_cf_direction_type_t direction;
	//US actions
	rtk_rg_cf_us_action_type_t us_action_field;
	//DS actions
	rtk_rg_cf_ds_action_type_t ds_action_field;
	
	rtk_rg_cvlan_tag_action_t action_cvlan;
	rtk_rg_svlan_tag_action_t action_svlan;
	rtk_rg_cfpri_action_t	action_cfpri;
	rtk_rg_sid_llid_action_t action_sid_or_llid;
	rtk_rg_dscp_action_t action_dscp;
	rtk_rg_uni_action_t action_uni;
	rtk_rg_fwd_action_t action_fwd;

}rtk_rg_aclHitAndAction_t;



#else

typedef enum rtk_rg_egrAction_e
{
	EGR_CSACT_DONE = 0,
	EGR_CACT_DONE ,
	EGR_CFPRI_ACT_DONE ,
	EGR_DSCP_REMARK_DONE ,
	EGR_UNI_ACT_DONE ,
	EGR_SID_ACT_DONE ,
	EGR_DROP_ACT_DONE ,
	EGR_LOG_ACT_DONE 
}rtk_rg_egrAction_t;

typedef struct rtk_rg_aclHitAndAction_s
{
	uint32 aclIgrHitMask[(MAX_ACL_SW_ENTRY_SIZE/32)+1]; //check RG_ACL 0~63 ingressPart hit or not:  1ULL & ruleNo is 0: not hit  1: hit
	uint32 aclEgrHitMask[(MAX_ACL_SW_ENTRY_SIZE/32)+1]; //check RG_ACL 0~63 egressPart hit or not:   1ULL & ruleNo is 0: not hit  1: hit
	uint32 aclEgrL2HitMask[(TOTAL_CF_ENTRY_SIZE/32)+1]; //check RG_CF 64~511 hit or not:  1ULL & ruleNo is 0: not hit  1: hit   PS:aclEgrL2HitMask[0] is reserved now
	int aclEgrHaveToCheckRuleIdx[MAX_ACL_SW_ENTRY_SIZE];//record which ingress part hit rule index, for speeding up egress pattern check. (-1 means invalid index, and rest array doen't need to check)


	int aclIgrRuleChecked; //record the  _rtk_rg_ingressACLPatternCheck has called! 0:not called before.  1:it has called, it cab be pass if called twice.
	int l34CFRuleHit; //0: none CF:0-63  rule hit.  1: at least one CF:0-63 rule hit
	uint32 l2CFHitAction;	//for CF[64-511] double hit check, forbidden the action to do again when related bit is valid. refer to struct rtk_rg_egrAction_t. 


	/*ACL + CF0~63 final decided action*/
	rtk_rg_acl_action_type_t action_type;
	rtk_rg_acl_qos_action_t qos_actions; /* only used for action_type=ACL_ACTION_TYPE_QOS */

	unsigned char action_dot1p_remarking_pri;
	unsigned char action_ip_precedence_remarking_pri;
	unsigned char action_dscp_remarking_pri;
	unsigned char action_queue_id;
	unsigned char action_share_meter;
	unsigned char action_stream_id_or_llid;
	unsigned char action_acl_priority;
	unsigned char action_redirect_portmask;
	unsigned char  action_acl_egress_internal_priority;
	rtk_rg_cvlan_tag_action_t action_acl_cvlan;
	rtk_rg_svlan_tag_action_t action_acl_svlan;


	/*CF64~511 final decided action*/
	int cf64to511RuleHit; //0:none cf64-511 rule hit.   1:any cf64-511 rule hit.  using for speed up need to do egressACL or not.
	rtk_rg_cf_direction_type_t direction;
	//US actions
	rtk_rg_cf_us_action_type_t us_action_field;
	//DS actions
	rtk_rg_cf_ds_action_type_t ds_action_field;
	
	rtk_rg_cvlan_tag_action_t action_cvlan;
	rtk_rg_svlan_tag_action_t action_svlan;
	rtk_rg_cfpri_action_t	action_cfpri;
	rtk_rg_sid_llid_action_t action_sid_or_llid;
	rtk_rg_dscp_action_t action_dscp;
	rtk_rg_log_action_t action_log;
	rtk_rg_uni_action_t action_uni;

}rtk_rg_aclHitAndAction_t;
#endif

typedef enum rtk_rg_cfDirection_e
{
	CF_DOWNSTREAM,
	CF_UPSTREAM,
	CF_LAN_TO_LAN,
}rtk_rg_cfDirection_t;

typedef enum rtk_rg_algAction_e
{
	RG_ALG_ACT_NORMAL,				//non-ALG packets
	RG_ALG_ACT_TO_FWDENGINE,		//ALG handled by fwdEngine packets
}rtk_rg_algAction_t;

typedef enum rtk_rg_l4Direction_e
{
	RG_L4_NOT_NAPT_FLOW=0, //non-layer4 flow packets
	RG_NAPT_OUTBOUND_FLOW,		
	RG_NAPTR_INBOUND_FLOW,
}rtk_rg_l4Direction_t;

typedef enum rtk_rg_ingressLocation_e
{
	RG_IGR_PHY_PORT=0,
	RG_IGR_PROTOCOL_STACK,
	RG_IGR_ARP_OR_ND,
	RG_IGR_IGMP_OR_MLD,
}rtk_rg_ingressLocation_t;

typedef enum rtk_rg_fwdDecision_e
{
	RG_FWD_DECISION_INITIAL=0,
	RG_FWD_DECISION_BRIDGING,
	RG_FWD_DECISION_ROUTING,
	RG_FWD_DECISION_V6ROUTING,
	RG_FWD_DECISION_NAPT,
	RG_FWD_DECISION_NAPTR,
	RG_FWD_DECISION_NORMAL_BC,		//broadcast or multicast
	RG_FWD_DECISION_NO_PS_BC,		//from protocol stack bc or mc, unknown DA, etc., don't go to protocol stack again
	RG_FWD_DECISION_TO_PS,			//after DA lookup, the destination port is CPU port 
	RG_FWD_DECISION_V6NAPT,
	RG_FWD_DECISION_V6NAPTR,
	RG_FWD_DECISION_PORT_ISO,		//stop because of port isolation
	RG_FWD_DECISION_PPPOE_MC,		//pppoe multicast, change DA to multicast DA and remove pppoe tag
}rtk_rg_fwdDecision_t;

#if defined(CONFIG_ROME_NAPT_SHORTCUT) || defined(CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT)
typedef enum rtk_rg_shortcutDecision_e
{
	RG_SC_NORMAL_PATH=0,
	RG_SC_MATCH,
	RG_SC_NEED_UPDATE,
	RG_SC_NEED_UPDATE_BEFORE_SEND,
	RG_SC_V6_NEED_UPDATE_BEFORE_SEND,
	RG_SC_STATEFUL_NEED_UPDATE_BEFORE_SEND,
}rtk_rg_shortcutDecision_t;
#endif

typedef enum rtk_rg_bindingDecision_e
{
	RG_BINDING_NOT_FINISHED,
	RG_BINDING_FINISHED,
	RG_BINDING_LAYER2,		//layer2 will do hw lookup, therefore this case is not necessary
	RG_BINDING_LAYER3,
	RG_BINDING_LAYER4,
}rtk_rg_bindingDecision_t;

#ifdef CONFIG_ROME_NAPT_SHORTCUT
typedef struct rtk_rg_napt_shortcut_s
{
	uint16	sport;
	uint16	dport;	
	ipaddr_t sip;
	ipaddr_t dip;

    uint32 spa:SC_BFW_SPA; // 4
    uint32 direction:SC_BFW_DIR; // 3
    uint32 isTcp:SC_BFW_ISTCP;  // 1
    uint32 isNapt:SC_BFW_ISNAPT; //L3 or L4 //1
    uint32 isBridge:SC_BFW_ISBRIDGE;    //for L2 //1
    uint32 isHairpinNat:SC_BFW_ISHAIRPINNAT; // 1
    uint32 notFinishUpdated:SC_BFW_NOTFINUPDATE; //indicate that this shortcut entry is not ready // 1
    uint32 vlanTagif:SC_BFW_VLANTAGIF; // 1
    uint32 serviceVlanTagif:SC_BFW_SVLANTAGIF; // 2
    uint32 dmac2cvlanTagif:SC_BFW_DMAC2CVLANTAGIF; // 1
	uint32 internalCFPri:SC_BFW_INTERCFPRI;	//internal priority after CF module //3
	uint32 serviceVlanID:SC_BFW_SVLANID; // 13

    uint32 internalVlanID:SC_BFW_INTERVLANID;   //internal vlan for filtering //13
    uint32 vlanID:SC_BFW_VLANID; // 13
    int32 dscp:SC_BFW_DSCP; // 6

    int32 dmac2cvlanID:SC_BFW_DMAC2CVLANID;     //-1 means not apply // 13
    uint32 tagAccType:SC_BFW_TAGACCTYPE; // 1
	uint32 priority:SC_BFW_PRIORITY;	//cvlan priority //3
	uint32 servicePriority:SC_BFW_SPRIORITY;	//svlan priority // 3
#ifdef CONFIG_GPON_FEATURE	
    uint32 streamID:SC_BFW_STREAMID;   //for GPON WAN
    uint32 reserve0:5; // padding to 32 bit
#else
    uint32 reserve0:12; // padding to 32 bit
#endif


#if defined(CONFIG_RTL9602C_SERIES)

    uint32 mibDirect:SC_BFW_MIBDIRECT;// 2
    int32 mibNetifIdx:SC_BFW_MIBIFIDX;    //for update mib counter // 5
    int32 mibTagDelta:SC_BFW_MIBTAGDELTA;   //add or minus for NIC offload vlan tag //4 
    int32 new_lut_idx:SC_BFW_LUTIDX;  //for da    // 12
    int32 new_intf_idx:SC_BFW_INTFIDX; //for sa // 5
    int32 new_eip_idx:SC_BFW_EIPIDX; //for outbound sip // 4
    
    int32 arpIdx:SC_BFW_ARPIDX;   //for updating idle time, -1 for non-arp // 11
    int32 smacL2Idx:SC_BFW_SMACL2IDX; //for updating idle time // 13
    uint32 uniPortmask:SC_BFW_UNIPORTMASK; // 8

    int32 naptIdx:SC_BFW_NAPTIDX; //for inbound dip,dport (naptInIdx), for outbound sport // 14
    uint32 idleSecs:SC_BFW_IDLETIME; //16
	uint32 reserve1:2; // padding to 32 bit

#elif defined(CONFIG_RTL9600_SERIES)

    int32 new_eip_idx:SC_BFW_EIPIDX; //for outbound sip // 4
    int32 naptIdx:SC_BFW_NAPTIDX; //for inbound dip,dport (naptInIdx), for outbound sport // 16
    int32 new_intf_idx:SC_BFW_INTFIDX; //for sa // 4
    uint32 uniPortmask:SC_BFW_UNIPORTMASK; // 8

    int32 new_lut_idx:SC_BFW_LUTIDX;   //for da    // 13
    int32 arpIdx:SC_BFW_ARPIDX;    //for updating idle time, -1 for non-arp // 11
    uint32 reserve1:8; // padding to 32 bit

	uint32 idleSecs:SC_BFW_IDLETIME; //16
	uint32 reserve2:16; // padding to 32 bit

#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)

    //for routing, if egress interface MTU is smaller than ingress interface MTU,
    //the packet should be fragmented if needed!
    //uint8 egressHasSmallerMTU;
    uint16 naptIdx; //for inbound dip,dport (naptInIdx), for outbound sport
    int smacL2Idx; //for updating idle time
    //action
    int16 new_lut_idx;  //for da    
    int8 new_intf_idx; //for sa
    int8 new_eip_idx; //for outbound sip
    int16 arpIdx;   //for updating idle time, -1 for non-arp
    uint32 uniPortmask;
#if 0

   rtk_rg_port_idx_t spa;
   rtk_rg_naptDirection_t  direction;

   uint8 isTcp:1; 

   uint8 isNapt:1; //L3 or L4
   uint8 isBridge:1;   //for L2
   uint8 isHairpinNat:1;
   uint8 notFinishUpdated:1; //indicate that this shortcut entry is not ready

   uint16 naptIdx; //for inbound dip,dport (naptInIdx), for outbound sport
#ifdef CONFIG_GPON_FEATURE	
   uint8 streamID; //for GPON WAN
#endif
   uint8 priority; //cvlan priority
   uint8 servicePriority;  //svlan priority
   uint8 internalCFPri;    //internal priority after CF module
   uint8 vlanTagif;
   uint8 serviceVlanTagif;
   uint8 dmac2cvlanTagif;
   uint8 tagAccType;

   uint16 internalVlanID;  //internal vlan for filtering
   int16 dscp;
   uint16 vlanID;
   uint16 serviceVlanID;
   int16 dmac2cvlanID;     //-1 means not apply
   //int8 macPort;
   //int8 extPort;
#endif
#else
#err
#endif
	
} rtk_rg_napt_shortcut_t;
#endif

#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
typedef struct rtk_rg_naptv6_shortcut_s
{
	uint16	sport;
	uint16	dport;	
	rtk_ipv6_addr_t	sip;
	rtk_ipv6_addr_t	dip;

    uint32 spa:SC_BFW_SPA; // 4
    uint32 macPort:SC_BFW_MACPORT; // 4
    uint32 extPort:SC_BFW_EXTPORT; // 4
    uint32 servicePriority:SC_BFW_SPRIORITY;    //svlan priority // 3
    uint32 internalCFPri:SC_BFW_INTERCFPRI;  //internal priority after CF module // 3
    uint32 isTcp:SC_BFW_ISTCP; // 1
    uint32 vlanID:SC_BFW_VLANID; // 13


    uint32 isBridge:SC_BFW_ISBRIDGE;   //for L2 //1
    uint32 internalVlanID:SC_BFW_INTERVLANID;   //internal vlan for filtering // 1
    uint32 serviceVlanID:SC_BFW_SVLANID; // 13
    uint32 priority:SC_BFW_PRIORITY;   //cvlan priority // 3
    int32 neighborIdx:SC_BFW_NEIGHBORIDX;   //for updating idle time, -1 for non-neighbor   // 8
    int32 dscp:SC_BFW_DSCP; // 6

    uint32 notFinishUpdated:SC_BFW_NOTFINUPDATE;   //indicate that this shortcut entry is not ready // 1
    uint32 tagAccType:SC_BFW_TAGACCTYPE; // 1
    uint32 vlanTagif:SC_BFW_VLANTAGIF; // 1
    uint32 serviceVlanTagif:SC_BFW_SVLANTAGIF; // 2
    uint32 dmac2cvlanTagif:SC_BFW_DMAC2CVLANTAGIF; // 1
    int32 dmac2cvlanID:SC_BFW_DMAC2CVLANID;      //-1 means not apply // 13

#ifdef CONFIG_GPON_FEATURE	
	uint32 streamID:SC_BFW_STREAMID;	//for GPON WAN //7
	uint32 reserve0:6;
#else
    uint32 reserve0:13;
#endif

#if defined(CONFIG_RTL9602C_SERIES)

	uint32 mibDirect:SC_BFW_MIBDIRECT; // 2
    uint32 uniPortmask:SC_BFW_UNIPORTMASK; //8
    int32 mibNetifIdx:SC_BFW_MIBIFIDX;    //for update mib counter // 5
    int32 mibTagDelta:SC_BFW_MIBTAGDELTA;    //add or minus for NIC offload vlan tag // 4
	int32 smacL2Idx:SC_BFW_SMACL2IDX; //for updating idle time //13 

    int32 new_lut_idx:SC_BFW_LUTIDX;  //for da  // 12  
    int32 new_intf_idx:SC_BFW_INTFIDX; //for sa // 5
    uint32 reserve1:15; // padding to 32 bit

	uint32 idleSecs:SC_BFW_IDLETIME; //16
	uint32 reserve2:16; // padding to 32 bit

#elif defined(CONFIG_RTL9600_SERIES)
    int32 new_intf_idx:SC_BFW_INTFIDX; //for sa // 4
    int32 new_lut_idx:SC_BFW_LUTIDX;  //for da    //13
    uint32 uniPortmask:SC_BFW_UNIPORTMASK; // 8
    uint32 reserve1:7; // padding to 32 bit

	uint32 idleSecs:SC_BFW_IDLETIME; //16
	uint32 reserve2:16; // padding to 32 bit

#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#if 0
  	rtk_rg_port_idx_t spa;
	uint8 isTcp:1;
	uint8 isBridge:1;	//for L2
	uint8 notFinishUpdated:1;	//indicate that this shortcut entry is not ready
	
#ifdef CONFIG_GPON_FEATURE	
	uint8 streamID;	//for GPON WAN
#endif
	uint8 priority;	//cvlan priority
	uint8 servicePriority;	//svlan priority
	uint8 internalCFPri;	//internal priority after CF module
	uint8 vlanTagif;
	uint8 serviceVlanTagif;
	uint8 dmac2cvlanTagif;
	int8 macPort;
	int8 extPort;
	uint8 tagAccType;
	uint16 internalVlanID;	//internal vlan for filtering
	int16 dscp;
	uint16 vlanID;
	uint16 serviceVlanID;
	int16 dmac2cvlanID;		//-1 means not apply
	
	int neighborIdx;	//for updating idle time, -1 for non-neighbor	
	int smacL2Idx; //for updating idle time
#endif
	uint32 uniPortmask;
	int8 new_intf_idx; //for sa
	int16 new_lut_idx;	//for da	

#else
#err
#endif
} rtk_rg_naptv6_shortcut_t;
#endif

typedef struct rtk_rg_pktHdr_s
{
	struct sk_buff *skb;
	
	/* NIC RX Priv */
	struct re_private *cp;
	
	/* NIC RX Desc */
	rtk_rg_rxdesc_t	*pRxDesc;

	/* Tag Info */
	rtk_rg_pkthdr_tagif_t	tagif;
	rtk_rg_pkthdr_tagif_t	egressTagif;
	uint8 *pDmac;
#if defined(CONFIG_RTL9602C_SERIES)
	//mib counter and cf decision
	uint8  dmac[ETHER_ADDR_LEN];	//used for mib counter update
#endif
	uint8 *pSmac;

	/* Packet Parsing Info */
	// STAG
	uint8 	*pSVlanTag;
	uint8	stagPri;
	uint8	stagDei;
	uint16	stagVid;	
	uint16	stagTpid;

	// CTAG	
	uint8 	*pCVlanTag;
	uint8	ctagPri;
	uint8	ctagCfi;
	uint16	ctagVid;	
	
	uint16 	etherType;

	// etherType=PPPoE(0x8863/0x8864)
	uint16 sessionId;
	uint16 *pPppoeLength;
	uint16 pppProtocal; //IPv4:0x0021 or 0xC021, IPv6:0x0057 or 0xC057
	
	// etherType=IPv4(0x0800),ARP(0806)
	uint8	ipv4HeaderLen;
	uint16 	l3Offset;
	uint8 	tos;
	uint8 	*pTos;
	uint16 	l3Len;
	uint16 	*pL3Len;
	uint32 	ipv4Sip;
	uint32 	ipv4Dip;
	uint32 	*pIpv4Sip;
	uint32 	*pIpv4Dip;
	uint16 	ipv4Checksum;
	uint16 	*pIpv4Checksum;
	uint8 	ipv4FragPacket; //ipv4MoreFragment==1 or ipv4FragmentOffset!=0
	uint8 	ipv4DontFragment;
	uint8 	ipv4MoreFragment;
	uint16 	ipv4FragmentOffset;
	uint16 	*pIpv4Identification;
	uint8 	ipv4TTL;
	uint8 	*pIpv4TTL;

	// etherType=IPv6(0x086dd)
	uint16 ipv6PayloadLen;
	uint8 *pIPv6HopLimit;
	uint8 *pIpv6Sip;
	uint8 *pIpv6Dip;
	uint8 ipv6FragPacket; //ipv6MoreFragment==1 or ipv6FragmentOffset!=0
	uint8 ipv6MoreFragment;
	uint16 ipv6FragmentOffset;
	uint16 ipv6FragId_First;
	uint16 ipv6FragId_Second;
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	//20141208LUKE: keep the fragment list pointer after _rtk_rg_ipv6StatefulDecision
	rtk_rg_ipv6_layer4_linkList_t	*pIPv6FragmentList; 	//used for update information after send fragment_one
#endif	

	//ipv4/ipv6
	uint8 ipProtocol;
	uint16 l4Offset;

	//ARP
	uint16	arpOpCode;
	
	// ipProtocol=TCP/UDP
	uint16 sport;
	uint16 dport;
	uint16 *pSport;
	uint16 *pDport;
	uint16 l4Checksum;	
	uint16 *pL4Checksum;

	//TCP
	rtk_rg_tcpFlags_t tcpFlags;
	uint32	tcpSeq;
	uint32	tcpAck;
	uint32	*pTcpSeq;
	uint32	*pTcpAck;
	uint8 	*pL4Payload;
	uint16	*pMssLength;
	uint8	headerLen;
	uint16 	tcpWindow;
	//HTTP(GET,POST) for URL Filter
	int		httpFirstPacket;

	//UDP 
	//siyuan add for alg function which may change the udp data length
	uint16 * pL4Len;
	uint16 l4Len;

	//PPTP decision
	uint16 pptpCtrlType;
	rtk_rg_pptpCallIds_t *pPptpCallId;
	rtk_rg_pptpCodes_t pptpCodes;
	uint16 *pGRECallID;		//receiver's callID
	uint32 GRESequence;
	uint32 GREAcknowledgment;
	uint32 *pGRESequence;
	uint32 *pGREAcknowledgment;
	uint32 *pServerGRESequence;		//pointer to server's sequence number

	//L2TP decision
	uint16 l2tpFlagVersion;

	//IGMP
	uint8	IGMPType;
	uint8   ingressIgmpMldDrop;

	//IGMPv6
	uint8	IGMPv6Type;

	//ICMPv4
	uint8	ICMPType;
	uint8	ICMPCode;
	uint16	ICMPIdentifier;
	uint16	ICMPSeqNum;
	//ICMPv6
	uint8	ICMPv6Type;
	uint8	ICMPv6Flag;

	/* Ingress Information */
	rtk_rg_port_idx_t ingressPort;
	rtk_rg_ingressLocation_t ingressLocation;

	/* WLAN */
	rtk_rg_mbssidDev_t wlan_dev_idx;
	
	/* FWD Decision */
	rtk_rg_fwdDecision_t	fwdDecision;
	
//==================================================================================

	//20140829LUKE: CAUTION!!this field will be used to count pktHdr size, DO NOT MOVE IT!!
	int COUNT_LENGTH_FIELD;

	//NAPT/NAPTR result
	rtk_rg_l4Direction_t l4Direction;
	int32 naptOutboundIndx;
	int32 naptrInboundIndx;
	uint8 naptrLookupHit; //0:non-hit, 1:hit first-callback(UPnP), 2:hit second-callback(Virtual Server), 3:hit third-callback(DMZ)

	/* Routing decision */
	int		sipL3Idx; //decision in _rtk_rg_sip_classification
	int		dipL3Idx; //decision in _rtk_rg_dip_classification	
	int		netifIdx; //wan intf
	int		extipIdx;
	int		dipArpOrNBIdx;
	int		nexthopIdx;
	int		isGatewayPacket;
	int		dmacL2Idx;
	//20140811LUKE: used for tcp_hw_learning_at_syn
	int		srcNetifIdx; //lan intf
	int		smacL2Idx;
#if defined(CONFIG_RTL9602C_SERIES)
	//mib counter and cf decision
	int 	mibNetifIdx; //init by FAIL
	rtk_rg_cf_direction_type_t mibDirect;
	char 	mibTagDelta;	//add or minus for NIC offload vlan tag
#endif

	/* Binding */
	int		bindNextHopIdx;
	int		layer2BindNetifIdx; //layer2 binding wan intf decision, only apply when destination port is WAN port(in rg_db.systemGlobal.wanPortMask.portmask)

	/* VLAN decision*/
	uint8	dmac2VlanTagif;
	uint8 	egressVlanTagif; //0:untag  1:tagged
#if defined(CONFIG_RTL9602C_SERIES)
	uint8 	egressServiceVlanTagif; //0:untag  1:tagged with tpid, 2:tag with tpid2,  3:tagged with original S-tag-tpid
#else
	uint8 	egressServiceVlanTagif; //0:untag,  1:tagged with tpid, 2:tag with 0x8100
#endif
	uint16 	internalVlanID;
	int		dmac2VlanID;
	uint16 	egressVlanID;
	uint16	egressServiceVlanID;
	uint16	egressVlanCfi;
	uint16	egressServiceVlanDei;

	uint16 	ingressDecideVlanID; //record the ingress decision result(acl,1q,port-based,PPB), only used for proc/rg/tcp_hw_learning_at_syn now. 

	/*ACL*/
	rtk_rg_aclHitAndAction_t aclDecision;

	/*QOS*/
	uint8	internalPriority;
	uint8	egressPriority;
	uint8	egressServicePriority;
	int16	egressDSCP;
	int	aclPriority;//record ACL priotity for internal priority decision
	int naptPriorityHitIf;//record is any naptPriority Rule hit
	int naptPriority;//record the napt assigned priority (valid when naptPriorityHitIf==1)

	/* ALG */
	rtk_rg_algAction_t algAction;
	uint8 algKeepExtPort;		//should I choose another port for replicate one?
	p_algRegisterFunction algRegFun;

#ifdef CONFIG_GPON_FEATURE
	/* Stream ID */
	uint8	streamID;
#endif

	/* FWD Decision */
	uint8	egressMACPort;
	uint8	egressExtPort;
	rtk_rg_portmask_t	multicastMacPortMask;
	rtk_rg_portmask_t	multicastExtPortMask;
	uint8	overMTU;		//if the packet is bigger than interface's MTU	
	uint8	aclHit;	//0:no any rule hit(can be added into short cut)  1:at lease one rule hit(can not add to short cut)
	int8	aclPolicyRoute;		//-1: no policy route hit 
	int8	aclPolicyRoute_arp2Dmac;		//0: use policy route's NH as DMAC, 1: use ARP's LUT as DMAC
	int8	swLutL2Only;		//0 means permit all, 1 means permit L2 only
//	uint8	egressHasSmallerMTU;	//if egress intf has smaller MTU, check if we need to split packet before send
	uint8	gponDsBcModuleRuleHit;	//notify master wifi vlanTagshould follow this hit decision
	uint32	egressUniPortmask; //ACL/CF force redirect egress port
	uint8	egressTagAccType;	//used for _rtk_rg_get_stagCtagAccelerationType
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	int32 	egressWlanDevIdx; 			//for rate limit, 0:root, 1:vap0, 2:vap1, 3:vap2, 4:vap3
#endif

#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
	rtk_rg_ipv6_connLookup_t ipv6_serverInLanLookup;	//upnp, virtual server, dmz hit or not
#endif

#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	int32	ipv6StatefulHashValue;	//keep the hash value for fill software data structure
	rtk_rg_ipv6_layer4_linkList_t	*pIPv6StatefulList;		//used for update information after normal path
#endif
	rtk_rg_bindingDecision_t 	bindingDecision;
	rtk_rg_fwdEngineReturn_t	directTxResult;		//do directTX may need broadcast, therefore we need to keep it's return value
#if defined(CONFIG_ROME_NAPT_SHORTCUT) || defined(CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT)
	rtk_rg_shortcutDecision_t	shortcutStatus;
	//uint32	currentShortcutIndex;
#ifdef CONFIG_ROME_NAPT_SHORTCUT
	rtk_rg_napt_shortcut_t *pCurrentShortcutEntry;
	rtk_rg_napt_shortcut_t *pInboundShortcutEntry;	//used when tcp_in_shortcut_learning_at_syn is enabled
	int currentShortcutIdx;
	int inboundShortcutIdx;
#endif
#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
	rtk_rg_naptv6_shortcut_t *pCurrentV6ShortcutEntry;
	int currentV6ShortcutIdx;
#endif
#endif
	int unmatched_cf_act;

	int direct;		//0: NAPT upstream  1:NAPT downstream  2:L2 Bridge 3:IPv6 Routing
	int naptIdx;	//pktHdr->direct is 0 : naptOutboundIdx  pktHdr->direct is 1 :naptInboundIdx
	int l3Modify;
	int l4Modify;
	unsigned int l2AgentCalled:1;	// 0: not call yet.  1: called
	unsigned int arpAgentCalled:1;	// 0: not call yet.  1: called
	unsigned int neighborAgentCalled:1;	// 0: not call yet.  1: called

	// delete before sending
	unsigned int delNaptConnection:1; //0: don't delete before send.  1: delete before send.

	// add after sending
	unsigned int addNaptAfterNicTx:1;

	// enable when PPTP or L2TP to save hw napt table usage
	unsigned int addNaptSwOnly:1;

	// data path is hairpinNat
	unsigned int isHairpinNat:1;
	
} rtk_rg_pktHdr_t;
/* End of Parser ========================================================== */

/* ALE ==================================================================== */

typedef enum rtk_rg_ipClassification_e
{
	IP_CLASS_FAIL = -1,
	IP_CLASS_NPI =0,
	IP_CLASS_NI  =1,
	IP_CLASS_LP  =2,
	IP_CLASS_RP  =3,
	IP_CLASS_NPE =4,
	IP_CLASS_NE  =5,
	MAX_SIP_CLASS =4,
	MAX_DIP_CLASS =6
} rtk_rg_ipClassification_t;


typedef enum rtk_rg_sipDipClassification_e
{
	SIP_DIP_CLASS_ROUTING=0,
	SIP_DIP_CLASS_NAT=1,
	SIP_DIP_CLASS_NAPT=2,
	SIP_DIP_CLASS_NATR=3,
	SIP_DIP_CLASS_NAPTR=4,
	SIP_DIP_CLASS_CPU=5,	
	SIP_DIP_CLASS_DROP=6,
	SIP_DIP_CLASS_HAIRPIN_NAT=7
} rtk_rg_sipDipClassification_t;






/* End of ALE ============================================================== */

/* Tables ================================================================= */
/*typedef struct rtk_rg_table_mac_s
{
	//HW table
	rtk_mac_t macAddr; 
} rtk_rg_table_mac_t;*/

typedef struct rtk_rg_table_vlan_s
{
	//HW table
    rtk_portmask_t MemberPortmask; /*bit'6 CPU port */
    rtk_portmask_t UntagPortmask;
	rtk_portmask_t Ext_portmask;   /*bit'0 CPU prot */
	rtk_fid_t fid;
    rtk_fidMode_t fidMode;
	rtk_pri_t priority;
	rtk_enable_t priorityEn;


	//SW table specific
	int valid;
	int addedAsCustomerVLAN;		//used to check this vlan created by cvlan apis or not
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	unsigned int wlan0DevMask;
#endif
} rtk_rg_table_vlan_t;

typedef struct rtk_rg_table_lut_s
{
	rtk_l2_addr_table_t rtk_lut;
	
	uint32 valid:1;
	uint32 fix_l34_vlan:1;
	uint8 redirect_http_req;
	uint32 idleSecs;
	char category;
	char wlan_device_idx;	//used only when rtk_lut is unicast and port is CPU and extport is 1 or 2
	uint8 ipv6_addr[16];
#if defined(CONFIG_RTL9602C_SERIES)
	uint32 arp_refCount;
#endif
	char dev_name[MAX_LANNET_DEV_NAME_LENGTH];	//for LANNetInfo
	rtk_rg_lanNet_connect_type_t conn_type;		//for LANNetInfo
} rtk_rg_table_lut_t;

typedef struct rtk_rg_table_netif_s
{
	rtk_l34_netif_entry_t	rtk_netif;
	int	l2_idx;
#if defined(CONFIG_RTL9602C_SERIES)
	rtk_l34_mib_t rtk_mib;
#endif
} rtk_rg_table_netif_t;

typedef struct rtk_rg_table_l3_s
{

	rtk_l34_routing_entry_t	rtk_l3;
	ipaddr_t	gateway_ip;		//20140703LUKE: used to compare gateway IP or not
	ipaddr_t	netmask;
} rtk_rg_table_l3_t;

typedef struct rtk_rg_table_extip_s
{	
	rtk_l34_ext_intip_entry_t	rtk_extip;

} rtk_rg_table_extip_t;

typedef struct rtk_rg_table_nexthop_s
{

	rtk_l34_nexthop_entry_t	rtk_nexthop;

} rtk_rg_table_nexthop_t;

typedef struct rtk_rg_table_pppoe_s
{
	
	rtk_l34_pppoe_entry_t	rtk_pppoe;

	uint32					idleSecs;
} rtk_rg_table_pppoe_t;

typedef struct rtk_rg_table_arp_s
{
	
	rtk_l34_arp_entry_t	rtk_arp;

	ipaddr_t ipv4Addr;	
	int staticEntry;
	int idleSecs;
	int8 sendReqCount;
	int8 routingIdx;
	//lan device info
	rtk_rg_lanNetInfo_t lanNetInfo;
} rtk_rg_table_arp_t;

typedef struct rtk_rg_vbind_linkList_s
{
	int8	wanIdx;
	uint16	vlanId;
		
	struct list_head vbd_list;
}rtk_rg_vbind_linkList_t;

typedef struct rtk_rg_arp_linkList_s
{
	uint16 idx;		//from MAX_ARP_HW_TABLE_SIZE to (MAX_ARP_SW_TABLE_SIZE - MAX_ARP_HW_TABLE_SIZE)
	
	struct list_head arp_list;
}rtk_rg_arp_linkList_t;

typedef struct rtk_rg_lut_linkList_s
{
	uint16 idx;		//from MAX_LUT_HW_TABLE_SIZE to MAX_LUT_HW_TABLE_SIZE+MAX_LUT_BCAM_TABLE_SIZE
	
	struct list_head lut_list;
}rtk_rg_lut_linkList_t;

#if defined(CONFIG_APOLLO_GPON_FPGATEST)
typedef struct rtk_rg_vmac_skb_linlList_s
{
	struct sk_buff *skb;
	
	struct list_head vmac_list;
}rtk_rg_vmac_skb_linlList_t;
#endif

typedef struct rtk_rg_table_wantype_s
{

	rtk_wanType_entry_t rtk_wantype;
	//sw
	unsigned int valid:1;
} rtk_rg_table_wantype_t;

typedef struct rtk_rg_table_naptIn_s
{
    rtk_l34_naptInbound_entry_t rtk_naptIn;
    uint32 hashIdx: MAX_NAPT_IN_SW_ENTRY_WIDTH; //naptIn hashed value, range from 0 ~ MAX_NAPT_IN_HW_TABLE_SIZE>>2 //13
    uint32 idleSecs: MAX_NAPT_IN_IDLESEC_WIDTH; // 12
    uint32 refCount: MAX_NAPT_IN_REFCOUNT_WIDTH;    // 1
    uint32 coneType: MAX_NAPT_IN_CONETYPE_WIDTH;    // 2
    uint32 canBeReplaced: MAX_NAPT_IN_CANBEREPLACE_WIDTH;   // 1
    uint32 priValid: MAX_NAPT_IN_PRIVALID_WIDTH;            // 1
    uint32 cannotAddToHw: MAX_NAPT_IN_CANTADDTOHW_WIDTH;    //0: add to hw if necessary, 1: do not add to hw due to DPI check // 1
    uint32 priValue: MAX_NAPT_IN_PRIVALUE_WIDTH;    // 3
    uint32 symmetricNaptOutIdx: MAX_NAPT_IN_NAPTOUTIDX_WIDTH;   // 15
} rtk_rg_table_naptIn_t;

typedef struct rtk_rg_table_naptOut_s
{
	rtk_l34_naptOutbound_entry_t	rtk_naptOut;
	uint16 remotePort;
	ipaddr_t remoteIp;
	uint32 state: MAX_NAPT_OUT_STATE_WIDTH;     // 4
	uint32 extPort: MAX_NAPT_OUT_PORT_WIDTH;    // 16
  	uint32 idleSecs: MAX_NAPT_OUT_IDLESEC_WIDTH;    // 12

	uint32 hashOutIdx: MAX_NAPT_OUT_SW_ENTRY_WIDTH;	//naptOut hashed value, range from 0 ~ MAX_NAPT_OUT_HW_TABLE_SIZE>>2 // 13
	uint32 canBeReplaced: MAX_NAPT_OUT_CANBEREPLACE_WIDTH; // 1
	uint32 priValid: MAX_NAPT_OUT_PRIVALID_WIDTH;   // 1
	uint32 priValue: MAX_NAPT_OUT_PRIVALUE_WIDTH;   // 3
	uint32 cannotAddToHw: MAX_NAPT_OUT_CANTADDTOHW_WIDTH;	//0: add to hw if necessary, 1: do not add to hw due to DPI check  / 1
} rtk_rg_table_naptOut_t;

typedef struct rtk_rg_table_naptOut_linkList_s
{
	struct rtk_rg_table_naptOut_linkList_s *pNext;
	uint16 idx;	
} rtk_rg_table_naptOut_linkList_t;

typedef struct rtk_rg_table_naptIn_linkList_s
{
	struct rtk_rg_table_naptIn_linkList_s *pNext;
	uint16 idx;	
} rtk_rg_table_naptIn_linkList_t;

typedef struct rtk_rg_table_icmp_flow_s
{
	uint8 valid;
	ipaddr_t internalIP;
	ipaddr_t remoteIP;
	uint16 IPID;		//identifier field in IP header
	uint16 inboundIPID;		//identifier field in IP header
	uint8 ICMPType;
	uint8 ICMPCode;
	uint16 ICMPID;		//identifier field in ICMP header (some type without this field)
	uint16 ICMPSeqNum;
}rtk_rg_table_icmp_flow_t;

typedef struct rtk_rg_table_icmp_linkList_s
{
	rtk_rg_table_icmp_flow_t icmpFlow;
	struct rtk_rg_table_icmp_linkList_s *pPrev, *pNext;
}rtk_rg_table_icmp_linkList_t;

typedef struct rtk_rg_table_bind_s
{
	rtk_binding_entry_t rtk_bind;

	//SW table specific
	unsigned int valid:1;
} rtk_rg_table_bind_t;

typedef struct rtk_rg_table_v6route_s
{
	rtk_ipv6Routing_entry_t rtk_v6route;
	int internal; //support for IPv6 NAPT, 0:internal  1:external
} rtk_rg_table_v6route_t;

typedef struct rtk_rg_table_v6ExtIp_s
{
	int valid;
	rtk_ipv6_addr_t	externalIp;
	int nextHopIdx;
} rtk_rg_table_v6ExtIp_t;//support for IPv6 NAPT

typedef struct rtk_rg_table_v6neighbor_s
{
	rtk_ipv6Neighbor_entry_t rtk_v6neighbor;

	int staticEntry;
	int idleSecs;
} rtk_rg_table_v6neighbor_t;

#if defined(CONFIG_RTL9602C_SERIES)
typedef struct rtk_rg_table_dslite_s
{
	rtk_l34_dsliteInf_entry_t rtk_dslite;
	int intfIdx;	//which interface point to this DSlite entry
} rtk_rg_table_dslite_t;
#endif
typedef struct rtk_rg_table_dsliteMc_s
{
	rtk_l34_dsliteMc_entry_t rtk_dsliteMc;
	rtk_ipv6_addr_t ipUPrefix64_AND_mask;
	rtk_ipv6_addr_t ipMPrefix64_AND_mask;
} rtk_rg_table_dsliteMc_t;


/* End of Tables ============================================================== */

/* System ============================================================== */
typedef int (*p_GWMACRequestCallBack)(ipaddr_t, int);		//IP address, Lut table idx

typedef struct rtk_rg_arp_request_s
{
	ipaddr_t reqIp;
	int volatile finished; //used to indicate the ARP request return or not
	p_GWMACRequestCallBack	gwMacReqCallBack;
} rtk_rg_arp_request_t;

typedef int (*p_IPV6GWMACRequestCallBack)(unsigned char*, int);		//IP address, Lut table idx

typedef struct rtk_rg_neighbor_discovery_s
{
	rtk_ipv6_addr_t reqIp;
	int volatile finished; //used to indicate the Neighbor Discovery return or not
	p_IPV6GWMACRequestCallBack	ipv6GwMacReqCallBack;
} rtk_rg_neighbor_discovery_t;


typedef struct rtk_rg_interface_info_global_s
{
	rtk_rg_intfInfo_t storedInfo;
	int valid;
	int lan_or_wan_index;		//index of lan or wan group
	union{
		rtk_rg_lanIntfConf_t *p_lanIntfConf;
		rtk_rg_ipStaticInfo_t *p_wanStaticInfo;
	};
} rtk_rg_interface_info_global_t;

typedef struct rtk_rg_wan_interface_group_info_s
{
	rtk_rg_interface_info_global_t *p_intfInfo;
	int index;		//index of netif table
	rtk_rg_wanIntfConf_t *p_wanIntfConf;
	unsigned int disableBroadcast:1;		//only for BD WAN which has same VLANID as LAN intf
} rtk_rg_wan_interface_group_info_t;

typedef struct rtk_rg_lan_interface_group_info_s
{
	rtk_rg_interface_info_global_t *p_intfInfo;
	int index;
} rtk_rg_lan_interface_group_info_t;

typedef struct rtk_rg_virtual_server_info_s
{
	rtk_rg_virtualServer_t* p_virtualServer;
	int index;
} rtk_rg_virtual_server_info_t;

typedef struct rtk_rg_upnp_info_s
{
	rtk_rg_upnpConnection_t* p_upnp;
	int index;
} rtk_rg_upnp_info_t;

typedef struct rtk_rg_port_proto_vid_s
{
	rtk_vlan_protoVlanCfg_t protoVLANCfg[MAX_PORT_PROTO_GROUP_SIZE];

}rtk_rg_port_proto_vid_t;

typedef struct rtk_rg_routing_arpInfo_s
{
	int routingIdx;
	int intfIdx;
	ipaddr_t notMask;		//0000..111, the "NOT" result of network mask
	int bitNum;				//the bits number of "zero" in network mask, /24=8, /25=7,.../30=2 => 31 minus ipMask in routing_entry
	int arpStart;
	int	arpEnd;
	unsigned char isLan;		//for LAN:1, for WAN:0
} rtk_rg_routing_arpInfo_t;

typedef struct rtk_rg_routing_linkList_s		//used when transfer sw ARP to hw ARP
{
	uint8 idx;
	uint8 bitNum;
	struct list_head route_list;
}rtk_rg_routing_linkList_t;

typedef enum rtk_rg_sa_learning_exceed_action_e
{
	SA_LEARN_EXCEED_ACTION_PERMIT=0,
	SA_LEARN_EXCEED_ACTION_PERMIT_L2,
	SA_LEARN_EXCEED_ACTION_DROP,
	SA_LEARN_EXCEED_ACTION_END,
}rtk_rg_sa_learning_exceed_action_t;

typedef enum rtk_rg_accessWanLimitType_e
{
	RG_ACCESSWAN_TYPE_UNLIMIT=0,	//turn off
	RG_ACCESSWAN_TYPE_PORT,
	RG_ACCESSWAN_TYPE_PORTMASK,
	RG_ACCESSWAN_TYPE_CATEGORY,
	RG_ACCESSWAN_TYPE_END,
}rtk_rg_accessWanLimitType_t;

typedef enum rtk_rg_qosWeightSelection_e
{
	WEIGHT_OF_PORTBASED,
	WEIGHT_OF_DOT1Q,
	WEIGHT_OF_DSCP,
	WEIGHT_OF_ACL,
	WEIGHT_OF_LUTFWD,
	WEIGHT_OF_SABASED,
	WEIGHT_OF_VLANBASED,
	WEIGHT_OF_SVLANBASED,
	WEIGHT_OF_L4BASED,
	WEIGHT_OF_END,
} rtk_rg_qosWeightSelection_t;


typedef enum rtk_rg_qosDscpRemarkSrcSelect_e
{
	DISABLED_DSCP_REMARK,
	ENABLED_DSCP_REMARK_AND_SRC_FROM_INTERNALPRI,
	ENABLED_DSCP_REMARK_AND_SRC_FROM_DSCP,
} rtk_rg_qosDscpRemarkSrcSelect_t;


typedef struct rtk_rg_qosInternalDecision_s		//used when SW Qos decision
{
	//internal pri decision
	uint8 internalPri;
	uint8 internalPriSelectWeight[WEIGHT_OF_END];
	uint8 qosDot1pPriRemapToInternalPriTbl[8];
	uint8 qosDscpRemapToInternalPri[64];
	uint8 qosPortBasedPriority[RTK_RG_MAC_PORT_MAX];

	//remarking by internal 
	rtk_rg_qosDscpRemarkSrcSelect_t qosDscpRemarkEgressPortEnableAndSrcSelect[RTK_RG_MAC_PORT_MAX];
	rtk_rg_enable_t qosDot1pPriRemarkByInternalPriEgressPortEnable[RTK_RG_MAC_PORT_MAX];
	uint8 qosDot1pPriRemarkByInternalPri[8];
	uint8 qosDscpRemarkByInternalPri[8];
	uint8 qosDscpRemarkByDscp[64];

	
}rtk_rg_qosInternalDecision_t;

typedef struct rtk_rg_cvidCpri2SidMapping_s{
	uint16 cvid;
	uint8 cpri;
	uint8 sid;
}rtk_rg_cvidCpri2SidMapping_t;

typedef enum  rtk_rg_gatewayServiceType_e
{
	GATEWAY_SERVER_SERVICE	= 0,	//lookup dport
	GATEWAY_CLIENT_SERVICE	= 1,	//lookup sport
}rtk_rg_gatewayServiceType_t;

typedef struct rtk_rg_gatewayServicePortEntry_s{
	int valid;
	unsigned short int port_num;
	rtk_rg_gatewayServiceType_t type;
}rtk_rg_gatewayServicePortEntry_t;

//for get and set rg init state APIs
typedef enum rtk_rg_initState_e
{
	RTK_RG_DURING_INIT=0,
	RTK_RG_INIT_FINISHED,
}rtk_rg_initState_t;


#define REG_SHIFT_BASE 0xa0046266 
#define REG_SHIFT_1 (REG_SHIFT_BASE - 0x00020000)
#define REG_SHIFT_2 (REG_SHIFT_BASE - 0x00010000)
#define REG_SHIFT_3_0 0x00000000
#define REG_SHIFT_3_1 0x00001000
#define REG_SHIFT_3_2 0x00002000


typedef enum rtk_rg_internal_support_bit_e
{
	RTK_RG_INTERNAL_SUPPORT_BIT0=(1<<0), 
	RTK_RG_INTERNAL_SUPPORT_BIT1=(1<<1), 
	RTK_RG_INTERNAL_SUPPORT_BIT2=(1<<2), 
	RTK_RG_INTERNAL_SUPPORT_BIT3=(1<<3), 
}rtk_rg_internal_support_bit_t;



typedef struct rtk_rg_wlan_binding_s
{
	unsigned int exist:1;			//0:device not exist, 1:device exist
	unsigned int set_bind:1;		//0:not binding to WAN, 1:binding
	
	int bind_wanIntf;
} rtk_rg_wlan_binding_t;


typedef enum rtk_rg_ipsec_passthru_e
{
    PASS_OLD = 0,		//old style: not work now!
    PASS_ENABLE,		//enable passthru
    PASS_DISABLE,		//disable passthru
    PASS_END			//end value
} rtk_rg_ipsec_passthru_t;

typedef struct rtk_rg_statistic_s
{
	//Packet Type
	uint32 perPortCnt_broadcast[RTK_RG_PORT_MAX];	
	uint32 perPortCnt_multicast[RTK_RG_PORT_MAX];
	uint32 perPortCnt_unicast[RTK_RG_PORT_MAX];
	uint32 perPortCnt_UDP[RTK_RG_PORT_MAX];
	uint32 perPortCnt_TCP[RTK_RG_PORT_MAX];

	//TCP Flags
	uint32 perPortCnt_SYN[RTK_RG_PORT_MAX];
	uint32 perPortCnt_SYN_ACK[RTK_RG_PORT_MAX];
	uint32 perPortCnt_FIN[RTK_RG_PORT_MAX];
	uint32 perPortCnt_FIN_ACK[RTK_RG_PORT_MAX];
	uint32 perPortCnt_FIN_PSH_ACK[RTK_RG_PORT_MAX];
	uint32 perPortCnt_RST[RTK_RG_PORT_MAX];
	uint32 perPortCnt_RST_ACK[RTK_RG_PORT_MAX];
	uint32 perPortCnt_ACK[RTK_RG_PORT_MAX];

	//ARP/NB
	uint32 perPortCnt_ARP_request[RTK_RG_PORT_MAX];
	uint32 perPortCnt_ARP_reply[RTK_RG_PORT_MAX];
	uint32 perPortCnt_NB_solicitation[RTK_RG_PORT_MAX];
	uint32 perPortCnt_NB_advertisement[RTK_RG_PORT_MAX];

	//Reason	
	uint32 perPortCnt_Reason[256][RTK_RG_EXT_PORT2]; //TTL

	//forwarding
	uint32 perPortCnt_slowPath[RTK_RG_PORT_MAX];
	uint32 perPortCnt_shortcut[RTK_RG_PORT_MAX];
	uint32 perPortCnt_shortcutv6[RTK_RG_PORT_MAX];
	uint32 perPortCnt_L2FWD[RTK_RG_PORT_MAX];
	uint32 perPortCnt_IPv4_L3FWD[RTK_RG_PORT_MAX];
	uint32 perPortCnt_IPv6_L3FWD[RTK_RG_PORT_MAX];	
	uint32 perPortCnt_L4FWD[RTK_RG_PORT_MAX];
	uint32 perPortCnt_Drop[RTK_RG_PORT_MAX];
	uint32 perPortCnt_ToPS[RTK_RG_PORT_MAX];
	uint32 perPortCnt_naptOutLRU[RTK_RG_PORT_MAX];
	uint32 perPortCnt_naptInLRU[RTK_RG_PORT_MAX];
	uint32 perPortCnt_v4ShortcutLRU[RTK_RG_PORT_MAX];
	uint32 perPortCnt_v6ShortcutLRU[RTK_RG_PORT_MAX];

	//skb prealloc,alloc,free
	uint32 perPortCnt_skb_pre_alloc_for_uc[RTK_RG_PORT_MAX];
	uint32 perPortCnt_skb_pre_alloc_for_mc_bc[RTK_RG_PORT_MAX];
	uint32 perPortCnt_skb_alloc[RTK_RG_PORT_MAX];
	uint32 perPortCnt_skb_free[RTK_RG_PORT_MAX];

	//nicTx, wifiTx
	uint32 perPortCnt_NIC_TX[RTK_RG_PORT_MAX];
	uint32 perPortCnt_WIFI_TX[RTK_RG_PORT_MAX];
	
}rtk_rg_statistic_t;

typedef struct rtk_rg_redirectHttpAll_s
{
	char pushweb[MAX_REDIRECT_PUSH_WEB_SIZE];
	unsigned int enable:2;	//0:disable, 1:enable, 2:enable with URL
}rtk_rg_redirectHttpAll_t;

typedef struct rtk_rg_redirectHttpURL_s
{
	char url_str[MAX_URL_FILTER_STR_LENGTH];
	char dst_url_str[MAX_URL_FILTER_STR_LENGTH];
	int16 count;	//-1: always redirect, 0: stop redirect
}rtk_rg_redirectHttpURL_t;

typedef struct rtk_rg_redirectHttpURL_linkList_s
{
	rtk_rg_redirectHttpURL_t url_data;
	atomic_t count;	//for atomic operation
	int16 url_len;
	struct list_head url_list;
}rtk_rg_redirectHttpURL_linkList_t;

typedef struct rtk_rg_redirectHttpWhiteList_s
{
	char url_str[MAX_URL_FILTER_STR_LENGTH];
	char keyword_str[MAX_URL_FILTER_STR_LENGTH];
}rtk_rg_redirectHttpWhiteList_t;

typedef struct rtk_rg_redirectHttpWhiteList_linkList_s
{
	rtk_rg_redirectHttpWhiteList_t white_data;
	int16 url_len;
	int16 keyword_len;
	struct list_head white_list;
}rtk_rg_redirectHttpWhiteList_linkList_t;

typedef struct rtk_rg_glb_system_s
{
	/* System Module */
	rtk_rg_pktHdr_t	pktHeader_1;
	struct rx_info rxInfoFromWLAN; //for WIFI to fwdEngineInput
	rtk_rg_sipDipClassification_t sipDipClass[MAX_SIP_CLASS][MAX_DIP_CLASS];
	unsigned int phyPortStatus;	//0:disable 1:enable

	rtk_rg_initParams_t initParam;
	unsigned int nxpRefCount[MAX_NEXTHOP_SW_TABLE_SIZE];		//store how many struct reference each nexthop entry
	//int bindToIntf[MAX_BIND_HW_TABLE_SIZE];					//store what interface index this binding connected with
	//int bindWithVLAN[MAX_BIND_HW_TABLE_SIZE];				//store what vlan this binding rule used
	rtk_rg_arp_request_t intfArpRequest[MAX_NETIF_SW_TABLE_SIZE<<1];	//each interface may issue one ARP request		//over MAX_NETIF_SIZE belong to PPTP&L2TP
	rtk_rg_neighbor_discovery_t intfNeighborDiscovery[MAX_NETIF_SW_TABLE_SIZE<<1];	//each interface may issue one Neighbor Discovery	//over MAX_NETIF_SIZE belong to DSLITE
	unsigned int vlanInit;
	unsigned int wanIntfTotalNum;
	unsigned int lanIntfTotalNum;
	unsigned int vlanBindTotalNum;			//indicate how many vlan-binding we have
	unsigned int ipv4FragmentQueueNum;		//indicate how many fragment packets in the queue
	//unsigned int pppoeBeforeCalled;			//indicate if we had called PPPoE Before
	unsigned int defaultTrapLUTIdx;			//indicate the index of default LUT used to trap to CPU
	unsigned int wanInfoSet;				//indicate which wan has been set info after add
	unsigned int nicIgmpModuleIndex;		//indicate IGMP snooping module's index after registration
	int defaultRouteSet;					//indicate which interface had set default route
	int defaultIPV6RouteSet;				//indicate which interface had set ipv6 default route
	int intfIdxForReset;						//indicate which wan has been reset and need to re-add in the same index
	rtk_rg_interface_info_global_t interfaceInfo[MAX_NETIF_SW_TABLE_SIZE];		//store each interface information, LAN or WAN
	rtk_rg_lan_interface_group_info_t lanIntfGroup[MAX_NETIF_SW_TABLE_SIZE];
	rtk_rg_wan_interface_group_info_t wanIntfGroup[MAX_NETIF_SW_TABLE_SIZE];
	int otherWanVlan[MAX_NETIF_SW_TABLE_SIZE];		//store Other Wan's VLANID used for traffic isolation
	rtk_rg_portmask_t wanPortMask,lanPortMask;
	//int bridgeWanNum;								//how many bridge WAN interface in system
	char vlan_proto_block_created;		//indicate the fwdVLAN_Proto_Block had been created or not
	int portBasedVID[RTK_RG_PORT_MAX];				//indicate which port-based VLAN ID should be used when untag(contain extension port)
	rtk_rg_port_proto_vid_t protoBasedVID[RTK_RG_PORT_MAX];	//indicate which port-and-protocol-based VLAN ID and GROUP ID would be used when match GROUP setting

#ifdef CONFIG_MASTER_WLAN0_ENABLE
	int wlan0DeviceBasedVID[MAX_WLAN_DEVICE_NUM];	//indicate which VLAN ID should be given when packet coming from WLAN
	rtk_rg_wlan_binding_t	wlan0BindDecision[MAX_WLAN_DEVICE_NUM];
#endif
	//rtk_rg_wlan_binding_t 	wlan1BindDecision[MAX_WLAN_DEVICE_NUM];
	 
	rtk_vlan_protoGroup_t protoGroup[MAX_PORT_PROTO_GROUP_SIZE];	//port and protocol group settings

	int layer2HouseKeepIndex;						//indicate the index last house keep end
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	int 			v6StatefulHouseKeepIndex;					//indicate the list last house keep end
	atomic_t 		v6StatefulConnectionNum;				//total allocated connection number
	unsigned int 	ipv6FragmentQueueNum;				//indicate how many fragment packets in the queue
#endif
	char portbinding_wan_idx[RTK_RG_PORT_MAX];		//record this port is port-binding to which WAN
	rtk_rg_portmask_t	non_binding_pmsk;				//record which port is not binding to any WAN	

	rtk_l34_bindAct_t	l34BindAction[L34_BIND_TYPE_END];
	rtk_enable_t		l34GlobalState[L34_GLOBAL_STATE_END];

#if 0//def CONFIG_GPON_FEATURE
	int8 untagBridgeGponWanIdx;		//only needed for PON port
#endif

	//Control Path switches
	int ctrlPathByProtocolStack_broadcast;		//deafult 0, let fwdEngine handle broadcast packets
	int ctrlPathByProtocolStack_ICMP;			//default 0, let fwdEngine handle ICMP packets(except OverMTU packets)

	rtk_rg_accessWanLimitType_t activeLimitFunction;
	
	//Access wan limit
	rtk_rg_portmask_t accessWanLimitPortMask_member;
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	unsigned int accessWanLimitPortMask_wlan0member;
#endif
	int accessWanLimitPortMask;
	int accessWanLimitCategory[WanAccessCategoryNum];
	rtk_rg_sa_learning_exceed_action_t accessWanLimitPortMaskAction;
	rtk_rg_sa_learning_exceed_action_t accessWanLimitCategoryAction[WanAccessCategoryNum];
	atomic_t accessWanLimitPortMaskCount;
	atomic_t accessWanLimitCategoryCount[WanAccessCategoryNum];

	//SA learning limit number - per port and control action - per port
	int sourceAddrLearningLimitNumber[RTK_RG_PORT_MAX];
	rtk_rg_sa_learning_exceed_action_t sourceAddrLearningAction[RTK_RG_PORT_MAX];
	atomic_t sourceAddrLearningCount[RTK_RG_PORT_MAX];
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	int wlan0SourceAddrLearningLimitNumber[MAX_WLAN_DEVICE_NUM];
	rtk_rg_sa_learning_exceed_action_t wlan0SourceAddrLearningAction[MAX_WLAN_DEVICE_NUM];
	atomic_t wlan0SourceAddrLearningCount[MAX_WLAN_DEVICE_NUM];
#endif

	unsigned int virtualServerTotalNum;
	unsigned int upnpTotalNum;
	rtk_rg_virtual_server_info_t virtualServerGroup[MAX_VIRTUAL_SERVER_SW_TABLE_SIZE];
	rtk_rg_upnp_info_t upnpGroup[MAX_UPNP_SW_TABLE_SIZE];

	//maintain ACL in fwdEngine
	int acl_SW_table_entry_size; //using for speed up RG_ACL which compared in fwdEninge.
	int acl_SWindex_sorting_by_weight[MAX_ACL_SW_ENTRY_SIZE];	//The array record the RG_ACL rule index which sorted by weight. The ACL rule priority should handled by this sorting result.
	int acl_SWindex_sorting_by_weight_and_ingress_cvid_action[MAX_ACL_SW_ENTRY_SIZE];	//The array record the RG_ACL rule index which sorted by weight, and the acl ingress_cvid action is valid. use for speed up fwdEngine internalVid decision.
#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM	
	int acl_SW_egress_intf_type_zero_num;	//number of ACL rules of type zero with egress intf idx
#endif
	uint32 cf_valid_mask[(TOTAL_CF_ENTRY_SIZE/32)+1]; //use for speed up CF[64-511] (or pattern 1) is need to check or not.  

	rtk_rg_filterControlType_t urlFilterMode;
	int urlFilter_totalNum;
	int urlFilter_valid_entry[MAX_URL_FILTER_ENTRY_SIZE];//promote urlFilter compare efficiency
	rtk_rg_macFilterSWEntry_t macFilter_table_Entry[MAX_MAC_FILTER_ENTRY_SIZE];

	rtk_rg_hwnatState_t hwnat_enable;
	rtk_rg_enable_t unknownDA_Trap_to_PS_enable;
	rtk_rg_enable_t igmp_Trap_to_PS_enable;
	rtk_rg_enable_t mld_Trap_to_PS_enable;
	rtk_rg_enable_t ipv6MC_translate_ingressVID_enable;
	rtk_rg_hwnatState_t strangeSA_drop;	//learned in LAN but come back in WAN
	rtk_rg_hwnatState_t fix_l34_to_untag_enable;
	int port_binding_by_protocal; //0:IPv4+IPv6 both bridge  1:binding IPv4, IPv6 Bridge 2:binding IPv6, IPv4 Bridge
	unsigned int port_binding_by_protocal_filter_vid; //filter port_binding_by_protocal with vid for downstream unicast. (for IPCP/IP6CP + vid donstream filter)
	rtk_rg_multicastProtocol_t multicastProtocol;	//0: IGMP & MLD, 1: IGMP, 2: MLD
	rtk_rg_enable_t pppoe_bc_passthrought_to_bindingWan_enable; //learned in LAN but come back in WAN
	rtk_rg_enable_t pppoe_mc_routing_trap; //trap PPPoE DS mutlicast routing packet (DA=gmac, DIP=multicast)
#if defined(RTK_RG_INGRESS_QOS_TEST_PATCH) && defined(CONFIG_RTL9600_SERIES)
	int qos_type;
	int qos_acl_patch[RTK_RG_MAC_PORT_MAX][9];
	int qos_acl_total_patch;
	int qos_ingress_rate[RTK_RG_MAC_PORT_MAX];
	int qos_ingress_total_rate;
#endif
	int acl_filter_idx_for_hwnat;		//Default set to 0 now
	int not_disconnect_ppp;		//use for callback delete interface, in case "Server disconnect ppp" will not need to call spppctl.  
#ifdef CONFIG_RG_CALLBACK
	int callback_regist; //for check callback function is registered, then /bin/spppctl & /bin/udhcpc  should not do hwnat again! : Chuck
#endif	
	rtk_rg_stormControlInfo_t  stormControlInfoEntry[MAX_STORMCONTROL_ENTRY_SIZE];
	rtk_rg_qosInternalDecision_t qosInternalDecision;
	
	rtk_rg_acl_reserved_global_t aclAndCfReservedRule;
#if defined(CONFIG_RTL9602C_SERIES)
	int mib_l2_wanif_idx;
	int mib_mc_wanif_idx;
	int cf_pattern0_size;
#endif

	unsigned int tpid;	//record SVLAN VS_TPID 
	unsigned int tpid2;	//record SVLAN VS_TPID2
	unsigned int tpid2_en;	//record SVLAN VS_TPID2 enabled or not
	rtk_rg_portmask_t service_pmsk;				//record Service PortMask
	rtk_rg_portmask_t svlan_EP_DMAC_CTRL_pmsk;	//record SVLAN_EP_DMAC_CTRL PortMask

	//timeout
#if defined(CONFIG_ROME_NAPT_SHORTCUT)
	int v4ShortCut_timeout;
	int v4Shoutcut_HouseKeepIndex; //indicate the index last house keep end
#endif
#if defined(CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT)
	int v6ShortCut_timeout;
	int v6Shoutcut_HouseKeepIndex; //indicate the index last house keep end
#endif
	int arp_timeout;
	int neighbor_timeout;
	int tcp_long_timeout;
	int tcp_short_timeout;
	int udp_long_timeout;
	int udp_short_timeout;
#if defined(CONFIG_RTL9600_SERIES)
#else	//support lut traffic bit
	int l2_timeout;
#endif

	//Disable TCP stateful tracking, create connection while any packet send from LAN.
	unsigned int tcpDisableStatefulTracking:1;

	//Keep original cvlan for packets from protocol stack (can be modified by ACL/CF)
	unsigned int keepPsOrigCvlan:1;

	//pppoe proxy only accept dial packet which is binding to pppoe WAN
	unsigned int pppoeProxyAllowBindingOnly:1;

	//Choice other inbound hashIdx when inbound 4-way full.
	uint32 enableL4ChoiceHwIn;
	uint32 tcpDoNotDelWhenRstFin;

	//NAPT short-timeout house keep
	uint32 tcpShortTimeoutHousekeepJiffies;  //0:disable 

	// statistic
	uint32 fwdStatistic;

	//ARP Request Timer
	int arp_requset_interval_sec;
	
	//HouseKeeping Timer
	int house_keep_sec;

	//IGMP Snooping Timer
	int igmp_sys_timer_sec;

	//IGMP max flow size
	int igmp_max_simultaneous_group_size;

	//IGMP current occupied flow size
	int igmp_simultaneous_group_size;

	//Multicast Query Timer
	int mcast_query_sec;

	//User defined force Client report time after query send
	int forceReportResponseTime;

	// TCP hw learning at SYN
	int tcp_hw_learning_at_syn;

	// TCP inbound shortcut learning at SYN
	unsigned int tcp_in_shortcut_learning_at_syn:1;

#ifdef CONFIG_ROME_NAPT_SHORTCUT
	//IPv4 shortcut switch
	unsigned int ipv4_shortcut_off:1;
#endif
#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
	//IPv6 shortcut switch
	unsigned int ipv6_shortcut_off:1;
#endif

	unsigned int gatherLanNetInfo:1;

	//software port isolation
	rtk_rg_portmask_t portIsolation[RTK_RG_PORT_MAX];
	
	//software rate limit
	int BCRateLimitPortMask;
	int BCRateLimitShareMeterIdx; // -1:diable rate limit,  0~31: shareMeter index
	int BCByteCount;
	int IPv6MCRateLimitPortMask;
	int IPv6MCRateLimitShareMeterIdx; // -1:diable rate limit,  0~31: shareMeter index
	int IPv6MCByteCount;
	int IPv4MCRateLimitPortMask;
	int IPv4MCRateLimitShareMeterIdx; // -1:diable rate limit,  0~31: shareMeter index
	int IPv4MCByteCount;
	int unKnownDARateLimitPortMask;
	int unKnownDARateLimitShareMeterIdx; // -1:diable rate limit,  0~31: shareMeter index
	int unKnownDAByteCount;
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	int wifiIngressRateLimitDevMask;
	int wifiIngressRateLimitDevOverMask;	//record which device had over meter
	int wifiIngressRateLimitMeter[MAX_WLAN_DEVICE_NUM];
	int wifiIngressByteCount[MAX_WLAN_DEVICE_NUM];
	int wifiEgressRateLimitDevMask;
	int wifiEgressRateLimitDevOverMask;	//record which device had over meter
	int wifiEgressRateLimitMeter[MAX_WLAN_DEVICE_NUM];
	int wifiEgressByteCount[MAX_WLAN_DEVICE_NUM];
#endif

	//L2 Interface p-bit remarking (Using QoS lutFwd to acheived)
	int IntfRmkEnabled;	//The mechanism enabed or not, 0:disabled  1:enabled
	int intfMappingToPbit[MAX_NETIF_SW_TABLE_SIZE]; //Each Interfcae default remark p-bit value;

	//ACL have to compare wlanDev. Only pure software can do this pattern.(skip HWLOOKUP if valid)
	int wlanDevPatternValidInACL;

	//trap ACL IP range pattern with drop/permit action, let fwdEngine handle it (the trapped IP range could be larger)
	int aclDropIpRangeBySwEnable;
	int aclPermitIpRangeBySwEnable;

	// NAPT hash max ways.
	uint32	enableL4MaxWays;
	uint32	enableL4WaysList;	
	uint8	l4InboundMaxWays[MAX_NAPT_IN_HW_TABLE_SIZE>>2];
	uint8	l4OutboundMaxWays[MAX_NAPT_OUT_HW_TABLE_SIZE>>2];

	// Congestion ctrl timer
	uint32 congestionCtrlIntervalMicroSecs; //0:disable
	uint32 congestionCtrlPortMask; //0:disable
	uint32 congestionCtrlSendTimesPerPort;
	uint32 congestionCtrlSendBytesPerSec[MAX_CONGESTION_CTRL_PORTS];
	uint32 congestionCtrlSendBytesInterval[MAX_CONGESTION_CTRL_PORTS];
	uint32 congestionCtrlInboundAckToHighQueue;
	uint32 congestionCtrlSendRemainderInNextGap;

	//ARP traffic table reference switch
	unsigned int arp_traffic_off:1;
	unsigned int arp_max_request_count;

	//WIFI TX redirect to port0
	uint32 wifiTxRedirect; //0:disable

	//pon port unmatch drop
	uint32 ponPortUnmatchCfDrop;

	//Protocol-Stack RX mirror to port0
	uint32 psRxMirrorToPort0;

	//igmp report packet egress filter portmask
	uint32 igmpReportPortmask;

	//igmp report packet ingress filter portmask
	uint32 igmpReportIngressPortmask;
	
	//igmp query packet filter portmask
	uint32 igmpMldQueryPortmask;

	//proc support read to pipe
	uint32 proc_to_pipe;

#ifdef __KERNEL__	
	struct tasklet_struct congestionCtrlTasklets;
#endif
	uint8 bridge_netIfIdx_drop_by_portocal[MAX_NETIF_SW_TABLE_SIZE]; //0:Both pass, 1:Drop IPv6(IPv4 Pass) 2:Drop IPv4(IPv6 Pass)

	//Over SRAM===================================================================
	rtk_rg_pktHdr_t	pktHeader_2;
	rtk_rg_pktHdr_t	pktHeader_broadcast;
	rtk_rg_statistic_t statistic;

	//maintain ACL in fwdEngine
	rtk_rg_aclFilterEntry_t acl_SW_table_entry[MAX_ACL_SW_ENTRY_SIZE]; //record all ACL informations(including RG_ACL & HW_ACL), the array index is the RG_ACL index
	rtk_rg_aclFilterAndQos_t acl_filter_temp[MAX_ACL_SW_ENTRY_SIZE]; //for acl del entry using
	rtk_enable_t check_acl_priority_action_for_rg_acl_of_l34_type; //0:all acl priority action is in fwdtype= ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET , _rtk_rg_ingressACLAction() can speed up    1 : acl priority action exist in fwdtype=L34 related type, _rtk_rg_ingressACLAction() have to check all rules.

	//maintain CF in fwdEngine
	rtk_rg_classifyEntry_t classify_SW_table_entry[TOTAL_CF_ENTRY_SIZE];


	//maintain naptPriority in fwdEngine (using link list)
	rtk_rg_sw_naptFilterAndQos_t napt_SW_table_entry[MAX_NAPT_FILER_SW_ENTRY_SIZE];
	rtk_rg_sw_naptFilterAndQos_t *pValidUsNaptPriorityRuleStart;
	rtk_rg_sw_naptFilterAndQos_t *pValidDsNaptPriorityRuleStart;

	//maintain Gpon Broadcast to Lan in fwdEngine
	rtk_rg_sw_gpon_ds_bc_vlanfilterAndRemarking_t gpon_SW_ds_bc_filter_table_entry[MAX_GPON_DS_BC_FILTER_SW_ENTRY_SIZE];
	

	rtk_rg_urlFilterEntry_t urlFilter_table_entry[MAX_URL_FILTER_ENTRY_SIZE];
	char urlFilter_parsingBuf[MAX_URL_FILTER_BUF_LENGTH];

	rtk_rg_forcePortalURL_t forcePortal_url_list[MAX_FORCE_PORTAL_URL_NUM+1];

	//dhcp trap
	int hwAclIdx_for_trap_dhcp[MAX_NETIF_SW_TABLE_SIZE]; //each intf can reserved one acl to trap DHCP packet to PS. 
	
	int dsliteControlSet[L34_DSLITE_CTRL_END];

	rtk_rg_ipsec_passthru_t		ipsec_passthru;
	rtk_rg_portmask_t stpBlockingPortmask;	
	rtk_rg_cached_port_isolation_t storedInfo;	//cached isolation value

	//gponDsBCModule (for gpon downstream braodcast by port remarking vlan)
	uint32 gponDsBCModuleEnable;
	
	rtk_rg_portmask_t virtualMAC_with_PON_switch_mask;
	int32 virtualMAC_with_PON_cputag_reg;		//keep the MAC_CPU_TAG_CTRL setting and restore when the function is disabled

	rtk_rg_internal_support_bit_t internalSupportMask;
	
	uint32 demo_dpiPreRouteCallback_retValue;
	uint32 demo_dpiFwdCallback_retValue;

	//debug tool for dump RG ACL parameter when calling add
	uint32 acl_rg_add_parameter_dump;

	//force diabled the pon port dmac2cvid
	unsigned int dmac2cvidDisabledPortmask;
	
} rtk_rg_glb_system_t;


typedef struct rtk_rg_tcpShortTimeout_s
{
	uint32 timeoutJiffies;
	uint16 naptOutIdx;
} rtk_rg_tcpShotTimeout_t;


typedef struct rtk_rg_fragment_queue_s
{

	struct sk_buff *queue_skb;		
	struct rx_info queue_rx_info;
	struct re_private queue_cp;


	rtk_rg_pktHdr_t queue_pktHdr;
	unsigned long queue_time;		//jiffies
	int8 occupied;
	rtk_rg_naptDirection_t direction;
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	rtk_l34_nexthop_type_t	wanType;
#endif
} rtk_rg_fragment_queue_t;

typedef struct rtk_rg_ipv4_fragment_out_s
{
	union{
		struct{
			int NaptOutboundEntryIndex;
			ipaddr_t intIp;
		}napt;	//for TCP, UDP
		struct{
			ipaddr_t intIp;
		}icmp;
	}pktInfo;
	rtk_rg_pkthdr_tagif_t layer4Type;
	rtk_rg_fwdEngineReturn_t fragAction;
	unsigned long beginIdleTime;
	unsigned short receivedLength;	//already received packet length
	unsigned short totalLength;	//calculated by last fragment
	unsigned char queueCount;
	unsigned short identification;		//IP header's identification
	
	struct rtk_rg_ipv4_fragment_out_s *pNext,*pPrev;
} rtk_rg_ipv4_fragment_out_t;

typedef struct rtk_rg_ipv4_fragment_in_s
{
	union{
		struct{
			int NaptOutboundEntryIndex;
			ipaddr_t remoteIp;
		}napt;	//for TCP, UDP
		struct{
			ipaddr_t remoteIp;
			ipaddr_t intIp;
		}icmp;
	}pktInfo;
	rtk_rg_pkthdr_tagif_t layer4Type;
	rtk_rg_fwdEngineReturn_t fragAction;
	unsigned long beginIdleTime;
	unsigned short receivedLength;	//already received packet length
	unsigned short totalLength;	//calculated by last fragment
	unsigned char queueCount;
	unsigned short identification;		//IP header's identification
	
	struct rtk_rg_ipv4_fragment_in_s *pNext,*pPrev;
} rtk_rg_ipv4_fragment_in_t;

typedef struct rtk_rg_saLearningLimitInfo_s
{	
	int learningLimitNumber;
	rtk_rg_sa_learning_exceed_action_t action;
}rtk_rg_saLearningLimitInfo_t;

typedef struct rtk_rg_accessWanLimitData_s
{
	rtk_rg_accessWanLimitType_t type;
	union
	{
		unsigned char category;
		rtk_rg_portmask_t port_mask;
	}data;
	int learningLimitNumber;
	int learningCount;
	rtk_rg_sa_learning_exceed_action_t action;
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	unsigned int wlan0_dev_mask;	//used for WLAN0 device access limit
#endif
}rtk_rg_accessWanLimitData_t;

typedef struct rtk_rg_accessWanLimitCategory_s
{
	unsigned char category;
	rtk_mac_t mac;
}rtk_rg_accessWanLimitCategory_t;

typedef struct rtk_rg_saLearningLimitProbe_s
{
#ifdef __KERNEL__
	struct timer_list timer;
#endif
	short l2Idx;
	short arpIdx;
	char arpCounter;
	rtk_rg_arp_request_t arpReq;
	short neighborIdx;
	char neighborCounter;
	rtk_rg_neighbor_discovery_t neighborReq;
	ipaddr_t v4IP;
	rtk_ipv6_addr_t v6IP;
	int v6Route;
	atomic_t activity;
}rtk_rg_saLearningLimitProbe_t;




typedef struct rtk_rg_table_ipfilter_s
{
	ipaddr_t filterIp;	
	int valid;
} rtk_rg_table_ipfilter_t;



typedef struct rtk_rg_table_ipmcgrp_s
{
	ipaddr_t groupIp;	
	rtk_portmask_t portMsk;
	int valid;
} rtk_rg_table_ipmcgrp_t;

typedef struct rtk_rg_table_wlan_mbssid_s
{
	rtk_mac_t			mac;
	int					wlan_dev_idx;
	int					vlan_tag_if; //support for WIFI DMAC2CVID
	int					vid; //support for WIFI DMAC2CVID
	long unsigned int	learn_jiffies;
} rtk_rg_table_wlan_mbssid_t;

typedef struct rtk_rg_nexthop_lan_host_s
{
	ipaddr_t ipAddr;
	rtk_mac_t macAddr;
	rtk_rg_mac_port_idx_t port;

	int rtIdx;
	int intfIdx;
	int macIdx;
	
	unsigned int valid:1;
}rtk_rg_nexthop_lan_host_t;

/* End of System ============================================================== */

typedef struct rtk_rg_globalDatabase_cache_s
{
	rtk_rg_pktHdr_t	pktHeader_1;
	rtk_rg_pktHdr_t	pktHeader_2;
	struct rx_info rxInfoFromWLAN; //for WIFI to fwdEngineInput
}rtk_rg_globalDatabase_cache_t;

typedef struct rtk_rg_congestionCtrlRing_s
{
	struct tx_info ptxInfo;
	struct tx_info ptxInfoMsk;
	struct sk_buff *pSkb;
} rtk_rg_congestionCtrlRing_t;

typedef struct rtk_rg_globalDatabase_s
{
	//20140819LUKE: only first 32KB reside in sram!
	rtk_rg_pktHdr_t *pktHdr;
	rtk_rg_glb_system_t systemGlobal;
	
	//rtk_rg_table_mac_t 			mac[MAX_LUT_SW_TABLE_SIZE];
	rtk_rg_table_vlan_t			vlan[MAX_VLAN_SW_TABLE_SIZE];
	rtk_rg_table_lut_t			lut[MAX_LUT_SW_TABLE_SIZE];
	rtk_rg_table_netif_t		netif[MAX_NETIF_SW_TABLE_SIZE];
	rtk_rg_table_l3_t			l3[MAX_L3_SW_TABLE_SIZE];
	rtk_rg_table_extip_t		extip[MAX_EXTIP_SW_TABLE_SIZE];
	rtk_rg_table_nexthop_t		nexthop[MAX_NEXTHOP_HW_TABLE_SIZE];
	rtk_rg_table_pppoe_t		pppoe[MAX_PPPOE_SW_TABLE_SIZE];	
	rtk_rg_table_arp_t			arp[MAX_ARP_SW_TABLE_SIZE];
	rtk_rg_table_wantype_t  	wantype[MAX_WANTYPE_SW_TABLE_SIZE];
	rtk_rg_table_naptIn_t		naptIn[MAX_NAPT_IN_SW_TABLE_SIZE];	
	rtk_rg_table_naptOut_t		naptOut[MAX_NAPT_OUT_SW_TABLE_SIZE];
	rtk_rg_table_bind_t 		bind[MAX_BIND_SW_TABLE_SIZE];	
	rtk_rg_table_v6route_t 		v6route[MAX_IPV6_ROUTING_SW_TABLE_SIZE];
	rtk_rg_table_v6ExtIp_t		v6Extip[MAX_NETIF_SW_TABLE_SIZE];//1-to-1 mapping to interface table
	rtk_rg_table_v6neighbor_t	v6neighbor[MAX_IPV6_NEIGHBOR_SW_TABLE_SIZE];
	rtk_rg_table_ipfilter_t		ipmcfilter[MAX_IPMCFILTER_HW_TABLE_SIZE];
	rtk_rg_table_ipmcgrp_t		ipmcgrp[MAX_IPMCGRP_HW_TABLE_SIZE];
	rtk_rg_table_wlan_mbssid_t	wlanMbssid[MAX_WLAN_MBSSID_SW_TABLE_SIZE];
	int wlanMbssidHeadIdx; //the first lookup entry of the wlanMbssid table.
#if defined(CONFIG_RTL9602C_SERIES)
	rtk_rg_table_dslite_t		dslite[MAX_DSLITE_SW_TABLE_SIZE];
#endif
	rtk_rg_table_dsliteMc_t		dsliteMc[MAX_DSLITEMC_SW_TABLE_SIZE];
	
	rtk_rg_table_naptOut_linkList_t	*pNaptOutFreeListHead;
	rtk_rg_table_naptOut_linkList_t	naptOutFreeList[MAX_NAPT_OUT_SW_TABLE_SIZE-MAX_NAPT_OUT_HW_TABLE_SIZE];
	rtk_rg_table_naptOut_linkList_t	*pNaptOutHashListHead[MAX_NAPT_OUT_HW_TABLE_SIZE>>2]; 
	
	rtk_rg_table_naptIn_linkList_t	*pNaptInFreeListHead;
	rtk_rg_table_naptIn_linkList_t	naptInFreeList[MAX_NAPT_IN_SW_TABLE_SIZE-MAX_NAPT_IN_HW_TABLE_SIZE];
	rtk_rg_table_naptIn_linkList_t	*pNaptInHashListHead[MAX_NAPT_IN_HW_TABLE_SIZE>>2]; 

	rtk_rg_table_icmp_linkList_t *pICMPCtrlFlowHead;
	rtk_rg_table_icmp_linkList_t icmpCtrlFlowLinkList[MAX_ICMPCTRLFLOW_SIZE];

	uint32	naptTcpExternPortUsed[65536/32]; // naptTcpExternPortUsed[0] bit0==>TCP PORT 0, bit31==>TCP PORT 31, naptTcpExternPortUsed[1] bit0==>TCP PORT 32...
	uint32	naptUdpExternPortUsed[65536/32]; // naptUdpExternPortUsed[0] bit0==>UDP PORT 0, bit31==>UDP PORT 31, naptTcpExternPortUsed[1] bit0==>UDP PORT 32...
	uint8	naptTcpExternPortUsedRefCount[65536];
	uint8	naptUdpExternPortUsedRefCount[65536];
#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
	uint32	ipv6naptTcpExternPortUsed[65536/32]; // naptTcpExternPortUsed[0] bit0==>TCP PORT 0, bit31==>TCP PORT 31, naptTcpExternPortUsed[1] bit0==>TCP PORT 32...
	uint32	ipv6naptUdpExternPortUsed[65536/32]; // naptUdpExternPortUsed[0] bit0==>UDP PORT 0, bit31==>UDP PORT 31, naptTcpExternPortUsed[1] bit0==>UDP PORT 32...
#endif
	
	atomic_t naptProtcolStackEntryNumber[2]; //Array 0 for UDP, array 1 for TCP
	atomic_t naptForwardEngineEntryNumber[2]; //Array 0 for UDP, array 1 for TCP
	uint32 longestIdleSecs;
	int32 longestIdleNaptIdx;
	uint8 portLinkStatusInitDone;
	uint32 portLinkupMask;

#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	struct list_head 				ipv6Layer4FreeListHead;
	rtk_rg_ipv6_layer4_linkList_t	ipv6Layer4FreeList[MAX_IPV6_STATEFUL_TABLE_SIZE];
	struct list_head				ipv6Layer4HashListHead[MAX_IPV6_STATEFUL_HASH_HEAD_SIZE];

	//fragment handling
	rtk_rg_fragment_queue_t 		ipv6FragmentQueue[MAX_IPV6_FRAGMENT_QUEUE_SIZE];
#endif

	//Routing and ARP related
	int routingArpInfoNum;	//numer of routing entry in routingArpInfoArray
	int *p_tempRoutingVlanInfoArray;
	int *p_routingVlanInfoArray;
	int routingVlanInfoArray_1[MAX_L3_SW_TABLE_SIZE];
	int routingVlanInfoArray_2[MAX_L3_SW_TABLE_SIZE];
	rtk_rg_routing_arpInfo_t *p_routingArpInfoArray;
	rtk_rg_routing_arpInfo_t *p_tempRoutingArpInfoArray;
	rtk_rg_routing_arpInfo_t routingArpInfoArray_1[MAX_L3_SW_TABLE_SIZE];		//used in ARP rearrangement mechanism
	rtk_rg_routing_arpInfo_t routingArpInfoArray_2[MAX_L3_SW_TABLE_SIZE];		//used in ARP rearrangement mechanism
	unsigned char arpTableCopied[MAX_ARP_HW_TABLE_SIZE];
	rtk_rg_table_arp_t tempArpTable[MAX_ARP_HW_TABLE_SIZE];
	rtk_l34_routing_entry_t tempL3Table[MAX_L3_SW_TABLE_SIZE];
	
	uint32	arpTrfIndicator[512/32];//arpTrfIndicator[0] bit0==>ARP[0], bit31==>ARP[31], arpTrfIndicator[1] bit0==>ARP[32]...
	
	uint32	naptValidSet[MAX_NAPT_OUT_SW_TABLE_SIZE/32];
	uint32	arpValidSet[MAX_ARP_HW_TABLE_SIZE/32];
	uint32	neighborValidSet[MAX_IPV6_NEIGHBOR_HW_TABLE_SIZE/32];
#if defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
	uint32	lutValidSet[(MAX_OF_RTL865x_L2TBL_ROW*RTL8651_L2TBL_COLUMN)/32];
#else
	uint32	lutValidSet[MAX_LUT_HW_TABLE_SIZE/32];
#endif
	rtk_rg_upnpConnection_t upnp[MAX_UPNP_SW_TABLE_SIZE];
	rtk_rg_virtualServer_t virtualServer[MAX_VIRTUAL_SERVER_SW_TABLE_SIZE];
#ifdef CONFIG_RG_NAPT_DMZ_SUPPORT
	rtk_rg_dmzInfo_t dmzInfo[MAX_DMZ_TABLE_SIZE];
#endif	
	rtk_rg_gatewayServicePortEntry_t gatewayServicePortEntry[MAX_GATEWAYSERVICEPORT_TABLE_SIZE];
	rtk_rg_fragment_queue_t ipv4FragmentQueue[MAX_IPV4_FRAGMENT_QUEUE_SIZE];
	//rtk_rg_fragment_napt_out_t ipv4FragmentOutTable[MAX_NAPT_OUT_HW_TABLE_SIZE];
	rtk_rg_ipv4_fragment_out_t	*pFragOutFreeListHead;
	rtk_rg_ipv4_fragment_out_t	fragOutFreeList[MAX_FRAG_OUT_FREE_TABLE_SIZE];
	rtk_rg_ipv4_fragment_out_t	*pFragOutHashListHead[MAX_NAPT_OUT_HW_TABLE_SIZE>>2]; 	//4-way hashed, we just need MAX_NAPT_OUT_HW_TABLE_SIZE/4 indexes
	
	rtk_rg_ipv4_fragment_in_t	*pFragInFreeListHead;
	rtk_rg_ipv4_fragment_in_t	fragInFreeList[MAX_FRAG_IN_FREE_TABLE_SIZE];
	rtk_rg_ipv4_fragment_in_t	*pFragInHashListHead[MAX_NAPT_IN_HW_TABLE_SIZE>>2];		//4-way hashed, we just need MAX_NAPT_IN_HW_TABLE_SIZE/4 indexes
#ifdef CONFIG_ROME_NAPT_SHORTCUT
	uint32 v4ShortCutValidSet[MAX_NAPT_SHORTCUT_SIZE/32];
	uint8 v4ShortCut_lastAddIdx[MAX_NAPT_SHORTCUT_SIZE>>MAX_NAPT_SHORTCUT_WAYS_SHIFT]; //last added index of ipv4 shortcut in the same way
	rtk_rg_napt_shortcut_t naptShortCut[MAX_NAPT_SHORTCUT_SIZE];
#endif
#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
	uint32 v6ShortCutValidSet[MAX_NAPT_V6_SHORTCUT_SIZE/32];
	uint8 v6ShortCut_lastAddIdx[MAX_NAPT_V6_SHORTCUT_SIZE>>MAX_NAPT_V6_SHORTCUT_WAYS_SHIFT]; //last added index of ipv6 shortcut in the same way
	rtk_rg_naptv6_shortcut_t naptv6ShortCut[MAX_NAPT_V6_SHORTCUT_SIZE];
#endif
	
	struct list_head softwareArpTableHead[MAX_ARP_SW_TABLE_HEAD];		//indexed by IP&0xff
	struct list_head softwareArpFreeListHead;
	rtk_rg_arp_linkList_t softwareArpFreeList[MAX_ARP_SW_TABLE_SIZE-MAX_ARP_HW_TABLE_SIZE];
#if defined(CONFIG_RTL9602C_SERIES)	
	struct list_head hardwareArpTableHead[MAX_ARP_HW_TABLE_HEAD];		//indexed by IP&0xff
	struct list_head hardwareArpFreeListHead;
	rtk_rg_arp_linkList_t hardwareArpFreeList[MAX_ARP_HW_TABLE_SIZE];
#endif	
	struct list_head softwareLutTableHead[MAX_LUT_SW_TABLE_HEAD];		//indexed by HASH(IVL or SVL)
	struct list_head softwareLutFreeListHead;
	rtk_rg_lut_linkList_t softwareLutFreeList[MAX_LUT_SW_TABLE_SIZE-MAX_LUT_HW_TABLE_SIZE];
	
#if defined(CONFIG_RTL9600_SERIES)
	struct list_head lutBCAMLinkListHead;
	struct list_head lutBCAMChosenLinkListHead; 	//the chosen victim will add to this list
#else	//support lut traffic bit
	struct list_head lutBCAMFreeListHead;
#if defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
	struct list_head lutBCAMTableHead[((MAX_OF_RTL865x_L2TBL_ROW*RTL8651_L2TBL_COLUMN)-MAX_LUT_BCAM_TABLE_SIZE)>>2];		//indexed by L2 hash 
#else
	struct list_head lutBCAMTableHead[(MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE)>>2];		//indexed by L2 hash 	
#endif
#endif
	rtk_rg_lut_linkList_t lutBCAMLinkList[MAX_LUT_BCAM_TABLE_SIZE];

	struct list_head vlanBindingListHead[RTK_RG_PORT_MAX];
	struct list_head vlanBindingFreeListHead;
	struct list_head vmacSkbListHead;		//store the virtualMAC skb for examination of PON's funcitionality
	struct list_head vmacSkbListFreeListHead;

	rtk_rg_redirectHttpAll_t redirectHttpAll;
	rtk_rg_redirectHttpURL_linkList_t redirectHttpURLFreeList[MAX_REDIRECT_URL_NUM];
	rtk_rg_redirectHttpWhiteList_linkList_t redirectHttpWhiteListFreeList[MAX_REDIRECT_WHITE_LIST_NUM];
	struct list_head redirectHttpURLListHead;
	struct list_head redirectHttpURLFreeListHead;
	struct list_head redirectHttpWhiteListListHead;
	struct list_head redirectHttpWhiteListFreeListHead;
	
	rtk_rg_vbind_linkList_t	vlanBindingFreeList[MAX_BIND_SW_TABLE_SIZE];
#if defined(CONFIG_APOLLO_GPON_FPGATEST)
	rtk_rg_vmac_skb_linlList_t vmacSkbFreeList[MAX_VMAC_SKB_QUEUE_SIZE];
#endif

#if defined(CONFIG_RTL9600_SERIES) 
	unsigned char layer2NextOfNewestCountIdx[(MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE)>>2];
#endif

	//ALG module
	rtk_rg_alg_type_t algFunctionMask;	//1: enable, 0: disable
	rtk_rg_alg_type_t algServInLanIpMask;	//1: ip setuped, 0: no ip
	rtk_rg_alg_serverIpMapping_t algServInLanIpMapping[MAX_ALG_SERV_IN_LAN_NUM];
	uint32	algTcpExternPortEnabled[65536/32]; // algTcpExternPortEnabled[0] bit0==>TCP PORT 0 has alg, bit31==>TCP PORT 31 has alg, algTcpExternPortEnabled[1] bit0==>TCP PORT 32 has alg...
	uint32	algUdpExternPortEnabled[65536/32]; // algUdpExternPortEnabled[0] bit0==>UDP PORT 0 has alg, bit31==>UDP PORT 31 has alg, algUdpExternPortEnabled[1] bit0==>UDP PORT 32 has alg...
	uint32	algTcpExternPortEnabled_SrvInLan[65536/32]; // algTcpExternPortEnabled_SrvInLan[0] bit0==>TCP PORT 0 has alg, bit31==>TCP PORT 31 has alg, algTcpExternPortEnabled_SrvInLan[1] bit0==>TCP PORT 32 has alg...
	uint32	algUdpExternPortEnabled_SrvInLan[65536/32]; // algUdpExternPortEnabled_SrvInLan[0] bit0==>UDP PORT 0 has alg, bit31==>UDP PORT 31 has alg, algUdpExternPortEnabled_SrvInLan[1] bit0==>UDP PORT 32 has alg...
	rtk_rg_alg_funcMapping_t algTcpFunctionMapping[MAX_ALG_FUNCTIONS];
	rtk_rg_alg_funcMapping_t algUdpFunctionMapping[MAX_ALG_FUNCTIONS];
	uint32  algUserDefinedPort[MAX_ALG_FUNCTIONS];
	int		algUserDefinedTimeout[MAX_ALG_FUNCTIONS];

	//Dynamic Port ALG assign
	rtk_rg_alg_dynamicPort_t algDynamicFreeList[MAX_ALG_DYNAMIC_PORT_NUM];
	struct list_head algDynamicFreeListHead;
	struct list_head algDynamicCheckListHead;

	//tcpShortTimeoutHouseKeeping
	rtk_rg_tcpShotTimeout_t tcpShortTimeoutRing[MAX_NAPT_OUT_SW_TABLE_SIZE];
	int tcpShortTimeoutFreedIdx;
	int tcpShortTimeoutRecycleIdx;

	//Congestion control timer
	uint32 congestionCtrlQueueIdx[2][MAX_CONGESTION_CTRL_PORTS]; //only support port 0~3
	uint32 congestionCtrlSendIdx[2][MAX_CONGESTION_CTRL_PORTS];
	uint32 congestionCtrlQueueCounter[2][MAX_CONGESTION_CTRL_PORTS];
	uint32 congestionCtrlMaxQueueCounter[2][MAX_CONGESTION_CTRL_PORTS];
	uint32 congestionCtrlFullDrop[2][MAX_CONGESTION_CTRL_PORTS];
	uint32 congestionCtrlSendedRemainder[MAX_CONGESTION_CTRL_PORTS];	
	rtk_rg_congestionCtrlRing_t congestionCtrlRing[2][MAX_CONGESTION_CTRL_PORTS][MAX_CONGESTION_CTRL_RING_SIZE]; //[0][][]=low queue, [1][][]=high queue.

	//Nexthop LAN data
	rtk_rg_nexthop_lan_host_t nexthop_lan_table[MAX_NETIF_SW_TABLE_SIZE];
	
#if defined(CONFIG_APOLLO_ROMEDRIVER) || defined(CONFIG_XDSL_ROMEDRIVER)
	//LUKE20130816: move to separate ALG file
#if 0
	//initialized and used at fwdEngine only
	rtk_rg_ftpCtrlFlowEntry_t *pAlgFTPCtrlFlowHead;
	rtk_rg_ftpCtrlFlowEntry_t algFTPCtrlFlowList[MAX_FTP_CTRL_FLOW_SIZE];
#endif
		
	//rtk_rg_pptpGreEntry_t *pptpGreInboundHeadPtr[MAX_PPTP_TBL_SIZE]; 
//20130821LUKE:close it because implemented by separate module
#if 0
	rtk_rg_pptpGreLinkList_t * pPPTPGreOutboundHead[MAX_NETIF_SW_TABLE_SIZE];
	rtk_rg_pptpGreLinkList_t pptpGreOutboundLinkList[MAX_NETIF_SW_TABLE_SIZE][MAX_PPTP_SESSION_SIZE];
	uint32	algPPTPExtCallIDEnabled[65536/32]; // algPPTPExtCallIDEnabled[0] bit0==>callID 0 has been used, bit31==> callID 31 has been used, algPPTPExtCallIDEnabled[1] bit0==>callID 32 has been used...	
#endif

#endif
	rtk_rg_alg_l2tp_linkList_t *pAlgL2TPCtrlFlowHead;
	rtk_rg_alg_l2tp_linkList_t algL2TPCtrlFlowLinkList[MAX_L2TP_CTRL_FLOW_SIZE];
	uint32	algL2TPExternTulIDUsed[65536/32]; // l2tpExternTulIDUsed[0] bit0==>TunnelID 0, bit31==>TunnelID 31, l2tpExternTulIDUsed[1] bit0==>TunnelID 32...
	
#ifdef CONFIG_RG_ROMEDRIVER_ALG_BATTLENET_SUPPORT
	ipaddr_t algBnetSCClient[RTK_RG_MAX_SC_CLIENT];
#endif
	//ACL decision backup
	rtk_rg_aclHitAndAction_t aclDecisionBackup;
} rtk_rg_globalDatabase_t;

//Global variable that used in Linux kernel or only modified when system begin
typedef struct rtk_rg_globalKernel_s
{
	rtk_rg_initState_t init_state;
	rtk_rg_debug_level_t debug_level;
	rtk_rg_debug_level_t filter_level;	//used to filter display message based on trace_filter
	rtk_rg_debug_trace_filter_bitmask_t trace_filter_bitmask[TRACFILTER_MAX];
	rtk_rgDebugTraceFilter_t trace_filter[TRACFILTER_MAX];
	uint32 traceFilterRuleMask; // enable trace filter rule mask 
	int l2_hw_aging;
	int cp3_execute_count;
	int cp3_execute_count_state;
	
	//int ppp_diaged[MAX_NETIF_HW_TABLE_SIZE];//record which intf is PPPoE & diag on,  use when init disconnect pppoe judgement

	struct rx_info rxInfoFromPS; //for Protocol Stack to fwdEngineInput
	struct rx_info rxInfoFromARPND; //for ARP or ND or Redirection to fwdEngineInput
	struct rx_info rxInfoFromIGMPMLD; //for IGMP or MLD to fwdEngineInput

	int timer_selector;

#ifdef __KERNEL__
#if defined(CONFIG_RTL8686NIC) || defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
	struct tx_info txDesc,txDescMask;
#else
	rtk_rg_txdesc_t txDesc,txDescMask;
#endif

	unsigned char protocolStackTxPortMask;	//keep the NIC dev_port_mapping from rtk_rg_fwdEngine_xmit to rtk_rg_fwdEngineInput
	struct proc_dir_entry *proc_rg;
	unsigned char proc_parsing_buf[512];
	struct timer_list fwdEngineHouseKeepingTimer;
	struct timer_list fwdEngineTcpShortTimeoutHouseKeepingTimer;
	struct timer_list fwdEngineCongestionCtrlTimer;
	struct timer_list arpRequestTimer[MAX_NETIF_HW_TABLE_SIZE<<1]; 	//over MAX_NETIF_SIZE belong to PPTP&L2TP
	int arpRequestTimerCounter[MAX_NETIF_HW_TABLE_SIZE<<1];	//over MAX_NETIF_SIZE belong to PPTP&L2TP
	struct timer_list neighborDiscoveryTimer[MAX_NETIF_HW_TABLE_SIZE<<1];		//over MAX_NETIF_SIZE belong to DSLITE
	int neighborDiscoveryTimerCounter[MAX_NETIF_HW_TABLE_SIZE<<1];	//over MAX_NETIF_SIZE belong to DSLITE
	struct timer_list swRateLimitTimer;
	struct timer_list mCastQuerytimer;

	//Mac Learning limit
	rtk_rg_saLearningLimitProbe_t lutReachLimit_port[RTK_RG_PORT_MAX];
	rtk_rg_saLearningLimitProbe_t lutReachLimit_portmask;
	rtk_rg_saLearningLimitProbe_t lutReachLimit_category[WanAccessCategoryNum];
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	rtk_rg_saLearningLimitProbe_t lutReachLimit_wlan0dev[MAX_WLAN_DEVICE_NUM];
#endif

	//struct semaphore wanStaticCalled;
	//struct semaphore wanDsliteCalled;
	//struct semaphore wanDHCPCalled;
	//struct semaphore wanPPPOEAfterCalled;
	//struct semaphore wanPPTPAfterCalled;
	//struct semaphore wanL2TPAfterCalled;
	//struct semaphore wanPPPOEDSLITEAfterCalled;
	//struct semaphore interfaceLock;			//used when interface add or del
	spinlock_t ipv4FragLock;			//used when ipv4 fragment packet add to or lookup table
	spinlock_t ipv4FragFreeLock;		//used when ipv4 fragment packet need to manipulate free list
	spinlock_t ipv4FragQueueLock;		//used when ipv4 fragment packet queue or dequeue
	spinlock_t igmpsnoopingLock;	
	spinlock_t initLock; 					//used when manipulate rg_kernel to mark as init transaction
	//struct semaphore saLearningLimitLock;		//used when check or add source address learning count
	//struct semaphore linkChangeHandlerLock;		//used when check and clear link-down indicator register
	spinlock_t naptTableLock;			//used when read/write napt table by rg API.
	
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	spinlock_t ipv6StatefulLock;		//used when access IPv6 stateful link-list data structure
	spinlock_t ipv6FragQueueLock;		//used when ipv6 fragment packet queue or dequeue
#endif
	spinlock_t algDynamicLock;
	unsigned long rg_lock_flags;

#else
	struct tx_info txDesc,txDescMask;
	//void *wanStaticCalled;
	//void *wanDsliteCalled;
	//void *wanDHCPCalled;
	//void *wanPPPOEAfterCalled;
	//void *wanPPTPAfterCalled;
	//void *wanL2TPAfterCalled;
	//void *wanPPPOEDSLITEAfterCalled;
	//void *interfaceLock;
	void *ipv4FragLock;
	void *ipv4FragFreeLock;
	void *ipv4FragQueueLock;
	void *igmpsnoopingLock;
	void *initLock;
	//void *saLearningLimitLock;
	//void *linkChangeHandlerLock;
	void *naptTableLock;
	
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	void *ipv6StatefulLock;
	void *ipv6FragQueueLock;
#endif
	void *algDynamicLock;
#endif
	unsigned int apolloChipId;
	unsigned int apolloRev;
	unsigned int apolloSubtype;

	unsigned short arp_number_for_LAN;
	unsigned short arp_number_for_WAN;

	rtk_rg_enable_t stag_enable;
	rtk_rg_enable_t layer2LookupMissFlood2CPU;

#if defined(CONFIG_RTL9602C_SERIES)
	int force_cf_pattern0_size_enable;
	int cf_pattern0_size; 
#endif


#ifdef CONFIG_SMP
	struct tasklet_struct rg_tasklets;
	struct rg_private rg_tasklet_data;
	spinlock_t rg_tasklet_queue_lock;
	atomic_t rg_tasklet_queue_entrance;
#endif	

	struct workqueue_struct *rg_callbackWQ;

}rtk_rg_globalKernel_t;

/* End of Tables ========================================================== */


struct platform
{
	int (*rtk_rg_api_module_init)(void);
	rtk_rg_err_code_t (*rtk_rg_driverVersion_get)(rtk_rg_VersionString_t *version_string);
	rtk_rg_err_code_t (*rtk_rg_initParam_get)(rtk_rg_initParams_t *init_param);
	rtk_rg_err_code_t (*rtk_rg_initParam_set)(rtk_rg_initParams_t *init_param);
	rtk_rg_err_code_t (*rtk_rg_lanInterface_add)(rtk_rg_lanIntfConf_t *lan_info,int *intf_idx);
//5	
	rtk_rg_err_code_t (*rtk_rg_wanInterface_add)(rtk_rg_wanIntfConf_t *wanintf, int *wan_intf_idx);
	rtk_rg_err_code_t (*rtk_rg_staticInfo_set)(int wan_intf_idx, rtk_rg_ipStaticInfo_t *static_info);
	rtk_rg_err_code_t (*rtk_rg_dhcpRequest_set)(int wan_intf_idx);
	rtk_rg_err_code_t (*rtk_rg_dhcpClientInfo_set)(int wan_intf_idx, rtk_rg_ipDhcpClientInfo_t *dhcpClient_info);
	rtk_rg_err_code_t (*rtk_rg_pppoeClientInfoBeforeDial_set)(int wan_intf_idx, rtk_rg_pppoeClientInfoBeforeDial_t *app_info);
//10
	rtk_rg_err_code_t (*rtk_rg_pppoeClientInfoAfterDial_set)(int wan_intf_idx, rtk_rg_pppoeClientInfoAfterDial_t *clientPppoe_info);
	rtk_rg_err_code_t (*rtk_rg_interface_del)(int lan_or_wan_intf_idx);
	rtk_rg_err_code_t (*rtk_rg_intfInfo_find)(rtk_rg_intfInfo_t *intf_info, int *valid_lan_or_wan_intf_idx);
	rtk_rg_err_code_t (*rtk_rg_cvlan_add)(rtk_rg_cvlan_info_t *cvlan_info);
	rtk_rg_err_code_t (*rtk_rg_cvlan_del)(int cvlan_id);
//15
	rtk_rg_err_code_t (*rtk_rg_cvlan_get)(rtk_rg_cvlan_info_t *cvlan_info);
	rtk_rg_err_code_t (*rtk_rg_vlanBinding_add)(rtk_rg_vlanBinding_t *vlan_binding_info, int *vlan_binding_idx);
	rtk_rg_err_code_t (*rtk_rg_vlanBinding_del)(int vlan_binding_idx);
	rtk_rg_err_code_t (*rtk_rg_vlanBinding_find)(rtk_rg_vlanBinding_t *vlan_binding_info, int *valid_idx);
	rtk_rg_err_code_t (*rtk_rg_algServerInLanAppsIpAddr_add)(rtk_rg_alg_serverIpMapping_t *srvIpMapping);
//20
	rtk_rg_err_code_t (*rtk_rg_algServerInLanAppsIpAddr_del)(rtk_rg_alg_type_t delServerMapping);
	rtk_rg_err_code_t (*rtk_rg_algApps_set)(rtk_rg_alg_type_t alg_app);
	rtk_rg_err_code_t (*rtk_rg_algApps_get)(rtk_rg_alg_type_t *alg_app);
	rtk_rg_err_code_t (*rtk_rg_dmzHost_set)(int wan_intf_idx, rtk_rg_dmzInfo_t *dmz_info);
	rtk_rg_err_code_t (*rtk_rg_dmzHost_get)(int wan_intf_idx, rtk_rg_dmzInfo_t *dmz_info);
//25
	rtk_rg_err_code_t (*rtk_rg_virtualServer_add)(rtk_rg_virtualServer_t *virtual_server, int *virtual_server_idx);
	rtk_rg_err_code_t (*rtk_rg_virtualServer_del)(int virtual_server_idx);
	rtk_rg_err_code_t (*rtk_rg_virtualServer_find)(rtk_rg_virtualServer_t *virtual_server, int *valid_idx);
	rtk_rg_err_code_t (*rtk_rg_aclFilterAndQos_add)(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx);
	rtk_rg_err_code_t (*rtk_rg_aclFilterAndQos_del)(int acl_filter_idx);
//30
	rtk_rg_err_code_t (*rtk_rg_aclFilterAndQos_find)(rtk_rg_aclFilterAndQos_t *acl_filter, int *valid_idx);
	rtk_rg_err_code_t (*rtk_rg_macFilter_add)(rtk_rg_macFilterEntry_t *macFilterEntry,int *mac_filter_idx);
	rtk_rg_err_code_t (*rtk_rg_macFilter_del)(int mac_filter_idx);
	rtk_rg_err_code_t (*rtk_rg_macFilter_find)(rtk_rg_macFilterEntry_t *macFilterEntry, int *valid_idx);
	rtk_rg_err_code_t (*rtk_rg_urlFilterString_add)(rtk_rg_urlFilterString_t *filter,int *url_idx);
//35
	rtk_rg_err_code_t (*rtk_rg_urlFilterString_del)(int url_idx);
	rtk_rg_err_code_t (*rtk_rg_urlFilterString_find)(rtk_rg_urlFilterString_t *filter, int *valid_idx);
	rtk_rg_err_code_t (*rtk_rg_upnpConnection_add)(rtk_rg_upnpConnection_t *upnp, int *upnp_idx);
	rtk_rg_err_code_t (*rtk_rg_upnpConnection_del)(int upnp_idx);
	rtk_rg_err_code_t (*rtk_rg_upnpConnection_find)(rtk_rg_upnpConnection_t *upnp, int *valid_idx);
//40
	rtk_rg_err_code_t (*rtk_rg_naptConnection_add)(rtk_rg_naptEntry_t *naptFlow, int *flow_idx);
	rtk_rg_err_code_t (*rtk_rg_naptConnection_del)(int flow_idx);
	rtk_rg_err_code_t (*rtk_rg_naptConnection_find)(rtk_rg_naptInfo_t *naptInfo,int *valid_idx);
	rtk_rg_err_code_t (*rtk_rg_multicastFlow_add)(rtk_rg_multicastFlow_t *mcFlow, int *flow_idx);
	rtk_rg_err_code_t (*rtk_rg_multicastFlow_del)(int flow_idx);
	/* martin ZHU add */
	rtk_rg_err_code_t (*rtk_rg_l2MultiCastFlow_add)(rtk_rg_l2MulticastFlow_t *l2McFlow,int *flow_idx);
//45
	rtk_rg_err_code_t (*rtk_rg_multicastFlow_find)(rtk_rg_multicastFlow_t *mcFlow, int *valid_idx);
	rtk_rg_err_code_t (*rtk_rg_macEntry_add)(rtk_rg_macEntry_t *macEntry, int *entry_idx);
	rtk_rg_err_code_t (*rtk_rg_macEntry_del)(int entry_idx);
	rtk_rg_err_code_t (*rtk_rg_macEntry_find)(rtk_rg_macEntry_t *macEntry,int *valid_idx);
	rtk_rg_err_code_t (*rtk_rg_arpEntry_add)(rtk_rg_arpEntry_t *arpEntry, int *arp_entry_idx);
//50
	rtk_rg_err_code_t (*rtk_rg_arpEntry_del)(int arp_entry_idx);
	rtk_rg_err_code_t (*rtk_rg_arpEntry_find)(rtk_rg_arpInfo_t *arpInfo,int *arp_valid_idx);
	rtk_rg_err_code_t (*rtk_rg_neighborEntry_add)(rtk_rg_neighborEntry_t *neighborEntry,int *neighbor_idx);
	rtk_rg_err_code_t (*rtk_rg_neighborEntry_del)(int neighbor_idx);
	rtk_rg_err_code_t (*rtk_rg_neighborEntry_find)(rtk_rg_neighborInfo_t *neighborInfo,int *neighbor_valid_idx);
//55
	rtk_rg_err_code_t (*rtk_rg_accessWanLimit_set)(rtk_rg_accessWanLimitData_t access_wan_info);
	rtk_rg_err_code_t (*rtk_rg_accessWanLimit_get)(rtk_rg_accessWanLimitData_t *access_wan_info);
	rtk_rg_err_code_t (*rtk_rg_accessWanLimitCategory_set)(rtk_rg_accessWanLimitCategory_t macCategory_info);
	rtk_rg_err_code_t (*rtk_rg_accessWanLimitCategory_get)(rtk_rg_accessWanLimitCategory_t *macCategory_info);
	rtk_rg_err_code_t (*rtk_rg_softwareSourceAddrLearningLimit_set)(rtk_rg_saLearningLimitInfo_t sa_learnLimit_info, rtk_rg_port_idx_t port_idx);
//60
	rtk_rg_err_code_t (*rtk_rg_softwareSourceAddrLearningLimit_get)(rtk_rg_saLearningLimitInfo_t *sa_learnLimit_info, rtk_rg_port_idx_t port_idx);
	rtk_rg_err_code_t (*rtk_rg_dosPortMaskEnable_set)(rtk_rg_mac_portmask_t dos_port_mask);
	rtk_rg_err_code_t (*rtk_rg_dosPortMaskEnable_get)(rtk_rg_mac_portmask_t *dos_port_mask);
	rtk_rg_err_code_t (*rtk_rg_dosType_set)(rtk_rg_dos_type_t dos_type,int dos_enabled,rtk_rg_dos_action_t dos_action);
	rtk_rg_err_code_t (*rtk_rg_dosType_get)(rtk_rg_dos_type_t dos_type,int *dos_enabled,rtk_rg_dos_action_t *dos_action);
//65
	rtk_rg_err_code_t (*rtk_rg_dosFloodType_set)(rtk_rg_dos_type_t dos_type,int dos_enabled,rtk_rg_dos_action_t dos_action,int dos_threshold);
	rtk_rg_err_code_t (*rtk_rg_dosFloodType_get)(rtk_rg_dos_type_t dos_type,int *dos_enabled,rtk_rg_dos_action_t *dos_action,int *dos_threshold);
	rtk_rg_err_code_t (*rtk_rg_portMirror_set)(rtk_rg_portMirrorInfo_t portMirrorInfo);
	rtk_rg_err_code_t (*rtk_rg_portMirror_get)(rtk_rg_portMirrorInfo_t *portMirrorInfo);
	rtk_rg_err_code_t (*rtk_rg_portMirror_clear)(void);
//70
	rtk_rg_err_code_t (*rtk_rg_portEgrBandwidthCtrlRate_set)(rtk_rg_mac_port_idx_t port, uint32 rate);
	rtk_rg_err_code_t (*rtk_rg_portIgrBandwidthCtrlRate_set)(rtk_rg_mac_port_idx_t port, uint32 rate);
	rtk_rg_err_code_t (*rtk_rg_portEgrBandwidthCtrlRate_get)(rtk_rg_mac_port_idx_t port, uint32 *rate);
	rtk_rg_err_code_t (*rtk_rg_portIgrBandwidthCtrlRate_get)(rtk_rg_mac_port_idx_t port, uint32 *rate);
	rtk_rg_err_code_t (*rtk_rg_phyPortForceAbility_set)(rtk_rg_mac_port_idx_t port, rtk_rg_phyPortAbilityInfo_t ability);
//75
	rtk_rg_err_code_t (*rtk_rg_phyPortForceAbility_get)(rtk_rg_mac_port_idx_t port, rtk_rg_phyPortAbilityInfo_t *ability);
	rtk_rg_err_code_t (*rtk_rg_cpuPortForceTrafficCtrl_set)(rtk_rg_enable_t tx_fc_state,	rtk_rg_enable_t rx_fc_state);
	rtk_rg_err_code_t (*rtk_rg_cpuPortForceTrafficCtrl_get)(rtk_rg_enable_t *pTx_fc_state,	rtk_rg_enable_t *pRx_fc_state);	
	rtk_rg_err_code_t (*rtk_rg_portMibInfo_get)(rtk_rg_mac_port_idx_t port, rtk_rg_port_mib_info_t *mibInfo);
	rtk_rg_err_code_t (*rtk_rg_portMibInfo_clear)(rtk_rg_mac_port_idx_t port);
//80
	rtk_rg_err_code_t (*rtk_rg_stormControl_add)(rtk_rg_stormControlInfo_t *stormInfo,int *stormInfo_idx);
	rtk_rg_err_code_t (*rtk_rg_stormControl_del)(int stormInfo_idx);
	rtk_rg_err_code_t (*rtk_rg_stormControl_find)(rtk_rg_stormControlInfo_t *stormInfo,int *stormInfo_idx);
	rtk_rg_err_code_t (*rtk_rg_shareMeter_set)(uint32 index, uint32 rate, rtk_rg_enable_t ifgInclude);
	rtk_rg_err_code_t (*rtk_rg_shareMeter_get)(uint32 index, uint32 *pRate , rtk_rg_enable_t *pIfgInclude);
//85
	rtk_rg_err_code_t (*rtk_rg_qosStrictPriorityOrWeightFairQueue_set)(rtk_rg_mac_port_idx_t port_idx,rtk_rg_qos_queue_weights_t q_weight);
	rtk_rg_err_code_t (*rtk_rg_qosStrictPriorityOrWeightFairQueue_get)(rtk_rg_mac_port_idx_t port_idx,rtk_rg_qos_queue_weights_t *pQ_weight);
	rtk_rg_err_code_t (*rtk_rg_qosInternalPriMapToQueueId_set)(int int_pri, int queue_id);
	rtk_rg_err_code_t (*rtk_rg_qosInternalPriMapToQueueId_get)(int int_pri, int *pQueue_id);
	rtk_rg_err_code_t (*rtk_rg_qosInternalPriDecisionByWeight_set)(rtk_rg_qos_priSelWeight_t weightOfPriSel);
//90
	rtk_rg_err_code_t (*rtk_rg_qosInternalPriDecisionByWeight_get)(rtk_rg_qos_priSelWeight_t *pWeightOfPriSel);
	rtk_rg_err_code_t (*rtk_rg_qosDscpRemapToInternalPri_set)(uint32 dscp,uint32 int_pri);
	rtk_rg_err_code_t (*rtk_rg_qosDscpRemapToInternalPri_get)(uint32 dscp,uint32 *pInt_pri);
	rtk_rg_err_code_t (*rtk_rg_qosPortBasedPriority_set)(rtk_rg_mac_port_idx_t port_idx,uint32 int_pri);
	rtk_rg_err_code_t (*rtk_rg_qosPortBasedPriority_get)(rtk_rg_mac_port_idx_t port_idx,uint32 *pInt_pri);
//95
	rtk_rg_err_code_t (*rtk_rg_qosDot1pPriRemapToInternalPri_set)(uint32 dot1p,uint32 int_pri);
	rtk_rg_err_code_t (*rtk_rg_qosDot1pPriRemapToInternalPri_get)(uint32 dot1p,uint32 *pInt_pri);
	rtk_rg_err_code_t (*rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_set)(rtk_rg_mac_port_idx_t rmk_port,rtk_rg_enable_t rmk_enable, rtk_rg_qos_dscpRmkSrc_t rmk_src_select);
	rtk_rg_err_code_t (*rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_get)(rtk_rg_mac_port_idx_t rmk_port,rtk_rg_enable_t *pRmk_enable, rtk_rg_qos_dscpRmkSrc_t *pRmk_src_select);
	rtk_rg_err_code_t (*rtk_rg_qosDscpRemarkByInternalPri_set)(int int_pri,int rmk_dscp);
//100
	rtk_rg_err_code_t (*rtk_rg_qosDscpRemarkByInternalPri_get)(int int_pri,int *pRmk_dscp);
	rtk_rg_err_code_t (*rtk_rg_qosDscpRemarkByDscp_set)(int dscp,int rmk_dscp);
	rtk_rg_err_code_t (*rtk_rg_qosDscpRemarkByDscp_get)(int dscp,int *pRmk_dscp);
	rtk_rg_err_code_t (*rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_set)(rtk_rg_mac_port_idx_t rmk_port, rtk_rg_enable_t rmk_enable);
	rtk_rg_err_code_t (*rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_get)(rtk_rg_mac_port_idx_t rmk_port, rtk_rg_enable_t *pRmk_enable);
//105
	rtk_rg_err_code_t (*rtk_rg_qosDot1pPriRemarkByInternalPri_set)(int int_pri,int rmk_dot1p);
	rtk_rg_err_code_t (*rtk_rg_qosDot1pPriRemarkByInternalPri_get)(int int_pri,int *pRmk_dot1p);
	rtk_rg_err_code_t (*rtk_rg_portBasedCVlanId_set)(rtk_rg_port_idx_t port_idx,int pvid);
	rtk_rg_err_code_t (*rtk_rg_portBasedCVlanId_get)(rtk_rg_port_idx_t port_idx,int *pPvid);
	rtk_rg_err_code_t (*rtk_rg_portStatus_get)(rtk_rg_mac_port_idx_t port, rtk_rg_portStatusInfo_t *portInfo);
//110
#ifdef CONFIG_RG_NAPT_PORT_COLLISION_PREVENTION
	rtk_rg_err_code_t (*rtk_rg_naptExtPortGet)(int isTcp,uint16 *pPort);
	rtk_rg_err_code_t (*rtk_rg_naptExtPortFree)(int isTcp,uint16 port);
#endif
	rtk_rg_err_code_t (*rtk_rg_classifyEntry_add)(rtk_rg_classifyEntry_t *classifyFilter);
	rtk_rg_err_code_t (*rtk_rg_classifyEntry_find)(int index, rtk_rg_classifyEntry_t *classifyFilter);
	rtk_rg_err_code_t (*rtk_rg_classifyEntry_del)(int index);
//115
	rtk_rg_err_code_t (*rtk_rg_svlanTpid_set)(uint32 svlan_tag_id);
	rtk_rg_err_code_t (*rtk_rg_svlanTpid_get)(uint32 *pSvlanTagId);
	rtk_rg_err_code_t (*rtk_rg_svlanServicePort_set)(rtk_port_t port, rtk_enable_t enable);
	rtk_rg_err_code_t (*rtk_rg_svlanServicePort_get)(rtk_port_t port, rtk_enable_t *pEnable);
	rtk_rg_err_code_t (*rtk_rg_pppoeInterfaceIdleTime_get)(int intfIdx,uint32 *idleSec);
//120
	rtk_rg_err_code_t (*rtk_rg_gatewayServicePortRegister_add)(rtk_rg_gatewayServicePortEntry_t *serviceEntry, int *index);
	rtk_rg_err_code_t (*rtk_rg_gatewayServicePortRegister_del)(int index);
	rtk_rg_err_code_t (*rtk_rg_gatewayServicePortRegister_find)(rtk_rg_gatewayServicePortEntry_t *serviceEntry, int *index);
	rtk_rg_err_code_t (*rtk_rg_wlanDevBasedCVlanId_set)(int wlan_idx,int dev_idx,int dvid);
	rtk_rg_err_code_t (*rtk_rg_wlanDevBasedCVlanId_get)(int wlan_idx,int dev_idx,int *pDvid);
//125
	rtk_rg_err_code_t (*rtk_rg_wlanSoftwareSourceAddrLearningLimit_set)(rtk_rg_saLearningLimitInfo_t sa_learnLimit_info, int wlan_idx, int dev_idx);	
	rtk_rg_err_code_t (*rtk_rg_wlanSoftwareSourceAddrLearningLimit_get)(rtk_rg_saLearningLimitInfo_t *sa_learnLimit_info, int wlan_idx, int dev_idx);
	rtk_rg_err_code_t (*rtk_rg_naptFilterAndQos_add)(int *index,rtk_rg_naptFilterAndQos_t *napt_filter);
	rtk_rg_err_code_t (*rtk_rg_naptFilterAndQos_del)(int index);
	rtk_rg_err_code_t (*rtk_rg_naptFilterAndQos_find)(int *index,rtk_rg_naptFilterAndQos_t *napt_filter);
//130
	rtk_rg_err_code_t (*rtk_rg_pptpClientInfoBeforeDial_set)(int wan_intf_idx, rtk_rg_pptpClientInfoBeforeDial_t *app_info);
	rtk_rg_err_code_t (*rtk_rg_pptpClientInfoAfterDial_set)(int wan_intf_idx, rtk_rg_pptpClientInfoAfterDial_t *clientPptp_info);
	rtk_rg_err_code_t (*rtk_rg_l2tpClientInfoBeforeDial_set)(int wan_intf_idx, rtk_rg_l2tpClientInfoBeforeDial_t *app_info);
	rtk_rg_err_code_t (*rtk_rg_l2tpClientInfoAfterDial_set)(int wan_intf_idx, rtk_rg_l2tpClientInfoAfterDial_t *clientL2tp_info);
	rtk_rg_err_code_t (*rtk_rg_stpBlockingPortmask_set)(rtk_rg_portmask_t Mask);
//135
	rtk_rg_err_code_t (*rtk_rg_stpBlockingPortmask_get)(rtk_rg_portmask_t *pMask);
	rtk_rg_err_code_t (*rtk_rg_portIsolation_set)(rtk_rg_port_isolation_t isolationSetting);
	rtk_rg_err_code_t (*rtk_rg_portIsolation_get)(rtk_rg_port_isolation_t *isolationSetting);
	rtk_rg_err_code_t (*rtk_rg_dsliteInfo_set)(int wan_intf_idx, rtk_rg_ipDslitStaticInfo_t *dslite_info);
	rtk_rg_err_code_t (*rtk_rg_pppoeDsliteInfoBeforeDial_set)(int wan_intf_idx, rtk_rg_pppoeClientInfoBeforeDial_t *app_info);
//140
	rtk_rg_err_code_t (*rtk_rg_pppoeDsliteInfoAfterDial_set)(int wan_intf_idx, rtk_rg_pppoeDsliteInfoAfterDial_t *pppoeDslite_info);
	rtk_rg_err_code_t (*rtk_rg_gponDsBcFilterAndRemarking_add)(rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t *filterRule,int *index);
	rtk_rg_err_code_t (*rtk_rg_gponDsBcFilterAndRemarking_del)(int index);
	rtk_rg_err_code_t (*rtk_rg_gponDsBcFilterAndRemarking_find)(int *index,rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t *filterRule);
	rtk_rg_err_code_t (*rtk_rg_gponDsBcFilterAndRemarking_del_all)(void);
//145
	rtk_rg_err_code_t (*rtk_rg_gponDsBcFilterAndRemarking_Enable)(rtk_rg_enable_t enable);
	rtk_rg_err_code_t (*rtk_rg_dsliteMcTable_set)(rtk_l34_dsliteMc_entry_t *pDsliteMcEntry);
	rtk_rg_err_code_t (*rtk_rg_dsliteMcTable_get)(rtk_l34_dsliteMc_entry_t *pDsliteMcEntry);
	rtk_rg_err_code_t (*rtk_rg_dsliteControl_set)(rtk_l34_dsliteCtrlType_t ctrlType, uint32 act);
	rtk_rg_err_code_t (*rtk_rg_dsliteControl_get)(rtk_l34_dsliteCtrlType_t ctrlType, uint32 *pAct);
//150
	rtk_rg_err_code_t (*rtk_rg_interfaceMibCounter_del)(int intf_idx);
	rtk_rg_err_code_t (*rtk_rg_interfaceMibCounter_get)(rtk_l34_mib_t *pMibCnt);
	rtk_rg_err_code_t (*rtk_rg_redirectHttpAll_set)(rtk_rg_redirectHttpAll_t *pRedirectHttpAll);
	rtk_rg_err_code_t (*rtk_rg_redirectHttpAll_get)(rtk_rg_redirectHttpAll_t *pRedirectHttpAll);
	rtk_rg_err_code_t (*rtk_rg_redirectHttpURL_add)(rtk_rg_redirectHttpURL_t *pRedirectHttpURL);
//155
	rtk_rg_err_code_t (*rtk_rg_redirectHttpURL_del)(rtk_rg_redirectHttpURL_t *pRedirectHttpURL);
	rtk_rg_err_code_t (*rtk_rg_redirectHttpWhiteList_add)(rtk_rg_redirectHttpWhiteList_t *pRedirectHttpWhiteList);
	rtk_rg_err_code_t (*rtk_rg_redirectHttpWhiteList_del)(rtk_rg_redirectHttpWhiteList_t *pRedirectHttpWhiteList);
	rtk_rg_err_code_t (*rtk_rg_svlanTpid2_enable_set)(rtk_rg_enable_t enable);
	rtk_rg_err_code_t (*rtk_rg_svlanTpid2_enable_get)(rtk_rg_enable_t *pEnable);
//160
	rtk_rg_err_code_t (*rtk_rg_svlanTpid2_set)(uint32 svlan_tag_id);
	rtk_rg_err_code_t (*rtk_rg_svlanTpid2_get)(uint32 *pSvlanTagId);


	
};



#endif

