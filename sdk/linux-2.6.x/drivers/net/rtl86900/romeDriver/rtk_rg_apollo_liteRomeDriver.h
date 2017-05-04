#ifndef __RTK_RG_APOLLO_LITEROMEDRIVER_H__
#define __RTK_RG_APOLLO_LITEROMEDRIVER_H__

#if 0
//Functions which used before Definitions should have Declarations
int32 rtk_rg_apollo_macEntry_add(rtk_rg_macEntry_t *macEntry, int *entry_idx);
int32 rtk_rg_apollo_macEntry_del(int entry_idx);
int32 rtk_rg_apollo_arpEntry_add(rtk_rg_arpEntry_t *arpEntry, int *arp_entry_idx);
int32 rtk_rg_apollo_arpEntry_del(int arp_entry_idx);
int32 rtk_rg_apollo_algApps_set(rtk_rg_alg_type_t alg_app);
int32 rtk_rg_apollo_vlanBinding_del(int vlan_binding_idx);
int32 rtk_rg_apollo_neighborEntry_find(rtk_rg_neighborInfo_t *neighborInfo,int *neighbor_valid_idx);
int32 rtk_rg_apollo_neighborEntry_del(int neighbor_idx);
int32 rtk_rg_apollo_naptConnection_del(int flow_idx);
int32 rtk_rg_apollo_virtualServer_find(rtk_rg_virtualServer_t *virtual_server, int *valid_idx);
int32 rtk_rg_apollo_virtualServer_del(int virtual_server_idx);
int32 rtk_rg_apollo_upnpConnection_find(rtk_rg_upnpConnection_t *upnp, int *valid_idx);
int32 rtk_rg_apollo_upnpConnection_del(int upnp_idx);
int32 rtk_rg_apollo_upnpConnection_add(rtk_rg_upnpConnection_t *upnp, int *upnp_idx);
int32 rtk_rg_apollo_dmzHost_set(int wan_intf_idx, rtk_rg_dmzInfo_t *dmz_info);
int32 rtk_rg_apollo_macEntry_find(rtk_rg_macEntry_t *macEntry,int *valid_idx);
int32 rtk_rg_apollo_neighborEntry_add(rtk_rg_neighborEntry_t *neighborEntry,int *neighbor_idx);
int32 rtk_rg_apollo_virtualServer_add(rtk_rg_virtualServer_t *virtual_server, int *virtual_server_idx);
int32 rtk_rg_apollo_qosDot1pPriRemarkByInternalPri_get(int int_pri,int *pRmk_dot1p);
int32 rtk_rg_apollo_softwareSourceAddrLearningLimit_set(rtk_rg_saLearningLimitInfo_t sa_learnLimit_info, rtk_rg_port_idx_t port_idx);
int32 rtk_rg_apollo_dosType_get(rtk_rg_dos_type_t dos_type,int *dos_enabled,rtk_rg_dos_action_t *dos_action);
int32 rtk_rg_apollo_multicastFlow_find(rtk_rg_multicastFlow_t *mcFlow, int *valid_idx);
int32 rtk_rg_apollo_multicastFlow_del(int flow_idx);
int32 rtk_rg_apollo_naptFilterAndQos_add(int *index,rtk_rg_naptFilterAndQos_t *napt_filter);
int32 rtk_rg_apollo_naptFilterAndQos_del(int index);
int32 rtk_rg_apollo_naptFilterAndQos_find(int *index,rtk_rg_naptFilterAndQos_t *napt_filter);
#endif

rtk_rg_err_code_t rtk_rg_apollo_stpBlockingPortmask_set(rtk_rg_portmask_t Mask);
rtk_rg_err_code_t rtk_rg_apollo_stpBlockingPortmask_get (rtk_rg_portmask_t *pMask);


