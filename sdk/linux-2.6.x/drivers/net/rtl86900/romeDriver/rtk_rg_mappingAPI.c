#if defined(CONFIG_APOLLO)
#include <common/error.h>
#include <rtk/init.h>
#include <rtk/l34_bind_config.h>
#include <rtk/svlan.h>
#include <dal/apollomp/raw/apollomp_raw_hwmisc.h>
#include <rtk/sec.h>
#include <rtk/stat.h>
#include <rtk/ponmac.h>
#include <rtk/l2.h>
#include <rtk/svlan.h>
#endif

#ifdef CONFIG_RTL9602C_SERIES
#include <ioal/mem32.h>
#endif

#if defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#include <rtk_rg_xdsl_extAPI.h>
#endif

#include <rtk_rg_internal.h>
#if defined(CONFIG_DEFAULTS_KERNEL_3_18)
#else
#include <linux/config.h>
#endif



/* for set age of lut entry to 1 */
#if defined(CONFIG_RTL9600_SERIES)		
#else	//support lut traffic bit
int32 _rtk_rg_l2_trafficBit_reset(rtk_l2_ucastAddr_t *pL2Addr)
{
	pL2Addr->age = 1;
	return rtk_l2_addr_add(pL2Addr);
}
#endif

/* don't care collision ,just add and update entry*/
int32 RTK_L2_ADDR_ADD(rtk_l2_ucastAddr_t *pL2Addr)
{
	int ret,l2Idx;

#if defined(CONFIG_RTL9600_SERIES)
	//20130801: for avoiding L2 age out issue. (pure L2 entries are always enabling ARP_USED.)
	pL2Addr->flags|=RTK_L2_UCAST_FLAG_ARP_USED;
#endif
	//set lut traffic bit to 1(age=7), so idle time will not be add in first round.
	pL2Addr->age=7;
	
	ret=rtk_l2_addr_add(pL2Addr);
	if(ret==RT_ERR_OK)
	{
		rg_db.lut[pL2Addr->index].rtk_lut.entryType=RTK_LUT_L2UC;
		memcpy(&rg_db.lut[pL2Addr->index].rtk_lut.entry.l2UcEntry,pL2Addr,sizeof(rtk_l2_ucastAddr_t));
		rg_db.lut[pL2Addr->index].valid=1;

		if(pL2Addr->index<MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE)		//4-way
		{
			//Keep the next of newest
			l2Idx=pL2Addr->index&0xfffffffc;
#if defined(CONFIG_RTL9600_SERIES) 
			rg_db.layer2NextOfNewestCountIdx[l2Idx>>2]=(pL2Addr->index-l2Idx+1)%4;
#endif
		}
#if defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#else	//support lut traffic bit
		if((pL2Addr->index>=MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE) && (pL2Addr->index<MAX_LUT_HW_TABLE_SIZE))
		{
			if(pL2Addr->flags&RTK_L2_UCAST_FLAG_IVL)
				_rtk_rg_lutCamListAdd(_rtk_rg_hash_mac_vid_efid(pL2Addr->mac.octet,pL2Addr->vid,0), pL2Addr->index);
			else
				_rtk_rg_lutCamListAdd(_rtk_rg_hash_mac_fid_efid(pL2Addr->mac.octet,pL2Addr->fid,0), pL2Addr->index);
		}
#endif
	}
	return ret;

}

