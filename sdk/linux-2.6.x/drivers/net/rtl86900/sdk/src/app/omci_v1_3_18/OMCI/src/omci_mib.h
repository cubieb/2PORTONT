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
 * Purpose : Definition of OMCI MIB related info
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI MIB definition
 */

#ifndef __OMCI_MIB_H__
#define __OMCI_MIB_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "../inc/omci_protocol.h"


typedef enum
{
    OMCI_ME_INIT_TYPE_ONU,
    OMCI_ME_INIT_TYPE_OLT,
} omci_me_init_type_t;

typedef enum
{
	// single selection
    OMCI_ME_TYPE_STANDARD       = 0x01,
    OMCI_ME_TYPE_STANDARD_PM    = 0x02,
    OMCI_ME_TYPE_PROPRIETARY    = 0x04,
    OMCI_ME_TYPE_PROPRIETARY_PM = 0x08,
    OMCI_ME_TYPE_PRIVATE        = 0x10,

	// multiple selection
    OMCI_ME_TYPE_NOT_MIB_UPLOAD = 0x100,
} omci_me_type_t;

typedef enum
{
    OMCI_ME_ATTR_ACCESS_READ	= 0x1,
    OMCI_ME_ATTR_ACCESS_WRITE	= 0x2,
    OMCI_ME_ATTR_ACCESS_SBC		= 0x4,
} omci_me_attr_access_mask_t;

typedef enum
{
	// single selection
	OMCI_ME_ATTR_TYPE_MANDATORY			= 0x01,
	OMCI_ME_ATTR_TYPE_M_NOT_SUPPORT		= 0x02,
	OMCI_ME_ATTR_TYPE_OPTIONAL			= 0x04,
	OMCI_ME_ATTR_TYPE_O_NOT_SUPPORT		= 0x08,
	OMCI_ME_ATTR_TYPE_PRIVATE			= 0x10,

	// multiple selection
    OMCI_ME_ATTR_TYPE_PM				= 0x100,
    OMCI_ME_ATTR_TYPE_TABLE				= 0x200,
} omci_me_attr_type_mask_t;

/* G.988 A.1.4.3 - Alarm reporting control */
typedef enum {
    OMCI_ME_ATTR_ARC_DISABLED	= 0,
    OMCI_ME_ATTR_ARC_ENABLED	= 1,
} omci_me_attr_arc_t;

typedef enum {
    OMCI_ME_ATTR_ARC_INTERVAL_EXPIRED_NOW		= 0,
    OMCI_ME_ATTR_ARC_INTERVAL_NEVER_EXPIRED		= 255,
} omci_me_attr_arc_interval_t;

/* G.988 A.1.6 - Administrative state considerations */
typedef enum {
    OMCI_ME_ATTR_ADMIN_STATE_UNLOCK		= 0,
    OMCI_ME_ATTR_ADMIN_STATE_LOCK		= 1,
} omci_me_attr_admin_state_t;

typedef enum {
    OMCI_ME_ATTR_OP_STATE_ENABLED   = 0,
    OMCI_ME_ATTR_OP_STATE_DISABLED	= 1,
} omci_me_attr_op_state_t;

typedef UINT32 omci_me_action_mask_t;

