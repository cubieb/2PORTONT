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
 * Purpose : Definition those Security command and APIs in the SDK diagnostic shell.
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
#include <rtk/sec.h>
#include <diag_str.h>
#include <dal/apollo/raw/apollo_raw_sec.h>



int8 * _diag_secGetThresholdName(apollo_raw_dos_flood_t flood)
{
    static int8 name[20];
	switch(flood)
	{
        case SYNFLOOD_DENY: osal_strcpy(name,"SYN flood thershold");break;
        case FINFLOOD_DENY: osal_strcpy(name,"FIN flood thershold");break;
        case ICMPFLOOD_DENY: osal_strcpy(name,"ICMP flood thershold");break;
	}
	return name;
}

int8 * _diag_secGetTypeName(rtk_sec_attackType_t type)
{
    static int8 name[30];
	switch(type)
	{
        case SYNFIN_DENY: osal_strcpy(name,"SYN FIN Deny");break;
        case XMA_DENY: osal_strcpy(name,"XMA Deny");break;
        case NULLSCAN_DENY: osal_strcpy(name,"NULLSCAN Deny");break;
        case SYN_SPORTL1024_DENY: osal_strcpy(name,"SYN Sport Less 1024 Deny");break;
        case TCPHDR_MIN_CHECK: osal_strcpy(name,"TCP Header Short");break;
        case ICMP_FRAG_PKTS_DENY: osal_strcpy(name,"ICMP Fragment Deny");break;
        case POD_DENY: osal_strcpy(name,"POD Deny");break;
        case BLAT_DENY: osal_strcpy(name,"Blat_Deny");break;
        case LAND_DENY: osal_strcpy(name,"LAND deny");break;
        case DAEQSA_DENY: osal_strcpy(name,"DAEQSA Deny");break;
        case TCP_FRAG_OFF_MIN_CHECK: osal_strcpy(name,"TCP Fragment Error");break;
        case UDPDOMB_DENY: osal_strcpy(name,"UDP Domb Deny");break;
        case SYNWITHDATA_DENY: osal_strcpy(name,"SYN With Data Deny");break;
        case SYNFLOOD_DENY: osal_strcpy(name,"SYN Flood Deny");break;
        case FINFLOOD_DENY: osal_strcpy(name,"FIN Flood Deny");break;
        case ICMPFLOOD_DENY: osal_strcpy(name,"ICMP Flood Deny");break;
	}
	return name;
}

/*
 * security get attack-prevent action
 */
cparser_result_t
cparser_cmd_security_get_attack_prevent_action(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_action_t action;
    uint32_t type;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Type                       Action\n");
    for (type = 0; type < ATTACK_TYPE_END; type++)
    {
        if ((ret = apollo_raw_sec_attackPreventAction_get(type, &action)) == RT_ERR_OK)
        {
            diag_util_printf("%-25s: ", _diag_secGetTypeName(type));
            diag_util_mprintf("%-8s\n", diagStr_actionStr[action]);
        }
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_security_get_attack_prevent_action */

/*
 * security get attack-prevent ( daeqsa-deny | land-deny | blat-deny | synfin-deny | xma-deny | nullscan-deny | tcphdr-min-check | syn-sportl1024-deny ) action
 */
cparser_result_t
cparser_cmd_security_get_attack_prevent_daeqsa_deny_land_deny_blat_deny_synfin_deny_xma_deny_nullscan_deny_tcphdr_min_check_syn_sportl1024_deny_action(
    cparser_context_t *context)
{
    int32 ret;
    rtk_action_t action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if ('d' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventAction_get(DAEQSA_DENY, &action), ret);
        diag_util_printf("%s: ", _diag_secGetTypeName(DAEQSA_DENY));
        diag_util_mprintf("%-8s\n", diagStr_actionStr[action]);
    }
    else if ('l' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventAction_get(LAND_DENY, &action), ret);
        diag_util_printf("%s: ", _diag_secGetTypeName(LAND_DENY));
        diag_util_mprintf("%-8s\n", diagStr_actionStr[action]);
    }
    else if ('b' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventAction_get(BLAT_DENY, &action), ret);
        diag_util_printf("%s: ", _diag_secGetTypeName(BLAT_DENY));
        diag_util_mprintf("%-8s\n", diagStr_actionStr[action]);
    }
    else if ('x' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventAction_get(XMA_DENY, &action), ret);
        diag_util_printf("%s: ", _diag_secGetTypeName(XMA_DENY));
        diag_util_mprintf("%-8s\n", diagStr_actionStr[action]);
    }
    else if ('n' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventAction_get(NULLSCAN_DENY, &action), ret);
        diag_util_printf("%s: ", _diag_secGetTypeName(NULLSCAN_DENY));
        diag_util_mprintf("%-8s\n", diagStr_actionStr[action]);
    }
    else if ('t' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventAction_get(TCPHDR_MIN_CHECK, &action), ret);
        diag_util_printf("%s: ", _diag_secGetTypeName(TCPHDR_MIN_CHECK));
        diag_util_mprintf("%-8s\n", diagStr_actionStr[action]);
    }
    else if ('s' == TOKEN_CHAR(3,0))
    {
        if ('f' == TOKEN_CHAR(3,3))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventAction_get(SYNFIN_DENY, &action), ret);
            diag_util_printf("%s: ", _diag_secGetTypeName(SYNFIN_DENY));
            diag_util_mprintf("%-8s\n", diagStr_actionStr[action]);
        }
        else if ('-' == TOKEN_CHAR(3,3))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventAction_get(SYN_SPORTL1024_DENY, &action), ret);
            diag_util_printf("%s: ", _diag_secGetTypeName(SYN_SPORTL1024_DENY));
            diag_util_mprintf("%-8s\n", diagStr_actionStr[action]);
        }
        return CPARSER_ERR_INVALID_PARAMS;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    diag_util_printf("\n");
    return CPARSER_OK;
}    /* end of cparser_cmd_security_get_attack_prevent_daeqsa_deny_land_deny_blat_deny_synfin_deny_xma_deny_nullscan_deny_tcphdr_min_check_syn_sportl1024_deny_action */