rtk_rg_err_code_t rtk_rg_apollo_accessWanLimitCategory_get(rtk_rg_accessWanLimitCategory_t *macCategory_info);
rtk_rg_err_code_t rtk_rg_apollo_accessWanLimitCategory_set(rtk_rg_accessWanLimitCategory_t macCategory_info);
rtk_rg_err_code_t rtk_rg_apollo_accessWanLimit_get(rtk_rg_accessWanLimitData_t *access_wan_info);
rtk_rg_err_code_t rtk_rg_apollo_accessWanLimit_set(rtk_rg_accessWanLimitData_t access_wan_info);
rtk_rg_err_code_t rtk_rg_apollo_aclFilterAndQos_add(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx);
rtk_rg_err_code_t rtk_rg_apollo_aclFilterAndQos_del(int acl_filter_idx);
rtk_rg_err_code_t rtk_rg_apollo_aclFilterAndQos_find(rtk_rg_aclFilterAndQos_t *acl_filter, int *valid_idx);
rtk_rg_err_code_t rtk_rg_apollo_gponDsBcFilterAndRemarking_Enable(rtk_rg_enable_t enable);
rtk_rg_err_code_t rtk_rg_apollo_gponDsBcFilterAndRemarking_add(rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t *filterRule,int *index);
rtk_rg_err_code_t rtk_rg_apollo_gponDsBcFilterAndRemarking_del(int index);
rtk_rg_err_code_t rtk_rg_apollo_gponDsBcFilterAndRemarking_del_all(void);
rtk_rg_err_code_t rtk_rg_apollo_gponDsBcFilterAndRemarking_find(int *index,rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t *filterRule);
rtk_rg_err_code_t rtk_rg_apollo_algApps_get(rtk_rg_alg_type_t *alg_app);
rtk_rg_err_code_t rtk_rg_apollo_algApps_set(rtk_rg_alg_type_t alg_app);
rtk_rg_err_code_t rtk_rg_apollo_dmzHost_set(int wan_intf_idx, rtk_rg_dmzInfo_t *dmz_info);
rtk_rg_err_code_t rtk_rg_apollo_algServerInLanAppsIpAddr_add(rtk_rg_alg_serverIpMapping_t *srvIpMapping);
rtk_rg_err_code_t rtk_rg_apollo_algServerInLanAppsIpAddr_del(rtk_rg_alg_type_t delServerMapping);
rtk_rg_err_code_t rtk_rg_apollo_arpEntry_add(rtk_rg_arpEntry_t *arpEntry, int *arp_entry_idx);
rtk_rg_err_code_t rtk_rg_apollo_arpEntry_del(int arp_entry_idx);
rtk_rg_err_code_t rtk_rg_apollo_arpEntry_find(rtk_rg_arpInfo_t *arpInfo,int *arp_valid_idx);
rtk_rg_err_code_t rtk_rg_apollo_classifyEntry_add(rtk_rg_classifyEntry_t *classifyFilter);
rtk_rg_err_code_t rtk_rg_apollo_classifyEntry_del(int index);
rtk_rg_err_code_t rtk_rg_apollo_classifyEntry_find(int index, rtk_rg_classifyEntry_t *classifyFilter);
rtk_rg_err_code_t rtk_rg_apollo_cpuPortForceTrafficCtrl_get(rtk_rg_enable_t *pTx_fc_state,	rtk_rg_enable_t *pRx_fc_state);
rtk_rg_err_code_t rtk_rg_apollo_cpuPortForceTrafficCtrl_set(rtk_rg_enable_t tx_fc_state,	rtk_rg_enable_t rx_fc_state);
rtk_rg_err_code_t rtk_rg_apollo_cvlan_add(rtk_rg_cvlan_info_t *cvlan_info);
rtk_rg_err_code_t rtk_rg_apollo_cvlan_del(int cvlan_id);
rtk_rg_err_code_t rtk_rg_apollo_cvlan_get(rtk_rg_cvlan_info_t *cvlan_info);
rtk_rg_err_code_t rtk_rg_apollo_dhcpClientInfo_set(int wan_intf_idx, rtk_rg_ipDhcpClientInfo_t *dhcpClient_info);
rtk_rg_err_code_t rtk_rg_apollo_dhcpRequest_set(int wan_intf_idx);
rtk_rg_err_code_t rtk_rg_apollo_dmzHost_set(int wan_intf_idx, rtk_rg_dmzInfo_t *dmz_info);
rtk_rg_err_code_t rtk_rg_apollo_dmzHost_get(int wan_intf_idx, rtk_rg_dmzInfo_t *dmz_info);
rtk_rg_err_code_t rtk_rg_apollo_dosFloodType_get(rtk_rg_dos_type_t dos_type,int *dos_enabled,rtk_rg_dos_action_t *dos_action,int *dos_threshold);
rtk_rg_err_code_t rtk_rg_apollo_dosFloodType_set(rtk_rg_dos_type_t dos_type,int dos_enabled,rtk_rg_dos_action_t dos_action,int dos_threshold);
rtk_rg_err_code_t rtk_rg_apollo_dosPortMaskEnable_get(rtk_rg_mac_portmask_t *dos_port_mask);
rtk_rg_err_code_t rtk_rg_apollo_dosPortMaskEnable_set(rtk_rg_mac_portmask_t dos_port_mask);
rtk_rg_err_code_t rtk_rg_apollo_dosType_get(rtk_rg_dos_type_t dos_type,int *dos_enabled,rtk_rg_dos_action_t *dos_action);
rtk_rg_err_code_t rtk_rg_apollo_dosType_set(rtk_rg_dos_type_t dos_type,int dos_enabled,rtk_rg_dos_action_t dos_action);
rtk_rg_err_code_t rtk_rg_apollo_driverVersion_get(rtk_rg_VersionString_t *version_string);
rtk_rg_err_code_t rtk_rg_apollo_gatewayServicePortRegister_add(rtk_rg_gatewayServicePortEntry_t *serviceEntry, int *index);
rtk_rg_err_code_t rtk_rg_apollo_gatewayServicePortRegister_del(int index);
rtk_rg_err_code_t rtk_rg_apollo_gatewayServicePortRegister_find(rtk_rg_gatewayServicePortEntry_t *serviceEntry, int *index);
rtk_rg_err_code_t rtk_rg_apollo_initParam_get(rtk_rg_initParams_t *init_param);
rtk_rg_err_code_t rtk_rg_apollo_initParam_set(rtk_rg_initParams_t *init_param);
rtk_rg_err_code_t rtk_rg_apollo_interface_del(int lan_or_wan_intf_idx);
rtk_rg_err_code_t rtk_rg_apollo_intfInfo_find(rtk_rg_intfInfo_t *intf_info, int *valid_lan_or_wan_intf_idx);
rtk_rg_err_code_t rtk_rg_apollo_lanInterface_add(rtk_rg_lanIntfConf_t *lan_info,int *intf_idx);
rtk_rg_err_code_t rtk_rg_apollo_macEntry_add(rtk_rg_macEntry_t *macEntry, int *entry_idx);
rtk_rg_err_code_t rtk_rg_apollo_macEntry_del(int entry_idx);
rtk_rg_err_code_t rtk_rg_apollo_macEntry_find(rtk_rg_macEntry_t *macEntry,int *valid_idx);
rtk_rg_err_code_t rtk_rg_apollo_macFilter_add(rtk_rg_macFilterEntry_t *macFilterEntry,int *mac_filter_idx);
rtk_rg_err_code_t rtk_rg_apollo_macFilter_del(int mac_filter_idx);
rtk_rg_err_code_t rtk_rg_apollo_macFilter_find(rtk_rg_macFilterEntry_t *macFilterEntry, int *valid_idx);
rtk_rg_err_code_t rtk_rg_apollo_multicastFlow_add(rtk_rg_multicastFlow_t *mcFlow, int *flow_idx);
rtk_rg_err_code_t rtk_rg_apollo_multicastFlow_del(int flow_idx);
/* martin zhu add */
rtk_rg_err_code_t rtk_rg_apollo_l2MultiCastFlow_add(rtk_rg_l2MulticastFlow_t *l2McFlow,int *flow_idx);
rtk_rg_err_code_t rtk_rg_apollo_multicastFlow_find(rtk_rg_multicastFlow_t *mcFlow, int *valid_idx);
rtk_rg_err_code_t rtk_rg_apollo_naptConnection_add(rtk_rg_naptEntry_t *naptFlow, int *flow_idx);
rtk_rg_err_code_t rtk_rg_apollo_naptConnection_del(int flow_idx);
rtk_rg_err_code_t rtk_rg_apollo_naptConnection_find(rtk_rg_naptInfo_t *naptEntry,int *valid_idx);
rtk_rg_err_code_t rtk_rg_apollo_naptExtPortFree(int isTcp,uint16 port);
rtk_rg_err_code_t rtk_rg_apollo_naptExtPortGet(int isTcp,uint16 *pPort);
rtk_rg_err_code_t rtk_rg_apollo_naptFilterAndQos_add(int *index,rtk_rg_naptFilterAndQos_t *napt_filter);
rtk_rg_err_code_t rtk_rg_apollo_naptFilterAndQos_del(int index);
rtk_rg_err_code_t rtk_rg_apollo_naptFilterAndQos_find(int *index,rtk_rg_naptFilterAndQos_t *napt_filter);
rtk_rg_err_code_t rtk_rg_apollo_neighborEntry_add(rtk_rg_neighborEntry_t *neighborEntry,int *neighbor_idx);
rtk_rg_err_code_t rtk_rg_apollo_neighborEntry_del(int neighbor_idx);
rtk_rg_err_code_t rtk_rg_apollo_neighborEntry_find(rtk_rg_neighborInfo_t *neighborInfo,int *neighbor_valid_idx);
rtk_rg_err_code_t rtk_rg_apollo_phyPortForceAbility_get(rtk_rg_mac_port_idx_t port, rtk_rg_phyPortAbilityInfo_t *ability);
rtk_rg_err_code_t rtk_rg_apollo_phyPortForceAbility_set(rtk_rg_mac_port_idx_t port, rtk_rg_phyPortAbilityInfo_t ability);
rtk_rg_err_code_t rtk_rg_apollo_portBasedCVlanId_get(rtk_rg_port_idx_t port_idx,int *pPvid)	;
rtk_rg_err_code_t rtk_rg_apollo_portBasedCVlanId_set(rtk_rg_port_idx_t port_idx,int pvid);
rtk_rg_err_code_t rtk_rg_apollo_wlanDevBasedCVlanId_set(int wlan_idx,int dev_idx,int dvid);
rtk_rg_err_code_t rtk_rg_apollo_portEgrBandwidthCtrlRate_get(rtk_rg_mac_port_idx_t port, uint32 *rate);
rtk_rg_err_code_t rtk_rg_apollo_portEgrBandwidthCtrlRate_set(rtk_rg_mac_port_idx_t port, uint32 rate);
rtk_rg_err_code_t rtk_rg_apollo_portIgrBandwidthCtrlRate_get(rtk_rg_mac_port_idx_t port, uint32 *rate);
rtk_rg_err_code_t rtk_rg_apollo_portIgrBandwidthCtrlRate_set(rtk_rg_mac_port_idx_t port, uint32 rate);
rtk_rg_err_code_t rtk_rg_apollo_portMibInfo_clear(rtk_rg_mac_port_idx_t port);
rtk_rg_err_code_t rtk_rg_apollo_portMibInfo_get(rtk_rg_mac_port_idx_t port, rtk_rg_port_mib_info_t *mibInfo);
rtk_rg_err_code_t rtk_rg_apollo_portMirror_clear(void);
rtk_rg_err_code_t rtk_rg_apollo_portMirror_get(rtk_rg_portMirrorInfo_t *portMirrorInfo);
rtk_rg_err_code_t rtk_rg_apollo_portMirror_set(rtk_rg_portMirrorInfo_t portMirrorInfo);
rtk_rg_err_code_t rtk_rg_apollo_portStatus_get(rtk_rg_mac_port_idx_t port, rtk_rg_portStatusInfo_t *portInfo);
rtk_rg_err_code_t rtk_rg_apollo_pppoeClientInfoAfterDial_set(int wan_intf_idx, rtk_rg_pppoeClientInfoAfterDial_t *clientPppoe_info);
rtk_rg_err_code_t rtk_rg_apollo_pppoeClientInfoBeforeDial_set(int wan_intf_idx, rtk_rg_pppoeClientInfoBeforeDial_t *app_info);
rtk_rg_err_code_t rtk_rg_apollo_pptpClientInfoAfterDial_set(int wan_intf_idx, rtk_rg_pptpClientInfoAfterDial_t *clientPptp_info);
rtk_rg_err_code_t rtk_rg_apollo_pptpClientInfoBeforeDial_set(int wan_intf_idx, rtk_rg_pptpClientInfoBeforeDial_t *app_info);
rtk_rg_err_code_t rtk_rg_apollo_l2tpClientInfoAfterDial_set(int wan_intf_idx, rtk_rg_l2tpClientInfoAfterDial_t *clientL2tp_info);
rtk_rg_err_code_t rtk_rg_apollo_l2tpClientInfoBeforeDial_set(int wan_intf_idx, rtk_rg_l2tpClientInfoBeforeDial_t *app_info);
rtk_rg_err_code_t rtk_rg_apollo_pppoeInterfaceIdleTime_get(int intfIdx,uint32 *idleSec);
rtk_rg_err_code_t rtk_rg_apollo_qosDot1pPriRemapToInternalPri_get(uint32 dot1p,uint32 *pInt_pri);
rtk_rg_err_code_t rtk_rg_apollo_qosDot1pPriRemapToInternalPri_set(uint32 dot1p,uint32 int_pri);
rtk_rg_err_code_t rtk_rg_apollo_qosDot1pPriRemarkByInternalPriEgressPortEnable_get(rtk_rg_mac_port_idx_t rmk_port, rtk_rg_enable_t *pRmk_enable);
rtk_rg_err_code_t rtk_rg_apollo_qosDot1pPriRemarkByInternalPriEgressPortEnable_set(rtk_rg_mac_port_idx_t rmk_port, rtk_rg_enable_t rmk_enable);
rtk_rg_err_code_t rtk_rg_apollo_qosDot1pPriRemarkByInternalPri_get(int int_pri,int *pRmk_dot1p);
rtk_rg_err_code_t rtk_rg_apollo_qosDot1pPriRemarkByInternalPri_set(int int_pri,int rmk_dot1p);
rtk_rg_err_code_t rtk_rg_apollo_qosDscpRemapToInternalPri_get(uint32 dscp,uint32 *pInt_pri);
rtk_rg_err_code_t rtk_rg_apollo_qosDscpRemapToInternalPri_set(uint32 dscp,uint32 int_pri);
rtk_rg_err_code_t rtk_rg_apollo_qosDscpRemarkByDscp_get(int dscp,int *pRmk_dscp);
rtk_rg_err_code_t rtk_rg_apollo_qosDscpRemarkByDscp_set(int dscp,int rmk_dscp);
rtk_rg_err_code_t rtk_rg_apollo_qosDscpRemarkByInternalPri_get(int int_pri,int *pDscp);
rtk_rg_err_code_t rtk_rg_apollo_qosDscpRemarkByInternalPri_set(int int_pri,int dscp);
rtk_rg_err_code_t rtk_rg_apollo_qosDscpRemarkEgressPortEnableAndSrcSelect_get(rtk_rg_mac_port_idx_t rmk_port,rtk_rg_enable_t *pRmk_enable, rtk_rg_qos_dscpRmkSrc_t *pRmk_src_select);
rtk_rg_err_code_t rtk_rg_apollo_qosDscpRemarkEgressPortEnableAndSrcSelect_set(rtk_rg_mac_port_idx_t rmk_port,rtk_rg_enable_t rmk_enable, rtk_rg_qos_dscpRmkSrc_t rmk_src_select);
rtk_rg_err_code_t rtk_rg_apollo_qosInternalPriDecisionByWeight_get(rtk_rg_qos_priSelWeight_t *pWeightOfPriSel);
rtk_rg_err_code_t rtk_rg_apollo_qosInternalPriDecisionByWeight_set(rtk_rg_qos_priSelWeight_t weightOfPriSel);
rtk_rg_err_code_t rtk_rg_apollo_qosInternalPriMapToQueueId_get(int int_pri, int *pQueue_Id);
rtk_rg_err_code_t rtk_rg_apollo_qosInternalPriMapToQueueId_set(int int_pri, int queue_id);
rtk_rg_err_code_t rtk_rg_apollo_qosPortBasedPriority_get(rtk_rg_mac_port_idx_t port_idx,uint32 *pInt_pri);
rtk_rg_err_code_t rtk_rg_apollo_qosPortBasedPriority_set(rtk_rg_mac_port_idx_t port_idx,uint32 int_pri);
rtk_rg_err_code_t rtk_rg_apollo_qosStrictPriorityOrWeightFairQueue_get(rtk_rg_mac_port_idx_t port_idx,rtk_rg_qos_queue_weights_t *pQ_weight);
rtk_rg_err_code_t rtk_rg_apollo_qosStrictPriorityOrWeightFairQueue_set(rtk_rg_mac_port_idx_t port_idx,rtk_rg_qos_queue_weights_t q_weight);
rtk_rg_err_code_t rtk_rg_apollo_shareMeter_get(uint32 index, uint32 *pRate , rtk_rg_enable_t *pIfgInclude);
rtk_rg_err_code_t rtk_rg_apollo_shareMeter_set(uint32 index, uint32 rate, rtk_rg_enable_t ifgInclude);
rtk_rg_err_code_t rtk_rg_apollo_softwareSourceAddrLearningLimit_get(rtk_rg_saLearningLimitInfo_t *sa_learnLimit_info, rtk_rg_port_idx_t port_idx);
rtk_rg_err_code_t rtk_rg_apollo_softwareSourceAddrLearningLimit_set(rtk_rg_saLearningLimitInfo_t sa_learnLimit_info, rtk_rg_port_idx_t port_idx);
rtk_rg_err_code_t rtk_rg_apollo_wlanSoftwareSourceAddrLearningLimit_set(rtk_rg_saLearningLimitInfo_t sa_learnLimit_info, int wlan_idx, int dev_idx);
rtk_rg_err_code_t rtk_rg_apollo_staticInfo_set(int wan_intf_idx, rtk_rg_ipStaticInfo_t *static_info);
rtk_rg_err_code_t rtk_rg_apollo_stormControl_add(rtk_rg_stormControlInfo_t *stormInfo,int *stormInfo_idx);
rtk_rg_err_code_t rtk_rg_apollo_stormControl_del(int stormInfo_idx);
rtk_rg_err_code_t rtk_rg_apollo_stormControl_find(rtk_rg_stormControlInfo_t *stormInfo,int *stormInfo_idx);
rtk_rg_err_code_t rtk_rg_apollo_svlanServicePort_get(rtk_port_t port, rtk_enable_t *pEnable);
rtk_rg_err_code_t rtk_rg_apollo_svlanServicePort_set(rtk_port_t port, rtk_enable_t enable);
rtk_rg_err_code_t rtk_rg_apollo_svlanTpid_get(uint32 *pSvlanTagId);
rtk_rg_err_code_t rtk_rg_apollo_svlanTpid_set(uint32 svlan_tag_id);
rtk_rg_err_code_t rtk_rg_apollo_upnpConnection_add(rtk_rg_upnpConnection_t *upnp, int *upnp_idx);
rtk_rg_err_code_t rtk_rg_apollo_upnpConnection_del(int upnp_idx);
rtk_rg_err_code_t rtk_rg_apollo_upnpConnection_find(rtk_rg_upnpConnection_t *upnp, int *valid_idx);
rtk_rg_err_code_t rtk_rg_apollo_urlFilterString_add(rtk_rg_urlFilterString_t *filter,int *url_idx);
rtk_rg_err_code_t rtk_rg_apollo_urlFilterString_del(int url_idx);
rtk_rg_err_code_t rtk_rg_apollo_urlFilterString_find(rtk_rg_urlFilterString_t *filter, int *valid_idx);
rtk_rg_err_code_t rtk_rg_apollo_virtualServer_add(rtk_rg_virtualServer_t *virtual_server, int *virtual_server_idx);
rtk_rg_err_code_t rtk_rg_apollo_virtualServer_del(int virtual_server_idx);
rtk_rg_err_code_t rtk_rg_apollo_virtualServer_find(rtk_rg_virtualServer_t *virtual_server, int *valid_idx);
rtk_rg_err_code_t rtk_rg_apollo_vlanBinding_add(rtk_rg_vlanBinding_t *vlan_binding_info, int *vlan_binding_idx);
rtk_rg_err_code_t rtk_rg_apollo_vlanBinding_del(int vlan_binding_idx);
rtk_rg_err_code_t rtk_rg_apollo_vlanBinding_find(rtk_rg_vlanBinding_t *vlan_binding_info, int *valid_idx);
rtk_rg_err_code_t rtk_rg_apollo_wlanDevBasedCVlanId_get(int wlan_idx,int dev_idx,int *pDvid);
rtk_rg_err_code_t rtk_rg_apollo_wlanDevBasedCVlanId_set(int wlan_idx,int dev_idx,int dvid);
rtk_rg_err_code_t rtk_rg_apollo_wlanSoftwareSourceAddrLearningLimit_get(rtk_rg_saLearningLimitInfo_t *sa_learnLimit_info, int wlan_idx, int dev_idx);
rtk_rg_err_code_t rtk_rg_apollo_wlanSoftwareSourceAddrLearningLimit_set(rtk_rg_saLearningLimitInfo_t sa_learnLimit_info, int wlan_idx, int dev_idx);

