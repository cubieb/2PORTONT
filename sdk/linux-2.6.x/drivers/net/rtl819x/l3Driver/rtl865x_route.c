/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.
* 
* Program : route table driver
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/
/*      @doc RTL_LAYEREDDRV_API

        @module rtl865x_route.c - RTL865x Home gateway controller Layered driver API documentation       |
        This document explains the API interface of the table driver module. Functions with rtl865x prefix
        are external functions.
        @normal Hyking Liu (Hyking_liu@realsil.com.cn) <date>

        Copyright <cp>2008 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

        @head3 List of Symbols |
        Here is a list of all functions and variables in this module.
        
        @index | RTL_LAYEREDDRV_API
*/
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include <linux/netdevice.h>
//#include <net/rtl/rtl865x_ppp.h>
#include <net/rtl/rtl865x_netif.h>
#include "common/rtl_errno.h"
//#include "rtl_utils.h"
#include "common/rtl865x_netif_local.h"
#include "rtl865x_ppp_local.h"
#include "rtl865x_route.h"
#include "rtl865x_ip.h"
#include "rtl865x_nexthop.h"
#include "rtl865x_arp.h"

#define Stop_keeping_defalut_route
//#include "common/rtl_glue.h"
#include "common/rtl865x_eventMgr.h" /*call back function....*/
#include "common/rtl865x_vlan.h"
#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL2.h"
#include "AsicDriver/rtl865x_asicL3.h"
#else
#include "AsicDriver/rtl865xC_tblAsicDrv.h"
#endif
#include "l2Driver/rtl865x_fdb.h"
#include "AsicDriver/rtl865xc_asicregs.h"

static rtl865x_route_t *rtl865x_route_freeHead;
static rtl865x_route_t *rtl865x_route_inusedHead;
static RTL_DECLARE_MUTEX(route_sem);

#if 0
void _rtl865x_route_print(void)
{
	int32 i;
	rtl865x_route_t *rt = NULL;
	rt = rtl865x_route_inusedHead;

	printk("=============inused list:\n");
	while(rt)
	{		
		printk("asicIdx(%d),ip(0x%x),mask(0x%x),nextHop(0x%x),process(0x%x),dst(%s) valid(%d)\n",rt->asicIdx, rt->ipAddr,rt->ipMask,rt->nextHop,rt->process,rt->dstNetif->name,rt->valid);
		rt = rt->next;
	}

	i = 0;
	rt = rtl865x_route_freeHead;
	while(rt)
	{
		i++;
		rt = rt->next;
	}
	
	printk("=======free list count(%d)\n",i);
}
#endif


static rtl865x_route_t* _rtl865x_getDefaultRoute(void)
{
	rtl865x_route_t *rt = NULL;	
	rt = rtl865x_route_inusedHead;

	while(rt)
	{
		if((rt->valid==1)&&(rt->asicIdx == RT_ASIC_ENTRY_NUM -1))
			return rt;
		rt = rt->next;
	}

	return NULL;
}

static rtl865x_route_t* _rtl865x_getRouteEntry(ipaddr_t ipAddr, ipaddr_t ipMask)
{
	rtl865x_route_t *rt = NULL;	
	rt = rtl865x_route_inusedHead;

	while(rt)
	{
		if((rt->valid==1)&&((rt->ipAddr&rt->ipMask)==(ipAddr&ipMask))&&(rt->ipMask==ipMask))
			return rt;
		rt = rt->next;
	}	
	return NULL;
}


