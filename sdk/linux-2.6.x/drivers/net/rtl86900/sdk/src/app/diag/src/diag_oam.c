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
 * Purpose : Definition those OAM command and APIs in the SDK diagnostic shell.
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
#include <dal/apollo/raw/apollo_raw_oam.h>

/*
 * oam dump
 */
cparser_result_t
cparser_cmd_oam_dump(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
    diag_util_mprintf("not implement");
    return CPARSER_OK;
}    /* end of cparser_cmd_oam_dump */

/*
 * oam get multiplexer port ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_oam_get_multiplexer_port_port_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    apollo_raw_oam_muxAct_t action;
    diag_portlist_t portlist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;
        
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
	
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    diag_util_mprintf("Get OAM Multiplexer\n"); 
    diag_util_mprintf("Port       Multiplexer Action \n"); 	
    diag_util_mprintf("-----------------------------\n"); 	
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {    
        DIAG_UTIL_ERR_CHK(apollo_raw_oam_multiplexer_get(port, &action), ret); 			
        diag_util_mprintf("%-10u  %s\n", port, diagStr_oamMuxAct[action]);   
    }	

    return CPARSER_OK;
}    /* end of cparser_cmd_oam_get_multiplexer_port_port_all */

/*
 * oam get parser port ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_oam_get_parser_port_port_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    apollo_raw_oam_parserAct_t action;
    diag_portlist_t portlist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;
        
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
	
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    diag_util_mprintf("Get OAM Parser\n"); 
    diag_util_mprintf("Port       Parser Action \n"); 	
    diag_util_mprintf("-----------------------------\n"); 	
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {    
        DIAG_UTIL_ERR_CHK(apollo_raw_oam_parser_get(port, &action), ret); 			
        diag_util_mprintf("%-10u  %s\n", port, diagStr_oamParserAct[action]);   
    }	

    return CPARSER_OK;
}    /* end of cparser_cmd_oam_get_parser_port_port_all */

/*
 * oam get state
 */
cparser_result_t
cparser_cmd_oam_get_state(
    cparser_context_t *context)
{
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;
        
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
	
    /*call API*/
    DIAG_UTIL_ERR_CHK(apollo_raw_oam_enable_get(&enable), ret);
	
    diag_util_mprintf("OAM State is %s\n",diagStr_enable[enable]);

    return CPARSER_OK;
}    /* end of cparser_cmd_oam_get_state */

/*
 * oam set multiplexer port ( <PORT_LIST:port> | all ) action ( forward | discard | from-cpu-only )
 */
cparser_result_t
cparser_cmd_oam_set_multiplexer_port_port_all_action_forward_discard_from_cpu_only(
    cparser_context_t *context,
    char * *port_ptr)
{
    apollo_raw_oam_muxAct_t action;
    diag_portlist_t portlist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;
        
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if ('f' == TOKEN_CHAR(6,0))
    {
        if ('o' == TOKEN_CHAR(6,1))
            action = RAW_OAM_MUX_FORWARD;
        else if ('r' == TOKEN_CHAR(6,1))
            action = RAW_OAM_MUX_CPUONLY;	
        else
        {
            diag_util_printf("User config : Error!\n");
            return CPARSER_NOT_OK;
        }			
    }
    else if ('d' == TOKEN_CHAR(6,0))
    {
        action = RAW_OAM_MUX_DISCARD;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }	
	
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    diag_util_mprintf("Set OAM Multiplexer\n"); 
    diag_util_mprintf("Port       Multiplexer Action \n"); 	
    diag_util_mprintf("-----------------------------\n"); 	
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {    
        DIAG_UTIL_ERR_CHK(apollo_raw_oam_multiplexer_set(port, action), ret); 			
        diag_util_mprintf("%-10u  %s\n", port, diagStr_oamMuxAct[action]);   
    }	

    return CPARSER_OK;
}    /* end of cparser_cmd_oam_set_multiplexer_port_port_all_action_forward_discard_from_cpu_only */

/*
 * oam set parser port ( <PORT_LIST:port> | all ) action ( forward | loopback | discard  )
 */
cparser_result_t
cparser_cmd_oam_set_parser_port_port_all_action_forward_loopback_discard(
    cparser_context_t *context,
    char * *port_ptr)
{
    apollo_raw_oam_parserAct_t action;
    diag_portlist_t portlist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;
        
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if ('f' == TOKEN_CHAR(6,0))
    {
        action = RAW_OAM_PARSER_FORWARD;
    }
    else if ('l' == TOKEN_CHAR(6,0))
    {
        action = RAW_OAM_PARSER_LOOPBACK;
    }
    else if ('d' == TOKEN_CHAR(6,0))
    {
        action = RAW_OAM_PARSER_DISCARD;
    }	
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }	
	
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    diag_util_mprintf("Set OAM Parser\n"); 
    diag_util_mprintf("Port       Parser Action \n"); 	
    diag_util_mprintf("-----------------------------\n"); 	
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {    
        DIAG_UTIL_ERR_CHK(apollo_raw_oam_parser_set(port, action), ret); 			
        diag_util_mprintf("%-10u  %s\n", port, diagStr_oamParserAct[action]);   
    }	

    return CPARSER_OK;
}    /* end of cparser_cmd_oam_set_parser_port_port_all_action_forward_loopback_discard_trap_to_cpu */

/*
 * oam set state ( disable | enable ) 
 */
cparser_result_t
cparser_cmd_oam_set_state_disable_enable(
    cparser_context_t *context)
{
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;
        
    DIAG_UTIL_PARAM_CHK();

    if('e'==TOKEN_CHAR(3,0))
        enable = ENABLED;
    else if('d'==TOKEN_CHAR(3,0))
        enable = DISABLED;

    DIAG_UTIL_ERR_CHK(apollo_raw_oam_enable_set(enable), ret);

    diag_util_mprintf("Set OAM State to %s\n",diagStr_enable[enable]);	

    return CPARSER_OK;
}    /* end of cparser_cmd_oam_set_state_disable_enable */

/*
 * oam set trap-priority <UINT:priority>
 */
cparser_result_t
cparser_cmd_oam_set_trap_priority_priority(
    cparser_context_t *context,
    uint32_t  *priority_ptr)
{
    int32 ret = RT_ERR_FAILED;
        
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_oam_priority_set(*priority_ptr), ret);

    diag_util_mprintf("Set OAM Trap Priority to %d\n",*priority_ptr);	

    return CPARSER_OK;
}    /* end of cparser_cmd_oam_set_trap_priority_priority */

/*
 * oam get trap-priority
 */
cparser_result_t
cparser_cmd_oam_get_trap_priority(
    cparser_context_t *context)
{
    uint32 priority;
    int32 ret = RT_ERR_FAILED;
        
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(apollo_raw_oam_priority_get(&priority), ret);

    diag_util_mprintf("OAM Trap Priority is %d\n",priority);	

    return CPARSER_OK;
}    /* end of cparser_cmd_oam_get_trap_priority */

