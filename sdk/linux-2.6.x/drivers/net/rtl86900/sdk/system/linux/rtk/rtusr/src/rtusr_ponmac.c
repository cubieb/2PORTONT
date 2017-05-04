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
 * $Revision: 61016 $
 * $Date: 2015-08-13 10:51:38 +0800 (Thu, 13 Aug 2015) $
 *
 * Purpose : Definition of PON MAC API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) queue configuration (PIR/CIR/Queue schuedule type)
 *           (2) flow and queue mapping
 */



/*
 * Include Files
 */
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <osal/lib.h>
#include <rtk/rtusr/include/rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>
#include <common/error.h>
#include <common/rt_type.h>
#include <rtk/ponmac.h>
#include <rtk/qos.h>

#include <math.h>
/*
 * Symbol Definition
 */

/*
 * Macro Declaration
 */


/* Module Name    : PON Mac                                  */
/* Sub-module Name: flow/t-cont/queue mapping */

/* Function Name:
 *      rtk_ponmac_init
 * Description:
 *      Configure PON MAC initial settings
 * Input:
 *      None.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
int32
rtk_ponmac_init(void)
{
    rtdrv_ponmacCfg_t ponmac_cfg;

    /* function body */
    SETSOCKOPT(RTDRV_PONMAC_INIT, &ponmac_cfg, rtdrv_ponmacCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_ponmac_init */


/* Function Name:
 *      rtk_ponmac_queue_add
 * Description:
 *      Add queue to given scheduler id and apply queue setting
 * Input:
 *      pQueue         - queue id and scheduler id for ths queue.
 *      pQueueCfg     - queue configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              					- OK
 *      RT_ERR_FAILED          					- Failed
 *      RT_ERR_NULL_POINTER    					- Pointer pQueueList point to NULL.
 *      RT_ERR_INPUT 							- Invalid input parameters.
 * Note:
 *      None
 */
int32
rtk_ponmac_queue_add(rtk_ponmac_queue_t *pQueue, rtk_ponmac_queueCfg_t *pQueueCfg)
{
    rtdrv_ponmacCfg_t ponmac_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pQueue), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pQueueCfg), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&ponmac_cfg.queue, pQueue, sizeof(rtk_ponmac_queue_t));
    osal_memcpy(&ponmac_cfg.queueCfg, pQueueCfg, sizeof(rtk_ponmac_queueCfg_t));
    SETSOCKOPT(RTDRV_PONMAC_QUEUE_ADD, &ponmac_cfg, rtdrv_ponmacCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_ponmac_queue_add */


/* Function Name:
 *      rtk_ponmac_queue_get
 * Description:
 *      get queue setting
 * Input:
 *      pQueue         - queue id and scheduler id for ths queue.
 * Output:
 *      pQueueCfg     - queue configuration
 * Return:
 *      RT_ERR_OK              					- OK
 *      RT_ERR_FAILED          					- Failed
 *      RT_ERR_NULL_POINTER    					- Pointer pQueueList point to NULL.
 *      RT_ERR_INPUT 							- Invalid input parameters.
 * Note:
 *      None
 */
int32
rtk_ponmac_queue_get(rtk_ponmac_queue_t *pQueue, rtk_ponmac_queueCfg_t *pQueueCfg)
{
    rtdrv_ponmacCfg_t ponmac_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pQueue), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pQueueCfg), RT_ERR_NULL_POINTER);

    /* function body */

    osal_memcpy(&ponmac_cfg.queue, pQueue, sizeof(rtk_ponmac_queue_t));
    osal_memcpy(&ponmac_cfg.queueCfg, pQueueCfg, sizeof(rtk_ponmac_queueCfg_t));

    GETSOCKOPT(RTDRV_PONMAC_QUEUE_GET, &ponmac_cfg, rtdrv_ponmacCfg_t, 1);
    osal_memcpy(pQueueCfg, &ponmac_cfg.queueCfg, sizeof(rtk_ponmac_queueCfg_t));

    return RT_ERR_OK;
}   /* end of rtk_ponmac_queue_get */


