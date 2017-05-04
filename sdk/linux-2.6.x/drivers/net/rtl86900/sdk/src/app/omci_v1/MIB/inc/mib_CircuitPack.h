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
 * Purpose : Definition of ME attribute: Circuit pack (6)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Circuit pack (6)
 */

#ifndef __MIB_CIRCUITPACK_TABLE_H__
#define __MIB_CIRCUITPACK_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table CircuitPack attribute index */
#define MIB_TABLE_CIRCUITPACK_ATTR_NUM (15)
#define MIB_TABLE_CIRCUITPACK_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_CIRCUITPACK_TYPE_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_CIRCUITPACK_NUMOFPORTS_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_CIRCUITPACK_SERIALNUM_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_CIRCUITPACK_VERSION_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_CIRCUITPACK_VID_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_CIRCUITPACK_ADMINSTATE_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_CIRCUITPACK_OPSTATE_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_CIRCUITPACK_BRIDGEDORIPIND_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_CIRCUITPACK_EQTID_INDEX ((MIB_ATTR_INDEX)10)
#define MIB_TABLE_CIRCUITPACK_CARDCFG_INDEX ((MIB_ATTR_INDEX)11)
#define MIB_TABLE_CIRCUITPACK_NUMOFTCONTBUFF_INDEX ((MIB_ATTR_INDEX)12)
#define MIB_TABLE_CIRCUITPACK_NUMOFPRIQ_INDEX ((MIB_ATTR_INDEX)13)
#define MIB_TABLE_CIRCUITPACK_NUMOFSCHEDULER_INDEX ((MIB_ATTR_INDEX)14)
#define MIB_TABLE_CIRCUITPACK_POWERSHED_INDEX ((MIB_ATTR_INDEX)15)


/* Table CircuitPack attribute len, only string attrubutes have length definition */
#define MIB_TABLE_CIRCUITPACK_SERIALNUM_LEN (8)
#define MIB_TABLE_CIRCUITPACK_VERSION_LEN (14)
#define MIB_TABLE_CIRCUITPACK_VID_LEN (4)
#define MIB_TABLE_CIRCUITPACK_EQTID_LEN (20)

// Table CircuitPack entry stucture
typedef struct {
    UINT16 EntityID; // index 1
    UINT8  Type;
    UINT8  NumOfPorts;
    CHAR   SerialNum[MIB_TABLE_CIRCUITPACK_SERIALNUM_LEN+1];
    CHAR   Version[MIB_TABLE_CIRCUITPACK_VERSION_LEN+1];
    CHAR   VID[MIB_TABLE_CIRCUITPACK_VID_LEN+1];
    UINT8  AdminState;
    UINT8  OpState;
    UINT8  BridgedorIPInd;
    CHAR   EqtID[MIB_TABLE_CIRCUITPACK_EQTID_LEN+1];
    UINT8  CardCfg;
    UINT8  NumOfTContBuff;
    UINT8  NumOfPriQ;
    UINT8  NumOfScheduler;
    UINT32 PowerShed;
} __attribute__((aligned)) MIB_TABLE_CIRCUITPACK_T;


#ifdef __cplusplus
}
#endif

#endif
