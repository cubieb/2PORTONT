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
 * Purpose : Definition those Trap command and APIs in the SDK diagnostic shell.
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

#include <dal/apollo/raw/apollo_raw_trap.h>


/*
 * trap set rma priority <UINT:priority>
 */
cparser_result_t
cparser_cmd_trap_set_rma_priority_priority(
    cparser_context_t *context,
    uint32_t  *priority_ptr)
{
    int32 ret = CPARSER_NOT_OK;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(apollo_raw_trap_rmaPriority_set(*priority_ptr), ret);
    return CPARSER_OK;
}    /* end of cparser_cmd_trap_set_rma_priority_priority */

/*
 * trap get rma priority
 */
cparser_result_t
cparser_cmd_trap_get_rma_priority(
    cparser_context_t *context)
{
    int32 ret = CPARSER_NOT_OK;
    uint32 priority;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(apollo_raw_trap_rmaPriority_get(&priority), ret);
    diag_util_mprintf("The RMA trap priorit = %u\n", priority);

    return CPARSER_OK;
}    /* end of cparser_cmd_trap_get_rma_priority */

/*
 * trap set rma layer2 <UINT:rma_tail> action ( drop | forward | forward-exclude-cpu | trap-to-cpu )
 */
cparser_result_t
cparser_cmd_trap_set_rma_layer2_rma_tail_action_drop_forward_forward_exclude_cpu_trap_to_cpu(
    cparser_context_t *context,
    uint32_t  *rma_tail_ptr)
{
    int32 ret = CPARSER_NOT_OK;
    rtk_action_t action;
    DIAG_UTIL_PARAM_CHK();
    
    if ('d' == TOKEN_CHAR(6, 0))
    {
        action = ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(6, 0))
    {
        action = ACTION_TRAP2CPU;
    }
    else if ('f' == TOKEN_CHAR(6, 0))
    {
         if ('-' == TOKEN_CHAR(6, 7))
            action = ACTION_FORWARD_EXCLUDE_CPU;
         else
            action = ACTION_FORWARD;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;
    DIAG_UTIL_ERR_CHK(apollo_raw_trap_rmaAction_set(*rma_tail_ptr, action), ret);
    return CPARSER_OK;
}    /* end of cparser_cmd_trap_set_rma_layer2_rma_tail_action_drop_forward_forward_exclude_cpu_trap_to_cpu */

/*
 * trap get rma layer2 <UINT:rma_tail> action
 */
cparser_result_t
cparser_cmd_trap_get_rma_layer2_rma_tail_action(
    cparser_context_t *context,
    uint32_t  *rma_tail_ptr)
{
    rtk_action_t action;
    uint32 ret = CPARSER_NOT_OK;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(apollo_raw_trap_rmaAction_get(*rma_tail_ptr, &action), ret);
    diag_util_mprintf("RMA 01-80-C2-00-00-%s%X action:%s \n",  (*rma_tail_ptr < 0x10)?"0":"", *rma_tail_ptr,
        (action == ACTION_DROP)?"drop":(action == ACTION_TRAP2CPU)?"trap-to-cpu":(action == ACTION_FORWARD)?"forward":"forward-exclude-cpu");
    return CPARSER_OK;
}    /* end of cparser_cmd_trap_get_rma_layer2_rma_tail_action */

/*
 * trap set rma layer2 <UINT:rma_tail> ( vlan-leaky | isolation-leaky | keep-vlan-format | bypass-storm-control )  state ( disable | enable )
 */
cparser_result_t
cparser_cmd_trap_set_rma_layer2_rma_tail_vlan_leaky_isolation_leaky_keep_vlan_format_bypass_storm_control_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *rma_tail_ptr)
{
    int32 ret = CPARSER_NOT_OK;
    rtk_enable_t enable;
    apollo_raw_rmaFeature_t type;
    DIAG_UTIL_PARAM_CHK();
    if ('b' == TOKEN_CHAR(5,0))
    {
        type = RAW_RMA_FUN_BYPASS_STORM;
    }
    else if ('i' == TOKEN_CHAR(5,0))
    {
        type = RAW_RMA_FUN_PISO_LEAKY;
    }
    else if ('k' == TOKEN_CHAR(5,0))
    {
        type = RAW_RMA_FUN_KEEP_CTGA_FMT;
    }
     else if ('v' == TOKEN_CHAR(5,0))
    {
        type = RAW_RMA_FUN_VLAN_LEAKY;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;
    
    if ('d' == TOKEN_CHAR(7,0))
    {
        enable = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(7,0))
    {
        enable = ENABLED;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;
    DIAG_UTIL_ERR_CHK(apollo_raw_trap_rmaFeature_set(*rma_tail_ptr, type, enable), ret);
    return CPARSER_OK;
}    /* end of cparser_cmd_trap_set_rma_layer2_rma_tail_vlan_leaky_isolation_leaky_keep_vlan_format_bypass_storm_control_state_disable_enable */

/*
 * trap get rma layer2 <UINT:rma_tail> ( vlan-leaky | isolation-leaky | keep-vlan-format | bypass-storm-control )
 */
cparser_result_t
cparser_cmd_trap_get_rma_layer2_rma_tail_vlan_leaky_isolation_leaky_keep_vlan_format_bypass_storm_control(
    cparser_context_t *context,
    uint32_t  *rma_tail_ptr)
{
    int32 ret = CPARSER_NOT_OK;
    rtk_enable_t enable;
    apollo_raw_rmaFeature_t type;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    if ('b' == TOKEN_CHAR(5,0))
    {
        type = RAW_RMA_FUN_BYPASS_STORM;
    }
    else if ('i' == TOKEN_CHAR(5,0))
    {
        type = RAW_RMA_FUN_PISO_LEAKY;
    }
    else if ('k' == TOKEN_CHAR(5,0))
    {
        type = RAW_RMA_FUN_KEEP_CTGA_FMT;
    }
     else if ('v' == TOKEN_CHAR(5,0))
    {
        type = RAW_RMA_FUN_VLAN_LEAKY;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_ERR_CHK(apollo_raw_trap_rmaFeature_get(*rma_tail_ptr, type, &enable), ret);
    diag_util_mprintf("RMA 01-80-C2-00-00-%s%X %s : %s \n",  (*rma_tail_ptr < 0x10)?"0":"", *rma_tail_ptr, TOKEN_STR(5), enable?"enable":"disable");
    return CPARSER_OK;
}    /* end of cparser_cmd_trap_get_rma_layer2_rma_tail_vlan_leaky_isolation_leaky_keep_vlan_format_bypass_storm_control */

/*
 * trap set rma ( cdp | csstp ) action ( drop | forward | forward-exclude-cpu | trap-to-cpu )
 */
cparser_result_t
cparser_cmd_trap_set_rma_cdp_csstp_action_drop_forward_forward_exclude_cpu_trap_to_cpu(
    cparser_context_t *context)
{
    int32 ret = CPARSER_NOT_OK;
    rtk_action_t action;
    uint32  rma_tail;
    DIAG_UTIL_PARAM_CHK();
    if ('d' == TOKEN_CHAR(3, 1))
    {
        rma_tail = 0xcc;
    }
    else if ('s' == TOKEN_CHAR(3, 1))
    {
        rma_tail = 0xcd;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;
    
    if ('d' == TOKEN_CHAR(5, 0))
    {
        action = ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(5, 0))
    {
        action = ACTION_TRAP2CPU;
    }
    else if ('f' == TOKEN_CHAR(5, 0))
    {
         if ('-' == TOKEN_CHAR(5, 7))
            action = ACTION_FORWARD_EXCLUDE_CPU;
         else
            action = ACTION_FORWARD;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;
    DIAG_UTIL_ERR_CHK(apollo_raw_trap_rmaCiscoAction_set(rma_tail, action), ret);
    return CPARSER_OK;
}    /* end of cparser_cmd_trap_set_rma_cdp_csstp_action_drop_forward_forward_exclude_cpu_trap_to_cpu */

/*
 * trap get rma ( cdp | csstp ) action
 */
cparser_result_t
cparser_cmd_trap_get_rma_cdp_csstp_action(
    cparser_context_t *context)
{
    int32 ret = CPARSER_NOT_OK;
    rtk_action_t action;
    uint32  rma_tail;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    if ('d' == TOKEN_CHAR(3, 1))
    {
        rma_tail = 0xcc;
    }
    else if ('s' == TOKEN_CHAR(3, 1))
    {
        rma_tail = 0xcd;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;
    DIAG_UTIL_ERR_CHK(apollo_raw_trap_rmaCiscoAction_get(rma_tail, &action), ret);
    diag_util_mprintf("RMA 01-00-0C-CC-CC-%X action:%s \n",  rma_tail,
        (action == ACTION_DROP)?"drop":(action == ACTION_TRAP2CPU)?"trap-to-cpu":(action == ACTION_FORWARD)?"forward":"forward-exclude-cpu");
    
    return CPARSER_OK;
}    /* end of cparser_cmd_trap_get_rma_cdp_csstp_action */

/*
  *trap set rma ( cdp | csstp ) ( vlan-leaky | isolation-leaky | keep-vlan-format | bypass-storm-control )  state ( disable | enable )
  */
cparser_result_t
cparser_cmd_trap_set_rma_cdp_csstp_vlan_leaky_isolation_leaky_keep_vlan_format_bypass_storm_control_state_disable_enable(
    cparser_context_t *context)
{
    int32 ret = CPARSER_NOT_OK;
    rtk_enable_t enable;
    apollo_raw_rmaFeature_t type;
    uint32 rma_tail;
    DIAG_UTIL_PARAM_CHK();
    if ('d' == TOKEN_CHAR(3, 1))
    {
        rma_tail = 0xcc;
    }
    else if ('s' == TOKEN_CHAR(3, 1))
    {
        rma_tail = 0xcd;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;
    
    if ('b' == TOKEN_CHAR(4,0))
    {
        type = RAW_RMA_FUN_BYPASS_STORM;
    }
    else if ('i' == TOKEN_CHAR(4,0))
    {
        type = RAW_RMA_FUN_PISO_LEAKY;
    }
    else if ('k' == TOKEN_CHAR(4,0))
    {
        type = RAW_RMA_FUN_KEEP_CTGA_FMT;
    }
     else if ('v' == TOKEN_CHAR(4,0))
    {
        type = RAW_RMA_FUN_VLAN_LEAKY;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;
    
    if ('d' == TOKEN_CHAR(6,0))
    {
        enable = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(6,0))
    {
        enable = ENABLED;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;
    DIAG_UTIL_ERR_CHK(apollo_raw_trap_rmaCiscoFeature_set(rma_tail, type, enable), ret);
    return CPARSER_OK;

}    /* end of cparser_cmd_trap_set_rma_cdp_csstp_vlan_leaky_isolation_leaky_keep_vlan_format_bypass_storm_control_state_disable_enable */

/*
 * trap get rma ( cdp | csstp ) ( vlan-leaky | isolation-leaky | keep-vlan-format | bypass-storm-control )
 */
cparser_result_t
cparser_cmd_trap_get_rma_cdp_csstp_vlan_leaky_isolation_leaky_keep_vlan_format_bypass_storm_control(
    cparser_context_t *context)
{
    int32 ret = CPARSER_NOT_OK;
    rtk_enable_t enable;
    apollo_raw_rmaFeature_t type;
    uint32 rma_tail;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    if ('d' == TOKEN_CHAR(3, 1))
    {
        rma_tail = 0xcc;
    }
    else if ('s' == TOKEN_CHAR(3, 1))
    {
        rma_tail = 0xcd;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;
    
    if ('b' == TOKEN_CHAR(4,0))
    {
        type = RAW_RMA_FUN_BYPASS_STORM;
    }
    else if ('i' == TOKEN_CHAR(4,0))
    {
        type = RAW_RMA_FUN_PISO_LEAKY;
    }
    else if ('k' == TOKEN_CHAR(4,0))
    {
        type = RAW_RMA_FUN_KEEP_CTGA_FMT;
    }
     else if ('v' == TOKEN_CHAR(4,0))
    {
        type = RAW_RMA_FUN_VLAN_LEAKY;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_ERR_CHK(apollo_raw_trap_rmaCiscoFeature_get(rma_tail, type, &enable), ret);
    diag_util_mprintf("RMA 01-00-0C-CC-CC-%X %s : %s \n",  rma_tail, TOKEN_STR(4), enable?"enable":"disable");
    return CPARSER_OK;

}    /* end of cparser_cmd_trap_get_rma_cdp_csstp_vlan_leaky_isolation_leaky_keep_vlan_format_bypass_storm_control */

/*
 * trap set igmp port ( <PORT_LIST:ports> | all ) ( igmpv1 | igmpv2 | igmpv3 | mldv1 | mldv2 ) ( drop | forward | trap-to-cpu )
 */
cparser_result_t
cparser_cmd_trap_set_igmp_port_ports_all_igmpv1_igmpv2_igmpv3_mldv1_mldv2_drop_forward_trap_to_cpu(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32                       ret;
    diag_portlist_t             portlist;
    rtk_port_t                  port = 0;
    apollo_raw_igmpmld_type_t   type;
    rtk_action_t                action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('i' == TOKEN_CHAR(5,0))
    {
        if ('1' == TOKEN_CHAR(5,5))
            type = RAW_TYPE_IGMPV1;
        else if ('2' == TOKEN_CHAR(5,5))
            type = RAW_TYPE_IGMPV2;
        else if ('3' == TOKEN_CHAR(5,5))
            type = RAW_TYPE_IGMPV3;
        else
            return CPARSER_ERR_INVALID_PARAMS;
    }
    else if('m' == TOKEN_CHAR(5,0))
    {
        if ('1' == TOKEN_CHAR(5,4))
            type = RAW_TYPE_MLDV1;
        else if ('2' == TOKEN_CHAR(5,4))
            type = RAW_TYPE_MLDV2;
        else
            return CPARSER_ERR_INVALID_PARAMS;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    if ('d' == TOKEN_CHAR(6,0))
        action = ACTION_DROP;
    else if ('f' == TOKEN_CHAR(6,0))
        action = ACTION_FORWARD;
    else if ('t' == TOKEN_CHAR(6,0))
        action = ACTION_TRAP2CPU;
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_trap_igmpAction_set(port, type, action), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_trap_set_igmp_port_ports_all_igmpv1_igmpv2_igmpv3_mldv1_mldv2_drop_forward_trap_to_cpu */

/*
 * trap get igmp port ( <PORT_LIST:ports> | all ) ( igmpv1 | igmpv2 | igmpv3 | mldv1 | mldv2 )
 */
cparser_result_t
cparser_cmd_trap_get_igmp_port_ports_all_igmpv1_igmpv2_igmpv3_mldv1_mldv2(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32                       ret;
    diag_portlist_t             portlist;
    rtk_port_t                  port = 0;
    apollo_raw_igmpmld_type_t   type;
    rtk_action_t                action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('i' == TOKEN_CHAR(5,0))
    {
        if ('1' == TOKEN_CHAR(5,5))
            type = RAW_TYPE_IGMPV1;
        else if ('2' == TOKEN_CHAR(5,5))
            type = RAW_TYPE_IGMPV2;
        else if ('3' == TOKEN_CHAR(5,5))
            type = RAW_TYPE_IGMPV3;
        else
            return CPARSER_ERR_INVALID_PARAMS;
    }
    else if('m' == TOKEN_CHAR(5,0))
    {
        if ('1' == TOKEN_CHAR(5,4))
            type = RAW_TYPE_MLDV1;
        else if ('2' == TOKEN_CHAR(5,4))
            type = RAW_TYPE_MLDV2;
        else
            return CPARSER_ERR_INVALID_PARAMS;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_trap_igmpAction_get(port, type, &action), ret);
        diag_util_printf("\n Port %d, %s Action: %s", port, diagStr_igmpTypeStr[type], diagStr_actionStr[action]);
    }

    diag_util_printf("\n");
    return CPARSER_OK;
}    /* end of cparser_cmd_trap_get_igmp_port_ports_all_igmpv1_igmpv2_igmpv3_mldv1_mldv2 */

/*
 * trap set igmp ( vlan-leaky | isolation-leaky ) ( disable | enable )
 */
cparser_result_t
cparser_cmd_trap_set_igmp_vlan_leaky_isolation_leaky_disable_enable(
    cparser_context_t *context)
{
    int32           ret;
    rtk_enable_t    state;

    DIAG_UTIL_PARAM_CHK();

    if ('v' == TOKEN_CHAR(3,0))
    {
        if ('e' == TOKEN_CHAR(4,0))
            state = ENABLED;
        else if ('d' == TOKEN_CHAR(4,0))
            state = DISABLED;
        else
            return CPARSER_ERR_INVALID_PARAMS;

        DIAG_UTIL_ERR_CHK(apollo_raw_trap_igmpVLANLeaky_set(state), ret);
    }
    else if('i' == TOKEN_CHAR(3,0))
    {
        if ('e' == TOKEN_CHAR(4,0))
            state = ENABLED;
        else if ('d' == TOKEN_CHAR(4,0))
            state = DISABLED;
        else
            return CPARSER_ERR_INVALID_PARAMS;

        DIAG_UTIL_ERR_CHK(apollo_raw_trap_igmpIsoLeaky_set(state), ret);
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    return CPARSER_OK;
}    /* end of cparser_cmd_trap_set_igmp_vlan_leaky_isolation_leaky_disable_enable */

/*
 * trap get igmp ( vlan-leaky | isolation-leaky )
 */
cparser_result_t
cparser_cmd_trap_get_igmp_vlan_leaky_isolation_leaky(
    cparser_context_t *context)
{
    int32           ret;
    rtk_enable_t    state;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if ('v' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_trap_igmpVLANLeaky_get(&state), ret);
        diag_util_printf("\n IGMP VLAN Leaky: %s\n", (state == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);
    }
    else if('i' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_trap_igmpIsoLeaky_get(&state), ret);
        diag_util_printf("\n IGMP Port Isolation Leaky: %s\n", (state == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    return CPARSER_OK;
}    /* end of cparser_cmd_trap_get_igmp_vlan_leaky_isolation_leaky */

/*
 * trap set igmp checksum-error ( drop | trap-to-cpu | forward )
 */
cparser_result_t
cparser_cmd_trap_set_igmp_checksum_error_drop_trap_to_cpu_forward(
    cparser_context_t *context)
{
    int32           ret;
    rtk_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if ('d' == TOKEN_CHAR(4,0))
    {
        action = ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(4,0))
    {
        action = ACTION_TRAP2CPU;
    }
    else if ('f' == TOKEN_CHAR(4,0))
    {
        action = ACTION_FORWARD;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_ERR_CHK(apollo_raw_trap_igmpChechsumError_set(action), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_trap_set_igmp_checksum_error_drop_trap_to_cpu_forward */

/*
 * trap get igmp checksum-error
 */
cparser_result_t
cparser_cmd_trap_get_igmp_checksum_error(
    cparser_context_t *context)
{
    int32           ret;
    rtk_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_trap_igmpChechsumError_get(&action), ret);
    diag_util_printf("\n IGMP Checksum Error Action: %s\n", diagStr_actionStr[action]);

    return CPARSER_OK;
}    /* end of cparser_cmd_trap_get_igmp_checksum_error */

