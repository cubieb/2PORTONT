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
 * Purpose : GMac Driver FSM Processor
 *
 * Feature : GMac Driver FSM Processor
 *
 */

#ifndef __GPON_FSM_H__
#define __GPON_FSM_H__

void gpon_fsm_init(void);

void gpon_fsm_event(gpon_dev_obj_t *obj, rtk_gpon_fsm_event_t event);

void gpon_fsm_to1_expire(uint32 data);

void gpon_fsm_to2_expire(uint32 data);

#endif  /* __GPON_FSM_H__ */