static int32 _rtl865x_synRouteToAsic(rtl865x_route_t *rt_t)
{
	int32 ret = FAILED;
	rtl865x_tblAsicDrv_routingParam_t asic_t;
	rtl865x_tblAsicDrv_l2Param_t asic_l2;
	rtl865x_netif_local_t *dstNetif = NULL;
	uint32 columIdx,fid;
	int32 pppIdx = 0;

	if(!rt_t->needInAsic)
		return SUCCESS;
		
#if 0
//#ifdef CONFIG_RTL_MULTI_ETH_WAN
	extern int enable_port_mapping;

	if (enable_port_mapping && !rt_t->is_internal)
		return SUCCESS;
#endif

	bzero(&asic_t, sizeof(rtl865x_tblAsicDrv_routingParam_t));

	if(rt_t == NULL)
	{
		printk("%s(%d):NULL!!!!!!!!!!!!!!!!\n",__FUNCTION__,__LINE__);
	}
	/*common information*/
	asic_t.ipAddr	= rt_t->ipAddr;
	asic_t.ipMask	= rt_t->ipMask;
	asic_t.ipAddr = rt_t->ipAddr;
	asic_t.ipMask = rt_t->ipMask;
	
	dstNetif = _rtl865x_getNetifByName(rt_t->dstNetif);
	if(dstNetif==NULL)
		return FAILED;
	
	asic_t.vidx = dstNetif->asicIdx;
	asic_t.internal = dstNetif->is_wan? 0 : 1;
	asic_t.DMZFlag = dstNetif->dmz? 1 : 0;
	asic_t.process = rt_t->process;
	
	switch (rt_t->process)
	{
	case RT_PPPOE:
		ret = rtl865x_getPppIdx(rt_t->un.pppoe.pppInfo, &pppIdx);
		
		asic_t.pppoeIdx = pppIdx;
		/*
		*if process==RT_PPPOE, the mac address of pppoe server is add in pppoe module,
		*so, we read the FDB information directly....
		*/
		ret = rtl865x_getVlanFilterDatabaseId(dstNetif->vid,&fid);		
		ret = rtl865x_Lookup_fdb_entry(fid, (ether_addr_t *)rt_t->un.pppoe.macInfo, FDB_DYNAMIC|FDB_STATIC, &columIdx,&asic_l2);

		if(ret != SUCCESS)
			printk("can't get l2 entry by mac.....\n");
		
		/*FIXME_hyking: update mac/fdb table reference count*/
		asic_t.nextHopRow = rtl8651_filterDbIndex( rt_t->un.pppoe.macInfo,fid);
		asic_t.nextHopColumn = columIdx;
		break;

	case RT_L2:
		/*
		* NOTE:this type not used now...
		* if we want to use it, please add FDB entry to sure this L2 entry in both software FDB table and Asic L2 table.
		*/
		ret = rtl865x_getVlanFilterDatabaseId(dstNetif->vid,&fid);		
		ret = rtl865x_Lookup_fdb_entry(fid, (ether_addr_t *)rt_t->un.direct.macInfo, FDB_STATIC, &columIdx,&asic_l2);
		if(ret != SUCCESS)
			printk("can't get l2 entry by mac.....\n");

		/*FIXME_hyking: update mac/fdb table reference count*/
		asic_t.nextHopRow = rtl8651_filterDbIndex(rt_t->un.direct.macInfo,fid);
		asic_t.nextHopColumn = columIdx;

		break;

	case RT_ARP:
		/*FIXME_hyking: update arp table reference count??*/
		asic_t.arpStart	= rt_t->un.arp.arpsta;
		asic_t.arpEnd	= rt_t->un.arp.arpend;	
		asic_t.arpIpIdx	= rt_t->un.arp.arpIpIdx;		
		break;
		
	case RT_CPU:
	case RT_DROP:
		/*do nothing*/
		break;
		
	case RT_NEXTHOP:
 		asic_t.nhStart		 = rt_t->un.nxthop.nxtHopSta;
		asic_t.nhNum	 	 = rt_t->un.nxthop.nxtHopEnd - rt_t->un.nxthop.nxtHopSta + 1;
		asic_t.nhNxt	 	 = asic_t.nhStart;
		asic_t.ipDomain		 = rt_t->un.nxthop.ipDomain;
		asic_t.nhAlgo	 	 = rt_t->un.nxthop.nhalog;	

		break;
		
	default:
		printk("Process_Type(%d) is not support!\n",rt_t->process);
	}

	if(rt_t->asicIdx > RT_ASIC_ENTRY_NUM-1)
	{
		printk("BUG!! %s(%d)....", __FUNCTION__,__LINE__);
		return FAILED;
	}
	
	ret = rtl8651_setAsicRouting(rt_t->asicIdx, &asic_t);

	return ret;
	
}

static int32 _rtl865x_updateDefaultRoute(rtl865x_route_t *rt, int32 action)
{
	int32 i;
	rtl865x_route_t *entry;
	int32 retval = FAILED;

	rtl865x_netif_local_t *netif = NULL;
	netif = _rtl865x_getSWNetifByName(rt->dstNetif);
	if(netif == NULL)
		return RTL_EINVALIDINPUT;

	entry = rt;
	if(entry == NULL)
		return RTL_EINVALIDINPUT;
	
	/*delete nexthop which is add by default route*/
	if(rt->process == RT_NEXTHOP)
		for ( i = entry->un.nxthop.nxtHopSta; i <= entry->un.nxthop.nxtHopEnd && entry->un.nxthop.nxtHopEnd != 0; i++)
		{
			retval = rtl865x_delNxtHop(NEXTHOP_L3, i);
		}

	entry->un.nxthop.nxtHopSta = 0;
	entry->un.nxthop.nxtHopEnd = 0;
	switch(action)
	{
		case RT_DEFAULT_RT_NEXTHOP_CPU:
			retval = rtl865x_addNxtHop(NEXTHOP_L3, (void*)entry, entry->dstNetif, 0,entry->srcIp);
			break;

		case RT_DEFAULT_RT_NEXTHOP_NORMAL:
			{
				rt->process = RT_NEXTHOP;	
				switch(netif->if_type)
				{
					case IF_ETHER:
						retval = rtl865x_addNxtHop(NEXTHOP_L3, (void*)rt, rt->dstNetif, rt->nextHop,entry->srcIp);
						break;
					case IF_PPPOE:
						{
							rtl865x_ppp_t *pppoe;
							
							pppoe = rtl865x_getPppByNetifName(rt->dstNetif);

							if(pppoe != NULL)
							{
								/*got pppoe session*/
								retval = rtl865x_addNxtHop(NEXTHOP_L3, (void*)rt, rt->dstNetif, pppoe->sessionId,entry->srcIp);
							}
							else
								/*nexthop's action is to CPU*/
								retval = rtl865x_addNxtHop(NEXTHOP_L3, (void*)rt, rt->dstNetif, 0,entry->srcIp);
						}
						break;
				}
			}
			break;
	}
	retval = _rtl865x_synRouteToAsic(entry);

	return retval;
}

