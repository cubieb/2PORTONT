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
 * Purpose : Definition those command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) switch commands.    
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
#include <dal/apollo/raw/apollo_raw_switch.h>
/*
 * switch get 48-pass-1 state
 */
cparser_result_t
cparser_cmd_switch_get_48_pass_1_state(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_enable_t enable;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_switch_48pass1Enable_get(&enable), ret);

    diag_util_mprintf("48 Pass 1 function: %s\n",diagStr_enable[enable]);

    return CPARSER_OK;
}    /* end of cparser_cmd_switch_get_48_pass_1_state */

/*
 * switch set 48-pass-1 state ( disable | enable )
 */
cparser_result_t
cparser_cmd_switch_set_48_pass_1_state_disable_enable(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();

    if ('d' == TOKEN_CHAR(4,0))
    {
        enable = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(4,0))
    {
        enable = ENABLED;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_switch_48pass1Enable_set(enable), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_switch_set_48_pass_1_state_disable_enable */

/*
 * switch set ipg-compensation state ( disable | enable )
 */
cparser_result_t
cparser_cmd_switch_set_ipg_compensation_state_disable_enable(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();

    if ('d' == TOKEN_CHAR(4,0))
    {
        enable = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(4,0))
    {
        enable = ENABLED;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_switch_shortIpgEnable_set(enable), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_switch_set_ipg_compensation_state_disable_enable */

/*
 * switch set ipg-compensation ( 65ppm | 90ppm )
 */
cparser_result_t
cparser_cmd_switch_set_ipg_compensation_65ppm_90ppm(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    apollo_raw_ipgCompMode_t mode;
    
    DIAG_UTIL_PARAM_CHK();

    if ('6' == TOKEN_CHAR(3,0))
    {
        mode = RAW_IPGCOMPMODE_65PPM;
    }
    else if ('9' == TOKEN_CHAR(3,0))
    {
        mode = RAW_IPGCOMPMODE_90PPM;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_switch_ipgCompensation_set(mode), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_switch_set_ipg_compensation_65ppm_90ppm */

/*
 * switch get ipg-compensation state
 */
cparser_result_t
cparser_cmd_switch_get_ipg_compensation_state(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_enable_t enable;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_switch_shortIpgEnable_get(&enable), ret);

    diag_util_mprintf("Short IPG function: %s\n",diagStr_enable[enable]);

    return CPARSER_OK;
}    /* end of cparser_cmd_switch_get_ipg_compensation_state */

/*
 * switch get ipg-compensation
 */
cparser_result_t
cparser_cmd_switch_get_ipg_compensation(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    apollo_raw_ipgCompMode_t mode;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_switch_ipgCompensation_get(&mode), ret);

    diag_util_mprintf("IPG compensation: %s\n",diagStr_ipgCompensation[mode]);


    return CPARSER_OK;
}    /* end of cparser_cmd_switch_get_ipg_compensation */


/*
 * switch get rx-check-crc port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t
cparser_cmd_switch_get_rx_check_crc_port_ports_all_state(
    cparser_context_t *context,
    char * *ports_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;
        
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port       Status \n"); 	
    diag_util_mprintf("-----------------------------\n"); 	
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {    
        DIAG_UTIL_ERR_CHK(apollo_raw_switch_rxCrcCheck_get(port, &enable), ret); 			
        diag_util_mprintf("%-10u  %s\n", port, diagStr_enable[enable]);   
    }	

    return CPARSER_OK;
}    /* end of cparser_cmd_switch_get_rx_check_crc_port_ports_all_state */

/*
 * switch set rx-check-crc port ( <PORT_LIST:ports> | all ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_switch_set_rx_check_crc_port_ports_all_state_disable_enable(
    cparser_context_t *context,
    char * *ports_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;
        
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if('e'==TOKEN_CHAR(6,0))
        enable = ENABLED;
    else if('d'==TOKEN_CHAR(6,0))
        enable = DISABLED;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {    
        DIAG_UTIL_ERR_CHK(apollo_raw_switch_rxCrcCheck_set(port, enable), ret); 			
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_switch_set_rx_check_crc_port_ports_all_state_disable_enable */

/*
 * switch set bypass-tx-crc state ( disable | enable )
 */
cparser_result_t
cparser_cmd_switch_set_bypass_tx_crc_state_disable_enable(
    cparser_context_t *context)
{
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;
        
    DIAG_UTIL_PARAM_CHK();

    if('e'==TOKEN_CHAR(4,0))
        enable = ENABLED;
    else if('d'==TOKEN_CHAR(4,0))
        enable = DISABLED;
  
    DIAG_UTIL_ERR_CHK(apollo_raw_switch_globalRxCrcCheck_set(enable), ret); 			

    return CPARSER_OK;
}    /* end of cparser_cmd_switch_set_bypass_tx_crc_state_disable_enable */

/*
 * switch get bypass-tx-crc state
 */
cparser_result_t
cparser_cmd_switch_get_bypass_tx_crc_state(
    cparser_context_t *context)
{
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;
        
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
   
    DIAG_UTIL_ERR_CHK(apollo_raw_switch_globalRxCrcCheck_get(&enable), ret); 			

    diag_util_mprintf("Bypass Tx CRC: %s\n", diagStr_enable[enable]); 

    return CPARSER_OK;
}    /* end of cparser_cmd_switch_get_bypass_tx_crc_state */



/*
 * switch set mac-addr <MACADDR:addr>
 */
cparser_result_t
cparser_cmd_switch_set_mac_addr_addr(
    cparser_context_t *context,
    cparser_macaddr_t  *addr_ptr)
{
    rtk_mac_t mac;
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

    osal_memcpy(&mac.octet, addr_ptr->octet, ETHER_ADDR_LEN);    
    DIAG_UTIL_ERR_CHK(apollo_raw_switch_macAddr_set(&mac), ret); 		

    return CPARSER_OK;
}    /* end of cparser_cmd_switch_set_mac_addr_addr */

/*
 * switch get mac-addr <MACADDR:addr>
 */
cparser_result_t
cparser_cmd_switch_get_mac_addr(
    cparser_context_t *context)
{
    rtk_mac_t mac;
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
   
    DIAG_UTIL_ERR_CHK(apollo_raw_switch_macAddr_get(&mac), ret); 		

    diag_util_mprintf("Switch MAC Address: %s\n", diag_util_inet_mactoa(&mac.octet[0]));

    return CPARSER_OK;
}    /* end of cparser_cmd_switch_get_mac_addr_addr */


/*
 * switch set max-pkt-len ( fe | ge ) port ( <PORT_LIST:ports> | all ) index <UINT:index>
 */
cparser_result_t
cparser_cmd_switch_set_max_pkt_len_fe_ge_port_ports_all_index_index(
    cparser_context_t *context,
    char * *ports_ptr,
    uint32_t  *index_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    apollo_raw_linkSpeed_t speed;
    int32 ret = RT_ERR_FAILED;
        
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if('g'==TOKEN_CHAR(3,0))
     {
        speed = LINKSPEED_GIGA;
     }
    else if('f'==TOKEN_CHAR(3,0))
    {
        speed = LINKSPEED_100M;
    }
	
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {    
        DIAG_UTIL_ERR_CHK(apollo_raw_switch_maxPktLenSpeed_set( port, speed, *index_ptr), ret); 			
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_switch_set_max_pkt_len_fe_ge_port_ports_all_index_index */

/*
 * switch get max-pkt-len ( fe | ge ) port ( <PORT_LIST:ports> | all )
 */
cparser_result_t
cparser_cmd_switch_get_max_pkt_len_fe_ge_port_ports_all(
    cparser_context_t *context,
    char * *ports_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    apollo_raw_linkSpeed_t speed;
    uint32 index;
    int32 ret = RT_ERR_FAILED;
        
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    diag_util_mprintf("Port       Speed       Config \n"); 	
    diag_util_mprintf("-----------------------------\n"); 	
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {    
        if('g'==TOKEN_CHAR(3,0))
        {
            speed = LINKSPEED_GIGA;
        }
        else if('f'==TOKEN_CHAR(3,0))
        {
            speed = LINKSPEED_100M;
        }
        DIAG_UTIL_ERR_CHK(apollo_raw_switch_maxPktLenSpeed_get(port, speed, &index), ret); 			
        diag_util_mprintf("%-10u  %s    %d\n", port, diagStr_portSpeed[speed], index);   
    }


    return CPARSER_OK;
}    /* end of cparser_cmd_switch_get_max_pkt_len_fe_ge_port_ports_all */


/*
 * switch set max-pkt-len index <UINT:index> length <UINT:len>
 */
cparser_result_t
cparser_cmd_switch_set_max_pkt_len_index_index_length_len(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *len_ptr)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(apollo_raw_switch_maxPktLen_set(*index_ptr, *len_ptr), ret); 			

    return CPARSER_OK;
}    /* end of cparser_cmd_switch_set_max_pkt_len_index_index_length_len */

/*
 * switch get max-pkt-len index <UINT:index>
 */
cparser_result_t
cparser_cmd_switch_get_max_pkt_len_index_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    uint32 length;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_switch_maxPktLen_get(*index_ptr, &length), ret); 			
    diag_util_mprintf("Max-Length Index %u is Length %u bytes.\n", *index_ptr, length);   

    return CPARSER_OK;
}    /* end of cparser_cmd_switch_get_max_pkt_len_index_index */

/*
 * switch set limit-pause state ( disable | enable )
 */
cparser_result_t
cparser_cmd_switch_set_limit_pause_state_disable_enable(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();

    if ('d' == TOKEN_CHAR(4,0))
    {
        enable = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(4,0))
    {
        enable = ENABLED;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_switch_limitPause_set(enable), ret);
	
    return CPARSER_OK;
}    /* end of cparser_cmd_switch_set_limit_pause_state_disable_enable */

/*
 * switch get limit-pause state
 */
cparser_result_t
cparser_cmd_switch_get_limit_pause_state(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_enable_t enable;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_switch_limitPause_get(&enable), ret);
    diag_util_mprintf("Limit Pause Frame: %s\n",diagStr_enable[enable]);

    return CPARSER_OK;
}    /* end of cparser_cmd_switch_get_limit_pause_state */

/*
 * switch set small-ipg-tag port ( <PORT_LIST:ports> | all ) state ( enable | disable )
 */
cparser_result_t
cparser_cmd_switch_set_small_ipg_tag_port_ports_all_state_enable_disable(
    cparser_context_t *context,
    char * *ports_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;
        
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if('e'==TOKEN_CHAR(6,0))
        enable = ENABLED;
    else if('d'==TOKEN_CHAR(6,0))
        enable = DISABLED;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {    
        DIAG_UTIL_ERR_CHK(apollo_raw_switch_smallTagIpg_set(port, enable), ret); 			
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_switch_set_small_ipg_tag_port_ports_all_state_enable_disable */

/*
 * switch get small-ipg-tag port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t
cparser_cmd_switch_get_small_ipg_tag_port_ports_all_state(
    cparser_context_t *context,
    char * *ports_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;
        
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port       Status \n"); 	
    diag_util_mprintf("-----------------------------\n"); 	
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {    
        DIAG_UTIL_ERR_CHK(apollo_raw_switch_smallTagIpg_get(port, &enable), ret); 			
        diag_util_mprintf("%-10u  %s\n", port, diagStr_enable[enable]);   
    }	

    return CPARSER_OK;
}    /* end of cparser_cmd_switch_get_small_ipg_tag_port_ports_all_state */


/*
 * switch get back-pressure
 */
cparser_result_t
cparser_cmd_switch_get_back_pressure(
    cparser_context_t *context)
{
    apollo_raw_backPressure_t state;
    int32 ret = RT_ERR_FAILED;
        
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_switch_backPressure_get(&state), ret); 			
    diag_util_mprintf("Back-pressure: %s\n",  diagStr_backPressure[state]);   
	
    return CPARSER_OK;
}    /* end of cparser_cmd_switch_get_back_pressure */

/*
 * switch set back-pressure ( jam | defer )
 */
cparser_result_t
cparser_cmd_switch_set_back_pressure_jam_defer(
    cparser_context_t *context)
{
    apollo_raw_backPressure_t state;
    int32 ret = RT_ERR_FAILED;
        
    DIAG_UTIL_PARAM_CHK();

    if('j'==TOKEN_CHAR(3,0))
        state = RAW_BACKPRESSURE_JAM;
    else if('d'==TOKEN_CHAR(3,0))
        state = RAW_BACKPRESSURE_DEFER;
   
    DIAG_UTIL_ERR_CHK(apollo_raw_switch_backPressure_set(state), ret); 			

    return CPARSER_OK;
}    /* end of cparser_cmd_switch_set_back_pressure_jam_defer */

/*
 * switch set small-pkt port ( <PORT_LIST:ports> | all ) state ( enable | disable )
 */
cparser_result_t
cparser_cmd_switch_set_small_pkt_port_ports_all_state_enable_disable(
    cparser_context_t *context,
    char * *ports_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;
        
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if('e'==TOKEN_CHAR(6,0))
        enable = ENABLED;
    else if('d'==TOKEN_CHAR(6,0))
        enable = DISABLED;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {    
        DIAG_UTIL_ERR_CHK(apollo_raw_switch_smallPkt_set(port, enable), ret); 			
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_switch_set_small_pkt_port_ports_all_state_enable_disable */

/*
 * switch get small-pkt port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t
cparser_cmd_switch_get_small_pkt_port_ports_all_state(
    cparser_context_t *context,
    char * *ports_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;
        
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port       Status \n"); 	
    diag_util_mprintf("-----------------------------\n"); 	
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {    
        DIAG_UTIL_ERR_CHK(apollo_raw_switch_smallPkt_get(port, &enable), ret); 			
        diag_util_mprintf("%-10u  %s\n", port, diagStr_enable[enable]);   
    }	

    return CPARSER_OK;
}    /* end of cparser_cmd_switch_get_small_pkt_port_ports_all_state */

/*
 * switch reset ( chip | global ) 
 */
cparser_result_t
cparser_cmd_switch_reset_global_chip(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    apollo_raw_chipReset_t reset=RAW_CHIPRESET_END;
    
    DIAG_UTIL_PARAM_CHK();

    if (!osal_strcmp(TOKEN_STR(2),"global"))
    {
        reset = RAW_SW_GLOBAL_RST;
    }  
    else if (!osal_strcmp(TOKEN_STR(2),"chip"))
    {
        reset = RAW_SW_CHIP_RST;
    }
    else
        return CPARSER_NOT_OK;
	
    DIAG_UTIL_ERR_CHK(apollo_raw_switch_chipReset_set(reset), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_switch_reset_gphy_global_rsgmii_config_queue_nic_voip_cpu_wdog_pon */



