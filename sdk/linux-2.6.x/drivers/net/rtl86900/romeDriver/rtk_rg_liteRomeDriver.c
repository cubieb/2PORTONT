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

#ifndef __RTK_RG_LITEROMEDRIVER_H__
#define __RTK_RG_LITEROMEDRIVER_H__

#include <rtk_rg_define.h>
#include <rtk_rg_struct.h>
#include <rtk_rg_liteRomeDriver.h>
#include <rtk_rg_internal.h>

#ifdef __linux__
#include <linux/init.h>
#include <linux/slab.h> //for kmalloc/kfree
#include <linux/delay.h> //for mdelay
#include <linux/skbuff.h>
#include <linux/timer.h>
#include <linux/proc_fs.h> //for create proc
#ifdef CONFIG_DEFAULTS_KERNEL_3_18
#else
#include <linux/config.h>
#endif
#include <linux/netdevice.h>
#endif

#ifdef CONFIG_DEFAULTS_KERNEL_3_18
static DEFINE_SPINLOCK(rgApiLock);
#else
spinlock_t rgApiLock = SPIN_LOCK_UNLOCKED; //used when each RG API is called.
#endif


unsigned long spinflags;
atomic_t rg_api_entrance = ATOMIC_INIT(0);

#define rg_api_lock_irqsave(lock, flags)			\
	do {						\
		if(atomic_read(&rg_api_entrance)>=1)	\
			WARNING("RG API LOCK REDUPLICATE ENTRANCE!!");	\
		atomic_inc(&rg_api_entrance);	\
		preempt_disable(); \
		local_bh_disable(); \
		spin_lock_irqsave(lock, flags);	\
		RGAPI("[Enter]"); \
	} while (0)

#define rg_api_unlock_irqrestore(lock, flags)			\
		do {						\
			RGAPI("[Exit]"); \
			spin_unlock_irqrestore(lock, flags);	\
			preempt_enable(); \
			local_bh_enable(); \
			atomic_dec(&rg_api_entrance);	\
			if(atomic_read(&rg_api_entrance)<0)	\
				WARNING("RG API UNLOCK REDUPLICATE RELEASE!!");	\
		} while (0)

