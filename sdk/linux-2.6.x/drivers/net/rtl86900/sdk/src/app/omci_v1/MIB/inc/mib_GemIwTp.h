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
 * Purpose : Definition of ME attribute: GEM IWTP (266)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: GEM IWTP (266)
 */

#ifndef __MIB_GEMIWTP_TABLE_H__
#define __MIB_GEMIWTP_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table GemIwTp attribute index */
#define MIB_TABLE_GEMIWTP_ATTR_NUM (9)
#define MIB_TABLE_GEMIWTP_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_GEMIWTP_GEMCTPPTR_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_GEMIWTP_IWOPT_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_GEMIWTP_SERVPROPTR_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_GEMIWTP_IWTPPTR_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_GEMIWTP_PPTPCOUNTER_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_GEMIWTP_OPSTATE_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_GEMIWTP_GALPROFPTR_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_GEMIWTP_GALLOOPBACKCFG_INDEX ((MIB_ATTR_INDEX)9)


/* Table GemIwTp attribute len, only string attrubutes have length definition */

// Table GemIwTp entry stucture
typedef struct {
    UINT16 EntityID; // index 1
    UINT16 GemCtpPtr;
    UINT8  IwOpt;
    UINT16 ServProPtr;
    UINT16 IwTpPtr;
    UINT8  PptpCounter;
    UINT8  OpState;
    UINT16 GalProfPtr;
    UINT8  GalLoopbackCfg;
} __attribute__((aligned)) MIB_TABLE_GEMIWTP_T;

typedef enum {
    GEM_IWTP_IW_OPTION_CIRCUIT_EMULATED_TDM     = 0,
    GEM_IWTP_IW_OPTION_MAC_BRIDGED_LAN          = 1,
    GEM_IWTP_IW_OPTION_VIDEO_RETURN_PATH        = 4,
    GEM_IWTP_IW_OPTION_IEEE_8021P_MAPPER        = 5,
    GEM_IWTP_IW_OPTION_DOWNSTREAM_BROADCAST     = 6,
    GEM_IWTP_IW_OPTION_MPLS_PW_TDM              = 7,
} gem_iwtp_attr_iw_option_t;


#ifdef __cplusplus
}
#endif

#endif
