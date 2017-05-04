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
 * $Revision: 39101 $
 * $Date: 2013-05-03 17:35:27 +0800 (Fri, 03 May 2013) $
 *
 * Purpose : Definition of I2C API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) I2C control
 *           (2) I2C read/write
 */



/*
 * Include Files
 */
#include <dal/apollo/dal_apollo.h>
#include <rtk/i2c.h>
#include <dal/apollo/dal_apollo_i2c.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
static uint32    i2c_init = INIT_NOT_COMPLETED;

/*
 * Macro Declaration
 */


/*
 * Function Declaration
 */
/* Function Name:
 *      dal_apollo_i2c_init
 * Description:
 *      Initialize i2c interface.
 * Input:
 *      i2cPort     - I2C port interface
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize before calling any other APIs.
 */
int32
dal_apollo_i2c_init(rtk_i2c_port_t i2cPort)
{
    uint32 data;
    uint32 value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_HWMISC), "");

    /* function body */
    if(I2C_PORT_0 == i2cPort)
    {
        reg_read(I2C_CLOCK_DIVr, &data);

        /* set address width to 8 bit */
        value = 0;
        reg_field_set(I2C_CLOCK_DIVr, I2C_AWf, &value, &data);

        /* set data width to 8 bit */
        value = 0;
        reg_field_set(I2C_CLOCK_DIVr, I2C_DWf, &value, &data);

        /* set clock to 400KHz bit */
        value = 0x4D;
        reg_field_set(I2C_CLOCK_DIVr, I2C_CLOCK_DIVf, &value, &data);

        reg_write(I2C_CLOCK_DIVr, &data);

        /* set enable */
        reg_field_read(IO_MODE_ENr, I2C_ENf, &value);
        value |= 0x1;
        reg_field_write(IO_MODE_ENr, I2C_ENf, &value);
    }
    else
    {
        reg_read(I2C_CLOCK_DIV_1r, &data);

        /* set address width to 8 bit */
        value = 0;
        reg_field_set(I2C_CLOCK_DIV_1r, I2C_AW_1f, &value, &data);

        /* set data width to 8 bit */
        value = 0;
        reg_field_set(I2C_CLOCK_DIV_1r, I2C_DW_1f, &value, &data);

        /* set clock to 400KHz bit */
        value = 0x4D;
        reg_field_set(I2C_CLOCK_DIV_1r, I2C_CLOCK_DIV_1f, &value, &data);

        reg_write(I2C_CLOCK_DIV_1r, &data);

        /* set enable */
        reg_field_read(IO_MODE_ENr, I2C_ENf, &value);
        value |= 0x2;
        reg_field_write(IO_MODE_ENr, I2C_ENf, &value);
    }

    /* Init status */
    i2c_init = INIT_COMPLETED;

    return RT_ERR_OK;
}   /* end of dal_apollo_i2c_init */

/* Function Name:
 *      dal_apollo_i2c_enable_set
 * Description:
 *      Enable/Disable I2C interface.
 * Input:
 *      i2cPort    - I2C port interface
 *      enable     - enable/disable state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *
 */
int32
dal_apollo_i2c_enable_set(rtk_i2c_port_t i2cPort, rtk_enable_t enable)
{
    uint32 val;
    uint32 data;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_HWMISC), "enable=%d",enable);

    /* check Init status */
    RT_INIT_CHK(i2c_init);

    /* parameter check */
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    if(ENABLED == enable)
        data = 1;
    else
        data = 0;

    reg_field_read(IO_MODE_ENr, I2C_ENf, &val);

    if(I2C_PORT_0 == i2cPort)
    {
        val &= ~(0x1<<0);
        val |= (data<<0);
    }
    else
    {
        val &= ~(0x1<<1);
        val |= (data<<1);
    }

    reg_field_write(IO_MODE_ENr, I2C_ENf, &val);

    return RT_ERR_OK;
}   /* end of dal_apollo_i2c_enable_set */

/* Function Name:
 *      dal_apollo_i2c_enable_get
 * Description:
 *      Get I2C interface state.
 * Input:
 *      i2cPort   - I2C port interface
 *      pEnable   - the pointer of enable/disable state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *
 */