int32 rtk_rg_driverVersion_get(rtk_rg_VersionString_t *version_string)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_driverVersion_get(version_string);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_initParam_get(rtk_rg_initParams_t *init_param)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_initParam_get(init_param);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_initParam_set(rtk_rg_initParams_t *init_param)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_initParam_set(init_param);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_lanInterface_add(rtk_rg_lanIntfConf_t *lan_info,int *intf_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_lanInterface_add(lan_info,intf_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_wanInterface_add(rtk_rg_wanIntfConf_t *wanintf, int *wan_intf_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_wanInterface_add(wanintf,wan_intf_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_staticInfo_set(int wan_intf_idx, rtk_rg_ipStaticInfo_t *static_info)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_staticInfo_set(wan_intf_idx,static_info);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_dsliteInfo_set(int wan_intf_idx, rtk_rg_ipDslitStaticInfo_t *dslite_info)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_dsliteInfo_set(wan_intf_idx,dslite_info);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_dhcpRequest_set(int wan_intf_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_dhcpRequest_set(wan_intf_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_dhcpClientInfo_set(int wan_intf_idx, rtk_rg_ipDhcpClientInfo_t *dhcpClient_info)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_dhcpClientInfo_set(wan_intf_idx,dhcpClient_info);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_pppoeClientInfoBeforeDial_set(int wan_intf_idx, rtk_rg_pppoeClientInfoBeforeDial_t *app_info)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_pppoeClientInfoBeforeDial_set(wan_intf_idx,app_info);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_pppoeClientInfoAfterDial_set(int wan_intf_idx, rtk_rg_pppoeClientInfoAfterDial_t *clientPppoe_info)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_pppoeClientInfoAfterDial_set(wan_intf_idx,clientPppoe_info);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_pptpClientInfoBeforeDial_set(int wan_intf_idx, rtk_rg_pptpClientInfoBeforeDial_t *app_info)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_pptpClientInfoBeforeDial_set(wan_intf_idx,app_info);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_pptpClientInfoAfterDial_set(int wan_intf_idx, rtk_rg_pptpClientInfoAfterDial_t *clientPptp_info)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_pptpClientInfoAfterDial_set(wan_intf_idx,clientPptp_info);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_l2tpClientInfoBeforeDial_set(int wan_intf_idx, rtk_rg_l2tpClientInfoBeforeDial_t *app_info)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_l2tpClientInfoBeforeDial_set(wan_intf_idx,app_info);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_l2tpClientInfoAfterDial_set(int wan_intf_idx, rtk_rg_l2tpClientInfoAfterDial_t *clientL2tp_info)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_l2tpClientInfoAfterDial_set(wan_intf_idx,clientL2tp_info);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_pppoeDsliteInfoBeforeDial_set(int wan_intf_idx, rtk_rg_pppoeClientInfoBeforeDial_t *app_info)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_pppoeDsliteInfoBeforeDial_set(wan_intf_idx,app_info);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_pppoeDsliteInfoAfterDial_set(int wan_intf_idx, rtk_rg_pppoeDsliteInfoAfterDial_t *pppoeDslite_info)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_pppoeDsliteInfoAfterDial_set(wan_intf_idx,pppoeDslite_info);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_interface_del(int lan_or_wan_intf_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_interface_del(lan_or_wan_intf_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_intfInfo_find(rtk_rg_intfInfo_t *intf_info, int *valid_lan_or_wan_intf_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_intfInfo_find(intf_info,valid_lan_or_wan_intf_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_cvlan_add(rtk_rg_cvlan_info_t *cvlan_info)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_cvlan_add(cvlan_info);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_cvlan_del(int cvlan_id)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_cvlan_del(cvlan_id);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_cvlan_get(rtk_rg_cvlan_info_t *cvlan_info)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_cvlan_get(cvlan_info);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_vlanBinding_add(rtk_rg_vlanBinding_t *vlan_binding_info, int *vlan_binding_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_vlanBinding_add(vlan_binding_info,vlan_binding_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_vlanBinding_del(int vlan_binding_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_vlanBinding_del(vlan_binding_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_vlanBinding_find(rtk_rg_vlanBinding_t *vlan_binding_info, int *valid_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_vlanBinding_find(vlan_binding_info,valid_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_algServerInLanAppsIpAddr_add(rtk_rg_alg_serverIpMapping_t *srvIpMapping)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_algServerInLanAppsIpAddr_add(srvIpMapping);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_algServerInLanAppsIpAddr_del(rtk_rg_alg_type_t delServerMapping)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_algServerInLanAppsIpAddr_del(delServerMapping);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_algApps_set(rtk_rg_alg_type_t alg_app)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_algApps_set(alg_app);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_algApps_get(rtk_rg_alg_type_t *alg_app)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_algApps_get(alg_app);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_dmzHost_set(int wan_intf_idx, rtk_rg_dmzInfo_t *dmz_info)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_dmzHost_set(wan_intf_idx,dmz_info);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_dmzHost_get(int wan_intf_idx, rtk_rg_dmzInfo_t *dmz_info)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_dmzHost_get(wan_intf_idx,dmz_info);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_virtualServer_add(rtk_rg_virtualServer_t *virtual_server, int *virtual_server_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_virtualServer_add(virtual_server,virtual_server_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_virtualServer_del(int virtual_server_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_virtualServer_del(virtual_server_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_virtualServer_find(rtk_rg_virtualServer_t *virtual_server, int *valid_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_virtualServer_find(virtual_server,valid_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_aclFilterAndQos_add(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_aclFilterAndQos_add(acl_filter,acl_filter_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_aclFilterAndQos_del(int acl_filter_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_aclFilterAndQos_del(acl_filter_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_naptFilterAndQos_add(int *index,rtk_rg_naptFilterAndQos_t *napt_filter){
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_naptFilterAndQos_add(index,napt_filter);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}
int32 rtk_rg_naptFilterAndQos_del(int index){
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_naptFilterAndQos_del(index);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}
int32 rtk_rg_naptFilterAndQos_find(int *index,rtk_rg_naptFilterAndQos_t *napt_filter){
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_naptFilterAndQos_find(index,napt_filter);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_aclFilterAndQos_find(rtk_rg_aclFilterAndQos_t *acl_filter, int *valid_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_aclFilterAndQos_find(acl_filter,valid_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_macFilter_add(rtk_rg_macFilterEntry_t *macFilterEntry,int *mac_filter_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_macFilter_add(macFilterEntry,mac_filter_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_macFilter_del(int mac_filter_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_macFilter_del(mac_filter_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_macFilter_find(rtk_rg_macFilterEntry_t *macFilterEntry, int *valid_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_macFilter_find(macFilterEntry,valid_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_urlFilterString_add(rtk_rg_urlFilterString_t *filter,int *url_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_urlFilterString_add(filter,url_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_urlFilterString_del(int url_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_urlFilterString_del(url_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_urlFilterString_find(rtk_rg_urlFilterString_t *filter, int *valid_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_urlFilterString_find(filter,valid_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_upnpConnection_add(rtk_rg_upnpConnection_t *upnp, int *upnp_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_upnpConnection_add(upnp,upnp_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_upnpConnection_del(int upnp_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_upnpConnection_del(upnp_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_upnpConnection_find(rtk_rg_upnpConnection_t *upnp, int *valid_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_upnpConnection_find(upnp,valid_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_naptConnection_add(rtk_rg_naptEntry_t *naptFlow, int *flow_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_naptConnection_add(naptFlow,flow_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_naptConnection_del(int flow_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_naptConnection_del(flow_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_naptConnection_find(rtk_rg_naptInfo_t *naptInfo,int *valid_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_naptConnection_find(naptInfo,valid_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_multicastFlow_add(rtk_rg_multicastFlow_t *mcFlow, int *flow_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_multicastFlow_add(mcFlow,flow_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_multicastFlow_del(int flow_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_multicastFlow_del(flow_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

/* martin ZHU add */
int32 rtk_rg_l2MultiCastFlow_add(rtk_rg_l2MulticastFlow_t *l2McFlow,int *flow_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_l2MultiCastFlow_add(l2McFlow, flow_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_multicastFlow_find(rtk_rg_multicastFlow_t *mcFlow, int *valid_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_multicastFlow_find(mcFlow,valid_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_macEntry_add(rtk_rg_macEntry_t *macEntry, int *entry_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_macEntry_add(macEntry,entry_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_macEntry_del(int entry_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_macEntry_del(entry_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_macEntry_find(rtk_rg_macEntry_t *macEntry,int *valid_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_macEntry_find(macEntry,valid_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_arpEntry_add(rtk_rg_arpEntry_t *arpEntry, int *arp_entry_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_arpEntry_add(arpEntry,arp_entry_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_arpEntry_del(int arp_entry_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_arpEntry_del(arp_entry_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_arpEntry_find(rtk_rg_arpInfo_t *arpInfo,int *arp_valid_idx)
{
	int r;
	//this API may call by Timer(it can't add mutex to lock.)
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_arpEntry_find(arpInfo,arp_valid_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_neighborEntry_add(rtk_rg_neighborEntry_t *neighborEntry,int *neighbor_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_neighborEntry_add(neighborEntry,neighbor_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_neighborEntry_del(int neighbor_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_neighborEntry_del(neighbor_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_neighborEntry_find(rtk_rg_neighborInfo_t *neighborInfo,int *neighbor_valid_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_neighborEntry_find(neighborInfo,neighbor_valid_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_accessWanLimit_set(rtk_rg_accessWanLimitData_t access_wan_info)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_accessWanLimit_set(access_wan_info);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_accessWanLimit_get(rtk_rg_accessWanLimitData_t *access_wan_info)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_accessWanLimit_get(access_wan_info);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_accessWanLimitCategory_set(rtk_rg_accessWanLimitCategory_t macCategory_info)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=(int32)pf.rtk_rg_accessWanLimitCategory_set(macCategory_info);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_accessWanLimitCategory_get(rtk_rg_accessWanLimitCategory_t *macCategory_info)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=(int32)pf.rtk_rg_accessWanLimitCategory_get(macCategory_info);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_softwareSourceAddrLearningLimit_set(rtk_rg_saLearningLimitInfo_t sa_learnLimit_info, rtk_rg_port_idx_t port_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_softwareSourceAddrLearningLimit_set(sa_learnLimit_info,port_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_softwareSourceAddrLearningLimit_get(rtk_rg_saLearningLimitInfo_t *sa_learnLimit_info, rtk_rg_port_idx_t port_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_softwareSourceAddrLearningLimit_get(sa_learnLimit_info,port_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_wlanSoftwareSourceAddrLearningLimit_set(rtk_rg_saLearningLimitInfo_t sa_learnLimit_info, int wlan_idx, int dev_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_wlanSoftwareSourceAddrLearningLimit_set(sa_learnLimit_info,wlan_idx,dev_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_wlanSoftwareSourceAddrLearningLimit_get(rtk_rg_saLearningLimitInfo_t *sa_learnLimit_info, int wlan_idx, int dev_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_wlanSoftwareSourceAddrLearningLimit_get(sa_learnLimit_info,wlan_idx,dev_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_dosPortMaskEnable_set(rtk_rg_mac_portmask_t dos_port_mask)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_dosPortMaskEnable_set(dos_port_mask);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_dosPortMaskEnable_get(rtk_rg_mac_portmask_t *dos_port_mask)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_dosPortMaskEnable_get(dos_port_mask);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_dosType_set(rtk_rg_dos_type_t dos_type,int dos_enabled,rtk_rg_dos_action_t dos_action)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_dosType_set(dos_type,dos_enabled,dos_action);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_dosType_get(rtk_rg_dos_type_t dos_type,int *dos_enabled,rtk_rg_dos_action_t *dos_action)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_dosType_get(dos_type,dos_enabled,dos_action);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_dosFloodType_set(rtk_rg_dos_type_t dos_type,int dos_enabled,rtk_rg_dos_action_t dos_action,int dos_threshold)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_dosFloodType_set(dos_type,dos_enabled,dos_action,dos_threshold);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_dosFloodType_get(rtk_rg_dos_type_t dos_type,int *dos_enabled,rtk_rg_dos_action_t *dos_action,int *dos_threshold)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_dosFloodType_get(dos_type,dos_enabled,dos_action,dos_threshold);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_portMirror_set(rtk_rg_portMirrorInfo_t portMirrorInfo)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_portMirror_set(portMirrorInfo);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_portMirror_get(rtk_rg_portMirrorInfo_t *portMirrorInfo)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_portMirror_get(portMirrorInfo);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_portMirror_clear(void)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_portMirror_clear();
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_portEgrBandwidthCtrlRate_set(rtk_rg_mac_port_idx_t port, uint32 rate)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_portEgrBandwidthCtrlRate_set(port,rate);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_portIgrBandwidthCtrlRate_set(rtk_rg_mac_port_idx_t port, uint32 rate)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_portIgrBandwidthCtrlRate_set(port,rate);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_portEgrBandwidthCtrlRate_get(rtk_rg_mac_port_idx_t port, uint32 *rate)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_portEgrBandwidthCtrlRate_get(port,rate);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_portIgrBandwidthCtrlRate_get(rtk_rg_mac_port_idx_t port, uint32 *rate)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_portIgrBandwidthCtrlRate_get(port,rate);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_phyPortForceAbility_set(rtk_rg_mac_port_idx_t port, rtk_rg_phyPortAbilityInfo_t ability)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_phyPortForceAbility_set(port,ability);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_phyPortForceAbility_get(rtk_rg_mac_port_idx_t port, rtk_rg_phyPortAbilityInfo_t *ability)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_phyPortForceAbility_get(port,ability);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_cpuPortForceTrafficCtrl_set(rtk_rg_enable_t tx_fc_state,	rtk_rg_enable_t rx_fc_state)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_cpuPortForceTrafficCtrl_set(tx_fc_state,rx_fc_state);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_cpuPortForceTrafficCtrl_get(rtk_rg_enable_t *pTx_fc_state,	rtk_rg_enable_t *pRx_fc_state)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_cpuPortForceTrafficCtrl_get(pTx_fc_state,pRx_fc_state);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_portMibInfo_get(rtk_rg_mac_port_idx_t port, rtk_rg_port_mib_info_t *mibInfo)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_portMibInfo_get(port,mibInfo);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_portMibInfo_clear(rtk_rg_mac_port_idx_t port)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_portMibInfo_clear(port);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_portIsolation_set(rtk_rg_port_isolation_t isolationSetting)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_portIsolation_set(isolationSetting);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_portIsolation_get(rtk_rg_port_isolation_t *isolationSetting)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_portIsolation_get(isolationSetting);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_stormControl_add(rtk_rg_stormControlInfo_t *stormInfo,int *stormInfo_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_stormControl_add(stormInfo,stormInfo_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_stormControl_del(int stormInfo_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_stormControl_del(stormInfo_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_stormControl_find(rtk_rg_stormControlInfo_t *stormInfo,int *stormInfo_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_stormControl_find(stormInfo,stormInfo_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_shareMeter_set(uint32 index, uint32 rate, rtk_rg_enable_t ifgInclude)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_shareMeter_set(index,rate,ifgInclude);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_shareMeter_get(uint32 index, uint32 *pRate , rtk_rg_enable_t *pIfgInclude)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_shareMeter_get(index,pRate,pIfgInclude);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32
rtk_rg_qosStrictPriorityOrWeightFairQueue_set(rtk_rg_mac_port_idx_t port_idx,rtk_rg_qos_queue_weights_t q_weight)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_qosStrictPriorityOrWeightFairQueue_set(port_idx,q_weight);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_qosStrictPriorityOrWeightFairQueue_get(rtk_rg_mac_port_idx_t port_idx,rtk_rg_qos_queue_weights_t *pQ_weight)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_qosStrictPriorityOrWeightFairQueue_get(port_idx,pQ_weight);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_qosInternalPriMapToQueueId_set(int int_pri, int queue_id)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_qosInternalPriMapToQueueId_set(int_pri,queue_id);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_qosInternalPriMapToQueueId_get(int int_pri, int *pQueue_id)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_qosInternalPriMapToQueueId_get(int_pri,pQueue_id);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_qosInternalPriDecisionByWeight_set(rtk_rg_qos_priSelWeight_t weightOfPriSel)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_qosInternalPriDecisionByWeight_set(weightOfPriSel);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_qosInternalPriDecisionByWeight_get(rtk_rg_qos_priSelWeight_t *pWeightOfPriSel)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_qosInternalPriDecisionByWeight_get(pWeightOfPriSel);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_qosDscpRemapToInternalPri_set(uint32 dscp,uint32 int_pri)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_qosDscpRemapToInternalPri_set(dscp,int_pri);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_qosDscpRemapToInternalPri_get(uint32 dscp,uint32 *pInt_pri)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_qosDscpRemapToInternalPri_get(dscp,pInt_pri);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_qosPortBasedPriority_set(rtk_rg_mac_port_idx_t port_idx,uint32 int_pri)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_qosPortBasedPriority_set(port_idx,int_pri);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_qosPortBasedPriority_get(rtk_rg_mac_port_idx_t port_idx,uint32 *pInt_pri)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_qosPortBasedPriority_get(port_idx,pInt_pri);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_qosDot1pPriRemapToInternalPri_set(uint32 dot1p,uint32 int_pri)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_qosDot1pPriRemapToInternalPri_set(dot1p,int_pri);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_qosDot1pPriRemapToInternalPri_get(uint32 dot1p,uint32 *pInt_pri)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_qosDot1pPriRemapToInternalPri_get(dot1p,pInt_pri);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_set(rtk_rg_mac_port_idx_t rmk_port,rtk_rg_enable_t rmk_enable, rtk_rg_qos_dscpRmkSrc_t rmk_src_select)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_set(rmk_port,rmk_enable,rmk_src_select);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_get(rtk_rg_mac_port_idx_t rmk_port,rtk_rg_enable_t *pRmk_enable, rtk_rg_qos_dscpRmkSrc_t *pRmk_src_select)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_get(rmk_port,pRmk_enable,pRmk_src_select);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_qosDscpRemarkByInternalPri_set(int int_pri,int rmk_dscp)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_qosDscpRemarkByInternalPri_set(int_pri,rmk_dscp);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_qosDscpRemarkByInternalPri_get(int int_pri,int *pRmk_dscp)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_qosDscpRemarkByInternalPri_get(int_pri,pRmk_dscp);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_qosDscpRemarkByDscp_set(int dscp,int rmk_dscp)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_qosDscpRemarkByDscp_set(dscp,rmk_dscp);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_qosDscpRemarkByDscp_get(int dscp,int *pRmk_dscp)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_qosDscpRemarkByDscp_get(dscp,pRmk_dscp);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_set(rtk_rg_mac_port_idx_t rmk_port, rtk_rg_enable_t rmk_enable)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_set(rmk_port,rmk_enable);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_get(rtk_rg_mac_port_idx_t rmk_port, rtk_rg_enable_t *pRmk_enable)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_get(rmk_port,pRmk_enable);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_qosDot1pPriRemarkByInternalPri_set(int int_pri,int rmk_dot1p)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_qosDot1pPriRemarkByInternalPri_set(int_pri,rmk_dot1p);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_qosDot1pPriRemarkByInternalPri_get(int int_pri,int *pRmk_dot1p)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_qosDot1pPriRemarkByInternalPri_get(int_pri,pRmk_dot1p);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_portBasedCVlanId_set(rtk_rg_port_idx_t port_idx,int pvid)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_portBasedCVlanId_set(port_idx,pvid);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_portBasedCVlanId_get(rtk_rg_port_idx_t port_idx,int *pPvid)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_portBasedCVlanId_get(port_idx,pPvid);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_wlanDevBasedCVlanId_set(int wlan_idx,int dev_idx,int dvid)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_wlanDevBasedCVlanId_set(wlan_idx,dev_idx,dvid);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_wlanDevBasedCVlanId_get(int wlan_idx,int dev_idx,int *pDvid)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_wlanDevBasedCVlanId_get(wlan_idx,dev_idx,pDvid);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_portStatus_get(rtk_rg_mac_port_idx_t port, rtk_rg_portStatusInfo_t *portInfo)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_portStatus_get(port,portInfo);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

#ifdef CONFIG_RG_NAPT_PORT_COLLISION_PREVENTION
int32 rtk_rg_naptExtPortGet(int isTcp,uint16 *pPort)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_naptExtPortGet(isTcp,pPort);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_naptExtPortFree(int isTcp,uint16 port)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_naptExtPortFree(isTcp,port);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}
#endif

int32 rtk_rg_classifyEntry_add(rtk_rg_classifyEntry_t *classifyFilter)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_classifyEntry_add(classifyFilter);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_classifyEntry_find(int index, rtk_rg_classifyEntry_t *classifyFilter)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_classifyEntry_find(index,classifyFilter);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_classifyEntry_del(int index)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_classifyEntry_del(index);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_svlanTpid_set(uint32 svlan_tag_id)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_svlanTpid_set(svlan_tag_id);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_svlanTpid_get(uint32 *pSvlanTagId)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_svlanTpid_get(pSvlanTagId);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_svlanServicePort_set(rtk_port_t port, rtk_enable_t enable)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_svlanServicePort_set(port, enable);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_svlanServicePort_get(rtk_port_t port, rtk_enable_t *pEnable)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_svlanServicePort_get(port, pEnable);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_svlanTpid2_enable_set(rtk_rg_enable_t enable)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_svlanTpid2_enable_set(enable);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}
int32 rtk_rg_svlanTpid2_enable_get(rtk_rg_enable_t *pEnable)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_svlanTpid2_enable_get(pEnable);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}
int32 rtk_rg_svlanTpid2_set(uint32 svlan_tag_id)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_svlanTpid2_set(svlan_tag_id);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}
int32 rtk_rg_svlanTpid2_get(uint32 *pSvlanTagId)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_svlanTpid2_get(pSvlanTagId);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}





int32 rtk_rg_pppoeInterfaceIdleTime_get(int intfIdx,uint32 *idleSec)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_pppoeInterfaceIdleTime_get(intfIdx,idleSec);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_gatewayServicePortRegister_add(rtk_rg_gatewayServicePortEntry_t *serviceEntry, int *index){
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_gatewayServicePortRegister_add(serviceEntry,index);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;

}

int32 rtk_rg_gatewayServicePortRegister_del(int index){
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_gatewayServicePortRegister_del(index);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;

}

int32 rtk_rg_gatewayServicePortRegister_find(rtk_rg_gatewayServicePortEntry_t *serviceEntry, int *index){
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_gatewayServicePortRegister_find(serviceEntry,index);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_stpBlockingPortmask_set(rtk_rg_portmask_t Mask){
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_stpBlockingPortmask_set(Mask);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}
int32 rtk_rg_stpBlockingPortmask_get(rtk_rg_portmask_t *pMask){
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_stpBlockingPortmask_get(pMask);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_gponDsBcFilterAndRemarking_Enable(rtk_rg_enable_t enable)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_gponDsBcFilterAndRemarking_Enable(enable);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}


int32 rtk_rg_gponDsBcFilterAndRemarking_add(rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t *filterRule,int *index)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_gponDsBcFilterAndRemarking_add(filterRule,index);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_gponDsBcFilterAndRemarking_del(int index)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_gponDsBcFilterAndRemarking_del(index);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_gponDsBcFilterAndRemarking_del_all(void)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_gponDsBcFilterAndRemarking_del_all();
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

int32 rtk_rg_gponDsBcFilterAndRemarking_find(int *index,rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t *filterRule){
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_gponDsBcFilterAndRemarking_find(index,filterRule);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

rtk_rg_err_code_t rtk_rg_dsliteMcTable_set(rtk_l34_dsliteMc_entry_t *pDsliteMcEntry){
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_dsliteMcTable_set(pDsliteMcEntry);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

rtk_rg_err_code_t rtk_rg_dsliteMcTable_get(rtk_l34_dsliteMc_entry_t *pDsliteMcEntry){
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_dsliteMcTable_get(pDsliteMcEntry);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

rtk_rg_err_code_t rtk_rg_dsliteControl_set(rtk_l34_dsliteCtrlType_t ctrlType, uint32 act){
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_dsliteControl_set(ctrlType,act);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

rtk_rg_err_code_t rtk_rg_dsliteControl_get(rtk_l34_dsliteCtrlType_t ctrlType, uint32 *pAct){
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_dsliteControl_get(ctrlType,pAct);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

rtk_rg_err_code_t rtk_rg_interfaceMibCounter_del(int intf_idx)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_interfaceMibCounter_del(intf_idx);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

rtk_rg_err_code_t rtk_rg_interfaceMibCounter_get(rtk_l34_mib_t *pMibCnt)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_interfaceMibCounter_get(pMibCnt);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

rtk_rg_err_code_t rtk_rg_redirectHttpAll_set(rtk_rg_redirectHttpAll_t *pRedirectHttpAll)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_redirectHttpAll_set(pRedirectHttpAll);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

rtk_rg_err_code_t rtk_rg_redirectHttpAll_get(rtk_rg_redirectHttpAll_t *pRedirectHttpAll)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_redirectHttpAll_get(pRedirectHttpAll);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

rtk_rg_err_code_t rtk_rg_redirectHttpURL_add(rtk_rg_redirectHttpURL_t *pRedirectHttpURL)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_redirectHttpURL_add(pRedirectHttpURL);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

rtk_rg_err_code_t rtk_rg_redirectHttpURL_del(rtk_rg_redirectHttpURL_t *pRedirectHttpURL)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_redirectHttpURL_del(pRedirectHttpURL);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

rtk_rg_err_code_t rtk_rg_redirectHttpWhiteList_add(rtk_rg_redirectHttpWhiteList_t *pRedirectHttpWhiteList)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_redirectHttpWhiteList_add(pRedirectHttpWhiteList);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}

rtk_rg_err_code_t rtk_rg_redirectHttpWhiteList_del(rtk_rg_redirectHttpWhiteList_t *pRedirectHttpWhiteList)
{
	int r;
	rg_api_lock_irqsave(&rgApiLock,spinflags);
	r=pf.rtk_rg_redirectHttpWhiteList_del(pRedirectHttpWhiteList);
	rg_api_unlock_irqrestore(&rgApiLock,spinflags);
	return r;
}


EXPORT_SYMBOL(rtk_rg_stpBlockingPortmask_set);
EXPORT_SYMBOL(rtk_rg_stpBlockingPortmask_get);

#ifdef __KERNEL__

int __init rtk_rg_api_module_init(void)
{
	//Don't lock sem_rgApiCall here! the lock has not inited!
	return pf.rtk_rg_api_module_init();
}

void __exit rtk_rg_api_module_exit(void)
{
}


module_init(rtk_rg_api_module_init);
module_exit(rtk_rg_api_module_exit);


EXPORT_SYMBOL(rtk_rg_wanInterface_add);
EXPORT_SYMBOL(rtk_rg_urlFilterString_find);
EXPORT_SYMBOL(rtk_rg_vlanBinding_add);
EXPORT_SYMBOL(rtk_rg_urlFilterString_add);
EXPORT_SYMBOL(rtk_rg_intfInfo_find);
EXPORT_SYMBOL(rtk_rg_initParam_get);
EXPORT_SYMBOL(rtk_rg_macEntry_del);
EXPORT_SYMBOL(rtk_rg_interface_del);
EXPORT_SYMBOL(rtk_rg_macEntry_find);
EXPORT_SYMBOL(rtk_rg_vlanBinding_del);
EXPORT_SYMBOL(rtk_rg_naptConnection_del);
EXPORT_SYMBOL(rtk_rg_aclFilterAndQos_del);
EXPORT_SYMBOL(rtk_rg_staticInfo_set);
EXPORT_SYMBOL(rtk_rg_macFilter_add);
EXPORT_SYMBOL(rtk_rg_initParam_set);
EXPORT_SYMBOL(rtk_rg_aclFilterAndQos_find);
EXPORT_SYMBOL(rtk_rg_macFilter_del);
EXPORT_SYMBOL(rtk_rg_virtualServer_add);
EXPORT_SYMBOL(rtk_rg_macEntry_add);
EXPORT_SYMBOL(rtk_rg_upnpConnection_add);
EXPORT_SYMBOL(rtk_rg_pppoeClientInfoAfterDial_set);
EXPORT_SYMBOL(rtk_rg_macFilter_find);
EXPORT_SYMBOL(rtk_rg_upnpConnection_del);
EXPORT_SYMBOL(rtk_rg_arpEntry_del);
EXPORT_SYMBOL(rtk_rg_multicastFlow_find);
EXPORT_SYMBOL(rtk_rg_virtualServer_find);
EXPORT_SYMBOL(rtk_rg_urlFilterString_del);
EXPORT_SYMBOL(rtk_rg_vlanBinding_find);
EXPORT_SYMBOL(rtk_rg_pppoeClientInfoBeforeDial_set);
EXPORT_SYMBOL(rtk_rg_dhcpClientInfo_set);
EXPORT_SYMBOL(rtk_rg_driverVersion_get);
EXPORT_SYMBOL(rtk_rg_multicastFlow_del);
EXPORT_SYMBOL(rtk_rg_virtualServer_del);
EXPORT_SYMBOL(rtk_rg_lanInterface_add);
EXPORT_SYMBOL(rtk_rg_aclFilterAndQos_add);
EXPORT_SYMBOL(rtk_rg_upnpConnection_find);
EXPORT_SYMBOL(rtk_rg_arpEntry_add);
EXPORT_SYMBOL(rtk_rg_naptConnection_find);
EXPORT_SYMBOL(rtk_rg_naptConnection_add);
EXPORT_SYMBOL(rtk_rg_arpEntry_find );
EXPORT_SYMBOL(rtk_rg_multicastFlow_add);
/* martin ZHU add */
EXPORT_SYMBOL(rtk_rg_l2MultiCastFlow_add);

EXPORT_SYMBOL(rtk_rg_qosInternalPriDecisionByWeight_set);
EXPORT_SYMBOL(rtk_rg_qosInternalPriDecisionByWeight_get);
EXPORT_SYMBOL(rtk_rg_qosDscpRemapToInternalPri_set);
EXPORT_SYMBOL(rtk_rg_qosDscpRemapToInternalPri_get);
EXPORT_SYMBOL(rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_set);
EXPORT_SYMBOL(rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_get);
EXPORT_SYMBOL(rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_set);
EXPORT_SYMBOL(rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_get);
EXPORT_SYMBOL(rtk_rg_classifyEntry_add);
EXPORT_SYMBOL(rtk_rg_classifyEntry_find);
EXPORT_SYMBOL(rtk_rg_classifyEntry_del);
EXPORT_SYMBOL(rtk_rg_phyPortForceAbility_set);
EXPORT_SYMBOL(rtk_rg_phyPortForceAbility_get);
EXPORT_SYMBOL(rtk_rg_portMibInfo_get);
EXPORT_SYMBOL(rtk_rg_portMibInfo_clear);
EXPORT_SYMBOL(rtk_rg_portStatus_get);
EXPORT_SYMBOL(rtk_rg_svlanTpid_set);
EXPORT_SYMBOL(rtk_rg_svlanTpid_get);
EXPORT_SYMBOL(rtk_rg_cvlan_add);
EXPORT_SYMBOL(rtk_rg_cvlan_get);
EXPORT_SYMBOL(rtk_rg_cvlan_del);
EXPORT_SYMBOL(rtk_rg_gponDsBcFilterAndRemarking_add);
EXPORT_SYMBOL(rtk_rg_gponDsBcFilterAndRemarking_del);
EXPORT_SYMBOL(rtk_rg_gponDsBcFilterAndRemarking_del_all);
EXPORT_SYMBOL(rtk_rg_gponDsBcFilterAndRemarking_find);
EXPORT_SYMBOL(rtk_rg_gponDsBcFilterAndRemarking_Enable);
EXPORT_SYMBOL(rtk_rg_dsliteMcTable_set);
EXPORT_SYMBOL(rtk_rg_dsliteMcTable_get);
EXPORT_SYMBOL(rtk_rg_dsliteControl_set);
EXPORT_SYMBOL(rtk_rg_dsliteControl_get);
EXPORT_SYMBOL(rtk_rg_interfaceMibCounter_del);
EXPORT_SYMBOL(rtk_rg_interfaceMibCounter_get);
EXPORT_SYMBOL(rtk_rg_stormControl_add);
EXPORT_SYMBOL(rtk_rg_stormControl_del);
EXPORT_SYMBOL(rtk_rg_stormControl_find);
EXPORT_SYMBOL(rtk_rg_shareMeter_set);
EXPORT_SYMBOL(rtk_rg_shareMeter_get);
EXPORT_SYMBOL(rtk_rg_redirectHttpAll_set);
EXPORT_SYMBOL(rtk_rg_redirectHttpAll_get);
EXPORT_SYMBOL(rtk_rg_redirectHttpURL_add);
EXPORT_SYMBOL(rtk_rg_redirectHttpURL_del);
EXPORT_SYMBOL(rtk_rg_redirectHttpWhiteList_add);
EXPORT_SYMBOL(rtk_rg_redirectHttpWhiteList_del);


#endif

#endif
