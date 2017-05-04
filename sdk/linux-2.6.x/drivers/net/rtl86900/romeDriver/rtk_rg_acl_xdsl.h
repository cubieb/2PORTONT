#ifndef RTK_RG_ACL_XDSL_H
#define RTK_RG_ACL_XDSL_H

//XDSL Platform
#define MIN_ACL_ENTRY_INDEX 	RESERVED_ACL_BEFORE
#define MAX_ACL_ENTRY_INDEX 	RESERVED_ACL_AFTER

#define MIN_ACL_IGR_ENTRY_INDEX 	rg_db.systemGlobal.aclAndCfReservedRule.aclLowerBoundary
#define MAX_ACL_IGR_ENTRY_INDEX 	rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary
#define MIN_ACL_EGR_ENTRY_INDEX 	rg_db.systemGlobal.aclAndCfReservedRule.cfLowerBoundary
#define MAX_ACL_EGR_ENTRY_INDEX 	rg_db.systemGlobal.aclAndCfReservedRule.cfUpperBoundary
#define MAX_ACL_HW_ENTRY_SIZE 			256
#define MAX_ACL_SW_ENTRY_SIZE 			512
#define xDSL_ACL_IGR_RSVD_HEAD_SIZE 	1	//ALL Trap
#define xDSL_ACL_IGR_RSVD_TAIL_SIZE 	1	//ALL Permit
#define MAX_ACL_IGR_ENTRY_SIZE 			128
#define xDSL_ACL_EGR_RSVD_HEAD_SIZE 	1	//ALL Trap
#define xDSL_ACL_EGR_RSVD_TAIL_SIZE 	1	//ALL Permit
#define MAX_ACL_EGR_ENTRY_SIZE 			128
#define MAX_ACL_TEMPLATE_SIZE 4 //Used to identify max. combined ACL rule entries
#define TOTAL_CF_ENTRY_SIZE 	MAX_ACL_EGR_ENTRY_SIZE

#define RESERVED_ACL_BEFORE rg_db.systemGlobal.aclAndCfReservedRule.aclLowerBoundary
#define RESERVED_ACL_AFTER rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary

#define RESERVED_CF_BEFORE 	rg_db.systemGlobal.aclAndCfReservedRule.cfLowerBoundary
#define RESERVED_CF_AFTER 	rg_db.systemGlobal.aclAndCfReservedRule.cfUpperBoundary

#define PURE_ACL_PATTERN_BITS \
(INGRESS_PORT_BIT|INGRESS_INTF_BIT|INGRESS_ETHERTYPE_BIT|INGRESS_CTAG_PRI_BIT|INGRESS_CTAG_VID_BIT|INGRESS_SMAC_BIT \
|INGRESS_DMAC_BIT|INGRESS_DSCP_BIT|INGRESS_L4_TCP_BIT|INGRESS_L4_UDP_BIT|INGRESS_IPV6_SIP_RANGE_BIT|INGRESS_IPV6_DIP_RANGE_BIT \
|INGRESS_IPV4_SIP_RANGE_BIT|INGRESS_IPV4_DIP_RANGE_BIT|INGRESS_L4_SPORT_RANGE_BIT|INGRESS_L4_DPORT_RANGE_BIT \
|INGRESS_L4_ICMP_BIT|INGRESS_IPV6_DSCP_BIT|INGRESS_STREAM_ID_BIT|INGRESS_STAG_PRI_BIT|INGRESS_STAG_VID_BIT|INGRESS_STAGIF_BIT|INGRESS_CTAGIF_BIT \
|INGRESS_L4_POROTCAL_VALUE_BIT|INGRESS_TOS_BIT|INGRESS_IPV6_TC_BIT|INGRESS_IPV6_SIP_BIT|INGRESS_IPV6_DIP_BIT|INGRESS_WLANDEV_BIT|INGRESS_IPV4_TAGIF_BIT|INGRESS_IPV6_TAGIF_BIT \
|INGRESS_CTAG_CFI_BIT|INGRESS_STAG_DEI_BIT)


#define PURE_CF_PATTERN_BITS \
(EGRESS_INTF_BIT|EGRESS_IPV4_SIP_RANGE_BIT|EGRESS_IPV4_DIP_RANGE_BIT \
|EGRESS_L4_SPORT_RANGE_BIT|EGRESS_L4_DPORT_RANGE_BIT|EGRESS_CTAG_PRI_BIT|EGRESS_CTAG_VID_BIT|INGRESS_EGRESS_PORTIDX_BIT|INTERNAL_PRI_BIT)

