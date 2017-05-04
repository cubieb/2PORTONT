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
 * $Revision: 39101 $
 * $Date: 2013-05-03 17:35:27 +0800 (Fri, 03 May 2013) $
 *
 * Purpose : Definition those interrupt command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *
 */

/*
 * Include Files
 */
#include <stdio.h>
#include <string.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <diag_util.h>
#include <diag_str.h>
#include <parser/cparser_priv.h>
#include <dal/apollo/raw/apollo_raw_intr.h>
#include <rtk/intr.h>


static void status_str_2_type(char *pStr, rtk_intr_status_t *pType)
{
    if(osal_strcmp(pStr,"speed-change") == 0)
        *pType = INTR_STATUS_SPEED_CHANGE;
    else if(osal_strcmp(pStr,"link-up") == 0)
        *pType = INTR_STATUS_LINKUP;
    else if(osal_strcmp(pStr,"link-down") == 0)
        *pType = INTR_STATUS_LINKDOWN;
    else if(osal_strcmp(pStr,"gphy") == 0)
        *pType = INTR_STATUS_GPHY;
    else
        *pType = INTR_STATUS_END;
}

static void intr_str_2_type(char *pStr, rtk_intr_type_t *pType)
{
    if(osal_strcmp(pStr,"all") == 0)
        *pType = INTR_TYPE_ALL;
    else if(osal_strcmp(pStr,"link-change") == 0)
        *pType = INTR_TYPE_LINK_CHANGE;
    else if(osal_strcmp(pStr,"meter-exceed") == 0)
        *pType = INTR_TYPE_METER_EXCEED;
    else if(osal_strcmp(pStr,"learn-over") == 0)
        *pType = INTR_TYPE_LEARN_OVER;
    else if(osal_strcmp(pStr,"speed-change") == 0)
        *pType = INTR_TYPE_SPEED_CHANGE;
    else if(osal_strcmp(pStr,"special-congestion") == 0)
        *pType = INTR_TYPE_SPECIAL_CONGEST;
    else if(osal_strcmp(pStr,"loop-detection") == 0)
        *pType = INTR_TYPE_LOOP_DETECTION;
    else if(osal_strcmp(pStr,"cable-diag") == 0)
        *pType = INTR_TYPE_CABLE_DIAG_FIN;
    else if(osal_strcmp(pStr,"acl") == 0)
        *pType = INTR_TYPE_ACL_ACTION;
    else if(osal_strcmp(pStr,"gphy") == 0)
        *pType = INTR_TYPE_GPHY;
    else if(osal_strcmp(pStr,"serdes") == 0)
        *pType = INTR_TYPE_SERDES;
    else if(osal_strcmp(pStr,"gpon") == 0)
        *pType = INTR_TYPE_GPON;
    else if(osal_strcmp(pStr,"epon") == 0)
        *pType = INTR_TYPE_EPON;
    else if(osal_strcmp(pStr,"ptp") == 0)
        *pType = INTR_TYPE_PTP;
    else if(osal_strcmp(pStr,"dying-gasp") == 0)
        *pType = INTR_TYPE_DYING_GASP;
    else if(osal_strcmp(pStr,"thermal") == 0)
        *pType = INTR_TYPE_THERMAL;
    else if(osal_strcmp(pStr,"adc") == 0)
        *pType = INTR_TYPE_ADC;
    else
        *pType = INTR_TYPE_END;
}

/*
 * interrupt set imr ( all | link-change | meter-exceed | learn-over | speed-change | special-congestion | loop-detection | cable-diag | acl | gphy | serdes | gpon | epon | ptp | dying-gasp | thermal | adc ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_interrupt_set_imr_all_link_change_meter_exceed_learn_over_speed_change_special_congestion_loop_detection_cable_diag_acl_gphy_serdes_gpon_epon_ptp_dying_gasp_thermal_adc_state_disable_enable(
    cparser_context_t *context)
{
    rtk_intr_type_t type;
    rtk_enable_t enable=DISABLED;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();

    if ('d' == TOKEN_CHAR(5,0))
        enable = DISABLED;
    else if ('e' == TOKEN_CHAR(5,0))
        enable = ENABLED;

    intr_str_2_type(TOKEN_STR(3), &type);

    DIAG_UTIL_ERR_CHK(apollo_raw_intr_imr_set(type, enable), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_interrupt_set_imr_all_link_change_meter_exceed_learn_over_speed_change_special_congestion_loop_detection_cable_diag_acl_gphy_serdes_gpon_epon_ptp_dying_gasp_thermal_adc_state_disable_enable */

/*
 * interrupt get imr ( all | link-change | meter-exceed | learn-over | speed-change | special-congestion | loop-detection | cable-diag | acl | gphy | serdes | gpon | epon | ptp | dying-gasp | thermal | adc )
 */
cparser_result_t
cparser_cmd_interrupt_get_imr_all_link_change_meter_exceed_learn_over_speed_change_special_congestion_loop_detection_cable_diag_acl_gphy_serdes_gpon_epon_ptp_dying_gasp_thermal_adc(
    cparser_context_t *context)
{
    rtk_intr_type_t type;
    uint32 mask;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    intr_str_2_type(TOKEN_STR(3), &type);

    DIAG_UTIL_ERR_CHK(apollo_raw_intr_imr_get(type, &mask), ret);

    if(INTR_TYPE_ALL == type)
        diag_util_printf("IMR: 0x%08x\n\r", mask);
    else
        diag_util_printf("IMR: %s\n\r", diagStr_enable[mask]);

    return CPARSER_OK;
}    /* end of cparser_cmd_interrupt_get_imr_all_link_change_meter_exceed_learn_over_speed_change_special_congestion_loop_detection_cable_diag_acl_gphy_serdes_gpon_epon_ptp_dying_gasp_thermal_adc */