int32 RTK_L2_ADDR_DEL(rtk_l2_ucastAddr_t *pL2Addr)
{
	int ret;
	ret=rtk_l2_addr_del(pL2Addr);
	if(ret==RT_ERR_OK)
	{
#if defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#else	//support lut traffic bit
		if((pL2Addr->index>=MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE) && (pL2Addr->index<MAX_LUT_HW_TABLE_SIZE))
			_rtk_rg_lutCamListDel(pL2Addr->index);
#endif
		//delete L34 related entries which refer to this l2 index
		_rtk_rg_layer2CleanL34ReferenceTable(pL2Addr->index);

		//2 20140402LUKE:static MAC should not add to learning count
		if((pL2Addr->flags&RTK_L2_UCAST_FLAG_STATIC)==0)
		{
			//------------------ Critical Section start -----------------------//
			//rg_lock(&rg_kernel.saLearningLimitLock);
			if(pL2Addr->port>=RTK_RG_PORT_CPU)
			{
				if(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask&(0x1<<(pL2Addr->ext_port+RTK_RG_PORT_CPU)))
					atomic_dec(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
				atomic_dec(&rg_db.systemGlobal.sourceAddrLearningCount[pL2Addr->ext_port+RTK_RG_PORT_CPU]);

				//decrease wlan's device count				
				if(pL2Addr->ext_port==(RTK_RG_EXT_PORT0-RTK_RG_PORT_CPU))
				{
#ifdef CONFIG_MASTER_WLAN0_ENABLE
					if(rg_db.systemGlobal.accessWanLimitPortMask_wlan0member&(0x1<<(rg_db.lut[pL2Addr->index].wlan_device_idx)))
						atomic_dec(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
					atomic_dec(&rg_db.systemGlobal.wlan0SourceAddrLearningCount[(int)rg_db.lut[pL2Addr->index].wlan_device_idx]);
#endif
				}

				if(_rtK_rg_checkCategoryPortmask_spa(pL2Addr->port+pL2Addr->ext_port)==SUCCESS)
					atomic_dec(&rg_db.systemGlobal.accessWanLimitCategoryCount[(unsigned int)rg_db.lut[pL2Addr->index].category]);
			}
			else
			{
				if(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask&(0x1<<(pL2Addr->port)))
					atomic_dec(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
				atomic_dec(&rg_db.systemGlobal.sourceAddrLearningCount[pL2Addr->port]);
				
				if(_rtK_rg_checkCategoryPortmask_spa(pL2Addr->port)==SUCCESS)
					atomic_dec(&rg_db.systemGlobal.accessWanLimitCategoryCount[(unsigned int)rg_db.lut[pL2Addr->index].category]);
			}		
			//------------------ Critical Section End -----------------------//
			//rg_unlock(&rg_kernel.saLearningLimitLock);
		}
		
		memset(&rg_db.lut[pL2Addr->index],0,sizeof(rtk_rg_table_lut_t));
	}
	return ret;
}
	
int32 RTK_L2_IPMCASTADDR_ADD(rtk_l2_ipMcastAddr_t *pIpmcastAddr)
{
	int ret;	
	ret=rtk_l2_ipMcastAddr_add(pIpmcastAddr);
	if(ret==RT_ERR_OK)
	{
#if defined(CONFIG_RTL9602C_SERIES)
		if(pIpmcastAddr->flags & RTK_L2_IPMCAST_FLAG_IPV6)
			rg_db.lut[pIpmcastAddr->index].rtk_lut.entryType=RTK_LUT_L3V6MC;
		else
#endif
		{
			rg_db.lut[pIpmcastAddr->index].rtk_lut.entryType=RTK_LUT_L3MC;
		}

		memcpy(&rg_db.lut[pIpmcastAddr->index].rtk_lut.entry.ipmcEntry,pIpmcastAddr,sizeof(rtk_l2_ipMcastAddr_t));
		rg_db.lut[pIpmcastAddr->index].valid=1;

#if defined(CONFIG_APOLLO)
#if defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#else	//support lut traffic bit
		if((pIpmcastAddr->index>=MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE) && (pIpmcastAddr->index<MAX_LUT_HW_TABLE_SIZE))
		{
			if(pIpmcastAddr->flags & RTK_L2_IPMCAST_FLAG_IPV6)
			{	
				_rtk_rg_lutCamListAdd(_hash_ipm_dipv6(&(pIpmcastAddr->dip6.ipv6_addr[0])), pIpmcastAddr->index);
			}
			else
			{
				if(pIpmcastAddr->flags &RTK_L2_IPMCAST_FLAG_IVL)
					_rtk_rg_lutCamListAdd(_hash_dip_vidfid_sipidx_sipfilter(1,pIpmcastAddr->dip,pIpmcastAddr->vid,pIpmcastAddr->sip_index,(pIpmcastAddr->flags&RTK_L2_IPMCAST_FLAG_SIP_FILTER)?1:0), pIpmcastAddr->index);
				else
					_rtk_rg_lutCamListAdd(_hash_dip_vidfid_sipidx_sipfilter(0,pIpmcastAddr->dip,pIpmcastAddr->fid,pIpmcastAddr->sip_index,(pIpmcastAddr->flags&RTK_L2_IPMCAST_FLAG_SIP_FILTER)?1:0), pIpmcastAddr->index);					
			}
		}
#endif
#endif
	}
	return ret;
}

int32 RTK_L2_IPMCASTADDR_DEL(rtk_l2_ipMcastAddr_t *pIpmcastAddr)
{
	int ret;
	ret=rtk_l2_ipMcastAddr_del(pIpmcastAddr);
	if(ret==RT_ERR_OK)
	{	
#if defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#else	//support lut traffic bit
		if((pIpmcastAddr->index>=MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE) && (pIpmcastAddr->index<MAX_LUT_HW_TABLE_SIZE))
			_rtk_rg_lutCamListDel(pIpmcastAddr->index);
#endif
		memset(&rg_db.lut[pIpmcastAddr->index],0,sizeof(rtk_rg_table_lut_t));
	}
	return ret;
}

int32 RTK_L2_MCASTADDR_ADD(rtk_l2_mcastAddr_t *pMcastAddr)
{
	int ret;
	ret=rtk_l2_mcastAddr_add(pMcastAddr);
	if(ret==RT_ERR_OK)
	{
		rg_db.lut[pMcastAddr->index].rtk_lut.entryType=RTK_LUT_L2MC;
		memcpy(&rg_db.lut[pMcastAddr->index].rtk_lut.entry.l2McEntry,pMcastAddr,sizeof(rtk_l2_mcastAddr_t));
		rg_db.lut[pMcastAddr->index].valid=1;
		
#if defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#else	//support lut traffic bit
		if((pMcastAddr->index>=MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE) && (pMcastAddr->index<MAX_LUT_HW_TABLE_SIZE))
		{
			if(pMcastAddr->flags&RTK_L2_MCAST_FLAG_IVL)
				_rtk_rg_lutCamListAdd(_rtk_rg_hash_mac_vid_efid(pMcastAddr->mac.octet,pMcastAddr->vid,0), pMcastAddr->index);
			else
				_rtk_rg_lutCamListAdd(_rtk_rg_hash_mac_fid_efid(pMcastAddr->mac.octet,pMcastAddr->fid,0), pMcastAddr->index);
		}
#endif
	}
	return ret;

}

int32 RTK_L2_MCASTADDR_DEL(rtk_l2_mcastAddr_t *pMcastAddr)
{
	int ret;
	ret=rtk_l2_mcastAddr_del(pMcastAddr);
	if(ret==RT_ERR_OK)
	{	
#if defined(CONFIG_RTL9600_SERIES)	|| defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)	
#else	//support lut traffic bit
		if((pMcastAddr->index>=MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE) && (pMcastAddr->index<MAX_LUT_HW_TABLE_SIZE))
			_rtk_rg_lutCamListDel(pMcastAddr->index);
#endif
		memset(&rg_db.lut[pMcastAddr->index],0,sizeof(rtk_rg_table_lut_t));
	}
	return ret;
}

#if defined(CONFIG_RTL9602C_SERIES)
int32 RTK_L2_IPMCSIPFILTER_ADD(ipaddr_t filterIp,int32 *idx)
{
	int i;
	int ret;
	int index=-1;
	
	//Is entry exist?
	for(i=0;i<MAX_IPMCFILTER_HW_TABLE_SIZE;i++)
	{
		//find first invaild entry
		if(rg_db.ipmcfilter[i].valid==FALSE && index==-1)
			index=i;
		if((rg_db.ipmcfilter[i].filterIp==filterIp)&&(rg_db.ipmcfilter[i].valid==TRUE))
		{
			return (RT_ERR_OK);
		}		
	}

	//check we have vaild inedx
	if(index==-1){ return (RT_ERR_FAILED);}

	ret=rtk_l2_ipmcSipFilter_set(index,filterIp);
	if(ret == RT_ERR_OK)
	{
		rg_db.ipmcfilter[index].valid=1;
		rg_db.ipmcfilter[index].filterIp=filterIp;
		*idx=index;
		return RT_ERR_OK;
	}
	return (RT_ERR_FAILED);

}

/* output to delIdx */
int32 RTK_L2_IPMCSIPFILTER_DEL(ipaddr_t filterIp,int32 *delIdx)
{
	int i;
	int ret;


	//find entry
	for(i=0;i<MAX_IPMCFILTER_HW_TABLE_SIZE;i++)
	{
		if((rg_db.ipmcfilter[i].filterIp==filterIp)&&(rg_db.ipmcfilter[i].valid==TRUE))
		{
			ret=rtk_l2_ipmcSipFilter_set(i,0);
			if(ret == RT_ERR_OK)
			{
				rg_db.ipmcfilter[i].valid=0;
				*delIdx=i;
				return RT_ERR_OK;
			}
		}		
	}

	return (RT_ERR_FAILED);	
}


#elif defined(CONFIG_RTL9600_SERIES)
int32 RTK_L2_IPMCGROUP_ADD(ipaddr_t gip, rtk_portmask_t *pPortmask)
{
	int i;

	//Is entry exist?
	for(i=0;i<MAX_IPMCGRP_HW_TABLE_SIZE;i++)
	{
		if((rg_db.ipmcgrp[i].groupIp==gip)&&(rg_db.ipmcgrp[i].valid==TRUE))
		{
			WARNING("Error Should Delete Gip:%d.%d.%d.%d First ",(gip>>24)&0xff,(gip>>16)&0xff,(gip>>8)&0xff,(gip)&0xff);
			return (RT_ERR_FAILED);
		}		
	}

	//to find an empty entry.
	for(i=0;i<MAX_IPMCGRP_HW_TABLE_SIZE;i++)
	{
		if(rg_db.ipmcgrp[i].valid==FALSE)
		{
			int r;			
			r=rtk_l2_ipmcGroup_add(gip,pPortmask);			
			if(r==RT_ERR_OK)
			{
				TABLE("ADD multicast Gip:%d.%d.%d.%d to Group Table %s portMask=%x",
					(gip>>24)&0xff,(gip>>16)&0xff,(gip>>8)&0xff,(gip)&0xff,pPortmask->bits[0]?"excludeMode":"includeMode",pPortmask->bits[0]);
				rg_db.ipmcgrp[i].groupIp=gip;
				rg_db.ipmcgrp[i].portMsk.bits[0]=pPortmask->bits[0];
				rg_db.ipmcgrp[i].valid=TRUE;
			}
			return r;
		}		
	}

	return (RT_ERR_FAILED);

}

int32 RTK_L2_IPMCGROUP_DEL(ipaddr_t gip)
{
	int i;
	for(i=0;i<MAX_IPMCGRP_HW_TABLE_SIZE;i++)
	{
		if((rg_db.ipmcgrp[i].groupIp==gip)&&(rg_db.ipmcgrp[i].valid==TRUE))
		{
			int r;
			r=rtk_l2_ipmcGroup_del(gip);
			if(r==RT_ERR_OK) rg_db.ipmcgrp[i].valid=FALSE;
			TABLE("Del multicast Gip:%d.%d.%d.%d from Group Table %s portMask=%x",
				(gip>>24)&0xff,(gip>>16)&0xff,(gip>>8)&0xff,(gip)&0xff,rg_db.ipmcgrp[i].portMsk.bits[0]?"excludeMode":"includeMode",rg_db.ipmcgrp[i].portMsk);
			return r;
		}		
	}
	return (RT_ERR_FAILED);
}
#endif

int32 RTK_L2_PORTLIMITLEARNINGCNT_SET(rtk_port_t port, uint32 macCnt)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_l2_portLimitLearningCnt_set(port, macCnt);
}

int32 RTK_L2_PORTLIMITLEARNINGCNTACTION_SET(rtk_port_t port, rtk_l2_limitLearnCntAction_t learningAction)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_l2_portLimitLearningCntAction_set(port, learningAction);
}

int32 RTK_L2_PORTLOOKUPMISSACTION_SET(rtk_port_t port, rtk_l2_lookupMissType_t type, rtk_action_t action)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_l2_portLookupMissAction_set(port, type, action);
}

