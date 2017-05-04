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
 * Purpose : Definition those Mirror command and APIs in the SDK diagnostic shell.
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
#include <dal/apollo/raw/apollo_raw_storm.h>
#include <dal/apollo/raw/apollo_raw_meter.h>
#include <dal/apollo/raw/apollo_raw_trap.h>
#include <dal/apollo/raw/apollo_raw_qos.h>

/*
 * bandwidth get egress ifg
 */
cparser_result_t
cparser_cmd_bandwidth_get_egress_ifg(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_ifg_include_t mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(apollo_raw_qos_wfqCalIncIfg_get(&mode), ret);
    diag_util_mprintf("Egress WFQ IFG Calculation is %s\n", mode?"Include":"Exclude");

    return CPARSER_OK;
}    /* end of cparser_cmd_bandwidth_get_egress_ifg */

/*
 * bandwidth get egress ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_bandwidth_get_egress_port_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    apollo_raw_ifg_include_t mode;
    uint32 rate;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    diag_util_mprintf("Egress Bandwidth Control  \n");
    diag_util_mprintf(" Port     Bandwidth\n");
    diag_util_mprintf("--------------------------------------------------------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_qos_bwCtrlEgrRate_get(port, &rate), ret);
        diag_util_printf("%-10u  ", port);
        if (APOLLO_QOS_GRANULARTY_MAX == rate )
	     diag_util_mprintf("               %s\n", "Disable");
        else
	     diag_util_mprintf("               %uKbps\n", rate<<3);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_bandwidth_get_egress_port_all */

/*
 * bandwidth get egress ( <PORT_LIST:port> | all ) queue-id ( <PORT_LIST:qid> | all )
 */
cparser_result_t
cparser_cmd_bandwidth_get_egress_port_all_queue_id_qid_all(
    cparser_context_t *context,
    char * *port_ptr,
    char * *qid_ptr)
{
    diag_portlist_t portlist;
    diag_mask_t  queuelist;
    rtk_port_t port;
    rtk_qid_t queue;
    uint32 idx;
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queuelist, 5), ret);

    diag_util_mprintf("Egress Bandwidth Control APR State/Index  \n");
    diag_util_printf("            ");
    DIAG_UTIL_MASK_SCAN(queuelist, queue)
    {
        diag_util_printf("Q%d      ",queue);
    }
    diag_util_mprintf("\n");
    diag_util_mprintf("--------------------------------------------------------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_printf("Port %2u ", port);
        DIAG_UTIL_MASK_SCAN(queuelist, queue)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_qos_aprEnable_get(port,queue,&enable), ret);
            DIAG_UTIL_ERR_CHK(apollo_raw_qos_aprMeter_get(port,queue,&idx), ret);
                diag_util_printf("   %3s/%d",enable?"En":"Dis", idx);
        }
        diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_bandwidth_get_egress_port_all_queue_id_qid_all */

/*
 * bandwidth get ingress bypass state
 */
cparser_result_t
cparser_cmd_bandwidth_get_ingress_bypass_state(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(apollo_raw_qos_bwCtrlIgrRateBypass_get(&enable), ret);
    diag_util_mprintf("Ingress Bandwidth Control Bypass is %s\n", diagStr_enable[enable]);

    return CPARSER_OK;
}    /* end of cparser_cmd_bandwidth_get_ingress_bypass_state */

/*
 * bandwidth get ingress flow-control high-threshold
 */
cparser_result_t
cparser_cmd_bandwidth_get_ingress_flow_control_high_threshold(
    cparser_context_t *context)
{
    uint32 highThreshold, lowThreshold;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_qos_bwCtrlIgrBound_get(&highThreshold ,&lowThreshold ), ret);
    diag_util_mprintf("Ingress Bandwidth Control High Threshold is %u\n",highThreshold);

    return CPARSER_OK;
}    /* end of cparser_cmd_bandwidth_get_ingress_flow_control_high_threshold */

/*
 * bandwidth get ingress flow-control low-threshold
 */
cparser_result_t
cparser_cmd_bandwidth_get_ingress_flow_control_low_threshold(
    cparser_context_t *context)
{
    uint32 highThreshold, lowThreshold;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_qos_bwCtrlIgrBound_get(&highThreshold ,&lowThreshold ), ret);
    diag_util_mprintf("Ingress Bandwidth Control Low Threshold is %u\n",lowThreshold);

    return CPARSER_OK;
}    /* end of cparser_cmd_bandwidth_get_ingress_flow_control_low_threshold */

/*
 * bandwidth get ingress flow-control ( <PORT_LIST:port> | all ) state
 */
cparser_result_t
cparser_cmd_bandwidth_get_ingress_flow_control_port_all_state(
    cparser_context_t *context,
    char * *port_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Ingress Bandwidth Control  \n");
    diag_util_mprintf(" Port     Flow-Control \n");
    diag_util_mprintf("--------------------------------------------------------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_qos_bwCtrlIgrRateFc_get(port, &enable), ret);
        diag_util_mprintf("%-10u  %s\n", port, diagStr_enable[enable]);
    }


    return CPARSER_OK;
}    /* end of cparser_cmd_bandwidth_get_ingress_flow_control_port_all_state */

