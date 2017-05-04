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
 * $Revision: 54353 $
 * $Date: 2014-12-25 20:03:23 +0800 (Thu, 25 Dec 2014) $
 *
 * Purpose : Definition those CPU command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *
 */

/*
 * Include Files
 */
#include <stdio.h>
#include <string.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <diag_util.h>
#include <diag_str.h>
#include <rtk/i2c.h>

/*
 * i2c init port <UINT:port>
 */
cparser_result_t
cparser_cmd_i2c_init_port_port(
    cparser_context_t *context,
    uint32_t  *port_ptr)
{
    int32 ret;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_i2c_init(*port_ptr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_i2c_init_port_port */

/*
 * i2c set state port <UINT:port> ( enable | disable )
 */
cparser_result_t
cparser_cmd_i2c_set_state_port_port_enable_disable(
    cparser_context_t *context,
    uint32_t  *port_ptr)
{
    rtk_enable_t enable;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(5,0))
        enable = ENABLED;
    else if('d' == TOKEN_CHAR(5,0))
        enable = DISABLED;

    DIAG_UTIL_ERR_CHK(rtk_i2c_enable_set(*port_ptr, enable), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_i2c_set_state_port_port_enable_disable */

/*
 * i2c get state port <UINT:port>
 */
cparser_result_t
cparser_cmd_i2c_get_state_port_port(
    cparser_context_t *context,
    uint32_t  *port_ptr)
{
    int32 ret;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_i2c_enable_get(*port_ptr, &enable), ret);

    diag_util_printf("I2C state: %s\n\r", diagStr_enable[enable]);

    return CPARSER_OK;
}    /* end of cparser_cmd_i2c_get_state_port_port */

/*
 * i2c set width port <UINT:port> ( 8-bit | 16-bit | 24-bit | 32-bit )
 */
cparser_result_t
cparser_cmd_i2c_set_width_port_port_8_bit_16_bit_24_bit_32_bit(
    cparser_context_t *context,
    uint32_t  *port_ptr)
{
    rtk_i2c_width_t width;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();

    if('8' == TOKEN_CHAR(5,0))
        width = I2C_WIDTH_8bit;
    else if('1' == TOKEN_CHAR(5,0))
        width = I2C_WIDTH_16bit;
    else if('2' == TOKEN_CHAR(5,0))
        width = I2C_WIDTH_24bit;
    else if('3' == TOKEN_CHAR(5,0))
        width = I2C_WIDTH_32bit;
    else
        width = I2C_WIDTH_8bit;



    DIAG_UTIL_ERR_CHK(rtk_i2c_width_set(*port_ptr, width), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_i2c_set_width_port_port_8_bit_16_bit_24_bit_32_bit */

/*
 * i2c get width port <UINT:port>
 */
cparser_result_t
cparser_cmd_i2c_get_width_port_port(
    cparser_context_t *context,
    uint32_t  *port_ptr)
{
    int32 ret;
    rtk_i2c_width_t width;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_i2c_width_get(*port_ptr, &width), ret);

    diag_util_printf("I2C width: %s\n\r", diagStr_i2cWidth[width]);

    return CPARSER_OK;
}    /* end of cparser_cmd_i2c_get_width_port_port */

/*
 * i2c set port <UINT:port> dev <UINT:dev_id> reg <UINT:register> data <UINT:data>
 */
cparser_result_t
cparser_cmd_i2c_set_port_port_dev_dev_id_reg_register_data_data(
    cparser_context_t *context,
    uint32_t  *port_ptr,
    uint32_t  *dev_id_ptr,
    uint32_t  *register_ptr,
    uint32_t  *data_ptr)
{
    rtk_i2c_width_t width;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_i2c_write(*port_ptr, *dev_id_ptr, *register_ptr, *data_ptr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_i2c_set_port_port_dev_dev_id_reg_register_data_data */

/*
 * i2c get port <UINT:port> dev <UINT:dev_id> reg <UINT:register>
 */
cparser_result_t
cparser_cmd_i2c_get_port_port_dev_dev_id_reg_register(
    cparser_context_t *context,
    uint32_t  *port_ptr,
    uint32_t  *dev_id_ptr,
    uint32_t  *register_ptr)
{
    uint32 data;
    int32  ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_i2c_read(*port_ptr, *dev_id_ptr, *register_ptr, &data), ret);

    diag_util_printf("  dev: 0x%x, reg: 0x%x, data: 0x%x \n\r", *dev_id_ptr, *register_ptr, data);

    return CPARSER_OK;
}    /* end of cparser_cmd_i2c_get_port_port_dev_dev_id_reg_register */

/*
 * i2c set clock port <UINT:port> <UINT:khz>
 */
cparser_result_t
cparser_cmd_i2c_set_clock_port_port_khz(
    cparser_context_t *context,
    uint32_t  *port_ptr,
    uint32_t  *khz_ptr)
{
    rtk_i2c_width_t width;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_i2c_clock_set(*port_ptr, *khz_ptr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_i2c_set_clock_port_port_khz */

/*
 * i2c get clock port <UINT:port>
 */
cparser_result_t
cparser_cmd_i2c_get_clock_port_port(
    cparser_context_t *context,
    uint32_t  *port_ptr)
{
    int32 ret;
    uint32 clock;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_i2c_clock_get(*port_ptr, &clock), ret);

    diag_util_printf("I2C clock: %d KHz\n\r", clock);

    return CPARSER_OK;
}    /* end of cparser_cmd_i2c_get_clock_port_port */

/*
 * i2c set eeprom-mirror dev <UINT:dev_id> addr-width ( 8-bit | 16-bit | 24-bit | 32-bit ) addr-ext-mode ( mode0 | mode1 | mode2 | mode3 )
 */
cparser_result_t
cparser_cmd_i2c_set_eeprom_mirror_dev_dev_id_addr_width_8_bit_16_bit_24_bit_32_bit_addr_ext_mode_mode0_mode1_mode2_mode3(
    cparser_context_t *context,
    uint32_t  *dev_id_ptr)
{
    rtk_i2c_eeprom_mirror_t eepMirrirCfg;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();

    eepMirrirCfg.devId = *dev_id_ptr;

    if('8' == TOKEN_CHAR(6,0))
        eepMirrirCfg.addrWidth = I2C_WIDTH_8bit;
    else if('1' == TOKEN_CHAR(6,0))
        eepMirrirCfg.addrWidth = I2C_WIDTH_16bit;
    else if('2' == TOKEN_CHAR(6,0))
        eepMirrirCfg.addrWidth = I2C_WIDTH_24bit;
    else if('3' == TOKEN_CHAR(6,0))
        eepMirrirCfg.addrWidth = I2C_WIDTH_32bit;
    else
        eepMirrirCfg.addrWidth = I2C_WIDTH_8bit;

    if('0' == TOKEN_CHAR(8,4))
        eepMirrirCfg.addrExtMode = I2C_ADDR_EXT_MODE_0;
    else if('1' == TOKEN_CHAR(8,4))
        eepMirrirCfg.addrExtMode = I2C_ADDR_EXT_MODE_1;
    else if('2' == TOKEN_CHAR(8,4))
        eepMirrirCfg.addrExtMode = I2C_ADDR_EXT_MODE_2;
    else if('3' == TOKEN_CHAR(8,4))
        eepMirrirCfg.addrExtMode = I2C_ADDR_EXT_MODE_3;
    else
        eepMirrirCfg.addrExtMode = I2C_ADDR_EXT_MODE_0;


    DIAG_UTIL_ERR_CHK(rtk_i2c_eepMirror_set(eepMirrirCfg), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_i2c_set_eeprom_mirror_dev_dev_id_addr_width_8_bit_16_bit_24_bit_32_bit_addr_ext_mode_mode0_mode1_mode2_mode3 */

/*
 * i2c get eeprom-mirror
 */
cparser_result_t
cparser_cmd_i2c_get_eeprom_mirror(
    cparser_context_t *context)
{
    int32 ret;
    rtk_i2c_eeprom_mirror_t eepMirrirCfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_i2c_eepMirror_get(&eepMirrirCfg), ret);

    diag_util_printf("I2C EEPROM Mirror: \n\r");
    diag_util_printf("    device ID     0x%x \n\r", eepMirrirCfg.devId);
    diag_util_printf("    addr width    %s \n\r", diagStr_i2cWidth[eepMirrirCfg.addrWidth]);
    diag_util_printf("    addr ext mode %s \n\r", diagStr_i2cEepMirrorAddrExtMode[eepMirrirCfg.addrExtMode]);

    return CPARSER_OK;
}    /* end of cparser_cmd_i2c_get_eeprom_mirror */

/*
 * i2c set eeprom-mirror-reg <UINT:register> data <UINT:data>
 */
cparser_result_t
cparser_cmd_i2c_set_eeprom_mirror_reg_register_data_data(
    cparser_context_t *context,
    uint32_t  *register_ptr,
    uint32_t  *data_ptr)
{
    rtk_i2c_width_t width;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_i2c_eepMirror_write(*register_ptr, *data_ptr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_i2c_set_eeprom_mirror_reg_register_data_data */

/*
 * i2c get eeprom-mirror-reg <UINT:register>
 */
cparser_result_t
cparser_cmd_i2c_get_eeprom_mirror_reg_register(
    cparser_context_t *context,
    uint32_t  *register_ptr)
{
    uint32 data;
    int32  ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_i2c_eepMirror_read(*register_ptr, &data), ret);

    diag_util_printf("  reg: 0x%x, data: 0x%x \n\r",*register_ptr, data);

    return CPARSER_OK;
}    /* end of cparser_cmd_i2c_get_eeprom_mirror_reg_register */

/*
 * i2c set addr-width port <UINT:port> ( 8-bit | 16-bit | 24-bit | 32-bit )
 */
cparser_result_t
cparser_cmd_i2c_set_addr_width_port_port_8_bit_16_bit_24_bit_32_bit(
    cparser_context_t *context,
    uint32_t  *port_ptr)
{
    rtk_i2c_width_t width;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();

    if('8' == TOKEN_CHAR(5,0))
        width = I2C_WIDTH_8bit;
    else if('1' == TOKEN_CHAR(5,0))
        width = I2C_WIDTH_16bit;
    else if('2' == TOKEN_CHAR(5,0))
        width = I2C_WIDTH_24bit;
    else if('3' == TOKEN_CHAR(5,0))
        width = I2C_WIDTH_32bit;
    else
        width = I2C_WIDTH_8bit;


    DIAG_UTIL_ERR_CHK(rtk_i2c_addrWidth_set(*port_ptr, width), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_i2c_set_addr_width_port_port_8_bit_16_bit_24_bit_32_bit */

/*
 * i2c get addr-width port <UINT:port>
 */
cparser_result_t
cparser_cmd_i2c_get_addr_width_port_port(
    cparser_context_t *context,
    uint32_t  *port_ptr)
{
    int32 ret;
    rtk_i2c_width_t width;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_i2c_addrWidth_get(*port_ptr, &width), ret);

    diag_util_printf("I2C width: %s\n\r", diagStr_i2cWidth[width]);

    return CPARSER_OK;
}    /* end of cparser_cmd_i2c_get_addr_width_port_port */


/*
 * i2c set data-width port <UINT:port> ( 8-bit | 16-bit | 24-bit | 32-bit )
 */
cparser_result_t
cparser_cmd_i2c_set_data_width_port_port_8_bit_16_bit_24_bit_32_bit(
    cparser_context_t *context,
    uint32_t  *port_ptr)
{
    rtk_i2c_width_t width;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();

    if('8' == TOKEN_CHAR(5,0))
        width = I2C_WIDTH_8bit;
    else if('1' == TOKEN_CHAR(5,0))
        width = I2C_WIDTH_16bit;
    else if('2' == TOKEN_CHAR(5,0))
        width = I2C_WIDTH_24bit;
    else if('3' == TOKEN_CHAR(5,0))
        width = I2C_WIDTH_32bit;
    else
        width = I2C_WIDTH_8bit;


    DIAG_UTIL_ERR_CHK(rtk_i2c_dataWidth_set(*port_ptr, width), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_i2c_set_data_width_port_port_8_bit_16_bit_24_bit_32_bit */

/*
 * i2c get data-width port <UINT:port>
 */
cparser_result_t
cparser_cmd_i2c_get_data_width_port_port(
    cparser_context_t *context,
    uint32_t  *port_ptr)
{
    int32 ret;
    rtk_i2c_width_t width;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_i2c_dataWidth_get(*port_ptr, &width), ret);

    diag_util_printf("I2C width: %s\n\r", diagStr_i2cWidth[width]);

    return CPARSER_OK;
}    /* end of cparser_cmd_i2c_get_data_width_port_port */

