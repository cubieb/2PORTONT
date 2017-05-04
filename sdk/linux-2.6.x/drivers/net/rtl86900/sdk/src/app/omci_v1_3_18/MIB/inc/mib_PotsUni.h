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
 */


#ifndef __MIB_POTSUNI_TABLE_H__
#define __MIB_POTSUNI_TABLE_H__

/* Table PotsUni attribute for STRING type define each entry length */

/* Table PotsUni attribute index */
#define MIB_TABLE_POTSUNI_ATTR_NUM (12)
#define MIB_TABLE_POTSUNI_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_POTSUNI_ADMINSTATE_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_POTSUNI_IWTPPTR_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_POTSUNI_ARC_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_POTSUNI_ARCINTERVAL_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_POTSUNI_IMPEDANCE_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_POTSUNI_TXPATH_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_POTSUNI_RXGAIN_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_POTSUNI_TXGAIN_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_POTSUNI_OPSTATE_INDEX ((MIB_ATTR_INDEX)10)
#define MIB_TABLE_POTSUNI_HOOKSTATE_INDEX ((MIB_ATTR_INDEX)11)
#define MIB_TABLE_POTSUNI_POTSHOLDOVERTIME_INDEX ((MIB_ATTR_INDEX)12)

/* Table PotsUni attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
	UINT8    AdminState;
	UINT16   IWTPPtr;
	UINT8    ARC;
	UINT8    ARCInterval;
	UINT8    Impedance;
	UINT8    TxPath;
	UINT8    RxGain;
	UINT8    TxGain;
	UINT8    OpState;
	UINT8    HookState;
	UINT16   POTSHoldoverTime;
} __attribute__((aligned)) MIB_TABLE_POTSUNI_T;

#endif /* __MIB_POTSUNI_TABLE_H__ */
