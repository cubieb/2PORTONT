/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.
* 
* Program : network interface driver
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/

/*      @doc RTL_LAYEREDDRV_API

        @module rtl865x_netif.c - RTL865x Home gateway controller Layered driver API documentation       |
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
#include "rtl_errno.h"
#include <net/rtl/rtl_alias.h>
//#include "rtl_utils.h"
//#include "rtl_glue.h"
#include <net/rtl/rtl865x_netif.h>
#include "rtl865x_netif_local.h"
#include "rtl865x_vlan.h" /*reference vlan*/
#include "rtl865x_eventMgr.h" /*call back function....*/
#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
#include "AsicDriver/rtl865x_asicBasic.h"
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL2.h"
#include <net/rtl/rtl_nic.h>
#else
#include "AsicDriver/rtl865xC_tblAsicDrv.h"
#endif

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
//#include "rtl865x_localPublic.h"
#endif
#ifdef CONFIG_RTL_MULTI_ETH_WAN
#include <linux/if_smux.h>
#endif


#ifdef CONFIG_RTL8676_Static_ACL
#include <linux/inetdevice.h>
static int rtl865x_netif_inetaddr_event(struct notifier_block *this, unsigned long event, void *ptr);
static int rtl865x_netif_netdev_event(struct notifier_block *this, unsigned long event, void *ptr);
static struct notifier_block rtl865x_netif_inetaddr_notifier = {
	.notifier_call = rtl865x_netif_inetaddr_event,
};
static struct notifier_block rtl865x_netif_netdev_notifier = {
	.notifier_call = rtl865x_netif_netdev_event,
};
#endif

//#define DBG_NETIF
#ifdef DBG_NETIF
#define DBG_NETIF_PRK printk
#else
#define DBG_NETIF_PRK(format, args...)
#endif

#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
static struct list_head AclChainLayoutList_HEAD;
static struct list_head AclChainListPool_HEAD;


int SW_ACL_STAT_WRITEASICACL_NUM	= 0;
int SW_ACL_STAT_LAYOUTCHANGE_NUM	= 0;


#define ACL_reserved_increse_unit	1
/* each netif need 3 acl rules (PREPROCESS :2    DEFAULT : 1)  , 2 acl rules  for QOS
	125 - 3*8 - 2*8 = 85 */
#define ACL_reserved_for_user_chain	85


#define MAX(a,b)  (((a)>(b))?(a):(b))
#define MIN(a,b)  (((a)<(b))?(a):(b))
#endif

static rtl865x_netif_local_t *netifTbl;
static int Netif_Asic2Sw_Mapping[NETIF_NUMBER]={-1};
static RTL_DECLARE_MUTEX(netif_sem);
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
static rtl865x_netif_local_t virtualNetIf;
#endif



static int32 _rtl865x_delNetif(char *name);


rtl865x_netif_local_t * getNetifTbl(void)
{
	return netifTbl;
}

static int32 _rtl865x_setAsicNetif(rtl865x_netif_local_t *entry)
{
	int32 retval = FAILED;
	rtl865x_tblAsicDrv_intfParam_t asicEntry;

	if(entry->is_slave == 1)
		return retval;
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	if(entry==(&virtualNetIf))
	{
		return FAILED;
	}
#endif
	memset(&asicEntry,0,sizeof(rtl865x_tblAsicDrv_intfParam_t));
	asicEntry.enableRoute = entry->enableRoute;
	asicEntry.inAclStart = entry->inAclStart;
	asicEntry.inAclEnd = entry->inAclEnd;
	asicEntry.outAclStart = entry->outAclStart;
	asicEntry.outAclEnd = entry->outAclEnd;
	//asicEntry.macAddr = entry->macAddr;
	memcpy(asicEntry.macAddr.octet,entry->macAddr.octet,ETHER_ADDR_LEN);
	asicEntry.macAddrNumber = entry->macAddrNumber;
	asicEntry.mtu = entry->mtu;
	asicEntry.vid = entry->vid;
	asicEntry.valid = entry->valid;


	
	retval = rtl8651_setAsicNetInterface( entry->asicIdx, &asicEntry);
	return retval;

}

rtl865x_netif_local_t *_rtl865x_getSWNetifByName(char *name)
{
	int32 i;
	rtl865x_netif_local_t *netif = NULL;
	
	if(name == NULL)
		return NULL;

	/* if name is nas0*, we can not get netifTbl by name directly. */
#if 0
#ifdef CONFIG_RTL_MULTI_ETH_WAN


//if (!strncmp(name, "nas0", 4))//
if(alias_name_are_eq(name,ALIASNAME_NAS0,ALIASNAME_MWNAS))
	{
		for(i = 0; i < NETIF_SW_NUMBER; i++)
		{
			if ((netifTbl[i].valid == 1) && !strcmp(netifTbl[i].name, name) && (netifTbl[i].if_type == IF_ETHER))
			{
				if(netifTbl[i].is_slave == 0)
					netif = &netifTbl[i];
				else
				{
					netif = netifTbl[i].master;
				}
				break;
			}
		}
		return netif;
	}

#endif
#endif


	for (i = 0; i < NETIF_SW_NUMBER; i++)
	{
		//printk("%s:%d,i(%d),valid(%d),ifname(%s),strlen of name(%d), netifTbl(0x%p),netifTblName(%s)\n",__FUNCTION__,__LINE__,i,netifTbl[i].valid,name,strlen(name),&netifTbl[i],netifTbl[i].name);
		if(netifTbl[i].valid == 1 && strlen(name) == strlen(netifTbl[i].name) && memcmp(netifTbl[i].name,name,strlen(name)) == 0)
		{
			netif = &netifTbl[i];			
			break;
		}
	}
	
	#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	if(virtualNetIf.valid == 1 && strlen(name) == strlen(virtualNetIf.name) && memcmp(virtualNetIf.name,name,strlen(name)) == 0)
	{
		netif = &virtualNetIf;
		
	}
	#endif

	return netif;
}

rtl865x_netif_local_t *_rtl865x_getNetifByName(char *name)
{
	int32 i;
	rtl865x_netif_local_t *netif = NULL;
	
	if(name == NULL)
		return NULL;

	/* if name is nas0*, we can not get netifTbl by name directly. */
#if 0
#ifdef CONFIG_RTL_MULTI_ETH_WAN

//if (!strncmp(name, "nas0", 4))//
if(alias_name_are_eq(name,ALIASNAME_NAS0,ALIASNAME_MWNAS))
	{
		int vid;
//		printk("%s %d %s\n", __func__, __LINE__, name);

		if ((vid = getVidOfSmuxDev(name)) == -1) {
			printk("%s device %s not found!\n", __func__, name);
			return NULL;
		}

		for(i = 0; i < NETIF_SW_NUMBER; i++)
		{
			if ((netifTbl[i].valid == 1) && (netifTbl[i].vid == vid) && (netifTbl[i].if_type == IF_ETHER))
			{
				if(netifTbl[i].is_slave == 0)
					netif = &netifTbl[i];
				else
				{
					netif = netifTbl[i].master;
				}
				break;
			}
		}
		return netif;
	}
#endif
#endif

	for(i = 0; i < NETIF_SW_NUMBER; i++)
	{
		//printk("i(%d),ifname(%s),netifTbl(0x%p),netifTblName(%s)\n",i,name,&netifTbl[i],netifTbl[i].name);
		if(netifTbl[i].valid == 1 && strlen(name) == strlen(netifTbl[i].name) && memcmp(netifTbl[i].name,name,strlen(name)) == 0)
		{
			if(netifTbl[i].is_slave == 0)
				netif = &netifTbl[i];
			else
			{
				netif = netifTbl[i].master;
			}
			break;
		}
	}
	#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	if(virtualNetIf.valid == 1 && strlen(name) == strlen(virtualNetIf.name) && memcmp(virtualNetIf.name,name,strlen(name)) == 0)
	{
		netif = &virtualNetIf;
	
	}
	#endif
	return netif;
}

/* input : vid
    output : netif_name */
void rtl865x_getMasterNetifByVid(int vid, char* netif_name)
{
	int i;
	
	if(!netif_name)
		return;	
	
	strcpy(netif_name,"");
	
	for(i = 0; i < NETIF_SW_NUMBER; i++)
	{	
		if(netifTbl[i].valid == 1 && netifTbl[i].is_slave == 0 && netifTbl[i].vid==vid)	
		{
			strcpy(netif_name,netifTbl[i].name);
			return;		
		}
	}
	return;
}
int32 rtl865x_netifExist(char *ifName)
{
	if(_rtl865x_getSWNetifByName(ifName)!=NULL)
		return 1;
	else
		return 0;
}

/* Be careful,.... NAPT and pure routing are WAN  (If according to netif->is_wan, pure routing netif is LAN) */
int32 rtl865x_netif_is_wan(char *ifName)
{
	rtl865x_netif_local_t* netif;
	netif=_rtl865x_getNetifByName(ifName);
	//if( netif && netif->is_wan == 1)
	if(netif && strcmp(netif->name,ALIASNAME_BR0))
		return 1;
	else
		return 0;
}

int32 rtl865x_netif_is_master(char *ifName)
{
	rtl865x_netif_local_t* netif;
	netif=_rtl865x_getNetifByName(ifName);
	if( netif && netif->is_slave == 0)
		return 1;
	else
		return 0;
}
rtl865x_netif_local_t *_rtl865x_getDefaultWanNetif(void)
{
	int32 i;
	rtl865x_netif_local_t *firstWan, *defNetif;
	firstWan = defNetif = NULL;
	
	for(i = 0; i < NETIF_SW_NUMBER; i++)
	{
		//printk("i(%d),netifTbl(0x%p)\n",i,&netifTbl[i]);
		if(netifTbl[i].valid == 1 && netifTbl[i].is_wan == 1 && !netifTbl[i].is_slave && firstWan == NULL)
			firstWan = &netifTbl[i];
			
		if(netifTbl[i].valid == 1 && !netifTbl[i].is_slave && netifTbl[i].is_defaultWan == 1)
		{
			defNetif = &netifTbl[i];
			break;
		}
	}

	/*if not found default wan, return wan interface first found*/
	if(defNetif == NULL)
	{
		defNetif = firstWan;
	}
	
	DBG_NETIF_PRK("Leave %s with dev %s\n",__func__,defNetif->name);
	return defNetif;
	
}

int32 _rtl865x_setDefaultWanNetif(char *name)
{
	rtl865x_netif_local_t *entry;
	entry = _rtl865x_getSWNetifByName(name);

	//printk("set default wan interface....(%s)\n",name);
	if(entry)
		entry->is_defaultWan = 1;
	
	return SUCCESS;
}

int32 _rtl865x_clearDefaultWanNetif(char *name)
{
	rtl865x_netif_local_t *entry;
	entry = _rtl865x_getSWNetifByName(name);

	//printk("set default wan interface....(%s)\n",name);
	if(entry)
		entry->is_defaultWan = 0;
	
	return SUCCESS;
}

static int32 _rtl865x_attachMasterNetif(char *slave, char *master)
{
	rtl865x_netif_local_t *slave_netif, *master_netif;

	DBG_NETIF_PRK("Enter %s (slave : %s    master : %s) \n",__func__,slave,master);
	slave_netif = _rtl865x_getSWNetifByName(slave);
#if defined (CONFIG_RTL_8676HWNAT) && defined (CONFIG_ETHWAN)
	if (NULL == slave_netif)
	{
		extern int32 rtl_add_ppp_netif(char *ifname);
		if (SUCCESS != rtl_add_ppp_netif(slave))
		{
			DBG_NETIF_PRK("Leave %s @ %d\n",__func__,__LINE__);
			return RTL_EENTRYNOTFOUND;
		}
		//get netif again, we have created a new netif through rtl_add_ppp_netif()
		slave_netif = _rtl865x_getSWNetifByName(slave);
	}
#endif
	master_netif = _rtl865x_getNetifByName(master);

	DBG_NETIF_PRK("(%s) slave_netif : %s \n",__func__,slave_netif?(char*)slave_netif->name:"NULL");
	DBG_NETIF_PRK("(%s) master_netif : %s \n",__func__,master_netif?(char*)master_netif->name:"NULL");
	if(slave_netif == NULL || master_netif == NULL)
	{
		DBG_NETIF_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return RTL_EENTRYNOTFOUND;
	}

	//printk("===%s(%d),slave(%s),master(%s),slave_netif->master(0x%p)\n",__FUNCTION__,__LINE__,slave,master,slave_netif->master);
	if(slave_netif->master != NULL)
	{
		DBG_NETIF_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return RTL_EENTRYALREADYEXIST;
	}

	slave_netif->master = master_netif;
#ifdef CONFIG_RTL_MULTI_ETH_WAN
	slave_netif->vid = master_netif->vid;
#endif

	DBG_NETIF_PRK("Leave %s\n",__func__);
	return SUCCESS;
	
}

static int32 _rtl865x_detachMasterNetif(char *slave)
{
	rtl865x_netif_local_t *slave_netif;

	DBG_NETIF_PRK("Enter %s (slave : %s ) \n",__func__,slave);
	printk("%s %d %s\n", __func__, __LINE__, slave);
	slave_netif = _rtl865x_getSWNetifByName(slave);
	DBG_NETIF_PRK("(%s) slave_netif : %s \n",__func__,slave_netif?(char*)slave_netif->name:"NULL");
	
	if(slave_netif == NULL)
	{
		DBG_NETIF_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return RTL_EENTRYNOTFOUND;
	}

	slave_netif ->master = NULL;
	printk("detach %s success.\n", slave);
	DBG_NETIF_PRK("Leave %s\n",__func__);
	return SUCCESS;
}



