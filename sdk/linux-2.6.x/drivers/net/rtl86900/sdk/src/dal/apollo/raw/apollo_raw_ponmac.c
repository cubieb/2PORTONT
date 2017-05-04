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
 * Purpose : switch asic-level PON MAC API 
 * Feature : PON MAC related functions
 *
 */

#include <dal/apollo/raw/apollo_raw_ponmac.h>


/* Function Name:
 *      apollo_raw_ponMacEgrRate_set
 * Description:
 *      Set PON Port egress rate
 * Input:
 *      rate 		- Egress rate of Ethernet Fiber mode, unit 8Kbps
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 * Note:
 *      None
 */
int32 apollo_raw_ponMacEgrRate_set(uint32 rate)
{

	uint32 set_val;
	int32 ret;

	/* Error Checking */
  
    /* set to register */
    set_val = rate;
    if ((ret = reg_field_write(PON_CFGr,EGR_RATEf, &set_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      apollo_raw_ponMacEgrRate_get
 * Description:
 *      Set PON Port egress rate
 * Input:
 *      rate 		- Egress rate of Ethernet Fiber mode, unit 8Kbps
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_ponMacEgrRate_get(uint32 *pRate)
{
	int32 ret;

	/* Error Checking */
    RT_PARAM_CHK((pRate  == NULL), RT_ERR_NULL_POINTER);
    
    /* read from register */
    if ((ret = reg_field_read(PON_CFGr,EGR_RATEf, pRate)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      apollo_raw_ponMacQueueDrainOut_set
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
int32 apollo_raw_ponMacQueueDrainOutState_set(uint32 qid)
{
	int32 ret;
#if 0    
    apollo_raw_ponmac_draintOutState_t state;
#endif
    uint32  data;
    uint32  tmpData;
    uint32  qidTmp;
    uint32  current;
    uint32  cnt;

	/* Error Checking */
    RT_PARAM_CHK((HAL_MAX_NUM_OF_PON_QUEUE() <= qid), RT_ERR_QUEUE_ID);

#if 0    
    /* set to register */
    if ((ret = reg_field_write(PON_DRAINOUT_CTRLr, QID_IDXf, &qid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    state = RAW_PONMAC_DRAINOUT_DOING;
    if ((ret = reg_field_write(PON_DRAINOUT_CTRLr, QID_ENf, &state)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }
#else
   
    reg_read(PON_PORT_CTRLr, &data);
    
    /* queue drain out */
    tmpData = 0;
    if ((ret = reg_field_set(PON_PORT_CTRLr, DRN_SELf, &tmpData,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_field_set(PON_PORT_CTRLr, DRN_IDXf, &qid,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }
    tmpData = 1;
    if ((ret = reg_field_set(PON_PORT_CTRLr, DRN_ENABLEf, &tmpData,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }


    /* drain out */
    if ((ret = reg_write(PON_PORT_CTRLr,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    /* check the queue is drained out */
    if(qid)
        qidTmp = 0;
    else
        qidTmp = 1;

    /*switch to other queue at the first*/
    if ((ret = reg_field_write(QUEUE_SEL_INDr, QCNT_SEL_INDf, &qidTmp)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }

    if ((ret = reg_field_write(QUEUE_SEL_INDr, QCNT_SEL_INDf, &qid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    cnt=0;
    do
    {
        if ((ret = reg_field_read(FC_PON_Q_USED_PAGE_CNTr, Q_USED_PAGE_CNTf, &current)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
            return ret;
        }
        cnt++;
    } while (current != 0 || cnt>1000);

    /* stop drain out */
    data = 0;
    if ((ret = reg_write(PON_PORT_CTRLr,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }


#endif
    return RT_ERR_OK;
}



/* Function Name:
 *      apollo_raw_ponMacQueueDrainOutState_get
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
int32 apollo_raw_ponMacQueueDrainOutState_get(apollo_raw_ponmac_draintOutState_t *pState)
{
	int32 ret;

	/* Error Checking */
    RT_PARAM_CHK((pState  == NULL), RT_ERR_NULL_POINTER);
     
    /* set to register */
    if ((ret = reg_field_read(PON_DRAINOUT_CTRLr, QID_ENf, pState)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      apollo_raw_ponMacTcontDrainOutState_set
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
int32 apollo_raw_ponMacTcontDrainOutState_set(uint32 tcont)
{
	int32 ret;
    apollo_raw_ponmac_draintOutState_t state;
    
	/* Error Checking */
    RT_PARAM_CHK((HAL_MAX_NUM_OF_GPON_TCONT() <= tcont), RT_ERR_GPON_EXCEED_MAX_TCONT);
    
    /* set to register */
    if ((ret = reg_field_write(PON_DRAINOUT_CTRLr, SCH_IDXf, &tcont)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    state = RAW_PONMAC_DRAINOUT_DOING;
    if ((ret = reg_field_write(PON_DRAINOUT_CTRLr, SCH_ENf, &state)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}



/* Function Name:
 *      apollo_raw_ponMacTcontDrainOutState_get
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
int32 apollo_raw_ponMacTcontDrainOutState_get(apollo_raw_ponmac_draintOutState_t *pState)
{
	int32 ret;

	/* Error Checking */
    RT_PARAM_CHK((pState  == NULL), RT_ERR_NULL_POINTER);
     
    /* set to register */
    if ((ret = reg_field_write(PON_DRAINOUT_CTRLr, SCH_ENf, pState)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}



/* Function Name:
 *      apollo_raw_ponMacSidToQueueMap_set
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
int32 apollo_raw_ponMacSidToQueueMap_set(uint32 sid, uint32 qid)
{
	int32 ret;
    
	/* Error Checking */
    RT_PARAM_CHK((HAL_MAX_NUM_OF_PON_QUEUE() <= qid), RT_ERR_QUEUE_ID);
    RT_PARAM_CHK((128 <= sid),RT_ERR_INPUT);
    
    /* set to register */
    if ((ret = reg_array_field_write(PON_QID_TO_SIDr,REG_ARRAY_INDEX_NONE, sid, QIDf, &qid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_ponMacSidToQueueMap_get
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
int32 apollo_raw_ponMacSidToQueueMap_get(uint32 sid, uint32 *pQid)
{
	int32 ret;
   
	/* Error Checking */
    RT_PARAM_CHK((128 <= sid),RT_ERR_INPUT);
    RT_PARAM_CHK((pQid  == NULL), RT_ERR_NULL_POINTER);
     
    /* set to register */
    if ((ret = reg_array_field_read(PON_QID_TO_SIDr,REG_ARRAY_INDEX_NONE, sid, QIDf, pQid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}



/* Function Name:
 *      apollo_raw_ponMacCirRate_set
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
int32 apollo_raw_ponMacCirRate_set(uint32 qid, uint32 rate)
{
	int32 ret;
    
	/* Error Checking */
    RT_PARAM_CHK((HAL_MAX_NUM_OF_PON_QUEUE() <= qid), RT_ERR_QUEUE_ID);
    RT_PARAM_CHK((HAL_PONMAC_PIR_CIR_RATE_MAX() < rate),RT_ERR_INPUT);
    
    /* set to register */
    if ((ret = reg_array_field_write(PON_QID_CIR_RATEr,REG_ARRAY_INDEX_NONE, qid, RATEf, &rate)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}




/* Function Name:
 *      apollo_raw_ponMacCirRate_get
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
int32 apollo_raw_ponMacCirRate_get(uint32 qid, uint32 *pRate)
{
	int32 ret;
    
	/* Error Checking */
    RT_PARAM_CHK((HAL_MAX_NUM_OF_PON_QUEUE() <= qid), RT_ERR_QUEUE_ID);
    RT_PARAM_CHK((pRate  == NULL), RT_ERR_NULL_POINTER);
    
    /* set to register */
    if ((ret = reg_array_field_read(PON_QID_CIR_RATEr,REG_ARRAY_INDEX_NONE, qid, RATEf, pRate)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}



/* Function Name:
 *      apollo_raw_ponMacPirRate_set
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
int32 apollo_raw_ponMacPirRate_set(uint32 qid, uint32 rate)
{
	int32 ret;
    
	/* Error Checking */
    RT_PARAM_CHK((HAL_MAX_NUM_OF_PON_QUEUE() <= qid), RT_ERR_QUEUE_ID);
    RT_PARAM_CHK((HAL_PONMAC_PIR_CIR_RATE_MAX() < rate),RT_ERR_INPUT);
    
    /* set to register */
    if ((ret = reg_array_field_write(PON_QID_PIR_RATEr,REG_ARRAY_INDEX_NONE, qid, RATEf, &rate)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}




/* Function Name:
 *      apollo_raw_ponMacPirRate_get
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
int32 apollo_raw_ponMacPirRate_get(uint32 qid, uint32 *pRate)
{
	int32 ret;
    
	/* Error Checking */
    RT_PARAM_CHK((HAL_MAX_NUM_OF_PON_QUEUE() <= qid), RT_ERR_QUEUE_ID);
    RT_PARAM_CHK((pRate  == NULL), RT_ERR_NULL_POINTER);
    
    /* set to register */
    if ((ret = reg_array_field_read(PON_QID_PIR_RATEr,REG_ARRAY_INDEX_NONE, qid, RATEf, pRate)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}





/* Function Name:
 *      apollo_raw_ponMacScheQmap_set
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
int32 apollo_raw_ponMacScheQmap_set(uint32 tcont, uint32 map)
{
	int32 ret;
    
	/* Error Checking */
    RT_PARAM_CHK((HAL_MAX_NUM_OF_GPON_TCONT() <= tcont), RT_ERR_GPON_EXCEED_MAX_TCONT);
    
    /* set to register */
    if ((ret = reg_array_field_write(PON_SCH_QMAPr,REG_ARRAY_INDEX_NONE, tcont, MAPPING_TBLf, &map)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}



/* Function Name:
 *      apollo_raw_ponMacScheQmap_get
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
int32 apollo_raw_ponMacScheQmap_get(uint32 tcont, uint32 *pMap)
{
	int32 ret;
    
	/* Error Checking */
    RT_PARAM_CHK((HAL_MAX_NUM_OF_GPON_TCONT() <= tcont), RT_ERR_GPON_EXCEED_MAX_TCONT);
    
    /* set to register */
    if ((ret = reg_array_field_read(PON_SCH_QMAPr,REG_ARRAY_INDEX_NONE, tcont, MAPPING_TBLf, pMap)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}




/* Function Name:
 *      apollo_raw_ponMac_queueType_set
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
int32 apollo_raw_ponMac_queueType_set(uint32 qid, apollo_raw_queue_type_t type)
{
	int32 ret;
    
	/* Error Checking */
    RT_PARAM_CHK((RAW_QOS_TYPE_END <= type), RT_ERR_INPUT);
    RT_PARAM_CHK((HAL_MAX_NUM_OF_PON_QUEUE() <= qid), RT_ERR_QUEUE_ID);
       
    /* set to register */
    if ((ret = reg_array_field_write(PON_WFQ_TYPEr,REG_ARRAY_INDEX_NONE, qid, QUEUE_TYPEf, &type)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;  
}


/* Function Name:
 *      apollo_raw_ponMac_queueType_get
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
int32 apollo_raw_ponMac_queueType_get(uint32 qid, apollo_raw_queue_type_t *pType)
{
	int32 ret;
    
	/* Error Checking */
    RT_PARAM_CHK((pType  == NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_MAX_NUM_OF_PON_QUEUE() <= qid), RT_ERR_QUEUE_ID);
       
    /* set to register */
    if ((ret = reg_array_field_read(PON_WFQ_TYPEr,REG_ARRAY_INDEX_NONE, qid, QUEUE_TYPEf, pType)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;  
}






/* Function Name:
 *      apollo_raw_ponMac_wfqWeight_set
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
int32 apollo_raw_ponMac_wfqWeight_set(uint32 qid, uint32 weight)
{
	int32 ret;
    
	/* Error Checking */
    RT_PARAM_CHK((128 <= weight), RT_ERR_INPUT);
    RT_PARAM_CHK((0 == weight), RT_ERR_INPUT);

    RT_PARAM_CHK((HAL_MAX_NUM_OF_PON_QUEUE() <= qid), RT_ERR_QUEUE_ID);
       
    /* set to register */
    if ((ret = reg_array_field_write(PON_WFQ_WEIGHTr,REG_ARRAY_INDEX_NONE, qid, WEIGHTf, &weight)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;  
}


/* Function Name:
 *      apollo_raw_ponMac_wfqWeight_get
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
int32 apollo_raw_ponMac_wfqWeight_get(uint32 qid, uint32 *pWeight)
{
	int32 ret;
    
	/* Error Checking */
    RT_PARAM_CHK((pWeight  == NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_MAX_NUM_OF_PON_QUEUE() <= qid), RT_ERR_QUEUE_ID);
       
    /* set to register */
    if ((ret = reg_array_field_read(PON_WFQ_WEIGHTr,REG_ARRAY_INDEX_NONE, qid, WEIGHTf, pWeight)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;  
}




/* Function Name:
 *      apollo_raw_ponMac_egrDropState_set
 * Description:
 *      Set PON MAC queue egress drop enable state
 * Input:
 *      qid 	- The queue ID wanted to set (0~127)
 *      state 	- egress drop enable state
  * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		
 *      RT_ERR_INPUT
 *      RT_ERR_QUEUE_ID
 * Note:
 *      None
 */
int32 apollo_raw_ponMac_egrDropState_set(uint32 qid, rtk_enable_t state)
{
	int32 ret;
	uint32 reg;
    
	/* Error Checking */
    RT_PARAM_CHK((RTK_ENABLE_END <= state ), RT_ERR_INPUT);
    RT_PARAM_CHK((HAL_MAX_NUM_OF_PON_QUEUE() <= qid), RT_ERR_QUEUE_ID);

    if (ENABLED == state)
        reg = 0;
    else
        reg = 1;  
             
    /* set to register */
    if ((ret = reg_array_field_write(FC_PON_Q_EGR_DROP_ENr,REG_ARRAY_INDEX_NONE, qid, THf, &reg)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;  
}




/* Function Name:
 *      apollo_raw_ponMac_egrDropState_get
 * Description:
 *      Get PON MAC queue egress drop enable state
 * Input:
 *      qid 	- The queue ID wanted to set (0~127)
  * Output:
 *      pState 	- egress drop enable state
 * Return:
 *      RT_ERR_OK 		
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_QUEUE_ID
 * Note:
 *      None
 */
int32 apollo_raw_ponMac_egrDropState_get(uint32 qid, rtk_enable_t *pState)
{
	int32 ret;
	uint32 reg;
    
	/* Error Checking */
    RT_PARAM_CHK((pState  == NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_MAX_NUM_OF_PON_QUEUE() <= qid), RT_ERR_QUEUE_ID);
       
    /* set to register */
    if ((ret = reg_array_field_read(FC_PON_Q_EGR_DROP_ENr,REG_ARRAY_INDEX_NONE, qid, THf, &reg)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    if (0 == reg)
        *pState = ENABLED;
    else
        *pState = DISABLED;
    return RT_ERR_OK;  
}




/* Function Name:
 *      apollo_raw_ponMacTcontEnable_set
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
int32 apollo_raw_ponMacTcontEnable_set(uint32 tcont, rtk_enable_t enable)
{
	int32 ret;
    
	/* Error Checking */
    RT_PARAM_CHK((HAL_MAX_NUM_OF_GPON_TCONT() <= tcont), RT_ERR_GPON_EXCEED_MAX_TCONT);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable ), RT_ERR_INPUT);
        
    /* set to register */
    if ((ret = reg_array_field_write(PON_TCONT_ENr,REG_ARRAY_INDEX_NONE, tcont, TCONT_ENf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}



/* Function Name:
 *      apollo_raw_ponMacTcontEnable_get
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
int32 apollo_raw_ponMacTcontEnable_get(uint32 tcont, rtk_enable_t *pEnable)
{
	int32 ret;
    
	/* Error Checking */
    RT_PARAM_CHK((HAL_MAX_NUM_OF_GPON_TCONT() <= tcont), RT_ERR_GPON_EXCEED_MAX_TCONT);
    RT_PARAM_CHK((pEnable  == NULL), RT_ERR_NULL_POINTER);
        
    /* set to register */
    if ((ret = reg_array_field_read(PON_TCONT_ENr,REG_ARRAY_INDEX_NONE, tcont, TCONT_ENf, pEnable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_PONMAC|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}