int32
dal_apollo_i2c_enable_get(rtk_i2c_port_t i2cPort, rtk_enable_t *pEnable)
{
    uint32 val;
    uint32 mask;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_HWMISC), "");

    /* check Init status */
    RT_INIT_CHK(i2c_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    if(I2C_PORT_0 == i2cPort)
        mask = 0x1<<0;
    else
        mask = 0x1<<1;

    /* function body */
    reg_field_read(IO_MODE_ENr, I2C_ENf, &val);
    if(val & mask)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    return RT_ERR_OK;
}   /* end of dal_apollo_i2c_enable_get */

/* Function Name:
 *      dal_apollo_i2c_width_set
 * Description:
 *      Set the data and address width of I2C interface.
 * Input:
 *      i2cPort   - I2C port interface
 *      width     - 8-bit or 16-bit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *
 */
int32
dal_apollo_i2c_width_set(rtk_i2c_port_t i2cPort, rtk_i2c_width_t width)
{
    uint32 val;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_HWMISC), "width=%d",width);

    /* check Init status */
    RT_INIT_CHK(i2c_init);

    /* parameter check */
    RT_PARAM_CHK((I2C_WIDTH_END <=width), RT_ERR_INPUT);

    /* function body */
    if(I2C_WIDTH_16bit == width)
        val = 1;
    else
        val = 0;

    if(I2C_PORT_0 == i2cPort)
    {
        /* set address width */
        reg_field_write(I2C_CLOCK_DIVr, I2C_AWf, &val);

        /* set data width */
        reg_field_write(I2C_CLOCK_DIVr, I2C_DWf, &val);
    }
    else
    {
        /* set address width */
        reg_field_write(I2C_CLOCK_DIV_1r, I2C_AW_1f, &val);

        /* set data width */
        reg_field_write(I2C_CLOCK_DIV_1r, I2C_DW_1f, &val);
    }

    return RT_ERR_OK;
}   /* end of dal_apollo_i2c_width_set */

/* Function Name:
 *      dal_apollo_i2c_width_get
 * Description:
 *      Get the data and address width of I2C interface.
 * Input:
 *      i2cPort    - I2C port interface
 *      pWidth     - the pointer of width
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *
 */
int32
dal_apollo_i2c_width_get(rtk_i2c_port_t i2cPort, rtk_i2c_width_t *pWidth)
{
    uint32 val;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_HWMISC), "");

    /* check Init status */
    RT_INIT_CHK(i2c_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pWidth), RT_ERR_NULL_POINTER);

    /* function body */
    if(I2C_PORT_0 == i2cPort)
    {
        reg_field_read(I2C_CLOCK_DIVr, I2C_AWf, &val);
    }
    else
    {
        reg_field_read(I2C_CLOCK_DIV_1r, I2C_AW_1f, &val);
    }

    if(val == 1)
        *pWidth = I2C_WIDTH_16bit;
    else
        *pWidth = I2C_WIDTH_8bit;

    return RT_ERR_OK;
}   /* end of dal_apollo_i2c_width_get */

/* Function Name:
 *      dal_apollo_i2c_write
 * Description:
 *      I2c write data.
 * Input:
 *      i2cPort   - I2C port interface
 *      devID     - the device ID
 *      regAddr   - register address
 *      data      - data value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *
 */
