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
 * Purpose :
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1)
 */


#ifndef __FEATURE_EXPORT_API_H__
#define __FEATURE_EXPORT_API_H__

#include <stdarg.h>

enum
{
    FEATURE_API_START                   = 0,
    FEATURE_API_MCAST_RTK_IPC_SET       = FEATURE_API_START,
    FEATURE_API_MCAST_RTK_IPC_GET,
    FEATURE_API_MCAST_RTK_IPC_CREATE,
    FEATURE_API_MCAST_RTK_IPC_DESTROY,
    FEATURE_API_L3SVC_MGMT_CFG_SET,
    FEATURE_API_BDP_00000001,
    FEATURE_API_BDP_00000004_DEL_CONN,
    FEATURE_API_BDP_00000004_UPDATE_CONN,
    FEATURE_API_BDP_00000004_DEL_PQ_CFG,
    FEATURE_API_BDP_00000004_UPDATE_PQ_CFG,
    FEATURE_API_BDP_00000004_SET_QID,
    FEATURE_API_BDP_00000004_DEL_QID,
    FEATURE_API_BDP_00000004_SET_GEM_FLOW,
    FEATURE_API_BDP_00000004_ACTIVE_BDG_CONN,
    FEATURE_API_BDP_00000008_CHECK_MODE,
    FEATURE_API_BDP_00000008_GEN_RULE,
    FEATURE_API_BDP_00000020,
    FEATURE_API_RDP_00000001_X_BDG_CONN,
    FEATURE_API_RDP_00000001_RESET_PPTP,
    FEATURE_API_MC_00000001,
    FEATURE_API_MC_00000002,
    FEATURE_API_ME_00000002,
    FEATURE_API_ME_00000004,
    FEATURE_API_ME_00000008,
    FEATURE_API_END
};

typedef enum
{
    FAL_OK                  = 0x100,
    FAL_FAIL,
    FAL_ERR_DISABLE,
    FAL_ERR_NOT_FOUND,
    FAL_ERR_NOT_INIT,
    FAL_ERR_NOT_REGISTER,
    FAL_ERR_UNKNOWN         = 0x1FF
}FAL_ERROR_CODE;

unsigned int feature_api_register(unsigned int apiID, unsigned int moduleID, unsigned int (*regFun)(va_list argp));
void feature_api_unregister(unsigned int apiID);

#endif

