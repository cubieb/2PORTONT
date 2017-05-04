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
 * Purpose : switch asic-level EPON API 
 * Feature : EPON related functions
 *
 */

#include <dal/apollo/raw/apollo_raw_epon.h>



/* Function Name:
 *      apollo_raw_epon_regMode_set
 * Description:
 *      Set EPON registeration mode
 * Input:
 *      mode 		- registeration mode (EPON_SW_REG,EPON_HW_REG)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_regMode_set(apollo_raw_epon_regMode_t mode)
{
 	int32   ret;
	uint32 tmp_val;   
	
	tmp_val = mode;	
    if ((ret = reg_field_write(EPON_RGSTR1r,HW_REGISTRATIONf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	 
    return RT_ERR_OK;   
}



/* Function Name:
 *      apollo_raw_epon_regMode_get
 * Description:
 *      Get EPON registeration mode
 * Input:
 *      mode 		- registeration mode (EPON_SW_REG,EPON_HW_REG)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_regMode_get(apollo_raw_epon_regMode_t *mode)
{
 	int32   ret;
	uint32 tmp_val;   
	
    if ((ret = reg_field_read(EPON_RGSTR1r,HW_REGISTRATIONf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	*mode = tmp_val;	

    return RT_ERR_OK;   
}




/* Function Name:
 *      apollo_raw_epon_regLlidIdx_set
 * Description:
 *      Set EPON registeration LLID index
 * Input:
 *      idx 		- registeration LLID index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_regLlidIdx_set(uint32 idx)
{
 	int32   ret;
	uint32 tmp_val;   

    RT_PARAM_CHK((idx >= 8), RT_ERR_INPUT);

	
	tmp_val = idx;	
    if ((ret = reg_field_write(EPON_RGSTR1r,REG_LLID_IDXf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	 
    return RT_ERR_OK;   
}



/* Function Name:
 *      apollo_raw_epon_regLlidIdx_get
 * Description:
 *      Get EPON registeration LLID index
 * Input:
 *      None
 * Output:
 *      idx 		- registeration LLID index
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_regLlidIdx_get(uint32 *idx)
{
 	int32   ret;
	uint32 tmp_val;   
	
    if ((ret = reg_field_read(EPON_RGSTR1r,REG_LLID_IDXf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	*idx = tmp_val;	

    return RT_ERR_OK;   
}



/* Function Name:
 *      apollo_raw_epon_regMac_set
 * Description:
 *      Set EPON registeration MAC address
 * Input:
 *      idx 		- registeration LLID index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_regMac_set(rtk_mac_t *mac)
{
 	int32   ret;
	uint8   tmp_octet[4];
  

	
	tmp_octet[3] = mac->octet[3];	
	tmp_octet[2] = mac->octet[2];	
	tmp_octet[1] = mac->octet[1];	
	tmp_octet[0] = mac->octet[0];	

    if ((ret = reg_field_write(EPON_RGSTR2r,REGISTER_MAC0f,(uint32 *)&tmp_octet)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }

	tmp_octet[3] = 0;	
	tmp_octet[2] = 0;	
	tmp_octet[1] = mac->octet[5];	
	tmp_octet[0] = mac->octet[4];	

    if ((ret = reg_field_write(EPON_RGSTR1r,REGISTER_MAC1f,(uint32 *)&tmp_octet)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }

	 
    return RT_ERR_OK;   
}



/* Function Name:
 *      apollo_raw_epon_regMac_get
 * Description:
 *      Get EPON registeration MAC address
 * Input:
 *      rtk_mac_t *mac 		- registeration MAC address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_regMac_get(rtk_mac_t *mac)
{
 	int32   ret;
	uint8   tmp_octet[4];
  

	

    if ((ret = reg_field_read(EPON_RGSTR2r,REGISTER_MAC0f,(uint32 *)&tmp_octet)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }

	mac->octet[3] = tmp_octet[3];	
	mac->octet[2] = tmp_octet[2];	
	mac->octet[1] = tmp_octet[1];	
	mac->octet[0] = tmp_octet[0];	


    if ((ret = reg_field_read(EPON_RGSTR1r,REGISTER_MAC1f,(uint32 *)&tmp_octet)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }

	mac->octet[4] = tmp_octet[0];	
	mac->octet[5] = tmp_octet[1];	

	 
    return RT_ERR_OK;   
}



/* Function Name:
 *      apollo_raw_epon_regReguest_set
 * Description:
 *      Set EPON registeration request mode
 * Input:
 *      mode 		- registeration request enable/disable 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_regReguest_set(rtk_enable_t mode)
{
 	int32   ret;
	uint32 tmp_val;   
	
    if(mode==DISABLED)
        tmp_val = 0;    
    else
        tmp_val = 1;
        
    if ((ret = reg_field_write(EPON_RGSTR3r,REGISTER_REQUESTf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	 
    return RT_ERR_OK;   
}



/* Function Name:
 *      apollo_raw_epon_regReguest_get
 * Description:
 *      Get EPON registeration request mode
 * Input:
 *      mode 		- registeration request enable/disable 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_regReguest_get(rtk_enable_t *mode)
{
 	int32   ret;
	uint32 tmp_val;   
	
    if ((ret = reg_field_read(EPON_RGSTR3r,REGISTER_REQUESTf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    if(tmp_val==0)
        *mode = DISABLED;    
    else
        *mode = ENABLED;

    return RT_ERR_OK;   
}




/* Function Name:
 *      apollo_raw_epon_regPendingGrantNum_set
 * Description:
 *      Set EPON registeration pendding grant number
 * Input:
 *      num		- pendding grant number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_regPendingGrantNum_set(uint8 num)
{
 	int32   ret;
	uint32 tmp_val;   

    RT_PARAM_CHK((num > 32), RT_ERR_INPUT);

	
	tmp_val = num;	
    if ((ret = reg_field_write(EPON_RGSTR3r,REG_PENDDING_GRANTf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	 
    return RT_ERR_OK;   
}



/* Function Name:
 *      apollo_raw_epon_regPendingGrantNum_get
 * Description:
 *      Get EPON registeration pendding grant number
 * Input:
 *      None 
 * Output:
 *      num		- pendding grant number * Return:
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_regPendingGrantNum_get(uint32 *num)
{
 	int32   ret;
	uint32 tmp_val;   
	
    if ((ret = reg_field_read(EPON_RGSTR3r,REG_PENDDING_GRANTf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	*num = tmp_val;	

    return RT_ERR_OK;   
}



/* Function Name:
 *      apollo_raw_epon_mpcpTimeoutVal_set
 * Description:
 *      Set EPON MPCP timeout value
 * Input:
 *      timeout_unit		- mpcp timeout value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      unit 10 ms, write to 0 for disable this timer
 */