enum
{
    OMCI_ME_ACTION_CREATE				= (1 << OMCI_MSG_TYPE_CREATE),
    OMCI_ME_ACTION_DELETE				= (1 << OMCI_MSG_TYPE_DELETE),
    OMCI_ME_ACTION_SET					= (1 << OMCI_MSG_TYPE_SET),
    OMCI_ME_ACTION_GET					= (1 << OMCI_MSG_TYPE_GET),
    OMCI_ME_ACTION_GET_ALL_ALARMS		= (1 << OMCI_MSG_TYPE_GET_ALL_ALARMS),
    OMCI_ME_ACTION_GET_ALL_ALARMS_NEXT	= (1 << OMCI_MSG_TYPE_GET_ALL_ALARMS_NEXT),
    OMCI_ME_ACTION_MIB_UPLOAD			= (1 << OMCI_MSG_TYPE_MIB_UPLOAD),
    OMCI_ME_ACTION_MIB_UPLOAD_NEXT		= (1 << OMCI_MSG_TYPE_MIB_UPLOAD_NEXT),
    OMCI_ME_ACTION_MIB_RESET			= (1 << OMCI_MSG_TYPE_MIB_RESET),
    OMCI_ME_ACTION_ALARM				= (1 << OMCI_MSG_TYPE_ALARM),
    OMCI_ME_ACTION_ATTR_VALUE_CHANGE	= (1 << OMCI_MSG_TYPE_ATTR_VALUE_CHANGE),
    OMCI_ME_ACTION_TEST					= (1 << OMCI_MSG_TYPE_TEST),
    OMCI_ME_ACTION_START_SW_DOWNLOAD	= (1 << OMCI_MSG_TYPE_START_SW_DOWNLOAD),
    OMCI_ME_ACTION_DOWNLOAD_SECTION		= (1 << OMCI_MSG_TYPE_DOWNLOAD_SECTION),
    OMCI_ME_ACTION_END_SW_DOWNLOAD		= (1 << OMCI_MSG_TYPE_END_SW_DOWNLOAD),
    OMCI_ME_ACTION_ACTIVATE_SW			= (1 << OMCI_MSG_TYPE_ACTIVATE_SW),
    OMCI_ME_ACTION_COMMIT_SW			= (1 << OMCI_MSG_TYPE_COMMIT_SW),
    OMCI_ME_ACTION_SYNCHRONIZE_TIME		= (1 << OMCI_MSG_TYPE_SYNCHRONIZE_TIME),
    OMCI_ME_ACTION_REBOOT				= (1 << OMCI_MSG_TYPE_REBOOT),
    OMCI_ME_ACTION_GET_NEXT				= (1 << OMCI_MSG_TYPE_GET_NEXT),
    OMCI_ME_ACTION_TEST_RESULT			= (1 << OMCI_MSG_TYPE_TEST_RESULT),
    OMCI_ME_ACTION_GET_CURRENT_DATA		= (1 << OMCI_MSG_TYPE_GET_CURRENT_DATA),
    OMCI_ME_ACTION_SET_TABLE			= (1 << OMCI_MSG_TYPE_SET_TABLE)
};

/* G.988 Table 9.1.5-1 - Plug-in unit types */
typedef enum {
	OMCI_PLUGIN_UNIT_TYPE_10BASE_T					= 22,
	OMCI_PLUGIN_UNIT_TYPE_100BASE_T					= 23,
	OMCI_PLUGIN_UNIT_TYPE_10_100BASE_T				= 24,
	OMCI_PLUGIN_UNIT_TYPE_POTS						= 32,
	OMCI_PLUGIN_UNIT_TYPE_GIGABIT_OPTICAL_ETHERNET	= 34,
	OMCI_PLUGIN_UNIT_TYPE_XDSL						= 35,
	OMCI_PLUGIN_UNIT_TYPE_VDSL						= 37,
	OMCI_PLUGIN_UNIT_TYPE_VIDEO_SERVICE				= 38,
	OMCI_PLUGIN_UNIT_TYPE_LCT						= 39,
	OMCI_PLUGIN_UNIT_TYPE_802_11					= 40,
	OMCI_PLUGIN_UNIT_TYPE_MOCA						= 46,
	OMCI_PLUGIN_UNIT_TYPE_10_100_1000BASE_T			= 47,
	OMCI_PLUGIN_UNIT_TYPE_VEIP						= 48,
	OMCI_PLUGIN_UNIT_TYPE_10G_ETHERNET				= 49,
	OMCI_PLUGIN_UNIT_TYPE_GPON_24881244				= 248,
} omci_plugin_unit_type_t;


#define OMCI_MIB_US_TM_ME_ID_BASE	(0x8000)

/* ONT-G  */
#define TXC_ONUG_INSTANCE_ID (0)