/*
 * security get attack-prevent ( daeqsa-deny | land-deny | blat-deny | synfin-deny | xma-deny | nullscan-deny | tcphdr-min-check | syn-sportl1024-deny ) state
 */
cparser_result_t
cparser_cmd_security_get_attack_prevent_daeqsa_deny_land_deny_blat_deny_synfin_deny_xma_deny_nullscan_deny_tcphdr_min_check_syn_sportl1024_deny_state(
    cparser_context_t *context)
{
    int32 ret;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if ('d' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventEnable_get(DAEQSA_DENY, &enable), ret);
        diag_util_printf("%s: ", _diag_secGetTypeName(DAEQSA_DENY));
        diag_util_mprintf("%s\n", diagStr_enable[enable]);
    }
    else if ('b' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventEnable_get(BLAT_DENY, &enable), ret);
        diag_util_printf("%s: ",  _diag_secGetTypeName(BLAT_DENY));
        diag_util_mprintf("%s\n", diagStr_enable[enable]);
    }
    else if ('l' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventEnable_get(LAND_DENY, &enable), ret);
        diag_util_printf("%s: ",  _diag_secGetTypeName(LAND_DENY));
        diag_util_mprintf("%s\n", diagStr_enable[enable]);
    }
    else if ('x' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventEnable_get(XMA_DENY, &enable), ret);
        diag_util_printf("%s: ",  _diag_secGetTypeName(XMA_DENY));
        diag_util_mprintf("%s\n", diagStr_enable[enable]);
    }
    else if ('n' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventEnable_get(NULLSCAN_DENY, &enable), ret);
        diag_util_printf("%s: ",  _diag_secGetTypeName(NULLSCAN_DENY));
        diag_util_mprintf("%s\n", diagStr_enable[enable]);
    }
    else if ('t' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventEnable_get(TCPHDR_MIN_CHECK, &enable), ret);
        diag_util_printf("%s: ",  _diag_secGetTypeName(TCPHDR_MIN_CHECK));
        diag_util_mprintf("%s\n", diagStr_enable[enable]);
    }
    else if ('s' == TOKEN_CHAR(3,0))
    {
        if ('f' == TOKEN_CHAR(3,3))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventEnable_get(SYNFIN_DENY, &enable), ret);
            diag_util_printf("%s: ",  _diag_secGetTypeName(SYNFIN_DENY));
            diag_util_mprintf("%s\n", diagStr_enable[enable]);
        }
        else if ('-' == TOKEN_CHAR(3,3))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventEnable_get(SYN_SPORTL1024_DENY, &enable), ret);
            diag_util_printf("%s: ", _diag_secGetTypeName(SYN_SPORTL1024_DENY));
            diag_util_mprintf("%s\n", diagStr_enable[enable]);
        }
        return CPARSER_ERR_INVALID_PARAMS;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    diag_util_printf("\n");
    return CPARSER_OK;
}    /* end of cparser_cmd_security_get_attack_prevent_daeqsa_deny_land_deny_blat_deny_synfin_deny_xma_deny_nullscan_deny_tcphdr_min_check_syn_sportl1024_deny_state */

