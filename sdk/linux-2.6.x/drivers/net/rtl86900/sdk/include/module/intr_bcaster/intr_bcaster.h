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
 * Purpose : Define interrupt broadcaster
 *
 * Feature : Broadcast interrupt event to kernel/user space
 */

#ifndef __INTR_BCASTER_H__
#define __INTR_BCASTER_H__


//#define USE_GENERIC_NETLINK_INTF


#ifdef USE_GENERIC_NETLINK_INTF
#include <net/genetlink.h>
#endif
#include "common/type.h"
#include "rtk/intr.h"


#ifdef USE_GENERIC_NETLINK_INTF
/* generic netlink attributes */
enum {
    INTR_BCASTER_GENL_ATTR_UNSPEC,
    INTR_BCASTER_GENL_ATTR_EVENT,
    __INTR_BCASTER_GENL_ATTR_MAX,
};
#define INTR_BCASTER_GENL_ATTR_MAX  (__INTR_BCASTER_GENL_ATTR_MAX - 1)

/* generic netlink commands */
enum {
    INTR_BCASTER_GENL_CMD_UNSPEC,
    INTR_BCASTER_GENL_CMD_EVENT,
    __INTR_BCASTER_GENL_CMD_MAX,
};
#define INTR_BCASTER_GENL_CMD_MAX   (__INTR_BCASTER_GENL_CMD_MAX - 1)

#define INTR_BCASTER_GENL_FAMILY_NAME               "intr_bcaster"
#define INTR_BCASTER_GENL_VERSION                   0x01
#define INTR_BCASTER_GENL_MCGRP_NAME_LINK_CHANGE    "link_change"
#define INTR_BCASTER_GENL_MCGRP_NAME_DYING_GASP     "dying_gasp"

/* generic netlink family definition */
static struct genl_family gIntrBcasterGenlFamily = {
    .id = GENL_ID_GENERATE,
    .name = INTR_BCASTER_GENL_FAMILY_NAME,
    .version = INTR_BCASTER_GENL_VERSION,
    .maxattr = INTR_BCASTER_GENL_ATTR_MAX,
};

/* generic netlink multicast group definition */
static struct genl_multicast_group gIntrBcasterGenlMcGrpLinkChange = {
    .name = INTR_BCASTER_GENL_MCGRP_NAME_LINK_CHANGE,
};
static struct genl_multicast_group gIntrBcasterGenlMcGrpDyingGasp = {
    .name = INTR_BCASTER_GENL_MCGRP_NAME_DYING_GASP,
};
#else
#define INTR_BCASTER_NETLINK_TYPE   NETLINK_UNUSED
#endif

typedef enum bcast_msg_type_e
{
	// interrupt
	MSG_TYPE_LINK_CHANGE = 0,
	MSG_TYPE_METER_EXCEED,
	MSG_TYPE_LEARN_OVER,
	MSG_TYPE_SPEED_CHANGE,
	MSG_TYPE_SPECIAL_CONGEST,
	MSG_TYPE_LOOP_DETECTION,
	MSG_TYPE_CABLE_DIAG_FIN,
	MSG_TYPE_ACL_ACTION,
	MSG_TYPE_GPHY,
	MSG_TYPE_SERDES,
	MSG_TYPE_GPON,
	MSG_TYPE_EPON,
	MSG_TYPE_PTP,
	MSG_TYPE_DYING_GASP,
	MSG_TYPE_THERMAL,
	MSG_TYPE_ADC,
	MSG_TYPE_EEPROM_UPDATE_110OR118,
	MSG_TYPE_EEPROM_UPDATE_128TO247,
	MSG_TYPE_PKTBUFFER_ERROR,
	// gpon event
	MSG_TYPE_ONU_STATE = 20,	
    MSG_TYPE_RLDP_LOOP_STATE_CHNG,
	MSG_TYPE_END
} bcast_msg_type_t;

// interrupt broadcaster message
typedef struct {
    bcast_msg_type_t    intrType;
    uint32              intrSubType;
    uint32              intrBitMask;
    rtk_enable_t        intrStatus;
} intrBcasterMsg_t;

#define MAX_BYTE_PER_INTR_BCASTER_MSG   (sizeof(intrBcasterMsg_t))

// interrupt broadcaster notifier callback
typedef void (*intrBcasterNotifierCb_t)(intrBcasterMsg_t   *pMsgData);



// interrupt broadcaster notifier
typedef struct {
    bcast_msg_type_t            notifyType;
    intrBcasterNotifierCb_t     notifierCb;

    // for internal use only
    struct notifier_block       *pNbAddr;
} intrBcasterNotifier_t;


extern int intr_bcaster_notifier_cb_register(intrBcasterNotifier_t     *pRegNotifier);
extern int intr_bcaster_notifier_cb_unregister(intrBcasterNotifier_t     *pUnregNotifier);
int queue_broadcast ( bcast_msg_type_t  intrType,
                      uint32          intrSubType,
                      uint32          intrBitMask,
                      rtk_enable_t    intrStatus );


#endif
