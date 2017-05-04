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
 * Purpose : Definition of ME attribute: VoIP line status (141)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: VoIP line status (141)
 */

#ifndef __MIB_VOIP_LINE_STATUS_H__
#define __MIB_VOIP_LINE_STATUS_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_VOIP_LINE_STATUS_ATTR_NUM (10)
#define MIB_TABLE_VOIP_LINE_STATUS_ENTITY_ID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_VOIP_LINE_STATUS_VOIP_CODEC_USED_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_VOIP_LINE_STATUS_VOIP_VOICE_SERVER_STATUS_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_VOIP_LINE_STATUS_VOIP_PORT_SESSION_TYPE_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_VOIP_LINE_STATUS_VOIP_CALL_1_PKT_PERIOD_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_VOIP_LINE_STATUS_VOIP_CALL_2_PKT_PERIOD_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_VOIP_LINE_STATUS_VOIP_CALL_1_DST_ADDR_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_VOIP_LINE_STATUS_VOIP_CALL_2_DST_ADDR_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_VOIP_LINE_STATUS_VOIP_LINE_STATE_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_VOIP_LINE_STATUS_EMERGENCY_CALL_STATUS_INDEX ((MIB_ATTR_INDEX)10)

#define MIB_TABLE_VOIP_LINE_STATUS_VOIP_CALL_DST_ADDR_LEN (25)


typedef enum {
    VLS_VOIP_CODEC_USED_PCMU        = 0,
    VLS_VOIP_CODEC_USED_GSM         = 3,
    VLS_VOIP_CODEC_USED_ITUT_G723   = 4,
    VLS_VOIP_CODEC_USED_DVI4_8KHZ	= 5,
    VLS_VOIP_CODEC_USED_DVI4_16KHZ	= 6,
    VLS_VOIP_CODEC_USED_LPC         = 7,
    VLS_VOIP_CODEC_USED_PCMA        = 8,
    VLS_VOIP_CODEC_USED_ITUT_G722   = 9,
    VLS_VOIP_CODEC_USED_L16_2CH     = 10,
    VLS_VOIP_CODEC_USED_L16_1CH     = 11,
    VLS_VOIP_CODEC_USED_QCELP       = 12,
    VLS_VOIP_CODEC_USED_CN          = 13,
    VLS_VOIP_CODEC_USED_MPA         = 14,
    VLS_VOIP_CODEC_USED_ITUT_G728   = 15,
    VLS_VOIP_CODEC_USED_DVI4_11KHZ  = 16,
    VLS_VOIP_CODEC_USED_DVI4_22KHZ  = 17,
    VLS_VOIP_CODEC_USED_ITUT_G729   = 18
} vls_attr_voip_codec_used_t;

typedef enum {
    VLS_SERVER_STATUS_NONE_INITIAL                  = 0,
    VLS_SERVER_STATUS_REGISTERED                    = 1,
    VLS_SERVER_STATUS_IN_SESSION                    = 2,
    VLS_SERVER_STATUS_FAIL_REG_ICMP_ERR             = 3,
    VLS_SERVER_STATUS_FAIL_REG_FAIL_TCP             = 4,
    VLS_SERVER_STATUS_FAIL_REG_FAIL_AUTH            = 5,
    VLS_SERVER_STATUS_FAIL_REG_TIMEOUT              = 6,
    VLS_SERVER_STATUS_FAIL_REG_SERVER_FAIL_CODE     = 7,
    VLS_SERVER_STATUS_FAIL_INV_ICMP_ERR             = 8,
    VLS_SERVER_STATUS_FAIL_INV_FAIL_TCP             = 9,
    VLS_SERVER_STATUS_FAIL_INV_FAIL_AUTH            = 10,
    VLS_SERVER_STATUS_FAIL_INV_TIMEOUT              = 11,
    VLS_SERVER_STATUS_FAIL_INV_SERVER_FAIL_CODE     = 12,
    VLS_SERVER_STATUS_PORT_NOT_CONFIGURED           = 13,
    VLS_SERVER_STATUS_CONFIG_DONE                   = 14,
    VLS_SERVER_STATUS_DISABLED_BY_SWITCH            = 15
} vls_attr_voip_voice_server_status_t;

typedef enum {
    VLS_PORT_SESSION_TYPE_IDLE_NONE     = 0,
    VLS_PORT_SESSION_TYPE_2WAY          = 1,
    VLS_PORT_SESSION_TYPE_3WAY          = 2,
    VLS_PORT_SESSION_TYPE_FAX_MODEM     = 3,
    VLS_PORT_SESSION_TYPE_TELEMETRY     = 4,
    VLS_PORT_SESSION_TYPE_CONFERENCE    = 5
} vls_attr_voip_port_session_type_t;

typedef enum {
    VLS_LINE_STATE_IDLE_ON_HOOK                 = 0,
    VLS_LINE_STATE_OFF_HOOK_DIAL_TONE           = 1,
    VLS_LINE_STATE_DIALING                      = 2,
    VLS_LINE_STATE_RINGING_FSK_ALERTING_DATA    = 3,
    VLS_LINE_STATE_AUDIBLE_RINGBACK             = 4,
    VLS_LINE_STATE_CONNECTING                   = 5,
    VLS_LINE_STATE_CONNECTED                    = 6,
    VLS_LINE_STATE_DISCONNECTING_AUDIBLE_IND    = 7,
    VLS_LINE_STATE_ROH_NO_TONE                  = 8,
    VLS_LINE_STATE_ROH_WITH_TONE                = 9,
    VLS_LINE_STATE_UNKNOWN_OR_UNDEFINED         = 10
} vls_attr_voip_line_state_t;

typedef struct {
	UINT16	EntityId;
	UINT16	VoipCodecUsed;
	UINT8	VoipVoiceServerStatus;
	UINT8	VoipPortSessionType;
	UINT16	VoipCall1PktPeriod;
	UINT16	VoipCall2PktPeriod;
    CHAR    VoipCall1DstAddr[MIB_TABLE_VOIP_LINE_STATUS_VOIP_CALL_DST_ADDR_LEN+1];
    CHAR    VoipCall2DstAddr[MIB_TABLE_VOIP_LINE_STATUS_VOIP_CALL_DST_ADDR_LEN+1];
	UINT8	VoipLineState;
	UINT8	EmergencyCallStatus;
} __attribute__((aligned)) MIB_TABLE_VOIP_LINE_STATUS_T;


#ifdef __cplusplus
}
#endif

#endif