/* Function Name:
 *      rtk_ponmac_queue_del
 * Description:
 *      delete queue from given scheduler id
 * Input:
 *      pQueue         - queue id and scheduler id for ths queue.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              					- OK
 *      RT_ERR_FAILED          					- Failed
 *      RT_ERR_INPUT 							- Invalid input parameters.
 * Note:
 *      None
 */
int32
rtk_ponmac_queue_del(rtk_ponmac_queue_t *pQueue)
{
    rtdrv_ponmacCfg_t ponmac_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pQueue), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&ponmac_cfg.queue, pQueue, sizeof(rtk_ponmac_queue_t));
    SETSOCKOPT(RTDRV_PONMAC_QUEUE_DEL, &ponmac_cfg, rtdrv_ponmacCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_ponmac_queue_del */



/* Function Name:
 *      rtk_ponmac_flow2Queue_set
 * Description:
 *      mapping flow to given queue
 * Input:
 *      flow          - flow id.
 *      pQueue       - queue id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              					- OK
 *      RT_ERR_FAILED          					- Failed
 *      RT_ERR_INPUT 							- Invalid input parameters.
 * Note:
 *      None
 */
int32
rtk_ponmac_flow2Queue_set(uint32  flow, rtk_ponmac_queue_t *pQueue)
{
    rtdrv_ponmacCfg_t ponmac_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pQueue), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&ponmac_cfg.flow, &flow, sizeof(uint32));
    osal_memcpy(&ponmac_cfg.queue, pQueue, sizeof(rtk_ponmac_queue_t));
    SETSOCKOPT(RTDRV_PONMAC_FLOW2QUEUE_SET, &ponmac_cfg, rtdrv_ponmacCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_ponmac_flow2Queue_set */


/* Function Name:
 *      rtk_ponmac_flow2Queue_get
 * Description:
 *      get queue id for this flow
 * Input:
 *      flow         - flow id.
 * Output:
 *      pQueue       - queue id.
 * Return:
 *      RT_ERR_OK              					- OK
 *      RT_ERR_FAILED          					- Failed
 *      RT_ERR_NULL_POINTER    					- Pointer pQueue point to NULL.
 *      RT_ERR_INPUT 							- Invalid input parameters.
 * Note:
 *      None
 */
int32
rtk_ponmac_flow2Queue_get(uint32  flow, rtk_ponmac_queue_t *pQueue)
{
    rtdrv_ponmacCfg_t ponmac_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pQueue), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&ponmac_cfg.flow, &flow, sizeof(uint32));
    GETSOCKOPT(RTDRV_PONMAC_FLOW2QUEUE_GET, &ponmac_cfg, rtdrv_ponmacCfg_t, 1);
    osal_memcpy(pQueue, &ponmac_cfg.queue, sizeof(rtk_ponmac_queue_t));

    return RT_ERR_OK;
}   /* end of rtk_ponmac_flow2Queue_get */


/* Function Name:
 *      rtk_ponmac_queueDrainOut_set
 * Description:
 *      Set pon queue drain out.
 * Input:
 *      pQueue - Specified which PON queue will be drain out
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
int32
rtk_ponmac_queueDrainOut_set(rtk_ponmac_queue_t *pQueue)
{
    rtdrv_ponmacCfg_t ponmac_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pQueue), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&ponmac_cfg.queue, pQueue, sizeof(rtk_ponmac_queue_t));
    SETSOCKOPT(RTDRV_PONMAC_QUEUEDRAINOUT_SET, &ponmac_cfg, rtdrv_ponmacCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_ponmac_queueDrainOut_set */

