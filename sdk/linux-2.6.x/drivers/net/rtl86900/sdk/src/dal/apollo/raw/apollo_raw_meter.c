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
 * Purpose : Definition those public shared meter APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Shared meter function configuration.
 *
 */

 #include <dal/apollo/raw/apollo_raw_meter.h>

 /* Function Name:
 *      apollo_raw_meter_rate_set
 * Description:
 *      Set share meter rate.
 * Input:
 *      meterIdx  - Shard meter index number (0~31)
 *      meterRate - Meter rate,unit: 8Kbps
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED              - Failed
 *      RT_ERR_OK 			    - Success
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
int32 apollo_raw_meter_rate_set(uint32 meterIdx, uint32 meterRate)
{
    int32 ret = RT_ERR_FAILED;
    uint32 data;

    RT_PARAM_CHK(meterIdx >= HAL_MAX_NUM_OF_METERING(), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(APOLLO_RATE_MAX < meterRate, RT_ERR_OUT_OF_RANGE);

    data = (meterRate >> 3);
    if ((ret = reg_array_field_write(METER_GLB_CTRLr, REG_ARRAY_INDEX_NONE, meterIdx, RATEf, &data)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_RATE|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}
/* Function Name:
 *      apollo_raw_meter_rate_get
 * Description:
 *      Get share meter global control.
 * Input:
 * meterIdx  - Shard meter index number (0~31)
 *
 * Output:
 *      pMeterRate  - Meter rate,unit: 8Kbps
 * Return:
 *      RT_ERR_OK 			    - Success
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER   - Null pointer
 * Note:
 *      None
 */
int32 apollo_raw_meter_rate_get(uint32 meterIdx, uint32 *pMeterRate)
{
    int32 ret = RT_ERR_FAILED;
    uint32 data;

    RT_PARAM_CHK(meterIdx >= HAL_MAX_NUM_OF_METERING(), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(NULL == pMeterRate, RT_ERR_NULL_POINTER);

	if (( ret = reg_array_field_read(METER_GLB_CTRLr, REG_ARRAY_INDEX_NONE, meterIdx, RATEf, &data)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_RATE|MOD_DAL), "");
        return ret;
    }

    *pMeterRate = (data << 3);
    return RT_ERR_OK;
}

 /* Function Name:
 *      apollo_raw_meter_ifg_set
 * Description:
 *      Set share meter global control.
 * Input:
 *      meterIdx  - Shard meter index number (0~31)
 *      ifg       - Share meter rate calculation with 20 bytes IPG.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			    - Success
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
int32 apollo_raw_meter_ifg_set(uint32 meterIdx, rtk_enable_t ifg)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK(meterIdx >= HAL_MAX_NUM_OF_METERING(), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(RTK_ENABLE_END <= ifg, RT_ERR_OUT_OF_RANGE);
    if ((ret = reg_array_field_write(METER_GLB_CTRLr, REG_ARRAY_INDEX_NONE, meterIdx, IFGf, &ifg)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_RATE|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}
/* Function Name:
 *      apollo_raw_meter_ifg_get
 * Description:
 *      Get share meter global control.
 * Input:
 * meterIdx  - Shard meter index number (0~31)
 *
 * Output:
 *      pIfg            - Share meter rate calculation with 20 bytes IPG.
 * Return:
 *      RT_ERR_OK 			    - Success
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER   - Null pointer
 * Note:
 *      None
 */
