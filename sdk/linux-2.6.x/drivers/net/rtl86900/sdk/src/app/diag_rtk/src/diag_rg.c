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
 *
 * $Revision: 65911 $
 * $Date: 2016-02-04 18:09:22 +0800 (Thu, 04 Feb 2016) $
 *
 * Purpose : Definition those XXX command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *
 */

/*
 * Include Files
 */
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <hal/common/halctrl.h>
#include <common/util/rt_util.h>
#include <diag_util.h>
#include <parser/cparser_priv.h>
#include <diag_str.h>

#include <rtk_rg_struct.h>




/*varibles*/
rtk_rg_VersionString_t version_string;
rtk_rg_intfInfo_t intf_info;
rtk_rg_lanIntfConf_t lan_intf;
rtk_rg_wanIntfConf_t wan_intf;
rtk_rg_ipStaticInfo_t wan_intf_static_info;
rtk_rg_ipDslitStaticInfo_t wan_intf_dslite_info;
rtk_rg_naptEntry_t naptFlow;
rtk_rg_naptInfo_t naptInfo;
rtk_rg_macEntry_t macEntry;
rtk_rg_arpEntry_t arpEntry;
rtk_rg_arpInfo_t arpInfo;
rtk_rg_urlFilterString_t urlFilter;
rtk_rg_initParams_t init_param;
rtk_rg_alg_type_t alg_app;
rtk_rg_pppoeClientInfoAfterDial_t clientPppoe_info;
rtk_rg_pppoeClientInfoBeforeDial_t clientPppoe_beforeDiag_info;
rtk_rg_pptpClientInfoAfterDial_t clientPptp_info;
rtk_rg_pptpClientInfoBeforeDial_t clientPptp_beforeDial_info;
rtk_rg_l2tpClientInfoAfterDial_t clientL2tp_info;
rtk_rg_l2tpClientInfoBeforeDial_t clientL2tp_beforeDial_info;
rtk_rg_pppoeDsliteInfoAfterDial_t clientPppoeDslite_info;
rtk_rg_pppoeClientInfoBeforeDial_t clientPppoeDslite_beforeDial_info;
rtk_rg_neighborEntry_t neighborEntry;
rtk_rg_neighborInfo_t neighborInfo;
rtk_rg_ipDhcpClientInfo_t dhcpClient_info;
rtk_rg_vlanBinding_t vlan_binding_info;
rtk_rg_macFilterEntry_t macFilterEntry;
rtk_rg_alg_serverIpMapping_t srvIpMapping;
rtk_rg_virtualServer_t virtual_server;
rtk_rg_cvlan_info_t cvlan_info;
rtk_rg_saLearningLimitInfo_t sa_learnLimit_info;
rtk_rg_phyPortAbilityInfo_t phyPort_ability;
rtk_rg_portMirrorInfo_t portMirrorInfo;
rtk_rg_upnpConnection_t upnpInfo;
rtk_rg_dmzInfo_t dmz_info;
rtk_rg_stormControlInfo_t stormInfo;
rtk_rg_qos_queue_weights_t q_weight;
rtk_rg_qos_pri2queue_t pri2qid;
rtk_rg_qos_priSelWeight_t weightOfPriSel;
rtk_rg_multicastFlow_t mcFlow;
rtk_rg_gatewayServicePortEntry_t gatewayServicePort;
rtk_rg_naptFilterAndQos_t naptFilterAndQos;
rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t gponDsBcFilterAndRemarking;
rtk_rg_redirectHttpAll_t redInfoAll;

/*internal function*/

void _diag_lanIntfShow(rtk_rg_lanIntfConf_t *lan_intf){
	diag_util_mprintf("ip_version: %d \n",lan_intf->ip_version);
	diag_util_mprintf("gateway-mac: %02X:%02X:%02X:%02X:%02X:%02X \n",
		lan_intf->gmac.octet[0],
		lan_intf->gmac.octet[1],
		lan_intf->gmac.octet[2],
		lan_intf->gmac.octet[3],
		lan_intf->gmac.octet[4],
		lan_intf->gmac.octet[5]);
	diag_util_mprintf("ip-addr: %s \n",diag_util_inet_ntoa(lan_intf->ip_addr));
	diag_util_mprintf("ip-mask: %s \n",diag_util_inet_ntoa(lan_intf->ip_network_mask));
	diag_util_mprintf("ipv6_addr: %s\n",diag_util_inet_n6toa( &lan_intf->ipv6_addr.ipv6_addr[0]));
	diag_util_mprintf("ipv6_network_mask_length: %d \n",lan_intf->ipv6_network_mask_length);
	diag_util_mprintf("port-mask: 0x%x \n",lan_intf->port_mask);
	diag_util_mprintf("untag_mask: 0x%x \n",lan_intf->untag_mask);
	diag_util_mprintf("intf-vlan_id: %d \n",lan_intf->intf_vlan_id);
	if(lan_intf->vlan_based_pri_enable==RTK_RG_ENABLED)
	{
		diag_util_mprintf("vlan-based-enable: enabled, pri:%d \n",lan_intf->vlan_based_pri);
	}
	else
	{
		diag_util_mprintf("vlan-based-enable: disabled \n");
	}
	diag_util_mprintf("mtu: %d \n",lan_intf->mtu);
	diag_util_mprintf("isIVL: %d \n",lan_intf->isIVL);
	diag_util_mprintf("replace_subnet: %d \n",lan_intf->replace_subnet);
}

void _diag_wanIntfShow(rtk_rg_wanIntfConf_t *wan_intf){
	diag_util_mprintf("wan_type: %d \n",wan_intf->wan_type);	
	diag_util_mprintf("gateway-mac: %02X:%02X:%02X:%02X:%02X:%02X \n",
		wan_intf->gmac.octet[0],
		wan_intf->gmac.octet[1],
		wan_intf->gmac.octet[2],
		wan_intf->gmac.octet[3],
		wan_intf->gmac.octet[4],
		wan_intf->gmac.octet[5]);
	diag_util_mprintf("wan-port: %d \n",wan_intf->wan_port_idx);
	diag_util_mprintf("port-binding-mask: 0x%x \n",wan_intf->port_binding_mask.portmask);
	diag_util_mprintf("vlan-binding-mask: 0x%x \n",wan_intf->vlan_binding_mask);
	diag_util_mprintf("egress-vlan-tag-on: %d \n",wan_intf->egress_vlan_tag_on);
	diag_util_mprintf("egress-vlan-id: %d \n",wan_intf->egress_vlan_id);
	if(wan_intf->vlan_based_pri_enable==RTK_RG_ENABLED)
	{
		diag_util_mprintf("vlan-based-pri-enabled: enabled, pri:%d \n",wan_intf->vlan_based_pri);
	}
	else
	{
		diag_util_mprintf("vlan-based-pri-enabled: disabled \n");
	}
	diag_util_mprintf("isIVL: %d \n",wan_intf->isIVL);
	diag_util_mprintf("none_internet: %d \n",wan_intf->none_internet);
	diag_util_mprintf("wlan0-binding-mask: 0x%x \n",wan_intf->wlan0_dev_binding_mask);
	diag_util_mprintf("replaceSame: %s \n",wan_intf->forcedAddNewIntf==1?"0":"1");
}

void _diag_wanIntfStaticInfoShow(rtk_rg_ipStaticInfo_t *wan_intf_static_info){
	diag_util_mprintf("ip_version: %d \n",wan_intf_static_info->ip_version);
	diag_util_mprintf("napt_enable: %d \n",wan_intf_static_info->napt_enable);
	diag_util_mprintf("ip-addr: %s \n",diag_util_inet_ntoa(wan_intf_static_info->ip_addr));
	diag_util_mprintf("ip_network_mask: %s \n",diag_util_inet_ntoa(wan_intf_static_info->ip_network_mask));
	diag_util_mprintf("ipv4_default_gateway_on: %d \n",wan_intf_static_info->ipv4_default_gateway_on);
	diag_util_mprintf("gateway_ipv4_addr: %s \n",diag_util_inet_ntoa(wan_intf_static_info->gateway_ipv4_addr));
	diag_util_mprintf("ipv6_addr: %s\n",diag_util_inet_n6toa( &wan_intf_static_info->ipv6_addr.ipv6_addr[0]));
	diag_util_mprintf("ipv6_mask_length: %d \n",wan_intf_static_info->ipv6_mask_length);
	diag_util_mprintf("ipv6_default_gateway_on: %d \n",wan_intf_static_info->ipv6_default_gateway_on);
	diag_util_mprintf("gateway_ipv6_addr: %s\n",diag_util_inet_n6toa( &wan_intf_static_info->gateway_ipv6_addr.ipv6_addr[0]));
	diag_util_mprintf("mtu: %d \n",wan_intf_static_info->mtu);
	diag_util_mprintf("routing_type: %s \n",wan_intf_static_info->static_route_with_arp==1?"route as ARP with NH for policy route":"route as NH for static route");
	
	int gw_mac_auto_learn_for_ipv4;
	int gw_mac_auto_learn_for_ipv6;
	rtk_mac_t gateway_mac_addr_for_ipv4;
	rtk_mac_t gateway_mac_addr_for_ipv6;
	diag_util_mprintf("gw_mac_auto_learn_for_ipv4: %d \n",wan_intf_static_info->gw_mac_auto_learn_for_ipv4);
		diag_util_mprintf("gateway_mac_addr_for_ipv4: %02X:%02X:%02X:%02X:%02X:%02X \n",
		wan_intf_static_info->gateway_mac_addr_for_ipv4.octet[0],
		wan_intf_static_info->gateway_mac_addr_for_ipv4.octet[1],
		wan_intf_static_info->gateway_mac_addr_for_ipv4.octet[2],
		wan_intf_static_info->gateway_mac_addr_for_ipv4.octet[3],
		wan_intf_static_info->gateway_mac_addr_for_ipv4.octet[4],
		wan_intf_static_info->gateway_mac_addr_for_ipv4.octet[5]);
	
	diag_util_mprintf("gw_mac_auto_learn_for_ipv6: %d \n",wan_intf_static_info->gw_mac_auto_learn_for_ipv6);
	diag_util_mprintf("gateway_mac_addr_for_ipv6: %02X:%02X:%02X:%02X:%02X:%02X \n",
		wan_intf_static_info->gateway_mac_addr_for_ipv6.octet[0],
		wan_intf_static_info->gateway_mac_addr_for_ipv6.octet[1],
		wan_intf_static_info->gateway_mac_addr_for_ipv6.octet[2],
		wan_intf_static_info->gateway_mac_addr_for_ipv6.octet[3],
		wan_intf_static_info->gateway_mac_addr_for_ipv6.octet[4],
		wan_intf_static_info->gateway_mac_addr_for_ipv6.octet[5]);
}

void _diag_macEntryShow(rtk_rg_macEntry_t *macEntry){
	diag_util_mprintf("mac: %02X:%02X:%02X:%02X:%02X:%02X \n",
		macEntry->mac.octet[0],
		macEntry->mac.octet[1],
		macEntry->mac.octet[2],
		macEntry->mac.octet[3],
		macEntry->mac.octet[4],
		macEntry->mac.octet[5]);
	diag_util_mprintf("isIVL: %d \n",macEntry->isIVL);
	diag_util_mprintf("fid: %d \n",macEntry->fid);
	diag_util_mprintf("vlan_id: %d \n",macEntry->vlan_id);
	diag_util_mprintf("port_idx: %d \n",macEntry->port_idx);
	diag_util_mprintf("static_entry: %d \n",macEntry->static_entry);
	diag_util_mprintf("auth: %d \n",macEntry->auth);	
#if defined(CONFIG_RTL9602C_SERIES)	
	diag_util_mprintf("ctag_if: %d \n",macEntry->ctag_if);
#endif
}

void _diag_arpEntryShow(rtk_rg_arpEntry_t *arpEntry){
	diag_util_mprintf("macEntryIdx: %d \n",arpEntry->macEntryIdx);
	diag_util_mprintf("ip: %s \n",diag_util_inet_ntoa(arpEntry->ipv4Addr));
	diag_util_mprintf("static_entry: %d \n",arpEntry->staticEntry);
}

void _diag_lanNetInfoShow(rtk_rg_lanNetInfo_t *lanNetInfo){
	rtk_rg_arpInfo_t *arpInfo = container_of(lanNetInfo, rtk_rg_arpInfo_t, lanNetInfo);
	diag_util_mprintf("devName: %s \n",lanNetInfo->dev_name);
	diag_util_mprintf("devType: %d \n",lanNetInfo->dev_type);
	diag_util_mprintf("brand: %d %s\n",lanNetInfo->brand,arpInfo->brandStr);
	diag_util_mprintf("os: %d %s\n",lanNetInfo->os,arpInfo->osStr);
	diag_util_mprintf("connType: %d \n",lanNetInfo->conn_type);
}

void _diag_naptConnShow(rtk_rg_naptInfo_t *naptInfo){
	diag_util_mprintf("is_tcp: %d \n",naptInfo->naptTuples.is_tcp);
	diag_util_mprintf("local_ip: %s \n",diag_util_inet_ntoa(naptInfo->naptTuples.local_ip));
	diag_util_mprintf("remote_ip: %s \n",diag_util_inet_ntoa(naptInfo->naptTuples.remote_ip));
	diag_util_mprintf("wan_intf_idx: %d \n",naptInfo->naptTuples.wan_intf_idx);
	diag_util_mprintf("local_port: %d \n",naptInfo->naptTuples.local_port);
	diag_util_mprintf("remote_port: %d \n",naptInfo->naptTuples.remote_port);
	diag_util_mprintf("external_port: %d \n",naptInfo->naptTuples.external_port);
	diag_util_mprintf("outbound_pri_valid: %d \n",naptInfo->naptTuples.outbound_pri_valid);
	diag_util_mprintf("outbound_priority: %d \n",naptInfo->naptTuples.outbound_priority);
	diag_util_mprintf("inbound_pri_valid: %d \n",naptInfo->naptTuples.inbound_pri_valid);
	diag_util_mprintf("inbound_priority: %d \n",naptInfo->naptTuples.inbound_priority);
}

void _diag_urlFilterShow(rtk_rg_urlFilterString_t* urlFilter){
	diag_util_mprintf("url_filter_string: %s \n",urlFilter->url_filter_string);
	diag_util_mprintf("path_filter_string: %s \n",urlFilter->path_filter_string);
	diag_util_mprintf("path_exactly_match: %d \n",urlFilter->path_exactly_match);
	diag_util_mprintf("wan_intf: %d \n",urlFilter->wan_intf);
}

void _diag_callbackShow(rtk_rg_initParams_t* init_param){
	diag_util_mprintf("initByHwCallBack: 0x%x \n",init_param->initByHwCallBack);
	diag_util_mprintf("arpAddByHwCallBack: 0x%x \n",init_param->arpAddByHwCallBack);
	diag_util_mprintf("arpDelByHwCallBack: 0x%x \n",init_param->arpDelByHwCallBack);
	diag_util_mprintf("macAddByHwCallBack: 0x%x \n",init_param->macAddByHwCallBack);
	diag_util_mprintf("macDelByHwCallBack: 0x%x \n",init_param->macDelByHwCallBack);
	diag_util_mprintf("routingAddByHwCallBack: 0x%x \n",init_param->routingAddByHwCallBack);
	diag_util_mprintf("routingDelByHwCallBack: 0x%x \n",init_param->routingDelByHwCallBack);
	diag_util_mprintf("naptAddByHwCallBack: 0x%x \n",init_param->naptAddByHwCallBack);
	diag_util_mprintf("naptDelByHwCallBack: 0x%x \n",init_param->naptDelByHwCallBack);
	diag_util_mprintf("bindingAddByHwCallBack: 0x%x \n",init_param->bindingAddByHwCallBack);
	diag_util_mprintf("bindingDelByHwCallBack: 0x%x \n",init_param->bindingDelByHwCallBack);
	diag_util_mprintf("interfaceAddByHwCallBack: 0x%x \n",init_param->interfaceAddByHwCallBack);
	diag_util_mprintf("interfaceDelByHwCallBack: 0x%x \n",init_param->interfaceDelByHwCallBack);
	diag_util_mprintf("neighborAddByHwCallBack: 0x%x \n",init_param->neighborAddByHwCallBack);
	diag_util_mprintf("neighborDelByHwCallBack: 0x%x \n",init_param->neighborDelByHwCallBack);
	diag_util_mprintf("v6RoutingAddByHwCallBack: 0x%x \n",init_param->v6RoutingAddByHwCallBack);
	diag_util_mprintf("v6RoutingDelByHwCallBack: 0x%x \n",init_param->v6RoutingDelByHwCallBack);
	diag_util_mprintf("pppoeBeforeDiagByHwCallBack: 0x%x \n",init_param->pppoeBeforeDiagByHwCallBack);	

}
 
void _diag_algAppShow(rtk_rg_alg_type_t* alg_app){

}

void _diag_pppoeClientInfoShow(rtk_rg_pppoeClientInfoAfterDial_t* clientPppoe_info){
	diag_util_mprintf("SessionID: %d \n",clientPppoe_info->sessionId);
	_diag_wanIntfStaticInfoShow(&clientPppoe_info->hw_info);
}

void _diag_pptpClientInfoShow(rtk_rg_pptpClientInfoAfterDial_t* clientPptp_info){
	diag_util_mprintf("IPv4_napt: %d \n",clientPptp_info->hw_info.napt_enable);
	diag_util_mprintf("IPv4_default_gw_on: %d \n",clientPptp_info->hw_info.ipv4_default_gateway_on);
	diag_util_mprintf("IP: %s \n",diag_util_inet_ntoa(clientPptp_info->hw_info.ip_addr));
	diag_util_mprintf("mask: %s \n",diag_util_inet_ntoa(clientPptp_info->hw_info.ip_network_mask));
	diag_util_mprintf("Gateway IP: %s \n",diag_util_inet_ntoa(clientPptp_info->hw_info.gateway_ipv4_addr));
	diag_util_mprintf("CallID: %d \n",clientPptp_info->callId);
	diag_util_mprintf("Gateway CallID: %d \n",clientPptp_info->gateway_callId);
}

void _diag_l2tpClientInfoShow(rtk_rg_l2tpClientInfoAfterDial_t* clientL2tp_info){
	diag_util_mprintf("IPv4_napt: %d \n",clientL2tp_info->hw_info.napt_enable);
	diag_util_mprintf("IPv4_default_gw_on: %d \n",clientL2tp_info->hw_info.ipv4_default_gateway_on);
	diag_util_mprintf("IP: %s \n",diag_util_inet_ntoa(clientL2tp_info->hw_info.ip_addr));
	diag_util_mprintf("mask: %s \n",diag_util_inet_ntoa(clientL2tp_info->hw_info.ip_network_mask));
	diag_util_mprintf("Gateway IP: %s \n",diag_util_inet_ntoa(clientL2tp_info->hw_info.gateway_ipv4_addr));
	diag_util_mprintf("Outer Port: %d \n",clientL2tp_info->outer_port);
	diag_util_mprintf("Gateway Outer Port: %d \n",clientL2tp_info->gateway_outer_port);
	diag_util_mprintf("TunnelID: %d \n",clientL2tp_info->tunnelId);
	diag_util_mprintf("SessionID: %d \n",clientL2tp_info->sessionId);	
	diag_util_mprintf("Gateway TunnelID: %d \n",clientL2tp_info->gateway_tunnelId);
	diag_util_mprintf("Gateway SessionID: %d \n",clientL2tp_info->gateway_sessionId);
}

void _diag_dsliteInfoShow(rtk_rg_ipDslitStaticInfo_t* client_info){
	_diag_wanIntfStaticInfoShow(&client_info->static_info);
	diag_util_mprintf("b4_addr: %s\n",diag_util_inet_n6toa( &client_info->rtk_dslite.ipB4.ipv6_addr[0]));
	diag_util_mprintf("aftr_addr: %s\n",diag_util_inet_n6toa( &client_info->rtk_dslite.ipAftr.ipv6_addr[0]));
	diag_util_mprintf("aftr_mac_auto_learn: %d \n",client_info->aftr_mac_auto_learn);
	diag_util_mprintf("gateway_mac_addr_for_ipv6: %02X:%02X:%02X:%02X:%02X:%02X \n",
		client_info->aftr_mac_addr.octet[0],
		client_info->aftr_mac_addr.octet[1],
		client_info->aftr_mac_addr.octet[2],
		client_info->aftr_mac_addr.octet[3],
		client_info->aftr_mac_addr.octet[4],
		client_info->aftr_mac_addr.octet[5]);
#if defined(CONFIG_RTL9602C_SERIES)
	diag_util_mprintf("hoplimit: %d \n",client_info->rtk_dslite.hopLimit);
	diag_util_mprintf("flowlabel: %x \n",client_info->rtk_dslite.flowLabel);
	if(client_info->rtk_dslite.tcOpt==RTK_L34_DSLITE_TC_OPT_ASSIGN)
		diag_util_mprintf("tc: %x \n",client_info->rtk_dslite.tc);
	else
		diag_util_mprintf("tc: from IPv4 TOS\n");
#endif
}

void _diag_pppoeDsliteInfoShow(rtk_rg_pppoeDsliteInfoAfterDial_t* clientPppoe_info){
	diag_util_mprintf("SessionID: %d \n",clientPppoe_info->sessionId);
	_diag_dsliteInfoShow(&clientPppoe_info->dslite_hw_info);
}

void _diag_dhcpClientInfoShow(rtk_rg_ipDhcpClientInfo_t* dhcpClient_info){
	diag_util_mprintf("stauts: %d \n",dhcpClient_info->stauts);
	_diag_wanIntfStaticInfoShow(&dhcpClient_info->hw_info);
}

void _diag_pppoeClientInfoBeforeDiagShow(rtk_rg_pppoeClientInfoBeforeDial_t* clientPppoe_beforeDiag_info){
	diag_util_mprintf("username: %s \n",clientPppoe_beforeDiag_info->username);
	diag_util_mprintf("password: %s \n",clientPppoe_beforeDiag_info->password);
	diag_util_mprintf("auth_type: %d \n",clientPppoe_beforeDiag_info->auth_type);
	diag_util_mprintf("pppoe_proxy_enable: %d \n",clientPppoe_beforeDiag_info->pppoe_proxy_enable);
	diag_util_mprintf("max_pppoe_proxy_num: %d \n",clientPppoe_beforeDiag_info->max_pppoe_proxy_num);
	diag_util_mprintf("auto_reconnect: %d \n",clientPppoe_beforeDiag_info->auto_reconnect);
	diag_util_mprintf("dial_on_demond: %d \n",clientPppoe_beforeDiag_info->dial_on_demond);
	diag_util_mprintf("idle_timeout_secs: %d \n",clientPppoe_beforeDiag_info->idle_timeout_secs);
	diag_util_mprintf("stauts: %s \n",clientPppoe_beforeDiag_info->stauts);
	diag_util_mprintf("dialOnDemondCallBack: %p \n",clientPppoe_beforeDiag_info->dialOnDemondCallBack);
	diag_util_mprintf("idleTimeOutCallBack: %p \n",clientPppoe_beforeDiag_info->idleTimeOutCallBack);
			
}

void _diag_pptpClientInfoBeforeDialShow(rtk_rg_pptpClientInfoBeforeDial_t* clientPptp_beforeDial_info){
	diag_util_mprintf("username: %s \n",clientPptp_beforeDial_info->username);
	diag_util_mprintf("password: %s \n",clientPptp_beforeDial_info->password);
	diag_util_mprintf("pptp_server_addr_for_ipv4: %s \n",diag_util_inet_ntoa(clientPptp_beforeDial_info->pptp_ipv4_addr));
}

void _diag_l2tpClientInfoBeforeDialShow(rtk_rg_l2tpClientInfoBeforeDial_t* clientL2tp_beforeDial_info){
	diag_util_mprintf("username: %s \n",clientL2tp_beforeDial_info->username);
	diag_util_mprintf("password: %s \n",clientL2tp_beforeDial_info->password);
	diag_util_mprintf("l2tp_server_addr_for_ipv4: %s \n",diag_util_inet_ntoa(clientL2tp_beforeDial_info->l2tp_ipv4_addr));
}
 
void _diag_neighborEntryShow(rtk_rg_neighborEntry_t* neighborEntry){
	//wait for struct sure!
	diag_util_mprintf("l2Idx: %d \n",neighborEntry->l2Idx);
	diag_util_mprintf("matchRouteIdx: %d \n",neighborEntry->matchRouteIdx);
	diag_util_mprintf("interfaceId: %02x%02x%02x%02x:%02x%02x%02x%02x\n",
		neighborEntry->interfaceId[0],
		neighborEntry->interfaceId[1],
		neighborEntry->interfaceId[2],
		neighborEntry->interfaceId[3],
		neighborEntry->interfaceId[4],
		neighborEntry->interfaceId[5],
		neighborEntry->interfaceId[6],
		neighborEntry->interfaceId[7]);
	diag_util_mprintf("valid: %d \n",neighborEntry->valid);
	diag_util_mprintf("staticEntry: %d \n",neighborEntry->staticEntry);

}

void _diag_bindingShow(rtk_rg_vlanBinding_t *vlan_binding_info){
	diag_util_mprintf("port_idx: %d \n",vlan_binding_info->port_idx);
	diag_util_mprintf("ingress_vid: %d \n",vlan_binding_info->ingress_vid);
	diag_util_mprintf("wan_intf_idx: %d \n",vlan_binding_info->wan_intf_idx);
}

void _diag_macFilterEntryShow(rtk_rg_macFilterEntry_t *macFilterEntry){
	diag_util_mprintf("mac: %02X:%02X:%02X:%02X:%02X:%02X \n",
	macFilterEntry->mac.octet[0],
	macFilterEntry->mac.octet[1],
	macFilterEntry->mac.octet[2],
	macFilterEntry->mac.octet[3],
	macFilterEntry->mac.octet[4],
	macFilterEntry->mac.octet[5]);
	diag_util_mprintf("direct: %d \n",macFilterEntry->direct);
}

void _diag_virtualServerShow(rtk_rg_virtualServer_t* virtual_server){
	diag_util_mprintf("ipversion: %d \n",virtual_server->ipversion);
	diag_util_mprintf("is_tcp: %d \n",virtual_server->is_tcp);
	diag_util_mprintf("wan_intf_idx: %d \n",virtual_server->wan_intf_idx);
	diag_util_mprintf("gateway_port_start: %d \n",virtual_server->gateway_port_start);
	diag_util_mprintf("local_ip: 0x%x \n",virtual_server->local_ip);
	diag_util_mprintf("local_ipv6: %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
		virtual_server->local_ipv6.ipv6_addr[0],virtual_server->local_ipv6.ipv6_addr[1],virtual_server->local_ipv6.ipv6_addr[2],virtual_server->local_ipv6.ipv6_addr[3],
		virtual_server->local_ipv6.ipv6_addr[4],virtual_server->local_ipv6.ipv6_addr[5],virtual_server->local_ipv6.ipv6_addr[6],virtual_server->local_ipv6.ipv6_addr[7],		
		virtual_server->local_ipv6.ipv6_addr[8],virtual_server->local_ipv6.ipv6_addr[9],virtual_server->local_ipv6.ipv6_addr[10],virtual_server->local_ipv6.ipv6_addr[11],
		virtual_server->local_ipv6.ipv6_addr[12],virtual_server->local_ipv6.ipv6_addr[13],virtual_server->local_ipv6.ipv6_addr[14],virtual_server->local_ipv6.ipv6_addr[15]);
	diag_util_mprintf("local_port_start: %d \n",virtual_server->local_port_start);
	diag_util_mprintf("mappingPortRangeCnt: %d \n",virtual_server->mappingPortRangeCnt);
	diag_util_mprintf("mappingType: %d\n",virtual_server->mappingType);
	diag_util_mprintf("valid: %d \n",virtual_server->valid);
	diag_util_mprintf("hookAlgType: %x \n",virtual_server->hookAlgType);
}
void _diag_cvlanInfoShow(rtk_rg_cvlan_info_t* cvlan_info){
	diag_util_mprintf("vlanId: %d \n",cvlan_info->vlanId);
	diag_util_mprintf("isIVL: %d \n",cvlan_info->isIVL);
	diag_util_mprintf("memberPortMask: 0x%x \n",cvlan_info->memberPortMask.portmask);
	diag_util_mprintf("untagPortMask: 0x%x \n",cvlan_info->untagPortMask.portmask);
	if(cvlan_info->vlan_based_pri_enable==RTK_RG_ENABLED)
		diag_util_mprintf("vlan-based-priority: ENABLE,%d \n",cvlan_info->vlan_based_pri);
}

void _diag_upnpInfoShow(rtk_rg_upnpConnection_t* upnpInfo){
	diag_util_mprintf("is_tcp: %d \n",upnpInfo->is_tcp);
	diag_util_mprintf("valid: %d \n",upnpInfo->valid);
	diag_util_mprintf("wan_intf_idx: %d \n",upnpInfo->wan_intf_idx);
	diag_util_mprintf("gateway_port: %d \n",upnpInfo->gateway_port);
	diag_util_mprintf("local_ip: 0x%x \n",upnpInfo->local_ip);
	diag_util_mprintf("local_port: %d \n",upnpInfo->local_port);
	diag_util_mprintf("limit_remote_ip: %d \n",upnpInfo->limit_remote_ip);
	diag_util_mprintf("limit_remote_port: %d \n",upnpInfo->limit_remote_port);
	diag_util_mprintf("remote_ip: 0x%x \n",upnpInfo->remote_ip);
	diag_util_mprintf("remote_port: %d \n",upnpInfo->remote_port);
	diag_util_mprintf("type: %d \n",upnpInfo->type);
	diag_util_mprintf("timeout: %d \n",upnpInfo->timeout);
	diag_util_mprintf("idle: %d \n",upnpInfo->idle);
}
void _diag_stormControlInfoShow(rtk_rg_stormControlInfo_t *stormInfo){
	diag_util_mprintf("valid: %d \n",stormInfo->valid);
	diag_util_mprintf("port: %d \n",stormInfo->port);
	diag_util_mprintf("stormType: %d \n",stormInfo->stormType);
	diag_util_mprintf("meterIdx: %d \n",stormInfo->meterIdx);
}
void _diag_multicastFlowShow(rtk_rg_multicastFlow_t *mcFlow){
	diag_util_mprintf("multicast_ipv4_addr: %s \n",diag_util_inet_ntoa(mcFlow->multicast_ipv4_addr));
	diag_util_mprintf("multicast_ipv6_addr: %s\n",diag_util_inet_n6toa( &mcFlow->multicast_ipv6_addr[0]));
	diag_util_mprintf("isIPv6: %d \n",mcFlow->isIPv6);
	diag_util_mprintf("port_mask: 0x%x \n",mcFlow->port_mask);
	diag_util_mprintf("isIVL: %d \n",mcFlow->isIVL);
	diag_util_mprintf("VLANID: %d \n",mcFlow->vlanID);
}

void _diag_rgInitShow(rtk_rg_initParams_t *init_param){
	diag_util_mprintf("igmpSnoopingEnable: %d \n",init_param->igmpSnoopingEnable);	
	diag_util_mprintf("macBasedTagDecision: %d \n",init_param->macBasedTagDecision);
	diag_util_mprintf("wanPortGponMode: %d \n",init_param->wanPortGponMode);
	diag_util_mprintf("ivlMulticastSupport: %d \n",init_param->ivlMulticastSupport);

	if(init_param->initByHwCallBack!=NULL) diag_util_mprintf("initByHwCallBack: 0x%x \n",init_param->initByHwCallBack);
	if(init_param->arpAddByHwCallBack!=NULL) diag_util_mprintf("arpAddByHwCallBack: 0x%x \n",init_param->arpAddByHwCallBack);
	if(init_param->arpDelByHwCallBack!=NULL) diag_util_mprintf("arpDelByHwCallBack: 0x%x \n",init_param->arpDelByHwCallBack);
	if(init_param->macAddByHwCallBack!=NULL) diag_util_mprintf("macAddByHwCallBack: 0x%x \n",init_param->macAddByHwCallBack);
	if(init_param->macDelByHwCallBack!=NULL) diag_util_mprintf("macDelByHwCallBack: 0x%x \n",init_param->macDelByHwCallBack);
	if(init_param->routingAddByHwCallBack!=NULL) diag_util_mprintf("routingAddByHwCallBack: 0x%x \n",init_param->routingAddByHwCallBack);
	if(init_param->routingDelByHwCallBack!=NULL) diag_util_mprintf("routingDelByHwCallBack: 0x%x \n",init_param->routingDelByHwCallBack);
	if(init_param->naptAddByHwCallBack!=NULL) diag_util_mprintf("naptAddByHwCallBack: 0x%x \n",init_param->naptAddByHwCallBack);
	if(init_param->naptDelByHwCallBack!=NULL) diag_util_mprintf("naptDelByHwCallBack: 0x%x \n",init_param->naptDelByHwCallBack);
	if(init_param->bindingAddByHwCallBack!=NULL) diag_util_mprintf("bindingAddByHwCallBack: 0x%x \n",init_param->bindingAddByHwCallBack);
	if(init_param->bindingDelByHwCallBack!=NULL) diag_util_mprintf("bindingDelByHwCallBack: 0x%x \n",init_param->bindingDelByHwCallBack);
	if(init_param->interfaceAddByHwCallBack!=NULL) diag_util_mprintf("interfaceAddByHwCallBack: 0x%x \n",init_param->interfaceAddByHwCallBack);
	if(init_param->interfaceDelByHwCallBack!=NULL) diag_util_mprintf("interfaceDelByHwCallBack: 0x%x \n",init_param->interfaceDelByHwCallBack);
	if(init_param->neighborAddByHwCallBack!=NULL) diag_util_mprintf("neighborAddByHwCallBack: 0x%x \n",init_param->neighborAddByHwCallBack);
	if(init_param->neighborDelByHwCallBack!=NULL) diag_util_mprintf("neighborDelByHwCallBack: 0x%x \n",init_param->neighborDelByHwCallBack);
	if(init_param->v6RoutingAddByHwCallBack!=NULL) diag_util_mprintf("v6RoutingAddByHwCallBack: 0x%x \n",init_param->v6RoutingAddByHwCallBack);
	if(init_param->v6RoutingDelByHwCallBack!=NULL) diag_util_mprintf("v6RoutingDelByHwCallBack: 0x%x \n",init_param->v6RoutingDelByHwCallBack);
	if(init_param->pppoeBeforeDiagByHwCallBack!=NULL) diag_util_mprintf("pppoeBeforeDiagByHwCallBack: 0x%x \n",init_param->pppoeBeforeDiagByHwCallBack);
	if(init_param->pptpBeforeDialByHwCallBack!=NULL) diag_util_mprintf("pptpBeforeDialByHwCallBack: 0x%x \n",init_param->pptpBeforeDialByHwCallBack);
	if(init_param->l2tpBeforeDialByHwCallBack!=NULL) diag_util_mprintf("l2tpBeforeDialByHwCallBack: 0x%x \n",init_param->l2tpBeforeDialByHwCallBack);
	if(init_param->naptInboundConnLookupFirstCallBack!=NULL) diag_util_mprintf("naptInboundConnLookupFirstCallBack: 0x%x \n",init_param->naptInboundConnLookupFirstCallBack);
	if(init_param->naptInboundConnLookupSecondCallBack!=NULL) diag_util_mprintf("naptInboundConnLookupSecondCallBack: 0x%x \n",init_param->naptInboundConnLookupSecondCallBack);
	if(init_param->naptInboundConnLookupThirdCallBack!=NULL) diag_util_mprintf("naptInboundConnLookupThirdCallBack: 0x%x \n",init_param->naptInboundConnLookupThirdCallBack);
	if(init_param->dhcpRequestByHwCallBack!=NULL) diag_util_mprintf("dhcpRequestByHwCallBack: 0x%x \n",init_param->dhcpRequestByHwCallBack);

}


void _diag_rgGatewayServicePortShow(rtk_rg_gatewayServicePortEntry_t *gatewayServicePort){
	diag_util_mprintf("valid: %d \n",gatewayServicePort->valid);	
	diag_util_mprintf("port_num: %d \n",gatewayServicePort->port_num);
	diag_util_mprintf("type: %s \n",gatewayServicePort->type?"SERVER(dport)":"CLIENT(sport)");
}

void _diag_rgNaptFilterAndQosShow(rtk_rg_naptFilterAndQos_t *naptFilterAndQos){

	diag_util_mprintf("direction: %s \n",naptFilterAndQos->direction?"Upstream":"Downstream");
	diag_util_mprintf("patterns: 0x%x \n",naptFilterAndQos->filter_fields);

	if(naptFilterAndQos->filter_fields&INGRESS_SIP)
		diag_util_mprintf("ingress_src_ipv4_addr: 0x%x \n",naptFilterAndQos->ingress_src_ipv4_addr);
	if(naptFilterAndQos->filter_fields&EGRESS_SIP)
		diag_util_mprintf("egress_src_ipv4_addr: 0x%x \n",naptFilterAndQos->egress_src_ipv4_addr);
	if(naptFilterAndQos->filter_fields&INGRESS_DIP)
		diag_util_mprintf("ingress_dest_ipv4_addr: 0x%x \n",naptFilterAndQos->ingress_dest_ipv4_addr);
	if(naptFilterAndQos->filter_fields&EGRESS_DIP)
		diag_util_mprintf("egress_dest_ipv4_addr: 0x%x \n",naptFilterAndQos->egress_dest_ipv4_addr);
	if(naptFilterAndQos->filter_fields&INGRESS_SPORT)
		diag_util_mprintf("ingress_src_l4_port: %d \n",naptFilterAndQos->ingress_src_l4_port);
	if(naptFilterAndQos->filter_fields&EGRESS_SPORT)
		diag_util_mprintf("egress_src_l4_port: %d \n",naptFilterAndQos->egress_src_l4_port);
	if(naptFilterAndQos->filter_fields&INGRESS_DPORT)
		diag_util_mprintf("ingress_dest_l4_port: %d \n",naptFilterAndQos->ingress_dest_l4_port);
	if(naptFilterAndQos->filter_fields&EGRESS_DPORT)
		diag_util_mprintf("egress_dest_l4_port: %d \n",naptFilterAndQos->egress_dest_l4_port);
	if(naptFilterAndQos->filter_fields&INGRESS_SIP_RANGE)
		diag_util_mprintf("ingress_src_ipv4_addr_range: 0x%x ~ 0x%x\n",naptFilterAndQos->ingress_src_ipv4_addr_range_start,naptFilterAndQos->ingress_src_ipv4_addr_range_end);
	if(naptFilterAndQos->filter_fields&INGRESS_DIP_RANGE)
		diag_util_mprintf("ingress_dest_ipv4_addr_range: 0x%x ~ 0x%x\n",naptFilterAndQos->ingress_dest_ipv4_addr_range_start,naptFilterAndQos->ingress_dest_ipv4_addr_range_end);
	if(naptFilterAndQos->filter_fields&INGRESS_SPORT_RANGE)
		diag_util_mprintf("ingress_src_l4_port_range: %d ~ %d\n",naptFilterAndQos->ingress_src_l4_port_range_start,naptFilterAndQos->ingress_src_l4_port_range_end);
	if(naptFilterAndQos->filter_fields&INGRESS_DPORT_RANGE)
		diag_util_mprintf("ingress_dest_l4_port_range: %d ~ %d\n",naptFilterAndQos->ingress_dest_l4_port_range_start,naptFilterAndQos->ingress_dest_l4_port_range_start);
	if(naptFilterAndQos->filter_fields&EGRESS_SIP_RANGE)
		diag_util_mprintf("egress_src_ipv4_addr_range: 0x%x ~ 0x%x\n",naptFilterAndQos->egress_src_ipv4_addr_range_start,naptFilterAndQos->egress_src_ipv4_addr_range_end);
	if(naptFilterAndQos->filter_fields&EGRESS_DIP_RANGE)
		diag_util_mprintf("egress_dest_ipv4_addr_range: 0x%x ~ 0x%x\n",naptFilterAndQos->egress_dest_ipv4_addr_range_start,naptFilterAndQos->egress_dest_ipv4_addr_range_end);
	if(naptFilterAndQos->filter_fields&EGRESS_SPORT_RANGE)
		diag_util_mprintf("egress_src_l4_port_range: %d ~ %d\n",naptFilterAndQos->egress_src_l4_port_range_start,naptFilterAndQos->egress_src_l4_port_range_end);
	if(naptFilterAndQos->filter_fields&EGRESS_DPORT_RANGE)
		diag_util_mprintf("egress_dest_l4_port_range: %d ~ %d\n",naptFilterAndQos->egress_dest_l4_port_range_start,naptFilterAndQos->egress_dest_l4_port_range_end);
	if(naptFilterAndQos->filter_fields&L4_PROTOCAL)
		diag_util_mprintf("ingress_l4_protocal: %d \n",naptFilterAndQos->ingress_l4_protocal);
	
	diag_util_mprintf("action: 0x%x\n",naptFilterAndQos->action_fields);
	diag_util_mprintf("assign_priority: %d \n",naptFilterAndQos->assign_priority);	
	diag_util_mprintf("drop: %s \n",(naptFilterAndQos->action_fields&NAPT_DROP_BIT)?"Enabled":"Disabled");	

	diag_util_mprintf("ruleType: %s \n",naptFilterAndQos->ruleType?"One Shot":"Persist");	
}


void _diag_rgGponDsBcFilterAndRemarkingShow(rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t *filterRule)
{
	diag_util_mprintf("filter_fields = 0x%x\n",filterRule->filter_fields);
	if(filterRule->filter_fields & GPON_DS_BC_FILTER_INGRESS_STREAMID_BIT) diag_util_mprintf("ingress_stream_id= %d\n",filterRule->ingress_stream_id);
	if(filterRule->filter_fields & GPON_DS_BC_FILTER_INGRESS_STAGIf_BIT) diag_util_mprintf("%s ",filterRule->ingress_stagIf?"WITH_STAG":"WITHOUT_STAG");
	if(filterRule->filter_fields & GPON_DS_BC_FILTER_INGRESS_SVID_BIT) diag_util_mprintf("svid=%d\n",filterRule->ingress_stag_svid);
	if(filterRule->filter_fields & GPON_DS_BC_FILTER_INGRESS_CTAGIf_BIT) diag_util_mprintf("%s ",filterRule->ingress_ctagIf?"WITH_CTAG":"WITHOUT_CTAG");
	if(filterRule->filter_fields & GPON_DS_BC_FILTER_INGRESS_CVID_BIT) diag_util_mprintf("cvid=%d\n",filterRule->ingress_ctag_cvid);
	if(filterRule->filter_fields & GPON_DS_BC_FILTER_EGRESS_PORT_BIT) diag_util_mprintf("egres_port_mask=0x%x\n",filterRule->egress_portmask.portmask);
	diag_util_mprintf("action: %d cvid=%d cpri=%d\n",filterRule->ctag_action.ctag_decision,filterRule->ctag_action.assigned_ctag_cvid,filterRule->ctag_action.assigned_ctag_cpri);
}

/*
 * rg get version
 */
cparser_result_t
cparser_cmd_rg_get_version(
    cparser_context_t *context)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	ret = rtk_rg_driverVersion_get(&version_string);
	if(ret==RT_ERR_RG_OK){
		diag_util_mprintf("%s\n",version_string.version_string);	
	}else{
		return CPARSER_NOT_OK;
	}
    
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_version */

/*
 * rg init
 */
cparser_result_t
cparser_cmd_rg_init(
    cparser_context_t *context)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_initParam_set(NULL);
	if(ret!=RT_ERR_RG_OK)
		return CPARSER_NOT_OK;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_init */


/*
 * rg get init
 */
cparser_result_t
cparser_cmd_rg_get_init(
    cparser_context_t *context)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&init_param, 0x0, sizeof(rtk_rg_initParams_t));	
	ret = rtk_rg_initParam_get(&init_param);
	if(ret!=RT_ERR_RG_OK)
		return CPARSER_NOT_OK;

	
    _diag_rgInitShow(&init_param);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_init */



/*
 * rg show lan-intf
 */
cparser_result_t
cparser_cmd_rg_show_lan_intf(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	_diag_lanIntfShow(&lan_intf);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_lan_intf */

/*
 * rg clear lan-intf
 */
cparser_result_t
cparser_cmd_rg_clear_lan_intf(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&lan_intf, 0x0, sizeof(rtk_rg_lanIntfConf_t));	
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_lan_intf */

/*
 * rg add lan-intf entry
 */
cparser_result_t
cparser_cmd_rg_add_lan_intf_entry(
    cparser_context_t *context)
{
	int ret;
	int intf_idx;
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rtk_rg_lanInterface_add(&lan_intf,&intf_idx);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add lan-intf failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add lan-intf[%d] success.\n",intf_idx);
	}
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_lan_intf_entry */


/*
 * rg set lan-intf ip-version <UINT:ip_version> gateway-mac <MACADDR:gmac> ip-addr <IPV4ADDR:ip> ip-mask <IPV4ADDR:mask> ipv6-addr <IPV6ADDR:ipv6_addr> ipv6_network_mask_length <UINT:ipv6_network_mask_length> port-mask <HEX:port_mask> untag-mask <HEX:untag_mask> intf-vlan_id <UINT:intf_vid> vlan-based-pri-enable enable vlan-based-pri <UINT:vlan_based_pri> mtu <UINT:mtu> isIVL <UINT:isIVL>
 */
cparser_result_t
cparser_cmd_rg_set_lan_intf_ip_version_ip_version_gateway_mac_gmac_ip_addr_ip_ip_mask_mask_ipv6_addr_ipv6_addr_ipv6_network_mask_length_ipv6_network_mask_length_port_mask_port_mask_untag_mask_untag_mask_intf_vlan_id_intf_vid_vlan_based_pri_enable_enable_vlan_based_pri_vlan_based_pri_mtu_mtu_isIVL_isIVL(
    cparser_context_t *context,
    uint32_t  *ip_version_ptr,
    cparser_macaddr_t  *gmac_ptr,
    uint32_t  *ip_ptr,
    uint32_t  *mask_ptr,
    char * *ipv6_addr_ptr,
    uint32_t  *ipv6_network_mask_length_ptr,
    uint32_t  *port_mask_ptr,
    uint32_t  *untag_mask_ptr,
    uint32_t  *intf_vid_ptr,
    uint32_t  *vlan_based_pri_ptr,
    uint32_t  *mtu_ptr,
    uint32_t  *isIVL_ptr)
{
    int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

	lan_intf.ip_version = *ip_version_ptr;
	osal_memcpy(lan_intf.gmac.octet, gmac_ptr->octet, ETHER_ADDR_LEN);
	lan_intf.ip_addr=*ip_ptr;
	lan_intf.ip_network_mask=*mask_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&lan_intf.ipv6_addr.ipv6_addr[0], TOKEN_STR(12)), ret);
	lan_intf.ipv6_network_mask_length = *ipv6_network_mask_length_ptr;
	lan_intf.port_mask.portmask=*port_mask_ptr;
	lan_intf.untag_mask.portmask=*untag_mask_ptr;
	lan_intf.intf_vlan_id=*intf_vid_ptr;
	lan_intf.vlan_based_pri_enable=RTK_RG_ENABLED;
	lan_intf.vlan_based_pri=*vlan_based_pri_ptr;
	lan_intf.mtu=*mtu_ptr;
	lan_intf.isIVL=*isIVL_ptr;
	lan_intf.replace_subnet=0;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_lan_intf_ip_version_ip_version_gateway_mac_gmac_ip_addr_ip_ip_mask_mask_ipv6_addr_ipv6_addr_ipv6_network_mask_length_ipv6_network_mask_length_port_mask_port_mask_untag_mask_untag_mask_intf_vlan_id_intf_vid_vlan_based_pri_enable_enable_vlan_based_pri_vlan_based_pri_mtu_mtu_isivl_isivl */

/*
 * rg set lan-intf ip-version <UINT:ip_version> gateway-mac <MACADDR:gmac> ip-addr <IPV4ADDR:ip> ip-mask <IPV4ADDR:mask> ipv6-addr <IPV6ADDR:ipv6_addr> ipv6_network_mask_length <UINT:ipv6_network_mask_length> port-mask <HEX:port_mask> untag-mask <HEX:untag_mask> intf-vlan_id <UINT:intf_vid> vlan-based-pri-enable disable mtu <UINT:mtu> isIVL <UINT:isIVL>
 */
cparser_result_t
cparser_cmd_rg_set_lan_intf_ip_version_ip_version_gateway_mac_gmac_ip_addr_ip_ip_mask_mask_ipv6_addr_ipv6_addr_ipv6_network_mask_length_ipv6_network_mask_length_port_mask_port_mask_untag_mask_untag_mask_intf_vlan_id_intf_vid_vlan_based_pri_enable_disable_mtu_mtu_isIVL_isIVL(
    cparser_context_t *context,
    uint32_t  *ip_version_ptr,
    cparser_macaddr_t  *gmac_ptr,
    uint32_t  *ip_ptr,
    uint32_t  *mask_ptr,
    char * *ipv6_addr_ptr,
    uint32_t  *ipv6_network_mask_length_ptr,
    uint32_t  *port_mask_ptr,
    uint32_t  *untag_mask_ptr,
    uint32_t  *intf_vid_ptr,
    uint32_t  *mtu_ptr,
    uint32_t  *isIVL_ptr)
{
    int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

	lan_intf.ip_version = *ip_version_ptr;
	osal_memcpy(lan_intf.gmac.octet, gmac_ptr->octet, ETHER_ADDR_LEN);
	lan_intf.ip_addr=*ip_ptr;
	lan_intf.ip_network_mask=*mask_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&lan_intf.ipv6_addr.ipv6_addr[0], TOKEN_STR(12)), ret);
	lan_intf.ipv6_network_mask_length = *ipv6_network_mask_length_ptr;
	lan_intf.port_mask.portmask=*port_mask_ptr;
	lan_intf.untag_mask.portmask=*untag_mask_ptr;
	lan_intf.intf_vlan_id=*intf_vid_ptr;
	lan_intf.vlan_based_pri_enable=RTK_RG_DISABLED;
	lan_intf.vlan_based_pri=-1;
	lan_intf.mtu=*mtu_ptr;
	lan_intf.isIVL=*isIVL_ptr;
	lan_intf.replace_subnet=0;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_lan_intf_ip_version_ip_version_gateway_mac_gmac_ip_addr_ip_ip_mask_mask_ipv6_addr_ipv6_addr_ipv6_network_mask_length_ipv6_network_mask_length_port_mask_port_mask_untag_mask_untag_mask_intf_vlan_id_intf_vid_vlan_based_pri_enable_disable_mtu_mtu_isivl_isivl */

/*
 * rg set lan-intf ip-version <UINT:ip_version> gateway-mac <MACADDR:gmac> ip-addr <IPV4ADDR:ip> ip-mask <IPV4ADDR:mask> ipv6-addr <IPV6ADDR:ipv6_addr> ipv6_network_mask_length <UINT:ipv6_network_mask_length> port-mask <HEX:port_mask> untag-mask <HEX:untag_mask> intf-vlan_id <UINT:intf_vid> vlan-based-pri-enable enable vlan-based-pri <UINT:vlan_based_pri> mtu <UINT:mtu> isIVL <UINT:isIVL> replace-subnet <UINT:replace_subnet>
 */
cparser_result_t
cparser_cmd_rg_set_lan_intf_ip_version_ip_version_gateway_mac_gmac_ip_addr_ip_ip_mask_mask_ipv6_addr_ipv6_addr_ipv6_network_mask_length_ipv6_network_mask_length_port_mask_port_mask_untag_mask_untag_mask_intf_vlan_id_intf_vid_vlan_based_pri_enable_enable_vlan_based_pri_vlan_based_pri_mtu_mtu_isIVL_isIVL_replace_subnet_replace_subnet(
    cparser_context_t *context,
    uint32_t  *ip_version_ptr,
    cparser_macaddr_t  *gmac_ptr,
    uint32_t  *ip_ptr,
    uint32_t  *mask_ptr,
    char * *ipv6_addr_ptr,
    uint32_t  *ipv6_network_mask_length_ptr,
    uint32_t  *port_mask_ptr,
    uint32_t  *untag_mask_ptr,
    uint32_t  *intf_vid_ptr,
    uint32_t  *vlan_based_pri_ptr,
    uint32_t  *mtu_ptr,
    uint32_t  *isIVL_ptr,
    uint32_t  *replace_subnet_ptr)
{
    int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

	lan_intf.ip_version = *ip_version_ptr;
	osal_memcpy(lan_intf.gmac.octet, gmac_ptr->octet, ETHER_ADDR_LEN);
	lan_intf.ip_addr=*ip_ptr;
	lan_intf.ip_network_mask=*mask_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&lan_intf.ipv6_addr.ipv6_addr[0], TOKEN_STR(12)), ret);
	lan_intf.ipv6_network_mask_length = *ipv6_network_mask_length_ptr;
	lan_intf.port_mask.portmask=*port_mask_ptr;
	lan_intf.untag_mask.portmask=*untag_mask_ptr;
	lan_intf.intf_vlan_id=*intf_vid_ptr;
	lan_intf.vlan_based_pri_enable=RTK_RG_ENABLED;
	lan_intf.vlan_based_pri=*vlan_based_pri_ptr;
	lan_intf.mtu=*mtu_ptr;
	lan_intf.isIVL=*isIVL_ptr;
	lan_intf.replace_subnet=*replace_subnet_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_lan_intf_ip_version_ip_version_gateway_mac_gmac_ip_addr_ip_ip_mask_mask_ipv6_addr_ipv6_addr_ipv6_network_mask_length_ipv6_network_mask_length_port_mask_port_mask_untag_mask_untag_mask_intf_vlan_id_intf_vid_vlan_based_pri_enable_enable_vlan_based_pri_vlan_based_pri_mtu_mtu_isivl_isivl_replace_subnet_replace_subnet */

/*
 * rg set lan-intf ip-version <UINT:ip_version> gateway-mac <MACADDR:gmac> ip-addr <IPV4ADDR:ip> ip-mask <IPV4ADDR:mask> ipv6-addr <IPV6ADDR:ipv6_addr> ipv6_network_mask_length <UINT:ipv6_network_mask_length> port-mask <HEX:port_mask> untag-mask <HEX:untag_mask> intf-vlan_id <UINT:intf_vid> vlan-based-pri-enable disable mtu <UINT:mtu> isIVL <UINT:isIVL> replace-subnet <UINT:replace_subnet>
 */
cparser_result_t
cparser_cmd_rg_set_lan_intf_ip_version_ip_version_gateway_mac_gmac_ip_addr_ip_ip_mask_mask_ipv6_addr_ipv6_addr_ipv6_network_mask_length_ipv6_network_mask_length_port_mask_port_mask_untag_mask_untag_mask_intf_vlan_id_intf_vid_vlan_based_pri_enable_disable_mtu_mtu_isIVL_isIVL_replace_subnet_replace_subnet(
    cparser_context_t *context,
    uint32_t  *ip_version_ptr,
    cparser_macaddr_t  *gmac_ptr,
    uint32_t  *ip_ptr,
    uint32_t  *mask_ptr,
    char * *ipv6_addr_ptr,
    uint32_t  *ipv6_network_mask_length_ptr,
    uint32_t  *port_mask_ptr,
    uint32_t  *untag_mask_ptr,
    uint32_t  *intf_vid_ptr,
    uint32_t  *mtu_ptr,
    uint32_t  *isIVL_ptr,
    uint32_t  *replace_subnet_ptr)
{
    int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

	lan_intf.ip_version = *ip_version_ptr;
	osal_memcpy(lan_intf.gmac.octet, gmac_ptr->octet, ETHER_ADDR_LEN);
	lan_intf.ip_addr=*ip_ptr;
	lan_intf.ip_network_mask=*mask_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&lan_intf.ipv6_addr.ipv6_addr[0], TOKEN_STR(12)), ret);
	lan_intf.ipv6_network_mask_length = *ipv6_network_mask_length_ptr;
	lan_intf.port_mask.portmask=*port_mask_ptr;
	lan_intf.untag_mask.portmask=*untag_mask_ptr;
	lan_intf.intf_vlan_id=*intf_vid_ptr;
	lan_intf.vlan_based_pri_enable=RTK_RG_DISABLED;
	lan_intf.vlan_based_pri=-1;
	lan_intf.mtu=*mtu_ptr;
	lan_intf.isIVL=*isIVL_ptr;
	lan_intf.replace_subnet=*replace_subnet_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_lan_intf_ip_version_ip_version_gateway_mac_gmac_ip_addr_ip_ip_mask_mask_ipv6_addr_ipv6_addr_ipv6_network_mask_length_ipv6_network_mask_length_port_mask_port_mask_untag_mask_untag_mask_intf_vlan_id_intf_vid_vlan_based_pri_enable_disable_mtu_mtu_isivl_isivl_replace_subnet_replace_subnet */

/*
 * rg show wan-intf
 */
cparser_result_t
cparser_cmd_rg_show_wan_intf(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	_diag_wanIntfShow(&wan_intf);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_wan_intf */

/*
 * rg clear wan-intf
 */
cparser_result_t
cparser_cmd_rg_clear_wan_intf(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&wan_intf, 0x0, sizeof(rtk_rg_wanIntfConf_t));	

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_wan_intf */

/*
 * rg add wan-intf entry
 */
cparser_result_t
cparser_cmd_rg_add_wan_intf_entry(
    cparser_context_t *context)
{
	int ret;
	int intf_idx;
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rtk_rg_wanInterface_add(&wan_intf,&intf_idx);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add wan-intf failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add wan-intf[%d] success.\n",intf_idx);
	}
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_wan_intf_entry */

/*
 * rg set wan-intf wan-type <UINT:wan_type> gateway-mac <MACADDR:gmac> wan-port <UINT:wan_port_idx> port-binding-mask <HEX:port_binding_mask> egress-vlan-tag-on <UINT:tag_on> egress-vlan-id <UINT:vlan_id> vlan-based-pri-enable enable vlan-based-pri <UINT:vlan_based_pri> isIVL <UINT:isIVL>
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_enable_vlan_based_pri_vlan_based_pri_isIVL_isIVL(
    cparser_context_t *context,
    uint32_t  *wan_type_ptr,
    cparser_macaddr_t  *gmac_ptr,
    uint32_t  *wan_port_idx_ptr,
    uint32_t  *port_binding_mask_ptr,
    uint32_t  *tag_on_ptr,
    uint32_t  *vlan_id_ptr,
    uint32_t  *vlan_based_pri_ptr,
    uint32_t  *isIVL_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	wan_intf.wan_type = *wan_type_ptr;
	osal_memcpy(wan_intf.gmac.octet, gmac_ptr->octet, ETHER_ADDR_LEN);
	wan_intf.wan_port_idx = *wan_port_idx_ptr;
	wan_intf.port_binding_mask.portmask= *port_binding_mask_ptr;
	wan_intf.egress_vlan_tag_on = *tag_on_ptr;
	wan_intf.egress_vlan_id = *vlan_id_ptr;
	wan_intf.vlan_based_pri_enable = RTK_RG_ENABLED;
	wan_intf.vlan_based_pri = *vlan_based_pri_ptr;
	wan_intf.isIVL=*isIVL_ptr;
	wan_intf.none_internet=0;
	wan_intf.wlan0_dev_binding_mask=0;
	wan_intf.forcedAddNewIntf=1;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_enable_vlan_based_pri_vlan_based_pri_isivl_isivl */

/*
 * rg set wan-intf wan-type <UINT:wan_type> gateway-mac <MACADDR:gmac> wan-port <UINT:wan_port_idx> port-binding-mask <HEX:port_binding_mask> egress-vlan-tag-on <UINT:tag_on> egress-vlan-id <UINT:vlan_id> vlan-based-pri-enable enable vlan-based-pri <UINT:vlan_based_pri> isIVL <UINT:isIVL> none_internet
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_enable_vlan_based_pri_vlan_based_pri_isIVL_isIVL_none_internet(
    cparser_context_t *context,
    uint32_t  *wan_type_ptr,
    cparser_macaddr_t  *gmac_ptr,
    uint32_t  *wan_port_idx_ptr,
    uint32_t  *port_binding_mask_ptr,
    uint32_t  *tag_on_ptr,
    uint32_t  *vlan_id_ptr,
    uint32_t  *vlan_based_pri_ptr,
    uint32_t  *isIVL_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	wan_intf.wan_type = *wan_type_ptr;
	osal_memcpy(wan_intf.gmac.octet, gmac_ptr->octet, ETHER_ADDR_LEN);
	wan_intf.wan_port_idx = *wan_port_idx_ptr;
	wan_intf.port_binding_mask.portmask= *port_binding_mask_ptr;
	wan_intf.egress_vlan_tag_on = *tag_on_ptr;
	wan_intf.egress_vlan_id = *vlan_id_ptr;
	wan_intf.vlan_based_pri_enable = RTK_RG_ENABLED;
	wan_intf.vlan_based_pri = *vlan_based_pri_ptr;
	wan_intf.isIVL=*isIVL_ptr;
	wan_intf.none_internet=1;
	wan_intf.wlan0_dev_binding_mask=0;
	wan_intf.forcedAddNewIntf=1;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_enable_vlan_based_pri_vlan_based_pri_isivl_isivl_none_internet */

/*
 * rg set wan-intf wan-type <UINT:wan_type> gateway-mac <MACADDR:gmac> wan-port <UINT:wan_port_idx> port-binding-mask <HEX:port_binding_mask> egress-vlan-tag-on <UINT:tag_on> egress-vlan-id <UINT:vlan_id> vlan-based-pri-enable disable isIVL <UINT:isIVL>
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_disable_isIVL_isIVL(
    cparser_context_t *context,
    uint32_t  *wan_type_ptr,
    cparser_macaddr_t  *gmac_ptr,
    uint32_t  *wan_port_idx_ptr,
    uint32_t  *port_binding_mask_ptr,
    uint32_t  *tag_on_ptr,
    uint32_t  *vlan_id_ptr,
    uint32_t  *isIVL_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	wan_intf.wan_type = *wan_type_ptr;
	osal_memcpy(wan_intf.gmac.octet, gmac_ptr->octet, ETHER_ADDR_LEN);
	wan_intf.wan_port_idx = *wan_port_idx_ptr;
	wan_intf.port_binding_mask.portmask= *port_binding_mask_ptr;
	wan_intf.egress_vlan_tag_on = *tag_on_ptr;
	wan_intf.egress_vlan_id = *vlan_id_ptr;
	wan_intf.vlan_based_pri_enable = RTK_RG_DISABLED;
	wan_intf.vlan_based_pri = -1;
	wan_intf.isIVL=*isIVL_ptr;
	wan_intf.none_internet=0;
	wan_intf.wlan0_dev_binding_mask=0;
	wan_intf.forcedAddNewIntf=1;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_disable_isivl_isivl */

/*
 * rg set wan-intf wan-type <UINT:wan_type> gateway-mac <MACADDR:gmac> wan-port <UINT:wan_port_idx> port-binding-mask <HEX:port_binding_mask> egress-vlan-tag-on <UINT:tag_on> egress-vlan-id <UINT:vlan_id> vlan-based-pri-enable disable isIVL <UINT:isIVL> none_internet
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_disable_isIVL_isIVL_none_internet(
    cparser_context_t *context,
    uint32_t  *wan_type_ptr,
    cparser_macaddr_t  *gmac_ptr,
    uint32_t  *wan_port_idx_ptr,
    uint32_t  *port_binding_mask_ptr,
    uint32_t  *tag_on_ptr,
    uint32_t  *vlan_id_ptr,
    uint32_t  *isIVL_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	wan_intf.wan_type = *wan_type_ptr;
	osal_memcpy(wan_intf.gmac.octet, gmac_ptr->octet, ETHER_ADDR_LEN);
	wan_intf.wan_port_idx = *wan_port_idx_ptr;
	wan_intf.port_binding_mask.portmask= *port_binding_mask_ptr;
	wan_intf.egress_vlan_tag_on = *tag_on_ptr;
	wan_intf.egress_vlan_id = *vlan_id_ptr;
	wan_intf.vlan_based_pri_enable = RTK_RG_DISABLED;
	wan_intf.vlan_based_pri = -1;
	wan_intf.isIVL=*isIVL_ptr;
	wan_intf.none_internet=1;
	wan_intf.wlan0_dev_binding_mask=0;
	wan_intf.forcedAddNewIntf=1;
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_disable_isivl_isivl_none_internet */

/*
 * rg set wan-intf wan-type <UINT:wan_type> gateway-mac <MACADDR:gmac> wan-port <UINT:wan_port_idx> port-binding-mask <HEX:port_binding_mask> egress-vlan-tag-on <UINT:tag_on> egress-vlan-id <UINT:vlan_id> vlan-based-pri-enable enable vlan-based-pri <UINT:vlan_based_pri> isIVL <UINT:isIVL> wlan0-binding-mask <HEX:wlan_binding_mask>
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_enable_vlan_based_pri_vlan_based_pri_isIVL_isIVL_wlan0_binding_mask_wlan_binding_mask(
    cparser_context_t *context,
    uint32_t  *wan_type_ptr,
    cparser_macaddr_t  *gmac_ptr,
    uint32_t  *wan_port_idx_ptr,
    uint32_t  *port_binding_mask_ptr,
    uint32_t  *tag_on_ptr,
    uint32_t  *vlan_id_ptr,
    uint32_t  *vlan_based_pri_ptr,
    uint32_t  *isIVL_ptr,
    uint32_t  *wlan_binding_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	wan_intf.wan_type = *wan_type_ptr;
	osal_memcpy(wan_intf.gmac.octet, gmac_ptr->octet, ETHER_ADDR_LEN);
	wan_intf.wan_port_idx = *wan_port_idx_ptr;
	wan_intf.port_binding_mask.portmask= *port_binding_mask_ptr;
	wan_intf.egress_vlan_tag_on = *tag_on_ptr;
	wan_intf.egress_vlan_id = *vlan_id_ptr;
	wan_intf.vlan_based_pri_enable = RTK_RG_ENABLED;
	wan_intf.vlan_based_pri = *vlan_based_pri_ptr;
	wan_intf.isIVL=*isIVL_ptr;
	wan_intf.none_internet=0;
	wan_intf.wlan0_dev_binding_mask=*wlan_binding_mask_ptr;
	wan_intf.forcedAddNewIntf=1;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_enable_vlan_based_pri_vlan_based_pri_isivl_isivl_wlan0_binding_mask_wlan_binding_mask */

/*
 * rg set wan-intf wan-type <UINT:wan_type> gateway-mac <MACADDR:gmac> wan-port <UINT:wan_port_idx> port-binding-mask <HEX:port_binding_mask> egress-vlan-tag-on <UINT:tag_on> egress-vlan-id <UINT:vlan_id> vlan-based-pri-enable enable vlan-based-pri <UINT:vlan_based_pri> isIVL <UINT:isIVL> none_internet wlan0-binding-mask <HEX:wlan_binding_mask>
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_enable_vlan_based_pri_vlan_based_pri_isIVL_isIVL_none_internet_wlan0_binding_mask_wlan_binding_mask(
    cparser_context_t *context,
    uint32_t  *wan_type_ptr,
    cparser_macaddr_t  *gmac_ptr,
    uint32_t  *wan_port_idx_ptr,
    uint32_t  *port_binding_mask_ptr,
    uint32_t  *tag_on_ptr,
    uint32_t  *vlan_id_ptr,
    uint32_t  *vlan_based_pri_ptr,
    uint32_t  *isIVL_ptr,
    uint32_t  *wlan_binding_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	wan_intf.wan_type = *wan_type_ptr;
	osal_memcpy(wan_intf.gmac.octet, gmac_ptr->octet, ETHER_ADDR_LEN);
	wan_intf.wan_port_idx = *wan_port_idx_ptr;
	wan_intf.port_binding_mask.portmask= *port_binding_mask_ptr;
	wan_intf.egress_vlan_tag_on = *tag_on_ptr;
	wan_intf.egress_vlan_id = *vlan_id_ptr;
	wan_intf.vlan_based_pri_enable = RTK_RG_ENABLED;
	wan_intf.vlan_based_pri = *vlan_based_pri_ptr;
	wan_intf.isIVL=*isIVL_ptr;
	wan_intf.none_internet=1;
	wan_intf.wlan0_dev_binding_mask=*wlan_binding_mask_ptr;
	wan_intf.forcedAddNewIntf=1;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_enable_vlan_based_pri_vlan_based_pri_isivl_isivl_none_internet_wlan0_binding_mask_wlan_binding_mask */

/*
 * rg set wan-intf wan-type <UINT:wan_type> gateway-mac <MACADDR:gmac> wan-port <UINT:wan_port_idx> port-binding-mask <HEX:port_binding_mask> egress-vlan-tag-on <UINT:tag_on> egress-vlan-id <UINT:vlan_id> vlan-based-pri-enable disable isIVL <UINT:isIVL> wlan0-binding-mask <HEX:wlan_binding_mask>
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_disable_isIVL_isIVL_wlan0_binding_mask_wlan_binding_mask(
    cparser_context_t *context,
    uint32_t  *wan_type_ptr,
    cparser_macaddr_t  *gmac_ptr,
    uint32_t  *wan_port_idx_ptr,
    uint32_t  *port_binding_mask_ptr,
    uint32_t  *tag_on_ptr,
    uint32_t  *vlan_id_ptr,
    uint32_t  *isIVL_ptr,
    uint32_t  *wlan_binding_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	wan_intf.wan_type = *wan_type_ptr;
	osal_memcpy(wan_intf.gmac.octet, gmac_ptr->octet, ETHER_ADDR_LEN);
	wan_intf.wan_port_idx = *wan_port_idx_ptr;
	wan_intf.port_binding_mask.portmask= *port_binding_mask_ptr;
	wan_intf.egress_vlan_tag_on = *tag_on_ptr;
	wan_intf.egress_vlan_id = *vlan_id_ptr;
	wan_intf.vlan_based_pri_enable = RTK_RG_DISABLED;
	wan_intf.vlan_based_pri = -1;
	wan_intf.isIVL=*isIVL_ptr;
	wan_intf.none_internet=0;
	wan_intf.wlan0_dev_binding_mask=*wlan_binding_mask_ptr;
	wan_intf.forcedAddNewIntf=1;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_disable_isivl_isivl_wlan0_binding_mask_wlan_binding_mask */

/*
 * rg set wan-intf wan-type <UINT:wan_type> gateway-mac <MACADDR:gmac> wan-port <UINT:wan_port_idx> port-binding-mask <HEX:port_binding_mask> egress-vlan-tag-on <UINT:tag_on> egress-vlan-id <UINT:vlan_id> vlan-based-pri-enable disable isIVL <UINT:isIVL> none_internet wlan0-binding-mask <HEX:wlan_binding_mask>
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_disable_isIVL_isIVL_none_internet_wlan0_binding_mask_wlan_binding_mask(
    cparser_context_t *context,
    uint32_t  *wan_type_ptr,
    cparser_macaddr_t  *gmac_ptr,
    uint32_t  *wan_port_idx_ptr,
    uint32_t  *port_binding_mask_ptr,
    uint32_t  *tag_on_ptr,
    uint32_t  *vlan_id_ptr,
    uint32_t  *isIVL_ptr,
    uint32_t  *wlan_binding_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	wan_intf.wan_type = *wan_type_ptr;
	osal_memcpy(wan_intf.gmac.octet, gmac_ptr->octet, ETHER_ADDR_LEN);
	wan_intf.wan_port_idx = *wan_port_idx_ptr;
	wan_intf.port_binding_mask.portmask= *port_binding_mask_ptr;
	wan_intf.egress_vlan_tag_on = *tag_on_ptr;
	wan_intf.egress_vlan_id = *vlan_id_ptr;
	wan_intf.vlan_based_pri_enable = RTK_RG_DISABLED;
	wan_intf.vlan_based_pri = -1;
	wan_intf.isIVL=*isIVL_ptr;
	wan_intf.none_internet=1;
	wan_intf.wlan0_dev_binding_mask=*wlan_binding_mask_ptr;
	wan_intf.forcedAddNewIntf=1;
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_disable_isivl_isivl_none_internet_wlan0_binding_mask_wlan_binding_mask */

/*
 * rg set wan-intf wan-type <UINT:wan_type> gateway-mac <MACADDR:gmac> wan-port <UINT:wan_port_idx> port-binding-mask <HEX:port_binding_mask> egress-vlan-tag-on <UINT:tag_on> egress-vlan-id <UINT:vlan_id> vlan-based-pri-enable enable vlan-based-pri <UINT:vlan_based_pri> isIVL <UINT:isIVL> replaceSame
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_enable_vlan_based_pri_vlan_based_pri_isIVL_isIVL_replaceSame(
    cparser_context_t *context,
    uint32_t  *wan_type_ptr,
    cparser_macaddr_t  *gmac_ptr,
    uint32_t  *wan_port_idx_ptr,
    uint32_t  *port_binding_mask_ptr,
    uint32_t  *tag_on_ptr,
    uint32_t  *vlan_id_ptr,
    uint32_t  *vlan_based_pri_ptr,
    uint32_t  *isIVL_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	wan_intf.wan_type = *wan_type_ptr;
	osal_memcpy(wan_intf.gmac.octet, gmac_ptr->octet, ETHER_ADDR_LEN);
	wan_intf.wan_port_idx = *wan_port_idx_ptr;
	wan_intf.port_binding_mask.portmask= *port_binding_mask_ptr;
	wan_intf.egress_vlan_tag_on = *tag_on_ptr;
	wan_intf.egress_vlan_id = *vlan_id_ptr;
	wan_intf.vlan_based_pri_enable = RTK_RG_ENABLED;
	wan_intf.vlan_based_pri = *vlan_based_pri_ptr;
	wan_intf.isIVL=*isIVL_ptr;
	wan_intf.none_internet=0;
	wan_intf.wlan0_dev_binding_mask=0;
	wan_intf.forcedAddNewIntf=0;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_enable_vlan_based_pri_vlan_based_pri_isivl_isivl_replacesame */

/*
 * rg set wan-intf wan-type <UINT:wan_type> gateway-mac <MACADDR:gmac> wan-port <UINT:wan_port_idx> port-binding-mask <HEX:port_binding_mask> egress-vlan-tag-on <UINT:tag_on> egress-vlan-id <UINT:vlan_id> vlan-based-pri-enable enable vlan-based-pri <UINT:vlan_based_pri> isIVL <UINT:isIVL> none_internet replaceSame
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_enable_vlan_based_pri_vlan_based_pri_isIVL_isIVL_none_internet_replaceSame(
    cparser_context_t *context,
    uint32_t  *wan_type_ptr,
    cparser_macaddr_t  *gmac_ptr,
    uint32_t  *wan_port_idx_ptr,
    uint32_t  *port_binding_mask_ptr,
    uint32_t  *tag_on_ptr,
    uint32_t  *vlan_id_ptr,
    uint32_t  *vlan_based_pri_ptr,
    uint32_t  *isIVL_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	wan_intf.wan_type = *wan_type_ptr;
	osal_memcpy(wan_intf.gmac.octet, gmac_ptr->octet, ETHER_ADDR_LEN);
	wan_intf.wan_port_idx = *wan_port_idx_ptr;
	wan_intf.port_binding_mask.portmask= *port_binding_mask_ptr;
	wan_intf.egress_vlan_tag_on = *tag_on_ptr;
	wan_intf.egress_vlan_id = *vlan_id_ptr;
	wan_intf.vlan_based_pri_enable = RTK_RG_ENABLED;
	wan_intf.vlan_based_pri = *vlan_based_pri_ptr;
	wan_intf.isIVL=*isIVL_ptr;
	wan_intf.none_internet=1;
	wan_intf.wlan0_dev_binding_mask=0;
	wan_intf.forcedAddNewIntf=0;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_enable_vlan_based_pri_vlan_based_pri_isivl_isivl_none_internet_replacesame */

/*
 * rg set wan-intf wan-type <UINT:wan_type> gateway-mac <MACADDR:gmac> wan-port <UINT:wan_port_idx> port-binding-mask <HEX:port_binding_mask> egress-vlan-tag-on <UINT:tag_on> egress-vlan-id <UINT:vlan_id> vlan-based-pri-enable disable isIVL <UINT:isIVL> replaceSame
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_disable_isIVL_isIVL_replaceSame(
    cparser_context_t *context,
    uint32_t  *wan_type_ptr,
    cparser_macaddr_t  *gmac_ptr,
    uint32_t  *wan_port_idx_ptr,
    uint32_t  *port_binding_mask_ptr,
    uint32_t  *tag_on_ptr,
    uint32_t  *vlan_id_ptr,
    uint32_t  *isIVL_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	wan_intf.wan_type = *wan_type_ptr;
	osal_memcpy(wan_intf.gmac.octet, gmac_ptr->octet, ETHER_ADDR_LEN);
	wan_intf.wan_port_idx = *wan_port_idx_ptr;
	wan_intf.port_binding_mask.portmask= *port_binding_mask_ptr;
	wan_intf.egress_vlan_tag_on = *tag_on_ptr;
	wan_intf.egress_vlan_id = *vlan_id_ptr;
	wan_intf.vlan_based_pri_enable = RTK_RG_DISABLED;
	wan_intf.vlan_based_pri = -1;
	wan_intf.isIVL=*isIVL_ptr;
	wan_intf.none_internet=0;
	wan_intf.wlan0_dev_binding_mask=0;
	wan_intf.forcedAddNewIntf=0;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_disable_isivl_isivl_replacesame */

/*
 * rg set wan-intf wan-type <UINT:wan_type> gateway-mac <MACADDR:gmac> wan-port <UINT:wan_port_idx> port-binding-mask <HEX:port_binding_mask> egress-vlan-tag-on <UINT:tag_on> egress-vlan-id <UINT:vlan_id> vlan-based-pri-enable disable isIVL <UINT:isIVL> none_internet replaceSame
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_disable_isIVL_isIVL_none_internet_replaceSame(
    cparser_context_t *context,
    uint32_t  *wan_type_ptr,
    cparser_macaddr_t  *gmac_ptr,
    uint32_t  *wan_port_idx_ptr,
    uint32_t  *port_binding_mask_ptr,
    uint32_t  *tag_on_ptr,
    uint32_t  *vlan_id_ptr,
    uint32_t  *isIVL_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	wan_intf.wan_type = *wan_type_ptr;
	osal_memcpy(wan_intf.gmac.octet, gmac_ptr->octet, ETHER_ADDR_LEN);
	wan_intf.wan_port_idx = *wan_port_idx_ptr;
	wan_intf.port_binding_mask.portmask= *port_binding_mask_ptr;
	wan_intf.egress_vlan_tag_on = *tag_on_ptr;
	wan_intf.egress_vlan_id = *vlan_id_ptr;
	wan_intf.vlan_based_pri_enable = RTK_RG_DISABLED;
	wan_intf.vlan_based_pri = -1;
	wan_intf.isIVL=*isIVL_ptr;
	wan_intf.none_internet=1;
	wan_intf.wlan0_dev_binding_mask=0;
	wan_intf.forcedAddNewIntf=0;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_disable_isivl_isivl_none_internet_replacesame */

/*
 * rg set wan-intf wan-type <UINT:wan_type> gateway-mac <MACADDR:gmac> wan-port <UINT:wan_port_idx> port-binding-mask <HEX:port_binding_mask> egress-vlan-tag-on <UINT:tag_on> egress-vlan-id <UINT:vlan_id> vlan-based-pri-enable enable vlan-based-pri <UINT:vlan_based_pri> isIVL <UINT:isIVL> wlan0-binding-mask <HEX:wlan_binding_mask> replaceSame
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_enable_vlan_based_pri_vlan_based_pri_isIVL_isIVL_wlan0_binding_mask_wlan_binding_mask_replaceSame(
    cparser_context_t *context,
    uint32_t  *wan_type_ptr,
    cparser_macaddr_t  *gmac_ptr,
    uint32_t  *wan_port_idx_ptr,
    uint32_t  *port_binding_mask_ptr,
    uint32_t  *tag_on_ptr,
    uint32_t  *vlan_id_ptr,
    uint32_t  *vlan_based_pri_ptr,
    uint32_t  *isIVL_ptr,
    uint32_t  *wlan_binding_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	wan_intf.wan_type = *wan_type_ptr;
	osal_memcpy(wan_intf.gmac.octet, gmac_ptr->octet, ETHER_ADDR_LEN);
	wan_intf.wan_port_idx = *wan_port_idx_ptr;
	wan_intf.port_binding_mask.portmask= *port_binding_mask_ptr;
	wan_intf.egress_vlan_tag_on = *tag_on_ptr;
	wan_intf.egress_vlan_id = *vlan_id_ptr;
	wan_intf.vlan_based_pri_enable = RTK_RG_ENABLED;
	wan_intf.vlan_based_pri = *vlan_based_pri_ptr;
	wan_intf.isIVL=*isIVL_ptr;
	wan_intf.none_internet=0;
	wan_intf.wlan0_dev_binding_mask=*wlan_binding_mask_ptr;
	wan_intf.forcedAddNewIntf=0;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_enable_vlan_based_pri_vlan_based_pri_isivl_isivl_wlan0_binding_mask_wlan_binding_mask_replacesame */

/*
 * rg set wan-intf wan-type <UINT:wan_type> gateway-mac <MACADDR:gmac> wan-port <UINT:wan_port_idx> port-binding-mask <HEX:port_binding_mask> egress-vlan-tag-on <UINT:tag_on> egress-vlan-id <UINT:vlan_id> vlan-based-pri-enable enable vlan-based-pri <UINT:vlan_based_pri> isIVL <UINT:isIVL> none_internet wlan0-binding-mask <HEX:wlan_binding_mask> replaceSame
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_enable_vlan_based_pri_vlan_based_pri_isIVL_isIVL_none_internet_wlan0_binding_mask_wlan_binding_mask_replaceSame(
    cparser_context_t *context,
    uint32_t  *wan_type_ptr,
    cparser_macaddr_t  *gmac_ptr,
    uint32_t  *wan_port_idx_ptr,
    uint32_t  *port_binding_mask_ptr,
    uint32_t  *tag_on_ptr,
    uint32_t  *vlan_id_ptr,
    uint32_t  *vlan_based_pri_ptr,
    uint32_t  *isIVL_ptr,
    uint32_t  *wlan_binding_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	wan_intf.wan_type = *wan_type_ptr;
	osal_memcpy(wan_intf.gmac.octet, gmac_ptr->octet, ETHER_ADDR_LEN);
	wan_intf.wan_port_idx = *wan_port_idx_ptr;
	wan_intf.port_binding_mask.portmask= *port_binding_mask_ptr;
	wan_intf.egress_vlan_tag_on = *tag_on_ptr;
	wan_intf.egress_vlan_id = *vlan_id_ptr;
	wan_intf.vlan_based_pri_enable = RTK_RG_ENABLED;
	wan_intf.vlan_based_pri = *vlan_based_pri_ptr;
	wan_intf.isIVL=*isIVL_ptr;
	wan_intf.none_internet=1;
	wan_intf.wlan0_dev_binding_mask=*wlan_binding_mask_ptr;
	wan_intf.forcedAddNewIntf=0;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_enable_vlan_based_pri_vlan_based_pri_isivl_isivl_none_internet_wlan0_binding_mask_wlan_binding_mask_replacesame */

/*
 * rg set wan-intf wan-type <UINT:wan_type> gateway-mac <MACADDR:gmac> wan-port <UINT:wan_port_idx> port-binding-mask <HEX:port_binding_mask> egress-vlan-tag-on <UINT:tag_on> egress-vlan-id <UINT:vlan_id> vlan-based-pri-enable disable isIVL <UINT:isIVL> wlan0-binding-mask <HEX:wlan_binding_mask> replaceSame
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_disable_isIVL_isIVL_wlan0_binding_mask_wlan_binding_mask_replaceSame(
    cparser_context_t *context,
    uint32_t  *wan_type_ptr,
    cparser_macaddr_t  *gmac_ptr,
    uint32_t  *wan_port_idx_ptr,
    uint32_t  *port_binding_mask_ptr,
    uint32_t  *tag_on_ptr,
    uint32_t  *vlan_id_ptr,
    uint32_t  *isIVL_ptr,
    uint32_t  *wlan_binding_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	wan_intf.wan_type = *wan_type_ptr;
	osal_memcpy(wan_intf.gmac.octet, gmac_ptr->octet, ETHER_ADDR_LEN);
	wan_intf.wan_port_idx = *wan_port_idx_ptr;
	wan_intf.port_binding_mask.portmask= *port_binding_mask_ptr;
	wan_intf.egress_vlan_tag_on = *tag_on_ptr;
	wan_intf.egress_vlan_id = *vlan_id_ptr;
	wan_intf.vlan_based_pri_enable = RTK_RG_DISABLED;
	wan_intf.vlan_based_pri = -1;
	wan_intf.isIVL=*isIVL_ptr;
	wan_intf.none_internet=0;
	wan_intf.wlan0_dev_binding_mask=*wlan_binding_mask_ptr;
	wan_intf.forcedAddNewIntf=0;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_disable_isivl_isivl_wlan0_binding_mask_wlan_binding_mask_replacesame */

/*
 * rg set wan-intf wan-type <UINT:wan_type> gateway-mac <MACADDR:gmac> wan-port <UINT:wan_port_idx> port-binding-mask <HEX:port_binding_mask> egress-vlan-tag-on <UINT:tag_on> egress-vlan-id <UINT:vlan_id> vlan-based-pri-enable disable isIVL <UINT:isIVL> none_internet wlan0-binding-mask <HEX:wlan_binding_mask> replaceSame
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_disable_isIVL_isIVL_none_internet_wlan0_binding_mask_wlan_binding_mask_replaceSame(
    cparser_context_t *context,
    uint32_t  *wan_type_ptr,
    cparser_macaddr_t  *gmac_ptr,
    uint32_t  *wan_port_idx_ptr,
    uint32_t  *port_binding_mask_ptr,
    uint32_t  *tag_on_ptr,
    uint32_t  *vlan_id_ptr,
    uint32_t  *isIVL_ptr,
    uint32_t  *wlan_binding_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	wan_intf.wan_type = *wan_type_ptr;
	osal_memcpy(wan_intf.gmac.octet, gmac_ptr->octet, ETHER_ADDR_LEN);
	wan_intf.wan_port_idx = *wan_port_idx_ptr;
	wan_intf.port_binding_mask.portmask= *port_binding_mask_ptr;
	wan_intf.egress_vlan_tag_on = *tag_on_ptr;
	wan_intf.egress_vlan_id = *vlan_id_ptr;
	wan_intf.vlan_based_pri_enable = RTK_RG_DISABLED;
	wan_intf.vlan_based_pri = -1;
	wan_intf.isIVL=*isIVL_ptr;
	wan_intf.none_internet=1;
	wan_intf.wlan0_dev_binding_mask=*wlan_binding_mask_ptr;
	wan_intf.forcedAddNewIntf=0;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_wan_intf_wan_type_wan_type_gateway_mac_gmac_wan_port_wan_port_idx_port_binding_mask_port_binding_mask_egress_vlan_tag_on_tag_on_egress_vlan_id_vlan_id_vlan_based_pri_enable_disable_isivl_isivl_none_internet_wlan0_binding_mask_wlan_binding_mask_replacesame */


/*
 * rg get intf entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_get_intf_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int ret;
	int intf_idx = *entry_index_ptr;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
	
	osal_memset(&intf_info, 0x0, sizeof(rtk_rg_intfInfo_t));	

	ret = rtk_rg_intfInfo_find(&intf_info, &intf_idx);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get intf failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		if(intf_idx==*entry_index_ptr){
			diag_util_mprintf("intf_name: %s\n",intf_info.intf_name);
			diag_util_mprintf("is_wan: %d\n",intf_info.is_wan);
			if(intf_info.is_wan){
				diag_util_mprintf("wan_intf:\n");
				diag_util_mprintf("==================================\n");
				_diag_wanIntfShow(&intf_info.wan_intf.wan_intf_conf);			
				//display wan's static info
				switch(intf_info.wan_intf.wan_intf_conf.wan_type)
				{
					case RTK_RG_STATIC:
						diag_util_mprintf("==================================\n");
						_diag_wanIntfStaticInfoShow(&intf_info.wan_intf.static_info);
						//osal_memcpy(&wan_intf_static_info, &intf_info.wan_intf.static_info, sizeof(rtk_rg_ipStaticInfo_t));
						diag_util_mprintf("==================================\n");
						break;
					case RTK_RG_DHCP:
						diag_util_mprintf("==================================\n");
						_diag_dhcpClientInfoShow(&intf_info.wan_intf.dhcp_client_info);
						//osal_memcpy(&dhcpClient_info, &intf_info.wan_intf.dhcp_client_info, sizeof(rtk_rg_ipDhcpClientInfo_t));
						diag_util_mprintf("==================================\n");
						break;
					case RTK_RG_PPPoE:
						diag_util_mprintf("==================================\n");
						_diag_pppoeClientInfoBeforeDiagShow(&intf_info.wan_intf.pppoe_info.before_dial);
						_diag_pppoeClientInfoShow(&intf_info.wan_intf.pppoe_info.after_dial);
						//osal_memcpy(&clientPppoe_beforeDiag_info, &intf_info.wan_intf.pppoe_info.before_dial, sizeof(rtk_rg_pppoeClientInfoBeforeDial_t));
						//osal_memcpy(&clientPppoe_info, &intf_info.wan_intf.pppoe_info.after_dial, sizeof(rtk_rg_pppoeClientInfoAfterDial_t));
						diag_util_mprintf("==================================\n");
						break;
					case RTK_RG_PPTP:
						diag_util_mprintf("==================================\n");
						_diag_pptpClientInfoBeforeDialShow(&intf_info.wan_intf.pptp_info.before_dial);
						_diag_pptpClientInfoShow(&intf_info.wan_intf.pptp_info.after_dial);
						//osal_memcpy(&clientPptp_beforeDial_info, &intf_info.wan_intf.pptp_info.before_dial, sizeof(rtk_rg_pptpClientInfoBeforeDial_t));
						//osal_memcpy(&clientPptp_info, &intf_info.wan_intf.pptp_info.after_dial, sizeof(rtk_rg_pptpClientInfoAfterDial_t));
						diag_util_mprintf("==================================\n");
						break;
					case RTK_RG_L2TP:
						diag_util_mprintf("==================================\n");
						_diag_l2tpClientInfoBeforeDialShow(&intf_info.wan_intf.l2tp_info.before_dial);
						_diag_l2tpClientInfoShow(&intf_info.wan_intf.l2tp_info.after_dial);
						//osal_memcpy(&clientL2tp_beforeDial_info, &intf_info.wan_intf.l2tp_info.before_dial, sizeof(rtk_rg_l2tpClientInfoBeforeDial_t));
						//osal_memcpy(&clientL2tp_info, &intf_info.wan_intf.l2tp_info.after_dial, sizeof(rtk_rg_l2tpClientInfoAfterDial_t));
						diag_util_mprintf("==================================\n");
						break;
					case RTK_RG_DSLITE:
						diag_util_mprintf("==================================\n");
						_diag_dsliteInfoShow(&intf_info.wan_intf.dslite_info);
						//osal_memcpy(&clientL2tp_beforeDial_info, &intf_info.wan_intf.l2tp_info.before_dial, sizeof(rtk_rg_l2tpClientInfoBeforeDial_t));
						//osal_memcpy(&clientL2tp_info, &intf_info.wan_intf.l2tp_info.after_dial, sizeof(rtk_rg_l2tpClientInfoAfterDial_t));
						diag_util_mprintf("==================================\n");
						break;
					case RTK_RG_PPPoE_DSLITE:
						diag_util_mprintf("==================================\n");
						_diag_pppoeDsliteInfoShow(&intf_info.wan_intf.pppoe_dslite_info.after_dial);
						//osal_memcpy(&clientL2tp_beforeDial_info, &intf_info.wan_intf.l2tp_info.before_dial, sizeof(rtk_rg_l2tpClientInfoBeforeDial_t));
						//osal_memcpy(&clientL2tp_info, &intf_info.wan_intf.l2tp_info.after_dial, sizeof(rtk_rg_l2tpClientInfoAfterDial_t));
						diag_util_mprintf("==================================\n");
						break;	
					default:
						break;
				}
				diag_util_mprintf("bind_wan_type_ipv4: %d\n",intf_info.wan_intf.bind_wan_type_ipv4);
				diag_util_mprintf("bind_wan_type_ipv6: %d\n",intf_info.wan_intf.bind_wan_type_ipv6);
				diag_util_mprintf("wirelessWan: %s\n",intf_info.wan_intf.wirelessWan==RG_WWAN_WIRED?"Wired":intf_info.wan_intf.wirelessWan==RG_WWAN_WLAN0_VXD?"WLAN0_VXD":"WLAN1_VXD");
				diag_util_mprintf("nexthop_ipv4: %d\n",intf_info.wan_intf.nexthop_ipv4);
				diag_util_mprintf("nexthop_ipv6: %d\n",intf_info.wan_intf.nexthop_ipv6);
				diag_util_mprintf("pppoe_idx: %d\n",intf_info.wan_intf.pppoe_idx);
				diag_util_mprintf("extip_idx: %d\n",intf_info.wan_intf.extip_idx);
				diag_util_mprintf("baseIntf_idx: %d\n",intf_info.wan_intf.baseIntf_idx);
#if defined(CONFIG_RTL9602C_SERIES)				
				diag_util_mprintf("dslite_idx: %d\n",intf_info.wan_intf.dslite_idx);
#endif
			}else{
				diag_util_mprintf("lan_intf:\n");
				diag_util_mprintf("==================================\n");
				_diag_lanIntfShow(&intf_info.lan_intf);
			}
		}else{
			diag_util_mprintf("intf[%d] is empty.\n",*entry_index_ptr);
			return CPARSER_NOT_OK;
		}
	}
 
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_intf_entry_entry_index */

/*
 * rg del intf entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_del_intf_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();

	ret = rtk_rg_interface_del(*entry_index_ptr);
	if(ret!=RT_ERR_RG_OK)
		return CPARSER_NOT_OK;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_del_intf_entry_entry_index */

/*
 * rg show wan-intf-static-info
 */
cparser_result_t
cparser_cmd_rg_show_wan_intf_static_info(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	
	_diag_wanIntfStaticInfoShow(&wan_intf_static_info);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_wan_intf_static_info */

/*
 * rg clear wan-intf-static-info
 */
cparser_result_t
cparser_cmd_rg_clear_wan_intf_static_info(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&wan_intf_static_info, 0x0, sizeof(rtk_rg_ipStaticInfo_t));	

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_wan_intf_static_info */


/*
 * rg add wan-intf-static-info intf-index <UINT:index>
 */
cparser_result_t
cparser_cmd_rg_add_wan_intf_static_info_intf_index_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rtk_rg_staticInfo_set(*index_ptr, &wan_intf_static_info);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add wan-intf-static-info failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{	
		diag_util_mprintf("add static info to interface[%d] success. \n",*index_ptr);
	}
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_wan_intf_static_info_intf_index_index */



/*
 * rg set wan-intf-static-info ip-version <UINT:ip_version>
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_static_info_ip_version_ip_version(
    cparser_context_t *context,
    uint32_t  *ip_version_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	wan_intf_static_info.ip_version=*ip_version_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_wan_intf_static_info_ip_version_ip_version */

/*
 * rg set wan-intf-static-info ip-version <UINT:ip_version> napt_enable <UINT:napt_enable> ip_addr <IPV4ADDR:ip_addr> ip_network_mask <IPV4ADDR:ip_network_mask> ipv4_default_gateway_on <UINT:ipv4_default_gateway_on> gateway_ipv4_addr <IPV4ADDR:gateway_ipv4_addr> mtu <UINT:mtu>
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_static_info_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu(
    cparser_context_t *context,
    uint32_t  *ip_version_ptr,
    uint32_t  *napt_enable_ptr,
    uint32_t  *ip_addr_ptr,
    uint32_t  *ip_network_mask_ptr,
    uint32_t  *ipv4_default_gateway_on_ptr,
    uint32_t  *gateway_ipv4_addr_ptr,
    uint32_t  *mtu_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	
	wan_intf_static_info.ip_version=*ip_version_ptr;
	wan_intf_static_info.napt_enable=*napt_enable_ptr;
	wan_intf_static_info.ip_addr=*ip_addr_ptr;
	wan_intf_static_info.ip_network_mask=*ip_network_mask_ptr;

	wan_intf_static_info.ipv4_default_gateway_on=*ipv4_default_gateway_on_ptr;
    wan_intf_static_info.gateway_ipv4_addr=*gateway_ipv4_addr_ptr;
	wan_intf_static_info.mtu=*mtu_ptr;
	wan_intf_static_info.static_route_with_arp=0;
	
	//gw_mac_auto_learn default setting enable
	wan_intf_static_info.gw_mac_auto_learn_for_ipv4=1;
	memset(wan_intf_static_info.gateway_mac_addr_for_ipv4.octet, 0x0, ETHER_ADDR_LEN);		
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_set_wan_intf_static_info_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu */

/*
 * rg set wan-intf-static-info ip-version <UINT:ip_version> napt_enable <UINT:napt_enable> ip_addr <IPV4ADDR:ip_addr> ip_network_mask <IPV4ADDR:ip_network_mask> ipv4_default_gateway_on <UINT:ipv4_default_gateway_on> gateway_ipv4_addr <IPV4ADDR:gateway_ipv4_addr> mtu <UINT:mtu> gw_mac_auto_learn_for_ipv4 <UINT:gw_mac_auto_learn_for_ipv4> gateway_mac_addr_for_ipv4 <MACADDR:gateway_mac_addr_for_ipv4>
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_static_info_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu_gw_mac_auto_learn_for_ipv4_gw_mac_auto_learn_for_ipv4_gateway_mac_addr_for_ipv4_gateway_mac_addr_for_ipv4(
    cparser_context_t *context,
    uint32_t  *ip_version_ptr,
    uint32_t  *napt_enable_ptr,
    uint32_t  *ip_addr_ptr,
    uint32_t  *ip_network_mask_ptr,
    uint32_t  *ipv4_default_gateway_on_ptr,
    uint32_t  *gateway_ipv4_addr_ptr,
    uint32_t  *mtu_ptr,
    uint32_t  *gw_mac_auto_learn_for_ipv4_ptr,
    cparser_macaddr_t  *gateway_mac_addr_for_ipv4_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	
	wan_intf_static_info.ip_version=*ip_version_ptr;
	wan_intf_static_info.napt_enable=*napt_enable_ptr;
	wan_intf_static_info.ip_addr=*ip_addr_ptr;
	wan_intf_static_info.ip_network_mask=*ip_network_mask_ptr;

	wan_intf_static_info.ipv4_default_gateway_on=*ipv4_default_gateway_on_ptr;
    wan_intf_static_info.gateway_ipv4_addr=*gateway_ipv4_addr_ptr;
	wan_intf_static_info.mtu=*mtu_ptr;
	wan_intf_static_info.static_route_with_arp=0;
	
	wan_intf_static_info.gw_mac_auto_learn_for_ipv4=*gw_mac_auto_learn_for_ipv4_ptr;
	osal_memcpy(wan_intf_static_info.gateway_mac_addr_for_ipv4.octet, gateway_mac_addr_for_ipv4_ptr->octet, ETHER_ADDR_LEN);
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_set_wan_intf_static_info_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu_gw_mac_auto_learn_for_ipv4_gw_mac_auto_learn_for_ipv4_gateway_mac_addr_for_ipv4_gateway_mac_addr_for_ipv4 */


/*
 * rg set wan-intf-static-info ip-version <UINT:ip_version> napt_enable <UINT:napt_enable> ip_addr <IPV4ADDR:ip_addr> ip_network_mask <IPV4ADDR:ip_network_mask> ipv4_default_gateway_on <UINT:ipv4_default_gateway_on> gateway_ipv4_addr <IPV4ADDR:gateway_ipv4_addr> mtu <UINT:mtu> routing-type <UINT:routing_type>
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_static_info_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu_routing_type_routing_type(
    cparser_context_t *context,
    uint32_t  *ip_version_ptr,
    uint32_t  *napt_enable_ptr,
    uint32_t  *ip_addr_ptr,
    uint32_t  *ip_network_mask_ptr,
    uint32_t  *ipv4_default_gateway_on_ptr,
    uint32_t  *gateway_ipv4_addr_ptr,
    uint32_t  *mtu_ptr,
    uint32_t  *routing_type_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	
	wan_intf_static_info.ip_version=*ip_version_ptr;
	wan_intf_static_info.napt_enable=*napt_enable_ptr;
	wan_intf_static_info.ip_addr=*ip_addr_ptr;
	wan_intf_static_info.ip_network_mask=*ip_network_mask_ptr;

	wan_intf_static_info.ipv4_default_gateway_on=*ipv4_default_gateway_on_ptr;
    wan_intf_static_info.gateway_ipv4_addr=*gateway_ipv4_addr_ptr;
	wan_intf_static_info.mtu=*mtu_ptr;
	wan_intf_static_info.static_route_with_arp=*routing_type_ptr;
	
	//gw_mac_auto_learn default setting enable
	wan_intf_static_info.gw_mac_auto_learn_for_ipv4=1;
	memset(wan_intf_static_info.gateway_mac_addr_for_ipv4.octet, 0x0, ETHER_ADDR_LEN);		
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_set_wan_intf_static_info_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu_routing_type_routing_type */

/*
 * rg set wan-intf-static-info ip-version <UINT:ip_version> napt_enable <UINT:napt_enable> ip_addr <IPV4ADDR:ip_addr> ip_network_mask <IPV4ADDR:ip_network_mask> ipv4_default_gateway_on <UINT:ipv4_default_gateway_on> gateway_ipv4_addr <IPV4ADDR:gateway_ipv4_addr> mtu <UINT:mtu> routing-type <UINT:routing_type> gw_mac_auto_learn_for_ipv4 <UINT:gw_mac_auto_learn_for_ipv4> gateway_mac_addr_for_ipv4 <MACADDR:gateway_mac_addr_for_ipv4> 
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_static_info_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu_routing_type_routing_type_gw_mac_auto_learn_for_ipv4_gw_mac_auto_learn_for_ipv4_gateway_mac_addr_for_ipv4_gateway_mac_addr_for_ipv4(
    cparser_context_t *context,
    uint32_t  *ip_version_ptr,
    uint32_t  *napt_enable_ptr,
    uint32_t  *ip_addr_ptr,
    uint32_t  *ip_network_mask_ptr,
    uint32_t  *ipv4_default_gateway_on_ptr,
    uint32_t  *gateway_ipv4_addr_ptr,
    uint32_t  *mtu_ptr,
    uint32_t  *routing_type_ptr,
    uint32_t  *gw_mac_auto_learn_for_ipv4_ptr,
    cparser_macaddr_t  *gateway_mac_addr_for_ipv4_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	
	wan_intf_static_info.ip_version=*ip_version_ptr;
	wan_intf_static_info.napt_enable=*napt_enable_ptr;
	wan_intf_static_info.ip_addr=*ip_addr_ptr;
	wan_intf_static_info.ip_network_mask=*ip_network_mask_ptr;

	wan_intf_static_info.ipv4_default_gateway_on=*ipv4_default_gateway_on_ptr;
    wan_intf_static_info.gateway_ipv4_addr=*gateway_ipv4_addr_ptr;
	wan_intf_static_info.mtu=*mtu_ptr;
	wan_intf_static_info.static_route_with_arp=*routing_type_ptr;
	
	wan_intf_static_info.gw_mac_auto_learn_for_ipv4=*gw_mac_auto_learn_for_ipv4_ptr;
	osal_memcpy(wan_intf_static_info.gateway_mac_addr_for_ipv4.octet, gateway_mac_addr_for_ipv4_ptr->octet, ETHER_ADDR_LEN);
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_set_wan_intf_static_info_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu_routing_type_routing_type_gw_mac_auto_learn_for_ipv4_gw_mac_auto_learn_for_ipv4_gateway_mac_addr_for_ipv4_gateway_mac_addr_for_ipv4 */


/*
 * rg set wan-intf-static-info-ipv6 ipv6_addr <IPV6ADDR:ipv6_addr> ipv6_mask_length <UINT:ipv6_mask_length> ipv6_default_gateway_on <UINT:ipv6_default_gateway_on> gateway_ipv6_addr <IPV6ADDR:gateway_ipv6_addr> mtu <UINT:mtu>
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_static_info_ipv6_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_mtu_mtu(
    cparser_context_t *context,
    char * *ipv6_addr_ptr,
    uint32_t  *ipv6_mask_length_ptr,
    uint32_t  *ipv6_default_gateway_on_ptr,
    char * *gateway_ipv6_addr_ptr,
    uint32_t  *mtu_ptr)
{
	int32  ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&wan_intf_static_info.ipv6_addr.ipv6_addr[0], TOKEN_STR(4)), ret);
    wan_intf_static_info.ipv6_mask_length=*ipv6_mask_length_ptr;
    wan_intf_static_info.ipv6_default_gateway_on=*ipv6_default_gateway_on_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&wan_intf_static_info.gateway_ipv6_addr.ipv6_addr[0], TOKEN_STR(10)), ret);

	wan_intf_static_info.gw_mac_auto_learn_for_ipv6=1;
	memset(wan_intf_static_info.gateway_mac_addr_for_ipv6.octet, 0x0, ETHER_ADDR_LEN);	
	wan_intf_static_info.mtu=*mtu_ptr;
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_set_wan_intf_static_info_ipv6_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_mtu_mtu */


/*
 * rg set wan-intf-static-info-ipv6 ipv6_addr <IPV6ADDR:ipv6_addr> ipv6_mask_length <UINT:ipv6_mask_length> ipv6_default_gateway_on <UINT:ipv6_default_gateway_on> gateway_ipv6_addr <IPV6ADDR:gateway_ipv6_addr> mtu <UINT:mtu> gw_mac_auto_learn_for_ipv6 <UINT:gw_mac_auto_learn_for_ipv6> gateway_mac_addr_for_ipv6 <MACADDR:gateway_mac_addr_for_ipv6>
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_static_info_ipv6_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_mtu_mtu_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6(
    cparser_context_t *context,
    char * *ipv6_addr_ptr,
    uint32_t  *ipv6_mask_length_ptr,
    uint32_t  *ipv6_default_gateway_on_ptr,
    char * *gateway_ipv6_addr_ptr,
    uint32_t  *mtu_ptr,
    uint32_t  *gw_mac_auto_learn_for_ipv6_ptr,
    cparser_macaddr_t  *gateway_mac_addr_for_ipv6_ptr)
{
	int32  ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&wan_intf_static_info.ipv6_addr.ipv6_addr[0], TOKEN_STR(4)), ret);
    wan_intf_static_info.ipv6_mask_length=*ipv6_mask_length_ptr;
    wan_intf_static_info.ipv6_default_gateway_on=*ipv6_default_gateway_on_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&wan_intf_static_info.gateway_ipv6_addr.ipv6_addr[0], TOKEN_STR(10)), ret);
	wan_intf_static_info.mtu=*mtu_ptr;

	wan_intf_static_info.gw_mac_auto_learn_for_ipv6=*gw_mac_auto_learn_for_ipv6_ptr;
	osal_memcpy(wan_intf_static_info.gateway_mac_addr_for_ipv6.octet, gateway_mac_addr_for_ipv6_ptr->octet, ETHER_ADDR_LEN);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_wan_intf_static_info_ipv6_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_mtu_mtu_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6 */



/*
 * rg set wan-intf-static-info-ipv6 ipv6_napt_enable <UINT:ipv6_napt_enable> ipv6_addr <IPV6ADDR:ipv6_addr> ipv6_mask_length <UINT:ipv6_mask_length> ipv6_default_gateway_on <UINT:ipv6_default_gateway_on> gateway_ipv6_addr <IPV6ADDR:gateway_ipv6_addr> mtu <UINT:mtu>
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_static_info_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_mtu_mtu(
    cparser_context_t *context,
    uint32_t  *ipv6_napt_enable_ptr,
    char * *ipv6_addr_ptr,
    uint32_t  *ipv6_mask_length_ptr,
    uint32_t  *ipv6_default_gateway_on_ptr,
    char * *gateway_ipv6_addr_ptr,
    uint32_t  *mtu_ptr)
{
	int32  ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
	wan_intf_static_info.ipv6_napt_enable = *ipv6_napt_enable_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&wan_intf_static_info.ipv6_addr.ipv6_addr[0], TOKEN_STR(6)), ret);
    wan_intf_static_info.ipv6_mask_length=*ipv6_mask_length_ptr;
    wan_intf_static_info.ipv6_default_gateway_on=*ipv6_default_gateway_on_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&wan_intf_static_info.gateway_ipv6_addr.ipv6_addr[0], TOKEN_STR(12)), ret);

	wan_intf_static_info.gw_mac_auto_learn_for_ipv6=1;
	memset(wan_intf_static_info.gateway_mac_addr_for_ipv6.octet, 0x0, ETHER_ADDR_LEN);	
	wan_intf_static_info.mtu=*mtu_ptr;
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_set_wan_intf_static_info_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_mtu_mtu */


/*
 * rg set wan-intf-static-info-ipv6 ipv6_napt_enable <UINT:ipv6_napt_enable> ipv6_addr <IPV6ADDR:ipv6_addr> ipv6_mask_length <UINT:ipv6_mask_length> ipv6_default_gateway_on <UINT:ipv6_default_gateway_on> gateway_ipv6_addr <IPV6ADDR:gateway_ipv6_addr> mtu <UINT:mtu> gw_mac_auto_learn_for_ipv6 <UINT:gw_mac_auto_learn_for_ipv6> gateway_mac_addr_for_ipv6 <MACADDR:gateway_mac_addr_for_ipv6>
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_static_info_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_mtu_mtu_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6(
    cparser_context_t *context,
    uint32_t  *ipv6_napt_enable_ptr,
    char * *ipv6_addr_ptr,
    uint32_t  *ipv6_mask_length_ptr,
    uint32_t  *ipv6_default_gateway_on_ptr,
    char * *gateway_ipv6_addr_ptr,
    uint32_t  *mtu_ptr,
    uint32_t  *gw_mac_auto_learn_for_ipv6_ptr,
    cparser_macaddr_t  *gateway_mac_addr_for_ipv6_ptr)
{
	int32  ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
	wan_intf_static_info.ipv6_napt_enable = *ipv6_napt_enable_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&wan_intf_static_info.ipv6_addr.ipv6_addr[0], TOKEN_STR(6)), ret);
    wan_intf_static_info.ipv6_mask_length=*ipv6_mask_length_ptr;
    wan_intf_static_info.ipv6_default_gateway_on=*ipv6_default_gateway_on_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&wan_intf_static_info.gateway_ipv6_addr.ipv6_addr[0], TOKEN_STR(12)), ret);
	wan_intf_static_info.mtu=*mtu_ptr;

	wan_intf_static_info.gw_mac_auto_learn_for_ipv6=*gw_mac_auto_learn_for_ipv6_ptr;
	osal_memcpy(wan_intf_static_info.gateway_mac_addr_for_ipv6.octet, gateway_mac_addr_for_ipv6_ptr->octet, ETHER_ADDR_LEN);

    return CPARSER_OK;

}    /* end of cparser_cmd_rg_set_wan_intf_static_info_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_mtu_mtu_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6 */

/*
 * rg show wan-intf-dslite-info
 */
cparser_result_t
cparser_cmd_rg_show_wan_intf_dslite_info(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	
	_diag_dsliteInfoShow(&wan_intf_dslite_info);
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_wan_intf_dslite_info */

/*
 * rg clear wan-intf-dslite-info
 */
cparser_result_t
cparser_cmd_rg_clear_wan_intf_dslite_info(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&wan_intf_dslite_info, 0x0, sizeof(rtk_rg_ipDslitStaticInfo_t));	
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_wan_intf_dslite_info */

/*
 * rg add wan-intf-dslite-info intf-index <UINT:index>
 */
cparser_result_t
cparser_cmd_rg_add_wan_intf_dslite_info_intf_index_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int ret;
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rtk_rg_dsliteInfo_set(*index_ptr, &wan_intf_dslite_info);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add wan-intf-dslite-info failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{	
		diag_util_mprintf("add dslite info to interface[%d] success. \n",*index_ptr);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_wan_intf_dslite_info_intf_index_index */

/*
 * rg set wan-intf-dslite-info ip-version <UINT:ip_version> napt_enable <UINT:napt_enable> ip_addr <IPV4ADDR:ip_addr> ip_network_mask <IPV4ADDR:ip_network_mask> ipv4_default_gateway_on <UINT:ipv4_default_gateway_on> mtu <UINT:mtu>
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_dslite_info_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_mtu_mtu(
    cparser_context_t *context,
    uint32_t  *ip_version_ptr,
    uint32_t  *napt_enable_ptr,
    uint32_t  *ip_addr_ptr,
    uint32_t  *ip_network_mask_ptr,
    uint32_t  *ipv4_default_gateway_on_ptr,
    uint32_t  *mtu_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	wan_intf_dslite_info.static_info.ip_version=*ip_version_ptr;
	wan_intf_dslite_info.static_info.napt_enable=*napt_enable_ptr;
	wan_intf_dslite_info.static_info.ip_addr=*ip_addr_ptr;
	wan_intf_dslite_info.static_info.ip_network_mask=*ip_network_mask_ptr;

	wan_intf_dslite_info.static_info.ipv4_default_gateway_on=*ipv4_default_gateway_on_ptr;
    wan_intf_dslite_info.static_info.gateway_ipv4_addr=0;
	wan_intf_dslite_info.static_info.mtu=*mtu_ptr;
	wan_intf_dslite_info.static_info.static_route_with_arp=0;
	
	//gw_mac_auto_learn default setting enable
	wan_intf_dslite_info.static_info.gw_mac_auto_learn_for_ipv4=1;
	memset(wan_intf_dslite_info.static_info.gateway_mac_addr_for_ipv4.octet, 0x0, ETHER_ADDR_LEN);	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_wan_intf_dslite_info_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_mtu_mtu */

/*
 * rg set wan-intf-dslite-info-ipv6 ipv6_napt_enable <UINT:ipv6_napt_enable> ipv6_addr <IPV6ADDR:ipv6_addr> ipv6_mask_length <UINT:ipv6_mask_length> ipv6_default_gateway_on <UINT:ipv6_default_gateway_on> gateway_ipv6_addr <IPV6ADDR:gateway_ipv6_addr> mtu <UINT:mtu> gw_mac_auto_learn_for_ipv6 <UINT:gw_mac_auto_learn_for_ipv6> gateway_mac_addr_for_ipv6 <MACADDR:gateway_mac_addr_for_ipv6> b4_ipv6_addr <IPV6ADDR:b4_ipv6_addr> aftr_ipv6_addr <IPV6ADDR:aftr_ipv6_addr> aftr_mac_auto_learn <UINT:aftr_mac_auto_learn> aftr_mac_addr <MACADDR:aftr_mac_addr>
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_dslite_info_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_mtu_mtu_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6_b4_ipv6_addr_b4_ipv6_addr_aftr_ipv6_addr_aftr_ipv6_addr_aftr_mac_auto_learn_aftr_mac_auto_learn_aftr_mac_addr_aftr_mac_addr(
    cparser_context_t *context,
    uint32_t  *ipv6_napt_enable_ptr,
    char * *ipv6_addr_ptr,
    uint32_t  *ipv6_mask_length_ptr,
    uint32_t  *ipv6_default_gateway_on_ptr,
    char * *gateway_ipv6_addr_ptr,
    uint32_t  *mtu_ptr,
    uint32_t  *gw_mac_auto_learn_for_ipv6_ptr,
    cparser_macaddr_t  *gateway_mac_addr_for_ipv6_ptr,
    char * *b4_ipv6_addr_ptr,
    char * *aftr_ipv6_addr_ptr,
    uint32_t  *aftr_mac_auto_learn_ptr,
    cparser_macaddr_t  *aftr_mac_addr_ptr)
{
    int32  ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
	wan_intf_dslite_info.static_info.ipv6_napt_enable=*ipv6_napt_enable_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(wan_intf_dslite_info.static_info.ipv6_addr.ipv6_addr, TOKEN_STR(6)), ret);
    wan_intf_dslite_info.static_info.ipv6_mask_length=*ipv6_mask_length_ptr;
    wan_intf_dslite_info.static_info.ipv6_default_gateway_on=*ipv6_default_gateway_on_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(wan_intf_dslite_info.static_info.gateway_ipv6_addr.ipv6_addr, TOKEN_STR(12)), ret);

	wan_intf_dslite_info.static_info.gw_mac_auto_learn_for_ipv6=*gw_mac_auto_learn_for_ipv6_ptr;
	osal_memcpy(wan_intf_dslite_info.static_info.gateway_mac_addr_for_ipv6.octet, gateway_mac_addr_for_ipv6_ptr->octet, ETHER_ADDR_LEN);
	wan_intf_dslite_info.static_info.mtu=*mtu_ptr;
	
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(wan_intf_dslite_info.rtk_dslite.ipB4.ipv6_addr, TOKEN_STR(20)), ret);
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(wan_intf_dslite_info.rtk_dslite.ipAftr.ipv6_addr, TOKEN_STR(22)), ret);
	wan_intf_dslite_info.aftr_mac_auto_learn=*aftr_mac_auto_learn_ptr;
	osal_memcpy(wan_intf_dslite_info.aftr_mac_addr.octet, aftr_mac_addr_ptr->octet, ETHER_ADDR_LEN);
	wan_intf_dslite_info.rtk_dslite.hopLimit=255;
	wan_intf_dslite_info.rtk_dslite.flowLabel=0;
	wan_intf_dslite_info.rtk_dslite.tcOpt=RTK_L34_DSLITE_TC_OPT_ASSIGN;
	wan_intf_dslite_info.rtk_dslite.tc=0;
	wan_intf_dslite_info.rtk_dslite.valid=1;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_wan_intf_dslite_info_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_mtu_mtu_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6_b4_ipv6_addr_b4_ipv6_addr_aftr_ipv6_addr_aftr_ipv6_addr_aftr_mac_auto_learn_aftr_mac_auto_learn_aftr_mac_addr_aftr_mac_addr */

/*
 * rg set wan-intf-dslite-info-ipv6 ipv6_napt_enable <UINT:ipv6_napt_enable> ipv6_addr <IPV6ADDR:ipv6_addr> ipv6_mask_length <UINT:ipv6_mask_length> ipv6_default_gateway_on <UINT:ipv6_default_gateway_on> gateway_ipv6_addr <IPV6ADDR:gateway_ipv6_addr> mtu <UINT:mtu> gw_mac_auto_learn_for_ipv6 <UINT:gw_mac_auto_learn_for_ipv6> gateway_mac_addr_for_ipv6 <MACADDR:gateway_mac_addr_for_ipv6> b4_ipv6_addr <IPV6ADDR:b4_ipv6_addr> aftr_ipv6_addr <IPV6ADDR:aftr_ipv6_addr> aftr_mac_auto_learn <UINT:aftr_mac_auto_learn> aftr_mac_addr <MACADDR:aftr_mac_addr> dslite_hoplimit <UINT:dslite_hoplimit> dslite_flowlabel <HEX:dslite_flowlabel> dslite_tc copy_from_IPv4_TOS
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_dslite_info_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_mtu_mtu_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6_b4_ipv6_addr_b4_ipv6_addr_aftr_ipv6_addr_aftr_ipv6_addr_aftr_mac_auto_learn_aftr_mac_auto_learn_aftr_mac_addr_aftr_mac_addr_dslite_hoplimit_dslite_hoplimit_dslite_flowlabel_dslite_flowlabel_dslite_tc_copy_from_IPv4_TOS(
    cparser_context_t *context,
    uint32_t  *ipv6_napt_enable_ptr,
    char * *ipv6_addr_ptr,
    uint32_t  *ipv6_mask_length_ptr,
    uint32_t  *ipv6_default_gateway_on_ptr,
    char * *gateway_ipv6_addr_ptr,
    uint32_t  *mtu_ptr,
    uint32_t  *gw_mac_auto_learn_for_ipv6_ptr,
    cparser_macaddr_t  *gateway_mac_addr_for_ipv6_ptr,
    char * *b4_ipv6_addr_ptr,
    char * *aftr_ipv6_addr_ptr,
    uint32_t  *aftr_mac_auto_learn_ptr,
    cparser_macaddr_t  *aftr_mac_addr_ptr,
    uint32_t  *dslite_hoplimit_ptr,
    uint32_t  *dslite_flowlabel_ptr)
{
    int32  ret = RT_ERR_FAILED;
#if defined(CONFIG_RTL9600_SERIES)
	return CPARSER_NOT_OK;
#else
    DIAG_UTIL_PARAM_CHK();
	wan_intf_dslite_info.static_info.ipv6_napt_enable=*ipv6_napt_enable_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(wan_intf_dslite_info.static_info.ipv6_addr.ipv6_addr, TOKEN_STR(6)), ret);
    wan_intf_dslite_info.static_info.ipv6_mask_length=*ipv6_mask_length_ptr;
    wan_intf_dslite_info.static_info.ipv6_default_gateway_on=*ipv6_default_gateway_on_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(wan_intf_dslite_info.static_info.gateway_ipv6_addr.ipv6_addr, TOKEN_STR(12)), ret);

	wan_intf_dslite_info.static_info.gw_mac_auto_learn_for_ipv6=*gw_mac_auto_learn_for_ipv6_ptr;
	osal_memcpy(wan_intf_dslite_info.static_info.gateway_mac_addr_for_ipv6.octet, gateway_mac_addr_for_ipv6_ptr->octet, ETHER_ADDR_LEN);
	wan_intf_dslite_info.static_info.mtu=*mtu_ptr;
	
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(wan_intf_dslite_info.rtk_dslite.ipB4.ipv6_addr, TOKEN_STR(20)), ret);
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(wan_intf_dslite_info.rtk_dslite.ipAftr.ipv6_addr, TOKEN_STR(22)), ret);
	wan_intf_dslite_info.aftr_mac_auto_learn=*aftr_mac_auto_learn_ptr;
	osal_memcpy(wan_intf_dslite_info.aftr_mac_addr.octet, aftr_mac_addr_ptr->octet, ETHER_ADDR_LEN);
	wan_intf_dslite_info.rtk_dslite.hopLimit=(*dslite_hoplimit_ptr)&0xff;
	wan_intf_dslite_info.rtk_dslite.flowLabel=(*dslite_flowlabel_ptr)&0xfffff;
	wan_intf_dslite_info.rtk_dslite.tcOpt=RTK_L34_DSLITE_TC_OPT_COPY_FROM_TOS;
	wan_intf_dslite_info.rtk_dslite.tc=0;
	wan_intf_dslite_info.rtk_dslite.valid=1;
    return CPARSER_OK;
#endif
}    /* end of cparser_cmd_rg_set_wan_intf_dslite_info_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_mtu_mtu_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6_b4_ipv6_addr_b4_ipv6_addr_aftr_ipv6_addr_aftr_ipv6_addr_aftr_mac_auto_learn_aftr_mac_auto_learn_aftr_mac_addr_aftr_mac_addr_dslite_hoplimit_dslite_hoplimit_dslite_flowlabel_dslite_flowlabel_dslite_tc_copy_from_ipv4_tos */

/*
 * rg set wan-intf-dslite-info-ipv6 ipv6_napt_enable <UINT:ipv6_napt_enable> ipv6_addr <IPV6ADDR:ipv6_addr> ipv6_mask_length <UINT:ipv6_mask_length> ipv6_default_gateway_on <UINT:ipv6_default_gateway_on> gateway_ipv6_addr <IPV6ADDR:gateway_ipv6_addr> mtu <UINT:mtu> gw_mac_auto_learn_for_ipv6 <UINT:gw_mac_auto_learn_for_ipv6> gateway_mac_addr_for_ipv6 <MACADDR:gateway_mac_addr_for_ipv6> b4_ipv6_addr <IPV6ADDR:b4_ipv6_addr> aftr_ipv6_addr <IPV6ADDR:aftr_ipv6_addr> aftr_mac_auto_learn <UINT:aftr_mac_auto_learn> aftr_mac_addr <MACADDR:aftr_mac_addr> dslite_hoplimit <UINT:dslite_hoplimit> dslite_flowlabel <HEX:dslite_flowlabel> dslite_tc <HEX:dslite_tc>
 */
cparser_result_t
cparser_cmd_rg_set_wan_intf_dslite_info_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_mtu_mtu_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6_b4_ipv6_addr_b4_ipv6_addr_aftr_ipv6_addr_aftr_ipv6_addr_aftr_mac_auto_learn_aftr_mac_auto_learn_aftr_mac_addr_aftr_mac_addr_dslite_hoplimit_dslite_hoplimit_dslite_flowlabel_dslite_flowlabel_dslite_tc_dslite_tc(
    cparser_context_t *context,
    uint32_t  *ipv6_napt_enable_ptr,
    char * *ipv6_addr_ptr,
    uint32_t  *ipv6_mask_length_ptr,
    uint32_t  *ipv6_default_gateway_on_ptr,
    char * *gateway_ipv6_addr_ptr,
    uint32_t  *mtu_ptr,
    uint32_t  *gw_mac_auto_learn_for_ipv6_ptr,
    cparser_macaddr_t  *gateway_mac_addr_for_ipv6_ptr,
    char * *b4_ipv6_addr_ptr,
    char * *aftr_ipv6_addr_ptr,
    uint32_t  *aftr_mac_auto_learn_ptr,
    cparser_macaddr_t  *aftr_mac_addr_ptr,
    uint32_t  *dslite_hoplimit_ptr,
    uint32_t  *dslite_flowlabel_ptr,
    uint32_t  *dslite_tc_ptr)
{
    int32  ret = RT_ERR_FAILED;
#if defined(CONFIG_RTL9600_SERIES)
	return CPARSER_NOT_OK;
#else
    DIAG_UTIL_PARAM_CHK();
	wan_intf_dslite_info.static_info.ipv6_napt_enable=*ipv6_napt_enable_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(wan_intf_dslite_info.static_info.ipv6_addr.ipv6_addr, TOKEN_STR(6)), ret);
    wan_intf_dslite_info.static_info.ipv6_mask_length=*ipv6_mask_length_ptr;
    wan_intf_dslite_info.static_info.ipv6_default_gateway_on=*ipv6_default_gateway_on_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(wan_intf_dslite_info.static_info.gateway_ipv6_addr.ipv6_addr, TOKEN_STR(12)), ret);

	wan_intf_dslite_info.static_info.gw_mac_auto_learn_for_ipv6=*gw_mac_auto_learn_for_ipv6_ptr;
	osal_memcpy(wan_intf_dslite_info.static_info.gateway_mac_addr_for_ipv6.octet, gateway_mac_addr_for_ipv6_ptr->octet, ETHER_ADDR_LEN);
	wan_intf_dslite_info.static_info.mtu=*mtu_ptr;
	
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(wan_intf_dslite_info.rtk_dslite.ipB4.ipv6_addr, TOKEN_STR(20)), ret);
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(wan_intf_dslite_info.rtk_dslite.ipAftr.ipv6_addr, TOKEN_STR(22)), ret);
	wan_intf_dslite_info.aftr_mac_auto_learn=*aftr_mac_auto_learn_ptr;
	osal_memcpy(wan_intf_dslite_info.aftr_mac_addr.octet, aftr_mac_addr_ptr->octet, ETHER_ADDR_LEN);
	wan_intf_dslite_info.rtk_dslite.hopLimit=(*dslite_hoplimit_ptr)&0xff;
	wan_intf_dslite_info.rtk_dslite.flowLabel=(*dslite_flowlabel_ptr)&0xfffff;
	wan_intf_dslite_info.rtk_dslite.tcOpt=RTK_L34_DSLITE_TC_OPT_ASSIGN;
	wan_intf_dslite_info.rtk_dslite.tc=(*dslite_tc_ptr)&0xff;
	wan_intf_dslite_info.rtk_dslite.valid=1;
    return CPARSER_OK;
#endif
}    /* end of cparser_cmd_rg_set_wan_intf_dslite_info_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_mtu_mtu_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6_b4_ipv6_addr_b4_ipv6_addr_aftr_ipv6_addr_aftr_ipv6_addr_aftr_mac_auto_learn_aftr_mac_auto_learn_aftr_mac_addr_aftr_mac_addr_dslite_hoplimit_dslite_hoplimit_dslite_flowlabel_dslite_flowlabel_dslite_tc_dslite_tc */



/*
 * rg show mac-entry
 */
cparser_result_t
cparser_cmd_rg_show_mac_entry(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	
	_diag_macEntryShow(&macEntry);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_mac_entry */

/*
 * rg clear mac-entry
 */
cparser_result_t
cparser_cmd_rg_clear_mac_entry(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

	osal_memset(&macEntry, 0x0, sizeof(rtk_rg_macEntry_t));	

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_mac_entry */

/*
 * rg add mac-entry entry
 */
cparser_result_t
cparser_cmd_rg_add_mac_entry_entry(
    cparser_context_t *context)
{
	int ret;
	int index;
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rtk_rg_macEntry_add(&macEntry, &index);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add macEntry failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("add macEntry[%d] success.\n",index);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_mac_entry_entry */

/*
 * rg get mac-entry entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_get_mac_entry_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int ret;
	int index = *entry_index_ptr;
	
	DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&macEntry, 0x0, sizeof(rtk_rg_macEntry_t));	
	ret = rtk_rg_macEntry_find(&macEntry,&index);
	
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get macEntry failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		if(index==*entry_index_ptr){	
			_diag_macEntryShow(&macEntry);
		}
		else{
			diag_util_mprintf("macEntry[%d] is empty.\n",*entry_index_ptr);
			return CPARSER_NOT_OK;
		}
	}
  
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_mac_entry_entry_entry_index */

/*
 * rg del mac-entry entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_del_mac_entry_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();

	ret = rtk_rg_macEntry_del(*entry_index_ptr);
	if(ret!=RT_ERR_RG_OK)
		return CPARSER_NOT_OK;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_del_mac_entry_entry_entry_index */

/*
 * rg set mac-entry mac-address <MACADDR:mac> isIVL <UINT:isIVL> fid <UINT:fid> vlan_id <UINT:vlan_id> port_idx <UINT:port_idx> static_entry <UINT:static_entry>
 */
cparser_result_t
cparser_cmd_rg_set_mac_entry_mac_address_mac_isIVL_isIVL_fid_fid_vlan_id_vlan_id_port_idx_port_idx_static_entry_static_entry(
    cparser_context_t *context,
    cparser_macaddr_t  *mac_ptr,
    uint32_t  *isIVL_ptr,
    uint32_t  *fid_ptr,
    uint32_t  *vlan_id_ptr,
    uint32_t  *port_idx_ptr,
    uint32_t  *static_entry_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	
	osal_memcpy(&macEntry.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
	macEntry.isIVL=*isIVL_ptr;	macEntry.fid=*fid_ptr;
	macEntry.vlan_id=*vlan_id_ptr;	macEntry.port_idx=*port_idx_ptr;
	macEntry.static_entry=*static_entry_ptr;
#if defined(CONFIG_RTL9600_SERIES)
#else
	if(macEntry.vlan_id==0)
		macEntry.ctag_if = 0;
	else
		macEntry.ctag_if = 1;
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_mac_entry_mac_address_mac_isivl_isivl_fid_fid_vlan_id_vlan_id_port_idx_port_idx_static_entry_static_entry */

/*
 * rg set mac-entry mac-address <MACADDR:mac> isIVL <UINT:isIVL> fid <UINT:fid> vlan_id <UINT:vlan_id> port_idx <UINT:port_idx> static_entry <UINT:static_entry> auth <UINT:auth>
 */
cparser_result_t
cparser_cmd_rg_set_mac_entry_mac_address_mac_isIVL_isIVL_fid_fid_vlan_id_vlan_id_port_idx_port_idx_static_entry_static_entry_auth_auth(
    cparser_context_t *context,
    cparser_macaddr_t  *mac_ptr,
    uint32_t  *isIVL_ptr,
    uint32_t  *fid_ptr,
    uint32_t  *vlan_id_ptr,
    uint32_t  *port_idx_ptr,
    uint32_t  *static_entry_ptr,
    uint32_t  *auth_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	
	osal_memcpy(&macEntry.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
	macEntry.isIVL=*isIVL_ptr;	macEntry.fid=*fid_ptr;
	macEntry.vlan_id=*vlan_id_ptr;	macEntry.port_idx=*port_idx_ptr;
	macEntry.static_entry=*static_entry_ptr;
	macEntry.auth=*auth_ptr;
#if defined(CONFIG_RTL9600_SERIES)
#else
	if(macEntry.vlan_id==0)
		macEntry.ctag_if = 0;
	else
		macEntry.ctag_if = 1;
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_mac_entry_mac_address_mac_isivl_isivl_fid_fid_vlan_id_vlan_id_port_idx_port_idx_static_entry_static_entry_auth_auth */

/*
 * rg set mac-entry mac-address <MACADDR:mac> isIVL <UINT:isIVL> fid <UINT:fid> vlan_id <UINT:vlan_id> ctag_if <UINT:ctag_if> port_idx <UINT:port_idx> static_entry <UINT:static_entry>
 */
cparser_result_t
cparser_cmd_rg_set_mac_entry_mac_address_mac_isIVL_isIVL_fid_fid_vlan_id_vlan_id_ctag_if_ctag_if_port_idx_port_idx_static_entry_static_entry(
    cparser_context_t *context,
    cparser_macaddr_t  *mac_ptr,
    uint32_t  *isIVL_ptr,
    uint32_t  *fid_ptr,
    uint32_t  *vlan_id_ptr,
    uint32_t  *ctag_if_ptr,
    uint32_t  *port_idx_ptr,
    uint32_t  *static_entry_ptr)
{
#if defined(CONFIG_RTL9600_SERIES)
	return CPARSER_NOT_OK;
#else
    DIAG_UTIL_PARAM_CHK();

	osal_memcpy(&macEntry.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
	macEntry.isIVL=*isIVL_ptr;	macEntry.fid=*fid_ptr;
	macEntry.vlan_id=*vlan_id_ptr;	macEntry.port_idx=*port_idx_ptr;
	macEntry.static_entry=*static_entry_ptr;
	macEntry.ctag_if = *ctag_if_ptr;

    return CPARSER_OK;
#endif
}    /* end of cparser_cmd_rg_set_mac_entry_mac_address_mac_isivl_isivl_fid_fid_vlan_id_vlan_id_ctag_if_ctag_if_port_idx_port_idx_static_entry_static_entry */

/*
 * rg set mac-entry mac-address <MACADDR:mac> isIVL <UINT:isIVL> fid <UINT:fid> vlan_id <UINT:vlan_id> ctag_if <UINT:ctag_if> port_idx <UINT:port_idx> static_entry <UINT:static_entry> auth <UINT:auth>
 */
cparser_result_t
cparser_cmd_rg_set_mac_entry_mac_address_mac_isIVL_isIVL_fid_fid_vlan_id_vlan_id_ctag_if_ctag_if_port_idx_port_idx_static_entry_static_entry_auth_auth(
    cparser_context_t *context,
    cparser_macaddr_t  *mac_ptr,
    uint32_t  *isIVL_ptr,
    uint32_t  *fid_ptr,
    uint32_t  *vlan_id_ptr,
    uint32_t  *ctag_if_ptr,
    uint32_t  *port_idx_ptr,
    uint32_t  *static_entry_ptr,
    uint32_t  *auth_ptr)
{
#if defined(CONFIG_RTL9600_SERIES)
	return CPARSER_NOT_OK;
#else
    DIAG_UTIL_PARAM_CHK();

	osal_memcpy(&macEntry.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
	macEntry.isIVL=*isIVL_ptr;	macEntry.fid=*fid_ptr;
	macEntry.vlan_id=*vlan_id_ptr;	macEntry.port_idx=*port_idx_ptr;
	macEntry.static_entry=*static_entry_ptr;
	macEntry.auth=*auth_ptr;
	macEntry.ctag_if = *ctag_if_ptr;

    return CPARSER_OK;
#endif
}    /* end of cparser_cmd_rg_set_mac_entry_mac_address_mac_isivl_isivl_fid_fid_vlan_id_vlan_id_ctag_if_ctag_if_port_idx_port_idx_static_entry_static_entry_auth_auth */

/*
 * rg show arp-entry
 */
cparser_result_t
cparser_cmd_rg_show_arp_entry(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	 
	_diag_arpEntryShow(&arpEntry);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_arp_entry */

/*
 * rg clear arp-entry
 */
cparser_result_t
cparser_cmd_rg_clear_arp_entry(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&arpEntry, 0x0, sizeof(rtk_rg_arpEntry_t)); 

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_arp_entry */

/*
 * rg add arp-entry entry
 */
cparser_result_t
cparser_cmd_rg_add_arp_entry_entry(
    cparser_context_t *context)
{
	int ret;
	int index;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	
	ret = rtk_rg_arpEntry_add(&arpEntry, &index);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add arpEntry failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add arpEntry[%d] success.\n",index);
	}
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_arp_entry_entry */

/*
 * rg get arp-entry entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_get_arp_entry_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int ret;
	int index = *entry_index_ptr;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&arpInfo, 0x0, sizeof(rtk_rg_arpInfo_t));	
	ret = rtk_rg_arpEntry_find(&arpInfo,&index);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get arpEntry failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		if(index==*entry_index_ptr){	
			_diag_arpEntryShow(&arpInfo.arpEntry);
			diag_util_mprintf("valid: %d \n",arpInfo.valid);
			diag_util_mprintf("idleSecs: %d \n",arpInfo.idleSecs);
			_diag_lanNetInfoShow(&arpInfo.lanNetInfo);
		}
		else{
			diag_util_mprintf("arpEntry[%d] is empty.\n",*entry_index_ptr);
			return CPARSER_NOT_OK;
		}
	}
  
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_arp_entry_entry_entry_index */

/*
 * rg del arp-entry entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_del_arp_entry_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
 
	 int ret;
	 DIAG_UTIL_PARAM_CHK();
	 
	 ret = rtk_rg_arpEntry_del(*entry_index_ptr);
	 if(ret!=RT_ERR_RG_OK)
		 return CPARSER_NOT_OK;
 
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_del_arp_entry_entry_entry_index */

/*
 * rg set arp-entry macEntryIdx <UINT:macEntryIdx> ip_addr <IPV4ADDR:ipv4> static_entry <UINT:static_entry> valid <UINT:valid>
 */
cparser_result_t
cparser_cmd_rg_set_arp_entry_macEntryIdx_macEntryIdx_ip_addr_ipv4_static_entry_static_entry_valid_valid(
    cparser_context_t *context,
    uint32_t  *macEntryIdx_ptr,
    uint32_t  *ipv4_ptr,
    uint32_t  *static_entry_ptr,
    uint32_t  *valid_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	arpEntry.macEntryIdx=*macEntryIdx_ptr;
	arpEntry.ipv4Addr=*ipv4_ptr;
	arpEntry.staticEntry=*static_entry_ptr;
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_arp_entry_macentryidx_macentryidx_ip_addr_ipv4_static_entry_static_entry_valid_valid */

/*
 * rg show napt-connection 
 */
cparser_result_t
cparser_cmd_rg_show_napt_connection(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	
	_diag_naptConnShow(&naptFlow);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_napt_connection */

/*
 * rg clear napt-connection 
 */
cparser_result_t
cparser_cmd_rg_clear_napt_connection(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&naptFlow, 0x0, sizeof(rtk_rg_naptEntry_t)); 

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_napt_connection */

/*
 * rg add napt-connection entry
 */
cparser_result_t
cparser_cmd_rg_add_napt_connection_entry(
    cparser_context_t *context)
{
	int ret;
	int index;
	
    DIAG_UTIL_PARAM_CHK();	
	DIAG_UTIL_OUTPUT_INIT();
	
	ret =  rtk_rg_naptConnection_add(&naptFlow, &index);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add naptConn failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add naptConn[%d] success.\n",index);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_napt_connection_entry */

/*
 * rg get napt-connection entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_get_napt_connection_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int ret;
	int index = *entry_index_ptr;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT(); 
	osal_memset(&naptInfo, 0x0, sizeof(rtk_rg_naptEntry_t));	

	ret =  rtk_rg_naptConnection_find(&naptInfo,&index);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get naptConn failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		if(index==*entry_index_ptr){	
			_diag_naptConnShow(&naptInfo);
		}
		else{
			diag_util_mprintf("naptConn[%d] is empty.\n",*entry_index_ptr);
			return CPARSER_NOT_OK;
		}
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_napt_connection_entry_entry_index */

/*
 * rg del napt-connection entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_del_napt_connection_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int ret;
	
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_naptConnection_del(*entry_index_ptr);
	if(ret!=RT_ERR_RG_OK)
		return CPARSER_NOT_OK;
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_del_napt_connection_entry_entry_index */

/*
 * rg set napt-connection is_tcp <UINT:is_tcp> local_ip <IPV4ADDR:local_ip> remote_ip <IPV4ADDR:remote_ip> wan_intf_idx <UINT:wan_intf_idx> local_port <UINT:local_port> remote_port <UINT:remote_port> external_port <UINT:external_port> outbound_pri_valid <UINT:outbound_pri_valid> outbound_priority <UINT:outbound_priority> inbound_pri_valid <UINT:inbound_pri_valid> inbound_priority <UINT:inbound_priority>
 */
cparser_result_t
cparser_cmd_rg_set_napt_connection_is_tcp_is_tcp_local_ip_local_ip_remote_ip_remote_ip_wan_intf_idx_wan_intf_idx_local_port_local_port_remote_port_remote_port_external_port_external_port_outbound_pri_valid_outbound_pri_valid_outbound_priority_outbound_priority_inbound_pri_valid_inbound_pri_valid_inbound_priority_inbound_priority(
    cparser_context_t *context,
    uint32_t  *is_tcp_ptr,
    uint32_t  *local_ip_ptr,
    uint32_t  *remote_ip_ptr,
    uint32_t  *wan_intf_idx_ptr,
    uint32_t  *local_port_ptr,
    uint32_t  *remote_port_ptr,
    uint32_t  *external_port_ptr,
    uint32_t  *outbound_pri_valid_ptr,
    uint32_t  *outbound_priority_ptr,
    uint32_t  *inbound_pri_valid_ptr,
    uint32_t  *inbound_priority_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	naptFlow.is_tcp = *is_tcp_ptr;
	naptFlow.local_ip = *local_ip_ptr;
	naptFlow.remote_ip = *remote_ip_ptr;
	naptFlow.wan_intf_idx = *wan_intf_idx_ptr;
	naptFlow.local_port = *local_port_ptr;
	naptFlow.remote_port = *remote_port_ptr;
	naptFlow.external_port = *external_port_ptr;
	naptFlow.outbound_pri_valid = *outbound_pri_valid_ptr;
	naptFlow.outbound_priority = *outbound_priority_ptr;
	naptFlow.inbound_pri_valid = *inbound_pri_valid_ptr;
	naptFlow.inbound_priority = *inbound_priority_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_napt_connection_is_tcp_is_tcp_local_ip_local_ip_remote_ip_remote_ip_wan_intf_idx_wan_intf_idx_local_port_local_port_remote_port_remote_port_external_port_external_port_outbound_pri_valid_outbound_pri_valid_outbound_priority_outbound_priority_inbound_pri_valid_inbound_pri_valid_inbound_priority_inbound_priority */



/*
 * rg clear url-filter entry
 */
cparser_result_t
cparser_cmd_rg_clear_url_filter_entry(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&urlFilter, 0x0, sizeof(rtk_rg_urlFilterString_t)); 
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_url_filter_entry */

/*
 * rg add url-filter entry
 */
cparser_result_t
cparser_cmd_rg_add_url_filter_entry(
    cparser_context_t *context)
{
	int ret;
	int index;

    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	
	ret =  rtk_rg_urlFilterString_add(&urlFilter,&index);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add urlFilter failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add urlFilter[%d] success.\n",index);
	}
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_url_filter_entry */

/*
 * rg set url-filter url <STRING:url> path <STRING:path> path-exactly-match <UINT:match> wan-interface <UINT:wanintf>
 */
cparser_result_t
cparser_cmd_rg_set_url_filter_url_url_path_path_path_exactly_match_match_wan_interface_wanintf(
    cparser_context_t *context,
    char * *url_ptr,
    char * *path_ptr,
    uint32_t  *match_ptr,
    uint32_t  *wanintf_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	
	osal_memcpy(urlFilter.url_filter_string, *url_ptr, MAX_URL_FILTER_STR_LENGTH);	
	osal_memcpy(urlFilter.path_filter_string, *path_ptr, MAX_URL_FILTER_PATH_LENGTH);	
	urlFilter.path_exactly_match = *match_ptr;
	urlFilter.wan_intf= *wanintf_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_url_filter_url_url_path_path_path_exactly_match_match_wan_interface_wanintf */

/*
 * rg get url-filter url-index <UINT:index>
 */
cparser_result_t
cparser_cmd_rg_get_url_filter_url_index_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("");

	int ret;
	int index = *index_ptr;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
	osal_memset(&urlFilter, 0x0, sizeof(rtk_rg_urlFilterString_t));	

	ret =  rtk_rg_urlFilterString_find(&urlFilter,&index);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get urlFilter failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		if(index==*index_ptr){	
			_diag_urlFilterShow(&urlFilter);
		}
		else{
			diag_util_mprintf("naptConn[%d] is empty.\n",*index_ptr);
			return CPARSER_NOT_OK;
		}
	}
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_url_filter_url_index_index */

/*
 * rg del url-filter url-index <UINT:index>
 */
cparser_result_t
cparser_cmd_rg_del_url_filter_url_index_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
	int ret;
	
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_urlFilterString_del(*index_ptr);
	if(ret!=RT_ERR_RG_OK)
		return CPARSER_NOT_OK;
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_del_url_filter_url_index_index */


/*
 * rg add port-isolation port <UINT:port> portmask <HEX:portmask>
 */
cparser_result_t
cparser_cmd_rg_add_port_isolation_port_port_portmask_portmask(
    cparser_context_t *context,
    uint32_t  *port_ptr,
    uint32_t  *portmask_ptr)
{
	int ret;
	rtk_rg_port_isolation_t setting;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	setting.port=*port_ptr;
	setting.portmask.portmask=*portmask_ptr;
	ret=rtk_rg_portIsolation_set(setting);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("add port[%d] isolation failed..ret=%d\n",*port_ptr,ret);
		return CPARSER_NOT_OK;
	}
	
	diag_util_mprintf("add port[%d] isolation success!\n",*port_ptr);
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_port_isolation_port_port_portmask_portmask */

/*
 * rg get port-isolation port <UINT:port>
 */
cparser_result_t
cparser_cmd_rg_get_port_isolation_port_port(
    cparser_context_t *context,
    uint32_t  *port_ptr)
{
	int ret;
	rtk_rg_port_isolation_t setting;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	setting.port=*port_ptr;
	ret=rtk_rg_portIsolation_get(&setting);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get port[%d] isolation failed..ret=%d\n",*port_ptr,ret);
		return CPARSER_NOT_OK;
	}
	
	diag_util_mprintf("port[%d] isolation %x\n",*port_ptr,setting.portmask.portmask);
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_port_isolation_port_port */


/*
 * rg show callback
 */
cparser_result_t
cparser_cmd_rg_show_callback(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	_diag_callbackShow(&init_param);
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_callback */

/*
 * rg clear callback
 */
cparser_result_t
cparser_cmd_rg_clear_callback(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&init_param, 0x0, sizeof(rtk_rg_initParams_t)); 

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_callback */

/*
 * rg set callback initByHwCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_initByHwCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.initByHwCallBack = *addr_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_initbyhwcallback_addr */

/*
 * rg set callback arpAddByHwCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_arpAddByHwCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.arpAddByHwCallBack = *addr_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_arpaddbyhwcallback_addr */

/*
 * rg set callback arpDelByHwCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_arpDelByHwCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.arpDelByHwCallBack = *addr_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_arpdelbyhwcallback_addr */

/*
 * rg set callback macAddByHwCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_macAddByHwCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.macAddByHwCallBack = *addr_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_macaddbyhwcallback_addr */

/*
 * rg set callback macDelByHwCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_macDelByHwCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.macDelByHwCallBack = *addr_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_macdelbyhwcallback_addr */

/*
 * rg set callback naptAddByHwCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_naptAddByHwCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.naptAddByHwCallBack = *addr_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_naptaddbyhwcallback_addr */

/*
 * rg set callback naptDelByHwCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_naptDelByHwCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.naptDelByHwCallBack = *addr_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_naptdelbyhwcallback_addr */

/*
 * rg set callback routingAddByHwCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_routingAddByHwCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.routingAddByHwCallBack = *addr_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_routingaddbyhwcallback_addr */

/*
 * rg set callback routingDelByHwCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_routingDelByHwCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.routingDelByHwCallBack = *addr_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_routingdelbyhwcallback_addr */

/*
 * rg set callback bindingAddByHwCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_bindingAddByHwCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.bindingAddByHwCallBack = *addr_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_bindingaddbyhwcallback_addr */

/*
 * rg set callback bindingDelByHwCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_bindingDelByHwCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.bindingDelByHwCallBack = *addr_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_bindingdelbyhwcallback_addr */

/*
 * rg set callback interfaceAddByHwCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_interfaceAddByHwCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.interfaceAddByHwCallBack = *addr_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_interfaceaddbyhwcallback_addr */

/*
 * rg set callback interfaceDelByHwCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_interfaceDelByHwCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.interfaceDelByHwCallBack = *addr_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_interfacedelbyhwcallback_addr */

/*
 * rg set callback pppoeBeforeDiagByHwCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_pppoeBeforeDiagByHwCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.pppoeBeforeDiagByHwCallBack = *addr_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_pppoebeforediagbyhwcallback_addr */

/*
 * rg set callback pptpBeforeDialByHwCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_pptpBeforeDialByHwCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.pptpBeforeDialByHwCallBack = *addr_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_pptpbeforedialbyhwcallback_addr */

/*
 * rg set callback l2tpBeforeDialByHwCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_l2tpBeforeDialByHwCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.l2tpBeforeDialByHwCallBack = *addr_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_l2tpbeforedialbyhwcallback_addr */

/*
 * rg set callback pppoeDsliteBeforeDiagByHwCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_pppoeDsliteBeforeDiagByHwCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.pppoeDsliteBeforeDialByHwCallBack = *addr_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_pppoedslitebeforediagbyhwcallback_addr */


/*
 * rg set callback neighborAddByHwCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_neighborAddByHwCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.neighborAddByHwCallBack = *addr_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_neighboraddbyhwcallback_addr */

/*
 * rg set callback neighborDelByHwCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_neighborDelByHwCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.neighborDelByHwCallBack = *addr_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_neighbordelbyhwcallback_addr */

/*
 * rg set callback v6RoutingAddByHwCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_v6RoutingAddByHwCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.v6RoutingAddByHwCallBack = *addr_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_v6routingaddbyhwcallback_addr */

/*
 * rg set callback v6RoutingDelByHwCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_v6RoutingDelByHwCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.v6RoutingDelByHwCallBack = *addr_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_v6routingdelbyhwcallback_addr */

/*
 * rg set callback softwareNaptInfoAddCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_softwareNaptInfoAddCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.softwareNaptInfoAddCallBack = *addr_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_softwarenaptinfoaddcallback_addr */

/*
 * rg set callback softwareNaptInfoDeleteCallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_softwareNaptInfoDeleteCallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.softwareNaptInfoDeleteCallBack = *addr_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_softwarenaptinfodeletecallback_addr */

/*
 * rg set callback naptPreRouteDPICallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_naptPreRouteDPICallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.naptPreRouteDPICallBack= *addr_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_naptpreroutedpicallback_addr */

/*
 * rg set callback naptForwardDPICallBack <HEX:addr>
 */
cparser_result_t
cparser_cmd_rg_set_callback_naptForwardDPICallBack_addr(
    cparser_context_t *context,
    uint32_t  *addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	init_param.naptForwardDPICallBack= *addr_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_callback_naptforwarddpicallback_addr */

/*
 * rg init callback igmpSnoopingEnable <UINT:igmpSnoopingEnable> macBasedTagDecision <UINT:macBasedTagDecision> wanPortGponMode <UINT:wanPortGponMode>
 */
cparser_result_t
cparser_cmd_rg_init_callback_igmpSnoopingEnable_igmpSnoopingEnable_macBasedTagDecision_macBasedTagDecision_wanPortGponMode_wanPortGponMode(
    cparser_context_t *context,
    uint32_t  *igmpSnoopingEnable_ptr,
    uint32_t  *macBasedTagDecision_ptr,
    uint32_t  *wanPortGponMode_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	init_param.igmpSnoopingEnable=*igmpSnoopingEnable_ptr;
	init_param.macBasedTagDecision=*macBasedTagDecision_ptr;
	init_param.wanPortGponMode=*wanPortGponMode_ptr;
	init_param.ivlMulticastSupport=0;
	ret = rtk_rg_initParam_set(&init_param);
	if(ret!=RT_ERR_RG_OK)
		return CPARSER_NOT_OK;

    return CPARSER_OK;

}    /* end of cparser_cmd_rg_init_callback_igmpsnoopingenable_igmpsnoopingenable_macbasedtagdecision_macbasedtagdecision_wanportgponmode_wanportgponmode */

/*
 * rg init callback igmpSnoopingEnable <UINT:igmpSnoopingEnable> macBasedTagDecision <UINT:macBasedTagDecision> wanPortGponMode <UINT:wanPortGponMode> ivlMulticastSupport <UINT:ivlMulticastSupport>
 */
cparser_result_t
cparser_cmd_rg_init_callback_igmpSnoopingEnable_igmpSnoopingEnable_macBasedTagDecision_macBasedTagDecision_wanPortGponMode_wanPortGponMode_ivlMulticastSupport_ivlMulticastSupport(
    cparser_context_t *context,
    uint32_t  *igmpSnoopingEnable_ptr,
    uint32_t  *macBasedTagDecision_ptr,
    uint32_t  *wanPortGponMode_ptr,
    uint32_t  *ivlMulticastSupport_ptr)
{
    int ret;
    DIAG_UTIL_PARAM_CHK();
	init_param.igmpSnoopingEnable=*igmpSnoopingEnable_ptr;
	init_param.macBasedTagDecision=*macBasedTagDecision_ptr;
	init_param.wanPortGponMode=*wanPortGponMode_ptr;
	init_param.ivlMulticastSupport=*ivlMulticastSupport_ptr;
	ret = rtk_rg_initParam_set(&init_param);
	if(ret!=RT_ERR_RG_OK)
		return CPARSER_NOT_OK;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_init_callback_igmpsnoopingenable_igmpsnoopingenable_macbasedtagdecision_macbasedtagdecision_wanportgponmode_wanportgponmode_ivlmulticastsupport_ivlmulticastsupport */


/*
 * rg init callback default igmpSnoopingEnable <UINT:igmpSnoopingEnable> macBasedTagDecision <UINT:macBasedTagDecision> wanPortGponMode <UINT:wanPortGponMode>
 */
cparser_result_t
cparser_cmd_rg_init_callback_default_igmpSnoopingEnable_igmpSnoopingEnable_macBasedTagDecision_macBasedTagDecision_wanPortGponMode_wanPortGponMode(
    cparser_context_t *context,
    uint32_t  *igmpSnoopingEnable_ptr,
    uint32_t  *macBasedTagDecision_ptr,
    uint32_t  *wanPortGponMode_ptr)
{
 	int ret;
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&init_param, 0x0, sizeof(rtk_rg_initParams_t)); 
	init_param.igmpSnoopingEnable=*igmpSnoopingEnable_ptr; //default enable
	init_param.macBasedTagDecision=*macBasedTagDecision_ptr;//default disable
	init_param.wanPortGponMode=*wanPortGponMode_ptr;//default disable
	init_param.initByHwCallBack = 0xfffffffe;
	ret = rtk_rg_initParam_set(&init_param);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("rg init failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_init_callback_default_igmpsnoopingenable_igmpsnoopingenable_macbasedtagdecision_macbasedtagdecision_wanportgponmode_wanportgponmode */

/*
 * rg init callback default igmpSnoopingEnable <UINT:igmpSnoopingEnable> macBasedTagDecision <UINT:macBasedTagDecision> wanPortGponMode <UINT:wanPortGponMode> ivlMulticastSupport <UINT:ivlMulticastSupport>
 */
cparser_result_t
cparser_cmd_rg_init_callback_default_igmpSnoopingEnable_igmpSnoopingEnable_macBasedTagDecision_macBasedTagDecision_wanPortGponMode_wanPortGponMode_ivlMulticastSupport_ivlMulticastSupport(
    cparser_context_t *context,
    uint32_t  *igmpSnoopingEnable_ptr,
    uint32_t  *macBasedTagDecision_ptr,
    uint32_t  *wanPortGponMode_ptr,
    uint32_t  *ivlMulticastSupport_ptr)
{
    int ret;
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&init_param, 0x0, sizeof(rtk_rg_initParams_t)); 
	init_param.igmpSnoopingEnable=*igmpSnoopingEnable_ptr; //default enable
	init_param.macBasedTagDecision=*macBasedTagDecision_ptr;//default disable
	init_param.wanPortGponMode=*wanPortGponMode_ptr;//default disable
	init_param.ivlMulticastSupport=*ivlMulticastSupport_ptr;
	init_param.initByHwCallBack = 0xfffffffe;
	ret = rtk_rg_initParam_set(&init_param);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("rg init failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_init_callback_default_igmpsnoopingenable_igmpsnoopingenable_macbasedtagdecision_macbasedtagdecision_wanportgponmode_wanportgponmode_ivlmulticastsupport_ivlmulticastsupport */


/*
 * rg init callback default igmpSnoopingEnable <UINT:igmpSnoopingEnable> macBasedTagDecision <UINT:macBasedTagDecision> wanPortGponMode <UINT:wanPortGponMode> fwdVlan_CPU <UINT:fwdVlan_CPU> fwdVlan_PROTO_Block <UINT:fwdVlan_PROTO_Block>
 */
cparser_result_t
cparser_cmd_rg_init_callback_default_igmpSnoopingEnable_igmpSnoopingEnable_macBasedTagDecision_macBasedTagDecision_wanPortGponMode_wanPortGponMode_fwdVlan_CPU_fwdVlan_CPU_fwdVlan_Proto_Block_fwdVlan_Proto_Block(
    cparser_context_t *context,
    uint32_t  *igmpSnoopingEnable_ptr,
    uint32_t  *macBasedTagDecision_ptr,
    uint32_t  *wanPortGponMode_ptr,
    uint32_t  *fwdVlan_CPU_ptr,
    uint32_t  *fwdVlan_Proto_Block_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&init_param, 0x0, sizeof(rtk_rg_initParams_t)); 
	init_param.igmpSnoopingEnable=*igmpSnoopingEnable_ptr; //default enable
	init_param.macBasedTagDecision=*macBasedTagDecision_ptr;//default disable
	init_param.wanPortGponMode=*wanPortGponMode_ptr;//default disable
	init_param.initByHwCallBack = 0xfffffffe;
	init_param.fwdVLAN_CPU=*fwdVlan_CPU_ptr;
	init_param.fwdVLAN_Proto_Block=*fwdVlan_Proto_Block_ptr;
	ret = rtk_rg_initParam_set(&init_param);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("rg init failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	return CPARSER_OK;
}    /* end of cparser_cmd_rg_init_callback_default_igmpsnoopingenable_igmpsnoopingenable_macbasedtagdecision_macbasedtagdecision_wanportgponmode_wanportgponmode_fwdvlan_cpu_fwdvlan_cpu_fwdvlan_proto_block_fwdvlan_proto_block */

/*
 * rg init callback default igmpSnoopingEnable <UINT:igmpSnoopingEnable> macBasedTagDecision <UINT:macBasedTagDecision> wanPortGponMode <UINT:wanPortGponMode> ivlMulticastSupport <UINT:ivlMulticastSupport> fwdVlan_CPU <UINT:fwdVlan_CPU> fwdVlan_Proto_Block <UINT:fwdVlan_Proto_Block>
 */
cparser_result_t
cparser_cmd_rg_init_callback_default_igmpSnoopingEnable_igmpSnoopingEnable_macBasedTagDecision_macBasedTagDecision_wanPortGponMode_wanPortGponMode_ivlMulticastSupport_ivlMulticastSupport_fwdVlan_CPU_fwdVlan_CPU_fwdVlan_Proto_Block_fwdVlan_Proto_Block(
    cparser_context_t *context,
    uint32_t  *igmpSnoopingEnable_ptr,
    uint32_t  *macBasedTagDecision_ptr,
    uint32_t  *wanPortGponMode_ptr,
    uint32_t  *ivlMulticastSupport_ptr,
    uint32_t  *fwdVlan_CPU_ptr,
    uint32_t  *fwdVlan_Proto_Block_ptr)
{
    int ret;
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&init_param, 0x0, sizeof(rtk_rg_initParams_t)); 
	init_param.igmpSnoopingEnable=*igmpSnoopingEnable_ptr; //default enable
	init_param.macBasedTagDecision=*macBasedTagDecision_ptr;//default disable
	init_param.wanPortGponMode=*wanPortGponMode_ptr;//default disable
	init_param.ivlMulticastSupport=*ivlMulticastSupport_ptr;
	init_param.initByHwCallBack = 0xfffffffe;
	init_param.fwdVLAN_CPU=*fwdVlan_CPU_ptr;
	init_param.fwdVLAN_Proto_Block=*fwdVlan_Proto_Block_ptr;
	ret = rtk_rg_initParam_set(&init_param);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("rg init failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	return CPARSER_OK;
}    /* end of cparser_cmd_rg_init_callback_default_igmpsnoopingenable_igmpsnoopingenable_macbasedtagdecision_macbasedtagdecision_wanportgponmode_wanportgponmode_ivlmulticastsupport_ivlmulticastsupport_fwdvlan_cpu_fwdvlan_cpu_fwdvlan_proto_block_fwdvlan_proto_block */

/*
 * rg init callback default igmpSnoopingEnable <UINT:igmpSnoopingEnable> macBasedTagDecision <UINT:macBasedTagDecision> wanPortGponMode <UINT:wanPortGponMode> fwdVlan_CPU <UINT:fwdVlan_CPU> fwdVlan_CPU_SVLAN <UINT:fwdVlan_CPU_SVLAN> fwdVlan_Proto_Block <UINT:fwdVlan_Proto_Block>
 */
cparser_result_t
cparser_cmd_rg_init_callback_default_igmpSnoopingEnable_igmpSnoopingEnable_macBasedTagDecision_macBasedTagDecision_wanPortGponMode_wanPortGponMode_fwdVlan_CPU_fwdVlan_CPU_fwdVlan_CPU_SVLAN_fwdVlan_CPU_SVLAN_fwdVlan_Proto_Block_fwdVlan_Proto_Block(
    cparser_context_t *context,
    uint32_t  *igmpSnoopingEnable_ptr,
    uint32_t  *macBasedTagDecision_ptr,
    uint32_t  *wanPortGponMode_ptr,
    uint32_t  *fwdVlan_CPU_ptr,
    uint32_t  *fwdVlan_CPU_SVLAN_ptr,
    uint32_t  *fwdVlan_Proto_Block_ptr)
{
#if defined(CONFIG_RTL9600_SERIES)
	return CPARSER_NOT_OK;
#else
	int ret;
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&init_param, 0x0, sizeof(rtk_rg_initParams_t)); 
	init_param.igmpSnoopingEnable=*igmpSnoopingEnable_ptr; //default enable
	init_param.macBasedTagDecision=*macBasedTagDecision_ptr;//default disable
	init_param.wanPortGponMode=*wanPortGponMode_ptr;//default disable
	init_param.initByHwCallBack = 0xfffffffe;
	init_param.fwdVLAN_CPU=*fwdVlan_CPU_ptr;
	init_param.fwdVLAN_CPU_SVLAN=*fwdVlan_CPU_SVLAN_ptr;
	init_param.fwdVLAN_Proto_Block=*fwdVlan_Proto_Block_ptr;
	ret = rtk_rg_initParam_set(&init_param);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("rg init failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	return CPARSER_OK;
#endif
}    /* end of cparser_cmd_rg_init_callback_default_igmpsnoopingenable_igmpsnoopingenable_macbasedtagdecision_macbasedtagdecision_wanportgponmode_wanportgponmode_fwdvlan_cpu_fwdvlan_cpu_fwdvlan_cpu_svlan_fwdvlan_cpu_svlan_fwdvlan_proto_block_fwdvlan_proto_block */

/*
 * rg init callback default igmpSnoopingEnable <UINT:igmpSnoopingEnable> macBasedTagDecision <UINT:macBasedTagDecision> wanPortGponMode <UINT:wanPortGponMode> ivlMulticastSupport <UINT:ivlMulticastSupport> fwdVlan_CPU <UINT:fwdVlan_CPU> fwdVlan_CPU_SVLAN <UINT:fwdVlan_CPU_SVLAN> fwdVlan_Proto_Block <UINT:fwdVlan_Proto_Block>
 */
cparser_result_t
cparser_cmd_rg_init_callback_default_igmpSnoopingEnable_igmpSnoopingEnable_macBasedTagDecision_macBasedTagDecision_wanPortGponMode_wanPortGponMode_ivlMulticastSupport_ivlMulticastSupport_fwdVlan_CPU_fwdVlan_CPU_fwdVlan_CPU_SVLAN_fwdVlan_CPU_SVLAN_fwdVlan_Proto_Block_fwdVlan_Proto_Block(
    cparser_context_t *context,
    uint32_t  *igmpSnoopingEnable_ptr,
    uint32_t  *macBasedTagDecision_ptr,
    uint32_t  *wanPortGponMode_ptr,
    uint32_t  *ivlMulticastSupport_ptr,
    uint32_t  *fwdVlan_CPU_ptr,
    uint32_t  *fwdVlan_CPU_SVLAN_ptr,
    uint32_t  *fwdVlan_Proto_Block_ptr)
{
#if defined(CONFIG_RTL9600_SERIES)
	return CPARSER_NOT_OK;
#else
    int ret;
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&init_param, 0x0, sizeof(rtk_rg_initParams_t)); 
	init_param.igmpSnoopingEnable=*igmpSnoopingEnable_ptr; //default enable
	init_param.macBasedTagDecision=*macBasedTagDecision_ptr;//default disable
	init_param.wanPortGponMode=*wanPortGponMode_ptr;//default disable
	init_param.ivlMulticastSupport=*ivlMulticastSupport_ptr;
	init_param.initByHwCallBack = 0xfffffffe;
	init_param.fwdVLAN_CPU=*fwdVlan_CPU_ptr;
	init_param.fwdVLAN_CPU_SVLAN=*fwdVlan_CPU_SVLAN_ptr;
	init_param.fwdVLAN_Proto_Block=*fwdVlan_Proto_Block_ptr;
	ret = rtk_rg_initParam_set(&init_param);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("rg init failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	return CPARSER_OK;
#endif
}    /* end of cparser_cmd_rg_init_callback_default_igmpsnoopingenable_igmpsnoopingenable_macbasedtagdecision_macbasedtagdecision_wanportgponmode_wanportgponmode_ivlmulticastsupport_ivlmulticastsupport_fwdvlan_cpu_fwdvlan_cpu_fwdvlan_cpu_svlan_fwdvlan_cpu_svlan_fwdvlan_proto_block_fwdvlan_proto_block */


#if 0
//rg configuration//init configuration//callback configuration//default configuration//igmpSnoopingEnable configuration//igmpSnoopingEnable//macBasedTagDecision configuration//macBasedTagDecision//wanPortGponMode configuration//wanPortGponMode//fwdVlan_CPU configuration//fwdVlan_CPU, 0:default value//fwdVlan_Proto_Block configuration//fwdVlan_Proto_Block, 0:defaule valuerg init callback default igmpSnoopingEnable <UINT:igmpSnoopingEnable> macBasedTagDecision <UINT:macBasedTagDecision> wanPortGponMode <UINT:wanPortGponMode> fwdVlan_CPU <UINT:fwdVlan_CPU> fwdVlan_Proto_Block <UINT:fwdVlan_Proto_Block>

/*
 * rg init callback default igmpSnoopingEnable <UINT:igmpSnoopingEnable> macBasedTagDecision <UINT:macBasedTagDecision> wanPortGponMode <UINT:wanPortGponMode> ivlMulticastSu
pport <UINT:ivlMulticastSupport> fwdVlan_CPU <UINT:fwdVlan_CPU> fwdVlan_Proto_Block <UINT:fwdVlan_Proto_Block> cf_pattern0_size <UINT:cf_pattern0_size> cf_l2_wan_default_idx
 <UINT:cf_l2_wan_default_idx> cf_mc_wan_default_idx <UINT:cf_mc_wan_default_idx>
 */
cparser_result_t
cparser_cmd_rg_init_callback_default_igmpSnoopingEnable_igmpSnoopingEnable_macBasedTagDecision_macBasedTagDecision_wanPortGponMode_wanPortGponMode_ivlMulticastSupport_ivlMulticastSupport_fwdVlan_CPU_fwdVlan_CPU_fwdVlan_Proto_Block_fwdVlan_Proto_Block_cf_pattern0_size_cf_pattern0_size_cf_l2_wan_default_idx_cf_l2_wan_default_idx_cf_mc_wan_default_idx_cf_mc_wan_default_idx(
    cparser_context_t *context,
    uint32_t  *igmpSnoopingEnable_ptr,
    uint32_t  *macBasedTagDecision_ptr,
    uint32_t  *wanPortGponMode_ptr,
    uint32_t  *ivlMulticastSupport_ptr,
    uint32_t  *fwdVlan_CPU_ptr,
    uint32_t  *fwdVlan_Proto_Block_ptr,
    uint32_t  *cf_pattern0_size_ptr,
    uint32_t  *cf_l2_wan_default_idx_ptr,
    uint32_t  *cf_mc_wan_default_idx_ptr)
{
#if defined(CONFIG_RTL9602C_SERIES)
    DIAG_UTIL_PARAM_CHK();
    int ret;
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&init_param, 0x0, sizeof(rtk_rg_initParams_t)); 
	init_param.igmpSnoopingEnable=*igmpSnoopingEnable_ptr; //default enable
	init_param.macBasedTagDecision=*macBasedTagDecision_ptr;//default disable
	init_param.wanPortGponMode=*wanPortGponMode_ptr;//default disable
	init_param.ivlMulticastSupport=*ivlMulticastSupport_ptr;
	init_param.initByHwCallBack = 0xfffffffe;
	init_param.fwdVLAN_CPU=*fwdVlan_CPU_ptr;
	init_param.fwdVLAN_Proto_Block=*fwdVlan_Proto_Block_ptr;
	init_param.cf_pattern0_size=*cf_pattern0_size_ptr;
	init_param.mib_l2_wanif_idx=*cf_l2_wan_default_idx_ptr;
	init_param.mib_mc_wanif_idx=*cf_mc_wan_default_idx_ptr;
	ret = rtk_rg_initParam_set(&init_param);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("rg init failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
#else
		diag_util_mprintf("[WARNING] chip not support this command\n");
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_init_callback_default_igmpsnoopingenable_igmpsnoopingenable_macbasedtagdecision_macbasedtagdecision_wanportgponmode_wanportgponmode_ivlmulticastsupport_ivlmulticastsupport_fwdvlan_cpu_fwdvlan_cpu_fwdvlan_proto_block_fwdvlan_proto_block_cf_pattern0_size_cf_pattern0_size_cf_l2_wan_default_idx_cf_l2_wan_default_idx_cf_mc_wan_default_idx_cf_mc_wan_default_idx */



//rg configuration//init configuration//callback configuration//default configuration//igmpSnoopingEnable configuration//igmpSnoopingEnable//macBasedTagDecision configuration//macBasedTagDecision//wanPortGponMode configuration//wanPortGponMode//ivlMulticastSupport configuration//ivlMulticastSupport, 0:disable 1:enable//fwdVlan_CPU configuration//fwdVlan_CPU, 0:default value//fwdVlan_Proto_Block configuration//fwdVlan_Proto_Block, 0:defaule valuerg init callback default igmpSnoopingEnable <UINT:igmpSnoopingEnable> macBasedTagDecision <UINT:macBasedTagDecision> wanPortGponMode <UINT:wanPortGponMode> ivlMulticastSupport <UINT:ivlMulticastSupport> fwdVlan_CPU <UINT:fwdVlan_CPU> fwdVlan_Proto_Block <UINT:fwdVlan_Proto_Block>

/*
 * rg init igmpSnoopingEnable <UINT:igmpSnoopingEnable> macBasedTagDecision <UINT:macBasedTagDecision> wanPortGponMode <UINT:wanPortGponMode> ivlMulticastSupport <UINT:ivlMulticastSupport> fwdVlan_CPU <UINT:fwdVlan_CPU> fwdVlan_Proto_Block <UINT:fwdVlan_Proto_Block> cf_pattern0_size <UINT:cf_pattern0_size> cf_l2_wan_default_idx <UINT:cf_l2_wan_default_idx> cf_mc_wan_default_idx <UINT:cf_mc_wan_default_idx>
 */
cparser_result_t
cparser_cmd_rg_init_igmpSnoopingEnable_igmpSnoopingEnable_macBasedTagDecision_macBasedTagDecision_wanPortGponMode_wanPortGponMode_ivlMulticastSupport_ivlMulticastSupport_fwdVlan_CPU_fwdVlan_CPU_fwdVlan_Proto_Block_fwdVlan_Proto_Block_cf_pattern0_size_cf_pattern0_size_cf_l2_wan_default_idx_cf_l2_wan_default_idx_cf_mc_wan_default_idx_cf_mc_wan_default_idx(
    cparser_context_t *context,
    uint32_t  *igmpSnoopingEnable_ptr,
    uint32_t  *macBasedTagDecision_ptr,
    uint32_t  *wanPortGponMode_ptr,
    uint32_t  *ivlMulticastSupport_ptr,
    uint32_t  *fwdVlan_CPU_ptr,
    uint32_t  *fwdVlan_Proto_Block_ptr,
    uint32_t  *cf_pattern0_size_ptr,
    uint32_t  *cf_l2_wan_default_idx_ptr,
    uint32_t  *cf_mc_wan_default_idx_ptr)
{
#if defined(CONFIG_RTL9602C_SERIES)
    DIAG_UTIL_PARAM_CHK();
    int ret;
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&init_param, 0x0, sizeof(rtk_rg_initParams_t)); 
	init_param.igmpSnoopingEnable=*igmpSnoopingEnable_ptr; //default enable
	init_param.macBasedTagDecision=*macBasedTagDecision_ptr;//default disable
	init_param.wanPortGponMode=*wanPortGponMode_ptr;//default disable
	init_param.ivlMulticastSupport=*ivlMulticastSupport_ptr;
	init_param.fwdVLAN_CPU=*fwdVlan_CPU_ptr;
	init_param.fwdVLAN_Proto_Block=*fwdVlan_Proto_Block_ptr;
	init_param.cf_pattern0_size=*cf_pattern0_size_ptr;
	init_param.mib_l2_wanif_idx=*cf_l2_wan_default_idx_ptr;
	init_param.mib_mc_wanif_idx=*cf_mc_wan_default_idx_ptr;
	ret = rtk_rg_initParam_set(&init_param);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("rg init failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
#else
	diag_util_mprintf("[WARNING] chip not support this command\n");
#endif
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_init_igmpsnoopingenable_igmpsnoopingenable_macbasedtagdecision_macbasedtagdecision_wanportgponmode_wanportgponmode_ivlmulticastsupport_ivlmulticastsupport_fwdvlan_cpu_fwdvlan_cpu_fwdvlan_proto_block_fwdvlan_proto_block_cf_pattern0_size_cf_pattern0_size_cf_l2_wan_default_idx_cf_l2_wan_default_idx_cf_mc_wan_default_idx_cf_mc_wan_default_idx */

#endif


/*
 * rg add algApps app_mask <HEX:appmask>
 */
cparser_result_t
cparser_cmd_rg_add_algApps_app_mask_appmask(
    cparser_context_t *context,
    uint32_t  *appmask_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	alg_app = *appmask_ptr;
	ret = rtk_rg_algApps_set(alg_app);
	if(ret!=RT_ERR_RG_OK)
		return CPARSER_NOT_OK;

    return CPARSER_OK;

}    /* end of cparser_cmd_rg_add_algapps_app_mask_appmask */

/*
 * rg get algApps
 */
cparser_result_t
cparser_cmd_rg_get_algApps(
    cparser_context_t *context)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
	ret = rtk_rg_algApps_get(&alg_app);
	if(ret!=RT_ERR_RG_OK)
		return CPARSER_NOT_OK;

    _diag_algAppShow(&alg_app);
	diag_util_mprintf("alg_app=0x%x\n",alg_app);
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_algapps */


/*
 * rg set serverInLanAppsIpAddr algType <UINT:algType> serverAddress <IPV4ADDR:serverAddress>
 */
cparser_result_t
cparser_cmd_rg_set_serverInLanAppsIpAddr_algType_algType_serverAddress_serverAddress(
    cparser_context_t *context,
    uint32_t  *algType_ptr,
    uint32_t  *serverAddress_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	srvIpMapping.algType = *algType_ptr;
	srvIpMapping.serverAddress = *serverAddress_ptr;
		
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_serverinlanappsipaddr_algtype_algtype_serveraddress_serveraddress */

/*
 * rg add serverInLanAppsIpAddr entry
 */
cparser_result_t
cparser_cmd_rg_add_serverInLanAppsIpAddr_entry(
    cparser_context_t *context)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	ret= rtk_rg_algServerInLanAppsIpAddr_add(&srvIpMapping);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add serverInLanAppsIpAddr failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add serverInLanAppsIpAddr success.\n");
	}
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_serverinlanappsipaddr_entry */

/*
 * rg del serverInLanAppsIpAddr algType <UINT:algType>
 */
cparser_result_t
cparser_cmd_rg_del_serverInLanAppsIpAddr_algType_algType(
    cparser_context_t *context,
    uint32_t  *algType_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	ret= rtk_rg_algServerInLanAppsIpAddr_del(*algType_ptr);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("del serverInLanAppsIpAddr failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("del serverInLanAppsIpAddr for algType[%x] success.\n",*algType_ptr);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_del_serverinlanappsipaddr_algtype_algtype */




/*
 * rg show pppoeClientInfoAfterDial
 */
cparser_result_t
cparser_cmd_rg_show_pppoeClientInfoAfterDial(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	_diag_pppoeClientInfoShow(&clientPppoe_info);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_pppoeclientinfoafterdial */

/*
 * rg add pppoeClientInfoAfterDial wan_intf_idx <UINT:wan_intf_idx>
 */
cparser_result_t
cparser_cmd_rg_add_pppoeClientInfoAfterDial_wan_intf_idx_wan_intf_idx(
    cparser_context_t *context,
    uint32_t  *wan_intf_idx_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_pppoeClientInfoAfterDial_set(*wan_intf_idx_ptr,&clientPppoe_info);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add wan-intf-pppoeClient-after-info failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{	
		diag_util_mprintf("add pppoe after-info to interface[%d] success. \n",*wan_intf_idx_ptr);
	}
	return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_pppoeclientinfoafterdial_wan_intf_idx_wan_intf_idx */

/*
 * rg clear pppoeClientInfoAfterDial
 */
cparser_result_t
cparser_cmd_rg_clear_pppoeClientInfoAfterDial(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&clientPppoe_info, 0x0, sizeof(rtk_rg_pppoeClientInfoAfterDial_t)); 

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_pppoeclientinfoafterdial */

/*
 * rg set pppoeClientInfoAfterDial sessionId <UINT:sessionId> ip-version <UINT:ip_version> napt_enable <UINT:napt_enable> ip_addr <IPV4ADDR:ip_addr> ip_network_mask <IPV4ADDR:ip_network_mask> ipv4_default_gateway_on <UINT:ipv4_default_gateway_on> gateway_ipv4_addr <IPV4ADDR:gateway_ipv4_addr> mtu <UINT:mtu>
 */
cparser_result_t
cparser_cmd_rg_set_pppoeClientInfoAfterDial_sessionId_sessionId_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu(
    cparser_context_t *context,
    uint32_t  *sessionId_ptr,
    uint32_t  *ip_version_ptr,
    uint32_t  *napt_enable_ptr,
    uint32_t  *ip_addr_ptr,
    uint32_t  *ip_network_mask_ptr,
    uint32_t  *ipv4_default_gateway_on_ptr,
    uint32_t  *gateway_ipv4_addr_ptr,
    uint32_t  *mtu_ptr)
{
	int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
	clientPppoe_info.sessionId = *sessionId_ptr;
	clientPppoe_info.hw_info.ip_version=*ip_version_ptr;
	clientPppoe_info.hw_info.napt_enable=*napt_enable_ptr;
	clientPppoe_info.hw_info.ip_addr=*ip_addr_ptr;
	clientPppoe_info.hw_info.ip_network_mask=*ip_network_mask_ptr;

	clientPppoe_info.hw_info.ipv4_default_gateway_on=*ipv4_default_gateway_on_ptr;
    clientPppoe_info.hw_info.gateway_ipv4_addr=*gateway_ipv4_addr_ptr;
	clientPppoe_info.hw_info.mtu=*mtu_ptr;
	
	//gw_mac_auto_learn default setting enable
	clientPppoe_info.hw_info.gw_mac_auto_learn_for_ipv4=1;
	memset(clientPppoe_info.hw_info.gateway_mac_addr_for_ipv4.octet, 0x0, ETHER_ADDR_LEN);		
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_set_pppoeclientinfoafterdial_sessionid_sessionid_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu */

/*
 * rg set pppoeClientInfoAfterDial sessionId <UINT:sessionId> ip-version <UINT:ip_version> napt_enable <UINT:napt_enable> ip_addr <IPV4ADDR:ip_addr> ip_network_mask <IPV4ADDR:ip_network_mask> ipv4_default_gateway_on <UINT:ipv4_default_gateway_on> gateway_ipv4_addr <IPV4ADDR:gateway_ipv4_addr> mtu <UINT:mtu> gw_mac_auto_learn_for_ipv4 <UINT:gw_mac_auto_learn_for_ipv4> gateway_mac_addr_for_ipv4 <MACADDR:gateway_mac_addr_for_ipv4>
 */
cparser_result_t
cparser_cmd_rg_set_pppoeClientInfoAfterDial_sessionId_sessionId_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu_gw_mac_auto_learn_for_ipv4_gw_mac_auto_learn_for_ipv4_gateway_mac_addr_for_ipv4_gateway_mac_addr_for_ipv4(
    cparser_context_t *context,
    uint32_t  *sessionId_ptr,
    uint32_t  *ip_version_ptr,
    uint32_t  *napt_enable_ptr,
    uint32_t  *ip_addr_ptr,
    uint32_t  *ip_network_mask_ptr,
    uint32_t  *ipv4_default_gateway_on_ptr,
    uint32_t  *gateway_ipv4_addr_ptr,
    uint32_t  *mtu_ptr,
    uint32_t  *gw_mac_auto_learn_for_ipv4_ptr,
    cparser_macaddr_t  *gateway_mac_addr_for_ipv4_ptr)
{
 	int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
	clientPppoe_info.sessionId = *sessionId_ptr;
	clientPppoe_info.hw_info.ip_version=*ip_version_ptr;
	clientPppoe_info.hw_info.napt_enable=*napt_enable_ptr;
	clientPppoe_info.hw_info.ip_addr=*ip_addr_ptr;
	clientPppoe_info.hw_info.ip_network_mask=*ip_network_mask_ptr;

	clientPppoe_info.hw_info.ipv4_default_gateway_on=*ipv4_default_gateway_on_ptr;
    clientPppoe_info.hw_info.gateway_ipv4_addr=*gateway_ipv4_addr_ptr;
	clientPppoe_info.hw_info.mtu=*mtu_ptr;
	
	clientPppoe_info.hw_info.gw_mac_auto_learn_for_ipv4=*gw_mac_auto_learn_for_ipv4_ptr;
	osal_memcpy(clientPppoe_info.hw_info.gateway_mac_addr_for_ipv4.octet, gateway_mac_addr_for_ipv4_ptr->octet, ETHER_ADDR_LEN);	
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_set_pppoeclientinfoafterdial_sessionid_sessionid_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu_gw_mac_auto_learn_for_ipv4_gw_mac_auto_learn_for_ipv4_gateway_mac_addr_for_ipv4_gateway_mac_addr_for_ipv4 */


/*
 * rg set pppoeClientInfoAfterDial-ipv6 ipv6_addr <IPV6ADDR:ipv6_addr> ipv6_mask_length <UINT:ipv6_mask_length> ipv6_default_gateway_on <UINT:ipv6_default_gateway_on> gateway_ipv6_addr <IPV6ADDR:gateway_ipv6_addr>
 */
cparser_result_t
cparser_cmd_rg_set_pppoeClientInfoAfterDial_ipv6_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr(
    cparser_context_t *context,
    char * *ipv6_addr_ptr,
    uint32_t  *ipv6_mask_length_ptr,
    uint32_t  *ipv6_default_gateway_on_ptr,
    char * *gateway_ipv6_addr_ptr)
{
	int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&clientPppoe_info.hw_info.ipv6_addr.ipv6_addr[0], TOKEN_STR(4)), ret);
    clientPppoe_info.hw_info.ipv6_mask_length=*ipv6_mask_length_ptr;
    clientPppoe_info.hw_info.ipv6_default_gateway_on=*ipv6_default_gateway_on_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&clientPppoe_info.hw_info.gateway_ipv6_addr.ipv6_addr[0], TOKEN_STR(10)), ret);
	
	//gw_mac_auto_learn default setting enable	
	clientPppoe_info.hw_info.gw_mac_auto_learn_for_ipv6=1;
	memset(clientPppoe_info.hw_info.gateway_mac_addr_for_ipv6.octet, 0x0, ETHER_ADDR_LEN);	
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_set_pppoeclientinfoafterdial_ipv6_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr */

/*
 * rg set pppoeClientInfoAfterDial-ipv6 ipv6_addr <IPV6ADDR:ipv6_addr> ipv6_mask_length <UINT:ipv6_mask_length> ipv6_default_gateway_on <UINT:ipv6_default_gateway_on> gateway_ipv6_addr <IPV6ADDR:gateway_ipv6_addr> gw_mac_auto_learn_for_ipv6 <UINT:gw_mac_auto_learn_for_ipv6> gateway_mac_addr_for_ipv6 <MACADDR:gateway_mac_addr_for_ipv6>
 */
cparser_result_t
cparser_cmd_rg_set_pppoeClientInfoAfterDial_ipv6_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6(
    cparser_context_t *context,
    char * *ipv6_addr_ptr,
    uint32_t  *ipv6_mask_length_ptr,
    uint32_t  *ipv6_default_gateway_on_ptr,
    char * *gateway_ipv6_addr_ptr,
    uint32_t  *gw_mac_auto_learn_for_ipv6_ptr,
    cparser_macaddr_t  *gateway_mac_addr_for_ipv6_ptr)
{
	int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&clientPppoe_info.hw_info.ipv6_addr.ipv6_addr[0], TOKEN_STR(4)), ret);
    clientPppoe_info.hw_info.ipv6_mask_length=*ipv6_mask_length_ptr;
    clientPppoe_info.hw_info.ipv6_default_gateway_on=*ipv6_default_gateway_on_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&clientPppoe_info.hw_info.gateway_ipv6_addr.ipv6_addr[0], TOKEN_STR(10)), ret);

	
	clientPppoe_info.hw_info.gw_mac_auto_learn_for_ipv6=*gw_mac_auto_learn_for_ipv6_ptr;
	osal_memcpy(clientPppoe_info.hw_info.gateway_mac_addr_for_ipv6.octet, gateway_mac_addr_for_ipv6_ptr->octet, ETHER_ADDR_LEN);
	
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_set_pppoeclientinfoafterdial_ipv6_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6 */


/*
 * rg set pppoeClientInfoAfterDial-ipv6 ipv6_napt_enable <UINT:ipv6_napt_enable> ipv6_addr <IPV6ADDR:ipv6_addr> ipv6_mask_length <UINT:ipv6_mask_length> ipv6_default_gateway_on <UINT:ipv6_default_gateway_on> gateway_ipv6_addr <IPV6ADDR:gateway_ipv6_addr>
 */
cparser_result_t
cparser_cmd_rg_set_pppoeClientInfoAfterDial_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr(
    cparser_context_t *context,
    uint32_t  *ipv6_napt_enable_ptr,
    char * *ipv6_addr_ptr,
    uint32_t  *ipv6_mask_length_ptr,
    uint32_t  *ipv6_default_gateway_on_ptr,
    char * *gateway_ipv6_addr_ptr)
{
	int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
	clientPppoe_info.hw_info.ipv6_napt_enable = *ipv6_napt_enable_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&clientPppoe_info.hw_info.ipv6_addr.ipv6_addr[0], TOKEN_STR(6)), ret);
    clientPppoe_info.hw_info.ipv6_mask_length=*ipv6_mask_length_ptr;
    clientPppoe_info.hw_info.ipv6_default_gateway_on=*ipv6_default_gateway_on_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&clientPppoe_info.hw_info.gateway_ipv6_addr.ipv6_addr[0], TOKEN_STR(12)), ret);
	
	//gw_mac_auto_learn default setting enable	
	clientPppoe_info.hw_info.gw_mac_auto_learn_for_ipv6=1;
	memset(clientPppoe_info.hw_info.gateway_mac_addr_for_ipv6.octet, 0x0, ETHER_ADDR_LEN);	
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_set_pppoeclientinfoafterdial_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr */

/*
 * rg set pppoeClientInfoAfterDial-ipv6 ipv6_napt_enable <UINT:ipv6_napt_enable> ipv6_addr <IPV6ADDR:ipv6_addr> ipv6_mask_length <UINT:ipv6_mask_length> ipv6_default_gateway_on <UINT:ipv6_default_gateway_on> gateway_ipv6_addr <IPV6ADDR:gateway_ipv6_addr> gw_mac_auto_learn_for_ipv6 <UINT:gw_mac_auto_learn_for_ipv6> gateway_mac_addr_for_ipv6 <MACADDR:gateway_mac_addr_for_ipv6>
 */
cparser_result_t
cparser_cmd_rg_set_pppoeClientInfoAfterDial_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6(
    cparser_context_t *context,
    uint32_t  *ipv6_napt_enable_ptr,
    char * *ipv6_addr_ptr,
    uint32_t  *ipv6_mask_length_ptr,
    uint32_t  *ipv6_default_gateway_on_ptr,
    char * *gateway_ipv6_addr_ptr,
    uint32_t  *gw_mac_auto_learn_for_ipv6_ptr,
    cparser_macaddr_t  *gateway_mac_addr_for_ipv6_ptr)
{
	int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
	clientPppoe_info.hw_info.ipv6_napt_enable = *ipv6_napt_enable_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&clientPppoe_info.hw_info.ipv6_addr.ipv6_addr[0], TOKEN_STR(6)), ret);
    clientPppoe_info.hw_info.ipv6_mask_length=*ipv6_mask_length_ptr;
    clientPppoe_info.hw_info.ipv6_default_gateway_on=*ipv6_default_gateway_on_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&clientPppoe_info.hw_info.gateway_ipv6_addr.ipv6_addr[0], TOKEN_STR(12)), ret);

	
	clientPppoe_info.hw_info.gw_mac_auto_learn_for_ipv6=*gw_mac_auto_learn_for_ipv6_ptr;
	osal_memcpy(clientPppoe_info.hw_info.gateway_mac_addr_for_ipv6.octet, gateway_mac_addr_for_ipv6_ptr->octet, ETHER_ADDR_LEN);
	
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_set_pppoeclientinfoafterdial_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6 */

/*
 * rg show pppoeClientInfoBeforeDial
 */
cparser_result_t
cparser_cmd_rg_show_pppoeClientInfoBeforeDial(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	_diag_pppoeClientInfoBeforeDiagShow(&clientPppoe_beforeDiag_info);
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_pppoeclientinfobeforedial */

/*
 * rg add pppoeClientInfoBeforeDial wan_intf_idx <UINT:wan_intf_idx>
 */
cparser_result_t
cparser_cmd_rg_add_pppoeClientInfoBeforeDial_wan_intf_idx_wan_intf_idx(
    cparser_context_t *context,
    uint32_t  *wan_intf_idx_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_pppoeClientInfoBeforeDial_set(*wan_intf_idx_ptr, &clientPppoe_beforeDiag_info);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add wan-intf-pppoeClient-before-info failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{	
		diag_util_mprintf("add pppoe before-info to interface[%d] success. \n",*wan_intf_idx_ptr);
	}
	return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_pppoeclientinfobeforedial_wan_intf_idx_wan_intf_idx */




/*
 * rg clear pppoeClientInfoBeforeDial
 */
cparser_result_t
cparser_cmd_rg_clear_pppoeClientInfoBeforeDial(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&clientPppoe_beforeDiag_info, 0x0, sizeof(rtk_rg_pppoeClientInfoBeforeDial_t)); 

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_pppoeclientinfobeforedial */

/*
 * rg set pppoeClientInfoBeforeDial username <STRING:username> password <STRING:password> auth_type <UINT:auth_type> pppoe_proxy_enable <UINT:pppoe_proxy_enable> max_pppoe_proxy_num <UINT:max_pppoe_proxy_num> auto_reconnect <UINT:auto_reconnect> dial_on_demond <UINT:dial_on_demond> idle_timeout_secs <UINT:idle_timeout_secs> status <UINT:status> dialOnDemondCallBack <HEX:dialOnDemondCallBack_addr> idleTimeOutCallBack <HEX:idleTimeOutCallBack_addr>
 */
cparser_result_t
cparser_cmd_rg_set_pppoeClientInfoBeforeDial_username_username_password_password_auth_type_auth_type_pppoe_proxy_enable_pppoe_proxy_enable_max_pppoe_proxy_num_max_pppoe_proxy_num_auto_reconnect_auto_reconnect_dial_on_demond_dial_on_demond_idle_timeout_secs_idle_timeout_secs_status_status_dialOnDemondCallBack_dialOnDemondCallBack_addr_idleTimeOutCallBack_idleTimeOutCallBack_addr(
    cparser_context_t *context,
    char * *username_ptr,
    char * *password_ptr,
    uint32_t  *auth_type_ptr,
    uint32_t  *pppoe_proxy_enable_ptr,
    uint32_t  *max_pppoe_proxy_num_ptr,
    uint32_t  *auto_reconnect_ptr,
    uint32_t  *dial_on_demond_ptr,
    uint32_t  *idle_timeout_secs_ptr,
    uint32_t  *status_ptr,
    uint32_t  *dialOnDemondCallBack_addr_ptr,
    uint32_t  *idleTimeOutCallBack_addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	
	osal_memcpy(clientPppoe_beforeDiag_info.username, *username_ptr, 32);	
	osal_memcpy(clientPppoe_beforeDiag_info.password, *password_ptr, 32);	
	clientPppoe_beforeDiag_info.auth_type= *auth_type_ptr;
	clientPppoe_beforeDiag_info.pppoe_proxy_enable= *pppoe_proxy_enable_ptr;
	clientPppoe_beforeDiag_info.max_pppoe_proxy_num= *max_pppoe_proxy_num_ptr;
	clientPppoe_beforeDiag_info.auto_reconnect= *auto_reconnect_ptr;
	clientPppoe_beforeDiag_info.dial_on_demond= *dial_on_demond_ptr;
	clientPppoe_beforeDiag_info.idle_timeout_secs= *idle_timeout_secs_ptr;
	clientPppoe_beforeDiag_info.stauts= *status_ptr;
	if(*dialOnDemondCallBack_addr_ptr==0x0){
		clientPppoe_beforeDiag_info.dialOnDemondCallBack=NULL;
	}else{
		clientPppoe_beforeDiag_info.dialOnDemondCallBack=*dialOnDemondCallBack_addr_ptr;
	}

	if(*idleTimeOutCallBack_addr_ptr==0x0){
		clientPppoe_beforeDiag_info.idleTimeOutCallBack=NULL;
	}else{
		clientPppoe_beforeDiag_info.idleTimeOutCallBack=*idleTimeOutCallBack_addr_ptr;
	}
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_pppoeclientinfobeforedial_username_username_password_password_auth_type_auth_type_pppoe_proxy_enable_pppoe_proxy_enable_max_pppoe_proxy_num_max_pppoe_proxy_num_auto_reconnect_auto_reconnect_dial_on_demond_dial_on_demond_idle_timeout_secs_idle_timeout_secs_status_status_dialondemondcallback_dialondemondcallback_addr_idletimeoutcallback_idletimeoutcallback_addr */

/*
 * rg show pptpClientInfoBeforeDial
 */
cparser_result_t
cparser_cmd_rg_show_pptpClientInfoBeforeDial(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	_diag_pptpClientInfoBeforeDialShow(&clientPptp_beforeDial_info);
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_pptpclientinfobeforedial */

/*
 * rg add pptpClientInfoBeforeDial wan_intf_idx <UINT:wan_intf_idx>
 */
cparser_result_t
cparser_cmd_rg_add_pptpClientInfoBeforeDial_wan_intf_idx_wan_intf_idx(
    cparser_context_t *context,
    uint32_t  *wan_intf_idx_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_pptpClientInfoBeforeDial_set(*wan_intf_idx_ptr,&clientPptp_beforeDial_info);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add wan-intf-pptpClient-before-info failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{	
		diag_util_mprintf("add pptp before-info to interface[%d] success. \n",*wan_intf_idx_ptr);
	}
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_pptpclientinfobeforedial_wan_intf_idx_wan_intf_idx */

/*
 * rg clear pptpClientInfoBeforeDial
 */
cparser_result_t
cparser_cmd_rg_clear_pptpClientInfoBeforeDial(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&clientPptp_beforeDial_info, 0x0, sizeof(rtk_rg_pptpClientInfoBeforeDial_t));
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_pptpclientinfobeforedial */

/*
 * rg set pptpClientInfoBeforeDial username <STRING:username> password <STRING:password> pptp_server_addr_for_ipv4 <IPV4ADDR:pptp_server_addr_for_ipv4>
 */
cparser_result_t
cparser_cmd_rg_set_pptpClientInfoBeforeDial_username_username_password_password_pptp_server_addr_for_ipv4_pptp_server_addr_for_ipv4(
    cparser_context_t *context,
    char * *username_ptr,
    char * *password_ptr,
    uint32_t  *pptp_server_addr_for_ipv4_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memcpy(clientPptp_beforeDial_info.username, *username_ptr, 32);	
	osal_memcpy(clientPptp_beforeDial_info.password, *password_ptr, 32);
	clientPptp_beforeDial_info.pptp_ipv4_addr=*pptp_server_addr_for_ipv4_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_pptpclientinfobeforedial_username_username_password_password_pptp_server_addr_for_ipv4_pptp_server_addr_for_ipv4 */

/*
 * rg show pptpClientInfoAfterDial
 */
cparser_result_t
cparser_cmd_rg_show_pptpClientInfoAfterDial(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	_diag_pptpClientInfoShow(&clientPptp_info);
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_pptpclientinfoafterdial */

/*
 * rg add pptpClientInfoAfterDial wan_intf_idx <UINT:wan_intf_idx>
 */
cparser_result_t
cparser_cmd_rg_add_pptpClientInfoAfterDial_wan_intf_idx_wan_intf_idx(
    cparser_context_t *context,
    uint32_t  *wan_intf_idx_ptr)
{
    int ret;
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_pptpClientInfoAfterDial_set(*wan_intf_idx_ptr,&clientPptp_info);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add wan-intf-pptpClient-after-info failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{	
		diag_util_mprintf("add pptp after-info to interface[%d] success. \n",*wan_intf_idx_ptr);
	}
	return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_pptpclientinfoafterdial_wan_intf_idx_wan_intf_idx */

/*
 * rg clear pptpClientInfoAfterDial
 */
cparser_result_t
cparser_cmd_rg_clear_pptpClientInfoAfterDial(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&clientPptp_info, 0x0, sizeof(rtk_rg_pptpClientInfoAfterDial_t)); 
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_pptpclientinfoafterdial */

/*
 * rg set pptpClientInfoAfterDial callId <UINT:callId> gateway_callId <UINT:gateway_callId> ipv4_addr <IPV4ADDR:ipv4_addr> ipv4_network_mask <IPV4ADDR:ipv4_network_mask> gateway_ipv4_addr <IPV4ADDR:gateway_ipv4_addr> mtu <UINT:mtu> ipv4_napt_enable <UINT:ipv4_napt_enable> ipv4_default_gateway_on <UINT:ipv4_default_gateway_on> gw_mac_auto_learn_for_ipv4 <UINT:gw_mac_auto_learn_for_ipv4> gateway_mac_addr_for_ipv4 <MACADDR:gateway_mac_addr_for_ipv4>
 */
cparser_result_t
cparser_cmd_rg_set_pptpClientInfoAfterDial_callId_callId_gateway_callId_gateway_callId_ipv4_addr_ipv4_addr_ipv4_network_mask_ipv4_network_mask_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu_ipv4_napt_enable_ipv4_napt_enable_ipv4_default_gateway_on_ipv4_default_gateway_on_gw_mac_auto_learn_for_ipv4_gw_mac_auto_learn_for_ipv4_gateway_mac_addr_for_ipv4_gateway_mac_addr_for_ipv4(
    cparser_context_t *context,
    uint32_t  *callId_ptr,
    uint32_t  *gateway_callId_ptr,
    uint32_t  *ipv4_addr_ptr,
    uint32_t  *ipv4_network_mask_ptr,
    uint32_t  *gateway_ipv4_addr_ptr,
    uint32_t  *mtu_ptr,
    uint32_t  *ipv4_napt_enable_ptr,
    uint32_t  *ipv4_default_gateway_on_ptr,
    uint32_t  *gw_mac_auto_learn_for_ipv4_ptr,
    cparser_macaddr_t  *gateway_mac_addr_for_ipv4_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	bzero(&clientPptp_info.hw_info,sizeof(rtk_rg_ipStaticInfo_t));
	clientPptp_info.hw_info.ip_version=IPVER_V4ONLY;
	clientPptp_info.hw_info.napt_enable=*ipv4_napt_enable_ptr;
	clientPptp_info.hw_info.ipv4_default_gateway_on=*ipv4_default_gateway_on_ptr;
	clientPptp_info.hw_info.ip_addr=*ipv4_addr_ptr;
	clientPptp_info.hw_info.ip_network_mask=*ipv4_network_mask_ptr;
	clientPptp_info.hw_info.gateway_ipv4_addr=*gateway_ipv4_addr_ptr;
	clientPptp_info.hw_info.mtu=*mtu_ptr;
	clientPptp_info.hw_info.gw_mac_auto_learn_for_ipv4=*gw_mac_auto_learn_for_ipv4_ptr;
	osal_memcpy(clientPptp_info.hw_info.gateway_mac_addr_for_ipv4.octet, gateway_mac_addr_for_ipv4_ptr->octet, ETHER_ADDR_LEN);	

	clientPptp_info.callId=*callId_ptr;
	clientPptp_info.gateway_callId=*gateway_callId_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_pptpclientinfoafterdial_callid_callid_gateway_callid_gateway_callid_ipv4_addr_ipv4_addr_ipv4_network_mask_ipv4_network_mask_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu_ipv4_napt_enable_ipv4_napt_enable_ipv4_default_gateway_on_ipv4_default_gateway_on_gw_mac_auto_learn_for_ipv4_gw_mac_auto_learn_for_ipv4_gateway_mac_addr_for_ipv4_gateway_mac_addr_for_ipv4 */


/*
 * rg show l2tpClientInfoBeforeDial
 */
cparser_result_t
cparser_cmd_rg_show_l2tpClientInfoBeforeDial(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	_diag_l2tpClientInfoBeforeDialShow(&clientL2tp_beforeDial_info);
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_l2tpclientinfobeforedial */

/*
 * rg add l2tpClientInfoBeforeDial wan_intf_idx <UINT:wan_intf_idx>
 */
cparser_result_t
cparser_cmd_rg_add_l2tpClientInfoBeforeDial_wan_intf_idx_wan_intf_idx(
    cparser_context_t *context,
    uint32_t  *wan_intf_idx_ptr)
{
    int ret;
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_l2tpClientInfoBeforeDial_set(*wan_intf_idx_ptr,&clientL2tp_beforeDial_info);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add wan-intf-l2tpClient-before-info failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{	
		diag_util_mprintf("add l2tp before-info to interface[%d] success. \n",*wan_intf_idx_ptr);
	}
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_l2tpclientinfobeforedial_wan_intf_idx_wan_intf_idx */

/*
 * rg clear l2tpClientInfoBeforeDial
 */
cparser_result_t
cparser_cmd_rg_clear_l2tpClientInfoBeforeDial(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&clientL2tp_beforeDial_info, 0x0, sizeof(rtk_rg_l2tpClientInfoBeforeDial_t));
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_l2tpclientinfobeforedial */

/*
 * rg set l2tpClientInfoBeforeDial username <STRING:username> password <STRING:password> l2tp_server_addr_for_ipv4 <IPV4ADDR:l2tp_server_addr_for_ipv4>
 */
cparser_result_t
cparser_cmd_rg_set_l2tpClientInfoBeforeDial_username_username_password_password_l2tp_server_addr_for_ipv4_l2tp_server_addr_for_ipv4(
    cparser_context_t *context,
    char * *username_ptr,
    char * *password_ptr,
    uint32_t  *l2tp_server_addr_for_ipv4_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memcpy(clientL2tp_beforeDial_info.username, *username_ptr, 32);	
	osal_memcpy(clientL2tp_beforeDial_info.password, *password_ptr, 32);
	clientL2tp_beforeDial_info.l2tp_ipv4_addr=*l2tp_server_addr_for_ipv4_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_l2tpclientinfobeforedial_username_username_password_password_l2tp_server_addr_for_ipv4_l2tp_server_addr_for_ipv4 */

/*
 * rg show l2tpClientInfoAfterDial
 */
cparser_result_t
cparser_cmd_rg_show_l2tpClientInfoAfterDial(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	_diag_l2tpClientInfoShow(&clientL2tp_info);
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_l2tpclientinfoafterdial */

/*
 * rg add l2tpClientInfoAfterDial wan_intf_idx <UINT:wan_intf_idx>
 */
cparser_result_t
cparser_cmd_rg_add_l2tpClientInfoAfterDial_wan_intf_idx_wan_intf_idx(
    cparser_context_t *context,
    uint32_t  *wan_intf_idx_ptr)
{
    int ret;
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_l2tpClientInfoAfterDial_set(*wan_intf_idx_ptr,&clientL2tp_info);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add wan-intf-l2tpClient-after-info failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{	
		diag_util_mprintf("add l2tp after-info to interface[%d] success. \n",*wan_intf_idx_ptr);
	}
	return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_l2tpclientinfoafterdial_wan_intf_idx_wan_intf_idx */

/*
 * rg clear l2tpClientInfoAfterDial
 */
cparser_result_t
cparser_cmd_rg_clear_l2tpClientInfoAfterDial(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&clientL2tp_info, 0x0, sizeof(rtk_rg_l2tpClientInfoAfterDial_t)); 
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_l2tpclientinfoafterdial */

/*
 * rg set l2tpClientInfoAfterDial outer_port <UINT:outer_port> gateway_outer_port <UINT:gateway_outer_port> tunnelId <UINT:tunnelId> sessionId <UINT:sessionId> gateway_tunnelId <UINT:gateway_tunnelId> gateway_sessionId <UINT:gateway_sessionId> ipv4_addr <IPV4ADDR:ipv4_addr> ipv4_network_mask <IPV4ADDR:ipv4_network_mask> gateway_ipv4_addr <IPV4ADDR:gateway_ipv4_addr> mtu <UINT:mtu> ipv4_napt_enable <UINT:ipv4_napt_enable> ipv4_default_gateway_on <UINT:ipv4_default_gateway_on> gw_mac_auto_learn_for_ipv4 <UINT:gw_mac_auto_learn_for_ipv4> gateway_mac_addr_for_ipv4 <MACADDR:gateway_mac_addr_for_ipv4>
 */
cparser_result_t
cparser_cmd_rg_set_l2tpClientInfoAfterDial_outer_port_outer_port_gateway_outer_port_gateway_outer_port_tunnelId_tunnelId_sessionId_sessionId_gateway_tunnelId_gateway_tunnelId_gateway_sessionId_gateway_sessionId_ipv4_addr_ipv4_addr_ipv4_network_mask_ipv4_network_mask_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu_ipv4_napt_enable_ipv4_napt_enable_ipv4_default_gateway_on_ipv4_default_gateway_on_gw_mac_auto_learn_for_ipv4_gw_mac_auto_learn_for_ipv4_gateway_mac_addr_for_ipv4_gateway_mac_addr_for_ipv4(
    cparser_context_t *context,
    uint32_t  *outer_port_ptr,
    uint32_t  *gateway_outer_port_ptr,
    uint32_t  *tunnelId_ptr,
    uint32_t  *sessionId_ptr,
    uint32_t  *gateway_tunnelId_ptr,
    uint32_t  *gateway_sessionId_ptr,
    uint32_t  *ipv4_addr_ptr,
    uint32_t  *ipv4_network_mask_ptr,
    uint32_t  *gateway_ipv4_addr_ptr,
    uint32_t  *mtu_ptr,
    uint32_t  *ipv4_napt_enable_ptr,
    uint32_t  *ipv4_default_gateway_on_ptr,
    uint32_t  *gw_mac_auto_learn_for_ipv4_ptr,
    cparser_macaddr_t  *gateway_mac_addr_for_ipv4_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	bzero(&clientL2tp_info.hw_info,sizeof(rtk_rg_ipStaticInfo_t));
	clientL2tp_info.hw_info.ip_version=IPVER_V4ONLY;
	clientL2tp_info.hw_info.napt_enable=*ipv4_napt_enable_ptr;
	clientL2tp_info.hw_info.ipv4_default_gateway_on=*ipv4_default_gateway_on_ptr;
	clientL2tp_info.hw_info.ip_addr=*ipv4_addr_ptr;
	clientL2tp_info.hw_info.ip_network_mask=*ipv4_network_mask_ptr;
	clientL2tp_info.hw_info.gateway_ipv4_addr=*gateway_ipv4_addr_ptr;
	clientL2tp_info.hw_info.mtu=*mtu_ptr;
	clientL2tp_info.hw_info.gw_mac_auto_learn_for_ipv4=*gw_mac_auto_learn_for_ipv4_ptr;
	osal_memcpy(clientL2tp_info.hw_info.gateway_mac_addr_for_ipv4.octet, gateway_mac_addr_for_ipv4_ptr->octet, ETHER_ADDR_LEN);	
	
	clientL2tp_info.outer_port=*outer_port_ptr;
	clientL2tp_info.gateway_outer_port=*gateway_outer_port_ptr;
	clientL2tp_info.tunnelId=*tunnelId_ptr;
	clientL2tp_info.sessionId=*sessionId_ptr;
	clientL2tp_info.gateway_tunnelId=*gateway_tunnelId_ptr;
	clientL2tp_info.gateway_sessionId=*gateway_sessionId_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_l2tpclientinfoafterdial_outer_port_outer_port_gateway_outer_port_gateway_outer_port_tunnelid_tunnelid_sessionid_sessionid_gateway_tunnelid_gateway_tunnelid_gateway_sessionid_gateway_sessionid_ipv4_addr_ipv4_addr_ipv4_network_mask_ipv4_network_mask_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu_ipv4_napt_enable_ipv4_napt_enable_ipv4_default_gateway_on_ipv4_default_gateway_on_gw_mac_auto_learn_for_ipv4_gw_mac_auto_learn_for_ipv4_gateway_mac_addr_for_ipv4_gateway_mac_addr_for_ipv4 */


/*
 * rg show pppoeDsliteInfoAfterDial
 */
cparser_result_t
cparser_cmd_rg_show_pppoeDsliteInfoAfterDial(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	_diag_pppoeDsliteInfoShow(&clientPppoeDslite_info);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_pppoedsliteinfoafterdial */

/*
 * rg add pppoeDsliteInfoAfterDial wan_intf_idx <UINT:wan_intf_idx>
 */
cparser_result_t
cparser_cmd_rg_add_pppoeDsliteInfoAfterDial_wan_intf_idx_wan_intf_idx(
    cparser_context_t *context,
    uint32_t  *wan_intf_idx_ptr)
{
    int ret;
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_pppoeDsliteInfoAfterDial_set(*wan_intf_idx_ptr,&clientPppoeDslite_info);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add wan-intf-pppoeDslite-after-info failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{	
		diag_util_mprintf("add pppoeDslite after-info to interface[%d] success. \n",*wan_intf_idx_ptr);
	}
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_pppoedsliteinfoafterdial_wan_intf_idx_wan_intf_idx */

/*
 * rg clear pppoeDsliteInfoAfterDial
 */
cparser_result_t
cparser_cmd_rg_clear_pppoeDsliteInfoAfterDial(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&clientPppoeDslite_info, 0x0, sizeof(rtk_rg_pppoeDsliteInfoAfterDial_t)); 
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_pppoedsliteinfoafterdial */

/*
 * rg set pppoeDsliteInfoAfterDial sessionId <UINT:sessionId> ip-version <UINT:ip_version> napt_enable <UINT:napt_enable> ip_addr <IPV4ADDR:ip_addr> ip_network_mask <IPV4ADDR:ip_network_mask> ipv4_default_gateway_on <UINT:ipv4_default_gateway_on> mtu <UINT:mtu>
 */
cparser_result_t
cparser_cmd_rg_set_pppoeDsliteInfoAfterDial_sessionId_sessionId_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_mtu_mtu(
    cparser_context_t *context,
    uint32_t  *sessionId_ptr,
    uint32_t  *ip_version_ptr,
    uint32_t  *napt_enable_ptr,
    uint32_t  *ip_addr_ptr,
    uint32_t  *ip_network_mask_ptr,
    uint32_t  *ipv4_default_gateway_on_ptr,
    uint32_t  *mtu_ptr)
{
    int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
	clientPppoeDslite_info.sessionId = *sessionId_ptr;
	clientPppoeDslite_info.dslite_hw_info.static_info.ip_version=*ip_version_ptr;
	clientPppoeDslite_info.dslite_hw_info.static_info.napt_enable=*napt_enable_ptr;
	clientPppoeDslite_info.dslite_hw_info.static_info.ip_addr=*ip_addr_ptr;
	clientPppoeDslite_info.dslite_hw_info.static_info.ip_network_mask=*ip_network_mask_ptr;

	clientPppoeDslite_info.dslite_hw_info.static_info.ipv4_default_gateway_on=*ipv4_default_gateway_on_ptr;
    clientPppoeDslite_info.dslite_hw_info.static_info.gateway_ipv4_addr=0;
	clientPppoeDslite_info.dslite_hw_info.static_info.mtu=*mtu_ptr;
	
	//gw_mac_auto_learn default setting enable
	clientPppoeDslite_info.dslite_hw_info.static_info.gw_mac_auto_learn_for_ipv4=1;
	memset(clientPppoeDslite_info.dslite_hw_info.static_info.gateway_mac_addr_for_ipv4.octet, 0x0, ETHER_ADDR_LEN);
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_pppoedsliteinfoafterdial_sessionid_sessionid_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_mtu_mtu */

/*
 * rg set pppoeDsliteInfoAfterDial-ipv6 ipv6_napt_enable <UINT:ipv6_napt_enable> ipv6_addr <IPV6ADDR:ipv6_addr> ipv6_mask_length <UINT:ipv6_mask_length> ipv6_default_gateway_on <UINT:ipv6_default_gateway_on> gateway_ipv6_addr <IPV6ADDR:gateway_ipv6_addr> gw_mac_auto_learn_for_ipv6 <UINT:gw_mac_auto_learn_for_ipv6> gateway_mac_addr_for_ipv6 <MACADDR:gateway_mac_addr_for_ipv6> b4_ipv6_addr <IPV6ADDR:b4_ipv6_addr> aftr_ipv6_addr <IPV6ADDR:aftr_ipv6_addr> aftr_mac_auto_learn <UINT:aftr_mac_auto_learn> aftr_mac_addr <MACADDR:aftr_mac_addr>
 */
cparser_result_t
cparser_cmd_rg_set_pppoeDsliteInfoAfterDial_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6_b4_ipv6_addr_b4_ipv6_addr_aftr_ipv6_addr_aftr_ipv6_addr_aftr_mac_auto_learn_aftr_mac_auto_learn_aftr_mac_addr_aftr_mac_addr(
    cparser_context_t *context,
    uint32_t  *ipv6_napt_enable_ptr,
    char * *ipv6_addr_ptr,
    uint32_t  *ipv6_mask_length_ptr,
    uint32_t  *ipv6_default_gateway_on_ptr,
    char * *gateway_ipv6_addr_ptr,
    uint32_t  *gw_mac_auto_learn_for_ipv6_ptr,
    cparser_macaddr_t  *gateway_mac_addr_for_ipv6_ptr,
    char * *b4_ipv6_addr_ptr,
    char * *aftr_ipv6_addr_ptr,
    uint32_t  *aftr_mac_auto_learn_ptr,
    cparser_macaddr_t  *aftr_mac_addr_ptr)
{
    int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
	clientPppoeDslite_info.dslite_hw_info.static_info.ipv6_napt_enable = *ipv6_napt_enable_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(clientPppoeDslite_info.dslite_hw_info.static_info.ipv6_addr.ipv6_addr, TOKEN_STR(6)), ret);
    clientPppoeDslite_info.dslite_hw_info.static_info.ipv6_mask_length=*ipv6_mask_length_ptr;
    clientPppoeDslite_info.dslite_hw_info.static_info.ipv6_default_gateway_on=*ipv6_default_gateway_on_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(clientPppoeDslite_info.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr, TOKEN_STR(12)), ret);
	
	clientPppoeDslite_info.dslite_hw_info.static_info.gw_mac_auto_learn_for_ipv6=*gw_mac_auto_learn_for_ipv6_ptr;
	osal_memcpy(clientPppoeDslite_info.dslite_hw_info.static_info.gateway_mac_addr_for_ipv6.octet, gateway_mac_addr_for_ipv6_ptr->octet, ETHER_ADDR_LEN);

	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(clientPppoeDslite_info.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr, TOKEN_STR(18)), ret);
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(clientPppoeDslite_info.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr, TOKEN_STR(20)), ret);
	clientPppoeDslite_info.dslite_hw_info.aftr_mac_auto_learn=*aftr_mac_auto_learn_ptr;
	osal_memcpy(clientPppoeDslite_info.dslite_hw_info.aftr_mac_addr.octet, aftr_mac_addr_ptr->octet, ETHER_ADDR_LEN);
	clientPppoeDslite_info.dslite_hw_info.rtk_dslite.hopLimit=255;
	clientPppoeDslite_info.dslite_hw_info.rtk_dslite.flowLabel=0;
	clientPppoeDslite_info.dslite_hw_info.rtk_dslite.tcOpt=RTK_L34_DSLITE_TC_OPT_ASSIGN;
	clientPppoeDslite_info.dslite_hw_info.rtk_dslite.tc=0;
	clientPppoeDslite_info.dslite_hw_info.rtk_dslite.valid=1;
   	return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_pppoedsliteinfoafterdial_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6_b4_ipv6_addr_b4_ipv6_addr_aftr_ipv6_addr_aftr_ipv6_addr_aftr_mac_auto_learn_aftr_mac_auto_learn_aftr_mac_addr_aftr_mac_addr */

/*
 * rg set pppoeDsliteInfoAfterDial-ipv6 ipv6_napt_enable <UINT:ipv6_napt_enable> ipv6_addr <IPV6ADDR:ipv6_addr> ipv6_mask_length <UINT:ipv6_mask_length> ipv6_default_gateway_on <UINT:ipv6_default_gateway_on> gateway_ipv6_addr <IPV6ADDR:gateway_ipv6_addr> gw_mac_auto_learn_for_ipv6 <UINT:gw_mac_auto_learn_for_ipv6> gateway_mac_addr_for_ipv6 <MACADDR:gateway_mac_addr_for_ipv6> b4_ipv6_addr <IPV6ADDR:b4_ipv6_addr> aftr_ipv6_addr <IPV6ADDR:aftr_ipv6_addr> aftr_mac_auto_learn <UINT:aftr_mac_auto_learn> aftr_mac_addr <MACADDR:aftr_mac_addr> dslite_hoplimit <UINT:dslite_hoplimit> dslite_flowlabel <HEX:dslite_flowlabel> dslite_tc copy_from_IPv4_TOS
 */
cparser_result_t
cparser_cmd_rg_set_pppoeDsliteInfoAfterDial_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6_b4_ipv6_addr_b4_ipv6_addr_aftr_ipv6_addr_aftr_ipv6_addr_aftr_mac_auto_learn_aftr_mac_auto_learn_aftr_mac_addr_aftr_mac_addr_dslite_hoplimit_dslite_hoplimit_dslite_flowlabel_dslite_flowlabel_dslite_tc_copy_from_IPv4_TOS(
    cparser_context_t *context,
    uint32_t  *ipv6_napt_enable_ptr,
    char * *ipv6_addr_ptr,
    uint32_t  *ipv6_mask_length_ptr,
    uint32_t  *ipv6_default_gateway_on_ptr,
    char * *gateway_ipv6_addr_ptr,
    uint32_t  *gw_mac_auto_learn_for_ipv6_ptr,
    cparser_macaddr_t  *gateway_mac_addr_for_ipv6_ptr,
    char * *b4_ipv6_addr_ptr,
    char * *aftr_ipv6_addr_ptr,
    uint32_t  *aftr_mac_auto_learn_ptr,
    cparser_macaddr_t  *aftr_mac_addr_ptr,
    uint32_t  *dslite_hoplimit_ptr,
    uint32_t  *dslite_flowlabel_ptr)
{
    int32  ret = RT_ERR_FAILED;
#if defined(CONFIG_RTL9600_SERIES)
	return CPARSER_NOT_OK;
#else
    DIAG_UTIL_PARAM_CHK();
	clientPppoeDslite_info.dslite_hw_info.static_info.ipv6_napt_enable = *ipv6_napt_enable_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(clientPppoeDslite_info.dslite_hw_info.static_info.ipv6_addr.ipv6_addr, TOKEN_STR(6)), ret);
    clientPppoeDslite_info.dslite_hw_info.static_info.ipv6_mask_length=*ipv6_mask_length_ptr;
    clientPppoeDslite_info.dslite_hw_info.static_info.ipv6_default_gateway_on=*ipv6_default_gateway_on_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(clientPppoeDslite_info.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr, TOKEN_STR(12)), ret);
	
	clientPppoeDslite_info.dslite_hw_info.static_info.gw_mac_auto_learn_for_ipv6=*gw_mac_auto_learn_for_ipv6_ptr;
	osal_memcpy(clientPppoeDslite_info.dslite_hw_info.static_info.gateway_mac_addr_for_ipv6.octet, gateway_mac_addr_for_ipv6_ptr->octet, ETHER_ADDR_LEN);

	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(clientPppoeDslite_info.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr, TOKEN_STR(18)), ret);
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(clientPppoeDslite_info.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr, TOKEN_STR(20)), ret);
	clientPppoeDslite_info.dslite_hw_info.aftr_mac_auto_learn=*aftr_mac_auto_learn_ptr;
	osal_memcpy(clientPppoeDslite_info.dslite_hw_info.aftr_mac_addr.octet, aftr_mac_addr_ptr->octet, ETHER_ADDR_LEN);
	clientPppoeDslite_info.dslite_hw_info.rtk_dslite.hopLimit=(*dslite_hoplimit_ptr)&0xff;
	clientPppoeDslite_info.dslite_hw_info.rtk_dslite.flowLabel=(*dslite_flowlabel_ptr)&0xfffff;
	clientPppoeDslite_info.dslite_hw_info.rtk_dslite.tcOpt=RTK_L34_DSLITE_TC_OPT_COPY_FROM_TOS;
	clientPppoeDslite_info.dslite_hw_info.rtk_dslite.tc=0;
	clientPppoeDslite_info.dslite_hw_info.rtk_dslite.valid=1;	
   	return CPARSER_OK;
#endif
}    /* end of cparser_cmd_rg_set_pppoedsliteinfoafterdial_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6_b4_ipv6_addr_b4_ipv6_addr_aftr_ipv6_addr_aftr_ipv6_addr_aftr_mac_auto_learn_aftr_mac_auto_learn_aftr_mac_addr_aftr_mac_addr_dslite_hoplimit_dslite_hoplimit_dslite_flowlabel_dslite_flowlabel_dslite_tc_copy_from_ipv4_tos */

/*
 * rg set pppoeDsliteInfoAfterDial-ipv6 ipv6_napt_enable <UINT:ipv6_napt_enable> ipv6_addr <IPV6ADDR:ipv6_addr> ipv6_mask_length <UINT:ipv6_mask_length> ipv6_default_gateway_on <UINT:ipv6_default_gateway_on> gateway_ipv6_addr <IPV6ADDR:gateway_ipv6_addr> gw_mac_auto_learn_for_ipv6 <UINT:gw_mac_auto_learn_for_ipv6> gateway_mac_addr_for_ipv6 <MACADDR:gateway_mac_addr_for_ipv6> b4_ipv6_addr <IPV6ADDR:b4_ipv6_addr> aftr_ipv6_addr <IPV6ADDR:aftr_ipv6_addr> aftr_mac_auto_learn <UINT:aftr_mac_auto_learn> aftr_mac_addr <MACADDR:aftr_mac_addr> dslite_hoplimit <UINT:dslite_hoplimit> dslite_flowlabel <HEX:dslite_flowlabel> dslite_tc <HEX:dslite_tc>
 */
cparser_result_t
cparser_cmd_rg_set_pppoeDsliteInfoAfterDial_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6_b4_ipv6_addr_b4_ipv6_addr_aftr_ipv6_addr_aftr_ipv6_addr_aftr_mac_auto_learn_aftr_mac_auto_learn_aftr_mac_addr_aftr_mac_addr_dslite_hoplimit_dslite_hoplimit_dslite_flowlabel_dslite_flowlabel_dslite_tc_dslite_tc(
    cparser_context_t *context,
    uint32_t  *ipv6_napt_enable_ptr,
    char * *ipv6_addr_ptr,
    uint32_t  *ipv6_mask_length_ptr,
    uint32_t  *ipv6_default_gateway_on_ptr,
    char * *gateway_ipv6_addr_ptr,
    uint32_t  *gw_mac_auto_learn_for_ipv6_ptr,
    cparser_macaddr_t  *gateway_mac_addr_for_ipv6_ptr,
    char * *b4_ipv6_addr_ptr,
    char * *aftr_ipv6_addr_ptr,
    uint32_t  *aftr_mac_auto_learn_ptr,
    cparser_macaddr_t  *aftr_mac_addr_ptr,
    uint32_t  *dslite_hoplimit_ptr,
    uint32_t  *dslite_flowlabel_ptr,
    uint32_t  *dslite_tc_ptr)
{
    int32  ret = RT_ERR_FAILED;
#if defined(CONFIG_RTL9600_SERIES)
	return CPARSER_NOT_OK;
#else
    DIAG_UTIL_PARAM_CHK();
	clientPppoeDslite_info.dslite_hw_info.static_info.ipv6_napt_enable = *ipv6_napt_enable_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(clientPppoeDslite_info.dslite_hw_info.static_info.ipv6_addr.ipv6_addr, TOKEN_STR(6)), ret);
    clientPppoeDslite_info.dslite_hw_info.static_info.ipv6_mask_length=*ipv6_mask_length_ptr;
    clientPppoeDslite_info.dslite_hw_info.static_info.ipv6_default_gateway_on=*ipv6_default_gateway_on_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(clientPppoeDslite_info.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr, TOKEN_STR(12)), ret);
	
	clientPppoeDslite_info.dslite_hw_info.static_info.gw_mac_auto_learn_for_ipv6=*gw_mac_auto_learn_for_ipv6_ptr;
	osal_memcpy(clientPppoeDslite_info.dslite_hw_info.static_info.gateway_mac_addr_for_ipv6.octet, gateway_mac_addr_for_ipv6_ptr->octet, ETHER_ADDR_LEN);

	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(clientPppoeDslite_info.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr, TOKEN_STR(18)), ret);
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(clientPppoeDslite_info.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr, TOKEN_STR(20)), ret);
	clientPppoeDslite_info.dslite_hw_info.aftr_mac_auto_learn=*aftr_mac_auto_learn_ptr;
	osal_memcpy(clientPppoeDslite_info.dslite_hw_info.aftr_mac_addr.octet, aftr_mac_addr_ptr->octet, ETHER_ADDR_LEN);
	clientPppoeDslite_info.dslite_hw_info.rtk_dslite.hopLimit=(*dslite_hoplimit_ptr)&0xff;
	clientPppoeDslite_info.dslite_hw_info.rtk_dslite.flowLabel=(*dslite_flowlabel_ptr)&0xfffff;
	clientPppoeDslite_info.dslite_hw_info.rtk_dslite.tcOpt=RTK_L34_DSLITE_TC_OPT_ASSIGN;
	clientPppoeDslite_info.dslite_hw_info.rtk_dslite.tc=(*dslite_tc_ptr)&0xff;
	clientPppoeDslite_info.dslite_hw_info.rtk_dslite.valid=1;
   	return CPARSER_OK;
#endif
}    /* end of cparser_cmd_rg_set_pppoedsliteinfoafterdial_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6_b4_ipv6_addr_b4_ipv6_addr_aftr_ipv6_addr_aftr_ipv6_addr_aftr_mac_auto_learn_aftr_mac_auto_learn_aftr_mac_addr_aftr_mac_addr_dslite_hoplimit_dslite_hoplimit_dslite_flowlabel_dslite_flowlabel_dslite_tc_dslite_tc */


/*
 * rg show pppoeDsliteInfoBeforeDial
 */
cparser_result_t
cparser_cmd_rg_show_pppoeDsliteInfoBeforeDial(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	_diag_pppoeClientInfoBeforeDiagShow(&clientPppoeDslite_beforeDial_info);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_pppoedsliteinfobeforedial */

/*
 * rg add pppoeDsliteInfoBeforeDial wan_intf_idx <UINT:wan_intf_idx>
 */
cparser_result_t
cparser_cmd_rg_add_pppoeDsliteInfoBeforeDial_wan_intf_idx_wan_intf_idx(
    cparser_context_t *context,
    uint32_t  *wan_intf_idx_ptr)
{
    int ret;
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_pppoeDsliteInfoBeforeDial_set(*wan_intf_idx_ptr, &clientPppoeDslite_beforeDial_info);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add wan-intf-pppoeDslite-before-info failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{	
		diag_util_mprintf("add pppoeDslite before-info to interface[%d] success. \n",*wan_intf_idx_ptr);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_pppoedsliteinfobeforedial_wan_intf_idx_wan_intf_idx */

/*
 * rg clear pppoeDsliteInfoBeforeDial
 */
cparser_result_t
cparser_cmd_rg_clear_pppoeDsliteInfoBeforeDial(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&clientPppoeDslite_beforeDial_info, 0x0, sizeof(rtk_rg_pppoeClientInfoBeforeDial_t));
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_pppoedsliteinfobeforedial */

/*
 * rg set pppoeDsliteInfoBeforeDial username <STRING:username> password <STRING:password> auth_type <UINT:auth_type> pppoe_proxy_enable <UINT:pppoe_proxy_enable> max_pppoe_proxy_num <UINT:max_pppoe_proxy_num> auto_reconnect <UINT:auto_reconnect> dial_on_demond <UINT:dial_on_demond> idle_timeout_secs <UINT:idle_timeout_secs> status <UINT:status> dialOnDemondCallBack <HEX:dialOnDemondCallBack_addr> idleTimeOutCallBack <HEX:idleTimeOutCallBack_addr>
 */
cparser_result_t
cparser_cmd_rg_set_pppoeDsliteInfoBeforeDial_username_username_password_password_auth_type_auth_type_pppoe_proxy_enable_pppoe_proxy_enable_max_pppoe_proxy_num_max_pppoe_proxy_num_auto_reconnect_auto_reconnect_dial_on_demond_dial_on_demond_idle_timeout_secs_idle_timeout_secs_status_status_dialOnDemondCallBack_dialOnDemondCallBack_addr_idleTimeOutCallBack_idleTimeOutCallBack_addr(
    cparser_context_t *context,
    char * *username_ptr,
    char * *password_ptr,
    uint32_t  *auth_type_ptr,
    uint32_t  *pppoe_proxy_enable_ptr,
    uint32_t  *max_pppoe_proxy_num_ptr,
    uint32_t  *auto_reconnect_ptr,
    uint32_t  *dial_on_demond_ptr,
    uint32_t  *idle_timeout_secs_ptr,
    uint32_t  *status_ptr,
    uint32_t  *dialOnDemondCallBack_addr_ptr,
    uint32_t  *idleTimeOutCallBack_addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	
	osal_memcpy(clientPppoeDslite_beforeDial_info.username, *username_ptr, 32);	
	osal_memcpy(clientPppoeDslite_beforeDial_info.password, *password_ptr, 32);	
	clientPppoeDslite_beforeDial_info.auth_type= *auth_type_ptr;
	clientPppoeDslite_beforeDial_info.pppoe_proxy_enable= *pppoe_proxy_enable_ptr;
	clientPppoeDslite_beforeDial_info.max_pppoe_proxy_num= *max_pppoe_proxy_num_ptr;
	clientPppoeDslite_beforeDial_info.auto_reconnect= *auto_reconnect_ptr;
	clientPppoeDslite_beforeDial_info.dial_on_demond= *dial_on_demond_ptr;
	clientPppoeDslite_beforeDial_info.idle_timeout_secs= *idle_timeout_secs_ptr;
	clientPppoeDslite_beforeDial_info.stauts= *status_ptr;
	if(*dialOnDemondCallBack_addr_ptr==0x0){
		clientPppoeDslite_beforeDial_info.dialOnDemondCallBack=NULL;
	}else{
		clientPppoeDslite_beforeDial_info.dialOnDemondCallBack=*dialOnDemondCallBack_addr_ptr;
	}

	if(*idleTimeOutCallBack_addr_ptr==0x0){
		clientPppoeDslite_beforeDial_info.idleTimeOutCallBack=NULL;
	}else{
		clientPppoeDslite_beforeDial_info.idleTimeOutCallBack=*idleTimeOutCallBack_addr_ptr;
	}
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_pppoedsliteinfobeforedial_username_username_password_password_auth_type_auth_type_pppoe_proxy_enable_pppoe_proxy_enable_max_pppoe_proxy_num_max_pppoe_proxy_num_auto_reconnect_auto_reconnect_dial_on_demond_dial_on_demond_idle_timeout_secs_idle_timeout_secs_status_status_dialondemondcallback_dialondemondcallback_addr_idletimeoutcallback_idletimeoutcallback_addr */


/*
 * rg show neighbor-entry
 */
cparser_result_t
cparser_cmd_rg_show_neighbor_entry(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	_diag_neighborEntryShow(&neighborEntry);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_neighbor_entry */

/*
 * rg clear neighbor-entry
 */
cparser_result_t
cparser_cmd_rg_clear_neighbor_entry(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&neighborEntry, 0x0, sizeof(rtk_rg_neighborEntry_t)); 

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_neighbor_entry */

/*
 * rg add neighbor-entry entry
 */
cparser_result_t
cparser_cmd_rg_add_neighbor_entry_entry(
    cparser_context_t *context)
{
	int ret;
	int index;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rtk_rg_neighborEntry_add(&neighborEntry,&index);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add neighborEntry failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add neighborEntry[%d] success.\n",index);
	}
	
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_add_neighbor_entry_entry */

/*
 * rg get neighbor-entry entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_get_neighbor_entry_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int ret;
	int index=*entry_index_ptr;

	DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
	osal_memset(&neighborInfo, 0x0, sizeof(rtk_rg_neighborInfo_t)); 

	ret = rtk_rg_neighborEntry_find(&neighborInfo,&index);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get neighborEntry failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		if(index==*entry_index_ptr){	
			_diag_neighborEntryShow(&neighborInfo.neighborEntry);
		}
		else{
			diag_util_mprintf("neighborEntry[%d] is empty.\n",*entry_index_ptr);
			return CPARSER_NOT_OK;
		}
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_neighbor_entry_entry_entry_index */

/*
 * rg del neighbor-entry entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_del_neighbor_entry_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int ret;
	DIAG_UTIL_PARAM_CHK();
	
	ret = rtk_rg_neighborEntry_del(*entry_index_ptr);
	if(ret!=RT_ERR_RG_OK)
		return CPARSER_NOT_OK;
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_del_neighbor_entry_entry_entry_index */

/*
 * rg set neighbor-entry l2Idx <UINT:l2Idx> matchRouteIdx <UINT:matchRouteIdx> interfaceId_up <HEX:ipv6_addr_63_32> interfaceId_low <HEX:ipv6_addr_31_0> staticEntry <UINT:staticEntry>
 */
cparser_result_t
cparser_cmd_rg_set_neighbor_entry_l2Idx_l2Idx_matchRouteIdx_matchRouteIdx_interfaceId_up_ipv6_addr_63_32_interfaceId_low_ipv6_addr_31_0_staticEntry_staticEntry(
    cparser_context_t *context,
    uint32_t  *l2Idx_ptr,
    uint32_t  *matchRouteIdx_ptr,
    uint32_t  *ipv6_addr_63_32_ptr,
    uint32_t  *ipv6_addr_31_0_ptr,
    uint32_t  *staticEntry_ptr)
{
    DIAG_UTIL_PARAM_CHK();
        neighborEntry.l2Idx = *l2Idx_ptr;
        neighborEntry.matchRouteIdx = *matchRouteIdx_ptr;
        osal_memcpy(&neighborEntry.interfaceId[0], ipv6_addr_63_32_ptr, 4);
        osal_memcpy(&neighborEntry.interfaceId[4], ipv6_addr_31_0_ptr, 4);
        neighborEntry.valid = 1;
        neighborEntry.staticEntry = *staticEntry_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_neighbor_entry_l2idx_l2idx_matchrouteidx_matchrouteidx_interfaceid_up_ipv6_addr_63_32_interfaceid_low_ipv6_addr_31_0_staticentry_staticentry */

/*
 * rg add dhcpRequest entry
 */
cparser_result_t
cparser_cmd_rg_add_dhcpRequest_entry(
    cparser_context_t *context)
{
	//directly use: rg set dhcpRequest 
    DIAG_UTIL_PARAM_CHK();
	return CPARSER_NOT_OK;
}    /* end of cparser_cmd_rg_add_dhcprequest_entry */

/*
 * rg set dhcpRequest wan-intf-idx <UINT:wan_intf_idx>
 */
cparser_result_t
cparser_cmd_rg_set_dhcpRequest_wan_intf_idx_wan_intf_idx(
    cparser_context_t *context,
    uint32_t  *wan_intf_idx_ptr)
{
 	int ret;
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_dhcpRequest_set(*wan_intf_idx_ptr);
	if(ret!=RT_ERR_RG_OK)
		return CPARSER_NOT_OK;
	return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_dhcprequest_wan_intf_idx_wan_intf_idx */

/*
 * rg show dhcpClientInfo
 */
cparser_result_t
cparser_cmd_rg_show_dhcpClientInfo(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	_diag_dhcpClientInfoShow(&dhcpClient_info);
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_dhcpclientinfo */

/*
 * rg add dhcpClientInfo wan_intf_idx <UINT:wan_intf_idx>
 */
cparser_result_t
cparser_cmd_rg_add_dhcpClientInfo_wan_intf_idx_wan_intf_idx(
    cparser_context_t *context,
    uint32_t  *wan_intf_idx_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_dhcpClientInfo_set(*wan_intf_idx_ptr, &dhcpClient_info);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add wan-intf-dhcpClient-info failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{	
		diag_util_mprintf("add dhcp info to interface[%d] success. \n",*wan_intf_idx_ptr);
	}
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_dhcpclientinfo_wan_intf_idx_wan_intf_idx */

/*
 * rg clear dhcpClientInfo
 */
cparser_result_t
cparser_cmd_rg_clear_dhcpClientInfo(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&dhcpClient_info, 0x0, sizeof(rtk_rg_ipDhcpClientInfo_t)); 

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_dhcpclientinfo */

/*
 * rg set dhcpClientInfo stauts <UINT:stauts> ip-version <UINT:ip_version> napt_enable <UINT:napt_enable> ip_addr <IPV4ADDR:ip_addr> ip_network_mask <IPV4ADDR:ip_network_mask> ipv4_default_gateway_on <UINT:ipv4_default_gateway_on> gateway_ipv4_addr <IPV4ADDR:gateway_ipv4_addr> mtu <UINT:mtu>
 */
cparser_result_t
cparser_cmd_rg_set_dhcpClientInfo_stauts_stauts_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu(
    cparser_context_t *context,
    uint32_t  *stauts_ptr,
    uint32_t  *ip_version_ptr,
    uint32_t  *napt_enable_ptr,
    uint32_t  *ip_addr_ptr,
    uint32_t  *ip_network_mask_ptr,
    uint32_t  *ipv4_default_gateway_on_ptr,
    uint32_t  *gateway_ipv4_addr_ptr,
    uint32_t  *mtu_ptr)
{
	int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
	dhcpClient_info.stauts= *stauts_ptr;
	dhcpClient_info.hw_info.ip_version=*ip_version_ptr;
	dhcpClient_info.hw_info.napt_enable=*napt_enable_ptr;
	dhcpClient_info.hw_info.ip_addr=*ip_addr_ptr;
	dhcpClient_info.hw_info.ip_network_mask=*ip_network_mask_ptr;

	dhcpClient_info.hw_info.ipv4_default_gateway_on=*ipv4_default_gateway_on_ptr;
    dhcpClient_info.hw_info.gateway_ipv4_addr=*gateway_ipv4_addr_ptr;
	dhcpClient_info.hw_info.mtu=*mtu_ptr;
	dhcpClient_info.hw_info.static_route_with_arp=0;
	
	//gw_mac_auto_learn default setting enable
	dhcpClient_info.hw_info.gw_mac_auto_learn_for_ipv4=1;
	memset(dhcpClient_info.hw_info.gateway_mac_addr_for_ipv4.octet, 0x0, ETHER_ADDR_LEN);		
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_set_dhcpclientinfo_stauts_stauts_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu */

/*
 * rg set dhcpClientInfo stauts <UINT:stauts> ip-version <UINT:ip_version> napt_enable <UINT:napt_enable> ip_addr <IPV4ADDR:ip_addr> ip_network_mask <IPV4ADDR:ip_network_mask> ipv4_default_gateway_on <UINT:ipv4_default_gateway_on> gateway_ipv4_addr <IPV4ADDR:gateway_ipv4_addr> mtu <UINT:mtu> gw_mac_auto_learn_for_ipv4 <UINT:gw_mac_auto_learn_for_ipv4> gateway_mac_addr_for_ipv4 <MACADDR:gateway_mac_addr_for_ipv4>
 */
cparser_result_t
cparser_cmd_rg_set_dhcpClientInfo_stauts_stauts_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu_gw_mac_auto_learn_for_ipv4_gw_mac_auto_learn_for_ipv4_gateway_mac_addr_for_ipv4_gateway_mac_addr_for_ipv4(
    cparser_context_t *context,
    uint32_t  *stauts_ptr,
    uint32_t  *ip_version_ptr,
    uint32_t  *napt_enable_ptr,
    uint32_t  *ip_addr_ptr,
    uint32_t  *ip_network_mask_ptr,
    uint32_t  *ipv4_default_gateway_on_ptr,
    uint32_t  *gateway_ipv4_addr_ptr,
    uint32_t  *mtu_ptr,
    uint32_t  *gw_mac_auto_learn_for_ipv4_ptr,
    cparser_macaddr_t  *gateway_mac_addr_for_ipv4_ptr)
{
	int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
	dhcpClient_info.stauts= *stauts_ptr;
	dhcpClient_info.hw_info.ip_version=*ip_version_ptr;
	dhcpClient_info.hw_info.napt_enable=*napt_enable_ptr;
	dhcpClient_info.hw_info.ip_addr=*ip_addr_ptr;
	dhcpClient_info.hw_info.ip_network_mask=*ip_network_mask_ptr;

	dhcpClient_info.hw_info.ipv4_default_gateway_on=*ipv4_default_gateway_on_ptr;
    dhcpClient_info.hw_info.gateway_ipv4_addr=*gateway_ipv4_addr_ptr;
	dhcpClient_info.hw_info.mtu=*mtu_ptr;
	dhcpClient_info.hw_info.static_route_with_arp=0;
	
	dhcpClient_info.hw_info.gw_mac_auto_learn_for_ipv4=*gw_mac_auto_learn_for_ipv4_ptr;
	osal_memcpy(dhcpClient_info.hw_info.gateway_mac_addr_for_ipv4.octet, gateway_mac_addr_for_ipv4_ptr->octet, ETHER_ADDR_LEN);	
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_set_dhcpclientinfo_stauts_stauts_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu_gw_mac_auto_learn_for_ipv4_gw_mac_auto_learn_for_ipv4_gateway_mac_addr_for_ipv4_gateway_mac_addr_for_ipv4 */

/*
 * rg set dhcpClientInfo stauts <UINT:stauts> ip-version <UINT:ip_version> napt_enable <UINT:napt_enable> ip_addr <IPV4ADDR:ip_addr> ip_network_mask <IPV4ADDR:ip_network_mask> ipv4_default_gateway_on <UINT:ipv4_default_gateway_on> gateway_ipv4_addr <IPV4ADDR:gateway_ipv4_addr> mtu <UINT:mtu> routing-type <UINT:routing_type>
 */
cparser_result_t
cparser_cmd_rg_set_dhcpClientInfo_stauts_stauts_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu_routing_type_routing_type(
    cparser_context_t *context,
    uint32_t  *stauts_ptr,
    uint32_t  *ip_version_ptr,
    uint32_t  *napt_enable_ptr,
    uint32_t  *ip_addr_ptr,
    uint32_t  *ip_network_mask_ptr,
    uint32_t  *ipv4_default_gateway_on_ptr,
    uint32_t  *gateway_ipv4_addr_ptr,
    uint32_t  *mtu_ptr,
    uint32_t  *routing_type_ptr)
{
	int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
	dhcpClient_info.stauts= *stauts_ptr;
	dhcpClient_info.hw_info.ip_version=*ip_version_ptr;
	dhcpClient_info.hw_info.napt_enable=*napt_enable_ptr;
	dhcpClient_info.hw_info.ip_addr=*ip_addr_ptr;
	dhcpClient_info.hw_info.ip_network_mask=*ip_network_mask_ptr;

	dhcpClient_info.hw_info.ipv4_default_gateway_on=*ipv4_default_gateway_on_ptr;
    dhcpClient_info.hw_info.gateway_ipv4_addr=*gateway_ipv4_addr_ptr;
	dhcpClient_info.hw_info.mtu=*mtu_ptr;
	dhcpClient_info.hw_info.static_route_with_arp=*routing_type_ptr;
	
	//gw_mac_auto_learn default setting enable
	dhcpClient_info.hw_info.gw_mac_auto_learn_for_ipv4=1;
	memset(dhcpClient_info.hw_info.gateway_mac_addr_for_ipv4.octet, 0x0, ETHER_ADDR_LEN);		
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_set_dhcpclientinfo_stauts_stauts_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu_routing_type_routing_type */

/*
 * rg set dhcpClientInfo stauts <UINT:stauts> ip-version <UINT:ip_version> napt_enable <UINT:napt_enable> ip_addr <IPV4ADDR:ip_addr> ip_network_mask <IPV4ADDR:ip_network_mask> ipv4_default_gateway_on <UINT:ipv4_default_gateway_on> gateway_ipv4_addr <IPV4ADDR:gateway_ipv4_addr> mtu <UINT:mtu> routing-type <UINT:routing_type> gw_mac_auto_learn_for_ipv4 <UINT:gw_mac_auto_learn_for_ipv4> gateway_mac_addr_for_ipv4 <MACADDR:gateway_mac_addr_for_ipv4>
 */
cparser_result_t
cparser_cmd_rg_set_dhcpClientInfo_stauts_stauts_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu_routing_type_routing_type_gw_mac_auto_learn_for_ipv4_gw_mac_auto_learn_for_ipv4_gateway_mac_addr_for_ipv4_gateway_mac_addr_for_ipv4(
    cparser_context_t *context,
    uint32_t  *stauts_ptr,
    uint32_t  *ip_version_ptr,
    uint32_t  *napt_enable_ptr,
    uint32_t  *ip_addr_ptr,
    uint32_t  *ip_network_mask_ptr,
    uint32_t  *ipv4_default_gateway_on_ptr,
    uint32_t  *gateway_ipv4_addr_ptr,
    uint32_t  *mtu_ptr,
    uint32_t  *routing_type_ptr,
    uint32_t  *gw_mac_auto_learn_for_ipv4_ptr,
    cparser_macaddr_t  *gateway_mac_addr_for_ipv4_ptr)
{
	int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
	dhcpClient_info.stauts= *stauts_ptr;
	dhcpClient_info.hw_info.ip_version=*ip_version_ptr;
	dhcpClient_info.hw_info.napt_enable=*napt_enable_ptr;
	dhcpClient_info.hw_info.ip_addr=*ip_addr_ptr;
	dhcpClient_info.hw_info.ip_network_mask=*ip_network_mask_ptr;

	dhcpClient_info.hw_info.ipv4_default_gateway_on=*ipv4_default_gateway_on_ptr;
    dhcpClient_info.hw_info.gateway_ipv4_addr=*gateway_ipv4_addr_ptr;
	dhcpClient_info.hw_info.mtu=*mtu_ptr;
	dhcpClient_info.hw_info.static_route_with_arp=*routing_type_ptr;
	
	dhcpClient_info.hw_info.gw_mac_auto_learn_for_ipv4=*gw_mac_auto_learn_for_ipv4_ptr;
	osal_memcpy(dhcpClient_info.hw_info.gateway_mac_addr_for_ipv4.octet, gateway_mac_addr_for_ipv4_ptr->octet, ETHER_ADDR_LEN);	
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_set_dhcpclientinfo_stauts_stauts_ip_version_ip_version_napt_enable_napt_enable_ip_addr_ip_addr_ip_network_mask_ip_network_mask_ipv4_default_gateway_on_ipv4_default_gateway_on_gateway_ipv4_addr_gateway_ipv4_addr_mtu_mtu_routing_type_routing_type_gw_mac_auto_learn_for_ipv4_gw_mac_auto_learn_for_ipv4_gateway_mac_addr_for_ipv4_gateway_mac_addr_for_ipv4 */

/*
 * rg set dhcpClientInfo-ipv6 ipv6_addr <IPV6ADDR:ipv6_addr> ipv6_mask_length <UINT:ipv6_mask_length> ipv6_default_gateway_on <UINT:ipv6_default_gateway_on> gateway_ipv6_addr <IPV6ADDR:gateway_ipv6_addr>
 */
cparser_result_t
cparser_cmd_rg_set_dhcpClientInfo_ipv6_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr(
    cparser_context_t *context,
    char * *ipv6_addr_ptr,
    uint32_t  *ipv6_mask_length_ptr,
    uint32_t  *ipv6_default_gateway_on_ptr,
    char * *gateway_ipv6_addr_ptr)
{
	int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&dhcpClient_info.hw_info.ipv6_addr.ipv6_addr[0], TOKEN_STR(4)), ret);
    dhcpClient_info.hw_info.ipv6_mask_length=*ipv6_mask_length_ptr;
    dhcpClient_info.hw_info.ipv6_default_gateway_on=*ipv6_default_gateway_on_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&dhcpClient_info.hw_info.gateway_ipv6_addr.ipv6_addr[0], TOKEN_STR(10)), ret);
	
	//gw_mac_auto_learn default setting enable	
	dhcpClient_info.hw_info.gw_mac_auto_learn_for_ipv6=1;
	memset(dhcpClient_info.hw_info.gateway_mac_addr_for_ipv6.octet, 0x0, ETHER_ADDR_LEN);	
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_set_dhcpclientinfo_ipv6_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr */

/*
 * rg set dhcpClientInfo-ipv6 ipv6_addr <IPV6ADDR:ipv6_addr> ipv6_mask_length <UINT:ipv6_mask_length> ipv6_default_gateway_on <UINT:ipv6_default_gateway_on> gateway_ipv6_addr <IPV6ADDR:gateway_ipv6_addr> gw_mac_auto_learn_for_ipv6 <UINT:gw_mac_auto_learn_for_ipv6> gateway_mac_addr_for_ipv6 <MACADDR:gateway_mac_addr_for_ipv6>
 */
cparser_result_t
cparser_cmd_rg_set_dhcpClientInfo_ipv6_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6(
    cparser_context_t *context,
    char * *ipv6_addr_ptr,
    uint32_t  *ipv6_mask_length_ptr,
    uint32_t  *ipv6_default_gateway_on_ptr,
    char * *gateway_ipv6_addr_ptr,
    uint32_t  *gw_mac_auto_learn_for_ipv6_ptr,
    cparser_macaddr_t  *gateway_mac_addr_for_ipv6_ptr)
{
	int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&dhcpClient_info.hw_info.ipv6_addr.ipv6_addr[0], TOKEN_STR(4)), ret);
    dhcpClient_info.hw_info.ipv6_mask_length=*ipv6_mask_length_ptr;
    dhcpClient_info.hw_info.ipv6_default_gateway_on=*ipv6_default_gateway_on_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&dhcpClient_info.hw_info.gateway_ipv6_addr.ipv6_addr[0], TOKEN_STR(10)), ret);

	
	dhcpClient_info.hw_info.gw_mac_auto_learn_for_ipv6=*gw_mac_auto_learn_for_ipv6_ptr;
	osal_memcpy(dhcpClient_info.hw_info.gateway_mac_addr_for_ipv6.octet, gateway_mac_addr_for_ipv6_ptr->octet, ETHER_ADDR_LEN);
	
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_set_dhcpclientinfo_ipv6_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6 */

/*
 * rg set dhcpClientInfo-ipv6 ipv6_napt_enable <UINT:ipv6_napt_enable> ipv6_addr <IPV6ADDR:ipv6_addr> ipv6_mask_length <UINT:ipv6_mask_length> ipv6_default_gateway_on <UINT:ipv6_default_gateway_on> gateway_ipv6_addr <IPV6ADDR:gateway_ipv6_addr>
 */
cparser_result_t
cparser_cmd_rg_set_dhcpClientInfo_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr(
    cparser_context_t *context,
    uint32_t  *ipv6_napt_enable_ptr,
    char * *ipv6_addr_ptr,
    uint32_t  *ipv6_mask_length_ptr,
    uint32_t  *ipv6_default_gateway_on_ptr,
    char * *gateway_ipv6_addr_ptr)
{
	int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
	dhcpClient_info.hw_info.ipv6_napt_enable = *ipv6_napt_enable_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&dhcpClient_info.hw_info.ipv6_addr.ipv6_addr[0], TOKEN_STR(6)), ret);
    dhcpClient_info.hw_info.ipv6_mask_length=*ipv6_mask_length_ptr;
    dhcpClient_info.hw_info.ipv6_default_gateway_on=*ipv6_default_gateway_on_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&dhcpClient_info.hw_info.gateway_ipv6_addr.ipv6_addr[0], TOKEN_STR(12)), ret);
	
	//gw_mac_auto_learn default setting enable	
	dhcpClient_info.hw_info.gw_mac_auto_learn_for_ipv6=1;
	memset(dhcpClient_info.hw_info.gateway_mac_addr_for_ipv6.octet, 0x0, ETHER_ADDR_LEN);	
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_set_dhcpclientinfo_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr */

/*
 * rg set dhcpClientInfo-ipv6 ipv6_napt_enable <UINT:ipv6_napt_enable> ipv6_addr <IPV6ADDR:ipv6_addr> ipv6_mask_length <UINT:ipv6_mask_length> ipv6_default_gateway_on <UINT:ipv6_default_gateway_on> gateway_ipv6_addr <IPV6ADDR:gateway_ipv6_addr> gw_mac_auto_learn_for_ipv6 <UINT:gw_mac_auto_learn_for_ipv6> gateway_mac_addr_for_ipv6 <MACADDR:gateway_mac_addr_for_ipv6>
 */
cparser_result_t
cparser_cmd_rg_set_dhcpClientInfo_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6(
    cparser_context_t *context,
    uint32_t  *ipv6_napt_enable_ptr,
    char * *ipv6_addr_ptr,
    uint32_t  *ipv6_mask_length_ptr,
    uint32_t  *ipv6_default_gateway_on_ptr,
    char * *gateway_ipv6_addr_ptr,
    uint32_t  *gw_mac_auto_learn_for_ipv6_ptr,
    cparser_macaddr_t  *gateway_mac_addr_for_ipv6_ptr)
{
	int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
	dhcpClient_info.hw_info.ipv6_napt_enable = *ipv6_napt_enable_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&dhcpClient_info.hw_info.ipv6_addr.ipv6_addr[0], TOKEN_STR(6)), ret);
    dhcpClient_info.hw_info.ipv6_mask_length=*ipv6_mask_length_ptr;
    dhcpClient_info.hw_info.ipv6_default_gateway_on=*ipv6_default_gateway_on_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&dhcpClient_info.hw_info.gateway_ipv6_addr.ipv6_addr[0], TOKEN_STR(12)), ret);

	
	dhcpClient_info.hw_info.gw_mac_auto_learn_for_ipv6=*gw_mac_auto_learn_for_ipv6_ptr;
	osal_memcpy(dhcpClient_info.hw_info.gateway_mac_addr_for_ipv6.octet, gateway_mac_addr_for_ipv6_ptr->octet, ETHER_ADDR_LEN);
	
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_set_dhcpclientinfo_ipv6_ipv6_napt_enable_ipv6_napt_enable_ipv6_addr_ipv6_addr_ipv6_mask_length_ipv6_mask_length_ipv6_default_gateway_on_ipv6_default_gateway_on_gateway_ipv6_addr_gateway_ipv6_addr_gw_mac_auto_learn_for_ipv6_gw_mac_auto_learn_for_ipv6_gateway_mac_addr_for_ipv6_gateway_mac_addr_for_ipv6 */

/*
 * rg show binding
 */
cparser_result_t
cparser_cmd_rg_show_binding(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	_diag_bindingShow(&vlan_binding_info);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_binding */

/*
 * rg clear binding
 */
cparser_result_t
cparser_cmd_rg_clear_binding(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&vlan_binding_info, 0x0, sizeof(rtk_rg_vlanBinding_t)); 

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_binding */

/*
 * rg add binding entry
 */
cparser_result_t
cparser_cmd_rg_add_binding_entry(
    cparser_context_t *context)
{
	int ret;
	int index;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rtk_rg_vlanBinding_add(&vlan_binding_info, &index);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_vlanBinding_add failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_vlanBinding_add[%d] success.\n",index);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_binding_entry */

/*
 * rg get binding entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_get_binding_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int ret;
	int index=*entry_index_ptr;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&vlan_binding_info, 0x0, sizeof(rtk_rg_vlanBinding_t)); 

	ret = rtk_rg_vlanBinding_find(&vlan_binding_info, &index);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get vlan_binding_info failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		if(index==*entry_index_ptr){	
			_diag_bindingShow(&vlan_binding_info);
		}
		else{
			diag_util_mprintf("vlan_binding_info[%d] is empty.\n",*entry_index_ptr);
			return CPARSER_NOT_OK;
		}
	}
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_binding_entry_entry_index */

/*
 * rg del binding entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_del_binding_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
  
	int ret;
	DIAG_UTIL_PARAM_CHK();
	
	ret = rtk_rg_vlanBinding_del(*entry_index_ptr);
	if(ret!=RT_ERR_RG_OK)
		return CPARSER_NOT_OK;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_del_binding_entry_entry_index */

/*
 * rg set binding port_idx <UINT:port_idx> ingress_vid <UINT:ingress_vid> wan_intf_idx <UINT:wan_intf_idx>
 */
cparser_result_t
cparser_cmd_rg_set_binding_port_idx_port_idx_ingress_vid_ingress_vid_wan_intf_idx_wan_intf_idx(
    cparser_context_t *context,
    uint32_t  *port_idx_ptr,
    uint32_t  *ingress_vid_ptr,
    uint32_t  *wan_intf_idx_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	vlan_binding_info.port_idx = *port_idx_ptr;
	vlan_binding_info.ingress_vid = *ingress_vid_ptr;
	vlan_binding_info.wan_intf_idx = *wan_intf_idx_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_binding_port_idx_port_idx_ingress_vid_ingress_vid_wan_intf_idx_wan_intf_idx */

           


/*
 * rg show macfilter
 */
cparser_result_t
cparser_cmd_rg_show_macfilter(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	_diag_macFilterEntryShow(&macFilterEntry);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_macfilter */

/*
 * rg clear macfilter
 */
cparser_result_t
cparser_cmd_rg_clear_macfilter(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&macFilterEntry, 0x0, sizeof(rtk_rg_macFilterEntry_t)); 

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_macfilter */

/*
 * rg add macfilter entry
 */
cparser_result_t
cparser_cmd_rg_add_macfilter_entry(
    cparser_context_t *context)
{
	int ret;
	int index;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rtk_rg_macFilter_add(&macFilterEntry,&index);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_macFilter_add failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_macFilter_add[%d] success.\n",index);
	}

    return CPARSER_OK;

}    /* end of cparser_cmd_rg_add_macfilter_entry */

/*
 * rg get macfilter entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_get_macfilter_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int ret;
	int index=*entry_index_ptr;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&macFilterEntry, 0x0, sizeof(rtk_rg_macFilterEntry_t)); 

	ret = rtk_rg_macFilter_find(&macFilterEntry,&index);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get rtk_rg_macFilter_find failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		if(index==*entry_index_ptr){	
			_diag_macFilterEntryShow(&macFilterEntry);
		}
		else{
			diag_util_mprintf("macFilterEntry[%d] is empty.\n",*entry_index_ptr);
			return CPARSER_NOT_OK;
		}
	}
	
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_get_macfilter_entry_entry_index */

/*
 * rg del macfilter entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_del_macfilter_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
  
	int ret;
	DIAG_UTIL_PARAM_CHK();
	
	ret = rtk_rg_macFilter_del(*entry_index_ptr);
	if(ret!=RT_ERR_RG_OK)
		return CPARSER_NOT_OK;

    return CPARSER_OK;

}    /* end of cparser_cmd_rg_del_macfilter_entry_entry_index */

/*
 * rg set macfilter mac <MACADDR:mac> direct <UINT:direct>
 */
cparser_result_t
cparser_cmd_rg_set_macfilter_mac_mac_direct_direct(
    cparser_context_t *context,
    cparser_macaddr_t  *mac_ptr,
    uint32_t  *direct_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	osal_memcpy(macFilterEntry.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
	macFilterEntry.direct =  *direct_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_macfilter_mac_mac_direct_direct */

/*
 * rg set macfilter mac <MACADDR:mac> direct <UINT:direct> ivl_svl <UINT:ivl_svl> vid <UINT:vid>
 */
cparser_result_t
cparser_cmd_rg_set_macfilter_mac_mac_direct_direct_ivl_svl_ivl_svl_vid_vid(
    cparser_context_t *context,
    cparser_macaddr_t  *mac_ptr,
    uint32_t  *direct_ptr,
    uint32_t  *ivl_svl_ptr,
    uint32_t  *vid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memcpy(macFilterEntry.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
	macFilterEntry.direct =  *direct_ptr;
	macFilterEntry.isIVL = *ivl_svl_ptr;
	macFilterEntry.vlan_id = *vid_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_macfilter_mac_mac_direct_direct_ivl_svl_ivl_svl_vid_vid */

/*
 * rg show virtualServer
 */
cparser_result_t
cparser_cmd_rg_show_virtualServer(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	_diag_virtualServerShow(&virtual_server);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_virtualserver */

/*
 * rg clear virtualServer
 */
cparser_result_t
cparser_cmd_rg_clear_virtualServer(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&virtual_server, 0x0, sizeof(rtk_rg_virtualServer_t)); 

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_virtualserver */

/*
 * rg add virtualServer entry
 */
cparser_result_t
cparser_cmd_rg_add_virtualServer_entry(
    cparser_context_t *context)
{
 	int ret;
	int index;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rtk_rg_virtualServer_add(&virtual_server, &index);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_virtualServer_add failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_virtualServer_add[%d] success.\n",index);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_virtualserver_entry */

/*
 * rg get virtualServer entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_get_virtualServer_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int ret;
	int index=*entry_index_ptr;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&virtual_server, 0x0, sizeof(rtk_rg_virtualServer_t)); 

	ret = rtk_rg_virtualServer_find(&virtual_server,&index);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get rtk_rg_virtualServer_find failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		if(index==*entry_index_ptr){	
			_diag_virtualServerShow(&virtual_server);
		}
		else{
			diag_util_mprintf("virtual_server[%d] is empty.\n",*entry_index_ptr);
			return CPARSER_NOT_OK;
		}
	}
	
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_get_virtualserver_entry_entry_index */

/*
 * rg del virtualServer entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_del_virtualServer_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int ret;
	DIAG_UTIL_PARAM_CHK();
	
	ret = rtk_rg_virtualServer_del(*entry_index_ptr);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_virtualServer_del failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_virtualServer_del[%d] success.\n",*entry_index_ptr);
		return CPARSER_OK;
	}	

}    /* end of cparser_cmd_rg_del_virtualserver_entry_entry_index */

/*
 * rg set virtualServer is_tcp <UINT:is_tcp> wan_intf_idx <UINT:wan_intf_idx> gateway_port_start <UINT:gateway_port_start> local_ip <IPV4ADDR:local_ip> local_port_start <UINT:local_port_start> mappingPortRangeCnt <UINT:mappingPortRangeCnt> mappingType <UINT:mappingType> valid <UINT:valid>
 */
cparser_result_t
cparser_cmd_rg_set_virtualServer_is_tcp_is_tcp_wan_intf_idx_wan_intf_idx_gateway_port_start_gateway_port_start_local_ip_local_ip_local_port_start_local_port_start_mappingPortRangeCnt_mappingPortRangeCnt_mappingType_mappingType_valid_valid(
    cparser_context_t *context,
    uint32_t  *is_tcp_ptr,
    uint32_t  *wan_intf_idx_ptr,
    uint32_t  *gateway_port_start_ptr,
    uint32_t  *local_ip_ptr,
    uint32_t  *local_port_start_ptr,
    uint32_t  *mappingPortRangeCnt_ptr,
    uint32_t  *mappingType_ptr,
    uint32_t  *valid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	virtual_server.is_tcp = *is_tcp_ptr;
	virtual_server.wan_intf_idx = *wan_intf_idx_ptr;
	virtual_server.gateway_port_start = *gateway_port_start_ptr;
	virtual_server.local_ip = *local_ip_ptr;
	virtual_server.local_port_start = *local_port_start_ptr;
	virtual_server.mappingPortRangeCnt = *mappingPortRangeCnt_ptr;
    virtual_server.mappingType = *mappingType_ptr;
	virtual_server.valid = *valid_ptr;
	virtual_server.hookAlgType = 0x0;
	virtual_server.disable_wan_check = 0x0;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_virtualserver_is_tcp_is_tcp_wan_intf_idx_wan_intf_idx_gateway_port_start_gateway_port_start_local_ip_local_ip_local_port_start_local_port_start_mappingportrangecnt_mappingportrangecnt_mappingtype_mappingtype_valid_valid */

/*
 * rg set virtualServer ipversion <UINT:ipversion> is_tcp <UINT:is_tcp> wan_intf_idx <UINT:wan_intf_idx> gateway_port_start <UINT:gateway_port_start> local_ip <IPV4ADDR:local_ip> local_ipv6 <IPV6ADDR:local_ipv6>  local_port_start <UINT:local_port_start> mappingPortRangeCnt <UINT:mappingPortRangeCnt> mappingType <UINT:mappingType> valid <UINT:valid>
 */
cparser_result_t
cparser_cmd_rg_set_virtualServer_ipversion_ipversion_is_tcp_is_tcp_wan_intf_idx_wan_intf_idx_gateway_port_start_gateway_port_start_local_ip_local_ip_local_ipv6_local_ipv6_local_port_start_local_port_start_mappingPortRangeCnt_mappingPortRangeCnt_mappingType_mappingType_valid_valid(
    cparser_context_t *context,
    uint32_t  *ipversion_ptr,
    uint32_t  *is_tcp_ptr,
    uint32_t  *wan_intf_idx_ptr,
    uint32_t  *gateway_port_start_ptr,
    uint32_t  *local_ip_ptr,
    char * *local_ipv6_ptr,
    uint32_t  *local_port_start_ptr,
    uint32_t  *mappingPortRangeCnt_ptr,
    uint32_t  *mappingType_ptr,
    uint32_t  *valid_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	virtual_server.ipversion = *ipversion_ptr;
	virtual_server.is_tcp = *is_tcp_ptr;
	virtual_server.wan_intf_idx = *wan_intf_idx_ptr;
	virtual_server.gateway_port_start = *gateway_port_start_ptr;
	virtual_server.local_ip = *local_ip_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&virtual_server.local_ipv6.ipv6_addr[0], TOKEN_STR(14)), ret);
	virtual_server.local_port_start = *local_port_start_ptr;
	virtual_server.mappingPortRangeCnt = *mappingPortRangeCnt_ptr;
    virtual_server.mappingType = *mappingType_ptr;
	virtual_server.valid = *valid_ptr;
	virtual_server.hookAlgType = 0x0;
	virtual_server.disable_wan_check = 0x0;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_virtualserver_ipversion_ipversion_is_tcp_is_tcp_wan_intf_idx_wan_intf_idx_gateway_port_start_gateway_port_start_local_ip_local_ip_local_ipv6_local_ipv6_local_port_start_local_port_start_mappingportrangecnt_mappingportrangecnt_mappingtype_mappingtype_valid_valid */

/*
 * rg set virtualServer is_tcp <UINT:is_tcp> wan_intf_idx <UINT:wan_intf_idx> gateway_port_start <UINT:gateway_port_start> local_ip <IPV4ADDR:local_ip> local_port_start <UINT:local_port_start> mappingPortRangeCnt <UINT:mappingPortRangeCnt> mappingType <UINT:mappingType> valid <UINT:valid> hookAlgType <HEX:hookAlgType> disable_wan_check <UINT:disable_wan_check>
 */
cparser_result_t
cparser_cmd_rg_set_virtualServer_is_tcp_is_tcp_wan_intf_idx_wan_intf_idx_gateway_port_start_gateway_port_start_local_ip_local_ip_local_port_start_local_port_start_mappingPortRangeCnt_mappingPortRangeCnt_mappingType_mappingType_valid_valid_hookAlgType_hookAlgType_disable_wan_check_disable_wan_check(
    cparser_context_t *context,
    uint32_t  *is_tcp_ptr,
    uint32_t  *wan_intf_idx_ptr,
    uint32_t  *gateway_port_start_ptr,
    uint32_t  *local_ip_ptr,
    uint32_t  *local_port_start_ptr,
    uint32_t  *mappingPortRangeCnt_ptr,
    uint32_t  *mappingType_ptr,
    uint32_t  *valid_ptr,
    uint32_t  *hookAlgType_ptr,
    uint32_t  *disable_wan_check_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	virtual_server.is_tcp = *is_tcp_ptr;
	virtual_server.wan_intf_idx = *wan_intf_idx_ptr;
	virtual_server.gateway_port_start = *gateway_port_start_ptr;
	virtual_server.local_ip = *local_ip_ptr;
	virtual_server.local_port_start = *local_port_start_ptr;
	virtual_server.mappingPortRangeCnt = *mappingPortRangeCnt_ptr;
    virtual_server.mappingType = *mappingType_ptr;
	virtual_server.valid = *valid_ptr;
	virtual_server.hookAlgType = *hookAlgType_ptr;
	virtual_server.disable_wan_check = *disable_wan_check_ptr;
		
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_virtualserver_is_tcp_is_tcp_wan_intf_idx_wan_intf_idx_gateway_port_start_gateway_port_start_local_ip_local_ip_local_port_start_local_port_start_mappingportrangecnt_mappingportrangecnt_mappingtype_mappingtype_valid_valid_hookalgtype_hookalgtype_disable_wan_check_disable_wan_check */

/*
 * rg set virtualServer ipversion <UINT:ipversion> is_tcp <UINT:is_tcp> wan_intf_idx <UINT:wan_intf_idx> gateway_port_start <UINT:gateway_port_start> local_ip <IPV4ADDR:local_ip> local_ipv6 <IPV6ADDR:local_ipv6>  local_port_start <UINT:local_port_start> mappingPortRangeCnt <UINT:mappingPortRangeCnt> mappingType <UINT:mappingType> valid <UINT:valid> hookAlgType <HEX:hookAlgType> disable_wan_check <UINT:disable_wan_check>
 */
cparser_result_t
cparser_cmd_rg_set_virtualServer_ipversion_ipversion_is_tcp_is_tcp_wan_intf_idx_wan_intf_idx_gateway_port_start_gateway_port_start_local_ip_local_ip_local_ipv6_local_ipv6_local_port_start_local_port_start_mappingPortRangeCnt_mappingPortRangeCnt_mappingType_mappingType_valid_valid_hookAlgType_hookAlgType_disable_wan_check_disable_wan_check(
    cparser_context_t *context,
    uint32_t  *ipversion_ptr,
    uint32_t  *is_tcp_ptr,
    uint32_t  *wan_intf_idx_ptr,
    uint32_t  *gateway_port_start_ptr,
    uint32_t  *local_ip_ptr,
    char * *local_ipv6_ptr,
    uint32_t  *local_port_start_ptr,
    uint32_t  *mappingPortRangeCnt_ptr,
    uint32_t  *mappingType_ptr,
    uint32_t  *valid_ptr,
    uint32_t  *hookAlgType_ptr,
    uint32_t  *disable_wan_check_ptr)
{
    int ret;
    DIAG_UTIL_PARAM_CHK();
	virtual_server.ipversion = *ipversion_ptr;
	virtual_server.is_tcp = *is_tcp_ptr;
	virtual_server.wan_intf_idx = *wan_intf_idx_ptr;
	virtual_server.gateway_port_start = *gateway_port_start_ptr;
	virtual_server.local_ip = *local_ip_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&virtual_server.local_ipv6.ipv6_addr[0], TOKEN_STR(14)), ret);
	virtual_server.local_port_start = *local_port_start_ptr;
	virtual_server.mappingPortRangeCnt = *mappingPortRangeCnt_ptr;
    virtual_server.mappingType = *mappingType_ptr;
	virtual_server.valid = *valid_ptr;
	virtual_server.hookAlgType = *hookAlgType_ptr;
	virtual_server.disable_wan_check = *disable_wan_check_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_virtualserver_ipversion_ipversion_is_tcp_is_tcp_wan_intf_idx_wan_intf_idx_gateway_port_start_gateway_port_start_local_ip_local_ip_local_ipv6_local_ipv6_local_port_start_local_port_start_mappingportrangecnt_mappingportrangecnt_mappingtype_mappingtype_valid_valid_hookalgtype_hookalgtype_disable_wan_check_disable_wan_check */


/*
 * rg show cvlan
 */
cparser_result_t
cparser_cmd_rg_show_cvlan(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	_diag_cvlanInfoShow(&cvlan_info);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_cvlan */

/*
 * rg clear cvlan
 */
cparser_result_t
cparser_cmd_rg_clear_cvlan(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&cvlan_info, 0x0, sizeof(rtk_rg_cvlan_info_t));

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_cvlan */

/*
 * rg add cvlan entry
 */
cparser_result_t
cparser_cmd_rg_add_cvlan_entry(
    cparser_context_t *context)
{
 	int ret;
	int index;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret =  rtk_rg_cvlan_add(&cvlan_info);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_cvlan_add failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_cvlan_add[%d] success.\n",cvlan_info.vlanId);
	}

    return CPARSER_OK;

}    /* end of cparser_cmd_rg_add_cvlan_entry */

/*
 * rg set cvlan vlanId <UINT:vlanId> isIVL <UINT:isIVL> memberPortMask <HEX:memberPortMask> untagPortMask <HEX:untagPortMask> vlan-based-pri-enable enable vlan-based-pri <UINT:vlan_based_pri>
 */
cparser_result_t
cparser_cmd_rg_set_cvlan_vlanId_vlanId_isIVL_isIVL_memberPortMask_memberPortMask_untagPortMask_untagPortMask_vlan_based_pri_enable_enable_vlan_based_pri_vlan_based_pri(
    cparser_context_t *context,
    uint32_t  *vlanId_ptr,
    uint32_t  *isIVL_ptr,
    uint32_t  *memberPortMask_ptr,
    uint32_t  *untagPortMask_ptr,
    uint32_t  *vlan_based_pri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	cvlan_info.vlanId=*vlanId_ptr;
	cvlan_info.isIVL=*isIVL_ptr;
	cvlan_info.memberPortMask.portmask=*memberPortMask_ptr;
	cvlan_info.untagPortMask.portmask=*untagPortMask_ptr;
	cvlan_info.vlan_based_pri_enable=RTK_RG_ENABLED;
	cvlan_info.vlan_based_pri=*vlan_based_pri_ptr;
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_cvlan_vlanid_vlanid_isivl_isivl_memberportmask_memberportmask_untagportmask_untagportmask_vlan_based_pri_enable_enable_vlan_based_pri_vlan_based_pri */

/*
 * rg set cvlan vlanId <UINT:vlanId> isIVL <UINT:isIVL> memberPortMask <HEX:memberPortMask> untagPortMask <HEX:untagPortMask> vlan-based-pri-enable disable
 */
cparser_result_t
cparser_cmd_rg_set_cvlan_vlanId_vlanId_isIVL_isIVL_memberPortMask_memberPortMask_untagPortMask_untagPortMask_vlan_based_pri_enable_disable(
    cparser_context_t *context,
    uint32_t  *vlanId_ptr,
    uint32_t  *isIVL_ptr,
    uint32_t  *memberPortMask_ptr,
    uint32_t  *untagPortMask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	cvlan_info.vlanId=*vlanId_ptr;
	cvlan_info.isIVL=*isIVL_ptr;
	cvlan_info.memberPortMask.portmask=*memberPortMask_ptr;
	cvlan_info.untagPortMask.portmask=*untagPortMask_ptr;
	cvlan_info.vlan_based_pri_enable=RTK_RG_DISABLED;
	cvlan_info.vlan_based_pri=-1;
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_cvlan_vlanid_vlanid_isivl_isivl_memberportmask_memberportmask_untagportmask_untagportmask_vlan_based_pri_enable_disable */

/*
 * rg del cvlan vlanId <UINT:vlanId>
 */
cparser_result_t
cparser_cmd_rg_del_cvlan_vlanId_vlanId(
    cparser_context_t *context,
    uint32_t  *vlanId_ptr)
{
	int ret;
	DIAG_UTIL_PARAM_CHK();
	
	ret = rtk_rg_cvlan_del(*vlanId_ptr);
	if(ret!=RT_ERR_RG_OK)
		return CPARSER_NOT_OK;
	return CPARSER_OK;
}    /* end of cparser_cmd_rg_del_cvlan_vlanid_vlanid */

/*
 * rg get cvlan vlanId <UINT:vlanId>
 */
cparser_result_t
cparser_cmd_rg_get_cvlan_vlanId_vlanId(
    cparser_context_t *context,
    uint32_t  *vlanId_ptr)
{
    int ret;
	int index;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&cvlan_info, 0x0, sizeof(rtk_rg_cvlan_info_t));
	cvlan_info.vlanId=*vlanId_ptr;
	ret =  rtk_rg_cvlan_get(&cvlan_info);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_cvlan_get failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		_diag_cvlanInfoShow(&cvlan_info);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_cvlan_vlanid_vlanid */


/*
 * rg set port-based-vlan port_idx <UINT:port_idx> vlanId <UINT:vlanId>
 */
cparser_result_t
cparser_cmd_rg_set_port_based_vlan_port_idx_port_idx_vlanId_vlanId(
    cparser_context_t *context,
    uint32_t  *port_idx_ptr,
    uint32_t  *vlanId_ptr)
{
    int ret;
    DIAG_UTIL_PARAM_CHK();

    ret = rtk_rg_portBasedCVlanId_set(*port_idx_ptr,*vlanId_ptr);
    if(ret!=RT_ERR_RG_OK)
    {
        diag_util_mprintf("set portBasedCVlanId failed. rg API return %x \n",ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("Port[%d] set Pvid[%d] success!\n",*port_idx_ptr,*vlanId_ptr);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_port_based_vlan_port_idx_port_idx_vlanid_vlanid */

/*
 * rg get port-based-vlan port_idx <UINT:port_idx>
 */
cparser_result_t
cparser_cmd_rg_get_port_based_vlan_port_idx_port_idx(
    cparser_context_t *context,
    uint32_t  *port_idx_ptr)
{
    int ret,pvid;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_rg_portBasedCVlanId_get(*port_idx_ptr,&pvid);
    if(ret!=RT_ERR_RG_OK)
    {
        diag_util_mprintf("get portBasedCVlanId failed. rg API return %x \n",ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("Port[%d] Pvid[%d]\n",*port_idx_ptr,pvid);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_port_based_vlan_port_idx_port_idx */

/*
 * rg set wlan-dev-based-vlan wlan_idx <UINT:wlan_idx> dev_idx <UINT:dev_idx> vlanId <UINT:vlanId>
 */
cparser_result_t
cparser_cmd_rg_set_wlan_dev_based_vlan_wlan_idx_wlan_idx_dev_idx_dev_idx_vlanId_vlanId(
    cparser_context_t *context,
    uint32_t  *wlan_idx_ptr,
    uint32_t  *dev_idx_ptr,
    uint32_t  *vlanId_ptr)
{
    int ret;
    DIAG_UTIL_PARAM_CHK();

    ret = rtk_rg_wlanDevBasedCVlanId_set(*wlan_idx_ptr,*dev_idx_ptr,*vlanId_ptr);
    if(ret!=RT_ERR_RG_OK)
    {
        diag_util_mprintf("set wlanDevBasedCVlanId failed. rg API return %x \n",ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("Wlan%d Dev[%d] set Dvid[%d] success!\n",*wlan_idx_ptr,*dev_idx_ptr,*vlanId_ptr);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_wlan_dev_based_vlan_wlan_idx_wlan_idx_dev_idx_dev_idx_vlanid_vlanid */

/*
 * rg get wlan-dev-based-vlan wlan_idx <UINT:wlan_idx> dev_idx <UINT:dev_idx>
 */
cparser_result_t
cparser_cmd_rg_get_wlan_dev_based_vlan_wlan_idx_wlan_idx_dev_idx_dev_idx(
    cparser_context_t *context,
    uint32_t  *wlan_idx_ptr,
    uint32_t  *dev_idx_ptr)
{
    int ret,dvid;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_rg_wlanDevBasedCVlanId_get(*wlan_idx_ptr,*dev_idx_ptr,&dvid);
    if(ret!=RT_ERR_RG_OK)
    {
        diag_util_mprintf("get wlanDevBasedCVlanId failed. rg API return %x \n",ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("Wlan%d Dev[%d] Dvid[%d]\n",*wlan_idx_ptr,*dev_idx_ptr,dvid);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_wlan_dev_based_vlan_wlan_idx_wlan_idx_dev_idx_dev_idx */

/*
 * rg add wanAccessLimitUnlimit
 */
cparser_result_t
cparser_cmd_rg_add_wanAccessLimitUnlimit(
    cparser_context_t *context)
{
    int ret;
	rtk_rg_accessWanLimitData_t access_wan_info;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	
	osal_memset(&access_wan_info, 0x0, sizeof(rtk_rg_accessWanLimitData_t)); 
	access_wan_info.type=RG_ACCESSWAN_TYPE_UNLIMIT;
	ret = rtk_rg_accessWanLimit_set(access_wan_info);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_accessWanLimit_set failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_accessWanLimit_set success.\n");
	}
    
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_wanaccesslimitunlimit */

/*
 * rg add wanAccessLimitPortMask port_mask <HEX:port_mask> learningLimitNumber unlimit action <UINT:action>
 */
cparser_result_t
cparser_cmd_rg_add_wanAccessLimitPortMask_port_mask_port_mask_learningLimitNumber_unlimit_action_action(
    cparser_context_t *context,
    uint32_t  *port_mask_ptr,
    uint32_t  *action_ptr)
{
    int ret;
	rtk_rg_accessWanLimitData_t access_wan_info;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	
	osal_memset(&access_wan_info, 0x0, sizeof(rtk_rg_accessWanLimitData_t)); 
	access_wan_info.type=RG_ACCESSWAN_TYPE_PORTMASK;
	access_wan_info.data.port_mask.portmask=*port_mask_ptr;
	access_wan_info.learningLimitNumber=-1;
	access_wan_info.action=*action_ptr;
	ret = rtk_rg_accessWanLimit_set(access_wan_info);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_accessWanLimit_set failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_accessWanLimit_set success.\n");
	}
    
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_wanaccesslimitportmask_port_mask_port_mask_learninglimitnumber_unlimit_action_action */

/*
 * rg add wanAccessLimitPortMask port_mask <HEX:port_mask> learningLimitNumber <UINT:learningLimitNumber> action <UINT:action>
 */
cparser_result_t
cparser_cmd_rg_add_wanAccessLimitPortMask_port_mask_port_mask_learningLimitNumber_learningLimitNumber_action_action(
    cparser_context_t *context,
    uint32_t  *port_mask_ptr,
    uint32_t  *learningLimitNumber_ptr,
    uint32_t  *action_ptr)
{
	int ret;
	rtk_rg_accessWanLimitData_t access_wan_info;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	
	osal_memset(&access_wan_info, 0x0, sizeof(rtk_rg_accessWanLimitData_t)); 
	access_wan_info.type=RG_ACCESSWAN_TYPE_PORTMASK;
	access_wan_info.data.port_mask.portmask=*port_mask_ptr;
	access_wan_info.learningLimitNumber=*learningLimitNumber_ptr;
	access_wan_info.action=*action_ptr;
	ret = rtk_rg_accessWanLimit_set(access_wan_info);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_accessWanLimit_set failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_accessWanLimit_set success.\n");
	}
    
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_wanaccesslimitportmask_port_mask_port_mask_learninglimitnumber_learninglimitnumber_action_action */

/*
 * rg add wanAccessLimitPortMask port_mask <HEX:port_mask> wlan_idx <UINT:wlan_idx> wlan_dev_mask <HEX:wlan_dev_mask> learningLimitNumber unlimit action <UINT:action>
 */
cparser_result_t
cparser_cmd_rg_add_wanAccessLimitPortMask_port_mask_port_mask_wlan_idx_wlan_idx_wlan_dev_mask_wlan_dev_mask_learningLimitNumber_unlimit_action_action(
    cparser_context_t *context,
    uint32_t  *port_mask_ptr,
    uint32_t  *wlan_idx_ptr,
    uint32_t  *wlan_dev_mask_ptr,
    uint32_t  *action_ptr)
{
    int ret;
	rtk_rg_accessWanLimitData_t access_wan_info;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	
	osal_memset(&access_wan_info, 0x0, sizeof(rtk_rg_accessWanLimitData_t)); 
	access_wan_info.type=RG_ACCESSWAN_TYPE_PORTMASK;
	access_wan_info.data.port_mask.portmask=*port_mask_ptr;
	access_wan_info.learningLimitNumber=-1;
	access_wan_info.action=*action_ptr;
	if(*wlan_idx_ptr!=0)
	{
		diag_util_mprintf("add rtk_rg_accessWanLimit_set failed. only support wlan 0!\n");
		return CPARSER_NOT_OK;
	}
#ifdef CONFIG_MASTER_WLAN0_ENABLE	
	access_wan_info.wlan0_dev_mask=*wlan_dev_mask_ptr;
#endif
	ret = rtk_rg_accessWanLimit_set(access_wan_info);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_accessWanLimit_set failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_accessWanLimit_set success.\n");
	}
    
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_wanaccesslimitportmask_port_mask_port_mask_wlan_idx_wlan_idx_wlan_dev_mask_wlan_dev_mask_learninglimitnumber_unlimit_action_action */

/*
 * rg add wanAccessLimitPortMask port_mask <HEX:port_mask> wlan_idx <UINT:wlan_idx> wlan_dev_mask <HEX:wlan_dev_mask> learningLimitNumber <UINT:learningLimitNumber> action <UINT:action>
 */
cparser_result_t
cparser_cmd_rg_add_wanAccessLimitPortMask_port_mask_port_mask_wlan_idx_wlan_idx_wlan_dev_mask_wlan_dev_mask_learningLimitNumber_learningLimitNumber_action_action(
    cparser_context_t *context,
    uint32_t  *port_mask_ptr,
    uint32_t  *wlan_idx_ptr,
    uint32_t  *wlan_dev_mask_ptr,
    uint32_t  *learningLimitNumber_ptr,
    uint32_t  *action_ptr)
{
    int ret;
	rtk_rg_accessWanLimitData_t access_wan_info;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	
	osal_memset(&access_wan_info, 0x0, sizeof(rtk_rg_accessWanLimitData_t)); 
	access_wan_info.type=RG_ACCESSWAN_TYPE_PORTMASK;
	access_wan_info.data.port_mask.portmask=*port_mask_ptr;
	access_wan_info.learningLimitNumber=*learningLimitNumber_ptr;
	access_wan_info.action=*action_ptr;
	if(*wlan_idx_ptr!=0)
	{
		diag_util_mprintf("add rtk_rg_accessWanLimit_set failed. only support wlan 0!\n");
		return CPARSER_NOT_OK;
	}
#ifdef CONFIG_MASTER_WLAN0_ENABLE	
	access_wan_info.wlan0_dev_mask=*wlan_dev_mask_ptr;
#endif
	ret = rtk_rg_accessWanLimit_set(access_wan_info);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_accessWanLimit_set failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_accessWanLimit_set success.\n");
	}
    
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_wanaccesslimitportmask_port_mask_port_mask_wlan_idx_wlan_idx_wlan_dev_mask_wlan_dev_mask_learninglimitnumber_learninglimitnumber_action_action */

/*
 * rg add wanAccessLimitCategory category <UINT:category> learningLimitNumber unlimit action <UINT:action>
 */
cparser_result_t
cparser_cmd_rg_add_wanAccessLimitCategory_category_category_learningLimitNumber_unlimit_action_action(
    cparser_context_t *context,
    uint32_t  *category_ptr,
    uint32_t  *action_ptr)
{
    int ret;
	rtk_rg_accessWanLimitData_t access_wan_info;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	
	osal_memset(&access_wan_info, 0x0, sizeof(rtk_rg_accessWanLimitData_t)); 
	access_wan_info.type=RG_ACCESSWAN_TYPE_CATEGORY;
	access_wan_info.data.category=*category_ptr;
	access_wan_info.learningLimitNumber=-1;
	access_wan_info.action=*action_ptr;
	ret = rtk_rg_accessWanLimit_set(access_wan_info);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_accessWanLimit_set failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_accessWanLimit_set success.\n");
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_wanaccesslimitcategory_category_category_learninglimitnumber_unlimit_action_action */

/*
 * rg add wanAccessLimitCategory category <UINT:category> learningLimitNumber <UINT:learningLimitNumber> action <UINT:action>
 */
cparser_result_t
cparser_cmd_rg_add_wanAccessLimitCategory_category_category_learningLimitNumber_learningLimitNumber_action_action(
    cparser_context_t *context,
    uint32_t  *category_ptr,
    uint32_t  *learningLimitNumber_ptr,
    uint32_t  *action_ptr)
{
    int ret;
	rtk_rg_accessWanLimitData_t access_wan_info;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	
	osal_memset(&access_wan_info, 0x0, sizeof(rtk_rg_accessWanLimitData_t)); 
	access_wan_info.type=RG_ACCESSWAN_TYPE_CATEGORY;
	access_wan_info.data.category=*category_ptr;
	access_wan_info.learningLimitNumber=*learningLimitNumber_ptr;
	access_wan_info.action=*action_ptr;
	ret = rtk_rg_accessWanLimit_set(access_wan_info);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_accessWanLimit_set failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_accessWanLimit_set success.\n");
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_wanaccesslimitcategory_category_category_learninglimitnumber_learninglimitnumber_action_action */

/*
 * rg set wanAccessLimitCategory mac <MACADDR:mac> category <UINT:category>
 */
cparser_result_t
cparser_cmd_rg_set_wanAccessLimitCategory_mac_mac_category_category(
    cparser_context_t *context,
    cparser_macaddr_t  *mac_ptr,
    uint32_t  *category_ptr)
{
	int ret;
	rtk_rg_accessWanLimitCategory_t macCategory_info;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	
	osal_memset(&macCategory_info, 0x0, sizeof(rtk_rg_accessWanLimitCategory_t)); 
	osal_memcpy(macCategory_info.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
	macCategory_info.category=*category_ptr;
	ret = rtk_rg_accessWanLimitCategory_set(macCategory_info);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_accessWanLimitCategory_set failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_accessWanLimitCategory_set success.\n");
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_wanaccesslimitcategory_mac_mac_category_category */

/*
 * rg get wanAccessLimitPortMask
 */
cparser_result_t
cparser_cmd_rg_get_wanAccessLimitPortMask(
    cparser_context_t *context)
{
    int ret;
	rtk_rg_accessWanLimitData_t access_wan_info;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&access_wan_info, 0x0, sizeof(rtk_rg_accessWanLimitData_t)); 

	access_wan_info.type=RG_ACCESSWAN_TYPE_PORTMASK;

	ret = rtk_rg_accessWanLimit_get(&access_wan_info);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get rtk_rg_accessWanLimit_get for portmask failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{	
		diag_util_mprintf("port_mask: 0x%x\n",access_wan_info.data.port_mask.portmask);
		diag_util_mprintf("learningLimitNumber: %d\n",access_wan_info.learningLimitNumber);
		diag_util_mprintf("learningCount: %d\n",access_wan_info.learningCount);
		diag_util_mprintf("action: %d\n",access_wan_info.action);
		return CPARSER_OK;
	}
}    /* end of cparser_cmd_rg_get_wanaccesslimitportmask */

/*
 * rg get wanAccessLimitCategory category <UINT:category>
 */
cparser_result_t
cparser_cmd_rg_get_wanAccessLimitCategory_category_category(
    cparser_context_t *context,
    uint32_t  *category_ptr)
{
    int ret;
	rtk_rg_accessWanLimitData_t access_wan_info;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&access_wan_info, 0x0, sizeof(rtk_rg_accessWanLimitData_t)); 

	access_wan_info.type=RG_ACCESSWAN_TYPE_CATEGORY;
	access_wan_info.data.category=*category_ptr;

	ret = rtk_rg_accessWanLimit_get(&access_wan_info);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get rtk_rg_accessWanLimit_get for category[%d] failed. rg API return %x \n",*category_ptr,ret);
		return CPARSER_NOT_OK;
	}
	else
	{	
		diag_util_mprintf("category: 0x%x\n",access_wan_info.data.category);
		diag_util_mprintf("learningLimitNumber: %d\n",access_wan_info.learningLimitNumber);
		diag_util_mprintf("learningCount: %d\n",access_wan_info.learningCount);
		diag_util_mprintf("action: %d\n",access_wan_info.action);
		return CPARSER_OK;
	}
}    /* end of cparser_cmd_rg_get_wanaccesslimitcategory_category_category */

/*
 * rg get wanAccessLimitCategory mac <MACADDR:mac>
 */
cparser_result_t
cparser_cmd_rg_get_wanAccessLimitCategory_mac_mac(
    cparser_context_t *context,
    cparser_macaddr_t  *mac_ptr)
{
    int ret;
	rtk_rg_accessWanLimitCategory_t macCategory_info;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&macCategory_info, 0x0, sizeof(rtk_rg_accessWanLimitCategory_t)); 
	osal_memcpy(macCategory_info.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);

	ret = rtk_rg_accessWanLimitCategory_get(&macCategory_info);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get rtk_rg_accessWanLimitCategory_get for mac[%02x:%02x:%02x:%02x:%02x:%02x] failed. rg API return %x \n",
			mac_ptr->octet[0],mac_ptr->octet[1],mac_ptr->octet[2],mac_ptr->octet[3],mac_ptr->octet[4],mac_ptr->octet[5],ret);
		return CPARSER_NOT_OK;
	}
	else
	{	
		diag_util_mprintf("MACAddr: %02x:%02x:%02x:%02x:%02x:%02x\n",mac_ptr->octet[0],mac_ptr->octet[1],mac_ptr->octet[2],mac_ptr->octet[3],mac_ptr->octet[4],mac_ptr->octet[5]);
		diag_util_mprintf("category: 0x%x\n",macCategory_info.category);
		return CPARSER_OK;
	}
}    /* end of cparser_cmd_rg_get_wanaccesslimitcategory_mac_mac */

/*
 * rg set softwareSourceAddrLearningLimit learningLimitNumber <UINT:learningLimitNumber> action <UINT:action>
 */
cparser_result_t
cparser_cmd_rg_set_softwareSourceAddrLearningLimit_learningLimitNumber_learningLimitNumber_action_action(
    cparser_context_t *context,
    uint32_t  *learningLimitNumber_ptr,
    uint32_t  *action_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	sa_learnLimit_info.learningLimitNumber=*learningLimitNumber_ptr;
	sa_learnLimit_info.action=*action_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_softwaresourceaddrlearninglimit_learninglimitnumber_learninglimitnumber_action_action */

/*
 * rg set softwareSourceAddrLearningLimit learningLimitNumber unlimit action <UINT:action>
 */
cparser_result_t
cparser_cmd_rg_set_softwareSourceAddrLearningLimit_learningLimitNumber_unlimit_action_action(
    cparser_context_t *context,
    uint32_t  *action_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	sa_learnLimit_info.learningLimitNumber=-1;
	sa_learnLimit_info.action=*action_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_softwaresourceaddrlearninglimit_learninglimitnumber_unlimit_action_action */


/*
 * rg add softwareSourceAddrLearningLimit port_idx <UINT:port_idx>
 */
cparser_result_t
cparser_cmd_rg_add_softwareSourceAddrLearningLimit_port_idx_port_idx(
    cparser_context_t *context,
    uint32_t  *port_idx_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	ret = rtk_rg_softwareSourceAddrLearningLimit_set(sa_learnLimit_info, *port_idx_ptr);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_softwareSourceAddrLearningLimit_set failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add sa_learnLimit_info success.\n");
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_softwaresourceaddrlearninglimit_port_idx_port_idx */

/*
 * rg add softwareSourceAddrLearningLimit wlan_idx <UINT:wlan_idx> device_idx <UINT:device_idx>
 */
cparser_result_t
cparser_cmd_rg_add_softwareSourceAddrLearningLimit_wlan_idx_wlan_idx_device_idx_device_idx(
    cparser_context_t *context,
    uint32_t  *wlan_idx_ptr,
    uint32_t  *device_idx_ptr)
{
    int ret;
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	ret = rtk_rg_wlanSoftwareSourceAddrLearningLimit_set(sa_learnLimit_info,*wlan_idx_ptr,*device_idx_ptr);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_wlanSoftwareSourceAddrLearningLimit_set failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_wlanSoftwareSourceAddrLearningLimit_set success.\n");
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_softwaresourceaddrlearninglimit_wlan_idx_wlan_idx_device_idx_device_idx */


/*
 * rg get softwareSourceAddrLearningLimit port_idx <UINT:port_idx>
 */
cparser_result_t
cparser_cmd_rg_get_softwareSourceAddrLearningLimit_port_idx_port_idx(
    cparser_context_t *context,
    uint32_t  *port_idx_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&sa_learnLimit_info, 0x0, sizeof(rtk_rg_saLearningLimitInfo_t)); 

	ret = rtk_rg_softwareSourceAddrLearningLimit_get(&sa_learnLimit_info,*port_idx_ptr);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get rtk_rg_softwareSourceAddrLearningLimit_get failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{	
		if(sa_learnLimit_info.learningLimitNumber<0)
			diag_util_mprintf("learningLimitNumber: unlimit\n");
		else
			diag_util_mprintf("learningLimitNumber: %d\n",sa_learnLimit_info.learningLimitNumber);
		diag_util_mprintf("action: %d\n",sa_learnLimit_info.action);
		return CPARSER_OK;
	}
}    /* end of cparser_cmd_rg_get_softwaresourceaddrlearninglimit_port_idx_port_idx */

/*
 * rg get softwareSourceAddrLearningLimit wlan_idx <UINT:wlan_idx> device_idx <UINT:device_idx>
 */
cparser_result_t
cparser_cmd_rg_get_softwareSourceAddrLearningLimit_wlan_idx_wlan_idx_device_idx_device_idx(
    cparser_context_t *context,
    uint32_t  *wlan_idx_ptr,
    uint32_t  *device_idx_ptr)
{
    int ret;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&sa_learnLimit_info, 0x0, sizeof(rtk_rg_saLearningLimitInfo_t)); 

	ret = rtk_rg_wlanSoftwareSourceAddrLearningLimit_get(&sa_learnLimit_info,*wlan_idx_ptr,*device_idx_ptr);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get rtk_rg_wlanSoftwareSourceAddrLearningLimit_get failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{	
		if(sa_learnLimit_info.learningLimitNumber<0)
			diag_util_mprintf("learningLimitNumber: unlimit\n");
		else
			diag_util_mprintf("learningLimitNumber: %d\n",sa_learnLimit_info.learningLimitNumber);
		diag_util_mprintf("action: %d\n",sa_learnLimit_info.action);
		return CPARSER_OK;
	}
}    /* end of cparser_cmd_rg_get_softwaresourceaddrlearninglimit_wlan_idx_wlan_idx_device_idx_device_idx */


/*
 * rg get softwareSourceAddrLearningLimit port_idx all
 */
cparser_result_t
cparser_cmd_rg_get_softwareSourceAddrLearningLimit_port_idx_all(
    cparser_context_t *context)
{
	int ret, port;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();


	for(port=0;port<RTK_RG_MAC_PORT_MAX;port++){
		osal_memset(&sa_learnLimit_info, 0x0, sizeof(rtk_rg_saLearningLimitInfo_t)); 
		ret = rtk_rg_softwareSourceAddrLearningLimit_get(&sa_learnLimit_info,port);
		if(ret!=RT_ERR_RG_OK)
		{
			diag_util_printf("get rtk_rg_softwareSourceAddrLearningLimit_get(port=%d) failed. rg API return %x \n",port,ret);
		}
		else
		{	
			diag_util_printf("port [%d] ",port);
			diag_util_printf("learningLimitNumber: %d ",sa_learnLimit_info.learningLimitNumber);
			diag_util_printf("action: %d\n",sa_learnLimit_info.action);
		}
	}
	return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_softwaresourceaddrlearninglimit_port_idx_all */


/*
 * rg set phyPortForceAbility force_disable_phy <UINT:force_disable_phy> valid <UINT:valid> speed <UINT:speed> duplex <UINT:duplex> flowCtrl <UINT:flowCtrl>
 */
cparser_result_t
cparser_cmd_rg_set_phyPortForceAbility_force_disable_phy_force_disable_phy_valid_valid_speed_speed_duplex_duplex_flowCtrl_flowCtrl(
    cparser_context_t *context,
    uint32_t  *force_disable_phy_ptr,
    uint32_t  *valid_ptr,
    uint32_t  *speed_ptr,
    uint32_t  *duplex_ptr,
    uint32_t  *flowCtrl_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	phyPort_ability.force_disable_phy=*force_disable_phy_ptr;
	phyPort_ability.valid=*valid_ptr;
	phyPort_ability.speed=*speed_ptr;
	phyPort_ability.duplex=*duplex_ptr;
	phyPort_ability.flowCtrl=*flowCtrl_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_phyportforceability_force_disable_phy_force_disable_phy_valid_valid_speed_speed_duplex_duplex_flowctrl_flowctrl */

/*
 * rg set phyPortForceAbility force_disable_phy <UINT:force_disable_phy> valid <UINT:valid> speed <UINT:speed> duplex <UINT:duplex> flowCtrl <UINT:flowCtrl> fc <UINT:fc> asym_fc <UINT:asym_fc>
 */
cparser_result_t
cparser_cmd_rg_set_phyPortForceAbility_force_disable_phy_force_disable_phy_valid_valid_speed_speed_duplex_duplex_flowCtrl_flowCtrl_fc_fc_asym_fc_asym_fc(
    cparser_context_t *context,
    uint32_t  *force_disable_phy_ptr,
    uint32_t  *valid_ptr,
    uint32_t  *speed_ptr,
    uint32_t  *duplex_ptr,
    uint32_t  *flowCtrl_ptr,
    uint32_t  *fc_ptr,
    uint32_t  *asym_fc_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	phyPort_ability.force_disable_phy=*force_disable_phy_ptr;
	phyPort_ability.valid=*valid_ptr;
	phyPort_ability.speed=*speed_ptr;
	phyPort_ability.duplex=*duplex_ptr;
	phyPort_ability.flowCtrl=*flowCtrl_ptr;
	phyPort_ability.fc=*fc_ptr;
	phyPort_ability.asym_fc=*asym_fc_ptr;
		
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_phyportforceability_force_disable_phy_force_disable_phy_valid_valid_speed_speed_duplex_duplex_flowctrl_flowctrl_fc_fc_asym_fc_asym_fc */

/*
 * rg add phyPortForceAbility port_idx <UINT:port_idx>
 */
cparser_result_t
cparser_cmd_rg_add_phyPortForceAbility_port_idx_port_idx(
    cparser_context_t *context,
    uint32_t  *port_idx_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	ret =  rtk_rg_phyPortForceAbility_set(*port_idx_ptr,phyPort_ability);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_phyPortForceAbility_set failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_phyPortForceAbility_set success.\n");
	}

    return CPARSER_OK;

}    /* end of cparser_cmd_rg_add_phyportforceability_port_idx_port_idx */


/*
 * rg get phyPortForceAbility port_idx <UINT:port>
 */
cparser_result_t
cparser_cmd_rg_get_phyPortForceAbility_port_idx_port(
    cparser_context_t *context,
    uint32_t  *port_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&phyPort_ability, 0x0, sizeof(rtk_rg_phyPortAbilityInfo_t)); 

	ret = rtk_rg_phyPortForceAbility_get(*port_ptr,&phyPort_ability);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get rtk_rg_phyPortForceAbility_get failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{	
		diag_util_mprintf("force_disable_phy: %d\n",phyPort_ability.force_disable_phy);
		if(phyPort_ability.force_disable_phy==0){//phy is not disabled
			diag_util_mprintf("valid: %d\n",phyPort_ability.valid);
			if(phyPort_ability.valid){
				diag_util_mprintf("port is link-up by forcing setting:\n");
				diag_util_mprintf("speed: %d\n",phyPort_ability.speed);
				diag_util_mprintf("duplex: %d\n",phyPort_ability.duplex);
				diag_util_mprintf("flowCtrl: %d\n",phyPort_ability.flowCtrl);
				diag_util_mprintf("fc: %d\n",phyPort_ability.fc);
				diag_util_mprintf("asym_fc: %d\n",phyPort_ability.asym_fc);
			}else{
				diag_util_mprintf("port is link-up by auto-negotiation setting!\n",phyPort_ability.flowCtrl);
			}
		}
		return CPARSER_OK;
	}

}    /* end of cparser_cmd_rg_get_phyportforceability_port_idx_port */

/*
 * rg add cpuPortForceTrafficCtrl tx_fc_state <UINT:tx_fc_state> rx_fc_state <UINT:rx_fc_state>
 */
cparser_result_t
cparser_cmd_rg_add_cpuPortForceTrafficCtrl_tx_fc_state_tx_fc_state_rx_fc_state_rx_fc_state(
    cparser_context_t *context,
    uint32_t  *tx_fc_state_ptr,
    uint32_t  *rx_fc_state_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	ret = rtk_rg_cpuPortForceTrafficCtrl_set( *tx_fc_state_ptr, *rx_fc_state_ptr);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_cpuPortForceTrafficCtrl_set failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_cpuPortForceTrafficCtrl_set success.\n");
	}
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_cpuportforcetrafficctrl_tx_fc_state_tx_fc_state_rx_fc_state_rx_fc_state */

/*
 * rg get cpuPortForceTrafficCtrl
 */
cparser_result_t
cparser_cmd_rg_get_cpuPortForceTrafficCtrl(
    cparser_context_t *context)
{
	int ret;
	int tx_fc;
	int rx_fc;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	ret = rtk_rg_cpuPortForceTrafficCtrl_get(&tx_fc,&rx_fc);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get rtk_rg_cpuPortForceTrafficCtrl_get failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{	
		diag_util_mprintf("tx_fc: %d\n",tx_fc);
		diag_util_mprintf("rx_fc: %d\n",rx_fc);	
		return CPARSER_OK;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_cpuportforcetrafficctrl */

/*
 * rg set portMirror monitorPort <UINT:monitorPort> enabledPortMask <HEX:enabledPortMask> direct <UINT:direct>
 */
cparser_result_t
cparser_cmd_rg_set_portMirror_monitorPort_monitorPort_enabledPortMask_enabledPortMask_direct_direct(
    cparser_context_t *context,
    uint32_t  *monitorPort_ptr,
    uint32_t  *enabledPortMask_ptr,
    uint32_t  *direct_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	portMirrorInfo.monitorPort=*monitorPort_ptr;
	portMirrorInfo.enabledPortMask.portmask=*enabledPortMask_ptr;
	portMirrorInfo.direct=*direct_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_portmirror_monitorport_monitorport_enabledportmask_enabledportmask_direct_direct */

/*
 * rg add portMirror entry
 */
cparser_result_t
cparser_cmd_rg_add_portMirror_entry(
    cparser_context_t *context)
{
	int ret;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	ret =  rtk_rg_portMirror_set(portMirrorInfo);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_portMirror_set failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_portMirror_set success.\n");
	}
	return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_portmirror_entry */

/*
 * rg get portMirror port <UINT:port>
 */
cparser_result_t
cparser_cmd_rg_get_portMirror_port_port(
    cparser_context_t *context,
    uint32_t  *port_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&portMirrorInfo, 0x0, sizeof(rtk_rg_portMirrorInfo_t)); 

	ret = rtk_rg_portMirror_get(&portMirrorInfo);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get rtk_rg_portMirror_get failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{	
		diag_util_mprintf("monitorPort: %d\n",portMirrorInfo.monitorPort);
		diag_util_mprintf("speeenabledPortMaskd: 0x%x\n",portMirrorInfo.enabledPortMask);
		diag_util_mprintf("direct: %d\n",portMirrorInfo.direct);
		return CPARSER_OK;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_portmirror_port_port */

/*
 * rg add dosPortMaskEnable portmask <HEX:portmask>
 */
cparser_result_t
cparser_cmd_rg_add_dosPortMaskEnable_portmask_portmask(
    cparser_context_t *context,
    uint32_t  *portmask_ptr)
{
 	int ret;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	ret = rtk_rg_dosPortMaskEnable_set(*portmask_ptr);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_dosPortMaskEnable_set failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_dosPortMaskEnable_set success.\n");
		return CPARSER_OK;
	}
}    /* end of cparser_cmd_rg_add_dosportmaskenable_portmask_portmask */

/*
 * rg get dosPortMaskEnable
 */
cparser_result_t
cparser_cmd_rg_get_dosPortMaskEnable(
    cparser_context_t *context)
{
	int ret;
	rtk_rg_mac_portmask_t dos_port_mask;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	ret = rtk_rg_dosPortMaskEnable_get(&dos_port_mask);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get rtk_rg_dosPortMaskEnable_get failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{	
		diag_util_mprintf("dos_port_mask: 0x%x\n",dos_port_mask.portmask);
		return CPARSER_OK;
	}

    return CPARSER_OK;

}    /* end of cparser_cmd_rg_get_dosportmaskenable */

/*
 * rg add dosType dos_type <UINT:dos_type> dos_enabled <UINT:dos_enabled> dos_action  <UINT:dos_action>
 */
cparser_result_t
cparser_cmd_rg_add_dosType_dos_type_dos_type_dos_enabled_dos_enabled_dos_action_dos_action(
    cparser_context_t *context,
    uint32_t  *dos_type_ptr,
    uint32_t  *dos_enabled_ptr,
    uint32_t  *dos_action_ptr)
{
 	int ret;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	ret = rtk_rg_dosType_set(*dos_type_ptr,*dos_enabled_ptr,*dos_action_ptr);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_dosType_set failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_dosType_set success.\n");
		return CPARSER_OK;
	}

}    /* end of cparser_cmd_rg_add_dostype_dos_type_dos_type_dos_enabled_dos_enabled_dos_action_dos_action */

/*
 * rg get dosType dos_type <UINT:dos_type>
 */
cparser_result_t
cparser_cmd_rg_get_dosType_dos_type_dos_type(
    cparser_context_t *context,
    uint32_t  *dos_type_ptr)
{
	int ret;
	rtk_rg_dos_type_t dos_type;
	int dos_enabled;
	rtk_rg_dos_action_t dos_action;
	
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	dos_type = *dos_type_ptr;
	ret = rtk_rg_dosType_get(dos_type,&dos_enabled,&dos_action);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get rtk_rg_dosType_get failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{	
		diag_util_mprintf("dos_type: %d\n",dos_type);
		diag_util_mprintf("dos_enabled: %d\n",dos_enabled);
		diag_util_mprintf("dos_action: %d\n",dos_action);
		return CPARSER_OK;
	}

    return CPARSER_OK;

}    /* end of cparser_cmd_rg_get_dostype_dos_type_dos_type */

/*
 * rg add dosFloodType dos_type <UINT:dos_type> dos_enabled <UINT:dos_enabled> dos_action  <UINT:dos_action> dos_threshold <UINT:dos_threshold>
 */
cparser_result_t
cparser_cmd_rg_add_dosFloodType_dos_type_dos_type_dos_enabled_dos_enabled_dos_action_dos_action_dos_threshold_dos_threshold(
    cparser_context_t *context,
    uint32_t  *dos_type_ptr,
    uint32_t  *dos_enabled_ptr,
    uint32_t  *dos_action_ptr,
    uint32_t  *dos_threshold_ptr)
{
 	int ret;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	ret = rtk_rg_dosFloodType_set(*dos_type_ptr,*dos_enabled_ptr,*dos_action_ptr,*dos_threshold_ptr);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_dosFloodType_set failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_dosFloodType_set success.\n");
		return CPARSER_OK;
	}

}    /* end of cparser_cmd_rg_add_dosfloodtype_dos_type_dos_type_dos_enabled_dos_enabled_dos_action_dos_action_dos_threshold_dos_threshold */

/*
 * rg get dosFloodType dos_type <UINT:dos_type>
 */
cparser_result_t
cparser_cmd_rg_get_dosFloodType_dos_type_dos_type(
    cparser_context_t *context,
    uint32_t  *dos_type_ptr)
{
	int ret;
	rtk_rg_dos_type_t dos_type;
	int dos_enabled;
	rtk_rg_dos_action_t dos_action;
	int dos_threshold;

	
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
	dos_type = *dos_type_ptr;
	ret = rtk_rg_dosFloodType_get(dos_type,&dos_enabled,&dos_action,&dos_threshold);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get rtk_rg_dosFloodType_get failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{	
		diag_util_mprintf("dos_type: %d\n",dos_type);
		diag_util_mprintf("dos_enabled: %d\n",dos_enabled);
		diag_util_mprintf("dos_action: %d\n",dos_action);
		diag_util_mprintf("dos_threshold: %d\n",dos_threshold);
		return CPARSER_OK;
	}

    return CPARSER_OK;

}    /* end of cparser_cmd_rg_get_dosfloodtype_dos_type_dos_type */

/*
 * rg add portEgrBandwidthCtrlRate port <UINT:port> rate <UINT:rate>
 */
cparser_result_t
cparser_cmd_rg_add_portEgrBandwidthCtrlRate_port_port_rate_rate(
    cparser_context_t *context,
    uint32_t  *port_ptr,
    uint32_t  *rate_ptr)
{
 	int ret;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	ret =  rtk_rg_portEgrBandwidthCtrlRate_set(*port_ptr, *rate_ptr);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("netfilter:add rtk_rate_portEgrBandwidthCtrlRate_set failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rate_portEgrBandwidthCtrlRate_set success.\n");
		return CPARSER_OK;
	}

}    /* end of cparser_cmd_rg_add_portegrbandwidthctrlrate_port_port_rate_rate */

/*
 * rg add portIgrBandwidthCtrlRate port <UINT:port> rate <UINT:rate>
 */
cparser_result_t
cparser_cmd_rg_add_portIgrBandwidthCtrlRate_port_port_rate_rate(
    cparser_context_t *context,
    uint32_t  *port_ptr,
    uint32_t  *rate_ptr)
{
 	int ret;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	ret =  rtk_rg_portIgrBandwidthCtrlRate_set(*port_ptr, *rate_ptr);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("netfilter:add rtk_rg_portIgrBandwidthCtrlRate_set failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_portIgrBandwidthCtrlRate_set success.\n");
		return CPARSER_OK;
	}

}    /* end of cparser_cmd_rg_add_portigrbandwidthctrlrate_port_port_rate_rate */

/*
 * rg get portEgrBandwidthCtrlRate port <UINT:port>
 */
cparser_result_t
cparser_cmd_rg_get_portEgrBandwidthCtrlRate_port_port(
    cparser_context_t *context,
    uint32_t  *port_ptr)
{
	int ret;
	unsigned int rate;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	ret = rtk_rg_portEgrBandwidthCtrlRate_get(*port_ptr,&rate);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get rtk_rg_portEgrBandwidthCtrlRate_get failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{	
		diag_util_mprintf("rate: %d\n",rate);
		return CPARSER_OK;
	}

    return CPARSER_OK;

}    /* end of cparser_cmd_rg_get_portegrbandwidthctrlrate_port_port */

/*
 * rg get portIgrBandwidthCtrlRate port <UINT:port>
 */
cparser_result_t
cparser_cmd_rg_get_portIgrBandwidthCtrlRate_port_port(
    cparser_context_t *context,
    uint32_t  *port_ptr)
{
	int ret;
	unsigned int rate;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	ret = rtk_rg_portIgrBandwidthCtrlRate_get(*port_ptr,&rate);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get rtk_rg_portIgrBandwidthCtrlRate_get failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{	
		diag_util_mprintf("rate: %d\n",rate);
		return CPARSER_OK;
	}

    return CPARSER_OK;

}    /* end of cparser_cmd_rg_get_portigrbandwidthctrlrate_port_port */


/*
 * rg show upnpConnection
 */
cparser_result_t
cparser_cmd_rg_show_upnpConnection(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	_diag_upnpInfoShow(&upnpInfo);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_upnpconnection */

/*
 * rg clear upnpConnection
 */
cparser_result_t
cparser_cmd_rg_clear_upnpConnection(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&upnpInfo, 0x0, sizeof(rtk_rg_upnpConnection_t));

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_upnpconnection */

/*
 * rg add upnpConnection entry
 */
cparser_result_t
cparser_cmd_rg_add_upnpConnection_entry(
    cparser_context_t *context)
{
 	int ret;
	int index;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret =  rtk_rg_upnpConnection_add(&upnpInfo, &index);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_upnpConnection_add failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_upnpConnection_add[%d] success.\n",index);
	}

    return CPARSER_OK;

}    /* end of cparser_cmd_rg_add_upnpconnection_entry */

/*
 * rg get upnpConnection entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_get_upnpConnection_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int ret;
	int index=*entry_index_ptr;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&upnpInfo, 0x0, sizeof(rtk_rg_upnpConnection_t));

	ret = rtk_rg_upnpConnection_find(&upnpInfo, &index);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get rtk_rg_upnpConnection_find failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		if(index==*entry_index_ptr){	
			_diag_upnpInfoShow(&upnpInfo);
		}
		else{
			diag_util_mprintf("rtk_rg_upnpConnection_find[%d] is empty.\n",*entry_index_ptr);
			return CPARSER_NOT_OK;
		}
	}
	
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_get_upnpconnection_entry_entry_index */

/*
 * rg del upnpConnection entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_del_upnpConnection_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int ret;
	DIAG_UTIL_PARAM_CHK();
	
	ret = rtk_rg_upnpConnection_del(*entry_index_ptr);
	if(ret!=RT_ERR_RG_OK)
		return CPARSER_NOT_OK;

    return CPARSER_OK;

}    /* end of cparser_cmd_rg_del_upnpconnection_entry_entry_index */

/*
 * rg set upnpConnection is_tcp <UINT:is_tcp> valid <UINT:valid> wan_intf_idx <UINT:wan_intf_idx> gateway_port <UINT:gateway_port> local_ip <IPV4ADDR:local_ip> local_port <UINT:local_port> limit_remote_ip <UINT:limit_remote_ip> limit_remote_port <UINT:limit_remote_port> remote_ip <IPV4ADDR:remote_ip> remote_port <UINT:remote_port> type <UINT:type> timeout <UINT:timeout>
 */
cparser_result_t
cparser_cmd_rg_set_upnpConnection_is_tcp_is_tcp_valid_valid_wan_intf_idx_wan_intf_idx_gateway_port_gateway_port_local_ip_local_ip_local_port_local_port_limit_remote_ip_limit_remote_ip_limit_remote_port_limit_remote_port_remote_ip_remote_ip_remote_port_remote_port_type_type_timeout_timeout(
    cparser_context_t *context,
    uint32_t  *is_tcp_ptr,
    uint32_t  *valid_ptr,
    uint32_t  *wan_intf_idx_ptr,
    uint32_t  *gateway_port_ptr,
    uint32_t  *local_ip_ptr,
    uint32_t  *local_port_ptr,
    uint32_t  *limit_remote_ip_ptr,
    uint32_t  *limit_remote_port_ptr,
    uint32_t  *remote_ip_ptr,
    uint32_t  *remote_port_ptr,
    uint32_t  *type_ptr,
    uint32_t  *timeout_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	upnpInfo.is_tcp=*is_tcp_ptr;
	upnpInfo.valid=*valid_ptr;
	upnpInfo.wan_intf_idx=*wan_intf_idx_ptr;
	upnpInfo.gateway_port=*gateway_port_ptr;
	upnpInfo.local_ip=*local_ip_ptr;
	upnpInfo.local_port=*local_port_ptr;
	upnpInfo.limit_remote_ip=*limit_remote_ip_ptr;
	upnpInfo.limit_remote_port=*limit_remote_port_ptr;
	upnpInfo.remote_ip=*remote_ip_ptr;
	upnpInfo.remote_port=*remote_port_ptr;
	upnpInfo.type=*type_ptr;
	upnpInfo.timeout=*timeout_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_upnpconnection_is_tcp_is_tcp_valid_valid_wan_intf_idx_wan_intf_idx_gateway_port_gateway_port_local_ip_local_ip_local_port_local_port_limit_remote_ip_limit_remote_ip_limit_remote_port_limit_remote_port_remote_ip_remote_ip_remote_port_remote_port_type_type_timeout_timeout */


/*
 * rg set dmzHost enabled <UINT:enabled> private_ip <IPV4ADDR:private_ip>
 */
cparser_result_t
cparser_cmd_rg_set_dmzHost_enabled_enabled_private_ip_private_ip(
    cparser_context_t *context,
    uint32_t  *enabled_ptr,
    uint32_t  *private_ip_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&dmz_info, 0x0, sizeof(rtk_rg_dmzInfo_t));
	dmz_info.enabled = *enabled_ptr;
	dmz_info.private_ip = *private_ip_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_dmzhost_enabled_enabled_private_ip_private_ip */

/*
 * rg set dmzHost enabled <UINT:enabled> ipversion <UINT:ipversion> private_ip <IPV4ADDR:private_ip> private_ipv6 <IPV6ADDR:private_ipv6>
 */
cparser_result_t
cparser_cmd_rg_set_dmzHost_enabled_enabled_ipversion_ipversion_private_ip_private_ip_private_ipv6_private_ipv6(
    cparser_context_t *context,
    uint32_t  *enabled_ptr,
    uint32_t  *ipversion_ptr,
    uint32_t  *private_ip_ptr,
    char * *private_ipv6_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&dmz_info, 0x0, sizeof(rtk_rg_dmzInfo_t));
	dmz_info.enabled = *enabled_ptr;
	dmz_info.ipversion = *ipversion_ptr;
	dmz_info.private_ip = *private_ip_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&dmz_info.private_ipv6.ipv6_addr[0], TOKEN_STR(10)), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_dmzhost_enabled_enabled_ipversion_ipversion_private_ip_private_ip_private_ipv6_private_ipv6 */


/*
 * rg add dmzHost wan_intf_idx <UINT:wan_intf_idx>
 */
cparser_result_t
cparser_cmd_rg_add_dmzHost_wan_intf_idx_wan_intf_idx(
    cparser_context_t *context,
    uint32_t  *wan_intf_idx_ptr)
{
	int ret;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	ret = rtk_rg_dmzHost_set(*wan_intf_idx_ptr,&dmz_info);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_dmzHost_set failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_dmzHost_set success.\n");
		return CPARSER_OK;
	}
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_dmzhost_wan_intf_idx_wan_intf_idx */

/*
 * rg get dmzHost wan_intf_idx <UINT:wan_intf_idx>
 */
cparser_result_t
cparser_cmd_rg_get_dmzHost_wan_intf_idx_wan_intf_idx(
    cparser_context_t *context,
    uint32_t  *wan_intf_idx_ptr)
{
	int ret;
	rtk_rg_dmzInfo_t dmz_info;
	osal_memset(&dmz_info, 0x0, sizeof(rtk_rg_dmzInfo_t));

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
	
	ret = rtk_rg_dmzHost_get(*wan_intf_idx_ptr,&dmz_info);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get rtk_rg_dmzHost_get failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{	
		diag_util_mprintf("enabled: %d\n",dmz_info.enabled);
		diag_util_mprintf("ipversion: 0x%x\n",dmz_info.ipversion);
		diag_util_mprintf("private_ip: 0x%x\n",dmz_info.private_ip);
		diag_util_mprintf("private_ipv6: %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
			dmz_info.private_ipv6.ipv6_addr[0],dmz_info.private_ipv6.ipv6_addr[1],dmz_info.private_ipv6.ipv6_addr[2],dmz_info.private_ipv6.ipv6_addr[3],
			dmz_info.private_ipv6.ipv6_addr[4],dmz_info.private_ipv6.ipv6_addr[5],dmz_info.private_ipv6.ipv6_addr[6],dmz_info.private_ipv6.ipv6_addr[7],		
			dmz_info.private_ipv6.ipv6_addr[8],dmz_info.private_ipv6.ipv6_addr[9],dmz_info.private_ipv6.ipv6_addr[10],dmz_info.private_ipv6.ipv6_addr[11],
			dmz_info.private_ipv6.ipv6_addr[12],dmz_info.private_ipv6.ipv6_addr[13],dmz_info.private_ipv6.ipv6_addr[14],dmz_info.private_ipv6.ipv6_addr[15]);
		return CPARSER_OK;
	}

    return CPARSER_OK;

}    /* end of cparser_cmd_rg_get_dmzhost_wan_intf_idx_wan_intf_idx */

/*
 * rg show stormControl
 */
cparser_result_t
cparser_cmd_rg_show_stormControl(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	_diag_stormControlInfoShow(&stormInfo);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_stormcontrol */

/*
 * rg clear stormControl
 */
cparser_result_t
cparser_cmd_rg_clear_stormControl(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&stormInfo, 0x0, sizeof(rtk_rg_stormControlInfo_t));

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_stormcontrol */

/*
 * rg add stormControl entry
 */
cparser_result_t
cparser_cmd_rg_add_stormControl_entry(
    cparser_context_t *context)
{
 	int ret;
	int index;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret =  rtk_rg_stormControl_add(&stormInfo, &index);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_stormControl_add failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_stormControl_add[%d] success.\n",index);
	}

    return CPARSER_OK;

}    /* end of cparser_cmd_rg_add_stormcontrol_entry */

/*
 * rg get stormControl entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_get_stormControl_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int ret;
	int index=*entry_index_ptr;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&stormInfo, 0x0, sizeof(rtk_rg_stormControlInfo_t));

	ret = rtk_rg_stormControl_find(&stormInfo, &index);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get rtk_rg_stormControl_find failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		if(index==*entry_index_ptr){	
			_diag_stormControlInfoShow(&stormInfo);
		}
		else{
			diag_util_mprintf("rtk_rg_stormControl_find[%d] is empty.\n",*entry_index_ptr);
			return CPARSER_NOT_OK;
		}
	}
	
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_get_stormcontrol_entry_entry_index */

/*
 * rg del stormControl entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_del_stormControl_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int ret;
	DIAG_UTIL_PARAM_CHK();
	
	ret = rtk_rg_stormControl_del(*entry_index_ptr);
	if(ret!=RT_ERR_RG_OK)
		return CPARSER_NOT_OK;

    return CPARSER_OK;

}    /* end of cparser_cmd_rg_del_stormcontrol_entry_entry_index */

/*
 * rg set stormControl valid <UINT:valid> port <UINT:port> stormType <UINT:stormType> meterIdx <UINT:meterIdx>
 */
cparser_result_t
cparser_cmd_rg_set_stormControl_valid_valid_port_port_stormType_stormType_meterIdx_meterIdx(
    cparser_context_t *context,
    uint32_t  *valid_ptr,
    uint32_t  *port_ptr,
    uint32_t  *stormType_ptr,
    uint32_t  *meterIdx_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	stormInfo.valid=*valid_ptr;
	stormInfo.port=*port_ptr;
	stormInfo.stormType=*stormType_ptr;
	stormInfo.meterIdx=*meterIdx_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_stormcontrol_valid_valid_port_port_stormtype_stormtype_meteridx_meteridx */

/*
 * rg add shareMeter index <UINT:index> rate <UINT:rate> ifgInclude <UINT:ifgInclude>
 */
cparser_result_t
cparser_cmd_rg_add_shareMeter_index_index_rate_rate_ifgInclude_ifgInclude(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *rate_ptr,
    uint32_t  *ifgInclude_ptr)
{
	int ret;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	ret = rtk_rg_shareMeter_set(*index_ptr, *rate_ptr, *ifgInclude_ptr);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_shareMeter_set failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_shareMeter_set success.\n");
		return CPARSER_OK;
	}

}    /* end of cparser_cmd_rg_add_sharemeter_index_index_rate_rate_ifginclude_ifginclude */

/*
 * rg get shareMeter index <UINT:index>
 */
cparser_result_t
cparser_cmd_rg_get_shareMeter_index_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
	int ret;
	uint32 rate;
	rtk_rg_enable_t ifgInclude;


    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
	
	ret = rtk_rg_shareMeter_get(*index_ptr,&rate,&ifgInclude);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get rtk_rg_shareMeter_get failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{	
		diag_util_mprintf("rate: %d\n",rate);
		diag_util_mprintf("ifgInclude: 0x%x\n",ifgInclude);
		return CPARSER_OK;
	}

    return CPARSER_OK;

}    /* end of cparser_cmd_rg_get_sharemeter_index_index */


/*
 * rg add qosStrictPriorityOrWeightFairQueue q_weight_0 <UINT:q_weight_0> q_weight_1 <UINT:q_weight_1> q_weight_2 <UINT:q_weight_2> q_weight_3 <UINT:q_weight_3> q_weight_4 <UINT:q_weight_4> q_weight_5 <UINT:q_weight_5> q_weight_6 <UINT:q_weight_6> q_weight_7 <UINT:q_weight_7> port <UINT:port>
 */
cparser_result_t
cparser_cmd_rg_add_qosStrictPriorityOrWeightFairQueue_q_weight_0_q_weight_0_q_weight_1_q_weight_1_q_weight_2_q_weight_2_q_weight_3_q_weight_3_q_weight_4_q_weight_4_q_weight_5_q_weight_5_q_weight_6_q_weight_6_q_weight_7_q_weight_7_port_port(
    cparser_context_t *context,
    uint32_t  *q_weight_0_ptr,
    uint32_t  *q_weight_1_ptr,
    uint32_t  *q_weight_2_ptr,
    uint32_t  *q_weight_3_ptr,
    uint32_t  *q_weight_4_ptr,
    uint32_t  *q_weight_5_ptr,
    uint32_t  *q_weight_6_ptr,
    uint32_t  *q_weight_7_ptr,
    uint32_t  *port_ptr)
{
  	int ret;	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	q_weight.weights[0]=*q_weight_0_ptr;
	q_weight.weights[1]=*q_weight_1_ptr;
	q_weight.weights[2]=*q_weight_2_ptr;
	q_weight.weights[3]=*q_weight_3_ptr;
	q_weight.weights[4]=*q_weight_4_ptr;
	q_weight.weights[5]=*q_weight_5_ptr;
	q_weight.weights[6]=*q_weight_6_ptr;
	q_weight.weights[7]=*q_weight_7_ptr;

	ret = rtk_rg_qosStrictPriorityOrWeightFairQueue_set(*port_ptr, q_weight);
	if(ret!=RT_ERR_RG_OK){
	  diag_util_mprintf("add qosStrictPriorityOrWeightFairQueue failed. rg API return %x \n",ret);
	  return CPARSER_NOT_OK;
	}
	else{
	  diag_util_mprintf("add qosStrictPriorityOrWeightFairQueue success.\n");
	  return CPARSER_OK;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_qosstrictpriorityorweightfairqueue_q_weight_0_q_weight_0_q_weight_1_q_weight_1_q_weight_2_q_weight_2_q_weight_3_q_weight_3_q_weight_4_q_weight_4_q_weight_5_q_weight_5_q_weight_6_q_weight_6_q_weight_7_q_weight_7_port_port */

/*
 * rg get qosStrictPriorityOrWeightFairQueue port <UINT:port> 
 */
cparser_result_t
cparser_cmd_rg_get_qosStrictPriorityOrWeightFairQueue_port_port(
    cparser_context_t *context,
    uint32_t  *port_ptr)
{
	int ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
	osal_memset(&q_weight, 0x0, sizeof(rtk_rg_qos_queue_weights_t));
	
	ret = rtk_rg_qosStrictPriorityOrWeightFairQueue_get(*port_ptr,&q_weight);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get qosStrictPriorityOrWeightFairQueue failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{	
		diag_util_mprintf("q_weight[0]: %d\n",q_weight.weights[0]);
		diag_util_mprintf("q_weight[1]: %d\n",q_weight.weights[1]);
		diag_util_mprintf("q_weight[2]: %d\n",q_weight.weights[2]);
		diag_util_mprintf("q_weight[3]: %d\n",q_weight.weights[3]);
		diag_util_mprintf("q_weight[4]: %d\n",q_weight.weights[4]);
		diag_util_mprintf("q_weight[5]: %d\n",q_weight.weights[5]);
		diag_util_mprintf("q_weight[6]: %d\n",q_weight.weights[6]);
		diag_util_mprintf("q_weight[7]: %d\n",q_weight.weights[7]);

		return CPARSER_OK;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_qosstrictpriorityorweightfairqueue_port_port */

/*
 * rg add qosInternalPriMapToQueueId intPri <UINT:intPri> queueId <UINT:queueId>
 */
cparser_result_t
cparser_cmd_rg_add_qosInternalPriMapToQueueId_intPri_intPri_queueId_queueId(
    cparser_context_t *context,
    uint32_t  *intPri_ptr,
    uint32_t  *queueId_ptr)
{

	int ret;
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rtk_rg_qosInternalPriMapToQueueId_set(*intPri_ptr,*queueId_ptr);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add qosInternalPriMapToQueueId failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add qosInternalPriMapToQueueId success.\n");
	    return CPARSER_OK;
	}
}    /* end of cparser_cmd_rg_add_qosinternalprimaptoqueueid_intpri_intpri_queueid_queueid */

/*
 * rg get qosInternalPriMapToQueueId
 */
cparser_result_t
cparser_cmd_rg_get_qosInternalPriMapToQueueId(
    cparser_context_t *context)
{
	int ret;
	int i;
	int queueId;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	for(i=0;i<=7;i++)
	{
		ret = rtk_rg_qosInternalPriMapToQueueId_get(i,&queueId);
		if(ret!=RT_ERR_RG_OK)
		{
			diag_util_mprintf("get qosInternalPriMapToQueueId failed. rg API return %x \n",ret);
			return CPARSER_NOT_OK;
		}
		else
		{
			diag_util_mprintf("internalPri[%d]-to-queue[%d]\n",i,queueId);
		}
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_qosinternalprimaptoqueueid */

/*
 * rg add qosInternalPriDecisionByWeight weight_of_portBased <UINT:weight_of_portBased> weight_of_dot1q <UINT:weight_of_dot1q> weight_of_dscp <UINT:weight_of_dscp> weight_of_acl <UINT:weight_of_acl> weight_of_lutFwd <UINT:weight_of_lutFwd>  weight_of_saBaed <UINT:weight_of_saBaed> weight_of_vlanBased <UINT:weight_of_vlanBased> weight_of_svlanBased <UINT:weight_of_svlanBased> weight_of_l4Based <UINT:weight_of_l4Based>
 */
cparser_result_t
cparser_cmd_rg_add_qosInternalPriDecisionByWeight_weight_of_portBased_weight_of_portBased_weight_of_dot1q_weight_of_dot1q_weight_of_dscp_weight_of_dscp_weight_of_acl_weight_of_acl_weight_of_lutFwd_weight_of_lutFwd_weight_of_saBaed_weight_of_saBaed_weight_of_vlanBased_weight_of_vlanBased_weight_of_svlanBased_weight_of_svlanBased_weight_of_l4Based_weight_of_l4Based(
    cparser_context_t *context,
    uint32_t  *weight_of_portBased_ptr,
    uint32_t  *weight_of_dot1q_ptr,
    uint32_t  *weight_of_dscp_ptr,
    uint32_t  *weight_of_acl_ptr,
    uint32_t  *weight_of_lutFwd_ptr,
    uint32_t  *weight_of_saBaed_ptr,
    uint32_t  *weight_of_vlanBased_ptr,
    uint32_t  *weight_of_svlanBased_ptr,
    uint32_t  *weight_of_l4Based_ptr)
{
	int ret;

    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
    weightOfPriSel.weight_of_portBased=*weight_of_portBased_ptr;
    weightOfPriSel.weight_of_dot1q=*weight_of_dot1q_ptr;
    weightOfPriSel.weight_of_dscp=*weight_of_dscp_ptr;
    weightOfPriSel.weight_of_acl=*weight_of_acl_ptr;
    weightOfPriSel.weight_of_lutFwd=*weight_of_lutFwd_ptr;
   	weightOfPriSel.weight_of_saBaed=*weight_of_saBaed_ptr;
    weightOfPriSel.weight_of_vlanBased=*weight_of_vlanBased_ptr;
    weightOfPriSel.weight_of_svlanBased=*weight_of_svlanBased_ptr;
    weightOfPriSel.weight_of_l4Based=*weight_of_l4Based_ptr;

	ret = rtk_rg_qosInternalPriDecisionByWeight_set(weightOfPriSel);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add qosInternalPriDecisionByWeight failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add qosInternalPriDecisionByWeight success.\n");
	}
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_qosinternalpridecisionbyweight_weight_of_portbased_weight_of_portbased_weight_of_dot1q_weight_of_dot1q_weight_of_dscp_weight_of_dscp_weight_of_acl_weight_of_acl_weight_of_lutfwd_weight_of_lutfwd_weight_of_sabaed_weight_of_sabaed_weight_of_vlanbased_weight_of_vlanbased_weight_of_svlanbased_weight_of_svlanbased_weight_of_l4based_weight_of_l4based */

/*
 * rg get qosInternalPriDecisionByWeight
 */
cparser_result_t
cparser_cmd_rg_get_qosInternalPriDecisionByWeight(
    cparser_context_t *context)
{
	int ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&weightOfPriSel, 0x0, sizeof(weightOfPriSel));

	ret = rtk_rg_qosInternalPriDecisionByWeight_get(&weightOfPriSel);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get qosInternalPriDecisionByWeight failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		diag_util_mprintf("weight_of_portBased: %d\n",weightOfPriSel.weight_of_portBased);
		diag_util_mprintf("weight_of_dot1q: %d\n",weightOfPriSel.weight_of_dot1q);
		diag_util_mprintf("weight_of_dscp: %d\n",weightOfPriSel.weight_of_dscp);
		diag_util_mprintf("weight_of_acl: %d\n",weightOfPriSel.weight_of_acl);
		diag_util_mprintf("weight_of_lutFwd: %d\n",weightOfPriSel.weight_of_lutFwd);
		diag_util_mprintf("weight_of_saBaed: %d\n",weightOfPriSel.weight_of_saBaed);
		diag_util_mprintf("weight_of_vlanBased: %d\n",weightOfPriSel.weight_of_vlanBased);
		diag_util_mprintf("weight_of_svlanBased: %d\n",weightOfPriSel.weight_of_svlanBased);
		diag_util_mprintf("weight_of_l4Based: %d\n",weightOfPriSel.weight_of_l4Based);

    	return CPARSER_OK;
	}
}    /* end of cparser_cmd_rg_get_qosinternalpridecisionbyweight */

/*
 * rg add qosDscpRemapToInternalPri dscp <UINT:dscp> intPri <UINT:intPri>
 */
cparser_result_t
cparser_cmd_rg_add_qosDscpRemapToInternalPri_dscp_dscp_intPri_intPri(
    cparser_context_t *context,
    uint32_t  *dscp_ptr,
    uint32_t  *intPri_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();	
	ret = rtk_rg_qosDscpRemapToInternalPri_set(*dscp_ptr,*intPri_ptr);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add qosDscpRemapToInternalPri failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add qosDscpRemapToInternalPri success.\n");
		return CPARSER_OK;
	}
}    /* end of cparser_cmd_rg_add_qosdscpremaptointernalpri_dscp_dscp_intpri_intpri */

/*
 * rg get qosDscpRemapToInternalPri
 */
cparser_result_t
cparser_cmd_rg_get_qosDscpRemapToInternalPri(
    cparser_context_t *context)
{
	int ret;
	int i;
	int intPri;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	for(i=0;i<64;i++)
	{
		ret = rtk_rg_qosDscpRemapToInternalPri_get(i,&intPri);
		if(ret!=RT_ERR_RG_OK)
		{
			diag_util_mprintf("get qosDscpRemapToInternalPri failed. rg API return %x \n",ret);
			return CPARSER_NOT_OK;
		}
		else
		{
			diag_util_mprintf("dscp[%d] mapping to internal-pri[%d]\n",i,intPri);
		}
	}
	return CPARSER_OK;	

}    /* end of cparser_cmd_rg_get_qosdscpremaptointernalpri */

/*
 * rg add qosPortBasedPriority port <UINT:port> intPri <UINT:intPri>
 */
cparser_result_t
cparser_cmd_rg_add_qosPortBasedPriority_port_port_intPri_intPri(
    cparser_context_t *context,
    uint32_t  *port_ptr,
    uint32_t  *intPri_ptr)
{
 	int ret;
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();	
	ret = rtk_rg_qosPortBasedPriority_set(*port_ptr,*intPri_ptr);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add qosPortBasedPriority failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add qosPortBasedPriority success.\n");
		return CPARSER_OK;
	}
}    /* end of cparser_cmd_rg_add_qosportbasedpriority_port_port_intpri_intpri */

/*
 * rg get qosPortBasedPriority
 */
cparser_result_t
cparser_cmd_rg_get_qosPortBasedPriority(
    cparser_context_t *context)
{
	int i;
	int ret;
	int intPri;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	for(i=0;i<RTK_RG_MAC_PORT_MAX;i++)
	{
		ret = rtk_rg_qosPortBasedPriority_get(i,&intPri);
		if(ret!=RT_ERR_RG_OK)
		{
			diag_util_mprintf("get qosPortBasedPriority failed. rg API return %x \n",ret);
			return CPARSER_NOT_OK;
		}
		else
		{
			diag_util_mprintf("port[%d] mapping to internal-pri[%d]\n",i,intPri);
		}
	}
	return CPARSER_OK;	
}    /* end of cparser_cmd_rg_get_qosportbasedpriority */

/*
 * rg add qosDot1pPriRemapToInternalPri dot1p <UINT:dot1p> intPri <UINT:intPri>
 */
cparser_result_t
cparser_cmd_rg_add_qosDot1pPriRemapToInternalPri_dot1p_dot1p_intPri_intPri(
    cparser_context_t *context,
    uint32_t  *dot1p_ptr,
    uint32_t  *intPri_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();

	ret = rtk_rg_qosDot1pPriRemapToInternalPri_set(*dot1p_ptr,*intPri_ptr);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("Add qosDot1pPriRemapToInternalPri failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		diag_util_mprintf("Add qosDot1pPriRemapToInternalPri success.\n");
		return CPARSER_OK;	
	}
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_qosdot1ppriremaptointernalpri_dot1p_dot1p_intpri_intpri */

/*
 * rg get qosDot1pPriRemapToInternalPri
 */
cparser_result_t
cparser_cmd_rg_get_qosDot1pPriRemapToInternalPri(
    cparser_context_t *context)
{
	int i;
	int ret;
	int int_pri;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	for(i=0;i<=7;i++)
	{
		ret = rtk_rg_qosDot1pPriRemapToInternalPri_get(i,&int_pri);
		if(ret!=RT_ERR_RG_OK)
		{
		 diag_util_mprintf("Get qosDot1pPriRemapToInternalPri failed. rg API return %x \n",ret);
		 return CPARSER_NOT_OK;
		}
		else
		{
		 diag_util_mprintf("Remap .1p priority[%d] to internal priority[%d]\n",i,int_pri);
		}
	}
	return CPARSER_OK;	
}    /* end of cparser_cmd_rg_get_qosdot1ppriremaptointernalpri */

/*
 * rg add qosDscpRemarkEgressPortEnableAndSrcSelect port <UINT:port> enabled <UINT:enabled> source_select <UINT:source_select>
 */
cparser_result_t
cparser_cmd_rg_add_qosDscpRemarkEgressPortEnableAndSrcSelect_port_port_enabled_enabled_source_select_source_select(
    cparser_context_t *context,
    uint32_t  *port_ptr,
    uint32_t  *enabled_ptr,
    uint32_t  *source_select_ptr)
{
	int ret;
	char *rmk_dscp_src[3]={"INT_PRI","DSCP"};
    DIAG_UTIL_PARAM_CHK();

	ret = rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_set(*port_ptr,*enabled_ptr,*source_select_ptr);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("Add qosDscpRemarkEgressPortEnableAndSrcSelect failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		diag_util_mprintf("Add qosDscpRemarkEgressPortEnableAndSrcSelect success.\n");
    	return CPARSER_OK;
	}
}    /* end of cparser_cmd_rg_add_qosdscpremarkegressportenableandsrcselect_port_port_enabled_enabled_source_select_source_select */

/*
 * rg get qosDscpRemarkEgressPortEnableAndSrcSelect
 */
cparser_result_t
cparser_cmd_rg_get_qosDscpRemarkEgressPortEnableAndSrcSelect(
    cparser_context_t *context)
{
	int ret;
	int i;
	rtk_rg_enable_t rmk_enable;
	int src_sel;
	char *rmk_dscp_src[3]={"INT_PRI","DSCP"};
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

 	for(i=0;i<RTK_RG_MAC_PORT_MAX;i++)
 	{
		ret = rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_get(i,&rmk_enable,&src_sel);

		if(ret!=RT_ERR_RG_OK)
		{
			diag_util_mprintf("Get qosDscpRemarkEgressPortEnableAndSrcSelect failed. rg API return %x \n",ret);
			return CPARSER_NOT_OK;
		}
		else
		{
			diag_util_mprintf("Dscp remark port [%d] enabled[%d] source[%s]\n",i,rmk_enable,rmk_dscp_src[src_sel]);
		}
 	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_qosdscpremarkegressportenableandsrcselect */

/*
 * rg add qosDscpRemarkByInternalPri intPri <UINT:intPri> remark_dscp <UINT:remark_dscp>
 */
cparser_result_t
cparser_cmd_rg_add_qosDscpRemarkByInternalPri_intPri_intPri_remark_dscp_remark_dscp(
    cparser_context_t *context,
    uint32_t  *intPri_ptr,
    uint32_t  *remark_dscp_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_qosDscpRemarkByInternalPri_set(*intPri_ptr,*remark_dscp_ptr);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add qosDscpRemarkByInternalPri failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add qosDscpRemarkByInternalPri success.\n");
		return CPARSER_OK;
	}
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_qosdscpremarkbyinternalpri_intpri_intpri_remark_dscp_remark_dscp */

/*
 * rg get qosDscpRemarkByInternalPri
 */
cparser_result_t
cparser_cmd_rg_get_qosDscpRemarkByInternalPri(
    cparser_context_t *context)
{
	int i;
	int ret;
	int dscp;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	for(i=0;i<=7;i++)
	{
		ret = rtk_rg_qosDscpRemarkByInternalPri_get(i,&dscp);
		if(ret!=RT_ERR_RG_OK)
		{
			diag_util_mprintf("get qosDscpRemarkByInternalPri failed. rg API return %x \n",ret);
			return CPARSER_NOT_OK;
		}
		else
		{
			diag_util_mprintf("By internal-pri[%d] remark DSCP[%d]\n",i,dscp);
		}
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_qosdscpremarkbyinternalpri */

/*
 * rg add qosDscpRemarkByDscp dscp <UINT:dscp> remark_dscp <UINT:remark_dscp>
 */
cparser_result_t
cparser_cmd_rg_add_qosDscpRemarkByDscp_dscp_dscp_remark_dscp_remark_dscp(
    cparser_context_t *context,
    uint32_t  *dscp_ptr,
    uint32_t  *remark_dscp_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();

	ret = rtk_rg_qosDscpRemarkByDscp_set(*dscp_ptr,*remark_dscp_ptr);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("add qosDscpRemarkByDscp failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		diag_util_mprintf("add qosDscpRemarkByDscp success\n");
    	return CPARSER_OK;
	}
}    /* end of cparser_cmd_rg_add_qosdscpremarkbydscp_dscp_dscp_remark_dscp_remark_dscp */

/*
 * rg get qosDscpRemarkByDscp
 */
cparser_result_t
cparser_cmd_rg_get_qosDscpRemarkByDscp(
    cparser_context_t *context)
{
	int i;
	int ret;
	int rmk_dscp;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	for(i=0;i<64;i++)
	{
	 	ret = rtk_rg_qosDscpRemarkByDscp_get(i,&rmk_dscp);
		if(ret!=RT_ERR_RG_OK)
		{
			diag_util_mprintf("Get qosDscpRemarkByDscp failed. rg API return %x \n",ret);
			return CPARSER_NOT_OK;
		}
		else
		{
			diag_util_mprintf("By DSCP[%d] remark DSCP[%d]\n",i,rmk_dscp);
		}
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_qosdscpremarkbydscp */

/*
 * rg add qosDot1pPriRemarkByInternalPriEgressPortEnable port <UINT:port> enabled <UINT:enabled>
 */
cparser_result_t
cparser_cmd_rg_add_qosDot1pPriRemarkByInternalPriEgressPortEnable_port_port_enabled_enabled(
    cparser_context_t *context,
    uint32_t  *port_ptr,
    uint32_t  *enabled_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();

	ret = rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_set(*port_ptr,*enabled_ptr);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("add qosDot1pPriRemarkByInternalPriEgressPortEnable failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		diag_util_mprintf("add qosDot1pPriRemarkByInternalPriEgressPortEnable success.\n");
		return CPARSER_OK;	
	}
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_qosdot1ppriremarkbyinternalpriegressportenable_port_port_enabled_enabled */

/*
 * rg get qosDot1pPriRemarkByInternalPriEgressPortEnable
 */
cparser_result_t
cparser_cmd_rg_get_qosDot1pPriRemarkByInternalPriEgressPortEnable(
    cparser_context_t *context)
{
	int ret;
	int i;
	rtk_rg_enable_t isEnable;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	for(i=0;i<RTK_RG_MAC_PORT_MAX;i++)
	{
		ret = rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_get(i,&isEnable);
		if(ret!=RT_ERR_RG_OK)
		{
			diag_util_mprintf("Get qosDot1pPriRemarkByInternalPriEgressPortEnable failed. rg API return %x \n",ret);
			return CPARSER_NOT_OK;
		}
		else
		{
			diag_util_mprintf("802.1p priority remark port[%d] enabled[%d]\n",i,isEnable);
		}
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_qosdot1ppriremarkbyinternalpriegressportenable */

/*
 * rg add qosDot1pPriRemarkByInternalPri intPri <UINT:intPri> dot1p <UINT:dot1p>
 */
cparser_result_t
cparser_cmd_rg_add_qosDot1pPriRemarkByInternalPri_intPri_intPri_dot1p_dot1p(
    cparser_context_t *context,
    uint32_t  *intPri_ptr,
    uint32_t  *dot1p_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();

	ret = rtk_rg_qosDot1pPriRemarkByInternalPri_set(*intPri_ptr,*dot1p_ptr);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("add qosDot1pPriRemarkByInternalPri failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		diag_util_mprintf("add qosDot1pPriRemarkByInternalPri success.\n");
		return CPARSER_OK;  
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_qosdot1ppriremarkbyinternalpri_intpri_intpri_dot1p_dot1p */

/*
 * rg get qosDot1pPriRemarkByInternalPri
 */
cparser_result_t
cparser_cmd_rg_get_qosDot1pPriRemarkByInternalPri(
    cparser_context_t *context)
{
	int i;
	int ret;
	int rmk_1p;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

 	for(i=0;i<=7;i++)
 	{
		ret = rtk_rg_qosDot1pPriRemarkByInternalPri_get(i,&rmk_1p);
		if(ret!=RT_ERR_RG_OK)
		{
			diag_util_mprintf("Get qosDot1pPriRemarkByInternalPri failed. rg API return %x \n",ret);
			return CPARSER_NOT_OK;
		}
		else
		{
			diag_util_mprintf("By internal priority[%d] remarking 802.1p priority[%d]\n",i,rmk_1p);
		}
 	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_qosdot1ppriremarkbyinternalpri */

/*
 * rg show multicastFlow
 */
cparser_result_t
cparser_cmd_rg_show_multicastFlow(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	_diag_multicastFlowShow(&mcFlow);

    return CPARSER_OK;

}    /* end of cparser_cmd_rg_show_multicastflow */

/*
 * rg clear multicastFlow
 */
cparser_result_t
cparser_cmd_rg_clear_multicastFlow(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&mcFlow, 0x0, sizeof(rtk_rg_multicastFlow_t));

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_multicastflow */

/*
 * rg add multicastFlow entry
 */
cparser_result_t
cparser_cmd_rg_add_multicastFlow_entry(
    cparser_context_t *context)
{
 	int ret;
	int index;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret =  rtk_rg_multicastFlow_add(&mcFlow, &index);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_multicastFlow_add failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_multicastFlow_add[%d] success.\n",index);
	}

    return CPARSER_OK;

}    /* end of cparser_cmd_rg_add_multicastflow_entry */


/*
 * rg get multicastFlow entry <UINT:flow_idx>
 */
cparser_result_t
cparser_cmd_rg_get_multicastFlow_entry_flow_idx(
    cparser_context_t *context,
    uint32_t  *flow_idx_ptr)
{
	int ret;
	int index=*flow_idx_ptr;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&mcFlow, 0x0, sizeof(rtk_rg_multicastFlow_t));

	ret = rtk_rg_multicastFlow_find(&mcFlow, &index);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get rtk_rg_multicastFlow_find failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		if(index==*flow_idx_ptr){	
			_diag_multicastFlowShow(&mcFlow);
		}
		else{
			diag_util_mprintf("rtk_rg_multicastFlow_find[%d] is empty. index=%d\n",*flow_idx_ptr,index);
			return CPARSER_NOT_OK;
		}
	}
	return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_multicastflow_entry_flow_idx */

/*
 * rg del multicastFlow entry <UINT:flow_idx>
 */
cparser_result_t
cparser_cmd_rg_del_multicastFlow_entry_flow_idx(
    cparser_context_t *context,
    uint32_t  *flow_idx_ptr)
{
	int ret;
	DIAG_UTIL_PARAM_CHK();
	
	ret = rtk_rg_multicastFlow_del(*flow_idx_ptr);
	if(ret!=RT_ERR_RG_OK)
		return CPARSER_NOT_OK;

    return CPARSER_OK;

}    /* end of cparser_cmd_rg_del_multicastflow_entry_flow_idx */

/*
 * rg set multicastFlow multicast_ipv4_addr <IPV4ADDR:multicast_ipv4_addr> multicast_ipv6_addr <IPV6ADDR:multicast_ipv6_addr> isIPv6 <UINT:isIPv6> port_mask <HEX:port_mask>
 */
cparser_result_t
cparser_cmd_rg_set_multicastFlow_multicast_ipv4_addr_multicast_ipv4_addr_multicast_ipv6_addr_multicast_ipv6_addr_isIPv6_isIPv6_port_mask_port_mask(
    cparser_context_t *context,
    uint32_t  *multicast_ipv4_addr_ptr,
    char * *multicast_ipv6_addr_ptr,
    uint32_t  *isIPv6_ptr,
    uint32_t  *port_mask_ptr)
{
    int ret;
	DIAG_UTIL_PARAM_CHK();
	mcFlow.multicast_ipv4_addr=*multicast_ipv4_addr_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&mcFlow.multicast_ipv6_addr[0], TOKEN_STR(6)), ret);
	mcFlow.isIPv6=*isIPv6_ptr;
	mcFlow.port_mask.portmask=*port_mask_ptr;

	return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_multicastflow_multicast_ipv4_addr_multicast_ipv4_addr_multicast_ipv6_addr_multicast_ipv6_addr_isipv6_isipv6_port_mask_port_mask */

/*
 * rg set multicastFlow multicast_ipv4_addr <IPV4ADDR:multicast_ipv4_addr> multicast_ipv6_addr <IPV6ADDR:multicast_ipv6_addr> isIPv6 <UINT:isIPv6> port_mask <HEX:port_mask> isIVL <UINT:isIVL> vlanId <UINT:vlanId>
 */
cparser_result_t
cparser_cmd_rg_set_multicastFlow_multicast_ipv4_addr_multicast_ipv4_addr_multicast_ipv6_addr_multicast_ipv6_addr_isIPv6_isIPv6_port_mask_port_mask_isIVL_isIVL_vlanId_vlanId(
    cparser_context_t *context,
    uint32_t  *multicast_ipv4_addr_ptr,
    char * *multicast_ipv6_addr_ptr,
    uint32_t  *isIPv6_ptr,
    uint32_t  *port_mask_ptr,
    uint32_t  *isIVL_ptr,
    uint32_t  *vlanId_ptr)
{
    int ret;
	DIAG_UTIL_PARAM_CHK();
	mcFlow.multicast_ipv4_addr=*multicast_ipv4_addr_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&mcFlow.multicast_ipv6_addr[0], TOKEN_STR(6)), ret);
	mcFlow.isIPv6=*isIPv6_ptr;
	mcFlow.port_mask.portmask=*port_mask_ptr;
	mcFlow.isIVL=*isIVL_ptr;
	mcFlow.vlanID=*vlanId_ptr;

	return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_multicastflow_multicast_ipv4_addr_multicast_ipv4_addr_multicast_ipv6_addr_multicast_ipv6_addr_isipv6_isipv6_port_mask_port_mask_isivl_isivl_vlanid_vlanid */

/*
 * rg get portStatus port <UINT:port_idx>
 */
cparser_result_t
cparser_cmd_rg_get_portStatus_port_port_idx(
    cparser_context_t *context,
    uint32_t  *port_idx_ptr)
{
	int ret;
	rtk_rg_portStatusInfo_t portInfo;
		
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    
	ret = rtk_rg_portStatus_get(*port_idx_ptr,&portInfo);

	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("Get rtk_rg_portStatus_get failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		diag_util_mprintf("port[%d]\n",*port_idx_ptr);
		diag_util_mprintf("linkStatus:%d\n",portInfo.linkStatus);
		diag_util_mprintf("linkSpeed:%d\n",portInfo.linkSpeed);
		diag_util_mprintf("linkDuplex:%d\n",portInfo.linkDuplex);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_portstatus_port_port_idx */

/*
 * rg add svlanTpid <HEX:tpid>
 */
cparser_result_t
cparser_cmd_rg_add_svlanTpid_tpid(
    cparser_context_t *context,
    uint32_t  *tpid_ptr)
{
  	int ret;
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();	
	ret = rtk_rg_svlanTpid_set(*tpid_ptr);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_svlanTpid_set failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_svlanTpid_set success.\n");
		return CPARSER_OK;
	}
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_svlantpid_tpid */

/*
 * rg get svlanTpid  */
cparser_result_t
cparser_cmd_rg_get_svlanTpid(
    cparser_context_t *context)
{
	int ret;	
	int tpid;
	DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	ret = rtk_rg_svlanTpid_get(&tpid);
	
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("rtk_rg_svlanTpid_get failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		diag_util_mprintf("tpid is 0x%x.\n",tpid);
		return CPARSER_NOT_OK;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_svlantpid */

/*
 * rg add svlanServicePort port <UINT:port> enable <UINT:enable>
 */
cparser_result_t
cparser_cmd_rg_add_svlanServicePort_port_port_enable_enable(
    cparser_context_t *context,
    uint32_t  *port_ptr,
    uint32_t  *enable_ptr)
{
  	int ret;
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();	
	ret = rtk_rg_svlanServicePort_set(*port_ptr,*enable_ptr);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_svlanServicePort_set failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_svlanServicePort_set success.\n");
		return CPARSER_OK;
	}
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_svlanserviceport_port_port_enable_enable */

/*
 * rg get svlanServicePort port <UINT:port> */
cparser_result_t
cparser_cmd_rg_get_svlanServicePort_port_port(
    cparser_context_t *context,
    uint32_t  *port_ptr)
{
	int ret;	
	rtk_enable_t enable=DISABLED;
	DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	ret = rtk_rg_svlanServicePort_get(*port_ptr,&enable);
	
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("rtk_rg_svlanServicePort_get failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		if(enable==1)
			diag_util_mprintf("ServicePort[%d] is Enabled.\n",*port_ptr);
		else
			diag_util_mprintf("ServicePort[%d] is Disabled.\n",*port_ptr);
			
		return CPARSER_NOT_OK;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_svlanserviceport_port_port */

/*
 * rg get pppoeInterfaceIdleTime interfaceIdx <UINT:interfaceIdx>
 */
cparser_result_t
cparser_cmd_rg_get_pppoeInterfaceIdleTime_interfaceIdx_interfaceIdx(
    cparser_context_t *context,
    uint32_t  *interfaceIdx_ptr)
{
	int ret;	
	uint32 idleSec;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	ret = rtk_rg_pppoeInterfaceIdleTime_get(*interfaceIdx_ptr,&idleSec);
	
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("rtk_rg_pppoeInterfaceIdleTime_get failed. rg API return 0x%x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		diag_util_mprintf("interface[%d] idle %d sec.\n",*interfaceIdx_ptr,idleSec);
	
		return CPARSER_NOT_OK;
	}

    return CPARSER_OK;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_pppoeinterfaceidletime_interfaceidx_interfaceidx */

/*
 * rg show gatewayServicePort
 */
cparser_result_t
cparser_cmd_rg_show_gatewayServicePort(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	_diag_rgGatewayServicePortShow(&gatewayServicePort);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_gatewayserviceport */

/*
 * rg clear gatewayServicePort
 */
cparser_result_t
cparser_cmd_rg_clear_gatewayServicePort(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&gatewayServicePort, 0x0, sizeof(rtk_rg_gatewayServicePortEntry_t));

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_gatewayserviceport */

/*
 * rg add gatewayServicePort entry
 */
cparser_result_t
cparser_cmd_rg_add_gatewayServicePort_entry(
    cparser_context_t *context)
{
	int ret, index;
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret =  rtk_rg_gatewayServicePortRegister_add(&gatewayServicePort, &index);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_gatewayServicePortRegister_add failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_gatewayServicePortRegister_add[%d] success.\n",index);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_gatewayserviceport_entry */

/*
 * rg get gatewayServicePort entry <UINT:idx>
 */
cparser_result_t
cparser_cmd_rg_get_gatewayServicePort_entry_idx(
    cparser_context_t *context,
    uint32_t  *idx_ptr)
{
	int ret;
	int index = *idx_ptr;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
	
	osal_memset(&gatewayServicePort, 0x0, sizeof(rtk_rg_gatewayServicePortEntry_t));

	ret = rtk_rg_gatewayServicePortRegister_find(&gatewayServicePort,&index);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("rtk_rg_gatewayServicePortRegister_find failed. rg API return 0x%x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{	
		diag_util_mprintf("valid: %d\n",gatewayServicePort.valid);
		diag_util_mprintf("port_num: %d\n",gatewayServicePort.port_num);
		diag_util_mprintf("type: %s\n",gatewayServicePort.type?"CLIENT(port)":"SERVICE(dport)");
		return CPARSER_OK;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_gatewayserviceport_entry_idx */

/*
 * rg del gatewayServicePort entry <UINT:idx>
 */
cparser_result_t
cparser_cmd_rg_del_gatewayServicePort_entry_idx(
    cparser_context_t *context,
    uint32_t  *idx_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	
	ret = rtk_rg_gatewayServicePortRegister_del(*idx_ptr);
	if(ret!=RT_ERR_RG_OK)
		return CPARSER_NOT_OK;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_del_gatewayserviceport_entry_idx */

/*
 * rg set gatewayServicePort valid <UINT:valid> port_number <UINT:port_number> type <UINT:type>
 */
cparser_result_t
cparser_cmd_rg_set_gatewayServicePort_valid_valid_port_number_port_number_type_type(
    cparser_context_t *context,
    uint32_t  *valid_ptr,
    uint32_t  *port_number_ptr,
    uint32_t  *type_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	gatewayServicePort.valid = *valid_ptr;
	gatewayServicePort.port_num = *port_number_ptr;
	gatewayServicePort.type = *type_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_gatewayserviceport_valid_valid_port_number_port_number_type_type */

/*
 * rg clear naptFilterAndQos
 */
cparser_result_t
cparser_cmd_rg_clear_naptFilterAndQos(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&naptFilterAndQos, 0x0, sizeof(rtk_rg_naptFilterAndQos_t));

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_naptfilterandqos */

/*
 * rg show naptFilterAndQos
 */
cparser_result_t
cparser_cmd_rg_show_naptFilterAndQos(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	_diag_rgNaptFilterAndQosShow(&naptFilterAndQos);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_naptfilterandqos */

/*
 * rg set naptFilterAndQos direction <UINT:direction>
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_direction_direction(
    cparser_context_t *context,
    uint32_t  *direction_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	naptFilterAndQos.direction = *direction_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_direction_direction */

/*
 * rg set naptFilterAndQos pattern ingress_src_ipv4_addr <IPV4ADDR:ingress_src_ipv4_addr>
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_pattern_ingress_src_ipv4_addr_ingress_src_ipv4_addr(
    cparser_context_t *context,
    uint32_t  *ingress_src_ipv4_addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	naptFilterAndQos.filter_fields |= INGRESS_SIP;
	naptFilterAndQos.ingress_src_ipv4_addr = *ingress_src_ipv4_addr_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_pattern_ingress_src_ipv4_addr_ingress_src_ipv4_addr */

/*
 * rg set naptFilterAndQos pattern egress_src_ipv4_addr <IPV4ADDR:egress_src_ipv4_addr>
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_pattern_egress_src_ipv4_addr_egress_src_ipv4_addr(
    cparser_context_t *context,
    uint32_t  *egress_src_ipv4_addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	naptFilterAndQos.filter_fields |= EGRESS_SIP;
	naptFilterAndQos.egress_src_ipv4_addr = *egress_src_ipv4_addr_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_pattern_egress_src_ipv4_addr_egress_src_ipv4_addr */

/*
 * rg set naptFilterAndQos pattern ingress_dest_ipv4_addr <IPV4ADDR:ingress_dest_ipv4_addr>
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_pattern_ingress_dest_ipv4_addr_ingress_dest_ipv4_addr(
    cparser_context_t *context,
    uint32_t  *ingress_dest_ipv4_addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	naptFilterAndQos.filter_fields |= INGRESS_DIP;
	naptFilterAndQos.ingress_dest_ipv4_addr = *ingress_dest_ipv4_addr_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_pattern_ingress_dest_ipv4_addr_ingress_dest_ipv4_addr */

/*
 * rg set naptFilterAndQos pattern egress_dest_ipv4_addr <IPV4ADDR:egress_dest_ipv4_addr>
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_pattern_egress_dest_ipv4_addr_egress_dest_ipv4_addr(
    cparser_context_t *context,
    uint32_t  *egress_dest_ipv4_addr_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	naptFilterAndQos.filter_fields |= EGRESS_DIP;
	naptFilterAndQos.egress_dest_ipv4_addr = *egress_dest_ipv4_addr_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_pattern_egress_dest_ipv4_addr_egress_dest_ipv4_addr */

/*
 * rg set naptFilterAndQos pattern ingress_src_l4_port <UINT:ingress_src_l4_port>
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_pattern_ingress_src_l4_port_ingress_src_l4_port(
    cparser_context_t *context,
    uint32_t  *ingress_src_l4_port_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	naptFilterAndQos.filter_fields |= INGRESS_SPORT;
	naptFilterAndQos.ingress_src_l4_port = *ingress_src_l4_port_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_pattern_ingress_src_l4_port_ingress_src_l4_port */

/*
 * rg set naptFilterAndQos pattern egress_src_l4_port <UINT:egress_src_l4_port>
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_pattern_egress_src_l4_port_egress_src_l4_port(
    cparser_context_t *context,
    uint32_t  *egress_src_l4_port_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	naptFilterAndQos.filter_fields |= EGRESS_SPORT;
	naptFilterAndQos.egress_src_l4_port = *egress_src_l4_port_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_pattern_egress_src_l4_port_egress_src_l4_port */

/*
 * rg set naptFilterAndQos pattern ingress_dest_l4_port <UINT:ingress_dest_l4_port>
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_pattern_ingress_dest_l4_port_ingress_dest_l4_port(
    cparser_context_t *context,
    uint32_t  *ingress_dest_l4_port_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	naptFilterAndQos.filter_fields |= INGRESS_DPORT;
	naptFilterAndQos.ingress_dest_l4_port = *ingress_dest_l4_port_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_pattern_ingress_dest_l4_port_ingress_dest_l4_port */

/*
 * rg set naptFilterAndQos pattern egress_dest_l4_port <UINT:egress_dest_l4_port>
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_pattern_egress_dest_l4_port_egress_dest_l4_port(
    cparser_context_t *context,
    uint32_t  *egress_dest_l4_port_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	naptFilterAndQos.filter_fields |= EGRESS_DPORT;
	naptFilterAndQos.egress_dest_l4_port = *egress_dest_l4_port_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_pattern_egress_dest_l4_port_egress_dest_l4_port */

/*
 * rg set naptFilterAndQos pattern ingress_l4_protocal <UINT:ingress_l4_protocal>
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_pattern_ingress_l4_protocal_ingress_l4_protocal(
    cparser_context_t *context,
    uint32_t  *ingress_l4_protocal_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	naptFilterAndQos.filter_fields |= L4_PROTOCAL;
	naptFilterAndQos.ingress_l4_protocal = *ingress_l4_protocal_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_pattern_ingress_l4_protocal_ingress_l4_protocal */

/*
 * rg set naptFilterAndQos pattern ingress_src_ipv4_addr_range ingress_src_ipv4_addr_range_start <IPV4ADDR:ingress_src_ipv4_addr_range_start> ingress_src_ipv4_addr_range_end <IPV4ADDR:ingress_src_ipv4_addr_range_end>
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_pattern_ingress_src_ipv4_addr_range_ingress_src_ipv4_addr_range_start_ingress_src_ipv4_addr_range_start_ingress_src_ipv4_addr_range_end_ingress_src_ipv4_addr_range_end(
    cparser_context_t *context,
    uint32_t  *ingress_src_ipv4_addr_range_start_ptr,
    uint32_t  *ingress_src_ipv4_addr_range_end_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	naptFilterAndQos.filter_fields |= INGRESS_SIP_RANGE;
	naptFilterAndQos.ingress_src_ipv4_addr_range_start = *ingress_src_ipv4_addr_range_start_ptr;
	naptFilterAndQos.ingress_src_ipv4_addr_range_end = *ingress_src_ipv4_addr_range_end_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_pattern_ingress_src_ipv4_addr_range_ingress_src_ipv4_addr_range_start_ingress_src_ipv4_addr_range_start_ingress_src_ipv4_addr_range_end_ingress_src_ipv4_addr_range_end */

/*
 * rg set naptFilterAndQos pattern ingress_dest_ipv4_addr_range ingress_dest_ipv4_addr_range_start <IPV4ADDR:ingress_dest_ipv4_addr_range_start> ingress_dest_ipv4_addr_range_end <IPV4ADDR:ingress_dest_ipv4_addr_range_end>
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_pattern_ingress_dest_ipv4_addr_range_ingress_dest_ipv4_addr_range_start_ingress_dest_ipv4_addr_range_start_ingress_dest_ipv4_addr_range_end_ingress_dest_ipv4_addr_range_end(
    cparser_context_t *context,
    uint32_t  *ingress_dest_ipv4_addr_range_start_ptr,
    uint32_t  *ingress_dest_ipv4_addr_range_end_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	naptFilterAndQos.filter_fields |= INGRESS_DIP_RANGE;
	naptFilterAndQos.ingress_dest_ipv4_addr_range_start = *ingress_dest_ipv4_addr_range_start_ptr;
	naptFilterAndQos.ingress_dest_ipv4_addr_range_end = *ingress_dest_ipv4_addr_range_end_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_pattern_ingress_dest_ipv4_addr_range_ingress_dest_ipv4_addr_range_start_ingress_dest_ipv4_addr_range_start_ingress_dest_ipv4_addr_range_end_ingress_dest_ipv4_addr_range_end */

/*
 * rg set naptFilterAndQos pattern ingress_src_l4_port_range ingress_src_l4_port_range_start <UINT:ingress_src_l4_port_range_start> ingress_src_l4_port_range_end <UINT:ingress_src_l4_port_range_end>
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_pattern_ingress_src_l4_port_range_ingress_src_l4_port_range_start_ingress_src_l4_port_range_start_ingress_src_l4_port_range_end_ingress_src_l4_port_range_end(
    cparser_context_t *context,
    uint32_t  *ingress_src_l4_port_range_start_ptr,
    uint32_t  *ingress_src_l4_port_range_end_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	naptFilterAndQos.filter_fields |= INGRESS_SPORT_RANGE;
	naptFilterAndQos.ingress_src_l4_port_range_start = *ingress_src_l4_port_range_start_ptr;
	naptFilterAndQos.ingress_src_l4_port_range_end = *ingress_src_l4_port_range_end_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_pattern_ingress_src_l4_port_range_ingress_src_l4_port_range_start_ingress_src_l4_port_range_start_ingress_src_l4_port_range_end_ingress_src_l4_port_range_end */

/*
 * rg set naptFilterAndQos pattern ingress_dest_l4_port_range ingress_dest_l4_port_range_start <UINT:ingress_dest_l4_port_range_start> ingress_dest_l4_port_range_end <UINT:ingress_dest_l4_port_range_end>
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_pattern_ingress_dest_l4_port_range_ingress_dest_l4_port_range_start_ingress_dest_l4_port_range_start_ingress_dest_l4_port_range_end_ingress_dest_l4_port_range_end(
    cparser_context_t *context,
    uint32_t  *ingress_dest_l4_port_range_start_ptr,
    uint32_t  *ingress_dest_l4_port_range_end_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	naptFilterAndQos.filter_fields |= INGRESS_DPORT_RANGE;
	naptFilterAndQos.ingress_dest_l4_port_range_start = *ingress_dest_l4_port_range_start_ptr;
	naptFilterAndQos.ingress_dest_l4_port_range_end = *ingress_dest_l4_port_range_end_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_pattern_ingress_dest_l4_port_range_ingress_dest_l4_port_range_start_ingress_dest_l4_port_range_start_ingress_dest_l4_port_range_end_ingress_dest_l4_port_range_end */



/*
 * rg set naptFilterAndQos pattern egress_src_ipv4_addr_range egress_src_ipv4_addr_range_start <IPV4ADDR:egress_src_ipv4_addr_range_start> egress_src_ipv4_addr_range_end <IPV4ADDR:egress_src_ipv4_addr_range_end>
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_pattern_egress_src_ipv4_addr_range_egress_src_ipv4_addr_range_start_egress_src_ipv4_addr_range_start_egress_src_ipv4_addr_range_end_egress_src_ipv4_addr_range_end(
    cparser_context_t *context,
    uint32_t  *egress_src_ipv4_addr_range_start_ptr,
    uint32_t  *egress_src_ipv4_addr_range_end_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	naptFilterAndQos.filter_fields |= EGRESS_SIP_RANGE;
	naptFilterAndQos.egress_src_ipv4_addr_range_start= *egress_src_ipv4_addr_range_start_ptr;
	naptFilterAndQos.egress_src_ipv4_addr_range_end= *egress_src_ipv4_addr_range_end_ptr;


    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_pattern_egress_src_ipv4_addr_range_egress_src_ipv4_addr_range_start_egress_src_ipv4_addr_range_start_egress_src_ipv4_addr_range_end_egress_src_ipv4_addr_range_end */

/*
 * rg set naptFilterAndQos pattern egress_dest_ipv4_addr_range egress_dest_ipv4_addr_range_start <IPV4ADDR:egress_dest_ipv4_addr_range_start> egress_dest_ipv4_addr_range_end <IPV4ADDR:egress_dest_ipv4_addr_range_end>
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_pattern_egress_dest_ipv4_addr_range_egress_dest_ipv4_addr_range_start_egress_dest_ipv4_addr_range_start_egress_dest_ipv4_addr_range_end_egress_dest_ipv4_addr_range_end(
    cparser_context_t *context,
    uint32_t  *egress_dest_ipv4_addr_range_start_ptr,
    uint32_t  *egress_dest_ipv4_addr_range_end_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	naptFilterAndQos.filter_fields |= EGRESS_DIP_RANGE;
	naptFilterAndQos.egress_dest_ipv4_addr_range_start= *egress_dest_ipv4_addr_range_start_ptr;
	naptFilterAndQos.egress_dest_ipv4_addr_range_end= *egress_dest_ipv4_addr_range_end_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_pattern_egress_dest_ipv4_addr_range_egress_dest_ipv4_addr_range_start_egress_dest_ipv4_addr_range_start_egress_dest_ipv4_addr_range_end_egress_dest_ipv4_addr_range_end */

/*
 * rg set naptFilterAndQos pattern egress_src_l4_port_range egress_src_l4_port_range_start <UINT:egress_src_l4_port_range_start> egress_src_l4_port_range_end <UINT:egress_src_l4_port_range_end>
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_pattern_egress_src_l4_port_range_egress_src_l4_port_range_start_egress_src_l4_port_range_start_egress_src_l4_port_range_end_egress_src_l4_port_range_end(
    cparser_context_t *context,
    uint32_t  *egress_src_l4_port_range_start_ptr,
    uint32_t  *egress_src_l4_port_range_end_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	naptFilterAndQos.filter_fields |= EGRESS_SPORT_RANGE;
	naptFilterAndQos.egress_src_l4_port_range_start= *egress_src_l4_port_range_start_ptr;
	naptFilterAndQos.egress_src_l4_port_range_end= *egress_src_l4_port_range_end_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_pattern_egress_src_l4_port_range_egress_src_l4_port_range_start_egress_src_l4_port_range_start_egress_src_l4_port_range_end_egress_src_l4_port_range_end */

/*
 * rg set naptFilterAndQos pattern egress_dest_l4_port_range egress_dest_l4_port_range_start <UINT:egress_dest_l4_port_range_start> egress_dest_l4_port_range_end <UINT:egress_dest_l4_port_range_end>
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_pattern_egress_dest_l4_port_range_egress_dest_l4_port_range_start_egress_dest_l4_port_range_start_egress_dest_l4_port_range_end_egress_dest_l4_port_range_end(
    cparser_context_t *context,
    uint32_t  *egress_dest_l4_port_range_start_ptr,
    uint32_t  *egress_dest_l4_port_range_end_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	naptFilterAndQos.filter_fields |= EGRESS_DPORT_RANGE;
	naptFilterAndQos.egress_dest_l4_port_range_start= *egress_dest_l4_port_range_start_ptr;
	naptFilterAndQos.egress_dest_l4_port_range_end= *egress_dest_l4_port_range_end_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_pattern_egress_dest_l4_port_range_egress_dest_l4_port_range_start_egress_dest_l4_port_range_start_egress_dest_l4_port_range_end_egress_dest_l4_port_range_end */



/*
 * rg set naptFilterAndQos weight <UINT:weight>
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_weight_weight(
    cparser_context_t *context,
    uint32_t  *weight_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	naptFilterAndQos.weight = *weight_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_weight_weight */


/*
 * rg set naptFilterAndQos action assign_priority <UINT:assign_priority>
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_action_assign_priority_assign_priority(
    cparser_context_t *context,
    uint32_t  *assign_priority_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	naptFilterAndQos.action_fields |=  ASSIGN_NAPT_PRIORITY_BIT;
	naptFilterAndQos.assign_priority = *assign_priority_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_action_assign_priority_assign_priority */

/*
 * rg set naptFilterAndQos action drop
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_action_drop(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	naptFilterAndQos.action_fields |= NAPT_DROP_BIT;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_action_drop */

/*
 * rg set naptFilterAndQos action permit
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_action_permit(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	naptFilterAndQos.action_fields |= NAPT_PERMIT_BIT;
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_action_permit */

/*
 * rg set naptFilterAndQos ruleType <UINT:ruleType>
 */
cparser_result_t
cparser_cmd_rg_set_naptFilterAndQos_ruleType_ruleType(
    cparser_context_t *context,
    uint32_t  *ruleType_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	naptFilterAndQos.ruleType=*ruleType_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_naptfilterandqos_ruletype_ruletype */

/*
 * rg add naptFilterAndQos entry
 */
cparser_result_t
cparser_cmd_rg_add_naptFilterAndQos_entry(
    cparser_context_t *context)
{
	int ret;
	int index;
    DIAG_UTIL_PARAM_CHK();
	
	ret =  rtk_rg_naptFilterAndQos_add(&index, &naptFilterAndQos);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add rtk_rg_naptFilterAndQos_add failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else{
		diag_util_mprintf("add rtk_rg_naptFilterAndQos_add[%d] success.\n",index);
	}

    return CPARSER_OK;

}    /* end of cparser_cmd_rg_add_naptfilterandqos_entry */


/*
 * rg del naptFilterAndQos entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_del_naptFilterAndQos_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_naptFilterAndQos_del(*entry_index_ptr);
	if(ret!=RT_ERR_RG_OK)
		return CPARSER_NOT_OK;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_del_naptfilterandqos_entry_entry_index */

/*
 * rg get naptFilterAndQos entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_get_naptFilterAndQos_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int ret;
	int index=*entry_index_ptr;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
	osal_memset(&naptFilterAndQos, 0x0, sizeof(rtk_rg_naptFilterAndQos_t));

 	ret = rtk_rg_naptFilterAndQos_find(&index, &naptFilterAndQos);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("get rtk_rg_naptFilterAndQos_find failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{

		if(index==*entry_index_ptr){	
			_diag_rgNaptFilterAndQosShow(&naptFilterAndQos);
		}
		else{
			diag_util_mprintf("rtk_rg_naptFilterAndQos_find[%d] is empty. index=%d\n",*entry_index_ptr,index);
			return CPARSER_NOT_OK;
		}
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_naptfilterandqos_entry_entry_index */

/*
 * rg set stpBlockingPortmask port_mask <HEX:portmask>
 */
cparser_result_t
cparser_cmd_rg_set_stpBlockingPortmask_port_mask_portmask(
    cparser_context_t *context,
    uint32_t  *portmask_ptr)
{
    int ret;
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_stpBlockingPortmask_set(*portmask_ptr);
	//diag_util_mprintf("rtk_rg_stpBlockingPortmask_set(%d)=%d",ret,*portmask_ptr);
    if(ret!=RT_ERR_RG_OK)
            return CPARSER_NOT_OK;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_stpblockingportmask_port_mask_portmask */

/*
 * rg get stpBlockingPortmask
 */
cparser_result_t
cparser_cmd_rg_get_stpBlockingPortmask(
    cparser_context_t *context)
{
	uint32_t  Mask;
	int ret;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_rg_stpBlockingPortmask_get(&Mask);	
    diag_util_mprintf("rtk_rg_stpBlockingPortmask_get 0x%x",Mask);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_stpblockingportmask */


/*
 * rg show gponDsBcFilter
 */
cparser_result_t
cparser_cmd_rg_show_gponDsBcFilter(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	_diag_rgGponDsBcFilterAndRemarkingShow(&gponDsBcFilterAndRemarking);
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_gpondsbcfilter */

/*
 * rg clear gponDsBcFilter
 */
cparser_result_t
cparser_cmd_rg_clear_gponDsBcFilter(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();	
	osal_memset(&gponDsBcFilterAndRemarking, 0x0, sizeof(rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t));

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_gpondsbcfilter */

/*
 * rg set gponDsBcFilter module <UINT:enable>
 */
cparser_result_t
cparser_cmd_rg_set_gponDsBcFilter_module_enable(
    cparser_context_t *context,
    uint32_t  *enable_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	int ret;
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_gponDsBcFilterAndRemarking_Enable(*enable_ptr);

	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("rtk_rg_gponDsBcFilterAndRemarking_Enable failed. rg API return 0x%x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{		
		diag_util_mprintf("add rtk_rg_gponDsBcFilterAndRemarking_Enable(%s) success.\n",(*enable_ptr)?"enable":"disable");
		return CPARSER_OK;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_gpondsbcfilter_module_enable */



/*
 * rg get gponDsBcFilter entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_get_gponDsBcFilter_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int index = *entry_index_ptr;
	int ret;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
	osal_memset(&gponDsBcFilterAndRemarking, 0x0, sizeof(rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t));
	ret = rtk_rg_gponDsBcFilterAndRemarking_find(&index,&gponDsBcFilterAndRemarking);

	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("rtk_rg_gponDsBcFilterAndRemarking_find failed. rg API return 0x%x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{		
		_diag_rgGponDsBcFilterAndRemarkingShow(&gponDsBcFilterAndRemarking);
		return CPARSER_OK;
	}
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_gpondsbcfilter_entry_entry_index */


/*
 * rg del gponDsBcFilter entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_del_gponDsBcFilter_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_gponDsBcFilterAndRemarking_del(*entry_index_ptr);

	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("rtk_rg_gponDsBcFilterAndRemarking_del failed. rg API return 0x%x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{		
		diag_util_mprintf("add rtk_rg_gponDsBcFilterAndRemarking_del[%d] success.\n",*entry_index_ptr);
		return CPARSER_OK;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_del_gpondsbcfilter_entry_entry_index */

/*
 * rg del gponDsBcFilter entry all
 */
cparser_result_t
cparser_cmd_rg_del_gponDsBcFilter_entry_all(
    cparser_context_t *context)
{
	int i,ret;
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_gponDsBcFilterAndRemarking_del_all();
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("rtk_rg_gponDsBcFilterAndRemarking_del[all] failed. rg API return 0x%x \n",ret);
	}
	else
	{
		diag_util_mprintf("clear rtk_rg_gponDsBcFilterAndRemarking_del[all] done.\n");
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_del_gpondsbcfilter_entry_all */



/*
 * rg add gponDsBcFilter entry
 */
cparser_result_t
cparser_cmd_rg_add_gponDsBcFilter_entry(
    cparser_context_t *context)
{
	int index = -1; //add to first empty entry
	int ret;
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_gponDsBcFilterAndRemarking_add(&gponDsBcFilterAndRemarking,&index);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("rtk_rg_gponDsBcFilterAndRemarking_add failed. rg API return 0x%x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{	
		diag_util_mprintf("add rtk_rg_gponDsBcFilterAndRemarking_add[%d] success.\n",index);
		return CPARSER_OK;
	}
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_gpondsbcfilter_entry */

/*
 * rg add gponDsBcFilter entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_add_gponDsBcFilter_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int index =  *entry_index_ptr; //add to specific entry 
	int ret;
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_gponDsBcFilterAndRemarking_add(&gponDsBcFilterAndRemarking,&index);
	if(ret!=RT_ERR_RG_OK)
	{
		diag_util_mprintf("rtk_rg_gponDsBcFilterAndRemarking_add failed. rg API return 0x%x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{	
		diag_util_mprintf("add rtk_rg_gponDsBcFilterAndRemarking_add[%d] success.\n",index);
		return CPARSER_OK;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_gpondsbcfilter_entry_entry_index */

/*
 * rg set gponDsBcFilter pattern ingress_stream_id <UINT:ingress_stream_id>
 */
cparser_result_t
cparser_cmd_rg_set_gponDsBcFilter_pattern_ingress_stream_id_ingress_stream_id(
    cparser_context_t *context,
    uint32_t  *ingress_stream_id_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	gponDsBcFilterAndRemarking.filter_fields|=GPON_DS_BC_FILTER_INGRESS_STREAMID_BIT;
	gponDsBcFilterAndRemarking.ingress_stream_id = *ingress_stream_id_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_gpondsbcfilter_pattern_ingress_stream_id_ingress_stream_id */

/*
 * rg set gponDsBcFilter pattern ingress_stagIf <UINT:ingress_stagIf>
 */
cparser_result_t
cparser_cmd_rg_set_gponDsBcFilter_pattern_ingress_stagIf_ingress_stagIf(
    cparser_context_t *context,
    uint32_t  *ingress_stagIf_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	gponDsBcFilterAndRemarking.filter_fields|=GPON_DS_BC_FILTER_INGRESS_STAGIf_BIT;
	gponDsBcFilterAndRemarking.ingress_stagIf= *ingress_stagIf_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_gpondsbcfilter_pattern_ingress_stagif_ingress_stagif */

/*
 * rg set gponDsBcFilter pattern ingress_ctagIf <UINT:ingress_ctagIf>
 */
cparser_result_t
cparser_cmd_rg_set_gponDsBcFilter_pattern_ingress_ctagIf_ingress_ctagIf(
    cparser_context_t *context,
    uint32_t  *ingress_ctagIf_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	gponDsBcFilterAndRemarking.filter_fields|=GPON_DS_BC_FILTER_INGRESS_CTAGIf_BIT;
	gponDsBcFilterAndRemarking.ingress_ctagIf= *ingress_ctagIf_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_gpondsbcfilter_pattern_ingress_ctagif_ingress_ctagif */

/*
 * rg set gponDsBcFilter pattern ingress_stag_svid <UINT:ingress_stag_svid>
 */
cparser_result_t
cparser_cmd_rg_set_gponDsBcFilter_pattern_ingress_stag_svid_ingress_stag_svid(
    cparser_context_t *context,
    uint32_t  *ingress_stag_svid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	gponDsBcFilterAndRemarking.filter_fields|=GPON_DS_BC_FILTER_INGRESS_SVID_BIT;
	gponDsBcFilterAndRemarking.ingress_stag_svid= *ingress_stag_svid_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_gpondsbcfilter_pattern_ingress_stag_svid_ingress_stag_svid */

/*
 * rg set gponDsBcFilter pattern ingress_ctag_cvid <UINT:ingress_ctag_cvid>
 */
cparser_result_t
cparser_cmd_rg_set_gponDsBcFilter_pattern_ingress_ctag_cvid_ingress_ctag_cvid(
    cparser_context_t *context,
    uint32_t  *ingress_ctag_cvid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	gponDsBcFilterAndRemarking.filter_fields|=GPON_DS_BC_FILTER_INGRESS_CVID_BIT;
	gponDsBcFilterAndRemarking.ingress_ctag_cvid= *ingress_ctag_cvid_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_gpondsbcfilter_pattern_ingress_ctag_cvid_ingress_ctag_cvid */

/*
 * rg set gponDsBcFilter pattern egress_portmask <HEX:egress_portmask>
 */
cparser_result_t
cparser_cmd_rg_set_gponDsBcFilter_pattern_egress_portmask_egress_portmask(
    cparser_context_t *context,
    uint32_t  *egress_portmask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	gponDsBcFilterAndRemarking.filter_fields|=GPON_DS_BC_FILTER_EGRESS_PORT_BIT;
	gponDsBcFilterAndRemarking.egress_portmask.portmask= *egress_portmask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_gpondsbcfilter_pattern_egress_portmask_egress_portmask */

/*
 * rg set gponDsBcFilter action tag_decision <UINT:tag_decision> tag_cvid <UINT:tag_cvid> tag_cpri <UINT:tag_cpri>
 */
cparser_result_t
cparser_cmd_rg_set_gponDsBcFilter_action_tag_decision_tag_decision_tag_cvid_tag_cvid_tag_cpri_tag_cpri(
    cparser_context_t *context,
    uint32_t  *tag_decision_ptr,
    uint32_t  *tag_cvid_ptr,
    uint32_t  *tag_cpri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	gponDsBcFilterAndRemarking.ctag_action.ctag_decision = *tag_decision_ptr;
	gponDsBcFilterAndRemarking.ctag_action.assigned_ctag_cvid = *tag_cvid_ptr;
	gponDsBcFilterAndRemarking.ctag_action.assigned_ctag_cpri = *tag_cpri_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_gpondsbcfilter_action_tag_decision_tag_decision_tag_cvid_tag_cvid_tag_cpri_tag_cpri */


/*
 * rg set dsliteMc index <UINT:index> mprefix64_addr <IPV6ADDR:mprefix64_addr> mprefix64_mask <IPV6ADDR:mprefix64_mask> uprefix64_addr <IPV6ADDR:uprefix64_addr> uprefix64_mask <IPV6ADDR:uprefix64_mask>
 */
cparser_result_t
cparser_cmd_rg_set_dsliteMc_index_index_mprefix64_addr_mprefix64_addr_mprefix64_mask_mprefix64_mask_uprefix64_addr_uprefix64_addr_uprefix64_mask_uprefix64_mask(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    char * *mprefix64_addr_ptr,
    char * *mprefix64_mask_ptr,
    char * *uprefix64_addr_ptr,
    char * *uprefix64_mask_ptr)
{
	int ret;
	rtk_l34_dsliteMc_entry_t dsliteMc;
    DIAG_UTIL_PARAM_CHK();

	dsliteMc.index=*index_ptr;
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(dsliteMc.ipMPrefix64.ipv6_addr, TOKEN_STR(6)), ret);
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(dsliteMc.ipMPrefix64Mask.ipv6_addr, TOKEN_STR(8)), ret);
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(dsliteMc.ipUPrefix64.ipv6_addr, TOKEN_STR(10)), ret);
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(dsliteMc.ipUPrefix64Mask.ipv6_addr, TOKEN_STR(12)), ret);
	ret =  rtk_rg_dsliteMcTable_set(&dsliteMc);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("set dsliteMc[%d] failed. rg API return %x \n",*index_ptr,ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("set dsliteMc[%d] success.\n",*index_ptr);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_dslitemc_index_index_mprefix64_addr_mprefix64_addr_mprefix64_mask_mprefix64_mask_uprefix64_addr_uprefix64_addr_uprefix64_mask_uprefix64_mask */

/*
 * rg get dsliteMc index <UINT:index> */
cparser_result_t
cparser_cmd_rg_get_dsliteMc_index_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
	int ret;
	rtk_l34_dsliteMc_entry_t dsliteMc;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	dsliteMc.index=*index_ptr;
    ret =  rtk_rg_dsliteMcTable_get(&dsliteMc);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("get dsliteMc[%d] failed. rg API return %x \n",*index_ptr,ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("dsliteMc[%d] success.\n",*index_ptr);
		diag_util_mprintf("\tmcast_prefix64: %s\n",diag_util_inet_n6toa(dsliteMc.ipMPrefix64.ipv6_addr));
		diag_util_mprintf("\tmcast_prefixMask: %s\n",diag_util_inet_n6toa(dsliteMc.ipMPrefix64Mask.ipv6_addr));
		diag_util_mprintf("\tsource_prefix64: %s\n",diag_util_inet_n6toa(dsliteMc.ipUPrefix64.ipv6_addr));
		diag_util_mprintf("\tsource_prefixMask: %s\n",diag_util_inet_n6toa(dsliteMc.ipUPrefix64Mask.ipv6_addr));
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_dslitemc_index_index */

/*
 * rg del interfaceMibCounter index <UINT:index>
 */
cparser_result_t
cparser_cmd_rg_del_interfaceMibCounter_index_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
	int ret;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	
	ret =  rtk_rg_interfaceMibCounter_del(*index_ptr);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("del interfaceMibCounter[%d] failed. rg API return %x \n",*index_ptr,ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("del interfaceMibCounter[%d] success.\n",*index_ptr);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_del_interfaceMibcounter_index_index */

/*
 * rg get interfaceMibCounter index <UINT:index>
 */
cparser_result_t
cparser_cmd_rg_get_interfaceMibCounter_index_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
	int ret;
	rtk_l34_mib_t mib;
	
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	mib.ifIndex=*index_ptr;
    ret =  rtk_rg_interfaceMibCounter_get(&mib);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("get interfaceMibCounter[%d] failed. rg API return %x \n",*index_ptr,ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("interfaceMibCounter[%d]:\n",*index_ptr);
		diag_util_mprintf("\tifInOctets:%llu\n",mib.ifInOctets);
		diag_util_mprintf("\tifInUcstPkts:%u\n",mib.ifInUcstPkts);
		diag_util_mprintf("\tifOutOctets:%llu\n",mib.ifOutOctets);
		diag_util_mprintf("\tifOutUcstPkts:%u\n",mib.ifOutUcstPkts);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_interfaceMibcounter_index_index */

/*
 * rg set redirectHttpAll disable
 */
cparser_result_t
cparser_cmd_rg_set_redirectHttpAll_disable(
    cparser_context_t *context)
{
    int ret;

    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	redInfoAll.enable=0;
	ret = rtk_rg_redirectHttpAll_set(&redInfoAll);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("set redirectHttpAll failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("set redirectHttpAll Disable success.\n");
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_redirecthttpall_disable */

/*
 * rg set redirectHttpAll enable <INT:enable> pushweb <STRING:pushweb>
 */
cparser_result_t
cparser_cmd_rg_set_redirectHttpAll_enable_enable_pushweb_pushweb(
    cparser_context_t *context,
    int32_t  *enable_ptr,
    char * *pushweb_ptr)
{
    int ret;

    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	osal_memcpy(redInfoAll.pushweb, *pushweb_ptr, MAX_REDIRECT_PUSH_WEB_SIZE);
	redInfoAll.enable=*enable_ptr;
	ret = rtk_rg_redirectHttpAll_set(&redInfoAll);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("set redirectHttpAll failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("set redirectHttpAll \"%s\" %s success.\n",*pushweb_ptr,*enable_ptr==1?"Enable":"Disable");
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_redirecthttpall_enable_enable_pushweb_pushweb */

/*
 * rg get redirectHttpAll
 */
cparser_result_t
cparser_cmd_rg_get_redirectHttpAll(
    cparser_context_t *context)
{
	int ret;
	
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    ret = rtk_rg_redirectHttpAll_get(&redInfoAll);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("get redirectHttpAll failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("get redirectHttpAll \"%s\" %s success.\n",redInfoAll.pushweb,redInfoAll.enable==1?"Enable":"Disable");
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_redirecthttpall */

/*
 * rg add redirectHttpURL url <STRING:url> dst_url <STRING:dst_url> count <INT:count>
 */
cparser_result_t
cparser_cmd_rg_add_redirectHttpURL_url_url_dst_url_dst_url_count_count(
    cparser_context_t *context,
    char * *url_ptr,
    char * *dst_url_ptr,
    int32_t  *count_ptr)
{
   	int ret;
	rtk_rg_redirectHttpURL_t redInfo;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	osal_memcpy(redInfo.url_str, *url_ptr, MAX_URL_FILTER_STR_LENGTH);
	osal_memcpy(redInfo.dst_url_str, *dst_url_ptr, MAX_URL_FILTER_STR_LENGTH);
	redInfo.count=*count_ptr;
	ret = rtk_rg_redirectHttpURL_add(&redInfo);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add redirectHttpURL failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("add redirectHttpURL \"%s\" count %d success.\n",*url_ptr,*count_ptr);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_redirecthttpurl_url_url_dst_url_dst_url_count_count */

/*
 * rg del redirectHttpURL url <STRING:url>
 */
cparser_result_t
cparser_cmd_rg_del_redirectHttpURL_url_url(
    cparser_context_t *context,
    char * *url_ptr)
{
    int ret;
	rtk_rg_redirectHttpURL_t redInfo;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	osal_memcpy(redInfo.url_str, *url_ptr, MAX_URL_FILTER_STR_LENGTH);
	ret = rtk_rg_redirectHttpURL_del(&redInfo);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("del redirectHttpURL failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("del redirectHttpURL \"%s\" success.\n",*url_ptr);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_del_redirecthttpurl_url_url */

/*
 * rg add redirectHttpWhiteList url <STRING:url> keyword <STRING:keyword>
 */
cparser_result_t
cparser_cmd_rg_add_redirectHttpWhiteList_url_url_keyword_keyword(
    cparser_context_t *context,
    char * *url_ptr,
    char * *keyword_ptr)
{
    int ret;
	rtk_rg_redirectHttpWhiteList_t redInfo;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	osal_memcpy(redInfo.url_str, *url_ptr, MAX_URL_FILTER_STR_LENGTH);
	osal_memcpy(redInfo.keyword_str, *keyword_ptr, MAX_URL_FILTER_STR_LENGTH);
	ret = rtk_rg_redirectHttpWhiteList_add(&redInfo);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("add redirectHttpWhiteList failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("add redirectHttpWhiteList \"%s\" keyword \"%s\"success.\n",*url_ptr,*keyword_ptr);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_redirecthttpwhitelist_url_url_keyword_keyword */

/*
 * rg del redirectHttpWhiteList url <STRING:url> keyword <STRING:keyword>
 */
cparser_result_t
cparser_cmd_rg_del_redirectHttpWhiteList_url_url_keyword_keyword(
    cparser_context_t *context,
    char * *url_ptr,
    char * *keyword_ptr)
{
    int ret;
	rtk_rg_redirectHttpWhiteList_t redInfo;
	
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	osal_memcpy(redInfo.url_str, *url_ptr, MAX_URL_FILTER_STR_LENGTH);
	osal_memcpy(redInfo.keyword_str, *keyword_ptr, MAX_URL_FILTER_STR_LENGTH);
	ret = rtk_rg_redirectHttpWhiteList_del(&redInfo);
	if(ret!=RT_ERR_RG_OK){
		diag_util_mprintf("del redirectHttpWhiteList failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("del redirectHttpWhiteList \"%s\" keyword \"%s\"success.\n",*url_ptr,*keyword_ptr);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_del_redirecthttpwhitelist_url_url_keyword_keyword */

