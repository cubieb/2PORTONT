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
 * Purpose : Definition of ME attribute: Call control PMHD (140)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Call control PMHD (140)
 */

#ifndef __MIB_CALL_CTRL_PMHD_TABLE_H__
#define __MIB_CALL_CTRL_PMHD_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_CALL_CTRL_PMHD_ATTR_NUM (8)
#define MIB_TABLE_CALL_CTRL_PMHD_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_CALL_CTRL_PMHD_INTERVALENDTIME_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_CALL_CTRL_PMHD_THRESHOLDDATA12ID_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_CALL_CTRL_PMHD_CALL_SETUP_FAILURES_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_CALL_CTRL_PMHD_CALL_SETUP_TIMER_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_CALL_CTRL_PMHD_CALL_TERMINATE_FAILURES_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_CALL_CTRL_PMHD_ANALOG_PORT_RELEASES_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_CALL_CTRL_PMHD_ANALOG_PORT_OFF_HOOK_TIMER_INDEX ((MIB_ATTR_INDEX)8)


typedef struct {
	UINT16   EntityId;
	UINT8    IntEndTime;
	UINT16   ThresholdID;
	UINT32   CallSetupFailures;
	UINT32   CallSetupTimer;
	UINT32   CallTerminateFailures;
	UINT32   AnalogPortReleases;
	UINT32   AnalogPortOffHookTimer;
} __attribute__((aligned)) MIB_TABLE_CALL_CTRL_PMHD_T;


#ifdef __cplusplus
}
#endif

#endif