/*
 * bandwidth get ingress ifg ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_bandwidth_get_ingress_ifg_port_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    apollo_raw_ifg_include_t mode;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Ingress Bandwidth Control  \n");
    diag_util_mprintf(" Port     IFG-State \n");
    diag_util_mprintf("--------------------------------------------------------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_qos_bwCtrlIgrRateCalIncIfg_get(port, &mode), ret);
        diag_util_mprintf("%-10u  %s\n", port, mode?"Include":"Exclude");
    }


    return CPARSER_OK;
}    /* end of cparser_cmd_bandwidth_get_ingress_ifg_port_all */

/*
 * bandwidth get ingress ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_bandwidth_get_ingress_port_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    rtk_enable_t enable;
    apollo_raw_ifg_include_t mode;
    uint32 rate;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    diag_util_mprintf("Ingress Bandwidth Control  \n");
    diag_util_mprintf(" Port     IFG-State      Flow-Control        Bandwidth\n");
    diag_util_mprintf("--------------------------------------------------------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_qos_bwCtrlIgrRateCalIncIfg_get(port, &mode), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_qos_bwCtrlIgrRate_get(port, &rate), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_qos_bwCtrlIgrRateFc_get(port, &enable), ret);
        diag_util_printf("%-10u  %-14s %7s", port, mode?"Include":"Exclude", diagStr_enable[enable]);
        if (APOLLO_QOS_GRANULARTY_MAX == rate )
	     diag_util_mprintf("             %s\n", "Disable");
        else
	     diag_util_mprintf("             %uKbps\n", rate<<3);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_bandwidth_get_ingress_port_all */

/*
 * bandwidth set egress ifg ( exclude | include )
 */
cparser_result_t
cparser_cmd_bandwidth_set_egress_ifg_exclude_include(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_ifg_include_t mode;

    DIAG_UTIL_PARAM_CHK();

    if ('i' == TOKEN_CHAR(4,0))
    {
        mode = RAW_QOS_IFG_INCLUDE;
    }
    else if ('e' == TOKEN_CHAR(4,0))
    {
        mode = RAW_QOS_IFG_EXCLUDE;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }
    /* set IFG include */
    DIAG_UTIL_ERR_CHK(apollo_raw_qos_wfqCalIncIfg_set(mode), ret);
    diag_util_mprintf("Set Egress WFQ IFG Calculation  %s\n", mode?"Include":"Exclude");

    return CPARSER_OK;
}    /* end of cparser_cmd_bandwidth_set_egress_ifg_exclude_include */

/*
 * bandwidth set egress ( <PORT_LIST:port> | all ) queue-id <UINT:qid> apr-index <UINT:apr>
 */
cparser_result_t
cparser_cmd_bandwidth_set_egress_port_all_queue_id_qid_apr_index_apr(
    cparser_context_t *context,
    char * *port_ptr,
    uint32_t  *qid_ptr,
    uint32_t  *apr_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    uint8  port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_qos_aprMeter_set(port,*qid_ptr,*apr_ptr), ret);
    }
    diag_util_lPortMask2str(port_list, &portlist.portmask);
    diag_util_mprintf("Ports: %s  Queue ID: %u APR Index: %u\n",port_list, *qid_ptr, *apr_ptr);

    return CPARSER_OK;
}    /* end of cparser_cmd_bandwidth_set_egress_port_all_queue_id_qid_apr_index_apr */


/*
 * bandwidth set egress ( <PORT_LIST:port> | all ) queue-id <UINT:qid> state ( disable | enable )
 */
cparser_result_t
cparser_cmd_bandwidth_set_egress_port_all_queue_id_qid_state_disable_enable(
    cparser_context_t *context,
    char * *port_ptr,
    uint32_t  *qid_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    rtk_enable_t enable;
    uint8  port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if('e'==TOKEN_CHAR(7,0))
        enable = ENABLED;
    else if('d'==TOKEN_CHAR(7,0))
        enable = DISABLED;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_qos_aprEnable_set(port,*qid_ptr,enable), ret);
    }
    diag_util_lPortMask2str(port_list, &portlist.portmask);
    diag_util_mprintf("Ports: %s  Queue ID: %u APR State: %s\n",port_list, *qid_ptr,  diagStr_enable[enable]);

    return CPARSER_OK;
}    /* end of cparser_cmd_bandwidth_set_egress_port_all_queue_id_qid_state_disable_enable */


/*
 * bandwidth set egress ( <PORT_LIST:port> | all ) state disable
 */
