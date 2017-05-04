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
 * Purpose : Definition of OMCI external API related info
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI external API definition
 */

#ifndef __OMCI_API_H__
#define __OMCI_API_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "gos_linux.h"
#include "gos_type.h"

typedef enum
{
	PON_OMCI_CMD_SN_SET,
	PON_OMCI_CMD_SN_GET,
	PON_OMCI_CMD_LOG_SET,
	PON_OMCI_CMD_LOG_GET,
	PON_OMCI_CMD_LOGFILE_SET,
	PON_OMCI_CMD_LOGFILE_GET,
	PON_OMCI_CMD_TABLE_GET,
	PON_OMCI_CMD_DEVMODE_SET,
	PON_OMCI_CMD_DEVMODE_GET,
	PON_OMCI_CMD_DUAL_MGMT_MODE_SET,
    PON_OMCI_CMD_DUAL_MGMT_MODE_GET,
    PON_OMCI_CMD_LOID_SET,
    PON_OMCI_CMD_LOID_GET,
    PON_OMCI_CMD_LOIDAUTH_GET_RSP,
    PON_OMCI_CMD_FAKE_OK_SET,
    PON_OMCI_CMD_PM_SET,
    PON_OMCI_CMD_TM_SET,
    PON_OMCI_CMD_DRV_VERSION_GET,
    PON_OMCI_CMD_IOT_VLAN_CFG_SET,
    PON_OMCI_CMD_CFLAG_SET,
    PON_OMCI_CMD_CFLAG_GET,

    PON_OMCI_CMD_MIB_CREATE,
    PON_OMCI_CMD_MIB_DELETE,
	PON_OMCI_CMD_MIB_SET,
	PON_OMCI_CMD_MIB_GET,
	PON_OMCI_CMD_MIB_GET_CURRENT,
	PON_OMCI_CMD_MIB_ALARM_GET,
	PON_OMCI_CMD_MIB_RESET,
	PON_OMCI_CMD_MIB_ATTR_GET_RSP,

	PON_OMCI_CMD_DUMP_AVL_TREE,
	PON_OMCI_CMD_DUMP_QUEUE_MAP,
	PON_OMCI_CMD_DUMP_TREE_CONN,
	PON_OMCI_CMD_DUMP_SRV_FLOW,

    PON_OMCI_CMD_SIM_ALARM,
    PON_OMCI_CMD_SIM_AVC,
    PON_OMCI_CMD_DETECT_IOT_VLAN,
    PON_OMCI_CMD_GEN_DOT,
    PON_OMCI_CMD_SHOW_REG_MOD,
    PON_OMCI_CMD_SHOW_REG_API,

	PON_OMCI_CMD_END,
} PON_OMCI_CMD_TYPE;

typedef struct
{
	PON_OMCI_CMD_TYPE	cmd;
	int			 		state;
	unsigned int		level;
	int					tableId;
	unsigned short		entityId;
	unsigned char		sn[9];
	char				filename[64];
	char				value[64];
    unsigned char       type;
    unsigned char       mode;
    unsigned char       pri;
    unsigned short      vid;
} PON_OMCI_CMD_T;


GOS_ERROR_CODE omci_SendCmdAndGet(PON_OMCI_CMD_T* pData);


#ifdef __cplusplus
}
#endif

#endif
