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
 * Purpose : Definition of ME handler: PPTP POTS UNI (53)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME handler: PPTP POTS UNI (53)
 */


#ifndef __MIB_PPTP_POTS_UNI_H__
#define __MIB_PPTP_POTS_UNI_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_PPTP_POTS_UNI_ATTR_NUM (13)
#define MIB_TABLE_PPTP_POTS_UNI_ENTITY_ID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_PPTP_POTS_UNI_ADMIN_STATE_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_PPTP_POTS_UNI_IWTP_PTR_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_PPTP_POTS_UNI_ARC_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_PPTP_POTS_UNI_ARC_INTVL_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_PPTP_POTS_UNI_IMPEDANCE_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_PPTP_POTS_UNI_TX_PATH_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_PPTP_POTS_UNI_RX_GAIN_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_PPTP_POTS_UNI_TX_GAIN_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_PPTP_POTS_UNI_OP_STATE_INDEX ((MIB_ATTR_INDEX)10)
#define MIB_TABLE_PPTP_POTS_UNI_HOOK_STATE_INDEX ((MIB_ATTR_INDEX)11)
#define MIB_TABLE_PPTP_POTS_UNI_POTS_HOLDOVER_TIME_INDEX ((MIB_ATTR_INDEX)12)
#define MIB_TABLE_PPTP_POTS_UNI_NOMINAL_FEED_VOLTAGE_INDEX ((MIB_ATTR_INDEX)13)


typedef enum {
    POTS_IMPEDANCE_600OHMS							= 0,
    POTS_IMPEDANCE_900OHMS							= 1,
    POTS_IMPEDANCE_C1_150NF_R1_750OHM_R2_270OHM		= 2,
    POTS_IMPEDANCE_C1_115NF_R1_820OHM_R2_220OHM		= 3,
    POTS_IMPEDANCE_C1_230NF_R1_1050OHM_R2_320OHM	= 4
} pptp_pots_uni_attr_impedance_t;

typedef enum {
    POTS_HOOK_STATE_ON_HOOK		= 0,
    POTS_HOOK_STATE_OFF_HOOK	= 1
} pptp_pots_uni_attr_hook_state_t;

typedef struct {
	UINT16   EntityId;
	UINT8    AdminState;
	UINT16   IwtpPtr;
	UINT8    Arc;
	UINT8    ArcIntvl;
	UINT8    Impedance;
	UINT8    TxPath;
	UINT8    RxGain;
	UINT8    TxGain;
	UINT8    OpState;
	UINT8    HookState;
	UINT16   PotsHoldoverTime;
	UINT8    NominalFeedVoltage;
} __attribute__((aligned)) MIB_TABLE_PPTP_POTS_UNI_T;


#ifdef __cplusplus
}
#endif

#endif