static int32 _rtl865x_arrangeRoute(rtl865x_route_t *start_rt, int32 start_idx)
{
	int32 count;
	rtl865x_route_t *rt = NULL;




	rt = start_rt;
	count = 0;
	while(rt)
	{	
	//printk("[%d] count: %d  \n",__LINE__,count);
		if(rt->valid)
		{
        	
			/*if the rule is default route...*/
			if(rt->ipMask == 0){
				rt->asicIdx = RT_ASIC_ENTRY_NUM-1;
				}
			else
			{
				/* entry number more than asic table's capacity*/
				/* entry index=RT_ASIC_ENTRY_NUM-1 is reserved for default route*/
				if((start_idx + count > RT_ASIC_ENTRY_NUM-2))
					break;
				
				/*delete old asic entry firstly...*/
				if(start_idx+count < rt->asicIdx && rt->asicIdx < RT_ASIC_ENTRY_NUM-1){
					rtl8651_delAsicRouting(rt->asicIdx);
                }
				rt->asicIdx = start_idx+count;

				//QL: count keeps the num of asic route rules, so don't increase if sync route to asic fail.
				if (!rt->needInAsic)
				{
					rt->asicIdx = 0x0F;
					rt = rt->next;
					continue;
				}

				_rtl865x_synRouteToAsic(rt);
			}			
		}

		/*next entry*/
		rt= rt->next;
		count++;
	}

	
	/*more route entry need to add?*/
	if(rt)
	{
		/*not enough asic table entry! have to update default route's action TOCPU*/
		rt = _rtl865x_getDefaultRoute();
		if(rt)
			_rtl865x_updateDefaultRoute(rt, RT_DEFAULT_RT_NEXTHOP_CPU);		
	}
	else
	{
		rt = _rtl865x_getDefaultRoute();
		if(rt)
			_rtl865x_updateDefaultRoute(rt, RT_DEFAULT_RT_NEXTHOP_NORMAL);
	}
	
	return SUCCESS;
}

static int32 _rtl865x_addRouteToInusedList(rtl865x_route_t *rt)
{
	int32 retval = FAILED;
	int32 start_idx = 0;
	rtl865x_route_t *entry,*fore_rt,*start_rt=NULL;
#ifdef CONFIG_RTL_MULTI_ETH_WAN
	extern int enable_port_mapping;
#endif

	fore_rt = NULL;
	entry = rtl865x_route_inusedHead;

	/*always set 0x0f when init..., this value would be reset in arrange route*/
	rt->asicIdx = 0x0f;
	rt->next = NULL;
		
	/*find the right position...*/
	while(entry)
	{
		if(entry->valid == 1)
		{
			if(entry->ipMask < rt->ipMask)
			{
				break;
			}
		}
		fore_rt = entry;
		entry = entry->next;
	}

	/*insert this rule after insert_entry*/
	if(fore_rt)
	{
		rt->next = fore_rt->next;
		fore_rt->next = rt;
		start_idx = fore_rt->asicIdx+1;
		start_rt = rt;
	}
	else		
	{
		/*insert head...*/
		rt->next = rtl865x_route_inusedHead;		
		rtl865x_route_inusedHead = rt;
		
		start_idx = 0;
		start_rt = rtl865x_route_inusedHead;
	}	

#ifdef CONFIG_RTL_MULTI_ETH_WAN
	if ( enable_port_mapping )
	{
		start_idx = 0;
		start_rt = rtl865x_route_inusedHead;
	}
#endif

	retval = _rtl865x_arrangeRoute(start_rt, start_idx);
	
	//_rtl865x_route_print();
	return retval;
	
}

static int32 _rtl865x_delRouteFromInusedList(rtl865x_route_t * rt)
{
	int32 retval,start_idx;
	rtl865x_route_t *fore_rt = NULL,*entry = NULL,*start_rt = NULL;

	entry = rtl865x_route_inusedHead;
	while(entry)
	{
		if(entry == rt)
			break;

		fore_rt = entry;		
		entry = entry->next;
	}

	/*fore_rt == NULL means delete list head*/
	if(fore_rt == NULL)
	{
		rtl865x_route_inusedHead = rtl865x_route_inusedHead->next;
		start_rt = rtl865x_route_inusedHead;
		start_idx = 0;
	}
	else
	{
		fore_rt->next = rt->next;
		start_rt = fore_rt->next;
		start_idx = fore_rt->asicIdx + 1;
	}

	/*delete route from asic*/
	if(rt->asicIdx < RT_ASIC_ENTRY_NUM)
		rtl8651_delAsicRouting(rt->asicIdx);
	
	retval = _rtl865x_arrangeRoute(start_rt, start_idx);
	rt->asicIdx = 0x0f;
	
	//_rtl865x_route_print();
	
	return retval;	
	
}


