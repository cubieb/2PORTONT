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
 * Purpose : Definition those Trunk command and APIs in the SDK diagnostic shell.
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

#include <dal/apollo/raw/apollo_raw_trunk.h>

/*
 * trunk dump
 */
cparser_result_t
cparser_cmd_trunk_dump(
    cparser_context_t *context)
{
    int32 ret;
    rtk_portmask_t portmask;
    raw_trunk_mode_t mode;
    uint8 portStr[20];
    raw_trunk_floodMode_t flood_mode;
    uint32 hashsel;
    uint32 algorithm;
    raw_trunk_port_t port;
    uint32 hash_val;
    rtk_enable_t state;
    rtk_portmask_t empty_portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_trunk_memberPort_get(&portmask), ret);
    DIAG_UTIL_ERR_CHK(diag_util_lPortMask2str(portStr, &portmask), ret);
    diag_util_printf("\n Trunk portmask: %s", portStr);

    DIAG_UTIL_ERR_CHK(apollo_raw_trunk_hashAlgorithm_get(&hashsel), ret);
    if(0 == hashsel)
    {
        diag_util_printf("\n Distribute Algorithm: None");
    }
    else
    {
        diag_util_printf("\n Distribute Algorithm:\n");
        for(algorithm = 0; algorithm < RAW_TRUNK_ALGORITHM_NUM; algorithm++)
        {
            if(hashsel & (0x01 << algorithm))
                diag_util_printf("  %s\n", diagStr_trunkAlgorithm[algorithm]);
        }
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_trunk_flood_get(&flood_mode), ret);
    diag_util_printf("\n Flood Mode: %s", diagStr_trunkFloodMode[flood_mode]);

    DIAG_UTIL_ERR_CHK(apollo_raw_trunk_flowControl_get(&state), ret);
    diag_util_printf("\n Trunk Flow Contrl: %s", diagStr_enable[state]);

    DIAG_UTIL_ERR_CHK(apollo_raw_port_queueEmpty_get(&empty_portmask), ret);
    DIAG_UTIL_ERR_CHK(diag_util_lPortMask2str(portStr, &empty_portmask), ret);

    diag_util_printf("\n Queue Empty Port Mask: %s", portStr);

    DIAG_UTIL_ERR_CHK(apollo_raw_trunk_mode_get(&mode), ret);
    diag_util_printf("\n Trunk Mode: %s", diagStr_trunkMode[mode]);

    for(hash_val = 0; hash_val <= RAW_TRUNK_HASH_VALUE_MAX; hash_val++)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_trunk_hashMapping_get(hash_val, &port), ret);
        diag_util_printf("\n Hash Value %2d to Port %d in trunk group", hash_val, port);
    }

    diag_util_printf("\n\n");
    return CPARSER_OK;

}    /* end of cparser_cmd_trunk_dump */

/*
 * trunk get distribute-algorithm
 */
cparser_result_t
cparser_cmd_trunk_get_distribute_algorithm(
    cparser_context_t *context)
{
    int32 ret;
    uint32 hashsel;
    uint32 algorithm;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_trunk_hashAlgorithm_get(&hashsel), ret);
    if(0 == hashsel)
    {
        diag_util_printf("\n Distribute Algorithm: None\n");
    }
    else
    {
        diag_util_printf("\n Distribute Algorithm:\n");
        for(algorithm = 0; algorithm < RAW_TRUNK_ALGORITHM_NUM; algorithm++)
        {
            if(hashsel & (0x01 << algorithm))
                diag_util_printf("  %s\n", diagStr_trunkAlgorithm[algorithm]);
        }
    }

    diag_util_printf("\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_trunk_get_distribute_algorithm */

/*
 * trunk get flood-mode
 */
cparser_result_t
cparser_cmd_trunk_get_flood_mode(
    cparser_context_t *context)
{
    int32 ret;
    raw_trunk_floodMode_t mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_trunk_flood_get(&mode), ret);
    diag_util_printf("\n Flood Mode: %s\n\n", diagStr_trunkFloodMode[mode]);

    return CPARSER_OK;
}    /* end of cparser_cmd_trunk_get_flood_mode */

/*
 * trunk get flow-control state
 */
cparser_result_t
cparser_cmd_trunk_get_flow_control_state(
    cparser_context_t *context)
{
    int32 ret;
    rtk_enable_t state;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_trunk_flowControl_get(&state), ret);
    diag_util_mprintf("\nTrunk Flow Contrl: %s\n\n", diagStr_enable[state]);

    return CPARSER_OK;
}    /* end of cparser_cmd_trunk_get_flow_control_state */


/*
 * trunk get hash-mapping hash_value all
 */