int32 RTK_L2_SRCPORTEGRFILTERMASK_SET(rtk_portmask_t * pFilter_portmask)
{
	rtk_portmask_t portmask;
	portmask.bits[0]=(pFilter_portmask->bits[0])&rg_db.systemGlobal.phyPortStatus;
	return rtk_l2_srcPortEgrFilterMask_set(&portmask);
}

int32 RTK_L34_NETIFTABLE_SET(uint32 idx, rtk_l34_netif_entry_t *entry)
{
	int ret;
	
	ret=rtk_l34_netifTable_set(idx,entry);
	if(ret==RT_ERR_OK)
	{
		memcpy(&rg_db.netif[idx].rtk_netif,entry,sizeof(*entry));
	}
	return ret;
}


int32 RTK_L34_ROUTINGTABLE_SET(uint32 idx, rtk_l34_routing_entry_t *entry)
{
	int ret;
	
	ret=rtk_l34_routingTable_set(idx,entry);
	if(ret==RT_ERR_OK)
	{
		memcpy(&rg_db.l3[idx].rtk_l3,entry,sizeof(*entry));
		if(entry->ipMask==0)
			rg_db.l3[idx].netmask=0;
		else
			rg_db.l3[idx].netmask=~((1<<(31-entry->ipMask))-1);
	}
	return ret;
}

