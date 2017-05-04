#ifndef RTK_RG_MAPPING_API_H
#define RTK_RG_MAPPING_API_H

#if defined(CONFIG_APOLLO)
#include <rtk/svlan.h>
#endif

/* for set age of lut entry to 1 */
#if defined(CONFIG_RTL9600_SERIES)		
#else	//support lut traffic bit
int32 _rtk_rg_l2_trafficBit_reset(rtk_l2_ucastAddr_t *pL2Addr);
#endif
int32 RTK_L2_ADDR_ADD(rtk_l2_ucastAddr_t *pL2Addr);
int32 RTK_L2_ADDR_DEL(rtk_l2_ucastAddr_t *pL2Addr);
int32 RTK_L2_IPMCASTADDR_ADD(rtk_l2_ipMcastAddr_t *pIpmcastAddr);
int32 RTK_L2_IPMCASTADDR_DEL(rtk_l2_ipMcastAddr_t *pIpmcastAddr);
int32 RTK_L2_MCASTADDR_ADD(rtk_l2_mcastAddr_t *pMcastAddr);
int32 RTK_L2_MCASTADDR_DEL(rtk_l2_mcastAddr_t *pMcastAddr);
int32 RTK_L2_IPMCSIPFILTER_ADD(ipaddr_t filterIp,int32 *idx);
int32 RTK_L2_IPMCSIPFILTER_DEL(ipaddr_t filterIp,int32 *delIdx);
int32 RTK_L2_IPMCGROUP_ADD(ipaddr_t gip, rtk_portmask_t *pPortmask);
int32 RTK_L2_IPMCGROUP_DEL(ipaddr_t gip);
int32 RTK_L2_PORTLIMITLEARNINGCNT_SET(rtk_port_t port, uint32 macCnt);
int32 RTK_L2_PORTLIMITLEARNINGCNTACTION_SET(rtk_port_t port, rtk_l2_limitLearnCntAction_t learningAction);
int32 RTK_L2_PORTLOOKUPMISSACTION_SET(rtk_port_t port, rtk_l2_lookupMissType_t type, rtk_action_t action);
int32 RTK_L2_SRCPORTEGRFILTERMASK_SET(rtk_portmask_t * pFilter_portmask);
int32 RTK_L34_NETIFTABLE_SET(uint32 idx, rtk_l34_netif_entry_t *entry);
int32 RTK_L34_ROUTINGTABLE_SET(uint32 idx, rtk_l34_routing_entry_t *entry);
int32 RTK_L34_EXTINTIPTABLE_SET(uint32 idx, rtk_l34_ext_intip_entry_t *entry);
int32 RTK_L34_NEXTHOPTABLE_SET(uint32 idx, rtk_l34_nexthop_entry_t *entry);
int32 RTK_L34_PPPOETABLE_SET(uint32 idx, rtk_l34_pppoe_entry_t *entry);
int32 RTK_L34_ARPTABLE_SET(uint32 idx, rtk_l34_arp_entry_t *entry);
int32 RTK_L34_NAPTINBOUNDTABLE_SET(int8 forced,uint32 idx, rtk_l34_naptInbound_entry_t *entry);
int32 RTK_L34_NAPTOUTBOUNDTABLE_SET(int8 forced,uint32 idx, rtk_l34_naptOutbound_entry_t *entry);
int32 RTK_L34_WANTYPETABLE_SET(uint32 idx,rtk_wanType_entry_t * entry);
int32 RTK_L34_BINDINGACTION_SET(rtk_l34_bindType_t bindType,rtk_l34_bindAct_t bindAction);
int32 RTK_L34_GLOBALSTATE_SET(rtk_l34_globalStateType_t stateType,rtk_enable_t state);
int32 RTK_L34_BINDINGTABLE_SET(uint32 idx,rtk_binding_entry_t * bindEntry);
int32 RTK_L34_IPV6ROUTINGTABLE_SET(uint32 idx,rtk_ipv6Routing_entry_t * ipv6RoutEntry);
int32 RTK_L34_IPV6NEIGHBORTABLE_SET(uint32 idx,rtk_ipv6Neighbor_entry_t * ipv6NeighborEntry);
#if defined(CONFIG_RTL9602C_SERIES)
int32 RTK_L34_DSLITEINFTABLE_SET(rtk_l34_dsliteInf_entry_t *pDsliteInfEntry);
#endif
int32 RTK_VLAN_CREATE(rtk_vlan_t vid);
int32 RTK_VLAN_PORT_SET(rtk_vlan_t vid,rtk_portmask_t * pMember_portmask,rtk_portmask_t * pUntag_portmask);
int32 RTK_VLAN_EXTPORT_SET(rtk_vlan_t vid,rtk_portmask_t * pExt_portmask);
int32 RTK_VLAN_FID_SET(rtk_vlan_t vid,rtk_fid_t fid);
int32 RTK_VLAN_FIDMODE_SET(rtk_vlan_t vid,rtk_fidMode_t mode);
int32 RTK_VLAN_PRIORITY_SET(rtk_vlan_t vid,rtk_pri_t priority);
int32 RTK_VLAN_PRIORITYENABLE_SET(rtk_vlan_t vid,rtk_enable_t enable);
int32 RTK_VLAN_DESTROY(rtk_vlan_t vid);
int32 RTK_VLAN_PORTPVID_SET(rtk_port_t port,uint32 pvid);
int32 RTK_VLAN_EXTPORTPVID_SET(uint32 extPort,uint32 pvid);
int32 RTK_VLAN_PROTOGROUP_SET(uint32 protoGroupIdx,rtk_vlan_protoGroup_t *pProtoGroup);
int32 RTK_VLAN_PORTPROTOVLAN_SET(rtk_port_t port,uint32 protoGroupIdx,rtk_vlan_protoVlanCfg_t * pVlanCfg);
int32 RTK_VLAN_PORTIGRFILTERENABLE_SET(rtk_port_t port, rtk_enable_t igr_filter);
int32 RTK_QOS_1PPRIREMAPGROUP_SET(uint32 grpIdx, rtk_pri_t dot1pPri, rtk_pri_t intPri, uint32 dp);;
int32 RTK_QOS_DSCPPRIREMAPDROUP_SET(uint32 grpIdx, uint32 dscp, rtk_pri_t intPri, uint32 dp);
int32 RTK_QOS_PORTPRI_SET(rtk_port_t port, rtk_pri_t intPri);
int32 RTK_QOS_PRISELGROUP_SET(uint32 grpIdx, rtk_qos_priSelWeight_t *pWeightOfPriSel);
int32 RTK_QOS_DSCPREMARKENABLE_SET(rtk_port_t port, rtk_enable_t enable);
int32 RTK_QOS_PORTDSCPREMARKSRCSEL_SET(rtk_port_t port, rtk_qos_dscpRmkSrc_t type);
int32 RTK_QOS_PORTPRIMAP_SET(rtk_port_t port, uint32 group);
/*avoid L2 CF directly call rtk classify api & using wrong index. it should call rtk_rg_classify_cfgEntry_add()*/;
int32 RTK_CLASSIFY_CFGENTRY_ADD(rtk_classify_cfg_t *pClassifyCfg);
/*avoid L2 CF directly call rtk classify api & using wrong index. it should call rtk_rg_classify_cfgEntry_add()*/;
int32 RTK_SVLAN_TPIDENTRY_SET(uint32 svlan_index, uint32 svlan_tag_id);
#if defined(CONFIG_RTL9602C_SERIES)
int32 RTK_SVLAN_TPIDENABLE_SET(uint32 svlanIndex, rtk_enable_t enable);
#endif
int32 RTK_SVLAN_SERVICEPORT_SET(rtk_port_t port, rtk_enable_t enable);
int32 RTK_SVLAN_SERVICEPORT_GET(rtk_port_t port, rtk_enable_t *pEnable);
int32 RTK_SVLAN_DMACVIDSELSTATE_SET(rtk_port_t port, rtk_enable_t enable);
int32 RTK_SVLAN_MEMBERPORT_SET(rtk_vlan_t svid, rtk_portmask_t * pSvlanPortmask, rtk_portmask_t * pSvlanUntagPortmask);
int32 RTK_ACL_IGRSTATE_SET(rtk_port_t port, rtk_enable_t state);
int32 RTK_ACL_IGRUNMATCHACTION_SET(rtk_port_t port, rtk_filter_unmatch_action_type_t action);
int32 RTK_TRAP_PORTIGMPMLDCTRLPKTACTION_SET(rtk_port_t port, rtk_trap_igmpMld_type_t igmpMldType, rtk_action_t action);
int32 RTK_PORT_ISOLATIONIPMCLEAKY_SET(rtk_port_t port, rtk_enable_t enable);
int32 RTK_PORT_ISOLATIONENTRY_SET(rtk_port_isoConfig_t mode, rtk_port_t port, rtk_portmask_t * pPortmask, rtk_portmask_t * pExtPortmask);
int32 RTK_PORT_ISOLATIONENTRYEXT_SET(rtk_port_isoConfig_t mode, rtk_port_t port, rtk_portmask_t *pPortmask, rtk_portmask_t *pExtPortmask);
int32 RTK_PORT_MACFORCEABILITY_GET(rtk_port_t port, rtk_port_macAbility_t * pMacAbility);
int32 RTK_PORT_MACFORCEABILITY_SET(rtk_port_t port,rtk_port_macAbility_t macAbility);
int32 RTK_PORT_MACFORCEABILITYSTATE_SET(rtk_port_t port,rtk_enable_t state);
int32 RTK_SWITCH_MAXPKTLENBYPORT_SET(rtk_port_t port, int pktlen);


