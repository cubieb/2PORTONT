 /*
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
 * Purpose : Definition those RLDP command and APIs in the SDK diagnostic shell.
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
#include <dal/apollo/raw/apollo_raw_rldp.h>


/*
 * rldp dump
 */
cparser_result_t
cparser_cmd_rldp_dump(
    cparser_context_t *context)
{
    rtk_enable_t enable;
    apollo_raw_rldp_runoutDscAct_t action;
    apollo_raw_rldp_triggerMode_t mode1;
    apollo_raw_rldp_indSrc_t mode2;
    apollo_raw_rldp_compare_t mode3;	
    apollo_raw_rldp_loopState_t state;	
    uint32 time;
    uint32 count;
    rtk_mac_t mac;
    rtk_port_t port, loopPort;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

     DIAG_UTIL_ERR_CHK(apollo_raw_rldp_enable_get(&enable), ret); 	
     diag_util_mprintf("RLDP %s\n", diagStr_enable[enable]); 	

     DIAG_UTIL_ERR_CHK(apollo_raw_rldp_dscRunout_get(&action), ret); 	
     diag_util_mprintf("RLDP  DSC run-out action: %s\n",  action?"None-drop":"Drop");  	

     DIAG_UTIL_ERR_CHK(apollo_raw_rldp_trigger_get(&mode1), ret); 	
     diag_util_mprintf("RLDP Trigger Mode: %s\n",  mode1?"Periodic":"SA moving");   	

     DIAG_UTIL_ERR_CHK(apollo_raw_rldp_indicateSrc_get(&mode2), ret); 	
     diag_util_mprintf("RLDP Indication Mode: %s\n",  mode2?"CPU":"ASIC");

    DIAG_UTIL_ERR_CHK(apollo_raw_rldp_compare_get(&mode3), ret); 	
    diag_util_mprintf("RLDP Compare Mode: %s\n",  mode3?"SA + Magic Number + Identifier":"SA + Magic Number");  	

    DIAG_UTIL_ERR_CHK(apollo_raw_rldp_checkStatePeriod_get(&time), ret); 
    diag_util_mprintf("RLDP Check State Period: %u\n",  time); 
		
    DIAG_UTIL_ERR_CHK(apollo_raw_rldp_loopStatePeriod_get(&time), ret); 
    diag_util_mprintf("RLDP Loop State Period: %u\n",  time); 
         
    DIAG_UTIL_ERR_CHK(apollo_raw_rldp_checkStateCnt_get(&count), ret); 
    diag_util_mprintf("RLDP Check State Re-try Count: %u\n",  count); 

    DIAG_UTIL_ERR_CHK(apollo_raw_rldp_loopStateCnt_get(&count), ret); 
    diag_util_mprintf("RLDP Loop State Re-try Count: %u\n",  count); 

    DIAG_UTIL_ERR_CHK(apollo_raw_rldp_magicNum_get(&mac), ret); 		
    diag_util_mprintf("RLDP Magic Number: %s\n", diag_util_inet_mactoa(&mac.octet[0]));	

    DIAG_UTIL_ERR_CHK(apollo_raw_rldp_randomNum_get(&mac), ret); 		
    diag_util_mprintf("RLDP Random Number: %s\n", diag_util_inet_mactoa(&mac.octet[0]));	

    diag_util_mprintf(" Port     RLDP-State   Force-Mode   Entering   Leaving   Looping   Loop-Port \n"); 
    for (port=0;port<APOLLO_PORTIDMAX;port++)
    { 
        if (port !=2)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_rldp_portEnable_get(port, &enable), ret); 	
            diag_util_mprintf("%-8u  %7s", port, diagStr_enable[enable]); 
            DIAG_UTIL_ERR_CHK(apollo_raw_rldp_portForceLoopState_get(port, &state), ret); 	
            diag_util_mprintf("       %7s ", diagStr_enable[state]); 				
            DIAG_UTIL_ERR_CHK(apollo_raw_rldp_portLoopEnterState_get(port, &state), ret); 	
            diag_util_printf("      %s", state?"O":"X"); 	
            DIAG_UTIL_ERR_CHK(apollo_raw_rldp_portLoopLeaveState_get(port, &state), ret); 	
            diag_util_printf("          %s", state?"O":"X"); 
            DIAG_UTIL_ERR_CHK(apollo_raw_rldp_portLoopState_get(port, &state), ret); 	
            diag_util_printf("          %s", state?"O":"X"); 
            DIAG_UTIL_ERR_CHK(apollo_raw_rldp_portLoopPortNum_get(port, &loopPort), ret); 	
            diag_util_mprintf("          %u\n",  loopPort); 			
        }
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_dump */

