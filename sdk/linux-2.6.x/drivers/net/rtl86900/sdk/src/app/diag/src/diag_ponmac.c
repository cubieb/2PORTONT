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
 * Purpose : Definition those PON MAC command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *           POn MAC 
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
#include <dal/apollo/raw/apollo_raw_ponmac.h>
#include <diag_str.h>

/*
 * pon_mac set egress-rate <UINT:rate>
 */
cparser_result_t
cparser_cmd_pon_mac_set_egress_rate_rate(
    cparser_context_t *context,
    uint32_t  *rate_ptr)
{
    int32 ret = RT_ERR_FAILED;
 
    DIAG_UTIL_PARAM_CHK();

    if((ret = apollo_raw_ponMacEgrRate_set(*rate_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 
    
    return CPARSER_OK;
}    /* end of cparser_cmd_pon_mac_set_egress_rate_rate */

/*
 * pon_mac get egreSs-rate
 */
cparser_result_t
cparser_cmd_pon_mac_get_egress_rate(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 rate;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    if((ret = apollo_raw_ponMacEgrRate_get(&rate)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 
    diag_util_mprintf("Egress Rate: %d\n",rate);

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_mac_get_egress_rate */

/*
 * pon_mac set drain-out queue <UINT:qid>
 */
cparser_result_t
cparser_cmd_pon_mac_set_drain_out_queue_qid(
    cparser_context_t *context,
    uint32_t  *qid_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    if((ret = apollo_raw_ponMacQueueDrainOutState_set(*qid_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 
    return CPARSER_OK;
}    /* end of cparser_cmd_pon_mac_set_drain_out_queue_qid */

/*
 * pon_mac set drain-out t-cont <UINT:tcont>
 */
cparser_result_t
cparser_cmd_pon_mac_set_drain_out_t_cont_tcont(
    cparser_context_t *context,
    uint32_t  *tcont_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    if((ret = apollo_raw_ponMacTcontDrainOutState_set(*tcont_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 
    return CPARSER_OK;
}    /* end of cparser_cmd_pon_mac_set_drain_out_t_cont_tcont */

/*
 * pon_mac get drain-out status
 */
cparser_result_t
cparser_cmd_pon_mac_get_drain_out_status(
    cparser_context_t *context)
{
    apollo_raw_ponmac_draintOutState_t state;
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if((ret = apollo_raw_ponMacTcontDrainOutState_get(&state)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 
    diag_util_mprintf("T-CONT draint out state: %s\n",(state == RAW_PONMAC_DRAINOUT_FIN)? DIAG_STR_FIN:DIAG_STR_DOING);

    if((ret = apollo_raw_ponMacQueueDrainOutState_get(&state)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 
    diag_util_mprintf("Queue draint out state: %s\n",(state == RAW_PONMAC_DRAINOUT_FIN)? DIAG_STR_FIN:DIAG_STR_DOING);

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_mac_get_drain_out_status */

/*
 * pon_mac set stream-to-queue-mapping stream <MASK_LIST:stream_id> queue <UINT:qid>  
 */
cparser_result_t
cparser_cmd_pon_mac_set_stream_to_queue_mapping_stream_stream_id_queue_qid(
    cparser_context_t *context,
    char * *stream_id_ptr,
    uint32_t  *qid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int32 sid;
    diag_mask_t mask;   
    
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK128(mask, 4), ret);

    DIAG_UTIL_MASK_SCAN(mask, sid)
    {
        if((ret = apollo_raw_ponMacSidToQueueMap_set(sid,*qid_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_mac_set_stream_to_queue_mapping_stream_stream_id_queue_qid */

/*
 * pon_mac get stream-to-queue-mapping stream <MASK_LIST:stream_id>
 */
cparser_result_t
cparser_cmd_pon_mac_get_stream_to_queue_mapping_stream_stream_id(
    cparser_context_t *context,
    char * *stream_id_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int32 sid;
    int32 qid;
    diag_mask_t mask;   
  
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK128(mask, 4), ret);

    diag_util_mprintf("Stream-id  Queue-id\n");

    DIAG_UTIL_MASK_SCAN(mask, sid)
    {
        if((ret = apollo_raw_ponMacSidToQueueMap_get(sid,&qid)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
        diag_util_mprintf("%-9d  %-8d\n",sid,qid);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_mac_get_stream_to_queue_mapping_stream_stream_id */

/*
 * pon_mac set PIR queue <MASK_LIST:qid> rate <UINT:rate>  
 */
cparser_result_t
cparser_cmd_pon_mac_set_PIR_queue_qid_rate_rate(
    cparser_context_t *context,
    char * *qid_ptr,
    uint32_t  *rate_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int32 qid;
    diag_mask_t mask;   
  
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK128(mask, 4), ret);

    DIAG_UTIL_MASK_SCAN(mask, qid)
    {
        if((ret = apollo_raw_ponMacPirRate_set(qid,*rate_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_mac_set_pir_queue_qid_rate_rate */

/*
 * pon_mac get PIR queue <MASK_LIST:qid> rate 
 */
cparser_result_t
cparser_cmd_pon_mac_get_PIR_queue_qid_rate(
    cparser_context_t *context,
    char * *qid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int32 rate;
    int32 qid;
    diag_mask_t mask;   
  
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK128(mask, 4), ret);

    diag_util_mprintf("Queue-id      Rate\n");

    DIAG_UTIL_MASK_SCAN(mask, qid)
    {
        if((ret = apollo_raw_ponMacPirRate_get(qid,&rate)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
        diag_util_mprintf("%-8d  %-8d\n",qid,rate);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_pon_mac_get_pir_queue_qid_rate */

/*
 * pon_mac set CIR queue <MASK_LIST:qid> rate <UINT:rate>  
 */
cparser_result_t
cparser_cmd_pon_mac_set_CIR_queue_qid_rate_rate(
    cparser_context_t *context,
    char * *qid_ptr,
    uint32_t  *rate_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int32 qid;
    diag_mask_t mask;   
  
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK128(mask, 4), ret);

    DIAG_UTIL_MASK_SCAN(mask, qid)
    {
        if((ret = apollo_raw_ponMacCirRate_set(qid,*rate_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_pon_mac_set_cir_queue_qid_rate_rate */

/*
 * pon_mac get CIR queue <MASK_LIST:qid> rate
 */
cparser_result_t
cparser_cmd_pon_mac_get_CIR_queue_qid_rate(
    cparser_context_t *context,
    char * *qid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int32 rate;
    int32 qid;
    diag_mask_t mask;   
  
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK128(mask, 4), ret);

    diag_util_mprintf("Queue-id      Rate\n");

    DIAG_UTIL_MASK_SCAN(mask, qid)
    {
        if((ret = apollo_raw_ponMacCirRate_get(qid,&rate)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
        diag_util_mprintf("%-8d  %-8d\n",qid,rate);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_pon_mac_get_cir_queue_qid_rate */

/*
 * pon_mac set t-cont-schedule-mapping t-cont <MASK_LIST:tcont> queue_mask <UINT:mask>  
 */
cparser_result_t
cparser_cmd_pon_mac_set_t_cont_schedule_mapping_t_cont_tcont_queue_mask_mask(
    cparser_context_t *context,
    char * *tcont_ptr,
    uint32_t  *mask_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int32 tcont;
    diag_mask_t mask;   
  
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK32(mask, 4), ret);

    DIAG_UTIL_MASK_SCAN(mask, tcont)
    {
        if((ret = apollo_raw_ponMacScheQmap_set(tcont,*mask_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_pon_mac_set_t_cont_schedule_mapping_t_cont_tcont_queue_mask_mask */

/*
 * pon_mac get t-cont-schedule-mapping t-cont <MASK_LIST:tcont> 
 */
cparser_result_t
cparser_cmd_pon_mac_get_t_cont_schedule_mapping_t_cont_tcont(
    cparser_context_t *context,
    char * *tcont_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int32 tcont;
    diag_mask_t mask;   
    int32 mapMask;
  
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK32(mask, 4), ret);

    diag_util_mprintf("T-CONT  %-10s\n","Qmap");

    DIAG_UTIL_MASK_SCAN(mask, tcont)
    {
        if((ret = apollo_raw_ponMacScheQmap_get(tcont,&mapMask)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
        diag_util_mprintf("%-6d  0x%-8.8x\n",tcont,mapMask);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_pon_mac_get_t_cont_schedule_mapping_t_cont_tcont */

/*
 * pon_mac set scheduling algorithm queue-id <MASK_LIST:qid> ( strict | wfq )  
 */
cparser_result_t
cparser_cmd_pon_mac_set_scheduling_algorithm_queue_id_qid_strict_wfq(
    cparser_context_t *context,
    char * *qid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int32 qid;
    diag_mask_t mask;   
    apollo_raw_queue_type_t qType;
    
    DIAG_UTIL_PARAM_CHK();

    if('s'==TOKEN_CHAR(6,0))
        qType = RAW_QOS_QUEUE_STRICT;
    else
        qType = RAW_QOS_QUEUE_WFQ;
    

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK128(mask, 5), ret);

    DIAG_UTIL_MASK_SCAN(mask, qid)
    {
        if((ret = apollo_raw_ponMac_queueType_set(qid,qType)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_pon_mac_set_scheduling_algorithm_queue_id_qid_strict_wfq */

/*
 * pon_mac get scheduling algorithm queue-id <MASK_LIST:qid>
 */
cparser_result_t
cparser_cmd_pon_mac_get_scheduling_algorithm_queue_id_qid(
    cparser_context_t *context,
    char * *qid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int32 qid;
    diag_mask_t mask;   
    apollo_raw_queue_type_t qType;
    
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK128(mask, 5), ret);
    
    diag_util_mprintf("%-8s %-10s\n","queue-id","queue-type");
    DIAG_UTIL_MASK_SCAN(mask, qid)
    {
        if((ret = apollo_raw_ponMac_queueType_get(qid,&qType)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
        diag_util_mprintf("%-8d %s\n",qid, diagStr_queueType[qType]);
    }
    
    return CPARSER_OK;
}    /* end of cparser_cmd_pon_mac_set_scheduling_algorithm_queue_id_qid */

/*
 * pon_mac set scheduling queue-weight queue-id <MASK_LIST:qid> weight <UINT:weight>
 */
cparser_result_t
cparser_cmd_pon_mac_set_scheduling_queue_weight_queue_id_qid_weight_weight(
    cparser_context_t *context,
    char * *qid_ptr,
    uint32_t  *weight_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int32 qid;
    diag_mask_t mask;   
    
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK128(mask, 5), ret);

    DIAG_UTIL_MASK_SCAN(mask, qid)
    {
        if((ret = apollo_raw_ponMac_wfqWeight_set(qid,*weight_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_pon_mac_set_scheduling_queue_weight_queue_id_qid_weight_weight */

/*
 * pon_mac get scheduling queue-weight queue-id <MASK_LIST:qid>
 */
cparser_result_t
cparser_cmd_pon_mac_get_scheduling_queue_weight_queue_id_qid(
    cparser_context_t *context,
    char * *qid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int32 qid;
    int32 weight;
    diag_mask_t mask;   
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK128(mask, 5), ret);
    diag_util_mprintf("%-8d %-8s\n","queue-id","weight");

    DIAG_UTIL_MASK_SCAN(mask, qid)
    {
        if((ret = apollo_raw_ponMac_wfqWeight_get(qid,&weight)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
        diag_util_mprintf("%-8d %-8d\n",qid,weight);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_pon_mac_get_scheduling_queue_weight_queue_id_qid */

/*
 * pon_mac set egress-drop queue-id <MASK_LIST:qid> state ( enable | disable )
 */
cparser_result_t
cparser_cmd_pon_mac_set_egress_drop_queue_id_qid_state_enable_disable(
    cparser_context_t *context,
    char * *qid_ptr)
{
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;
    int32 qid;
    diag_mask_t mask;
        
    DIAG_UTIL_PARAM_CHK();

    if('e'==TOKEN_CHAR(6,0))
        enable = ENABLED;
    else
        enable = DISABLED;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK128(mask, 4), ret);    
    DIAG_UTIL_MASK_SCAN(mask, qid)
    {
        if((ret = apollo_raw_ponMac_egrDropState_set(qid,enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_mac_set_egress_drop_queue_id_qid_state_enable_disable */

/*
 * pon_mac get egress-drop queue-id <MASK_LIST:qid> state
 */
cparser_result_t
cparser_cmd_pon_mac_get_egress_drop_queue_id_qid_state(
    cparser_context_t *context,
    char * *qid_ptr)
{
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;
    int32 qid;
    diag_mask_t mask;
      
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK128(mask, 4), ret);    

    diag_util_mprintf("%-8d %-8s\n","queue-id","state");
    DIAG_UTIL_MASK_SCAN(mask, qid)
    {
        if((ret = apollo_raw_ponMac_egrDropState_get(qid,&enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
        diag_util_mprintf("%-8d %-8s\n",qid,diagStr_enable[enable]);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_pon_mac_get_egress_drop_queue_id_qid_state */

/*
 * pon_mac set scheduling t-cont <MASK_LIST:tcont> state ( enable | disable ) 
 */
cparser_result_t
cparser_cmd_pon_mac_set_scheduling_t_cont_tcont_state_enable_disable(
    cparser_context_t *context,
    char * *tcont_ptr)
{
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;
    int32 tcont;
    diag_mask_t mask;
        
    DIAG_UTIL_PARAM_CHK();

    if('e'==TOKEN_CHAR(6,0))
        enable = ENABLED;
    else
        enable = DISABLED;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK32(mask, 4), ret);    
    DIAG_UTIL_MASK_SCAN(mask, tcont)
    {
        if((ret = apollo_raw_ponMacTcontEnable_set(tcont,enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_mac_set_scheduling_t_cont_tcont_state_enable_disable */

/*
 * pon_mac get scheduling t-cont <MASK_LIST:tcont> state
 */
cparser_result_t
cparser_cmd_pon_mac_get_scheduling_t_cont_tcont_state(
    cparser_context_t *context,
    char * *tcont_ptr)
{
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;
    int32 tcont;
    diag_mask_t mask;
      
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK32(mask, 4), ret);    

    diag_util_mprintf("%-8d %-8s\n","T-CONT","State");
    DIAG_UTIL_MASK_SCAN(mask, tcont)
    {
        if((ret = apollo_raw_ponMacTcontEnable_get(tcont,&enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
        diag_util_mprintf("%-8d %-8s\n",tcont,diagStr_enable[enable]);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_pon_mac_get_scheduling_t_cont_tcont_state */


