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
 * Purpose : GPON MAC Driver Debug Functions
 *
 * Feature : GPON MAC Driver Debug Functions
 *
 */
 
#include <dal/apollo/gpon/gpon_defs.h>
#include <dal/apollo/gpon/gpon_debug.h>
#include <dal/apollo/raw/apollo_raw_gpon.h>


static int32 gpon_dbg_flag=FALSE;

int32 gpon_dbg_para_laser_get(rtk_gpon_laser_para_t *pPara)
{
    int32 opten, optpolar, cdren, cdrpolar, filter;
    uint8 on, off;
    int32 ret;

    if((ret = apollo_raw_gpon_losCfg_read(&opten,&optpolar,&cdren,&cdrpolar,&filter))!=RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG,MOD_GPON,"apollo_raw_gpon_losCfg_read fail:0x%x\n\r",ret);
        return RT_ERR_FAILED;
    }
    pPara->laser_optic_los_en       = opten;
    pPara->laser_optic_los_polar    = optpolar;
    pPara->laser_cdr_los_en         = cdren;
    pPara->laser_cdr_los_polar      = cdrpolar;
    pPara->laser_los_filter_en      = filter;

    if((ret = apollo_raw_gpon_laserTime_read(&on,&off))!=RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG,MOD_GPON,"apollo_raw_gpon_laserTime_read fail:0x%x\n\r",ret);
        return RT_ERR_FAILED;
    }
    pPara->laser_us_on  = on;
    pPara->laser_us_off = off;

    return RT_ERR_OK;
}

int32 gpon_dbg_para_dsPhy_get(rtk_gpon_ds_physical_para_t *para)
{
    int32 ret;

    if((ret=apollo_raw_gpon_dsScramble_enable_read(&para->ds_scramble_en))!=RT_ERR_OK)
        return ret;
    if((ret=apollo_raw_gpon_dsFec_bypass_read(&para->ds_fec_bypass))!=RT_ERR_OK)
        return ret;
    return apollo_raw_gpon_dsFecThrd_read(&para->ds_fec_thrd);
}

int32 gpon_dbg_para_dsPloam_get(rtk_gpon_ds_ploam_para_t *para)
{
    int32 ret;
#if 0 /* ploam_crc_check is removed in GPON_MAC_SWIO_v1.1 */
    if((ret=apollo_raw_gpon_dsPloam_crcCheck_enable_read(&para->ds_ploam_crc_chk))!=RT_ERR_OK)
        return ret;
#endif
    if((ret=apollo_raw_gpon_dsPloam_onuIdFilter_read(&para->ds_ploam_onuid_filter))!=RT_ERR_OK)
        return ret;
    if((ret=apollo_raw_gpon_dsPloam_bcAccept_read(&para->ds_ploam_broadcast_accpt))!=RT_ERR_OK)
        return ret;
    ret = apollo_raw_gpon_dsPloam_dropCrc_read(&para->ds_ploam_drop_crc_err);

    return ret;
}

int32 gpon_dbg_para_dsBwmap_get(rtk_gpon_ds_bwMap_para_t *para)
{
    int32 ret;

    if((ret=apollo_raw_gpon_dsBwmap_crcCheck_enable_read(&para->ds_bwmap_crc_chk))!=RT_ERR_OK)
        return ret;
    if((ret=apollo_raw_gpon_dsBwmap_filterOnuId_read(&para->ds_bwmap_onuid_filter))!=RT_ERR_OK)
        return ret;

    ret = apollo_raw_gpon_dsBwmap_plend_read(&para->ds_bwmap_plend_mode);

    return ret;
}

int32 gpon_dbg_para_dsGem_get(rtk_gpon_ds_gem_para_t *para)
{
    return apollo_raw_gpon_assembleTimeout_read(&para->assemble_timer);
}

