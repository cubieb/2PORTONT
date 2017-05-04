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
 * Purpose : Define diag shell functions for dot1x.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) dot1x diag shell.
 */


#include <common/debug/rt_log.h>
#include <common/rt_error.h>
#include <common/rt_type.h>
#include <diag_util.h>
#include <diag_str.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>
#include <dal/apollo/raw/apollo_raw_qos.h>

/*
 * qos set priority-selector system port_weight <UINT:port_weight> dot1q_weight <UINT:dot1q_weight> dscp_weight <UINT:dscp_weight> acl_weight <UINT:acl_weight> cvlan_weight <UINT:cvlan_weight> lutfwd_weight <UINT:lutfwd_weight> sa_weight <UINT:sa_weight> svlan_weight <UINT:svlan_weight> l4_weight <UINT:l4_weight>
 */
cparser_result_t
cparser_cmd_qos_set_priority_selector_system_port_weight_port_weight_dot1q_weight_dot1q_weight_dscp_weight_dscp_weight_acl_weight_acl_weight_cvlan_weight_cvlan_weight_lutfwd_weight_lutfwd_weight_sa_weight_sa_weight_svlan_weight_svlan_weight_l4_weight_l4_weight(
    cparser_context_t *context,
    uint32_t  *port_weight_ptr,
    uint32_t  *dot1q_weight_ptr,
    uint32_t  *dscp_weight_ptr,
    uint32_t  *acl_weight_ptr,
    uint32_t  *cvlan_weight_ptr,
    uint32_t  *lutfwd_weight_ptr,
    uint32_t  *sa_weight_ptr,
    uint32_t  *svlan_weight_ptr,
    uint32_t  *l4_weight_ptr)
{
    int32 ret = RT_ERR_FAILED;
  
    DIAG_UTIL_PARAM_CHK();

    if((ret = apollo_raw_qos_priDecisionWeight_set(RAW_QOS_PRI_TYPE_SVLAN, *svlan_weight_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }       

    if((ret = apollo_raw_qos_priDecisionWeight_set(RAW_QOS_PRI_TYPE_SA, *sa_weight_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }       

    if((ret = apollo_raw_qos_priDecisionWeight_set(RAW_QOS_PRI_TYPE_LUTFWD, *lutfwd_weight_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }  

    if((ret = apollo_raw_qos_priDecisionWeight_set(RAW_QOS_PRI_TYPE_CVLAN, *cvlan_weight_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }  

    if((ret = apollo_raw_qos_priDecisionWeight_set(RAW_QOS_PRI_TYPE_ACL, *acl_weight_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 
    
    if((ret = apollo_raw_qos_priDecisionWeight_set(RAW_QOS_PRI_TYPE_DSCP, *dscp_weight_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 

    if((ret = apollo_raw_qos_priDecisionWeight_set(RAW_QOS_PRI_TYPE_DOT1Q, *dot1q_weight_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 

    if((ret = apollo_raw_qos_priDecisionWeight_set(RAW_QOS_PRI_TYPE_PORT, *port_weight_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 

    if((ret = apollo_raw_qos_priDecisionWeight_set(RAW_QOS_PRI_TYPE_L4, *l4_weight_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_priority_selector_system_port_weight_port_weight_dot1q_weight_dot1q_weight_dscp_weight_dscp_weight_acl_weight_acl_weight_cvlan_weight_cvlan_weight_lutfwd_weight_lutfwd_weight_sa_weight_sa_weight_svlan_weight_svlan_weight_l4_weight_l4_weight */



/*
 * qos set priority-selector system port <UINT:weights> 
 */
cparser_result_t
cparser_cmd_qos_set_priority_selector_system_port_weights(
    cparser_context_t *context,
    uint32_t  *weights_ptr)
{
    int32 ret = RT_ERR_FAILED;
  
    DIAG_UTIL_PARAM_CHK();

    if((ret = apollo_raw_qos_priDecisionWeight_set(RAW_QOS_PRI_TYPE_PORT, *weights_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 

    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_priority_selector_system_port_weight */

/*
 * qos set priority-selector system dot1q <UINT:weight> 
 */
cparser_result_t
cparser_cmd_qos_set_priority_selector_system_dot1q_weight(
    cparser_context_t *context,
    uint32_t  *weight_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if((ret = apollo_raw_qos_priDecisionWeight_set(RAW_QOS_PRI_TYPE_DOT1Q, *weight_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_priority_selector_system_dot1q_weight */

/*
 * qos set priority-selector system dscp <UINT:weight> 
 */
cparser_result_t
cparser_cmd_qos_set_priority_selector_system_dscp_weight(
    cparser_context_t *context,
    uint32_t  *weight_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if((ret = apollo_raw_qos_priDecisionWeight_set(RAW_QOS_PRI_TYPE_DSCP, *weight_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     
    
    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_priority_selector_system_dscp_weight */

/*
 * qos set priority-selector system acl <UINT:weight> 
 */
cparser_result_t
cparser_cmd_qos_set_priority_selector_system_acl_weight(
    cparser_context_t *context,
    uint32_t  *weight_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if((ret = apollo_raw_qos_priDecisionWeight_set(RAW_QOS_PRI_TYPE_ACL, *weight_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_priority_selector_system_acl_weight */

/*
 * qos set priority-selector system lut <UINT:weight> 
 */
cparser_result_t
cparser_cmd_qos_set_priority_selector_system_lut_weight(
    cparser_context_t *context,
    uint32_t  *weight_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if((ret = apollo_raw_qos_priDecisionWeight_set(RAW_QOS_PRI_TYPE_LUTFWD, *weight_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_priority_selector_system_lut_weight */

/*
 * qos set priority-selector system smac <UINT:weight> 
 */
cparser_result_t
cparser_cmd_qos_set_priority_selector_system_smac_weight(
    cparser_context_t *context,
    uint32_t  *weight_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if((ret = apollo_raw_qos_priDecisionWeight_set(RAW_QOS_PRI_TYPE_SA, *weight_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_priority_selector_system_smac_weight */

/*
 * qos set priority-selector system svlan <UINT:weight> 
 */
cparser_result_t
cparser_cmd_qos_set_priority_selector_system_svlan_weight(
    cparser_context_t *context,
    uint32_t  *weight_ptr)
{
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    
    if((ret = apollo_raw_qos_priDecisionWeight_set(RAW_QOS_PRI_TYPE_SVLAN, *weight_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }  
    
    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_priority_selector_system_svlan_weight */


/*
 * qos set priority-selector system vlan <UINT:weight> 
 */
cparser_result_t
cparser_cmd_qos_set_priority_selector_system_vlan_weight(
    cparser_context_t *context,
    uint32_t  *weight_ptr)
{
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    
    if((ret = apollo_raw_qos_priDecisionWeight_set(RAW_QOS_PRI_TYPE_CVLAN, *weight_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }  
    
    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_priority_selector_system_svlan_weight */


/*
 * qos set priority-selector system l4 <UINT:weight> 
 */
cparser_result_t
cparser_cmd_qos_set_priority_selector_system_l4_weight(
    cparser_context_t *context,
    uint32_t  *weight_ptr)
{
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();

    if((ret = apollo_raw_qos_priDecisionWeight_set(RAW_QOS_PRI_TYPE_L4, *weight_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_priority_selector_system_l4_weight */



/*
 * qos set priority-to-queue-table index <UINT:index> priority <MASK_LIST:priority> queue-id <UINT:qid>
 */
cparser_result_t
cparser_cmd_qos_set_priority_to_queue_table_index_index_priority_priority_queue_id_qid(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    char * *priority_ptr,
    uint32_t  *qid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int32 pri;
    diag_mask_t mask;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK8(mask, 6), ret);

    DIAG_UTIL_MASK_SCAN(mask, pri)
    {
        if((ret = apollo_raw_qos_priToQidMappingTable_set(*index_ptr,pri,*qid_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        } 
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_priority_to_queue_table_table_index_priority_priority_queue_id_qid */



/*
 * qos set remapping port ( <PORT_LIST:ports> | all ) inter_priority <UINT:inter_priority>
 */
cparser_result_t
cparser_cmd_qos_set_remapping_port_ports_all_inter_priority_inter_priority(
    cparser_context_t *context,
    char * *ports_ptr,
    uint32_t  *inter_priority_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {    
        if((ret = apollo_raw_qos_portBasePri_set(port,*inter_priority_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_remapping_port_ports_all_inter_priority_inter_priority */

/*
 * qos set remapping dot1p system dot1p_priority <UINT:dot1p_priority> inter_priority <UINT:internal_priority>
 */
cparser_result_t
cparser_cmd_qos_set_remapping_dot1p_system_dot1p_priority_dot1p_priority_inter_priority_internal_priority(
    cparser_context_t *context,
    uint32_t  *dot1p_priority_ptr,
    uint32_t  *internal_priority_ptr)
{
    int32 ret = RT_ERR_FAILED;
 
    DIAG_UTIL_PARAM_CHK();
    if((ret = apollo_raw_qos_1QPriRemap_set(*dot1p_priority_ptr,*internal_priority_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     

    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_remapping_dot1p_system_dot1p_priority_dot1p_priority_inter_priority_internal_priority */



/*
 * qos set remapping dscp system dscp <MASK_LIST:dscp_list> inter_priority <UINT:inter_priority>
 */
cparser_result_t
cparser_cmd_qos_set_remapping_dscp_system_dscp_dscp_list_inter_priority_inter_priority(
    cparser_context_t *context,
    char * *dscp_list_ptr,
    uint32_t  *inter_priority_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int32 dscpValue;
    diag_mask_t mask;    
    DIAG_UTIL_PARAM_CHK();


    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK64(mask, 6), ret);

    DIAG_UTIL_MASK_SCAN(mask, dscpValue)
    {
        if((ret = apollo_raw_qos_dscpPriRemap_set(dscpValue,*inter_priority_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
    }
    
    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_remapping_dscp_system_dscp_dscp_list_inter_priority_inter_priority */



/*
 * qos set remarking port ( <PORT_LIST:ports> | all ) ( dot1p | dscp ) state ( enable | disable ) 
 */
cparser_result_t
cparser_cmd_qos_set_remarking_port_ports_all_dot1p_dscp_state_enable_disable(
    cparser_context_t *context,
    char * *ports_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t enable;
    
    DIAG_UTIL_PARAM_CHK();

    if('e'==TOKEN_CHAR(7,0))
        enable = ENABLED;
    else
        enable = DISABLED;
        

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if('o'==TOKEN_CHAR(5,1))
    {/*dot1p*/
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {    
            if((ret = apollo_raw_qos_1qRemarkAbility_set(port,enable)) != RT_ERR_OK)
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }     
        }
    }
    else
    {/*dscp*/
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {    
            if((ret = apollo_raw_qos_dscpRemarkAbility_set(port,enable)) != RT_ERR_OK)
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }     
        }
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_remarking_port_ports_all_dot1p_dscp_state_enable_disable */

/*
 * qos set remarking dot1p system inter_priority <UINT:inter_priority> dot1p_priority <UINT:dot1p_priority>
 */
cparser_result_t
cparser_cmd_qos_set_remarking_dot1p_system_inter_priority_inter_priority_dot1p_priority_dot1p_priority(
    cparser_context_t *context,
    uint32_t  *inter_priority_ptr,
    uint32_t  *dot1p_priority_ptr)
{
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();

    if((ret = apollo_raw_qos_1qRemarkPri_set(*inter_priority_ptr,*dot1p_priority_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     

    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_remarking_dot1p_system_inter_priority_inter_priority_dot1p_priority_dot1p_priority */

/*
 * qos set remarking port ( <PORT_LIST:ports> | all ) dscp system source ( int-pri | dscp )
 */
cparser_result_t
cparser_cmd_qos_set_remarking_port_ports_all_dscp_system_source_int_pri_dscp(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_dscp_source_t  source;
    diag_portlist_t portlist;
    rtk_port_t port;
        
    DIAG_UTIL_PARAM_CHK();

    if('i'==TOKEN_CHAR(8,0))
        source = RAW_QOS_DSCP_SOURCE_INTR_PRI;
    else
        source = RAW_QOS_DSCP_SOURCE_ORI_DSCP;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
     
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if((ret = apollo_raw_qos_dscpRemarkSource_set(port, source)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
    }


    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_remarking_dscp_system_source_int_pri_dscp */

/*
 * qos get remarking port ( <PORT_LIST:ports> | all ) dscp system source
 */
cparser_result_t
cparser_cmd_qos_get_remarking_port_ports_all_dscp_system_source(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port;
    apollo_raw_dscp_source_t  source;
        
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    
    diag_util_mprintf("%-4s %-11s\n","Port","DSCP source");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if((ret = apollo_raw_qos_dscpRemarkSource_get(port, &source)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
        if(RAW_QOS_DSCP_SOURCE_INTR_PRI == source)
            diag_util_mprintf("%-4d %-11s\n",port,"int-pri");
        else
            diag_util_mprintf("%-4d %-11s\n",port,"dscp");
    }
    


    return CPARSER_OK;
}    /* end of cparser_cmd_qos_get_remarking_dscp_system_source */

/*
 * qos set remarking dscp system inter_priority <UINT:inter_priority> remarking_dscp <UINT:dscp>
 */
cparser_result_t
cparser_cmd_qos_set_remarking_dscp_system_inter_priority_inter_priority_remarking_dscp_dscp(
    cparser_context_t *context,
    uint32_t  *inter_priority_ptr,
    uint32_t  *dscp_ptr)
{
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();

    if((ret = apollo_raw_qos_interPriRemarkDscp_set(*inter_priority_ptr, *dscp_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }  

    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_remarking_dscp_system_inter_priority_inter_priority_remarking_dscp_dscp */


/*
 * qos set remarking dscp system original_dscp <MASK_LIST:dscp_list> remarking_dscp <UINT:remarking_dscp>
 */
cparser_result_t
cparser_cmd_qos_set_remarking_dscp_system_original_dscp_dscp_list_remarking_dscp_remarking_dscp(
    cparser_context_t *context,
    char * *dscp_list_ptr,
    uint32_t  *remarking_dscp_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int32 dscpValue;
    diag_mask_t mask;    

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK64(mask, 6), ret);

    DIAG_UTIL_MASK_SCAN(mask, dscpValue)    

    if((ret = apollo_raw_qos_dscpRemarkDscp_set(dscpValue, *remarking_dscp_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }  

    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_remarking_dscp_system_original_dscp_dscp_list_remarking_dscp_remarking_dscp */


/*
 * qos get priority-selector system
 */
cparser_result_t
cparser_cmd_qos_get_priority_selector_system(
    cparser_context_t *context)
{
    uint32 weight;
    int32 ret = RT_ERR_FAILED;
        
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("%-8s %s\n","TYPE","Weight");

    if((ret = apollo_raw_qos_priDecisionWeight_get(RAW_QOS_PRI_TYPE_PORT, &weight)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 
    diag_util_mprintf("%-8s %-2d\n","Port",weight);

    if((ret = apollo_raw_qos_priDecisionWeight_get(RAW_QOS_PRI_TYPE_DOT1Q, &weight)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 
    diag_util_mprintf("%-8s %-2d\n","Dot1Q",weight);

    if((ret = apollo_raw_qos_priDecisionWeight_get(RAW_QOS_PRI_TYPE_DSCP, &weight)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 
    diag_util_mprintf("%-8s %-2d\n","DSCP",weight);

    if((ret = apollo_raw_qos_priDecisionWeight_get(RAW_QOS_PRI_TYPE_ACL, &weight)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 
    diag_util_mprintf("%-8s %-2d\n","ACL",weight);

    if((ret = apollo_raw_qos_priDecisionWeight_get(RAW_QOS_PRI_TYPE_CVLAN, &weight)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 
    diag_util_mprintf("%-8s %-2d\n","CVLAN",weight);
    
    if((ret = apollo_raw_qos_priDecisionWeight_get(RAW_QOS_PRI_TYPE_LUTFWD, &weight)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 
    diag_util_mprintf("%-8s %-2d\n","LUT_FWD",weight);
    
    if((ret = apollo_raw_qos_priDecisionWeight_get(RAW_QOS_PRI_TYPE_SA, &weight)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 
    diag_util_mprintf("%-8s %-2d\n","SA",weight);

    if((ret = apollo_raw_qos_priDecisionWeight_get(RAW_QOS_PRI_TYPE_SVLAN, &weight)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 
    diag_util_mprintf("%-8s %-2d\n","SVALN",weight);

    if((ret = apollo_raw_qos_priDecisionWeight_get(RAW_QOS_PRI_TYPE_L4, &weight)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 
    diag_util_mprintf("%-8s %-2d\n","L4",weight);

    return CPARSER_OK;
}    /* end of cparser_cmd_qos_get_priority_selector_system */

/*
 * qos get remapping port ( <PORT_LIST:ports> | all )
 */
cparser_result_t
cparser_cmd_qos_get_remapping_port_ports_all(
    cparser_context_t *context,
    char * *ports_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;
    uint32 inter_priority;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    
    diag_util_mprintf("Port prioirty\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {    
        if((ret = apollo_raw_qos_portBasePri_get(port,&inter_priority)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
        diag_util_mprintf("%-4d %-4d\n",port,inter_priority);

    }
    return CPARSER_OK;
}    /* end of cparser_cmd_qos_get_remapping_port_ports_all */

/*
 * qos get remapping dot1p system
 */
cparser_result_t
cparser_cmd_qos_get_remapping_dot1p_system(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    int32 dot1pPri;
    int32 pri;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Priority Remap_priority\n");
    for(dot1pPri =0; dot1pPri <= APOLLO_PRIMAX ; dot1pPri++ )
    {
        if((ret = apollo_raw_qos_1QPriRemap_get(dot1pPri,&pri)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        diag_util_mprintf("%-8d  %-4d\n",dot1pPri,pri);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_qos_get_remapping_dot1p_system */

/*
 * qos get remapping dscp system
 */
cparser_result_t
cparser_cmd_qos_get_remapping_dscp_system(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    int32 dscp;
    int32 pri;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("DSCP  Priority\n");
    for(dscp =0; dscp <= APOLLO_DSCPMAX ; dscp++ )
    {
        if((ret = apollo_raw_qos_dscpPriRemap_get(dscp,&pri)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        diag_util_mprintf("%-4d  %-4d\n",dscp,pri);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_qos_get_remapping_dscp_system */

/*
 * qos get remarking port ( <PORT_LIST:ports> | all ) ( dot1p | dscp ) state
 */
cparser_result_t
cparser_cmd_qos_get_remarking_port_ports_all_dot1p_dscp_state(
    cparser_context_t *context,
    char * *ports_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t enable;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    
    diag_util_mprintf("Port State\n");
    if('o'==TOKEN_CHAR(5,1))
    {/*dot1p*/
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {    
            if((ret = apollo_raw_qos_1qRemarkAbility_get(port,&enable)) != RT_ERR_OK)
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }     
            diag_util_mprintf("%-4d %s\n",port,diagStr_enable[enable]);
        }
    }
    else
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {    
            if((ret = apollo_raw_qos_dscpRemarkAbility_get(port,&enable)) != RT_ERR_OK)
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }     
            diag_util_mprintf("%-4d %s\n",port,diagStr_enable[enable]);
        }
    }
    
    return CPARSER_OK;
}    /* end of cparser_cmd_qos_get_remarking_port_ports_all_state */

/*
 * qos get remarking dscp system source ( int-pri | dscp )
 */
cparser_result_t
cparser_cmd_qos_get_remarking_dscp_system_source_int_pri_dscp(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 remark_dscp;
    uint32 oriDscp;
    uint32 intPri;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    
    if('d'==TOKEN_CHAR(6,0))
    {/*dscp*/    
        diag_util_mprintf("%-8s %-11s \n","Ori_DSCP Remark_DSCP");
        for(oriDscp = 0; oriDscp <= APOLLO_DSCPMAX ; oriDscp ++)
        {    
            if((ret = apollo_raw_qos_dscpRemarkDscp_get(oriDscp,&remark_dscp)) != RT_ERR_OK)
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }     
            diag_util_mprintf("%-8d %-11d\n",oriDscp,remark_dscp);
    
        }
    }
    else
    {/*dscp*/    
        diag_util_mprintf("%-8s %-11s \n","INT_Pri Remark_DSCP");
        for(intPri = 0; intPri <= APOLLO_PRIMAX ; intPri ++)
        {    
            if((ret = apollo_raw_qos_interPriRemarkDscp_get(intPri,&remark_dscp)) != RT_ERR_OK)
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }     
            diag_util_mprintf("%-7d %-11d\n",intPri,remark_dscp);
    
        }
    }
    
    return CPARSER_OK;
}    /* end of cparser_cmd_qos_get_remarking_dscp_system */

/*
 * qos get remarking dot1p system */
cparser_result_t
cparser_cmd_qos_get_remarking_dot1p_system(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 inter_pri;
    uint32 remark_pri;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("%-3s  %-13s\n","Pri", "Remarking_pri");
    
    for(inter_pri = 0; inter_pri <= APOLLO_PRIMAX ; inter_pri++)
    {
        if((ret = apollo_raw_qos_1qRemarkPri_get(inter_pri,&remark_pri)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
        diag_util_mprintf("%-3d  %-13d\n",inter_pri, remark_pri);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_qos_get_remarking_dot1p_system */

#if defined(CMD_QOS_GET_REMARKING_DSCP_PORT_PORTS_ALL)
/*
 * qos get remarking dscp port ( <PORT_LIST:ports> | all )
 */
cparser_result_t
cparser_cmd_qos_get_remarking_dscp_port_ports_all(
    cparser_context_t *context,
    char * *ports_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("not implement");

    return CPARSER_OK;
}    /* end of cparser_cmd_qos_get_remarking_dscp_port_ports_all */
#endif
/*
 * qos set priority-to-queue-mapping port ( <PORT_LIST:ports> | all ) priority-to-queue-table-idx <UINT:index>
 */
cparser_result_t
cparser_cmd_qos_set_priority_to_queue_mapping_port_ports_all_priority_to_queue_table_idx_index(
    cparser_context_t *context,
    char * *ports_ptr,
    uint32_t  *index_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {    
        if((ret = apollo_raw_qos_portQidMapIdx_set(port,*index_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_priority_to_queue_mapping_port_ports_all_priority_to_queue_table_idx_index */

/*
 * qos set priority-to-queue-table table_idx <UINT:index> pri_0_qid <UINT:pri_0_qid> pri_1_qid <UINT:pri_1_qid> pri_2_qid <UINT:pri_2_qid> pri_3_qid <UINT:pri_3_qid> pri_4_qid <UINT:pri_4_qid> pri_5_qid <UINT:pri_5_qid> pri_6_qid <UINT:pri_6_qid> pri_7_qid <UINT:pri_7_qid>
 */
cparser_result_t
cparser_cmd_qos_set_priority_to_queue_table_table_idx_index_pri_0_qid_pri_0_qid_pri_1_qid_pri_1_qid_pri_2_qid_pri_2_qid_pri_3_qid_pri_3_qid_pri_4_qid_pri_4_qid_pri_5_qid_pri_5_qid_pri_6_qid_pri_6_qid_pri_7_qid_pri_7_qid(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *pri_0_qid_ptr,
    uint32_t  *pri_1_qid_ptr,
    uint32_t  *pri_2_qid_ptr,
    uint32_t  *pri_3_qid_ptr,
    uint32_t  *pri_4_qid_ptr,
    uint32_t  *pri_5_qid_ptr,
    uint32_t  *pri_6_qid_ptr,
    uint32_t  *pri_7_qid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if((ret = apollo_raw_qos_priToQidMappingTable_set(*index_ptr,0,*pri_0_qid_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     

    if((ret = apollo_raw_qos_priToQidMappingTable_set(*index_ptr,1,*pri_1_qid_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     
    if((ret = apollo_raw_qos_priToQidMappingTable_set(*index_ptr,2,*pri_2_qid_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     
    if((ret = apollo_raw_qos_priToQidMappingTable_set(*index_ptr,3,*pri_3_qid_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     
    if((ret = apollo_raw_qos_priToQidMappingTable_set(*index_ptr,4,*pri_4_qid_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     
    if((ret = apollo_raw_qos_priToQidMappingTable_set(*index_ptr,5,*pri_5_qid_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     
    if((ret = apollo_raw_qos_priToQidMappingTable_set(*index_ptr,6,*pri_6_qid_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     
    if((ret = apollo_raw_qos_priToQidMappingTable_set(*index_ptr,7,*pri_7_qid_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     

    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_priority_to_queue_table_table_idx_index_pri_0_qid_pri_0_qid_pri_1_qid_pri_1_qid_pri_2_qid_pri_2_qid_pri_3_qid_pri_3_qid_pri_4_qid_pri_4_qid_pri_5_qid_pri_5_qid_pri_6_qid_pri_6_qid_pri_7_qid_pri_7_qid */

/*
 * qos get priority-to-queue-mapping port ( <PORT_LIST:ports> | all )
 */
cparser_result_t
cparser_cmd_qos_get_priority_to_queue_mapping_port_ports_all(
    cparser_context_t *context,
    char * *ports_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;
    uint32 index;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    diag_util_mprintf("Port  Index\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {    
        if((ret = apollo_raw_qos_portQidMapIdx_get(port,&index)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    diag_util_mprintf("%-4d  %-4d\n",port,index);
            
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_qos_get_priority_to_queue_mapping_port_ports_all */

/*
 * qos get priority-to-queue-table table_idx <UINT:index>
 */
cparser_result_t
cparser_cmd_qos_get_priority_to_queue_table_table_idx_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    uint32 pri;
    uint32 qid;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    diag_util_mprintf("Index pri_0 pri_1 pri_2 pri_3 pri_4 pri_5 pri_6 pri_7\n");
    diag_util_printf("%-5d ",*index_ptr);
    for(pri=0 ;pri <=APOLLO_PRIMAX; pri++ )
    {
        if((ret = apollo_raw_qos_priToQidMappingTable_get(*index_ptr,pri,&qid)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        diag_util_printf("%-5d ",qid);
    }    
    diag_util_mprintf("\n");
    
    return CPARSER_OK;
}    /* end of cparser_cmd_qos_get_priority_to_queue_table_table_idx_index */

/*
 * qos get priority-to-queue-table
 */
cparser_result_t
cparser_cmd_qos_get_priority_to_queue_table(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 pri;
    uint32 qid;
    uint32 index;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    diag_util_mprintf("Index pri_0 pri_1 pri_2 pri_3 pri_4 pri_5 pri_6 pri_7\n");

    for(index=0 ;index <APOLLO_PRI_TO_QUEUE_TBL_SIZE; index++ )
    {
        diag_util_printf("%-5d ",index);

        for(pri=0 ;pri <=APOLLO_PRIMAX; pri++ )
        {
            if((ret = apollo_raw_qos_priToQidMappingTable_get(index,pri,&qid)) != RT_ERR_OK)
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            diag_util_printf("%-5d ",qid);
        }    
        diag_util_printf("\n");
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_qos_get_priority_to_queue_table */

/*
 * qos set remapping forward-to-cpu system internal_priority <UINT:internal_priority> remapping_priority <UINT:remapping_priority>
 */
cparser_result_t
cparser_cmd_qos_set_remapping_forward_to_cpu_system_internal_priority_internal_priority_remapping_priority_remapping_priority(
    cparser_context_t *context,
    uint32_t  *internal_priority_ptr,
    uint32_t  *remapping_priority_ptr)
{
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if((ret = apollo_raw_qos_priRemapToCpu_set(*internal_priority_ptr,*remapping_priority_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     

    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_remapping_forward_to_cpu_system_internal_priority_internal_priority_remapping_priority_remapping_priority */

/*
 * qos get remapping forward-to-cpu system all
 */
cparser_result_t
cparser_cmd_qos_get_remapping_forward_to_cpu_system_all(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 pri;
    uint32 inrPri;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    for(inrPri = 0; inrPri <= APOLLO_PRIMAX; inrPri++)
    {
        if((ret = apollo_raw_qos_priRemapToCpu_get(inrPri,&pri)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        diag_util_mprintf("Priority:%2d Remapping prority:%2d\n",inrPri,pri);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_qos_get_remapping_forward_to_cpu_system_all */

/*
 * qos get remapping forward-to-cpu system internal_priority <UINT:internal_priority>
 */
cparser_result_t
cparser_cmd_qos_get_remapping_forward_to_cpu_system_internal_priority_internal_priority(
    cparser_context_t *context,
    uint32_t  *internal_priority_ptr)
{
    int32 ret = RT_ERR_FAILED;
    uint32 pri;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    
    if((ret = apollo_raw_qos_priRemapToCpu_get(*internal_priority_ptr,&pri)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("Priority:%2d Remapping prority:%2d\n",*internal_priority_ptr,pri);
    return CPARSER_OK;
}    /* end of cparser_cmd_qos_get_remapping_forward_to_cpu_system_internal_priority_internal_priority */

/*
 * qos set remapping ethernet-av port ( <PORT_LIST:ports> | all ) state ( enable | disable )
 */
cparser_result_t
cparser_cmd_qos_set_remapping_ethernet_av_port_ports_all_state_enable_disable(
    cparser_context_t *context,
    char * *ports_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t enable;
    
    DIAG_UTIL_PARAM_CHK();

    if('e'==TOKEN_CHAR(7,0))
        enable = ENABLED;
    else
        enable = DISABLED;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {    
        if((ret = apollo_raw_qos_ethernetAvEnable_set(port,enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_remapping_ethernet_av_port_ports_all_state_enable_disable */

/*
 * qos get remapping ethernet-av port ( <PORT_LIST:ports> | all )
 */
cparser_result_t
cparser_cmd_qos_get_remapping_ethernet_av_port_ports_all(
    cparser_context_t *context,
    char * *ports_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t enable;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    diag_util_mprintf("Port State\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {    
        if((ret = apollo_raw_qos_ethernetAvEnable_get(port,&enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        diag_util_mprintf("%-4d %s\n",port,diagStr_enable[enable]);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_qos_get_remapping_ethernet_av_port_ports_all */

/*
 * qos set remapping ethernet-av inter_priority <UINT:inter_priority> user_priority <UINT:user_priority>
 */
cparser_result_t
cparser_cmd_qos_set_remapping_ethernet_av_inter_priority_inter_priority_user_priority_user_priority(
    cparser_context_t *context,
    uint32_t  *inter_priority_ptr,
    uint32_t  *user_priority_ptr)
{
    int32 ret = RT_ERR_FAILED;
    
    if((ret = apollo_raw_qos_ethernetAvRemapPri_set(*inter_priority_ptr,*user_priority_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     

    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_remapping_ethernet_av_inter_priority_inter_priority_user_priority_user_priority */

/*
 * qos get remapping ethernet-av
 */
cparser_result_t
cparser_cmd_qos_get_remapping_ethernet_av(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 pri;
    uint32 inrPri;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    for(inrPri = 0; inrPri <= APOLLO_PRIMAX; inrPri++)
    {
        if((ret = apollo_raw_qos_ethernetAvRemapPri_get(inrPri,&pri)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        diag_util_mprintf("Priority:%2d Remapping prority:%2d\n",inrPri,pri);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_qos_get_remapping_ethernet_av */

/*
 * qos get remapping ethernet-av inter_priority <UINT:inter_priority>
 */
cparser_result_t
cparser_cmd_qos_get_remapping_ethernet_av_inter_priority_inter_priority(
    cparser_context_t *context,
    uint32_t  *inter_priority_ptr)
{
    int32 ret = RT_ERR_FAILED;
    uint32 pri;

    DIAG_UTIL_PARAM_CHK();

    if((ret = apollo_raw_qos_ethernetAvRemapPri_get(*inter_priority_ptr,&pri)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    diag_util_mprintf("Priority:%2d Remapping prority:%2d\n",*inter_priority_ptr,pri);

    return CPARSER_OK;
}    /* end of cparser_cmd_qos_get_remapping_ethernet_av_inter_priority_inter_priority */

/*
 * qos set scheduling algorithm port ( <PORT_LIST:ports> | all ) queue-id <UINT:qid> ( strict | wfq )
 */
cparser_result_t
cparser_cmd_qos_set_scheduling_algorithm_port_ports_all_queue_id_qid_strict_wfq(
    cparser_context_t *context,
    char * *ports_ptr,
    uint32_t  *qid_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;
    apollo_raw_queue_type_t type;
    
    DIAG_UTIL_PARAM_CHK();

    if('s'==TOKEN_CHAR(8,0))
        type = RAW_QOS_QUEUE_STRICT;
    else
        type = RAW_QOS_QUEUE_WFQ;
        

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {    
        if((ret = apollo_raw_qos_queueType_set(port, *qid_ptr ,type)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_scheduling_algorithm_port_ports_all_queue_id_qid_strict_wfq */

/*
 * qos get scheduling algorithm port ( <PORT_LIST:ports> | all ) queue-id <UINT:qid>
 */
cparser_result_t
cparser_cmd_qos_get_scheduling_algorithm_port_ports_all_queue_id_qid(
    cparser_context_t *context,
    char * *ports_ptr,
    uint32_t  *qid_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;
    apollo_raw_queue_type_t type;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if('s'==TOKEN_CHAR(8,0))
        type = RAW_QOS_QUEUE_STRICT;
    else
        type = RAW_QOS_QUEUE_WFQ;
        
    diag_util_mprintf("%-5s %-8s %-4s\n","Port", "Queue-id", "Type");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {    
        if((ret = apollo_raw_qos_queueType_get(port, *qid_ptr ,&type)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
        diag_util_mprintf("%-5d %-8d %s\n",port, *qid_ptr, diagStr_queueType[type]);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_qos_get_scheduling_algorithm_port_ports_all_queue_id_qid */

/*
 * qos get scheduling queue-weight port ( <PORT_LIST:ports> | all ) queue-id <UINT:qid>
 */
cparser_result_t
cparser_cmd_qos_get_scheduling_queue_weight_port_ports_all_queue_id_qid(
    cparser_context_t *context,
    char * *ports_ptr,
    uint32_t  *qid_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;
    uint32 weight;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("%-5s %-8s %-4s\n","Port", "Queue-id", "Weight");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {   
    
        if((ret = apollo_raw_qos_wfqWeight_get(port, *qid_ptr ,&weight)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
        diag_util_mprintf("%-5d %-8d %d\n",port, *qid_ptr, weight);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_qos_get_scheduling_queue_weight_port_ports_all_queue_id_qid */

/*
 * qos set scheduling queue-weight port ( <PORT_LIST:ports> | all ) queue-id 0 weight <UINT:weight>
 */
cparser_result_t
cparser_cmd_qos_set_scheduling_queue_weight_port_ports_all_queue_id_0_weight_weight(
    cparser_context_t *context,
    char * *ports_ptr,
    uint32_t  *weight_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {    
        if((ret = apollo_raw_qos_wfqWeight_set(port, 0 ,*weight_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_scheduling_queue_weight_port_ports_all_queue_id_0_weight_weight */

/*
 * qos set scheduling queue-weight port ( <PORT_LIST:ports> | all ) queue-id <UINT:qid> weight <UINT:weight>
 */
cparser_result_t
cparser_cmd_qos_set_scheduling_queue_weight_port_ports_all_queue_id_qid_weight_weight(
    cparser_context_t *context,
    char * *ports_ptr,
    uint32_t  *qid_ptr,
    uint32_t  *weight_ptr)
{
    diag_portlist_t portlist;
    rtk_port_t port;
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {    
        if((ret = apollo_raw_qos_wfqWeight_set(port, *qid_ptr ,*weight_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_qos_set_scheduling_queue_weight_port_ports_all_queue_id_qid_weight_weight */