typedef enum rtk_rg_aclSWEntry_type_s
{
	UNUSE = 0,
	ACL_USE,
	QOS_USE,
	OTHERS_USE,
} rtk_rg_aclSWEntry_type_t;

/*ACL Reserved*/
typedef enum rtk_rg_aclAndCf_reserved_type_e
{

	/*HEAD PATCH: The lower index number, the higher acl&cf priority*/
	RTK_RG_ACLANDCF_RESERVED_EPON_DROP_AND_INTERRUPT=0,
	RTK_RG_ACLANDCF_RESERVED_INTF0_DHCP_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF1_DHCP_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF2_DHCP_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF3_DHCP_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF4_DHCP_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF5_DHCP_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF6_DHCP_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF7_DHCP_TRAP,	
	RTK_RG_ACLANDCF_RESERVED_WIFI_DATAPATH_MASTER2SLAVE_PATCH, //Dual wifi reserved especial vid, pri for datapath(Master=>slave), we translate vid to 1 for avoiding vlan filter & pri to special slave queue.
	RTK_RG_ACLANDCF_RESERVED_STPBLOCKING,
	RTK_RG_ACLANDCF_RESERVED_MULTICAST_RMA_TRAP,
	RTK_RG_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP,
	RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP,
	RTK_RG_ACLANDCF_RESERVED_ALL_TRAP,
	RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP,	
	RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY,
	RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF1_DSLITE_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF2_DSLITE_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF3_DSLITE_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF4_DSLITE_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF5_DSLITE_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF6_DSLITE_TRAP,
	RTK_RG_ACLANDCF_RESERVED_INTF7_DSLITE_TRAP,	
	RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF0_MC_ROUTING_TRAP,
	RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF1_MC_ROUTING_TRAP,
	RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF2_MC_ROUTING_TRAP,
	RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF3_MC_ROUTING_TRAP,
	RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF4_MC_ROUTING_TRAP,
	RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF5_MC_ROUTING_TRAP,
	RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF6_MC_ROUTING_TRAP,
	RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF7_MC_ROUTING_TRAP,
	RTK_RG_ACLANDCF_RESERVED_IPV6_INTF0_LINK_LOCAL_TRAP,
	RTK_RG_ACLANDCF_RESERVED_IPV6_INTF1_LINK_LOCAL_TRAP,
	RTK_RG_ACLANDCF_RESERVED_IPV6_INTF2_LINK_LOCAL_TRAP,
	RTK_RG_ACLANDCF_RESERVED_IPV6_INTF3_LINK_LOCAL_TRAP,
	RTK_RG_ACLANDCF_RESERVED_IPV6_INTF4_LINK_LOCAL_TRAP,
	RTK_RG_ACLANDCF_RESERVED_IPV6_INTF5_LINK_LOCAL_TRAP,
	RTK_RG_ACLANDCF_RESERVED_IPV6_INTF6_LINK_LOCAL_TRAP,
	RTK_RG_ACLANDCF_RESERVED_IPV6_INTF7_LINK_LOCAL_TRAP,
	RTK_RG_ACLANDCF_RESERVED_HEAD_END,

	/*TAIL PATCH: The lower index number, the lower acl&cf priority*/

#if 0
    RTK_RG_ACLANDCF_RESERVED_URLFILTER_TRAP,
#endif
    RTK_RG_ACLANDCF_RESERVED_IGMP_TO_SLAVE_WIFI_BLOCK,
    RTK_RG_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT,
    RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV6_PASSTHROUGHT,
#if 0
    RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV6,
    RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV4,
	RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864,
	RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF7ISPPPOEWAN, //pppoe packet to intf7 avoid hit RTK_RG_ACL_RESERVED_PPPoEPASSTHROUGHT_REMARKIN_BY_DMAC2CVID_DOWNSTREAM_ETHTER_8864
	RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF6ISPPPOEWAN, //pppoe packet to intf6 avoid hit RTK_RG_ACL_RESERVED_PPPoEPASSTHROUGHT_REMARKIN_BY_DMAC2CVID_DOWNSTREAM_ETHTER_8864
	RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF5ISPPPOEWAN, //pppoe packet to intf5 avoid hit RTK_RG_ACL_RESERVED_PPPoEPASSTHROUGHT_REMARKIN_BY_DMAC2CVID_DOWNSTREAM_ETHTER_8864
	RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF4ISPPPOEWAN, //pppoe packet to intf4 avoid hit RTK_RG_ACL_RESERVED_PPPoEPASSTHROUGHT_REMARKIN_BY_DMAC2CVID_DOWNSTREAM_ETHTER_8864
	RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF3ISPPPOEWAN, //pppoe packet to intf3 avoid hit RTK_RG_ACL_RESERVED_PPPoEPASSTHROUGHT_REMARKIN_BY_DMAC2CVID_DOWNSTREAM_ETHTER_8864
	RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF2ISPPPOEWAN, //pppoe packet to intf2 avoid hit RTK_RG_ACL_RESERVED_PPPoEPASSTHROUGHT_REMARKIN_BY_DMAC2CVID_DOWNSTREAM_ETHTER_8864
	RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF1ISPPPOEWAN, //pppoe packet to intf1 avoid hit RTK_RG_ACL_RESERVED_PPPoEPASSTHROUGHT_REMARKIN_BY_DMAC2CVID_DOWNSTREAM_ETHTER_8864
	RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF0ISPPPOEWAN, //pppoe packet to intf0 avoid hit RTK_RG_ACL_RESERVED_PPPoEPASSTHROUGHT_REMARKIN_BY_DMAC2CVID_DOWNSTREAM_ETHTER_8864
#endif
    RTK_RG_ACLANDCF_RESERVED_WIFI_MASTER_EXTPORT_PATCH,//replace src extPort[0](from master) information to egress extPortMask
    RTK_RG_ACLANDCF_RESERVED_WIFI_SLAVE_EXTPORT_PATCH,//replace src extPort[1](from slave) information to egress extPortMask
//#ifdef CONFIG_RG_PPPOE_AND_VALN_ISSUE_PATCH
    RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_SVIDCOPY2CVID_PATCH, //Wan:PPPoE+Vlan Lan:link-up 100Mbps  have parsing bug=> look 0x8100 as stag in wan port, and copy stag to ctag
//#endif
//#ifdef CONFIG_RG_PPPOE_AND_VALN_ISSUE_PATCH
    RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI7REMAPTOINTERNALPRI_PATCH, //Qos remarking pacth for using Stag (SPri[7]=based on Qos API=>internal-priority)
    RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI6REMAPTOINTERNALPRI_PATCH, //Qos remarking pacth for using Stag (SPri[6]=based on Qos API=>internal-priority)
    RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI5REMAPTOINTERNALPRI_PATCH, //Qos remarking pacth for using Stag (SPri[5]=based on Qos API=>internal-priority)
    RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI4REMAPTOINTERNALPRI_PATCH, //Qos remarking pacth for using Stag (SPri[4]=based on Qos API=>internal-priority)
    RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI3REMAPTOINTERNALPRI_PATCH, //Qos remarking pacth for using Stag (SPri[3]=based on Qos API=>internal-priority)
    RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI2REMAPTOINTERNALPRI_PATCH, //Qos remarking pacth for using Stag (SPri[2]=based on Qos API=>internal-priority)
    RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI1REMAPTOINTERNALPRI_PATCH, //Qos remarking pacth for using Stag (SPri[1]=based on Qos API=>internal-priority)
    RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI0REMAPTOINTERNALPRI_PATCH, //Qos remarking pacth for using Stag (SPri[0]=based on Qos API=>internal-priority)
//#endif
#if 0
	RTK_RG_ACLANDCF_RESERVED_PON_INTFDEFAULTSSIDREMAP_PATCH, 
	RTK_RG_ACLANDCF_RESERVED_PON_INTF7SSIDREMAP_PATCH, 
	RTK_RG_ACLANDCF_RESERVED_PON_INTF6SSIDREMAP_PATCH, 
	RTK_RG_ACLANDCF_RESERVED_PON_INTF5SSIDREMAP_PATCH, 
	RTK_RG_ACLANDCF_RESERVED_PON_INTF4SSIDREMAP_PATCH, 
	RTK_RG_ACLANDCF_RESERVED_PON_INTF3SSIDREMAP_PATCH, 
	RTK_RG_ACLANDCF_RESERVED_PON_INTF2SSIDREMAP_PATCH, 
	RTK_RG_ACLANDCF_RESERVED_PON_INTF1SSIDREMAP_PATCH, 
	RTK_RG_ACLANDCF_RESERVED_PON_INTF0SSIDREMAP_PATCH, //FIXME: bits not enough
#endif
	RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT0_WITHOUT_FILTER,
	RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT1_WITHOUT_FILTER,
	RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT2_WITHOUT_FILTER,
	RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT3_WITHOUT_FILTER,
	RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT4_WITHOUT_FILTER,
	RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT5_WITHOUT_FILTER,
	RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT6_WITHOUT_FILTER,

	RTK_RG_ACLANDCF_RESERVED_ALL_PERMIT,
    RTK_RG_ACLANDCF_RESERVED_TAIL_END,
}rtk_rg_aclAndCf_reserved_type_t;