/*
 * security get attack-prevent dsl ( <PORT_LIST:dsl> | dsl_all ) state
 */
cparser_result_t
cparser_cmd_security_get_attack_prevent_dsl_dsl_dsl_all_state(
    cparser_context_t *context,
    char * *dsl_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t dsllist;
    rtk_port_t dsl = 0;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_DSLLIST(dsllist, 4), ret);

    diag_util_mprintf("Dsl State\n");
    DIAG_UTIL_PORTMASK_SCAN(dsllist, dsl)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_dslvcAttackPreventEnable_get(dsl, &enable), ret);
        diag_util_mprintf("%-4d %s\n", dsl, diagStr_enable[enable]);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_security_get_attack_prevent_dsl_dsl_dsl_all_state */

/*
 * security get attack-prevent port ( <PORT_LIST:port> | all ) state
 */
cparser_result_t
cparser_cmd_security_get_attack_prevent_port_port_all_state(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port State\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_portAttackPreventEnable_get(port, &enable), ret);
        diag_util_mprintf("%-4d %s\n", port, diagStr_enable[enable]);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_security_get_attack_prevent_port_port_all_state */

/*
 * security get attack-prevent state
 */
cparser_result_t
cparser_cmd_security_get_attack_prevent_state(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t enable;
    uint32_t type;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Type                       State\n");
    for (type = 0; type < ATTACK_TYPE_END; type++)
    {
        if ((ret = apollo_raw_sec_attackPreventEnable_get(type, &enable)) == RT_ERR_OK)
        {
            diag_util_printf("%-25s: ", _diag_secGetTypeName(type));
            diag_util_mprintf("%-8s\n", diagStr_enable[enable]);
        }
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_security_get_attack_prevent_state */

/*
 * security get attack-prevent ( syn-flood | fin-flood | icmp-flood ) action
 */
cparser_result_t
cparser_cmd_security_get_attack_prevent_syn_flood_fin_flood_icmp_flood_action(
    cparser_context_t *context)
{
    int32 ret;
    rtk_action_t action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if ('s' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventAction_get(SYNFLOOD_DENY, &action), ret);
        diag_util_printf("%s: ", _diag_secGetTypeName(SYNFLOOD_DENY));
        diag_util_mprintf("%-8s\n", diagStr_actionStr[action]);
    }
    else if ('f' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventAction_get(FINFLOOD_DENY, &action), ret);
        diag_util_printf("%s: ", _diag_secGetTypeName(FINFLOOD_DENY));
        diag_util_mprintf("%-8s\n", diagStr_actionStr[action]);
    }
    else if ('i' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventAction_get(ICMPFLOOD_DENY, &action), ret);
        diag_util_printf("%s: ", _diag_secGetTypeName(ICMPFLOOD_DENY));
        diag_util_mprintf("%-8s\n", diagStr_actionStr[action]);
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    diag_util_printf("\n");
    return CPARSER_OK;
}    /* end of cparser_cmd_security_get_attack_prevent_syn_flood_fin_flood_icmp_flood_action */

/*
 * security get attack-prevent ( syn-flood | fin-flood | icmp-flood ) state
 */
