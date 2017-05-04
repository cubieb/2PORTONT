/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 61639 $
 * $Date: 2015-09-04 08:31:34 +0800 (Fri, 04 Sep 2015) $
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
#include <diag_str.h>
#include <rtk/ponmac.h>
#include <hal/mac/reg.h>

#ifdef CONFIG_SDK_APOLLOMP
#include <hal/chipdef/apollomp/rtk_apollomp_reg_struct.h>
#include <dal/apollomp/raw/apollomp_raw_ponmac.h>
#endif

#ifdef CONFIG_SDK_APOLLO
#include <hal/chipdef/apollo/apollo_reg_struct.h>
#include <dal/apollo/raw/apollo_raw_ponmac.h>
#endif

#ifdef CONFIG_SDK_RTL9601B
#include <hal/chipdef/rtl9601b/rtk_rtl9601b_reg_struct.h>
#include <dal/rtl9601b/dal_rtl9601b_ponmac.h>
#endif

#ifdef CONFIG_SDK_RTL9602C
#include <hal/chipdef/rtl9602c/rtk_rtl9602c_reg_struct.h>
#include <dal/rtl9602c/dal_rtl9602c_ponmac.h>
#endif

static rtk_ponmac_queueCfg_t  globalQueueCfg;
static rtk_ponmac_queue_t     globalQueue;


/*
 * pon init
 */
