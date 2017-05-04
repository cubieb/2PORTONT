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
 * Purpose : switch asic-level ACL API 
 * Feature : ACL related functions
 *
 */
 
#include <dal/apollo/raw/apollo_raw_acl.h>
#include <osal/lib.h>

/* Function Name:
 *      apollo_raw_acl_state_set
 * Description:
 *      Set port acl function enable/disable
 * Input:
 *      port 	- Physical port number (0~6)
 *      enabled - ENABLED, DISABLED
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED       
 *      RT_ERR_OK 		    
 *      RT_ERR_PORT_ID      
 *      RT_ERR_INPUT     
 * Note:
 *      None
 */
int32 apollo_raw_acl_state_set(rtk_port_t port, rtk_enable_t enabled)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
	RT_PARAM_CHK((RTK_ENABLE_END <= enabled), RT_ERR_INPUT);

    if ((ret = reg_array_field_write(ACL_ENr, port, REG_ARRAY_INDEX_NONE,ENf, &enabled)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	
	return RT_ERR_OK; 		
}


/* Function Name:
 *      apollo_raw_acl_state_get
 * Description:
 *      Get port acl function enable/disable
 * Input:
 *      port 	    - Physical port number (0~6)
 * Output:
 *      pEnabled    - ENABLED, DISABLED
 * Return:
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_OK 		    - Success
 *      RT_ERR_PORT_ID      - Invalid port number
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      None
 */
int32 apollo_raw_acl_state_get(rtk_port_t port, rtk_enable_t *pEnabled)
{
    int32 ret;

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(pEnabled == NULL,RT_ERR_NULL_POINTER);
	
    if ((ret = reg_array_field_read(ACL_ENr, port, REG_ARRAY_INDEX_NONE,ENf, pEnabled))!=RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
    
    return RT_ERR_OK;		
}



/* Function Name:
 *      apollo_raw_acl_permit_set
 * Description:
 *      Set port acl function unmatched permit action
 * Input:
 *      port 	- Physical port number (0~6)
 *      enabled - ENABLED, DISABLED
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED       
 *      RT_ERR_OK 		    
 *      RT_ERR_PORT_ID      
 *      RT_ERR_INPUT      
 * Note:
 *      None
 */
int32 apollo_raw_acl_permit_set(rtk_port_t port, rtk_enable_t enabled)
{
	int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
	RT_PARAM_CHK((RTK_ENABLE_END <= enabled), RT_ERR_INPUT);
	
	if ((ret = reg_array_field_write(ACL_PERMITr, port, REG_ARRAY_INDEX_NONE, PERMITf, &enabled)) != RT_ERR_OK)
	{
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
		return ret;
	}
    
	return RT_ERR_OK; 		
}
/* Function Name:
 *      apollo_raw_acl_permit_get
 * Description:
 * 	    Get port acl function unmatched permit action
 * Input:
 * 	    port	 - Physical port number (0~6)
 * Output:
 * 	    pEnabled - ENABLED, DISABLED
 * Return:
 *      RT_ERR_FAILED       - Failed
 * 	    RT_ERR_OK		    - Success
 * 	    RT_ERR_PORT_ID      - Invalid port number
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 * 	    None
 */
int32 apollo_raw_acl_permit_get(rtk_port_t port, rtk_enable_t *pEnabled)
{
    int32 ret;

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
	RT_PARAM_CHK(pEnabled == NULL,RT_ERR_NULL_POINTER);
    
	if ((ret = reg_array_field_read(ACL_PERMITr, port, REG_ARRAY_INDEX_NONE, PERMITf, pEnabled)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
		return ret;
	}
    
	return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_acl_actCtrl_set
 * Description:
 *      Set ACL rule matched Action Control Bits
 * Input:
 *      pAclActCtrl     - ACL action and rule control 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				
 *      RT_ERR_FAILED           
 *      RT_ERR_OUT_OF_RANGE  	
 * Note:
 *	    None
 */
int32 apollo_raw_acl_actCtrl_set(apollo_raw_acl_actCtrl_t *pAclActCtrl)
{
    int32 ret = RT_ERR_FAILED;
    
	RT_PARAM_CHK(pAclActCtrl == NULL,RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pAclActCtrl->idx > APOLLO_ACLACTIONMAX, RT_ERR_OUT_OF_RANGE);
	RT_PARAM_CHK(pAclActCtrl->valid >= RTK_ENABLE_END, RT_ERR_CHIP_NOT_SUPPORTED);
	RT_PARAM_CHK(pAclActCtrl->not >= RTK_ENABLE_END, RT_ERR_CHIP_NOT_SUPPORTED);
	RT_PARAM_CHK(pAclActCtrl->int_cf_en >= RTK_ENABLE_END, RT_ERR_CHIP_NOT_SUPPORTED);
	RT_PARAM_CHK(pAclActCtrl->cvlan_en >= RTK_ENABLE_END, RT_ERR_CHIP_NOT_SUPPORTED);
	RT_PARAM_CHK(pAclActCtrl->svlan_en >= RTK_ENABLE_END, RT_ERR_CHIP_NOT_SUPPORTED);
	RT_PARAM_CHK(pAclActCtrl->fwd_en >= RTK_ENABLE_END, RT_ERR_CHIP_NOT_SUPPORTED);
	RT_PARAM_CHK(pAclActCtrl->polic_en >= RTK_ENABLE_END, RT_ERR_CHIP_NOT_SUPPORTED);
	RT_PARAM_CHK(pAclActCtrl->pri_remark_en >= RTK_ENABLE_END, RT_ERR_CHIP_NOT_SUPPORTED);
   
    if ((ret = reg_array_field_write(ACL_ACTIONr, REG_ARRAY_INDEX_NONE, pAclActCtrl->idx, VALIDf, &pAclActCtrl->valid)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }

    if ((ret = reg_array_field_write(ACL_ACTIONr, REG_ARRAY_INDEX_NONE, pAclActCtrl->idx, NOTf, &pAclActCtrl->not)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }

    if ((ret = reg_array_field_write(ACL_ACTIONr, REG_ARRAY_INDEX_NONE, pAclActCtrl->idx, INT_CFf, &pAclActCtrl->int_cf_en)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }

    if ((ret = reg_array_field_write(ACL_ACTIONr, REG_ARRAY_INDEX_NONE, pAclActCtrl->idx, FWDf, &pAclActCtrl->fwd_en)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }

    if ((ret = reg_array_field_write(ACL_ACTIONr, REG_ARRAY_INDEX_NONE, pAclActCtrl->idx, POLICINGf, &pAclActCtrl->polic_en)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }

    if ((ret = reg_array_field_write(ACL_ACTIONr, REG_ARRAY_INDEX_NONE, pAclActCtrl->idx, PRIf, &pAclActCtrl->pri_remark_en)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }

    if ((ret = reg_array_field_write(ACL_ACTIONr, REG_ARRAY_INDEX_NONE, pAclActCtrl->idx, SVLANf, &pAclActCtrl->svlan_en)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }

    if ((ret = reg_array_field_write(ACL_ACTIONr, REG_ARRAY_INDEX_NONE, pAclActCtrl->idx, CVLANf, &pAclActCtrl->cvlan_en)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      apollo_raw_acl_actCtrl_get
 * Description:
 *      Set ACL rule matched Action Control Bits
 * Input:
 *      None
 * Output:
 *      pAclActCtrl     - ACL action and rule control 
 * Return:
 *      RT_ERR_OK 				
 *      RT_ERR_FAILED           
 *      RT_ERR_OUT_OF_RANGE  	
 * Note:
 *	    None
 */
int32 apollo_raw_acl_actCtrl_get(apollo_raw_acl_actCtrl_t *pAclActCtrl)
{
    int32 ret = RT_ERR_FAILED;
    
	RT_PARAM_CHK(pAclActCtrl == NULL,RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pAclActCtrl->idx > APOLLO_ACLACTIONMAX, RT_ERR_OUT_OF_RANGE);

    if ((ret = reg_array_field_read(ACL_ACTIONr, REG_ARRAY_INDEX_NONE, pAclActCtrl->idx, VALIDf, &pAclActCtrl->valid)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }

    if ((ret = reg_array_field_read(ACL_ACTIONr, REG_ARRAY_INDEX_NONE, pAclActCtrl->idx, NOTf, &pAclActCtrl->not)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }

    if ((ret = reg_array_field_read(ACL_ACTIONr, REG_ARRAY_INDEX_NONE, pAclActCtrl->idx, INT_CFf, &pAclActCtrl->int_cf_en)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }

    if ((ret = reg_array_field_read(ACL_ACTIONr, REG_ARRAY_INDEX_NONE, pAclActCtrl->idx, FWDf, &pAclActCtrl->fwd_en)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }

    if ((ret = reg_array_field_read(ACL_ACTIONr, REG_ARRAY_INDEX_NONE, pAclActCtrl->idx, POLICINGf, &pAclActCtrl->polic_en)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }

    if ((ret = reg_array_field_read(ACL_ACTIONr, REG_ARRAY_INDEX_NONE, pAclActCtrl->idx, PRIf, &pAclActCtrl->pri_remark_en)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }

    if ((ret = reg_array_field_read(ACL_ACTIONr, REG_ARRAY_INDEX_NONE, pAclActCtrl->idx, SVLANf, &pAclActCtrl->svlan_en)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }

    if ((ret = reg_array_field_read(ACL_ACTIONr, REG_ARRAY_INDEX_NONE, pAclActCtrl->idx, CVLANf, &pAclActCtrl->cvlan_en)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_acl_act_set
 * Description:
 *      Set ACL rule matched Action
 * Input:
 *      pAclAct - ACL action stucture for setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_OK 				- Success
 *      RT_ERR_OUT_OF_RANGE  	- Invalid ACL rule index (0-63)
 *     RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *	    None
 */
 
int32 apollo_raw_acl_act_set(apollo_raw_acl_act_t *pAclAct)
{
	int32 ret = RT_ERR_FAILED;
	uint32 fieldData[APOLLO_ACLACTTBLEN];

	RT_PARAM_CHK(pAclAct->idx > APOLLO_ACLACTIONMAX, RT_ERR_OUT_OF_RANGE);
	RT_PARAM_CHK(pAclAct == NULL, RT_ERR_NULL_POINTER);
    
	osal_memset(fieldData, 0, sizeof(fieldData));
    if ((ret = table_field_set(ACL_ACTIONt, ACL_ACTION_CFIDXtf, &pAclAct->cfidx, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_set(ACL_ACTIONt, ACL_ACTION_CFACTtf, &pAclAct->cfact, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_set(ACL_ACTIONt, ACL_ACTION_ACLINTtf, &pAclAct->aclint, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_set(ACL_ACTIONt, ACL_ACTION_PRIDXtf, &pAclAct->pridx, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_set(ACL_ACTIONt, ACL_ACTION_PRIACTtf, &pAclAct->priact,fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_set(ACL_ACTIONt, ACL_ACTION_FWD_PMSKtf, &pAclAct->fwdpmask, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_set(ACL_ACTIONt, ACL_ACTION_FWDACTtf, &pAclAct->fwdact, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_set(ACL_ACTIONt, ACL_ACTION_METER_IDXtf, &pAclAct->meteridx, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_set(ACL_ACTIONt, ACL_ACTION_POLICACTtf, &pAclAct->policact, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_set(ACL_ACTIONt, ACL_ACTION_SVIDXtf, &pAclAct->svidx_sact, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_set(ACL_ACTIONt, ACL_ACTION_SACTtf, &pAclAct->sact, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_set(ACL_ACTIONt, ACL_ACTION_CVIDXtf, &pAclAct->cvidx_cact, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_set(ACL_ACTIONt, ACL_ACTION_CACTtf, &pAclAct->cact, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
    if ((ret = table_write(ACL_ACTIONt, pAclAct->idx, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
    
	return RT_ERR_OK;
}
/* Function Name:
 *      apollo_acl_act_get
 * Description:
 *      Get ACL rule matched Action
 * Input:
 *	    None
 * Output:
 *      pAclAct 	- ACL action stucture for setting
 * Return:
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_OK 				- Success
 *      RT_ERR_OUT_OF_RANGE  	- Invalid ACL rule index (0-63)
 *      RT_ERR_NULL_POINTER     - input parameter is null pointer
  * Note:
 *	    None
 */
int32 apollo_raw_acl_act_get(apollo_raw_acl_act_t *pAclAct)
{
	int32 ret = RT_ERR_FAILED;
	uint32 fieldData[APOLLO_ACLACTTBLEN];

	RT_PARAM_CHK(pAclAct->idx > APOLLO_ACLACTIONNO, RT_ERR_OUT_OF_RANGE);
	RT_PARAM_CHK(NULL == pAclAct,RT_ERR_NULL_POINTER);	
	osal_memset(fieldData,0,sizeof(fieldData));
	if ((ret = table_read(ACL_ACTIONt, pAclAct->idx, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
    if ((ret = table_field_get(ACL_ACTIONt, ACL_ACTION_CFIDXtf, &pAclAct->cfidx, fieldData)) != RT_ERR_OK)
		{
			RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
			return ret;
		}
	if ((ret = table_field_get(ACL_ACTIONt, ACL_ACTION_CFACTtf, &pAclAct->cfact, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_get(ACL_ACTIONt, ACL_ACTION_ACLINTtf, &pAclAct->aclint, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_get(ACL_ACTIONt, ACL_ACTION_PRIDXtf, &pAclAct->pridx, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_get(ACL_ACTIONt, ACL_ACTION_PRIACTtf, &pAclAct->priact, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_get(ACL_ACTIONt, ACL_ACTION_FWD_PMSKtf, &pAclAct->fwdpmask, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_get(ACL_ACTIONt, ACL_ACTION_FWDACTtf, &pAclAct->fwdact, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_get(ACL_ACTIONt, ACL_ACTION_METER_IDXtf, &pAclAct->meteridx, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_get(ACL_ACTIONt, ACL_ACTION_POLICACTtf, &pAclAct->policact, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_get(ACL_ACTIONt, ACL_ACTION_SVIDXtf, &pAclAct->svidx_sact, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_get(ACL_ACTIONt, ACL_ACTION_SACTtf, &pAclAct->sact, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_get(ACL_ACTIONt, ACL_ACTION_CVIDXtf, &pAclAct->cvidx_cact, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = table_field_get(ACL_ACTIONt, ACL_ACTION_CACTtf, &pAclAct->cact, fieldData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	
	
	return RT_ERR_OK;
}

/* Function Name:
 *      apollo_acl_rule_set
 * Description:
 *      Set acl rule content
 * Input:
 *      pAclRule - ACL rule stucture for setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_OK 				- Success
 *      RT_ERR_OUT_OF_RANGE  	- Invalid ACL rule index (0-63)
 *      RT_ERR_NULL_POINTER     - input parameter is null pointer
 * Note:
 *		System supported 64 shared 289-bit ACL ingress rule. Index was available at range 0-63 only. 
 *		If software want to modify ACL rule, the ACL function should be disable at first or unspecify 
 *		acl action will be executed. 
 *		One ACL rule structure has three parts setting:
 *		Bit 0-143		Data Bits of this Rule
 *		Bit	144		Valid Bit
 *		Bit 145-288	Care Bits of this Rule
 *		There are four kinds of field in Data Bits and Care Bits: Active Portmask, Type, Tag Exist, and 8 fields
 */
 
int32 apollo_raw_acl_rule_set(apollo_raw_acl_ruleEntry_t *pAclRule)
{
	int32 ret = RT_ERR_FAILED;
	uint32 i;
    uint32 entryIdx;
    uint32 aclFieldNum = 4;
    uint32 modeField0 = ACL_DATA2_FIELD0_0tf;
    uint32 modeMask0 = ACL_MASK2_FIELD0_0tf;
	uint32 fieldData[APOLLO_ACLRULETBLEN];
    uint32 fieldMask[APOLLO_ACLRULETBLEN];
    uint32 fieldData2[APOLLO_ACLRULETBLEN];
    uint32 fieldMask2[APOLLO_ACLRULETBLEN];
	uint32 tmpData;
    
	RT_PARAM_CHK(NULL == pAclRule , RT_ERR_NULL_POINTER);	
    RT_PARAM_CHK(1 < pAclRule->valid, RT_ERR_OUT_OF_RANGE);

    if (APOLLO_ACL_MODE_64ENTRIES == pAclRule->mode)
    {
    
        RT_PARAM_CHK(APOLLO_ACLRULEMAX < pAclRule->idx, RT_ERR_OUT_OF_RANGE);   
        osal_memset(fieldData, 0, sizeof(fieldData));
        for (i=0; i<APOLLO_ACLRULEFIELDNO; i++)
        {   
            if ((ret = table_field_set(ACL_DATAt, ACL_DATA_FIELD0tf-i, &pAclRule->data_bits.field[i], fieldData)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
        }
        if ((ret = table_field_set(ACL_DATAt,ACL_DATA_PMSKtf, &(pAclRule->data_bits.active_portmsk), fieldData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
            return ret;
        }
        if ((ret = table_field_set(ACL_DATAt, ACL_DATA_TYPEtf, &(pAclRule->data_bits.type),fieldData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
            return ret;
        }
        if ((ret = table_field_set(ACL_DATAt, ACL_DATA_TAGStf, &(pAclRule->data_bits.tag_exist),fieldData)) != RT_ERR_OK)
        { 
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
            return ret;
        }
        osal_memset(fieldMask,0,sizeof(fieldMask));
        for (i=0; i<APOLLO_ACLRULEFIELDNO; i++)
        {
            if ((ret = table_field_set(ACL_MASKt, ACL_MASK_FIELD0tf-i, &pAclRule->care_bits.field[i], fieldMask)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }       
        
        }
        if ((ret = table_field_set(ACL_MASKt, ACL_MASK_PMSKtf, &(pAclRule->care_bits.active_portmsk), fieldMask)) != RT_ERR_OK)
        {
             RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
             return ret;
        }
        if ((ret = table_field_set(ACL_MASKt, ACL_MASK_TYPEtf, &(pAclRule->care_bits.type),fieldMask)) != RT_ERR_OK)
        {
             RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
             return ret;
        }
        if ((ret = table_field_set(ACL_MASKt, ACL_MASK_TAGStf, &(pAclRule->care_bits.tag_exist), fieldMask)) != RT_ERR_OK)
        {
             RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
             return ret;
        }
        for (i = 0; i < APOLLO_ACLRULETBLEN; i++)
        {
            fieldData2[i] = fieldMask[i]&fieldData[i];
            fieldMask2[i] = fieldMask[i]&~fieldData[i];
        }

        if ((table_field_set(ACL_DATAt, ACL_DATA_VALIDtf, &pAclRule->valid, fieldData2)) != RT_ERR_OK)
        {   
             RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
             return ret;
        }
        if ((ret = table_write(ACL_DATAt, pAclRule->idx, fieldData2)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
            return ret;
        }
        if ((ret = table_write(ACL_MASKt, pAclRule->idx, fieldMask2)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
            return ret;
        }

    }
    else if (APOLLO_ACL_MODE_128ENTRIES == pAclRule->mode)
    {
    
	    RT_PARAM_CHK(APOLLO_ACLACTIONMAX < pAclRule->idx, RT_ERR_OUT_OF_RANGE);	
        osal_memset(fieldData, 0, sizeof(fieldData));
        entryIdx = pAclRule->idx&APOLLO_ACLRULEMASK;
        if ((ret = table_read(ACL_DATAt, entryIdx, fieldData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
            return ret;
        }
        osal_memset(fieldMask, 0, sizeof(fieldMask));
        if ((ret = table_read(ACL_MASKt, entryIdx, fieldMask)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
             return ret;
        }
        if (pAclRule->idx >= APOLLO_ACLRULENO)
        {
            aclFieldNum = 3;
            modeField0 = ACL_DATA2_FIELD0_1tf;
            tmpData = pAclRule->data_bits.active_portmsk & pAclRule->care_bits.active_portmsk;
            if ((ret = table_field_set(ACL_DATA2t, ACL_DATA2_PMSK_1tf, &tmpData, fieldData)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
            tmpData = pAclRule->data_bits.type & pAclRule->care_bits.type;
            if ((ret = table_field_set(ACL_DATA2t, ACL_DATA2_TYPE_1tf, &tmpData, fieldData)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
            tmpData = pAclRule->data_bits.tag_exist & pAclRule->care_bits.tag_exist;
            if ((ret = table_field_set(ACL_DATA2t, ACL_DATA2_TAGS_1tf, &tmpData, fieldData)) != RT_ERR_OK)
            { 
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
   
            modeMask0 = ACL_MASK2_FIELD0_1tf;
            tmpData = pAclRule->care_bits.active_portmsk&(~pAclRule->data_bits.active_portmsk);
            if ((ret = table_field_set(ACL_MASK2t, ACL_MASK2_PMSK_1tf, &tmpData, fieldMask)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
            tmpData = pAclRule->care_bits.type&(~pAclRule->data_bits.type);
            if ((ret = table_field_set(ACL_MASK2t, ACL_MASK2_TYPE_1tf, &tmpData, fieldMask)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
            tmpData = pAclRule->care_bits.tag_exist&(~pAclRule->data_bits.tag_exist);
            if ((ret = table_field_set(ACL_MASK2t, ACL_MASK2_TAGS_1tf, &tmpData, fieldMask)) != RT_ERR_OK)
            { 
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }

            
        }
        else {
            tmpData = pAclRule->data_bits.active_portmsk & pAclRule->care_bits.active_portmsk;
            if ((ret = table_field_set(ACL_DATA2t, ACL_DATA2_PMSK_0tf, &tmpData, fieldData)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
            tmpData = pAclRule->data_bits.type & pAclRule->care_bits.type;
            if ((ret = table_field_set(ACL_DATA2t, ACL_DATA2_TYPE_0tf, &tmpData, fieldData)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
            tmpData = pAclRule->data_bits.tag_exist & pAclRule->care_bits.tag_exist;
            if ((ret = table_field_set(ACL_DATA2t, ACL_DATA2_TAGS_0tf, &tmpData, fieldData)) != RT_ERR_OK)
            { 
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }

            tmpData = pAclRule->care_bits.active_portmsk&(~pAclRule->data_bits.active_portmsk);
            if ((ret = table_field_set(ACL_MASK2t, ACL_MASK2_PMSK_0tf, &tmpData, fieldMask)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
            tmpData = pAclRule->care_bits.type&(~pAclRule->data_bits.type);
            if ((ret = table_field_set(ACL_MASK2t, ACL_MASK2_TYPE_0tf, &tmpData, fieldMask)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
            tmpData = pAclRule->care_bits.tag_exist&(~pAclRule->data_bits.tag_exist);
            if ((ret = table_field_set(ACL_MASK2t, ACL_MASK2_TAGS_0tf, &tmpData, fieldMask)) != RT_ERR_OK)
            { 
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
        }             
	    for (i = 0; i < aclFieldNum; i++)
        {   
            tmpData = pAclRule->care_bits.field[i]&pAclRule->data_bits.field[i];
            if ((ret = table_field_set(ACL_DATA2t, modeField0-i, &pAclRule->data_bits.field[i], fieldData)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
            tmpData = pAclRule->care_bits.field[i]&(~pAclRule->data_bits.field[i]);
            if ((ret = table_field_set(ACL_MASK2t, modeMask0-i, &tmpData, fieldMask)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
        }
        if ((table_field_set(ACL_DATAt, ACL_DATA_VALIDtf, &pAclRule->valid, fieldData)) != RT_ERR_OK)
       	{   
             RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
             return ret;
        }	
  
        if ((ret = table_write(ACL_DATAt, entryIdx, fieldData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
            return ret;
        }

        if ((ret = table_write(ACL_MASKt, entryIdx, fieldMask)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
            return ret;
        }     
    }
    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_acl_rule_get
 * Description:
 *      Get acl rule content
 * Input:
 *		None
 * Output:
 *      pAclRule - ACL rule stucture 
 * Return:
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_OK 				- Success
 *      RT_ERR_OUT_OF_RANGE  	- Invalid ACL rule index (0-63)
 *      RT_ERR_NULL_POINTER     - input parameter is null pointer
 * Note:
 *		None
 */
int32 apollo_raw_acl_rule_get(apollo_raw_acl_ruleEntry_t *pAclRule)
{
	int32 ret = RT_ERR_FAILED;
	uint32 i;
    uint32 entryIdx;
    uint32 aclFieldNum = 4;
    uint32 modeField0 = ACL_DATA2_FIELD0_0tf;
    uint32 modeMask0 = ACL_MASK2_FIELD0_0tf;
    uint32 fieldData[APOLLO_ACLRULETBLEN];
    uint32 fieldMask[APOLLO_ACLRULETBLEN];

	RT_PARAM_CHK(NULL == pAclRule, RT_ERR_NULL_POINTER);

    if (APOLLO_ACL_MODE_64ENTRIES == pAclRule->mode)
    {
	    RT_PARAM_CHK(pAclRule->idx > APOLLO_ACLRULEMAX, RT_ERR_OUT_OF_RANGE);	
	
	    osal_memset(fieldData, 0x00, sizeof(fieldData));
        if ((ret = table_read(ACL_DATAt, pAclRule->idx, fieldData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
            return ret;
        }
    
	    for (i=0; i<APOLLO_ACLRULEFIELDNO ;i++)
    	{
            if((ret = table_field_get(ACL_DATAt, ACL_DATA_FIELD0tf-i, &pAclRule->data_bits.field[i], fieldData)) != RT_ERR_OK)
      	    {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
           	}
	    }
    	if ((ret = table_field_get(ACL_DATAt, ACL_DATA_PMSKtf, &(pAclRule->data_bits.active_portmsk), fieldData)) != RT_ERR_OK)
    	{
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
            return ret;
    	}
        
        if ((ret = table_field_get(ACL_DATAt, ACL_DATA_TYPEtf, &(pAclRule->data_bits.type), fieldData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
            return ret;
        }
    	if ((ret = table_field_get(ACL_DATAt, ACL_DATA_TAGStf, &(pAclRule->data_bits.tag_exist), fieldData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
            return ret;
        }

        /*retrive valit bit from acl data bits*/
        if ((ret = table_field_get(ACL_DATAt, ACL_DATA_VALIDtf, &pAclRule->valid, fieldData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
            return ret;
        }
        
	    osal_memset(fieldMask, 0x00, sizeof(fieldMask));
        if ((ret = table_read(ACL_MASKt, pAclRule->idx, fieldMask)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
            return ret;
        }
        for (i = 0; i < APOLLO_ACLRULETBLEN; i++)
            fieldMask[i] = fieldMask[i]^fieldData[i];

      	for (i = 0; i < APOLLO_ACLRULEFIELDNO; i++)
        { 
            if ((ret = table_field_get(ACL_MASKt, ACL_MASK_FIELD0tf-i, &pAclRule->care_bits.field[i], fieldMask)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
    
            }
        }
        if ((ret = table_field_get(ACL_MASKt, ACL_MASK_PMSKtf, &(pAclRule->care_bits.active_portmsk), fieldMask)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
            return ret;    
        }
        if ((ret = table_field_get(ACL_MASKt, ACL_MASK_TYPEtf, &(pAclRule->care_bits.type), fieldMask)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
            return ret;
        }
    	if ((ret = table_field_get(ACL_MASKt, ACL_MASK_TAGStf, &(pAclRule->care_bits.tag_exist), fieldMask)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
            return ret;
        }

    }
    else if (APOLLO_ACL_MODE_128ENTRIES == pAclRule->mode)
    {
        
        RT_PARAM_CHK(pAclRule->idx > APOLLO_ACLACTIONMAX, RT_ERR_OUT_OF_RANGE);	
        
        osal_memset(fieldData, 0, sizeof(fieldData));
        entryIdx = pAclRule->idx&APOLLO_ACLRULEMASK;
        if ((ret = table_read(ACL_DATAt, entryIdx, fieldData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
            return ret;
        }
        osal_memset(fieldMask, 0, sizeof(fieldMask));
        if ((ret = table_read(ACL_MASKt, entryIdx, fieldMask)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
             return ret;
        }

        for (i = 0; i < APOLLO_ACLRULETBLEN; i++)
            fieldMask[i] =  fieldMask[i]^fieldData[i];
        if (pAclRule->idx >= APOLLO_ACLRULENO)
        {
        
            aclFieldNum = 3;
            modeField0 = ACL_DATA2_FIELD0_1tf;
            if ((ret = table_field_get(ACL_DATA2t, ACL_DATA2_PMSK_1tf, &(pAclRule->data_bits.active_portmsk), fieldData)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
            if ((ret = table_field_get(ACL_DATA2t, ACL_DATA2_TYPE_1tf, &(pAclRule->data_bits.type), fieldData)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
            if ((ret = table_field_get(ACL_DATA2t, ACL_DATA2_TAGS_1tf, &(pAclRule->data_bits.tag_exist), fieldData)) != RT_ERR_OK)
            { 
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
            modeMask0 = ACL_MASK2_FIELD0_1tf;
            if ((ret = table_field_get(ACL_MASK2t, ACL_MASK2_PMSK_1tf, &(pAclRule->care_bits.active_portmsk), fieldMask)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
            if ((ret = table_field_get(ACL_MASK2t, ACL_MASK2_TYPE_1tf, &(pAclRule->care_bits.type), fieldMask)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
            if ((ret = table_field_get(ACL_MASK2t, ACL_MASK2_TAGS_1tf, &(pAclRule->care_bits.tag_exist), fieldMask)) != RT_ERR_OK)
            { 
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }            
            
        }
        else {
            if ((ret = table_field_get(ACL_DATA2t, ACL_DATA2_PMSK_0tf, &(pAclRule->data_bits.active_portmsk), fieldData)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
            if ((ret = table_field_get(ACL_DATA2t, ACL_DATA2_TYPE_0tf, &(pAclRule->data_bits.type), fieldData)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
            if ((ret = table_field_get(ACL_DATA2t, ACL_DATA2_TAGS_0tf, &(pAclRule->data_bits.tag_exist), fieldData)) != RT_ERR_OK)
            { 
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
            
            if ((ret = table_field_get(ACL_MASK2t, ACL_MASK2_PMSK_0tf, &(pAclRule->care_bits.active_portmsk), fieldMask)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
            if ((ret = table_field_get(ACL_MASK2t, ACL_MASK2_TYPE_0tf, &(pAclRule->care_bits.type), fieldMask)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
            if ((ret = table_field_get(ACL_MASK2t, ACL_MASK2_TAGS_0tf, &(pAclRule->care_bits.tag_exist), fieldMask)) != RT_ERR_OK)
            { 
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
         }             
        
	    for (i = 0; i < aclFieldNum; i++)
        {   
            if ((ret = table_field_get(ACL_DATA2t, modeField0-i, &pAclRule->data_bits.field[i], fieldData)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
            if ((ret = table_field_get(ACL_MASK2t, modeMask0-i, &pAclRule->care_bits.field[i], fieldMask)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
                return ret;
            }
        }
        if ((ret = table_field_get(ACL_DATA2t, ACL_DATA2_VALIDtf, &pAclRule->valid, fieldData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
            return ret;
        }
        

    }
	return RT_ERR_OK;
}


/* Function Name:
 *     apollo_acl_template_set
 * Description:
 *      Set fields of a ACL Template
 * Input:
 *      pAclType - ACL type stucture for setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_OK 				- Success
 *      RT_ERR_OUT_OF_RANGE  	- Invalid ACL template index 
 *      RT_ERR_NULL_POINTER     - input parameter is null pointer
 *
 * Note:
 *	    The API can set type field of the 5 ACL rule templates.
 *		Each type has 8 fields. One field means what data in one field of a ACL rule means
 *		8 fields of ACL rule 0~63 is descripted by one type in ACL group
 */
int32 apollo_raw_acl_template_set(apollo_raw_acl_template_t *pAclType)
{	
	int ret;
	int idx2;
    uint32 tmpData;

    RT_PARAM_CHK(pAclType == NULL, RT_ERR_NULL_POINTER);	
	RT_PARAM_CHK(pAclType->idx > APOLLO_ACLTEMPLATEMAX, RT_ERR_OUT_OF_RANGE);
   
	for (idx2 = 0; idx2< APOLLO_ACLRULEFIELDNO; idx2++)
    {    
        tmpData = pAclType->field[idx2];

	    if ((ret = reg_array_field_write(ACL_TEMPLATE_CTRLr, pAclType->idx, idx2,FIELDf, &tmpData)) != RT_ERR_OK )
        {
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
            return ret;
        }
    }
	return RT_ERR_OK;
}
/* Function Name:
 *      apollo_acl_template_get
 * Description:
 *      Get fields of a ACL Template
 * Input:
 *	    None
 * Output:
 *      pAclType - ACL type stucture for setting
 * Return:
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_OK               - Success
 *      RT_ERR_OUT_OF_RANGE     - Invalid ACL template index
 *      RT_ERR_NULL_POINTER     - input parameter is null pointer
 * Note:
 *	    None
 */
int32 apollo_raw_acl_template_get(apollo_raw_acl_template_t *pAclType)
{
	int32 ret;
	uint32 idx2;
    uint32 tmpData;
    
    RT_PARAM_CHK(pAclType == NULL, RT_ERR_NULL_POINTER);	
	RT_PARAM_CHK(pAclType->idx > APOLLO_ACLTEMPLATEMAX, RT_ERR_OUT_OF_RANGE);
    
    for (idx2 = 0; idx2 < APOLLO_ACLRULEFIELDNO; idx2++)
    {
        if ((ret = reg_array_field_read(ACL_TEMPLATE_CTRLr, pAclType->idx, idx2, FIELDf, &tmpData)) != RT_ERR_OK )
        {
            RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
            return ret;
        }
        
        pAclType->field[idx2] = tmpData;
    }
    
 	return RT_ERR_OK;
}

/* Function Name:
 *      apollo_acl_portRange_set
 * Description:
 *      Set ACL TCP/UDP range check
 * Input:
 *      pPortRange  - port range configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_OK 				- Success
 *      RT_ERR_OUT_OF_RANGE  	- Invalid index (0-15)
  *     RT_ERR_NULL_POINTER     - input parameter is null pointer
 * Note:
 *	    None
 */
int32 apollo_raw_acl_portRange_set(apollo_raw_acl_portRange_t *pPortRange)
{
	int ret;

    ret = RT_ERR_FAILED;
    RT_PARAM_CHK(NULL == pPortRange, RT_ERR_NULL_POINTER);
	RT_PARAM_CHK(pPortRange->idx > APOLLO_ACLRANGEMAX1, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pPortRange->type >= APOLLO_ACL_RNGL4PORTTYPE_END, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pPortRange->upperPort > APOLLO_L4PORT_MAX, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pPortRange->lowerPort > APOLLO_L4PORT_MAX, RT_ERR_OUT_OF_RANGE);
   
	if ((ret = reg_array_field_write(RNG_CHK_L4PORT_RNGr, REG_ARRAY_INDEX_NONE, pPortRange->idx, TYPEf, &pPortRange->type)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = reg_array_field_write(RNG_CHK_L4PORT_RNGr, REG_ARRAY_INDEX_NONE, pPortRange->idx, L4PORT_UPPERf, &pPortRange->upperPort)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = reg_array_field_write(RNG_CHK_L4PORT_RNGr, REG_ARRAY_INDEX_NONE, pPortRange->idx, L4PORT_LOWERf, &pPortRange->lowerPort)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	return RT_ERR_OK;
}
/* Function Name:
 *      apollo_acl_portRange_get
 * Description:
 *      Get ACL TCP/UDP range check
 * Input:
 *	    None
 * Output:
 *      pPortRange  - port range configuration
 * Return:
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_OK               - Success
 *      RT_ERR_OUT_OF_RANGE     - Invalid index (0-15)
 *      RT_ERR_NULL_POINTER     - input parameter is null pointer
 * Note:
 *	    None
 */
int32 apollo_raw_acl_portRange_get(apollo_raw_acl_portRange_t *pPortRange)
{
	int ret;

    ret = RT_ERR_FAILED;
    RT_PARAM_CHK(NULL == pPortRange, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pPortRange->idx > APOLLO_ACLRANGEMAX1, RT_ERR_OUT_OF_RANGE);
    
	if ((ret = reg_array_field_read(RNG_CHK_L4PORT_RNGr, REG_ARRAY_INDEX_NONE, pPortRange->idx, TYPEf, &pPortRange->type)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = reg_array_field_read(RNG_CHK_L4PORT_RNGr, REG_ARRAY_INDEX_NONE, pPortRange->idx, L4PORT_UPPERf,  &pPortRange->upperPort)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if((ret = reg_array_field_read(RNG_CHK_L4PORT_RNGr, REG_ARRAY_INDEX_NONE, pPortRange->idx, L4PORT_LOWERf, &pPortRange->lowerPort)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	return RT_ERR_OK;

}
/* Function Name:
 *      apollo_acl_vidRange_set
 * Description:
 *      Set ACL VID range check
 * Input:
 *      pVidRange  - vid range configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_OK 				- Success
 *      RT_ERR_OUT_OF_RANGE  	- Invalid index (0-7)
 * Note:
 *	    None
 */
int32 apollo_raw_acl_vidRange_set(apollo_raw_acl_vidRange_t *pVidRange)
{
	int ret;
    ret = RT_ERR_FAILED;
    RT_PARAM_CHK(NULL == pVidRange, RT_ERR_NULL_POINTER);
	RT_PARAM_CHK(pVidRange->idx > APOLLO_ACLRANGEMAX2, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pVidRange->type >= APOLLO_ACL_RNGVIDTYPE_END, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pVidRange->upperVid > APOLLO_VIDMAX, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pVidRange->lowerVid > APOLLO_VIDMAX, RT_ERR_OUT_OF_RANGE);

    if ((ret = reg_array_field_write(RNG_CHK_VID_RNGr, REG_ARRAY_INDEX_NONE, pVidRange->idx, TYPEf, &pVidRange->type)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = reg_array_field_write(RNG_CHK_VID_RNGr, REG_ARRAY_INDEX_NONE, pVidRange->idx, VID_UPPERf, &pVidRange->upperVid)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = reg_array_field_write(RNG_CHK_VID_RNGr, REG_ARRAY_INDEX_NONE, pVidRange->idx, VID_LOWERf, &pVidRange->lowerVid)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	return RT_ERR_OK;

}
/* Function Name:
 *      apollo_acl_vidRange_get
 * Description:
 *      Get ACL VID range check
 * Input:
 *	    None
 * Output:
 *      pVidRange  - vid range configuration
 * Return:
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_OK 				- Success
 *      RT_ERR_OUT_OF_RANGE  	- Invalid index (0-7)
 *      RT_ERR_NULL_POINTER     - input parameter is null pointer
 * Note:
 *	    None
 */
int32 apollo_raw_acl_vidRange_get(apollo_raw_acl_vidRange_t *pVidRange)
{
	int ret;
    ret = RT_ERR_FAILED;
    RT_PARAM_CHK(NULL == pVidRange, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pVidRange->idx > APOLLO_ACLRANGEMAX2, RT_ERR_OUT_OF_RANGE);
    
	if ((ret = reg_array_field_read(RNG_CHK_VID_RNGr, REG_ARRAY_INDEX_NONE, pVidRange->idx, TYPEf, &pVidRange->type)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = reg_array_field_read(RNG_CHK_VID_RNGr, REG_ARRAY_INDEX_NONE, pVidRange->idx, VID_UPPERf, &pVidRange->upperVid)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if((ret = reg_array_field_read(RNG_CHK_VID_RNGr, REG_ARRAY_INDEX_NONE, pVidRange->idx, VID_LOWERf, &pVidRange->lowerVid)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	return RT_ERR_OK;

}

/* Function Name:
 *      apollo_acl_ipRange_set
 * Description:
 *      Set ACL IP range check
 * Input:
 *      pIpRange  - ip range configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_OK 				- Success
 *      RT_ERR_OUT_OF_RANGE  	- Invalid index (0-7)
 * Note:
 *	    None
 */
int32 apollo_raw_acl_ipRange_set(apollo_raw_acl_ipRange_t *pIpRange)
{
	int ret;
    ret = RT_ERR_FAILED;
    RT_PARAM_CHK(NULL == pIpRange, RT_ERR_NULL_POINTER);
	RT_PARAM_CHK(pIpRange->idx > APOLLO_ACLRANGEMAX2, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pIpRange->type >= APOLLO_ACL_RNGIPTYPE_END, RT_ERR_OUT_OF_RANGE);
    
	if ((ret = reg_array_field_write(RNG_CHK_IP_RNGr, REG_ARRAY_INDEX_NONE, pIpRange->idx, TYPEf, &pIpRange->type)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = reg_array_field_write(RNG_CHK_IP_RNGr, REG_ARRAY_INDEX_NONE, pIpRange->idx, IP_UPPERf, &pIpRange->upperIp)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if((ret = reg_array_field_write(RNG_CHK_IP_RNGr, REG_ARRAY_INDEX_NONE, pIpRange->idx, IP_LOWERf, &pIpRange->lowerIp)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	return RT_ERR_OK;
}
/* Function Name:
 *      apollo_acl_ipRange_get
 * Description:
 *      Get ACL IP range check
 * Input:
 *	    None
 * Output:
 *      pIpRange  - ip range configuration
 * Return:
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_OK 				- Success
 *      RT_ERR_OUT_OF_RANGE  	- Invalid index (0-7)
 *      RT_ERR_NULL_POINTER     - input parameter is null pointer
 * Note:
 *	    None
 */
int32 apollo_raw_acl_ipRange_get(apollo_raw_acl_ipRange_t *pIpRange)
{
	int ret;
    ret = RT_ERR_FAILED;
    RT_PARAM_CHK(NULL == pIpRange, RT_ERR_NULL_POINTER);
	RT_PARAM_CHK(pIpRange->idx > APOLLO_ACLRANGEMAX2, RT_ERR_OUT_OF_RANGE);
    
	if ((ret = reg_array_field_read(RNG_CHK_IP_RNGr, REG_ARRAY_INDEX_NONE, pIpRange->idx, TYPEf, &pIpRange->type)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = reg_array_field_read(RNG_CHK_IP_RNGr, REG_ARRAY_INDEX_NONE, pIpRange->idx, IP_UPPERf, &pIpRange->upperIp)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = reg_array_field_read(RNG_CHK_IP_RNGr, REG_ARRAY_INDEX_NONE, pIpRange->idx, IP_LOWERf, &pIpRange->lowerIp)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	return RT_ERR_OK;

}

/* Function Name:
 *      apollo_acl_pktLenRange_set
 * Description:
 *      Set ACL packets length range check
 * Input:
 *      pPktLenRange  - packet length range configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_OK 				- Success
 *      RT_ERR_OUT_OF_RANGE  	- Invalid index (0-7)
 * Note:
 *	    None
 */
int32 apollo_raw_acl_pktLenRange_set(apollo_raw_acl_pktLenRange_t *pPktLenRange)
{
	int ret;

    RT_PARAM_CHK(NULL == pPktLenRange, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pPktLenRange->idx > APOLLO_ACLRANGEMAX2, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pPktLenRange->type >= APOLLO_ACL_RNGPKTLENTYPE_END, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pPktLenRange->upperPktLen > APOLLO_RAW_ACL_PKTLEN_MAX, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pPktLenRange->lowerPktLen > APOLLO_RAW_ACL_PKTLEN_MAX, RT_ERR_OUT_OF_RANGE);
	
	if ((ret = reg_array_field_write(RNG_CHK_PKTLEN_RNGr, REG_ARRAY_INDEX_NONE, pPktLenRange->idx, TYPEf, &pPktLenRange->type)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = reg_array_field_write(RNG_CHK_PKTLEN_RNGr, REG_ARRAY_INDEX_NONE, pPktLenRange->idx, PKTLEN_UPPERf, &pPktLenRange->upperPktLen)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = reg_array_field_write(RNG_CHK_PKTLEN_RNGr, REG_ARRAY_INDEX_NONE, pPktLenRange->idx, PKTLEN_LOWERf, &pPktLenRange->lowerPktLen)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	return RT_ERR_OK;
}
/* Function Name:
 *      apollo_acl_pktLenRange_get
 * Description:
 *      Get ACL IP range check
 * Input:
 *	    None
 * Output:
 *      pPktLenRange  - packet length range configuration
 * Return:
 *      RT_ERR_FAILED           
 *      RT_ERR_OK               
 *      RT_ERR_OUT_OF_RANGE     
 *      RT_ERR_NULL_POINTER     
 * Note:
 *	    None
 */
int32 apollo_raw_acl_pktLenRange_get(apollo_raw_acl_pktLenRange_t *pPktLenRange)
{
	int ret;
    ret = RT_ERR_FAILED;
    
    RT_PARAM_CHK(NULL == pPktLenRange, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pPktLenRange->idx > APOLLO_ACLRANGEMAX2, RT_ERR_OUT_OF_RANGE);
    
	if ((ret = reg_array_field_read(RNG_CHK_PKTLEN_RNGr, REG_ARRAY_INDEX_NONE, pPktLenRange->idx, TYPEf, &pPktLenRange->type)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if ((ret = reg_array_field_read(RNG_CHK_PKTLEN_RNGr, REG_ARRAY_INDEX_NONE, pPktLenRange->idx, PKTLEN_UPPERf, &pPktLenRange->upperPktLen)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	if((ret = reg_array_field_read(RNG_CHK_PKTLEN_RNGr, REG_ARRAY_INDEX_NONE,pPktLenRange->idx, PKTLEN_LOWERf, &pPktLenRange->lowerPktLen)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
    
	return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_acl_mode_set
 * Description:
 *      Set port acl function enable/disable
 * Input:
 *      mode    - 64 entries mode or 128 entries mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED       
 *      RT_ERR_OK 		    
 *      RT_ERR_CHIP_NOT_SUPPORTED           
 * Note:
 *      None
 */
int32 apollo_raw_acl_mode_set(apollo_raw_acl_modeTypes_t mode)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK(APOLLO_ACL_MODE_END <= mode, RT_ERR_CHIP_NOT_SUPPORTED);
    
    if ((ret = reg_field_write(ACL_CFGr, MODEf, &mode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	
	return RT_ERR_OK; 
}

/* Function Name:
 *      apollo_raw_acl_mode_set
 * Description:
 *      Set port acl function enable/disable
 * Input:
 *      None
 * Output:
 *      pMode    - 64 entries mode or 128 entries mode
 * Return:
 *      RT_ERR_FAILED       
 *      RT_ERR_OK 		    
 *      RT_ERR_NULL_POINTER     
 * Note:
 *      None
 */
int32 apollo_raw_acl_mode_get(apollo_raw_acl_modeTypes_t *pMode)
{
    int32 ret = RT_ERR_FAILED;

    RT_PARAM_CHK(NULL == pMode, RT_ERR_NULL_POINTER);
    
    if ((ret = reg_field_read(ACL_CFGr, MODEf, pMode)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_ACL|MOD_DAL), "");
        return ret;
    }
	
	return RT_ERR_OK; 
}