int32 _rtl865x_setNetifMac(rtl865x_netif_t *netif)
{
	int32 retval = FAILED;
	rtl865x_netif_local_t *entry;

#if defined(CONFIG_RTL_LAYERED_DRIVER_ACL) && defined(CONFIG_RTL_HW_NAPT) 
	rtl865x_AclRule_t *rule;	
#endif

	DBG_NETIF_PRK("Enter %s (name:%s   addr:%02X:%02X:%02X:%02X:%02X:%02X:) \n"
		,__func__,netif->name
		,netif->macAddr.octet[0],netif->macAddr.octet[1],netif->macAddr.octet[2]
		,netif->macAddr.octet[3],netif->macAddr.octet[4],netif->macAddr.octet[5]);
		
	if(netif == NULL)
		return RTL_EINVALIDINPUT;
	entry = _rtl865x_getNetifByName(netif->name);
	
	if(entry == NULL)
		return RTL_EENTRYNOTFOUND;

	entry->macAddr = netif->macAddr;

	/*update asic table*/
	retval = _rtl865x_setAsicNetif(entry);

	/* Update the acl rule related to mac addr */
#if defined(CONFIG_RTL_LAYERED_DRIVER_ACL) && defined(CONFIG_RTL_HW_NAPT) 
	rtl865x_flush_allAcl_fromChain(netif->name,RTL865X_ACL_IPv4MUL_PPPoE);

	rule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
	if(!rule)
	{				
		printk("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
		return FAILED;
	}		
	memset(rule, 0,sizeof(rtl865x_AclRule_t));	
	rule->ruleType_ = RTL865X_ACL_DSTFILTER;
	rule->actionType_ = RTL865X_ACL_PERMIT;
	rule->pktOpApp_ = RTL865X_ACL_ALL_LAYER;
	rule->direction_ = RTL865X_ACL_INGRESS;
	memcpy(rule->un_ty.dstFilterMac_.octet,netif->macAddr.octet,ETHER_ADDR_LEN);
	rule->un_ty.dstFilterMacMask_.octet[0] = 0xff;	
	rule->un_ty.dstFilterMacMask_.octet[1] = 0xff;	
	rule->un_ty.dstFilterMacMask_.octet[2] = 0xff;
	rule->un_ty.dstFilterMacMask_.octet[3] = 0xff;	
	rule->un_ty.dstFilterMacMask_.octet[4] = 0xff;	
	rule->un_ty.dstFilterMacMask_.octet[5] = 0xff;
	rule->un_ty.dstFilterIpAddr_		= 0xE0000000;
	rule->un_ty.dstFilterIpAddrMask_= 0xF0000000;			
	rule->un_ty.dstFilterPortUpperBound_ = 0xFFFF;	
	rule->un_ty.dstFilterPortLowerBound_ = 0x0000;
	rtl865x_add_acl(rule, netif->name, RTL865X_ACL_IPv4MUL_PPPoE,1,0);
	kfree(rule);	
#endif

	
	return retval;
	
}

int32 _rtl865x_setNetifMtu(rtl865x_netif_t *netif)
{
	int32 retval = FAILED;
	rtl865x_netif_local_t *entry;
	entry = _rtl865x_getNetifByName(netif->name);

	if(entry == NULL)
		return RTL_EENTRYNOTFOUND;

	entry->mtu = netif->mtu;

	/*update asic table*/
	retval = _rtl865x_setAsicNetif(entry);
	
	return retval;
	
}



	

char* _rtl865x_getNetifNameByidx(int32 idx)
{

	if(idx<0 || idx>=NETIF_SW_NUMBER)
		return NULL;
	
	if(netifTbl[idx].valid==0)
		return NULL;
	return netifTbl[idx].name;
}
int32 _rtl865x_getAclFromAsic(int32 index, rtl865x_AclRule_t *rule)
{
	rtl865xc_tblAsic_aclTable_t    entry;

	if(index >= RTL865X_ACL_MAX_NUMBER + RTL865X_ACL_RESERVED_NUMBER || rule == NULL)
		return FAILED;
	_rtl8651_readAsicEntry(TYPE_ACL_RULE_TABLE, index, &entry);
	bzero(rule, sizeof(rtl865x_AclRule_t));

	switch(entry.ruleType) {

	case RTL865X_ACL_MAC: /* Ethernet rule type */
		 rule->un_ty.dstMac_.octet[0] 	= entry.is.ETHERNET.dMacP47_32 >> 8;
		 rule->un_ty.dstMac_.octet[1] 	= entry.is.ETHERNET.dMacP47_32 & 0xff;
		 rule->un_ty.dstMac_.octet[2] 	= entry.is.ETHERNET.dMacP31_16 >> 8;
		 rule->un_ty.dstMac_.octet[3] 	= entry.is.ETHERNET.dMacP31_16 & 0xff;
		 rule->un_ty.dstMac_.octet[4] 	= entry.is.ETHERNET.dMacP15_0 >> 8;
		 rule->un_ty.dstMac_.octet[5] 	= entry.is.ETHERNET.dMacP15_0 & 0xff;
		 rule->un_ty.dstMacMask_.octet[0] = entry.is.ETHERNET.dMacM47_32 >> 8;
		 rule->un_ty.dstMacMask_.octet[1] = entry.is.ETHERNET.dMacM47_32 & 0xff;
		 rule->un_ty.dstMacMask_.octet[2] = entry.is.ETHERNET.dMacM31_16 >> 8;
		 rule->un_ty.dstMacMask_.octet[3] = entry.is.ETHERNET.dMacM31_16 & 0xff;
		 rule->un_ty.dstMacMask_.octet[4] = entry.is.ETHERNET.dMacM15_0 >> 8;
		 rule->un_ty.dstMacMask_.octet[5] = entry.is.ETHERNET.dMacM15_0 & 0xff;
		 rule->un_ty.srcMac_.octet[0]	  = entry.is.ETHERNET.sMacP47_32 >> 8;
		 rule->un_ty.srcMac_.octet[1]	  = entry.is.ETHERNET.sMacP47_32 & 0xff;
		 rule->un_ty.srcMac_.octet[2]	  = entry.is.ETHERNET.sMacP31_16 >> 8;
		 rule->un_ty.srcMac_.octet[3]	  = entry.is.ETHERNET.sMacP31_16 & 0xff;
		 rule->un_ty.srcMac_.octet[4]	  = entry.is.ETHERNET.sMacP15_0 >> 8;
		 rule->un_ty.srcMac_.octet[5]	  = entry.is.ETHERNET.sMacP15_0 & 0xff;
		 rule->un_ty.srcMacMask_.octet[0] = entry.is.ETHERNET.sMacM47_32 >> 8;
		 rule->un_ty.srcMacMask_.octet[1] = entry.is.ETHERNET.sMacM47_32 & 0xff;
		 rule->un_ty.srcMacMask_.octet[2] = entry.is.ETHERNET.sMacM31_16 >> 8;
		 rule->un_ty.srcMacMask_.octet[3] = entry.is.ETHERNET.sMacM31_16 & 0xff;
		 rule->un_ty.srcMacMask_.octet[4] = entry.is.ETHERNET.sMacM15_0 >> 8;
		 rule->un_ty.srcMacMask_.octet[5] = entry.is.ETHERNET.sMacM15_0 & 0xff;
		 rule->un_ty.typeLen_			  = entry.is.ETHERNET.ethTypeP;
		 rule->un_ty.typeLenMask_		  = entry.is.ETHERNET.ethTypeM;
		 rule->ruleType_            = entry.ruleType;
		 break;

	case RTL865X_ACL_IP: /* IP mask rule type */
	case RTL865X_ACL_IP_RANGE: /* IP range rule type*/
		 rule->un_ty.tos_		  = entry.is.L3L4.is.IP.IPTOSP;
		 rule->un_ty.tosMask_	  = entry.is.L3L4.is.IP.IPTOSM;
		 rule->un_ty.ipProto_	  = entry.is.L3L4.is.IP.IPProtoP;
		 rule->un_ty.ipProtoMask_ = entry.is.L3L4.is.IP.IPProtoM;
		 rule->un_ty.ipFlag_	  = entry.is.L3L4.is.IP.IPFlagP;
		 rule->un_ty.ipFlagMask_  = entry.is.L3L4.is.IP.IPFlagM;
		 rule->un_ty.ipFOP_ = entry.is.L3L4.is.IP.FOP;
		 rule->un_ty.ipFOM_ = entry.is.L3L4.is.IP.FOM;
		 rule->un_ty.ipHttpFilterM_ = entry.is.L3L4.is.IP.HTTPM;
		 rule->un_ty.ipHttpFilter_	= entry.is.L3L4.is.IP.HTTPP;
		 rule->un_ty.ipIdentSrcDstIp_ = entry.is.L3L4.is.IP.identSDIPM;
		 rule->ruleType_= entry.ruleType;
		 goto l3l4_shared;
	   
	case RTL865X_ACL_ICMP: /* ICMP  (ip is mask) rule type */
	case RTL865X_ACL_ICMP_IPRANGE: /* ICMP (ip is  range) rule type */
		 rule->un_ty.tos_ = entry.is.L3L4.is.ICMP.IPTOSP;
		 rule->un_ty.tosMask_ = entry.is.L3L4.is.ICMP.IPTOSM;
		 rule->un_ty.icmpType_ = entry.is.L3L4.is.ICMP.ICMPTypeP;
		 rule->un_ty.icmpTypeMask_ = entry.is.L3L4.is.ICMP.ICMPTypeM;
		 rule->un_ty.icmpCode_ = entry.is.L3L4.is.ICMP.ICMPCodeP;
		 rule->un_ty.icmpCodeMask_ = entry.is.L3L4.is.ICMP.ICMPCodeM;
 		 rule->ruleType_ = entry.ruleType;
		 goto l3l4_shared;

	case RTL865X_ACL_IGMP: /* IGMP (ip is mask) rule type */
	case RTL865X_ACL_IGMP_IPRANGE: /* IGMP (ip is range) rule type */	
		 rule->un_ty.tos_ = entry.is.L3L4.is.IGMP.IPTOSP;
		 rule->un_ty.tosMask_ = entry.is.L3L4.is.IGMP.IPTOSM;
		 rule->un_ty.igmpType_ = entry.is.L3L4.is.IGMP.IGMPTypeP;
		 rule->un_ty.igmpTypeMask_ = entry.is.L3L4.is.IGMP.IGMPTypeM;
 		 rule->ruleType_ = entry.ruleType;
		 goto l3l4_shared;

	case RTL865X_ACL_TCP: /* TCP rule type */
	case RTL865X_ACL_TCP_IPRANGE:
		 rule->un_ty.tos_ = entry.is.L3L4.is.TCP.IPTOSP;
		 rule->un_ty.tosMask_ = entry.is.L3L4.is.TCP.IPTOSM;
		 rule->un_ty.tcpFlag_ = entry.is.L3L4.is.TCP.TCPFlagP;
		 rule->un_ty.tcpFlagMask_ = entry.is.L3L4.is.TCP.TCPFlagM;
		 rule->un_ty.tcpSrcPortUB_ = entry.is.L3L4.is.TCP.TCPSPUB;
		 rule->un_ty.tcpSrcPortLB_ = entry.is.L3L4.is.TCP.TCPSPLB;
		 rule->un_ty.tcpDstPortUB_ = entry.is.L3L4.is.TCP.TCPDPUB;
		 rule->un_ty.tcpDstPortLB_ = entry.is.L3L4.is.TCP.TCPDPLB;
	 	 rule->ruleType_ = entry.ruleType;		 
         goto l3l4_shared;

	case RTL865X_ACL_UDP: /* UDP rule type */
	case RTL865X_ACL_UDP_IPRANGE:
		 rule->un_ty.tos_ = entry.is.L3L4.is.UDP.IPTOSP;
		 rule->un_ty.tosMask_ = entry.is.L3L4.is.UDP.IPTOSM;
		 rule->un_ty.udpSrcPortUB_ = entry.is.L3L4.is.UDP.UDPSPUB;
		 rule->un_ty.udpSrcPortLB_ = entry.is.L3L4.is.UDP.UDPSPLB;
		 rule->un_ty.udpDstPortUB_ = entry.is.L3L4.is.UDP.UDPDPUB;
		 rule->un_ty.udpDstPortLB_ = entry.is.L3L4.is.UDP.UDPDPLB;
		 rule->ruleType_ = entry.ruleType;
l3l4_shared:
		rule->un_ty.srcIpAddr_ = entry.is.L3L4.sIPP;
		rule->un_ty.srcIpAddrMask_ = entry.is.L3L4.sIPM;
		rule->un_ty.dstIpAddr_ = entry.is.L3L4.dIPP;
		rule->un_ty.dstIpAddrMask_ = entry.is.L3L4.dIPM;
		break;

 	case RTL865X_ACL_SRCFILTER: /* Source Filter */
	case RTL865X_ACL_SRCFILTER_IPRANGE:	
		 rule->un_ty.srcFilterMac_.octet[0] 	= entry.is.SRC_FILTER.sMacP47_32 >> 8;
		 rule->un_ty.srcFilterMac_.octet[1] 	= entry.is.SRC_FILTER.sMacP47_32 & 0xff;
		 rule->un_ty.srcFilterMac_.octet[2] 	= entry.is.SRC_FILTER.sMacP31_16 >> 8;
		 rule->un_ty.srcFilterMac_.octet[3] 	= entry.is.SRC_FILTER.sMacP31_16 & 0xff;
		 rule->un_ty.srcFilterMac_.octet[4] 	= entry.is.SRC_FILTER.sMacP15_0 >> 8;
		 rule->un_ty.srcFilterMac_.octet[5] 	= entry.is.SRC_FILTER.sMacP15_0 & 0xff;
		 if ( entry.is.SRC_FILTER.sMacM3_0&0x8)		 
	 	{
			 rule->un_ty.srcFilterMacMask_.octet[0] = 0xff;
			 rule->un_ty.srcFilterMacMask_.octet[1] = 0xff;
			 rule->un_ty.srcFilterMacMask_.octet[2] = 0xff;
			 rule->un_ty.srcFilterMacMask_.octet[3] = 0xff;
			 rule->un_ty.srcFilterMacMask_.octet[4] = 0xff;
			 rule->un_ty.srcFilterMacMask_.octet[5] = 0xF0|entry.is.SRC_FILTER.sMacM3_0;
	 	}
		 else
	 	{
			 rule->un_ty.srcFilterMacMask_.octet[0] = 0x0;
			 rule->un_ty.srcFilterMacMask_.octet[1] = 0x0;
			 rule->un_ty.srcFilterMacMask_.octet[2] = 0x0;
			 rule->un_ty.srcFilterMacMask_.octet[3] = 0x0;
			 rule->un_ty.srcFilterMacMask_.octet[4] = 0x0;
			 rule->un_ty.srcFilterMacMask_.octet[5] = entry.is.SRC_FILTER.sMacM3_0;
	 	}

		 rule->un_ty.srcFilterPort_ = entry.is.SRC_FILTER.spaP;
		 rule->un_ty.srcFilterVlanIdx_ = entry.is.SRC_FILTER.sVidP;
		 rule->un_ty.srcFilterVlanIdxMask_ = entry.is.SRC_FILTER.sVidM;
		 if(entry.is.SRC_FILTER.protoType == 2) rule->un_ty.srcFilterIgnoreL4_ = 1;
		 else if(entry.is.SRC_FILTER.protoType == 1) rule->un_ty.srcFilterIgnoreL3L4_ = 1;
		 rule->un_ty.srcFilterIpAddr_ = entry.is.SRC_FILTER.sIPP;
		 rule->un_ty.srcFilterIpAddrMask_ = entry.is.SRC_FILTER.sIPM;
		 rule->un_ty.srcFilterPortUpperBound_ = entry.is.SRC_FILTER.SPORTUB;
		 rule->un_ty.srcFilterPortLowerBound_ = entry.is.SRC_FILTER.SPORTLB;
	 	 rule->ruleType_ = entry.ruleType;
		 break;
		 
	case RTL865X_ACL_DSTFILTER: /* Destination Filter */
	case RTL865X_ACL_DSTFILTER_IPRANGE: /* Destination Filter(IP range) */
		 rule->un_ty.dstFilterMac_.octet[0] 	= entry.is.DST_FILTER.dMacP47_32 >> 8;
		 rule->un_ty.dstFilterMac_.octet[1] 	= entry.is.DST_FILTER.dMacP47_32 & 0xff;
		 rule->un_ty.dstFilterMac_.octet[2] 	= entry.is.DST_FILTER.dMacP31_16 >> 8;
		 rule->un_ty.dstFilterMac_.octet[3] 	= entry.is.DST_FILTER.dMacP31_16 & 0xff;
		 rule->un_ty.dstFilterMac_.octet[4] 	= entry.is.DST_FILTER.dMacP15_0 >> 8;
		 rule->un_ty.dstFilterMac_.octet[5] 	= entry.is.DST_FILTER.dMacP15_0 & 0xff;
		 if ( entry.is.DST_FILTER.dMacM3_0&0x8)		 
	 	{
			 rule->un_ty.dstFilterMacMask_.octet[0] = 0xff;
			 rule->un_ty.dstFilterMacMask_.octet[1] = 0xff;
			 rule->un_ty.dstFilterMacMask_.octet[2] = 0xff;
			 rule->un_ty.dstFilterMacMask_.octet[3] = 0xff;
			 rule->un_ty.dstFilterMacMask_.octet[4] = 0xff;
			 rule->un_ty.dstFilterMacMask_.octet[5] = 0xF0|entry.is.DST_FILTER.dMacM3_0;
	 	}
		 else
	 	{
			 rule->un_ty.dstFilterMacMask_.octet[0] = 0x0;
			 rule->un_ty.dstFilterMacMask_.octet[1] = 0x0;
			 rule->un_ty.dstFilterMacMask_.octet[2] = 0x0;
			 rule->un_ty.dstFilterMacMask_.octet[3] = 0x0;
			 rule->un_ty.dstFilterMacMask_.octet[4] = 0x0;
			 rule->un_ty.dstFilterMacMask_.octet[5] = entry.is.DST_FILTER.dMacM3_0;
	 	}

		
		 rule->un_ty.dstFilterVlanIdx_ = entry.is.DST_FILTER.vidP;
		 rule->un_ty.dstFilterVlanIdxMask_ = entry.is.DST_FILTER.vidM;
		 if(entry.is.DST_FILTER.protoType == 1) rule->un_ty.dstFilterIgnoreL3L4_ = 1;
		 else if(entry.is.DST_FILTER.protoType == 2) rule->un_ty.dstFilterIgnoreL4_ = 1;
		 rule->un_ty.dstFilterIpAddr_ = entry.is.DST_FILTER.dIPP;
		 rule->un_ty.dstFilterIpAddrMask_ = entry.is.DST_FILTER.dIPM;
		 rule->un_ty.dstFilterPortUpperBound_ = entry.is.DST_FILTER.DPORTUB;
		 rule->un_ty.dstFilterPortLowerBound_ = entry.is.DST_FILTER.DPORTLB;
 	 	 rule->ruleType_ = entry.ruleType;
		 break;		
	default: return FAILED; /* Unknown rule type */

	}

	rule->aclIdx = index;

	switch(entry.actionType) {

	case RTL865X_ACL_PERMIT:
	case RTL865X_ACL_REDIRECT_ETHER:
	case RTL865X_ACL_DROP:
	case RTL865X_ACL_TOCPU:
	case RTL865X_ACL_LEGACY_DROP:
	case RTL865X_ACL_DROPCPU_LOG:
	case RTL865X_ACL_MIRROR:
	case RTL865X_ACL_REDIRECT_PPPOE:
	case RTL865X_ACL_MIRROR_KEEP_MATCH:
		rule->L2Idx_ = entry.nextHop ;
		rule->netifIdx_ =  entry.vid;
		rule->pppoeIdx_ = entry.PPPoEIndex;		 
		 break;
		 
	case RTL865X_ACL_DEFAULT_REDIRECT:
		rule->nexthopIdx_ = entry.nextHop;		
		break;
	
	case RTL865X_ACL_DROP_RATE_EXCEED_PPS:
		rule->ratelimtIdx_ = entry.nextHop;
		break;
	case RTL865X_ACL_LOG_RATE_EXCEED_PPS:
		rule->ratelimtIdx_ = entry.nextHop;
		break;
	case RTL865X_ACL_DROP_RATE_EXCEED_BPS:
		rule->ratelimtIdx_ = entry.nextHop;
		break;
	case RTL865X_ACL_LOG_RATE_EXCEED_BPS:
		rule->ratelimtIdx_ = entry.nextHop;
		break;
	case RTL865X_ACL_PRIORITY:
		rule->priority_ = entry.nextHop;
		break;
		
	}

	rule->actionType_ = entry.actionType;
	rule->pktOpApp_ = entry.pktOpApp;

	return SUCCESS;
	
}

static int32 _rtl865x_setAclToAsic(int32 startIdx, rtl865x_AclRule_t *rule)
{
	rtl865xc_tblAsic_aclTable_t entry;

	if(rule->aclIdx >= RTL865X_ACL_MAX_NUMBER + RTL865X_ACL_RESERVED_NUMBER || rule == NULL)
		return FAILED;

	memset(&entry, 0, sizeof(entry));
	switch(rule->ruleType_)
	{

	case RTL865X_ACL_MAC: /* Etnernet type rule: 0x0000 */
		 entry.is.ETHERNET.dMacP47_32 = rule->un_ty.dstMac_.octet[0]<<8 | rule->un_ty.dstMac_.octet[1];
		 entry.is.ETHERNET.dMacP31_16 = rule->un_ty.dstMac_.octet[2]<<8 | rule->un_ty.dstMac_.octet[3];
		 entry.is.ETHERNET.dMacP15_0 = rule->un_ty.dstMac_.octet[4]<<8 | rule->un_ty.dstMac_.octet[5];
		 entry.is.ETHERNET.dMacM47_32 = rule->un_ty.dstMacMask_.octet[0]<<8 | rule->un_ty.dstMacMask_.octet[1];
		 entry.is.ETHERNET.dMacM31_16 = rule->un_ty.dstMacMask_.octet[2]<<8 | rule->un_ty.dstMacMask_.octet[3];
		 entry.is.ETHERNET.dMacM15_0 = rule->un_ty.dstMacMask_.octet[4]<<8 | rule->un_ty.dstMacMask_.octet[5];
		 entry.is.ETHERNET.sMacP47_32 = rule->un_ty.srcMac_.octet[0]<<8 | rule->un_ty.srcMac_.octet[1];
		 entry.is.ETHERNET.sMacP31_16 = rule->un_ty.srcMac_.octet[2]<<8 | rule->un_ty.srcMac_.octet[3];
		 entry.is.ETHERNET.sMacP15_0 = rule->un_ty.srcMac_.octet[4]<<8 | rule->un_ty.srcMac_.octet[5];
		 entry.is.ETHERNET.sMacM47_32 = rule->un_ty.srcMacMask_.octet[0]<<8 | rule->un_ty.srcMacMask_.octet[1];
		 entry.is.ETHERNET.sMacM31_16 = rule->un_ty.srcMacMask_.octet[2]<<8 | rule->un_ty.srcMacMask_.octet[3];
		 entry.is.ETHERNET.sMacM15_0 = rule->un_ty.srcMacMask_.octet[4]<<8 | rule->un_ty.srcMacMask_.octet[5];
		 entry.is.ETHERNET.ethTypeP = rule->un_ty.typeLen_;
		 entry.is.ETHERNET.ethTypeM = rule->un_ty.typeLenMask_;

		 entry.ruleType = rule->ruleType_;
		 break;

	case RTL865X_ACL_IP: /* IP Rule Type: 0x0010 */
	case RTL865X_ACL_IP_RANGE:
		 entry.is.L3L4.is.IP.IPTOSP = rule->un_ty.tos_;
		 entry.is.L3L4.is.IP.IPTOSM = rule->un_ty.tosMask_;
		 entry.is.L3L4.is.IP.IPProtoP = rule->un_ty.ipProto_;
		 entry.is.L3L4.is.IP.IPProtoM = rule->un_ty.ipProtoMask_;
		 entry.is.L3L4.is.IP.IPFlagP = rule->un_ty.ipFlag_;
		 entry.is.L3L4.is.IP.IPFlagM = rule->un_ty.ipFlagMask_;
		 entry.is.L3L4.is.IP.FOP = rule->un_ty.ipFOP_;
		 entry.is.L3L4.is.IP.FOM = rule->un_ty.ipFOM_;
		 entry.is.L3L4.is.IP.HTTPP = entry.is.L3L4.is.IP.HTTPM = rule->un_ty.ipHttpFilter_;
		 entry.is.L3L4.is.IP.identSDIPP = entry.is.L3L4.is.IP.identSDIPM = rule->un_ty.ipIdentSrcDstIp_;
		 
		 goto l3l4_shared;
		 
	case RTL865X_ACL_ICMP:
	case RTL865X_ACL_ICMP_IPRANGE:
		 entry.is.L3L4.is.ICMP.IPTOSP = rule->un_ty.tos_;
		 entry.is.L3L4.is.ICMP.IPTOSM = rule->un_ty.tosMask_;
		 entry.is.L3L4.is.ICMP.ICMPTypeP = rule->un_ty.icmpType_;
		 entry.is.L3L4.is.ICMP.ICMPTypeM = rule->un_ty.icmpTypeMask_;
		 entry.is.L3L4.is.ICMP.ICMPCodeP = rule->un_ty.icmpCode_;
		 entry.is.L3L4.is.ICMP.ICMPCodeM = rule->un_ty.icmpCodeMask_;
 		 goto l3l4_shared;

	case RTL865X_ACL_IGMP:
	case RTL865X_ACL_IGMP_IPRANGE:
		 entry.is.L3L4.is.IGMP.IPTOSP = rule->un_ty.tos_;
		 entry.is.L3L4.is.IGMP.IPTOSM = rule->un_ty.tosMask_;
		 entry.is.L3L4.is.IGMP.IGMPTypeP = rule->un_ty.igmpType_;
		 entry.is.L3L4.is.IGMP.IGMPTypeM = rule->un_ty.igmpTypeMask_; 
  		
 		 goto l3l4_shared;

	case RTL865X_ACL_TCP:
	case RTL865X_ACL_TCP_IPRANGE:
		 entry.is.L3L4.is.TCP.IPTOSP = rule->un_ty.tos_;
		 entry.is.L3L4.is.TCP.IPTOSM = rule->un_ty.tosMask_;
		 entry.is.L3L4.is.TCP.TCPFlagP = rule->un_ty.tcpFlag_;
		 entry.is.L3L4.is.TCP.TCPFlagM = rule->un_ty.tcpFlagMask_;
		 entry.is.L3L4.is.TCP.TCPSPUB = rule->un_ty.tcpSrcPortUB_;
		 entry.is.L3L4.is.TCP.TCPSPLB = rule->un_ty.tcpSrcPortLB_;
		 entry.is.L3L4.is.TCP.TCPDPUB = rule->un_ty.tcpDstPortUB_;
		 entry.is.L3L4.is.TCP.TCPDPLB = rule->un_ty.tcpDstPortLB_;
 		 
         goto l3l4_shared;

	case RTL865X_ACL_UDP:
	case RTL865X_ACL_UDP_IPRANGE:
		 entry.is.L3L4.is.UDP.IPTOSP = rule->un_ty.tos_;
		 entry.is.L3L4.is.UDP.IPTOSM = rule->un_ty.tosMask_;
		 entry.is.L3L4.is.UDP.UDPSPUB = rule->un_ty.udpSrcPortUB_;
		 entry.is.L3L4.is.UDP.UDPSPLB = rule->un_ty.udpSrcPortLB_;
		 entry.is.L3L4.is.UDP.UDPDPUB = rule->un_ty.udpDstPortUB_;
		 entry.is.L3L4.is.UDP.UDPDPLB = rule->un_ty.udpDstPortLB_;
  		 
l3l4_shared:
		 entry.ruleType = rule->ruleType_;
		 entry.is.L3L4.sIPP = rule->un_ty.srcIpAddr_;
		 entry.is.L3L4.sIPM = rule->un_ty.srcIpAddrMask_;
		 entry.is.L3L4.dIPP = rule->un_ty.dstIpAddr_;
		 entry.is.L3L4.dIPM = rule->un_ty.dstIpAddrMask_;
		 break;

 	case RTL865X_ACL_SRCFILTER:
 	case RTL865X_ACL_SRCFILTER_IPRANGE:
		 rule->un_ty.srcFilterMac_.octet[0] = rule->un_ty.srcFilterMac_.octet[0] & rule->un_ty.srcFilterMacMask_.octet[0];
		 rule->un_ty.srcFilterMac_.octet[1] = rule->un_ty.srcFilterMac_.octet[1] & rule->un_ty.srcFilterMacMask_.octet[1];
		 rule->un_ty.srcFilterMac_.octet[2] = rule->un_ty.srcFilterMac_.octet[2] & rule->un_ty.srcFilterMacMask_.octet[2];
		 rule->un_ty.srcFilterMac_.octet[3] = rule->un_ty.srcFilterMac_.octet[3] & rule->un_ty.srcFilterMacMask_.octet[3];
		 rule->un_ty.srcFilterMac_.octet[4] = rule->un_ty.srcFilterMac_.octet[4] & rule->un_ty.srcFilterMacMask_.octet[4];
		 rule->un_ty.srcFilterMac_.octet[5] = rule->un_ty.srcFilterMac_.octet[5] & rule->un_ty.srcFilterMacMask_.octet[5];
 		 
		 entry.is.SRC_FILTER.sMacP47_32 = rule->un_ty.srcFilterMac_.octet[0]<<8 | rule->un_ty.srcFilterMac_.octet[1];
		 entry.is.SRC_FILTER.sMacP31_16 = rule->un_ty.srcFilterMac_.octet[2]<<8 | rule->un_ty.srcFilterMac_.octet[3];
		 entry.is.SRC_FILTER.sMacP15_0 = rule->un_ty.srcFilterMac_.octet[4]<<8 | rule->un_ty.srcFilterMac_.octet[5];
		 entry.is.SRC_FILTER.sMacM3_0 =rule->un_ty.srcFilterMacMask_.octet[5] &0xf;

		 rule->un_ty.srcFilterVlanId_ = rule->un_ty.srcFilterVlanId_ & rule->un_ty.srcFilterVlanIdMask_;
		 entry.is.SRC_FILTER.spaP = rule->un_ty.srcFilterPort_; 
		 entry.is.SRC_FILTER.sVidP = rule->un_ty.srcFilterVlanId_;
		 entry.is.SRC_FILTER.sVidM = rule->un_ty.srcFilterVlanIdMask_;
		 if(rule->un_ty.srcFilterIgnoreL3L4_)
		 	entry.is.SRC_FILTER.protoType = 1;
		 else if(rule->un_ty.srcFilterIgnoreL4_)
		 	entry.is.SRC_FILTER.protoType = 2;
		 else
		 	entry.is.SRC_FILTER.protoType = 0;
		 
		 entry.is.SRC_FILTER.sIPP = rule->un_ty.srcFilterIpAddr_;
		 entry.is.SRC_FILTER.sIPM = rule->un_ty.srcFilterIpAddrMask_;
		 entry.is.SRC_FILTER.SPORTUB = rule->un_ty.srcFilterPortUpperBound_;
		 entry.is.SRC_FILTER.SPORTLB = rule->un_ty.srcFilterPortLowerBound_;

		 entry.ruleType = rule->ruleType_;
		 break;
		 
	case RTL865X_ACL_DSTFILTER:
 	case RTL865X_ACL_DSTFILTER_IPRANGE:
		 entry.is.DST_FILTER.dMacP47_32 = rule->un_ty.dstFilterMac_.octet[0]<<8 | rule->un_ty.dstFilterMac_.octet[1];
		 entry.is.DST_FILTER.dMacP31_16 = rule->un_ty.dstFilterMac_.octet[2]<<8 | rule->un_ty.dstFilterMac_.octet[3];
		 entry.is.DST_FILTER.dMacP15_0 = rule->un_ty.dstFilterMac_.octet[4]<<8 | rule->un_ty.dstFilterMac_.octet[5];
		 entry.is.DST_FILTER.dMacM3_0 =  rule->un_ty.dstFilterMacMask_.octet[5]&0xf;
		 entry.is.DST_FILTER.vidP = rule->un_ty.dstFilterVlanIdx_;
		 entry.is.DST_FILTER.vidM = rule->un_ty.dstFilterVlanIdxMask_;
		 if(rule->un_ty.dstFilterIgnoreL3L4_)
		 	entry.is.DST_FILTER.protoType = 1;
		 else if(rule->un_ty.dstFilterIgnoreL4_)
		 	entry.is.DST_FILTER.protoType = 2;
		 else
		 	entry.is.DST_FILTER.protoType = 0;
		 entry.is.DST_FILTER.dIPP = rule->un_ty.dstFilterIpAddr_;
		 entry.is.DST_FILTER.dIPM = rule->un_ty.dstFilterIpAddrMask_;
		 entry.is.DST_FILTER.DPORTUB = rule->un_ty.dstFilterPortUpperBound_;
		 entry.is.DST_FILTER.DPORTLB = rule->un_ty.dstFilterPortLowerBound_;
		 
    		 entry.ruleType = rule->ruleType_;
		 break;
 
	default: return FAILED; /* Unknown rule type */
	
	}
	
	switch(rule->actionType_)
	{
	case RTL865X_ACL_PERMIT:
	case RTL865X_ACL_REDIRECT_ETHER:
	case RTL865X_ACL_DROP:
	case RTL865X_ACL_TOCPU:
	case RTL865X_ACL_LEGACY_DROP:
	case RTL865X_ACL_DROPCPU_LOG:
	case RTL865X_ACL_MIRROR:
	case RTL865X_ACL_REDIRECT_PPPOE:
	case RTL865X_ACL_MIRROR_KEEP_MATCH:
		 entry.nextHop = rule->L2Idx_;
		 entry.vid = rule->netifIdx_;
		 entry.PPPoEIndex = rule->pppoeIdx_;		 
		 break;
		 
	case RTL865X_ACL_DEFAULT_REDIRECT:
		entry.nextHop = rule->nexthopIdx_;		
		break;
	
	case RTL865X_ACL_DROP_RATE_EXCEED_PPS:
		entry.nextHop = rule->ratelimtIdx_;
		break;
	case RTL865X_ACL_LOG_RATE_EXCEED_PPS:
		entry.nextHop = rule->ratelimtIdx_;
		break;
	case RTL865X_ACL_DROP_RATE_EXCEED_BPS:
		entry.nextHop = rule->ratelimtIdx_;
		break;
	case RTL865X_ACL_LOG_RATE_EXCEED_BPS:
		entry.nextHop = rule->ratelimtIdx_;
		break;
	case RTL865X_ACL_PRIORITY:
		entry.nextHop = rule->priority_;
		break;
		
	}

	entry.actionType = rule->actionType_;
	entry.pktOpApp = rule->pktOpApp_;

	
	return _rtl8651_forceAddAsicEntry(TYPE_ACL_RULE_TABLE, startIdx, &entry);
}

/*config the reserved acl rules: default permit/drop/toCPU*/
static int32 _rtl865x_confReservedAcl(void)
{
	rtl865x_AclRule_t defAcl;

	/*default permit*/
	memset(&defAcl,0,sizeof(rtl865x_AclRule_t));
	defAcl.actionType_ = RTL865X_ACL_PERMIT;
	defAcl.aclIdx = RTL865X_ACLTBL_PERMIT_ALL;
	defAcl.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
	_rtl865x_setAclToAsic(defAcl.aclIdx,&defAcl);

	/*default drop*/
	defAcl.actionType_ = RTL865X_ACL_DROP;
	defAcl.aclIdx = RTL865X_ACLTBL_DROP_ALL;
	defAcl.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
	_rtl865x_setAclToAsic(defAcl.aclIdx, &defAcl);

	/*default to cpu*/
	defAcl.actionType_ = RTL865X_ACL_TOCPU;
	defAcl.aclIdx = RTL865X_ACLTBL_ALL_TO_CPU;
	defAcl.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
	_rtl865x_setAclToAsic(defAcl.aclIdx, &defAcl);

	/*hyking:set default permit when network interface decision miss match*/
	#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
	rtl865x_setDefACLForNetDecisionMiss(RTL865X_ACLTBL_PERMIT_ALL,RTL865X_ACLTBL_PERMIT_ALL,RTL865X_ACLTBL_PERMIT_ALL,RTL865X_ACLTBL_PERMIT_ALL);
	#endif
	
	return SUCCESS;
	
}

/*
@func int32 | rtl865x_reinit_acl |memory reinit.
@rvalue SUCCESS | Success.
@comm
	this API must be called when system boot.
*/
int32 rtl865x_reinit_acl(void)
{
	_rtl865x_confReservedAcl();
	return SUCCESS;
}

void rtl865x_showACL_DataField(rtl865x_AclRuleData_t *ruleData,int rule_type)
{
	switch(rule_type)
	{
		case RTL865X_ACL_MAC:
			printk(" rule type: Ethernet\n");
			printk("\tether type: %x   ether type mask: %x\n", ruleData->typeLen_, ruleData->typeLenMask_);
			printk("\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
					ruleData->dstMac_.octet[0], ruleData->dstMac_.octet[1], ruleData->dstMac_.octet[2],
					ruleData->dstMac_.octet[3], ruleData->dstMac_.octet[4], ruleData->dstMac_.octet[5],
					ruleData->dstMacMask_.octet[0], ruleData->dstMacMask_.octet[1], ruleData->dstMacMask_.octet[2],
					ruleData->dstMacMask_.octet[3], ruleData->dstMacMask_.octet[4], ruleData->dstMacMask_.octet[5]
					);
			
			printk( "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
					ruleData->srcMac_.octet[0], ruleData->srcMac_.octet[1], ruleData->srcMac_.octet[2],
					ruleData->srcMac_.octet[3], ruleData->srcMac_.octet[4], ruleData->srcMac_.octet[5],
					ruleData->srcMacMask_.octet[0], ruleData->srcMacMask_.octet[1], ruleData->srcMacMask_.octet[2],
					ruleData->srcMacMask_.octet[3], ruleData->srcMacMask_.octet[4], ruleData->srcMacMask_.octet[5]
				);
			break;

		case RTL865X_ACL_IP:
			printk(" rule type: IP\n");
			printk( "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (ruleData->dstIpAddr_>>24),
					((ruleData->dstIpAddr_&0x00ff0000)>>16), ((ruleData->dstIpAddr_&0x0000ff00)>>8),
					(ruleData->dstIpAddr_&0xff), (ruleData->dstIpAddrMask_>>24), ((ruleData->dstIpAddrMask_&0x00ff0000)>>16),
					((ruleData->dstIpAddrMask_&0x0000ff00)>>8), (ruleData->dstIpAddrMask_&0xff)
					);
			printk("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (ruleData->srcIpAddr_>>24),
					((ruleData->srcIpAddr_&0x00ff0000)>>16), ((ruleData->srcIpAddr_&0x0000ff00)>>8),
					(ruleData->srcIpAddr_&0xff), (ruleData->srcIpAddrMask_>>24), ((ruleData->srcIpAddrMask_&0x00ff0000)>>16),
					((ruleData->srcIpAddrMask_&0x0000ff00)>>8), (ruleData->srcIpAddrMask_&0xff)
					);
			printk("\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
					ruleData->tos_, ruleData->tosMask_, ruleData->ipProto_, ruleData->ipProtoMask_, ruleData->ipFlag_, ruleData->ipFlagMask_
				);
			
			printk("\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
					ruleData->ipFOP_, ruleData->ipFOM_, ruleData->ipHttpFilter_, ruleData->ipHttpFilterM_, ruleData->ipIdentSrcDstIp_,
					ruleData->ipIdentSrcDstIpM_
					);
			printk( "\t<DF:%x> <MF:%x>\n", ruleData->ipDF_, ruleData->ipMF_); 
				break;
				
		case RTL865X_ACL_IP_RANGE:
			printk(" rule type: IP Range\n");
			printk("\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (ruleData->dstIpAddr_>>24),
					((ruleData->dstIpAddr_&0x00ff0000)>>16), ((ruleData->dstIpAddr_&0x0000ff00)>>8),
					(ruleData->dstIpAddr_&0xff), (ruleData->dstIpAddrMask_>>24), ((ruleData->dstIpAddrMask_&0x00ff0000)>>16),
					((ruleData->dstIpAddrMask_&0x0000ff00)>>8), (ruleData->dstIpAddrMask_&0xff)
					);
			printk("\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (ruleData->srcIpAddr_>>24),
					((ruleData->srcIpAddr_&0x00ff0000)>>16), ((ruleData->srcIpAddr_&0x0000ff00)>>8),
					(ruleData->srcIpAddr_&0xff), (ruleData->srcIpAddrMask_>>24), ((ruleData->srcIpAddrMask_&0x00ff0000)>>16),
					((ruleData->srcIpAddrMask_&0x0000ff00)>>8), (ruleData->srcIpAddrMask_&0xff)
					);
			printk("\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
					ruleData->tos_, ruleData->tosMask_, ruleData->ipProto_, ruleData->ipProtoMask_, ruleData->ipFlag_, ruleData->ipFlagMask_
					);
			printk("\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
					ruleData->ipFOP_, ruleData->ipFOM_, ruleData->ipHttpFilter_, ruleData->ipHttpFilterM_, ruleData->ipIdentSrcDstIp_,
					ruleData->ipIdentSrcDstIpM_
					);
				printk("\t<DF:%x> <MF:%x>\n", ruleData->ipDF_, ruleData->ipMF_); 
				break;			
		case RTL865X_ACL_ICMP:
			printk(" rule type: ICMP\n");
			printk("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (ruleData->dstIpAddr_>>24),
					((ruleData->dstIpAddr_&0x00ff0000)>>16), ((ruleData->dstIpAddr_&0x0000ff00)>>8),
					(ruleData->dstIpAddr_&0xff), (ruleData->dstIpAddrMask_>>24), ((ruleData->dstIpAddrMask_&0x00ff0000)>>16),
					((ruleData->dstIpAddrMask_&0x0000ff00)>>8), (ruleData->dstIpAddrMask_&0xff)
					);
			printk("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (ruleData->srcIpAddr_>>24),
					((ruleData->srcIpAddr_&0x00ff0000)>>16), ((ruleData->srcIpAddr_&0x0000ff00)>>8),
					(ruleData->srcIpAddr_&0xff), (ruleData->srcIpAddrMask_>>24), ((ruleData->srcIpAddrMask_&0x00ff0000)>>16),
					((ruleData->srcIpAddrMask_&0x0000ff00)>>8), (ruleData->srcIpAddrMask_&0xff)
					);
			printk("\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
					ruleData->tos_, ruleData->tosMask_, ruleData->icmpType_, ruleData->icmpTypeMask_, 
					ruleData->icmpCode_, ruleData->icmpCodeMask_);
			break;
		case RTL865X_ACL_ICMP_IPRANGE:
			printk(" rule type: ICMP IP RANGE\n");
			printk("\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (ruleData->dstIpAddr_>>24),
					((ruleData->dstIpAddr_&0x00ff0000)>>16), ((ruleData->dstIpAddr_&0x0000ff00)>>8),
					(ruleData->dstIpAddr_&0xff), (ruleData->dstIpAddrMask_>>24), ((ruleData->dstIpAddrMask_&0x00ff0000)>>16),
					((ruleData->dstIpAddrMask_&0x0000ff00)>>8), (ruleData->dstIpAddrMask_&0xff)
					);
			printk("\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (ruleData->srcIpAddr_>>24),
					((ruleData->srcIpAddr_&0x00ff0000)>>16), ((ruleData->srcIpAddr_&0x0000ff00)>>8),
					(ruleData->srcIpAddr_&0xff), (ruleData->srcIpAddrMask_>>24), ((ruleData->srcIpAddrMask_&0x00ff0000)>>16),
					((ruleData->srcIpAddrMask_&0x0000ff00)>>8), (ruleData->srcIpAddrMask_&0xff)
					);
			printk("\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
					ruleData->tos_, ruleData->tosMask_, ruleData->icmpType_, ruleData->icmpTypeMask_, 
					ruleData->icmpCode_, ruleData->icmpCodeMask_);
			break;
		case RTL865X_ACL_IGMP:
			printk(" rule type: IGMP\n");
			printk("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (ruleData->dstIpAddr_>>24),
					((ruleData->dstIpAddr_&0x00ff0000)>>16), ((ruleData->dstIpAddr_&0x0000ff00)>>8),
					(ruleData->dstIpAddr_&0xff), (ruleData->dstIpAddrMask_>>24), ((ruleData->dstIpAddrMask_&0x00ff0000)>>16),
					((ruleData->dstIpAddrMask_&0x0000ff00)>>8), (ruleData->dstIpAddrMask_&0xff)
					);
			printk("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (ruleData->srcIpAddr_>>24),
					((ruleData->srcIpAddr_&0x00ff0000)>>16), ((ruleData->srcIpAddr_&0x0000ff00)>>8),
					(ruleData->srcIpAddr_&0xff), (ruleData->srcIpAddrMask_>>24), ((ruleData->srcIpAddrMask_&0x00ff0000)>>16),
					((ruleData->srcIpAddrMask_&0x0000ff00)>>8), (ruleData->srcIpAddrMask_&0xff)
					);
			printk("\tTos: %x   TosM: %x   type: %x   typeM: %x\n", ruleData->tos_, ruleData->tosMask_,
					ruleData->igmpType_, ruleData->igmpTypeMask_
					);
			break;


		case RTL865X_ACL_IGMP_IPRANGE:
			printk(" rule type: IGMP IP RANGE\n");
			printk("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (ruleData->dstIpAddr_>>24),
					((ruleData->dstIpAddr_&0x00ff0000)>>16), ((ruleData->dstIpAddr_&0x0000ff00)>>8),
					(ruleData->dstIpAddr_&0xff), (ruleData->dstIpAddrMask_>>24), ((ruleData->dstIpAddrMask_&0x00ff0000)>>16),
					((ruleData->dstIpAddrMask_&0x0000ff00)>>8), (ruleData->dstIpAddrMask_&0xff)
					);
			printk("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (ruleData->srcIpAddr_>>24),
					((ruleData->srcIpAddr_&0x00ff0000)>>16), ((ruleData->srcIpAddr_&0x0000ff00)>>8),
					(ruleData->srcIpAddr_&0xff), (ruleData->srcIpAddrMask_>>24), ((ruleData->srcIpAddrMask_&0x00ff0000)>>16),
					((ruleData->srcIpAddrMask_&0x0000ff00)>>8), (ruleData->srcIpAddrMask_&0xff)
					);
			printk("\tTos: %x   TosM: %x   type: %x   typeM: %x\n", ruleData->tos_, ruleData->tosMask_,
					ruleData->igmpType_, ruleData->igmpTypeMask_
					);
			break;

		case RTL865X_ACL_TCP:
			printk(" rule type: TCP\n");
			printk("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (ruleData->dstIpAddr_>>24),
					((ruleData->dstIpAddr_&0x00ff0000)>>16), ((ruleData->dstIpAddr_&0x0000ff00)>>8),
					(ruleData->dstIpAddr_&0xff), (ruleData->dstIpAddrMask_>>24), ((ruleData->dstIpAddrMask_&0x00ff0000)>>16),
					((ruleData->dstIpAddrMask_&0x0000ff00)>>8), (ruleData->dstIpAddrMask_&0xff)
					);
			printk("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (ruleData->srcIpAddr_>>24),
					((ruleData->srcIpAddr_&0x00ff0000)>>16), ((ruleData->srcIpAddr_&0x0000ff00)>>8),
					(ruleData->srcIpAddr_&0xff), (ruleData->srcIpAddrMask_>>24), ((ruleData->srcIpAddrMask_&0x00ff0000)>>16),
					((ruleData->srcIpAddrMask_&0x0000ff00)>>8), (ruleData->srcIpAddrMask_&0xff)
					);
			printk("\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
					ruleData->tos_, ruleData->tosMask_, ruleData->tcpSrcPortLB_, ruleData->tcpSrcPortUB_,
					ruleData->tcpDstPortLB_, ruleData->tcpDstPortUB_
					);
			printk("\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
					ruleData->tcpFlag_, ruleData->tcpFlagMask_, ruleData->tcpURG_, ruleData->tcpACK_,
					ruleData->tcpPSH_, ruleData->tcpRST_, ruleData->tcpSYN_, ruleData->tcpFIN_
					);
			break;
		case RTL865X_ACL_TCP_IPRANGE:
				printk(" rule type: TCP IP RANGE\n");
				printk("\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (ruleData->dstIpAddr_>>24),
					((ruleData->dstIpAddr_&0x00ff0000)>>16), ((ruleData->dstIpAddr_&0x0000ff00)>>8),
					(ruleData->dstIpAddr_&0xff), (ruleData->dstIpAddrMask_>>24), ((ruleData->dstIpAddrMask_&0x00ff0000)>>16),
					((ruleData->dstIpAddrMask_&0x0000ff00)>>8), (ruleData->dstIpAddrMask_&0xff)
					);
				printk("\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (ruleData->srcIpAddr_>>24),
					((ruleData->srcIpAddr_&0x00ff0000)>>16), ((ruleData->srcIpAddr_&0x0000ff00)>>8),
					(ruleData->srcIpAddr_&0xff), (ruleData->srcIpAddrMask_>>24), ((ruleData->srcIpAddrMask_&0x00ff0000)>>16),
					((ruleData->srcIpAddrMask_&0x0000ff00)>>8), (ruleData->srcIpAddrMask_&0xff)
					);
				printk("\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
					ruleData->tos_, ruleData->tosMask_, ruleData->tcpSrcPortLB_, ruleData->tcpSrcPortUB_,
					ruleData->tcpDstPortLB_, ruleData->tcpDstPortUB_
					);
				printk("\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
					ruleData->tcpFlag_, ruleData->tcpFlagMask_, ruleData->tcpURG_, ruleData->tcpACK_,
					ruleData->tcpPSH_, ruleData->tcpRST_, ruleData->tcpSYN_, ruleData->tcpFIN_
				);
			break;

		case RTL865X_ACL_UDP:
			printk(" rule type: UDP\n");
			printk("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (ruleData->dstIpAddr_>>24),
					((ruleData->dstIpAddr_&0x00ff0000)>>16), ((ruleData->dstIpAddr_&0x0000ff00)>>8),
					(ruleData->dstIpAddr_&0xff), (ruleData->dstIpAddrMask_>>24), ((ruleData->dstIpAddrMask_&0x00ff0000)>>16),
					((ruleData->dstIpAddrMask_&0x0000ff00)>>8), (ruleData->dstIpAddrMask_&0xff)
					);
			printk("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (ruleData->srcIpAddr_>>24),
					((ruleData->srcIpAddr_&0x00ff0000)>>16), ((ruleData->srcIpAddr_&0x0000ff00)>>8),
					(ruleData->srcIpAddr_&0xff), (ruleData->srcIpAddrMask_>>24), ((ruleData->srcIpAddrMask_&0x00ff0000)>>16),
					((ruleData->srcIpAddrMask_&0x0000ff00)>>8), (ruleData->srcIpAddrMask_&0xff)
					);
			printk("\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
					ruleData->tos_, ruleData->tosMask_, ruleData->udpSrcPortLB_, ruleData->udpSrcPortUB_,
					ruleData->udpDstPortLB_, ruleData->udpDstPortUB_
					);
			break;				
		case RTL865X_ACL_UDP_IPRANGE:
			printk(" rule type: UDP IP RANGE\n");
			printk("\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (ruleData->dstIpAddr_>>24),
					((ruleData->dstIpAddr_&0x00ff0000)>>16), ((ruleData->dstIpAddr_&0x0000ff00)>>8),
					(ruleData->dstIpAddr_&0xff), (ruleData->dstIpAddrMask_>>24), ((ruleData->dstIpAddrMask_&0x00ff0000)>>16),
					((ruleData->dstIpAddrMask_&0x0000ff00)>>8), (ruleData->dstIpAddrMask_&0xff)
					);
			printk("\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (ruleData->srcIpAddr_>>24),
					((ruleData->srcIpAddr_&0x00ff0000)>>16), ((ruleData->srcIpAddr_&0x0000ff00)>>8),
					(ruleData->srcIpAddr_&0xff), (ruleData->srcIpAddrMask_>>24), ((ruleData->srcIpAddrMask_&0x00ff0000)>>16),
					((ruleData->srcIpAddrMask_&0x0000ff00)>>8), (ruleData->srcIpAddrMask_&0xff)
					);
			printk("\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
					ruleData->tos_, ruleData->tosMask_, ruleData->udpSrcPortLB_, ruleData->udpSrcPortUB_,
					ruleData->udpDstPortLB_, ruleData->udpDstPortUB_
				);
			break;				

		
		case RTL865X_ACL_SRCFILTER:
			printk(" rule type: Source Filter\n");
			printk("\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n", 
					ruleData->srcFilterMac_.octet[0], ruleData->srcFilterMac_.octet[1], ruleData->srcFilterMac_.octet[2], 
					ruleData->srcFilterMac_.octet[3], ruleData->srcFilterMac_.octet[4], ruleData->srcFilterMac_.octet[5],
					ruleData->srcFilterMacMask_.octet[0], ruleData->srcFilterMacMask_.octet[1], ruleData->srcFilterMacMask_.octet[2],
					ruleData->srcFilterMacMask_.octet[3], ruleData->srcFilterMacMask_.octet[4], ruleData->srcFilterMacMask_.octet[5]
					);
			printk("\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
					ruleData->srcFilterVlanIdx_, ruleData->srcFilterVlanIdxMask_, ruleData->srcFilterPort_, ruleData->srcFilterPortMask_,
					(ruleData->srcFilterIgnoreL3L4_==TRUE? 2: (ruleData->srcFilterIgnoreL4_ == 1? 1: 0))
					);
			printk("\tsip: %d.%d.%d.%d   sipM: %d.%d.%d.%d\n", (ruleData->srcFilterIpAddr_>>24),
					((ruleData->srcFilterIpAddr_&0x00ff0000)>>16), ((ruleData->srcFilterIpAddr_&0x0000ff00)>>8),
					(ruleData->srcFilterIpAddr_&0xff), (ruleData->srcFilterIpAddrMask_>>24),
					((ruleData->srcFilterIpAddrMask_&0x00ff0000)>>16), ((ruleData->srcFilterIpAddrMask_&0x0000ff00)>>8),
					(ruleData->srcFilterIpAddrMask_&0xff)
					);
			printk("\tsportL: %d   sportU: %d\n", ruleData->srcFilterPortLowerBound_, ruleData->srcFilterPortUpperBound_);
			break;

		case RTL865X_ACL_SRCFILTER_IPRANGE:
			printk(" rule type: Source Filter(IP Range)\n");
			printk("\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n", 
					ruleData->srcFilterMac_.octet[0], ruleData->srcFilterMac_.octet[1], ruleData->srcFilterMac_.octet[2], 
					ruleData->srcFilterMac_.octet[3], ruleData->srcFilterMac_.octet[4], ruleData->srcFilterMac_.octet[5],
					ruleData->srcFilterMacMask_.octet[0], ruleData->srcFilterMacMask_.octet[1], ruleData->srcFilterMacMask_.octet[2],
					ruleData->srcFilterMacMask_.octet[3], ruleData->srcFilterMacMask_.octet[4], ruleData->srcFilterMacMask_.octet[5]
					);
			printk("\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
					ruleData->srcFilterVlanIdx_, ruleData->srcFilterVlanIdxMask_, ruleData->srcFilterPort_, ruleData->srcFilterPortMask_,
					(ruleData->srcFilterIgnoreL3L4_==TRUE? 2: (ruleData->srcFilterIgnoreL4_ == 1? 1: 0))
					);
			printk("\tsipU: %d.%d.%d.%d   sipL: %d.%d.%d.%d\n", (ruleData->srcFilterIpAddr_>>24),
					((ruleData->srcFilterIpAddr_&0x00ff0000)>>16), ((ruleData->srcFilterIpAddr_&0x0000ff00)>>8),
					(ruleData->srcFilterIpAddr_&0xff), (ruleData->srcFilterIpAddrMask_>>24),
					((ruleData->srcFilterIpAddrMask_&0x00ff0000)>>16), ((ruleData->srcFilterIpAddrMask_&0x0000ff00)>>8),
					(ruleData->srcFilterIpAddrMask_&0xff)
					);
			printk("\tsportL: %d   sportU: %d\n", ruleData->srcFilterPortLowerBound_, ruleData->srcFilterPortUpperBound_);
			break;

		case RTL865X_ACL_DSTFILTER:
			printk(" rule type: Destination Filter\n");
			printk("\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n", 
					ruleData->dstFilterMac_.octet[0], ruleData->dstFilterMac_.octet[1], ruleData->dstFilterMac_.octet[2], 
					ruleData->dstFilterMac_.octet[3], ruleData->dstFilterMac_.octet[4], ruleData->dstFilterMac_.octet[5],
					ruleData->dstFilterMacMask_.octet[0], ruleData->dstFilterMacMask_.octet[1], ruleData->dstFilterMacMask_.octet[2],
					ruleData->dstFilterMacMask_.octet[3], ruleData->dstFilterMacMask_.octet[4], ruleData->dstFilterMacMask_.octet[5]
					);
			printk("\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
					ruleData->dstFilterVlanIdx_, ruleData->dstFilterVlanIdxMask_, 
					(ruleData->dstFilterIgnoreL3L4_==TRUE? 2: (ruleData->dstFilterIgnoreL4_ == 1? 1: 0)), 
					ruleData->dstFilterPortLowerBound_, ruleData->dstFilterPortUpperBound_
					);
			printk("\tdip: %d.%d.%d.%d   dipM: %d.%d.%d.%d\n", (ruleData->dstFilterIpAddr_>>24),
					((ruleData->dstFilterIpAddr_&0x00ff0000)>>16), ((ruleData->dstFilterIpAddr_&0x0000ff00)>>8),
					(ruleData->dstFilterIpAddr_&0xff), (ruleData->dstFilterIpAddrMask_>>24),
					((ruleData->dstFilterIpAddrMask_&0x00ff0000)>>16), ((ruleData->dstFilterIpAddrMask_&0x0000ff00)>>8),
					(ruleData->dstFilterIpAddrMask_&0xff)
					);
			break;
		case RTL865X_ACL_DSTFILTER_IPRANGE:
			printk(" rule type: Destination Filter(IP Range)\n");
			printk("\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n", 
					ruleData->dstFilterMac_.octet[0], ruleData->dstFilterMac_.octet[1], ruleData->dstFilterMac_.octet[2], 
					ruleData->dstFilterMac_.octet[3], ruleData->dstFilterMac_.octet[4], ruleData->dstFilterMac_.octet[5],
					ruleData->dstFilterMacMask_.octet[0], ruleData->dstFilterMacMask_.octet[1], ruleData->dstFilterMacMask_.octet[2],
					ruleData->dstFilterMacMask_.octet[3], ruleData->dstFilterMacMask_.octet[4], ruleData->dstFilterMacMask_.octet[5]
					);
			printk("\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
					ruleData->dstFilterVlanIdx_, ruleData->dstFilterVlanIdxMask_, 
					(ruleData->dstFilterIgnoreL3L4_==TRUE? 2: (ruleData->dstFilterIgnoreL4_ == 1? 1: 0)), 
					ruleData->dstFilterPortLowerBound_, ruleData->dstFilterPortUpperBound_
					);
			printk("\tdipU: %d.%d.%d.%d   dipL: %d.%d.%d.%d\n", (ruleData->dstFilterIpAddr_>>24),
					((ruleData->dstFilterIpAddr_&0x00ff0000)>>16), ((ruleData->dstFilterIpAddr_&0x0000ff00)>>8),
					(ruleData->dstFilterIpAddr_&0xff), (ruleData->dstFilterIpAddrMask_>>24),
					((ruleData->dstFilterIpAddrMask_&0x00ff0000)>>16), ((ruleData->dstFilterIpAddrMask_&0x0000ff00)>>8),
					(ruleData->dstFilterIpAddrMask_&0xff)
				);
			break;
		case RTL865X_ACL_802D1P:
			printk(" rule type: 802.1P (Dump Rule, in sw only )\n");
			printk("\t802.1p: 0x%X \n", ruleData->vlanTagPri_);
			break;

			default:
			printk("ruleData->ruleType_(0x%x) Unknown ?? \n", rule_type);

	}	
}
void rtl865x_showACL(rtl865x_AclRule_t *rule)
{
	int8 *actionT[] = { "permit", "redirect to ether", "drop", "to cpu", "legacy drop", 
					"drop for log", "mirror", "redirect to pppoe", "default redirect", "mirror keep match", 
					"drop rate exceed pps", "log rate exceed pps", "drop rate exceed bps", "log rate exceed bps","priority "
					};
	printk(" [%d] rule action: %s  ", rule->aclIdx, actionT[rule->actionType_]);
	rtl865x_showACL_DataField(&rule->un_ty,rule->ruleType_);

	switch (rule->actionType_) 
	{
		case RTL865X_ACL_PERMIT:
		case RTL865X_ACL_REDIRECT_ETHER:
		case RTL865X_ACL_DROP:
		case RTL865X_ACL_TOCPU:
		case RTL865X_ACL_LEGACY_DROP:
		case RTL865X_ACL_DROPCPU_LOG:
		case RTL865X_ACL_MIRROR:
		case RTL865X_ACL_REDIRECT_PPPOE:
		case RTL865X_ACL_MIRROR_KEEP_MATCH:
			printk("\tnetifIdx: %d   pppoeIdx: %d   l2Idx:%d  ", rule->netifIdx_, rule->pppoeIdx_, rule->L2Idx_);
			break;

		case RTL865X_ACL_PRIORITY: 
			printk("\tprioirty: %d   ",  rule->priority_) ;
			break;
			
		case RTL865X_ACL_DEFAULT_REDIRECT:
			printk("\tnextHop:%d  ",  rule->nexthopIdx_);
			break;

		case RTL865X_ACL_DROP_RATE_EXCEED_PPS:
		case RTL865X_ACL_LOG_RATE_EXCEED_PPS:
		case RTL865X_ACL_DROP_RATE_EXCEED_BPS:
		case RTL865X_ACL_LOG_RATE_EXCEED_BPS:
			printk("\tratelimitIdx: %d  ",  rule->ratelimtIdx_);
			break;
		default: 
			;
		
	}
	printk("\tpktOpApp: %d ",  rule->pktOpApp_);
	printk("\tdirection: %d\n",  rule->direction_);	
}

#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
static int32 __rtl865x_setACL(rtl865x_AclRule_t *rule , int index)
{
	//DBG_ACL_PRK("Enter %s (index:%d)\n",__func__,index);

	if( rule==NULL || index<0 || index>=RTL865X_ACL_MAX_NUMBER)
	{
		DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return FAILED;
	}

	rule->aclIdx = index;

	//#ifdef DBG_ACL
	//__rtl865x_showACL(rule);
	//#endif

	if(_rtl865x_setAclToAsic(index, rule)==SUCCESS)
	{
		SW_ACL_STAT_WRITEASICACL_NUM ++;
		return SUCCESS;
	}
	else
	{
		DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return FAILED;
	}
}


static int32 __rtl865x_clearACL(int index)
{
	rtl865x_AclRule_t rule;

	//DBG_ACL_PRK("Enter %s (index:%d)\n",__func__,index);

	if(index<0 || index>=RTL865X_ACL_MAX_NUMBER )
	{
		DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return FAILED;
	}

	/* create a dummy acl rule (this acl rule will be skipped when lookup)*/
	memset(&rule, 0,sizeof(rtl865x_AclRule_t));		
	rule.ruleType_	= RTL865X_ACL_MAC;
	rule.actionType_	= RTL865X_ACL_PERMIT;		
	rule.pktOpApp_ 	= 0;

	if(_rtl865x_setAclToAsic(index, &rule)==SUCCESS)
	{
		SW_ACL_STAT_WRITEASICACL_NUM ++;
		return SUCCESS;
	}
	else
	{
		DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return FAILED;
	}
}

/* 
	FUNC 	: __rtl865x_rearrange_aclchain()	
 	USAGE 	: According to the original order , we rewrites all acl rules from new_start index continuously	 
 	
Note. 
	(1) This functions won't chage reserved_guarantee_minumun !  It only changes reserved_start ,  reserved_end
	(2) It is caller's responsibility to promise that there is no overlap with other aclchain
*/
static int32 __rtl865x_rearrange_aclchain(rtl865x_acl_chain_t *acl_chain, int new_start_idx, int new_end_idx)
{
	rtl865x_AclRule_t* 	acl_rule;
	rtl865x_AclRule_t** 	original_order_array=NULL;
	int old_idx;
	int new_idx;
	

	DBG_ACL_PRK("Enter %s (acl_chain:0x%p  new_start_idx:%d   new_end_idx:%d)\n",__func__,acl_chain,new_start_idx,new_end_idx);

	if( (new_end_idx-new_start_idx) < acl_chain->acl_rules_num )
	{
		DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return FAILED;
	}

	if( (new_end_idx-new_start_idx) < acl_chain->reserved_guarantee_minumun )
	{
		DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return FAILED;
	}

	/*  Backup the original asic idx */
	original_order_array = kmalloc(sizeof(rtl865x_AclRule_t*)*(acl_chain->reserved_end-acl_chain->reserved_start), GFP_KERNEL);
	if(original_order_array==NULL)
	{
		DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return FAILED;
	}	
	for(old_idx=acl_chain->reserved_start ; old_idx<acl_chain->reserved_end ; old_idx++)
	{
		original_order_array[old_idx-acl_chain->reserved_start] = NULL ; 
		list_for_each_entry(acl_rule,&acl_chain->AclRuleListHEAD,AclRuleList) 
		{
			if(acl_rule->aclIdx == old_idx)
			{
				original_order_array[old_idx-acl_chain->reserved_start] = acl_rule;			
				break;
			}
		}			
	}	
		

	/* According to the original order , reset its asic idx */
	new_idx = new_start_idx;
	for(old_idx=acl_chain->reserved_start ; old_idx<acl_chain->reserved_end ; old_idx++)
	{
		acl_rule = original_order_array[old_idx-acl_chain->reserved_start];
		if(acl_rule)
		{			
			__rtl865x_setACL(acl_rule,new_idx);
			new_idx ++;
		}
	}

	/* If there still exists empty acl entries in reserved room  , clear it !*/
	for( ; new_idx<new_end_idx ; new_idx++)
	{
		__rtl865x_clearACL(new_idx);
	}

	acl_chain->reserved_start 	= new_start_idx;
	acl_chain->reserved_end 	= new_end_idx;

	kfree(original_order_array);

	return SUCCESS;
}

static int __rtl865x_free_aclentry_num(void)
{	
	int need_allocated_minimun = 0;
	rtl865x_acl_chain_t *acl_chain_entry;
	list_for_each_entry(acl_chain_entry,&AclChainLayoutList_HEAD,AclChainLayoutList) 
	{
		need_allocated_minimun += MAX(acl_chain_entry->reserved_guarantee_minumun , acl_chain_entry->acl_rules_num);	
	}

	if(need_allocated_minimun>RTL865X_ACL_MAX_NUMBER)
		printk("(%s %d)Warning BUG.... (need_allocated_minimun=%d ???)\n",__func__,__LINE__,need_allocated_minimun);

	return (RTL865X_ACL_MAX_NUMBER-need_allocated_minimun);
}

static int8 rtl865x_sameAclRule(rtl865x_AclRule_t *rule1, rtl865x_AclRule_t *rule2)
{

	if (rule1->actionType_ != rule2->actionType_ ||  rule1->pktOpApp_ != rule2->pktOpApp_ )
		return FALSE;
	
	if(rtl865x_sameAclRuleDataField(&rule1->un_ty,&rule2->un_ty,rule1->ruleType_,rule2->ruleType_)==FALSE)
		return FALSE;

	return TRUE;				
}

#ifdef DBG_ACL
static void _rtl865x_aclsync_debug(void)
{
	int i,j;
	for(i = 0; i<NETIF_SW_NUMBER; i++)
	{	
		rtl865x_netif_local_t *netif = &netifTbl[i];		
		
		if(netif->valid && netif->is_slave == 0)
		{
			/* skip the netif that there exists the other netif using the same acl chain list*/
			int has_the_same_chain_netif = 0;	
			rtl865x_acl_chain_t*		acl_chain_entry;
			rtl865x_AclRule_t*		acl_rule_entry;	
			int hw_acl_index;
			
			for(j = 0; j<i; j++)
			{
				if(netifTbl[j].valid && netifTbl[j].is_slave==0 && netifTbl[j].NetifAclChainListHEAD_ptr == netif->NetifAclChainListHEAD_ptr)
				{
					has_the_same_chain_netif = 1;
					break;
				}
			}			
			if(has_the_same_chain_netif)
				continue;	

			/* If the netif refer to system reserved acl (index 125~127), skip these case */
			if(netif->inAclStart == RTL865X_ACLTBL_ALL_TO_CPU && netif->inAclEnd == RTL865X_ACLTBL_ALL_TO_CPU)
				continue;
			if(netif->inAclStart == RTL865X_ACLTBL_PERMIT_ALL && netif->inAclEnd == RTL865X_ACLTBL_PERMIT_ALL)
				continue;
			if(netif->inAclStart == RTL865X_ACLTBL_DROP_ALL && netif->inAclEnd == RTL865X_ACLTBL_DROP_ALL)
				continue;


			/* initialize */
			list_for_each_entry(acl_chain_entry, netif->NetifAclChainListHEAD_ptr,NetifAclChainList)
			{
		        list_for_each_entry(acl_rule_entry,&acl_chain_entry->AclRuleListHEAD,AclRuleList)
				{
					acl_rule_entry->__dbg_touch__ = 0;
				}		
		    }			

			for(hw_acl_index=netif->inAclStart;hw_acl_index<=netif->inAclEnd;hw_acl_index++)
			{
				rtl865x_AclRule_t asic_acl;
				_rtl865x_getAclFromAsic(hw_acl_index, &asic_acl);	
				
				if(asic_acl.pktOpApp_ != 0)
				{
					rtl865x_AclRule_t*		acl_rule_target = NULL;	
					list_for_each_entry(acl_chain_entry, netif->NetifAclChainListHEAD_ptr,NetifAclChainList)
					{
				        list_for_each_entry(acl_rule_entry,&acl_chain_entry->AclRuleListHEAD,AclRuleList)
						{
							if(acl_rule_entry->aclIdx == hw_acl_index)
							{
								acl_rule_target = acl_rule_entry;
								acl_rule_target->__dbg_touch__ = 1;
								break;	
							}							
						}
						if(acl_rule_target!=NULL)
							break;
				    }

					if(acl_rule_target==NULL)
					{
						DBG_ACL_PRK("(%s %d)===== BUG     hw_acl_index:%d  , but no acl in sw======\n",__func__,__LINE__,hw_acl_index);
						BUG();
					}

					if(rtl865x_sameAclRule(acl_rule_target,&asic_acl)!=TRUE)
					{
						DBG_ACL_PRK("(%s %d)===== BUG     hw_acl_index:%d  , but no sync with sw======\n",__func__,__LINE__,hw_acl_index);
						DBG_ACL_PRK("----in hw---\n");
						rtl865x_showACL(&asic_acl);
						DBG_ACL_PRK("----in sw---\n");
						rtl865x_showACL(acl_rule_target);
						BUG();
					}
				}
			}

			
			list_for_each_entry(acl_chain_entry, netif->NetifAclChainListHEAD_ptr,NetifAclChainList)
			{
		        list_for_each_entry(acl_rule_entry,&acl_chain_entry->AclRuleListHEAD,AclRuleList)
				{
					if(acl_rule_entry->__dbg_touch__==0)
					{
						DBG_ACL_PRK("(%s %d)===== BUG     sw'acl not in hw======\n",__func__,__LINE__);
						DBG_ACL_PRK("----in sw---\n");
						rtl865x_showACL(acl_rule_entry);
						BUG();
					}
				}		
		    }	
		}				
	}
}
#endif


static int32 rtl865x_setDefACLForAllNetif(uint8 start_ingressAclIdx, uint8 end_ingressAclIdx,uint8 start_egressAclIdx,uint8 end_egressAclIdx)
{
	rtl865x_netif_local_t *netif = NULL;
	int32 i;
	for(i = 0 ; i < NETIF_SW_NUMBER; i++)
	{
		netif = &netifTbl[i];
		if(netif->valid == 0 || netif->is_slave == 1)
			continue;
		
		netif->inAclStart = start_ingressAclIdx;
		netif->inAclEnd = end_ingressAclIdx;
		netif->outAclStart = start_egressAclIdx;
		netif->outAclEnd = end_egressAclIdx;
		_rtl865x_setAsicNetif(netif);
	}
#if defined (CONFIG_RTL_LOCAL_PUBLIC)   
	rtl865x_setDefACLForNetDecisionMiss(start_ingressAclIdx,end_ingressAclIdx,start_egressAclIdx,end_egressAclIdx);
#endif

	return SUCCESS;
}


/* 
	FUNC 	:  _rtl865x_change_aclchain_reserved_num()	
 	USAGE 	:  Change some aclchain's reserved number 

Note. 
	1. It won't change reserved_guarantee_minumun !  It only determine its new reserved_start ,  reserved_end   (end - start = new_reserved_num )
	2. It MAYBE re-plan acl layout if this target_acl_chain will overlap with the other chain
	    It leads to trap packets to CPU in a short time
	3. It is caller's responsibility to put this target acl_chain into the appropriate place in AclChainLayoutList before call this function 
*/
static int32 _rtl865x_change_aclchain_reserved_num(rtl865x_acl_chain_t *target_acl_chain, int new_reserved_num)
{
	rtl865x_acl_chain_t *acl_chain_entry;	
	int final_objective;
	int i,j;

	DBG_ACL_PRK("Enter %s (acl_chain:0x%p  new_reserved:%d)\n",__func__,target_acl_chain,new_reserved_num);

	/* check 1.  new reserved room has to be large or equal to guarantee minumun reserved num */
	if(new_reserved_num < target_acl_chain->reserved_guarantee_minumun)
	{
		DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return FAILED;
	}

	/* check 2.  new reserved room has to be large enough to load the existed acl rules */
	if(new_reserved_num < target_acl_chain->acl_rules_num)
	{
		DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return FAILED;
	}

	/* check 3.  we need not change layout if new_reserved_num is not large than old_reserved_num  */
	if(new_reserved_num == (target_acl_chain->reserved_end - target_acl_chain->reserved_start))
	{
		/* need not do anything ... */
		DBG_ACL_PRK("(%s)  reserved_num is the same.. , we need not do anything \n",__func__);
		return SUCCESS;
	}
	else if(new_reserved_num < (target_acl_chain->reserved_end - target_acl_chain->reserved_start))
	{			
		DBG_ACL_PRK("(%s)  reserved_num less than the orignal reserved num (%d)\n",__func__,target_acl_chain->reserved_end - target_acl_chain->reserved_start);

		/* we only need to compress acl rules backward in this chain and reset its reserved_end  
			( Because we want to clear acl entries not using anymore)

			ex. old reserved num = 10 , new = 6   
			
			OXXOOXXXOO
			^                       ^
			start                  end

			OOOOOXXXXX
			^              ^
			start        end
		*/
		__rtl865x_rearrange_aclchain(target_acl_chain,target_acl_chain->reserved_start,target_acl_chain->reserved_end);
		target_acl_chain->reserved_end = target_acl_chain->reserved_start+new_reserved_num;
		return SUCCESS;
	}	

	/* check 4.  If new_reserved_num is  large than old_reserved_num , check whether there exists more free acl entries ?*/
	if(  (new_reserved_num-(target_acl_chain->reserved_end - target_acl_chain->reserved_start))
		> __rtl865x_free_aclentry_num())
	{
		printk("(%s %d)warning !! There's no any empty acl entries\n",__func__,__LINE__);
		DBG_ACL_PRK("Leave %s @ %d  (free num : %d)\n",__func__,__LINE__,__rtl865x_free_aclentry_num());
		return FAILED;
	}	

	/*  Initialize all chain's new start/end index  */
	list_for_each_entry(acl_chain_entry,&AclChainLayoutList_HEAD,AclChainLayoutList) 
	{
		acl_chain_entry->_arrange_start_ 	= acl_chain_entry->reserved_start;
		acl_chain_entry->_arrange_end_ 	= acl_chain_entry->reserved_end;
	}

	/* we has to earn more "final_objective" acl entries for this object acl chain*/
	final_objective = new_reserved_num;	
	DBG_ACL_PRK("(%s) yo~ let's start!  our final objective is %d \n",__func__,final_objective);


	/* Step1. Determine this target acl chain's start/end index */
	if(target_acl_chain->AclChainLayoutList.prev == &AclChainLayoutList_HEAD)
		target_acl_chain->_arrange_start_ = 0;
	else
	{
		rtl865x_acl_chain_t *acl_chain_pre = list_entry(target_acl_chain->AclChainLayoutList.prev,rtl865x_acl_chain_t,AclChainLayoutList);
		target_acl_chain->_arrange_start_ = acl_chain_pre->reserved_end;
	}

	if(target_acl_chain->AclChainLayoutList.next == &AclChainLayoutList_HEAD)
		target_acl_chain->_arrange_end_ = MIN(RTL865X_ACL_MAX_NUMBER,target_acl_chain->_arrange_start_ + new_reserved_num);
	else
	{
		rtl865x_acl_chain_t *acl_chain_nxt = list_entry(target_acl_chain->AclChainLayoutList.next,rtl865x_acl_chain_t,AclChainLayoutList);
		target_acl_chain->_arrange_end_ = MIN(acl_chain_nxt->reserved_start,target_acl_chain->_arrange_start_ + new_reserved_num);
	}

	final_objective -= (target_acl_chain->_arrange_end_ - target_acl_chain->_arrange_start_);
	DBG_ACL_PRK("(%s) After tune object aclchain (start : %d -> %d  , end : %d -> %d)   , final objective = %d \n",__func__
		,target_acl_chain->reserved_start	,target_acl_chain->_arrange_start_
		,target_acl_chain->reserved_end	,target_acl_chain->_arrange_end_ , final_objective);


	/*  Step2.  Determine every acl chain's start/end index before this target chain  */
	if(target_acl_chain->AclChainLayoutList.prev != &AclChainLayoutList_HEAD && final_objective>0)
	{		
		list_for_each_entry_reverse(acl_chain_entry,&target_acl_chain->AclChainLayoutList,AclChainLayoutList) 
		{
			rtl865x_acl_chain_t *acl_chain_need_move_backward;
			int over_reserved;
			int need_shrink;
			int total_move_backward;

			DBG_ACL_PRK("(%s) Try acl_chain %p... (the final_objective is %d) \n",__func__,acl_chain_entry,final_objective);

			/*  Step2.1 Try to shrink this acl chain's reserved num */
			over_reserved	= (acl_chain_entry->reserved_end - acl_chain_entry->reserved_start) 
								- MAX(acl_chain_entry->reserved_guarantee_minumun, acl_chain_entry->acl_rules_num);
			need_shrink		= MIN(over_reserved,final_objective);
			acl_chain_entry->_arrange_start_ 	= acl_chain_entry->reserved_start;
			acl_chain_entry->_arrange_end_	= acl_chain_entry->reserved_end - need_shrink;
			final_objective -= need_shrink;
			DBG_ACL_PRK("(%s)      (1) Try to shrink this acl chain's reserved num  (start: %d -> %d    end: %d -> %d   final_objective:%d) \n",__func__
				,acl_chain_entry->reserved_start,acl_chain_entry->_arrange_start_
				,acl_chain_entry->reserved_end,acl_chain_entry->_arrange_end_,final_objective);
			
			/*  Step2.2  Try to move this acl chain backward */
			if(final_objective>0)
			{
				int move_backward;
				if(acl_chain_entry->AclChainLayoutList.prev == &AclChainLayoutList_HEAD)
					move_backward = MIN(acl_chain_entry->_arrange_start_ - 0 , final_objective);
				else
				{
					rtl865x_acl_chain_t *acl_chain_pre = list_entry(acl_chain_entry->AclChainLayoutList.prev,rtl865x_acl_chain_t,AclChainLayoutList);
					move_backward = MIN(acl_chain_entry->_arrange_start_ - acl_chain_pre->_arrange_end_ , final_objective);
				}		

				acl_chain_entry->_arrange_start_	-= move_backward;
				acl_chain_entry->_arrange_end_	-= move_backward;
				final_objective					-= move_backward;
				DBG_ACL_PRK("(%s)      (2) Try to move this acl chain's backward  (start: %d -> %d    end: %d -> %d   final_objective:%d) \n",__func__
				,acl_chain_entry->reserved_start,acl_chain_entry->_arrange_start_
				,acl_chain_entry->reserved_end,acl_chain_entry->_arrange_end_,final_objective);
			}

			/* Step2.3  Move  each aclchain backward between this chain and target chain*/
			total_move_backward = acl_chain_entry->reserved_end - acl_chain_entry->_arrange_end_;
			DBG_ACL_PRK("(%s)     (3) Move backward  %d  for each aclchain between this chain and target chain  \n",__func__,total_move_backward);
			list_for_each_entry(acl_chain_need_move_backward,&acl_chain_entry->AclChainLayoutList,AclChainLayoutList) 
			{			
				if(acl_chain_need_move_backward == target_acl_chain)
					break;			

				acl_chain_need_move_backward->_arrange_start_	-= total_move_backward;
				acl_chain_need_move_backward->_arrange_end_	-= total_move_backward;
				DBG_ACL_PRK("(%s)            Move  acl chain 0x%p backward  (start: %d -> %d    end: %d -> %d ) \n",__func__,acl_chain_need_move_backward,
					acl_chain_need_move_backward->_arrange_start_+total_move_backward , acl_chain_need_move_backward->_arrange_start_,
					acl_chain_need_move_backward->_arrange_end_+total_move_backward , acl_chain_need_move_backward->_arrange_end_);
			}

			/* Step2.4 Finally , we are able to enlarge target chain's reserver num */
			{
				rtl865x_acl_chain_t *acl_chain_pre 	= list_entry(target_acl_chain->AclChainLayoutList.prev,rtl865x_acl_chain_t,AclChainLayoutList);			
				#ifdef DBG_ACL
				int original_start 					= target_acl_chain->_arrange_start_;	
				#endif
				target_acl_chain->_arrange_start_  = acl_chain_pre->_arrange_end_;				
				DBG_ACL_PRK("(%s) Finally , we enlarge target chain's reserver num  (start: %d -> %d    end: %d -> %d ) \n",__func__,
					original_start , target_acl_chain->_arrange_start_,
					target_acl_chain->_arrange_end_ , target_acl_chain->_arrange_end_);			
			}			 
			
			if(final_objective<=0)
				break; //exit foreach loop

			if(acl_chain_entry->AclChainLayoutList.prev == &AclChainLayoutList_HEAD)
				break; //exit foreach loop
		}
	}


	/*  Step3.  Determine every acl chain's start/end index after this target chain  */
	if(target_acl_chain->AclChainLayoutList.next != &AclChainLayoutList_HEAD && final_objective>0)
	{		
		list_for_each_entry(acl_chain_entry,&target_acl_chain->AclChainLayoutList,AclChainLayoutList) 
		{
			rtl865x_acl_chain_t *acl_chain_need_move_forward;
			int over_reserved;
			int need_shrink;
			int total_move_forward;

			DBG_ACL_PRK("(%s) Try acl_chain %p... (the final_objective is %d) \n",__func__,acl_chain_entry,final_objective);

			/*  Step2.1 Try to shrink this acl chain's reserved num */
			over_reserved	= (acl_chain_entry->reserved_end - acl_chain_entry->reserved_start) 
								- MAX(acl_chain_entry->reserved_guarantee_minumun, acl_chain_entry->acl_rules_num);
			need_shrink		= MIN(over_reserved,final_objective);
			acl_chain_entry->_arrange_end_	= acl_chain_entry->reserved_end; 	
			acl_chain_entry->_arrange_start_	= acl_chain_entry->reserved_start+need_shrink; 			
			final_objective -= need_shrink;
			DBG_ACL_PRK("(%s)      (1) Try to shrink this acl chain's reserved num  (start: %d -> %d    end: %d -> %d   final_objective:%d) \n",__func__
				,acl_chain_entry->reserved_start,acl_chain_entry->_arrange_start_
				,acl_chain_entry->reserved_end,acl_chain_entry->_arrange_end_,final_objective);
			
			/*  Step2.2  Try to move this acl chain forward */
			if(final_objective>0)
			{
				int move_forward;
				if(acl_chain_entry->AclChainLayoutList.next == &AclChainLayoutList_HEAD)
					move_forward = MIN(RTL865X_ACL_MAX_NUMBER 		- acl_chain_entry->_arrange_end_ , final_objective);
				else
				{
					rtl865x_acl_chain_t *acl_chain_nxt = list_entry(acl_chain_entry->AclChainLayoutList.next,rtl865x_acl_chain_t,AclChainLayoutList);
					move_forward = MIN(acl_chain_nxt->_arrange_start_ 	- acl_chain_entry->_arrange_end_ , final_objective);
				}		

				acl_chain_entry->_arrange_start_	+= move_forward;
				acl_chain_entry->_arrange_end_	+= move_forward;
				final_objective					-= move_forward;
				DBG_ACL_PRK("(%s)      (2) Try to move this acl chain's forward  (start: %d -> %d    end: %d -> %d   final_objective:%d) \n",__func__
				,acl_chain_entry->reserved_start,acl_chain_entry->_arrange_start_
				,acl_chain_entry->reserved_end,acl_chain_entry->_arrange_end_,final_objective);
			}

			/* Step2.3  Move  each aclchain forward between  this chain and target chain */
			total_move_forward = acl_chain_entry->_arrange_start_ - acl_chain_entry->reserved_start;
			DBG_ACL_PRK("(%s)     (3) Move forward  %d  for each aclchain between this chain ans target chain   \n",__func__,total_move_forward);
			list_for_each_entry_reverse(acl_chain_need_move_forward,&acl_chain_entry->AclChainLayoutList,AclChainLayoutList) 
			{			
				if(acl_chain_need_move_forward == target_acl_chain)
					break;			

				acl_chain_need_move_forward->_arrange_start_	+= total_move_forward;
				acl_chain_need_move_forward->_arrange_end_	+= total_move_forward;
				DBG_ACL_PRK("(%s)            Move  acl chain 0x%p backward  (start: %d -> %d    end: %d -> %d ) \n",__func__,acl_chain_need_move_forward,
					acl_chain_need_move_forward->_arrange_start_-total_move_forward , acl_chain_need_move_forward->_arrange_start_,
					acl_chain_need_move_forward->_arrange_end_-total_move_forward , acl_chain_need_move_forward->_arrange_end_);
			}

			/* Step2.4 Finally , we are able to enlarge target chain's reserver num */
			{
				rtl865x_acl_chain_t *acl_chain_nxt 	= list_entry(target_acl_chain->AclChainLayoutList.next,rtl865x_acl_chain_t,AclChainLayoutList);			
				#ifdef DBG_ACL
				int original_end 					= target_acl_chain->_arrange_end_;	
				#endif
				target_acl_chain->_arrange_end_  = acl_chain_nxt->_arrange_start_;				
				DBG_ACL_PRK("(%s) Finally , we enlarge target chain's reserver num  (start: %d -> %d    end: %d -> %d ) \n",__func__,
					target_acl_chain->_arrange_start_ , target_acl_chain->_arrange_start_,
					original_end , target_acl_chain->_arrange_end_);			
			}			 
			
			if(final_objective<=0)
				break; //exit foreach loop

			if(acl_chain_entry->AclChainLayoutList.next == &AclChainLayoutList_HEAD)
				break; //exit foreach loop
		}
	}



	if(final_objective>0)
	{		
		DBG_ACL_PRK("Leave %s @ %d  (BUG...., we have calculate total free acl entries before...)\n",__func__,__LINE__);
		return FAILED;
	}


	/*  Step4.  Finally , we re-arrange all acl chain's layout   */	
	DBG_ACL_PRK("(%s)===========Final Layout======================== \n",__func__);
	list_for_each_entry(acl_chain_entry,&AclChainLayoutList_HEAD,AclChainLayoutList)
	{
		DBG_ACL_PRK("chain %p  (start: %d -> %d    end: %d -> %d) \n",acl_chain_entry
				,acl_chain_entry->reserved_start,acl_chain_entry->_arrange_start_
				,acl_chain_entry->reserved_end,acl_chain_entry->_arrange_end_);			
	}

	/*  	Step4.1   Set all netif's acl to TRAP at first   */
	rtl865x_setDefACLForAllNetif(RTL865X_ACLTBL_ALL_TO_CPU,RTL865X_ACLTBL_ALL_TO_CPU,RTL865X_ACLTBL_PERMIT_ALL,RTL865X_ACLTBL_PERMIT_ALL);

	/*  	Step4.2   Reset all acl chains   */
	list_for_each_entry(acl_chain_entry,&AclChainLayoutList_HEAD,AclChainLayoutList)
	{
		__rtl865x_rearrange_aclchain(acl_chain_entry,acl_chain_entry->_arrange_start_,acl_chain_entry->_arrange_end_);		
	}

	/*  	Step4.3   Reset all netif's acl range   */		
	for(i = 0; i<NETIF_SW_NUMBER; i++)
	{	
		rtl865x_netif_local_t *netif = &netifTbl[i];		
		
		if(netif->valid && netif->is_slave == 0)
		{
			/* skip the netif that there exists the other netif using the same acl chain list*/
			int has_the_same_chain_netif = 0;	
			int acl_range_start;
			int acl_range_end;	
			
			for(j = 0; j<i; j++)
			{
				if(netifTbl[j].valid && netifTbl[j].is_slave==0 && netifTbl[j].NetifAclChainListHEAD_ptr == netif->NetifAclChainListHEAD_ptr)
				{
					has_the_same_chain_netif = 1;
					break;
				}
			}			
			if(has_the_same_chain_netif)
				continue;	


			if(list_empty(netif->NetifAclChainListHEAD_ptr))
			{
				acl_range_start = acl_range_end = RTL865X_ACLTBL_ALL_TO_CPU;
			}
			else
			{
				rtl865x_acl_chain_t *first_acl_chain	= list_entry(netif->NetifAclChainListHEAD_ptr->next,rtl865x_acl_chain_t,NetifAclChainList);
				rtl865x_acl_chain_t *last_acl_chain	= list_entry(netif->NetifAclChainListHEAD_ptr->prev,rtl865x_acl_chain_t,NetifAclChainList);

				if(first_acl_chain->reserved_start == last_acl_chain->reserved_end)
					acl_range_start = acl_range_end = RTL865X_ACLTBL_ALL_TO_CPU;
				else
				{			
					acl_range_start	= first_acl_chain->reserved_start;
					acl_range_end 	= last_acl_chain->reserved_end-1;
				}
			}

			netif->inAclStart 	= acl_range_start;
			netif->inAclEnd 	= acl_range_end;
			netif->outAclStart= RTL865X_ACLTBL_PERMIT_ALL;
			netif->outAclEnd	= RTL865X_ACLTBL_PERMIT_ALL;	
			
			_rtl865x_setAsicNetif(netif);
			DBG_ACL_PRK("(%s) set netif %s 's acl range :   %d ~ %d \n",__func__,netif->name,acl_range_start,acl_range_end);			
		}				
	}

	SW_ACL_STAT_LAYOUTCHANGE_NUM ++;
	
	#ifdef DBG_ACL
	_rtl865x_aclsync_debug();
	#endif
	
	return SUCCESS;
}

/* Note. this will just compare un_ty and ruleType_*/
int8 rtl865x_sameAclRuleDataField(
	rtl865x_AclRuleData_t *rule1_data, rtl865x_AclRuleData_t *rule2_data, int rule1_datatype, int rule2_datatype)
{
	if (rule1_datatype != rule2_datatype )
		return FALSE;

	switch(rule1_datatype) {
	case RTL865X_ACL_MAC:
			 if (rule1_data->typeLen_ != rule2_data->typeLen_ || rule1_data->typeLenMask_ != rule2_data->typeLenMask_)
		 	return FALSE;
			 if (memcmp(&rule1_data->dstMac_, &rule2_data->dstMac_, sizeof(ether_addr_t)) || 
			 	 memcmp(&rule1_data->dstMacMask_, &rule2_data->dstMacMask_, sizeof(ether_addr_t)) ||
				 memcmp(&rule1_data->srcMac_, &rule2_data->srcMac_, sizeof(ether_addr_t)) ||
				 memcmp(&rule1_data->srcMacMask_, &rule2_data->srcMacMask_, sizeof(ether_addr_t)) )
			 return FALSE;
		 return TRUE;
	case RTL865X_ACL_IP:
	case RTL865X_ACL_IP_RANGE:
			 if (rule1_data->ipProto_ != rule2_data->ipProto_ || rule1_data->ipProtoMask_ != rule2_data->ipProtoMask_ ||
				rule1_data->ipFlag_ != rule2_data->ipFlag_ || rule1_data->ipFlagMask_ != rule2_data->ipFlagMask_)
			return FALSE; 
		break;
			
	case RTL865X_ACL_ICMP:
	case RTL865X_ACL_ICMP_IPRANGE:
			 if (rule1_data->icmpType_ != rule2_data->icmpType_ || rule1_data->icmpTypeMask_ != rule2_data->icmpTypeMask_ ||
				rule1_data->icmpCode_ != rule2_data->icmpCode_ || rule1_data->icmpCodeMask_ != rule2_data->icmpCodeMask_)
			return FALSE; 
		 break;

	case RTL865X_ACL_IGMP:
	case RTL865X_ACL_IGMP_IPRANGE:
			 if(rule1_data->igmpType_ != rule2_data->igmpType_ || rule1_data->igmpTypeMask_ != rule2_data->igmpTypeMask_)
		 	return FALSE; 
		 break;
	case RTL865X_ACL_TCP:
	case RTL865X_ACL_TCP_IPRANGE:
			 if(rule1_data->tcpFlag_ != rule2_data->tcpFlag_ || rule1_data->tcpFlagMask_ != rule2_data->tcpFlagMask_ ||
				rule1_data->tcpSrcPortUB_ != rule2_data->tcpSrcPortUB_ || rule1_data->tcpSrcPortLB_ != rule2_data->tcpSrcPortLB_ ||
				rule1_data->tcpDstPortUB_ != rule2_data->tcpDstPortUB_ || rule1_data->tcpDstPortLB_ != rule2_data->tcpDstPortLB_)
		 	return FALSE; 
		 break;
	case RTL865X_ACL_UDP:
	case RTL865X_ACL_UDP_IPRANGE:
			 if(rule1_data->udpSrcPortUB_ != rule2_data->udpSrcPortUB_ || rule1_data->udpSrcPortLB_ != rule2_data->udpSrcPortLB_ ||
				rule1_data->udpDstPortUB_ != rule2_data->udpDstPortUB_ || rule1_data->udpDstPortLB_ != rule2_data->udpDstPortLB_)
			return FALSE;
		 break;

	case RTL865X_ACL_SRCFILTER:
	case RTL865X_ACL_SRCFILTER_IPRANGE:
			if((rule1_data->srcFilterPort_ != rule2_data->srcFilterPort_)||
				memcmp(&rule1_data->srcFilterMac_, &rule2_data->srcFilterMac_, sizeof(ether_addr_t)) != 0||
				memcmp(&rule1_data->srcFilterMacMask_, &rule2_data->srcFilterMacMask_,sizeof(ether_addr_t)) != 0||
				(rule1_data->srcFilterVlanIdx_ != rule2_data->srcFilterVlanIdx_)||
				(rule1_data->srcFilterVlanIdxMask_ != rule2_data->srcFilterVlanIdxMask_)||
				(rule1_data->srcFilterIgnoreL3L4_ != rule2_data->srcFilterIgnoreL3L4_)||
				(rule1_data->srcFilterIgnoreL4_ != rule2_data->srcFilterIgnoreL4_))
		{
			return FALSE;
		}

			if(rule1_data->srcFilterIgnoreL4_==0 && rule1_data->srcFilterIgnoreL3L4_==0)
		{
				if((rule1_data->srcFilterPortUpperBound_ != rule2_data->srcFilterPortUpperBound_)||
				   (rule1_data->srcFilterPortLowerBound_ != rule2_data->srcFilterPortLowerBound_))
				return FALSE;
		}

			if(rule1_data->srcFilterIgnoreL3L4_==0)
		{
				if((rule1_data->srcFilterIpAddr_ != rule2_data->srcFilterIpAddr_)||
					(rule2_data->srcFilterIpAddrMask_ != rule2_data->srcFilterIpAddrMask_))
				return FALSE;
		}
		
		break;
		
	case RTL865X_ACL_DSTFILTER:
	case RTL865X_ACL_DSTFILTER_IPRANGE:
			if(	memcmp(&rule1_data->dstFilterMac_, &rule2_data->dstFilterMac_, sizeof(ether_addr_t)) != 0||
				memcmp(&rule1_data->dstFilterMacMask_, &rule2_data->dstFilterMacMask_,sizeof(ether_addr_t)) != 0||
				(rule1_data->dstFilterVlanIdx_ != rule2_data->dstFilterVlanIdx_)||
				(rule1_data->dstFilterVlanIdxMask_ != rule2_data->dstFilterVlanIdxMask_)||
				(rule1_data->dstFilterIgnoreL3L4_ != rule2_data->dstFilterIgnoreL3L4_)||
				(rule1_data->dstFilterIgnoreL4_ != rule2_data->dstFilterIgnoreL4_))
		{
			return FALSE;
		}

			if(rule1_data->dstFilterIgnoreL4_==0 && rule1_data->dstFilterIgnoreL4_==0)
		{
				if((rule1_data->dstFilterPortUpperBound_ != rule2_data->dstFilterPortUpperBound_)||
				   (rule1_data->dstFilterPortLowerBound_ != rule2_data->dstFilterPortLowerBound_))
				return FALSE;
		}

			if(rule1_data->dstFilterIgnoreL3L4_==0)
		{
				if((rule1_data->dstFilterIpAddr_ != rule2_data->dstFilterIpAddr_)||
					(rule2_data->dstFilterIpAddrMask_ != rule2_data->dstFilterIpAddrMask_))
				return FALSE;
		}
		
		break;
	case RTL865X_ACL_802D1P:
			if(rule1_data->vlanTagPri_ != rule2_data->vlanTagPri_)
			return FALSE;
		else
			return TRUE;
	default: return FALSE; /* Unknown rule type */
	
	}
	/* Compare common part */
	if (rule1_data->srcIpAddr_ != rule2_data->srcIpAddr_ || rule1_data->srcIpAddrMask_ != rule2_data->srcIpAddrMask_ ||
		rule1_data->dstIpAddr_ != rule2_data->dstIpAddr_ || rule1_data->dstIpAddrMask_ != rule2_data->dstIpAddrMask_ ||
		rule1_data->tos_ != rule2_data->tos_ || rule1_data->tosMask_ != rule2_data->tosMask_ )
		return FALSE;
	return TRUE;	
}

static int32 _rtl865x_flush_allAcl_fromChain(rtl865x_acl_chain_t*	acl_chain_flush)
{	
	rtl865x_AclRule_t*		acl_rule_entry;	
	rtl865x_AclRule_t*		acl_rule_nxt;

	DBG_ACL_PRK("Enter %s (acl_chain : %p)\n",__func__,acl_chain_flush);

	/*remove all aclrule*/
	list_for_each_entry_safe(acl_rule_entry,acl_rule_nxt,&acl_chain_flush->AclRuleListHEAD,AclRuleList)
	{		
		__rtl865x_clearACL(acl_rule_entry->aclIdx);
		list_del(&acl_rule_entry->AclRuleList);		
		kfree(acl_rule_entry);	
	}
	acl_chain_flush->acl_rules_num=0;	

	return SUCCESS;
}

static int32 _rtl865x_unRegist_aclChain(rtl865x_acl_chain_t* acl_chain_unreg)
{
	DBG_ACL_PRK("Enter %s (acl_chain : %p)\n",__func__,acl_chain_unreg);

	/*remove all aclrule*/
	if(_rtl865x_flush_allAcl_fromChain(acl_chain_unreg)!=SUCCESS)
	{
		DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return RTL_ENETIFINVALID;	
	}	

	/* remove from LIST */
	list_del(&acl_chain_unreg->NetifAclChainList);
	list_del(&acl_chain_unreg->AclChainLayoutList);
	kfree(acl_chain_unreg);

	return SUCCESS;
}

static int32 _rtl865x_change_minumun_reserved(rtl865x_acl_chain_t* acl_chain_change,int new_minumun_reserved)
{
	DBG_ACL_PRK("Enter %s (acl_chain : %p , new : %d)\n",__func__,acl_chain_change,new_minumun_reserved);

	if(new_minumun_reserved > acl_chain_change->reserved_guarantee_minumun)
	{
		if(_rtl865x_change_aclchain_reserved_num(acl_chain_change,new_minumun_reserved)!=SUCCESS)
		{
			DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
			return FAILED;	
		}	
	}

	acl_chain_change->reserved_guarantee_minumun = new_minumun_reserved;
	return SUCCESS;
}

static int32 _rtl865x_reassign_userchain_minumun_reserved(void)
{
	int wan_num=0;
	rtl865x_acl_chain_chainlist_t* 	acl_chainlist_entry;
	

	list_for_each_entry(acl_chainlist_entry,&AclChainListPool_HEAD,AclChainList)
	{
		if(acl_chainlist_entry->vlan_id != RTL_LANVLANID)
		{
			wan_num++;
		}
	}


	list_for_each_entry(acl_chainlist_entry,&AclChainListPool_HEAD,AclChainList)
	{
		rtl865x_acl_chain_t* acl_chain_entry;
		rtl865x_acl_chain_t* acl_chain_target=NULL;

		list_for_each_entry(acl_chain_entry,&acl_chainlist_entry->AclChainListHEAD,NetifAclChainList)
		{
			if(acl_chain_entry->chain_priority == RTL865X_ACL_USER_USED)
			{
				acl_chain_target = acl_chain_entry;
				break;
			}
		}

		if(acl_chain_target==NULL)
		{
			printk("(%s %d) Warning!! AclChainList 0x%p does not have user chain \n",__func__,__LINE__
				,&acl_chainlist_entry->AclChainListHEAD);
			continue;
		}

		if(acl_chainlist_entry->vlan_id == RTL_LANVLANID)
		{
			if(wan_num==0)
			{
				if(_rtl865x_change_minumun_reserved(acl_chain_target,(ACL_reserved_for_user_chain) )!=SUCCESS)
				{
					DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
					return FAILED;	
				}
			}
			else
			{
				if(_rtl865x_change_minumun_reserved(acl_chain_target,ACL_reserved_for_user_chain*2/3 )!=SUCCESS)
				{
					DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
					return FAILED;	
				}
			}
			
		}
		else
		{
			if(_rtl865x_change_minumun_reserved(acl_chain_target,(ACL_reserved_for_user_chain/3/wan_num) )!=SUCCESS)
			{
				DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
				return FAILED;	
			}
		}
	}

	return SUCCESS;
}

static int32 _rtl865x_associate_aclchainlist(rtl865x_netif_local_t* netif)
{
	rtl865x_acl_chain_chainlist_t* acl_chainlist_entry;
	rtl865x_acl_chain_chainlist_t* acl_chainlist_entry_target=NULL;

	DBG_ACL_PRK("Enter %s (netif :%s)\n",__func__,netif->name);

	list_for_each_entry(acl_chainlist_entry,&AclChainListPool_HEAD,AclChainList)
	{
		if(acl_chainlist_entry->vlan_id == netif->vid)
		{
			acl_chainlist_entry_target = acl_chainlist_entry;
			break;
		}
	}

	if(acl_chainlist_entry_target==NULL)
	{
		/* create a new one */
		acl_chainlist_entry_target = kmalloc(sizeof(rtl865x_acl_chain_chainlist_t), GFP_KERNEL);
		if(!acl_chainlist_entry_target)
		{
			DBG_ACL_PRK("Leave %s @ %d(RTL_EINVALIDINPUT)\n",__func__,__LINE__);
			return FAILED;
		}
		
		INIT_LIST_HEAD(&acl_chainlist_entry_target->AclChainListHEAD);
		acl_chainlist_entry_target->vlan_id 		= netif->vid;
		acl_chainlist_entry_target->ref_count 	= 0;		
		
		INIT_LIST_HEAD(&acl_chainlist_entry_target->AclChainList);
		list_add_tail(&acl_chainlist_entry_target->AclChainList,&AclChainListPool_HEAD);
	}

	acl_chainlist_entry_target->ref_count++;
	netif->NetifAclChainListHEAD_ptr = &acl_chainlist_entry_target->AclChainListHEAD;
	return SUCCESS;
}

static int32 _rtl865x_dissociate_aclchainlist(rtl865x_netif_local_t* netif)
{
	rtl865x_acl_chain_chainlist_t* acl_chainlist_entry;
	rtl865x_acl_chain_chainlist_t* acl_chainlist_entry_target=NULL;

	DBG_ACL_PRK("Enter %s (netif :%s)\n",__func__,netif->name);

	list_for_each_entry(acl_chainlist_entry,&AclChainListPool_HEAD,AclChainList)
	{
		if(acl_chainlist_entry->vlan_id == netif->vid)
		{
			acl_chainlist_entry_target = acl_chainlist_entry;
			acl_chainlist_entry_target->ref_count -- ;
			break;
		}
	}

	if(acl_chainlist_entry_target->ref_count == 0)
	{	
		/* unregister all chains */	
		rtl865x_acl_chain_t*		acl_chain_entry;
		rtl865x_acl_chain_t*		acl_chain_next;
		
		list_for_each_entry_safe(acl_chain_entry,acl_chain_next,&acl_chainlist_entry_target->AclChainListHEAD,NetifAclChainList)
		{		
			if(_rtl865x_unRegist_aclChain(acl_chain_entry)!=SUCCESS)
			{				
				DBG_ACL_PRK("Leave %s @ %d \n",__func__,__LINE__);
				return FAILED;
			}	
		}	

		if(!list_empty(&acl_chainlist_entry_target->AclChainListHEAD))
		{				
			DBG_ACL_PRK("Leave %s @ %d (BUG...)\n",__func__,__LINE__);
			return FAILED;
		}	

		list_del(&acl_chainlist_entry_target->AclChainList);
		kfree(acl_chainlist_entry_target);
	}
	return SUCCESS;
}




static int32 _rtl865x_add_acl(rtl865x_AclRule_t *rule, char *netifName,int32 priority
								,int must_add_tail,int allow_change_layout)
{
	rtl865x_netif_local_t*				assign_netif = NULL;	
	rtl865x_acl_chain_chainlist_t*	 	acl_chainlist_entry;
	rtl865x_acl_chain_t*		acl_chain_need_to_add_rule_ARRAY[NETIF_NUMBER];
	int						acl_chain_need_to_add_rule_NUM;	
	int						total_free_acl_rules_we_need;		
	int i;


	DBG_ACL_PRK("Enter %s (netifName:%s priority:%d must_add_tail:%d  allow_change_layout:%d) \n",__func__
		,netifName,priority,must_add_tail,allow_change_layout);
	#ifdef DBG_ACL
	rtl865x_showACL(rule);
	#endif

	/* santity check */
	if(rule == NULL)
	{
		DBG_ACL_PRK("Leave %s @ %d(RTL_EINVALIDINPUT)\n",__func__,__LINE__);
		return RTL_EINVALIDINPUT;
	}

	if(netifName!=NULL) // add this acl rule to some specific netif 
	{
		assign_netif = _rtl865x_getNetifByName(netifName);
		if(assign_netif == NULL)
		{
			DBG_ACL_PRK("Leave %s @ %d(RTL_EINVALIDINPUT)\n",__func__,__LINE__);
			return RTL_EINVALIDINPUT;
		}
	}
	else
		assign_netif = NULL; // add this acl rule to each netif 


	/*  Determine acl chain(s) that we need to add this acl rule to */
	acl_chain_need_to_add_rule_NUM	= 0;
	total_free_acl_rules_we_need	= 0;
	list_for_each_entry(acl_chainlist_entry,&AclChainListPool_HEAD,AclChainList)
	{		
		rtl865x_acl_chain_t*		acl_chain_entry;
		rtl865x_acl_chain_t*		acl_chain_target=NULL;
		rtl865x_AclRule_t*		acl_rule_entry;	
		int 						need_to_create_new_acl;		

		/*  If the user has assigned the specific netif */
		if(assign_netif!=NULL && assign_netif->NetifAclChainListHEAD_ptr != &acl_chainlist_entry->AclChainListHEAD)
			continue;

		list_for_each_entry(acl_chain_entry,&acl_chainlist_entry->AclChainListHEAD,NetifAclChainList)
		{
			if(acl_chain_entry->chain_priority == priority)
			{
				acl_chain_target = acl_chain_entry;
				break;
			}
		}

		if(acl_chain_target==NULL)
		{
			printk("(%s %d) Warning!! AclChainList 0x%p does not have chain whose priority = %d \n",__func__,__LINE__
				,&acl_chainlist_entry->AclChainListHEAD,priority);
			continue; /* skip this netif */
		}

		/* dupilicate check */	
		need_to_create_new_acl = 1;
		list_for_each_entry(acl_rule_entry,&acl_chain_target->AclRuleListHEAD,AclRuleList)
		{
			if(rtl865x_sameAclRule(acl_rule_entry, rule)==TRUE)
			{
				need_to_create_new_acl = 0;
				DBG_ACL_PRK("(%s) AclChainList 0x%p already has the same acl rule in  the chain whose priority = %d\n",__func__
					,&acl_chainlist_entry->AclChainListHEAD,priority);
				break;
			}
		}

		if(need_to_create_new_acl)
		{
			if((acl_chain_target->reserved_end - acl_chain_target->reserved_start)==acl_chain_target->acl_rules_num)
			{
				if(!allow_change_layout)
				{
					/* However,  the caller does not allow us to change aclchain layout */
					DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
					return RTL_EINVALIDINPUT;
				}

				/* this chain has no more acl buffer */
				total_free_acl_rules_we_need += ACL_reserved_increse_unit;					
			}				

			acl_chain_need_to_add_rule_ARRAY[acl_chain_need_to_add_rule_NUM] = acl_chain_target;
			acl_chain_need_to_add_rule_NUM ++;
		}
		
	}


	DBG_ACL_PRK("(%s) we need %d more free acl entries \n",__func__,total_free_acl_rules_we_need);
	if(__rtl865x_free_aclentry_num() < total_free_acl_rules_we_need)
	{
		DBG_ACL_PRK("Leave %s @ %d  (free num : %d)\n",__func__,__LINE__,__rtl865x_free_aclentry_num());
		return FAILED;
	}

	/* Start to create new ACL rules ....*/
	for(i = 0; i<acl_chain_need_to_add_rule_NUM; i++)
	{		
		rtl865x_acl_chain_t*		acl_chain_target = acl_chain_need_to_add_rule_ARRAY[i];
		rtl865x_AclRule_t*		acl_rule_new;	
		int						avalible_index;

		/* enlarge its reserved num if there is no empty acl buffer in this chain  */
		if((acl_chain_target->reserved_end - acl_chain_target->reserved_start)==acl_chain_target->acl_rules_num)
		{
			if(_rtl865x_change_aclchain_reserved_num(acl_chain_target,acl_chain_target->acl_rules_num+ACL_reserved_increse_unit))
			{
				DBG_ACL_PRK("Leave %s @ %d  (BUG...., we have calculate total free acl entries before...)\n",__func__,__LINE__);
				return FAILED;
			}				
		}

		/* find an available acl index in this chain */
		avalible_index = -1;
		if(!must_add_tail)
		{
			/* Free to find it ! */
			int candicate_index;
			for(candicate_index=acl_chain_target->reserved_start ; candicate_index<acl_chain_target->reserved_end ; candicate_index++)
			{
				int 					this_index_has_been_used=0;
				rtl865x_AclRule_t*	acl_rule_entry;	
				list_for_each_entry(acl_rule_entry,&acl_chain_target->AclRuleListHEAD,AclRuleList)
				{
					if(acl_rule_entry->aclIdx == candicate_index)
					{
						this_index_has_been_used = 1;
						break; // break list_for_each_entry
					}
				}

				if(!this_index_has_been_used)
				{
					avalible_index = candicate_index;
					break; //break for loop
				}
			}
		}
		else
		{
			if(list_empty(&acl_chain_target->AclRuleListHEAD))
			{
				avalible_index = acl_chain_target->reserved_start;
			}
			else
			{
				/* This rule has to be added after the all existing acl rules  */
				int the_max_index_in_existing_rules = -1;
				rtl865x_AclRule_t*	acl_rule_entry;	
				list_for_each_entry(acl_rule_entry,&acl_chain_target->AclRuleListHEAD,AclRuleList)
				{
					if(acl_rule_entry->aclIdx > the_max_index_in_existing_rules)
						the_max_index_in_existing_rules = acl_rule_entry->aclIdx;			
				}			

				if(the_max_index_in_existing_rules==acl_chain_target->reserved_end-1)
				{
					/* compress it    OXOOXO=>OOOOXX  */
					__rtl865x_rearrange_aclchain(acl_chain_target,acl_chain_target->reserved_start,acl_chain_target->reserved_end);
					avalible_index = acl_chain_target->reserved_start + acl_chain_target->acl_rules_num;
				}
				else
					avalible_index = the_max_index_in_existing_rules+1;	
			}
		}

		if(avalible_index==-1)
		{
			DBG_ACL_PRK("Leave %s @ %d  (BUG....)\n",__func__,__LINE__);
			return FAILED;
		}

		/* create new acl rules */
		acl_rule_new = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
		memcpy(acl_rule_new,rule,sizeof(rtl865x_AclRule_t));
		INIT_LIST_HEAD(&acl_rule_new->AclRuleList);
		list_add_tail(&acl_rule_new->AclRuleList,&acl_chain_target->AclRuleListHEAD);	
		
		__rtl865x_setACL(acl_rule_new,avalible_index);				
		acl_chain_target->acl_rules_num++;
	}
	
	return SUCCESS;	
}

static int32 _rtl865x_del_acl(rtl865x_AclRule_t *rule, char *netifName,int32 priority)
{
	rtl865x_netif_local_t*				assign_netif = NULL;		
	rtl865x_acl_chain_chainlist_t*	 	acl_chainlist_entry;	

	DBG_ACL_PRK("Enter %s (netifName:%s priority:%d ) \n",__func__,netifName,priority);
	#ifdef DBG_ACL
	rtl865x_showACL(rule);
	#endif

	/* santity check */
	if(rule == NULL)
	{
		DBG_ACL_PRK("Leave %s @ %d(RTL_EINVALIDINPUT)\n",__func__,__LINE__);
		return RTL_EINVALIDINPUT;
	}

	if(netifName!=NULL) // add this acl rule to some specific netif 
	{
		assign_netif = _rtl865x_getNetifByName(netifName);
		if(assign_netif == NULL)
		{
			DBG_ACL_PRK("Leave %s @ %d(RTL_EINVALIDINPUT)\n",__func__,__LINE__);
			return RTL_EINVALIDINPUT;
		}
	}
	else
		assign_netif = NULL; // add this acl rule to each netif 


	list_for_each_entry(acl_chainlist_entry,&AclChainListPool_HEAD,AclChainList)
	{		
		rtl865x_acl_chain_t*		acl_chain_entry;
		rtl865x_acl_chain_t*		acl_chain_target=NULL;
		rtl865x_AclRule_t*		acl_rule_entry;	
		rtl865x_AclRule_t*		acl_rule_nxt;		
		
		/*  If the user has assigned the specific netif */
		if(assign_netif!=NULL && assign_netif->NetifAclChainListHEAD_ptr != &acl_chainlist_entry->AclChainListHEAD)
			continue;

		list_for_each_entry(acl_chain_entry,&acl_chainlist_entry->AclChainListHEAD,NetifAclChainList)
		{
			if(acl_chain_entry->chain_priority == priority)
			{
				acl_chain_target = acl_chain_entry;
				break;
			}
		}

		if(acl_chain_target==NULL)
		{
			printk("(%s %d) Warning!! chain 0x%p does not have chain whose priority = %d \n",__func__,__LINE__
				,&acl_chainlist_entry->AclChainListHEAD,priority);
			continue; /* skip this netif */
		}
			
		list_for_each_entry_safe(acl_rule_entry,acl_rule_nxt,&acl_chain_target->AclRuleListHEAD,AclRuleList)
		{
			if(rtl865x_sameAclRule(acl_rule_entry, rule)==TRUE)
			{				
				__rtl865x_clearACL(acl_rule_entry->aclIdx);
				list_del(&acl_rule_entry->AclRuleList);					
				kfree(acl_rule_entry);
				acl_chain_target->acl_rules_num--;					
				break;
			}
		}		
	}

	return SUCCESS;	
}

/*
@func int32 | rtl865x_add_acl |add an ACL Rule to acl chain.
@parm rtl865x_AclRule_t* | rule | realtek ACL rule
@parm char* | netifName | network interface Name.
@parm int32 | priority | priority of this acl chain.
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDINPUT | ACL rule is NULL or netifName is NULL
@rvalue RTL_EENTRYNOTFOUND | acl chain with priority is not found
@rvalue FAILED | Failed
@comm
	ACL rule structure: please refer in header file.
*/

/* Note. This rule will be mem-coped to acl chain list , so the caller could free acl rule after passing it in this func.

	allow_change_layout : 	If yes (non-zero value), when aclchain's reserved acl buffer is not enough, it will allocate more free acl entries
						However, this action maybe lead to CPU overloading because some pkts are traped to CPU when acl layout changes in a short time

						If no (zero), it returns  FAILED when aclchain's reserved acl buffer is not enough for adding new new acl rules


	must_add_tail:			Input no (zero) when you does not care the acl rules's order
						This will decrese the number of writing ACL asic table
*/

int32 rtl865x_add_acl(rtl865x_AclRule_t *rule, char *netifName,int32 priority
								,int must_add_tail,int allow_change_layout)
{	
	int32 retval = FAILED;
	unsigned long flags;	
	//printk("********%s(%d)*********,netif(%s),priority(%d)\n",__FUNCTION__,__LINE__,netifName,priority);

	//rtl_down_interruptible(&netif_sem);
	local_irq_save(flags);
	retval = _rtl865x_add_acl(rule,netifName,priority,must_add_tail,allow_change_layout);	
	//rtl_up(&netif_sem);
	local_irq_restore(flags);
	//printk("********%s(%d)*********retval(%d)\n",__FUNCTION__,__LINE__,retval);
	return retval;	
}

/*
@func int32 | rtl865x_del_acl |del an ACL Rule from the acl chain.
@parm rtl865x_AclRule_t* | rule | realtek ACL rule
@parm char* | netifName | network interface Name.
@parm int32 | priority | priority of this acl chain.
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDINPUT | ACL rule is NULL or netifName is NULL
@rvalue RTL_EENTRYNOTFOUND | acl chain with priority is not found
@rvalue FAILED | Failed
@comm
	ACL rule structure: please refer in header file.
*/
int32 rtl865x_del_acl(rtl865x_AclRule_t *rule, char *netifName,int32 priority)
{	
	int32 retval = FAILED;
	unsigned long flags;	
	//rtl_down_interruptible(&netif_sem);
	local_irq_save(flags);
	retval = _rtl865x_del_acl(rule,netifName,priority);	
	//rtl_up(&netif_sem);
	local_irq_restore(flags);
	return retval;		
}
int32 rtl865x_regist_aclChain(char *netifName, int32 priority, uint32 minumun_reserved_acl_num)
{
	rtl865x_netif_local_t*	netif;
	rtl865x_acl_chain_t*		acl_chain_entry;
	rtl865x_acl_chain_t*		acl_chain_new;

	int 						insert_finish;
	rtl865x_acl_chain_t*		pre_acl_chain_in_the_netif;
	rtl865x_acl_chain_t*		nxt_acl_chain_in_the_netif;


	DBG_ACL_PRK("Enter %s (netifName:%s  priority:%d   minumun_reserved_acl_num:%d)\n",__func__
		,netifName,priority,minumun_reserved_acl_num);

	#ifdef DBG_ACL
	printk("=======Before register===========\n");
	rtl865x_show_allAclChains();
	printk("============================\n");
	#endif

	
	netif = _rtl865x_getSWNetifByName(netifName);
	

	/* santity check */	
	if(netif == NULL)
	{
		DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return RTL_ENETIFINVALID;	
	}
	
	if(netif->NetifAclChainListHEAD_ptr == NULL)
	{
		DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return RTL_ENETIFINVALID;	
	}

	/* duplicate check */
	list_for_each_entry(acl_chain_entry,netif->NetifAclChainListHEAD_ptr,NetifAclChainList)
	{
		if(acl_chain_entry->chain_priority == priority)
		{
			DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
			return RTL_EENTRYALREADYEXIST;	
		}
	}	

	acl_chain_new = kmalloc(sizeof(rtl865x_acl_chain_t), GFP_KERNEL);
	if(acl_chain_new==NULL)
	{
		DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return RTL_ENETIFINVALID;	
	}	

	memset(acl_chain_new,0,sizeof(rtl865x_acl_chain_t));
	acl_chain_new->chain_priority					= priority;
	acl_chain_new->reserved_guarantee_minumun	= minumun_reserved_acl_num;
	acl_chain_new->acl_rules_num					= 0;
	INIT_LIST_HEAD(&acl_chain_new->AclRuleListHEAD);


	/* insert it into netif's acl chain list */
	insert_finish = 0;
	INIT_LIST_HEAD(&acl_chain_new->NetifAclChainList);
	list_for_each_entry(acl_chain_entry, netif->NetifAclChainListHEAD_ptr,NetifAclChainList)
	{
        if (priority < acl_chain_entry->chain_priority ) 
        {
        	list_add_tail(&acl_chain_new->NetifAclChainList, &acl_chain_entry->NetifAclChainList);
			insert_finish = 1;
			break;
        }		
    }	
	if(insert_finish==0)
		list_add_tail(&acl_chain_new->NetifAclChainList, netif->NetifAclChainListHEAD_ptr);


	if(acl_chain_new->NetifAclChainList.prev == netif->NetifAclChainListHEAD_ptr)
		pre_acl_chain_in_the_netif = NULL;
	else
		pre_acl_chain_in_the_netif = list_entry(acl_chain_new->NetifAclChainList.prev,rtl865x_acl_chain_t,NetifAclChainList);

	if(acl_chain_new->NetifAclChainList.next == netif->NetifAclChainListHEAD_ptr)
		nxt_acl_chain_in_the_netif = NULL;
	else
		nxt_acl_chain_in_the_netif = list_entry(acl_chain_new->NetifAclChainList.next,rtl865x_acl_chain_t,NetifAclChainList);


	/* insert it into netif's acl chain layout list and start to reserve ACL entries */	
	INIT_LIST_HEAD(&acl_chain_new->AclChainLayoutList);	
	if(list_is_singular(netif->NetifAclChainListHEAD_ptr)) /* This is the first new chain in netif */
		list_add_tail(&acl_chain_new->AclChainLayoutList, &AclChainLayoutList_HEAD);
	else if(list_is_last(&acl_chain_new->NetifAclChainList,netif->NetifAclChainListHEAD_ptr)) /* This is the last chain in netif */
		list_add(&acl_chain_new->AclChainLayoutList, &pre_acl_chain_in_the_netif->AclChainLayoutList);
	else
		list_add_tail(&acl_chain_new->AclChainLayoutList, &nxt_acl_chain_in_the_netif->AclChainLayoutList);	


	if(acl_chain_new->AclChainLayoutList.prev == &AclChainLayoutList_HEAD)	
		acl_chain_new->reserved_start 	= acl_chain_new->reserved_end = 0;	
	else
	{
		rtl865x_acl_chain_t *acl_chain_prv = list_entry(acl_chain_new->AclChainLayoutList.prev,rtl865x_acl_chain_t,AclChainLayoutList);
		acl_chain_new->reserved_start 	= acl_chain_new->reserved_end =acl_chain_prv->reserved_end;
	}
	
	if(_rtl865x_change_aclchain_reserved_num(acl_chain_new,minumun_reserved_acl_num)!=SUCCESS)
	{
		list_del(&acl_chain_new->NetifAclChainList);
		list_del(&acl_chain_new->AclChainLayoutList);
		kfree(acl_chain_new);		
		DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return RTL_ENETIFINVALID;	
	}

	#ifdef DBG_ACL
	printk("=======After register===========\n");
	rtl865x_show_allAclChains();
	printk("============================\n");
	#endif
	
	return SUCCESS;
}

int32 rtl865x_aclChain_change_minumun_reserved(char *netifName, int32 priority, uint32 new_minumun_reserved)
{
	rtl865x_netif_local_t*	netif;
	rtl865x_acl_chain_t*		acl_chain_entry=NULL;
	rtl865x_acl_chain_t*		acl_chain_change=NULL;

	DBG_ACL_PRK("Enter %s (netifName:%s  priority:%d   new_minumun_reserved:%d)\n",__func__
		,netifName,priority,new_minumun_reserved);

	netif = _rtl865x_getSWNetifByName(netifName);
	if(netif == NULL)
	{
		DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return RTL_ENETIFINVALID;	
	}

	list_for_each_entry(acl_chain_entry,netif->NetifAclChainListHEAD_ptr,NetifAclChainList)
	{
		if(acl_chain_entry->chain_priority == priority)
		{
			acl_chain_change = acl_chain_entry;
			break;
		}
	}

	if(acl_chain_change == NULL)
	{
		DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return RTL_ENETIFINVALID;	
	}	

	if(_rtl865x_change_minumun_reserved(acl_chain_change,new_minumun_reserved)!=SUCCESS)
	{
		DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return RTL_ENETIFINVALID;	
	}

	return SUCCESS;
}


int32 rtl865x_unRegist_aclChain(char *netifName, int32 priority)
{
	rtl865x_netif_local_t*	netif;
	rtl865x_acl_chain_t*		acl_chain_entry=NULL;
	rtl865x_acl_chain_t*		acl_chain_del=NULL;


	DBG_ACL_PRK("Enter %s (netifName:%s  priority:%d)\n",__func__,netifName,priority);
	
	netif = _rtl865x_getSWNetifByName(netifName);
	if(netif == NULL)
	{
		DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return RTL_ENETIFINVALID;	
	}

	
	list_for_each_entry(acl_chain_entry,netif->NetifAclChainListHEAD_ptr,NetifAclChainList)
	{
		if(acl_chain_entry->chain_priority == priority)
		{
			acl_chain_del = acl_chain_entry;
			break;
		}
	}

	if(acl_chain_del == NULL)
	{
		DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return RTL_ENETIFINVALID;	
	}	

	if(_rtl865x_unRegist_aclChain(acl_chain_del)!=SUCCESS)
	{
		DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return RTL_ENETIFINVALID;	
	}	
	
	return SUCCESS;
}


int32 rtl865x_flush_allAcl_fromChain(char *netifName, int32 priority)
{
	rtl865x_netif_local_t*			assign_netif = NULL;	
	rtl865x_acl_chain_chainlist_t*	acl_chainlist_entry;	

	DBG_ACL_PRK("Enter %s (netifName:%s  priority:%d)\n",__func__,netifName,priority);

	if(netifName!=NULL) // clean acl rule for specific netif 
	{
		assign_netif = _rtl865x_getNetifByName(netifName);
		if(assign_netif == NULL)
		{
			DBG_ACL_PRK("Leave %s @ %d(RTL_EINVALIDINPUT)\n",__func__,__LINE__);
			return RTL_EINVALIDINPUT;
		}
	}
	else
		assign_netif = NULL; // clean acl rule on each netif 


	list_for_each_entry(acl_chainlist_entry,&AclChainListPool_HEAD,AclChainList)
	{		
		rtl865x_acl_chain_t*		acl_chain_entry;

		/*  If the user has assigned the specific netif */
		if(assign_netif!=NULL && assign_netif->NetifAclChainListHEAD_ptr != &acl_chainlist_entry->AclChainListHEAD)
			continue;

		list_for_each_entry(acl_chain_entry,&acl_chainlist_entry->AclChainListHEAD,NetifAclChainList)
		{
			if(acl_chain_entry->chain_priority == priority)
			{
				_rtl865x_flush_allAcl_fromChain(acl_chain_entry);
			}
		}	
	}	

	return SUCCESS;
}

int32 rtl865x_show_allAclChains(void)
{		
	int32 i;
	rtl865x_acl_chain_chainlist_t*	 	acl_chainlist_entry;
	
	for(i = 0; i < NETIF_SW_NUMBER; i++)
	{
		if(netifTbl[i].valid == 1)
		{
			printk("Netif  %s \tAclChainListHead :0x%p\n",netifTbl[i].name,netifTbl[i].NetifAclChainListHEAD_ptr);
		}
	}
	printk("\n");

	list_for_each_entry(acl_chainlist_entry,&AclChainListPool_HEAD,AclChainList)
	{
		printk("AclChainListHead  0x%p , refcount : %d vid=%d\n"
			,&acl_chainlist_entry->AclChainListHEAD,acl_chainlist_entry->ref_count,acl_chainlist_entry->vlan_id);
	}
	printk("\n");

	printk("Total number of writing acl asic table : %d \n",SW_ACL_STAT_WRITEASICACL_NUM);
	printk("Total number of changing acl layout  : %d \n",SW_ACL_STAT_LAYOUTCHANGE_NUM);
	printk("Total free acl entries  : %d \n",__rtl865x_free_aclentry_num());
	printk("\n");


	list_for_each_entry(acl_chainlist_entry,&AclChainListPool_HEAD,AclChainList)
	{		
		rtl865x_acl_chain_t *acl_chain_entry;			
		
		printk("==== AclChainListHead  0x%p ====\n",&acl_chainlist_entry->AclChainListHEAD);				

		list_for_each_entry(acl_chain_entry,&acl_chainlist_entry->AclChainListHEAD,NetifAclChainList)
		{					
			rtl865x_AclRule_t *rule;						
			printk("-- [  chain(%p):priority(%d),rulecnt(%d) ,reserved_minumun(%d),reserved acl buffer(%d -> %d) ]--\n",acl_chain_entry,acl_chain_entry->chain_priority,acl_chain_entry->acl_rules_num
				,acl_chain_entry->reserved_guarantee_minumun,acl_chain_entry->reserved_start,acl_chain_entry->reserved_end);		
			list_for_each_entry(rule,&acl_chain_entry->AclRuleListHEAD,AclRuleList)
			{
				rtl865x_showACL(rule);
			}			
		}			
	}

	#ifdef DBG_ACL
	_rtl865x_aclsync_debug();
	#endif
	
	return SUCCESS;	
}

/*
@func int32 | rtl865x_init_acl_chain |memory init for acl chains.
@rvalue SUCCESS | Success.
@comm
	this API must be called when system boot.
*/
int32 rtl865x_init_acl_chain(void)
{
	
	INIT_LIST_HEAD(&AclChainLayoutList_HEAD);
	INIT_LIST_HEAD(&AclChainListPool_HEAD);

	SW_ACL_STAT_WRITEASICACL_NUM		= 0;
	SW_ACL_STAT_LAYOUTCHANGE_NUM	= 0;
	
	return SUCCESS;
}

/*
@func int32 | rtl865x_init_acl |memory init for acl rules.
@rvalue SUCCESS | Success.
@comm
	this API must be called when system boot.
*/
int32 rtl865x_init_acl(void)
{	
	_rtl865x_confReservedAcl();
	
	/*init acl chains*/
	rtl865x_init_acl_chain();
	return SUCCESS;
}



#ifdef CONFIG_FAST_PATH_MODULE
EXPORT_SYMBOL(rtl865x_del_acl);
EXPORT_SYMBOL(rtl865x_add_acl);
#endif
#endif //CONFIG_RTL_LAYERED_DRIVER_ACL
int32 rtl865x_attachMasterNetif(char *slave, char *master)
{
	return _rtl865x_attachMasterNetif(slave, master);
}

int32 rtl865x_detachMasterNetif(char *slave)
{
	return _rtl865x_detachMasterNetif(slave);
}

int32 _rtl865x_addNetif(rtl865x_netif_t *netif)
{
	rtl865x_netif_local_t *entry;
#if defined(CONFIG_RTL_LAYERED_DRIVER_ACL)
	rtl865x_AclRule_t *rule;
#endif
	int32 retval = FAILED;
	int32 i;
	int32 sw_entry_idx=-1;
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	int asicIdx;
	rtl865xc_tblAsic_netifTable_t asicEntry;
#endif

	DBG_NETIF_PRK("Enter %s (vid:%d name:%s mac:%02x: %02x: %02x: %02x: %02x: %02x)\n",__func__,netif->vid,netif->name	
	,netif->macAddr.octet[0],netif->macAddr.octet[1],netif->macAddr.octet[2],netif->macAddr.octet[3],netif->macAddr.octet[4],netif->macAddr.octet[5]);

	if(netif == NULL)
	{
		DBG_NETIF_PRK("(%s)return RTL_EINVALIDINPUT\n",__func__);
		return RTL_EINVALIDINPUT;
	}

	/*duplicate entry....*/
	entry = _rtl865x_getSWNetifByName(netif->name);
	if(entry)
	{
		DBG_NETIF_PRK("(%s)return RTL_EENTRYALREADYEXIST\n",__func__);
		return RTL_EENTRYALREADYEXIST;
	}

	/*get sw_netif buffer*/
	for(i = 0; i < NETIF_SW_NUMBER; i++)
	{
		if(netifTbl[i].valid == 0)
		{
			sw_entry_idx = i;
			break;
		}
	}

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	for (asicIdx=0;asicIdx<RTL865XC_NETIFTBL_SIZE;asicIdx++)
	{	
		_rtl8651_readAsicEntry(TYPE_NETINTERFACE_TABLE, asicIdx, &asicEntry);
		if (asicEntry.valid==0)
		{
			break;
		}
	}	
	if(netif->forMacBasedMCast==TRUE)
	{
		asicIdx=RTL865XC_NETIFTBL_SIZE-1;
	}
#endif


	if (sw_entry_idx == -1)
	{
		printk("(%s) Warning , there is no more empty entry in Software Netif Table\n ",__func__);
		return RTL_ENOFREEBUFFER;
	}



	DBG_NETIF_PRK("(%s)----------Before changing ----------------------\n",__func__);
	for(i = 0; i < NETIF_SW_NUMBER; i++)
	{
		DBG_NETIF_PRK("[%02d] valid:%d  name:%s vid:%d  is_slave:%d  acl_chainlist_head:%p\n",i
			,netifTbl[i].valid,netifTbl[i].name,netifTbl[i].vid,netifTbl[i].is_slave
			,netifTbl[i].NetifAclChainListHEAD_ptr);
	}
	DBG_NETIF_PRK("------------------------------------------\n");




	/*create new sw_netif entry*/
	entry = &netifTbl[sw_entry_idx];

	memset(entry, 0, sizeof(rtl865x_netif_local_t));
	entry->valid = 1;
	entry->mtu = netif->mtu;
	entry->if_type = netif->if_type;
	entry->macAddr = netif->macAddr;
	entry->vid = netif->vid;
	entry->is_wan = netif->is_wan;
	entry->dmz = netif->dmz;
	entry->is_slave = netif->is_slave;
	memcpy(entry->name,netif->name,MAX_IFNAMESIZE);
	entry->enableRoute = netif->enableRoute; 
	entry->macAddrNumber = 1;
	entry->inAclEnd = entry->inAclStart = entry->outAclEnd = entry->outAclStart = RTL865X_ACLTBL_PERMIT_ALL; /*default permit...*/
	entry->master = NULL;

	DBG_NETIF_PRK("(%s) create new sw_netif entry in netifTbl[idx = %d] , name=%s \n",__func__,sw_entry_idx,entry->name);


	/*  check whether to create new hw_netif entry?   */
	if(entry->is_slave == 0)		
	{	
		rtl865x_netif_local_t* samevid_netif = NULL;		
		for(i = 0; i < NETIF_SW_NUMBER; i++)
		{	
			if(netifTbl[i].valid == 1 && netifTbl[i].is_slave == 0 && netifTbl[i].vid==entry->vid && (&netifTbl[i]!=entry))	
			{
				samevid_netif = &netifTbl[i];
				break;		
			}
		}

		/*   Step 1.  Set  hw_netif asic idx  */
		if(samevid_netif)
		{
			/* if there exists the other master netif with the same vid , just copy ! */
			entry->asicIdx = samevid_netif->asicIdx;			
		}
		else
		{			
			int asic_idx=0;

			/* find new available hw asic index */
			for(asic_idx=0;asic_idx<NETIF_NUMBER;asic_idx++)		
				if(Netif_Asic2Sw_Mapping[asic_idx] == -1)
					break;		
			if(asic_idx==NETIF_NUMBER)
			{				
				printk("(%s) Warning , there is no more empty entry in Asic Netif Table\n ",__func__);
				return RTL_ENOFREEBUFFER;
			}	
			Netif_Asic2Sw_Mapping[asic_idx] = sw_entry_idx ;
			entry->asicIdx = asic_idx;
		}


		
		#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL  /* if define this macro, it means that the driver can write/read acl. Otherwise, always permit */
		
		/*   Step 2.  Set  acl chain  */
		_rtl865x_associate_aclchainlist(entry);		

		
		/*   Step 3.  Initialize  acl chain only if this is a new vid */
		if(!samevid_netif)
		{				
			#ifdef CONFIG_RTL_HW_NAPT
			/* Note. When the mode is HWNAT (default acl policy is trap)
				We only permit :
					1. IPv4 Multicast (Let asic multicast table to decide whether to multicast hwacc)
					2. L2/L34 Unicast				
			*/

			#ifdef CONFIG_RTL8196E_IPCHECKSUM_ERROR_PATCH
			/***  RTL865X_ACL_ICBUG_PATCH   ***
					Trap the brdiged packet :
						(1) unicast packet (DA = unicast ) with both vlan and pppoe packet
						(2) multiacst packet (DA = 01:00:5e:xx:xx:xx)
					8196d IC bug : hwacc leads to IP checksum error when enable L2 DSCP remarking
						If the packet's L2 header >=  SA + DA + 12 bytes 
			******************************************/			
			retval = rtl865x_regist_aclChain(netif->name, RTL865X_ACL_ICBUG_PATCH,0);	
			#endif
			
			/***  RTL865X_ACL_QOS_USED0,1  ***
					the acl rule for ipQos 
			*****************************/
			#ifdef CONFIG_RTL_HW_QOS_SUPPORT		
			retval = rtl865x_regist_aclChain(netif->name, RTL865X_ACL_QOS_USED0,2);
			#endif			

			/***  RTL865X_ACL_IPv4MUL  ***
					Permit the IPv4 Multicast (dmac = 01:00:5e......)  
			********************************/			
			retval = rtl865x_regist_aclChain(netif->name, RTL865X_ACL_IPv4MUL,1);			

			rule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
			if(!rule)
			{				
				printk("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
				return FAILED;
			}		
			memset(rule, 0,sizeof(rtl865x_AclRule_t));	
			rule->ruleType_ = RTL865X_ACL_MAC;
			rule->actionType_ = RTL865X_ACL_PERMIT;
			rule->pktOpApp_ = RTL865X_ACL_ALL_LAYER;
			rule->direction_ = RTL865X_ACL_INGRESS;
			rule->un_ty.dstMac_.octet[0] = 0x01;
			rule->un_ty.dstMac_.octet[1] = 0x00;
			rule->un_ty.dstMac_.octet[2] = 0x5e;
			rule->un_ty.dstMacMask_.octet[0] = 0xff;	
			rule->un_ty.dstMacMask_.octet[1] = 0xff;	
			rule->un_ty.dstMacMask_.octet[2] = 0xff;		
			rtl865x_add_acl(rule, netif->name, RTL865X_ACL_IPv4MUL,1,0);
			kfree(rule);

			/***  RTL865X_ACL_IPv4MUL_PPPoE  ***
					Permit the IPv4 Multicast (dmac = gateway mac)  
			********************************/			
			retval = rtl865x_regist_aclChain(netif->name, RTL865X_ACL_IPv4MUL_PPPoE,1);			
			
			rule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
			if(!rule)
			{				
				printk("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
				return FAILED;
			}		
			memset(rule, 0,sizeof(rtl865x_AclRule_t));	
			rule->ruleType_ = RTL865X_ACL_DSTFILTER;
			rule->actionType_ = RTL865X_ACL_PERMIT;
			rule->pktOpApp_ = RTL865X_ACL_ALL_LAYER;
			rule->direction_ = RTL865X_ACL_INGRESS;
			memcpy(rule->un_ty.dstFilterMac_.octet,netif->macAddr.octet,ETHER_ADDR_LEN);
			rule->un_ty.dstFilterMacMask_.octet[0] = 0xff;	
			rule->un_ty.dstFilterMacMask_.octet[1] = 0xff;	
			rule->un_ty.dstFilterMacMask_.octet[2] = 0xff;
			rule->un_ty.dstFilterMacMask_.octet[3] = 0xff;	
			rule->un_ty.dstFilterMacMask_.octet[4] = 0xff;	
			rule->un_ty.dstFilterMacMask_.octet[5] = 0xff;
			rule->un_ty.dstFilterIpAddr_	= 0xE0000000;
			rule->un_ty.dstFilterIpAddrMask_= 0xF0000000;			
			rule->un_ty.dstFilterPortUpperBound_ = 0xFFFF;	
			rule->un_ty.dstFilterPortLowerBound_ = 0x0000;
			rtl865x_add_acl(rule, netif->name, RTL865X_ACL_IPv4MUL_PPPoE,1,0);
			kfree(rule);

			/***  RTL865X_ACL_PREPROCESS_LOCALHOST  ***
					when enable static acl, we will set some filter (mac , ip/port  filter) rule to drop some flow
					therefore, we need another rule to promise that all pkts toward localhost will be permit
			******************************************/
			#ifdef CONFIG_RTL8676_Static_ACL
			retval = rtl865x_regist_aclChain(netif->name, RTL865X_ACL_PREPROCESS_LOCALHOST,1);	
			entry->ipv4_addr=0x00000000;	//initialized empty value
			#endif	

			
			/*** RTL865X_ACL_USER_USED *** 
					when static acl     :   it is used to add iptables filter rules
					when dynamic acl :   it is used to add permit rules for hw-acc
			     The manipulation functions is in rtl865x_acl_control.c                    
			********************************/			
			retval = rtl865x_regist_aclChain(netif->name, RTL865X_ACL_USER_USED,0);
			if(_rtl865x_reassign_userchain_minumun_reserved()!=SUCCESS)
			{
				DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
				return FAILED;	
			}		
		#endif /* CONFIG_RTL_HW_NAPT */	
		
			/***  RTL865X_ACL_DEFAULT  ***
					the last rules in acl , default rule in acl  
			*****************************/
			retval = rtl865x_regist_aclChain(netif->name, RTL865X_ACL_DEFAULT,1);		
			
			rule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
			if(!rule)
			{				
				printk("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
				return FAILED;
			}		
			memset(rule, 0,sizeof(rtl865x_AclRule_t));		
			rule->ruleType_	= RTL865X_ACL_MAC;
		#ifdef CONFIG_RTL_HW_L2_ONLY
			rule->actionType_	= RTL865X_ACL_PERMIT;
		#else
			rule->actionType_	= RTL865X_ACL_TOCPU;		
		#endif
			rule->pktOpApp_ 	= RTL865X_ACL_ALL_LAYER;
			rule->direction_ = RTL865X_ACL_INGRESS;	
			rtl865x_add_acl(rule, netif->name, RTL865X_ACL_DEFAULT,1,0);
			kfree(rule);			
			
		}
		else
		{
			entry->inAclEnd 	= samevid_netif->inAclEnd;
			entry->inAclStart = samevid_netif->inAclStart;
			entry->outAclEnd = samevid_netif->outAclEnd;
			entry->outAclStart = samevid_netif->outAclStart;
		}
		#endif //CONFIG_RTL_LAYERED_DRIVER_ACL		
		
		/*  Step 4. Set hw_netif into ASIC  */
		retval = _rtl865x_setAsicNetif(entry);		
	}
	else
	{
		/* Slave netif , need not do anything about hw_netif
			just wait user let this slave netif attch to some other master netif .. */
	}


	DBG_NETIF_PRK("(%s)----------After changing ----------------------\n",__func__);
	for(i = 0; i < NETIF_SW_NUMBER; i++)
	{
		DBG_NETIF_PRK("[%02d] valid:%d  name:%s vid:%d  is_slave:%d  acl_chainlist_head:%p\n",i
			,netifTbl[i].valid,netifTbl[i].name,netifTbl[i].vid,netifTbl[i].is_slave
			,netifTbl[i].NetifAclChainListHEAD_ptr);
	}
	DBG_NETIF_PRK("------------------------------------------\n");
	
	#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
	rtl865x_raiseEvent(EVENT_ADD_NETIF, (void*)entry);
	#endif
	
	DBG_NETIF_PRK("Leave %s with SUCCESS\n",__func__);	
	return SUCCESS;
}

static int32 _rtl865x_delNetif(char *name)
{
	rtl865x_netif_local_t *entry;
	int i;
	#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
	int need_reassign_othernetif_userchain_reserved_num = 0;
	#endif

	
	DBG_NETIF_PRK("Enter %s (name:%s)\n",__func__,name);

	

	/*FIXME:hyking, get swNetif entry.....*/
	entry = _rtl865x_getSWNetifByName(name);
	if(entry == NULL)
		return RTL_EENTRYNOTFOUND;


	DBG_NETIF_PRK("(%s)----------Before changing ----------------------\n",__func__);
	for(i = 0; i < NETIF_SW_NUMBER; i++)
	{
		DBG_NETIF_PRK("[%02d] valid:%d  name:%s vid:%d  is_slave:%d  acl_chainlist_head:%p\n",i
			,netifTbl[i].valid,netifTbl[i].name,netifTbl[i].vid,netifTbl[i].is_slave
			,netifTbl[i].NetifAclChainListHEAD_ptr);
	}
	DBG_NETIF_PRK("------------------------------------------\n");


	/*  if there does not exist the other master netif with the same vid
	        =>  delete hw_netif entry   */
	if(entry->is_slave == 0)		
	{
		rtl865x_netif_local_t* samevid_netif = NULL;		
		
		for(i = 0; i < NETIF_SW_NUMBER; i++)
		{	
			if(netifTbl[i].valid == 1 && netifTbl[i].is_slave == 0 && netifTbl[i].vid==entry->vid && (&netifTbl[i]!=entry))	
			{
				samevid_netif = &netifTbl[i];
				break;		
			}
		}

		if(!samevid_netif)
		{	
			/* Step 1. delete hw_entry (clear asic mapping )*/
			Netif_Asic2Sw_Mapping[entry->asicIdx] = -1;
			if(rtl865x_delNetInterfaceByVid(entry->vid)!=SUCCESS)
			{				
				DBG_NETIF_PRK("Leave %s @ %d \n",__func__,__LINE__);
				return FAILED;
			}

			/* Step 2. now delete all slave interface whose master is the deleting master interface*/
			{
				int32 i ;
				for(i = 0; i < NETIF_SW_NUMBER; i++)
				{
					if(netifTbl[i].valid == 1 && netifTbl[i].is_slave == 1 && netifTbl[i].master == entry)
						netifTbl[i].master = NULL;
				}
			}
			
			#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
			need_reassign_othernetif_userchain_reserved_num = 1;
			#endif	

		}

		/* step3. deref aclchain list */
		#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
		if(_rtl865x_dissociate_aclchainlist(entry)!=SUCCESS)
		{				
			DBG_NETIF_PRK("Leave %s @ %d \n",__func__,__LINE__);
			return FAILED;
		}
		#endif		
	}	
	
	/* step4. clear sw_netif entry  */
	memset(entry,0,sizeof(rtl865x_netif_local_t));

	
	/* step5. reassign othernetif userchain's reserved num */
	#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
	if(need_reassign_othernetif_userchain_reserved_num && _rtl865x_reassign_userchain_minumun_reserved()!=SUCCESS)
	{
		DBG_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return FAILED;	
	}
	#endif
	
	DBG_NETIF_PRK("(%s)----------After changing ----------------------\n",__func__);
	for(i = 0; i < NETIF_SW_NUMBER; i++)
	{
		DBG_NETIF_PRK("[%02d] valid:%d  name:%s vid:%d  is_slave:%d  acl_chainlist_head:%p\n",i
			,netifTbl[i].valid,netifTbl[i].name,netifTbl[i].vid,netifTbl[i].is_slave
			,netifTbl[i].NetifAclChainListHEAD_ptr);
	}
	DBG_NETIF_PRK("------------------------------------------\n");

	#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
	rtl865x_raiseEvent(EVENT_DEL_NETIF, (void*)entry);
	#endif

	DBG_NETIF_PRK("Leave %s with SUCCESS\n",__func__);
	
	return SUCCESS;

}


/*
@func int32 | rtl865x_addNetif |add network interface.
@parm rtl865x_netif_t* | netif | network interface
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDINPUT | netif is NULL
@rvalue RTL_EENTRYALREADYEXIST | netif is already exist
@rvalue RTL_ENOFREEBUFFER | no netif to used
@rvalue FAILED | Failed
@comm
	rtl865x_netif_t: please refer in header file.
*/

int32 rtl865x_addNetif(rtl865x_netif_t *netif)
{
	int32 retval = FAILED;
	unsigned long flags;	
	//rtl_down_interruptible(&netif_sem);
	local_irq_save(flags);
	retval = _rtl865x_addNetif(netif);
	//rtl_up(&netif_sem);
	local_irq_restore(flags);
	return retval;
}


/*
@func int32 | rtl865x_delNetif |delete network interface.
@parm char* | ifName | network interface name
@rvalue SUCCESS | Success.
@rvalue RTL_EENTRYNOTFOUND | network interface is NOT found
@rvalue RTL_EREFERENCEDBYOTHER | netif is referenced by onter table entry
@rvalue FAILED | Failed
@comm	
*/
int32 rtl865x_delNetif(char *ifName)
{
	int32 retval = FAILED;
	unsigned long flags;	
	//rtl_down_interruptible(&netif_sem);
	local_irq_save(flags);
	retval = _rtl865x_delNetif(ifName);
	//rtl_up(&netif_sem);
	local_irq_restore(flags);
	return retval;
}
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
int32 rtl865x_addVirtualNetif(rtl865x_netif_t *netif)
{

	rtl865x_netif_local_t *entry;
	int32 retval = FAILED;
	if(netif == NULL)
		return RTL_EINVALIDINPUT;

	/*duplicate entry....*/
	entry = _rtl865x_getSWNetifByName(netif->name);
	if(entry)
		return RTL_EENTRYALREADYEXIST;


	/*add new entry*/
	entry = &virtualNetIf;

	memset(entry, 0, sizeof(rtl865x_netif_local_t));

	entry->valid = 1;
	memcpy(entry->name,netif->name,MAX_IFNAMESIZE);
	entry->inAclEnd = entry->inAclStart = entry->outAclEnd = entry->outAclStart = RTL865X_ACLTBL_PERMIT_ALL; /*default permit...*/
	entry->refCnt = 1;

	
#ifdef 	CONFIG_RTL_LAYERED_DRIVER_ACL
	entry->chainListHead[RTL865X_ACL_INGRESS] = NULL;
	entry->chainListHead[RTL865X_ACL_EGRESS] = NULL;
#endif


#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
	/*register 2 ingress chains: system/user*/
	retval = rtl865x_regist_aclChain(netif->name, RTL865X_ACL_SYSTEM_USED, RTL865X_ACL_INGRESS,RTL865X_ACL_FORCE);
	retval = rtl865x_regist_aclChain(netif->name, RTL865X_ACL_USER_USED, RTL865X_ACL_INGRESS,RTL865X_ACL_NOT_FORCE);
#endif //CONFIG_RTL_LAYERED_DRIVER_ACL

	return SUCCESS;
}

int32 rtl865x_delVirtualNetif(char *ifName)
{
	int32 retval = FAILED;
	unsigned long flags;	
	//rtl_down_interruptible(&netif_sem);
	local_irq_save(flags);
	retval = _rtl865x_delNetif(ifName);
	//rtl_up(&netif_sem);
	local_irq_restore(flags);
	_rtl865x_confReservedAcl();
	return retval;
}
#endif
#if 0
/*
@func int32 | rtl865x_referNetif |reference network interface entry.
@parm char* | ifName | network interface name
@rvalue SUCCESS | Success.
@rvalue RTL_EENTRYNOTFOUND | network interface is NOT found
@rvalue FAILED | Failed
@comm
when other table entry refer network interface table entry, please call this API.
*/
int32 rtl865x_referNetif(char *ifName)
{
	int32 retval = FAILED;
	unsigned long flags;	
	//rtl_down_interruptible(&netif_sem);
	local_irq_save(flags);
	retval = _rtl865x_referNetif(ifName);
	//rtl_up(&netif_sem);
	local_irq_restore(flags);
	return retval;
}

/*
@func int32 | rtl865x_deReferNetif |dereference network interface.
@parm char* | ifName | network interface name
@rvalue SUCCESS | Success.
@rvalue RTL_EENTRYNOTFOUND | network interface is NOT found
@rvalue FAILED | Failed
@comm	
this API should be called after rtl865x_referNetif.
*/
int32 rtl865x_deReferNetif(char *ifName)
{
	int32 retval = FAILED;
	unsigned long flags;	
	//rtl_down_interruptible(&netif_sem);
	local_irq_save(flags);
	retval = _rtl865x_deReferNetif(ifName);
	//rtl_up(&netif_sem);
	local_irq_restore(flags);
	return retval;
}
#endif

int32 rtl865x_setPortToNetif(char *name,uint32 port)
{
	int32 ret;
	rtl865x_netif_local_t *entry;
	entry = _rtl865x_getNetifByName(name);
	
	if(entry == NULL)
		return FAILED;

	ret = rtl8651_setPortToNetif(port, entry->asicIdx);	
	return ret;
}


/*
@func int32 | rtl865x_setNetifMac |config network interface Mac address.
@parm rtl865x_netif_t* | netif | netif name&MAC address
@rvalue SUCCESS | Success.
@rvalue RTL_EENTRYNOTFOUND | network interface is NOT found
@rvalue FAILED | Failed
@comm	
*/
int32 rtl865x_setNetifMac(rtl865x_netif_t *netif)
{
	int32 retval = FAILED;
	unsigned long flags;	
	//rtl_down_interruptible(&netif_sem);
	local_irq_save(flags);
	//august: do not printk this msg, it can be misunderstood in some case!
	//printk("%s set netif mac to: %2x:%2x:%2x:%2x:%2x:%2x\n", netif->name, netif->macAddr.octet[0], netif->macAddr.octet[1], netif->macAddr.octet[2], 
		//netif->macAddr.octet[3], netif->macAddr.octet[4], netif->macAddr.octet[5]);
	retval = _rtl865x_setNetifMac(netif);
	//rtl_up(&netif_sem);
	local_irq_restore(flags);
	return retval;
}

/*
@func int32 | rtl865x_setNetifMtu |config network interface MTU.
@parm rtl865x_netif_t* | netif | netif name & MTU
@rvalue SUCCESS | Success.
@rvalue RTL_EENTRYNOTFOUND | network interface is NOT found
@rvalue FAILED | Failed
@comm	
*/
int32 rtl865x_setNetifMtu(rtl865x_netif_t *netif)
{
	int32 retval = FAILED;
	unsigned long flags;	
	//rtl_down_interruptible(&netif_sem);
	local_irq_save(flags);
	retval = _rtl865x_setNetifMtu(netif);
	//rtl_up(&netif_sem);	
	local_irq_restore(flags);
	return retval;
}


#ifdef CONFIG_RTL8676_Static_ACL
static void rtl865x_netif_rewrite_acl_preprocess_localhost(void)
{
	int i;
	rtl865x_AclRule_t *rule;	
	
	rtl865x_flush_allAcl_fromChain(NULL,RTL865X_ACL_PREPROCESS_LOCALHOST,RTL865X_ACL_INGRESS);	

	for(i = 0; i < NETIF_SW_NUMBER; i++)
	{
		if(netifTbl[i].valid == 0 || netifTbl[i].is_slave == 1 || netifTbl[i].ipv4_addr == 0x00000000)

			continue;
				
		/* permit all pkts to local host  */
		rule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
		if(!rule)						
			printk("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);			
				
		memset(rule, 0,sizeof(rtl865x_AclRule_t));	
		rule->ruleType_ = RTL865X_ACL_IP;
		rule->actionType_ = RTL865X_ACL_PERMIT;
		rule->pktOpApp_ = RTL865X_ACL_ALL_LAYER;
		rule->direction_ = RTL865X_ACL_INGRESS;			
		rule->dstIpAddr_		= netifTbl[i].ipv4_addr;		
		rule->dstIpAddrMask_	= 0xFFFFFFFF;					
		rtl865x_add_acl(rule, netifTbl[i].name, RTL865X_ACL_PREPROCESS_LOCALHOST);
		kfree(rule);	
	}
}
static int rtl865x_netif_inetaddr_event(struct notifier_block *this, unsigned long event, void *ptr)
{	
	struct in_ifaddr *ifa = (struct in_ifaddr *)ptr;
	struct net_device *dev = ifa->ifa_dev->dev;
	rtl865x_netif_local_t *netif = _rtl865x_getNetifByName(dev->name);	

	if(!netif)
		return NOTIFY_DONE;

	switch (event) {
	case NETDEV_UP:			
		netif->ipv4_addr = ifa->ifa_local;
		break;
	case NETDEV_DOWN:	
		netif->ipv4_addr = 0x00000000; //clear
		break;
	}
	rtl865x_netif_rewrite_acl_preprocess_localhost();	
	return NOTIFY_DONE;
}
static int rtl865x_netif_netdev_event(struct notifier_block *this, unsigned long event, void *ptr)
{	
	struct net_device *dev = ptr;
	struct in_device *in_dev = __in_dev_get_rtnl(dev);
	rtl865x_netif_local_t *netif = _rtl865x_getNetifByName(dev->name);

	if(!netif)
		return NOTIFY_DONE;

	switch (event) {
	case NETDEV_UP:
		for_ifa(in_dev) {
			netif->ipv4_addr = ifa->ifa_local;
		} endfor_ifa(in_dev);
		break;
	case NETDEV_DOWN:
		netif->ipv4_addr = 0x00000000; //clear
		break;
	}
	rtl865x_netif_rewrite_acl_preprocess_localhost();
	return NOTIFY_DONE;
}
#endif

/*
@func int32 | rtl865x_initNetifTable | initialize network interface table.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failed,system should be reboot.
*/
int32 rtl865x_initNetifTable(void)
{
	int i;
	TBL_MEM_ALLOC(netifTbl, rtl865x_netif_local_t, NETIF_SW_NUMBER);	
	memset(netifTbl,0,sizeof(rtl865x_netif_local_t)*NETIF_SW_NUMBER);	
	
	for(i=0;i<NETIF_NUMBER;i++)
		Netif_Asic2Sw_Mapping[i] = -1;


#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
	/*init reserved acl in function init_acl...*/
#else
	_rtl865x_confReservedAcl();
#endif

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	memset(&virtualNetIf,0,sizeof(rtl865x_netif_local_t));
#endif

	/* if static acl, we has to change acl rule in RTL865X_ACL_PREPROCESS_LOCALHOST when localhost's IP changes */
#ifdef CONFIG_RTL8676_Static_ACL
	register_inetaddr_notifier(&rtl865x_netif_inetaddr_notifier);
	register_netdevice_notifier(&rtl865x_netif_netdev_notifier);
#endif

	return SUCCESS;	
}

/*
@func int32 | rtl865x_enableNetifRouting |config network interface operation layer.
@parm rtl865x_netif_local_t* | netif | netif & enableRoute
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDINPUT | input is invalid
@rvalue FAILED | Failed
@comm	
*/
int32 rtl865x_enableNetifRouting(rtl865x_netif_local_t *netif)
{
	int32 retval = FAILED;
	
	if(netif == NULL)
		return RTL_EINVALIDINPUT;
	if(netif ->enableRoute == 1)
		return SUCCESS;

	netif->enableRoute = 1;
	retval = _rtl865x_setAsicNetif(netif);
	return retval;
}

/*
@func int32 | rtl865x_disableNetifRouting |config network interface operation layer.
@parm rtl865x_netif_local_t* | netif | netif & enableRoute
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDINPUT | input is invalid
@rvalue FAILED | Failed
@comm	
*/
int32 rtl865x_disableNetifRouting(rtl865x_netif_local_t *netif)
{
	int32 retval = FAILED;
	
	if(netif == NULL)
		return RTL_EINVALIDINPUT;
	
	if(netif ->enableRoute == 0)
		return SUCCESS;

	netif->enableRoute = 0;
	retval = _rtl865x_setAsicNetif(netif);
	return retval;
}

/*
@func int32 | rtl865x_disableNetifRouting |config network interface operation layer.
@parm rtl865x_netif_local_t* | netif | netif & enableRoute
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDINPUT | input is invalid
@rvalue FAILED | Failed
@comm	
*/
int32 rtl865x_reinitNetifTable(void)
{
	int32 i;
	unsigned long flags;	
	//rtl_down_interruptible(&netif_sem);
	local_irq_save(flags);

	for(i = 0; i < NETIF_SW_NUMBER; i++)
	{
		if(netifTbl[i].valid)
		{
			_rtl865x_delNetif(netifTbl[i].name);
		}
	}
	//memset(netifTbl,0,sizeof(rtl865x_netif_local_t)*NETIF_SW_NUMBER);

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	if(virtualNetIf.valid)
	{
		_rtl865x_delNetif(virtualNetIf.name);
	}
#endif

	//rtl_up(&netif_sem);
	local_irq_restore(flags);
	return SUCCESS;
}


#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
uint32 rtl865x_getExternalPortMask(void)
{
	int32 i;
	rtl865x_netif_local_t *netif = NULL;
	uint32 externalPortMask=0;

	for(i = 0; i < NETIF_SW_NUMBER; i++)
	{
		netif = &netifTbl[i];		
		if((netif->valid == 1) && (netif->is_wan==1))
		{
			externalPortMask|=rtl865x_getVlanPortMask(netif->vid);
		}
	}

	return externalPortMask;
}

int32 rtl865x_getNetifVid(char *name, uint32 *vid)
{
	rtl865x_netif_local_t *entry;

	if(name == NULL)
	{
		return FAILED;
	}

	entry = _rtl865x_getNetifByName(name);

	if(entry == NULL)
	{
		return FAILED;
	}
	
	*vid=(uint32)(entry->vid);
	return SUCCESS;
	
}

int32 rtl865x_getNetifType(char *name,uint32 *type)
{
	rtl865x_netif_local_t *entry;

	if(name == NULL)
	{
		return FAILED;
	}

	if(type==NULL)
	{
		return FAILED;
	}
	
	entry = _rtl865x_getNetifByName(name);

	if(entry == NULL)
	{
		return FAILED;
	}
	
	*type=(uint32)(entry->if_type);
	return SUCCESS;
	
}
#endif
