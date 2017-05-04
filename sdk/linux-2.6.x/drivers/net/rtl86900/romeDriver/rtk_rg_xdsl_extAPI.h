#ifndef RTK_RG_XDSL_EXTAPI_H
#define RTK_RG_XDSL_EXTAPI_H



//#include <rtk_rg_internal.h>
#include <rtk_rg_define.h>
#include <rtk_rg_struct.h>
#include <rtk_rg_debug.h>

#define CONFIG_APOLLO 0 //for sourceInsign check apollo flag correct
#undef CONFIG_APOLLO

#ifndef CONFIG_XDSL_RG_DIAGSHELL
#define APOLLOMP_CHIP_ID 	2	//for temp compiler pass FIXME plz
#define CHIP_REV_ID_C	 	2 	//for temp compiler pass FIXME plz
#define CHIP_REV_ID_0		2	//for temp compiler pass FIXME plz
#define CHIP_REV_ID_B		2	//for temp compiler pass FIXME plz
#endif

#define RTK_RG_FWDENGINE_HOUSE_KEEP_SECOND 1


//my function
int32 rtk_hwEnable(int32 enable);
int32 rtk_pppoe_getIdx(int32 sessionId);
void _rtk_rg_refresh_tcp_longTimeout(int32 longTimeout);
void _rtk_rg_refresh_udp_longTimeout(int32 longTimeout);
int32 _rtk_syncLutTb4way_FormHw2Sw(unsigned char *mac);



// multiple definition 
#define rtk_svlan_tpidEntry_get _rtk_svlan_tpidEntry_get
#define rtk_svlan_servicePort_get _rtk_svlan_servicePort_get
//#define rtk_vlan_portIgrFilterEnable_set _rtk_vlan_portIgrFilterEnable_set
#define rtk_qos_1pRemarkEnable_get _rtk_qos_1pRemarkEnable_get
#define rtk_l2_addr_get _rtk_l2_addr_get


/* xdsl proc api*/
int32 rtk_rg_xdsl_init( struct file *filp, const char *buff,unsigned long len, void *data );
int32 rtk_rg_xdsl_lanInterface_add( struct file *filp, const char *buff,unsigned long len, void *data );
int32 rtk_rg_xdsl_wanInterface_add( struct file *filp, const char *buff,unsigned long len, void *data );
int32 rtk_rg_xdsl_staticInfo_set( struct file *filp, const char *buff,unsigned long len, void *data );
int32 rtk_rg_xdsl_pppoeClientInfoBeforeDial_set( struct file *filp, const char *buff,unsigned long len, void *data );
int32 rtk_rg_xdsl_pppoeClientInfoAfterDial_set( struct file *filp, const char *buff,unsigned long len, void *data );
int32 rtk_rg_xdsl_dhcpRequest_set( struct file *filp, const char *buff,unsigned long len, void *data );
int32 rtk_rg_xdsl_dhcpClientInfo_set( struct file *filp, const char *buff,unsigned long len, void *data );

/* acl */
int32 rtk_rg_xdsl_acl_cmd( struct file *filp, const char *buff,unsigned long len, void *data );
int _dump_xdsl_acl(void);

/* xdsl */
int32 rtk_rg_fwdEngine_test(void);
void rtk_rg_xdsl_test_napt(void);
int32 rtk_rg_xdsl_cvlan( struct file *filp, const char *buff,unsigned long len, void *data );

/* qos */
int32 rtk_rg_xdsl_qos( struct file *filp, const char *buff,unsigned long len, void *data );