rtk_rg_err_code_t rtk_rg_apollo_wanInterface_add(rtk_rg_wanIntfConf_t *wanintf, int *wan_intf_idx);
rtk_rg_err_code_t rtk_rg_apollo_portIsolation_set(rtk_rg_port_isolation_t isolationSetting);
rtk_rg_err_code_t rtk_rg_apollo_portIsolation_get(rtk_rg_port_isolation_t *isolationSetting);
rtk_rg_err_code_t rtk_rg_apollo_dsliteInfo_set(int wan_intf_idx, rtk_rg_ipDslitStaticInfo_t *dslite_info);
rtk_rg_err_code_t rtk_rg_apollo_pppoeDsliteInfoBeforeDial_set(int wan_intf_idx, rtk_rg_pppoeClientInfoBeforeDial_t *app_info);
rtk_rg_err_code_t rtk_rg_apollo_pppoeDsliteInfoAfterDial_set(int wan_intf_idx, rtk_rg_pppoeDsliteInfoAfterDial_t *pppoeDslite_info);
rtk_rg_err_code_t rtk_rg_apollo_dsliteMcTable_set(rtk_l34_dsliteMc_entry_t *pDsliteMcEntry);
rtk_rg_err_code_t rtk_rg_apollo_dsliteMcTable_get(rtk_l34_dsliteMc_entry_t *pDsliteMcEntry);
rtk_rg_err_code_t rtk_rg_apollo_dsliteControl_set(rtk_l34_dsliteCtrlType_t ctrlType, uint32 act);
rtk_rg_err_code_t rtk_rg_apollo_dsliteControl_get(rtk_l34_dsliteCtrlType_t ctrlType, uint32 *pAct);
rtk_rg_err_code_t rtk_rg_apollo_redirectHttpAll_set(rtk_rg_redirectHttpAll_t *pRedirectHttpAll);
rtk_rg_err_code_t rtk_rg_apollo_redirectHttpAll_get(rtk_rg_redirectHttpAll_t *pRedirectHttpAll);
rtk_rg_err_code_t rtk_rg_apollo_redirectHttpURL_add(rtk_rg_redirectHttpURL_t *pRedirectHttpURL);
rtk_rg_err_code_t rtk_rg_apollo_redirectHttpURL_del(rtk_rg_redirectHttpURL_t *pRedirectHttpURL);
rtk_rg_err_code_t rtk_rg_apollo_redirectHttpWhiteList_add(rtk_rg_redirectHttpWhiteList_t *pRedirectHttpWhiteList);
rtk_rg_err_code_t rtk_rg_apollo_redirectHttpWhiteList_del(rtk_rg_redirectHttpWhiteList_t *pRedirectHttpWhiteList);

