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
 * Purpose : Definition of Dual Management callbacks
 */

#ifndef __OMCI_DM_CB_H__
#define __OMCI_DM_CB_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "rtk/ponmac.h"
#include "rtk_rg_struct.h"
#include "omci_dm_sd.h"


typedef struct
{
	unsigned int				wanIdx;								// zero-based wan idx
	unsigned char				wanType;							// refers to if_smux.h
	rtk_rg_aclFilterAndQos_t	rgAclEntry;							// rg us acl entry
	rtk_rg_aclFilterAndQos_t	rgUsSwAclEntry;						// for bridge wan only
	unsigned int				usFlowId[WAN_PONMAC_QUEUE_MAX];		// pre-allocate us flow id
	unsigned int				gemPortId;							// pon gem port id
	rtk_ponmac_queueCfg_t		queueCfg[WAN_PONMAC_QUEUE_MAX];		// pon queue cfg
	unsigned char				queueSts[WAN_PONMAC_QUEUE_MAX];		// pon queue disable/enable
	unsigned int				tcontId;							// pon t-cont id
} omci_dm_pon_wan_info_t;

typedef struct
{
	int (*omci_dm_pon_wan_info_set)(omci_dm_pon_wan_info_t *pPonWanInfo);
	int (*omci_dm_pon_wan_info_del)(unsigned int wanIdx);
} omci_dmm_cb_t;


void omci_dmm_cb_register(omci_dmm_cb_t *p);
void omci_dmm_cb_unregister(void);


#ifdef __cplusplus
}
#endif

#endif