int32 gpon_dbg_para_dsEth_get(rtk_gpon_ds_eth_para_t *para)
{
    int32 ret;

    if((ret = apollo_raw_gpon_dsEth_fcsCheck_enable_read(&para->ds_eth_crc_chk)) != RT_ERR_OK)
        return ret;

    return apollo_raw_gpon_dsEth_pti_read(&para->ds_eth_pti_mask,&para->ds_eth_pti_ptn);
}

int32 gpon_dbg_para_dsOmci_get(rtk_gpon_ds_omci_para_t *para)
{
    return apollo_raw_gpon_dsOmci_pti_read(&para->ds_omci_pti_mask,&para->ds_omci_pti_ptn);
}

int32 gpon_dbg_para_usPhy_get(rtk_gpon_us_physical_para_t *para)
{
    int32 ret;

    if((ret =  apollo_raw_gpon_usScramble_enable_read(&para->us_scramble_en)) != RT_ERR_OK)
        return ret;
    if((ret =  apollo_raw_gpon_usBurstPolarity_read(&para->us_burst_en_polar)) != RT_ERR_OK)
        return ret;
    if((ret =  apollo_raw_gpon_usSmalSstartProc_enable_read(&para->small_sstart_en)) != RT_ERR_OK)
        return ret;
    if((ret =  apollo_raw_gpon_usSuppressLaser_enable_read(&para->suppress_laser_en)) != RT_ERR_OK)
        return ret;
    return ret;
}

int32 gpon_dbg_para_usPloam_get(rtk_gpon_us_ploam_para_t *para)
{
    return apollo_raw_gpon_ploam_enable_read(&para->us_ploam_en);
}

int32 gpon_dbg_para_usDbr_get(rtk_gpon_us_dbr_para_t *para)
{
    return apollo_raw_gpon_dbru_enable_read(&para->us_dbru_en);
}

#if 0 /* OPT power save is removed in GPON_MAC_SWIO_r1.1 */
int32 gpon_dbg_para_optPwrSav_get(rtk_gpon_us_opt_pwr_sav_t *para)
{
    int32 ret;

    if((ret = apollo_raw_gpon_gemUs_txDis_read(&para->us_opt_pwr_sav_en)) != RT_ERR_OK)
        return ret;
    if((ret = apollo_raw_gpon_gemUs_optAheadCycle_read(&para->us_opt_ahead_cycle)) != RT_ERR_OK)
        return ret;
    return apollo_raw_gpon_gemUs_optBehindCycle_read(&para->us_opt_behind_cycle);
}
#endif

int32 gpon_dbg_dsFlow_get(uint32 flowId, rtk_gpon_dsFlow_attr_t *attr)
{
    uint32 gem_port, cfg;
    int32 ret;

    ret = apollo_raw_gpon_dsPort_read(flowId, &gem_port, &cfg);
    if(ret == RT_ERR_OK)
    {
        attr->gem_port_id = gem_port;
        attr->type = (cfg&(1<<1))?RTK_GPON_FLOW_TYPE_ETH:
                     (cfg&(1<<2))?RTK_GPON_FLOW_TYPE_OMCI:
                     (cfg&(1<<3))?RTK_GPON_FLOW_TYPE_TDM:
                     3;
        attr->multicast = (cfg&(1<<0))?1:0;
        attr->aes_en = (cfg&(1<<4))?1:0;
    }
    return ret;
}

int32 gpon_dbg_usFlow_get(uint32 flowId, uint32 *gem_port)
{
    return apollo_raw_gpon_gemUs_portCfg_read(flowId, gem_port);
}

int32 gpon_dbg_multicastAddrCheck_get(uint32 *ipv4_pattern, uint32 *ipv6_pattern)
{
    int32 ret;
    ret = apollo_raw_gpon_ipv4McAddrPtn_read(ipv4_pattern);
    if(ret == RT_ERR_OK)
        ret = apollo_raw_gpon_ipv6McAddrPtn_read(ipv6_pattern);
    return ret;
}

