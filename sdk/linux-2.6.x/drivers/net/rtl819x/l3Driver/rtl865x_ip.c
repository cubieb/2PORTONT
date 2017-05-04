/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.
* 
* Program : ip table driver
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/

/*      @doc RTL_LAYEREDDRV_API

        @module rtl865x_ip.c - RTL865x Home gateway controller Layered driver API documentation       |
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
#include "common/rtl_errno.h"
//#include "rtl_utils.h"
//#include <net/rtl/rtl865x_ip_api.h>
#include "rtl865x_ip.h"
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
#include <net/rtl/rtl865x_localPublic.h>
#endif
#include "common/rtl865x_eventMgr.h" /*call back function....*/

#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL3.h"	
#else
#include "AsicDriver/rtl865xC_tblAsicDrv.h"
#endif

#include "rtl865x_route.h"



static rtl865x_ip_entry_t *rtl865x_ipTable;

#define IP_TABLE_INDEX(entry)	(entry - rtl865x_ipTable)
static RTL_DECLARE_MUTEX(ip_sem);

static int32 _rtl865x_delIp(ipaddr_t extIp);


void rtl865x_extIP_show(void)
{
	int32 i ;
	for(i = 0; i < IP_NUMBER; i++)
	{
		printk("[%02i]  ",i);
		if(rtl865x_ipTable[i].valid)
		{
			printk("intIp:%u.%u.%u.%u\textIp:%u.%u.%u.%u",NIPQUAD(rtl865x_ipTable[i].intIp),NIPQUAD(rtl865x_ipTable[i].extIp));
			switch(rtl865x_ipTable[i].type)	
			{
				case IP_TYPE_NAPT:
					printk("\ttype: IP_TYPE_NAPT");
					break;
				case IP_TYPE_NAT:
					printk("\ttype: IP_TYPE_NAT");
					break;
				case IP_TYPE_LOCALSERVER:
					printk("\ttype: IP_TYPE_LOCALSERVER");
					break;
				case IP_TYPE_RESERVED:
					printk("\ttype: IP_TYPE_RESERVED");
					break;
			}
			printk("\tref_cnt:%d",rtl865x_ipTable[i].ref_count);
		}
		printk("\n");
	}
}

/*
@func int32 | rtl865x_initIpTable | initialize ip table.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failed,system should be reboot.
*/
int32 rtl865x_initIpTable(void)
{
	TBL_MEM_ALLOC(rtl865x_ipTable, rtl865x_ip_entry_t, IP_NUMBER);	
	memset(rtl865x_ipTable,0,sizeof(rtl865x_ip_entry_t)*IP_NUMBER);
	return SUCCESS;	
}

/*
@func int32 | rtl865x_initIpTable | reinitialize ip table.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failed.
*/
int32 rtl865x_reinitIpTable(void)
{
	int32 i ;
	for(i = 0; i < IP_NUMBER; i++)
	{
		if(rtl865x_ipTable[i].valid)
			_rtl865x_delIp(rtl865x_ipTable[i].extIp);
	}
	return SUCCESS;
}


