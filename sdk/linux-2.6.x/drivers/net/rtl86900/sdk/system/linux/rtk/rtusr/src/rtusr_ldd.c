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
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <osal/lib.h>
#include <rtk/rtusr/include/rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>


/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */


/*
 * Macro Declaration
 */


/*
 * Function Declaration
 */
int32 rtk_ldd_i2c_init(rtk_i2c_port_t i2cPort)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    ldd_cfg.i2cPort = i2cPort;
    SETSOCKOPT(RTDRV_LDD_I2C_INIT, &ldd_cfg, rtdrv_lddCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_ldd_gpio_init(void)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    SETSOCKOPT(RTDRV_LDD_GPIO_INIT, &ldd_cfg, rtdrv_lddCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_ldd_reset(rtk_ldd_reset_mode_t mode)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    ldd_cfg.reset = mode;
    SETSOCKOPT(RTDRV_LDD_RESET, &ldd_cfg, rtdrv_lddCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_ldd_calibration_state_set(rtk_enable_t state)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    ldd_cfg.state = state;
    SETSOCKOPT(RTDRV_LDD_CALIBRATION_STATE_SET, &ldd_cfg, rtdrv_lddCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_ldd_parameter_set(uint32 length, uint32 offset, uint8 *flash_data)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    ldd_cfg.length = length;
    ldd_cfg.offset = offset;
    ldd_cfg.flash_data = flash_data;
    SETSOCKOPT(RTDRV_LDD_PARAMETER_SET, &ldd_cfg, rtdrv_lddCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_ldd_parameter_get(uint32 length, uint32 offset, uint8 *flash_data)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == flash_data), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    ldd_cfg.length = length;
    ldd_cfg.offset = offset;
    ldd_cfg.flash_data = flash_data;
    GETSOCKOPT(RTDRV_LDD_PARAMETER_GET, &ldd_cfg, rtdrv_lddCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_ldd_apcEnableFlow_set(rtk_ldd_apc_func_t func, rtk_ldd_loop_mode_t mode)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    ldd_cfg.apc_func = func;
    ldd_cfg.loop_mode = mode;
    SETSOCKOPT(RTDRV_LDD_APC_ENABLE_SET, &ldd_cfg, rtdrv_lddCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_ldd_loopMode_set(rtk_ldd_loop_mode_t mode)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    ldd_cfg.loop_mode = mode;
    SETSOCKOPT(RTDRV_LDD_LOOP_MODE_SET, &ldd_cfg, rtdrv_lddCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_ldd_laserLut_set(rtk_ldd_loop_mode_t mode, uint8 *lut_data)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == lut_data), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    ldd_cfg.loop_mode = mode;
    ldd_cfg.flash_data = lut_data;
    SETSOCKOPT(RTDRV_LDD_LASER_LUT_SET, &ldd_cfg, rtdrv_lddCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_ldd_apdLut_set(uint8 *lut_data)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == lut_data), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    ldd_cfg.flash_data = lut_data;
    SETSOCKOPT(RTDRV_LDD_APD_LUT_SET, &ldd_cfg, rtdrv_lddCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_ldd_txEnableFlow_set(rtk_ldd_tx_func_t func, rtk_ldd_loop_mode_t mode, uint8 *lut_data)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == lut_data), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    ldd_cfg.tx_func = func;
    ldd_cfg.loop_mode = mode;
    ldd_cfg.flash_data = lut_data;
    SETSOCKOPT(RTDRV_LDD_TX_ENABLE_SET, &ldd_cfg, rtdrv_lddCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_ldd_rxEnableFlow_set(rtk_ldd_rx_func_t func)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    ldd_cfg.rx_func = func;
    SETSOCKOPT(RTDRV_LDD_RX_ENABLE_SET, &ldd_cfg, rtdrv_lddCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_ldd_powerOnStatus_get(uint32 *result)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == result), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    GETSOCKOPT(RTDRV_LDD_POWER_ON_GET, &ldd_cfg, rtdrv_lddCfg_t, 1);
    *result = ldd_cfg.result;

    return RT_ERR_OK;
}

int32 rtk_ldd_tx_power_get(uint32 mpd0, uint32 *v_mpd, uint32 *i_mpd)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == v_mpd), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == i_mpd), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    ldd_cfg.mpd0 = mpd0;
    GETSOCKOPT(RTDRV_LDD_TX_POWER_GET, &ldd_cfg, rtdrv_lddCfg_t, 1);
    *i_mpd = ldd_cfg.mpd_i;
    *v_mpd = ldd_cfg.mpd1;

    return RT_ERR_OK;
}

int32 rtk_ldd_rx_power_get(uint32 rssi_v0, uint32 *v_rssi,uint32 *i_rssi)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == v_rssi), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == i_rssi), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    ldd_cfg.rssi_v0 = rssi_v0;
    GETSOCKOPT(RTDRV_LDD_RX_POWER_GET, &ldd_cfg, rtdrv_lddCfg_t, 1);
    *i_rssi = ldd_cfg.rssi_i;
    *v_rssi = ldd_cfg.rssi_voltage;

    return RT_ERR_OK;
}

