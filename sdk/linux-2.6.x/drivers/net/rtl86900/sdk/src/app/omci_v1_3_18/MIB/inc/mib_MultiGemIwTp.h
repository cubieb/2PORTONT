/*
 * Copyright (C) 2014 Realtek Semiconductor Corp. 
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated, 
 * modified or distributed under the authorized license from Realtek. 
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER 
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED. 
 *
 * Purpose : Definition of ME attribute: Multicast GEM IWTP (281)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Multicast GEM IWTP (281)
 */

#ifndef __MIB_MULTIGEMIWTP_TABLE_H__
#define __MIB_MULTIGEMIWTP_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table MultiGemIwTp attribute index */
#define MIB_TABLE_MULTIGEMIWTP_ATTR_NUM (11)
#define MIB_TABLE_MULTIGEMIWTP_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_MULTIGEMIWTP_GEMCTPPTR_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_MULTIGEMIWTP_IWOPT_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_MULTIGEMIWTP_SERVPROPTR_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_MULTIGEMIWTP_IWTPPTR_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_MULTIGEMIWTP_PPTPCOUNTER_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_MULTIGEMIWTP_OPSTATE_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_MULTIGEMIWTP_GALPROFPTR_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_MULTIGEMIWTP_GALLOOPBACKCFG_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_MULTIGEMIWTP_IPV4MCASTADDRTABLE_INDEX ((MIB_ATTR_INDEX)10)
#define MIB_TABLE_MULTIGEMIWTP_IPV6MCASTADDRTABLE_INDEX ((MIB_ATTR_INDEX)11)



/* Table MultiGemIwTp attribute len, only string attrubutes have length definition */
#define MIB_TABLE_MULTIGEMIWTP_IPV4MCASTADDRTABLE_LEN (12)
#define MIB_TABLE_MULTIGEMIWTP_MSB_LEN (12)
#define MIB_TABLE_MULTIGEMIWTP_IPV6MCASTADDRTABLE_LEN (24)

typedef struct omci_multicast_addr_entry_s
{
	UINT16 gemPort;
	UINT16 secKey;
	UINT32 dstAddrStart;
	UINT32 dstAddrStop;
} __attribute__((packed)) omci_multicast_addr_entry_t;

typedef struct omci_ipv4_multicast_addr_entry_s
{
	omci_multicast_addr_entry_t ipv4Entry;
} __attribute__((packed)) omci_ipv4_multicast_addr_entry_t;

typedef struct omci_ipv6_multicast_addr_entry_s
{
	omci_multicast_addr_entry_t ipv6Entry;
	UINT8 msbAddr[MIB_TABLE_MULTIGEMIWTP_MSB_LEN];
} __attribute__((packed)) omci_ipv6_multicast_addr_entry_t;


typedef struct multiGemIwTpIpv4TableEntry_s
{
	omci_ipv4_multicast_addr_entry_t tableEntry;
	LIST_ENTRY(multiGemIwTpIpv4TableEntry_s) entries;
} __attribute__((aligned)) multiGemIwTpIpv4TableEntry_t;

typedef struct multiGemIwTpIpv6TableEntry_s
{
	omci_ipv6_multicast_addr_entry_t tableEntry;	
	LIST_ENTRY(multiGemIwTpIpv6TableEntry_s) entries;
} __attribute__((aligned)) multiGemIwTpIpv6TableEntry_t;


// Table MultiGemIwTp entry stucture
typedef struct {
    UINT16 EntityID;
    UINT16 GemCtpPtr;
    UINT8  IwOpt;
    UINT16 ServProPtr;
    UINT16 IwTpPtr;	///Not Used 1 for G.988
    UINT8  PptpCounter;
    UINT8  OpState;
    UINT16 GalProfPtr;
    UINT8  GalLoopbackCfg; //Not Used 2 for G.988
    UINT8  Ipv4MCastAddrTable[MIB_TABLE_MULTIGEMIWTP_IPV4MCASTADDRTABLE_LEN];
    UINT8  Ipv6MCastAddrTable[MIB_TABLE_MULTIGEMIWTP_IPV6MCASTADDRTABLE_LEN];
	UINT32 curIpv4EntryCnt;
	UINT32 curIpv6EntryCnt;
    LIST_HEAD(multiGemIwTpIpv4Head, multiGemIwTpIpv4TableEntry_s) ipv4Head;
    LIST_HEAD(multiGemIwTpIpv6Head, multiGemIwTpIpv6TableEntry_s) ipv6Head;
} __attribute__((aligned)) MIB_TABLE_MULTIGEMIWTP_T;


#ifdef __cplusplus
}
#endif

#endif
