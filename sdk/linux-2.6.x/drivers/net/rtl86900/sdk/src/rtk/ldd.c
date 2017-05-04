/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 58888 $
 * $Date: 2015-05-22 13:31:10 +0800 (Fri, 22 May 2015) $
 *
 * Purpose : Definition of LDD APIs
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) LDD control APIs
 *           (2) DDMI APIs
 */


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/init.h>
#include <rtk/default.h>
#include <dal/dal_mgmt.h>
#include <osal/lib.h>
#include <rtk/ldd.h>


/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */
rtk_ldd_mapper_t ldd_mapper;
rtk_ldd_mapper_t *pLddMapper = &ldd_mapper;


/*
 * Macro Declaration
 */


/*
 * Function Declaration
 */
int32 rtk_lddMapper_attach(rtk_ldd_mapper_t *mapper)
{
    if (NULL == mapper)
        return RT_ERR_FAILED;

    osal_memcpy(pLddMapper, mapper, sizeof(rtk_ldd_mapper_t));

    return RT_ERR_OK;
}

int32 rtk_lddMapper_deattach(void)
{
    osal_memset(pLddMapper, 0, sizeof(rtk_ldd_mapper_t));

    return RT_ERR_OK;
}

int32 rtk_ldd_i2c_init(rtk_i2c_port_t port)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_i2c_init)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_i2c_init(port);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_gpio_init(void)
{
    /* no GPIO pin info? */
#if 1
    return RT_ERR_OK;
#else
    int32   ret;

    if (NULL == pLddMapper->ldd_gpio_init)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_gpio_init();
    RTK_API_UNLOCK();

    return ret;
#endif
}

int32 rtk_ldd_reset(rtk_ldd_reset_mode_t mode)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_reset)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_reset(mode);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_calibration_state_set(rtk_enable_t state)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_calibration_state_set)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_calibration_state_set(state);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_parameter_set(uint32 length, uint32 offset, uint8 *flash_data)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_parameter_set)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_parameter_set(length, offset, flash_data);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_parameter_get(uint32 length, uint32 offset, uint8 *flash_data)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_parameter_get)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_parameter_get(length, offset, flash_data);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_loopMode_set(rtk_ldd_loop_mode_t mode)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_loop_mode_set)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_loop_mode_set(mode);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_laserLut_set(rtk_ldd_loop_mode_t mode, uint8 *lut_data)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_laser_lut_set)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_laser_lut_set(mode, lut_data);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_apdLut_set(uint8 *lut_data)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_apd_lut_set)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_apd_lut_set(lut_data);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_apcEnableFlow_set(rtk_ldd_apc_func_t func, rtk_ldd_loop_mode_t mode)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_apc_enable_set)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_apc_enable_set(func, mode);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_txEnableFlow_set(rtk_ldd_tx_func_t func, rtk_ldd_loop_mode_t mode, uint8 *lut_data)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_tx_enable_set)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_tx_enable_set(func, mode, lut_data);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_rxEnableFlow_set(rtk_ldd_rx_func_t func)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_rx_enable_set)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_rx_enable_set(func);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_powerOnStatus_get(uint32 *result)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_power_on_get)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_power_on_get(result);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_tx_power_get(uint32 mpd0, uint32 *v_mpd, uint32 *i_mpd)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_tx_power_get)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_tx_power_get(mpd0, v_mpd, i_mpd);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_rx_power_get(uint32 rssi_v0, uint32 *v_rssi, uint32 *i_rssi)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_rx_power_get)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_rx_power_get(rssi_v0, v_rssi, i_rssi);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_rssiVoltage_get(uint32 *rssi_voltage)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_rssi_voltage_get)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_rssi_voltage_get(rssi_voltage);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_rssiV0_get(uint32 *rssi_v0)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_rssi_v0_get)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_rssi_v0_get(1, rssi_v0);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_vdd_get(uint32 *vdd)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_vdd_get)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_vdd_get(vdd);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_mpd0_get(uint16 count, uint32 *value)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_mpd0_get)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_mpd0_get(count, value);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_temperature_get(uint16 *temp)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_temperature_get)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_temperature_get(temp);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_tx_bias_get(uint32 *bias)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_tx_bias_get)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_tx_bias_get(bias);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_tx_mod_get(uint32 *mod)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_tx_mod_get)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_tx_mod_get(mod);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_tx_bias_set(uint32 bias)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_tx_bias_set)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_tx_bias_set(bias);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_tx_mod_set(uint32 mod)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_tx_mod_set)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_tx_mod_set(mod);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_driver_version_get(uint32 *ver)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_driver_version_get)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_driver_version_get(ver);
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_steering_mode_set(void)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_steering_mode_set)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_steering_mode_set();
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_integration_mode_set(void)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_integration_mode_set)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_integration_mode_set();
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_steering_mode_fixup(void)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_steering_mode_fixup)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_steering_mode_fixup();
    RTK_API_UNLOCK();

    return ret;
}

int32 rtk_ldd_integration_mode_fixup(void)
{
    int32   ret;

    if (NULL == pLddMapper->ldd_integration_mode_fixup)
        return RT_ERR_OK;

    RTK_API_LOCK();
    ret = pLddMapper->ldd_integration_mode_fixup();
    RTK_API_UNLOCK();

    return ret;
}