#if 0
static void
_get_data_by_type(rtk_transceiver_parameter_type_t type, rtk_transceiver_data_t *pSrcData, rtk_transceiver_data_t *pDstData)
{
    double tmp = 0;
#ifdef DO_NOT_USE_LOG10
    switch(type)
    {
		case RTK_TRANSCEIVER_PARA_TYPE_VENDOR_NAME:
		case RTK_TRANSCEIVER_PARA_TYPE_VENDOR_PART_NUM:
			pSrcData->buf[TRANSCEIVER_LEN-1]='\0';
			break;
		default:
			break;
	}
	osal_memcpy(pDstData, pSrcData, sizeof(rtk_transceiver_data_t));
#else
    switch(type)
    {
	case RTK_TRANSCEIVER_PARA_TYPE_VENDOR_NAME:
	case RTK_TRANSCEIVER_PARA_TYPE_VENDOR_PART_NUM:
		pSrcData->buf[TRANSCEIVER_LEN-1]='\0';
		osal_memcpy(pDstData, pSrcData, sizeof(rtk_transceiver_data_t));
		break;
	case RTK_TRANSCEIVER_PARA_TYPE_TEMPERATURE:
		if (128 >= pSrcData->buf[0]) //MSB: pSrcData->buf[0]; LSB: pSrcData->buf[1]
		{
			tmp = (-1)*((~(pSrcData->buf[0]))+1)+((double)(pSrcData->buf[1])*1/256);
			snprintf(pDstData->buf, sizeof(rtk_transceiver_data_t), "%f C", tmp);
		}else{
			tmp = pSrcData->buf[0]+((double)(pSrcData->buf[1])*1/256);
			snprintf(pDstData->buf, sizeof(rtk_transceiver_data_t), "%f C", tmp);
		}
		break;
	case RTK_TRANSCEIVER_PARA_TYPE_VOLTAGE:
		tmp = (double)((pSrcData->buf[0] << 8) | pSrcData->buf[1])*1/10000;
		snprintf(pDstData->buf, sizeof(rtk_transceiver_data_t), "%f V", tmp);
		break;
	case RTK_TRANSCEIVER_PARA_TYPE_TX_POWER:
	case RTK_TRANSCEIVER_PARA_TYPE_RX_POWER:
		tmp = log10(((double)((pSrcData->buf[0] << 8) | pSrcData->buf[1])*1/10000))*10;
		snprintf(pDstData->buf, sizeof(rtk_transceiver_data_t), "%f  dBm", tmp);
		break;
	}
#endif
	return;
}
#endif
/* Function Name:
*      rtk_ponmac_transceiver_get
* Description:
*      Get transceiver value by the specific type
* Input:
*      type            - the transceiver parameter type
* Output:
*      pPara           - the pointer of data for the specific transceiver parameter
* Return:
*      RT_ERR_OK       - successful
*      others          - fail
* Note:
*
*/
int32
rtk_ponmac_transceiver_get(rtk_transceiver_parameter_type_t type, rtk_transceiver_data_t *pData)
{
    rtdrv_ponmacCfg_t ponmac_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memset(&ponmac_cfg, 0, sizeof(rtdrv_ponmacCfg_t));
    osal_memcpy(&ponmac_cfg.type, &type, sizeof(rtk_transceiver_parameter_type_t));
    GETSOCKOPT(RTDRV_PONMAC_TRANSCEIVER_GET, &ponmac_cfg, rtdrv_ponmacCfg_t, 1);
    osal_memcpy(pData, &ponmac_cfg.data, sizeof(rtk_transceiver_data_t));

    return RT_ERR_OK;
}   /* end of rtk_transceiver_get */

/* Function Name:
 *      rtk_ponmac_mode_set
 * Description:
 *      set pon mac mode
 * Input:
 *      mode         - pon mode, epon or gpon
 *      state 	     - disable or enable
 * Output:
 *
 * Return:
 *      RT_ERR_OK              						- OK
 *      RT_ERR_FAILED          					- Failed
 *      RT_ERR_NULL_POINTER    					- Pointer pQueue point to NULL.
 *      RT_ERR_INPUT 							- Invalid input parameters.
 * Note:
 *      None
 */
