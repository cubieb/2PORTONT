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
 * Purpose : Definition of ME attribute: FEC PMHD (312)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: FEC PMHD (312)
 */

#ifndef __MIB_FEC_PMHD_TABLE_H__
#define __MIB_FEC_PMHD_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_FEC_PMHD_ATTR_NUM (8)
#define MIB_TABLE_FEC_PMHD_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_FEC_PMHD_INTERVALENDTIME_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_FEC_PMHD_THRESHOLDDATA12ID_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_FEC_PMHD_COR_BYTES_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_FEC_PMHD_COR_CODE_WORDS_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_FEC_PMHD_UNCOR_CODE_WORDS_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_FEC_PMHD_TOTAL_CODE_WORDS_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_FEC_PMHD_FEC_SECONDS_INDEX ((MIB_ATTR_INDEX)8)


typedef struct {
	UINT16   EntityId;
	UINT8    IntEndTime;
	UINT16   ThresholdID;	
	UINT32   CorBytes;
	UINT32   CorCodeWords;
	UINT32   UncorCodeWords;
	UINT32   TotalCodeWords;
	UINT16   FecSeconds;
} __attribute__((aligned)) MIB_TABLE_FEC_PMHD_T;


#ifdef __cplusplus
}
#endif

#endif
