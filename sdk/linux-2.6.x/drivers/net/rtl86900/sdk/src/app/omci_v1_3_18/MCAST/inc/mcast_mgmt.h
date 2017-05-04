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
 * Purpose : Definition of rtk/rg or customized snooping mode related define
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1)
 */
#ifndef __MCAST_MGMT_H__
#define __MCAST_MGMT_H__

#ifdef  __cplusplus
extern "C" {
#endif


#include "mcast_wrapper.h"

typedef enum omci_mcast_snp_mode_s
{
    SNP_MODE_RTK = 0,
    SNP_MODE_RG,
	SNP_MODE_END,
} omci_mcast_snp_mode_t;

typedef struct omci_mcast_wrapper_info_s
{
    omci_mcast_snp_mode_t   mode_id;
    omci_mcast_wrapper_t 	*pWrapper;
} omci_mcast_wrapper_info_t;

extern omci_mcast_wrapper_t* omci_mcast_wrapper_find(omci_mcast_snp_mode_t snp_mode);

#ifdef  __cplusplus
}
#endif

#endif