cparser_result_t
cparser_cmd_bandwidth_set_egress_port_all_state_disable(
    cparser_context_t *context,
    char * *port_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    uint32 tmpRate;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    tmpRate = APOLLO_QOS_GRANULARTY_MAX;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    diag_util_mprintf("Set Egress Bandwidth Status\n");
    diag_util_mprintf("Port       Bandwidth \n");
    diag_util_mprintf("-----------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_qos_bwCtrlEgrRate_set(port, tmpRate), ret);
        diag_util_mprintf("%-10u  %s\n", port, diagStr_enable[DISABLED]);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_bandwidth_set_egress_port_all_state_disable */

/*
 * bandwidth set egress ( <PORT_LIST:port> | all ) state enable rate <UINT:rate>
 */
cparser_result_t
cparser_cmd_bandwidth_set_egress_port_all_state_enable_rate_rate(
    cparser_context_t *context,
    char * *port_ptr,
    uint32_t  *rate_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    uint32 tmpRate;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    tmpRate = (*rate_ptr)>>3;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    diag_util_mprintf("Set Egress Bandwidth\n");
    diag_util_mprintf("Port       Bandwidth \n");
    diag_util_mprintf("-----------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_qos_bwCtrlEgrRate_set(port, tmpRate), ret);
        diag_util_mprintf("%-10u  %u\n", port, tmpRate<<3);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_bandwidth_set_egress_port_all_state_enable_rate_rate */

/*
 * bandwidth set ingress bypass state ( disable | enable )
 */
cparser_result_t
cparser_cmd_bandwidth_set_ingress_bypass_state_disable_enable(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();

    if ('e' == TOKEN_CHAR(5,0))
    {
        enable = ENABLED;
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        enable = DISABLED;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_qos_bwCtrlIgrRateBypass_set(enable), ret);
    diag_util_mprintf("Set Ingress Bandwidth Bypass to  %s\n", diagStr_enable[enable]);

    return CPARSER_OK;
}    /* end of cparser_cmd_bandwidth_set_ingress_bypass_state_disable_enable */

/*
 * bandwidth set ingress flow-control high-threshold <UINT:thresh>
 */
cparser_result_t
cparser_cmd_bandwidth_set_ingress_flow_control_high_threshold_thresh(
    cparser_context_t *context,
    uint32_t  *thresh_ptr)
{
    uint32 highThreshold, lowThreshold;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(apollo_raw_qos_bwCtrlIgrBound_get(&highThreshold ,&lowThreshold ), ret);
    DIAG_UTIL_ERR_CHK(apollo_raw_qos_bwCtrlIgrBound_set(*thresh_ptr, lowThreshold), ret);
    diag_util_mprintf("Set Ingress Bandwidth Control High Threshold to %u\n",*thresh_ptr);

    return CPARSER_OK;
}    /* end of cparser_cmd_bandwidth_set_ingress_flow_control_high_threshold_thresh */

/*
 * bandwidth set ingress flow-control low-threshold <UINT:thresh>
 */
cparser_result_t
cparser_cmd_bandwidth_set_ingress_flow_control_low_threshold_thresh(
    cparser_context_t *context,
    uint32_t  *thresh_ptr)
{
    uint32 highThreshold, lowThreshold;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(apollo_raw_qos_bwCtrlIgrBound_get(&highThreshold ,&lowThreshold ), ret);
    DIAG_UTIL_ERR_CHK(apollo_raw_qos_bwCtrlIgrBound_set(highThreshold, *thresh_ptr), ret);
    diag_util_mprintf("Set Ingress Bandwidth Control LowThreshold to %u\n",*thresh_ptr);

    return CPARSER_OK;
}    /* end of cparser_cmd_bandwidth_set_ingress_flow_control_low_threshold_thresh */

/*
 * bandwidth set ingress flow-control ( <PORT_LIST:port> | all ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_bandwidth_set_ingress_flow_control_port_all_state_disable_enable(
    cparser_context_t *context,
    char * *port_ptr)
{

    diag_portlist_t portlist;
    rtk_port_t port;
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if ('e' == TOKEN_CHAR(6,0))
    {
        enable = ENABLED;
    }
    else if ('d' == TOKEN_CHAR(6,0))
    {
        enable = DISABLED;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    diag_util_mprintf("Set Ingress Bandwidth Control Flow Control Mode\n");
    diag_util_mprintf("Port       Bandwidth \n");
    diag_util_mprintf("-----------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_qos_bwCtrlIgrRateFc_set(port, enable), ret);
        diag_util_mprintf("%-10u  %s\n", port, diagStr_enable[enable]);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_bandwidth_set_ingress_flow_control_port_all_state_disable_enable */

/*
 * bandwidth set ingress ifg ( <PORT_LIST:port> | all ) ( exclude | include )
 */
cparser_result_t
cparser_cmd_bandwidth_set_ingress_ifg_port_all_exclude_include(
    cparser_context_t *context,
    char * *port_ptr)
{
    apollo_raw_ifg_include_t mode;
    diag_portlist_t portlist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if ('i' == TOKEN_CHAR(5,0))
    {
        mode = RAW_QOS_IFG_INCLUDE;
    }
    else if ('e' == TOKEN_CHAR(5,0))
    {
        mode = RAW_QOS_IFG_EXCLUDE;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    diag_util_mprintf("Set Ingress IFG State\n");
    diag_util_mprintf("Port       IFG \n");
    diag_util_mprintf("-----------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_qos_bwCtrlIgrRateCalIncIfg_set(port, mode), ret);
        diag_util_mprintf("%-10u  %s\n", port, mode?"Include":"Exclude");
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_bandwidth_set_ingress_ifg_port_all_exclude_include */

/*
 * bandwidth set ingress ( <PORT_LIST:port> | all ) state disable
 */
cparser_result_t
cparser_cmd_bandwidth_set_ingress_port_all_state_disable(
    cparser_context_t *context,
    char * *port_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    uint32 tmpRate;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    tmpRate = APOLLO_QOS_GRANULARTY_MAX;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    diag_util_mprintf("Set Ingress Bandwidth Status\n");
    diag_util_mprintf("Port       Bandwidth \n");
    diag_util_mprintf("-----------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_qos_bwCtrlIgrRate_set(port, tmpRate), ret);
        diag_util_mprintf("%-10u  %s\n", port, diagStr_enable[DISABLED]);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_bandwidth_set_ingress_port_all_state_disable */

/*
 * bandwidth set ingress ( <PORT_LIST:port> | all ) state enable rate <UINT:rate>
 */
cparser_result_t
cparser_cmd_bandwidth_set_ingress_port_all_state_enable_rate_rate(
    cparser_context_t *context,
    char * *port_ptr,
    uint32_t  *rate_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    uint32 tmpRate;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    tmpRate = (*rate_ptr)>>3;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    diag_util_mprintf("Set Ingress Bandwidth\n");
    diag_util_mprintf("Port       Bandwidth \n");
    diag_util_mprintf("-----------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_qos_bwCtrlIgrRate_set(port, tmpRate), ret);
        diag_util_mprintf("%-10u  %u\n", port, tmpRate<<3);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_bandwidth_set_ingress_port_all_state_enable_rate_rate */

/*
 * meter get entry <MASK_LIST:index>
 */
cparser_result_t
cparser_cmd_meter_get_entry_index(
    cparser_context_t *context,
    char * *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    uint32 meterRate;
    rtk_enable_t ifg;
    uint32 lbthreshold;
    uint32 exceed;
    uint32 index;
    diag_mask_t mask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK32(mask, 3), ret);
    DIAG_UTIL_MASK_SCAN(mask, index)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_meter_rate_get(index, &meterRate), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_meter_ifg_get(index, &ifg), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_meter_bucketSize_get(index, &lbthreshold), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_meter_exceed_get(index, &exceed), ret);
        diag_util_mprintf("Meter idx = %u, meter rate = %u Kbps, IFG:%s,\nburst size = %d, Exceed = %s\n", index, meterRate*8, ifg?"Include":"Exclude", lbthreshold, exceed?"Exceed":"Not exceed");
    }

    diag_util_mprintf("");

    return CPARSER_OK;
}    /* end of cparser_cmd_meter_get_entry_index */

/*
 * meter get entry <MASK_LIST:index> burst-size
 */
cparser_result_t
cparser_cmd_meter_get_entry_index_burst_size(
    cparser_context_t *context,
    char * *index_ptr)
{
    uint32 lbthreshold;
    int32 ret = RT_ERR_FAILED;
    uint32 index;
    diag_mask_t mask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK32(mask, 3), ret);
    DIAG_UTIL_MASK_SCAN(mask, index)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_meter_bucketSize_get(index, &lbthreshold), ret);
        diag_util_mprintf("Meter idx = %u, burst size = %u\n", index, lbthreshold);
    }
    return CPARSER_OK;

}    /* end of cparser_cmd_meter_get_entry_index_burst_size */

/*
 * meter get entry <MASK_LIST:index> ifg
 */
cparser_result_t
cparser_cmd_meter_get_entry_index_ifg(
    cparser_context_t *context,
    char * *index_ptr)
{
    rtk_enable_t ifg;
    int32 ret = RT_ERR_FAILED;
    uint32 index;
    diag_mask_t mask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK32(mask, 3), ret);
    DIAG_UTIL_MASK_SCAN(mask, index)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_meter_ifg_get(index, &ifg), ret);
        diag_util_mprintf("Meter idx = %u, IFG = %s\n", index, ifg?"Include":"Exclude");
    }
    return CPARSER_OK;

}    /* end of cparser_cmd_meter_get_entry_index_ifg */

/*
 * meter get entry <MASK_LIST:index> meter-exceed
 */
cparser_result_t
cparser_cmd_meter_get_entry_index_meter_exceed(
    cparser_context_t *context,
    char * *index_ptr)
{
    uint32 exceed;
    int32 ret = RT_ERR_FAILED;
    uint32 index;
    diag_mask_t mask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK32(mask, 3), ret);
    DIAG_UTIL_MASK_SCAN(mask, index)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_meter_exceed_get(index, &exceed), ret);
        diag_util_mprintf("Meter idx = %u, Exceed = %s\n", index, exceed?"Exceed":"Not exceed");
    }
    return CPARSER_OK;

}    /* end of cparser_cmd_meter_get_entry_index_meter_exceed */

