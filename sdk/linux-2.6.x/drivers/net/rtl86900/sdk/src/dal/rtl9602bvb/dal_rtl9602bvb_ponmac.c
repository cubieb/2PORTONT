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
 * $Revision: 46533 $
 * $Date: 2014-02-18 13:43:05 +0800 (Tue, 18 Feb 2014) $
 *
 * Purpose : Definition of PON MAC API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) PON mac
 */

/*
 * Include Files
 */
#include <dal/rtl9602bvb/dal_rtl9602bvb.h>
#include <dal/rtl9602bvb/dal_rtl9602bvb_ponmac.h>
#include <dal/rtl9602bvb/dal_rtl9602bvb_gpon.h>
#include <dal/rtl9602bvb/dal_rtl9602bvb_pbo.h>
#include <osal/time.h>

/*
 * Symbol Definition
 */

static uint32    ponmac_init = INIT_NOT_COMPLETED;
static rtk_ponmac_mode_t ponMode = PONMAC_MODE_GPON;

/*
 * Macro Declaration
 */


static int32 _ponmac_physicalQueueId_get(uint32 schedulerId, uint32 logicalQId, uint32 *pPhysicalQid)
{

    if(PONMAC_MODE_GPON == ponMode)
        *pPhysicalQid = RTL9602BVB_TCONT_QUEUE_MAX *(schedulerId/8) + logicalQId;
    else
        *pPhysicalQid = RTL9602BVB_TCONT_QUEUE_MAX *(schedulerId/4) + logicalQId;
    return RT_ERR_OK;
}


/* Function Name:
 *      _rtl9602bvb_ponMacQueueDrainOut_set
 * Description:
 *      Set PON mac per queue draint out state
 * Input:
 *      qid 		- Specify the draint out Queue id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_QUEUE_ID
 * Note:
 *      None
 */
