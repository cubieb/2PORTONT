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
 * Purpose : Definition of ME attribute: PPTP Ethernet UNI (11)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: PPTP Ethernet UNI (11)
 */

#ifndef __MIB_ETHUNI_TABLE_H__
#define __MIB_ETHUNI_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table EthUni attribute index */
#define MIB_TABLE_ETHUNI_ATTR_NUM (16)
#define MIB_TABLE_ETHUNI_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_ETHUNI_EXPECTEDTYPE_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_ETHUNI_SENSEDTYPE_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_ETHUNI_AUTODECTECTCFG_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_ETHUNI_ETHLOOPBACKCFG_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_ETHUNI_ADMINSTATE_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_ETHUNI_OPSTATE_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_ETHUNI_DUPLEXIND_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_ETHUNI_MAXFRAMESIZE_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_ETHUNI_DTEORDCEIND_INDEX ((MIB_ATTR_INDEX)10)
#define MIB_TABLE_ETHUNI_PAUSETIME_INDEX ((MIB_ATTR_INDEX)11)
#define MIB_TABLE_ETHUNI_BRIDGEDORIPIND_INDEX ((MIB_ATTR_INDEX)12)
#define MIB_TABLE_ETHUNI_ARC_INDEX ((MIB_ATTR_INDEX)13)
#define MIB_TABLE_ETHUNI_ARCINTERVAL_INDEX ((MIB_ATTR_INDEX)14)
#define MIB_TABLE_ETHUNI_PPPOEFILTER_INDEX ((MIB_ATTR_INDEX)15)
#define MIB_TABLE_ETHUNI_POWERCONTROL_INDEX ((MIB_ATTR_INDEX)16)


/* Table EthUni attribute len, only string attrubutes have length definition */

typedef enum {
    PPTP_ETH_UNI_AUTO_DETECT_CFG_AUTO_AUTO          = 0x00,
    PPTP_ETH_UNI_AUTO_DETECT_CFG_10M_FULL_DUPLEX    = 0x01,
    PPTP_ETH_UNI_AUTO_DETECT_CFG_100M_FULL_DUPLEX   = 0x02,
    PPTP_ETH_UNI_AUTO_DETECT_CFG_1000M_FULL_DUPLEX  = 0x03,
    PPTP_ETH_UNI_AUTO_DETECT_CFG_AUTO_FULL_DUPLEX   = 0x04,
    PPTP_ETH_UNI_AUTO_DETECT_CFG_10G_FULL_DUPLEX    = 0x05,
    PPTP_ETH_UNI_AUTO_DETECT_CFG_10M_AUTO           = 0x10,
    PPTP_ETH_UNI_AUTO_DETECT_CFG_10M_HALF_DUPLEX    = 0x11,
    PPTP_ETH_UNI_AUTO_DETECT_CFG_100M_HALF_DUPLEX   = 0x12,
    PPTP_ETH_UNI_AUTO_DETECT_CFG_1000M_HALF_DUPLEX  = 0x13,
    PPTP_ETH_UNI_AUTO_DETECT_CFG_AUTO_HALF_DUPLEX   = 0x14,
    PPTP_ETH_UNI_AUTO_DETECT_CFG_1000M_AUTO         = 0x20,
    PPTP_ETH_UNI_AUTO_DETECT_CFG_100M_AUTO          = 0x30,
} pptp_eth_uni_attr_auto_detection_configuration_t;

typedef enum {
    PPTP_ETH_UNI_CFG_IND_UNKNOWN                        = 0x00,
    PPTP_ETH_UNI_CFG_IND_10BASET_FULL_DUPLEX            = 0x01,
    PPTP_ETH_UNI_CFG_IND_100BASET_FULL_DUPLEX           = 0x02,
    PPTP_ETH_UNI_CFG_IND_GIGABIT_ETHERNET_FULL_DUPLEX   = 0x03,
    PPTP_ETH_UNI_CFG_IND_10G_ETHERNET_FULL_DUPLEX       = 0x04,
    PPTP_ETH_UNI_CFG_IND_10BASET_HALF_DUPLEX            = 0x11,
    PPTP_ETH_UNI_CFG_IND_100BASET_HALF_DUPLEX           = 0x12,
    PPTP_ETH_UNI_CFG_IND_GIGABIT_ETHERNET_HALF_DUPLEX   = 0x13,
} pptp_eth_uni_attr_configuration_ind_t;

typedef enum {
    PPTP_ETH_UNI_LOOPBACK_CFG_NO_LOOPBACK       = 0x00,
    PPTP_ETH_UNI_LOOPBACK_CFG_DS_PHY_LOOPBACK   = 0x03,
} pptp_eth_uni_attr_loopback_configuration_t;

typedef enum {
    PPTP_ETH_UNI_SENSE_TYPE_AVC_10BASET             = 1,
    PPTP_ETH_UNI_SENSE_TYPE_AVC_100BASET            = 2,
    PPTP_ETH_UNI_SENSE_TYPE_AVC_GIGABIT_ETHERNET    = 3,
    PPTP_ETH_UNI_SENSE_TYPE_AVC_10G_ETHERNET        = 4,
} pptp_eth_uni_attr_sense_type_avc_t;

// Table EthUni entry stucture
typedef struct {
    UINT16 EntityID; // index 1
    UINT8  ExpectedType;
    UINT8  SensedType;
    UINT8  AutoDectectCfg;
    UINT8  EthLoopbackCfg;
    UINT8  AdminState;
    UINT8  OpState;
    UINT8  DuplexInd;
    UINT16 MaxFrameSize;
    UINT8  DTEorDCEInd;
    UINT16 PauseTime;
    UINT8  BridgedorIPInd;
    UINT8  ARC;
    UINT8  ARCInterval;
    UINT8  PppoeFilter;
    UINT8  PowerControl;
} __attribute__((aligned)) MIB_TABLE_ETHUNI_T;


#ifdef __cplusplus
}
#endif

#endif