int32
rtk_ponmac_mode_set(rtk_ponmac_mode_t mode)
{
    rtdrv_ponmacCfg_t mode_cfg;

    /* parameter check */
    RT_PARAM_CHK((mode >= PONMAC_MODE_END), RT_ERR_INPUT);

    /* function body */
    mode_cfg.mode = mode;
    SETSOCKOPT(RTDRV_PONMAC_MODE_SET, &mode_cfg, rtdrv_ponmacCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_ponmac_mode_set */


/* Function Name:
 *      rtk_ponmac_mode_get
 * Description:
 *      set pon mac mode
 * Input:
 *      mode         - pon mode, epon or gpon
 * Output:
 *      *state 	     - point of state, enable or disable
 * Return:
 *      RT_ERR_OK              						- OK
 *      RT_ERR_FAILED          					- Failed
 *      RT_ERR_NULL_POINTER    					- Pointer pQueue point to NULL.
 *      RT_ERR_INPUT 							- Invalid input parameters.
 * Note:
 *      None
 */
int32
rtk_ponmac_mode_get(rtk_ponmac_mode_t *pMode)
{
    rtdrv_ponmacCfg_t mode_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);

    /* function body */
    GETSOCKOPT(RTDRV_PONMAC_MODE_GET, &mode_cfg, rtdrv_ponmacCfg_t, 1);
    *pMode = mode_cfg.mode;

    return RT_ERR_OK;
}   /* end of rtk_ponmac_mode_get */


/* Function Name:
 *      rtk_ponmac_serdesCdr_reset
 * Description:
 *      Serdes CDR reset
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
int32
rtk_ponmac_serdesCdr_reset(void)
{
    rtdrv_ponmacCfg_t mode_cfg;

    /* function body */
    SETSOCKOPT(RTDRV_PONMAC_SERDESCDR_RESET, &mode_cfg, rtdrv_ponmacCfg_t, 1);

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_ponmac_linkState_get
 * Description:
 *      check SD and Sync state of GPON/EPON
 * Input:
 *      mode - GPON or EPON mode
 * Output:
 *      pSd     - pointer of signal detect
 *      pSync   - pointer of sync state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
int32
rtk_ponmac_linkState_get(rtk_ponmac_mode_t mode, uint32 *pSd, uint32 *pSync)
{
    rtdrv_ponmacCfg_t linkState;

    /* parameter check */
    RT_PARAM_CHK((NULL == pSd), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pSync), RT_ERR_NULL_POINTER);

    linkState.mode = mode;
    /* function body */
    GETSOCKOPT(RTDRV_PONMAC_LINKSTATE_GET, &linkState, rtdrv_ponmacCfg_t, 1);
    *pSd = linkState.sdState;
    *pSync = linkState.syncState;

    return RT_ERR_OK;
}   /* end of rtk_ponmac_linkState_get */


/* Function Name:
 *      rtk_ponmac_bwThreshold_set
 * Description:
 *      Set PON Port threshold
 * Input:
 *      bwThreshold    - PON BW_THRESHOLD
 *      reqBwThreshold - PON REQ_BW_THRESHOLD	
 * Output:
 *      N/A
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
int32
rtk_ponmac_bwThreshold_set(uint32 bwThreshold, uint32 reqBwThreshold)
{
    rtdrv_ponmacCfg_t threshold_cfg;

    /* parameter check */

    /* function body */
	threshold_cfg.bwThreshold = bwThreshold;
	threshold_cfg.reqBwThreshold = reqBwThreshold;
    SETSOCKOPT(RTDRV_PONMAC_BWTHRESHOLD_SET, &threshold_cfg, rtdrv_ponmacCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_ponmac_bwThreshold_set */


/* Function Name:
 *      rtk_ponmac_bwThreshold_get
 * Description:
 *      Get PON Port threshold
 * Input:
 *      N/A	
 * Output:
 *		pBwThreshold    - PON BW_THRESHOLD
 *      pReqBwThreshold - PON REQ_BW_THRESHOLD
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
int32
rtk_ponmac_bwThreshold_get(uint32 *pBwThreshold, uint32 *pReqBwThreshold)
{
    rtdrv_ponmacCfg_t threshold_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pBwThreshold), RT_ERR_NULL_POINTER);
	RT_PARAM_CHK((NULL == pReqBwThreshold), RT_ERR_NULL_POINTER);

    /* function body */
    GETSOCKOPT(RTDRV_PONMAC_BWTHRESHOLD_GET, &threshold_cfg, rtdrv_ponmacCfg_t, 1);
    *pBwThreshold = threshold_cfg.bwThreshold;
	*pReqBwThreshold = threshold_cfg.reqBwThreshold;

    return RT_ERR_OK;
}   /* end of rtk_ponmac_bwThreshold_get */

