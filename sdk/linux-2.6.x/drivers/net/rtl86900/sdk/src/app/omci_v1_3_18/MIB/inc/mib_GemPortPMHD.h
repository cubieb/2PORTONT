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
 * Purpose : Definition of ME attribute: GEM port PMHD (267)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: GEM port PMHD (267)
 */

#ifndef __MIB_GEM_PORT_PMHD_TABLE_H__
#define __MIB_GEM_PORT_PMHD_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_GEM_PORT_PMHD_ATTR_NUM (10)
#define MIB_TABLE_GEM_PORT_PMHD_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_GEM_PORT_PMHD_INTERVALENDTIME_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_GEM_PORT_PMHD_THRESHOLDDATA12ID_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_GEM_PORT_PMHD_LOST_PKTS_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_GEM_PORT_PMHD_MISINSERTED_PKTS_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_GEM_PORT_PMHD_RECEIVED_PKTS_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_GEM_PORT_PMHD_RECEIVED_BLOCKS_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_GEM_PORT_PMHD_TRANSMITTED_BLOCKS_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_GEM_PORT_PMHD_IMPAIRED_BLOCKS_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_GEM_PORT_PMHD_TRANSMITTED_PKTS_INDEX ((MIB_ATTR_INDEX)10)


typedef struct {
	UINT16   EntityId;
	UINT8    IntEndTime;
	UINT16   ThresholdID;	
	UINT32   LostPkts;
	UINT32   MisinsertedPkts;
	UINT64   ReceivedPkts;
	UINT64   ReceivedBlocks;
	UINT64   TransmittedBlocks;
	UINT32   ImpairedBlocks;
	UINT64   TransmittedPkts;
} __attribute__((aligned)) MIB_TABLE_GEM_PORT_PMHD_T;


#ifdef __cplusplus
}
#endif

#endif
