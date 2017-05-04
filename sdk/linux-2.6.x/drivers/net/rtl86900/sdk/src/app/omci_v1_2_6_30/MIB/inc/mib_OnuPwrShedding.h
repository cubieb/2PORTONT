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
 * Purpose : Definition of ME attribute: ONU power shedding (133)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: ONU power shedding (133)
 */

#ifndef __MIB_ONU_POWER_SHEDDING_H__
#define __MIB_ONU_POWER_SHEDDING_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_ONU_PWR_SHEDDING_ATTR_NUM (16)
#define MIB_TABLE_ONU_PWR_SHEDDING_ENTITY_ID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_ONU_PWR_SHEDDING_RESTORE_PWR_INTVL_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_ONU_PWR_SHEDDING_DATA_SHED_INTVL_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_ONU_PWR_SHEDDING_VOICE_SHED_INTVL_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_ONU_PWR_SHEDDING_VIDEO_OVERLAY_SHED_INTVL_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_ONU_PWR_SHEDDING_VIDEO_RETURN_SHED_INTVL_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_ONU_PWR_SHEDDING_DSL_SHED_INTVL_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_ONU_PWR_SHEDDING_ATM_SHED_INTVL_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_ONU_PWR_SHEDDING_CES_SHED_INTVL_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_ONU_PWR_SHEDDING_FRAME_SHED_INTVL_INDEX ((MIB_ATTR_INDEX)10)
#define MIB_TABLE_ONU_PWR_SHEDDING_SDH_SONET_SHED_INTVL_INDEX ((MIB_ATTR_INDEX)11)
#define MIB_TABLE_ONU_PWR_SHEDDING_SHEDDING_STATUS_INDEX ((MIB_ATTR_INDEX)12)
#define MIB_TABLE_ONU_PWR_SHEDDING_DATA_SHED_INTVL_REMAINED_INDEX ((MIB_ATTR_INDEX)13)
#define MIB_TABLE_ONU_PWR_SHEDDING_DATA_RESET_INTVL_REMAINED_INDEX ((MIB_ATTR_INDEX)14)
#define MIB_TABLE_ONU_PWR_SHEDDING_VOICE_SHED_INTVL_REMAINED_INDEX ((MIB_ATTR_INDEX)15)
#define MIB_TABLE_ONU_PWR_SHEDDING_VOICE_RESET_INTVL_REMAINED_INDEX ((MIB_ATTR_INDEX)16)


typedef enum {
    OPS_SHEDDING_INTVL_DISABLE_SHEDDING		= 0,
    OPS_SHEDDING_INTVL_IMMEDIATE_SHEDDING	= 1,
} ops_attr_shedding_interval_t;

typedef enum {
    OPS_SHEDDING_STATUS_DATA_CLASS				= (1 << 15),
    OPS_SHEDDING_STATUS_VOICE_CLASS				= (1 << 14),
    OPS_SHEDDING_STATUS_VIDEO_OVERLAY_CLASS		= (1 << 13),
    OPS_SHEDDING_STATUS_VIDEO_RETURN_CLASS		= (1 << 12),
    OPS_SHEDDING_STATUS_DSL_CLASS				= (1 << 11),
    OPS_SHEDDING_STATUS_ATM_CLASS				= (1 << 10),
    OPS_SHEDDING_STATUS_CES_CLASS				= (1 << 9),
    OPS_SHEDDING_STATUS_FRAME_CLASS				= (1 << 8),
    OPS_SHEDDING_STATUS_SDH_SONET_CLASS			= (1 << 7)
} ops_attr_shedding_status_class_t;

typedef struct {
	UINT16	EntityId;
	UINT16	RestorePwrIntvl;
	UINT16	DataShedIntvl;
	UINT16	VoiceShedIntvl;
	UINT16	VideoOverlayShedIntvl;
	UINT16	VideoReturnShedIntvl;
	UINT16	DslShedIntvl;
	UINT16	AtmShedIntvl;
	UINT16	CesShedIntvl;
	UINT16	FrameShedIntvl;
	UINT16	SdhSonetShedIntvl;
	UINT16	SheddingStatus;
	UINT16	DataShedIntvlRemained;
	UINT16	DataResetIntvlRemained;
	UINT16	VoiceShedIntvlRemained;
	UINT16	VoiceResetIntvlRemained;
} __attribute__((aligned)) MIB_TABLE_ONU_PWR_SHEDDING_T;


#ifdef __cplusplus
}
#endif

#endif
