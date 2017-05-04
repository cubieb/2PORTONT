/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.
* 
* Program : network interface driver header file
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/

#ifndef RTL865X_NETIF_LOCAL_H
#define RTL865X_NETIF_LOCAL_H

#include <net/rtl/rtl865x_netif.h>
#if !defined(REDUCE_MEMORY_SIZE_FOR_16M)
#define REDUCE_MEMORY_SIZE_FOR_16M
#endif



#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
typedef struct _rtl865x_acl_chainlist_s
{
	struct list_head 	AclChainListHEAD;	/* The list head of acl chains */
	int	ref_count;
	int	vlan_id;

	struct list_head 	AclChainList;
}rtl865x_acl_chain_chainlist_t;
typedef struct _rtl865x_acl_chain_s
{
	int chain_priority; 					/*  chain priority in netif's acl chain list */
	int reserved_guarantee_minumun;		/*  (reserved_end-reserved_start)	must  >= reserved_guarantee_minumun  */
	int reserved_start;					/*  start index in asic acl table */
	int reserved_end;						/*  end index in asic acl table */
	int acl_rules_num;					/* (reserved_end-reserved_start)   must  >= acl_rules_num */
	struct list_head NetifAclChainList;
	struct list_head AclChainLayoutList;
	struct list_head AclRuleListHEAD;
	
	/* private used */
	int _arrange_start_;
	int _arrange_end_;
}rtl865x_acl_chain_t;
#endif

/*the following fields are invalid when the interface is slave interface:
* inAclStart, inAclEnd, outAclStart, outAclEnd,asicIdx,chainListHead
*/
typedef struct rtl865x_netif_local_s
{
	uint16 	vid; /*netif->vid*/
	uint16 	mtu; /*netif's MTU*/	
	uint16 	macAddrNumber; /*how many continuous mac is attached*/
	uint16 	inAclStart, inAclEnd, outAclStart, outAclEnd; /*acl index*/
	uint16 	enableRoute; /*enable route*/
	uint32 	valid:1,	/*valid?*/
		if_type:5, /*interface type, IF_ETHER, IF_PPPOE*/
		//refCnt:5, /*referenc count by other table entry*/
		asicIdx:4,
		is_wan:1, /*this interface is wan?*/
		is_defaultWan:1, /*if there is multiple wan interface, which interface is default wan*/
		dmz:1,	/*dmz interface?*/
		is_slave:1; /*is slave interface*/
	
	ether_addr_t macAddr;
	uint8	name[MAX_IFNAMESIZE];
#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
	struct list_head* 	NetifAclChainListHEAD_ptr;
#endif //CONFIG_RTL_LAYERED_DRIVER_ACL
	struct rtl865x_netif_local_s *master; /*point master interface when this interface is slave interface*/
#ifdef CONFIG_RTL8676_Static_ACL
	__be32 ipv4_addr;
#endif
}rtl865x_netif_local_t;

#define	RTL_ACL_INGRESS	0
#define	RTL_ACL_EGRESS	1

#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
typedef struct rtl865x_aclBuf_s
{
	int16 totalCnt;
	int16 freeCnt;
	rtl865x_AclRule_t *freeHead;
}rtl865x_aclBuf_t;
#endif


int32 rtl865x_enableNetifRouting(rtl865x_netif_local_t *netif);
int32 rtl865x_disableNetifRouting(rtl865x_netif_local_t *netif);
rtl865x_netif_local_t *_rtl865x_getNetifByName(char *name);
rtl865x_netif_local_t *_rtl865x_getSWNetifByName(char * name);
rtl865x_netif_local_t *_rtl865x_getDefaultWanNetif(void);
int32 _rtl865x_setDefaultWanNetif(char *name);
int32 _rtl865x_clearDefaultWanNetif(char * name);
//int32 _rtl865x_getNetifIdxByNameExt(uint8 *name);
char* _rtl865x_getNetifNameByidx(int32 idx);

#define ForEachMasterNetif_Declaration\
		extern rtl865x_netif_local_t * getNetifTbl(void);\
		rtl865x_netif_local_t *netif_table = getNetifTbl();\
		int netif_idx;

#define ForEachMasterNetif_Start\
		for(netif_idx=0;netif_idx<NETIF_SW_NUMBER;netif_idx++)\
		{\
			rtl865x_netif_local_t *netif = &(netif_table[netif_idx]);\
			if(netif_table[netif_idx].valid == 0 || netif_table[netif_idx].is_slave)\
				continue;
				
#define ForEachMasterNetif_End\
		}


#define ForEachMasterWanNetif_Declaration\
		extern rtl865x_netif_local_t * getNetifTbl(void);\
		rtl865x_netif_local_t *netif_table = getNetifTbl();\
		int netif_idx;

#define ForEachMasterWanNetif_Start\
		for(netif_idx=0;netif_idx<NETIF_SW_NUMBER;netif_idx++)\
		{\
			rtl865x_netif_local_t *netif = &(netif_table[netif_idx]);\
			if(netif_table[netif_idx].valid == 0 || netif_table[netif_idx].is_slave || rtl865x_netif_is_wan(netif_table[netif_idx].name)==0)\
				continue;
				
#define ForEachMasterWanNetif_End\
		}

#endif
