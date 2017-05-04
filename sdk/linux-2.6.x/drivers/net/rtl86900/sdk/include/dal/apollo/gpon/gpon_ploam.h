/*
 * Copyright (C) 2011 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * 
 * 
 * $Revision: 39101 $
 * $Date: 2013-05-03 17:35:27 +0800 (Fri, 03 May 2013) $
 *
 * Purpose : GPON MAC Driver PLOAM Processor
 *
 * Feature : GPON MAC Driver PLOAM Processor
 *
 */
#ifndef __GPON_PLOAM_H__
#define __GPON_PLOAM_H__

int32 gpon_ploam_tx(gpon_dev_obj_t* obj, int32        urgent, rtk_gpon_ploam_t* ploam);

int32 gpon_ploam_rx(gpon_dev_obj_t* obj);

int32 gpon_ploam_init(gpon_dev_obj_t* obj);

int32 gpon_ploam_deinit(gpon_dev_obj_t* obj);

int32 gpon_ploam_snPloam_set(gpon_dev_obj_t* obj);

void gpon_ploam_usUrgPloamQ_empty_report(gpon_dev_obj_t* obj);

void gpon_ploam_usNrmPloamQ_Empty_report(gpon_dev_obj_t* obj);



#endif  /* __GPON_PLOAM_H__ */