/*
 * interrupt get ims ( all | link-change | meter-exceed | learn-over | speed-change | special-congestion | loop-detection | cable-diag | acl | gphy | serdes | gpon | epon | ptp | dying-gasp | thermal | adc )
 */
cparser_result_t
cparser_cmd_interrupt_get_ims_all_link_change_meter_exceed_learn_over_speed_change_special_congestion_loop_detection_cable_diag_acl_gphy_serdes_gpon_epon_ptp_dying_gasp_thermal_adc(
    cparser_context_t *context)
{
    rtk_intr_type_t type;
    uint32 state;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    intr_str_2_type(TOKEN_STR(3), &type);

    DIAG_UTIL_ERR_CHK(apollo_raw_intr_ims_get(type, &state), ret);

    if(INTR_TYPE_ALL == type)
        diag_util_printf("IMS: 0x%08x\n\r", state);
    else
        diag_util_printf("IMS: %s\n\r", diagStr_enable[state]);

    return CPARSER_OK;
}    /* end of cparser_cmd_interrupt_get_ims_all_link_change_meter_exceed_learn_over_speed_change_special_congestion_loop_detection_cable_diag_acl_gphy_serdes_gpon_epon_ptp_dying_gasp_thermal_adc */

/*
 * interrupt clear ims ( all | link-change | meter-exceed | learn-over | speed-change | special-congestion | loop-detection | cable-diag | acl | gphy | serdes | gpon | epon | ptp | dying-gasp | thermal | adc )
 */
cparser_result_t
cparser_cmd_interrupt_clear_ims_all_link_change_meter_exceed_learn_over_speed_change_special_congestion_loop_detection_cable_diag_acl_gphy_serdes_gpon_epon_ptp_dying_gasp_thermal_adc(
    cparser_context_t *context)
{
    rtk_intr_type_t type;
    rtk_enable_t enable=DISABLED;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();

    intr_str_2_type(TOKEN_STR(3), &type);

    DIAG_UTIL_ERR_CHK(apollo_raw_intr_ims_clear(type), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_interrupt_clear_ims_all_link_change_meter_exceed_learn_over_speed_change_special_congestion_loop_detection_cable_diag_acl_gphy_serdes_gpon_epon_ptp_dying_gasp_thermal_adc */

/*
 * interrupt get status ( speed-chang | link-up | link-down | gphy )
 */
cparser_result_t
cparser_cmd_interrupt_get_status_speed_change_link_up_link_down_gphy(
    cparser_context_t *context)
{
    rtk_intr_status_t type;
    uint32 state;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    status_str_2_type(TOKEN_STR(3), &type);

    if( INTR_STATUS_SPEED_CHANGE == type)
        DIAG_UTIL_ERR_CHK(apollo_raw_intr_sts_speed_change_get(&state), ret);
    else if ( INTR_STATUS_LINKUP == type)
        DIAG_UTIL_ERR_CHK(apollo_raw_intr_sts_linkup_get(&state), ret);
    else if ( INTR_STATUS_LINKDOWN == type)
        DIAG_UTIL_ERR_CHK(apollo_raw_intr_sts_linkdown_get(&state), ret);
    else if ( INTR_STATUS_GPHY == type)
        DIAG_UTIL_ERR_CHK(apollo_raw_intr_sts_gphy_get(&state), ret);

    diag_util_printf("Status: 0x%08x\n\r", state);

    return CPARSER_OK;
}    /* end of cparser_cmd_interrupt_get_status_speed_chang_link_up_link_down_gphy */

/*
 * interrupt clear status ( speed-chang | link-up | link-down | gphy )
 */
cparser_result_t
cparser_cmd_interrupt_clear_status_speed_change_link_up_link_down_gphy(
    cparser_context_t *context)
{
    rtk_intr_status_t type;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();

    status_str_2_type(TOKEN_STR(3), &type);

    if( INTR_STATUS_SPEED_CHANGE == type)
        DIAG_UTIL_ERR_CHK(apollo_raw_intr_sts_speed_change_clear(), ret);
    else if ( INTR_STATUS_LINKUP == type)
        DIAG_UTIL_ERR_CHK(apollo_raw_intr_sts_linkup_clear(), ret);
    else if ( INTR_STATUS_LINKDOWN == type)
        DIAG_UTIL_ERR_CHK(apollo_raw_intr_sts_linkdown_clear(), ret);
    else if ( INTR_STATUS_GPHY == type)
        DIAG_UTIL_ERR_CHK(apollo_raw_intr_sts_gphy_clear(), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_interrupt_clear_status_speed_chang_link_up_link_down_gphy */

/*
 * interrupt set polarity ( high | low )
 */
cparser_result_t
cparser_cmd_interrupt_set_polarity_high_low(
    cparser_context_t *context)
{
    rtk_intr_polarity_t polar;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();

    if ('h' == TOKEN_CHAR(3,0))
        polar = INTR_POLAR_HIGH;
    else if ('l' == TOKEN_CHAR(3,0))
        polar = INTR_POLAR_LOW;

    DIAG_UTIL_ERR_CHK(apollo_raw_intr_polarity_set(polar), ret);
    return CPARSER_OK;
}    /* end of cparser_cmd_interrupt_set_polarity_high_low */

/*
 * interrupt get polarity
 */
cparser_result_t
cparser_cmd_interrupt_get_polarity(
    cparser_context_t *context)
{
    rtk_intr_polarity_t polar;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_intr_polarity_get(&polar), ret);

    diag_util_printf("Polarity: %s\n\r", diagStr_polarity[polar]);

    return CPARSER_OK;
}    /* end of cparser_cmd_interrupt_get_polarity */

