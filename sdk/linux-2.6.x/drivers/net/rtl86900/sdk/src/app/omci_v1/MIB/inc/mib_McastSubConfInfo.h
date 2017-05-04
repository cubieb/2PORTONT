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
 * Purpose : Definition of ME attribute: Multicast subscriber config info (310)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Multicast subscriber config info (310)
 */

#ifndef __MIB_MCASTSUBCONFINFO_TABLE_H__
#define __MIB_MCASTSUBCONFINFO_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MIB_TABLE_SVCPKGTBL_LEN (20)
#define MIB_TABLE_ALLOWPREVIEWGRPTBL_LEN (22)
#define MIB_TABLE_PREVIEWGRPIPADDR_LEN (16)

/* Table McastSubConfInfo attribute index */
#define MIB_TABLE_MCASTSUBCONFINFO_ATTR_NUM (8)
#define MIB_TABLE_MCASTSUBCONFINFO_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_MCASTSUBCONFINFO_METYPE_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_MCASTSUBCONFINFO_MCASTOPERPROFPTR_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_MCASTSUBCONFINFO_MAXSIMGROUPS_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_MCASTSUBCONFINFO_MAXMCASTBANDWIDTH_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_MCASTSUBCONFINFO_BANDWIDTHENFORCEMENT_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_MCASTSUBCONFINFO_MCASTSVCPKGTABLE_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_MCASTSUBCONFINFO_ALLOWPREVIEWGRPTABLE_INDEX ((MIB_ATTR_INDEX)8)

enum {
	IGMP_SNP,
	IGMP_SPR,
	IGMP_PROXY,
};

typedef struct omci_mcast_svc_pkg_raw_entry_s
{
#ifdef OMCI_X86
	union
	{
		struct
		{
			UINT16 rowKey : 10;
			UINT16 reserved : 4;
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
            UINT16 reserved : 4;
            UINT16 rowKey : 10;
        } bit;
        UINT16 val;
    } tableCtrl;
#endif
	UINT16 vidUni;
	UINT16 maxSimGroups;
	UINT32 maxMcastBw; 
	UINT16 mcastOperProfPtr; 
	UINT8 resv[8];
} __attribute__((packed)) omci_mcast_svc_pkg_raw_entry_t;

typedef struct part0_s
{
	UINT8 srcIpAddr[MIB_TABLE_PREVIEWGRPIPADDR_LEN];
	UINT16 vidAni;
	UINT16 vidUni;    
} __attribute__((packed)) part0_t;

typedef struct part1_s
{
	UINT8	dstIpAddr[MIB_TABLE_PREVIEWGRPIPADDR_LEN];
	UINT16	duration;
	UINT16	timeleft;
} __attribute__((packed)) part1_t;

typedef struct omci_allowed_preview_group_table_entry_s
{
#ifdef OMCI_X86
	union
	{
		struct
		{
			UINT16 rowKey : 10;
			UINT16 rsv : 1;
			UINT16 rowPart : 3;
			UINT16 setCtrl : 2;
		} bit;
		UINT16 val;
	} tableCtrl;
#else
    union
    {
        struct
        {
            UINT16 setCtrl : 2;
            UINT16 rowPart : 3;
			UINT16 rsv : 1;
            UINT16 rowKey : 10;
        } bit;
        UINT16 val;
    } tableCtrl;
#endif
	//based on rowPart 
    union
    {   
        part0_t part0;
        part1_t part1;
    } row;
} __attribute__((packed)) omci_allowed_preview_group_table_entry_t;

typedef struct mopTableEntry_s
{
	omci_mcast_svc_pkg_raw_entry_t tableEntry;
	LIST_ENTRY(mopTableEntry_s) entries;
} __attribute__((aligned)) mopTableEntry_t;

typedef struct allowPreviewGrpTblEntry_s
{
	omci_allowed_preview_group_table_entry_t tableEntry;
	LIST_ENTRY(allowPreviewGrpTblEntry_s) entries;
} __attribute__((aligned)) allowPreviewGrpTblEntry_t;


/* Table McastSubConfInfo attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
	UINT8    MeType;
	UINT16   McastOperProfPtr;
	UINT16   MaxSimGroups;
	UINT32   MaxMcastBandwidth;
	UINT8    BandwidthEnforcement;
	UINT8    McastSvcPkgTbl[MIB_TABLE_SVCPKGTBL_LEN];
	UINT8	 AllowPreviewGroupsTbl[MIB_TABLE_ALLOWPREVIEWGRPTBL_LEN];
	UINT32	 curMopCnt;
	UINT32	 curPreGrpCnt;
	LIST_HEAD(mopTableEntryHead_t, mopTableEntry_s) MOPhead;
	LIST_HEAD(allowPreviewGrpTblEntryHead_t, allowPreviewGrpTblEntry_s) allowPreviewGrpHead;
} __attribute__((aligned)) MIB_TABLE_MCASTSUBCONFINFO_T;


#ifdef __cplusplus
}
#endif

#endif
