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
 * Purpose : switch asic-level Auto Fallback API
 * Feature : Auto Fallback related functions
 *
 */

#include <dal/apollo/raw/apollo_raw_autofallback.h>

/* Function Name:
 *      apollo_raw_autofallback_reducePL_set
 * Description:
 *      Set Auto Fallback Reduce Power Level.
 * Input:
 *      state   - State of Reduce Power Level
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_autofallback_reducePL_set(rtk_enable_t state)
{
    int32 ret;

    RT_PARAM_CHK((state >= RTK_ENABLE_END), RT_ERR_INPUT);

    if ((ret = reg_field_write(FB_CTRLr, PL_DEC_ENf, (uint32 *)&state)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_autofallback_reducePL_get
 * Description:
 *      Get Auto Fallback Reduce Power Level.
 * Input:
 *      None
 * Output:
 *      pState  - State of Reduce Power Level
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_autofallback_reducePL_get(rtk_enable_t *pState)
{
    int32 ret;

    RT_PARAM_CHK((NULL == pState), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(FB_CTRLr, PL_DEC_ENf, (uint32 *)pState)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_autofallback_timeoutTH_set
 * Description:
 *      Set Auto Fallback timeout threshold.
 * Input:
 *      time    - timeout threshold (from 0 ~ 1020ms, 4ms per step)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 apollo_raw_autofallback_timeoutTH_set(uint32 time)
{
    int32 ret;
    uint32 regdata;

    RT_PARAM_CHK((time > RAW_AUTOFALLBACK_TIMEOUT_MAX), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(((time % RAW_AUTOFALLBACK_TIMEOUT_STEP) != 0), RT_ERR_INPUT);

    regdata = time / RAW_AUTOFALLBACK_TIMEOUT_STEP;
    if ((ret = reg_field_write(FB_CTRLr, TO_THf, &regdata)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_autofallback_timeoutTH_get
 * Description:
 *      Get Auto Fallback timeout threshold.
 * Input:
 *      None
 * Output:
 *      pTime    - timeout threshold (from 0 ~ 1020ms, 4ms per step)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_autofallback_timeoutTH_get(uint32 *pTime)
{
    int32 ret;
    uint32 regdata;

    RT_PARAM_CHK((NULL == pTime), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(FB_CTRLr, TO_THf, &regdata)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    *pTime = regdata * RAW_AUTOFALLBACK_TIMEOUT_STEP;
    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_autofallback_timeoutIgnore_set
 * Description:
 *      Set Auto Fallback Timeout Ignore function.
 * Input:
 *      state   - State of RTimeout Ignore function
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_autofallback_timeoutIgnore_set(rtk_enable_t state)
{
    int32 ret;

    RT_PARAM_CHK((state >= RTK_ENABLE_END), RT_ERR_INPUT);

    if ((ret = reg_field_write(FB_CTRLr, TO_IGNOREf, (uint32 *)&state)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_autofallback_timeoutIgnore_get
 * Description:
 *      Get Auto Fallback Timeout Ignore function.
 * Input:
 *      None
 * Output:
 *      pState  - State of RTimeout Ignore function
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_autofallback_timeoutIgnore_get(rtk_enable_t *pState)
{
    int32 ret;

    RT_PARAM_CHK((NULL == pState), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(FB_CTRLr, TO_IGNOREf, (uint32 *)pState)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_autofallback_maxMonitorCount_set
 * Description:
 *      Set Auto Fallback Max Monitor Count
 * Input:
 *      mon_count   - Max Monitor Count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_autofallback_maxMonitorCount_set(raw_autofallback_monitor_t mon_count)
{
    int32 ret;

    RT_PARAM_CHK((mon_count >= RAW_MONITOR_END), RT_ERR_INPUT);

    if ((ret = reg_field_write(FB_CTRLr, MAX_THf, (uint32 *)&mon_count)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_autofallback_maxMonitorCount_get
 * Description:
 *      Get Auto Fallback Max Monitor Count
 * Input:
 *      None.
 * Output:
 *      pMon_count  - Max Monitor Count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_autofallback_maxMonitorCount_get(raw_autofallback_monitor_t *pMon_count)
{
    int32 ret;

    RT_PARAM_CHK((NULL == pMon_count), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(FB_CTRLr, MAX_THf, (uint32 *)pMon_count)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_autofallback_maxErrorCount_set
 * Description:
 *      Set Auto Fallback Error Count
 * Input:
 *      err_count   - Max Error Count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_autofallback_maxErrorCount_set(raw_autofallback_error_t err_count)
{
    int32 ret;

    RT_PARAM_CHK((err_count >= RAW_ERROR_END), RT_ERR_INPUT);

    if ((ret = reg_field_write(FB_CTRLr, ERR_THf, (uint32 *)&err_count)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_autofallback_maxErrorCount_get
 * Description:
 *      Get Auto Fallback Error Count
 * Input:
 *      None.
 * Output:
 *      pErr_count  - Max Error Count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_autofallback_maxErrorCount_get(raw_autofallback_error_t *pErr_count)
{
    int32 ret;

    RT_PARAM_CHK((NULL == pErr_count), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(FB_CTRLr, ERR_THf, (uint32 *)pErr_count)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_autofallback_timer_set
 * Description:
 *      Set Auto Fallback timer state
 * Input:
 *      state       - Timer state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_autofallback_timer_set(rtk_enable_t state)
{
    int32 ret;
    uint32 regdata;

    RT_PARAM_CHK((state >= RTK_ENABLE_END), RT_ERR_INPUT);

    regdata = (state == ENABLED) ? 0 : 1;
    if ((ret = reg_field_write(FB_CTRLr, STOP_TMRf, (uint32 *)&regdata)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_autofallback_timer_get
 * Description:
 *      Get Auto Fallback timer state
 * Input:
 *      None.
 * Output:
 *      pState      - Timer state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_autofallback_timer_get(rtk_enable_t *pState)
{
    int32 ret;
    uint32 regdata;

    RT_PARAM_CHK((NULL == pState), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(FB_CTRLr, STOP_TMRf, &regdata)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    *pState = (regdata == 0) ? ENABLED : DISABLED;
    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_autofallback_enable_set
 * Description:
 *      Set Auto Fallback Per port state
 * Input:
 *      port        - Port ID
 *      state       - State
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_PORT_ID
 * Note:
 *      None
 */