/* ONT2-G */
#define TXC_ONU2G_INSTANCE_ID (0)


/* CardHolder */

#define TXC_CARDHLD_SLOT_TYPE_PLUGGABLE		(0x00 << 8)
#define TXC_CARDHLD_SLOT_TYPE_INTEGRATED	(0x01 << 8)
#define TXC_CARDHLD_PON_SLOT_TYPE_ID  (0x80)
#define TXC_CARDHLD_GE_SLOT_TYPE_ID   (0x1)
#define TXC_CARDHLD_FE_SLOT_TYPE_ID   (0x4)
#define TXC_CARDHLD_VEIP_SLOT_TYPE_ID (0x6)

#define m_omci_is_slot_id_valid(x)	(TXC_CARDHLD_PON_SLOT_TYPE_ID == x || \
										TXC_CARDHLD_FE_SLOT_TYPE_ID == x || \
										TXC_CARDHLD_GE_SLOT_TYPE_ID == x || \
										TXC_CARDHLD_VEIP_SLOT_TYPE_ID == x)

#define OMCI_IS_PLUGIN_UNIT_TYPE_VALID(x) (OMCI_PLUGIN_UNIT_TYPE_GPON_24881244 == x || \
											OMCI_PLUGIN_UNIT_TYPE_10_100BASE_T == x || \
											OMCI_PLUGIN_UNIT_TYPE_10_100_1000BASE_T == x || \
											OMCI_PLUGIN_UNIT_TYPE_VEIP == x)

// ethernet mode: ( 4 X FE + 1 X GE ) or ( 2 X GE ), port id from 0 to 4


#define TXC_CARDHLD_PON_SLOT     (0)
#define TXC_CARDHLD_ETH_GE_SLOT  (1)
#define TXC_CARDHLD_ETH_FE_SLOT  (2)
#define TXC_CARDHLD_VEIP_SLOT    (3)

#define TXC_CARDHLD_SLOT_NUM     (4)

#define TXC_GET_CARDTYPE_BY_SLOT_ID(slotId) (( slotId  == TXC_CARDHLD_PON_SLOT)    ? (OMCI_PLUGIN_UNIT_TYPE_GPON_24881244) :   \
( slotId  == TXC_CARDHLD_ETH_FE_SLOT) ? (OMCI_PLUGIN_UNIT_TYPE_10_100BASE_T)  :   \
( slotId  == TXC_CARDHLD_ETH_GE_SLOT) ? (OMCI_PLUGIN_UNIT_TYPE_10_100_1000BASE_T)  :   \
OMCI_PLUGIN_UNIT_TYPE_VEIP)

#define TXC_GET_SLOT_NUM_BY_SLOT_ID(slotId) ((slotId) == TXC_CARDHLD_PON_SLOT ? TXC_CARDHLD_PON_SLOT_TYPE_ID : \
                                             (slotId) == TXC_CARDHLD_ETH_FE_SLOT ? TXC_CARDHLD_FE_SLOT_TYPE_ID : \
                                             (slotId) == TXC_CARDHLD_ETH_GE_SLOT ? TXC_CARDHLD_GE_SLOT_TYPE_ID : \
					     					 TXC_CARDHLD_VEIP_SLOT_TYPE_ID)


// not sure
#define TXC_PRIO_Q_MAX_SIZE        (0xCCC)


/*SWImage*/
#define TXC_DEFAULT_SW_VERSION    "RTL9600V1.01"

#define OMCI_SW_DL_IMAGE_UBOOT_FLAG_NAME	(32)
#define OMCI_SW_DL_IMAGE_UBOOT_FLAG_BUFFER	(32)
#define OMCI_SW_DL_IMAGE_UBOOT_FLAG_COMMAND_BUFFER \
	(OMCI_SW_DL_IMAGE_UBOOT_FLAG_NAME + OMCI_SW_DL_IMAGE_UBOOT_FLAG_BUFFER + 32)

#define OMCI_BOA_MIB_BUFFER_LEN             (256)
#ifdef __cplusplus
}
#endif

#endif
