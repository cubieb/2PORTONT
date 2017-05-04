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
 * Purpose : switch asic-level Mirror API
 * Feature : Mirror related functions
 *
 */

#include <dal/apollo/raw/apollo_raw_rldp.h>

/* Function Name:
 *      apollo_raw_rldp_dscRunout_set
 * Description:
 *      Set RLDP DSC runout action
 * Input:
 *      action   - RLDP action
 * Output:
 *      Note.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_dscRunout_set(apollo_raw_rldp_runoutDscAct_t action)
{
    int32 ret;

    RT_PARAM_CHK((RAW_RLDP_RUNOUT_ACTION_END <= action), RT_ERR_INPUT);

    if ((ret = reg_field_write(RLDP_CTRL_0r, ACT_RUNOUTDSCf, &action)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_dscRunout_set */

/* Function Name:
 *      apollo_raw_rldp_dscRunout_get
 * Description:
 *      Get RLDP DSC runout action
 * Input:
 *      None.
 * Output:
 *      pAction   - RLDP action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_dscRunout_get(apollo_raw_rldp_runoutDscAct_t *pAction)
{
    int32 ret;

    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);	

    if ((ret = reg_field_read(RLDP_CTRL_0r, ACT_RUNOUTDSCf, pAction)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_dscRunout_get */


/* Function Name:
 *      apollo_raw_rldp_trigger_set
 * Description:
 *      Set RLDP trigger mode
 * Input:
 *      mode   - RLDP trigger mode
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_trigger_set(apollo_raw_rldp_triggerMode_t mode)
{
    int32 ret;

    RT_PARAM_CHK((RAW_RLDP_TRIG_END<=mode), RT_ERR_INPUT);

    if ((ret = reg_field_write(RLDP_CTRL_1r, TRIG_MODEf, &mode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_trigger_set */

/* Function Name:
 *      apollo_raw_rldp_trigger_get
 * Description:
 *      Get RLDP trigger mode
 * Input:
 *      None. 
 * Output:
 *      pMode   - RLDP trigger mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_trigger_get(apollo_raw_rldp_triggerMode_t *pMode)
{
    int32 ret;

    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);	

    if ((ret = reg_field_read(RLDP_CTRL_1r, TRIG_MODEf, pMode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_trigger_get */


/* Function Name:
 *      apollo_raw_rldp_enable_set
 * Description:
 *      Set RLDP state
 * Input:
 *      mode   - RLDP state
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_enable_set(rtk_enable_t mode)
{
    int32 ret;

    RT_PARAM_CHK((RTK_ENABLE_END<=mode), RT_ERR_INPUT);

    if ((ret = reg_field_write(RLDP_CTRL_1r, ENf, &mode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_enable_set */

/* Function Name:
 *      apollo_raw_rldp_enable_get
 * Description:
 *      Get RLDP state
 * Input:
 *      None.
 * Output:
 *      pMode   - RLDP state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_enable_get(rtk_enable_t *pMode)
{
    int32 ret;

    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);	

    if ((ret = reg_field_read(RLDP_CTRL_1r, ENf, pMode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_enable_get */


/* Function Name:
 *      apollo_raw_rldp_indicateSrc_set
 * Description:
 *      Set RLDP LED/Buzzer source
 * Input:
 *      mode   - RLDP indication sourrce
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_indicateSrc_set(apollo_raw_rldp_indSrc_t mode)
{
    int32 ret;

    RT_PARAM_CHK((RAW_RLDP_INDSRC_END<=mode), RT_ERR_INPUT);

    if ((ret = reg_field_write(RLDP_CTRL_1r, IND_SRCf, &mode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_indicateSrc_set */

/* Function Name:
 *      apollo_raw_rldp_indicateSrc_get
 * Description:
 *      Get RLDP LED/Buzzer source
 * Input:
 *      None.
 * Output:
 *      pMode   - RLDP indication sourrce
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_indicateSrc_get(apollo_raw_rldp_indSrc_t *pMode)
{
    int32 ret;

    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);	

    if ((ret = reg_field_read(RLDP_CTRL_1r, IND_SRCf, pMode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_indicateSrc_get */

/* Function Name:
 *      apollo_raw_rldp_handle_set
 * Description:
 *      Set RLDP handle
 * Input:
 *      mode   - RLDP handle
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_handle_set(apollo_raw_rldp_handle_t mode)
{
    int32 ret;

    RT_PARAM_CHK((RAW_RLDP_HANDLE_END<=mode), RT_ERR_INPUT);

    if ((ret = reg_field_write(RLDP_CTRL_1r, CPU_HANDLEf, &mode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_handle_set */