static int32 _rtl9602bvb_ponMacQueueDrainOutState_set(uint32 qid)
{
	int32 ret;
    uint32  data;
    uint32  tmpData;
    uint32  busy;
    uint32  i;


	/* Error Checking */
    RT_PARAM_CHK((HAL_MAX_NUM_OF_PON_QUEUE() <= qid), RT_ERR_QUEUE_ID);

    data = 0;

    /* queue drain out */
    tmpData = 1;
    if ((ret = reg_field_set(RTL9602BVB_DRN_CMDr, RTL9602BVB_CFG_DRN_QUEUE_MODEf, &tmpData,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_field_set(RTL9602BVB_DRN_CMDr, RTL9602BVB_CFG_DRN_IDXf, &qid,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }
    tmpData = 1;
    if ((ret = reg_field_set(RTL9602BVB_DRN_CMDr, RTL9602BVB_DRN_PSf, &tmpData,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    /* drain out */
    if ((ret = reg_write(RTL9602BVB_DRN_CMDr,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    /* check busy bit */
    i = 0;
    do
    {
        if ((ret = reg_field_read(RTL9602BVB_DRN_CMDr, RTL9602BVB_DRN_FLGf, &busy)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
            return ret;
        }

        i++;
    } while (busy && (i < 10000));

    if(i >= 10000)
        return RT_ERR_FAILED;

    return RT_ERR_OK;
}



/* Function Name:
 *      _rtl9602bvb_ponMacQueueDrainOutState_get
 * Description:
 *      Get PON mac queue draint out state
 * Input:
 *      None
 * Output:
 *      pState - pon queue draint out status
 * Return:
 *      RT_ERR_OK
 * Note:
 *      None
 */
static int32 _rtl9602bvb_ponMacQueueDrainOutState_get(rtl9602bvb_ponmac_draintOutState_t *pState)
{
	int32 ret;

	/* Error Checking */
    RT_PARAM_CHK((pState  == NULL), RT_ERR_NULL_POINTER);

    /* set to register */
    if ((ret = reg_field_read(RTL9602BVB_DRN_CMDr, RTL9602BVB_DRN_FLGf, pState)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      _rtl9602bvb_ponMacTcontDrainOutState_set
 * Description:
 *      Set PON mac per T-cont draint out state
 * Input:
 *      tcont 		- Specify the draint out T-cont id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_GPON_EXCEED_MAX_TCONT
 * Note:
 *      None
 */
static int32 _rtl9602bvb_ponMacTcontDrainOutState_set(uint32 tcont)
{
	int32 ret;
    uint32 data;
    uint32 tmpData;
    uint32 i;
    uint32 busy;

	/* Error Checking */
    RT_PARAM_CHK((HAL_MAX_NUM_OF_GPON_TCONT() <= tcont), RT_ERR_GPON_EXCEED_MAX_TCONT);

    data = 0;

    /* set to register */
    tmpData = 0;
    if ((ret = reg_field_set(RTL9602BVB_DRN_CMDr, RTL9602BVB_CFG_DRN_QUEUE_MODEf, &tmpData,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_field_set(RTL9602BVB_DRN_CMDr, RTL9602BVB_CFG_DRN_IDXf, &tcont,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }
    tmpData = 1;
    if ((ret = reg_field_set(RTL9602BVB_DRN_CMDr, RTL9602BVB_DRN_PSf, &tmpData,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    /* set to register */
    if ((ret = reg_write(RTL9602BVB_DRN_CMDr,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    /* check busy bit */
    i = 0;
    do
    {
        if ((ret = reg_field_read(RTL9602BVB_DRN_CMDr, RTL9602BVB_DRN_FLGf, &busy)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
            return ret;
        }

        i++;
    } while (busy && (i < 10000));

    if(i >= 10000)
        return RT_ERR_FAILED;

    return RT_ERR_OK;
}



/* Function Name:
 *      _rtl9602bvb_ponMacTcontDrainOutState_get
 * Description:
 *      Get PON mac T-cont draint out state
 * Input:
 *      None
 * Output:
 *      pState - T-cont draint out status
 * Return:
 *      RT_ERR_OK
 * Note:
 *      None
 */
static int32 _rtl9602bvb_ponMacTcontDrainOutState_get(rtl9602bvb_ponmac_draintOutState_t *pState)
{
	int32 ret;

	/* Error Checking */
    RT_PARAM_CHK((pState  == NULL), RT_ERR_NULL_POINTER);


    /* set to register */
    if ((ret = reg_field_read(RTL9602BVB_DRN_CMDr, RTL9602BVB_DRN_FLGf, pState)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}



/* Function Name:
 *      _rtl9602bvb_ponMacSidToQueueMap_set
 * Description:
 *      Set PON mac session id to queue maping
 * Input:
 *      sid 		- Specify the session id
 *      qid 		- Specify the mapping queue id id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_INPUT
 *      RT_ERR_QUEUE_ID
 * Note:
 *      None
 */
static int32 _rtl9602bvb_ponMacSidToQueueMap_set(uint32 sid, uint32 qid)
{
	int32 ret;

	/* Error Checking */
    RT_PARAM_CHK((HAL_MAX_NUM_OF_PON_QUEUE() <= qid), RT_ERR_QUEUE_ID);
    RT_PARAM_CHK((HAL_CLASSIFY_SID_NUM() <= sid),RT_ERR_INPUT);

    /* set to register */
    if ((ret = reg_array_field_write(RTL9602BVB_PON_SID2QIDr,REG_ARRAY_INDEX_NONE, sid, RTL9602BVB_CFG_SID2QIDf, &qid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      _rtl9602bvb_ponMacSidToQueueMap_get
 * Description:
 *      Get PON mac session id to queue maping
 * Input:
 *      tcont 		- Specify the draint out T-cont id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_INPUT
 *      RT_ERR_QUEUE_ID
 * Note:
 *      None
 */
static int32 _rtl9602bvb_ponMacSidToQueueMap_get(uint32 sid, uint32 *pQid)
{
	int32 ret;

	/* Error Checking */
    RT_PARAM_CHK((HAL_CLASSIFY_SID_NUM() <= sid),RT_ERR_INPUT);
    RT_PARAM_CHK((pQid  == NULL), RT_ERR_NULL_POINTER);

    /* set to register */
    if ((ret = reg_array_field_read(RTL9602BVB_PON_SID2QIDr,REG_ARRAY_INDEX_NONE, sid, RTL9602BVB_CFG_SID2QIDf, pQid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}



/* Function Name:
 *      _rtl9602bvb_ponMacCirRate_set
 * Description:
 *      Per queue set PON mac CIR rate
 * Input:
 *      qid 		- - Specify the queue id
 *      rate 		- Specify the CIR rate unit 64kbps
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_INPUT
 *      RT_ERR_QUEUE_ID
 * Note:
 *      None
 */
static int32 _rtl9602bvb_ponMacCirRate_set(uint32 qid, uint32 rate)
{
	int32 ret;

	/* Error Checking */
    RT_PARAM_CHK((HAL_MAX_NUM_OF_PON_QUEUE() <= qid), RT_ERR_QUEUE_ID);
    RT_PARAM_CHK((HAL_PONMAC_PIR_CIR_RATE_MAX() < rate),RT_ERR_INPUT);

    /* set to register */
    if ((ret = reg_array_field_write(RTL9602BVB_PON_QID_CIR_RATEr,REG_ARRAY_INDEX_NONE, qid, RTL9602BVB_RATEf, &rate)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}




/* Function Name:
 *      _rtl9602bvb_ponMacCirRate_get
 * Description:
 *      Per queue set PON mac CIR rate
 * Input:
 *      qid 		- - Specify the queue id
 * Output:
 *      pRate       - Specify the CIR rate unit 64kbps
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_QUEUE_ID
 * Note:
 *      None
 */
static int32 _rtl9602bvb_ponMacCirRate_get(uint32 qid, uint32 *pRate)
{
	int32 ret;

	/* Error Checking */
    RT_PARAM_CHK((HAL_MAX_NUM_OF_PON_QUEUE() <= qid), RT_ERR_QUEUE_ID);
    RT_PARAM_CHK((pRate  == NULL), RT_ERR_NULL_POINTER);

    /* set to register */
    if ((ret = reg_array_field_read(RTL9602BVB_PON_QID_CIR_RATEr,REG_ARRAY_INDEX_NONE, qid, RTL9602BVB_RATEf, pRate)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}



/* Function Name:
 *      _rtl9602bvb_ponMacPirRate_set
 * Description:
 *      Per queue set PON mac PIR rate
 * Input:
 *      qid 		- Specify the queue id
 *      rate 		- Specify the CIR rate unit 64kbps
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_INPUT
 *      RT_ERR_QUEUE_ID
 * Note:
 *      None
 */
static int32 _rtl9602bvb_ponMacPirRate_set(uint32 qid, uint32 rate)
{
	int32 ret;

	/* Error Checking */
    RT_PARAM_CHK((HAL_MAX_NUM_OF_PON_QUEUE() <= qid), RT_ERR_QUEUE_ID);
    RT_PARAM_CHK((HAL_PONMAC_PIR_CIR_RATE_MAX() < rate),RT_ERR_INPUT);

    /* set to register */
    if ((ret = reg_array_field_write(RTL9602BVB_PON_QID_PIR_RATEr,REG_ARRAY_INDEX_NONE, qid, RTL9602BVB_RATEf, &rate)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}




/* Function Name:
 *      _rtl9602bvb_ponMacPirRate_get
 * Description:
 *      Per queue set PON mac PIR rate
 * Input:
 *      qid 		- - Specify the queue id
 * Output:
 *      pRate       - Specify the PIR rate unit 64kbps
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_QUEUE_ID
 * Note:
 *      None
 */
static int32 _rtl9602bvb_ponMacPirRate_get(uint32 qid, uint32 *pRate)
{
	int32 ret;

	/* Error Checking */
    RT_PARAM_CHK((HAL_MAX_NUM_OF_PON_QUEUE() <= qid), RT_ERR_QUEUE_ID);
    RT_PARAM_CHK((pRate  == NULL), RT_ERR_NULL_POINTER);

    /* set to register */
    if ((ret = reg_array_field_read(RTL9602BVB_PON_QID_PIR_RATEr,REG_ARRAY_INDEX_NONE, qid, RTL9602BVB_RATEf, pRate)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}





/* Function Name:
 *      _rtl9602bvb_ponMacScheQmap_set
 * Description:
 *      Per T-CONT/LLID set schedule queue map
 * Input:
 *      tcont 		- Specify T-CONT or LLID
 *      map 		- Specify schedule queue mapping mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_INPUT
 *      RT_ERR_QUEUE_ID
 * Note:
 *      None
 */
static int32 _rtl9602bvb_ponMacScheQmap_set(uint32 tcont, uint32 map)
{
	int32 ret;

	/* Error Checking */
    RT_PARAM_CHK(((HAL_MAX_NUM_OF_GPON_TCONT()) <= tcont), RT_ERR_GPON_EXCEED_MAX_TCONT);

    /* set to register */
    if ((ret = reg_array_field_write(RTL9602BVB_PON_SCH_QMAPr,REG_ARRAY_INDEX_NONE, tcont, RTL9602BVB_MAPPING_TBLf, &map)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}



/* Function Name:
 *      _rtl9602bvb_ponMacScheQmap_get
 * Description:
 *      Per T-CONT/LLID get schedule queue map
 * Input:
 *      tcont 		- Specify T-CONT or LLID
 * Output:
 *      pMap 		- Specify schedule queue mapping mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_INPUT
 *      RT_ERR_QUEUE_ID
 * Note:
 *      None
 */
static int32 _rtl9602bvb_ponMacScheQmap_get(uint32 tcont, uint32 *pMap)
{
	int32 ret;

	/* Error Checking */
    RT_PARAM_CHK((HAL_MAX_NUM_OF_GPON_TCONT() <= tcont), RT_ERR_GPON_EXCEED_MAX_TCONT);

    /* set to register */
    if ((ret = reg_array_field_read(RTL9602BVB_PON_SCH_QMAPr,REG_ARRAY_INDEX_NONE, tcont, RTL9602BVB_MAPPING_TBLf, pMap)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}




/* Function Name:
 *      _rtl9602bvb_ponMac_queueType_set
 * Description:
 *      Set queue set queue type for PON MAC
 * Input:
 *      qid 	- The queue ID wanted to set (0~127)
 *      type 	- queue type  0: strict 1:WFQ
  * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_INPUT
 *      RT_ERR_QUEUE_ID
 * Note:
 *      None
 */
static int32 _rtl9602bvb_ponMac_queueType_set(uint32 qid, rtk_qos_queue_type_t type)
{
	int32 ret;
    uint32 wData;

	/* Error Checking */
    RT_PARAM_CHK((QUEUE_TYPE_END <= type), RT_ERR_INPUT);
    RT_PARAM_CHK((HAL_MAX_NUM_OF_PON_QUEUE() <= qid), RT_ERR_QUEUE_ID);

    if(STRICT_PRIORITY==type)
        wData = 0;
    else
        wData = 1;

    /* set to register */
    if ((ret = reg_array_field_write(RTL9602BVB_PON_WFQ_TYPEr,REG_ARRAY_INDEX_NONE, qid, RTL9602BVB_QUEUE_TYPEf, &wData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      _rtl9602bvb_ponMac_queueType_get
 * Description:
 *      Set queue get queue type for PON MAC
 * Input:
 *      qid 	- The queue ID wanted to set (0~127)
 *      type 	- queue type  0: strict 1:WFQ
  * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_QUEUE_ID
 * Note:
 *      None
 */
static int32 _rtl9602bvb_ponMac_queueType_get(uint32 qid, rtk_qos_queue_type_t *pType)
{
	int32 ret;
    uint32 rData;
	/* Error Checking */
    RT_PARAM_CHK((pType  == NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_MAX_NUM_OF_PON_QUEUE() <= qid), RT_ERR_QUEUE_ID);

    /* set to register */
    if ((ret = reg_array_field_read(RTL9602BVB_PON_WFQ_TYPEr,REG_ARRAY_INDEX_NONE, qid, RTL9602BVB_QUEUE_TYPEf, &rData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }
    if(0==rData)
        *pType= STRICT_PRIORITY;
    else
        *pType= WFQ_WRR_PRIORITY;

    return RT_ERR_OK;
}






/* Function Name:
 *      _rtl9602bvb_ponMac_wfqWeight_set
 * Description:
 *      Set queue set WFQ queue weight for PON MAC
 * Input:
 *      qid 	- The queue ID wanted to set (0~127)
 *      weight 	- WFQ weight
  * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_INPUT
 *      RT_ERR_QUEUE_ID
 * Note:
 *      None
 */
static int32 _rtl9602bvb_ponMac_wfqWeight_set(uint32 qid, uint32 weight)
{
	int32 ret;

	/* Error Checking */
    RT_PARAM_CHK((1024 <= weight), RT_ERR_INPUT);

    RT_PARAM_CHK((0 == weight), RT_ERR_INPUT);

    RT_PARAM_CHK((HAL_MAX_NUM_OF_PON_QUEUE() <= qid), RT_ERR_QUEUE_ID);

    /* set to register */
    if ((ret = reg_array_field_write(RTL9602BVB_PON_WFQ_WEIGHTr,REG_ARRAY_INDEX_NONE, qid, RTL9602BVB_WEIGHT_PONf, &weight)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      _rtl9602bvb_ponMac_wfqWeight_get
 * Description:
 *      Set queue set WFQ queue weight for PON MAC
 * Input:
 *      qid 	- The queue ID wanted to set (0~127)
  * Output:
 *      pWeight 	- WFQ weight
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_QUEUE_ID
 * Note:
 *      None
 */
static int32 _rtl9602bvb_ponMac_wfqWeight_get(uint32 qid, uint32 *pWeight)
{
	int32 ret;

	/* Error Checking */
    RT_PARAM_CHK((pWeight  == NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_MAX_NUM_OF_PON_QUEUE() <= qid), RT_ERR_QUEUE_ID);

    /* set to register */
    if ((ret = reg_array_field_read(RTL9602BVB_PON_WFQ_WEIGHTr,REG_ARRAY_INDEX_NONE, qid, RTL9602BVB_WEIGHT_PONf, pWeight)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}






/* Function Name:
 *      _rtl9602bvb_ponMacTcontEnable_set
 * Description:
 *      Per T-CONT set schedule enable state
 * Input:
 *      tcont 		- Specify T-CONT
 *      enable 		- Specify T-CONT schedule enable state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_INPUT
 *      RT_ERR_QUEUE_ID
 * Note:
 *      None
 */
static int32 _rtl9602bvb_ponMacTcontEnable_set(uint32 tcont, rtk_enable_t enable)
{
	int32 ret;

	/* Error Checking */
    RT_PARAM_CHK(((HAL_MAX_NUM_OF_GPON_TCONT()) <= tcont), RT_ERR_GPON_EXCEED_MAX_TCONT);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable ), RT_ERR_INPUT);

    /* set to register */
    if ((ret = reg_array_field_write(RTL9602BVB_PON_TCONT_ENr,REG_ARRAY_INDEX_NONE, tcont, RTL9602BVB_TCONT_ENf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}



/* Function Name:
 *      _rtl9602bvb_ponMacTcontEnable_get
 * Description:
 *      Per T-CONT get schedule enable state
 * Input:
 *      tcont 		- Specify T-CONT
 * Output:
 *      pEnable 		- T-CONT schedule enable state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_INPUT
 *      RT_ERR_QUEUE_ID
 * Note:
 *      None
 */
static int32 _rtl9602bvb_ponMacTcontEnable_get(uint32 tcont, rtk_enable_t *pEnable)
{
	int32 ret;

	/* Error Checking */
    RT_PARAM_CHK((HAL_MAX_NUM_OF_GPON_TCONT() <= tcont), RT_ERR_GPON_EXCEED_MAX_TCONT);
    RT_PARAM_CHK((pEnable  == NULL), RT_ERR_NULL_POINTER);

    /* set to register */
    if ((ret = reg_array_field_read(RTL9602BVB_PON_TCONT_ENr,REG_ARRAY_INDEX_NONE, tcont, RTL9602BVB_TCONT_ENf, pEnable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}



static int32 _ponmac_schedulerQueue_get(uint32 physicalQid, uint32 *pSchedulerId, uint32 *pLogicalQId)
{
    int32   ret;
    uint32  qMask;
    uint32  maxSchedhlerId;
    uint32  schedhlerBase;
    uint32  i;

    *pLogicalQId = physicalQid%HAL_PONMAC_TCONT_QUEUE_MAX();

    if(PONMAC_MODE_GPON == ponMode)
    {
        maxSchedhlerId = 7;
        schedhlerBase = (physicalQid/HAL_PONMAC_TCONT_QUEUE_MAX()) * 8;
    }
    else
    {
        maxSchedhlerId = 2;
        schedhlerBase = (physicalQid/HAL_PONMAC_TCONT_QUEUE_MAX()) * 2;
    }

    if(physicalQid == (HAL_MAX_NUM_OF_PON_QUEUE() - 1))
    {
        *pSchedulerId = HAL_MAX_NUM_OF_GPON_TCONT() - 1;
        *pLogicalQId = 0;
        return RT_ERR_OK;
    }

    for(i = schedhlerBase; i<schedhlerBase+maxSchedhlerId ; i++ )
    {
        if ((ret = _rtl9602bvb_ponMacScheQmap_get(i, &qMask)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
            return ret;
        }
        if(qMask & 1<<(*pLogicalQId))
        {
            *pSchedulerId = i;
            return RT_ERR_OK;
        }
    }
    *pSchedulerId = schedhlerBase;
    return RT_ERR_OK;
}

/* Module Name    : PON Mac                                  */
/* Sub-module Name: flow/t-cont/queue mapping */


/* Function Name:
 *      dal_rtl9602bvb_ponmac_init
 * Description:
 *      Configure PON MAC initial settings
 * Input:
 *      None.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_QUEUE_NUM - Invalid queue number
 * Note:
 */
int32
dal_rtl9602bvb_ponmac_init(void)
{
    int32  ret;
    uint32 tcont,wData;
    uint32 physicalQid;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PONMAC),"%s",__FUNCTION__);

#ifdef FPGA_DEFINED
    /*fpga do not have RTL9602BVB_WSDS_ANA_24*/
#else /*FPGA_DEFINED*/

#ifdef CONFIG_BEN_DIFFERENTIAL
    wData = 0;
    if ((ret = reg_field_write(RTL9602BVB_WSDS_ANA_24r, RTL9602BVB_REG_BEN_TTL_OUTf, &wData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    wData = 1;
    if ((ret = reg_field_write(RTL9602BVB_WSDS_ANA_18r, RTL9602BVB_REG_BEN_SEL_CMLf, &wData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }
#else /*CONFIG_BEN_DIFFERENTIAL*/
    /* TBD
	wData = 1;
    if ((ret = reg_field_write(RTL9602BVB_WSDS_ANA_24r, RTL9602BVB_REG_BEN_TTL_OUTf, &wData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }*/
#endif /*CONFIG_BEN_DIFFERENTIAL*/
#endif /*FPGA_DEFINED*/

    /*init PON BW_THRESHOLD*/
    wData = 5;
    if ((ret = reg_field_write(RTL9602BVB_PON_BW_THRESr, RTL9602BVB_CFG_BW_LAST_THRESf, &wData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    /*init PON REQ_BW_THRESHOLD*/
    wData = 5;
    if ((ret = reg_field_write(RTL9602BVB_PON_BW_THRESr, RTL9602BVB_CFG_BW_RUNT_THRESf, &wData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    /*drant out all queue when pon mac init*/
    if(INIT_COMPLETED == ponmac_init)
    {
        uint32 schedulerId;

        for(schedulerId = 0 ; schedulerId < HAL_MAX_NUM_OF_GPON_TCONT() ; schedulerId++ )
        {
            if((ret= _rtl9602bvb_ponMacTcontDrainOutState_set(schedulerId)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
                return ret;
            }
        }
    }

    for(tcont = 0; tcont <HAL_MAX_NUM_OF_GPON_TCONT()-1; tcont++ )
    {
        /*disable T-cont schedule */
        if ((ret = _rtl9602bvb_ponMacTcontEnable_set(tcont, DISABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
            return ret;
        }
        /*clear T-cont schedule mask*/
        if ((ret = _rtl9602bvb_ponMacScheQmap_set(tcont, 0)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
            return ret;
        }
    }

    wData = 1;
    if ((ret = reg_field_write(RTL9602BVB_PON_SCH_CTRLr, RTL9602BVB_PON_GEN_PIR_DROPf, &wData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    /*set all queue to strict, disable CIR/PIR and disable egress drop*/
    for(physicalQid=0 ; physicalQid<HAL_MAX_NUM_OF_PON_QUEUE(); physicalQid++)
    {
        /*set queue schedule type*/
        if ((ret = _rtl9602bvb_ponMac_queueType_set(physicalQid, STRICT_PRIORITY)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
            return ret;
        }

        /*set PIR CIR*/
        if ((ret = _rtl9602bvb_ponMacCirRate_set(physicalQid, 0x0)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
            return ret;
        }

        if ((ret = _rtl9602bvb_ponMacPirRate_set(physicalQid, HAL_PONMAC_PIR_CIR_RATE_MAX())) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
            return ret;
        }

        /*set default weight to 1*/
        if ((ret = _rtl9602bvb_ponMac_wfqWeight_set(physicalQid, 0x1)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
            return ret;
        }
    }

    /* set OMCI MPCP priority to 7 */
    wData = 7;
    if ((ret = reg_field_write(RTL9602BVB_PON_TRAP_CFGr, RTL9602BVB_OMCI_MPCP_PRIORITYf, &wData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    ponmac_init = INIT_COMPLETED;

    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_ponmac_init */


/* Function Name:
 *      dal_rtl9602bvb_ponmac_queue_add
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
 *      RT_ERR_NULL_POINTER    					- Pointer pQueueList/pQueue point to NULL.
 *      RT_ERR_INPUT 							- Invalid input parameters.
 * Note:
 *      None
 */
int32
dal_rtl9602bvb_ponmac_queue_add(rtk_ponmac_queue_t *pQueue, rtk_ponmac_queueCfg_t *pQueueCfg)
{
    int32   ret;
    uint32  qMask;
    uint32  physicalQid;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PONMAC),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(ponmac_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pQueue), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pQueueCfg), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pQueue->schedulerId >= HAL_MAX_NUM_OF_GPON_TCONT()), RT_ERR_INPUT);
    RT_PARAM_CHK((pQueue->queueId >= RTL9602BVB_TCONT_QUEUE_MAX), RT_ERR_INPUT);

    RT_PARAM_CHK((pQueueCfg->type >= QUEUE_TYPE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((pQueueCfg->egrssDrop >= RTK_ENABLE_END), RT_ERR_INPUT);


    /* add queue to t-cont schedule mask*/
    if ((ret = _rtl9602bvb_ponMacScheQmap_get(pQueue->schedulerId, &qMask)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    if(qMask == 0)
    {/*first queue add to this t-cont enable t-cont schedule*/
        if ((ret = _rtl9602bvb_ponMacTcontEnable_set(pQueue->schedulerId, ENABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
            return ret;
        }
    }

    /*add queue to qMask*/
    qMask = qMask | (1<<pQueue->queueId);
    if ((ret = _rtl9602bvb_ponMacScheQmap_set(pQueue->schedulerId, qMask)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    /*set Queue id and flow id mapping */
    _ponmac_physicalQueueId_get(pQueue->schedulerId, pQueue->queueId, &physicalQid);

    /*set queue schedule type*/
    if(pQueueCfg->type == STRICT_PRIORITY)
    {
        if ((ret = _rtl9602bvb_ponMac_queueType_set(physicalQid, STRICT_PRIORITY)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
            return ret;
        }
        /* for safe, set weight to 1 when strict */
        if ((ret = _rtl9602bvb_ponMac_wfqWeight_set(physicalQid, 1)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
            return ret;
        }
    }
    else
    {
        /*set wfq weight*/
        if ((ret = _rtl9602bvb_ponMac_wfqWeight_set(physicalQid, pQueueCfg->weight)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
            return ret;
        }
        if ((ret = _rtl9602bvb_ponMac_queueType_set(physicalQid, WFQ_WRR_PRIORITY)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
            return ret;
        }
    }


    /*set PIR CIR*/
    if ((ret = _rtl9602bvb_ponMacCirRate_set(physicalQid, pQueueCfg->cir)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    if ((ret = _rtl9602bvb_ponMacPirRate_set(physicalQid, pQueueCfg->pir)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }



    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_ponmac_queue_add */



/* Function Name:
 *      dal_rtl9602bvb_ponmac_queue_get
 * Description:
 *      get queue setting
 * Input:
 *      pQueue         - queue id and scheduler id for ths queue.
 * Output:
 *      pQueueCfg     - queue configuration
 * Return:
 *      RT_ERR_OK              					- OK
 *      RT_ERR_FAILED          					- Failed
 *      RT_ERR_NULL_POINTER    					- Pointer pQueueList/pQueue point to NULL.
 *      RT_ERR_INPUT 							- Invalid input parameters.
 * Note:
 *      None
 */
int32
dal_rtl9602bvb_ponmac_queue_get(rtk_ponmac_queue_t *pQueue, rtk_ponmac_queueCfg_t *pQueueCfg)
{
    int32   ret;
    rtk_enable_t   enable;
    uint32  qMask;
    uint32  physicalQid;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PONMAC),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(ponmac_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pQueue), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pQueueCfg), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pQueue->schedulerId >= HAL_MAX_NUM_OF_GPON_TCONT()), RT_ERR_INPUT);
    RT_PARAM_CHK((pQueue->queueId >= RTL9602BVB_TCONT_QUEUE_MAX), RT_ERR_INPUT);

    if ((ret = _rtl9602bvb_ponMacTcontEnable_get(pQueue->schedulerId, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    if(DISABLED==enable)
        return RT_ERR_ENTRY_NOTFOUND;

    if ((ret = _rtl9602bvb_ponMacScheQmap_get(pQueue->schedulerId, &qMask)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    if(!(qMask & 1<<(pQueue->queueId)))
        return RT_ERR_ENTRY_NOTFOUND;

    _ponmac_physicalQueueId_get(pQueue->schedulerId, pQueue->queueId, &physicalQid);

    if ((ret = _rtl9602bvb_ponMac_queueType_get(physicalQid, &(pQueueCfg->type))) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    /*set wfq weight*/
    if ((ret = _rtl9602bvb_ponMac_wfqWeight_get(physicalQid, &(pQueueCfg->weight))) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    /*set PIR CIR*/
    if ((ret = _rtl9602bvb_ponMacCirRate_get(physicalQid, &(pQueueCfg->cir))) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    if ((ret = _rtl9602bvb_ponMacPirRate_get(physicalQid, &(pQueueCfg->pir))) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }


    /*get egress drop*/
    pQueueCfg->egrssDrop = DISABLED;


    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_ponmac_queue_get */


/* Function Name:
 *      dal_rtl9602bvb_ponmac_queue_del
 * Description:
 *      delete queue from given scheduler id
 * Input:
 *      pQueue         - queue id and scheduler id for ths queue.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              					- OK
 *      RT_ERR_FAILED          					- Failed
 *      RT_ERR_NULL_POINTER    					- Pointer pQueue point to NULL.
 *      RT_ERR_INPUT 							- Invalid input parameters.
 * Note:
 *      None
 */
int32
dal_rtl9602bvb_ponmac_queue_del(rtk_ponmac_queue_t *pQueue)
{
    int32   ret;
    uint32  qMask;
    rtk_enable_t   enable;
    uint32  queueId;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PONMAC),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(ponmac_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pQueue), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pQueue->schedulerId >= HAL_MAX_NUM_OF_GPON_TCONT()), RT_ERR_INPUT);
    RT_PARAM_CHK((pQueue->queueId >= RTL9602BVB_TCONT_QUEUE_MAX), RT_ERR_INPUT);

    if ((ret = _rtl9602bvb_ponMacTcontEnable_get(pQueue->schedulerId, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    if(DISABLED==enable)
        return RT_ERR_ENTRY_NOTFOUND;

    if ((ret = _rtl9602bvb_ponMacScheQmap_get(pQueue->schedulerId, &qMask)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    if(!(qMask & 1<<(pQueue->queueId)))
        return RT_ERR_ENTRY_NOTFOUND;


    /*remove queue from qMask*/
    qMask = qMask & (~(1<<pQueue->queueId));
    if ((ret = _rtl9602bvb_ponMacScheQmap_set(pQueue->schedulerId, qMask)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

        /*drant out queue*/
    queueId = ((pQueue->schedulerId)/8) * 32 + pQueue->queueId;
    if ((ret = _rtl9602bvb_ponMacQueueDrainOutState_set(queueId)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    if(0 == qMask)
    {/*no queue map to this t-cont disable schedule for this t-cont*/
        if ((ret = _rtl9602bvb_ponMacTcontEnable_set(pQueue->schedulerId, DISABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_ponmac_queue_del */



/* Function Name:
 *      dal_rtl9602bvb_ponmac_flow2Queue_set
 * Description:
 *      mapping flow to given queue
 * Input:
 *      flow          - flow id.
 *      pQueue        - queue id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              					- OK
 *      RT_ERR_FAILED          					- Failed
 *      RT_ERR_NULL_POINTER    					- Pointer pQueue point to NULL.
 *      RT_ERR_INPUT 							- Invalid input parameters.
 * Note:
 *      None
 */
int32
dal_rtl9602bvb_ponmac_flow2Queue_set(uint32  flow, rtk_ponmac_queue_t *pQueue)
{
    int32   ret;
    uint32  physicalQid;


    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PONMAC),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(ponmac_init);

    /* parameter check */
    RT_PARAM_CHK((HAL_CLASSIFY_SID_NUM() <= flow), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pQueue), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pQueue->schedulerId >= HAL_MAX_NUM_OF_GPON_TCONT()), RT_ERR_INPUT);
    RT_PARAM_CHK((pQueue->queueId >= RTL9602BVB_TCONT_QUEUE_MAX), RT_ERR_INPUT);

    _ponmac_physicalQueueId_get(pQueue->schedulerId, pQueue->queueId, &physicalQid);

    if ((ret = _rtl9602bvb_ponMacSidToQueueMap_set(flow, physicalQid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_ponmac_flow2Queue_set */


/* Function Name:
 *      dal_rtl9602bvb_ponmac_flow2Queue_get
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
dal_rtl9602bvb_ponmac_flow2Queue_get(uint32  flow, rtk_ponmac_queue_t *pQueue)
{
    int32   ret;
    uint32  physicalQid;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PONMAC),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(ponmac_init);

    /* parameter check */
    RT_PARAM_CHK((HAL_CLASSIFY_SID_NUM() <= flow), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pQueue), RT_ERR_NULL_POINTER);

    if ((ret = _rtl9602bvb_ponMacSidToQueueMap_get(flow, &physicalQid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    if ((ret = _ponmac_schedulerQueue_get(physicalQid, &(pQueue->schedulerId), &(pQueue->queueId))) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_ponmac_flow2Queue_get */

/* Function Name:
 *      dal_rtl9602bvb_ponmac_maxPktLen_set
 * Description:
 *      set pon port max packet length
 * Input:
 *      length         - max accept packet length
 *	  state 	     - enable or disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              						- OK
 *      RT_ERR_FAILED          					- Failed
 * Note:
 *      None
 */
int32 dal_rtl9602bvb_ponmac_maxPktLen_set(uint32 length)
{
    uint32 data;
    int32  ret;

    if ((ret = reg_field_write(RTL9602BVB_MAX_LENGTH_CFG1r, RTL9602BVB_ACCEPT_MAX_LENTH_CFG1f, &length)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    data = 1;
    if ((ret = reg_array_field_write(RTL9602BVB_ACCEPT_MAX_LEN_CTRLr, HAL_GET_PON_PORT(), REG_ARRAY_INDEX_NONE, RTL9602BVB_MAX_LENGTH_GIGAf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }
    data = 1;
    if ((ret = reg_array_field_write(RTL9602BVB_ACCEPT_MAX_LEN_CTRLr, HAL_GET_PON_PORT(), REG_ARRAY_INDEX_NONE, RTL9602BVB_MAX_LENGTH_10_100f, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;

}

#ifdef CONFIG_TX_POWER_TURN_ON_FEATURE
static int32 _ponmac_txPower_enable(rtk_enable_t enable)
{
    int32  ret;

    /* set TX power GPIO pin to output 0 */
    if ((ret = dal_rtl9602bvb_gpio_mode_set(CONFIG_TX_POWER_GPO_PIN, GPIO_OUTPUT)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }
    if ((ret = dal_rtl9602bvb_gpio_state_set(CONFIG_TX_POWER_GPO_PIN, ENABLED)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }
    if(enable == ENABLED)
    {
        if ((ret = dal_rtl9602bvb_gpio_databit_set(CONFIG_TX_POWER_GPO_PIN,0)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
            return ret;
        }
    }
    else
    {
        if ((ret = dal_rtl9602bvb_gpio_databit_set(CONFIG_TX_POWER_GPO_PIN,1)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
            return ret;
        }
    }
    return RT_ERR_OK;

}
#endif

/* Function Name:
 *      dal_rtl9602bvb_ponmac_mode_set
 * Description:
 *      set pon mac mode
 * Input:
 *      mode         - pon mode, epon or gpon
 *	  state 	     - enable or disable
 * Output:
 *      pQueue       - queue id.
 * Return:
 *      RT_ERR_OK              						- OK
 *      RT_ERR_FAILED          					- Failed
 *      RT_ERR_NULL_POINTER    					- Pointer pQueue point to NULL.
 *      RT_ERR_INPUT 							- Invalid input parameters.
 * Note:
 *      None
 */
int32
dal_rtl9602bvb_ponmac_mode_set(rtk_ponmac_mode_t mode)
{
    int32 ret;
    uint32 data;
    static uint32 pbo_init = 0;
    rtk_ponmac_queueCfg_t   queueCfg;
    rtk_ponmac_queue_t logicalQueue;
    uint32 queueId;
    uint32 flowId,tcont;


    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PONMAC),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(ponmac_init);

    RT_PARAM_CHK((PONMAC_MODE_END <= mode), RT_ERR_INPUT);

    switch(mode)
    {
        case PONMAC_MODE_GPON:
        {
            /* mapping all flow to t-cont 15 queue 31, and set SID invalid */
            logicalQueue.schedulerId = 15;
            logicalQueue.queueId     = 31;
            for(flowId=0 ; flowId<HAL_CLASSIFY_SID_NUM()-1 ; flowId++)
            {
                if((ret= dal_rtl9602bvb_ponmac_flow2Queue_set(flowId, &logicalQueue)) != RT_ERR_OK)
                {
                    ponmac_init = INIT_NOT_COMPLETED;
                    return ret;
                }

                data = 0x0;
                if ((ret = reg_array_field_write(RTL9602BVB_PON_SIDVALIDr, REG_ARRAY_INDEX_NONE, flowId, RTL9602BVB_CFG_SID2VALIDf, &data)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                    return ret;
                }
            }

            /* omci flow to queue */
            logicalQueue.schedulerId = RTL9602BVB_GPON_OMCI_TCONT_ID;
            logicalQueue.queueId = RTL9602BVB_GPON_OMCI_QUEUE_ID;
            if((ret= dal_rtl9602bvb_ponmac_flow2Queue_set(RTL9602BVB_GPON_OMCI_FLOW_ID, &logicalQueue)) != RT_ERR_OK)
            {
                return ret;
            }
            /* set omci sid valid */
            data = 0x1;
            if ((ret = reg_array_field_write(RTL9602BVB_PON_SIDVALIDr, REG_ARRAY_INDEX_NONE, RTL9602BVB_GPON_OMCI_FLOW_ID, RTL9602BVB_CFG_SID2VALIDf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
            /* set CFG_OMCI_SID */
            data = RTL9602BVB_GPON_OMCI_FLOW_ID;
            if ((ret = reg_field_write(RTL9602BVB_PON_OMCI_CFGr, RTL9602BVB_CFG_OMCI_SIDf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }

            if(!pbo_init)
            {
#if !defined(FPGA_DEFINED)
                if ((ret = rtl9602bvb_raw_pbo_init(RTL9602BVB_PBO_PONMODE_GPON, 128, 128)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
                    return ret;
                }
#endif
                pbo_init = 1;
            }

            /* set the serdes mode to GPON mode */
            data = 0x8;
            if ((ret = reg_field_write(RTL9602BVB_SDS_CFGr, RTL9602BVB_CFG_SDS_MODEf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }

            /* WSDS_ANA_1A.REG_RX_KP2_2 = 6 */
            /* <M> DEL REG
			data = 0x6;
            if ((ret = reg_field_write(RTL9602BVB_WSDS_ANA_1Ar, RTL9602BVB_REG_RX_KP2_2f, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }*/

            /* reset serdes including digital and analog */
            /* no force sds */
            data = 0x0;
            if ((ret = reg_write(RTL9602BVB_WSDS_DIG_01r, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }

            /* reset serdes including digital and analog, and GPON MAC */
            data = 0x1;
            if ((ret = reg_field_write(RTL9602BVB_SOFTWARE_RSTr, RTL9602BVB_CMD_SDS_RST_PSf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
            osal_time_mdelay(10);

            /* force sds for not reset GPON MAC when SD down */
            data = 0xc;
            if ((ret = reg_write(RTL9602BVB_WSDS_DIG_01r, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
            /* <M> DEL REG
			data = 0x0;
            if ((ret = reg_write(RTL9602BVB_WSDS_ANA_1Cr, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }*/

            /* Serdes amplitude 500 mv */
            /* <M> DEL REG
			data = 0x1164;
            if ((ret = reg_write(RTL9602BVB_WSDS_ANA_0Er, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }*/
            /* BEN on */
            data = 0x1;
            if ((ret = reg_field_write(RTL9602BVB_WSDS_DIG_18r, RTL9602BVB_BEN_OEf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
            /* pon port allow undersize packet */
            data = 1;
            if ((ret = reg_array_field_write(RTL9602BVB_P_MISCr, HAL_GET_PON_PORT(), REG_ARRAY_INDEX_NONE, RTL9602BVB_RX_SPCf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
            /* set ben force mode to 0 */
            data = 0;
            if ((ret = reg_field_write(RTL9602BVB_FORCE_BENr, RTL9602BVB_BEN_FORCE_MODEf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }

            /* set max packet len to 2031 (2047-4(ctag)-4(stag)-8(pppoe)=2031) */
            if((ret = dal_rtl9602bvb_ponmac_maxPktLen_set(2031)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
        }
        break;

        case PONMAC_MODE_EPON:
        {
            if(!pbo_init)
            {
#if !defined(FPGA_DEFINED)
                if ((ret = rtl9602bvb_raw_pbo_init(RTL9602BVB_PBO_PONMODE_EPON, 128, 128)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
                    return ret;
                }
#endif
                pbo_init = 1;
            }

            #if 0 /* No such register in RTL9602BVB */
            data = 0;
            if ((ret = reg_field_write(RTL9602BVB_PON_MODE_CFGr,RTL9602BVB_GPON_ENf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
            data = 1;
            if ((ret = reg_field_write(RTL9602BVB_PON_MODE_CFGr,RTL9602BVB_EPON_ENf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
            #endif

            /* set the serdes mode to EPON mode */
            data = 0xc;
            if ((ret = reg_field_write(RTL9602BVB_SDS_CFGr, RTL9602BVB_CFG_SDS_MODEf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }

            /* change mode to EPON, must reset switch */
            data = 0x1;
            if ((ret = reg_field_write(RTL9602BVB_SOFTWARE_RSTr, RTL9602BVB_SW_RSTf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
            osal_time_mdelay(10);

            /* WSDS_ANA_1A.REG_RX_KP2_2 = 6 */
            /* <M> DEL REG
			data = 0x6;
            if ((ret = reg_field_write(RTL9602BVB_WSDS_ANA_1Ar, RTL9602BVB_REG_RX_KP2_2f, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }*/

            /* Serdes amplitude 500 mv */
            /* <M> DEL REG
			data = 0x1164;
            if ((ret = reg_write(RTL9602BVB_WSDS_ANA_0Er, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }*/

            /* reset serdes including digital and analog */
            data = 0x0;
            if ((ret = reg_field_write(RTL9602BVB_WSDS_DIG_00r, RTL9602BVB_CFG_SFT_RSTBf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
            data = 0x1;
            if ((ret = reg_field_write(RTL9602BVB_WSDS_DIG_00r, RTL9602BVB_CFG_SFT_RSTBf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
            osal_time_mdelay(10);

            /* BEN on */
            data = 0x1;
            if ((ret = reg_field_write(RTL9602BVB_WSDS_DIG_18r, RTL9602BVB_BEN_OEf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }

            /*serdes patch*/
            /* WSDS_ANA_18.REG_RX_KP1_2 = 3 */
            /* <M> DEL REG
			data = 0x3;
            if ((ret = reg_field_write(RTL9602BVB_WSDS_ANA_18r, RTL9602BVB_REG_RX_KP1_2f, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }*/
            /* WSDS_ANA_1A.REG_RX_KP2_2 = 4 */
            /* <M> DEL REG>
			data = 0x4;
            if ((ret = reg_field_write(RTL9602BVB_WSDS_ANA_1Ar, RTL9602BVB_REG_RX_KP2_2f, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }*/
            /* SDS_REG1.SDS_FRC_TX = 0 */
#if 0
            data = 0;
            if ((ret = reg_field_write(RTL9602BVB_SDS_REG1r, RTL9602BVB_SDS_FRC_RXf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
#endif
            /*PCS derdes patch*/
            data = 0x3;
            if ((ret = reg_write(RTL9602BVB_SDS_EXT_REG12r, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }

            /*data = 0x4e6a;*/
            data = 0x4eff;
            if ((ret = reg_write(RTL9602BVB_SDS_EXT_REG13r, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
            /*data = 0x1562;*/
            data = 0x1400;
            if ((ret = reg_write(RTL9602BVB_SDS_EXT_REG15r, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
            data = 0xbd2a;
            if ((ret = reg_write(RTL9602BVB_SDS_EXT_REG16r, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }

            data = 0x1059;
            if ((ret = reg_write(RTL9602BVB_SDS_REG7r, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
#if 0
            data = 0x3F;
            if ((ret = reg_field_write(RTL9602BVB_SDS_REG24r,RTL9602BVB_CFG_FIB2G_TXCFG_NP_P0f,&data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
#endif
            /* set ben force mode to 0 */
            data = 0;
            if ((ret = reg_field_write(RTL9602BVB_FORCE_BENr, RTL9602BVB_BEN_FORCE_MODEf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }

            /* force sds for not reset EPON MAC when SD down */
            data = 0xc;
            if ((ret = reg_write(RTL9602BVB_WSDS_DIG_01r, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }

            /* set max packet len to 2031 (2047-4(ctag)-4(stag)-8(pppoe)=2031) */
            if((ret = dal_rtl9602bvb_ponmac_maxPktLen_set(2031)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
        }

        for(tcont = 0; tcont <HAL_MAX_NUM_OF_GPON_TCONT()-1; tcont++ )
        {
            /*disable T-cont schedule */
            if ((ret = _rtl9602bvb_ponMacTcontEnable_set(tcont, DISABLED)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
                return ret;
            }
            /*clear T-cont schedule mask*/
            if ((ret = _rtl9602bvb_ponMacScheQmap_set(tcont, 0)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
                return ret;
            }
        }

        /*for epon default flow to queue is one to one mapping*/
        for(flowId=0 ; flowId<HAL_CLASSIFY_SID_NUM()-1 ; flowId++)
        {
        
            if((ret= _rtl9602bvb_ponMacSidToQueueMap_set(flowId, flowId)) != RT_ERR_OK)
            {
                ponmac_init = INIT_NOT_COMPLETED;
                return ret;
            }
        }


        break;

        case PONMAC_MODE_FIBER_1G:
        {
            /*mapping queue 0~7 to schedule id 0 if not GPON/EPON */
            memset(&queueCfg, 0, sizeof(rtk_ponmac_queueCfg_t));

            logicalQueue.schedulerId = 0 ;

            queueCfg.cir       = 0x0;
            queueCfg.pir       = HAL_PONMAC_PIR_CIR_RATE_MAX();
            queueCfg.type      = STRICT_PRIORITY;
            queueCfg.egrssDrop = DISABLED;
            ponmac_init = INIT_COMPLETED;
            for(queueId = 0 ; queueId <= RTK_DOT1P_PRIORITY_MAX ; queueId++ )
            {
                logicalQueue.queueId = queueId;

                if((ret= dal_rtl9602bvb_ponmac_queue_add(&logicalQueue, &queueCfg)) != RT_ERR_OK)
                {
                    ponmac_init = INIT_NOT_COMPLETED;
                    return ret;
                }
            }
            logicalQueue.schedulerId = 0;
            logicalQueue.queueId     = 0;

            for(flowId=0 ; flowId<HAL_CLASSIFY_SID_NUM()-1 ; flowId++)
            {
                if((ret= dal_rtl9602bvb_ponmac_flow2Queue_set(flowId, &logicalQueue)) != RT_ERR_OK)
                {
                    ponmac_init = INIT_NOT_COMPLETED;
                    return ret;
                }
            }

            #if 0 /* No such register in RTL9602BVB */
            data = 0;
            if ((ret = reg_field_write(RTL9602BVB_PON_MODE_CFGr,RTL9602BVB_GPON_ENf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
            data = 0;
            if ((ret = reg_field_write(RTL9602BVB_PON_MODE_CFGr,RTL9602BVB_EPON_ENf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
            #endif

            /* =========== Scheduling config =========== */
            /* Config queue mapping for fiber */
            /* add queue 0 to scheduler 0*/
            data = 1;
            if ((ret = _rtl9602bvb_ponMacScheQmap_set(0, data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
                return ret;
            }
            /* enable scheduler 0*/
            if ((ret = _rtl9602bvb_ponMacTcontEnable_set(0, ENABLED)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
                return ret;
            }

#if 0 /* TBD 20150428 */
            /* =========== Port config =========== */
            /* disable EEE for port 4 */
            data = 0;
            if ((ret = reg_array_write(RTL9602BVB_P_EEECFGr, HAL_GET_PON_PORT(), REG_ARRAY_INDEX_NONE, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_GPON | MOD_DAL), "");
                return ret;
            }
#endif

            /* =========== Serdes patch =========== */
            /* WSDS_ANA_18 */
            /* <M> DEL REG
			data = 0x01;
            if ((ret = reg_write(RTL9602BVB_WSDS_ANA_18r, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }*/
            /* WSDS_DIG_0A */
            data = 0;
            if ((ret = reg_write(RTL9602BVB_WSDS_DIG_0Ar, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
            /* WSDS_DIG_0C */
            data = 96;
            if ((ret = reg_write(RTL9602BVB_WSDS_DIG_0Cr, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
            /* WSDS_ANA_00 */
            /*data = 0x0801;
            if ((ret = reg_write(RTL9602BVB_WSDS_ANA_00r, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }*/
            /* WSDS_DIG_18 */
            data = 0x1000;
            if ((ret = reg_write(RTL9602BVB_WSDS_DIG_18r, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
            /* WSDS_ANA_0E */
            /* <M> DEL REG
			data = 0x1464;
            if ((ret = reg_write(RTL9602BVB_WSDS_ANA_0Er, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }*/

            /* =========== Fiber phy config =========== */
            /* Set AN enable, full duplex */
            data = 0x1140;
            if ((ret = reg_write(RTL9602BVB_FIB_REG0r, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
            /* Set flow control ability */
            data = 0x01A0;
            if ((ret = reg_write(RTL9602BVB_FIB_REG4r, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }

            #if 0 /* No such register in RTL9602BVB */
            /* select fiber */
            data = 1;
            if ((ret = reg_field_write(RTL9602BVB_UTP_FIBER_AUTODETr, RTL9602BVB_SEL_FIBf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
            #endif
    	    /* set the serdes mode to fiber 1G mode */
            data = 0x4;
            if ((ret = reg_field_write(RTL9602BVB_SDS_CFGr, RTL9602BVB_CFG_SDS_MODEf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
#if 0
			/* set the serdes mode to fiber 1G mode */
            data = 0x4;
            if ((ret = reg_field_write(RTL9602BVB_SDS_REG4r, RTL9602BVB_CFG_FRC_SDS_MODEf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
#endif
            /* set ben force mode to 0 */
            data = 0;
            if ((ret = reg_field_write(RTL9602BVB_FORCE_BENr, RTL9602BVB_BEN_FORCE_MODEf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
                return ret;
            }
#if 0
            /*WSDS_ANA_0E.REG_TX_EN_EMPHAS =0   REG_TX_AMP=1*/
            data = 0x0;
            if ((ret = reg_field_write(RTL9602BVB_WSDS_ANA_0Er, RTL9602BVB_REG_TX_EN_EMPHASf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_GPON | MOD_DAL), "");
                return ret;
            }
            data = 0x1;
            if ((ret = reg_field_write(RTL9602BVB_WSDS_ANA_0Er, RTL9602BVB_REG_TX_AMPf, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_GPON | MOD_DAL), "");
                return ret;
            }
            /*force MAC4 speed 1000M*/
            /*ABLTY_FORCE_MODE = 0x10 */
            data = 0x10;
            if ((ret = reg_write(RTL9602BVB_ABLTY_FORCE_MODEr, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
                return ret;
            }
            /*force spped to 1000M*/
            data = 0x76;
            if ((ret = reg_array_write(RTL9602BVB_FORCE_P_ABLTYr,4 ,REG_ARRAY_INDEX_NONE, &data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
                return ret;
            }
#endif
        }
        break;

        default:
            break;
    }

    /*WSDS_ANA_10*/
    /* <M> DEL REG
	data = 0x11B8;
    if ((ret = reg_write(RTL9602BVB_WSDS_ANA_10r, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }*/

    /*WSDS_ANA_1C*/
    /* <M> DEL REG
	data = 0;
    if ((ret = reg_write(RTL9602BVB_WSDS_ANA_1Cr, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }*/

#ifdef CONFIG_TX_POWER_TURN_ON_FEATURE
    /* turn on tx power */
    if((ret =_ponmac_txPower_enable(ENABLED)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC | MOD_DAL), "");
        return ret;
    }
#endif

	ponMode = mode;
	return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl9602bvb_ponmac_mode_get
 * Description:
 *      set pon mac mode
 * Input:
 *      mode         - pon mode, epon or gpon
 * Output:
 *      *pState       - mode state
 * Return:
 *      RT_ERR_OK              						- OK
 *      RT_ERR_FAILED          					- Failed
 *      RT_ERR_NULL_POINTER    					- Pointer pQueue point to NULL.
 *      RT_ERR_INPUT 							- Invalid input parameters.
 * Note:
 *      None
 */
int32
dal_rtl9602bvb_ponmac_mode_get(rtk_ponmac_mode_t *pMode)
{
 	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PONMAC),"%s",__FUNCTION__);

    /* check Init status */
   	RT_INIT_CHK(ponmac_init);

    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);

    *pMode=ponMode;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl9602bvb_ponmac_queueDrainOut_set
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
dal_rtl9602bvb_ponmac_queueDrainOut_set(rtk_ponmac_queue_t *pQueue)
{
    uint32 queueId;
	int32  ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PONMAC), "");

    /* check Init status */
    RT_INIT_CHK(ponmac_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pQueue), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pQueue->schedulerId >= HAL_MAX_NUM_OF_GPON_TCONT()), RT_ERR_INPUT);
    RT_PARAM_CHK((pQueue->queueId > RTL9602BVB_TCONT_QUEUE_MAX), RT_ERR_INPUT);

    /* function body */
    if(pQueue->queueId < RTL9602BVB_TCONT_QUEUE_MAX)
    {
        queueId = ((pQueue->schedulerId)/8) * 32 + pQueue->queueId;
        if ((ret = _rtl9602bvb_ponMacQueueDrainOutState_set(queueId)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
            return ret;
        }
    }
    else
    {
        if ((ret = _rtl9602bvb_ponMacTcontDrainOutState_set(pQueue->schedulerId)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
            return ret;
        }
    }

    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_ponmac_queueDrainOut_set */

/* Function Name:
 *      dal_rtl9602bvb_ponmac_opticalPolarity_get
 * Description:
 *      Get the current optical output polarity
 * Input:
 *      None
 * Output:
 *      pPolarity  - the current output polarity
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 */
int32
dal_rtl9602bvb_ponmac_opticalPolarity_get(rtk_ponmac_polarity_t *pPolarity)
{
	int32  ret;
	uint32 val;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PONMAC), "");

    /* check Init status */
    RT_INIT_CHK(ponmac_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPolarity), RT_ERR_NULL_POINTER);

    /* function body */
    if ((ret = reg_field_read(RTL9602BVB_WSDS_DIG_12r,RTL9602BVB_CFG_FRC_BEN_INVf,&val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    if(0 == val)
    {
        *pPolarity = PONMAC_POLARITY_HIGH;
    }
    else
    {
        *pPolarity = PONMAC_POLARITY_LOW;
    }

    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_ponmac_opticalPolarity_get */

/* Function Name:
 *      dal_rtl9602bvb_ponmac_opticalPolarity_set
 * Description:
 *      Set optical output polarity
 * Input:
 *      polarity - the optical output polarity
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
int32
dal_rtl9602bvb_ponmac_opticalPolarity_set(rtk_ponmac_polarity_t polarity)
{
	int32  ret;
	uint32 val;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PONMAC), "");

    /* check Init status */
    RT_INIT_CHK(ponmac_init);

    /* parameter check */
    RT_PARAM_CHK((PONMAC_POLARITY_END <= polarity), RT_ERR_INPUT);

    /* function body */
    if(PONMAC_POLARITY_HIGH == polarity)
    {
        val = 0;
    }
    else
    {
        val = 1;
    }

    if ((ret = reg_field_write(RTL9602BVB_WSDS_DIG_12r,RTL9602BVB_CFG_FRC_BEN_INVf,&val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_ponmac_opticalPolarity_set */

#if 0
/* Function Name:
 *      dal_rtl9602bvb_ponmac_losState_get
 * Description:
 *      Get the current optical lost of signal (LOS) state
 * Input:
 *      None
 * Output:
 *      pEnable  - the current optical lost of signal state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 */
int32
dal_rtl9602bvb_ponmac_losState_get(rtk_enable_t *pState)
{
	uint16  tmpVal;
 	int32   ret;
    rtl9602bvb_sds_acc_t sds_acc;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PONMAC), "");

    /* check Init status */
    RT_INIT_CHK(ponmac_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pState), RT_ERR_NULL_POINTER);

    /* function body */
    sds_acc.index = RTL9602BVB_SDS_IDX_PON;
    sds_acc.page = RTL9602BVB_SDS_PAGE_SDS_EXT;
    sds_acc.regaddr = 29;

    if ((ret = _rtl9602bvb_serdes_ind_read(sds_acc, &tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    if(1==(tmpVal & (1<<8)))
        *pState = DISABLED;
    else
        *pState = ENABLED;

    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_ponmac_losState_get */
#endif

/* Function Name:
 *      dal_rtl9602bvb_ponmac_sidValid_get
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
dal_rtl9602bvb_ponmac_sidValid_get(uint32 sid, rtk_enable_t *pValid)
{
	int32  ret;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PONMAC), "");

    /* check Init status */
    RT_INIT_CHK(ponmac_init);

    /* parameter check */
    RT_PARAM_CHK((HAL_MAX_NUM_OF_GPON_FLOW() <= sid), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pValid), RT_ERR_NULL_POINTER);

    /* function body */
    if ((ret = reg_array_field_read(RTL9602BVB_PON_SIDVALIDr,REG_ARRAY_INDEX_NONE, sid, RTL9602BVB_CFG_SID2VALIDf, pValid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_ponmac_sidValid_get */

/* Function Name:
 *      dal_rtl9602bvb_ponmac_sidValid_set
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
dal_rtl9602bvb_ponmac_sidValid_set(uint32 sid, rtk_enable_t valid)
{
	int32  ret;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PONMAC), "");

    /* check Init status */
    RT_INIT_CHK(ponmac_init);

    /* parameter check */
    RT_PARAM_CHK((HAL_MAX_NUM_OF_GPON_FLOW() <= sid), RT_ERR_INPUT);
    RT_PARAM_CHK((RTK_ENABLE_END <= valid), RT_ERR_INPUT);

    /* function body */

    if ((ret = reg_array_field_write(RTL9602BVB_PON_SIDVALIDr,REG_ARRAY_INDEX_NONE, sid, RTL9602BVB_CFG_SID2VALIDf, &valid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_ponmac_sidValid_set */


/* Function Name:
 *      dal_rtl9602bvb_ponmac_schedulingType_get
 * Description:
 *      Get the scheduling types and weights of queues on specific port in egress scheduling.
 * Input:
 *      None
 * Output:
 *      pQueueType  - the WFQ schedule type (DAL_RTL9602BVB_QOS_WFQ or DAL_RTL9602BVB_QOS_WRR)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 */
int32
dal_rtl9602bvb_ponmac_schedulingType_get(rtk_qos_scheduling_type_t *pQueueType)
{
    uint32  val;
	int32 ret;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PONMAC), "");

    /* check Init status */
    RT_INIT_CHK(ponmac_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pQueueType), RT_ERR_NULL_POINTER);

    /* function body */

    if ((ret = reg_field_read(RTL9602BVB_PON_SCH_CTRLr, RTL9602BVB_PON_WFQ_MODEf, &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PONMAC), "");
        return ret;
    }
    if(0==val)
        *pQueueType=RTK_QOS_WFQ;
    else
        *pQueueType=RTK_QOS_WRR;



    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_ponmac_schedulingType_get */


/* Function Name:
 *      dal_rtl9602bvb_ponmac_schedulingType_set
 * Description:
 *      Get the scheduling types and weights of queues on specific port in egress scheduling.
 * Input:
 *      None
 * Output:
 *      queueType  - the WFQ schedule type (DAL_RTL9602BVB_QOS_WFQ or DAL_RTL9602BVB_QOS_WRR)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 */
int32
dal_rtl9602bvb_ponmac_schedulingType_set(rtk_qos_scheduling_type_t queueType)
{
    uint32  val;
	int32 ret;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PONMAC), "queueType=%d",queueType);

    /* check Init status */
    RT_INIT_CHK(ponmac_init);

    /* parameter check */
    RT_PARAM_CHK((SCHEDULING_TYPE_END <=queueType), RT_ERR_INPUT);

    /* function body */
    if(RTK_QOS_WFQ==queueType)
        val=0;
    else
        val=1;
        
    if ((ret = reg_field_write(RTL9602BVB_PON_SCH_CTRLr, RTL9602BVB_PON_WFQ_MODEf, &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PONMAC), "");
        return ret;
    }        
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_ponmac_schedulingType_set */


/* Function Name:
 *      dal_rtl9602bvb_ponmac_egrBandwidthCtrlRate_get
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
dal_rtl9602bvb_ponmac_egrBandwidthCtrlRate_get(uint32 *pRate)
{
    uint32 data;
	int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PONMAC), "");

    /* check Init status */
    RT_INIT_CHK(ponmac_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pRate), RT_ERR_NULL_POINTER);

    /* function body */
    if ((ret = reg_field_read(RTL9602BVB_PON_EGR_RATEr, RTL9602BVB_RATEf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }
    
    *pRate = data;
    
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_ponmac_egrBandwidthCtrlRate_get */

/* Function Name:
 *      dal_rtl9602bvb_ponmac_egrBandwidthCtrlRate_set
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
 *      (2) The unit of granularity in rtl9602bvb is 8Kbps.
 */
int32
dal_rtl9602bvb_ponmac_egrBandwidthCtrlRate_set(uint32 rate)
{
    uint32 data;
	int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PONMAC), "rate=%d",rate);

    /* check Init status */
    RT_INIT_CHK(ponmac_init);

    /* parameter check */
    RT_PARAM_CHK((RTL9602BVB_EGRESS_RATE_MAX < rate), RT_ERR_INPUT);

    /* function body */
    data = rate;
    if ((ret = reg_field_write(RTL9602BVB_PON_EGR_RATEr, RTL9602BVB_RATEf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }
    
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_ponmac_egrBandwidthCtrlRate_set */



/* Function Name:
 *      dal_rtl9602bvb_ponmac_egrBandwidthCtrlRate_get
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
dal_rtl9602bvb_ponmac_egrScheduleIdRate_get(uint32 scheduleId,uint32 *pRate)
{
	int32 ret;
    uint32 data;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PONMAC), "scheduleId=%d",scheduleId);

    /* check Init status */
    RT_INIT_CHK(ponmac_init);

    /* parameter check */
    RT_PARAM_CHK((RTL9602BVB_GPON_TCONT_MAX <=scheduleId), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pRate), RT_ERR_NULL_POINTER);

    /* function body */
    if ((ret = reg_array_field_read(RTL9602BVB_PON_SCH_RATEr,REG_ARRAY_INDEX_NONE, scheduleId, RTL9602BVB_RATEf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }
    
    *pRate = data;
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_ponmac_egrScheduleIdRate_get */





/* Function Name:
 *      dal_rtl9602bvb_ponmac_egrBandwidthCtrlRate_set
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
dal_rtl9602bvb_ponmac_egrScheduleIdRate_set(uint32 scheduleId,uint32 rate)
{
	int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PONMAC), "scheduleId=%d,rate=%d",scheduleId, rate);

    /* check Init status */
    RT_INIT_CHK(ponmac_init);

    /* parameter check */
    RT_PARAM_CHK((RTL9602BVB_GPON_TCONT_MAX <=scheduleId), RT_ERR_INPUT);
    RT_PARAM_CHK((HAL_PONMAC_PIR_CIR_RATE_MAX() < rate), RT_ERR_INPUT);

    /* function body */
    if ((ret = reg_array_field_write(RTL9602BVB_PON_SCH_RATEr,REG_ARRAY_INDEX_NONE, scheduleId, RTL9602BVB_RATEf, &rate)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_ponmac_egrScheduleIdRate_set */


