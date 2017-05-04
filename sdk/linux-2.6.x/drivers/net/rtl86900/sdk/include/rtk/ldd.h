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
 * $Revision: 42664 $
 * $Date: 2013-09-09 11:17:16 +0800 (Mon, 09 Sep 2013) $
 *
 * Purpose : Definition of LDD API
 *
 * Feature : Provide the APIs to enable and configure LDD
 *
 */

#ifndef __RTK_LDD_API_H__
#define __RTK_LDD_API_H__


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/i2c.h>


/*
 * Symbol Definition
 */

#define LDD_DRIVER_VERSION_MAJOR(V) ((V&0xFF0000)>>16)
#define LDD_DRIVER_VERSION_MINOR(V) ((V&0x00FF00)>>8)
#define LDD_DRIVER_VERSION_PATCH(V) (V&0x0000FF)

/*
 * Data Declaration
 */
typedef enum
{
    LDD_RESET_CHIP       = 0,
    LDD_RESET_DIGITAL    = 1,
    LDD_RESET_8051       = 2,
    LDD_RESET_ANALOG     = 3,
    LDD_RESET_END
} rtk_ldd_reset_mode_t;

typedef enum
{
    LDD_LOOP_DOL         = 0,
    LDD_LOOP_DCL         = 1,
    LDD_LOOP_SCL_BIAS    = 2,
    LDD_LOOP_SCL_MOD     = 3,
    LDD_LOOP_MODE_END
} rtk_ldd_loop_mode_t;

typedef enum
{
    LDD_APC_MCU_READY,
    LDD_APC_CHECK_READY,
    LDD_APC_TX_SD_THRESHOLD,
    LDD_APC_BIAS_POWER_ON,
    LDD_APC_POWER_SET,
    LDD_APC_DIGITAL_POWER_ON,
    LDD_APC_R_PROCESS_DETECT,
    LDD_APC_TX_RPD_RCODE_OUT,
    LDD_APC_OFFSET_CALIBR,
    LDD_APC_MEASURE_MPD0,
    LDD_APC_WAIT_CALIBR,
    LDD_APC_MOVE_CALIBR_RESULT,
    LDD_APC_FUNC_END
} rtk_ldd_apc_func_t;

typedef enum
{
    LDD_TX_BIAS_MOD_PROTECT,
    LDD_TX_ENABLE_TX,
    LDD_TX_BIAS_MOD_LUT,
    LDD_TX_P0_P1_PAV,
    LDD_TX_LOOP_MODE,
    LDD_TX_BURST_MODE_CTRL,
    LDD_TX_WAIT_500MS,
    LDD_TX_FAULT_ENABLE,
    LDD_TX_ENABLE_TX_SD,
    LDD_TX_FUNC_END
} rtk_ldd_tx_func_t;

typedef enum
{
    LDD_RX_BOOSTER,
    LDD_RX_RX,
    LDD_RX_RSSI,
    LDD_RX_TX_SD_PIN,
    LDD_RX_DAC,
    LDD_RX_THRESHOLD,
    LDD_RX_ENABLE_RX_SD,
    LDD_RX_FUNC_END
} rtk_ldd_rx_func_t;

typedef struct
{
    uint32  rssi_v0;
    uint32  rssi_voltage;
    uint32  rssi_i;
    uint32  rssi_v0_2;
    uint32  rssi_voltage_2;
    uint32  rssi_i_2;
    uint32  mpd0;
    uint32  mpd1;
    uint32  mpd_i;
    uint32  bias;
    uint32  modulation;
    uint32  temperature;
    uint32  vdd;
    int32   rx_a;
    int32   rx_b;
    int32   rx_c;
    int32   tx_a;
    int32   tx_b;
    int32   tx_c;
    int8    temp_offset;   /* temperature offset of Tj and Tbosa */
    uint8   temp_intr_scale; /* granuality of temperature interrupt 
			       (for Vapd temperature compensation) */
    uint8   rx_th;           /* RX SD/LOS assert threshold */
    uint8   rx_deth;         /* RX SD/LOS de-assert threshold */
} rtk_ldd_param_t;