/* Function Name:
 *      apollo_raw_rldp_handle_get
 * Description:
 *      Get RLDP handle
 * Input:
 *      None.
 * Output:
 *      pMode   - RLDP handle
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_handle_get(apollo_raw_rldp_handle_t *pMode)
{
    int32 ret;

    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);	

    if ((ret = reg_field_read(RLDP_CTRL_1r, CPU_HANDLEf, pMode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_handle_get */

/* Function Name:
 *      apollo_raw_rldp_compare_set
 * Description:
 *      Set RLDP packet compare
 * Input:
 *      mode   - RLDP packet compare
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_compare_set(apollo_raw_rldp_compare_t mode)
{
    int32 ret;

    RT_PARAM_CHK((RAW_RLDP_CMP_END<=mode), RT_ERR_INPUT);

    if ((ret = reg_field_write(RLDP_CTRL_1r, CMP_TYPEf, &mode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_compare_set */

/* Function Name:
 *      apollo_raw_rldp_compare_get
 * Description:
 *      Get RLDP packet compare
 * Input:
 *      None.
 * Output:
 *      pMode   - RLDP packet compare
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_compare_get(apollo_raw_rldp_compare_t *pMode)
{
    int32 ret;

    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);	

    if ((ret = reg_field_read(RLDP_CTRL_1r, CMP_TYPEf, pMode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_compare_get */

/* Function Name:
 *      apollo_raw_rldp_genRandom_set
 * Description:
 *      Set RLDP random number generation
 * Input:
 *      enable   - RLDP random number generation enable
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_genRandom_set(rtk_enable_t enable)
{
    int32 ret;

    RT_PARAM_CHK((RTK_ENABLE_END<=enable), RT_ERR_INPUT);

    if ((ret = reg_field_write(RLDP_CTRL_1r, GEN_RNDMf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_enable_set */

/* Function Name:
 *      apollo_raw_rldp_randomNum_get
 * Description:
 *      Get RLDP random number
 * Input:
 *      pRand - random number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_INPUT  				- Invalid input parameter
 * Note:
 *      None
 */
int32 apollo_raw_rldp_randomNum_get(rtk_mac_t *pRand)
{
    int32 ret;
    uint32 tmp[2];

    if(pRand == NULL)
        return RT_ERR_NULL_POINTER;	

    if ((ret = reg_field_read(RLDP_RNDM_NUMr, NUM_31_0f, &tmp[0])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_RLDP | MOD_DAL), "");
        return ret;
    }

    if ((ret = reg_field_read(RLDP_RNDM_NUMr, NUM_47_32f, &tmp[1])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_RLDP | MOD_DAL), "");
        return ret;
    }	

    pRand->octet[0] = (tmp[1]&0x0000FF00) >>8;
    pRand->octet[1] = tmp[1]&0x000000FF;	
    pRand->octet[2] = (tmp[0]&0xFF000000) >>24;
    pRand->octet[3] = (tmp[0]&0x00FF0000) >>16;
    pRand->octet[4] = (tmp[0]&0x0000FF00) >>8;
    pRand->octet[5] = tmp[0]&0x000000FF;
	 
    return RT_ERR_OK;
}/*end of apollo_raw_rldp_randomNum_get*/

/* Function Name:
 *      apollo_raw_rldp_magicNum_set
 * Description:
 *      Set RLDP magic number
 * Input:
 *      pMagic - magic number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_INPUT  				- Invalid input parameter
 * Note:
 *      None
 */