static int32 _rtl865x_addIp(ipaddr_t intIp, ipaddr_t extIp, uint32 ip_type,char* output_netif)
{
	int i;
	rtl865x_ip_entry_t *entry = NULL;
	rtl865x_tblAsicDrv_extIntIpParam_t asicIp;
	int32 retval = FAILED;

	//printk("%s %d intIP:%x extIP:%x\n", __func__, __LINE__, intIp, extIp);
	if(ip_type < IP_TYPE_NAPT || ip_type > IP_TYPE_LOCALSERVER)
		return RTL_EINVALIDINPUT;

	/*duplicate check*/
	for(i = 0; i < IP_NUMBER; i++)
	{
		if(rtl865x_ipTable[i].extIp == extIp && rtl865x_ipTable[i].intIp == intIp
			&& rtl865x_ipTable[i].type == ip_type && !strcmp(rtl865x_ipTable[i].output_netif,output_netif))
		{
			rtl865x_ipTable[i].ref_count ++;
			return SUCCESS;				
		}
	}
	
	/*found a valid entry*/
	for(i = 0; i < IP_NUMBER; i++)
	{
		if(rtl865x_ipTable[i].valid == 0)
		{
			entry = &rtl865x_ipTable[i];
			break;
		}
	}

	if(entry == NULL)
		return RTL_ENOFREEBUFFER;

		
	/*update releated information*/
	entry->valid = 1;
	entry->intIp = intIp;
	entry->extIp = extIp;
	entry->type = ip_type;
	entry->ref_count = 1;
	strcpy(entry->output_netif,output_netif);
		
	/*from 865xC, this field is invalid...*/
	entry->nexthop = NULL;


	/*add this ip entry to asic*/
	/* Set asic */
	bzero(&asicIp, sizeof(rtl865x_tblAsicDrv_extIntIpParam_t));
	asicIp.extIpAddr	= extIp;
	asicIp.intIpAddr	= intIp;
	asicIp.localPublic	= (ip_type == IP_TYPE_LOCALSERVER)? TRUE: FALSE;
	asicIp.nat			= (ip_type == IP_TYPE_NAT)? TRUE: FALSE;
	asicIp.nhIndex		= 0;
	
	retval = rtl8651_setAsicExtIntIpTable(IP_TABLE_INDEX(entry), &asicIp);

	if(ip_type==IP_TYPE_NAPT)
		rtl865x_refreshRouteArpExtIP(output_netif,extIp);

	//rtl8651_setAsicOperationLayer(4);
	

	return SUCCESS;
	
}

static int32 _rtl865x_delIp(ipaddr_t extIp)
{
	int i;
	rtl865x_ip_entry_t *entry = NULL;

	/*found the entry*/
	for(i = 0; i < IP_NUMBER; i++)
	{
		if(rtl865x_ipTable[i].valid == 1 && rtl865x_ipTable[i].extIp == extIp)
		{
			entry = &rtl865x_ipTable[i];
			break;
		}
	}

	if(entry==NULL)
		return RTL_EENTRYNOTFOUND;
		
	/*update asic ip table*/
	entry->ref_count -- ; 
	if(entry->ref_count ==0)
	{
		rtl8651_delAsicExtIntIpTable(IP_TABLE_INDEX(entry));	
		memset(entry,0,sizeof(rtl865x_ip_entry_t));
		#if defined CONFIG_RTL_LOCAL_PUBLIC
		if(rtl865x_isLocalPublicIp(extIp)==TRUE)
		{

		}
		else
		{
			rtl8651_setAsicOperationLayer(2);
		}
		#else
		rtl8651_setAsicOperationLayer(2);
		#endif
	}

	return SUCCESS;
	
}

static rtl865x_ip_entry_t* _rtl865x_getIpEntryByExtIp(ipaddr_t extIp)
{
	int32 i;
	rtl865x_ip_entry_t *entry = NULL;
	
	for(i = 0; i < IP_NUMBER; i++)
		if(rtl865x_ipTable[i].valid == 1 && rtl865x_ipTable[i].extIp == extIp)
		{
			entry = &rtl865x_ipTable[i];
			break;
		}
	return entry;
	
}

/*
@func rtl865x_ip_entry_t* | rtl865x_getIpEntryByIp | get ip entry .
@parm ipaddr_t | extIp | ip address
@rvalue ip_entry | Success.
@rvalue NULL | Failed
*/
rtl865x_ip_entry_t* rtl865x_getIpEntryByIp(ipaddr_t extIp)
{
	return _rtl865x_getIpEntryByExtIp(extIp);
}


/*
@func int32 | rtl865x_getIpIdxByExtIp | get asic idex .
@parm ipaddr_t | extIp | ip address
@parm int32* | idx | index
@rvalue SUCCESS | Success.
@rvalue FAILED | Failed
*/
int32 rtl865x_getIpIdxByExtIp(ipaddr_t extIp, int32 *idx)
{
	int32 retval = FAILED;
	rtl865x_ip_entry_t *entry = NULL;

	entry = _rtl865x_getIpEntryByExtIp(extIp);
	if(entry)
	{
		if(idx)
			*idx = IP_TABLE_INDEX(entry);
		retval = SUCCESS;
	}

	return retval;
}

