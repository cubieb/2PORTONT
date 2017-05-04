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
 * Purpose : Definition of ME attribute: Traffic scheduler (278)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Traffic scheduler (278)
 */

#ifndef __MIB_SCHEDULER_TABLE_H__
#define __MIB_SCHEDULER_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table Scheduler attribute index */
#define MIB_TABLE_SCHEDULER_ATTR_NUM (5)
#define MIB_TABLE_SCHEDULER_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_SCHEDULER_TCONTPTR_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_SCHEDULER_SCHEDULERPTR_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_SCHEDULER_POLICY_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_SCHEDULER_PRIWEIGHT_INDEX ((MIB_ATTR_INDEX)5)


/* Table Scheduler attribute len, only string attrubutes have length definition */

// Table Scheduler entry stucture
typedef struct {
    UINT16 EntityID; // index 1
    UINT16 TcontPtr;
    UINT16 SchedulerPtr;
    UINT8  Policy;
    UINT8  PriWeight;
} __attribute__((aligned)) MIB_TABLE_SCHEDULER_T;


#ifdef __cplusplus
}
#endif

#endif
