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
 * Purpose : Definition of ME attribute: VoIP config data (138)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: VoIP config data (138)
 */

#ifndef __MIB_VOIP_CFG_DATA_H__
#define __MIB_VOIP_CFG_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_VOIP_CFG_DATA_ATTR_NUM (9)
#define MIB_TABLE_VOIP_CFG_DATA_ENTITY_ID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_VOIP_CFG_DATA_AVAILABLE_SIG_PROTOCOLS_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_VOIP_CFG_DATA_SIG_PROTOCOL_USED_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_VOIP_CFG_DATA_AVAILABLE_VOIP_CFG_METHODS_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_VOIP_CFG_DATA_VOIP_CFG_METHOD_USED_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_VOIP_CFG_DATA_VOIP_CFG_ADDR_PTR_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_VOIP_CFG_DATA_VOIP_CFG_STATE_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_VOIP_CFG_DATA_RETRIEVE_PROFILE_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_VOIP_CFG_DATA_PROFILE_VERSION_INDEX ((MIB_ATTR_INDEX)9)

#define MIB_TABLE_VOIP_CFG_DATA_PROFILE_VERSION_LEN (25)


typedef enum {
    VCD_AVAILABLE_SIG_PROTOCOL_SIP		= (1 << 0),
    VCD_AVAILABLE_SIG_PROTOCOL_H248		= (1 << 1),
    VCD_AVAILABLE_SIG_PROTOCOL_MGCP		= (1 << 2)
} vcd_attr_available_sig_protocol_t;

typedef enum {
    VCD_SIG_PROTOCOL_USED_NONE		= 0,
    VCD_SIG_PROTOCOL_USED_SIP		= 1,
    VCD_SIG_PROTOCOL_USED_H248		= 2,
    VCD_SIG_PROTOCOL_USED_MGCP		= 3,
    VCD_SIG_PROTOCOL_USED_NON_OMCI	= 0xFF
} vcd_attr_sig_protocol_used_t;

typedef enum {
    VCD_AVAILABLE_VOIP_CFG_METHOD_OMCI			= (1 << 0),
    VCD_AVAILABLE_VOIP_CFG_METHOD_CFG_FILE		= (1 << 1),
    VCD_AVAILABLE_VOIP_CFG_METHOD_TR069			= (1 << 2),
    VCD_AVAILABLE_VOIP_CFG_METHOD_SIPPING		= (1 << 3)
} vcd_attr_available_voip_cfg_method_t;

typedef enum {
    VCD_CFG_METHOD_USED_NONE		= 0,
    VCD_CFG_METHOD_USED_OMCI		= 1,
    VCD_CFG_METHOD_USED_CFG_FILE	= 2,
    VCD_CFG_METHOD_USED_TR069		= 3,
    VCD_CFG_METHOD_USED_SIPPING		= 4
} vcd_attr_voip_cfg_method_used_t;

typedef enum {
    VCD_CFG_STATE_INACTIVE		= 0,
    VCD_CFG_STATE_ACTIVE		= 1,
    VCD_CFG_STATE_INITIALIZING	= 2,
    VCD_CFG_STATE_FAULT			= 3
} vcd_attr_voip_cfg_state_t;

typedef struct {
	UINT16	EntityId;
	UINT8	AvailableSigProtocols;
	UINT8	SigProtocolUsed;
	UINT32	AvailableVoipCfgMethod;
	UINT8	VoipCfgMethodUsed;
	UINT16	VoipCfgAddrPtr;
	UINT8	VoipCfgState;
	UINT8	RetrieveProfile;
	CHAR	ProfileVersion[MIB_TABLE_VOIP_CFG_DATA_PROFILE_VERSION_LEN+1];
} __attribute__((aligned)) MIB_TABLE_VOIP_CFG_DATA_T;


#ifdef __cplusplus
}
#endif

#endif
