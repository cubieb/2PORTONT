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
 * Purpose : Definition of ME attribute: ONT2-G (257)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: ONT2-G (257)
 */

#ifndef __MIB_ONTG2_TABLE_H__
#define __MIB_ONTG2_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table Ont2g attribute index */
#define MIB_TABLE_ONT2G_ATTR_NUM (15)
#define MIB_TABLE_ONT2G_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_ONT2G_EQTID_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_ONT2G_OMCCVER_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_ONT2G_VPCODE_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_ONT2G_SECCAPABILITY_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_ONT2G_SECMODE_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_ONT2G_NUMOFPRIQ_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_ONT2G_NUMOFSCHEDULER_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_ONT2G_MODE_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_ONT2G_NUMOFGEMPORT_INDEX ((MIB_ATTR_INDEX)10)
#define MIB_TABLE_ONT2G_SYSUPTIME_INDEX ((MIB_ATTR_INDEX)11)
#define MIB_TABLE_ONT2G_CONNECTIVITY_CAPABILITY_INDEX ((MIB_ATTR_INDEX)12)
#define MIB_TABLE_ONT2G_CURRENT_CONNECTIVITY_MODE_INDEX ((MIB_ATTR_INDEX)13)
#define MIB_TABLE_ONT2G_QOS_CONFIGURATION_FLEXIBILITY_INDEX ((MIB_ATTR_INDEX)14)
#define MIB_TABLE_ONT2G_PRIORITY_QUEUE_SCALE_FACTOR_INDEX ((MIB_ATTR_INDEX)15)

/* Table Ont2g attribute len, only string attrubutes have length definition */
#define MIB_TABLE_ONT2G_EQTID_LEN (20)

typedef enum {
    ONU2G_OMCC_VERSION_0x80 = 0x80,
    ONU2G_OMCC_VERSION_0x81,
    ONU2G_OMCC_VERSION_0x82,
    ONU2G_OMCC_VERSION_0x83,
    ONU2G_OMCC_VERSION_0x84,
    ONU2G_OMCC_VERSION_0x85,
    ONU2G_OMCC_VERSION_0x86,
    ONU2G_OMCC_VERSION_0x96 = 0x96,
    ONU2G_OMCC_VERSION_0xA0 = 0xA0,
    ONU2G_OMCC_VERSION_0xA1,
    ONU2G_OMCC_VERSION_0xA2,
    ONU2G_OMCC_VERSION_0xA3,
    ONU2G_OMCC_VERSION_0xB0 = 0xB0,
    ONU2G_OMCC_VERSION_0xB1,
    ONU2G_OMCC_VERSION_0xB2,
    ONU2G_OMCC_VERSION_0xB3,
} onu2g_attr_omcc_version_t;

typedef enum {
    ONU2G_CONNECTIVITY_CAPABILITY_NONE      = 0,
    ONU2G_CONNECTIVITY_CAPABILITY_N_TO_1    = (1 << 0),
    ONU2G_CONNECTIVITY_CAPABILITY_1_TO_M    = (1 << 1),
    ONU2G_CONNECTIVITY_CAPABILITY_1_TO_P    = (1 << 2),
    ONU2G_CONNECTIVITY_CAPABILITY_N_TO_M    = (1 << 3),
    ONU2G_CONNECTIVITY_CAPABILITY_1_TO_MP   = (1 << 4),
    ONU2G_CONNECTIVITY_CAPABILITY_N_TO_P    = (1 << 5),
    ONU2G_CONNECTIVITY_CAPABILITY_N_TO_MP   = (1 << 6),
} onu2g_attr_connectivity_capability_t;

typedef enum {
    ONU2G_CURRENT_CONNECTIVITY_MODE_DEFAULT     = 0,
    ONU2G_CURRENT_CONNECTIVITY_MODE_N_TO_1      = 1,
    ONU2G_CURRENT_CONNECTIVITY_MODE_1_TO_M      = 2,
    ONU2G_CURRENT_CONNECTIVITY_MODE_1_TO_P      = 3,
    ONU2G_CURRENT_CONNECTIVITY_MODE_N_TO_M      = 4,
    ONU2G_CURRENT_CONNECTIVITY_MODE_1_TO_MP     = 5,
    ONU2G_CURRENT_CONNECTIVITY_MODE_N_TO_P      = 6,
    ONU2G_CURRENT_CONNECTIVITY_MODE_N_TO_MP     = 7,
} onu2g_attr_current_connectivity_mode_t;

typedef enum {
    ONU2G_QOS_CFG_FLEXIBILITY_NONE              = 0,
    ONU2G_QOS_CFG_PQ_RELATED_PORT_PORT_RW       = (1 << 0),
    ONU2G_QOS_CFG_PQ_TRAFFIC_SCHEDULER_RW       = (1 << 1),
    ONU2G_QOS_CFG_TS_TCONT_RW                   = (1 << 2),
    ONU2G_QOS_CFG_TS_POLICY_RW                  = (1 << 3),
    ONU2G_QOS_CFG_TC_POLICY_RW                  = (1 << 4),
    ONU2G_QOS_CFG_PQ_RELATED_PORT_PRIORITY_RW   = (1 << 5),
} onu2g_attr_qos_configuration_flexibility_t;

// Table Ont2g entry stucture
typedef struct {
    UINT16 EntityID; // index 1
    CHAR   EqtID[MIB_TABLE_ONT2G_EQTID_LEN+1];
    UINT8  OMCCVer;
    UINT16 VPCode;
    UINT8  SecCapability;
    UINT8  SecMode;
    UINT16 NumOfPriQ;
    UINT8  NumOfScheduler;
    UINT8  Mode;
    UINT16 NumOfGemPort;
    UINT32 SysUpTime;
    UINT16 ConnectivityCapability;
    UINT8  CurrentConnectivityMode;
    UINT16 QosConfigurationFlexibility;
    UINT16 PriorityQueueScaleFactor;
} __attribute__((aligned)) MIB_TABLE_ONT2G_T;


#ifdef __cplusplus
}
#endif

#endif