/*
 * rldp set state ( disable | enable )
 */
cparser_result_t
cparser_cmd_rldp_set_state_disable_enable(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t enable;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();	

    if('e' == TOKEN_CHAR(3,0))
        enable = ENABLED;
    else if ('d' == TOKEN_CHAR(3,0))
        enable = DISABLED;
    else
        return CPARSER_NOT_OK;		
        
    DIAG_UTIL_ERR_CHK(apollo_raw_rldp_enable_set(enable), ret); 
    diag_util_mprintf("RLDP  %s\n",  diagStr_enable[enable]);   	

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_set_state_disable_enable */

/*
 * rldp get state
 */
cparser_result_t
cparser_cmd_rldp_get_state(
    cparser_context_t *context)
{
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

     DIAG_UTIL_ERR_CHK(apollo_raw_rldp_enable_get(&enable), ret); 	
     diag_util_mprintf("RLDP %s\n", diagStr_enable[enable]); 	

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_get_state */

/*
 * rldp set runout ( not-drop | drop )
 */
cparser_result_t
cparser_cmd_rldp_set_runout_not_drop_drop(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_rldp_runoutDscAct_t action;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();	

    if('n' == TOKEN_CHAR(3,0))
        action = RAW_RLDP_RUNOUT_FORWARD;
    else if ('d' == TOKEN_CHAR(3,0))
        action = RAW_RLDP_RUNOUT_DROP;
    else
        return CPARSER_NOT_OK;		
        
    DIAG_UTIL_ERR_CHK(apollo_raw_rldp_dscRunout_set(action), ret); 
    diag_util_mprintf("RLDP  DSC run-out action: %s\n",  action?"None-drop":"Drop");   

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_set_runout_not_drop_drop */

/*
 * rldp get runout
 */
cparser_result_t
cparser_cmd_rldp_get_runout(
    cparser_context_t *context)
{
    apollo_raw_rldp_runoutDscAct_t action;
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

     DIAG_UTIL_ERR_CHK(apollo_raw_rldp_dscRunout_get(&action), ret); 	
    diag_util_mprintf("RLDP  DSC run-out action: %s\n",  action?"None-drop":"Drop");  	

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_get_runout */

/*
 * rldp set trigger ( sa-moving | periodic )
 */
cparser_result_t
cparser_cmd_rldp_set_trigger_sa_moving_periodic(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_rldp_triggerMode_t mode;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();	

    if('s' == TOKEN_CHAR(3,0))
        mode = RAW_RLDP_TRIG_SAMOVING;
    else if ('p' == TOKEN_CHAR(3,0))
        mode= RAW_RLDP_TRIG_PERIODIC;
    else
        return CPARSER_NOT_OK;		
        
    DIAG_UTIL_ERR_CHK(apollo_raw_rldp_trigger_set(mode), ret); 
    diag_util_mprintf("RLDP Trigger Mode: %s\n",  mode?"Periodic":"SA moving");   

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_set_trigger_sa_moving_periodic */

/*
 * rldp get trigger
 */
cparser_result_t
cparser_cmd_rldp_get_trigger(
    cparser_context_t *context)
{
    apollo_raw_rldp_triggerMode_t mode;
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

     DIAG_UTIL_ERR_CHK(apollo_raw_rldp_trigger_get(&mode), ret); 	
     diag_util_mprintf("RLDP Trigger Mode: %s\n",  mode?"Periodic":"SA moving");   

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_get_trigger */

/*
 * rldp set indicate ( asic | cpu )
 */
cparser_result_t
cparser_cmd_rldp_set_indicate_asic_cpu(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_rldp_indSrc_t mode;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();	

    if('a' == TOKEN_CHAR(3,0))
        mode = RAW_RLDP_INDSRC_ASIC;
    else if ('c' == TOKEN_CHAR(3,0))
        mode= RAW_RLDP_INDSRC_CPU;
    else
        return CPARSER_NOT_OK;		
        
    DIAG_UTIL_ERR_CHK(apollo_raw_rldp_indicateSrc_set(mode), ret); 
    diag_util_mprintf("RLDP Indication Mode: %s\n",  mode?"CPU":"ASIC");  

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_set_indicate_asic_cpu */

/*
 * rldp get indicate
 */
cparser_result_t
cparser_cmd_rldp_get_indicate(
    cparser_context_t *context)
{
    apollo_raw_rldp_indSrc_t mode;
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

     DIAG_UTIL_ERR_CHK(apollo_raw_rldp_indicateSrc_get(&mode), ret); 	
    diag_util_mprintf("RLDP Indication Mode: %s\n",  mode?"CPU":"ASIC");

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_get_indicate */

/*
 * rldp set compare ( no-identifier | identifier )
 */
cparser_result_t
cparser_cmd_rldp_set_compare_no_identifier_identifier(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_rldp_compare_t mode;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();	

    if('n' == TOKEN_CHAR(3,0))
        mode = RAW_RLDP_CMP_SA_MAG;
    else if ('i' == TOKEN_CHAR(3,0))
        mode= RAW_RLDP_CMP_SA_MAG_IDF;
    else
        return CPARSER_NOT_OK;		
        
    DIAG_UTIL_ERR_CHK(apollo_raw_rldp_compare_set(mode), ret); 
    diag_util_mprintf("RLDP Compare Mode: %s\n",  mode?"SA + Magic Number + Identifier":"SA + Magic Number");  

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_set_compare_no_identifier_identifier */

/*
 * rldp get compare
 */
cparser_result_t
cparser_cmd_rldp_get_compare(
    cparser_context_t *context)
{
    apollo_raw_rldp_compare_t mode;
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_rldp_compare_get(&mode), ret); 	
    diag_util_mprintf("RLDP Compare Mode: %s\n",  mode?"SA + Magic Number + Identifier":"SA + Magic Number");  

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_get_compare */

/*
 * rldp set handle ( asic | cpu )
 */
cparser_result_t
cparser_cmd_rldp_set_handle_asic_cpu(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_rldp_handle_t mode;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();	

    if('a' == TOKEN_CHAR(3,0))
        mode = RAW_RLDP_HANDLE_ASIC;
    else if ('c' == TOKEN_CHAR(3,0))
        mode= RAW_RLDP_HANDLE_CPU;
    else
        return CPARSER_NOT_OK;		
        
    DIAG_UTIL_ERR_CHK(apollo_raw_rldp_handle_set(mode), ret); 
    diag_util_mprintf("RLDP Handle Mode: %s\n",  mode?"CPU":"ASIC");  

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_set_handle_asic_cpu */

/*
 * rldp get handle
 */
cparser_result_t
cparser_cmd_rldp_get_handle(
    cparser_context_t *context)
{
    apollo_raw_rldp_handle_t mode;
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

     DIAG_UTIL_ERR_CHK(apollo_raw_rldp_handle_get(&mode), ret); 	
    diag_util_mprintf("RLDP Handle Mode: %s\n",  mode?"CPU":"ASIC");

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_get_handle */

/*
 * rldp set genrandom
 */
cparser_result_t
cparser_cmd_rldp_set_genrandom(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED; 
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();	 	
        
    DIAG_UTIL_ERR_CHK(apollo_raw_rldp_genRandom_set(ENABLED), ret); 
    diag_util_mprintf("RLDP  re-generate random number");

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_set_genrandom */

/*
 * rldp set ( check | loop ) period <UINT:time>
 */
cparser_result_t
cparser_cmd_rldp_set_check_loop_period_time(
    cparser_context_t *context,
    uint32_t  *time_ptr)
{
    int32 ret = RT_ERR_FAILED;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();	

    if('c' == TOKEN_CHAR(2,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_rldp_checkStatePeriod_set(*time_ptr), ret); 
        diag_util_mprintf("RLDP Check State Period: %u\n",  *time_ptr); 
    }
    else if ('l' == TOKEN_CHAR(2,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_rldp_loopStatePeriod_set(*time_ptr), ret); 
        diag_util_mprintf("RLDP Loop State Period: %u\n",  *time_ptr); 
    }
    else
        return CPARSER_NOT_OK;		 

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_set_check_loop_period_time */

/*
 * rldp get ( check | loop ) period
 */
cparser_result_t
cparser_cmd_rldp_get_check_loop_period(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 time;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();	

    if('c' == TOKEN_CHAR(2,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_rldp_checkStatePeriod_get(&time), ret); 
        diag_util_mprintf("RLDP Check State Period: %u\n",  time); 
    }
    else if ('l' == TOKEN_CHAR(2,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_rldp_loopStatePeriod_get(&time), ret); 
        diag_util_mprintf("RLDP Loop State Period: %u\n",  time); 
    }
    else
        return CPARSER_NOT_OK;	

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_get_check_loop_period */

/*
 * rldp set ( check | loop ) number <UINT:count>
 */
cparser_result_t
cparser_cmd_rldp_set_check_loop_number_count(
    cparser_context_t *context,
    uint32_t  *count_ptr)
{
    int32 ret = RT_ERR_FAILED;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();	

    if('c' == TOKEN_CHAR(2,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_rldp_checkStateCnt_set(*count_ptr), ret); 
        diag_util_mprintf("RLDP Check State Re-try Count: %u\n",  *count_ptr); 
    }
    else if ('l' == TOKEN_CHAR(2,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_rldp_loopStateCnt_set(*count_ptr), ret); 
        diag_util_mprintf("RLDP Loop State Re-try Count: %u\n",  *count_ptr); 
    }
    else
        return CPARSER_NOT_OK;	

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_set_check_loop_number_count */

/*
 * rldp get ( check | loop ) number
 */
cparser_result_t
cparser_cmd_rldp_get_check_loop_number(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 count;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();	

    if('c' == TOKEN_CHAR(2,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_rldp_checkStatePeriod_get(&count), ret); 
        diag_util_mprintf("RLDP Check State Re-try Count: %u\n",  count); 
    }
    else if ('l' == TOKEN_CHAR(2,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_rldp_loopStateCnt_get(&count), ret); 
        diag_util_mprintf("RLDP Loop State Re-try Count: %u\n",  count); 
    }
    else
        return CPARSER_NOT_OK;	

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_get_check_loop_number */

/*
 * rldp set magic <MACADDR:number>
 */
cparser_result_t
cparser_cmd_rldp_set_magic_number(
    cparser_context_t *context,
    cparser_macaddr_t  *number_ptr)
{
    rtk_mac_t mac;
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

    osal_memcpy(&mac.octet, number_ptr->octet, ETHER_ADDR_LEN);    
    DIAG_UTIL_ERR_CHK(apollo_raw_rldp_magicNum_set(&mac), ret); 	
    diag_util_mprintf("RLDP MAgic Number: %s\n", diag_util_inet_mactoa(&mac.octet[0]));
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_set_magic_number */

/*
 * rldp get magic
 */
cparser_result_t
cparser_cmd_rldp_get_magic(
    cparser_context_t *context)
{
    rtk_mac_t mac;
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
   
    DIAG_UTIL_ERR_CHK(apollo_raw_rldp_magicNum_get(&mac), ret); 		

    diag_util_mprintf("RLDP Magic Number: %s\n", diag_util_inet_mactoa(&mac.octet[0]));

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_get_magic */

/*
 * rldp get random
 */
cparser_result_t
cparser_cmd_rldp_get_random(
    cparser_context_t *context)
{
    rtk_mac_t mac;
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
   
    DIAG_UTIL_ERR_CHK(apollo_raw_rldp_randomNum_get(&mac), ret); 		

    diag_util_mprintf("RLDP Random Number: %s\n", diag_util_inet_mactoa(&mac.octet[0]));

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_get_random */

/*
 * rldp set port ( <PORT_LIST:port> | all ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_rldp_set_port_port_all_state_disable_enable(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port;
    rtk_enable_t enable;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();	
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if('e' == TOKEN_CHAR(5,0))
        enable = ENABLED;
    else if ('d' == TOKEN_CHAR(5,0))
        enable = DISABLED;
    else
        return CPARSER_NOT_OK;		
        
    /* exclude port 2 */ 
    portlist.portmask.bits[0] = portlist.portmask.bits[0] & 0xFFFFFFFB; 
    diag_util_mprintf("Set RLDP port state\n"); 
    diag_util_mprintf("Port       RLDP-State \n"); 		
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    { 
        DIAG_UTIL_ERR_CHK(apollo_raw_rldp_portEnable_set(port, enable), ret); 
        diag_util_mprintf("%-10u  %s\n", port, diagStr_enable[enable]);   		
    } 
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_set_port_port_all_state_disable_enable */

/*
 * rldp get port ( <PORT_LIST:port> | all ) state
 */
cparser_result_t
cparser_cmd_rldp_get_port_port_all_state(
    cparser_context_t *context,
    char * *port_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
	
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    /* exclude port 2 */ 
    portlist.portmask.bits[0] = portlist.portmask.bits[0] & 0xFFFFFFFB; 	
    diag_util_mprintf("RLDP port state  \n"); 
    diag_util_mprintf(" Port     RLDP-State \n"); 
    diag_util_mprintf("--------------------------------------------------------------------------\n"); 		
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {  
        DIAG_UTIL_ERR_CHK(apollo_raw_rldp_portEnable_get(port, &enable), ret); 	
        diag_util_mprintf("%-10u  %s\n", port, diagStr_enable[enable]); 				
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_get_port_port_all_state */

/*
 * rldp set port ( <PORT_LIST:port> | all ) ( enter-clear | leave-clear )
 */
cparser_result_t
cparser_cmd_rldp_set_port_port_all_enter_clear_leave_clear(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port;
    uint32 state;
    uint8   buf[UTIL_PORT_MASK_BUFFER_LENGTH];		   
	
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();	
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);	
        
    /* exclude port 2 */ 
    portlist.portmask.bits[0] = portlist.portmask.bits[0] & 0xFFFFFFFB; 
    diag_util_mprintf("Set RLDP Status Clear\n"); 		
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    { 
        if('e' == TOKEN_CHAR(4,0))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_rldp_portLoopEnterState_set(port, ENABLED), ret); 
            state = 1;
        }
        else if ('l' == TOKEN_CHAR(4,0))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_rldp_portLoopLeaveState_set(port, ENABLED), ret); 
            state = 0;			
        }
        else
            return CPARSER_NOT_OK;	   		
    } 
    diag_util_lPortMask2str(buf, &portlist.portmask);		
    diag_util_mprintf("Ports:%s %s Clear\n",buf, state?"Entering Status":"Leaving Status"); 
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_set_port_port_all_enter_clear_leave_clear */

/*
 * rldp get port ( <PORT_LIST:port> | all ) status
 */
cparser_result_t
cparser_cmd_rldp_get_port_port_all_status(
    cparser_context_t *context,
    char * *port_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    apollo_raw_rldp_loopState_t state;
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
	
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    /* exclude port 2 */ 
    portlist.portmask.bits[0] = portlist.portmask.bits[0] & 0xFFFFFFFB; 	
    diag_util_mprintf("RLDP Port Status: O for triggered, X for none triggered  \n"); 
    diag_util_mprintf(" Port     Entering    Leaving    Looping \n"); 
    diag_util_mprintf("--------------------------------------------------------------------------\n"); 		
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {  
        DIAG_UTIL_ERR_CHK(apollo_raw_rldp_portLoopEnterState_get(port, &state), ret); 	
        diag_util_printf("%-7u       %s", port, state?"O":"X"); 	
        DIAG_UTIL_ERR_CHK(apollo_raw_rldp_portLoopLeaveState_get(port, &state), ret); 	
        diag_util_printf("         %s", state?"O":"X"); 
        DIAG_UTIL_ERR_CHK(apollo_raw_rldp_portLoopState_get(port, &state), ret); 	
        diag_util_printf("         %s\n", state?"O":"X"); 		
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_get_port_port_all_enter_leave_loop */

/*
 * rldp set port ( <PORT_LIST:port> | all ) ctrl-loop ( none-looping | looping )
 */
cparser_result_t
cparser_cmd_rldp_set_port_port_all_ctrl_loop_none_looping_looping(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port;
    apollo_raw_rldp_loopState_t  state;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();	
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if('l' == TOKEN_CHAR(5,0))
        state = RAW_RLDP_LOOPING;
    else if ('n' == TOKEN_CHAR(5,0))
        state = RAW_RLDP_NORMAL;
    else
        return CPARSER_NOT_OK;		
        
    /* exclude port 2 */ 
    portlist.portmask.bits[0] = portlist.portmask.bits[0] & 0xFFFFFFFB; 
    diag_util_mprintf("Set RLDP port force state\n"); 
    diag_util_mprintf("Port       Force-State \n"); 		
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    { 
        DIAG_UTIL_ERR_CHK(apollo_raw_rldp_portForceLoopState_set(port, state), ret); 
        diag_util_mprintf("%-10u  %s\n", port, state?"Loop":"None-loop");   		
    } 	

    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_set_port_port_all_ctrl_loop_none_looping_looping */

/*
 * rldp get port ( <PORT_LIST:port> | all ) ctrl-loop
 */
cparser_result_t
cparser_cmd_rldp_get_port_port_all_ctrl_loop(
    cparser_context_t *context,
    char * *port_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    apollo_raw_rldp_loopState_t  state;
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();	
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    /* exclude port 2 */ 
    portlist.portmask.bits[0] = portlist.portmask.bits[0] & 0xFFFFFFFB; 	
    diag_util_mprintf("RLDP port force state  \n"); 
    diag_util_mprintf(" Port    Force-State \n"); 
    diag_util_mprintf("--------------------------------------------------------------------------\n"); 		
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {  
        DIAG_UTIL_ERR_CHK(apollo_raw_rldp_portForceLoopState_get(port, &state), ret); 	
        diag_util_mprintf("%-10u  %s\n", port, state?"Loop":"None-loop"); 				
    }
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_get_port_port_all_ctrl_loop */

/*
 * rldp get port ( <PORT_LIST:port> | all ) loop-port
 */
cparser_result_t
cparser_cmd_rldp_get_port_port_all_loop_port(
    cparser_context_t *context,
    char * *port_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    rtk_port_t loopPort;
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();	
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    /* exclude port 2 */ 
    portlist.portmask.bits[0] = portlist.portmask.bits[0] & 0xFFFFFFFB; 	
    diag_util_mprintf("RLDP looped port  \n"); 
    diag_util_mprintf(" Port    Loop-Port \n"); 
    diag_util_mprintf("--------------------------------------------------------------------------\n"); 		
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {  
        DIAG_UTIL_ERR_CHK(apollo_raw_rldp_portLoopPortNum_get(port, &loopPort), ret); 	
        diag_util_mprintf("%-10u  %u\n", port,  loopPort); 				
    }
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rldp_get_port_port_all_loop_port */

/*
 * rlpp set trap ( disable | enable )
 */
cparser_result_t
cparser_cmd_rlpp_set_trap_disable_enable(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t enable;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();	

    if('e' == TOKEN_CHAR(3,0))
        enable = ENABLED;
    else if ('d' == TOKEN_CHAR(3,0))
        enable = DISABLED;
    else
        return CPARSER_NOT_OK;		
        
    DIAG_UTIL_ERR_CHK(apollo_raw_rlpp_trap_set(enable), ret); 
    diag_util_mprintf("RLPP Trap  %s\n",  diagStr_enable[enable]);   	
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rlpp_set_trap_disable_enable */

/*
 * rlpp get trap
 */
cparser_result_t
cparser_cmd_rlpp_get_trap(
    cparser_context_t *context)
{
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

     DIAG_UTIL_ERR_CHK(apollo_raw_rlpp_trap_get(&enable), ret); 	
     diag_util_mprintf("RLPP Trap to CPU %s\n", diagStr_enable[enable]); 				

    return CPARSER_OK;
}    /* end of cparser_cmd_rlpp_get_trap */

