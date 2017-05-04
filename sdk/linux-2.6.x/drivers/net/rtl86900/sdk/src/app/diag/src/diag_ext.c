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
 * Purpose : Definition those Extension command and APIs in the SDK diagnostic shell.
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
#include <parser/cparser_priv.h>
#include <dal/apollo/raw/apollo_raw_congest.h>
#include <dal/apollo/raw/apollo_raw_switch.h>
#include <diag_str.h>


/*
 * port get special-congest ( <PORT_LIST:ports> | all )
 */
cparser_result_t
cparser_cmd_port_get_special_congest_ports_all(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;
    uint32              sustTimer;
    uint32              cgstTimer;

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == *ports_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_OUTPUT_INIT();
    
    /* show specific port isolation info */
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);  

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sc_sustTmr_get(port, &sustTimer), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_sc_cgstTmr_get(port, &cgstTimer), ret);
        diag_util_mprintf("Port %2u, sustain timer = %u sec, congest timer = %u sec\n", port, sustTimer, cgstTimer);
    }
    
    return CPARSER_OK;

}    /* end of cparser_cmd_port_get_special_congest_ports_all */

/*
 * port get special-congest ( <PORT_LIST:ports> | all ) indicator
 */
cparser_result_t
cparser_cmd_port_get_special_congest_ports_all_indicator(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;
    uint32              Occur;

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == *ports_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_OUTPUT_INIT();
    
    /* show specific port isolation info */
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);  

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sc_cgstInd_get(port, &Occur), ret);
        diag_util_mprintf("\tPort %2u : ever occur = %u\n", port, Occur);
    }
    
    return CPARSER_OK;

}    /* end of cparser_cmd_port_get_special_congest_ports_all_reset_occur */

/*
 * port set special-congest ( <PORT_LIST:ports> | all ) sustain-timer <UINT:second>
 */
cparser_result_t
cparser_cmd_port_set_special_congest_ports_all_sustain_timer_second(
    cparser_context_t *context,
    char * *ports_ptr,
    uint32_t  *second_ptr)
{
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    diag_portlist_t    portlist;
    DIAG_UTIL_PARAM_CHK();  
    RT_PARAM_CHK((NULL == *ports_ptr), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sc_sustTmr_set(port, *second_ptr), ret);
    }  
    return CPARSER_OK;

}    /* end of cparser_cmd_port_set_special_congest_ports_all_sustain_timer_second */

/*
 * port clear special-congest ( <PORT_LIST:ports> | all ) indicator
 */
cparser_result_t
cparser_cmd_port_clear_special_congest_ports_all_indicator(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    diag_portlist_t    portlist;
    DIAG_UTIL_PARAM_CHK();  
    RT_PARAM_CHK((NULL == *ports_ptr), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sc_cgstInd_reset(port), ret);
    }  
    return CPARSER_OK;
}    /* end of cparser_cmd_port_set_special_congest_ports_all_reset_occur */

/*
 * iol set max-length state ( disable | enable )
 */