int32 apollo_raw_meter_ifg_get(uint32 meterIdx, rtk_enable_t *pIfg)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK(meterIdx >= HAL_MAX_NUM_OF_METERING(), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(NULL == pIfg, RT_ERR_NULL_POINTER);

    if (( ret = reg_array_field_read(METER_GLB_CTRLr, REG_ARRAY_INDEX_NONE, meterIdx, IFGf, pIfg)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_RATE|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      apollo_raw_meter_bucketSize_set
 * Description:
 *      Set bucket size of shared meter.
 * Input:
 *      meterIdx   - Shard meter index number (0~31)
 *      lbthreshold - Bucket size of shared meter
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			    - Success
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
int32 apollo_raw_meter_bucketSize_set(uint32 meterIdx, uint32 lbthreshold)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK(meterIdx >= HAL_MAX_NUM_OF_METERING(), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(APOLLO_METER_BUCKETSIZE_MAX < lbthreshold, RT_ERR_OUT_OF_RANGE);

    if ((ret = reg_array_field_write(METER_GLB_CTRLr, REG_ARRAY_INDEX_NONE, meterIdx, BUCKET_SIZEf, &lbthreshold)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_RATE|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}
/* Function Name:
 *      apollo_raw_meter_bucketSize_get
 * Description:
 *      Get Bucket size of shared meter.
 * Input:
 * meterIdx  - Shard meter index number (0~31)
 *
 * Output:
 *      lbthreshold - Bucket size of shared meter
 *
 * Return:
 *      RT_ERR_OK 			    - Success
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER   - Null pointer
 * Note:
 *      None
 */
int32 apollo_raw_meter_bucketSize_get(uint32 meterIdx, uint32 *plbthreshold)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK(meterIdx >= HAL_MAX_NUM_OF_METERING(), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(NULL == plbthreshold, RT_ERR_NULL_POINTER);

	if (( ret = reg_array_field_read(METER_GLB_CTRLr, REG_ARRAY_INDEX_NONE, meterIdx, BUCKET_SIZEf, plbthreshold)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_RATE|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_meter_exceed_set
 * Description:
 *      Set to clear meter leaky buckets exceeding status.
 * Input:
 *      meterIdx   - Shard meter index number (0~31)
  * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			    - Success
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
int32 apollo_raw_meter_exceed_set(uint32 meterIdx)
{
    int32 ret = RT_ERR_FAILED;
    uint32 tmpData;
    tmpData = 1;
    RT_PARAM_CHK(meterIdx >= HAL_MAX_NUM_OF_METERING(), RT_ERR_OUT_OF_RANGE);
    if ((ret = reg_array_field_write(METER_LB_EXCEED_STSr, REG_ARRAY_INDEX_NONE, meterIdx, LB_EXCEEDf, &tmpData)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_RATE|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}
/* Function Name:
 *      apollo_raw_meter_exceed_get
 * Description:
 *      Get meter leaky buckets exceeding status.
 * Input:
 *      meterIdx  - Shard meter index number (0~31)
 *
 * Output:
 *      pStatus - 0: rate doesn't exceed 1: rate exceeds
 *
 * Return:
 *      RT_ERR_OK 			    - Success
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER   - Null pointer
 * Note:
 *      None
 */
int32 apollo_raw_meter_exceed_get(uint32 meterIdx, uint32 *pStatus)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK(meterIdx >= HAL_MAX_NUM_OF_METERING(), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(NULL == pStatus, RT_ERR_NULL_POINTER);

	if (( ret = reg_array_field_read(METER_LB_EXCEED_STSr, REG_ARRAY_INDEX_NONE, meterIdx, LB_EXCEEDf, pStatus)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_RATE|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}
/* Function Name:
 *      apollo_raw_meter_btCtrOp_set
 * Description:
 *      Set to consume token exceed requirement or not.
 * Input:
 *      meterOp   - Consume token exceed requirement or not
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED              - Failed
 *      RT_ERR_OK 			    - Success
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
int32 apollo_raw_meter_btCtrOp_set(apollo_raw_meter_op_t meterOp)
{
    int32 ret = RT_ERR_FAILED;

    if ((ret = reg_field_write(METER_TB_CTRLr, METER_OPf, &meterOp)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_RATE|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}
/* Function Name:
 *      apollo_raw_meter_btCtrOp_get
 * Description:
 *      Get to consume token exceed requirement or not.
 * Input:
 * None
 *
 * Output:
 *      pMeterOp   - Consume token exceed requirement or not
 * Return:
 *      RT_ERR_FAILED              - Failed
 *      RT_ERR_OK 			    - Success
 *      RT_ERR_NULL_POINTER   - Null pointer
 * Note:
 *      None
 */
int32 apollo_raw_meter_btCtrOp_get(apollo_raw_meter_op_t *pMeterOp)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK(NULL == pMeterOp, RT_ERR_NULL_POINTER);
    if ((ret = reg_field_read(METER_TB_CTRLr, METER_OPf, pMeterOp)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_RATE|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_meter_btCtr_set
 * Description:
 *      Set the tick time of the leaky bucket for a meter block.
 * Input:
 *      tickPeriod   - Meter bucket refresh timing tick, uint 1/system clock frequency
 *      tkn              - Refresh bytes counter of shared meter
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			    - Success
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
int32 apollo_raw_meter_btCtr_set(uint32 tickPeriod, uint32 tkn)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK(APOLLO_RAW_METER_TICK_PERIOD_MAX < tickPeriod, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(APOLLO_RAW_METER_TKN_MAX < tkn, RT_ERR_OUT_OF_RANGE);

    if ((ret = reg_field_write(METER_TB_CTRLr, TICK_PERIODf, &tickPeriod)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_RATE|MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_field_write(METER_TB_CTRLr, TKNf, &tkn)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_RATE|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}
/* Function Name:
 *      apollo_raw_meter_btCtr_get
 * Description:
 *      Get the tick time of the leaky bucket for a meter block.
 * Input:
 * None
 *
 * Output:
 *      pTickPeriod   - Meter bucket refresh timing tick, uint 1/system clock frequency
 *      pTkn              - Refresh bytes counter of shared meter
 * Return:
 *      RT_ERR_FAILED              - Failed
 *      RT_ERR_OK 			    - Success
 *      RT_ERR_NULL_POINTER   - Null pointer
 * Note:
 *      None
 */
int32 apollo_raw_meter_btCtr_get(uint32 *pTickPeriod, uint32 *pTkn)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK(NULL == pTickPeriod, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(NULL == pTkn, RT_ERR_NULL_POINTER);
    if ((ret = reg_field_read(METER_TB_CTRLr, TICK_PERIODf, pTickPeriod)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_RATE|MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_field_read(METER_TB_CTRLr, TKNf, pTkn)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_RATE|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}
/* Function Name:
 *      apollo_raw_meter_ponBtCtrOp_set
 * Description:
 *      Set to consume token exceed requirement or not.
 * Input:
 *      meterOp   - Consume token exceed requirement or not
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED              - Failed
 *      RT_ERR_OK 			    - Success
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
int32 apollo_raw_meter_ponBtCtrOp_set(apollo_raw_meter_op_t meterOp)
{
    int32 ret = RT_ERR_FAILED;

    if ((ret = reg_field_write(PON_TB_CTRLr, METER_OPf, &meterOp)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_RATE|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}
/* Function Name:
 *      apollo_raw_meter_ponBtCtrOp_get
 * Description:
 *      Get to consume token exceed requirement or not.
 * Input:
 * None
 *
 * Output:
 *      pMeterOp   - Consume token exceed requirement or not
 * Return:
 *      RT_ERR_FAILED              - Failed
 *      RT_ERR_OK 			    - Success
 *      RT_ERR_NULL_POINTER   - Null pointer
 * Note:
 *      None
 */
int32 apollo_raw_meter_ponBtCtrOp_get(apollo_raw_meter_op_t *pMeterOp)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK(NULL == pMeterOp, RT_ERR_NULL_POINTER);
    if ((ret = reg_field_read(PON_TB_CTRLr, METER_OPf, pMeterOp)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_RATE|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_meter_ponBtCtr_set
 * Description:
 *      Set the tick time of the leaky bucket in PON MAC
 * Input:
 *      tickPeriod   - Meter bucket refresh timing tick for PON port, uint 1/system clock frequency
 *      tkn            - Refresh bytes counter of shared meter for PON port.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			    - Success
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
int32 apollo_raw_meter_ponBtCtr_set(uint32 tickPeriod, uint32 tkn)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK(APOLLO_RAW_METER_TICK_PERIOD_MAX < tickPeriod, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(APOLLO_RAW_METER_TKN_MAX < tkn, RT_ERR_OUT_OF_RANGE);

    if ((ret = reg_field_write(PON_TB_CTRLr, TICK_PERIODf, &tickPeriod)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_RATE|MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_field_write(PON_TB_CTRLr, TKNf, &tkn)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_RATE|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}
/* Function Name:
 *      apollo_raw_meter_ponBtCtr_get
 * Description:
 *      Get he tick time of the leaky bucket in PON MAC.
 * Input:
 * None
 *
 * Output:
 *      pTickPeriod   - Meter bucket refresh timing tick for PON port, uint 1/system clock frequency
 *      pTkn            - Refresh bytes counter of shared meter for PON port.
 * Return:
 *      RT_ERR_FAILED              - Failed
 *      RT_ERR_OK 			    - Success
 *      RT_ERR_NULL_POINTER   - Null pointer
 * Note:
 *      None
 */
int32 apollo_raw_meter_ponBtCtr_get(uint32 *pTickPeriod, uint32 *pTkn)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK(NULL == pTickPeriod, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(NULL == pTkn, RT_ERR_NULL_POINTER);
    if ((ret = reg_field_read(PON_TB_CTRLr, TICK_PERIODf, pTickPeriod)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_RATE|MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_field_read(PON_TB_CTRLr, TKNf, pTkn)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_RATE|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}

