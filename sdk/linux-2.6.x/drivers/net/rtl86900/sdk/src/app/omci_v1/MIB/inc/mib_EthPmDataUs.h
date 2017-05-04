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
 * Purpose : Definition of ME attribute: Ethernet frame PMHD US (322)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Ethernet frame PMHD US (322)
 */

#ifndef __MIB_ETHPMDATAUS_TABLE_H__
#define __MIB_ETHPMDATAUS_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table EthPmDataUs attribute index */
#define MIB_TABLE_ETHPMDATAUS_ATTR_NUM (17)
#define MIB_TABLE_ETHPMDATAUS_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_ETHPMDATAUS_INTENDTIME_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_ETHPMDATAUS_THRESHOLDID_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_ETHPMDATAUS_DROPEVENTS_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_ETHPMDATAUS_OCTETS_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_ETHPMDATAUS_PACKETS_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_ETHPMDATAUS_BROADCASTPACKETS_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_ETHPMDATAUS_MULTICASTPACKETS_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_ETHPMDATAUS_CRCERRPACKETS_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_ETHPMDATAUS_UNDERSIZEPACKETS_INDEX ((MIB_ATTR_INDEX)10)
#define MIB_TABLE_ETHPMDATAUS_OVERSIZEPACKETS_INDEX ((MIB_ATTR_INDEX)11)
#define MIB_TABLE_ETHPMDATAUS_OCTETS64_INDEX ((MIB_ATTR_INDEX)12)
#define MIB_TABLE_ETHPMDATAUS_OCTETS65TO127_INDEX ((MIB_ATTR_INDEX)13)
#define MIB_TABLE_ETHPMDATAUS_OCTETS128TO255_INDEX ((MIB_ATTR_INDEX)14)
#define MIB_TABLE_ETHPMDATAUS_OCTETS256TO511_INDEX ((MIB_ATTR_INDEX)15)
#define MIB_TABLE_ETHPMDATAUS_OCTETS512TO1023_INDEX ((MIB_ATTR_INDEX)16)
#define MIB_TABLE_ETHPMDATAUS_OCTETS1024TO1518_INDEX ((MIB_ATTR_INDEX)17)

/* Table EthPmDataUs attribute len, only string attrubutes have length definition */

// Table EthPmDataUs entry stucture
typedef struct {
	UINT16   EntityId;
	UINT8    IntEndTime;
	UINT16   ThresholdID;
	UINT32   DropEvents;
	UINT32   Octets;
	UINT32   Packets;
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
} __attribute__((aligned)) MIB_TABLE_ETHPMDATAUS_T;


#ifdef __cplusplus
}
#endif

#endif