#define rtk_l2_addr_add PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l2_addr_del PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l2_ipMcastAddr_add PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l2_ipMcastAddr_del PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l2_mcastAddr_add PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l2_mcastAddr_del PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l2_portLimitLearningCnt_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l2_ipmcgroup_add PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l2_ipmcgroup_del PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l2_portLimitLearningCnt_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l2_portLimitLearningCntAction_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l2_portLookupMissAction_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l2_srcPortEgrFilterMask_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l34_netifTable_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l34_routingTable_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l34_extIntIPTable_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l34_nexthopTable_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l34_pppoeTable_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l34_arpTable_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l34_naptInboundTable_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l34_naptOutboundTable_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l34_wanTypeTable_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l34_bindingAction_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l34_globalState_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l34_bindingTable_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l34_ipv6RoutingTable_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_l34_ipv6NeighborTable_set PLEASE_USE_RTK_RG_APOLLO_API()
#if defined(CONFIG_RTL9602C_SERIES)
#define rtk_l34_dsliteInfTable_set PLEASE_USE_RTK_RG_APOLLO_API()
#endif
#define rtk_vlan_create PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_vlan_port_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_vlan_extport_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_vlan_fid_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_vlan_fidMode_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_vlan_priority_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_vlan_priorityEnable_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_vlan_destroy PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_vlan_portPvid_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_vlan_extPortPvid_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_vlan_protoGroup_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_vlan_portProtoVlan_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_vlan_portIgrFilterEnable_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_qos_1pPriRemapGroup_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_qos_dscpPriRemapGroup_set  PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_qos_portPri_set  PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_qos_priSelGroup_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_qos_dscpRemarkEnable_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_qos_portDscpRemarkSrcSel_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_qos_portPriMap_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_classify_cfgEntry_add PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_svlan_tpidEntry_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_svlan_serviceport_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_svlan_serviceport_get PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_svlan_dmacVidSelState_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_svlan_memberPort_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_acl_igrState_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_acl_igrUnmatchAction_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_trap_portIgmpMldCtrlPktAction_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_port_isolationIpmcLeaky_set	PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_port_isolationEntry_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_port_isolationEntryExt_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_port_macForceAbility_get PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_port_macForceAbility_set PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_port_macForceAbilityState_set PLEASE_USE_RTK_RG_APOLLO_API()

#endif
