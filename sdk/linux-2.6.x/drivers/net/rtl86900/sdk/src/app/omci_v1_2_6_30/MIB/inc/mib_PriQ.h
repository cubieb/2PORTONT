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
 * Purpose : Definition of ME attribute: Priority queue (277)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Priority queue (277)
 */

#ifndef __MIB_PRIQ_TABLE_H__
#define __MIB_PRIQ_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table PriQ attribute index */
#define MIB_TABLE_PRIQ_ATTR_NUM (17)
#define MIB_TABLE_PRIQ_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_PRIQ_QCFGOPT_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_PRIQ_MAXQSIZE_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_PRIQ_ALLOCQSIZE_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_PRIQ_RESETINTERVAL_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_PRIQ_THRESHOLD_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_PRIQ_RELATEDPORT_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_PRIQ_SCHEDULERPTR_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_PRIQ_WEIGHT_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_PRIQ_BPOPERATION_INDEX ((MIB_ATTR_INDEX)10)
#define MIB_TABLE_PRIQ_BPTIME_INDEX ((MIB_ATTR_INDEX)11)
#define MIB_TABLE_PRIQ_BPOCCTHRESHOLD_INDEX ((MIB_ATTR_INDEX)12)
#define MIB_TABLE_PRIQ_BPCLRTHRESHOLD_INDEX ((MIB_ATTR_INDEX)13)
#define MIB_TABLE_PRIQ_PKT_DROP_QUEUE_THRESHOLD_INDEX ((MIB_ATTR_INDEX)14)
#define MIB_TABLE_PRIQ_PKT_DROP_MAX_PROBABILITY_INDEX ((MIB_ATTR_INDEX)15)
#define MIB_TABLE_PRIQ_QUEUE_DROP_AVG_COEFFICIENT_INDEX ((MIB_ATTR_INDEX)16)
#define MIB_TABLE_PRIQ_DROP_PRECEDENCE_COLOUR_MARKING_INDEX ((MIB_ATTR_INDEX)17)


/* Table PriQ attribute len, only string attrubutes have length definition */

typedef enum {
    PQ_QUEUE_CFG_OPT_DEDICATE_BUFFER,
    PQ_QUEUE_CFG_OPT_SHARED_BUFFER,
} pq_attr_queue_cfg_opt_t;

typedef enum {
    PQ_DROP_COLOUR_NO_MARKING,
    PQ_DROP_COLOUR_INTERNAL_MARKING,
    PQ_DROP_COLOUR_DEI_MARKING,
    PQ_DROP_COLOUR_PCP_8P0D_MARKING,
    PQ_DROP_COLOUR_PCP_7P1D_MARKING,
    PQ_DROP_COLOUR_PCP_6P2D_MARKING,
    PQ_DROP_COLOUR_PCP_5P3D_MARKING,
    PQ_DROP_COLOUR_DSCP_AF_CLASS_MARKING,
} pq_attr_drop_precendence_colour_marking_t;

// Table PriQ entry stucture
typedef struct {
    UINT16 EntityID; // index 1
    UINT8  QCfgOpt;
    UINT16 MaxQSize;
    UINT16 AllocQSize;
    UINT16 ResetInterval;
    UINT16 Threshold;
    UINT32 RelatedPort;
    UINT16 SchedulerPtr;
    UINT8  Weight;
    UINT16 BPOperation;
    UINT32 BPTime;
    UINT16 BPOccThreshold;
    UINT16 BPClrThreshold;
    UINT64 PktDropQThold;
    UINT16 PktDropMaxP;
    UINT8  QueueDropWQ;
    UINT8  DropPColorMarking;
} __attribute__((aligned)) MIB_TABLE_PRIQ_T;


#ifdef __cplusplus
}
#endif

#endif
