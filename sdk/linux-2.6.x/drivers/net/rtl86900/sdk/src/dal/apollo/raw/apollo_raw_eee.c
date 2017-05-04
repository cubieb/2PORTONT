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
 * Purpose : switch asic-level EEE API
 * Feature : EEE related functions
 *
 */

#include <dal/apollo/raw/apollo_raw_eee.h>

/* Function Name:
 *      apollo_raw_eee_lldpTrapPri_set
 * Description:
 *      Set EEE lldp trapping priority  
 * Input:
 *      priority 		- trapping priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PRIORITY
 * Note:
 *      None
 */
int32 apollo_raw_eee_lldpTrapPri_set(rtk_pri_t priority)
{
    int32 ret;
    
    RT_PARAM_CHK((RTK_DOT1P_PRIORITY_MAX < priority), RT_ERR_PRIORITY);

    if ((ret = reg_field_write(EEELLDP_CTRL_1r, TRAP_PRIf, &priority)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EEE), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_eee_lldpTrapPri_set */

/* Function Name:
 *      apollo_raw_eee_lldpTrapPri_get
 * Description:
 *      Get EEE lldp trapping priority  
 * Input:
 *      None
 * Output:
 *      None
 *      pPriority 		- trapping priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_eee_lldpTrapPri_get(rtk_pri_t *pPriority)
{
    int32 ret;

    RT_PARAM_CHK((pPriority==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(EEELLDP_CTRL_1r, TRAP_PRIf, pPriority)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EEE), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_eee_lldpTrapPri_get */

/* Function Name:
 *      apollo_raw_eee_lldpEnable_set
 * Description:
 *      set eee lldp function enable
 * Input:
 *      enable      - enable/disable
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_INPUT  				- Invalid input parameter
 * Note:
 *      None
 */
int32 apollo_raw_eee_lldpEnable_set(rtk_enable_t enable)
{
	int32   ret;

    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    if ((ret = reg_field_write(EEELLDP_CTRL_0r,ENf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_EEE), "");
        return ret;
    }  
    
    return RT_ERR_OK;
} /* end of apollo_raw_eee_lldpEnable_set */

/* Function Name:
 *      apollo_raw_eee_lldpEnable_get
 * Description:
 *      get eee lldp function enable
 * Input:
 *      none
 * Output:
 *      pEnable      - enable/disable 
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_INPUT  				- Invalid input parameter
 * Note:
 *      None
 */
int32 apollo_raw_eee_lldpEnable_get(rtk_enable_t *pEnable)
{
	int32   ret;

    RT_PARAM_CHK((pEnable==NULL), RT_ERR_NULL_POINTER);
 
  
    if ((ret = reg_field_read(EEELLDP_CTRL_0r,ENf, pEnable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_EEE), "");
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of apollo_raw_eee_lldpEnable_get */

/* Function Name:
 *      apollo_raw_eee_lldpTrapEnable_set
 * Description:
 *      set eee lldp trap function enable
 * Input:
 *      enable      - enable/disable
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_INPUT  				- Invalid input parameter
 * Note:
 *      None
 */
int32 apollo_raw_eee_lldpTrapEnable_set(rtk_enable_t enable)
{
	int32   ret;

    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    if ((ret = reg_field_write(EEELLDP_CTRL_0r,TRAP_ENf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_EEE), "");
        return ret;
    }  
    
    return RT_ERR_OK;
} /* end of apollo_raw_eee_lldpTrapEnable_set */

/* Function Name:
 *      apollo_raw_eee_lldpTrapEnable_get
 * Description:
 *      get eee lldp trap function enable
 * Input:
 *      none
 * Output:
 *      pEnable      - enable/disable 
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_INPUT  				- Invalid input parameter
 * Note:
 *      None
 */
int32 apollo_raw_eee_lldpTrapEnable_get(rtk_enable_t *pEnable)
{
	int32   ret;

    RT_PARAM_CHK((pEnable==NULL), RT_ERR_NULL_POINTER);
 
  
    if ((ret = reg_field_read(EEELLDP_CTRL_0r,TRAP_ENf, pEnable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_EEE), "");
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of apollo_raw_eee_lldpTrapEnable_get */

/* Function Name:
 *      apollo_raw_eee_lldpSubtype_set
 * Description:
 *      set eee lldp subtype field
 * Input:
 *      subtype      - subtype field of eee lldp
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_INPUT  				- Invalid input parameter
 * Note:
 *      None
 */
int32 apollo_raw_eee_lldpSubtype_set(uint32 subtype)
{
	int32   ret;

    RT_PARAM_CHK((subtype > APOLLO_EEELLDP_SUBTYPE_MAX), RT_ERR_INPUT);

    if ((ret = reg_field_write(EEELLDP_CTRL_0r,SUBTYPEf, &subtype)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_EEE), "");
        return ret;
    }  
    
    return RT_ERR_OK;
} /* end of apollo_raw_eee_lldpSubtype_set */

/* Function Name:
 *      apollo_raw_eee_lldpSubtype_get
 * Description:
 *      get eee lldp subtype field
 * Input:
 *      none
 * Output:
 *      pSubtype      - subtype field of eee lldp
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_INPUT  				- Invalid input parameter
 * Note:
 *      None
 */
int32 apollo_raw_eee_lldpSubtype_get(rtk_enable_t *pSubtype)
{
	int32   ret;

    RT_PARAM_CHK((pSubtype==NULL), RT_ERR_NULL_POINTER);
 
  
    if ((ret = reg_field_read(EEELLDP_CTRL_0r,SUBTYPEf, pSubtype)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_VLAN|MOD_EEE), "");
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of apollo_raw_eee_lldpSubtype_get */