static int32 _rtl865x_usedNetifInRoute(int8 *ifname)
{

	rtl865x_route_t *rt = NULL;
	rt = rtl865x_route_inusedHead;

	while(rt)
	{	
		if(memcmp(rt->dstNetif,ifname,strlen(ifname)) == 0)
			return SUCCESS;
		rt = rt->next;
	}	
	return FAILED;
}

static int32 _rtl865x_addRoute(ipaddr_t ipAddr, ipaddr_t ipMask, ipaddr_t nextHop, int8 * ifName,ipaddr_t srcIp,int needAddIntoHW)
{
	rtl865x_netif_local_t *netif = NULL;
	rtl865x_route_t *rt = NULL,*tmp_rt = NULL;
	int32 idx;
	int32 netSize = 0, usedArpCnt = 0;
	int32 retval = FAILED;

	//printk("%s %x/%x nextHop %x ifname %s srcip %x!\n", __func__, ipAddr, ipMask, nextHop, ifName, srcIp);
	/*para check*/
	netif = _rtl865x_getSWNetifByName(ifName);

	if(netif == NULL)
		return RTL_EINVALIDINPUT;
	
	if(netif->if_type == IF_NONE)
		return RTL_ENOLLTYPESPECIFY;


	/* if this is a slave netif,  check wthether its master netif existed ?  
		ex. when we set up a pppoe dsl-wan , we add routing entry of ppp0
		      ppp0 existed in sw_netif , but vc0 is not existed  => we cannot add ppp0 into sw_l3
	*/
	if(netif->is_slave && netif->master== NULL)
		return RTL_EINVALIDINPUT;
	idx = 0;
	for(idx = 0; idx < 32; idx++)
		if((1<<idx) & ipMask)
			break;

	netSize = 1<<idx;

	if(netSize > RT_MAX_ARP_SIZE)
		return RTL_EINVALIDINPUT;

	
	/*
	*duplicate entry check:
	*	in Driver system, default route is always exist.
	*	so if ipMask == 0, it's means that default route should be update...
	*/
	if(ipMask != 0 && (rt = _rtl865x_getRouteEntry(ipAddr, ipMask)) != NULL)
	{
		//rt->ref_count++;
		return RTL_EENTRYALREADYEXIST;
	}

	/*add default route: just update the default route becase the default route always exist!*/
	if(ipMask == 0)
	{
		rt = _rtl865x_getDefaultRoute();
		/*deference rt's orginal netif*/
		//if(rt && rt->dstNetif)
			//rtl865x_deReferNetif(rt->dstNetif->name);
	}

	/*allocate a new buffer for adding entry*/
	if(rt == NULL)
	{
		rt = rtl865x_route_freeHead;

		if(rt)
		{
			rtl865x_route_freeHead = rt->next;
			
		}
	}
	
	if(rt == NULL)
	{
		/*no buffer, default route should be TOCPU?*/
		return RTL_ENOFREEBUFFER;
	}

#if 0
//#ifdef CONFIG_RTL_MULTI_ETH_WAN

if ((ifName == NULL) ||
//strcmp(ifName,ALIASNAME_BR0))
!alias_name_is_eq(CMD_CMP,ifName,ALIASNAME_BR0))
		rt->is_internal = 0;
	else
		rt->is_internal = 1;
#endif
		
	/*common information*/
	rt->ipAddr 	= ipAddr & ipMask;
	rt->ipMask 	= ipMask;
	rt->nextHop	= nextHop;
	rt->srcIp		= srcIp;
	rt->needInAsic= needAddIntoHW;
	strcpy(rt->dstNetif,netif->name);	
	
	/*don't specify the nexthop ip address, it's means that:
	* all packet match this route entry should be forward by network interface with arp
	*/

	if(nextHop == 0 && ipMask != 0)
	{
		switch(netif->if_type)
		{
			case IF_ETHER:
				
				rt->process = RT_ARP;
				tmp_rt = rtl865x_route_inusedHead;
				while(tmp_rt)
				{
					if(tmp_rt->valid && tmp_rt->process == RT_ARP && !strcmp(tmp_rt->dstNetif,netif->name))
						usedArpCnt += tmp_rt->un.arp.arpend - tmp_rt->un.arp.arpsta + 1;
					
					tmp_rt = tmp_rt->next;
				}
				
				if((usedArpCnt + netSize) > RT_MAX_ARP_SIZE)
				{
					printk("!!!!ERROR!!!usedArp(%d),netsize(%d)\n",usedArpCnt,netSize);
					goto addFailed;
				}

				/*allocate arp entry for this route rule...*/
				
				retval = rtl865x_arp_tbl_alloc(rt);
				if( retval != SUCCESS)
				{
					printk("error!!can't allocate arp for this route entry....retval(%d)\n",retval);
					goto addFailed;
				}

				rt->un.arp.arpIpIdx = 0; /*FIXME_hyking[this field is invalid, right?]*/
				
				break;

			case IF_PPPOE:
				{
					rtl865x_ppp_t *ppp = NULL;

					rt->process = RT_PPPOE;
					ppp = rtl865x_getPppByNetifName(netif->name);					

					if(ppp == NULL)
					{
						printk("error!!can't get pppoe session information by interface name(%s)\n",netif->name);
						goto addFailed;
					}

					rt->un.pppoe.macInfo = &ppp->server_mac;
					rt->un.pppoe.pppInfo = ppp;
					
					/*update reference...*/
					rtl865x_referPpp(ppp->sessionId);
#ifdef CONFIG_RTL_MULTI_ETH_WAN
					/*got pppoe session*/
					retval = rtl865x_getNxtHopIdx(NEXTHOP_DEFREDIRECT_ACL, netif->name, ppp->sessionId, srcIp);
					if (-1 == retval) {
						retval = rtl865x_addNxtHop(NEXTHOP_DEFREDIRECT_ACL, (void*)rt, netif->name, ppp->sessionId, srcIp);
						if(retval != SUCCESS)
						{
							printk("error!!add nexthop error! retval (%d)\n",retval);
							goto addFailed;
						}
					}
#endif
				}
				break;
			case IF_PPTP:
			case IF_L2TP:
				{
					rtl865x_ppp_t *ppp = NULL;

					rt->process = RT_L2;
					ppp = rtl865x_getPppByNetifName(netif->name);					

					if(ppp == NULL)
					{
						/*printk("Warning!!CAn't get pptp/l2tp session information by interface name(%s)\n",netif->name);*/
						goto addFailed;
					}

					rt->un.direct.macInfo = &ppp->server_mac;
				}
				break;

			default:
				printk("lltype(%d) is not support now....\n",netif->if_type);
				goto addFailed;
		}
		
	}
	else
	{
		/*if default is valid, delete nexthop firstly...*/
		if(rt->valid == 1 && rt->process == RT_NEXTHOP)
		{
			int i;  
			for ( i = rt->un.nxthop.nxtHopSta; i <= rt->un.nxthop.nxtHopEnd; i++)
			{
				retval = rtl865x_delNxtHop(NEXTHOP_L3, i);
			}       
		}
		
		/*use nexthop type*/
		rt->process = RT_NEXTHOP;			
		switch(netif->if_type)
		{
			case IF_ETHER:
				retval = rtl865x_addNxtHop(NEXTHOP_L3, (void*)rt, netif->name, nextHop,srcIp);
				break;
			case IF_PPPOE:
			case IF_PPTP:
			case IF_L2TP:
				{
					rtl865x_ppp_t *pppoe;
					
					pppoe = rtl865x_getPppByNetifName(netif->name);

					if(pppoe != NULL)
					{
						/*got pppoe session*/
						retval = rtl865x_addNxtHop(NEXTHOP_L3, (void*)rt, netif->name, pppoe->sessionId,srcIp);
					}
					else
					{
						/*nexthop's action is to CPU*/
						//retval = rtl865x_addNxtHop(NEXTHOP_L3, (void*)rt, netif->name, 0,srcIp);
						goto addFailed;
					}
				}
				break;

			default:
				retval = FAILED;
				break;
				
		}
		
		if(retval != SUCCESS)
		{
			printk("error!!add nexthop error! retval (%d)\n",retval);
			goto addFailed;	
		}		
		rt->un.nxthop.nhalog = RT_ALOG_SIP; /* use per-source IP */
		rt->un.nxthop.ipDomain = RT_DOMAIN_16_1;		
	}

	rt->valid		= 1;
	rt->ref_count	= 1;
	/*update reference....*/
	//printk("%s rtl865x_referNetif %s\n", __func__, netif->name);
	//rtl865x_referNetif(netif->name);
	if(ipMask == 0)
		_rtl865x_setDefaultWanNetif(netif->name);

	/**/
	if(rt->asicIdx == RT_ASIC_ENTRY_NUM-1)
	{
		retval = _rtl865x_synRouteToAsic(rt);
	}
	else
	{		
		/*insert the adding route to inused list*/
		retval = _rtl865x_addRouteToInusedList(rt);		
	}

	/*if route is add, please enable Routing for the releated netif*/
	retval = rtl865x_enableNetifRouting(netif);
	return retval;
	
addFailed:
	if(rt->asicIdx == RT_ASIC_ENTRY_NUM -1)
	{
		_rtl865x_updateDefaultRoute(rt, RT_DEFAULT_RT_NEXTHOP_CPU);
	}
	else
	{
		/*free this route entry and return error code...*/	
		memset(rt,0,sizeof(rtl865x_route_t));
		rt->next = rtl865x_route_freeHead;
		rtl865x_route_freeHead = rt;
	}
	return retval;
	
}

