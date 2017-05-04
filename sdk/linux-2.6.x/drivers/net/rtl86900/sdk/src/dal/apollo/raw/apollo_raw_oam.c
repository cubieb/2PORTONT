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

#include <dal/apollo/raw/apollo_raw_oam.h>

/* Function Name:
 *      apollo_raw_oam_parser_set
 * Description:
 *      Set OAM parser action
 * Input:
 *      port	- port id
 *      action   - OAM parser action
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
int32 apollo_raw_oam_parser_set(rtk_port_t port,  apollo_raw_oam_parserAct_t action)
{
    int32 ret;

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((action >= RAW_OAM_PARSER_ACTION_END), RT_ERR_INPUT);

    if ((ret = reg_array_field_write(OAM_P_CTRL_0r, port, REG_ARRAY_INDEX_NONE, OAM_PARSERf, &action)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_oam_parser_set */

/* Function Name:
 *      apollo_raw_oam_parser_get
 * Description:
 *      Get OAM parser action
 * Input:
 *      port	- port id
 * Output:
 *      pAction   - OAM parser action
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
int32 apollo_raw_oam_parser_get(rtk_port_t port,  apollo_raw_oam_parserAct_t *pAction)
{
    int32 ret;

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);

    if ((ret = reg_array_field_read(OAM_P_CTRL_0r, port, REG_ARRAY_INDEX_NONE, OAM_PARSERf, pAction)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }


    return RT_ERR_OK;
} /* end of apollo_raw_oam_parser_get */


/* Function Name:
 *      apollo_raw_oam_multiplexer_set
 * Description:
 *      Set OAM multiplexer action
 * Input:
 *      port	- port id
 *      action   - OAM multiplexer action
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
int32 apollo_raw_oam_multiplexer_set(rtk_port_t port,  apollo_raw_oam_muxAct_t action)
{
    int32 ret;

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((action >= RAW_OAM_MUX_ACTION_END), RT_ERR_INPUT);

    if ((ret = reg_array_field_write(OAM_P_CTRL_1r, port, REG_ARRAY_INDEX_NONE, OAM_MULTIPLEXERf, &action)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_oam_multiplexer_set */

/* Function Name:
 *      apollo_raw_oam_multiplexer_get
 * Description:
 *      Get OAM multiplexer action
 * Input:
 *      port	- port id
 * Output:
 *      pAction   - OAM multiplexer action
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
int32 apollo_raw_oam_multiplexer_get(rtk_port_t port,  apollo_raw_oam_muxAct_t *pAction)
{
    int32 ret;

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);	

    if ((ret = reg_array_field_read(OAM_P_CTRL_1r, port, REG_ARRAY_INDEX_NONE, OAM_MULTIPLEXERf, pAction)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_oam_multiplexer_get */


/* Function Name:
 *      apollo_raw_oam_priority_set
 * Description:
 *      Set OAM trap priority
 * Input:
 *      priority       - OAM trap priority
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_oam_priority_set(uint32 priority)
{
    int32 ret;

    RT_PARAM_CHK((HAL_INTERNAL_PRIORITY_MAX() < priority), RT_ERR_PRIORITY);

    if ((ret = reg_field_write(OAM_CTRL_0r, OAM_PRIOIRTYf, &priority)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_oam_priority_set */

/* Function Name:
 *      apollo_raw_oam_priority_get
 * Description:
 *      Get OAM trap priority
 * Input:
 *      pPriority       - OAM trap priority
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_oam_priority_get(uint32 *pPriority)
{
    int32 ret;

    RT_PARAM_CHK((NULL == pPriority), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(OAM_CTRL_0r, OAM_PRIOIRTYf, pPriority)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_oam_priority_get */


/* Function Name:
 *      apollo_raw_oam_enable_set
 * Description:
 *      Set OAM state
 * Input:
 *      state       - OAM state
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_oam_enable_set(rtk_enable_t state)
{
    int32 ret;

    RT_PARAM_CHK(RTK_ENABLE_END<= state, RT_ERR_INPUT);

    if ((ret = reg_field_write(OAM_CTRL_1r, OAM_ENABLEf, &state)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_oam_enable_set */



/* Function Name:
 *      apollo_raw_oam_enable_get
 * Description:
 *      Get OAM state
 * Input:
 *      None.
 * Output:
 *      pState      - OAM state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_oam_enable_get(rtk_enable_t *pState)
{
    int32 ret;

    RT_PARAM_CHK((NULL == pState), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(OAM_CTRL_1r, OAM_ENABLEf, pState)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_oam_enable_get */

