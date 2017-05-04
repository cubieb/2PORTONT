#ifndef __RTK_API_QOS_H__
#define __RTK_API_QOS_H__

#include "rtk_api_common.h"
#include "rtk_types_common.h"
#include "rtk_error.h"

#define RTK_MAX_NUM_OF_PRIORITY                   8
#define RTK_MAX_NUM_OF_QUEUE                      8
#define RTK_MAX_NUM_OF_PORT                     8
#define QOS_WEIGHT_MAX							128



typedef struct rtk_priority_select_s
{
    rtk_uint32 port_pri;
    rtk_uint32 dot1q_pri;
    rtk_uint32 acl_pri;
    rtk_uint32 dscp_pri;
    rtk_uint32 cvlan_pri;
    rtk_uint32 svlan_pri;
    rtk_uint32 dmac_pri;
    rtk_uint32 smac_pri;
} rtk_priority_select_t;




typedef struct rtk_qos_pri2queue_s
{
    rtk_uint32 pri2queue[RTK_MAX_NUM_OF_PRIORITY];
} rtk_qos_pri2queue_t;

typedef struct rtk_qos_queue_weights_s
{
    rtk_uint32 weights[RTK_MAX_NUM_OF_QUEUE];
} rtk_qos_queue_weights_t;

typedef enum rtk_qos_scheduling_type_e
{
    WFQ = 0,        /* Weighted-Fair-Queue */
    WRR,            /* Weighted-Round-Robin */
    SCHEDULING_TYPE_END
} rtk_qos_scheduling_type_t;




typedef rtk_uint32  rtk_qid_t;        /* queue id type */
typedef rtk_uint32  rtk_port_t;        /* port is type */
typedef rtk_uint32  rtk_pri_t;         /* priority vlaue */
typedef rtk_uint32  rtk_queue_num_t;    /* queue number*/
typedef rtk_uint32  rtk_rate_t;     /* rate type  */


rtk_api_ret_t rtk_qos_init(rtk_queue_num_t queueNum);
rtk_api_ret_t rtk_qos_schedulingQueue_set(rtk_port_t port, rtk_qos_queue_weights_t *pQweights);
rtk_api_ret_t rtk_qos_portPri_set(rtk_port_t port, rtk_pri_t int_pri);
rtk_api_ret_t rtk_qos_portPri_get(rtk_port_t port, rtk_pri_t *pInt_pri);
ret_t rtl8367b_getAsicOutputQueueMappingIndex(rtk_uint32 port, rtk_uint32 *pIndex );
rtk_api_ret_t rtk_qos_schedulingQueue_get(rtk_port_t port, rtk_qos_queue_weights_t *pQweights);
rtk_api_ret_t rtk_qos_priSel_set(rtk_priority_select_t *pPriDec);
rtk_api_ret_t rtk_qos_priMap_set(rtk_queue_num_t queue_num, rtk_qos_pri2queue_t *pPri2qid);
rtk_api_ret_t rtk_qos_priMap_get(rtk_queue_num_t queue_num, rtk_qos_pri2queue_t *pPri2qid);
rtk_api_ret_t rtk_rate_egrBandwidthCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pIfg_include);

#endif
