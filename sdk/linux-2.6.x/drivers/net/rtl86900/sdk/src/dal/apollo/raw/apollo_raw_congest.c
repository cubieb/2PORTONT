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
 * Purpose : switch asic-level special congest mode API 
 * Feature : Special congest mode related functions
 *
 */
 
#include <dal/apollo/raw/apollo_raw_congest.h>

/* Function Name:
 *      apollo_raw_sc_cgstInd_reset
 * Description:
 *      Reset TX special congest occurs status. 
 * Input:
 *      portMask 	- Port mask to clean   
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *	    None
 */
 
int32 apollo_raw_sc_cgstInd_reset(rtk_port_t port)
{
    int32 ret = RT_ERR_FAILED;
    uint32 portMask;
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    portMask = 1<<port;
    if ((ret = reg_field_write(SC_P_CTRL_1r, CGST_INDf, &portMask)) != RT_ERR_OK )
    {
        RT_ERR(ret, (MOD_FLOWCTRL|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_sc_cgstInd_get
 * Description:
 *      Get TX special congest ever occurs.
 * Input:
 *      None
 * Output:
 *      pPortMask 	- Port mask of special congest ever occurs
 * Return:
 *      RT_ERR_OK                  - OK
 *      RT_ERR_FAILED            - Failed
 *      RT_ERR_PORT_ID         - Invalid port number 
 *      RT_ERR_NULL_POINTER - input parameter is null pointer 
 * Note:
 *	    None
 */
int32 apollo_raw_sc_cgstInd_get(rtk_port_t port, uint32 *pOccur)
{
    int32 ret = RT_ERR_FAILED;
    uint32 portMask;
    RT_PARAM_CHK(NULL == pOccur, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);

    *pOccur = 0;
    if ((ret = reg_field_read(SC_P_CTRL_1r, CGST_INDf, &portMask)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_FLOWCTRL|MOD_DAL), "");
        return ret;
    }
    if ((1<<port)&portMask)
        *pOccur = 1;
    
    return RT_ERR_OK;  

}

/* Function Name:
 *      apollo_raw_sc_sustTmr_set
 * Description:
 *      Set congest sustain timer limited. 
 * Input:
 *      sustTimer 	- Congest sustain timer limited, unit in seconds.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 *      RT_ERR_PORT_ID  - Invalid port number
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *	    None
 */
 
int32 apollo_raw_sc_sustTmr_set(rtk_port_t port, uint32 sustTimer)
{
    int32 ret = RT_ERR_FAILED;
    RT_PARAM_CHK((APOLLO_RAW_SC_SUSTEN_TIMER_MAX < sustTimer), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);

    if ((ret = reg_array_field_write(SC_P_CTRL_0r, port, REG_ARRAY_INDEX_NONE, CGST_SUST_TMR_LMTf, &sustTimer)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_FLOWCTRL|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_sc_sustTmr_get
 * Description:
 *      Get congest sustain timer limited.
 * Input:
 *      None
 * Output:
 *      pSustTimer 	- Congest sustain timer limited, unit in seconds.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED     - Failed
 *      RT_ERR_PORT_ID  - Invalid port number
 *      RT_ERR_NULL_POINTER - input parameter is null pointer 
 * Note:
 *	    None
 */
 
int32 apollo_raw_sc_sustTmr_get(rtk_port_t port, uint32 *pSustTimer)
{
    int32 ret = RT_ERR_FAILED;
    RT_PARAM_CHK(NULL == pSustTimer, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    if ((ret = reg_array_field_read(SC_P_CTRL_0r, port, REG_ARRAY_INDEX_NONE, CGST_SUST_TMR_LMTf, pSustTimer)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_FLOWCTRL|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;  

}

/* Function Name:
 *      apollo_raw_sc_cgstTmr_get
 * Description:
 *      Get congest timer
 * Input:
 *      None
 * Output:
 *      pCgstTimer 	- Congest timer, unit in seconds.
 * Return:
 *      RT_ERR_OK                  - OK
 *      RT_ERR_FAILED            - Failed
 *      RT_ERR_PORT_ID         - Invalid port number
 *      RT_ERR_NULL_POINTER - input parameter is null pointer 
 * Note:
 *	    None
 */
 
int32 apollo_raw_sc_cgstTmr_get(rtk_port_t port, uint32 *pCgstTimer)
{
    int32 ret = RT_ERR_FAILED;
    RT_PARAM_CHK(NULL == pCgstTimer, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    if ((ret = reg_array_field_read(SC_P_CTRL_0r, port, REG_ARRAY_INDEX_NONE, CGST_TMRf, pCgstTimer)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_FLOWCTRL|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;  

}

