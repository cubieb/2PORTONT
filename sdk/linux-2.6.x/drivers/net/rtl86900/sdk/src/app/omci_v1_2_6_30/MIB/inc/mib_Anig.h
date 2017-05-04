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
 * Purpose : Definition of ME attribute: ANI-G (263)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: ANI-G (263)
 */

#ifndef __MIB_ANIG_TABLE_H__
#define __MIB_ANIG_TABLE_H__


/* Table Anig attribute index */
#define MIB_TABLE_ANIG_ATTR_NUM (21)
#define MIB_TABLE_ANIG_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_ANIG_SRIND_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_ANIG_NUMOFTCONT_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_ANIG_GEMBLKLEN_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_ANIG_PBDBARPT_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_ANIG_ONUDBARPT_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_ANIG_SFTHRESHOLD_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_ANIG_SDTHRESHOLD_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_ANIG_ARC_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_ANIG_ARCINTERVAL_INDEX ((MIB_ATTR_INDEX)10)
#define MIB_TABLE_ANIG_OPTICALSIGNALLEVEL_INDEX ((MIB_ATTR_INDEX)11)
#define MIB_TABLE_ANIG_LOWOPTHRESHOLD_INDEX ((MIB_ATTR_INDEX)12)
#define MIB_TABLE_ANIG_UPPOPTHRESHOLD_INDEX ((MIB_ATTR_INDEX)13)
#define MIB_TABLE_ANIG_ONTRSPTIME_INDEX ((MIB_ATTR_INDEX)14)
#define MIB_TABLE_ANIG_TRANOPTICLEVEL_INDEX ((MIB_ATTR_INDEX)15)
#define MIB_TABLE_ANIG_LOWTRANPOWTHRESHOLD_INDEX ((MIB_ATTR_INDEX)16)
#define MIB_TABLE_ANIG_UPPTRANPOWTHRESHOLD_INDEX ((MIB_ATTR_INDEX)17)
#define MIB_TABLE_ANIG_LOW_DFL_RX_THR_IDX ((MIB_ATTR_INDEX)18)
#define MIB_TABLE_ANIG_UPP_DFL_RX_THR_IDX ((MIB_ATTR_INDEX)19)
#define MIB_TABLE_ANIG_LOW_DFL_TX_THR_IDX ((MIB_ATTR_INDEX)20)
#define MIB_TABLE_ANIG_UPP_DFL_TX_THR_IDX ((MIB_ATTR_INDEX)21)


/* Table Anig attribute len, only string attrubutes have length definition */
#define ANIG_DFL_RX_THR   (0xFF)
#define ANIG_DFL_TX_THR   (0x81)

typedef enum {
    ANIG_TEST_TYPE_SELF_TEST    = 7,
} anig_test_type_t;

typedef enum {
    ANIG_TEST_SELF_TEST_RESULT_UNSUPPORTED          = 0,
    ANIG_TEST_SELF_TEST_RESULT_POWER_FEED_VOLTAGE   = 1,
    ANIG_TEST_SELF_TEST_RESULT_RX_OPTICAL_POWER     = 3,
    ANIG_TEST_SELF_TEST_RESULT_MEAN_OPTICAL_POWER   = 5,
    ANIG_TEST_SELF_TEST_RESULT_LASER_BIAS_CURRENT   = 9,
    ANIG_TEST_SELF_TEST_RESULT_TEMPERATURE          = 12,
} anig_test_self_test_result_t;

// Table Anig entry stucture
typedef struct {
    UINT16	EntityID; // index 1
    UINT8	SRInd;
    UINT16	NumOfTcont;
    UINT16	GemBlkLen;
    UINT8	PbDbaRpt;
    UINT8	OnuDbaRpt;
    UINT8	SFThreshold;
    UINT8	SDThreshold;
    UINT8	ARC;
    UINT8	ARCInterval;
    UINT16	OpticalSignalLevel;
    UINT8	LowOpThreshold;
    UINT8	UppOpThreshold;
    UINT16	OntRspTime;
	UINT16	TranOpticLevel;
	UINT8	LowTranPowThreshold;
	UINT8	UppTranPowThreshold;
    UINT8   LowDflRxThreshold;
    UINT8   UppDflRxThreshold;
    UINT8   LowDflTxThreshold;
    UINT8   UppDflTxThreshold;
} __attribute__((aligned)) MIB_TABLE_ANIG_T;


#endif