/* Function Name:
 *      rtk_ponmac_sidValid_get
 * Description:
 *      Get sid valid
 * Input:
 *      sid     - sid
 * Output:
 *      pValid  - the current sid valid
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 */
int32
rtk_ponmac_sidValid_get(uint32 sid, uint32 *pValid)
{
    rtdrv_ponmacCfg_t ponmac_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pValid), RT_ERR_NULL_POINTER);

    /* function body */
    ponmac_cfg.flow = sid;
    GETSOCKOPT(RTDRV_PONMAC_FLOWVALID_GET, &ponmac_cfg, rtdrv_ponmacCfg_t, 1);
    *pValid = ponmac_cfg.state;

    return RT_ERR_OK;
}   /* end of rtk_ponmac_sidValid_get */

/* Function Name:
 *      rtk_ponmac_sidValid_set
 * Description:
 *      Set sid valit
 * Input:
 *      sid     - sid
 *      valid   - valid or not
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
int32
rtk_ponmac_sidValid_set(uint32 sid, uint32 valid)
{
    rtdrv_ponmacCfg_t ponmac_cfg;

    /* function body */
    ponmac_cfg.flow = sid;
    ponmac_cfg.state = valid;
    SETSOCKOPT(RTDRV_PONMAC_FLOWVALID_SET, &ponmac_cfg, rtdrv_ponmacCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_ponmac_sidValid_set */

/* Function Name:
 *      rtk_ponmac_schedulingType_get
 * Description:
 *      Get the scheduling types and weights of queues on specific port in egress scheduling.
 * Input:
 *      None
 * Output:
 *      pQueueType  - the WFQ schedule type (RTK_QOS_WFQ or RTK_QOS_WRR)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 */
int32
rtk_ponmac_schedulingType_get(rtk_qos_scheduling_type_t *pQueueType)
{
    rtdrv_ponmacCfg_t ponmac_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pQueueType), RT_ERR_NULL_POINTER);

    /* function body */

    osal_memcpy(&ponmac_cfg.queueType, pQueueType, sizeof(rtk_qos_scheduling_type_t));
    GETSOCKOPT(RTDRV_PONMAC_SCHEDULINGTYPE_GET, &ponmac_cfg, rtdrv_ponmacCfg_t, 1);
    osal_memcpy(pQueueType, &ponmac_cfg.queueType, sizeof(rtk_qos_scheduling_type_t));

    return RT_ERR_OK;
}   /* end of rtk_ponmac_schedulingType_get */


/* Function Name:
 *      rtk_ponmac_schedulingType_set
 * Description:
 *      Get the scheduling types and weights of queues on specific port in egress scheduling.
 * Input:
 *      None
 * Output:
 *      queueType  - the WFQ schedule type (RTK_QOS_WFQ or RTK_QOS_WRR)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 */
