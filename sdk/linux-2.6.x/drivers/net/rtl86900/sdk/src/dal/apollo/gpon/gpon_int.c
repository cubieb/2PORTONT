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
 * Purpose : GMac Driver Interrupt Processor
 *
 * Feature : GMac Driver Interrupt Processor
 *
 */

#include <dal/apollo/gpon/gpon_defs.h>
#include <dal/apollo/gpon/gpon_int.h>
#include <dal/apollo/gpon/gpon_fsm.h>
#include <dal/apollo/gpon/gpon_ploam.h>
#include <dal/apollo/gpon/gpon_omci.h>
#include <dal/apollo/gpon/gpon_alm.h>
#include <dal/apollo/gpon/gpon_res.h>
#include <dal/apollo/raw/apollo_raw_gpon.h>

extern gpon_drv_obj_t *g_gponmac_drv;
void gpon_isr_entry(void)
{
    uint32 mask,stat,value;
    gpon_dev_obj_t* obj;
    uint32 los, lof, lom, ds_fec, us_fec;

    GPON_OS_Log(GPON_LOG_LEVEL_NORMAL,"dal_apollo_gpon_isr_entry");

    if(!g_gponmac_drv || !g_gponmac_drv->dev || g_gponmac_drv->status!=GPON_DRV_STATUS_ACTIVATE)
    {
        return;
    }

    obj = g_gponmac_drv->dev;

    GPON_OS_Lock(g_gponmac_drv->lock);

    /* disable top mask */
    apollo_raw_gpon_top_mask_read(&mask);
    apollo_raw_gpon_top_mask_write(0);
    GPON_OS_Log(GPON_LOG_LEVEL_INFO,"Top Interrupt mask 0");

    /* read top int status */
    apollo_raw_gpon_top_intr_read(&stat);
    GPON_OS_Log(GPON_LOG_LEVEL_INFO,"Top Interrupt status %04x",stat);

    /* check int status */
    if(apollo_raw_gpon_top_gtcDsIntr_check(stat))
    {
        apollo_raw_gpon_gtcDs_intr_read(&value);
        GPON_OS_Log(GPON_LOG_LEVEL_INFO,"GTC D/S Interrupt status %04x",value);

        if(apollo_raw_gpon_gtcDsIntr_los_check(value))
        {
            apollo_raw_gpon_gtcDs_los_read(&los);
            gpon_alarm_event(obj,RTK_GPON_ALARM_LOS,los);
        }
        if(apollo_raw_gpon_gtcDsIntr_lof_check(value))
        {
            apollo_raw_gpon_gtcDs_lof_read(&lof);
            gpon_alarm_event(obj,RTK_GPON_ALARM_LOF,lof);
        }
        if(apollo_raw_gpon_gtcDsIntr_snReq_check(value))
        {
            gpon_fsm_event(obj,GPON_FSM_EVENT_RX_SN_REQ);
        }
        if(apollo_raw_gpon_gtcDsIntr_rangReq_check(value))
        {
            gpon_fsm_event(obj,GPON_FSM_EVENT_RX_RANGING_REQ);
        }
        if(apollo_raw_gpon_gtcDsIntr_ploam_check(value))
        {
            gpon_ploam_rx(obj);
        }
        if(apollo_raw_gpon_gtcDsIntr_lom_check(value))
        {
            apollo_raw_gpon_gtcDs_lom_read(&lom);
            gpon_alarm_event(obj,RTK_GPON_ALARM_LOM,lom);
        }
        if(apollo_raw_gpon_gtcDsIntr_fec_check(value))
        {
            apollo_raw_gpon_gtcDs_fec_read(&ds_fec);
            gpon_dev_dsFec_report(obj,ds_fec);
        }
    }
#if defined(OLD_FPGA_DEFINED) /* the GEM downstram interrupt is removed. */
    if(apollo_raw_gpon_top_gemDsIntr_check(stat))
    {
        apollo_raw_gpon_gemDs_intr_read(&value);
        GPON_OS_Log(GPON_LOG_LEVEL_INFO,"GEM D/S Interrupt status %04x ",value);

        if(apollo_raw_gpon_gemDsIntr_omci_check(value))
        {
            gpon_omci_rx_reg(obj);
        }
    }
#endif
    if(apollo_raw_gpon_top_gtcUsIntr_check(stat))
    {
        apollo_raw_gpon_gtcUs_intr_read(&value);
        GPON_OS_Log(GPON_LOG_LEVEL_INFO,"GTC U/S Interrupt status %04x",value);

        if(apollo_raw_gpon_gtcUsIntr_fec_check(value))
        {
            apollo_raw_gpon_gtcUs_fec_read(&us_fec);
            gpon_dev_usFec_report(obj,us_fec);
        }
        if(apollo_raw_gpon_gtcUsIntr_urgPloamEmpty_check(value))
        {
            gpon_ploam_usUrgPloamQ_empty_report(obj);
        }
        if(apollo_raw_gpon_gtcUsIntr_nrmPloamEmpty_check(value))
        {
            gpon_ploam_usNrmPloamQ_Empty_report(obj);
        }
    }
    if(apollo_raw_gpon_top_gemUsIntr_check(stat))
    {
        apollo_raw_gpon_gemUs_intr_read(&value);
        GPON_OS_Log(GPON_LOG_LEVEL_INFO,"GEM U/S Interrupt status %04x",value);

        if(apollo_raw_gpon_gemUsIntr_sdLong_check(value))
        {
            GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"U/S SD Long interrupt");
        }
        if(apollo_raw_gpon_gemUsIntr_sdDiff_check(value))
        {
            GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"U/S SD Diff interrupt");
        }
        if(apollo_raw_gpon_gemUsIntr_reqDelay_check(value))
        {
            GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"U/S request delay interrupt");
        }
        if(apollo_raw_gpon_gemUsIntr_bcLess6_check(value))
        {
            GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"U/S byte count less than 6 interrupt");
        }
        if(apollo_raw_gpon_gemUsIntr_errPLI_check(value))
        {
            GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"U/S error PLI interrupt");
        }
        if(apollo_raw_gpon_gemUsIntr_burstLarge_check(value))
        {
            GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"U/S burst large interrupt");
        }
        if(apollo_raw_gpon_gemUsIntr_bankTooMuch_check(value))
        {
            GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"U/S bank too much interrupt");
        }
        if(apollo_raw_gpon_gemUsIntr_bankRemain_check(value))
        {
            GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"U/S bank remain interrupt");
        }
        if(apollo_raw_gpon_gemUsIntr_bankOverflow_check(value))
        {
            GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"U/S bank overflow interrupt");
        }
        if(apollo_raw_gpon_gemUsIntr_bankUnderflow_check(value))
        {
            GPON_OS_Log(GPON_LOG_LEVEL_WARNING,"U/S bank underflow interrupt");
        }
    }

    /* enable top mask */
    apollo_raw_gpon_top_mask_write(mask);
    GPON_OS_Log(GPON_LOG_LEVEL_INFO,"Top Interrupt mask 0x%x",mask);

    GPON_OS_Unlock(g_gponmac_drv->lock);
}