int32 apollo_raw_rldp_magicNum_set(rtk_mac_t *pMagic)
{
    int32 ret;
    uint32 tmp[2];
	
    if(pMagic == NULL)
        return RT_ERR_NULL_POINTER;	

    tmp[0] = (pMagic->octet[2]<<24) | (pMagic->octet[3]<<16) | (pMagic->octet[4]<<8) | (pMagic->octet[5]); 

    tmp[1] = (pMagic->octet[0]<<8) | (pMagic->octet[1]);

    if ((ret = reg_field_write(RLDP_MAGIC_NUMr, NUM_31_0f, &tmp[0])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_RLDP | MOD_DAL), "");
        return ret;
    }

    if ((ret = reg_field_write(RLDP_MAGIC_NUMr, NUM_47_32f, &tmp[1])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_RLDP | MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}/*end of apollo_raw_rldp_magicNum_set*/

/* Function Name:
 *      apollo_raw_rldp_magicNum_get
 * Description:
 *      Get magic number
 * Input:
 *      pMagic - magic number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_INPUT  				- Invalid input parameter
 * Note:
 *      None
 */
int32 apollo_raw_rldp_magicNum_get(rtk_mac_t *pMagic)
{
    int32 ret;
    uint32 tmp[2];

    if(pMagic == NULL)
        return RT_ERR_NULL_POINTER;	

    if ((ret = reg_field_read(RLDP_MAGIC_NUMr, NUM_31_0f, &tmp[0])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_RLDP | MOD_DAL), "");
        return ret;
    }

    if ((ret = reg_field_read(RLDP_MAGIC_NUMr, NUM_47_32f, &tmp[1])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_RLDP | MOD_DAL), "");
        return ret;
    }	

    pMagic->octet[0] = (tmp[1]&0x0000FF00) >>8;
    pMagic->octet[1] = tmp[1]&0x000000FF;	
    pMagic->octet[2] = (tmp[0]&0xFF000000) >>24;
    pMagic->octet[3] = (tmp[0]&0x00FF0000) >>16;
    pMagic->octet[4] = (tmp[0]&0x0000FF00) >>8;
    pMagic->octet[5] = tmp[0]&0x000000FF;
	 
    return RT_ERR_OK;
}/*end of apollo_raw_rldp_magicNum_get*/

/* Function Name:
 *      apollo_raw_rldp_checkStatePeriod_set
 * Description:
 *      Set RLDP check state period
 * Input:
 *      period   - RLDP check state period
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_checkStatePeriod_set(uint32 period)
{
    int32 ret;

    RT_PARAM_CHK((APOLLO_RLDP_PERIOD_MAX<period), RT_ERR_INPUT);

    if ((ret = reg_field_write(RLDP_CHK_STS_CTRLr, PERIODf, &period)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_checkStatePeriod_set */

/* Function Name:
 *      apollo_raw_rldp_checkStatePeriod_get
 * Description:
 *      Get RLDP check state period
 * Input:
 *      None.
 * Output:
 *      pPeriod   - RLDP check state period
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_checkStatePeriod_get(uint32 *pPeriod)
{
    int32 ret;

    RT_PARAM_CHK((NULL == pPeriod), RT_ERR_NULL_POINTER);	

    if ((ret = reg_field_read(RLDP_CHK_STS_CTRLr, PERIODf, pPeriod)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_checkStatePeriod_get */

/* Function Name:
 *      apollo_raw_rldp_checkStateCnt_set
 * Description:
 *      Set RLDP check state count
 * Input:
 *      count   - RLDP check state count
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_checkStateCnt_set(uint32 count)
{
    int32 ret;

    RT_PARAM_CHK((APOLLO_RLDP_RESEND_CNT_MAX<count), RT_ERR_INPUT);

    if ((ret = reg_field_write(RLDP_CHK_STS_CTRLr, CNTf, &count)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_checkStatePeriod_set */

/* Function Name:
 *      apollo_raw_rldp_checkStateCnt_get
 * Description:
 *      Get RLDP check state count
 * Input:
 *      None.
 * Output:
 *      pCount   - RLDP check state count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_checkStateCnt_get(uint32 *pCount)
{
    int32 ret;

    RT_PARAM_CHK((NULL == pCount), RT_ERR_NULL_POINTER);	

    if ((ret = reg_field_read(RLDP_CHK_STS_CTRLr, CNTf, pCount)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_checkStateCnt_get */

/* Function Name:
 *      apollo_raw_rldp_loopStatePeriod_set
 * Description:
 *      Set RLDP loop state period
 * Input:
 *      period   - RLDP loop state period
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_loopStatePeriod_set(uint32 period)
{
    int32 ret;

    RT_PARAM_CHK((APOLLO_RLDP_PERIOD_MAX<period), RT_ERR_INPUT);

    if ((ret = reg_field_write(RLDP_LP_STS_CTRLr, PERIODf, &period)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_loopStatePeriod_set */