int32 rtk_ldd_rssiVoltage_get(uint32 *rssi_voltage)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == rssi_voltage), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    GETSOCKOPT(RTDRV_LDD_RSSI_VOLTAGE_GET, &ldd_cfg, rtdrv_lddCfg_t, 1);
    *rssi_voltage = ldd_cfg.rssi_voltage;

    return RT_ERR_OK;
}

int32 rtk_ldd_rssiV0_get(uint32 *rssi_v0)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == rssi_v0), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    GETSOCKOPT(RTDRV_LDD_RSSI_V0_GET, &ldd_cfg, rtdrv_lddCfg_t, 1);
    *rssi_v0 = ldd_cfg.rssi_v0;

    return RT_ERR_OK;
}

int32 rtk_ldd_vdd_get(uint32 *vdd)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == vdd), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    GETSOCKOPT(RTDRV_LDD_VDD_GET, &ldd_cfg, rtdrv_lddCfg_t, 1);
    *vdd = ldd_cfg.vdd;

    return RT_ERR_OK;
}

int32 rtk_ldd_mpd0_get(uint16 count, uint32 *value)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == value), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    ldd_cfg.count= count;
    GETSOCKOPT(RTDRV_LDD_MPD0_GET, &ldd_cfg, rtdrv_lddCfg_t, 1);

    *value = ldd_cfg.mpd0;

    return RT_ERR_OK;
}

int32 rtk_ldd_temperature_get(uint16 *temp)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == temp), RT_ERR_NULL_POINTER);

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    GETSOCKOPT(RTDRV_LDD_TEMPERATURE_GET, &ldd_cfg, rtdrv_lddCfg_t, 1);
    *temp = ldd_cfg.temperature;

    return RT_ERR_OK;
}

int32 rtk_ldd_tx_bias_get(uint32 *bias)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == bias), RT_ERR_NULL_POINTER);

    /* function body */
    GETSOCKOPT(RTDRV_LDD_TX_BIAS_GET, &ldd_cfg, rtdrv_lddCfg_t, 1);
    *bias = ldd_cfg.tx_bias;

    return RT_ERR_OK;
}

int32 rtk_ldd_tx_mod_get(uint32 *mod)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == mod), RT_ERR_NULL_POINTER);

    /* function body */
    GETSOCKOPT(RTDRV_LDD_TX_MOD_GET, &ldd_cfg, rtdrv_lddCfg_t, 1);
    *mod = ldd_cfg.tx_mod;

    return RT_ERR_OK;
}

int32 rtk_ldd_tx_bias_set(uint32 bias)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    ldd_cfg.tx_bias = bias;
    SETSOCKOPT(RTDRV_LDD_TX_BIAS_SET, &ldd_cfg, rtdrv_lddCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_ldd_tx_mod_set(uint32 mod)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    ldd_cfg.tx_mod = mod;
    SETSOCKOPT(RTDRV_LDD_TX_MOD_SET, &ldd_cfg, rtdrv_lddCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_ldd_driver_version_get(uint32 *ver)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == ver), RT_ERR_NULL_POINTER);

    /* function body */
    GETSOCKOPT(RTDRV_LDD_DRIVER_VERSION_GET, &ldd_cfg, rtdrv_lddCfg_t, 1);
    *ver = ldd_cfg.driver_version;

    return RT_ERR_OK;
}

int32 rtk_ldd_steering_mode_set(void)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    SETSOCKOPT(RTDRV_LDD_STEERING_MODE_SET, &ldd_cfg, rtdrv_lddCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_ldd_integration_mode_set(void)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    SETSOCKOPT(RTDRV_LDD_INTEGRATION_MODE_SET, &ldd_cfg, rtdrv_lddCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_ldd_steering_mode_fixup(void)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    SETSOCKOPT(RTDRV_LDD_STEERING_MODE_FIXUP, &ldd_cfg, rtdrv_lddCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_ldd_integration_mode_fixup(void)
{
    rtdrv_lddCfg_t ldd_cfg;

    /* function body */
    osal_memset(&ldd_cfg, 0, sizeof(rtdrv_lddCfg_t));
    SETSOCKOPT(RTDRV_LDD_INTEGRATION_MODE_FIXUP, &ldd_cfg, rtdrv_lddCfg_t, 1);

    return RT_ERR_OK;
}