int32 RTK_L34_EXTINTIPTABLE_SET(uint32 idx, rtk_l34_ext_intip_entry_t *entry)
{
	int ret;
	ret=rtk_l34_extIntIPTable_set(idx,entry);
	if(ret==RT_ERR_OK)
	{
		memcpy(&rg_db.extip[idx].rtk_extip,entry,sizeof(*entry));
	}
	return ret;
}

int32 RTK_L34_NEXTHOPTABLE_SET(uint32 idx, rtk_l34_nexthop_entry_t *entry)
{
	int ret;
	ret=rtk_l34_nexthopTable_set(idx,entry);
	if(ret==RT_ERR_OK)
	{
		memcpy(&rg_db.nexthop[idx].rtk_nexthop,entry,sizeof(*entry));
	}
	return ret;
}

int32 RTK_L34_PPPOETABLE_SET(uint32 idx, rtk_l34_pppoe_entry_t *entry)
{
	int ret;
	ret=rtk_l34_pppoeTable_set(idx,entry);
	if(ret==RT_ERR_OK)
	{
		memcpy(&rg_db.pppoe[idx].rtk_pppoe,entry,sizeof(*entry));
	}
	return ret;
}

int32 RTK_L34_ARPTABLE_SET(uint32 idx, rtk_l34_arp_entry_t *entry)
{
	int ret;
	//printk("rtk_l34_arpTable_set(%d,idx=%d,ipAddr=0x%x,nhIdx=0x%x,valid=%d)\n",idx,entry->index,entry->ipAddr,entry->nhIdx,entry->valid);
	ret=rtk_l34_arpTable_set(idx,entry);
	if(ret==RT_ERR_OK)
	{
		memcpy(&rg_db.arp[idx].rtk_arp,entry,sizeof(*entry));
	}
	return ret;
}

int32 RTK_L34_NAPTINBOUNDTABLE_SET(int8 forced,uint32 idx, rtk_l34_naptInbound_entry_t *entry)
{
	int ret=RT_ERR_OK;
	if(idx<MAX_NAPT_IN_HW_TABLE_SIZE)	
		ret=rtk_l34_naptInboundTable_set(forced,idx,entry);
	
	if(ret==RT_ERR_OK)
	{
		memcpy(&rg_db.naptIn[idx].rtk_naptIn,entry,sizeof(*entry));
	}
	return ret;
}

int32 RTK_L34_NAPTOUTBOUNDTABLE_SET(int8 forced,uint32 idx, rtk_l34_naptOutbound_entry_t *entry)
{
	int ret=RT_ERR_OK;
	if(idx<MAX_NAPT_OUT_HW_TABLE_SIZE)		
		ret=rtk_l34_naptOutboundTable_set(forced,idx,entry);
	
	if(ret==RT_ERR_OK)
	{
		memcpy(&rg_db.naptOut[idx].rtk_naptOut,entry,sizeof(*entry));
	}
	return ret;
}