/* Tables Access*/
//L2 table
int32 rtk_l2_addr_add(rtk_l2_ucastAddr_t *pL2Addr);
int32 rtk_l2_addr_del(rtk_l2_ucastAddr_t *pL2Addr);
int32 rtk_l2_nextValidEntry_get(int32	*pScanIdx,rtk_l2_addr_table_t     *pL2Entry);
int32 rtk_l2_addr_get(rtk_l2_ucastAddr_t *pL2Addr);
//nexthop table
int32 rtk_l34_nexthopTable_set(uint32 idx, rtk_l34_nexthop_entry_t *entry);
//netif table
int32 rtk_l34_netifTable_set(uint32 idx, rtk_l34_netif_entry_t *entry);
//routing table
int32 rtk_l34_routingTable_set(uint32 idx, rtk_l34_routing_entry_t *entry);
//extip table
int32 rtk_l34_extIntIPTable_set(uint32 idx, rtk_l34_ext_intip_entry_t *entry);
//arp tabke
int32 rtk_l34_arpTable_set(uint32 idx, rtk_l34_arp_entry_t *entry);
//vlan table
int32 rtk_vlan_init(void);
int32 rtk_vlan_create(rtk_vlan_t vid);
int32 rtk_vlan_port_set(rtk_vlan_t vid,rtk_portmask_t *pMember_portmask, rtk_portmask_t *pUntag_portmask);
int32 rtk_vlan_fid_set(rtk_vlan_t vid, rtk_fid_t fid);
int32 rtk_vlan_extPort_set(rtk_vlan_t vid,rtk_portmask_t *pExt_portmask);
int32 rtk_vlan_destroy(rtk_vlan_t vid);
//pvid
int32 rtk_vlan_portPvid_set(rtk_port_t port, uint32 pvid);
int32 rtk_vlan_extPortPvid_set(uint32 extPort, uint32 pvid);
//NAPT table
int32 rtk_l34_naptInboundTable_set(int8 forced, uint32 idx,rtk_l34_naptInbound_entry_t *entry);
int32 rtk_l34_naptOutboundTable_set(int8 forced, uint32 idx,rtk_l34_naptOutbound_entry_t *entry);

/* NIC Function*/
int re8686_send_with_txInfo_and_mask(struct sk_buff *skb, struct tx_info* ptxInfo, int ring_num, struct tx_info* ptxInfoMask);



#define FIXEME_FUNCITON 1//for sourceInsight 
#if FIXEME_FUNCITON
int32 rtk_classify_portRange_set(rtk_classify_rangeCheck_l4Port_t *pRangeEntry);
int32 rtk_classify_ipRange_set(rtk_classify_rangeCheck_ip_t *pRangeEntry);
int32 rtk_classify_cfPri2Dscp_set(rtk_pri_t pri, rtk_dscp_t dscp);

int32 rtk_l34_init(void);
int32 rtk_l34_hsdState_set(rtk_enable_t hsdState);
int32 rtk_l34_bindingAction_get(rtk_l34_bindType_t bindType, rtk_l34_bindAct_t *bindAction);
int32 rtk_l34_globalState_get(rtk_l34_globalStateType_t stateType,rtk_enable_t *pState);
int32 rtk_l34_globalState_set(rtk_l34_globalStateType_t stateType,rtk_enable_t state);
int32 rtk_l34_hwL4TrfWrkTbl_Clear(rtk_l34_l4_trf_t l4TrfTable);
int32 rtk_l34_wanTypeTable_set(uint32 idx, rtk_wanType_entry_t *wanTypeEntry);
int32 rtk_l34_pppoeTable_set(uint32 idx, rtk_l34_pppoe_entry_t *pppEntry);
int32 rtk_l34_ipv6RoutingTable_set(uint32 idx, rtk_ipv6Routing_entry_t *ipv6RoutEntry);
int32 rtk_l34_ipv6NeighborTable_set(uint32 idx,rtk_ipv6Neighbor_entry_t *ipv6NeighborEntry);
int32 rtk_l34_wanRoutMode_set(rtk_l34_wanRouteMode_t wanRouteMode);
int32 rtk_l34_lookupMode_set(rtk_l34_lookupMode_t lookupMode);
int32 rtk_l34_hsabMode_set(rtk_l34_hsba_mode_t hsabMode);
int32 rtk_l34_nexthopTable_get(uint32 idx, rtk_l34_nexthop_entry_t *nextHopEntry);
int32 rtk_l34_ipmcTransTable_set(uint32 idx, rtk_l34_ipmcTrans_entry_t *ipmcEntry);