cparser_result_t
cparser_cmd_security_get_attack_prevent_syn_flood_fin_flood_icmp_flood_state(
    cparser_context_t *context)
{
    int32 ret;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if ('s' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventEnable_get(SYNFLOOD_DENY, &enable), ret);
        diag_util_printf("%s: ", _diag_secGetTypeName(SYNFLOOD_DENY));
        diag_util_mprintf("%s\n", diagStr_enable[enable]);
    }
    else if ('f' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventEnable_get(FINFLOOD_DENY, &enable), ret);
        diag_util_printf("%s: ", _diag_secGetTypeName(FINFLOOD_DENY));
        diag_util_mprintf("%s\n", diagStr_enable[enable]);
    }
    else if ('i' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventEnable_get(ICMPFLOOD_DENY, &enable), ret);
        diag_util_printf("%s: ", _diag_secGetTypeName(ICMPFLOOD_DENY));
        diag_util_mprintf("%s\n", diagStr_enable[enable]);
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    diag_util_printf("\n");
    return CPARSER_OK;
}    /* end of cparser_cmd_security_get_attack_prevent_syn_flood_fin_flood_icmp_flood_state */

/*
 * security get attack-prevent ( syn-flood | fin-flood | icmp-flood ) threshold
 */
cparser_result_t
cparser_cmd_security_get_attack_prevent_syn_flood_fin_flood_icmp_flood_threshold(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 threshold;
    
    if ('s' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_floodThreshold_get(SYNFLOOD_DENY, &threshold), ret);
        diag_util_mprintf("%s: %d\n", _diag_secGetThresholdName(SYNFLOOD_DENY), threshold);
    }
    else if ('f' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_floodThreshold_get(FINFLOOD_DENY, &threshold), ret);
        diag_util_mprintf("%s: %d\n", _diag_secGetThresholdName(FINFLOOD_DENY), threshold);
    }
    else if ('i' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_floodThreshold_get(ICMPFLOOD_DENY, &threshold), ret);
        diag_util_mprintf("%s: %d\n", _diag_secGetThresholdName(ICMPFLOOD_DENY), threshold);
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    
    return CPARSER_OK;
}    /* end of cparser_cmd_security_get_attack_prevent_syn_flood_fin_flood_icmp_flood_threshold */

/*
 * security get attack-prevent ( tcp-frag-off-min-check | icmp-frag-pkts-deny | pod-deny | udp-bomb | syn-with-data ) action
 */
cparser_result_t
cparser_cmd_security_get_attack_prevent_tcp_frag_off_min_check_icmp_frag_pkts_deny_pod_deny_udp_bomb_syn_with_data_action(
    cparser_context_t *context)
{
    int32 ret;
    rtk_action_t action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if ('t' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventAction_get(TCP_FRAG_OFF_MIN_CHECK, &action), ret);
        diag_util_printf("%s: ",_diag_secGetTypeName(TCP_FRAG_OFF_MIN_CHECK));
        diag_util_mprintf("%-8s\n", diagStr_actionStr[action]);
    }
    else if ('i' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventAction_get(ICMP_FRAG_PKTS_DENY, &action), ret);
        diag_util_printf("%s: ", _diag_secGetTypeName(ICMP_FRAG_PKTS_DENY));
        diag_util_mprintf("%-8s\n", diagStr_actionStr[action]);
    }
    else if ('p' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventAction_get(POD_DENY, &action), ret);
        diag_util_printf("%s: ", _diag_secGetTypeName(POD_DENY));
        diag_util_mprintf("%-8s\n", diagStr_actionStr[action]);
    }
    else if ('u' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventAction_get(UDPDOMB_DENY, &action), ret);
        diag_util_printf("%s: ", _diag_secGetTypeName(UDPDOMB_DENY));
        diag_util_mprintf("%-8s\n", diagStr_actionStr[action]);
    }
    else if ('s' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventAction_get(SYNWITHDATA_DENY, &action), ret);
        diag_util_printf("%s: ", _diag_secGetTypeName(SYNWITHDATA_DENY));
        diag_util_mprintf("%-8s\n", diagStr_actionStr[action]);
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    diag_util_printf("\n");
    return CPARSER_OK;
}    /* end of cparser_cmd_security_get_attack_prevent_tcp_frag_off_min_check_icmp_frag_pkts_deny_pod_deny_udp_bomb_syn_with_data_action */

/*
 * security get attack-prevent ( tcp-frag-off-min-check | icmp-frag-pkts-deny | pod-deny | udp-bomb | syn-with-data ) state
 */
