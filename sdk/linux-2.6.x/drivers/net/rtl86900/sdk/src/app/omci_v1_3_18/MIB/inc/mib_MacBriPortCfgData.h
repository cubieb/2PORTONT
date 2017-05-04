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
 * Purpose : Definition of ME attribute: MAC bridge port configuration data (47)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: MAC bridge port configuration data (47)
 */

#ifndef __MIB_MACBRIPORTCFGDATA_TABLE_H__
#define __MIB_MACBRIPORTCFGDATA_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table MacBriPortCfgData attribute index */
#define MIB_TABLE_MACBRIPORTCFGDATA_ATTR_NUM (14)
#define MIB_TABLE_MACBRIPORTCFGDATA_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_MACBRIPORTCFGDATA_BRIDGEIDPTR_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_MACBRIPORTCFGDATA_PORTNUM_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_MACBRIPORTCFGDATA_TPTYPE_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_MACBRIPORTCFGDATA_TPPOINTER_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_MACBRIPORTCFGDATA_PORTPRIORITY_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_MACBRIPORTCFGDATA_PORTPATHCOST_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_MACBRIPORTCFGDATA_PORTSPANTREEIND_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_MACBRIPORTCFGDATA_ENCAPMETHOD_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_MACBRIPORTCFGDATA_LANFCSIND_INDEX ((MIB_ATTR_INDEX)10)
#define MIB_TABLE_MACBRIPORTCFGDATA_PORTMACADDR_INDEX ((MIB_ATTR_INDEX)11)
#define MIB_TABLE_MACBRIPORTCFGDATA_OUTBOUNDTD_INDEX ((MIB_ATTR_INDEX)12)
#define MIB_TABLE_MACBRIPORTCFGDATA_INBOUNDTD_INDEX ((MIB_ATTR_INDEX)13)
#define MIB_TABLE_MACBRIPORTCFGDATA_NUMOFALLOWEDMAC_INDEX ((MIB_ATTR_INDEX)14)


/* Table MacBriPortCfgData attribute len, only string attrubutes have length definition */
#define MIB_TABLE_MACBRIPORTCFGDATA_PORTMACADDR_LEN (6)

typedef enum {
    MBPCD_TP_TYPE_PPTP_ETH_UNI          = 1,
    MBPCD_TP_TYPE_IW_VCC_TP             = 2,
    MBPCD_TP_TYPE_IEEE_8021P_MAPPER     = 3,
    MBPCD_TP_TYPE_IP_HOST_IPV6_HOST     = 4,
    MBPCD_TP_TYPE_GEM_IWTP              = 5,
    MBPCD_TP_TYPE_MCAST_GEM_IWTP        = 6,
    MBPCD_TP_TYPE_PPTP_XDSL_UNI_PART_1  = 7,
    MBPCD_TP_TYPE_PPTP_VDSL_UNI         = 8,
    MBPCD_TP_TYPE_ETH_FLOW_TP           = 9,
    MBPCD_TP_TYPE_VEIP                  = 11,
    MBPCD_TP_TYPE_PPTP_MOCA_UNI         = 12,
} mbpcd_attr_tp_type_t;

// Table MacBriPortCfgData entry stucture
typedef struct {
    UINT16 EntityID; // index 1
    UINT16 BridgeIdPtr;
    UINT8  PortNum;
    UINT8  TPType;
    UINT16 TPPointer;
    UINT16 PortPriority;
    UINT16 PortPathCost;
    UINT8  PortSpanTreeInd;
    UINT8  EncapMethod;
    UINT8  LanFCSInd;
    UINT8  PortMacAddr[MIB_TABLE_MACBRIPORTCFGDATA_PORTMACADDR_LEN];
    UINT16 OutboundTD;
    UINT16 InboundTD;
    UINT8  NumOfAllowedMac;
} __attribute__((aligned)) MIB_TABLE_MACBRIPORTCFGDATA_T;


#ifdef __cplusplus
}
#endif

#endif
