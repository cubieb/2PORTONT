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
 * Purpose : Definition of ME attribute: IEEE 802.1p mapper service profile (130)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: IEEE 802.1p mapper service profile (130)
 */

#ifndef __MIB_MAP8021PSERVPROF_TABLE_H__
#define __MIB_MAP8021PSERVPROF_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table Map8021pServProf attribute index */
#define MIB_TABLE_MAP8021PSERVPROF_ATTR_NUM (14)
#define MIB_TABLE_MAP8021PSERVPROF_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_MAP8021PSERVPROF_PPTPUNIPTR_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_MAP8021PSERVPROF_IWTPPTRPBIT0_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_MAP8021PSERVPROF_IWTPPTRPBIT1_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_MAP8021PSERVPROF_IWTPPTRPBIT2_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_MAP8021PSERVPROF_IWTPPTRPBIT3_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_MAP8021PSERVPROF_IWTPPTRPBIT4_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_MAP8021PSERVPROF_IWTPPTRPBIT5_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_MAP8021PSERVPROF_IWTPPTRPBIT6_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_MAP8021PSERVPROF_IWTPPTRPBIT7_INDEX ((MIB_ATTR_INDEX)10)
#define MIB_TABLE_MAP8021PSERVPROF_UNMARKFRMOPT_INDEX ((MIB_ATTR_INDEX)11)
#define MIB_TABLE_MAP8021PSERVPROF_DSCPMAP2PBIT_INDEX ((MIB_ATTR_INDEX)12)
#define MIB_TABLE_MAP8021PSERVPROF_DEFPBITMARK_INDEX ((MIB_ATTR_INDEX)13)
#define MIB_TABLE_MAP8021PSERVPROF_TPTYPE_INDEX ((MIB_ATTR_INDEX)14)


/* Table Map8021pServProf attribute len, only string attrubutes have length definition */
#define MIB_TABLE_MAP8021PSERVPROF_DSCPMAP2PBIT_LEN (24)

// Table Map8021pServProf entry stucture
typedef struct {
    UINT16 EntityID; // index 1
    UINT16 TpPtr;
    UINT16 IwTpPtrPbit0;
    UINT16 IwTpPtrPbit1;
    UINT16 IwTpPtrPbit2;
    UINT16 IwTpPtrPbit3;
    UINT16 IwTpPtrPbit4;
    UINT16 IwTpPtrPbit5;
    UINT16 IwTpPtrPbit6;
    UINT16 IwTpPtrPbit7;
    UINT8  UnmarkFrmOpt;
    UINT8  DscpMap2Pbit[MIB_TABLE_MAP8021PSERVPROF_DSCPMAP2PBIT_LEN];
    UINT8  DefPbitMark;
    UINT8  TPType;
} __attribute__((aligned)) MIB_TABLE_MAP8021PSERVPROF_T;


enum {
	MAP_8021P_TP_TYPE_BRIDGE = 0,
	MAP_8021P_TP_TYPE_PPTP_ETHUNI,
	MAP_8021P_TP_TYPE_IPHOST,
	MAP_8021P_TP_TYPE_ETHFLOW,
	MAP_8021P_TP_TYPE_PPTP_XDSLUNI,
	MAP_8021P_TP_TYPE_PPTP_80211UNI,
	MAP_8021P_TP_TYPE_PPTP_MOCAUNI
};

enum {
    MAP_8021P_UNMARKED_OPT_DSCP21P    = 0,
    MAP_8021P_UNMARKED_OPT_DEFPBIT    = 1
};

#ifdef __cplusplus
}
#endif

#endif