typedef struct
{
    int32   (*ldd_i2c_init)(rtk_i2c_port_t port);
    int32   (*ldd_gpio_init)(void);
    int32   (*ldd_reset)(rtk_ldd_reset_mode_t mode);
    int32   (*ldd_calibration_state_set)(rtk_enable_t state);
    int32   (*ldd_parameter_set)(uint32 length, uint32 offset, uint8 *flash_data);
    int32   (*ldd_parameter_get)(uint32 length, uint32 offset, uint8 *flash_data);
    int32   (*ldd_loop_mode_set)(rtk_ldd_loop_mode_t mode);
    int32   (*ldd_laser_lut_set)(rtk_ldd_loop_mode_t mode, uint8 *lut_data);
    int32   (*ldd_apd_lut_set)(uint8 *lut_data);
    int32   (*ldd_apc_enable_set)(rtk_ldd_apc_func_t func, rtk_ldd_loop_mode_t mode);
    int32   (*ldd_tx_enable_set)(rtk_ldd_tx_func_t func, rtk_ldd_loop_mode_t mode, uint8 *lut_data);
    int32   (*ldd_rx_enable_set)(rtk_ldd_rx_func_t func);
    int32   (*ldd_power_on_get)(uint32 *result);
    int32   (*ldd_tx_power_get)(uint32 mpd0, uint32 *v_mpd, uint32 *i_mpd);
    int32   (*ldd_rx_power_get)(uint32 rssi_v0, uint32 *v_rssi, uint32 *i_rssi);
    int32   (*ldd_rssi_voltage_get)(uint32 *rssi_voltage);
    int32   (*ldd_rssi_v0_get)(uint16 count, uint32 *rssi_v0);
    int32   (*ldd_vdd_get)(uint32 *vdd);
    int32   (*ldd_mpd0_get)(uint16 count, uint32 *value);
    int32   (*ldd_temperature_get)(uint16 *temp);
    int32   (*ldd_tx_bias_get)(uint32 *bias);
    int32   (*ldd_tx_mod_get)(uint32 *mod);
    int32   (*ldd_tx_bias_set)(uint32 bias);
    int32   (*ldd_tx_mod_set)(uint32 mod);
    int32   (*ldd_driver_version_get)(uint32 *ver);
    int32   (*ldd_steering_mode_set)(void);
    int32   (*ldd_integration_mode_set)(void);
    int32   (*ldd_steering_mode_fixup)(void);
    int32   (*ldd_integration_mode_fixup)(void);
} rtk_ldd_mapper_t;


/*
 * Function Declaration
 */
extern int32
rtk_lddMapper_attach(rtk_ldd_mapper_t *mapper);

extern int32
rtk_lddMapper_deattach(void);

extern int32
rtk_ldd_i2c_init(rtk_i2c_port_t port);

extern int32
rtk_ldd_gpio_init(void);

extern int32
rtk_ldd_reset(rtk_ldd_reset_mode_t mode);

extern int32
rtk_ldd_calibration_state_set(rtk_enable_t state);

extern int32
rtk_ldd_parameter_set(uint32 length, uint32 offset, uint8 *flash_data);

extern int32
rtk_ldd_parameter_get(uint32 length, uint32 offset, uint8 *flash_data);

extern int32
rtk_ldd_loopMode_set(rtk_ldd_loop_mode_t mode);

extern int32
rtk_ldd_laserLut_set(rtk_ldd_loop_mode_t mode, uint8 *lut_data);

extern int32
rtk_ldd_apdLut_set(uint8 *lut_data);

extern int32
rtk_ldd_apcEnableFlow_set(rtk_ldd_apc_func_t func, rtk_ldd_loop_mode_t mode);

extern int32
rtk_ldd_txEnableFlow_set(rtk_ldd_tx_func_t func, rtk_ldd_loop_mode_t mode, uint8 *lut_data);

extern int32
rtk_ldd_rxEnableFlow_set(rtk_ldd_rx_func_t func);

extern int32
rtk_ldd_powerOnStatus_get(uint32 *result);

extern int32
rtk_ldd_tx_power_get(uint32 mpd0, uint32 *v_mpd, uint32 *i_mpd);

extern int32
rtk_ldd_rx_power_get(uint32 rssi_v0, uint32 *v_rssi,uint32 *i_rssi);

extern int32
rtk_ldd_rssiVoltage_get(uint32 *rssi_voltage);

extern int32
rtk_ldd_rssiV0_get(uint32 *rssi_v0);

extern int32
rtk_ldd_vdd_get(uint32 *vdd);

extern int32
rtk_ldd_mpd0_get(uint16 count, uint32 *value);

extern int32
rtk_ldd_temperature_get(uint16 *temp);

extern int32
rtk_ldd_tx_bias_get(uint32 *bias);

extern int32
rtk_ldd_tx_mod_get(uint32 *mod);

extern int32
rtk_ldd_tx_bias_set(uint32 bias);

extern int32
rtk_ldd_tx_mod_set(uint32 mod);

extern int32
rtk_ldd_driver_version_get(uint32 *ver);

extern int32
rtk_ldd_steering_mode_set(void);

extern int32
rtk_ldd_integration_mode_set(void);

extern int32
rtk_ldd_steering_mode_fixup(void);

extern int32
rtk_ldd_integration_mode_fixup(void);
#endif  /* __RTK_LDD_API_H__ */
