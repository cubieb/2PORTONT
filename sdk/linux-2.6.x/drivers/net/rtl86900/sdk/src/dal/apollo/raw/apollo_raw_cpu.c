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
 * Purpose : Definition those public Trap APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *            1) Packets trap to CPU setting.
 *            2) RMA (Reserved MAC address).
 *
 */

#include <dal/apollo/raw/apollo_raw_cpu.h>

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */
/* prefix of reserve multicast address */

/* Function Name:
 *      apollo_raw_cpu_aware_port_mask_set
 * Description:
 *      Set CPU aware port mask.
 * Input:
 *      port_mask        - CPU aware port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_MASK
 * Note:
 *      None.
 */
int32 apollo_raw_cpu_aware_port_mask_set(rtk_portmask_t port_mask)
{
    uint32 enable;
    uint32 port;
    int32 ret;

    /* parameter check */
    RT_PARAM_CHK(!(HAL_IS_PORTMASK_VALID(port_mask)), RT_ERR_PORT_MASK);

    for(port=0; port<APOLLO_PORTNO; port++)
    {
        if(RTK_PORTMASK_IS_PORT_SET(port_mask, port))
        {
            enable = 1;
            if ((ret = reg_array_field_write(MAC_CPU_TAG_AWARE_CTRLr,port,REG_ARRAY_INDEX_NONE,ENf,&enable)) != RT_ERR_OK)
                RT_ERR(ret, (MOD_DAL|MOD_CPU), "");
        }
        else
        {
            enable = 0;
            if ((ret = reg_array_field_write(MAC_CPU_TAG_AWARE_CTRLr,port,REG_ARRAY_INDEX_NONE,ENf,&enable)) != RT_ERR_OK)
                RT_ERR(ret, (MOD_DAL|MOD_CPU), "");
        }
    }

    return RT_ERR_OK;
} /* end of apollo_raw_cpu_aware_port_mask_set */

/* Function Name:
 *      apollo_raw_cpu_aware_port_mask_get
 * Description:
 *      Get CPU aware port mask.
 * Input:
 *      pPort_mask        - pointer of return CPU aware port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None.
 */
int32 apollo_raw_cpu_aware_port_mask_get(rtk_portmask_t *pPort_mask)
{
    uint32 enable;
    uint32 port;
    int32 ret;

    /* parameter check */
    RT_PARAM_CHK((NULL==pPort_mask), RT_ERR_NULL_POINTER);

    osal_memset(pPort_mask, 0x0, sizeof(rtk_portmask_t));
    for(port=0; port<APOLLO_PORTNO; port++)
    {
        if ((ret = reg_array_field_read(MAC_CPU_TAG_AWARE_CTRLr,port,REG_ARRAY_INDEX_NONE,ENf,&enable))
            != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_CPU), "");
        }

        if(enable)
        {
            RTK_PORTMASK_PORT_SET(*pPort_mask, port);
        }
        else
        {
            RTK_PORTMASK_PORT_CLEAR(*pPort_mask, port);
        }
    }

    return RT_ERR_OK;
} /* end of apollo_raw_cpu_aware_port_mask_get */

/* Function Name:
 *      apollo_raw_cpu_tag_format_set
 * Description:
 *      Set CPU tag format.
 * Input:
 *      mode        - CPU tag format mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None.
 */
int32 apollo_raw_cpu_tag_format_set(rtk_cpu_tag_fmt_t mode)
{
    int32 ret;
    uint32 data;

    /* parameter check */
    RT_PARAM_CHK((mode>=CPU_TAG_FMT_END), RT_ERR_OUT_OF_RANGE);

    data = mode;
    if ((ret = reg_field_write(MAC_CPU_TAG_CTRLr, TAG_FORMATf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_CPU), "");
    }

    return ret;
} /* end of apollo_raw_cpu_tag_format_set */

/* Function Name:
 *      apollo_raw_cpu_tag_format_get
 * Description:
 *      Get CPU tag format.
 * Input:
 *      pMode        - pointer of return CPU tag format mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None.
 */
int32 apollo_raw_cpu_tag_format_get(rtk_cpu_tag_fmt_t *pMode)
{
    int32 ret;
    uint32 data;

    /* parameter check */
    RT_PARAM_CHK((NULL==pMode), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(MAC_CPU_TAG_CTRLr, TAG_FORMATf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_CPU), "");
    }
    *pMode = data;

    return RT_ERR_OK;
} /* end of apollo_raw_cpu_tag_format_get */

/* Function Name:
 *      apollo_raw_cpu_dsl_en_set
 * Description:
 *      Set DSL enable state.
 * Input:
 *      state        - DSL enable state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None.
 */
int32 apollo_raw_cpu_dsl_en_set(rtk_enable_t state)
{
    int32 ret;
    uint32 data;

    /* parameter check */
    RT_PARAM_CHK((state>=RTK_ENABLE_END), RT_ERR_OUT_OF_RANGE);

    data = state;
    if ((ret = reg_field_write(MAC_CPU_TAG_CTRLr, DSL_PORT_ENf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_CPU), "");
    }

    return ret;
} /* end of apollo_raw_cpu_dsl_en_set */

/* Function Name:
 *      apollo_raw_cpu_dsl_en_get
 * Description:
 *      Get DSL enable state.
 * Input:
 *      pState        - pointer of return DSL enable state.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None.
 */
int32 apollo_raw_cpu_dsl_en_get(rtk_enable_t *pState)
{
    int32 ret;
    uint32 data;

    /* parameter check */
    RT_PARAM_CHK((NULL==pState), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(MAC_CPU_TAG_CTRLr, DSL_PORT_ENf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_CPU), "");
    }
    *pState = data;

    return RT_ERR_OK;
} /* end of apollo_raw_cpu_dsl_en_get */

/* Function Name:
 *      apollo_raw_cpu_trap_insert_tag_set
 * Description:
 *      Set stae of insert CPU tag for trap port
 * Input:
 *      state        - insert tag state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None.
 */
int32 apollo_raw_cpu_trap_insert_tag_set(rtk_enable_t state)
{
    int32 ret;
    uint32 data;

    /* parameter check */
    RT_PARAM_CHK((state>=RTK_ENABLE_END), RT_ERR_OUT_OF_RANGE);

    data = state;
    if ((ret = reg_field_write(MAC_CPU_TAG_CTRLr, TRAP_TAGET_INSERT_ENf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_CPU), "");
    }

    return ret;
} /* end of apollo_raw_cpu_trap_insert_tag_set */

/* Function Name:
 *      apollo_raw_cpu_trap_insert_tag_get
 * Description:
 *      Get stae of insert CPU tag for trap port
 * Input:
 *      pState        - pointer of return insert tag state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None.
 */
int32 apollo_raw_cpu_trap_insert_tag_get(rtk_enable_t *pState)
{
    int32 ret;
    uint32 data;

    /* parameter check */
    RT_PARAM_CHK((NULL==pState), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(MAC_CPU_TAG_CTRLr, TRAP_TAGET_INSERT_ENf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_CPU), "");
    }
    *pState = data;

    return RT_ERR_OK;
} /* end of apollo_raw_cpu_trap_insert_tag_get */