/*
 * meter get entry <MASK_LIST:index> rate
 */
cparser_result_t
cparser_cmd_meter_get_entry_index_rate(
    cparser_context_t *context,
    char * *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    uint32 meterRate;
    rtk_enable_t ifg;
    uint32 index;
    diag_mask_t mask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK32(mask, 3), ret);
    DIAG_UTIL_MASK_SCAN(mask, index)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_meter_rate_get(index, &meterRate), ret);
        diag_util_mprintf("Meter idx = %u, meter rate = %u Kbps\n", index, meterRate*8);
    }
    return CPARSER_OK;

}    /* end of cparser_cmd_meter_get_entry_index_rate */

/*
 * meter get bt_ctr
 */
cparser_result_t
cparser_cmd_meter_get_bt_ctr(
    cparser_context_t *context)
{
    int32 ret = CPARSER_NOT_OK;
    uint32 tickPeriod;
    uint32 tkn;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(apollo_raw_meter_btCtr_get(&tickPeriod, &tkn), ret);
    diag_util_mprintf("Tick period = %u, TKN = %u\n", tickPeriod, tkn);
    return CPARSER_OK;
}    /* end of cparser_cmd_meter_get_bt_ctr */

/*
 * meter get pon_bt_ctr
 */
cparser_result_t
cparser_cmd_meter_get_pon_bt_ctr(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
    int32 ret = CPARSER_NOT_OK;
    uint32 tickPeriod;
    uint32 tkn;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(apollo_raw_meter_ponBtCtr_get(&tickPeriod, &tkn), ret);
    diag_util_mprintf("Tick period = %u, TKN = %u\n", tickPeriod, tkn);
    return CPARSER_OK;
}    /* end of cparser_cmd_meter_get_pon_bt_ctr */