int32 rtk_l2_init(void);
int32 rtk_l2_ipmcMode_get(rtk_l2_ipmcMode_t *pMode);
int32 rtk_l2_portLimitLearningCnt_set(rtk_port_t port, uint32 macCnt);
int32 rtk_l2_portLimitLearningCntAction_set(rtk_port_t port, rtk_l2_limitLearnCntAction_t learningAction);
int32 rtk_l2_ipMcastAddr_add(rtk_l2_ipMcastAddr_t *pIpmcastAddr);
int32 rtk_l2_ipMcastAddr_del(rtk_l2_ipMcastAddr_t *pIpmcastAddr);
int32 rtk_l2_mcastAddr_add(rtk_l2_mcastAddr_t *pMcastAddr);
int32 rtk_l2_mcastAddr_del(rtk_l2_mcastAddr_t *pMcastAddr);
int32 rtk_l2_ipmcGroup_add(ipaddr_t gip, rtk_portmask_t *pPortmask);
int32 rtk_l2_ipmcGroup_del(ipaddr_t gip);
int32 rtk_l2_portLookupMissAction_set(rtk_port_t port, rtk_l2_lookupMissType_t type, rtk_action_t action);
int32 rtk_l2_addr_delAll(uint32 includeStatic);
int32 rtk_l2_lookupMissFloodPortMask_set(rtk_l2_lookupMissType_t type, rtk_portmask_t *pFlood_portmask);
int32 rtk_l2_ipmcMode_set(rtk_l2_ipmcMode_t mode);
int32 rtk_l2_ipmcGroupLookupMissHash_set(rtk_l2_ipmcHashOp_t ipmcHash);
int32 rtk_l2_lookupMissAction_set(rtk_l2_lookupMissType_t type, rtk_action_t action);
int32 rtk_l2_srcPortEgrFilterMask_set(rtk_portmask_t *pFilter_portmask);
int32 rtk_l2_extPortEgrFilterMask_set(rtk_portmask_t *pExt_portmask);
int32 rtk_l2_nextValidAddr_get(int32 *pScanIdx,rtk_l2_ucastAddr_t  *pL2UcastData);

int32 rtk_intr_linkdownStatus_clear(void);
//int32 rtk_intr_imr_set(rtk_intr_type_t intr, rtk_enable_t enable);
//int32 rtk_irq_isr_register(rtk_intr_type_t intr, void (*fun)(void));

int32 rtk_trap_portIgmpMldCtrlPktAction_set(rtk_port_t port, rtk_trap_igmpMld_type_t igmpMldType, rtk_action_t action);


int32 rtk_svlan_unmatchAction_set(rtk_svlan_action_t action, rtk_vlan_t svid);
int32 rtk_svlan_create(rtk_vlan_t svid);
int32 rtk_svlan_servicePort_set(rtk_port_t port, rtk_enable_t enable);
int32 _rtk_svlan_servicePort_get(rtk_port_t port, rtk_enable_t *pEnable);
int32 rtk_svlan_dmacVidSelState_set(rtk_port_t port, rtk_enable_t enable);
int32 _rtk_svlan_tpidEntry_get(uint32 svlanIndex, uint32 *pSvlanTagId); 
int32 rtk_svlan_tpidEntry_set(uint32 svlan_index, uint32 svlan_tag_id);
int32 rtk_svlan_memberPort_set(rtk_vlan_t svid, rtk_portmask_t *pSvlanPortmask, rtk_portmask_t *pSvlanUntagPortmask);
int32 rtk_svlan_fidEnable_set(rtk_vlan_t svid, rtk_enable_t enable);
int32 rtk_svlan_fid_set(rtk_vlan_t svid, rtk_fid_t fid);
int32 rtk_svlan_untagAction_set(rtk_svlan_action_t action, rtk_vlan_t svid);
int32 rtk_svlan_sp2cUnmatchCtagging_set(rtk_enable_t state);
int32 rtk_svlan_dmacVidSelForcedState_set(rtk_enable_t enable);
int32 rtk_svlan_deiKeepState_set(rtk_enable_t enable);

int32 _rtk_vlan_portIgrFilterEnable_set(rtk_port_t port, rtk_enable_t enable);
int32 rtk_vlan_vlanFunctionEnable_set(rtk_enable_t enable);
int32 rtk_vlan_protoGroup_set(uint32 protoGroupIdx,rtk_vlan_protoGroup_t *pProtoGroup);
int32 rtk_vlan_portProtoVlan_set(rtk_port_t port,uint32 protoGroupIdx,rtk_vlan_protoVlanCfg_t *pVlanCfg);
int32 rtk_vlan_extPortPvid_get(uint32 extPort, uint32 *pPvid);
int32 rtk_vlan_portPvid_get(rtk_port_t port, uint32 *pPvid);