/* Function Name:
 *      apollo_raw_rldp_loopStatePeriod_get
 * Description:
 *      Get RLDP loop state period
 * Input:
 *      None.
 * Output:
 *      pPeriod   - RLDP loop state period
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_loopStatePeriod_get(uint32 *pPeriod)
{
    int32 ret;

    RT_PARAM_CHK((NULL == pPeriod), RT_ERR_NULL_POINTER);	

    if ((ret = reg_field_read(RLDP_LP_STS_CTRLr, PERIODf, pPeriod)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_loopStatePeriod_get */

/* Function Name:
 *      apollo_raw_rldp_loopStateCnt_set
 * Description:
 *      Set RLDP loop state count
 * Input:
 *      count   - RLDP loop state count
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_loopStateCnt_set(uint32 count)
{
    int32 ret;

    RT_PARAM_CHK((APOLLO_RLDP_RESEND_CNT_MAX<count), RT_ERR_INPUT);

    if ((ret = reg_field_write(RLDP_LP_STS_CTRLr, CNTf, &count)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_loopStateCnt_set */

/* Function Name:
 *      apollo_raw_rldp_loopStateCnt_get
 * Description:
 *      Get RLDP loop state count
 * Input:
 *      None.
 * Output:
 *      pCount   - RLDP loop state count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_loopStateCnt_get(uint32 *pCount)
{
    int32 ret;

    RT_PARAM_CHK((NULL == pCount), RT_ERR_NULL_POINTER);	

    if ((ret = reg_field_read(RLDP_LP_STS_CTRLr, CNTf, pCount)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_loopStateCnt_get */


/* Function Name:
 *      apollo_raw_rldp_portEnable_set
 * Description:
 *      Set RLDP per port enable
 * Input:
 *      port	- port id
 *      enable	- RLDP enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_portEnable_set(rtk_port_t port,  rtk_enable_t enable)
{
    int32 ret;

    RT_PARAM_CHK(((HAL_GET_MAX_GE_PORT() < port) || (HAL_GET_RGMII_PORT() == port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RTK_ENABLE_END<= enable), RT_ERR_INPUT);	


    if (port < HAL_GET_RGMII_PORT() ) 
    {
        if ((ret = reg_array_field_write(RLDP_P1_0_TX_ENr, port, REG_ARRAY_INDEX_NONE, ENf, &enable)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
            return ret;
        }
    }
    else if (port > HAL_GET_RGMII_PORT() ) 
    {
        if ((ret = reg_array_field_write(RLDP_P3_5_TX_ENr, (port-HAL_GET_PON_PORT()), REG_ARRAY_INDEX_NONE, ENf, &enable)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_portEnable_set */


/* Function Name:
 *      apollo_raw_rldp_portEnable_get
 * Description:
 *      Get RLDP per port enable
 * Input:
 *      port	- port id
 * Output:
 *      pEnable   - RLDP enable
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_portEnable_get(rtk_port_t port,  rtk_enable_t *pEnable)
{
    int32 ret;

    RT_PARAM_CHK(((HAL_GET_MAX_GE_PORT() < port) || (HAL_GET_RGMII_PORT() == port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_INPUT);	


    if (port < HAL_GET_RGMII_PORT() ) 
    {
        if ((ret = reg_array_field_read(RLDP_P1_0_TX_ENr, port, REG_ARRAY_INDEX_NONE, ENf, pEnable)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
            return ret;
        }
    }
    else if (port > HAL_GET_RGMII_PORT() ) 
    {
        if ((ret = reg_array_field_read(RLDP_P3_5_TX_ENr, (port-HAL_GET_PON_PORT()), REG_ARRAY_INDEX_NONE, ENf, pEnable)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_portEnable_get */

/* Function Name:
 *      apollo_raw_rldp_portLoopEnterState_set
 * Description:
 *      Clear RLDP per port enter loop state
 * Input:
 *      port	- port id
 *      clear	- clear enter loop state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_portLoopEnterState_set(rtk_port_t port,  uint32  clear)
{
    int32 ret;

    RT_PARAM_CHK(((HAL_GET_MAX_GE_PORT() < port) || (HAL_GET_RGMII_PORT() == port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RTK_ENABLE_END<= clear), RT_ERR_INPUT);	


    if (port < HAL_GET_RGMII_PORT() ) 
    {
        if ((ret = reg_array_field_write(RLDP_P1_0_LP_ENTER_STSr, port, REG_ARRAY_INDEX_NONE, STSf, &clear)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
            return ret;
        }
    }
    else if (port > HAL_GET_RGMII_PORT() ) 
    {
        if ((ret = reg_array_field_write(RLDP_P3_5_LP_ENTER_STSr, (port-HAL_GET_PON_PORT()), REG_ARRAY_INDEX_NONE, STSf, &clear)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_portLoopEnterState_set */


