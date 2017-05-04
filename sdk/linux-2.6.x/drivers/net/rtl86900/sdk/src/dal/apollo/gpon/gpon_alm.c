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
 * Purpose : GMac Driver Alarm Management
 *
 * Feature : Provide the APIs of GMac Driver Alarm Management
 *
 */

#include <dal/apollo/gpon/gpon_defs.h>
#include <dal/apollo/gpon/gpon_fsm.h>
#include <dal/apollo/gpon/gpon_alm.h>
#include <dal/apollo/raw/apollo_raw_gpon.h>
#include <dal/apollo/gpon/gpon_debug.h>

int32 gpon_alarm_event(gpon_dev_obj_t* obj, rtk_gpon_alarm_type_t alarm, int32        status)
{
    int32        olds,news;
    if(alarm>=RTK_GPON_ALARM_MAX)
    {
        return RT_ERR_OUT_OF_RANGE;
    }

    if(obj->alarm_tbl[alarm]!=status)
    {
        GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"Alarm %s Event %d [baseaddr:%p]",gpon_dbg_alarm_str(alarm),status,obj->base_addr);
        olds = obj->alarm_tbl[RTK_GPON_ALARM_LOS] | obj->alarm_tbl[RTK_GPON_ALARM_LOF];
        obj->alarm_tbl[alarm] = status;
        if(alarm==RTK_GPON_ALARM_LOF || alarm==RTK_GPON_ALARM_LOS)
        {
            news = obj->alarm_tbl[RTK_GPON_ALARM_LOS] | obj->alarm_tbl[RTK_GPON_ALARM_LOF];

            if(olds!=news)
            {
                if(news)
                {
                    gpon_fsm_event(obj,GPON_FSM_EVENT_LOS_DETECT);
                }
                else
                {
                    gpon_fsm_event(obj,GPON_FSM_EVENT_LOS_CLEAR);
                }
            }
        }

        if(obj->fault_callback[alarm])
        {
            (*obj->fault_callback[alarm])(status);
        }
    }

    return RT_ERR_OK;
}

int32 gpon_alarm_status_get(gpon_dev_obj_t* obj, rtk_gpon_alarm_type_t alarm, int32 *pStatus)
{
    *pStatus = obj->alarm_tbl[alarm];
    return RT_ERR_OK;
}

void gpon_alarm_status_check(gpon_dev_obj_t* obj)
{
    uint32 los, lof, lom;

    apollo_raw_gpon_gtcDs_los_read(&los);
    apollo_raw_gpon_gtcDs_lof_read(&lof);
    apollo_raw_gpon_gtcDs_lom_read(&lom);

    gpon_alarm_event(obj,RTK_GPON_ALARM_LOS,los);
    gpon_alarm_event(obj,RTK_GPON_ALARM_LOF,lof);
    gpon_alarm_event(obj,RTK_GPON_ALARM_LOM,lom);
}

void gpon_alarm_status_reset(gpon_dev_obj_t* obj)
{
    int32        olds;
    uint32 i;

    olds = obj->alarm_tbl[RTK_GPON_ALARM_LOS] | obj->alarm_tbl[RTK_GPON_ALARM_LOF];
    if(!olds)
    {
        gpon_fsm_event(obj,GPON_FSM_EVENT_LOS_DETECT);
    }

    for(i=0;i<RTK_GPON_ALARM_MAX;i++)
    {
        if(!obj->alarm_tbl[i])
        {
            obj->alarm_tbl[i] = TRUE;
            if(obj->fault_callback[i])
            {
                (*obj->fault_callback[i])(TRUE);
            }
        }
    }
}

