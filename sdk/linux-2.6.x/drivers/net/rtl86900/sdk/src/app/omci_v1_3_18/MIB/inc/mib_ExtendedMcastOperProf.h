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

#ifndef __MIB_EXTENDEDMCASTOPERPROF_TABLE_H__
#define __MIB_EXTENDEDMCASTOPERPROF_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_EXTDYACLTABLE_LEN (30)
#define MIB_TABLE_EXTSTATICACLTABLE_LEN (30)
#define MIB_TABLE_EXTLOSTGROUPLISTTABLE_LEN (16)
#define MIB_TABLE_EXTDSIGMPANDMULTICASTTCI_LEN (3)
#define MIB_TABLE_EXTQUERIERIPADDR_LEN (16)


/* Table extMcastOperProf attribute index */
#define MIB_TABLE_EXTMCASTOPERPROF_ATTR_NUM (17)
#define MIB_TABLE_EXTMCASTOPERPROF_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_EXTMCASTOPERPROF_IGMPVERSION_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_EXTMCASTOPERPROF_IGMPFUN_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_EXTMCASTOPERPROF_IMMEDIATELEAVE_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_EXTMCASTOPERPROF_USIGMPTCI_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_EXTMCASTOPERPROF_USIGMPTAGCONTROL_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_EXTMCASTOPERPROF_USIGMPRATE_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_EXTMCASTOPERPROF_DYACLTABLE_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_EXTMCASTOPERPROF_STATICACLTABLE_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_EXTMCASTOPERPROF_LOSTGROUPLISTTABLE_INDEX ((MIB_ATTR_INDEX)10)
#define MIB_TABLE_EXTMCASTOPERPROF_ROBUSTNESS_INDEX ((MIB_ATTR_INDEX)11)
#define MIB_TABLE_EXTMCASTOPERPROF_QUERIERIPADDRESS_INDEX ((MIB_ATTR_INDEX)12)
#define MIB_TABLE_EXTMCASTOPERPROF_QUERYINTERVAL_INDEX ((MIB_ATTR_INDEX)13)
#define MIB_TABLE_EXTMCASTOPERPROF_QUERYMAXRESPONSETIME_INDEX ((MIB_ATTR_INDEX)14)
#define MIB_TABLE_EXTMCASTOPERPROF_LASTMEMBERQUERYINTERVAL_INDEX ((MIB_ATTR_INDEX)15)
#define MIB_TABLE_EXTMCASTOPERPROF_UNAUTHORIZEDJOINREQUESTBEHAVIOUR_INDEX ((MIB_ATTR_INDEX)16)
#define MIB_TABLE_EXTMCASTOPERPROF_DOWNSTREAMIGMPMULTICASTTCI_INDEX ((MIB_ATTR_INDEX)17)

typedef struct format0_s
{
	UINT16 gemId;
	UINT16 aniVid;
	UINT32 ImputedGrpBw;
	UINT16 previewLen;
	UINT16 previewRepeatTime;
	UINT16 previewRepeatCnt;
	UINT16 previewResetTime;
	UINT32 vendorSpecificUse;
	UINT8 resv[8];	    
} __attribute__((packed)) format0_t;

typedef struct format1_s
{
	UINT8 dstIpAddrStartRange[16];
	UINT8 resv[12];	    
} __attribute__((packed)) format1_t;

typedef struct format2_s
{
	UINT8 dstIpAddrEndRange[16];
	UINT8 resv[12];	    
} __attribute__((packed)) format2_t;

typedef struct format3_s
{
	UINT8 resv[28];	    
} __attribute__((packed)) format3_t;

typedef struct omci_ext_acl_raw_entry_s
{
#ifdef OMCI_X86
	union
	{
		struct
		{
			UINT16 rowKey : 10;
			UINT16 rsv : 2;
			UINT16 rowPartId : 2;
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
            UINT16 rowPartId : 2;
            UINT16 rsv : 2;
            UINT16 rowKey : 10;
        } bit;
        UINT16 val;
    } tableCtrl;
#endif

	//based on rowPart 
    union
    {   
        format0_t format0;
        format1_t format1;
		format2_t format2;
		format3_t format3;
    } row;
} __attribute__((packed)) omci_ext_acl_raw_entry_t;

typedef struct omci_ext_lost_raw_entry_s
{
	UINT8    destIp[MIB_TABLE_EXTQUERIERIPADDR_LEN];
} __attribute__((aligned)) omci_ext_lost_raw_entry_t;

typedef struct extAclTableEntry_s
{	
	omci_ext_acl_raw_entry_t tableEntry;
	LIST_ENTRY(extAclTableEntry_s) entries;
} __attribute__((aligned)) extAclTableEntry_t;

typedef struct extLostGrpTableEntry_s
{
	omci_ext_lost_raw_entry_t tableEntry;	
	LIST_ENTRY(extLostGrpTableEntry_s) entries;
} __attribute__((aligned)) extLostGrpTableEntry_t;

LIST_HEAD(extAclHead, extAclTableEntry_s);

/* Table ExtMcastOperProf attribute len, only string attrubutes have length definition */
typedef struct
{
	UINT16   EntityId;
	UINT8    IGMPVersion;
	UINT8    IGMPFun;
	UINT8    ImmediateLeave;
	UINT16   UsIGMPTci;
	UINT8    UsIGMPTagControl;
	UINT32   UsIGMPRate;
	UINT8    DyACLTable[MIB_TABLE_EXTDYACLTABLE_LEN];
	UINT8    StaticACLTable[MIB_TABLE_EXTSTATICACLTABLE_LEN];
	UINT8    LostGroupListTable[MIB_TABLE_EXTLOSTGROUPLISTTABLE_LEN];
	UINT8    Robustness;
	UINT8    QuerierIpAddress[MIB_TABLE_EXTQUERIERIPADDR_LEN];
	UINT32   QueryInterval;
	UINT32   QueryMaxResponseTime;
	UINT32   LastMemberQueryInterval;
	UINT8    UnauthorizedJoinRequestBehaviour;
	UINT8    DownstreamIgmpMulticastTci[MIB_TABLE_DSIGMPANDMULTICASTTCI_LEN];
	UINT16	 curDaclCnt;
	UINT16	 curSaclCnt;
	struct extAclHead DACLhead;
	struct extAclHead SACLhead;
	LIST_HEAD(extLostHead, extLostGrpTableEntry_s) LOSThead;
}__attribute__((aligned)) MIB_TABLE_EXTMCASTOPERPROF_T;

#ifdef __cplusplus
}
#endif

#endif