cparser_result_t
cparser_cmd_trunk_get_hash_mapping_hash_value_all(
    cparser_context_t *context)
{
    int32 ret;
    raw_trunk_port_t port;
    uint32 hash_value;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    for(hash_value = 0; hash_value <= RAW_TRUNK_HASH_VALUE_MAX; hash_value++)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_trunk_hashMapping_get(hash_value, &port), ret);
        diag_util_printf("\n Hash Value %d to Port %d in trunk group", hash_value, port);
    }

    diag_util_printf("\n\n");
    return CPARSER_OK;
}    /* end of cparser_cmd_trunk_get_hash_mapping_hash_value_all */

/*
 * trunk get hash-mapping hash_value <UINT:hash_value>
 */
cparser_result_t
cparser_cmd_trunk_get_hash_mapping_hash_value_hash_value(
    cparser_context_t *context,
    uint32_t  *hash_value_ptr)
{
    int32 ret;
    raw_trunk_port_t port;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_PARAM_RANGE_CHK( (*hash_value_ptr > RAW_TRUNK_HASH_VALUE_MAX), ret);

    DIAG_UTIL_ERR_CHK(apollo_raw_trunk_hashMapping_get((uint32)*hash_value_ptr, &port), ret);
    diag_util_printf("\n Hash Value %d to Port %d in trunk group\n\n", *hash_value_ptr, port);

    return CPARSER_OK;
}    /* end of cparser_cmd_trunk_get_hash_mapping_hash_value_hash_value */

/*
 * trunk get member-port
 */
cparser_result_t
cparser_cmd_trunk_get_member_port(
    cparser_context_t *context)
{
    int32 ret;
    rtk_portmask_t portmask;
    uint8 portStr[20];

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_trunk_memberPort_get(&portmask), ret);
    DIAG_UTIL_ERR_CHK(diag_util_lPortMask2str(portStr, &portmask), ret);

    diag_util_printf("\n Trunk portmask: %s\n\n", portStr);

    return CPARSER_OK;
}    /* end of cparser_cmd_trunk_get_member_port */

/*
 * trunk get mode
 */
cparser_result_t
cparser_cmd_trunk_get_mode(
    cparser_context_t *context)
{
    int32 ret;
    raw_trunk_mode_t mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_trunk_mode_get(&mode), ret);

    diag_util_printf("Trunk Mode: %s\n\n", diagStr_trunkMode[mode]);
    return CPARSER_OK;
}    /* end of cparser_cmd_trunk_get_mode */

/*
 * trunk get queue-empty
 */
cparser_result_t
cparser_cmd_trunk_get_queue_empty(
    cparser_context_t *context)
{
    int32 ret;
    rtk_portmask_t empty_portmask;
    uint8 portStr[20];

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_port_queueEmpty_get(&empty_portmask), ret);
    DIAG_UTIL_ERR_CHK(diag_util_lPortMask2str(portStr, &empty_portmask), ret);

    diag_util_printf("\n Queue Empty Port Mask: %s\n\n", portStr);

    return CPARSER_OK;
}    /* end of cparser_cmd_trunk_get_queue_empty */

/*
 * trunk set distribute-algorithm { dst-ip } { dst-l4-port } { dst-mac } { src-ip } { src-l4-port } { src-mac } { src-port }
 */
cparser_result_t
cparser_cmd_trunk_set_distribute_algorithm_dst_ip_dst_l4_port_dst_mac_src_ip_src_l4_port_src_mac_src_port(
    cparser_context_t *context)
{
    int32 ret;
    uint32 token;
    uint32 hashsel = 0;

    DIAG_UTIL_PARAM_CHK();

    for(token = 3; token < TOKEN_NUM(); token++)
    {
        if ('d' == TOKEN_CHAR(token, 0))
        {
            if ('i' == TOKEN_CHAR(token, 4))
                hashsel |= RAW_TRUNK_HASH_DEST_IP;
            else if ('l' == TOKEN_CHAR(token, 4))
                hashsel |= RAW_TRUNK_HASH_DEST_L4PORT;
            else if ('m' == TOKEN_CHAR(token, 4))
                hashsel |= RAW_TRUNK_HASH_DEST_MAC;
            else
                return CPARSER_ERR_INVALID_PARAMS;
        }
        else if ('s' == TOKEN_CHAR(token, 0))
        {
            if ('i' == TOKEN_CHAR(token, 4))
                hashsel |= RAW_TRUNK_HASH_SOURCE_IP;
            else if ('l' == TOKEN_CHAR(token, 4))
                hashsel |= RAW_TRUNK_HASH_SOURCE_L4PORT;
            else if ('m' == TOKEN_CHAR(token, 4))
                hashsel |= RAW_TRUNK_HASH_SOURCE_MAC;
            else if ('p' == TOKEN_CHAR(token, 4))
                hashsel |= RAW_TRUNK_HASH_SOURCE_PORT;
            else
                return CPARSER_ERR_INVALID_PARAMS;
        }
        else
            return CPARSER_ERR_INVALID_PARAMS;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_trunk_hashAlgorithm_set(hashsel), ret);
    return CPARSER_OK;
}    /* end of cparser_cmd_trunk_set_distribute_algorithm_dst_ip_dst_l4_port_dst_mac_src_ip_src_l4_port_src_mac_src_port */