typedef enum rtk_rg_aclSWEntry_used_tables_field_s
{
	APOLLOFE_RG_ACL_USED_INGRESS_SIP4TABLE = (1<<0),
	APOLLOFE_RG_ACL_USED_INGRESS_DIP4TABLE = (1<<1),
	APOLLOFE_RG_ACL_USED_INGRESS_SIP6TABLE = (1<<2),
	APOLLOFE_RG_ACL_USED_INGRESS_DIP6TABLE = (1<<3),
	APOLLOFE_RG_ACL_USED_INGRESS_SPORTTABLE = (1<<4),
	APOLLOFE_RG_ACL_USED_INGRESS_DPORTTABLE = (1<<5),
	APOLLOFE_RG_ACL_USED_EGRESS_SIP4TABLE = (1<<6),
	APOLLOFE_RG_ACL_USED_EGRESS_DIP4TABLE = (1<<7),
	APOLLOFE_RG_ACL_USED_EGRESS_SIP6TABLE = (1<<8),
	APOLLOFE_RG_ACL_USED_EGRESS_DIP6TABLE = (1<<9),
	APOLLOFE_RG_ACL_USED_EGRESS_SPORTTABLE = (1<<10),
	APOLLOFE_RG_ACL_USED_EGRESS_DPORTTABLE = (1<<11),
} rtk_rg_aclSWEntry_used_tables_field_t;

