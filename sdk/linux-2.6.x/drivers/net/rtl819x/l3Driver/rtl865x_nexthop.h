/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.
* 
* Program : nexthop table driver
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/
#ifndef	RTL865X_NEXTHOP_H
#define	RTL865X_NEXTHOP_H

#if !defined(REDUCE_MEMORY_SIZE_FOR_16M)
#define REDUCE_MEMORY_SIZE_FOR_16M
#endif


typedef struct rtl865x_nextHopEntry_s 
{
	char dstNetif[IFNAMSIZ];	/* dst network interface*/
	uint32	nexthop;			/* ethernet: nexthop ip address, pppoe: session Id */
	//rtl865x_ip_entry_t *srcIp_t;		/* for nexthop source ip table index, it's invalid  now*/
	uint32 srcIp;		/* for nexthop source ip (IG)*/

	uint16 valid:1,		/* 0: Invalid, 1: Invalid */	
		nextHopType:3,			/* IF_ETHER, IF_PPPOE */
		flag:3;			/* bit0:referenced by l3, bit2:referenced by acl */

	uint16	refCnt;			/* Reference Count */
	uint32	entryIndex;			/* Entry Index */

} rtl865x_nextHopEntry_t;

#define NEXTHOP_L3					0x01
#define NEXTHOP_DEFREDIRECT_ACL	0x02
#ifndef CONFIG_RTL_MULTI_ETH_WAN
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
#define NXTHOP_ENTRY_NUM	16
#else
#if defined(REDUCE_MEMORY_SIZE_FOR_16M)
#define NXTHOP_ENTRY_NUM	4
#else
#define NXTHOP_ENTRY_NUM	16
#endif
#endif
#else
#define NXTHOP_ENTRY_NUM	32
#endif
int32 rtl865x_initNxtHopTable(void);
int32 rtl865x_reinitNxtHopTable(void);
int32 rtl865x_addNxtHop(uint32 attr, void *ref_ptr, char *if_name, uint32 nexthop,uint32 srcIp);
int32 rtl865x_delNxtHop(uint32 attr, uint32 entryIdx);
rtl865x_nextHopEntry_t * rtl865x_getNexthopList(void);

#if defined(CONFIG_RTL_LOCAL_PUBLIC) || defined(CONFIG_RTL_MULTI_ETH_WAN)
int32 rtl865x_getNxtHopIdx(uint32 attr, char *netif_name, uint32 nexthop,uint32 srcIp);
int32 rtl865x_delNxtHopBySessionID(uint32 attr, char *netif_name, uint32 session);
#endif

#endif