/*
 * trunk set flood-mode ( normal | to-1st-logic-port )
 */
cparser_result_t
cparser_cmd_trunk_set_flood_mode_normal_to_1st_logic_port(
    cparser_context_t *context)
{
    int32 ret;
    raw_trunk_floodMode_t mode;

    DIAG_UTIL_PARAM_CHK();

    if ('t' == TOKEN_CHAR(3, 0))
        mode = RAW_TRUNK_FWD_TO_FIRST_PORT;
    else if ('n' == TOKEN_CHAR(3, 0))
        mode = RAW_TRUNK_NORMAL_FLOOD;
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_ERR_CHK(apollo_raw_trunk_flood_set(mode), ret);
    return CPARSER_OK;
}    /* end of cparser_cmd_trunk_set_flood_mode_normal_to_1st_logic_port */

/*
 * trunk set flow-control state ( disable | enable )
 */
cparser_result_t
cparser_cmd_trunk_set_flow_control_state_disable_enable(
    cparser_context_t *context)
{
    int32 ret;
    rtk_enable_t state;

    DIAG_UTIL_PARAM_CHK();

    if ('e' == TOKEN_CHAR(4, 0))
        state = ENABLED;
    else if ('d' == TOKEN_CHAR(4, 0))
        state = DISABLED;
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_ERR_CHK(apollo_raw_trunk_flowControl_set(state), ret);
    return CPARSER_OK;
}    /* end of cparser_cmd_trunk_set_flow_control_state_disable_enable */

/*
 * trunk set hash-mapping hash-value <UINT:hash_value> port <UINT:port>
 */
cparser_result_t
cparser_cmd_trunk_set_hash_mapping_hash_value_hash_value_port_port(
    cparser_context_t *context,
    uint32_t  *hash_value_ptr,
    uint32_t  *port_ptr)
{
    int32 ret;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_PARAM_RANGE_CHK( (*hash_value_ptr > RAW_TRUNK_HASH_VALUE_MAX), ret);
    DIAG_UTIL_PARAM_RANGE_CHK( (*port_ptr >= RAW_TRUNK_PORT_END), ret);

    DIAG_UTIL_ERR_CHK(apollo_raw_trunk_hashMapping_set(*hash_value_ptr, (raw_trunk_port_t)*port_ptr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_trunk_set_hash_mapping_hash_value_hash_value_port_port */

/*
 * trunk set hash-mapping hash-value all port <UINT:port>
 */
cparser_result_t
cparser_cmd_trunk_set_hash_mapping_hash_value_all_port_port(
    cparser_context_t *context,
    uint32_t  *port_ptr)
{
    int32 ret;
    int32 hashValue;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK( (*port_ptr >= RAW_TRUNK_PORT_END), ret);

    for(hashValue = 0; hashValue <= RAW_TRUNK_HASH_VALUE_MAX; hashValue++)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_trunk_hashMapping_set(hashValue, (raw_trunk_port_t)*port_ptr), ret);
    }
    
    return CPARSER_OK;
}    /* end of cparser_cmd_trunk_set_hash_mapping_hash_value_all_port_port */

/*
 * trunk set member-port ( <PORT_LIST:ports> | none )
 */
cparser_result_t
cparser_cmd_trunk_set_member_port_ports_none(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_ERR_CHK(apollo_raw_trunk_memberPort_set(&(portlist.portmask)), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_trunk_set_member_port_ports_none */

/*
 * trunk set mode ( dumb | normal )
 */
cparser_result_t
cparser_cmd_trunk_set_mode_dumb_normal(
    cparser_context_t *context)
{
    int32 ret;
    raw_trunk_mode_t mode;

    DIAG_UTIL_PARAM_CHK();

    if ('d' == TOKEN_CHAR(3,0))
        mode = RAW_TRUNK_DUMB_MODE;
    else if ('n' == TOKEN_CHAR(3,0))
        mode = RAW_TRUNK_USER_MODE;
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_ERR_CHK(apollo_raw_trunk_mode_set(mode), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_trunk_set_mode_dumb_normal */