cparser_result_t
cparser_cmd_pon_init(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    /*init pon module*/
    DIAG_UTIL_ERR_CHK(rtk_ponmac_init(), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_init */


/*
 * pon clear
 */
cparser_result_t
cparser_cmd_pon_clear(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    /*clear queue config global setting*/
    memset(&globalQueueCfg,0x0, sizeof(rtk_ponmac_queueCfg_t));
    memset(&globalQueue,0x0, sizeof(rtk_ponmac_queue_t));

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_clear */


/*
 * pon set drain-out t-cont <UINT:tcont> queue-id <MASK_LIST:qid>
 */
cparser_result_t
cparser_cmd_pon_set_drain_out_t_cont_tcont_queue_id_qid(
    cparser_context_t *context,
    uint32_t  *tcont_ptr,
    char * *qid_ptr)
{
    diag_mask_t mask;
    int32 ret = RT_ERR_FAILED;
    uint32 index;
    uint32 queueId;
    rtk_ponmac_queue_t pon_queue;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK128(mask, 6), ret);
    DIAG_UTIL_MASK_SCAN(mask, index)
    {
        queueId = ((*tcont_ptr)/8) * 32 + index;
        pon_queue.schedulerId = *tcont_ptr;
        pon_queue.queueId = index;

        switch(DIAG_UTIL_CHIP_TYPE)
        {
#ifdef CONFIG_SDK_APOLLO
            case APOLLO_CHIP_ID:
                DIAG_UTIL_ERR_CHK(apollo_raw_ponMacQueueDrainOutState_set(queueId), ret);
                break;
#endif
#ifdef CONFIG_SDK_APOLLOMP
            case APOLLOMP_CHIP_ID:
                DIAG_UTIL_ERR_CHK(rtk_ponmac_queueDrainOut_set(&pon_queue), ret);
                break;
#endif
#ifdef CONFIG_SDK_RTL9601B
            case RTL9601B_CHIP_ID:
                DIAG_UTIL_ERR_CHK(rtk_ponmac_queueDrainOut_set(&pon_queue), ret);
                break;
#endif

            default:
                DIAG_UTIL_ERR_CHK(rtk_ponmac_queueDrainOut_set(&pon_queue), ret);
                break;
        }
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_set_drain_out_t_cont_tcont_queue_id_qid */

/*
 * pon set drain-out t-cont <UINT:tcont>
 */
cparser_result_t
cparser_cmd_pon_set_drain_out_t_cont_tcont(
    cparser_context_t *context,
    uint32_t  *tcont_ptr,
    char * *qid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rtk_ponmac_queue_t pon_queue;

    DIAG_UTIL_PARAM_CHK();

    pon_queue.schedulerId = *tcont_ptr;
    pon_queue.queueId = 32;

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#ifdef CONFIG_SDK_RTL9601B
        case RTL9601B_CHIP_ID:
            DIAG_UTIL_ERR_CHK(rtk_ponmac_queueDrainOut_set(&pon_queue), ret);
            break;
#endif
        default:
            DIAG_UTIL_ERR_CHK(rtk_ponmac_queueDrainOut_set(&pon_queue), ret);
            break;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_set_drain_out_t_cont_tcont_queue_id_qid */


/*
 * pon get drain-out status
 */
cparser_result_t
cparser_cmd_pon_get_drain_out_status(
    cparser_context_t *context)
{
    uint32 state;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();


    switch(DIAG_UTIL_CHIP_TYPE)
    {
#ifdef CONFIG_SDK_APOLLO
        case APOLLO_CHIP_ID:
            DIAG_UTIL_ERR_CHK(apollo_raw_ponMacQueueDrainOutState_get((apollo_raw_ponmac_draintOutState_t *)&state), ret);
            break;
#endif
#ifdef CONFIG_SDK_APOLLOMP
        case APOLLOMP_CHIP_ID:
            DIAG_UTIL_ERR_CHK(apollomp_raw_ponMacQueueDrainOutState_get((apollomp_raw_ponmac_draintOutState_t *)&state), ret);
            break;
#endif
        default:
            diag_util_mprintf("%s\n",DIAG_STR_NOTSUPPORT);
            return CPARSER_NOT_OK;
            break;
    }
    diag_util_mprintf("drant out state:%d\n",state);

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_get_drain_out_status */

/*
 * pon get stream <MASK_LIST:sid>
 */
cparser_result_t
cparser_cmd_pon_get_stream_sid(
    cparser_context_t *context,
    char * *sid_ptr)
{
    diag_mask_t mask;
    int32 ret = RT_ERR_FAILED;
    uint32 sid;
    rtk_ponmac_queue_t queue;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK128(mask, 3), ret);
    DIAG_UTIL_MASK_SCAN(mask, sid)
    {
        ret = rtk_ponmac_flow2Queue_get(sid ,&queue);
        if(ret != RT_ERR_OK)
            continue;

        ret = rtk_ponmac_sidValid_get(sid, &enable);
        if(ret != RT_ERR_OK)
        {
            diag_util_mprintf("sid:%3d scheduler id:%3d queue id:%3d\n",sid,queue.schedulerId, queue.queueId);
        }
        else
        {
            diag_util_mprintf("sid:%3d scheduler id:%3d queue id:%3d state:%s\n",
                sid,queue.schedulerId, queue.queueId, (enable == ENABLED) ? "enable" : "disable");
        }
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_get_stream_sid */

/*
 * pon set stream <MASK_LIST:sid> t-cont <UINT:tcont> queue-id <UINT:qid>
 */
cparser_result_t
cparser_cmd_pon_set_stream_sid_t_cont_tcont_queue_id_qid(
    cparser_context_t *context,
    char * *sid_ptr,
    uint32_t  *tcont_ptr,
    uint32_t  *qid_ptr)
{
    diag_mask_t mask;
    uint32 sid;
    rtk_ponmac_queue_t queue;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    queue.schedulerId=*tcont_ptr;
    queue.queueId=*qid_ptr;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK128(mask, 3), ret);
    DIAG_UTIL_MASK_SCAN(mask, sid)
    {
        DIAG_UTIL_ERR_CHK(rtk_ponmac_flow2Queue_set(sid,&queue), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_set_stream_sid_t_cont_tcont_queue_id_qid */

/*
 * pon set stream <MASK_LIST:sid> llid <UINT:llid> queue-id <UINT:qid>
 */
cparser_result_t
cparser_cmd_pon_set_stream_sid_llid_llid_queue_id_qid(
    cparser_context_t *context,
    char * *sid_ptr,
    uint32_t  *llid_ptr,
    uint32_t  *qid_ptr)
{
    diag_mask_t mask;
    uint32 sid;
    rtk_ponmac_queue_t queue;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    queue.schedulerId=*llid_ptr;
    queue.queueId=*qid_ptr;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK128(mask, 3), ret);
    DIAG_UTIL_MASK_SCAN(mask, sid)
    {
        DIAG_UTIL_ERR_CHK(rtk_ponmac_flow2Queue_set(sid,&queue), ret);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_pon_set_stream_sid_llid_llid_queue_id_qid */

/*
 * pon set stream <MASK_LIST:sid> state ( enable | disable ) 
 */
cparser_result_t
cparser_cmd_pon_set_stream_sid_state_enable_disable(
    cparser_context_t *context,
    char * *sid_ptr)
{
    diag_mask_t mask;
    uint32 sid;
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if('e'==TOKEN_CHAR(5,0))
        enable = ENABLED;
    else
        enable = DISABLED;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK128(mask, 3), ret);
    DIAG_UTIL_MASK_SCAN(mask, sid)
    {
        DIAG_UTIL_ERR_CHK(rtk_ponmac_sidValid_set(sid, enable), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_set_stream_sid_state_enable_disable */

/*
 * pon get t-cont <UINT:tcont> queue-id <UINT:qid>
 */
cparser_result_t
cparser_cmd_pon_get_t_cont_tcont_queue_id_qid(
    cparser_context_t *context,
    uint32_t  *tcont_ptr,
    uint32_t  *qid_ptr)
{
    rtk_ponmac_queue_t queue;
    int32 ret = RT_ERR_FAILED;
    rtk_ponmac_queueCfg_t  queueCfg;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    queue.schedulerId=*tcont_ptr;
    queue.queueId=*qid_ptr;

    DIAG_UTIL_ERR_CHK(rtk_ponmac_queue_get(&queue,&queueCfg), ret);

    memcpy(&globalQueue,&queue, sizeof(rtk_ponmac_queue_t));
    memcpy(&globalQueueCfg,&queueCfg, sizeof(rtk_ponmac_queueCfg_t));


    diag_util_mprintf("CIR:%d\n",queueCfg.cir);
    diag_util_mprintf("PIR:%d\n",queueCfg.pir);
    diag_util_mprintf("queue Type:%s\n",diagStr_queueType[queueCfg.type]);
    diag_util_mprintf("WFQ weight:%d\n",queueCfg.weight);
    diag_util_mprintf("Egress Drop:%s\n",diagStr_enable[queueCfg.egrssDrop]);

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_get_t_cont_tcont_queue_id_qid */

/*
 * pon add t-cont <UINT:tcont> queue-id <UINT:qid>
 */
cparser_result_t
cparser_cmd_pon_add_t_cont_tcont_queue_id_qid(
    cparser_context_t *context,
    uint32_t  *tcont_ptr,
    uint32_t  *qid_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();


    if(globalQueue.schedulerId != *tcont_ptr ||  globalQueue.queueId != *qid_ptr)
    {
        memset(&globalQueueCfg,0x0, sizeof(rtk_ponmac_queueCfg_t));
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_ponmac_queue_add(&globalQueue,&globalQueueCfg), ret);


    return CPARSER_OK;
}    /* end of cparser_cmd_pon_add_t_cont_tcont_queue_id_qid */

/*
 * pon del t-cont <UINT:tcont> queue-id <UINT:qid>
 */
cparser_result_t
cparser_cmd_pon_del_t_cont_tcont_queue_id_qid(
    cparser_context_t *context,
    uint32_t  *tcont_ptr,
    uint32_t  *qid_ptr)
{
    rtk_ponmac_queue_t queue;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    queue.schedulerId=*tcont_ptr;
    queue.queueId=*qid_ptr;

    DIAG_UTIL_ERR_CHK(rtk_ponmac_queue_del(&queue), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_del_t_cont_tcont_queue_id_qid */

/*
 * pon set t-cont <UINT:tcont> queue-id <UINT:qid> pir rate <UINT:rate>
 */
cparser_result_t
cparser_cmd_pon_set_t_cont_tcont_queue_id_qid_pir_rate_rate(
    cparser_context_t *context,
    uint32_t  *tcont_ptr,
    uint32_t  *qid_ptr,
    uint32_t  *rate_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if(globalQueue.schedulerId != *tcont_ptr ||  globalQueue.queueId != *qid_ptr)
    {
        memset(&globalQueueCfg,0x0, sizeof(rtk_ponmac_queueCfg_t));
        globalQueue.schedulerId = *tcont_ptr;
        globalQueue.queueId = *qid_ptr;
    }

    globalQueueCfg.pir = *rate_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_set_t_cont_tcont_queue_id_qid_pir_rate_rate */

/*
 * pon set t-cont <UINT:tcont> queue-id <UINT:qid> cir rate <UINT:rate>
 */
cparser_result_t
cparser_cmd_pon_set_t_cont_tcont_queue_id_qid_cir_rate_rate(
    cparser_context_t *context,
    uint32_t  *tcont_ptr,
    uint32_t  *qid_ptr,
    uint32_t  *rate_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if(globalQueue.schedulerId != *tcont_ptr ||  globalQueue.queueId != *qid_ptr)
    {
        memset(&globalQueueCfg,0x0, sizeof(rtk_ponmac_queueCfg_t));
        globalQueue.schedulerId = *tcont_ptr;
        globalQueue.queueId = *qid_ptr;
    }

    globalQueueCfg.cir = *rate_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_set_t_cont_tcont_queue_id_qid_cir_rate_rate */

/*
 * pon set t-cont <UINT:tcont> queue-id <UINT:qid> scheduling type ( strict | wfq )
 */
cparser_result_t
cparser_cmd_pon_set_t_cont_tcont_queue_id_qid_scheduling_type_strict_wfq(
    cparser_context_t *context,
    uint32_t  *tcont_ptr,
    uint32_t  *qid_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if(globalQueue.schedulerId != *tcont_ptr ||  globalQueue.queueId != *qid_ptr)
    {
        memset(&globalQueueCfg,0x0, sizeof(rtk_ponmac_queueCfg_t));
        globalQueue.schedulerId = *tcont_ptr;
        globalQueue.queueId = *qid_ptr;
    }

    if('s'==TOKEN_CHAR(8,0))
        globalQueueCfg.type = STRICT_PRIORITY;
    else
        globalQueueCfg.type = WFQ_WRR_PRIORITY;

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_set_t_cont_tcont_queue_id_qid_scheduling_type_strict_wfq */

/*
 * pon set t-cont <UINT:tcont> queue-id <UINT:qid> scheduling weight <UINT:weight>
 */
cparser_result_t
cparser_cmd_pon_set_t_cont_tcont_queue_id_qid_scheduling_weight_weight(
    cparser_context_t *context,
    uint32_t  *tcont_ptr,
    uint32_t  *qid_ptr,
    uint32_t  *weight_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if(globalQueue.schedulerId != *tcont_ptr ||  globalQueue.queueId != *qid_ptr)
    {
        memset(&globalQueueCfg,0x0, sizeof(rtk_ponmac_queueCfg_t));
        globalQueue.schedulerId = *tcont_ptr;
        globalQueue.queueId = *qid_ptr;
    }
    globalQueueCfg.weight = *weight_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_set_t_cont_tcont_queue_id_qid_scheduling_weight_weight */

/*
 * pon set t-cont <UINT:tcont> queue-id <UINT:qid> egress-drop state ( enable | disable )
 */
cparser_result_t
cparser_cmd_pon_set_t_cont_tcont_queue_id_qid_egress_drop_state_enable_disable(
    cparser_context_t *context,
    uint32_t  *tcont_ptr,
    uint32_t  *qid_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if(globalQueue.schedulerId != *tcont_ptr ||  globalQueue.queueId != *qid_ptr)
    {
        memset(&globalQueueCfg,0x0, sizeof(rtk_ponmac_queueCfg_t));
        globalQueue.schedulerId = *tcont_ptr;
        globalQueue.queueId = *qid_ptr;
    }

    if('e'==TOKEN_CHAR(8,0))
        globalQueueCfg.egrssDrop = ENABLED;
    else
        globalQueueCfg.egrssDrop = DISABLED;

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_set_t_cont_tcont_queue_id_qid_egress_drop_state_enable_disable */

/*
 * pon get llid <UINT:llid> queue-id <UINT:qid>
 */
cparser_result_t
cparser_cmd_pon_get_llid_llid_queue_id_qid(
    cparser_context_t *context,
    uint32_t  *llid_ptr,
    uint32_t  *qid_ptr)
{
    rtk_ponmac_queue_t queue;
    int32 ret = RT_ERR_FAILED;
    rtk_ponmac_queueCfg_t  queueCfg;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    queue.schedulerId=*llid_ptr;
    queue.queueId=*qid_ptr;

    DIAG_UTIL_ERR_CHK(rtk_ponmac_queue_get(&queue,&queueCfg), ret);

    memcpy(&globalQueue,&queue, sizeof(rtk_ponmac_queue_t));
    memcpy(&globalQueueCfg,&queueCfg, sizeof(rtk_ponmac_queueCfg_t));


    diag_util_mprintf("CIR:%d\n",queueCfg.cir);
    diag_util_mprintf("PIR:%d\n",queueCfg.pir);
    diag_util_mprintf("queue Type:%s\n",diagStr_queueType[queueCfg.type]);
    diag_util_mprintf("WFQ weight:%d\n",queueCfg.weight);
    diag_util_mprintf("Egress Drop:%s\n",diagStr_enable[queueCfg.egrssDrop]);

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_get_llid_llid_queue_id_qid */

/*
 * pon add llid <UINT:llid> queue-id <UINT:qid>
 */
cparser_result_t
cparser_cmd_pon_add_llid_llid_queue_id_qid(
    cparser_context_t *context,
    uint32_t  *llid_ptr,
    uint32_t  *qid_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if(globalQueue.schedulerId != *llid_ptr ||  globalQueue.queueId != *qid_ptr)
    {
        memset(&globalQueueCfg,0x0, sizeof(rtk_ponmac_queueCfg_t));
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_ponmac_queue_add(&globalQueue,&globalQueueCfg), ret);


    return CPARSER_OK;
}    /* end of cparser_cmd_pon_add_llid_llid_queue_id_qid */

/*
 * pon del llid <UINT:llid> queue-id <UINT:qid>
 */
cparser_result_t
cparser_cmd_pon_del_llid_llid_queue_id_qid(
    cparser_context_t *context,
    uint32_t  *llid_ptr,
    uint32_t  *qid_ptr)
{
    rtk_ponmac_queue_t queue;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    queue.schedulerId=*llid_ptr;
    queue.queueId=*qid_ptr;

    DIAG_UTIL_ERR_CHK(rtk_ponmac_queue_del(&queue), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_del_llid_llid_queue_id_qid */

/*
 * pon set llid <UINT:llid> queue-id <UINT:qid> pir rate <UINT:rate>
 */
cparser_result_t
cparser_cmd_pon_set_llid_llid_queue_id_qid_pir_rate_rate(
    cparser_context_t *context,
    uint32_t  *llid_ptr,
    uint32_t  *qid_ptr,
    uint32_t  *rate_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if(globalQueue.schedulerId != *llid_ptr ||  globalQueue.queueId != *qid_ptr)
    {
        memset(&globalQueueCfg,0x0, sizeof(rtk_ponmac_queueCfg_t));
        globalQueue.schedulerId = *llid_ptr;
        globalQueue.queueId = *qid_ptr;
    }

    globalQueueCfg.pir = *rate_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_set_llid_llid_queue_id_qid_pir_rate_rate */

/*
 * pon set llid <UINT:llid> queue-id <UINT:qid> cir rate <UINT:rate>
 */
cparser_result_t
cparser_cmd_pon_set_llid_llid_queue_id_qid_cir_rate_rate(
    cparser_context_t *context,
    uint32_t  *llid_ptr,
    uint32_t  *qid_ptr,
    uint32_t  *rate_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if(globalQueue.schedulerId != *llid_ptr ||  globalQueue.queueId != *qid_ptr)
    {
        memset(&globalQueueCfg,0x0, sizeof(rtk_ponmac_queueCfg_t));
        globalQueue.schedulerId = *llid_ptr;
        globalQueue.queueId = *qid_ptr;
    }

    globalQueueCfg.cir = *rate_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_pon_set_llid_llid_queue_id_qid_cir_rate_rate */

/*
 * pon set llid <UINT:llid> queue-id <UINT:qid> scheduling type ( strict | wfq )
 */
cparser_result_t
cparser_cmd_pon_set_llid_llid_queue_id_qid_scheduling_type_strict_wfq(
    cparser_context_t *context,
    uint32_t  *llid_ptr,
    uint32_t  *qid_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if(globalQueue.schedulerId != *llid_ptr ||  globalQueue.queueId != *qid_ptr)
    {
        memset(&globalQueueCfg,0x0, sizeof(rtk_ponmac_queueCfg_t));
        globalQueue.schedulerId = *llid_ptr;
        globalQueue.queueId = *qid_ptr;
    }

    if('s'==TOKEN_CHAR(8,0))
        globalQueueCfg.type = STRICT_PRIORITY;
    else
        globalQueueCfg.type = WFQ_WRR_PRIORITY;

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_set_llid_llid_queue_id_qid_scheduling_type_strict_wfq */

/*
 * pon set llid <UINT:llid> queue-id <UINT:qid> scheduling weight <UINT:weight>
 */
cparser_result_t
cparser_cmd_pon_set_llid_llid_queue_id_qid_scheduling_weight_weight(
    cparser_context_t *context,
    uint32_t  *llid_ptr,
    uint32_t  *qid_ptr,
    uint32_t  *weight_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if(globalQueue.schedulerId != *llid_ptr ||  globalQueue.queueId != *qid_ptr)
    {
        memset(&globalQueueCfg,0x0, sizeof(rtk_ponmac_queueCfg_t));
        globalQueue.schedulerId = *llid_ptr;
        globalQueue.queueId = *qid_ptr;
    }
    globalQueueCfg.weight = *weight_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_pon_set_llid_llid_queue_id_qid_scheduling_weight_weight */

/*
 * pon set llid <UINT:llid> queue-id <UINT:qid> egress-drop state ( enable | disable )
 */
cparser_result_t
cparser_cmd_pon_set_llid_llid_queue_id_qid_egress_drop_state_enable_disable(
    cparser_context_t *context,
    uint32_t  *llid_ptr,
    uint32_t  *qid_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if(globalQueue.schedulerId != *llid_ptr ||  globalQueue.queueId != *qid_ptr)
    {
        memset(&globalQueueCfg,0x0, sizeof(rtk_ponmac_queueCfg_t));
        globalQueue.schedulerId = *llid_ptr;
        globalQueue.queueId = *qid_ptr;
    }

    if('e'==TOKEN_CHAR(8,0))
        globalQueueCfg.egrssDrop = ENABLED;
    else
        globalQueueCfg.egrssDrop = DISABLED;
    return CPARSER_OK;
}    /* end of cparser_cmd_pon_set_llid_llid_queue_id_qid_egress_drop_state_enable_disable */



static void __pon_mac_scheduler_queue_list_get(int32 isTcont, uint32 scheduleId)
{
    int32 ret;
    uint32 qmap,regIndex;
    uint32 queue,queueOffset,queueBase;;


    switch(DIAG_UTIL_CHIP_TYPE)
    {
#ifdef CONFIG_SDK_APOLLO
        case APOLLO_CHIP_ID:
            if(isTcont == 1)
                regIndex = scheduleId;
            else
                regIndex = (scheduleId/2)*8 + (scheduleId%2);

            reg_array_read(PON_SCH_QMAPr,REG_ARRAY_INDEX_NONE, regIndex, &qmap);
            break;
#endif
#ifdef CONFIG_SDK_APOLLOMP
        case APOLLOMP_CHIP_ID:
            if(isTcont == 1)
                regIndex = scheduleId;
            else
                regIndex = (scheduleId/2)*8 + (scheduleId%2);

            reg_array_read(APOLLOMP_PON_SCH_QMAPr,REG_ARRAY_INDEX_NONE, regIndex, &qmap);

            if(isTcont == 1)
                queueOffset = scheduleId/8;
            else
                queueOffset = scheduleId/2;

            break;
#endif
#ifdef CONFIG_SDK_RTL9601B
        case RTL9601B_CHIP_ID:
            regIndex = scheduleId;
            if(scheduleId < 8)
            {
                reg_array_read(RTL9601B_PON_SCH_QMAPr,REG_ARRAY_INDEX_NONE, regIndex, &qmap);
            }
            else
            {
                diag_util_mprintf("9601B can not set Tcont 8\n");
                return;
            }
            queueOffset = 0;
            break;
#endif
#ifdef CONFIG_SDK_RTL9602C
        case RTL9602C_CHIP_ID:

            regIndex = scheduleId;

            if(scheduleId < 17)
            {
                reg_array_read(RTL9602C_PON_SCH_QMAPr,REG_ARRAY_INDEX_NONE, regIndex, &qmap);
            }
            else
            {
                diag_util_mprintf("9602C can not set Tcont more thean 16\n");
                return;
            }

            if(isTcont == 1)
                queueOffset = scheduleId/8;
            else
                queueOffset = scheduleId/4;

            break;
#endif
        default:
            diag_util_mprintf("%s\n",DIAG_STR_NOTSUPPORT);
            return;
            break;
    }
    queueBase = queueOffset*32;

    for(queue=0;queue<32;queue++)
    {
        diag_util_printf("queue %-3d:",queueBase+queue);
        if(qmap&(1<<queue))
            diag_util_printf("O\n");
        else
            diag_util_printf("X\n");
    }
    return;
}


/*
 * pon get t-cont <UINT:tcont> queue-list
 */
cparser_result_t
cparser_cmd_pon_get_t_cont_tcont_queue_list(
    cparser_context_t *context,
    uint32_t  *tcont_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if(*tcont_ptr >=32)
        diag_util_mprintf("T-CONT out of range\n");

    __pon_mac_scheduler_queue_list_get(1,*tcont_ptr);

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_get_t_cont_tcont_queue_list */


/*
 * pon get t-cont <UINT:tcont>
 */
cparser_result_t
cparser_cmd_pon_get_t_cont_tcont(
    cparser_context_t *context,
    uint32_t  *tcont_ptr)
{
    diag_mask_t mask;
    int32 ret = RT_ERR_FAILED;
    uint32 index;
	uint32 qmap,regIndex;
	uint32 queue,queueOffset,queueBase;;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if(*tcont_ptr >=32)
        diag_util_mprintf("T-CONT out of range\n");

	index = *tcont_ptr;

	diag_util_printf("Tcont QID0 1 2 3 4 5 6 7 8 9 10        15        20        25        30\n");

	switch(DIAG_UTIL_CHIP_TYPE)
	{
#ifdef CONFIG_SDK_APOLLO
		case APOLLO_CHIP_ID:
			reg_array_read(PON_SCH_QMAPr,REG_ARRAY_INDEX_NONE, index, &qmap);
			break;
#endif
#ifdef CONFIG_SDK_APOLLOMP
		case APOLLOMP_CHIP_ID:
			reg_array_read(APOLLOMP_PON_SCH_QMAPr,REG_ARRAY_INDEX_NONE, index, &qmap);
			break;
#endif
#ifdef CONFIG_SDK_RTL9601B
		case RTL9601B_CHIP_ID:
			if(index < 8)
			{
				reg_array_read(RTL9601B_PON_SCH_QMAPr,REG_ARRAY_INDEX_NONE, index, &qmap);
			}
			else
			{
				diag_util_mprintf("9601B can not set Tcont 8\n");
				return ret;
			}
			break;
#endif
#ifdef CONFIG_SDK_RTL9602C
		case RTL9602C_CHIP_ID:
			if(index < 17)
			{
				reg_array_read(RTL9602C_PON_SCH_QMAPr,REG_ARRAY_INDEX_NONE, index, &qmap);
			}
			else
			{
				diag_util_mprintf("9602C can not set Tcont more than 16\n");
				return ret;
			}
			break;
#endif
		default:
			diag_util_mprintf("%s\n",DIAG_STR_NOTSUPPORT);
			return ret;
			break;
	}

	diag_util_printf("%-9d",index);

	queueOffset = index/8;

	queueBase = queueOffset*32;

	for(queue=0;queue<32;queue++)
	{

		if(qmap&(1<<queue))
			diag_util_printf("O ");
		else
			diag_util_printf("X ");
	}
	diag_util_printf("\n");


    return CPARSER_OK;
}    /* end of cparser_cmd_pon_get_t_cont_tcont */





/*
 * pon get llid <UINT:llid> queue-list
 */
cparser_result_t
cparser_cmd_pon_get_llid_llid_queue_list(
    cparser_context_t *context,
    uint32_t  *llid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if(*llid_ptr >=8)
        diag_util_mprintf("LLID out of range\n");

    __pon_mac_scheduler_queue_list_get(0, *llid_ptr);

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_get_llid_llid_queue_list */


/*
 * pon reset
 */
cparser_result_t
cparser_cmd_pon_reset(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 queueId;
    rtk_ponmac_queue_t logicalQueue;

    DIAG_UTIL_PARAM_CHK();

    /*init pon module*/
    DIAG_UTIL_ERR_CHK(rtk_ponmac_init(), ret);

    /*remove queue 0~7 from schedule id 0*/
    logicalQueue.schedulerId = 0 ;
    for(queueId = 0 ; queueId <= RTK_DOT1P_PRIORITY_MAX ; queueId++ )
    {
        logicalQueue.queueId = queueId;
        DIAG_UTIL_ERR_CHK(rtk_ponmac_queue_del(&logicalQueue), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_reset */

/*
* pon get transceiver ( vendor-name | part-number | temperature | voltage | bias-current | tx-power | rx-power )
*/
cparser_result_t
cparser_cmd_pon_get_transceiver_vendor_name_part_number_temperature_voltage_bias_current_tx_power_rx_power(
    cparser_context_t *context)
{
    rtk_transceiver_data_t dataCfg, readableCfg;
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if(0 == osal_strcmp(TOKEN_STR(3), "vendor-name"))
    {
    	DIAG_UTIL_ERR_CHK(rtk_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_VENDOR_NAME, &dataCfg), ret);
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_VENDOR_NAME, &dataCfg, &readableCfg);
	diag_util_mprintf("Vendor Name: ");
    }else if (0 == osal_strcmp(TOKEN_STR(3), "part-number"))
    {
    	DIAG_UTIL_ERR_CHK(rtk_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_VENDOR_PART_NUM, &dataCfg), ret);
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_VENDOR_PART_NUM, &dataCfg, &readableCfg);
	diag_util_mprintf("Part Number: ");
    }else if (0 == osal_strcmp(TOKEN_STR(3), "temperature"))
    {
    	DIAG_UTIL_ERR_CHK(rtk_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_TEMPERATURE, &dataCfg), ret);
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_TEMPERATURE, &dataCfg, &readableCfg);
	diag_util_mprintf("Temperature: ");
    }else if (0 == osal_strcmp(TOKEN_STR(3), "voltage"))
    {
    	DIAG_UTIL_ERR_CHK(rtk_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_VOLTAGE, &dataCfg), ret);
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_VOLTAGE, &dataCfg, &readableCfg);
	diag_util_mprintf("Voltage: ");
    }else if (0 == osal_strcmp(TOKEN_STR(3), "bias-current"))
    {
    	DIAG_UTIL_ERR_CHK(rtk_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_BIAS_CURRENT, &dataCfg), ret);
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_BIAS_CURRENT, &dataCfg, &readableCfg);
	diag_util_mprintf("Bias Current: ");
    }else if (0 == osal_strcmp(TOKEN_STR(3), "tx-power"))
    {
    	DIAG_UTIL_ERR_CHK(rtk_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_TX_POWER, &dataCfg), ret);
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_TX_POWER, &dataCfg, &readableCfg);
	diag_util_mprintf("Tx Power: ");
    }else if (0 == osal_strcmp(TOKEN_STR(3), "rx-power"))
    {
    	DIAG_UTIL_ERR_CHK(rtk_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_RX_POWER, &dataCfg), ret);
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_RX_POWER, &dataCfg, &readableCfg);
	diag_util_mprintf("Rx Power: ");
    }
	

    diag_util_mprintf("%s\n", readableCfg.buf);
    return CPARSER_OK;
}    /* end of cparser_cmd_pon_get_transceiver_type_type */

/*
 * pon set scheduling-type ( wfq | wrr )
 */
cparser_result_t
cparser_cmd_pon_set_scheduling_type_wfq_wrr(
    cparser_context_t *context)
{
    rtk_qos_scheduling_type_t type;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if ('r' == TOKEN_CHAR(3,1))
        type = RTK_QOS_WRR;  /*wrr*/
    else
        type = RTK_QOS_WFQ;  /*wfq*/

    DIAG_UTIL_ERR_CHK(rtk_ponmac_schedulingType_set(type), ret);


    return CPARSER_OK;
}    /* end of cparser_cmd_pon_set_scheduling_type_wfq_wrr */

/*
 * pon get scheduling-type
 */
cparser_result_t
cparser_cmd_pon_get_scheduling_type(
    cparser_context_t *context)
{
    rtk_qos_scheduling_type_t type;
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_ponmac_schedulingType_get(&type), ret);
    
    diag_util_mprintf("schedule type:");

    if(RTK_QOS_WRR==type)
        diag_util_mprintf("WRR\n");
    else
        diag_util_mprintf("WFQ\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_get_scheduling_type */

/*
 * pon set schedule-id <UINT:id> rate <UINT:rate>
 */
cparser_result_t
cparser_cmd_pon_set_schedule_id_id_rate_rate(
    cparser_context_t *context,
    uint32_t  *id_ptr,
    uint32_t  *rate_ptr)
{
    int32 ret = RT_ERR_FAILED;
 
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_ponmac_egrScheduleIdRate_set(*id_ptr,*rate_ptr), ret);
  

    return CPARSER_OK;
}    /* end of cparser_cmd_pon_set_schedule_id_id_rate_rate */

/*
 * pon get schedule-id <UINT:id> rate
 */
cparser_result_t
cparser_cmd_pon_get_schedule_id_id_rate(
    cparser_context_t *context,
    uint32_t  *id_ptr)
{
    int32 ret = RT_ERR_FAILED;
    uint32 rate; 
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_ponmac_egrScheduleIdRate_get(*id_ptr, &rate), ret);
    
    diag_util_mprintf("egress rate: %d\n",rate);
    
    return CPARSER_OK;
}    /* end of cparser_cmd_pon_get_schedule_id_id_rate */

/*
 * pon set egress rate <UINT:rate>
 */
cparser_result_t
cparser_cmd_pon_set_egress_rate_rate(
    cparser_context_t *context,
    uint32_t  *rate_ptr)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(rtk_ponmac_egrBandwidthCtrlRate_set(*rate_ptr), ret);
  
    return CPARSER_OK;
}    /* end of cparser_cmd_pon_set_egress_rate_rate */

/*
 * pon get egress rate
 */
cparser_result_t
cparser_cmd_pon_get_egress_rate(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 rate;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_ponmac_egrBandwidthCtrlRate_get(&rate), ret);
    
    diag_util_mprintf("egress rate: %d\n",rate);
    
    return CPARSER_OK;
}    /* end of cparser_cmd_pon_get_egress_rate */

