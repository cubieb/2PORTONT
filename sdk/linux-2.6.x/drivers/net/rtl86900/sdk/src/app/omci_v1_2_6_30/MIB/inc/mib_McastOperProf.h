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
 * Purpose : Definition of ME attribute: Multicast operations profile (309)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Multicast operations profile (309)
 */

#ifndef __MIB_MCASTOPERPROF_TABLE_H__
#define __MIB_MCASTOPERPROF_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table SWImage attribute len, only string attrubutes have length definition */
#define MIB_TABLE_DYACLTABLE_LEN (24)
#define MIB_TABLE_STATICACLTABLE_LEN (24)
#define MIB_TABLE_LOSTGROUPLISTTABLE_LEN (10)
#define MIB_TABLE_DSIGMPANDMULTICASTTCI_LEN (3)

/* Table McastOperProf attribute index */
#define MIB_TABLE_MCASTOPERPROF_ATTR_NUM (17)
#define MIB_TABLE_MCASTOPERPROF_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_MCASTOPERPROF_IGMPVERSION_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_MCASTOPERPROF_IGMPFUN_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_MCASTOPERPROF_IMMEDIATELEAVE_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_MCASTOPERPROF_USIGMPTCI_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_MCASTOPERPROF_USIGMPTAGCONTROL_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_MCASTOPERPROF_USIGMPRATE_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_MCASTOPERPROF_DYACLTABLE_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_MCASTOPERPROF_STATICACLTABLE_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_MCASTOPERPROF_LOSTGROUPLISTTABLE_INDEX ((MIB_ATTR_INDEX)10)
#define MIB_TABLE_MCASTOPERPROF_ROBUSTNESS_INDEX ((MIB_ATTR_INDEX)11)
#define MIB_TABLE_MCASTOPERPROF_QUERIERIPADDRESS_INDEX ((MIB_ATTR_INDEX)12)
#define MIB_TABLE_MCASTOPERPROF_QUERYINTERVAL_INDEX ((MIB_ATTR_INDEX)13)
#define MIB_TABLE_MCASTOPERPROF_QUERYMAXRESPONSETIME_INDEX ((MIB_ATTR_INDEX)14)
#define MIB_TABLE_MCASTOPERPROF_LASTMEMBERQUERYINTERVAL_INDEX ((MIB_ATTR_INDEX)15)
#define MIB_TABLE_MCASTOPERPROF_UNAUTHORIZEDJOINREQUESTBEHAVIOUR_INDEX ((MIB_ATTR_INDEX)16)
#define MIB_TABLE_MCASTOPERPROF_DOWNSTREAMIGMPMULTICASTTCI_INDEX ((MIB_ATTR_INDEX)17)

#define LEADING_12BYTE_ADDR_LEN (12)

enum
{
	PASS,
	STRIP_TAG,
	ADD_TAG,
	REPLACE_TAG,
	REPLACE_VID,
	ADD_VIDUNI_TAG,
	REPLACE_VIDUNI_TAG,
	REPLACE_VID2VIDUNI,
	RESERVED,
};

enum
{
	ROW_PART_0,
	ROW_PART_1,
	ROW_PART_2,
};

typedef struct rowPart0_s
{
	UINT16 gemId;
	UINT16 aniVid;
	UINT32 sip;
	UINT32 dipStartRange;
	UINT32 dipEndRange;
	UINT32 ImputedGrpBw;
	UINT16 resv;
} __attribute__((packed)) rowPart0_t;

typedef struct rowPart1_s
{
	UINT8 ipv6Sip[LEADING_12BYTE_ADDR_LEN];
	UINT16 previewLen;
	UINT16 previewRepeatTime;
	UINT16 previewRepeatCnt;
	UINT16 previewResetTime;
	UINT16 resv;
} __attribute__((packed)) rowPart1_t;

typedef struct rowPart2_s
{
	UINT8 ipv6Dip[LEADING_12BYTE_ADDR_LEN];
	UINT8 resv[10];
} __attribute__((packed)) rowPart2_t;

typedef struct omci_acl_raw_entry_s
{
#ifdef OMCI_X86
	union
	{
		struct
		{
			UINT16 rowKey : 10;
			UINT16 test : 1;
			UINT16 rowPartId : 3;
			UINT16 setCtrl: 2;
		} bit;
		UINT16 val;
	} tableCtrl;
#else
    union
    {
        struct
        {
            UINT16 setCtrl: 2;
            UINT16 rowPartId : 3;
            UINT16 test : 1;
            UINT16 rowKey : 10;
        } bit;
        UINT16 val;
    } tableCtrl;
#endif
	union
	{
        rowPart0_t rowPart0;
        rowPart1_t rowPart1;
		rowPart2_t rowPart2;
	} rowPart;
} __attribute__((packed)) omci_acl_raw_entry_t;

typedef struct omci_acl_gem_vlan_entry_s
{
	UINT16 gemId;
	UINT16 aniVid;
	UINT16 refCnt;
} __attribute__((packed)) omci_acl_gem_vlan_entry_t;

typedef struct
{
	UINT16 vid;
	UINT32 sip;			//four least significant bytes for IPv6
	UINT32 mcastDip;	//four least significant bytes for IPv6
} __attribute__((aligned)) omci_lost_raw_entry_t;

typedef struct aclTableEntry_s
{
	omci_acl_raw_entry_t tableEntry;
	LIST_ENTRY(aclTableEntry_s) entries;
} __attribute__((aligned)) aclTableEntry_t;

typedef struct aclGemVlanEntry_s
{
	omci_acl_gem_vlan_entry_t tableEntry;
	LIST_ENTRY(aclGemVlanEntry_s) entries;
} __attribute__((aligned)) aclGemVlanEntry_t;

typedef struct lostGrpTableEntry_s
{
	omci_lost_raw_entry_t tableEntry;
	LIST_ENTRY(lostGrpTableEntry_s) entries;
} __attribute__((aligned)) lostGrpTableEntry_t;

LIST_HEAD(aclHead, aclTableEntry_s);
LIST_HEAD(aclGemVlanHead, aclGemVlanEntry_s);

/* Table McastOperProf attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
	UINT8    IGMPVersion;
	UINT8    IGMPFun;
	UINT8    ImmediateLeave;
	UINT16   UsIGMPTci;
	UINT8    UsIGMPTagControl;
	UINT32   UsIGMPRate;
	UINT8    DyACLTable[MIB_TABLE_DYACLTABLE_LEN];
	UINT8    StaticACLTable[MIB_TABLE_STATICACLTABLE_LEN];
	UINT8    LostGroupListTable[MIB_TABLE_LOSTGROUPLISTTABLE_LEN];
	UINT8    Robustness;
	UINT32   QuerierIpAddress;
	UINT32   QueryInterval;
	UINT32   QueryMaxResponseTime;
	UINT32   LastMemberQueryInterval;
	UINT8    UnauthorizedJoinRequestBehaviour;
	UINT8    DownstreamIgmpMulticastTci[MIB_TABLE_DSIGMPANDMULTICASTTCI_LEN];
	UINT16	 curDaclCnt;
	UINT16	 curSaclCnt;
	struct aclHead DACLhead;
	struct aclGemVlanHead dAclGemVlanHead;
	struct aclHead SACLhead;
	LIST_HEAD(lostHead, lostGrpTableEntry_s) LOSThead;
} __attribute__((aligned)) MIB_TABLE_MCASTOPERPROF_T;


#ifdef __cplusplus
}
#endif

#endif
