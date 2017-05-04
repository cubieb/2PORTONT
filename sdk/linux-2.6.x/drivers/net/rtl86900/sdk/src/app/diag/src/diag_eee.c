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
 * Purpose : Definition those EEE command and APIs in the SDK diagnostic shell.
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

/*
 * eee get port ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_eee_get_port_port_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("not implement");

    return CPARSER_OK;
}    /* end of cparser_cmd_eee_get_port_port_all */

/*
 * eee set port ( <PORT_LIST:port> | all ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_eee_set_port_port_all_state_disable_enable(
    cparser_context_t *context,
    char * *port_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    diag_util_mprintf("not implement");

    return CPARSER_OK;
}    /* end of cparser_cmd_eee_set_port_port_all_state_disable_enable */

/*
 * eee get lldp state
 */
cparser_result_t
cparser_cmd_eee_get_lldp_state(
    cparser_context_t *context)
{
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    
    if((ret = apollo_raw_eee_lldpEnable_get(&enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("State: %s\n",diagStr_enable[enable]);
    
    return CPARSER_OK;
}    /* end of cparser_cmd_eee_get_lldp_state */

/*
 * eee set lldp state ( disable | enable )
 */
cparser_result_t
cparser_cmd_eee_set_lldp_state_disable_enable(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t enable;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if(TOKEN_CHAR(4,0)=='e')
        enable = ENABLED;
    else
        enable = DISABLED;
        
    if((ret = apollo_raw_eee_lldpEnable_set(enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_eee_set_lldp_state_disable_enable */

/*
 * eee get lldp trap state
 */
cparser_result_t
cparser_cmd_eee_get_lldp_trap_state(
    cparser_context_t *context)
{
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    
    if((ret = apollo_raw_eee_lldpTrapEnable_get(&enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("State: %s\n",diagStr_enable[enable]);
    
    return CPARSER_OK;
}    /* end of cparser_cmd_eee_get_lldp_trap_state */

/*
 * eee set lldp trap state ( disable | enable )
 */
cparser_result_t
cparser_cmd_eee_set_lldp_trap_state_disable_enable(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t enable;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if(TOKEN_CHAR(5,0)=='e')
        enable = ENABLED;
    else
        enable = DISABLED;
        
    if((ret = apollo_raw_eee_lldpTrapEnable_set(enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_eee_set_lldp_trap_state_disable_enable */

/*
 * eee set lldp trap-priority <UINT:priority>
 */
cparser_result_t
cparser_cmd_eee_set_lldp_trap_priority_priority(
    cparser_context_t *context,
    uint32_t  *priority_ptr)
{
    int32 ret;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(apollo_raw_eee_lldpTrapPri_set(*priority_ptr), ret);    

    return CPARSER_OK;
}    /* end of cparser_cmd_eee_set_lldp_trap_priority_priority */

/*
 * eee get lldp trap-priority
 */
cparser_result_t
cparser_cmd_eee_get_lldp_trap_priority(
    cparser_context_t *context)
{
    int32 ret;
    rtk_pri_t priority;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_eee_lldpTrapPri_get(&priority), ret);    

    diag_util_mprintf("trap priority: %d\n", priority);

    return CPARSER_OK;
}    /* end of cparser_cmd_eee_get_lldp_trap_priority */

/*
 * eee set lldp sub-type <UINT:subtype>
 */
cparser_result_t
cparser_cmd_eee_set_lldp_sub_type_subtype(
    cparser_context_t *context,
    uint32_t  *subtype_ptr)
{
    int32 ret;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(apollo_raw_eee_lldpSubtype_set(*subtype_ptr), ret);    

    return CPARSER_OK;
}    /* end of cparser_cmd_eee_set_lldp_sub_type_subtype */

/*
 * eee get lldp sub-type
 */
cparser_result_t
cparser_cmd_eee_get_lldp_sub_type(
    cparser_context_t *context)
{
    int32 ret;
    int32 subtype;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();


    DIAG_UTIL_ERR_CHK(apollo_raw_eee_lldpSubtype_get(&subtype), ret);    

    diag_util_mprintf("subtype: %d\n", subtype);

    return CPARSER_OK;
}    /* end of cparser_cmd_eee_get_lldp_sub_type */

