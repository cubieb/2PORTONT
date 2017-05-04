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
 * Purpose : Definition of ME attribute: Extended VLAN tagging operation configuration data (171)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Extended VLAN tagging operation configuration data (171)
 */

#ifndef __MIB_EXTVLANTAGOPERCFGDATA_TABLE_H__
#define __MIB_EXTVLANTAGOPERCFGDATA_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Other */
#define MIB_TABLE_RECEIVEDFRAMEVLANTAGGINGOPERTABLE_LEN (16)
#define MIB_TABLE_DSCPTOPBITMAPPING_LEN (24)

/* Table ExtVlanTagOperCfgData attribute index */
#define MIB_TABLE_EXTVLANTAGOPERCFGDATA_ATTR_NUM (9)
#define MIB_TABLE_EXTVLANTAGOPERCFGDATA_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_EXTVLANTAGOPERCFGDATA_ASSOCIATIONTYPE_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_EXTVLANTAGOPERCFGDATA_RECEIVEDFRAMEVLANTAGOPERTABLEMAXSIZE_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_EXTVLANTAGOPERCFGDATA_INPUTTPID_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_EXTVLANTAGOPERCFGDATA_OUTPUTTPID_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_EXTVLANTAGOPERCFGDATA_DSMODE_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_EXTVLANTAGOPERCFGDATA_RECEIVEDFRAMEVLANTAGGINGOPERTABLE_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_EXTVLANTAGOPERCFGDATA_ASSOCIATEDMEPOINT_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_EXTVLANTAGOPERCFGDATA_DSCPTOPBITMAPPING_INDEX ((MIB_ATTR_INDEX)9)

typedef struct
{
#ifdef OMCI_X86
    union
    {
        struct
        {
            UINT32 word1Pad : 12;
            UINT32 filterOuterTpId : 3;
            UINT32 filterOuterVid : 13;
            UINT32 filterOuterPri : 4;
        } bit;
        UINT32 val;
    } outerFilterWord;

    union
    {
        struct
        {
            UINT32 filterEthType : 4;
            UINT32 word2Pad : 8;
            UINT32 filterInnerTpId : 3;
            UINT32 filterInnerVid : 13;
            UINT32 filterInnerPri : 4;
        } bit;
        UINT32 val;
    } innerFilterWord;

    union
    {
        struct
        {
            UINT32 treatmentOuterTpId : 3;
            UINT32 treatmentOuterVid : 13;
            UINT32 treatmentOuterPri: 4;
            UINT32 word3Pad : 10;
            UINT32 treatment : 2;
        } bit;
        UINT32 val;
    } outerTreatmentWord;

    union
    {
        struct
        {
            UINT32 treatmentInnerTpId : 3;
            UINT32 treatmentInnerVid : 13;
            UINT32 treatmentInnerPri : 4;
            UINT32 word4Pad : 12;
        } bit;
        UINT32 val;
    } innerTreatmentWord;
#else
    union
    {
        struct
        {
            UINT32 filterOuterPri : 4;
            UINT32 filterOuterVid : 13;
            UINT32 filterOuterTpId : 3;
            UINT32 word1Pad : 12;
        } bit;
        UINT32 val;
    } outerFilterWord;

    union
    {
        struct
        {
            UINT32 filterInnerPri : 4;
            UINT32 filterInnerVid : 13;
            UINT32 filterInnerTpId : 3;
            UINT32 word2Pad : 8;
            UINT32 filterEthType : 4;
        } bit;
        UINT32 val;
    } innerFilterWord;

    union
    {
        struct
        {
            UINT32 treatment : 2;
            UINT32 word3Pad : 10;
            UINT32 treatmentOuterPri: 4;
            UINT32 treatmentOuterVid : 13;
            UINT32 treatmentOuterTpId : 3;
        } bit;
        UINT32 val;
    } outerTreatmentWord;

    union
    {
        struct
        {
            UINT32 word4Pad : 12;
            UINT32 treatmentInnerPri : 4;
            UINT32 treatmentInnerVid : 13;
            UINT32 treatmentInnerTpId : 3;
        } bit;
        UINT32 val;
    } innerTreatmentWord;
#endif /* Big endian. */
} omci_extvlan_row_entry_t;


typedef struct extvlanTableEntry_s{

	omci_extvlan_row_entry_t tableEntry;
	LIST_ENTRY(extvlanTableEntry_s) entries;

}extvlanTableEntry_t;