rtk_rg_err_code_t rtk_rg_apollo_svlanTpid2_enable_set(rtk_rg_enable_t enable);
rtk_rg_err_code_t rtk_rg_apollo_svlanTpid2_enable_get(rtk_rg_enable_t *pEnable);
rtk_rg_err_code_t rtk_rg_apollo_svlanTpid2_set(uint32 svlan_tag_id);
rtk_rg_err_code_t rtk_rg_apollo_svlanTpid2_get(uint32 *pSvlanTagId);


int rtk_rg_apollo_api_module_init(void);


#define rtk_rg_api_module_init(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_driverVersion_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_initParam_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_initParam_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_lanInterface_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//5
#define rtk_rg_wanInterface_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_staticInfo_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_dhcpRequest_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_dhcpClientInfo_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_pppoeClientInfoBeforeDial_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//10
#define rtk_rg_pppoeClientInfoAfterDial_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_interface_del(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_intfInfo_find(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_cvlan_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_cvlan_del(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//15
#define rtk_rg_cvlan_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_vlanBinding_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_vlanBinding_del(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_vlanBinding_find(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_algServerInLanAppsIpAddr_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//20
#define rtk_rg_algServerInLanAppsIpAddr_del(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_algApps_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_algApps_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_dmzHost_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_dmzHost_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//25
#define rtk_rg_virtualServer_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_virtualServer_del(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_virtualServer_find(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_aclFilterAndQos_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_aclFilterAndQos_del(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//30
#define rtk_rg_aclFilterAndQos_find(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_macFilter_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_macFilter_del(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_macFilter_find(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_urlFilterString_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//35
#define rtk_rg_urlFilterString_del(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_urlFilterString_find(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_upnpConnection_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_upnpConnection_del(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_upnpConnection_find(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//40
#define rtk_rg_naptConnection_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_naptConnection_del(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_naptConnection_find(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_multicastFlow_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_multicastFlow_del(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
/* martin zhu add */
#define rtk_rg_l2MultiCastFlow_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//45
#define rtk_rg_multicastFlow_find(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_macEntry_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_macEntry_del(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_macEntry_find(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_arpEntry_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//50
#define rtk_rg_arpEntry_del(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_arpEntry_find(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_neighborEntry_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_neighborEntry_del(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_neighborEntry_find(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//55
#define rtk_rg_accessWanLimit_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_accessWanLimit_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_accessWanLimitCategory_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_accessWanLimitCategory_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_softwareSourceAddrLearningLimit_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//60
#define rtk_rg_softwareSourceAddrLearningLimit_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_dosPortMaskEnable_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_dosPortMaskEnable_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_dosType_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_dosType_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//65
#define rtk_rg_dosFloodType_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_dosFloodType_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_portMirror_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_portMirror_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_portMirror_clear(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//70
#define rtk_rg_portEgrBandwidthCtrlRate_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_portIgrBandwidthCtrlRate_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_portEgrBandwidthCtrlRate_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_portIgrBandwidthCtrlRate_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_phyPortForceAbility_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//75
#define rtk_rg_phyPortForceAbility_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_cpuPortForceTrafficCtrl_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_cpuPortForceTrafficCtrl_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_portMibInfo_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_portMibInfo_clear(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//80
#define rtk_rg_stormControl_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_stormControl_del(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_stormControl_find(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_shareMeter_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_shareMeter_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//85
#define rtk_rg_qosStrictPriorityOrWeightFairQueue_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_qosStrictPriorityOrWeightFairQueue_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_qosInternalPriMapToQueueId_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_qosInternalPriMapToQueueId_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_qosInternalPriDecisionByWeight_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//90
#define rtk_rg_qosInternalPriDecisionByWeight_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_qosDscpRemapToInternalPri_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_qosDscpRemapToInternalPri_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_qosPortBasedPriority_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_qosPortBasedPriority_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//95
#define rtk_rg_qosDot1pPriRemapToInternalPri_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_qosDot1pPriRemapToInternalPri_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_qosDscpRemarkByInternalPri_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//100
#define rtk_rg_qosDscpRemarkByInternalPri_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_qosDscpRemarkByDscp_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_qosDscpRemarkByDscp_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//105
#define rtk_rg_qosDot1pPriRemarkByInternalPri_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_qosDot1pPriRemarkByInternalPri_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_portBasedCVlanId_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_portBasedCVlanId_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_portStatus_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//110
#ifdef CONFIG_RG_NAPT_PORT_COLLISION_PREVENTION
#define rtk_rg_naptExtPortGet(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_naptExtPortFree(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#endif
#define rtk_rg_classifyEntry_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_classifyEntry_find(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_classifyEntry_del(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//115
#define rtk_rg_svlanTpid_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_svlanTpid_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_svlanServicePort_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_svlanServicePort_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_pppoeInterfaceIdleTime_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//120
#define rtk_rg_gatewayServicePortRegister_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_gatewayServicePortRegister_del(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_gatewayServicePortRegister_find(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_wlanDevBasedCVlanId_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_wlanDevBasedCVlanId_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//125
#define rtk_rg_wlanSoftwareSourceAddrLearningLimit_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_wlanSoftwareSourceAddrLearningLimit_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_naptFilterAndQos_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_naptFilterAndQos_del(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_naptFilterAndQos_find(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//130
#define rtk_rg_pptpClientInfoBeforeDial_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_pptpClientInfoAfterDial_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_l2tpClientInfoBeforeDial_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_l2tpClientInfoAfterDial_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_stpBlockingPortmask_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//135
#define rtk_rg_stpBlockingPortmask_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_portIsolation_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_portIsolation_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_dsliteInfo_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_pppoeDsliteInfoBeforeDial_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//140
#define rtk_rg_pppoeDsliteInfoAfterDial_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_gponDsBcFilterAndRemarking_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_gponDsBcFilterAndRemarking_del(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_gponDsBcFilterAndRemarking_find(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_gponDsBcFilterAndRemarking_del_all(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//145
#define rtk_rg_gponDsBcFilterAndRemarking_Enable(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_dsliteMcTable_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_dsliteMcTable_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_dsliteControl_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_dsliteControl_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//150
#define rtk_rg_interfaceMibCounter_del(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_interfaceMibCounter_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_redirectHttpAll_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_redirectHttpAll_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_redirectHttpURL_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//155
#define rtk_rg_redirectHttpURL_del(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_redirectHttpWhiteList_add(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_redirectHttpWhiteList_del(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()

#define rtk_rg_svlanTpid2_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_svlanTpid2_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
//160
#define rtk_rg_svlanTpid2_enable_get(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()
#define rtk_rg_svlanTpid2_enable_set(arg,args...) PLEASE_USE_RTK_RG_APOLLO_API()

#endif
