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
 * Purpose : Definition of ME attribute: ONT-G (256)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: ONT-G (256)
 */

#ifndef __MIB_ONTG_TABLE_H__
#define __MIB_ONTG_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table Ontg attribute index */
#define MIB_TABLE_ONTG_ATTR_NUM (15)

/*ONT-G attribute index*/
enum {
	MIB_TABLE_ONTG_ENTITYID_INDEX=1,
	MIB_TABLE_ONTG_VID_INDEX,
	MIB_TABLE_ONTG_VERSION_INDEX,
	MIB_TABLE_ONTG_SERIALNUM_INDEX,
	MIB_TABLE_ONTG_TRAFFMGTOPT_INDEX,
	MIB_TABLE_ONTG_ATMCCOPT_INDEX,
	MIB_TABLE_ONTG_BATTERYBACK_INDEX,
	MIB_TABLE_ONTG_ADMINSTATE_INDEX,
	MIB_TABLE_ONTG_OPSTATE_INDEX,
    MIB_TABLE_ONTG_ONU_SURVIVAL_TIME_INDEX,
    MIB_TABLE_ONTG_LOGICAL_ONU_ID_INDEX,
    MIB_TABLE_ONTG_LOGICAL_PASSWORD_INDEX,
    MIB_TABLE_ONTG_CREDENTIALS_STATUS_INDEX,
    MIB_TABLE_ONTG_EXTENDED_TC_LAYER_OPTIONS_INDEX,
	MIB_TABLE_ONTG_ONTSTATE_INDEX
};


/* Table Ontg attribute len, only string attrubutes have length definition */
#define MIB_TABLE_ONTG_VID_LEN (4)
#define MIB_TABLE_ONTG_VERSION_LEN (14)
#define MIB_TABLE_ONTG_SERIALNUM_LEN (8)
#define MIB_TABLE_ONTG_LOID_LEN (24)
#define MIB_TABLE_ONTG_LP_LEN (12)

typedef enum {
    ONUG_TM_OPTION_PRIORITY_CONTROLLED,
    ONUG_TM_OPTION_RATE_CONTROLLED,
    ONUG_TM_OPTION_PRIORITY_RATE_CONTROLLED,
} onug_attr_traffic_mgmt_option_t;

// Table Ontg entry stucture
typedef struct {
    UINT16 EntityID; // index 1
    CHAR   VID[MIB_TABLE_ONTG_VID_LEN+1];
    CHAR   Version[MIB_TABLE_ONTG_VERSION_LEN+1];
    CHAR   SerialNum[MIB_TABLE_ONTG_SERIALNUM_LEN+1];
    UINT8  TraffMgtOpt;
    UINT8  AtmCCOpt;
    UINT8  BatteryBack;
    UINT8  AdminState;
    UINT8  OpState;
    UINT8  OnuSurvivalTime;
    CHAR   LogicalOnuID[MIB_TABLE_ONTG_LOID_LEN+1];
    CHAR   LogicalPassword[MIB_TABLE_ONTG_LP_LEN+1];
    UINT8  CredentialsStatus;
    UINT16 ExtendedTcLayerOptions;
    UINT8  OntState;
} __attribute__((aligned)) MIB_TABLE_ONTG_T;


#ifdef __cplusplus
}
#endif

#endif
