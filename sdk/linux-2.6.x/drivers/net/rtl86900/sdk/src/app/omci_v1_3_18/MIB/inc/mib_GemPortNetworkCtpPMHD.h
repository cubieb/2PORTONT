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
 * Purpose : Definition of ME attribute: GEM port network CTP PMHD (341)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: GEM port network CTP PMHD (341)
 */

#ifndef __MIB_GPNC_PMHD_TABLE_H__
#define __MIB_GPNC_PMHD_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_GPNC_PMHD_ATTR_NUM (8)
#define MIB_TABLE_GPNC_PMHD_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_GPNC_PMHD_INTERVALENDTIME_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_GPNC_PMHD_THRESHOLDDATA12ID_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_GPNC_PMHD_TX_GEM_FRAMES_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_GPNC_PMHD_RX_GEM_FRAMES_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_GPNC_PMHD_RX_PAYLOAD_BYTES_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_GPNC_PMHD_TX_PAYLOAD_BYTES_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_GPNC_PMHD_ENCRYPT_KEY_ERRORS_INDEX ((MIB_ATTR_INDEX)8)


typedef struct {
	UINT16   EntityId;
	UINT8    IntEndTime;
	UINT16   ThresholdID;	
	UINT32   TxGemFrames;
	UINT32   RxGemFrames;
	UINT64   RxPayloadBytes;
	UINT64   TxPayloadBytes;
	UINT32   EncryptKeyErrors;
} __attribute__((aligned)) MIB_TABLE_GPNC_PMHD_T;


#ifdef __cplusplus
}
#endif

#endif
