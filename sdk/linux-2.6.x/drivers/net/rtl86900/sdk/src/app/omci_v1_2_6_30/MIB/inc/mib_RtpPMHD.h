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
 * Purpose : Definition of ME attribute: RTP PMHD (144)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: RTP PMHD (144)
 */

#ifndef __MIB_RTP_PMHD_TABLE_H__
#define __MIB_RTP_PMHD_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_RTP_PMHD_ATTR_NUM (9)
#define MIB_TABLE_RTP_PMHD_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_RTP_PMHD_INTERVALENDTIME_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_RTP_PMHD_THRESHOLDDATA12ID_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_RTP_PMHD_RTP_ERRORS_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_RTP_PMHD_PKT_LOSS_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_RTP_PMHD_MAX_JITTER_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_RTP_PMHD_MAX_TIME_RTCP_PKTS_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_RTP_PMHD_BUFFER_UNDERFLOWS_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_RTP_PMHD_BUFFER_OVERFLOWS_INDEX ((MIB_ATTR_INDEX)9)


typedef struct {
	UINT16   EntityId;
	UINT8    IntEndTime;
	UINT16   ThresholdID;
	UINT32   RtpErrors;
	UINT32   PktLoss;
	UINT32   MaxJitter;
	UINT32   MaxTimeRtcpPkts;
	UINT32   BufferUnderflows;
	UINT32   BufferOverflows;
} __attribute__((aligned)) MIB_TABLE_RTP_PMHD_T;


#ifdef __cplusplus
}
#endif

#endif