/*
@func int32 | rtl865x_addIp | add ip table entry.
@parm ipaddr_t | intIp | internal ip address
@parm ipaddr_t | extIp | external ip address
@parm uint32 | ip_type | entry type. support IP_TYPE_NAPT/IP_TYPE_NAT/IP_TYPE_LOCALSERVER
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDINPUT | Invalid input.
@rvalue RTL_EENTRYALREADYEXIST | entry already exist.
@rvalue RTL_ENOFREEBUFFER | not enough buffer in System.
@rvalue FAILED | Failed.
@comm
the extIp is the primary key of the ip table.
*/
int32 rtl865x_addIp(ipaddr_t intIp, ipaddr_t extIp, uint32 ip_type, char* output_netif)
{
	int32 retval = FAILED;
	unsigned long flags;	
	//rtl_down_interruptible(&ip_sem);
	local_irq_save(flags);
	retval = _rtl865x_addIp(intIp, extIp, ip_type,output_netif);
	//rtl_up(&ip_sem);
	local_irq_restore(flags);
	return retval;

}

/*
@func int32 | rtl865x_delIp | delete ip table entry.
@parm ipaddr_t | extIp | external ip address
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDINPUT | Invalid input.
@rvalue RTL_EENTRYNOTFOUND | not found.
@rvalue FAILED | Failed.
@comm
the extIp is the primary key of the ip table.
*/
int32 rtl865x_delIp(ipaddr_t extIp)
{
	int32 retval = FAILED;
	unsigned long flags;
	//rtl_down_interruptible(&ip_sem);
	local_irq_save(flags);
	retval = _rtl865x_delIp(extIp);
	//rtl_up(&ip_sem);
	local_irq_restore(flags);
	return retval;
	
}

int32 rtl865x_getIPIdx(rtl865x_ip_entry_t *entry, int32 *idx)
{
	if(idx)
	{
		*idx =IP_TABLE_INDEX(entry);
		return SUCCESS;
	}
	return FAILED;
}

#ifdef CONFIG_RTL_MULTI_ETH_WAN
rtl865x_ip_entry_t* rtl865x_getIpEntryByIdx(int32 idx)
{
	if ((idx < IP_NUMBER) && (rtl865x_ipTable[idx].valid == 1))
		return &rtl865x_ipTable[idx];

	return NULL;
}
#endif

#ifdef CONFIG_RTL_HARDWARE_NAT  
char masq_if[IP_NUMBER][IFNAMSIZ] = { "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0"};
unsigned int hw_napt_ip[IP_NUMBER] = {0};
unsigned int masq_index=0;
#endif

int32 add_masq_intf(char *name){
	int i;

	for(i=0; i<IP_NUMBER; i++){
		if(!strcmp(masq_if[i], name))
				return SUCCESS;
	}	

	for(i=0; i<IP_NUMBER; i++){
		if(masq_if[i][0]=='\0'){
			strcpy(masq_if[i], name);
			return SUCCESS;
		}
	}
	return FAILED; 
}

int32 del_masq_intf(char *name){
	int i;
	for(i=0; i<IP_NUMBER; i++){
		if(!strcmp(masq_if[i], name))
			masq_if[i][0] = '\0';
	}
	return SUCCESS;
}

int32 flush_masq_intf(void){
	int i;
	for(i=0; i<IP_NUMBER; i++){
		masq_if[i][0] = '\0';
	}
	return SUCCESS;
}

int32 check_intf_in_masq_intf(char *name){
	int i;
	for(i=0; i<IP_NUMBER; i++){
		if(!strcmp(masq_if[i], name))
			return SUCCESS;
	}
	return FAILED;
}

void dump_masq_if(void){
	int i;
	for(i=0; i<IP_NUMBER; i++){
		if(masq_if[i][0]!='\0')
			printk("idx%d  %s\n", i, masq_if[i]);
	}
}


