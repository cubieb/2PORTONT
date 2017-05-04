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
 * Purpose : Definition of ME attribute: Dot1 rate limiter (298)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Dot1 rate limiter (298)
 */

#ifndef __MIB_DOT1_RATE_LIMITER_H__
#define __MIB_DOT1_RATE_LIMITER_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_DOT1_RATE_LIMITER_ATTR_NUM (6)
#define MIB_TABLE_DOT1_RATE_LIMITER_ENTITY_ID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_DOT1_RATE_LIMITER_PARENT_ME_PTR_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_DOT1_RATE_LIMITER_TP_TYPE_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_DOT1_RATE_LIMITER_US_UC_FLOOD_RATE_PTR_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_DOT1_RATE_LIMITER_US_BC_RATE_PTR_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_DOT1_RATE_LIMITER_US_MC_PAYLOAD_RATE_PTR_INDEX ((MIB_ATTR_INDEX)6)


typedef enum {
    DOT1_RATE_LIMITER_TP_TYPE_MAC_BRIDGE			= 1,
    DOT1_RATE_LIMITER_TP_TYPE_IEEE_8021P_MAPPER		= 2,
} dot1_rate_limiter_attr_tp_type_t;

typedef struct {
	UINT16	EntityId;
	UINT16	ParentMePtr;
	UINT8	TpType;
	UINT16	UsUcFloodRatePtr;
	UINT16	UsBcRatePtr;
	UINT16	UsMcPayloadRatePtr;
} __attribute__((aligned)) MIB_TABLE_DOT1_RATE_LIMITER_T;


#ifdef __cplusplus
}
#endif

#endif