/*
 * meter reset entry <MASK_LIST:index> meter-exceed
 */
cparser_result_t
cparser_cmd_meter_reset_entry_index_meter_exceed(
    cparser_context_t *context,
    char * *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    uint32 index;
    diag_mask_t mask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK32(mask, 3), ret);
    DIAG_UTIL_MASK_SCAN(mask, index)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_meter_exceed_set(index), ret);
    }
    return CPARSER_OK;

}    /* end of cparser_cmd_meter_reset_entry_index_meter_exceed */

/*
 * meter set entry <MASK_LIST:index> burst-size <UINT:lb_size>
 */
cparser_result_t
cparser_cmd_meter_set_entry_index_burst_size_lb_size(
    cparser_context_t *context,
    char * *index_ptr,
    uint32_t  *lb_size_ptr)
{
    int32           ret = RT_ERR_FAILED;
    uint32 index;
    diag_mask_t mask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK32(mask, 3), ret);
    DIAG_UTIL_MASK_SCAN(mask, index)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_meter_bucketSize_set(index, *lb_size_ptr), ret);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_meter_set_entry_index_burst_size_lb_size */

/*
 * meter set entry <MASK_LIST:index> ifg ( exclude | include )
 */
cparser_result_t
cparser_cmd_meter_set_entry_index_ifg_exclude_include(
    cparser_context_t *context,
    char * *index_ptr)
{
    int32   ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;
    uint32 index;
    diag_mask_t mask;

    DIAG_UTIL_PARAM_CHK();

    if ('i' == TOKEN_CHAR(5,0))
    {
        enable = ENABLED;
    }
    else if ('e' == TOKEN_CHAR(5,0))
    {
        enable = DISABLED;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    /* set IFG include */
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK32(mask, 3), ret);
    DIAG_UTIL_MASK_SCAN(mask, index)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_meter_ifg_set(index, enable), ret);
    }
    return CPARSER_OK;

}    /* end of cparser_cmd_meter_set_entry_index_ifg_exclude_include */

/*
 * meter set entry <MASK_LIST:index> rate <UINT:rate>
 */
cparser_result_t
cparser_cmd_meter_set_entry_index_rate_rate(
    cparser_context_t *context,
    char * *index_ptr,
    uint32_t  *rate_ptr)
{
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    uint32          rateTemp;
    uint32 index;
    diag_mask_t mask;

    DIAG_UTIL_PARAM_CHK();
    rateTemp = *rate_ptr;
    if(*rate_ptr > 1048568 || *rate_ptr%8 != 0 )
    {
        diag_util_printf("The rate range would be in 8~1048568, and must be exactiy divisible by 8!\n");
        return CPARSER_ERR_INVALID_PARAMS;
    }
    rateTemp = (*rate_ptr) >> 3;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK32(mask, 3), ret);
    DIAG_UTIL_MASK_SCAN(mask, index)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_meter_rate_set(index, rateTemp), ret);
    }
    return CPARSER_OK;

}    /* end of cparser_cmd_meter_set_entry_index_rate_rate */