static int32 _rtl865x_delRoute( ipaddr_t ipAddr, ipaddr_t ipMask )
{
	rtl865x_route_t *entry;
	int32 i;
	int32 retval = 0;
	rtl865x_netif_local_t *netif = NULL;

	entry = _rtl865x_getRouteEntry(ipAddr, ipMask);

	if(entry == NULL)
		return RTL_EENTRYNOTFOUND;

	netif = _rtl865x_getSWNetifByName(entry->dstNetif);

	if(entry->asicIdx == RT_ASIC_ENTRY_NUM-1)
	{
		#ifndef Stop_keeping_defalut_route
		/*if default route
		* 1. reset default route
		* 2. reset entry->netif...
		*/
		rtl865x_netif_local_t *netif = NULL;
		#endif
		_rtl865x_clearDefaultWanNetif(entry->dstNetif);

		#ifndef Stop_keeping_defalut_route
		netif = _rtl865x_getDefaultWanNetif();
		if(netif==NULL)
		{
			return RTL_EINVNETIFNAME;
		}
		
		if(netif != entry->dstNetif)
		{
			//rtl865x_deReferNetif(entry->dstNetif->name);
			entry->dstNetif = netif;
			//printk("%s rtl865x_referNetif %s\n", __func__, netif->name);
			//rtl865x_referNetif(netif->name);
		}
		
		retval = _rtl865x_updateDefaultRoute(entry, RT_DEFAULT_RT_NEXTHOP_CPU);
		#endif
	}
	#ifndef Stop_keeping_defalut_route
	else		
	{
		/*not default route*/
	#endif
		switch(entry->process)
		{
			case RT_PPPOE:
				{
					rtl865x_ppp_t *ppp = entry->un.pppoe.pppInfo;
					if(ppp)
						rtl865x_deReferPpp(ppp->sessionId);
					retval = rtl865x_delNxtHopBySessionID(NEXTHOP_DEFREDIRECT_ACL, entry->dstNetif, ppp->sessionId);
					if( retval != SUCCESS)
					{
						printk("======error!!can't del nexthop entry for this route entry....retval\n");					
					}
				}
				break;
			case RT_L2:
				/*
				* NOTE:this type not used now...
				* if we want to use it, please DELETE FDB entry to sure this L2 entry is deleted both software FDB table and Asic L2 table.
				*/
				break;
			case RT_ARP:
				/*free arp*/
				retval = rtl865x_arp_tbl_free(entry);
				if( retval != SUCCESS)
				{
					printk("======error!!can't FREE arp entry for this route entry....retval(%d)\n",retval);					
				}
				break;				

			case RT_CPU:
			case RT_DROP:
				/*do nothing*/
				
				break;

			case RT_NEXTHOP:
				/*delete nexthop which is add by l3*/
				for ( i = entry->un.nxthop.nxtHopSta; i <= entry->un.nxthop.nxtHopEnd; i++)
				{
					retval = rtl865x_delNxtHop(NEXTHOP_L3, i);
				}				
				break;
		}

		/*FIXME_hyking: update netif reference count*/
		//rtl865x_deReferNetif(entry->dstNetif->name);

		
		/*remove from inused list...*/		
		_rtl865x_delRouteFromInusedList(entry);

		if(_rtl865x_usedNetifInRoute(entry->dstNetif) == FAILED)
		{			
			if(netif)
				rtl865x_disableNetifRouting(netif);
		}

		/*add to free list*/
		memset(entry,0,sizeof(rtl865x_route_t));
		entry->next = rtl865x_route_freeHead;
		rtl865x_route_freeHead = entry;

		retval = SUCCESS;		
	#ifndef Stop_keeping_defalut_route
	}	
	#endif
	
	//_rtl865x_route_print();
	return retval;
	
}