#if 0 /* wellknown addr is removed in GPON_MAC_SWIO_r1.1 */
int32 gpon_dbg_wellKnownAddr_get(int32 *mode, uint32 *addr)
{
    int32 ret;
    ret = apollo_raw_gpon_dsEth_wellKnownAddrFwd_read(mode);
    if(ret != RT_ERR_OK)
        return ret;
    ret = apollo_raw_gpon_wellKnownAddr_read(addr);
    return ret;
}
#endif

int32 gpon_dbg_broadcastPass_get(int32 *mode)
{
    return apollo_raw_gpon_dsEth_bcPAss_read(mode);
}

int32 gpon_dbg_nonMcastPass_get(int32 *mode)
{
    return apollo_raw_gpon_dsEth_nonMcPass_read(mode);
}

int32 gpon_dbg_macFilterMode_get(rtk_gpon_macTable_exclude_mode_t *mode)
{
    return apollo_raw_gpon_dsEth_macFilterMode_read(mode);
}

int32 gpon_dbg_mcForceMode_get(rtk_gpon_mc_force_mode_t *ipv4, rtk_gpon_mc_force_mode_t *ipv6)
{
    return apollo_raw_gpon_dsEth_mcForceMode_read(ipv4,ipv6);
}

int32 gpon_dbg_macEntry_get(uint32 index, rtk_gpon_macTable_entry_t *pEntry)
{
    uint32 value;
    int32 ret;

    RT_PARAM_CHK((index >= GPON_DEV_MAX_MACTBL_NUM), RT_ERR_OUT_OF_RANGE);

    ret = apollo_raw_gpon_macEntry_read(index, &value);
    osal_printf("%s:%d index=%d, value=%x\n\r",__FUNCTION__,__LINE__,index,value);

    pEntry->mac_addr[0]=0x0;
    pEntry->mac_addr[1]=0x0;
    pEntry->mac_addr[2]=0x0;
    pEntry->mac_addr[3]=(value>>16)&0xFF;
    pEntry->mac_addr[4]=(value>>8)&0xFF;
    pEntry->mac_addr[5]=value&0xFF;

    return ret;
}

int32 gpon_dbg_rdi_get(int32 *pEnable)
{
    return apollo_raw_gpon_rdi_read(pEnable);
}
#if 0 /* gemloop is removed in GPON_MAC_SWIO_v1.1 */
int32 gpon_dbg_gemLoop_set(gpon_dev_obj_t* obj, int32 enable)
{
    if(NULL == obj)
        return RT_ERR_NULL_POINTER;

    GPON_OS_Log(GPON_LOG_LEVEL_DEBUG,"Set Loop GEM %d [baseaddr:%p]",enable,obj->base_addr);
    apollo_raw_gpon_gemLoop_write(enable);
    return RT_ERR_OK;
}

int32 gpon_dbg_gemLoop_get(gpon_dev_obj_t* obj, int32 *pEnable)
{
    if(NULL == obj)
        return RT_ERR_NULL_POINTER;

    return apollo_raw_gpon_gemLoop_read(pEnable);
}
#endif
#if 0 /* page GEM FCB is removed */
rtk_gpon_laser_status_t GMac_Dbg_get_gemport_drop_his(gpon_dev_obj_t* obj, rtk_gpon_dropHisStatus_t* history)
{
    if(NULL == obj)
        return RT_ERR_NULL_POINTER;

    return gmac_read_status_drop_his(history);
}
#endif /* page GEM FCB is removed */

int32 gpon_dbg_forceLaser_set(gpon_dev_obj_t* obj, rtk_gpon_laser_status_t enable)
{
    if(NULL == obj)
        return RT_ERR_NULL_POINTER;

    GPON_OS_Log(GPON_LOG_LEVEL_DEBUG,"Set Force Laser %d [baseaddr:%p]",enable,obj->base_addr);
    apollo_raw_gpon_forceLaser_write(enable);
    return RT_ERR_OK;
}

int32 gpon_dbg_forceLaser_get(gpon_dev_obj_t* obj, rtk_gpon_laser_status_t *pEnable)
{
    if(NULL == obj)
        return RT_ERR_NULL_POINTER;
    return apollo_raw_gpon_forceLaser_read(pEnable);
}