typedef enum rtk_rg_aclSWEntry_used_tables_index_s
{
	APOLLOFE_RG_ACL_USED_INGRESS_SIP4TABLE_INDEX = 0,
	APOLLOFE_RG_ACL_USED_INGRESS_DIP4TABLE_INDEX = 1,
	APOLLOFE_RG_ACL_USED_INGRESS_SIP6TABLE_INDEX = 2,
	APOLLOFE_RG_ACL_USED_INGRESS_DIP6TABLE_INDEX = 3,
	APOLLOFE_RG_ACL_USED_INGRESS_SPORTTABLE_INDEX = 4,
	APOLLOFE_RG_ACL_USED_INGRESS_DPORTTABLE_INDEX = 5,
	APOLLOFE_RG_ACL_USED_EGRESS_SIP4TABLE_INDEX = 6,
	APOLLOFE_RG_ACL_USED_EGRESS_DIP4TABLE_INDEX = 7,
	APOLLOFE_RG_ACL_USED_EGRESS_SIP6TABLE_INDEX = 8,
	APOLLOFE_RG_ACL_USED_EGRESS_DIP6TABLE_INDEX = 9,
	APOLLOFE_RG_ACL_USED_EGRESS_SPORTTABLE_INDEX = 10,
	APOLLOFE_RG_ACL_USED_EGRESS_DPORTTABLE_INDEX = 11,
	USED_TABLE_END = 12,
} rtk_rg_aclSWEntry_used_tables_index_t;

typedef struct rtk_rg_aclAndCf_reserved_WifiMasterExtportPatch_s
{
	int igrPmsk;
	int egrPmsk;
}rtk_rg_aclAndCf_reserved_WifiMasterExtportPatch_t;

typedef struct rtk_rg_aclAndCf_reserved_WifiSlaveExtportPatch_s
{
	int igrPmsk;
	int egrPmsk;
}rtk_rg_aclAndCf_reserved_WifiSlaveExtportPatch_t;

typedef struct rtk_rg_aclAndCf_reserved_WifiDatapathMaster2SlavePatch_s
{
	uint32 igrCvid;
	uint32 igrCpri;
	uint32 tranCvid;
	uint32 tranCpri;
}rtk_rg_aclAndCf_reserved_WifiDatapathMaster2SlavePatch_t;


//#ifdef CONFIG_RG_PPPOE_AND_VALN_ISSUE_PATCH
typedef struct rtk_rg_aclAndCf_reserved_pppoeCvidIssueSvidCopy2CvidPatch_s
{
	uint32 wanPort;
}rtk_rg_aclAndCf_reserved_pppoeCvidIssueSvidCopy2CvidPatch_t;

typedef struct rtk_rg_aclAndCf_reserved_pppoeCvidIssueSpriRemap2InternalPriPatch_s
{
	int wanPort;
	int spri;
	int intpri;
}rtk_rg_aclAndCf_reserved_pppoeCvidIssueSpriRemap2InternalPriPatch_t;
//#endif