int32 RTK_L34_WANTYPETABLE_SET(uint32 idx,rtk_wanType_entry_t * entry)
{	
	int ret;
#if defined(CONFIG_APOLLO_RLE0371) || defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
	ret=RT_ERR_OK;
#else
	ret=rtk_l34_wanTypeTable_set(idx,entry);
#endif
	if(ret==RT_ERR_OK)
	{
		memcpy(&rg_db.wantype[idx].rtk_wantype,entry,sizeof(rtk_wanType_entry_t));
	}
	return ret;
}

int32 RTK_L34_BINDINGACTION_SET(rtk_l34_bindType_t bindType,rtk_l34_bindAct_t bindAction)
{
	int ret;
#if defined(CONFIG_APOLLO_RLE0371) || defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
	ret=RT_ERR_OK;
#else
	ret=rtk_l34_bindingAction_set(bindType,bindAction);
#endif
	if(ret==RT_ERR_OK)
	{
		rg_db.systemGlobal.l34BindAction[bindType]=bindAction;
	}
	return ret;
}

int32 RTK_L34_GLOBALSTATE_SET(rtk_l34_globalStateType_t stateType,rtk_enable_t state)
{
	int ret;

	ret=rtk_l34_globalState_set(stateType,state);
	if(ret==RT_ERR_OK)
	{
		rg_db.systemGlobal.l34GlobalState[stateType]=state;
	}
	return ret;
}

int32 RTK_L34_BINDINGTABLE_SET(uint32 idx,rtk_binding_entry_t * bindEntry)
{
	int ret;
	ret=rtk_l34_bindingTable_set(idx,bindEntry);
	if(ret==RT_ERR_OK)
	{
		//Copy to software MIB
		memcpy(&rg_db.bind[idx].rtk_bind,bindEntry,sizeof(rtk_binding_entry_t));
	}
	return ret;
}

int32 RTK_L34_IPV6ROUTINGTABLE_SET(uint32 idx,rtk_ipv6Routing_entry_t * ipv6RoutEntry)
{
	int ret=RT_ERR_OK;
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	//assign HW table to TRAP, SW table keep original
	rtk_l34_ipv6RouteType_t orig_type;
	orig_type=ipv6RoutEntry->type;
	ipv6RoutEntry->type=L34_IPV6_ROUTE_TYPE_TRAP;

#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
	if(idx==V6_DEFAULT_ROUTE_IDX){
		//patch for HW: CONFIG_RG_IPV6_NAPT_SUPPORT makes software routing table entry to 8.
		ret=rtk_l34_ipv6RoutingTable_set(V6_HW_DEFAULT_ROUTE_IDX,ipv6RoutEntry);//only force set HWNAT default route to trap, rest routing will not be set to HWNAT while IPv6 NAPT.
	}
#else
	ret=rtk_l34_ipv6RoutingTable_set(idx,ipv6RoutEntry);
#endif

	ipv6RoutEntry->type=orig_type;
#else
	ret=rtk_l34_ipv6RoutingTable_set(idx,ipv6RoutEntry);
#endif

	if(ret==RT_ERR_OK)
	{
		//Copy to software MIB
		memcpy(&rg_db.v6route[idx].rtk_v6route,ipv6RoutEntry,sizeof(rtk_ipv6Routing_entry_t));
	}
	return ret;
}

int32 RTK_L34_IPV6NEIGHBORTABLE_SET(uint32 idx,rtk_ipv6Neighbor_entry_t * ipv6NeighborEntry)
{
	int ret;
	ret=rtk_l34_ipv6NeighborTable_set(idx,ipv6NeighborEntry);
	if(ret==RT_ERR_OK)
	{
		//Copy to software MIB
		memcpy(&rg_db.v6neighbor[idx].rtk_v6neighbor,ipv6NeighborEntry,sizeof(rtk_ipv6Neighbor_entry_t));
	}
	return ret;
}


int32 RTK_VLAN_CREATE(rtk_vlan_t vid)
{
	int ret;
	ret=rtk_vlan_create(vid);
	if(ret==RT_ERR_OK || ret==RT_ERR_VLAN_EXIST)
		rg_db.vlan[vid].valid = 1;		//Enable software MIB record
	return ret;
}

int32 RTK_VLAN_PORT_SET(rtk_vlan_t vid,rtk_portmask_t * pMember_portmask,rtk_portmask_t * pUntag_portmask)
{
	int ret;
	rtk_portmask_t portmask,portmask2;
	portmask.bits[0]=(pMember_portmask->bits[0])&rg_db.systemGlobal.phyPortStatus;
	portmask2.bits[0]=(pUntag_portmask->bits[0])&rg_db.systemGlobal.phyPortStatus;
	ret=rtk_vlan_port_set(vid,&portmask,&portmask2);
	if(ret==RT_ERR_OK)
	{
		//Copy to software MIB
		memcpy(&rg_db.vlan[vid].MemberPortmask,&portmask,sizeof(rtk_portmask_t));
		memcpy(&rg_db.vlan[vid].UntagPortmask,&portmask2,sizeof(rtk_portmask_t));
	}
	return ret;
}