int32 apollo_raw_autofallback_enable_set(rtk_port_t port, rtk_enable_t state)
{
    int32 ret;

    RT_PARAM_CHK((port > APOLLO_PORTIDMAX), RT_ERR_PORT_ID);
    RT_PARAM_CHK((state >= RTK_ENABLE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((2 == port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((6 == port), RT_ERR_PORT_ID);

    if(port < 2)
    {
        if ((ret = reg_array_field_write(FB_P0_1_CFGr, (uint32)port, REG_ARRAY_INDEX_NONE, ENf, (uint32 *)&state)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }
    else
    {
        if ((ret = reg_array_field_write(FB_P3_5_CFGr, (uint32)(port - 3), REG_ARRAY_INDEX_NONE, ENf, (uint32 *)&state)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_autofallback_enable_get
 * Description:
 *      Get Auto Fallback Per port state
 * Input:
 *      port        - Port ID
 * Output:
 *      pState      - State
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_PORT_ID
 * Note:
 *      None
 */
int32 apollo_raw_autofallback_enable_get(rtk_port_t port, rtk_enable_t *pState)
{
    int32 ret;

    RT_PARAM_CHK((port > APOLLO_PORTIDMAX), RT_ERR_PORT_ID);
    RT_PARAM_CHK((2 == port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((6 == port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pState), RT_ERR_NULL_POINTER);

    if(port < 2)
    {
        if ((ret = reg_array_field_read(FB_P0_1_CFGr, (uint32)port, REG_ARRAY_INDEX_NONE, ENf, (uint32 *)pState)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }
    else
    {
        if ((ret = reg_array_field_read(FB_P3_5_CFGr, (uint32)(port - 3), REG_ARRAY_INDEX_NONE, ENf, (uint32 *)pState)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_autofallback_restorePL_get
 * Description:
 *      Get Auto Fallback Restore Power Level state
 * Input:
 *      port        - Port ID
 * Output:
 *      pState      - Restore Power Level State
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_PORT_ID
 * Note:
 *      None
 */
int32 apollo_raw_autofallback_restorePL_get(rtk_port_t port, raw_autofallback_restorePL_t *pState)
{
    int32 ret;

    RT_PARAM_CHK((port > APOLLO_PORTIDMAX), RT_ERR_PORT_ID);
    RT_PARAM_CHK((2 == port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((6 == port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pState), RT_ERR_NULL_POINTER);

    if(port < 2)
    {
        if ((ret = reg_array_field_read(FB_P0_1_CFGr, (uint32)port, REG_ARRAY_INDEX_NONE, RST_PLf, (uint32 *)pState)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }
    else
    {
        if ((ret = reg_array_field_read(FB_P3_5_CFGr, (uint32)(port - 3), REG_ARRAY_INDEX_NONE, RST_PLf, (uint32 *)pState)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_autofallback_validFlow_get
 * Description:
 *      Get Auto Fallback per port Valid Flow State
 * Input:
 *      port        - Port ID
 * Output:
 *      pValidFlow  - Valid Flow State
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_PORT_ID
 * Note:
 *      None
 */
int32 apollo_raw_autofallback_validFlow_get(rtk_port_t port, raw_autofallback_validFlow_t *pValidFlow)
{
    int32 ret;

    RT_PARAM_CHK((port > APOLLO_PORTIDMAX), RT_ERR_PORT_ID);
    RT_PARAM_CHK((2 == port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((6 == port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pValidFlow), RT_ERR_NULL_POINTER);

    if(port < 2)
    {
        if ((ret = reg_array_field_read(FB_P0_1_CFGr, (uint32)port, REG_ARRAY_INDEX_NONE, VALID_FLOWf, (uint32 *)pValidFlow)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }
    else
    {
        if ((ret = reg_array_field_read(FB_P3_5_CFGr, (uint32)(port - 3), REG_ARRAY_INDEX_NONE, VALID_FLOWf, (uint32 *)pValidFlow)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_autofallback_errorCount_get
 * Description:
 *      Get Auto Fallback per port Error Counter
 * Input:
 *      port        - Port ID
 * Output:
 *      pErrCount   - Error Count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_PORT_ID
 * Note:
 *      None
 */
int32 apollo_raw_autofallback_errorCount_get(rtk_port_t port, uint32 *pErrCount)
{
    int32 ret;

    RT_PARAM_CHK((port > APOLLO_PORTIDMAX), RT_ERR_PORT_ID);
    RT_PARAM_CHK((2 == port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((6 == port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pErrCount), RT_ERR_NULL_POINTER);

    if(port < 2)
    {
        if ((ret = reg_array_field_read(FB_P0_1_ERR_CNTr, (uint32)port, REG_ARRAY_INDEX_NONE, CNTf, (uint32 *)pErrCount)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }
    else
    {
        if ((ret = reg_array_field_read(FB_P3_5_ERR_CNTr, (uint32)(port - 3), REG_ARRAY_INDEX_NONE, CNTf, (uint32 *)pErrCount)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_autofallback_monitorCount_get
 * Description:
 *      Get Auto Fallback per port Monitor Counter
 * Input:
 *      port        - Port ID
 * Output:
 *      pMonCount   - Monitor Count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_PORT_ID
 * Note:
 *      None
 */
int32 apollo_raw_autofallback_monitorCount_get(rtk_port_t port, uint32 *pMonCount)
{
    int32 ret;

    RT_PARAM_CHK((port > APOLLO_PORTIDMAX), RT_ERR_PORT_ID);
    RT_PARAM_CHK((2 == port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((6 == port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pMonCount), RT_ERR_NULL_POINTER);

    if(port < 2)
    {
        if ((ret = reg_array_field_read(FB_P0_1_MONITOR_CNTr, (uint32)port, REG_ARRAY_INDEX_NONE, CNTf, (uint32 *)pMonCount)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }
    else
    {
        if ((ret = reg_array_field_read(FB_P0_1_MONITOR_CNTr, (uint32)(port - 3), REG_ARRAY_INDEX_NONE, CNTf, (uint32 *)pMonCount)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    return RT_ERR_OK;
}