cparser_result_t
cparser_cmd_security_get_attack_prevent_tcp_frag_off_min_check_icmp_frag_pkts_deny_pod_deny_udp_bomb_syn_with_data_state(
    cparser_context_t *context)
{
    int32 ret;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if ('t' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventEnable_get(TCP_FRAG_OFF_MIN_CHECK, &enable), ret);
        diag_util_printf("%s: ", _diag_secGetTypeName(TCP_FRAG_OFF_MIN_CHECK));
        diag_util_mprintf("%s\n", diagStr_enable[enable]);
    }
    else if ('i' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventEnable_get(ICMP_FRAG_PKTS_DENY, &enable), ret);
        diag_util_printf("%s: ", _diag_secGetTypeName(ICMP_FRAG_PKTS_DENY));
        diag_util_mprintf("%s\n", diagStr_enable[enable]);
    }
    else if ('p' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventEnable_get(POD_DENY, &enable), ret);
        diag_util_printf("%s: ", _diag_secGetTypeName(POD_DENY));
        diag_util_mprintf("%s\n", diagStr_enable[enable]);
    }
    else if ('u' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventEnable_get(UDPDOMB_DENY, &enable), ret);
        diag_util_printf("%s: ", _diag_secGetTypeName(UDPDOMB_DENY));
        diag_util_mprintf("%s\n", diagStr_enable[enable]);
    }
    else if ('s' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventEnable_get(SYNWITHDATA_DENY, &enable), ret);
        diag_util_printf("%s: ", _diag_secGetTypeName(SYNWITHDATA_DENY));
        diag_util_mprintf("%s\n", diagStr_enable[enable]);
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    diag_util_printf("\n");
    return CPARSER_OK;
}    /* end of cparser_cmd_security_get_attack_prevent_tcp_frag_off_min_check_icmp_frag_pkts_deny_pod_deny_udp_bomb_syn_with_data_state */

/*
 * security set attack-prevent ( daeqsa-deny | land-deny | blat-deny | synfin-deny | xma-deny | nullscan-deny | tcphdr-min-check | syn-sportl1024-deny ) action ( drop | trap-to-cpu )
 */