int32 RTK_VLAN_EXTPORT_SET(rtk_vlan_t vid,rtk_portmask_t * pExt_portmask)
{
	int ret;
	ret=rtk_vlan_extPort_set(vid,pExt_portmask);
	if(ret==RT_ERR_OK)
	{
		//Copy to software MIB
		memcpy(&rg_db.vlan[vid].Ext_portmask,pExt_portmask,sizeof(rtk_portmask_t));
	}
	return ret;
}

int32 RTK_VLAN_FID_SET(rtk_vlan_t vid,rtk_fid_t fid)
{
	int ret;
	ret=rtk_vlan_fid_set(vid,fid);
	if(ret==RT_ERR_OK)
	{
		//Copy to software MIB
		memcpy(&rg_db.vlan[vid].fid,&fid,sizeof(rtk_fid_t));
	}
	return ret;
}

int32 RTK_VLAN_FIDMODE_SET(rtk_vlan_t vid,rtk_fidMode_t mode)
{

	int ret;
	ret=rtk_vlan_fidMode_set(vid,mode);
	if(ret==RT_ERR_OK)
	{
		//Copy to software MIB
		memcpy(&rg_db.vlan[vid].fidMode,&mode,sizeof(rtk_fidMode_t));
	}
	return ret;
}


int32 RTK_VLAN_PRIORITY_SET(rtk_vlan_t vid,rtk_pri_t priority)
{
	int ret;
	ret=rtk_vlan_priority_set(vid,priority);
	if(ret==RT_ERR_OK)
	{
		//Copy to software MIB
		memcpy(&rg_db.vlan[vid].priority,&priority,sizeof(rtk_pri_t));
	}
	return ret;
}


int32 RTK_VLAN_PRIORITYENABLE_SET(rtk_vlan_t vid,rtk_enable_t enable)
{
	int ret;
	ret=rtk_vlan_priorityEnable_set(vid,enable);
	if(ret==RT_ERR_OK)
	{
		//Copy to software MIB
		memcpy(&rg_db.vlan[vid].priorityEn,&enable,sizeof(rtk_enable_t));
	}
	return ret;
}

int32 RTK_VLAN_DESTROY(rtk_vlan_t vid)
{
	int ret;
	ret=rtk_vlan_destroy(vid);
	if(ret==RT_ERR_OK)
		bzero(&rg_db.vlan[vid],sizeof(rtk_rg_table_vlan_t));		//Disable software MIB record
	return ret;
}

int32 RTK_VLAN_PORTPVID_SET(rtk_port_t port,uint32 pvid)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	ret=rtk_vlan_portPvid_set(port,pvid);
	if(ret==RT_ERR_OK)
	{
		rg_db.systemGlobal.portBasedVID[port]=pvid;
#if defined(CONFIG_APOLLO)
#if defined(CONFIG_RTL9600_SERIES)
//#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
		if (rg_kernel.stag_enable==RTK_RG_ENABLED) {
			if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT2) && (rg_db.systemGlobal.service_pmsk.portmask&(1<<port)))
			{		
				ASSERT_EQ(_rtk_rg_acl_reserved_stag_ingressCVidFromPVID(pvid, port),RT_ERR_RG_OK);
			}
		}
//#endif
#endif
#endif
	}
	return ret;
}

int32 RTK_VLAN_EXTPORTPVID_SET(uint32 extPort,uint32 pvid)
{
	int ret;
	ret=rtk_vlan_extPortPvid_set(extPort,pvid);
	if(ret==RT_ERR_OK)
		rg_db.systemGlobal.portBasedVID[extPort+RTK_RG_PORT_CPU]=pvid;
	return ret;
}

int32 RTK_VLAN_PROTOGROUP_SET(uint32 protoGroupIdx,rtk_vlan_protoGroup_t *pProtoGroup)
{
	int ret;
	ret=rtk_vlan_protoGroup_set(protoGroupIdx,pProtoGroup);
	if(ret==RT_ERR_OK)
	{
		//Copy to software MIB
		memcpy(&rg_db.systemGlobal.protoGroup[protoGroupIdx],pProtoGroup,sizeof(rtk_vlan_protoGroup_t));
	}
	return ret;
}

int32 RTK_VLAN_PORTPROTOVLAN_SET(rtk_port_t port,uint32 protoGroupIdx,rtk_vlan_protoVlanCfg_t * pVlanCfg)
{
	int ret;
	ret=rtk_vlan_portProtoVlan_set(port,protoGroupIdx,pVlanCfg);
	if(ret==RT_ERR_OK)
	{
		//Copy to software MIB
		memcpy(&rg_db.systemGlobal.protoBasedVID[port].protoVLANCfg[protoGroupIdx],pVlanCfg,sizeof(rtk_vlan_protoVlanCfg_t));
	}
	return ret;
}

int32 RTK_VLAN_PORTIGRFILTERENABLE_SET(rtk_port_t port, rtk_enable_t igr_filter)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
#if defined(CONFIG_APOLLO)
	return rtk_vlan_portIgrFilterEnable_set(port, igr_filter);
#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
	return _rtk_vlan_portIgrFilterEnable_set(port, igr_filter);
#endif
}


int32 RTK_QOS_1PPRIREMAPGROUP_SET(
    uint32      grpIdx,
    rtk_pri_t   dot1pPri,
    rtk_pri_t   intPri,
    uint32      dp)
{
	int ret;
	ret=rtk_qos_1pPriRemapGroup_set(grpIdx,dot1pPri,intPri,dp);
	if(ret==RT_ERR_OK)
		rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemapToInternalPriTbl[dot1pPri]=intPri;
	
	return ret;
}