int32
dal_apollo_i2c_write(rtk_i2c_port_t i2cPort, uint32 devID, uint32 regAddr, uint32 data)
{
    uint32 value;
    uint32 en;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_HWMISC), "port=%d,devID=%d,regAddr=%d",i2cPort, devID, regAddr);

    /* check Init status */
    RT_INIT_CHK(i2c_init);

    /* function body */
    if(I2C_PORT_0 == i2cPort)
    {
        /* set device ID */
        reg_field_write(I2C_CLOCK_DIVr, I2C_DEV_IDf, &devID);
        /* set write data */
        reg_field_write(I2C_IND_WDr, WR_DATf, &data);

        /* set regAddr */
        value=0;
        reg_field_set(I2C_IND_CMDr, ADRf, &regAddr, &value);
        /* set write */
        en = 1;
        reg_field_set(I2C_IND_CMDr, WRENf, &en, &value);
        /* set cmd en */
        en = 1;
        reg_field_set(I2C_IND_CMDr, CMD_ENf, &en, &value);

        reg_write(I2C_IND_CMDr, &value);

        /* check busy */
        do{
            reg_field_read(I2C_IND_RDr, BUSYf, &value);
        }while(value == 1);
    }
    else
    {
        /* set device ID */
        reg_field_write(I2C_CLOCK_DIV_1r, I2C_DEV_ID_1f, &devID);
        /* set write data */
        reg_field_write(I2C_IND_WD_1r, WR_DAT_1f, &data);

        /* set regAddr */
        value=0;
        reg_field_set(I2C_IND_CMD_1r, ADR_1f, &regAddr, &value);
        /* set write */
        en = 1;
        reg_field_set(I2C_IND_CMD_1r, WREN_1f, &en, &value);
        /* set cmd en */
        en = 1;
        reg_field_set(I2C_IND_CMD_1r, CMD_EN_1f, &en, &value);

        reg_write(I2C_IND_CMD_1r, &value);

        /* check busy */
        do{
            reg_field_read(I2C_IND_RD_1r, BUSY_1f, &value);
        }while(value == 1);
    }

    return RT_ERR_OK;
}   /* end of dal_apollo_i2c_write */

/* Function Name:
 *      dal_apollo_i2c_read
 * Description:
 *      I2c read data.
 * Input:
 *      i2cPort   - I2C port interface
 *      devID     - the device ID
 *      regAddr   - register address
 *      pData     - the pointer of returned data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *
 */
int32
dal_apollo_i2c_read(rtk_i2c_port_t i2cPort, uint32 devID, uint32 regAddr, uint32 *pData)
{
    uint32 value;
    uint32 en;
    uint32 busy;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_HWMISC), "port=%d,devID=%d,regAddr=%d",i2cPort, devID, regAddr);

    /* check Init status */
    RT_INIT_CHK(i2c_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    /* function body */
    if(I2C_PORT_0 == i2cPort)
    {
        /* set device ID */
        reg_field_write(I2C_CLOCK_DIVr, I2C_DEV_IDf, &devID);

        /* set regAddr */
        value=0;
        reg_field_set(I2C_IND_CMDr, ADRf, &regAddr, &value);
        /* set read */
        en = 0;
        reg_field_set(I2C_IND_CMDr, WRENf, &en, &value);
        /* set cmd en */
        en = 1;
        reg_field_set(I2C_IND_CMDr, CMD_ENf, &en, &value);

        reg_write(I2C_IND_CMDr, &value);

        do
        {
            reg_read(I2C_IND_RDr, &value);
            reg_field_get(I2C_IND_RDr, BUSYf, &busy, &value);
        } while (busy == 1);

        reg_field_get(I2C_IND_RDr, RD_DATf, pData, &value);
    }
    else
    {
        /* set device ID */
        reg_field_write(I2C_CLOCK_DIV_1r, I2C_DEV_ID_1f, &devID);

        /* set regAddr */
        value=0;
        reg_field_set(I2C_IND_CMD_1r, ADR_1f, &regAddr, &value);
        /* set read */
        en = 0;
        reg_field_set(I2C_IND_CMD_1r, WREN_1f, &en, &value);
        /* set cmd en */
        en = 1;
        reg_field_set(I2C_IND_CMD_1r, CMD_EN_1f, &en, &value);

        reg_write(I2C_IND_CMD_1r, &value);

        do
        {
            reg_read(I2C_IND_RD_1r, &value);
            reg_field_get(I2C_IND_RD_1r, BUSY_1f, &busy, &value);
        } while (busy == 1);

        reg_field_get(I2C_IND_RD_1r, RD_DAT_1f, pData, &value);
    }

    return RT_ERR_OK;
}   /* end of dal_apollo_i2c_read */


