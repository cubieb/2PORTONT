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
 * Purpose : Definition of ME attribute: MAC bridge port PMHD (52)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: MAC bridge port PMHD (52)
 */

#ifndef __MIB_MACBRIDGEPORTPMHD_TABLE_H__
#define __MIB_MACBRIDGEPORTPMHD_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table MacBridgePortPMHD attribute index */
#define MIB_TABLE_MACBRIDGEPORTPMHD_ATTR_NUM (8)
#define MIB_TABLE_MACBRIDGEPORTPMHD_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_MACBRIDGEPORTPMHD_INTERVALENDTIME_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_MACBRIDGEPORTPMHD_THRESHOLDDATA12ID_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_MACBRIDGEPORTPMHD_FORWARDEDFRAMECOUNTER_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_MACBRIDGEPORTPMHD_DELAYEXCEEDEDDISCARD_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_MACBRIDGEPORTPMHD_MTUEXCEEDEDDISCARD_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_MACBRIDGEPORTPMHD_RECEIVEDFRAMECOUNTER_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_MACBRIDGEPORTPMHD_RECEIVEDANDDISCARDEDCOUNTER_INDEX ((MIB_ATTR_INDEX)8)

/* Table MacBridgePortPMHD attribute len, only string attrubutes have length definition */

// Table MacBridgePortPMHD entry stucture
typedef struct {
	UINT16   EntityId;
	UINT8    IntEndTime;
	UINT16   ThresholdID;
	UINT32   ForwardedFrameCounter;
	UINT32   DelayExceededDiscard;
	UINT32   MtuExceededDiscard;
	UINT32   ReceivedFrameCounter;
	UINT32   ReceivedAndDiscardedCounter;
} __attribute__((aligned)) MIB_TABLE_MACBRIDGEPORTPMHD_T;


#ifdef __cplusplus
}
#endif

#endif