/*
 * meter set bt_ctr tick_period <UINT:period> tkn <UINT:tkn>
 */
cparser_result_t
cparser_cmd_meter_set_bt_ctr_tick_period_period_tkn_tkn(
    cparser_context_t *context,
    uint32_t  *period_ptr,
    uint32_t  *tkn_ptr)
{
    int32 ret = CPARSER_NOT_OK;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(apollo_raw_meter_btCtr_set(*period_ptr, *tkn_ptr), ret);
    return CPARSER_OK;
}    /* end of cparser_cmd_meter_set_bt_ctr_tick_period_period_tkn_tkn */

/*
 * meter set pon_bt_ctr tick_period <UINT:period> tkn <UINT:tkn>
 */
cparser_result_t
cparser_cmd_meter_set_pon_bt_ctr_tick_period_period_tkn_tkn(
    cparser_context_t *context,
    uint32_t  *period_ptr,
    uint32_t  *tkn_ptr)
{
    int32 ret = CPARSER_NOT_OK;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(apollo_raw_meter_ponBtCtr_set(*period_ptr, *tkn_ptr), ret);
    return CPARSER_OK;
}    /* end of cparser_cmd_meter_set_pon_bt_ctr_tick_period_period_tkn_tkn */
/*
 * storm-control get ( broadcast | multicast | unknown-multicast | unknown-unicast ) port ( <PORT_LIST:ports> | all )
 */
cparser_result_t
cparser_cmd_storm_control_get_broadcast_multicast_unknown_multicast_unknown_unicast_port_ports_all(
    cparser_context_t *context)
{
    int32 ret;
    diag_portlist_t portlist;
    rtk_port_t port;
    raw_storm_type_t type;
    rtk_enable_t state;
    uint32 meter_id;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('b' == TOKEN_CHAR(2, 0))
        type = RAW_STORM_BC;
    else if ('m' == TOKEN_CHAR(2, 0))
        type = RAW_STORM_MC;
    else if ('u' == TOKEN_CHAR(2, 0))
    {
        if ('m' == TOKEN_CHAR(2, 8))
            type = RAW_STORM_UNKN_MC;
        else if ('u' == TOKEN_CHAR(2, 8))
            type = RAW_STORM_UNKN_UC;
        else
            return CPARSER_ERR_INVALID_PARAMS;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_stormControlState_get(type, port, &state), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_stormControlMeter_get(type, port, &meter_id), ret);

        if(ENABLED == state)
            diag_util_printf("\n Port %d state: %s, Meter index = %d", port, diagStr_enable[state], meter_id);
        else
            diag_util_printf("\n Port %d state: %s", port, diagStr_enable[state]);
    }

    diag_util_printf("\n\n");
    return CPARSER_OK;

}    /* end of cparser_cmd_storm_control_get_broadcast_multicast_unknown_multicast_unknown_unicast_port_ports_all */

/*
 * storm-control get ( broadcast | multicast | unknown-multicast | unknown-unicast ) alternated
 */
