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
 * Purpose : switch VLAN RAW API 
 * Feature : 802.1X related functions
 *
 */
#include <dal/apollo/raw/apollo_raw_dot1x.h>

/* Function Name:
 *      apollo_raw_dot1x_unauthAct_set
 * Description:
 *      Set 802.1x unauth. behavior configuration
 * Input:
 *      port 	- Physical port number (0~7)
 *      proc 	- 802.1x unauth. behavior configuration 0:drop 1:trap to CPU 2:Guest VLAN
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_PORT_ID  	- Invalid port number
 *      RT_ERR_DOT1X_PROC  	- Unauthorized behavior error
 * Note:
 *      None
 */
int32 apollo_raw_dot1x_unauthAct_set(uint32 port, uint32 proc)
{
    int ret;

    if(port >= APOLLO_PORTNO)
		return RT_ERR_PORT_ID;

	if(proc >= RAW_DOT1X_UNAUTH_END)
		return RT_ERR_DOT1X_PROC;

    if ((ret = reg_array_field_write(DOT1X_P_CTRLr, port, REG_ARRAY_INDEX_NONE, UNAUTH_ACTf, &proc)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}
/* Function Name:
 *      apollo_raw_dot1x_unauthAct_get
 * Description:
 *      Get 802.1x unauth. behavior configuration
 * Input:
 *      port	- Physical port number (0~7)
 *      pProc 	- 802.1x unauth. behavior configuration 0:drop 1:trap to CPU 2:Guest VLAN
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 * Note:
 *      None
 */
int32 apollo_raw_dot1x_unauthAct_get(uint32 port, uint32* pProc)
{
    int ret;

    if(port >= APOLLO_PORTNO)
		return RT_ERR_PORT_ID;

    if ((ret = reg_array_field_read(DOT1X_P_CTRLr, port, REG_ARRAY_INDEX_NONE, UNAUTH_ACTf, pProc)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_dot1x_guestVidx_set
 * Description:
 *      Set 802.1x guest vlan index
 * Input:
 *      index	- 802.1x guest vlan index (0~31)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_DOT1X_GVLANIDX  	- Invalid cvid index
 * Note:
 *      None
 */
int32 apollo_raw_dot1x_guestVidx_set(uint32 index)
{   
    int ret;

    if(index >= APOLLO_CVIDXNO)
		return RT_ERR_DOT1X_GVLANIDX;

    if ((ret = reg_field_write(DOT1X_CFG_1r, DOT1X_GVIDXf, &index)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}
/* Function Name:
 *      apollo_raw_dot1x_guestVidx_get
 * Description:
 *      Get 802.1x guest vlan index
 * Input:
 *      pIndex 	- 802.1x guest vlan index (0~31)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_dot1x_guestVidx_get(uint32 *pIndex)
{
    int ret;
    if ((ret = reg_field_read(DOT1X_CFG_1r, DOT1X_GVIDXf, pIndex)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      apollo_raw_dot1x_trapPri_set
 * Description:
 *      Set 802.1x trap priority
 * Input:
 *      pri  	- 802.1x trap priority (0~7)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_PRIORITY     	- Invalid priority value
 * Note:
 *      None
 */
int32 apollo_raw_dot1x_trapPri_set(uint32 pri)
{   
    int ret;

    RT_PARAM_CHK((APOLLO_PRIMAX < pri), RT_ERR_PRIORITY);

    if ((ret = reg_field_write(DOT1X_CFG_0r, DOT1X_PRIORTYf, &pri)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      apollo_raw_dot1x_trapPri_get
 * Description:
 *      Get 802.1x trap priority
 * Input:
 *      None
 * Output:
 *      pri  	- 802.1x trap priority (0~7)
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_PRIORITY     	- Invalid priority value
 * Note:
 *      None
 */
int32 apollo_raw_dot1x_trapPri_get(uint32 *pri)
{   
    int ret;

    RT_PARAM_CHK((NULL == pri), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(DOT1X_CFG_0r, DOT1X_PRIORTYf, pri)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}



/* Function Name:
 *      apollo_raw_dot1x_guestVlanOpdir_set
 * Description:
 *      Set 802.1x guest vlan talk to auth. DA
 * Input:
 *      pEnabled  	- enable mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_INPUT     	- Invalid input value
 * Note:
 *      None
 */
int32 apollo_raw_dot1x_guestVlanOpdir_set(uint32 Enabled)
{   
    int ret;

    RT_PARAM_CHK((RTK_ENABLE_END <= Enabled), RT_ERR_INPUT);

    if ((ret = reg_field_write(DOT1X_CFG_1r, DOT1X_GVOPDIRf, &Enabled)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      apollo_raw_dot1x_guestVlanOpdir_get
 * Description:
 *      Get 802.1x guest vlan talk to auth. DA
 * Input:
 *      None
 * Output:
 *      pEnabled  	- enable mode
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_NULL_POINTER     - Invalid priority value
 * Note:
 *      None
 */
int32 apollo_raw_dot1x_guestVlanOpdir_get(uint32 *pEnabled)
{   
    int ret;

    RT_PARAM_CHK((NULL == pEnabled), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(DOT1X_CFG_1r, DOT1X_GVOPDIRf, pEnabled)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}




/* Function Name:
 *      apollo_raw_dot1x_macBaseVlanOpdir_get
 * Description:
 *      Set 802.1x mac-based operational direction
 * Input:
 *      Opdir  	- operational direction
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_INPUT     	- Invalid input value
 * Note:
 *      None
 */
int32 apollo_raw_dot1x_macBaseVlanOpdir_set(apollo_raw_dot1x_opDir_t opdir)
{   
    int ret;

    RT_PARAM_CHK((RAW_DOT1X_DIRECT_END <= opdir), RT_ERR_INPUT);

    if ((ret = reg_field_write(DOT1X_CFG_1r, DOT1X_MAC_OPDIRf, &opdir)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      apollo_raw_dot1x_macBaseVlanOpdir_get
 * Description:
 *      Get 802.1x mac-based operational direction
 * Input:
 *      None
 * Output:
 *      pOpdir  	- operational direction
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_NULL_POINTER     - Invalid priority value
 * Note:
 *      None
 */
int32 apollo_raw_dot1x_macBaseVlanOpdir_get(apollo_raw_dot1x_opDir_t *pOpdir)
{   
    int ret;

    RT_PARAM_CHK((NULL == pOpdir), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(DOT1X_CFG_1r, DOT1X_MAC_OPDIRf, pOpdir)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}



/* Function Name:
 *      apollo_raw_dot1x_portBasedEnable_set
 * Description:
 *      Set 802.1x port-based port enable configuration
 * Input:
 *      port 	- Physical port number (0~7)
 *      Enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_PORT_ID  	- Invalid port number
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_dot1x_portBasedEnable_set(uint32 port, uint32 enable)
{
    int ret;

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    if ((ret = reg_array_field_write(DOT1X_P_CTRLr, port, REG_ARRAY_INDEX_NONE, PB_ENf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}
/* Function Name:
 *      apollo_raw_dot1x_portBasedEnable_get
 * Description:
 *      Get 802.1x port-based port enable configuration
 * Input:
 *      port 	- Physical port number (0~7)
 * Output:
 *      pEnabled - 1: enabled, 0: disabled
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_PORT_ID  	- Invalid port number
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_dot1x_portBasedEnable_get(uint32 port, uint32 *pEnable)
{
    int ret;

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    if ((ret = reg_array_field_read(DOT1X_P_CTRLr, port, REG_ARRAY_INDEX_NONE, PB_ENf, pEnable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}



/* Function Name:
 *      apollo_raw_dot1x_portAuthState_get
 * Description:
 *      Set 802.1x port auth status
 * Input:
 *      port 	- Physical port number (0~7)
 *      auth    - Authorized state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 *      RT_ERR_DOT1X_PROC  	- Unauthorized behavior error
 * Note:
 *      None
 */
int32 apollo_raw_dot1x_portAuthState_set(uint32 port, apollo_raw_dot1x_authState_t auth)
{
    int ret;

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RAW_DOT1X_AUTH_END <= auth), RT_ERR_INPUT);

    if ((ret = reg_array_field_write(DOT1X_P_CTRLr, port, REG_ARRAY_INDEX_NONE, PB_AUTHf, &auth)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}
/* Function Name:
 *      apollo_raw_dot1x_portAuthState_get
 * Description:
 *      Get 802.1x port auth status

 * Input:
 *      port 	- Physical port number (0~7)
 * Output:
 *      auth    - Authorized state
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_PORT_ID  	- Invalid port number
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_dot1x_portAuthState_get(uint32 port, apollo_raw_dot1x_authState_t *pAuth)
{
    int ret;

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pAuth), RT_ERR_NULL_POINTER);

    if ((ret = reg_array_field_read(DOT1X_P_CTRLr, port, REG_ARRAY_INDEX_NONE, PB_AUTHf, pAuth)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}




/* Function Name:
 *      apollo_raw_dot1x_portBaseVlanOpdir_set
 * Description:
 *      Set 802.1x port-based operational direction
 * Input:
 *      Opdir  	- operational direction
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_INPUT     	- Invalid input value
 * Note:
 *      None
 */
int32 apollo_raw_dot1x_portBaseVlanOpdir_set(uint32 port, apollo_raw_dot1x_opDir_t opdir)
{   
    int ret;

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RAW_DOT1X_DIRECT_END <= opdir), RT_ERR_INPUT);

    if ((ret = reg_array_field_write(DOT1X_P_CTRLr, port, REG_ARRAY_INDEX_NONE, PB_DIRf, &opdir)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      apollo_raw_dot1x_portBaseVlanOpdir_get
 * Description:
 *      Get 802.1x port-based operational direction
 * Input:
 *      None
 * Output:
 *      pOpdir  	- operational direction
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_NULL_POINTER     - Invalid priority value
 * Note:
 *      None
 */
int32 apollo_raw_dot1x_portBaseVlanOpdir_get(uint32 port, apollo_raw_dot1x_opDir_t *pOpdir)
{   
    int ret;

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pOpdir), RT_ERR_NULL_POINTER);

    if ((ret = reg_array_field_read(DOT1X_P_CTRLr, port, REG_ARRAY_INDEX_NONE, PB_DIRf, pOpdir)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}



/* Function Name:
 *      apollo_raw_dot1x_macBasedEnable_set
 * Description:
 *      Set 802.1x mac-based port enable configuration
 * Input:
 *      port 	- Physical port number (0~7)
 *      Enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 *      RT_ERR_DOT1X_PROC  	- Unauthorized behavior error
 * Note:
 *      None
 */
int32 apollo_raw_dot1x_macBasedEnable_set(uint32 port, uint32 enable)
{
    int ret;

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    if ((ret = reg_array_field_write(DOT1X_P_CTRLr, port, REG_ARRAY_INDEX_NONE, MAC_ENf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}
/* Function Name:
 *      apollo_raw_dot1x_macBasedEnable_get
 * Description:
 *      Get 802.1x mac-based port enable configuration
 * Input:
 *      port 	- Physical port number (0~7)
 * Output:
 *      pEnabled - 1: enabled, 0: disabled
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_PORT_ID  	- Invalid port number
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_dot1x_macBasedEnable_get(uint32 port, uint32 *pEnable)
{
    int ret;

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    if ((ret = reg_array_field_read(DOT1X_P_CTRLr, port, REG_ARRAY_INDEX_NONE, MAC_ENf, pEnable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DOT1X|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}