int32 RTK_QOS_DSCPPRIREMAPDROUP_SET(
	uint32      grpIdx,
    uint32      dscp,
    rtk_pri_t   intPri,
    uint32      dp)
{
	int ret;
	ret=rtk_qos_dscpPriRemapGroup_set(grpIdx,dscp,intPri,dp);

	if(ret==RT_ERR_OK)
		rg_db.systemGlobal.qosInternalDecision.qosDscpRemapToInternalPri[dscp]=intPri;

	return ret;
}

int32 RTK_QOS_PORTPRI_SET(rtk_port_t port, rtk_pri_t intPri)
{
	int ret;
	ret=rtk_qos_portPri_set(port,intPri);
	if(ret==RT_ERR_OK)
		rg_db.systemGlobal.qosInternalDecision.qosPortBasedPriority[port]=intPri;

	return ret;
}

int32 RTK_QOS_PRISELGROUP_SET(uint32 grpIdx, rtk_qos_priSelWeight_t *pWeightOfPriSel)
{	
	int ret;
	ret=rtk_qos_priSelGroup_set(grpIdx,pWeightOfPriSel);
	if(ret==RT_ERR_OK){
		rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_PORTBASED]=pWeightOfPriSel->weight_of_portBased;
		rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_DOT1Q]=pWeightOfPriSel->weight_of_dot1q;
		rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_DSCP]=pWeightOfPriSel->weight_of_dscp;
		rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_ACL]=pWeightOfPriSel->weight_of_acl;
		rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_LUTFWD]=pWeightOfPriSel->weight_of_lutFwd;
		rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_SABASED]=pWeightOfPriSel->weight_of_saBaed;
		rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_VLANBASED]=pWeightOfPriSel->weight_of_vlanBased;
		rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_SVLANBASED]=pWeightOfPriSel->weight_of_svlanBased;
		rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_L4BASED]=pWeightOfPriSel->weight_of_l4Based;
	}	
	return ret;
}

int32 RTK_QOS_DSCPREMARKENABLE_SET(rtk_port_t port, rtk_enable_t enable)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_qos_dscpRemarkEnable_set(port, enable);
}

int32 RTK_QOS_PORTDSCPREMARKSRCSEL_SET(rtk_port_t port, rtk_qos_dscpRmkSrc_t type)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_qos_portDscpRemarkSrcSel_set(port, type);
}

int32 RTK_QOS_PORTPRIMAP_SET(rtk_port_t port, uint32 group)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_qos_portPriMap_set(port, group);
}


/*avoid L2 CF directly call rtk classify api & using wrong index. it should call rtk_rg_classify_cfgEntry_add()*/
int32 RTK_CLASSIFY_CFGENTRY_ADD(rtk_classify_cfg_t *pClassifyCfg){
	int ret;
	ret=rtk_classify_cfgEntry_add(pClassifyCfg);
	return ret;
}

/*avoid L2 CF directly call rtk classify api & using wrong index. it should call rtk_rg_classify_cfgEntry_add()*/
int32 RTK_SVLAN_TPIDENTRY_SET(uint32 svlan_index, uint32 svlan_tag_id){
	int ret;
	ret=rtk_svlan_tpidEntry_set(svlan_index,svlan_tag_id);
	if(ret==RT_ERR_OK){
		if(svlan_index==0){//set tpid1
			#ifdef CONFIG_RTL9602C_SERIES
				//sync gmac tpid for parsing and checksum offload
				uint32 val;
				ASSERT_EQ(ioal_socMem32_read(0xB8012064, &val),RT_ERR_OK);
				val = ((svlan_tag_id & 0xffff)<<16) | (val & 0xffff);
				ASSERT_EQ(ioal_socMem32_write(0xB8012064, val),RT_ERR_OK);
			#endif
				rg_db.systemGlobal.tpid = svlan_tag_id;
		}
		else if(svlan_index==1)
		{
			//gmac not support tpid2
			rg_db.systemGlobal.tpid2 = svlan_tag_id;
		}
	}

	return ret;
}

#if defined(CONFIG_RTL9602C_SERIES)
int32 RTK_SVLAN_TPIDENABLE_SET(uint32 svlanIndex, rtk_enable_t enable){
	int ret;
	ret=rtk_svlan_tpidEnable_set(svlanIndex, enable);
	if(ret==RT_ERR_OK){
		rg_db.systemGlobal.tpid2_en = enable;
	}	
	return ret;
}
#endif

int32 RTK_SVLAN_SERVICEPORT_SET(rtk_port_t port, rtk_enable_t enable)
{
	//DEBUG("\ndebug: RTK_SVLAN_SERVICEPORT_SET port=0x%x enable=%d\n", port, enable);
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	ret=rtk_svlan_servicePort_set(port,enable);
	if(ret==RT_ERR_OK){
		rg_db.systemGlobal.service_pmsk.portmask &= (~(1<<port));
		if(enable==ENABLED)
		{
			rg_db.systemGlobal.service_pmsk.portmask|= (1<<port);
		}
	}
	return ret;
}