int32 _rtk_qos_1pRemarkEnable_get(rtk_port_t port, rtk_enable_t *pEnable);
int32 rtk_qos_1pPriRemapGroup_set(uint32 grpIdx,rtk_pri_t dot1pPri,rtk_pri_t intPri,uint32 dp);
int32 rtk_qos_dscpPriRemapGroup_set(uint32 grpIdx,uint32 dscp,rtk_pri_t intPri,uint32 dp);
int32 rtk_qos_portPri_set(rtk_port_t port, rtk_pri_t intPri);
int32 rtk_qos_priSelGroup_set(uint32 grpIdx, rtk_qos_priSelWeight_t *pWeightOfPriSel);
int32 rtk_qos_fwd2CpuPriRemap_set(rtk_pri_t intPri,rtk_pri_t rempPri);
int32 rtk_qos_portPriMap_set(rtk_port_t port, uint32 group);
int32 rtk_qos_schedulingQueue_set(rtk_port_t port, rtk_qos_queue_weights_t *pQweights);
int32 rtk_qos_schedulingQueue_get(rtk_port_t port, rtk_qos_queue_weights_t *pQweights);
int32 rtk_qos_priMap_get(uint32 group, rtk_qos_pri2queue_t *pPri2qid);
int32 rtk_qos_priMap_set(uint32 group, rtk_qos_pri2queue_t *pPri2qid);
int32 rtk_qos_priSelGroup_get(uint32 grpIdx, rtk_qos_priSelWeight_t *pWeightOfPriSel);
int32 rtk_qos_dscpPriRemapGroup_get(uint32 grpIdx,uint32 dscp,rtk_pri_t *pIntPri,uint32 *pDp);
int32 rtk_qos_1pPriRemapGroup_get(uint32 grpIdx,rtk_pri_t dot1pPri,rtk_pri_t *pIntPri,uint32 *pDp);
int32 rtk_qos_dscpRemarkEnable_set(rtk_port_t port, rtk_enable_t enable);
int32 rtk_qos_portDscpRemarkSrcSel_set(rtk_port_t port, rtk_qos_dscpRmkSrc_t type);
int32 rtk_qos_dscpRemarkEnable_get(rtk_port_t port, rtk_enable_t *pEnable);
int32 rtk_qos_portDscpRemarkSrcSel_get(rtk_port_t port, rtk_qos_dscpRmkSrc_t *pType);
int32 rtk_qos_dscpRemarkGroup_set(uint32 grpIdx,rtk_pri_t intPri,uint32 dp,uint32 dscp);
int32 rtk_qos_dscpRemarkGroup_get(uint32 grpIdx,rtk_pri_t intPri,uint32 dp,uint32 *pDscp);
int32 rtk_qos_dscp2DscpRemarkGroup_set(uint32 grpIdx,uint32 dscp, uint32 rmkDscp);
int32 rtk_qos_dscp2DscpRemarkGroup_get(uint32 grpIdx, uint32 dscp, uint32 *pDscp);
int32 rtk_qos_1pRemarkEnable_set(rtk_port_t port, rtk_enable_t enable);
int32 rtk_qos_1pRemarkGroup_set(uint32 grpIdx,rtk_pri_t intPri,uint32 dp,rtk_pri_t dot1pPri);
int32 rtk_qos_1pRemarkGroup_get(uint32 grpIdx,rtk_pri_t intPri,uint32 dp,rtk_pri_t *pDot1pPri);
int32 rtk_qos_portPri_get(rtk_port_t port, rtk_pri_t *pIntPri);

/* acl asic dirver api*/
int32 rtk_acl_template_get(rtk_acl_template_t *aclTemplate);
int32 rtk_acl_igrRuleEntry_del(uint32 index);
int32 rtk_acl_igrRuleEntry_add(rtk_acl_ingress_entry_t *pAclRule);
int32 rtk_acl_template_set(rtk_acl_template_t *aclTemplate);
int32 rtk_acl_igrRuleField_add(rtk_acl_ingress_entry_t *pAclRule, rtk_acl_field_t *pAclField);
int32 rtk_acl_igrRuleEntry_get(rtk_acl_ingress_entry_t *pAclRule);
int32 rtk_acl_igrRuleMode_set(rtk_acl_igr_rule_mode_t mode);
int32 rtk_acl_igrState_set(rtk_port_t port, rtk_enable_t state);
int32 rtk_acl_igrUnmatchAction_set(rtk_port_t port, rtk_filter_unmatch_action_type_t action);
int32 rtk_acl_fieldSelect_set(rtk_acl_field_entry_t *pFieldEntry);
int32 rtk_acl_portRange_set(rtk_acl_rangeCheck_l4Port_t *pRangeEntry);
int32 rtk_acl_ipRange_set(rtk_acl_rangeCheck_ip_t *pRangeEntry);
int32 rtk_acl_portRange_get(rtk_acl_rangeCheck_l4Port_t *pRangeEntry);
int32 rtk_acl_ipRange_get(rtk_acl_rangeCheck_ip_t *pRangeEntry);