int32 apollo_raw_epon_mpcpTimeoutVal_set(uint8 timeout_unit)
{
 	int32   ret;
	uint32 tmp_val;   
	
	tmp_val = timeout_unit;	

    if ((ret = reg_field_write(EPON_TIMER_CONFIG1r,MPCP_TIMEOUT_VALUEf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	 
    return RT_ERR_OK;   
}



/* Function Name:
 *      apollo_raw_epon_mpcpTimeoutVal_get
 * Description:
 *      Get EPON MPCP timeout value
 * Input:
 *      None 
 * Output:
 *      timeout_unit		- mpcp timeout value
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_mpcpTimeoutVal_get(uint8 *timeout_unit)
{
 	int32   ret;
	uint32  tmp_val;   
	
    if ((ret = reg_field_read(EPON_TIMER_CONFIG1r,MPCP_TIMEOUT_VALUEf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	*timeout_unit = tmp_val;	

    return RT_ERR_OK;   
}




/* Function Name:
 *      apollo_raw_epon_regLlidImr_set
 * Description:
 *      Set EPON LLID TX IMR
 * Input:
 *      mode		- enable /disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 */
int32 apollo_raw_epon_regLlidImr_set(rtk_enable_t mode)
{
 	int32   ret;
	uint32  tmp_val;   
	
    if(mode==DISABLED)
        tmp_val = 0;    
    else
        tmp_val = 1;
        
    if ((ret = reg_field_write(EPON_INTRr,REG_LLID_TX_IMRf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	 
    return RT_ERR_OK;   
}



/* Function Name:
 *      apollo_raw_epon_regLlidImr_get
 * Description:
 *      Get EPON LLID TX IMR
 * Input:
 *      None 
 * Output:
 *      mode		- enable /disable
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_regLlidImr_get(rtk_enable_t *mode)
{
 	int32   ret;
	uint32  tmp_val;   
	
    if ((ret = reg_field_read(EPON_INTRr,REG_LLID_TX_IMRf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    
    if(tmp_val==0)
        *mode = DISABLED;    
    else
        *mode = ENABLED;
        
    return RT_ERR_OK;   
}




/* Function Name:
 *      apollo_raw_epon_timeDriftImr_set
 * Description:
 *      Set EPON time drift IMR
 * Input:
 *      mode		- enable /disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 */
int32 apollo_raw_epon_timeDriftImr_set(rtk_enable_t mode)
{
 	int32   ret;
	uint32  tmp_val;   
	
    if(mode==DISABLED)
        tmp_val = 0;    
    else
        tmp_val = 1;
        
    if ((ret = reg_field_write(EPON_INTRr,TIME_DRIFT_IMRf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	 
    return RT_ERR_OK;   
}



/* Function Name:
 *      apollo_raw_epon_timeDriftImr_get
 * Description:
 *      Get EPON time drift IMR
 * Input:
 *      None 
 * Output:
 *      mode		- enable /disable
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_timeDriftImr_get(rtk_enable_t *mode)
{
 	int32   ret;
	uint32  tmp_val;   
	
    if ((ret = reg_field_read(EPON_INTRr,TIME_DRIFT_IMRf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    
    if(tmp_val==0)
        *mode = DISABLED;    
    else
        *mode = ENABLED;
        
    return RT_ERR_OK;   
}



/* Function Name:
 *      apollo_raw_epon_mpcpTimeoutImr_set
 * Description:
 *      Set EPON mpcp timeout IMR
 * Input:
 *      mode		- enable /disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 */
int32 apollo_raw_epon_mpcpTimeoutImr_set(rtk_enable_t mode)
{
 	int32   ret;
	uint32  tmp_val;   
	
    if(mode==DISABLED)
        tmp_val = 0;    
    else
        tmp_val = 1;
        
    if ((ret = reg_field_write(EPON_INTRr,MPCP_TIMEOUT_IMRf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	 
    return RT_ERR_OK;   
}



/* Function Name:
 *      apollo_raw_epon_mpcpTimeoutImr_get
 * Description:
 *      Get EPON mpcp timeout IMR
 * Input:
 *      None 
 * Output:
 *      mode		- enable /disable
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_mpcpTimeoutImr_get(rtk_enable_t *mode)
{
 	int32   ret;
	uint32  tmp_val;   
	
    if ((ret = reg_field_read(EPON_INTRr,MPCP_TIMEOUT_IMRf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    
    if(tmp_val==0)
        *mode = DISABLED;    
    else
        *mode = ENABLED;
        
    return RT_ERR_OK;   
}



/* Function Name:
 *      apollo_raw_epon_mpcpTimeoutIms_clear
 * Description:
 *      Clear EPON mpcp timeout IMS
 * Input:
 *      mode		- enable /disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 */
int32 apollo_raw_epon_mpcpTimeoutIms_clear(void)
{
 	int32   ret;
	uint32  tmp_val;   
	
    tmp_val = 1;
        
    if ((ret = reg_field_write(EPON_INTRr,MPCP_TIMEOUT_IMSf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	 
    return RT_ERR_OK;   
}




/* Function Name:
 *      apollo_raw_epon_timeDriftIms_clear
 * Description:
 *      Clear EPON time drift IMS
 * Input:
 *      mode		- enable /disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 */
int32 apollo_raw_epon_timeDriftIms_clear(void)
{
 	int32   ret;
	uint32  tmp_val;   
	
    tmp_val = 1;
        
    if ((ret = reg_field_write(EPON_INTRr,TIME_DRIFT_IMSf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	 
    return RT_ERR_OK;   
}




/* Function Name:
 *      apollo_raw_epon_regLlidTxIms_clear
 * Description:
 *      Clear EPON LLID TX IMS
 * Input:
 *      mode		- enable /disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 */
int32 apollo_raw_epon_regLlidTxIms_clear(void)
{
 	int32   ret;
	uint32  tmp_val;   
	
    tmp_val = 1;
        
    if ((ret = reg_field_write(EPON_INTRr,REG_LLID_TX_IMSf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	 
    return RT_ERR_OK;   
}



/* Function Name:
 *      apollo_raw_epon_mpcpTimeoutImr_get
 * Description:
 *      Get EPON mpcp timeout IMS
 * Input:
 *      None 
 * Output:
 *      status		- enable /disable
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_mpcpTimeoutIms_get(rtk_enable_t *status)
{
 	int32   ret;
	uint32  tmp_val;   
	
    if ((ret = reg_field_read(EPON_INTRr,MPCP_TIMEOUT_IMSf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    
    if(tmp_val==0)
        *status = DISABLED;    
    else
        *status = ENABLED;
        
    return RT_ERR_OK;   
}

/* Function Name:
 *      apollo_raw_epon_timeDriftIms_get
 * Description:
 *      Get EPON time drift IMS
 * Input:
 *      None 
 * Output:
 *      status		- enable /disable
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_timeDriftIms_get(rtk_enable_t *status)
{
 	int32   ret;
	uint32  tmp_val;   
	
    if ((ret = reg_field_read(EPON_INTRr,TIME_DRIFT_IMSf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    
    if(tmp_val==0)
        *status = DISABLED;    
    else
        *status = ENABLED;
        
    return RT_ERR_OK;   
}


/* Function Name:
 *      apollo_raw_epon_llidTxIms_get
 * Description:
 *      Get EPON LLID TX IMS
 * Input:
 *      None 
 * Output:
 *      status		- enable /disable
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_llidTxIms_get(rtk_enable_t *status)
{
 	int32   ret;
	uint32  tmp_val;   
	
    if ((ret = reg_field_read(EPON_INTRr,REG_LLID_TX_IMSf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    
    if(tmp_val==0)
        *status = DISABLED;    
    else
        *status = ENABLED;
        
    return RT_ERR_OK;   
}




/* Function Name:
 *      apollo_raw_epon_discSyncTime_get
 * Description:
 *      Get EPON discovery sync time
 * Input:
 *      None 
 * Output:
 *      syncTime		- discovery sync time
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_discSyncTime_get(uint32 *syncTime)
{
 	int32   ret;
	uint32  tmp_val;   
	
    if ((ret = reg_field_read(SYNC_TIMEr,DISC_SYNC_TIMEf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    
    *syncTime = tmp_val;    
        
    return RT_ERR_OK;   
}



/* Function Name:
 *      apollo_raw_epon_discSyncTime_set
 * Description:
 *      Set EPON discovery sync time
 * Input:
 *      syncTime		- discovery sync time
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_discSyncTime_set(uint32 syncTime)
{
 	int32   ret;
	
    if ((ret = reg_field_write(SYNC_TIMEr,DISC_SYNC_TIMEf,&syncTime)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    
       
    return RT_ERR_OK;   
}


/* Function Name:
 *      apollo_raw_epon_normalSyncTime_get
 * Description:
 *      Get EPON normal sync time
 * Input:
 *      None 
 * Output:
 *      syncTime		- discovery sync time
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_normalSyncTime_get(uint32 *syncTime)
{
 	int32   ret;
	uint32  tmp_val;   
	
    if ((ret = reg_field_read(SYNC_TIMEr,NORMAL_SYNC_TIMEf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    
    *syncTime = tmp_val;    
        
    return RT_ERR_OK;   
}



/* Function Name:
 *      apollo_raw_epon_normalSyncTime_set
 * Description:
 *      Set EPON normal sync time
 * Input:
 *      syncTime		- normal sync time
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_normalSyncTime_set(uint32 syncTime)
{
 	int32   ret;
	
    if ((ret = reg_field_write(SYNC_TIMEr,DISC_SYNC_TIMEf,&syncTime)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
       
    return RT_ERR_OK;   
}




/* Function Name:
 *      apollo_raw_epon_laserOnTime_set
 * Description:
 *      Set EPON laser on time
 * Input:
 *      syncTime		- normal sync time
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_laserOnTime_set(uint32 laserTime)
{
 	int32   ret;
	
    if ((ret = reg_field_write(LASER_ON_OFF_TIMEr,LASER_ON_TIMEf,&laserTime)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
       
    return RT_ERR_OK;   
}




/* Function Name:
 *      apollo_raw_epon_laserOffTime_set
 * Description:
 *      Set EPON laser off time
 * Input:
 *      syncTime		- normal sync time
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_laserOffTime_set(uint32 laserTime)
{
 	int32   ret;
	
    if ((ret = reg_field_write(LASER_ON_OFF_TIMEr,LASER_OFF_TIMEf,&laserTime)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
       
    return RT_ERR_OK;   
}



/* Function Name:
 *      apollo_raw_epon_laserOnTime_get
 * Description:
 *      Get EPON laser on time
 * Input:
 *      None 
 * Output:
 *      syncTime		- discovery sync time
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_laserOnTime_get(uint32 *laserTime)
{
 	int32   ret;
	uint32  tmp_val;   
	
    if ((ret = reg_field_read(LASER_ON_OFF_TIMEr,LASER_ON_TIMEf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    
    *laserTime = tmp_val;    
        
    return RT_ERR_OK;   
}



/* Function Name:
 *      apollo_raw_epon_laserOffTime_get
 * Description:
 *      Get EPON laser on time
 * Input:
 *      None 
 * Output:
 *      syncTime		- discovery sync time
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_laserOffTime_get(uint32 *laserTime)
{
 	int32   ret;
	uint32  tmp_val;   
	
    if ((ret = reg_field_read(LASER_ON_OFF_TIMEr,LASER_OFF_TIMEf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    
    *laserTime = tmp_val;    
        
    return RT_ERR_OK;   
}




/* Function Name:
 *      apollo_raw_epon_quardThd_get
 * Description:
 *      Get quard threshold
 * Input:
 *      None 
 * Output:
 *      quardTh		- quard threshold
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_quardThd_get(apollo_raw_epon_quardThd_t *quardTh)
{
 	int32   ret;
	uint32  tmp_val;   
	
    if ((ret = reg_field_read(EPON_TIME_CTRLr,QUARD_THRESHOLDf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    
    *quardTh = tmp_val;    
        
    return RT_ERR_OK;   
}




/* Function Name:
 *      apollo_raw_epon_quardThd_set
 * Description:
 *      Set quard threshold
 * Input:
 *      quardTh		- quard threshold
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_quardThd_set(apollo_raw_epon_quardThd_t quardTh)
{
 	int32   ret;
	uint32  tmp_val; 
	
	tmp_val = (uint32)quardTh;	
    if ((ret = reg_field_write(EPON_TIME_CTRLr,QUARD_THRESHOLDf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
       
    return RT_ERR_OK;   
}

/* Function Name:
 *      apollo_raw_epon_rttAdj_get
 * Description:
 *      Get EPON RTT adjust value
 * Input:
 *      None 
 * Output:
 *      rttAdj		
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_rttAdj_get(int16 *rttAdj)
{
 	int32   ret;
	uint32  tmp_val;   
	int16   *ptmp_int_val; 
    if ((ret = reg_field_read(EPON_TIME_CTRLr,RTT_ADJf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    ptmp_int_val = (int16 *)&tmp_val;
    ptmp_int_val ++;
    
    *rttAdj = *ptmp_int_val;    
        
    return RT_ERR_OK;   
}


/* Function Name:
 *      apollo_raw_epon_rttAdj_set
 * Description:
 *      Set EPON RTT adjust value
 * Input:
 *      None 
 * Output:
 *      rttAdj		
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_rttAdj_set(int16 rttAdj)
{
 	int32   ret;
	uint32  tmp_val;   
	int16   *ptmp_int_val; 
	
	ptmp_int_val = (int16 *)&tmp_val;
	ptmp_int_val ++;
	
	*ptmp_int_val = rttAdj;
	
    if ((ret = reg_field_write(EPON_TIME_CTRLr,RTT_ADJf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
  
    return RT_ERR_OK;   
}


/* Function Name:
 *      apollo_raw_epon_StopLocalTime_set
 * Description:
 *      Set EPON local time stop mode
 * Input:
 *      None 
 * Output:
 *      mode		
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_StopLocalTime_set(rtk_enable_t enable)
{
 	int32   ret;
	uint32  tmp_val;   
	
    if(enable==DISABLED)
        tmp_val = 0;    
    else
        tmp_val = 1;
    if ((ret = reg_field_write(EP_MISCr,STOP_LOCAL_TIMEf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
  
    return RT_ERR_OK;   
}


/* Function Name:
 *      apollo_raw_epon_StopLocalTime_get
 * Description:
 *      Set EPON local time stop mode
 * Input:
 *      None 
 * Output:
 *      mode		
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_StopLocalTime_get(rtk_enable_t *enable)
{
 	int32   ret;
	uint32  tmp_val;   

    if ((ret = reg_field_read(EP_MISCr,STOP_LOCAL_TIMEf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	
    if(tmp_val==0)
        *enable = DISABLED;    
    else
        *enable = ENABLED; 
    return RT_ERR_OK;   
}





/* Function Name:
 *      apollo_raw_epon_fecEnable_set
 * Description:
 *      Set EPON FEC enable mode
 * Input:
 *      None 
 * Output:
 *      mode		
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_fecEnable_set(rtk_enable_t enable)
{
 	int32   ret;
	uint32  tmp_val;   
	
    if(enable==DISABLED)
        tmp_val = 0;    
    else
        tmp_val = 1;
    if ((ret = reg_field_write(EP_MISCr,FEC_ENABLEf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
  
    return RT_ERR_OK;   
}


/* Function Name:
 *      apollo_raw_epon_fecEnable_get
 * Description:
 *      Set EPON local time stop mode
 * Input:
 *      None 
 * Output:
 *      mode		
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_fecEnable_get(rtk_enable_t *enable)
{
 	int32   ret;
	uint32  tmp_val;   

    if ((ret = reg_field_read(EP_MISCr,FEC_ENABLEf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	
    if(tmp_val==0)
        *enable = DISABLED;    
    else
        *enable = ENABLED; 
        
        
    return RT_ERR_OK;   
}





/* Function Name:
 *      apollo_raw_epon_fecUsEnable_set
 * Description:
 *      Set EPON FEC upstream enable mode
 * Input:
 *      None 
 * Output:
 *      mode		
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_fecUsEnable_set(rtk_enable_t enable)
{
 	int32   ret;
	uint32  tmp_val;   
	
    if(enable==DISABLED)
        tmp_val = 0;    
    else
        tmp_val = 1;
    if ((ret = reg_field_write(EPON_FEC_CONFIGr,FEC_US_ENf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
  
    return RT_ERR_OK;   
}


/* Function Name:
 *      apollo_raw_epon_fecUsEnable_get
 * Description:
 *      Get EPON FEC upstream enable mode
 * Input:
 *      None 
 * Output:
 *      mode		
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_fecUsEnable_get(rtk_enable_t *enable)
{
 	int32   ret;
	uint32  tmp_val;   

    if ((ret = reg_field_read(EPON_FEC_CONFIGr,FEC_US_ENf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	
    if(tmp_val==0)
        *enable = DISABLED;    
    else
        *enable = ENABLED; 
        
        
    return RT_ERR_OK;   
}




/* Function Name:
 *      apollo_raw_epon_fecDsEnable_set
 * Description:
 *      Set EPON FEC down stream enable mode
 * Input:
 *      None 
 * Output:
 *      mode		
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_fecDsEnable_set(rtk_enable_t enable)
{
 	int32   ret;
	uint32  tmp_val;   
	
    if(enable==DISABLED)
        tmp_val = 0;    
    else
        tmp_val = 1;
    if ((ret = reg_field_write(EPON_FEC_CONFIGr,FEC_DS_ENf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
  
    return RT_ERR_OK;   
}


/* Function Name:
 *      apollo_raw_epon_fecDsEnable_get
 * Description:
 *      Get EPON FEC downstream enable mode
 * Input:
 *      None 
 * Output:
 *      mode		
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_fecDsEnable_get(rtk_enable_t *enable)
{
 	int32   ret;
	uint32  tmp_val;   

    if ((ret = reg_field_read(EPON_FEC_CONFIGr,FEC_DS_ENf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	
    if(tmp_val==0)
        *enable = DISABLED;    
    else
        *enable = ENABLED; 
        
        
    return RT_ERR_OK;   
}




/* Function Name:
 *      apollo_raw_epon_powerSavingEnable_set
 * Description:
 *      Set EPON power saving enable mode
 * Input:
 *      None 
 * Output:
 *      mode		
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_powerSavingEnable_set(rtk_enable_t enable)
{
 	int32   ret;
	uint32  tmp_val;   
	
    if(enable==DISABLED)
        tmp_val = 0;    
    else
        tmp_val = 1;
    if ((ret = reg_field_write(EP_MISCr,POWER_SAVING_ENf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
  
    return RT_ERR_OK;   
}


/* Function Name:
 *      apollo_raw_epon_powerSavingEnable_get
 * Description:
 *      Set EPON local time stop mode
 * Input:
 *      None 
 * Output:
 *      mode		
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_powerSavingEnable_get(rtk_enable_t *enable)
{
 	int32   ret;
	uint32  tmp_val;   

    if ((ret = reg_field_read(EP_MISCr,POWER_SAVING_ENf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	
    if(tmp_val==0)
        *enable = DISABLED;    
    else
        *enable = ENABLED; 
        
        
    return RT_ERR_OK;   
}




/* Function Name:
 *      apollo_raw_epon_powerSavingStatus_get
 * Description:
 *      Set EPON local time stop mode
 * Input:
 *      None 
 * Output:
 *      mode		
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_powerSavingStatus_get(rtk_enable_t *enable)
{
 	int32   ret;
	uint32  tmp_val;   

    if ((ret = reg_field_read(EP_MISCr,POWER_SAVING_MODEf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	
    if(tmp_val==0)
        *enable = DISABLED;    
    else
        *enable = ENABLED; 
        
        
    return RT_ERR_OK;   
}





/* Function Name:
 *      apollo_raw_epon_mpcpGateHandle_get
 * Description:
 *      Get EPON mpcp gate handle mode
 * Input:
 *      None 
 * Output:
 *      mode		
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_mpcpGateHandle_get(apollo_raw_epon_gatehandle_t *mode)
{
 	int32   ret;
	uint32  tmp_val;   

    if ((ret = reg_field_read(EPON_MPCP_CTRr,GATE_HANDLEf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	
    *mode = tmp_val;    
        
    return RT_ERR_OK;   
}




/* Function Name:
 *      apollo_raw_epon_mpcpGateHandle_set
 * Description:
 *      Set EPON mpcp gate handle mode
 * Input:
 *      None 
 * Output:
 *      mode		
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_mpcpGateHandle_set(apollo_raw_epon_gatehandle_t mode)
{
 	int32   ret;
	uint32  tmp_val;   
    
    tmp_val = mode;   
    
    if ((ret = reg_field_write(EPON_MPCP_CTRr,GATE_HANDLEf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	
     
        
    return RT_ERR_OK;   
}







/* Function Name:
 *      apollo_raw_epon_mpcpOtherHandle_get
 * Description:
 *      Get EPON mpcp opcode not gate and register handle mode
 * Input:
 *      None 
 * Output:
 *      mode		
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_mpcpOtherHandle_get(apollo_raw_epon_mpcpHandle_t *mode)
{
 	int32   ret;
	uint32  tmp_val;   

    if ((ret = reg_field_read(EPON_MPCP_CTRr,OTHER_HANDLEf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	
    *mode = tmp_val;    
        
    return RT_ERR_OK;   
}




/* Function Name:
 *      apollo_raw_epon_mpcpOtherHandle_set 
 * Description:
 *      Set EPON mpcp opcode not gate and register handle mode
 * Input:
 *      None 
 * Output:
 *      mode		
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_mpcpOtherHandle_set(apollo_raw_epon_mpcpHandle_t mode)
{
 	int32   ret;
	uint32  tmp_val;   
    
    tmp_val = mode;   
    
    if ((ret = reg_field_write(EPON_MPCP_CTRr,OTHER_HANDLEf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	
     
        
    return RT_ERR_OK;   
}



/* Function Name:
 *      apollo_raw_epon_mpcpInvalidLenHandle_get
 * Description:
 *      Get EPON mpcp packet length not 64 bytes handle mode
 * Input:
 *      None 
 * Output:
 *      mode		
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_mpcpInvalidLenHandle_get(apollo_raw_epon_mpcpHandle_t *mode)
{
 	int32   ret;
	uint32  tmp_val;   

    if ((ret = reg_field_read(EPON_MPCP_CTRr,INVALID_LEN_HANDLEf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	
    *mode = tmp_val;    
        
    return RT_ERR_OK;   
}




/* Function Name:
 *      apollo_raw_epon_mpcpInvalidLenHandle_set 
 * Description:
 *      Set  EPON mpcp packet length not 64 bytes handle mode
 * Input:
 *      None 
 * Output:
 *      mode		
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_mpcpInvalidLenHandle_set(apollo_raw_epon_mpcpHandle_t mode)
{
 	int32   ret;
	uint32  tmp_val;   
    
    tmp_val = mode;   
    
    if ((ret = reg_field_write(EPON_MPCP_CTRr,INVALID_LEN_HANDLEf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	
     
        
    return RT_ERR_OK;   
}




/* Function Name:
 *      apollo_raw_epon_llidTable_set 
 * Description:
 *      Set EPON llid table
 * Input:
 *      None 
 * Output:
 *      mode		
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_llidTable_set(uint32 llid_idx, apollo_raw_epon_llid_table_t *entry)
{
 	int32   ret;
	uint32  tmp_val,tmp_field_val;   

    
    if ((ret = reg_array_read(LLID_TABLEr,REG_ARRAY_INDEX_NONE,llid_idx,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    
    tmp_field_val = entry->llid;
    
    if ((ret = reg_field_set(LLID_TABLEr,LLIDf,&tmp_field_val,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	
    tmp_field_val = entry->valid;
    
    if ((ret = reg_field_set(LLID_TABLEr,VALIDf,&tmp_field_val,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  

    tmp_field_val = entry->report_timer;
    
    if ((ret = reg_field_set(LLID_TABLEr,REPORT_TIMERf,&tmp_field_val,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  

    if ((ret = reg_array_write(LLID_TABLEr,REG_ARRAY_INDEX_NONE,llid_idx,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }        
    return RT_ERR_OK;   
}





/* Function Name:
 *      apollo_raw_epon_llidTable_set 
 * Description:
 *      Set EPON llid table
 * Input:
 *      None 
 * Output:
 *      mode		
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_llidTable_get(uint32 llid_idx,apollo_raw_epon_llid_table_t *entry)
{
 	int32   ret;
	uint32  tmp_val,tmp_field_val;   

    
    if ((ret = reg_array_read(LLID_TABLEr,REG_ARRAY_INDEX_NONE,llid_idx,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    
    
    if ((ret = reg_field_get(LLID_TABLEr,LLIDf,&tmp_field_val,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    entry->llid = tmp_field_val;

	
    if ((ret = reg_field_get(LLID_TABLEr,REPORT_TIMERf,&tmp_field_val,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  
    entry->report_timer = tmp_field_val;

   if ((ret = reg_field_get(LLID_TABLEr,REPORT_TIMEOUTf,&tmp_field_val,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  
    entry->is_report_timeout = tmp_field_val;
   
    return RT_ERR_OK;   
}





/* Function Name:
 *      apollo_raw_epon_grant_list_get 
 * Description:
 *      get EPON grant list information
 * Input:
 *      None 
 * Output:
 *      mode		
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_grant_list_get(uint32 grant_idx, apollo_raw_epon_grant_t *entry)
{
 	int32   ret;
	uint32  tmp_val,tmp_field_val;   

    
    if ((ret = reg_array_read(EPON_GRANT_LIST0r,REG_ARRAY_INDEX_NONE,grant_idx,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    entry->grant_start = tmp_val;


    if ((ret = reg_array_read(EPON_GRANT_LIST1r,REG_ARRAY_INDEX_NONE,grant_idx,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    entry->grant_end = tmp_val;


    if ((ret = reg_array_read(EPON_GRANT_LIST2r,REG_ARRAY_INDEX_NONE,grant_idx,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    if ((ret = reg_field_get(EPON_GRANT_LIST2r,FORCE_REPORTf,&tmp_field_val,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  
    entry->is_force_report = tmp_field_val;   

    if ((ret = reg_field_get(EPON_GRANT_LIST2r,DISCf,&tmp_field_val,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  
    entry->is_disc = tmp_field_val;   
    
    if ((ret = reg_field_get(EPON_GRANT_LIST2r,LLID_IDXf,&tmp_field_val,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  
    entry->llid_idx = tmp_field_val; 
         
    return RT_ERR_OK;   
}




/* Function Name:
 *      apollo_raw_epon_dbgTxCtrl_get
 * Description:
 *      Get quard threshold
 * Input:
 *      None 
 * Output:
 *      quardTh		- quard threshold
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_epon_dbgTxCtrlLLidIdx_get(uint32 *idx)
{
 	int32   ret;
	uint32  tmp_val;   
	
    if ((ret = reg_field_read(EPON_TX_CTRLr,LLID_IDXf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }

    *idx = tmp_val;    
        
    return RT_ERR_OK;   
}