rtl865x_route_t* _rtl85x_getRouteEntry(ipaddr_t dst)
{
	rtl865x_route_t *tmpRtEntry = NULL;
	rtl865x_route_t *rt=rtl865x_route_inusedHead;
	uint32 mask;

	mask = 0;
	while(rt)
	{
		if (rt->valid == 1 && rt->ipAddr == (rt->ipMask & dst) && mask <= rt->ipMask) {
			mask = rt->ipMask;
			tmpRtEntry = rt;
		}
		rt = rt->next;
	}
	return tmpRtEntry;
}

/*
@func int32 | rtl865x_addRoute |add a route entry.
@parm ipaddr_t | ipAddr | ip address.
@parm ipaddr_t | ipMask | ip mask.
@parm ipaddr_t | nextHop | the route's next hop.
@parm int8* | ifName | destination network interface. 
@parm ipaddr_t | srcIp |source IP
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@rvalue RTL_EINVALIDINPUT | invalid input.
@rvalue RTL_ENOLLTYPESPECIFY | network interface's link type is not specified.
@rvalue RTL_EENTRYALREADYEXIST | route entry is already exist.
@rvalue RTL_ENOFREEBUFFER | not enough memory in system.
@comm
	if ifName=NULL, it means the destionation network interface of route entry with ip/ipMask/nextHop is default wan.
*/
int32 rtl865x_addRoute(ipaddr_t ipAddr, ipaddr_t ipMask, ipaddr_t nextHop,int8 * ifName,ipaddr_t srcIp,int needAddIntoHW)
{
	int32 retval = 0;
	unsigned long flags;	
	//printk("========%s(%d), ip(0x%x),mask(0x%x),ifname(%s),nxthop(0x%x)\n",__FUNCTION__,__LINE__,ipAddr,ipMask,ifName,nextHop);
	//rtl_down_interruptible(&route_sem);
	local_irq_save(flags);
	retval = _rtl865x_addRoute(ipAddr,ipMask,nextHop,ifName,srcIp,needAddIntoHW);		
	//rtl_up(&route_sem);
	local_irq_restore(flags);	
	//printk("========%s(%d), ip(0x%x),mask(0x%x),ifname(%s),nxthop(0x%x),retval(%d)\n",__FUNCTION__,__LINE__,ipAddr,ipMask,ifName,nextHop,retval);
	//_rtl865x_route_print();
	return retval;
}
/*
@func int32 | rtl865x_delRoute |delete a route entry.
@parm ipaddr_t | ipAddr | ipAddress.
@parm ipaddr_t | ipMask | ipMask.
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@rvalue RTL_EENTRYNOTFOUND | not found the entry.
@comm	
*/
int32 rtl865x_delRoute(ipaddr_t ipAddr, ipaddr_t ipMask)
{

	int32 retval = 0;
	unsigned long flags;	
	//printk("========%s(%d), ip(0x%x),mask(0x%x)\n",__FUNCTION__,__LINE__,ipAddr,ipMask);
	//rtl_down_interruptible(&route_sem);
	local_irq_save(flags);
	retval = _rtl865x_delRoute(ipAddr,ipMask);
	//rtl_up(&route_sem);	
	local_irq_restore(flags);
	//printk("==================================retval(%d)\n",retval);
	return retval;

}

