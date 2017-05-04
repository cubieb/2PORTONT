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
 * Purpose : Definition of ME attribute: Ethernet frame extended PM (334)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Ethernet frame extended PM (334)
 */

#ifndef __MIB_ETHEXTPMDATA_TABLE_H__
#define __MIB_ETHEXTPMDATA_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "omci_pm.h"


/* Table EthExtPmData attribute index */
#define MIB_TABLE_ETHEXTPMDATA_ATTR_NUM (18)
#define MIB_TABLE_ETHEXTPMDATA_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_ETHEXTPMDATA_INTENDTIME_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_ETHEXTPMDATA_CONTROLBLOCK_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_ETHEXTPMDATA_DROPEVENTS_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_ETHEXTPMDATA_OCTETS_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_ETHEXTPMDATA_FRAMES_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_ETHEXTPMDATA_BROADCASTPACKETS_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_ETHEXTPMDATA_MULTICASTPACKETS_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_ETHEXTPMDATA_CRCERRPACKETS_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_ETHEXTPMDATA_UNDERSIZEPACKETS_INDEX ((MIB_ATTR_INDEX)10)
#define MIB_TABLE_ETHEXTPMDATA_OVERSIZEPACKETS_INDEX ((MIB_ATTR_INDEX)11)
#define MIB_TABLE_ETHEXTPMDATA_OCTETS64_INDEX ((MIB_ATTR_INDEX)12)
#define MIB_TABLE_ETHEXTPMDATA_OCTETS65TO127_INDEX ((MIB_ATTR_INDEX)13)
#define MIB_TABLE_ETHEXTPMDATA_OCTETS128TO255_INDEX ((MIB_ATTR_INDEX)14)
#define MIB_TABLE_ETHEXTPMDATA_OCTETS256TO511_INDEX ((MIB_ATTR_INDEX)15)
#define MIB_TABLE_ETHEXTPMDATA_OCTETS512TO1023_INDEX ((MIB_ATTR_INDEX)16)
#define MIB_TABLE_ETHEXTPMDATA_OCTETS1024TO1518_INDEX ((MIB_ATTR_INDEX)17)
#define MIB_TABLE_ETHEXTPMDATA_LAST_HISTORY_PTR_INDEX ((MIB_ATTR_INDEX)18)

/* Table EthExtPmData attribute len, only string attrubutes have length definition */
#define MIB_TABLE_ETHEXTPMDATA_CTRL_BLK_LEN (16)

typedef enum {
    EFEPM_PARENT_ME_CLASS_MBCD          		= 46,
    EFEPM_PARENT_ME_CLASS_MBPCD             	= 47,
    EFEPM_PARENT_ME_CLASS_PPTP_ETH_UNI     		= 11,
    EFEPM_PARENT_ME_CLASS_PPTP_XDSL_UNI_PART_1	= 98,
    EFEPM_PARENT_ME_CLASS_GEM_IWTP              = 266,
    EFEPM_PARENT_ME_CLASS_MCAST_GEM_IWTP        = 281,
    EFEPM_PARENT_ME_CLASS_VEIP  				= 329,
    EFEPM_PARENT_ME_CLASS_PPTP_MOCA_UNI         = 162,
} efepm_attr_ctrl_blk_parent_me_class_t;

typedef enum {
	EFEPM_CTRL_FIELDS_DIRECTION_UPSTREAM	= 0,
	EFEPM_CTRL_FIELDS_DIRECTION_DOWNSTREAM	= 1,
} efepm_attr_ctrl_blk_ctrl_fields_direction_t;

typedef enum {
	EFEPM_CTRL_FIELDS_BIT_CONTINUOUS_ACCUM_MODE		= 0,
	EFEPM_CTRL_FIELDS_BIT_DIRECTIONALITY			= 1,
	EFEPM_CTRL_FIELDS_BIT_P_BIT_FILTERING			= 14,
	EFEPM_CTRL_FIELDS_BIT_VID_BIT_FILTERING			= 15,
} efepm_attr_ctrl_blk_ctrl_fields_bit_t;

// Table EthExtPmData entry stucture
typedef struct {
	omci_me_attr_ext_pm_ctrl_blk_t	Common;
	UINT16							TCI;
	UINT16							Reserved;
} __attribute__((packed)) efepm_attr_ctrl_blk_t;

typedef struct {
	UINT16   EntityId;
	UINT8    IntEndTime;
	UINT8    ControlBlock[MIB_TABLE_ETHEXTPMDATA_CTRL_BLK_LEN];
	UINT32   DropEvents;
	UINT32   Octets;
	UINT32   Frames;
	UINT32   BroadcastPackets;
	UINT32   MulticastPackets;
	UINT32   CrcErrPackets;
	UINT32   UndersizePackets;
	UINT32   OversizePackets;
	UINT32   Octets64;
	UINT32   Octets65to127;
	UINT32   Octets128to255;
	UINT32   Octets256to511;
	UINT32   Octets512to1023;
	UINT32   Octets1024to1518;
	void	 *pLastHistory;
} __attribute__((aligned)) MIB_TABLE_ETHEXTPMDATA_T;


#ifdef __cplusplus
}
#endif

#endif
