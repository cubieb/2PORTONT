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

#include <dal/apollo/raw/apollo_raw_mirror.h>

/* Function Name:
 *      apollo_raw_mirrorCfg_set
 * Description:
 *      Set Mirror configuration
 * Input:
 *      pMirror_cfg     - Mirror Configuration
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
int32 apollo_raw_mirrorCfg_set(raw_mirror_cfg_t *pMirror_cfg)
{
    int32 ret;
    uint32 regData;

    RT_PARAM_CHK((NULL == pMirror_cfg), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pMirror_cfg->monitor_port > APOLLO_PORTIDMAX), RT_ERR_PORT_ID);
    RT_PARAM_CHK((pMirror_cfg->source_portmask.bits[0] > APOLLO_PORTMASK), RT_ERR_PORT_MASK);
    RT_PARAM_CHK(RTK_PORTMASK_IS_PORT_SET(pMirror_cfg->source_portmask, pMirror_cfg->monitor_port), RT_ERR_INPUT);
    RT_PARAM_CHK((pMirror_cfg->mirror_tx >= RTK_ENABLE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((pMirror_cfg->mirror_rx >= RTK_ENABLE_END), RT_ERR_INPUT);

    regData = pMirror_cfg->source_portmask.bits[0];
    if ((ret = reg_field_write(MIR_CTRLr, MIR_SRC_PMSKf, &regData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }

    regData = pMirror_cfg->mirror_tx;
    if ((ret = reg_field_write(MIR_CTRLr, MIR_TXf, &regData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }

    regData = pMirror_cfg->mirror_rx;
    if ((ret = reg_field_write(MIR_CTRLr, MIR_RXf, &regData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }

    regData = pMirror_cfg->monitor_port;
    if ((ret = reg_field_write(MIR_CTRLr, MIR_MONITOR_PORTf, &regData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_mirrorCfg_set */

/* Function Name:
 *      apollo_raw_mirrorCfg_get
 * Description:
 *      Get Mirror configuration
 * Input:
 *      None.
 * Output:
 *      pMirror_cfg     - Mirror Configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_mirrorCfg_get(raw_mirror_cfg_t *pMirror_cfg)
{
    int32 ret;
    uint32 regData;

    RT_PARAM_CHK((NULL == pMirror_cfg), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(MIR_CTRLr, MIR_SRC_PMSKf, &regData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    pMirror_cfg->source_portmask.bits[0] = regData;

    if ((ret = reg_field_read(MIR_CTRLr, MIR_TXf, &regData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    pMirror_cfg->mirror_tx = regData;

    if ((ret = reg_field_read(MIR_CTRLr, MIR_RXf, &regData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    pMirror_cfg->mirror_rx = regData;

    if ((ret = reg_field_read(MIR_CTRLr, MIR_MONITOR_PORTf, &regData)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }
    pMirror_cfg->monitor_port = regData;

    return RT_ERR_OK;
} /* end of apollo_raw_mirrorCfg_set */

/* Function Name:
 *      apollo_raw_mirrorIso_set
 * Description:
 *      Set Mirror Isolation
 * Input:
 *      state       - The state of Mirror Isolation
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_mirrorIso_set(rtk_enable_t state)
{
    int32 ret;

    RT_PARAM_CHK((state >= RTK_ENABLE_END), RT_ERR_INPUT);

    if ((ret = reg_field_write(MIR_CTRLr, MIR_ISOf, (uint32 *)&state)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_mirrorIso_set */

/* Function Name:
 *      apollo_raw_mirrorIso_get
 * Description:
 *      Get Mirror Isolation
 * Input:
 *      None.
 * Output:
 *      pState      - The state of Mirror Isolation
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_mirrorIso_get(rtk_enable_t *pState)
{
    int32 ret;

    RT_PARAM_CHK((NULL == pState), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(MIR_CTRLr, MIR_ISOf, (uint32 *)pState)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_MIRROR), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_mirrorIso_get */