int32 gpon_dbg_forceIdle_set(gpon_dev_obj_t* obj, int32 enable)
{
    if(NULL == obj)
        return RT_ERR_NULL_POINTER;
    GPON_OS_Log(GPON_LOG_LEVEL_DEBUG,"Set Force Idle %d [baseaddr:%p]",enable,obj->base_addr);
    apollo_raw_gpon_gemUs_forceIdle_write(enable);
    return RT_ERR_OK;
}

int32 gpon_dbg_forceIdle_get(gpon_dev_obj_t* obj, int32 *pEnable)
{
    if(NULL == obj)
        return RT_ERR_NULL_POINTER;
    return apollo_raw_gpon_gemUs_forceIdle_read(pEnable);
}

#if 0
int32 gpon_dbg_forcePrbs_set(gpon_dev_obj_t* obj, int32 enable)
{
    if(NULL == obj)
        return RT_ERR_NULL_POINTER;

    GPON_OS_Log(GPON_LOG_LEVEL_DEBUG,"Set Force PRBS %d [baseaddr:%p]",enable,obj->base_addr);
    apollo_raw_gpon_forceUsPrbs_write(enable);
    return RT_ERR_OK;
}

int32 gpon_dbg_forcePrbs_get(gpon_dev_obj_t* obj, int32 *pEnable)
{
    if(NULL == obj)
        return RT_ERR_NULL_POINTER;
    return apollo_raw_gpon_forceUsPrbs_read(pEnable);
}
#endif

int32 gpon_dbg_dsFec_get(gpon_dev_obj_t* obj, int32 *pEnable)
{
    if(NULL == obj)
        return RT_ERR_NULL_POINTER;
    return apollo_raw_gpon_gtcDs_fec_read(pEnable);
}

const char* gpon_dbg_drv_status_str(gpon_drv_status_t status)
{
    switch(status)
    {
        case GPON_DRV_STATUS_INIT:
            return "Initial";
        case GPON_DRV_STATUS_READY:
            return "Ready";
        case GPON_DRV_STATUS_ACTIVATE:
            return "Active";
        default:
            return "Error";
    }
}

const char* gpon_dbg_fsm_status_str(rtk_gpon_fsm_status_t status)
{
    switch(status)
    {
        case RTK_GPONMAC_FSM_STATE_UNKNOWN:
            return "Unknown";
        case RTK_GPONMAC_FSM_STATE_O1:
            return "Initial State(O1)";
        case RTK_GPONMAC_FSM_STATE_O2:
            return "Standby State(O2)";
        case RTK_GPONMAC_FSM_STATE_O3:
            return "Serial Number State(O3)";
        case RTK_GPONMAC_FSM_STATE_O4:
            return "Ranging State(O4)";
        case RTK_GPONMAC_FSM_STATE_O5:
            return "Operation State(O5)";
        case RTK_GPONMAC_FSM_STATE_O6:
            return "POPUP State(O6)";
        case RTK_GPONMAC_FSM_STATE_O7:
            return "Emergency Stop State(O7)";
        default:
            return "Error";
    }
}

const char* gpon_dbg_fsm_event_str(rtk_gpon_fsm_event_t event)
{
    switch(event)
    {
        case GPON_FSM_EVENT_LOS_CLEAR:
            return "LOS Clear";
        case GPON_FSM_EVENT_RX_UPSTREAM:
            return "Rcv Upstream";
        case GPON_FSM_EVENT_RX_SN_REQ:
            return "Rcv SN Req";
        case GPON_FSM_EVENT_RX_ONUID:
            return "Assigned ONU ID";
        case GPON_FSM_EVENT_RX_RANGING_REQ:
            return "Rcv Ranging Req";
        case GPON_FSM_EVENT_RX_EQD:
            return "Rcv Ranging Time";
        case GPON_FSM_EVENT_TO1_EXPIRE:
            return "TO1 Expire";
        case GPON_FSM_EVENT_RX_DEACTIVATE:
            return "Deactive";
        case GPON_FSM_EVENT_LOS_DETECT:
            return "LOS Detected";
        case GPON_FSM_EVENT_RX_BC_POPUP:
            return "Broadcast POPUP";
        case GPON_FSM_EVENT_RX_DIRECT_POPUP:
            return "Direct POPUP";
        case GPON_FSM_EVENT_TO2_EXPIRE:
            return "TO2 Expire";
        case GPON_FSM_EVENT_RX_DISABLE:
            return "Disable";
        case GPON_FSM_EVENT_RX_ENABLE:
            return "Enable";
        default:
            return "Error";
    }
}