cparser_result_t
cparser_cmd_iol_set_max_length_state_disable_enable(
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

    if ((ret = reg_field_write(IOL_RXDROP_CFGr, RX_IOL_MAX_LENGTH_CFGf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L2|MOD_DAL), "");
        return ret;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_iol_set_max_length_state_disable_enable */

/*
 * iol get max-length
 */
cparser_result_t
cparser_cmd_iol_get_max_length(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_enable_t enable;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if ((ret = reg_field_read(IOL_RXDROP_CFGr, RX_IOL_MAX_LENGTH_CFGf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L2|MOD_DAL), "");
        return ret;
    }

    diag_util_mprintf("IOL max length checking: %s\n", diagStr_enable[enable]);

    return CPARSER_OK;
}    /* end of cparser_cmd_iol_get_max_length */

/*
 * iol set error-length state ( disable | enable )
 */
cparser_result_t
cparser_cmd_iol_set_error_length_state_disable_enable(
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

    if ((ret = reg_field_write(IOL_RXDROP_CFGr, RX_IOL_ERROR_LENGTH_CFGf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L2|MOD_DAL), "");
        return ret;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_iol_set_error_length_state_disable_enable */

/*
 * iol get error-length
 */
cparser_result_t
cparser_cmd_iol_get_error_length(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_enable_t enable;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if ((ret = reg_field_read(IOL_RXDROP_CFGr, RX_IOL_ERROR_LENGTH_CFGf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L2|MOD_DAL), "");
        return ret;
    }

    diag_util_mprintf("IOL error length checking: %s\n", diagStr_enable[enable]);

    return CPARSER_OK;
}    /* end of cparser_cmd_iol_get_error_length */

/*
 * iol set drop-16-collision state ( disable | enable )
 */
cparser_result_t
cparser_cmd_iol_set_drop_16_collision_state_disable_enable(
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

    if ((ret = reg_field_write(CFG_UNHIOLr, IOL_16DROPf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L2|MOD_DAL), "");
        return ret;
    }


    return CPARSER_OK;
}    /* end of cparser_cmd_iol_set_drop_16_collision_state_disable_enable */

/*
 * iol get drop-16-collision */
cparser_result_t
cparser_cmd_iol_get_drop_16_collision(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_enable_t enable;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if ((ret = reg_field_read(CFG_UNHIOLr, IOL_16DROPf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L2|MOD_DAL), "");
        return ret;
    }

    diag_util_mprintf("IOL drop after 16 collisions: %s\n", diagStr_enable[enable]);


    return CPARSER_OK;
}    /* end of cparser_cmd_iol_get_drop_16_collision */

/*
 * parser set field-seletor index <UINT:index> format ( default | raw | llc | arp | ipv4-header | ipv6-header | ip-payload | l4-payload ) offset <UINT:offset>
 */
cparser_result_t
cparser_cmd_parser_set_field_seletor_index_index_format_default_raw_llc_arp_ipv4_header_ipv6_header_ip_payload_l4_payload_offset_offset(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *offset_ptr)
{
    int32           ret = RT_ERR_FAILED;
    apollo_raw_parserFieldSelctorMode_t mode;
    
    DIAG_UTIL_PARAM_CHK();  

    if ('d' == TOKEN_CHAR(6,0))
    {
        mode = RAW_FIELDSELMODE_DEFAULT;
    }
    else if ('r' == TOKEN_CHAR(6,0))
    {
        mode = RAW_FIELDSELMODE_RAW;
    }
    else if ('c' == TOKEN_CHAR(6,2))
    {
        mode = RAW_FIELDSELMODE_LLC;
    }
    else if ('a' == TOKEN_CHAR(6,0))
    {
        mode = RAW_FIELDSELMODE_ARP;
    }
    else if ('4' == TOKEN_CHAR(6,1))
    {
        mode = RAW_FIELDSELMODE_L4PAYLOAD;
    }
    else if ('4' == TOKEN_CHAR(6,3))
    {
        mode = RAW_FIELDSELMODE_IPV4;
    }
    else if ('6' == TOKEN_CHAR(6,3))
    {
        mode = RAW_FIELDSELMODE_IPV6;
    }
    else if ('p' == TOKEN_CHAR(6,3))
    {
        mode = RAW_FIELDSELMODE_IPPAYLOAD;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_switch_parserFieldSelector_set(*index_ptr, mode, *offset_ptr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_parser_set_field_seletor_index_index_format_default_raw_llc_arp_ipv4_header_ipv6_header_ip_payload_l4_payload_offset_offset */

/*
 * parser get field-seletor index <UINT:index> 
 */
cparser_result_t
cparser_cmd_parser_get_field_seletor_index_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32           ret = RT_ERR_FAILED;
    apollo_raw_parserFieldSelctorMode_t mode;
    uint32 offset;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    
    DIAG_UTIL_ERR_CHK(apollo_raw_switch_parserFieldSelector_get(*index_ptr, &mode, &offset), ret);


    diag_util_mprintf("Index Offset Mode\n");
    diag_util_mprintf("%-5d %-6d %s\n",*index_ptr, offset,  diagStr_selectorMode[mode]);

    return CPARSER_OK;
}    /* end of cparser_cmd_parser_get_field_seletor_index_index */