int32 RTK_SWITCH_MAXPKTLENBYPORT_SET(rtk_port_t port, int pktlen)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_switch_maxPktLenByPort_set(port, pktlen);
}

int32 RTK_SVLAN_SERVICEPORT_GET(rtk_port_t port, rtk_enable_t *pEnable)
{
	int ret=RT_ERR_OK;
	//DEBUG("\ndebug: RTK_SVLAN_SERVICEPORT_GET port=0x%x enable=%d\n", port, *pEnable);
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_svlan_servicePort_get(port, pEnable);
}

int32 RTK_SVLAN_DMACVIDSELSTATE_SET(rtk_port_t port, rtk_enable_t enable)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	ret=rtk_svlan_dmacVidSelState_set(port, enable);
	if(ret==RT_ERR_OK){
		rg_db.systemGlobal.svlan_EP_DMAC_CTRL_pmsk.portmask &= (~(1<<port));
		if(enable==ENABLED)
		{
			rg_db.systemGlobal.svlan_EP_DMAC_CTRL_pmsk.portmask|= (1<<port);
		}
	}
	return ret;
}

int32 RTK_SVLAN_MEMBERPORT_SET(rtk_vlan_t svid, rtk_portmask_t * pSvlanPortmask, rtk_portmask_t * pSvlanUntagPortmask)
{
	rtk_portmask_t portmask,portmask2;
	portmask.bits[0]=(pSvlanPortmask->bits[0])&rg_db.systemGlobal.phyPortStatus;
	portmask2.bits[0]=(pSvlanUntagPortmask->bits[0])&rg_db.systemGlobal.phyPortStatus;
	return rtk_svlan_memberPort_set(svid, &portmask, &portmask2);
}

int32 RTK_SVLAN_FIDENABLE_SET(rtk_vlan_t svid, rtk_enable_t enable)
{
	return rtk_svlan_fidEnable_set(svid, enable);
}

int32 RTK_SVLAN_FID_SET(rtk_vlan_t svid, rtk_fid_t fid)
{
	return rtk_svlan_fid_set(svid, fid);
}

int32 RTK_SVLAN_UNTAGACTION_SET(rtk_svlan_action_t action, rtk_vlan_t svid)
{
	return rtk_svlan_untagAction_set(action,svid);
}

int32 RTK_ACL_IGRSTATE_SET(rtk_port_t port, rtk_enable_t state)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_acl_igrState_set(port, state);
}

int32 RTK_ACL_IGRUNMATCHACTION_SET(rtk_port_t port, rtk_filter_unmatch_action_type_t action)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_acl_igrUnmatchAction_set(port, action);
}

int32 RTK_TRAP_PORTIGMPMLDCTRLPKTACTION_SET(rtk_port_t port, rtk_trap_igmpMld_type_t igmpMldType, rtk_action_t action)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_trap_portIgmpMldCtrlPktAction_set(port, igmpMldType, action);
}

int32 RTK_PORT_ISOLATIONIPMCLEAKY_SET(rtk_port_t port, rtk_enable_t enable)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_port_isolationIpmcLeaky_set(port, enable);
}

int32 RTK_PORT_ISOLATIONENTRY_SET(rtk_port_isoConfig_t mode, rtk_port_t port, rtk_portmask_t * pPortmask, rtk_portmask_t * pExtPortmask)
{
	int ret=RT_ERR_OK;
	rtk_portmask_t portmask;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	portmask.bits[0]=(pPortmask->bits[0])&rg_db.systemGlobal.phyPortStatus;
	return rtk_port_isolationEntry_set(mode, port, &portmask, pExtPortmask);
}

int32 RTK_PORT_ISOLATIONENTRYEXT_SET(rtk_port_isoConfig_t mode, rtk_port_t port, rtk_portmask_t *pPortmask, rtk_portmask_t *pExtPortmask)
{
	int ret=RT_ERR_OK;
	rtk_portmask_t portmask;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	portmask.bits[0]=(pPortmask->bits[0])&rg_db.systemGlobal.phyPortStatus;
	return rtk_port_isolationEntryExt_set(mode, port, &portmask, pExtPortmask);
}

int32 RTK_PORT_MACFORCEABILITY_GET(rtk_port_t port, rtk_port_macAbility_t * pMacAbility)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_port_macForceAbility_get(port, pMacAbility);
}

int32 RTK_PORT_MACFORCEABILITY_SET(rtk_port_t port,rtk_port_macAbility_t macAbility)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_port_macForceAbility_set(port, macAbility);
}

int32 RTK_PORT_MACFORCEABILITYSTATE_SET(rtk_port_t port,rtk_enable_t state)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_port_macForceAbilityState_set(port, state);
}

#if defined(CONFIG_RTL9602C_SERIES)
int32 RTK_L34_DSLITEINFTABLE_SET(rtk_l34_dsliteInf_entry_t *pDsliteInfEntry)
{
	int ret;
	
	ret=rtk_l34_dsliteInfTable_set(pDsliteInfEntry);
	if(ret==RT_ERR_OK)
	{
		memcpy(&rg_db.dslite[pDsliteInfEntry->index].rtk_dslite,pDsliteInfEntry,sizeof(*pDsliteInfEntry));
	}
	return ret;
}
#endif