cparser_result_t
cparser_cmd_storm_control_get_broadcast_multicast_unknown_multicast_unknown_unicast_alternated(
    cparser_context_t *context)
{
    int32 ret;
    raw_storm_type_t type;
    raw_storm_alt_type_t alt_type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if ('b' == TOKEN_CHAR(2, 0))
        type = RAW_STORM_BC;
    else if ('m' == TOKEN_CHAR(2, 0))
        type = RAW_STORM_MC;
    else if ('u' == TOKEN_CHAR(2, 0))
    {
        if ('m' == TOKEN_CHAR(2, 8))
            type = RAW_STORM_UNKN_MC;
        else if ('u' == TOKEN_CHAR(2, 8))
            type = RAW_STORM_UNKN_UC;
        else
            return CPARSER_ERR_INVALID_PARAMS;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_ERR_CHK(apollo_raw_stormControlAlt_get(type, &alt_type), ret);
    diag_util_printf("\n %s Storm control", diagStr_stormType[type]);
    diag_util_printf("\n Alternative Type: %s\n\n", diagStr_stormAltType[alt_type]);

    return CPARSER_OK;
}    /* end of cparser_cmd_storm_control_get_broadcast_multicast_unknown_multicast_unknown_unicast_alternated */


/*
 * storm-control get igmp bypass state
 */
cparser_result_t
cparser_cmd_storm_control_get_igmp_bypass_state(
    cparser_context_t *context)
{
    int32 ret;
    rtk_enable_t state;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_trap_igmpBypassStrom_get(&state), ret);

    diag_util_printf("\n IGMP bypass Storm-Control: %s", diagStr_enable[state]);
    diag_util_printf("\n\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_storm_control_get_igmp_bypass_state */

/*
 * storm-control get rma ( cdp | csstp ) bypass state
 */
cparser_result_t
cparser_cmd_storm_control_get_rma_cdp_csstp_bypass_state(
    cparser_context_t *context)
{
    int32 ret;
    uint32 rmaTail;
    rtk_enable_t state;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if ('d' == TOKEN_CHAR(3, 1))
    {
        rmaTail = 0xCC;
    }
    else if ('s' == TOKEN_CHAR(3, 1))
    {
        rmaTail = 0xCD;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;


    DIAG_UTIL_ERR_CHK(apollo_raw_trap_rmaCiscoFeature_get(rmaTail, RAW_RMA_FUN_BYPASS_STORM, &state), ret);
    diag_util_printf("\n %s Bypass Storm-Control state: %s", (rmaTail == 0xCD) ? "CSSTP" : "CDP", diagStr_enable[state]);
    diag_util_printf("\n\n");
    return CPARSER_OK;
}    /* end of cparser_cmd_storm_control_get_rma_cdp_csstp_bypass_state */

/*
 * storm-control get rma layer2 <UINT:rma_tail> bypass state
 */
cparser_result_t
cparser_cmd_storm_control_get_rma_layer2_rma_tail_bypass_state(
    cparser_context_t *context,
    uint32_t  *rma_tail_ptr)
{
    int32 ret;
    rtk_enable_t state;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();


    DIAG_UTIL_ERR_CHK(apollo_raw_trap_rmaFeature_get(*rma_tail_ptr, RAW_RMA_FUN_BYPASS_STORM, &state), ret);
    diag_util_printf("\n RMA 01-80-C2-00-00-%02X bypass Storm-Control state: %s", *rma_tail_ptr, diagStr_enable[state]);
    diag_util_printf("\n\n");

    return CPARSER_OK;

}    /* end of cparser_cmd_storm_control_get_rma_layer2_rma_tail_bypass_state */

/*
 * storm-control set ( broadcast | multicast | unknown-multicast | unknown-unicast ) port ( <PORT_LIST:ports> | all ) state disable
 */
cparser_result_t
cparser_cmd_storm_control_set_broadcast_multicast_unknown_multicast_unknown_unicast_port_ports_all_state_disable(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret;
    diag_portlist_t portlist;
    rtk_port_t port;
    raw_storm_type_t type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('b' == TOKEN_CHAR(2, 0))
        type = RAW_STORM_BC;
    else if ('m' == TOKEN_CHAR(2, 0))
        type = RAW_STORM_MC;
    else if ('u' == TOKEN_CHAR(2, 0))
    {
        if ('m' == TOKEN_CHAR(2, 8))
            type = RAW_STORM_UNKN_MC;
        else if ('u' == TOKEN_CHAR(2, 8))
            type = RAW_STORM_UNKN_UC;
        else
            return CPARSER_ERR_INVALID_PARAMS;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_stormControlState_set(type, port, DISABLED), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_storm_control_set_broadcast_multicast_unknown_multicast_unknown_unicast_port_ports_all_state_disable */

/*
 * storm-control set ( broadcast | multicast | unknown-multicast | unknown-unicast ) port ( <PORT_LIST:ports> | all ) state enable meter <UINT:index>
 */
cparser_result_t
cparser_cmd_storm_control_set_broadcast_multicast_unknown_multicast_unknown_unicast_port_ports_all_state_enable_meter_index(
    cparser_context_t *context,
    char * *port_ptr,
    uint32_t  *index_ptr)
{
    int32 ret;
    diag_portlist_t portlist;
    rtk_port_t port;
    raw_storm_type_t type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('b' == TOKEN_CHAR(2, 0))
        type = RAW_STORM_BC;
    else if ('m' == TOKEN_CHAR(2, 0))
        type = RAW_STORM_MC;
    else if ('u' == TOKEN_CHAR(2, 0))
    {
        if ('m' == TOKEN_CHAR(2, 8))
            type = RAW_STORM_UNKN_MC;
        else if ('u' == TOKEN_CHAR(2, 8))
            type = RAW_STORM_UNKN_UC;
        else
            return CPARSER_ERR_INVALID_PARAMS;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_stormControlState_set(type, port, ENABLED), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_stormControlMeter_set(type, port, (uint32)*index_ptr), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_storm_control_set_broadcast_multicast_unknown_multicast_unknown_unicast_port_ports_all_state_enable_meter_index */

/*
 * storm-control set ( broadcast | multicast | unknown-multicast | unknown-unicast ) alternated disable
 */
cparser_result_t
cparser_cmd_storm_control_set_broadcast_multicast_unknown_multicast_unknown_unicast_alternated_disable(
    cparser_context_t *context)
{
    int32 ret;
    raw_storm_type_t type;

    DIAG_UTIL_PARAM_CHK();

    if ('b' == TOKEN_CHAR(2, 0))
        type = RAW_STORM_BC;
    else if ('m' == TOKEN_CHAR(2, 0))
        type = RAW_STORM_MC;
    else if ('u' == TOKEN_CHAR(2, 0))
    {
        if ('m' == TOKEN_CHAR(2, 8))
            type = RAW_STORM_UNKN_MC;
        else if ('u' == TOKEN_CHAR(2, 8))
            type = RAW_STORM_UNKN_UC;
        else
            return CPARSER_ERR_INVALID_PARAMS;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_ERR_CHK(apollo_raw_stormControlAlt_set(type, RAW_STORM_ALT_DEFAULT), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_storm_control_set_broadcast_multicast_unknown_multicast_unknown_unicast_alternated_disable */

/*
 * storm-control set ( broadcast | multicast | unknown-multicast | unknown-unicast ) alternated enable ( arp-storm | dhcp-storm | igmp-mld-storm )
 */
cparser_result_t
cparser_cmd_storm_control_set_broadcast_multicast_unknown_multicast_unknown_unicast_alternated_enable_arp_storm_dhcp_storm_igmp_mld_storm(
    cparser_context_t *context)
{
    int32 ret;
    raw_storm_type_t type;
    raw_storm_alt_type_t alt_type;

    DIAG_UTIL_PARAM_CHK();

    if ('b' == TOKEN_CHAR(2, 0))
        type = RAW_STORM_BC;
    else if ('m' == TOKEN_CHAR(2, 0))
        type = RAW_STORM_MC;
    else if ('u' == TOKEN_CHAR(2, 0))
    {
        if ('m' == TOKEN_CHAR(2, 8))
            type = RAW_STORM_UNKN_MC;
        else if ('u' == TOKEN_CHAR(2, 8))
            type = RAW_STORM_UNKN_UC;
        else
            return CPARSER_ERR_INVALID_PARAMS;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    if ('a' == TOKEN_CHAR(5, 0))
        alt_type = RAW_STORM_ALT_ARP;
    else if ('d' == TOKEN_CHAR(5, 0))
        alt_type = RAW_STORM_ALT_DHCP;
    else if ('i' == TOKEN_CHAR(5, 0))
        alt_type = RAW_STORM_ALT_IGMP_MLD;
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_ERR_CHK(apollo_raw_stormControlAlt_set(type, alt_type), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_storm_control_set_broadcast_multicast_unknown_multicast_unknown_unicast_alternated_enable_arp_storm_dhcp_storm_igmp_mld_storm */

/*
 * storm-control set igmp bypass state ( disable | enable )
 */
cparser_result_t
cparser_cmd_storm_control_set_igmp_bypass_state_disable_enable(
    cparser_context_t *context)
{
    int32 ret;
    rtk_enable_t state;

    DIAG_UTIL_PARAM_CHK();

    if ('d' == TOKEN_CHAR(5, 0))
        state = DISABLED;
    else if ('e' == TOKEN_CHAR(5, 0))
        state = ENABLED;
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_ERR_CHK(apollo_raw_trap_igmpBypassStrom_set(state), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_storm_control_set_igmp_bypass_state_disable_enable */

/*
 * storm-control set rma ( cdp | csstp ) bypass state ( disable | enable )
 */
cparser_result_t
cparser_cmd_storm_control_set_rma_cdp_csstp_bypass_state_disable_enable(
    cparser_context_t *context)
{
    int32 ret;
    uint32 rmaTail;
    rtk_enable_t state;

    DIAG_UTIL_PARAM_CHK();

    if ('d' == TOKEN_CHAR(3, 1))
    {
        rmaTail = 0xCC;
    }
    else if ('s' == TOKEN_CHAR(3, 1))
    {
        rmaTail = 0xCD;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    if ('d' == TOKEN_CHAR(6, 0))
        state = DISABLED;
    else if ('e' == TOKEN_CHAR(6, 0))
        state = ENABLED;
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_ERR_CHK(apollo_raw_trap_rmaCiscoFeature_set(rmaTail, RAW_RMA_FUN_BYPASS_STORM, state), ret);
    return CPARSER_OK;
}    /* end of cparser_cmd_storm_control_set_rma_cdp_csstp_bypass_state_disable_enable */

/*
 * storm-control set rma layer2 <UINT:rma_tail> bypass state ( disable | enable )
 */
cparser_result_t
cparser_cmd_storm_control_set_rma_layer2_rma_tail_bypass_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *rma_tail_ptr)
{
    int32 ret;
    rtk_enable_t state;

    DIAG_UTIL_PARAM_CHK();

    if ('d' == TOKEN_CHAR(7, 0))
        state = DISABLED;
    else if ('e' == TOKEN_CHAR(7, 0))
        state = ENABLED;
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_ERR_CHK(apollo_raw_trap_rmaFeature_set(*rma_tail_ptr, RAW_RMA_FUN_BYPASS_STORM, state), ret);
    return CPARSER_OK;
}    /* end of cparser_cmd_storm_control_set_rma_layer2_rma_tail_bypass_state_disable_enable */