/* cf asic dirver api*/
int32 rtk_classify_cfgEntry_del(uint32 entryIdx);
int32 rtk_classify_cfgEntry_add(rtk_classify_cfg_t *pClassifyCfg);
int32 rtk_classify_cfgEntry_get(rtk_classify_cfg_t *pClassifyCfg);
int32 rtk_classify_cfSel_set(rtk_port_t port, rtk_classify_cf_sel_t cfSel);
int32 rtk_classify_unmatchAction_set(rtk_classify_unmatch_action_t action);
int32 rtk_classify_portRange_get(rtk_classify_rangeCheck_l4Port_t *pRangeEntry);
int32 rtk_classify_ipRange_get(rtk_classify_rangeCheck_ip_t *pRangeEntry);
int32 rtk_classify_cfPri2Dscp_get(rtk_pri_t pri, rtk_dscp_t *pDscp);
int32 rtk_classify_field_add(rtk_classify_cfg_t *pClassifyEntry, rtk_classify_field_t *pClassifyField);

int32 rtk_port_isolationEntryExt_set(rtk_port_isoConfig_t mode, rtk_port_t port, rtk_portmask_t *pPortmask, rtk_portmask_t *pExtPortmask);
int32 rtk_port_isolationEntry_set(rtk_port_isoConfig_t mode, rtk_port_t port, rtk_portmask_t *pPortmask, rtk_portmask_t *pExtPortmask);
int32 rtk_port_isolationCtagPktConfig_set(rtk_port_isoConfig_t mode);
int32 rtk_port_isolationL34PktConfig_set(rtk_port_isoConfig_t mode);
int32 rtk_port_isolationIpmcLeaky_set(rtk_port_t port, rtk_enable_t enable);
int32 rtk_port_macForceAbility_get(rtk_port_t port,rtk_port_macAbility_t *pMacAbility);
int32 rtk_port_macForceAbility_set(rtk_port_t port,rtk_port_macAbility_t macAbility);
int32 rtk_port_macForceAbilityState_set(rtk_port_t port,rtk_enable_t state);
int32 rtk_port_phyReg_get(rtk_port_t port,uint32 page,rtk_port_phy_reg_t  reg,uint32 *pData);
int32 rtk_port_phyReg_set(rtk_port_t port,uint32 page,rtk_port_phy_reg_t  reg,uint32 data);
int32 rtk_port_phyAutoNegoAbility_set(rtk_port_t port,rtk_port_phy_ability_t  *pAbility);
int32 rtk_port_phyAutoNegoAbility_get(rtk_port_t port,rtk_port_phy_ability_t  *pAbility);
int32 rtk_port_macForceAbilityState_get(rtk_port_t port,rtk_enable_t *pState);
int32 rtk_port_link_get(rtk_port_t port, rtk_port_linkStatus_t *pLinkStatus);
int32 rtk_port_speedDuplex_get(rtk_port_t port,rtk_port_speed_t *pSpeed,rtk_port_duplex_t *pDuplex);

int32 rtk_mirror_portBased_set(rtk_port_t mirroringPort, rtk_portmask_t *pMirroredRxPortmask, rtk_portmask_t *pMirroredTxPortmask);
int32 rtk_mirror_portBased_get(rtk_port_t *pMirroringPort, rtk_portmask_t *pMirroredRxPortmask, rtk_portmask_t *pMirroredTxPortmask);

int32 rtk_ponmac_queue_get(rtk_ponmac_queue_t *pQueue, rtk_ponmac_queueCfg_t *pQueueCfg);
int32 rtk_ponmac_queue_add(rtk_ponmac_queue_t *pQueue, rtk_ponmac_queueCfg_t *pQueueCfg);