int32
rtk_ponmac_schedulingType_set(rtk_qos_scheduling_type_t queueType)
{
    rtdrv_ponmacCfg_t ponmac_cfg;

    /* function body */
    osal_memcpy(&ponmac_cfg.queueType, &queueType, sizeof(rtk_qos_scheduling_type_t));
    SETSOCKOPT(RTDRV_PONMAC_SCHEDULINGTYPE_SET, &ponmac_cfg, rtdrv_ponmacCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_ponmac_schedulingType_set */


/* Function Name:
 *      rtk_ponmac_egrBandwidthCtrlRate_get
 * Description:
 *      Get the egress bandwidth control rate.
 * Input:
 *      None
 * Output:
 *      pRate - egress bandwidth control rate
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      (1) The actual rate is "rate * chip granularity".
 *      (2) The unit of granularity in Apollo is 8Kbps.
 */
int32
rtk_ponmac_egrBandwidthCtrlRate_get(uint32 *pRate)
{
    rtdrv_ponmacCfg_t ponmac_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pRate), RT_ERR_NULL_POINTER);

    /* function body */

    osal_memcpy(&ponmac_cfg.rate, pRate, sizeof(uint32));
    GETSOCKOPT(RTDRV_PONMAC_EGRBANDWIDTHCTRLRATE_GET, &ponmac_cfg, rtdrv_ponmacCfg_t, 1);
    osal_memcpy(pRate, &ponmac_cfg.rate, sizeof(uint32));

    return RT_ERR_OK;
}   /* end of rtk_ponmac_egrBandwidthCtrlRate_get */

/* Function Name:
 *      rtk_ponmac_egrBandwidthCtrlRate_set
 * Description:
 *      Set the pon port egress bandwidth control rate.
 * Input:
 *      rate - egress bandwidth control rate
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_RATE    - Invalid input rate
 * Note:
 *      (1) The actual rate is "rate * chip granularity".
 *      (2) The unit of granularity in Apollo is 8Kbps.
 */
int32
rtk_ponmac_egrBandwidthCtrlRate_set(uint32 rate)
{
    rtdrv_ponmacCfg_t ponmac_cfg;

    /* function body */
    osal_memcpy(&ponmac_cfg.rate, &rate, sizeof(uint32));
    SETSOCKOPT(RTDRV_PONMAC_EGRBANDWIDTHCTRLRATE_SET, &ponmac_cfg, rtdrv_ponmacCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_ponmac_egrBandwidthCtrlRate_set */



/* Function Name:
 *      rtk_ponmac_egrBandwidthCtrlRate_get
 * Description:
 *      Set the pon port egress bandwidth control rate.
 * Input:
 *      scheduleId - T-cont is or LLID index
 * Output:
 *      pRate - egress bandwidth control rate
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_RATE    - Invalid input rate
 * Note:
 *      (1) The actual rate is "rate * chip granularity".
 *      (2) The unit of granularity in Apollo is 8Kbps.
 */
int32
rtk_ponmac_egrScheduleIdRate_get(uint32 scheduleId,uint32 *pRate)
{
    rtdrv_ponmacCfg_t ponmac_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pRate), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memcpy(&ponmac_cfg.scheduleId, &scheduleId, sizeof(uint32));
    GETSOCKOPT(RTDRV_PONMAC_EGRSCHEDULEIDRATE_GET, &ponmac_cfg, rtdrv_ponmacCfg_t, 1);
    osal_memcpy(pRate, &ponmac_cfg.rate, sizeof(uint32));

    return RT_ERR_OK;
}   /* end of rtk_ponmac_egrScheduleIdRate_get */





/* Function Name:
 *      rtk_ponmac_egrBandwidthCtrlRate_set
 * Description:
 *      Set the pon port egress bandwidth control rate.
 * Input:
 *      scheduleId - T-cont is or LLID index
 *      rate - egress bandwidth control rate
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_RATE    - Invalid input rate
 * Note:
 *      (1) The actual rate is "rate * chip granularity".
 *      (2) The unit of granularity in Apollo is 8Kbps.
 */
int32
rtk_ponmac_egrScheduleIdRate_set(uint32 scheduleId,uint32 rate)
{
    rtdrv_ponmacCfg_t ponmac_cfg;

    /* function body */
    osal_memcpy(&ponmac_cfg.scheduleId, &scheduleId, sizeof(uint32));
    osal_memcpy(&ponmac_cfg.rate, &rate, sizeof(uint32));
    SETSOCKOPT(RTDRV_PONMAC_EGRSCHEDULEIDRATE_SET, &ponmac_cfg, rtdrv_ponmacCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_ponmac_egrScheduleIdRate_set */

