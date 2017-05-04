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
 * Purpose : Definition those public Strom Control APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Storm Control function configuration.
 *
 */

#include <dal/apollo/raw/apollo_raw_storm.h>

/* Function Name:
 *      apollo_raw_stormControlState_set
 * Description:
 *      Set the state of Storm Control.
 * Input:
 *      type            - Storm Control type
 *      port            - The port ID
 *      state           - State
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 *      RT_ERR_PORT_ID
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_stormControlState_set(raw_storm_type_t type, rtk_port_t port, rtk_enable_t state)
{
    int32 ret;
    uint32 storm_reg[RAW_STORM_END] = {
        STORM_CTRL_UM_CTRLr,
        STORM_CTRL_UC_CTRLr,
        STORM_CTRL_MC_CTRLr,
        STORM_CTRL_BC_CTRLr
    };

    RT_PARAM_CHK((type >= RAW_STORM_END), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((port > APOLLO_PORTIDMAX), RT_ERR_PORT_ID);
    RT_PARAM_CHK((state >= RTK_ENABLE_END), RT_ERR_INPUT);

    if ((ret = reg_array_field_write(storm_reg[type], port, REG_ARRAY_INDEX_NONE, ENf, (uint32 *)&state)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stormControlState_set */

/* Function Name:
 *      apollo_raw_stormControlState_get
 * Description:
 *      Get the state of Storm Control.
 * Input:
 *      type            - Storm Control type
 *      port            - The port ID
 * Output:
 *      pState          - State
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 *      RT_ERR_PORT_ID
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_stormControlState_get(raw_storm_type_t type, rtk_port_t port, rtk_enable_t *pState)
{
    int32 ret;
    uint32 storm_reg[RAW_STORM_END] = {
        STORM_CTRL_UM_CTRLr,
        STORM_CTRL_UC_CTRLr,
        STORM_CTRL_MC_CTRLr,
        STORM_CTRL_BC_CTRLr
    };

    RT_PARAM_CHK((type >= RAW_STORM_END), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((port > APOLLO_PORTIDMAX), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pState), RT_ERR_NULL_POINTER);

    if ((ret = reg_array_field_read(storm_reg[type], port, REG_ARRAY_INDEX_NONE, ENf, (uint32 *)pState)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stormControlState_get */

/* Function Name:
 *      apollo_raw_stormControlMeter_set
 * Description:
 *      Set the Meter ID of Storm Control.
 * Input:
 *      type            - Storm Control type
 *      port            - The port ID
 *      meter_idx       - Meter Index
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 *      RT_ERR_PORT_ID
 * Note:
 *      None
 */
int32 apollo_raw_stormControlMeter_set(raw_storm_type_t type, rtk_port_t port, uint32 meter_idx)
{
    int32 ret;
    uint32 storm_reg[RAW_STORM_END] = {
        STORM_CTRL_UM_METER_IDXr,
        STORM_CTRL_UC_METER_IDXr,
        STORM_CTRL_MC_METER_IDXr,
        STORM_CTRL_BC_METER_IDXr
    };

    RT_PARAM_CHK((type >= RAW_STORM_END), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((port > APOLLO_PORTIDMAX), RT_ERR_PORT_ID);
    RT_PARAM_CHK((meter_idx >= HAL_MAX_NUM_OF_METERING()), RT_ERR_OUT_OF_RANGE);

    if ((ret = reg_array_field_write(storm_reg[type], port, REG_ARRAY_INDEX_NONE, IDXf, (uint32 *)&meter_idx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stormControlMeter_set */

/* Function Name:
 *      apollo_raw_stormControlMeter_get
 * Description:
 *      Get the Meter ID of Storm Control.
 * Input:
 *      type            - Storm Control type
 *      port            - The port ID
 * Output:
 *      pMeter_idx      - Meter Index
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 *      RT_ERR_PORT_ID
 * Note:
 *      None
 */
int32 apollo_raw_stormControlMeter_get(raw_storm_type_t type, rtk_port_t port, uint32 *pMeter_idx)
{
    int32 ret;
    uint32 storm_reg[RAW_STORM_END] = {
        STORM_CTRL_UM_METER_IDXr,
        STORM_CTRL_UC_METER_IDXr,
        STORM_CTRL_MC_METER_IDXr,
        STORM_CTRL_BC_METER_IDXr
    };

    RT_PARAM_CHK((type >= RAW_STORM_END), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((port > APOLLO_PORTIDMAX), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pMeter_idx), RT_ERR_NULL_POINTER);

    if ((ret = reg_array_field_read(storm_reg[type], port, REG_ARRAY_INDEX_NONE, IDXf, (uint32 *)pMeter_idx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    return RT_ERR_OK;
} /* apollo_raw_stormControlMeter_get */

/* Function Name:
 *      apollo_raw_stormControlAlt_set
 * Description:
 *      Set alternative type of Storm Control.
 * Input:
 *      type            - Storm Control type
 *      port            - The port ID
 *      alt_type        - Alternative type
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 apollo_raw_stormControlAlt_set(raw_storm_type_t type, raw_storm_alt_type_t alt_type)
{
    int32 ret;
    uint32 storm_field[RAW_STORM_ALT_TYPE_END] = {
        UNMC_TYPEf,
        UNDA_TYPEf,
        MC_TYPEf,
        BC_TYPEf
    };

    RT_PARAM_CHK((type >= RAW_STORM_END), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((alt_type >= RAW_STORM_ALT_TYPE_END), RT_ERR_OUT_OF_RANGE);

    if ((ret = reg_field_write(STORM_CTRL_ALT_TYPE_SELr, storm_field[type], (uint32 *)&alt_type)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stormControlAlt_set */

/* Function Name:
 *      apollo_raw_stormControlAlt_get
 * Description:
 *      Get alternative type of Storm Control.
 * Input:
 *      type            - Storm Control type
 *      port            - The port ID
 * Output:
 *      pAlt_type       - Alternative type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_stormControlAlt_get(raw_storm_type_t type, raw_storm_alt_type_t *pAlt_type)
{
    int32 ret;
    uint32 storm_field[RAW_STORM_ALT_TYPE_END] = {
        UNMC_TYPEf,
        UNDA_TYPEf,
        MC_TYPEf,
        BC_TYPEf
    };

    RT_PARAM_CHK((type >= RAW_STORM_END), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pAlt_type), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(STORM_CTRL_ALT_TYPE_SELr, storm_field[type], (uint32 *)pAlt_type)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_stormControlAlt_set */