/*
@func int32 | rtl865x_getRouteEntry |according the destination ip address, get the matched route entry.
@parm ipaddr_t | dst | destionation ip address.
@parm rtl865x_route_t* | rt | retrun value: route entry pointer
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@comm	
*/
int32 rtl865x_getRouteEntry(ipaddr_t dst,rtl865x_route_t *rt)
{
	int32 retval = FAILED;
	rtl865x_route_t *ret_entry = NULL;

	ret_entry = _rtl85x_getRouteEntry(dst);
	if(ret_entry && rt)
	{
		memcpy(rt,ret_entry,sizeof(rtl865x_route_t));		
		retval = SUCCESS;
	}
	return retval;
}
static int32 rtl865x_route_eventHandle_swnetif_change(void *para)
{
	int32 retval = EVENT_CONTINUE_EXECUTE;
	rtl865x_syncRouteToAsic();	
	return retval;
}

static int32 _rtl865x_route_register_event(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId=DEFAULT_COMMON_EVENT_LIST_ID;
	eventParam.eventId=EVENT_ADD_NETIF;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_route_eventHandle_swnetif_change;
	rtl865x_registerEvent(&eventParam);

	memset(&eventParam,0,sizeof(rtl865x_event_Param_t));	
	eventParam.eventLayerId=DEFAULT_COMMON_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_NETIF;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_route_eventHandle_swnetif_change;
	rtl865x_registerEvent(&eventParam);
	
	return SUCCESS;

}

static int32 _rtl865x_route_unRegister_event(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId=DEFAULT_COMMON_EVENT_LIST_ID;
	eventParam.eventId=EVENT_ADD_NETIF;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_route_eventHandle_swnetif_change;
	rtl865x_unRegisterEvent(&eventParam);

	memset(&eventParam,0,sizeof(rtl865x_event_Param_t));	
	eventParam.eventLayerId=DEFAULT_COMMON_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_NETIF;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_route_eventHandle_swnetif_change;
	rtl865x_unRegisterEvent(&eventParam);
	
	return SUCCESS;
}

/*
@func int32 | rtl865x_initRouteTable |initialize route tabel.
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@comm	
*/
int32 rtl865x_initRouteTable(void)
{
	int32 i;
	rtl865x_route_t *rt;
	rtl865x_route_freeHead = NULL;
	rtl865x_route_inusedHead = NULL;

	/*malloc buffer*/
	TBL_MEM_ALLOC(rt, rtl865x_route_t, RT_DRV_ENTRY_NUM);	
	memset(rt,0,sizeof(rtl865x_route_t)*RT_DRV_ENTRY_NUM);
	for(i = 0; i < RT_DRV_ENTRY_NUM; i++)
	{
		rt[i].next = rtl865x_route_freeHead;
		rtl865x_route_freeHead = &rt[i];		
	}	
	_rtl865x_route_register_event();
	return SUCCESS;	
}