const char* gpon_dbg_mac_filter_mode_str(rtk_gpon_macTable_exclude_mode_t status)
{
    switch(status)
    {
        case RTK_GPON_MACTBL_MODE_EXCLUDE:
            return "Excluding";
        case RTK_GPON_MACTBL_MODE_INCLUDE:
            return "Including";
        default:
            return "Error";
    }
}

const char* gpon_dbg_mac_force_mode_str(rtk_gpon_mc_force_mode_t status)
{
    switch(status)
    {
        case RTK_GPON_MCFORCE_MODE_NORMAL:
            return "Normal";
        case RTK_GPON_MCFORCE_MODE_PASS:
            return "Force pass";
		case RTK_GPON_MCFORCE_MODE_DROP:
            return "Force drop";
        default:
            return "Error";
    }
}

const char* gpon_dbg_mac_force_laser_str(rtk_gpon_laser_status_t status)
{
    switch(status)
    {
        case RTK_GPON_LASER_STATUS_NORMAL:
            return "Normal";
        case RTK_GPON_LASER_STATUS_FORCE_ON:
            return "Force on";
		case RTK_GPON_LASER_STATUS_FORCE_OFF:
            return "Force off";
        default:
            return "Error";
    }
}

const char* gpon_dbg_alarm_str(rtk_gpon_alarm_type_t status)
{
    switch(status)
    {
        case RTK_GPON_ALARM_NONE:
            return "None";
        case RTK_GPON_ALARM_LOS:
            return "LOS";
        case RTK_GPON_ALARM_LOF:
            return "LOF";
        case RTK_GPON_ALARM_LOM:
            return "LOM";
        case RTK_GPON_ALARM_MAX:
            return "MAX";
        default:
            return "Error";
    }
}

const char* gpon_dbg_flow_type_str(rtk_gpon_flowType_t type)
{
    switch(type)
    {
        case RTK_GPON_FLOW_TYPE_OMCI:
            return "OMCI";
        case RTK_GPON_FLOW_TYPE_ETH:
            return "ETH";
        case RTK_GPON_FLOW_TYPE_TDM:
            return "TDM";
        default:
            return "----";
    }
}

extern gpon_drv_obj_t *g_gponmac_drv;

int32 gpon_dbg_print(const char *format, ...)
{
    /* determine the length of the output string */
    int32 result = RT_ERR_FAILED;
    static uint8 buf[LOG_BUFSIZE_DEFAULT];   /* init value will be given by RT_LOG_FORMATTED_OUTPUT */

    if(1 != gpon_dbg_flag)
        return RT_ERR_OK;

    /* formatted output conversion */
    RT_LOG_FORMATTED_OUTPUT(buf, format, result);


    if (result < 0)
    {
        osal_printf("dbg_print: %d\n\r", result);
        return RT_ERR_FAILED;
    }
#if 0
	if(g_gponmac_drv->dev->status != RTK_GPONMAC_FSM_STATE_O5)
	{
		return RT_ERR_OK;
	}
#endif
    /* start logging */
    osal_printf("%s\n\r", buf);

    return RT_ERR_OK;
}


extern void 
gpon_dbg_enable(int32 enable)
{
    gpon_dbg_flag = enable;
}