/* Function Name:
 *      apollo_raw_rldp_portLoopEnterState_get
 * Description:
 *      Get RLDP per port enter loop state
 * Input:
 *      port	- port id
 * Output:
 *      pState   - RLDP enter loop state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_portLoopEnterState_get(rtk_port_t port,  apollo_raw_rldp_loopState_t *pState)
{
    int32 ret;

    RT_PARAM_CHK(((HAL_GET_MAX_GE_PORT() < port) || (HAL_GET_RGMII_PORT() == port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pState), RT_ERR_INPUT);	


    if (port < HAL_GET_RGMII_PORT() ) 
    {
        if ((ret = reg_array_field_read(RLDP_P1_0_LP_ENTER_STSr, port, REG_ARRAY_INDEX_NONE, STSf, pState)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
            return ret;
        }
    }
    else if (port > HAL_GET_RGMII_PORT() ) 
    {
        if ((ret = reg_array_field_read(RLDP_P3_5_LP_ENTER_STSr, (port-HAL_GET_PON_PORT()), REG_ARRAY_INDEX_NONE, STSf, pState)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_portLoopEnterState_get */

/* Function Name:
 *      apollo_raw_rldp_portLoopLeaveState_set
 * Description:
 *      Clear RLDP per port leave loop state
 * Input:
 *      port	- port id
 *      clear	- clear leave loop state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_portLoopLeaveState_set(rtk_port_t port,  uint32  clear)
{
    int32 ret;

    RT_PARAM_CHK(((HAL_GET_MAX_GE_PORT() < port) || (HAL_GET_RGMII_PORT() == port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RTK_ENABLE_END<= clear), RT_ERR_INPUT);	


    if (port < HAL_GET_RGMII_PORT() ) 
    {
        if ((ret = reg_array_field_write(RLDP_P1_0_LP_LEAVE_STSr, port, REG_ARRAY_INDEX_NONE, STSf, &clear)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
            return ret;
        }
    }
    else if (port > HAL_GET_RGMII_PORT() ) 
    {
        if ((ret = reg_array_field_write(RLDP_P3_5_LP_LEAVE_STSr, (port-HAL_GET_PON_PORT()), REG_ARRAY_INDEX_NONE, STSf, &clear)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_portLoopLeaveState_set */


/* Function Name:
 *      apollo_raw_rldp_portLoopLeaveState_get
 * Description:
 *      Get RLDP per port leave loop state
 * Input:
 *      port	- port id
 * Output:
 *      pState   - RLDP leave loop state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_portLoopLeaveState_get(rtk_port_t port,  apollo_raw_rldp_loopState_t *pState)
{
    int32 ret;

    RT_PARAM_CHK(((HAL_GET_MAX_GE_PORT() < port) || (HAL_GET_RGMII_PORT() == port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pState), RT_ERR_INPUT);	


    if (port < HAL_GET_RGMII_PORT() ) 
    {
        if ((ret = reg_array_field_read(RLDP_P1_0_LP_LEAVE_STSr, port, REG_ARRAY_INDEX_NONE, STSf, pState)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
            return ret;
        }
    }
    else if (port > HAL_GET_RGMII_PORT() ) 
    {
        if ((ret = reg_array_field_read(RLDP_P3_5_LP_LEAVE_STSr, (port-HAL_GET_PON_PORT()), REG_ARRAY_INDEX_NONE, STSf, pState)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_portLoopLeaveState_get */

/* Function Name:
 *      apollo_raw_rldp_portLoopState_get
 * Description:
 *      Get RLDP per port loop state
 * Input:
 *      port	- port id
 * Output:
 *      pState   - RLDP loop state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_portLoopState_get(rtk_port_t port,  apollo_raw_rldp_loopState_t *pState)
{
    int32 ret;

    RT_PARAM_CHK(((HAL_GET_MAX_GE_PORT() < port) || (HAL_GET_RGMII_PORT() == port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pState), RT_ERR_INPUT);	


    if (port < HAL_GET_RGMII_PORT() ) 
    {
        if ((ret = reg_array_field_read(RLDP_P1_0_LP_STSr, port, REG_ARRAY_INDEX_NONE, STSf, pState)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
            return ret;
        }
    }
    else if (port > HAL_GET_RGMII_PORT() ) 
    {
        if ((ret = reg_array_field_read(RLDP_P3_5_LP_STSr, (port-HAL_GET_PON_PORT()), REG_ARRAY_INDEX_NONE, STSf, pState)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_portLoopState_get */