cparser_result_t
cparser_cmd_security_set_attack_prevent_daeqsa_deny_land_deny_blat_deny_synfin_deny_xma_deny_nullscan_deny_tcphdr_min_check_syn_sportl1024_deny_action_drop_trap_to_cpu(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_action_t action;
    rtk_sec_attackType_t attackType;

    DIAG_UTIL_PARAM_CHK();

    if ('d' == TOKEN_CHAR(5,0))
    {
        action = ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(5,0))
    {
        action = ACTION_TRAP2CPU;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    if ('d' == TOKEN_CHAR(3,0))
    {
        attackType = DAEQSA_DENY;
    }
    else if ('l' == TOKEN_CHAR(3,0))
    {
        attackType = LAND_DENY;
    }
    else if ('b' == TOKEN_CHAR(3,0))
    {
        attackType = BLAT_DENY;
    }
    else if ('t' == TOKEN_CHAR(3,0))
    {
        attackType = TCPHDR_MIN_CHECK;
    }
    else if ('s' == TOKEN_CHAR(3,0))
    {
        if ('f' == TOKEN_CHAR(3,3))
        {
            attackType = SYNFIN_DENY;
        }
        else if ('-' == TOKEN_CHAR(3,3))
        {
            attackType = SYN_SPORTL1024_DENY;
        }
        else
            return CPARSER_ERR_INVALID_PARAMS;
    }
    else if ('x' == TOKEN_CHAR(3,0))
    {
        attackType = XMA_DENY;
    }
    else if ('n' == TOKEN_CHAR(3,0))
    {
        attackType = NULLSCAN_DENY;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventAction_set(attackType, action), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_security_set_attack_prevent_daeqsa_deny_land_deny_blat_deny_synfin_deny_xma_deny_nullscan_deny_tcphdr_min_check_syn_sportl1024_deny_action_drop_trap_to_cpu */

/*
 * security set attack-prevent ( daeqsa-deny | land-deny | blat-deny | synfin-deny | xma-deny | nullscan-deny | tcphdr-min-check | syn-sportl1024-deny ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_security_set_attack_prevent_daeqsa_deny_land_deny_blat_deny_synfin_deny_xma_deny_nullscan_deny_tcphdr_min_check_syn_sportl1024_deny_state_disable_enable(
    cparser_context_t *context)
{
    rtk_enable_t enable;
    int32     ret = RT_ERR_FAILED;
    rtk_sec_attackType_t attackType;

    DIAG_UTIL_PARAM_CHK();

    if ('d' == TOKEN_CHAR(5,0))
    {
        enable = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(5,0))
    {
        enable = ENABLED;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    if ('d' == TOKEN_CHAR(3,0))
    {
        attackType = DAEQSA_DENY;
    }
    else if ('l' == TOKEN_CHAR(3,0))
    {
        attackType = LAND_DENY;
    }
    else if ('b' == TOKEN_CHAR(3,0))
    {
        attackType = BLAT_DENY;
    }
    else if ('t' == TOKEN_CHAR(3,0))
    {
        attackType = TCPHDR_MIN_CHECK;
    }
    else if ('s' == TOKEN_CHAR(3,0))
    {
        if ('f' == TOKEN_CHAR(3,3))
        {
            attackType = SYNFIN_DENY;
        }
        else if ('-' == TOKEN_CHAR(3,3))
        {
            attackType = SYN_SPORTL1024_DENY;
        }
        else
            return CPARSER_ERR_INVALID_PARAMS;
    }
    else if ('x' == TOKEN_CHAR(3,0))
    {
        attackType = XMA_DENY;
    }
    else if ('n' == TOKEN_CHAR(3,0))
    {
        attackType = NULLSCAN_DENY;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventEnable_set(attackType, enable), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_security_set_attack_prevent_daeqsa_deny_land_deny_blat_deny_synfin_deny_xma_deny_nullscan_deny_tcphdr_min_check_syn_sportl1024_deny_state_disable_enable */

/*
 * security set attack-prevent dsl ( <PORT_LIST:dsl> | dsl_all ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_security_set_attack_prevent_dsl_dsl_dsl_all_state_disable_enable(
    cparser_context_t *context,
    char * *dsl_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t dsllist;
    rtk_port_t dsl;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_DSLLIST(dsllist, 4), ret);

    if ('e' == TOKEN_CHAR(6,0))
        enable = ENABLED;
    else
        enable = DISABLED;

    DIAG_UTIL_PORTMASK_SCAN(dsllist, dsl)
    {
        if ((ret = apollo_raw_sec_dslvcAttackPreventEnable_set(dsl,enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_security_set_attack_prevent_dsl_dsl_dsl_all_state_disable_enable */

/*
 * security set attack-prevent port ( <PORT_LIST:port> | all ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_security_set_attack_prevent_port_port_all_state_disable_enable(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('e' == TOKEN_CHAR(6,0))
        enable = ENABLED;
    else
        enable = DISABLED;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = apollo_raw_sec_portAttackPreventEnable_set(port,enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_security_set_attack_prevent_port_port_all_state_disable_enable */

/*
 * security set attack-prevent ( syn-flood | fin-flood | icmp-flood ) action ( drop | trap-to-cpu )
 */
cparser_result_t
cparser_cmd_security_set_attack_prevent_syn_flood_fin_flood_icmp_flood_action_drop_trap_to_cpu(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_action_t action;
    rtk_sec_attackType_t attackType;

    DIAG_UTIL_PARAM_CHK();

    if ('d' == TOKEN_CHAR(5,0))
    {
        action = ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(5,0))
    {
        action = ACTION_TRAP2CPU;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    if ('s' == TOKEN_CHAR(3,0))
    {
        attackType = SYNFLOOD_DENY;
    }
    else if ('f' == TOKEN_CHAR(3,0))
    {
        attackType = FINFLOOD_DENY;
    }
    else if ('i' == TOKEN_CHAR(3,0))
    {
        attackType = ICMPFLOOD_DENY;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventAction_set(attackType, action), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_security_set_attack_prevent_syn_flood_fin_flood_icmp_flood_action_drop_trap_to_cpu */

/*
 * security set attack-prevent ( syn-flood | fin-flood | icmp-flood ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_security_set_attack_prevent_syn_flood_fin_flood_icmp_flood_state_disable_enable(
    cparser_context_t *context)
{
    rtk_enable_t enable;
    int32     ret = RT_ERR_FAILED;
    rtk_sec_attackType_t attackType;

    DIAG_UTIL_PARAM_CHK();

    if ('d' == TOKEN_CHAR(5,0))
    {
        enable = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(5,0))
    {
        enable = ENABLED;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    if ('s' == TOKEN_CHAR(3,0))
    {
        attackType = SYNFLOOD_DENY;
    }
    else if ('f' == TOKEN_CHAR(3,0))
    {
        attackType = FINFLOOD_DENY;
    }
    else if ('i' == TOKEN_CHAR(3,0))
    {
        attackType = ICMPFLOOD_DENY;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventEnable_set(attackType, enable), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_security_set_attack_prevent_syn_flood_fin_flood_icmp_flood_state_disable_enable */

/*
 * security set attack-prevent ( syn-flood | fin-flood | icmp-flood ) threshold <UINT:th>
 */
cparser_result_t
cparser_cmd_security_set_attack_prevent_syn_flood_fin_flood_icmp_flood_threshold_th(
    cparser_context_t *context,
    uint32_t  *th_ptr)
{
    int32 ret = RT_ERR_FAILED;
    uint32 threshold;
    
    if ('s' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_floodThreshold_set(SYNFLOOD_DENY, *th_ptr), ret);
    }
    else if ('f' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_floodThreshold_set(FINFLOOD_DENY, *th_ptr), ret);
    }
    else if ('i' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_sec_floodThreshold_set(ICMPFLOOD_DENY, *th_ptr), ret);
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    return CPARSER_OK;
}    /* end of cparser_cmd_security_set_attack_prevent_syn_flood_fin_flood_icmp_flood_threshold_th */

/*
 * security set attack-prevent ( tcp-frag-off-min-check | icmp-frag-pkts-deny | pod-deny | udp-bomb | syn-with-data ) action ( drop | trap-to-cpu )
 */
cparser_result_t
cparser_cmd_security_set_attack_prevent_tcp_frag_off_min_check_icmp_frag_pkts_deny_pod_deny_udp_bomb_syn_with_data_action_drop_trap_to_cpu(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_action_t action;
    rtk_sec_attackType_t attackType;

    DIAG_UTIL_PARAM_CHK();

    if ('d' == TOKEN_CHAR(5,0))
    {
        action = ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(5,0))
    {
        action = ACTION_TRAP2CPU;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    if ('t' == TOKEN_CHAR(3,0))
    {
        attackType = TCP_FRAG_OFF_MIN_CHECK;
    }
    else if ('i' == TOKEN_CHAR(3,0))
    {
        attackType = ICMP_FRAG_PKTS_DENY;
    }
    else if ('p' == TOKEN_CHAR(3,0))
    {
        attackType = POD_DENY;
    }
    else if ('u' == TOKEN_CHAR(3,0))
    {
        attackType = UDPDOMB_DENY;
    }
    else if ('s' == TOKEN_CHAR(3,0))
    {
        attackType = SYNWITHDATA_DENY;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventAction_set(attackType, action), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_security_set_attack_prevent_tcp_frag_off_min_check_icmp_frag_pkts_deny_pod_deny_udp_bomb_syn_with_data_action_drop_trap_to_cpu */

/*
 * security set attack-prevent ( tcp-frag-off-min-check | icmp-frag-pkts-deny | pod-deny | udp-bomb | syn-with-data ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_security_set_attack_prevent_tcp_frag_off_min_check_icmp_frag_pkts_deny_pod_deny_udp_bomb_syn_with_data_state_disable_enable(
    cparser_context_t *context)
{
    rtk_enable_t enable;
    int32     ret = RT_ERR_FAILED;
    rtk_sec_attackType_t attackType;

    DIAG_UTIL_PARAM_CHK();

    if ('d' == TOKEN_CHAR(5,0))
    {
        enable = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(5,0))
    {
        enable = ENABLED;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    if ('t' == TOKEN_CHAR(3,0))
    {
        attackType = TCP_FRAG_OFF_MIN_CHECK;
    }
    else if ('i' == TOKEN_CHAR(3,0))
    {
        attackType = ICMP_FRAG_PKTS_DENY;
    }
    else if ('p' == TOKEN_CHAR(3,0))
    {
        attackType = POD_DENY;
    }
    else if ('u' == TOKEN_CHAR(3,0))
    {
        attackType = UDPDOMB_DENY;
    }
    else if ('s' == TOKEN_CHAR(3,0))
    {
        attackType = SYNWITHDATA_DENY;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_ERR_CHK(apollo_raw_sec_attackPreventEnable_set(attackType, enable), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_security_set_attack_prevent_tcp_frag_off_min_check_icmp_frag_pkts_deny_pod_deny_udp_bomb_syn_with_data_state_disable_enable */