int32 rtk_rate_shareMeterBucket_set(uint32 index, uint32 bucketSize);
int32 rtk_rate_portEgrBandwidthCtrlRate_set(rtk_port_t port, uint32 rate);
int32 rtk_rate_portIgrBandwidthCtrlRate_set(rtk_port_t port, uint32 rate);
int32 rtk_rate_portEgrBandwidthCtrlRate_get(rtk_port_t port, uint32 *pRate);
int32 rtk_rate_portIgrBandwidthCtrlRate_get(rtk_port_t port, uint32 *pRate);
int32 rtk_rate_stormControlEnable_get(rtk_rate_storm_group_ctrl_t  *stormCtrl);
int32 rtk_rate_stormControlEnable_set(rtk_rate_storm_group_ctrl_t  *stormCtrl);
int32 rtk_rate_stormControlPortEnable_set(rtk_port_t port,rtk_rate_storm_group_t  stormType,rtk_enable_t enable);
int32 rtk_rate_stormControlMeterIdx_set(rtk_port_t port,rtk_rate_storm_group_t  stormType,uint32 index);
int32 rtk_rate_shareMeter_set(uint32 index, uint32 rate, rtk_enable_t ifgInclude);
int32 rtk_rate_shareMeter_get(uint32 index, uint32 *pRate , rtk_enable_t *pIfgInclude);
int32 rtk_rate_egrBandwidthCtrlIncludeIfg_set(rtk_enable_t ifgInclude);

int32 rtk_sec_portAttackPreventState_set(rtk_port_t port, rtk_enable_t enable);
int32 rtk_sec_portAttackPreventState_get(rtk_port_t port, rtk_enable_t *pEnable);
int32 rtk_sec_attackPrevent_set(rtk_sec_attackType_t attackType,rtk_action_t action);
int32 rtk_sec_attackPrevent_get(rtk_sec_attackType_t attackType,rtk_action_t *pAction);
int32 rtk_sec_attackFloodThresh_set(rtk_sec_attackFloodType_t type, uint32 floodThresh);
int32 rtk_sec_attackFloodThresh_get(rtk_sec_attackFloodType_t type, uint32 *pFloodThresh);


int32 rtk_switch_version_get(uint32 *pChipId, uint32 *pRev, uint32 *pSubtype);
int32 rtk_switch_phyPortId_get(rtk_switch_port_name_t portName, int32 *pPortId);
int32 rtk_init_without_pon(void);
int32 rtk_stat_port_getAll(rtk_port_t port, rtk_stat_port_cntr_t *pPortCntrs);
int32 rtk_stat_port_reset(rtk_port_t port);


int intr_bcaster_notifier_cb_register(intrBcasterNotifier_t     *pRegNotifier);
int re8670_rx_skb (struct re_private *cp, struct sk_buff *skb, struct rx_info *pRxInfo);
struct sk_buff *re8670_getAlloc(unsigned int size);




#endif //FIXEME_FUNCITON

#define NOT_SUPPORT_FUNCTION 1 //for sourceInsight 
#if NOT_SUPPORT_FUNCTION
//=====================NOT_SUPPORT function===============================//
/* bind liteRome function (not support)*/
rtk_rg_fwdEngineReturn_t _rtk_rg_bindingRuleCheck(rtk_rg_pktHdr_t *pPktHdr, int *wanGroupIdx);
int32 _rtk_rg_updatingVlanBind(int wanIdx,int v6wanTypeIdx);
int32 _rtk_rg_addBindFromPortmask(unsigned int pmsk, unsigned int expmsk, int intfIdx, int wantypeIdx, int v6WantypeIdx);
int32 _rtk_rg_updateBindWanIntf(rtk_rg_wanIntfConf_t *wanintf);
int32 _rtk_rg_updateNoneBindingPortmask(uint32 wanPmsk);
void _rtk_rg_deletingPortBindFromInterface(int intfIdx);
int _rtk_rg_portBindingByProtocal_change( struct file *filp, const char *buff,unsigned long len, void *data );
int _rtk_rg_portBindingByProtocal_state(char *page, char **start, off_t off, int count, int *eof, void *data);
int32 _rtk_rg_updateBindOtherWanPortBasedVID(rtk_rg_wanIntfConf_t *otherWanIntf);
/* asic bind function*/
int32 rtk_l34_bindingAction_set(rtk_l34_bindType_t bindType, rtk_l34_bindAct_t bindAction);
int32 rtk_l34_bindingTable_set(uint32 idx,rtk_binding_entry_t *bindEntry); 

int32 rtk_vlan_priorityEnable_set(rtk_vlan_t vid, rtk_enable_t enable);
int32 rtk_vlan_priority_set(rtk_vlan_t vid, rtk_pri_t priority);
int32 rtk_vlan_fidMode_set(rtk_vlan_t vid, rtk_fidMode_t mode);
#endif //NOT_SUPPORT_FUNCTION




#endif  //RTK_RG_XDSL_EXTAPI_H