/* Function Name:
 *      apollo_raw_rldp_portForceLoopState_set
 * Description:
 *      Set RLDP per port CPU force loop state
 * Input:
 *      port	- port id
 *      state	- force loop state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_portForceLoopState_set(rtk_port_t port,  apollo_raw_rldp_loopState_t  state)
{
    int32 ret;

    RT_PARAM_CHK(((HAL_GET_MAX_GE_PORT() < port) || (HAL_GET_RGMII_PORT() == port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RAW_RLDP_LOOP_STATE_END<= state), RT_ERR_INPUT);	


    if (port < HAL_GET_RGMII_PORT() ) 
    {
        if ((ret = reg_array_field_write(RLDP_P1_0_CPU_LP_STSr, port, REG_ARRAY_INDEX_NONE, STSf, &state)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
            return ret;
        }
    }
    else if (port > HAL_GET_RGMII_PORT() ) 
    {
        if ((ret = reg_array_field_write(RLDP_P3_5_CPU_LP_STSr, (port-HAL_GET_PON_PORT()), REG_ARRAY_INDEX_NONE, STSf, &state)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_portForceLoopState_set */


/* Function Name:
 *      apollo_raw_rldp_portForceLoopState_get
 * Description:
 *      Get RLDP per port CPU force loop state
 * Input:
 *      port	- port id
 * Output:
 *      pState   - force loop state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_portForceLoopState_get(rtk_port_t port,  apollo_raw_rldp_loopState_t *pState)
{
    int32 ret;

    RT_PARAM_CHK(((HAL_GET_MAX_GE_PORT() < port) || (HAL_GET_RGMII_PORT() == port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pState), RT_ERR_INPUT);	


    if (port < HAL_GET_RGMII_PORT() ) 
    {
        if ((ret = reg_array_field_read(RLDP_P1_0_CPU_LP_STSr, port, REG_ARRAY_INDEX_NONE, STSf, pState)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
            return ret;
        }
    }
    else if (port > HAL_GET_RGMII_PORT() ) 
    {
        if ((ret = reg_array_field_read(RLDP_P3_5_CPU_LP_STSr, (port-HAL_GET_PON_PORT()), REG_ARRAY_INDEX_NONE, STSf, pState)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_portForceLoopState_get */

/* Function Name:
 *      apollo_raw_rldp_portLoopPortNum_get
 * Description:
 *      Get RLDP per port loop port number
 * Input:
 *      port	- port id
 * Output:
 *      pLoopPort   - loop port number
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rldp_portLoopPortNum_get(rtk_port_t port,  rtk_port_t *pLoopPort)
{
    int32 ret;

    RT_PARAM_CHK(((HAL_GET_MAX_GE_PORT() < port) || (HAL_GET_RGMII_PORT() == port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pLoopPort), RT_ERR_INPUT);	


    if (port < HAL_GET_RGMII_PORT() ) 
    {
        if ((ret = reg_array_field_read(RLDP_P1_0_LP_PNUMr, port, REG_ARRAY_INDEX_NONE, P_NUMf, pLoopPort)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
            return ret;
        }
    }
    else if (port > HAL_GET_RGMII_PORT() ) 
    {
        if ((ret = reg_array_field_read(RLDP_P3_5_LP_PNUMr, (port-HAL_GET_PON_PORT()), REG_ARRAY_INDEX_NONE, P_NUMf, pLoopPort)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rldp_portLoopPortNum_get */



/* Function Name:
 *      apollo_raw_rlpp_trap_set
 * Description:
 *      Set RLPP packet trap to CPU
 * Input:
 *      enable   - enable trap
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rlpp_trap_set(rtk_enable_t enable)
{
    int32 ret;

    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    if ((ret = reg_field_write(RLPP_CTRLr, TRAP_ENf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rlpp_trap_set */

/* Function Name:
 *      apollo_raw_rlpp_trap_get
 * Description:
 *      Get RLPP packet action
 * Input:
 *      None.
 * Output:
 *      pEnable   - enable trap
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_rlpp_trap_get(rtk_enable_t *pEnable)
{
    int32 ret;

    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);	

    if ((ret = reg_field_read(RLPP_CTRLr, TRAP_ENf, pEnable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RLDP), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_rlpp_trap_get */