/* Table ExtVlanTagOperCfgData attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
	UINT8    AssociationType;
	UINT16   ReceivedFrameVlanTagOperTableMaxSize;
	UINT16   InputTPID;
	UINT16   OutputTPID;
	UINT8    DsMode;
	UINT8    ReceivedFrameVlanTaggingOperTable[MIB_TABLE_RECEIVEDFRAMEVLANTAGGINGOPERTABLE_LEN];
	UINT16   AssociatedMePoint;
	UINT8    DscpToPbitMapping[MIB_TABLE_DSCPTOPBITMAPPING_LEN];
	UINT16	 curExtTableEntryCnt;
	LIST_HEAD(extvlanHead,extvlanTableEntry_s) head;
} __attribute__((aligned)) MIB_TABLE_EXTVLANTAGOPERCFGDATA_T;


typedef enum {
    EVTOCD_ASSOC_TYPE_MAC_BRIDGE_PORT       = 0,
    EVTOCD_ASSOC_TYPE_IEEE_8021P_MAPPER     = 1,
    EVTOCD_ASSOC_TYPE_PPTP_ETH_UNI          = 2,
    EVTOCD_ASSOC_TYPE_IP_HOST_IPV6_HOST     = 3,
    EVTOCD_ASSOC_TYPE_PPTP_XDSL_UNI         = 4,
    EVTOCD_ASSOC_TYPE_GEM_IWTP              = 5,
    EVTOCD_ASSOC_TYPE_MCAST_GEM_IWTP        = 6,
    EVTOCD_ASSOC_TYPE_PPTP_MOCA_UNI         = 7,
    EVTOCD_ASSOC_TYPE_ETH_FLOW_TP           = 9,
    EVTOCD_ASSOC_TYPE_VEIP                  = 10,
    EVTOCD_ASSOC_TYPE_MPLS_PSEUDO_TP        = 11,
} evtocd_attr_assoc_type_t;

typedef enum {
    EVTOCD_DS_MODE_INVERSE_ALL                  = 0,
    EVTOCD_DS_MODE_FORWARD_WITHOUT_INVERSE      = 1,
    EVTOCD_DS_MODE_INVERSE_VID_PBIT_OR_FORWARD  = 2,
    EVTOCD_DS_MODE_INVERSE_VID_OR_FORWARD       = 3,
    EVTOCD_DS_MODE_INVERSE_PBIT_OR_FORWARD      = 4,
    EVTOCD_DS_MODE_INVERSE_VID_PBIT_OR_DISCARD  = 5,
    EVTOCD_DS_MODE_INVERSE_VID_OR_DISCARD       = 6,
    EVTOCD_DS_MODE_INVERSE_PBIT_OR_DISCARD      = 7,
    EVTOCD_DS_MODE_DISCARD_ALL                  = 8,
} evtocd_attr_ds_mode_t;

#define EVTOCD_TBL_PRI_LEGAL_MIN     (0)
#define EVTOCD_TBL_PRI_LEGAL_MAX     (7)
#define EVTOCD_TBL_VID_LEGAL_MIN     (0)
#define EVTOCD_TBL_VID_LEGAL_MAX     (4094)

#define m_EVTOCD_TBL_PRI_IS_LEGAL(x) \
            ((x) >= EVTOCD_TBL_PRI_LEGAL_MIN && \
            (x) <= EVTOCD_TBL_PRI_LEGAL_MAX
#define m_EVTOCD_TBL_VID_IS_LEGAL(x) \
            ((x) >= EVTOCD_TBL_VID_LEGAL_MIN && \
            (x) <= EVTOCD_TBL_VID_LEGAL_MAX

enum {
    EVTOCD_TBL_FILTER_PRI_IGNORE_OTHER_FIELD    = 15,
    EVTOCD_TBL_FILTER_PRI_DEFAULT_TAG_RULE      = 14,
    EVTOCD_TBL_FILTER_PRI_DO_NOT_FILTER         = 8
};

enum {
    EVTOCD_TBL_FILTER_VID_DO_NOT_FILTER     = 4096,
};

enum {
    EVTOCD_TBL_FILTER_TPID_DO_NOT_FILTER        = 0,
    EVTOCD_TBL_FILTER_TPID_8100                 = 4,
    EVTOCD_TBL_FILTER_TPID_INPUT_TPID           = 5,
    EVTOCD_TBL_FILTER_TPID_INPUT_TPID_DEI_0     = 6,
    EVTOCD_TBL_FILTER_TPID_INPUT_TPID_DEI_1     = 7,
};

enum {
    EVTOCD_TBL_FILTER_ET_DO_NOT_FILTER      = 0,
    EVTOCD_TBL_FILTER_ET_IPOE_0800          = 1,
    EVTOCD_TBL_FILTER_ET_PPPOE_8863_8864    = 2,
    EVTOCD_TBL_FILTER_ET_ARP_0806           = 3,
    EVTOCD_TBL_FILTER_ET_IPV6_86DD          = 4,
};

enum {
    EVTOCD_TBL_TREATMENT_DISCARD_FRAME      = 3,
};

enum {
    EVTOCD_TBL_TREATMENT_PRI_COPY_FROM_INNER    = 8,
    EVTOCD_TBL_TREATMENT_PRI_COPY_FROM_OUTER    = 9,
    EVTOCD_TBL_TREATMENT_PRI_DERIVE_FROM_DSCP   = 10,
    EVTOCD_TBL_TREATMENT_PRI_DO_NOT_ADD_TAG     = 15,
};

enum {
    EVTOCD_TBL_TREATMENT_VID_COPY_FROM_INNER    = 4096,
    EVTOCD_TBL_TREATMENT_VID_COPY_FROM_OUTER    = 4097,
};

enum {
    EVTOCD_TBL_TREATMENT_TPID_COPY_FROM_INNER       = 0,
    EVTOCD_TBL_TREATMENT_TPID_COPY_FROM_OUTER       = 1,
    EVTOCD_TBL_TREATMENT_TPID_OUTPUT_DEI_CP_INNER   = 2,
    EVTOCD_TBL_TREATMENT_TPID_OUTPUT_DEI_CP_OUTER   = 3,
    EVTOCD_TBL_TREATMENT_TPID_8100                  = 4,
    EVTOCD_TBL_TREATMENT_TPID_OUTPUT_TPID_DEI_0     = 6,
    EVTOCD_TBL_TREATMENT_TPID_OUTPUT_TPID_DEI_1     = 7,
};


#ifdef __cplusplus
}
#endif

#endif