typedef struct rtk_rg_aclAndCf_reserved_pppoepassthroughtIntfIsPppoewanPatch_s
{
	rtk_mac_t gmac;
}rtk_rg_aclAndCf_reserved_pppoepassthroughtIntfIsPppoewanPatch_t;


typedef struct rtk_rg_aclAndCf_reserved_pppoepassthroughtDefaulRuletPatch_s
{
	uint32 lanPmsk;
	uint32 wanPmsk;
	uint32 remarkVid;
}rtk_rg_aclAndCf_reserved_pppoepassthroughtDefaulTrapRuletPatch_t;


typedef struct rtk_rg_aclAndCf_reserved_ponIntfSsidRemapPatch_s
{
	int intfIdx;
	int ssid;
	int vid;
}rtk_rg_aclAndCf_reserved_ponIntfSsidRemapPatch_t;

typedef struct rtk_rg_aclAndCf_reserved_multicast_vid_translate_for_ipv4_s{
	int vid;
}rtk_rg_aclAndCf_reserved_multicastVidTranslateForIpv4_t;

typedef struct rtk_rg_aclAndCf_reserved_multicast_vid_translate_for_ipv6_s{
	int vid;
}rtk_rg_aclAndCf_reserved_multicastVidTranslateForIpv6_t;

typedef struct rtk_rg_aclAndCf_reserved_AssignCvlanForServicePortWithoutFilter_s{
	int service_port; //physical port which enabled service port ability.
	int assigned_vid;
}rtk_rg_aclAndCf_reserved_AssignCvlanForServicePortWithoutFilter_t;

typedef struct rtk_rg_aclAndCf_reserved_intf_dhcp_trap_s{
	rtk_mac_t gmac;
}rtk_rg_aclAndCf_reserved_intf_dhcp_trap_t;

typedef struct rtk_rg_aclAndCf_reserved_intf_dslite_trap_s{
	rtk_mac_t smac;
	rtk_ipv6_addr_t ipv6_dip;
}rtk_rg_aclAndCf_reserved_intf_dslite_trap_t;

typedef struct rtk_rg_aclAndCf_reserved_pppoe_intf_multicast_routing_trap_s{
	rtk_mac_t gmac;
}rtk_rg_aclAndCf_reserved_pppoe_intf_multicast_routing_trap_t;

typedef struct rtk_rg_aclAndCf_reserved_intf_linkLocal_trap_s{
	rtk_mac_t gmac;
}rtk_rg_aclAndCf_reserved_intf_linkLocal_trap_t;

typedef struct rtk_rg_aclAndCf_reserved_ack_packet_assign_priority_s{
	int priority;
}rtk_rg_aclAndCf_reserved_ack_packet_assign_priority_t;

/*ACL Reserved*/
typedef struct rtk_rg_aclAndCf_reserved_global_s
{
	uint32 aclLowerBoundary; //The End index of acl reserve rules in the head
	uint32 aclUpperBoundary; //The start index of acl reserve rules in the tail
	uint32 cfLowerBoundary;  //The start index of cf reserve rules in the head
	uint32 cfUpperBoundary;  //The start index of cf reserve rules in the tail
	uint8 reservedMask[RTK_RG_ACLANDCF_RESERVED_TAIL_END];

	//rtk_rg_aclAndCf_reserved_type_t reservedMask;

	uint32 acl_MC_temp_permit_idx; //this index is reserved for user ACL rearrange, and reserved by type RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP or RTK_RG_ACLANDCF_RESERVED_ALL_TRAP

	rtk_rg_aclAndCf_reserved_AssignCvlanForServicePortWithoutFilter_t 	assignCvlanForServicePortWithoutFilterPara[7];//each array stand for one port 
	rtk_rg_aclAndCf_reserved_intf_dhcp_trap_t intf_dhcp_trap_para[8]; //each array stand for one intf.Idx.
	rtk_rg_aclAndCf_reserved_intf_dslite_trap_t intf_dslite_trap_para[8];//each array stand for one intf.Idx.
	rtk_rg_aclAndCf_reserved_pppoe_intf_multicast_routing_trap_t pppoe_intf_multicast_routing_trap_para[8];//each array stand for one intf.Idx.
	rtk_rg_aclAndCf_reserved_intf_linkLocal_trap_t intf_link_local_trap_para[8]; //each array stand for one intf.Idx.
	rtk_rg_aclAndCf_reserved_ack_packet_assign_priority_t ack_packet_assign_priority;	
}rtk_rg_acl_reserved_global_t;

#endif //end of #ifndef RTK_RG_ACL_XDSL_H