/*
@func int32 | rtl865x_reinitRouteTable |reinitialize route tabel.
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@comm	
*/
int32 rtl865x_reinitRouteTable(void)
{
	rtl865x_route_t *rt;
	_rtl865x_route_unRegister_event();

	rt = rtl865x_route_inusedHead;
	while(rt && rt->asicIdx != RT_ASIC_ENTRY_NUM -1)
	{
		_rtl865x_delRoute(rt->ipAddr,rt->ipMask);
		rt = rtl865x_route_inusedHead;
	}

	/*delete the last route*/
	rt = rtl865x_route_inusedHead;
	if(rt)
	{
		/*FIXME_hyking: update netif reference count*/
		//rtl865x_deReferNetif(rt->dstNetif->name);
		
		/*remove from inused list...*/		
		_rtl865x_delRouteFromInusedList(rt);

		/*add to free list*/
		memset(rt,0,sizeof(rtl865x_route_t));
		rt->next = rtl865x_route_freeHead;
		rtl865x_route_freeHead = rt;
	}
	_rtl865x_route_register_event();
	return SUCCESS;
}

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
int rtl865x_getLanRoute(rtl865x_route_t routeTbl[], int tblSize )
{
	int cnt=0;
	rtl865x_route_t *rt = NULL;	
	rt = rtl865x_route_inusedHead;

	while(rt)
	{	
		
		if((rt->valid==1) && (rt->dstNetif->is_wan==0))
		{			
			memcpy(&routeTbl[cnt], rt, sizeof(rtl865x_route_t) );
			cnt++;
		}
		rt = rt->next;
	}	
	return cnt;
}
#endif

void rtl865x_refreshRouteArpExtIP(char* netifname,u32 extip)
{
	int sync=0;
	rtl865x_route_t *rt = rtl865x_route_inusedHead;

	while(rt)
	{
		if(rt->process == RT_ARP)
		{		
			if(!strcmp(rt->dstNetif,netifname))
			{
				uint32 extip_idx;
				if(rtl865x_getIpIdxByExtIp(extip, &extip_idx)==SUCCESS && rt->un.arp.arpIpIdx != extip_idx)
				{
					rt->un.arp.arpIpIdx = extip_idx;
					sync = 1;
				}
			}		
		}		
		rt = rt->next;
	}

	if(sync)
		_rtl865x_arrangeRoute(rtl865x_route_inusedHead, 0);
	
}

#ifdef CONFIG_RTL_MULTI_ETH_WAN
int32 rtl865x_syncRouteToAsic(void)
{
	return (_rtl865x_arrangeRoute(rtl865x_route_inusedHead, 0));
}

int rtl865x_clearAsicRoutingTable(void)
{
	rtl865x_tblAsicDrv_routingParam_t rth;
	int index;

	for (index=0; index<RTL8651_ROUTINGTBL_SIZE; index++) {
		rtl8651_getAsicRouting(index, &rth);
		if (!rth.internal)
			rtl8651_delAsicRouting(index);
	}
#if 0
	memset(&rth, 0, sizeof(rtl865x_tblAsicDrv_routingParam_t));
	rth.process = 0x4; /* CPU */
	rth.ipAddr = 0;
	rth.ipMask = 0;
	rth.vidx = 0;
	rth.internal = 0;
	rtl8651_setAsicRouting(RTL8651_ROUTINGTBL_SIZE-1, &rth);
#else
	//_rtl865x_arrangeRoute(rtl865x_route_inusedHead, 0);
#endif

	return SUCCESS;
}

#endif
rtl865x_route_t * rtl865x_getRouteList(void)
{
	return rtl865x_route_inusedHead;
}

#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER_L3)
int32 rtl_fn_hash_insert(int ifindex, char *ifname, unsigned int ipDst, unsigned int ipMask, unsigned ipGw){
	int rc = FAILED;
	unsigned int srcIp,srcMask;
	struct net_device *netif=NULL;
	extern int rtl865x_getDevIpAndNetmask(struct net_device * dev, unsigned int *ipAddr, unsigned int *netMask );

	if(!gHwNatEnabled)
		return FAILED;

	if(ifindex)
	{
		netif = __dev_get_by_index(&init_net,ifindex);
	}
	else
	{
		netif=NULL;
	}

	rtl865x_getDevIpAndNetmask(netif,&srcIp,&srcMask);
	if (!ipDst || (!MULTICAST(ipDst) && !LOOPBACK(ipDst) && (ipDst != 0xffffffff))) 
	{ 
		if ((ifname == NULL) ||
		//strcmp(ifName,ALIASNAME_BR0))
		!alias_name_is_eq(CMD_CMP,ifname,ALIASNAME_BR0))
				rc = rtl865x_addRoute(ipDst,ipMask,ipGw,ifname,srcIp,0);
			else
				rc = rtl865x_addRoute(ipDst,ipMask,ipGw,ifname,srcIp,1);
		
	}
	return rc;
}

int32 rtl_fn_hash_delete(unsigned ipDst, unsigned int ipMask){
	int rc = FAILED;
			
	if (!ipDst || (!MULTICAST(ipDst) && !LOOPBACK(ipDst) && (ipDst != 0xffffffff))) { 
		rc = rtl865x_delRoute(ipDst, ipMask);
	}
	return rc;
}

#endif

