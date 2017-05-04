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
 * Purpose : Definition of Security API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) attack prevention 
 */


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <common/rt_error.h>

#include <dal/apollo/dal_apollo.h>
#include <rtk/led.h>
#include <dal/apollo/dal_apollo_led.h>
/*
 * Symbol Definition
 */

#define APOLLO_PARALLEL_LED_MAX  16



typedef enum apollo_led_paralleIoPin_e
{
    APOLLO_LED_IO_LED0  = 15,
    APOLLO_LED_IO_LED1  = 16,
    APOLLO_LED_IO_LED2  = 13,
    APOLLO_LED_IO_LED3  = 14,
    APOLLO_LED_IO_LED4  = 11,
    APOLLO_LED_IO_LED5  = 12,
    APOLLO_LED_IO_LED6  = 9,
    APOLLO_LED_IO_LED7  = 10,
    APOLLO_LED_IO_LED8  = 7,
    APOLLO_LED_IO_LED9  = 8,
    APOLLO_LED_IO_LED10 = 6,
    APOLLO_LED_IO_LED11 = 5,
    APOLLO_LED_IO_LED12 = 4,
    APOLLO_LED_IO_LED13 = 3,
    APOLLO_LED_IO_LED14 = 2,
    APOLLO_LED_IO_LED15 = 0,
    APOLLO_LED_IO_LED16 = 1
}apollo_led_paralleIoBit_t;



typedef enum apollo_led_operation_e
{
    APOLLO_LED_OP_PARALLEL = 0,
    APOLLO_LED_OP_SERIAL   = 1,
    APOLLO_LED_OP_END,
}apollo_led_operation_t;



typedef enum apollo_led_active_e
{
    APOLLO_LED_ACTIVE_HIGH = 0,
    APOLLO_LED_ACTIVE_LOW  = 1,
    APOLLO_LED_ACTIVE_END,
}apollo_led_active_t;



typedef enum apollo_led_enable_e
{
    APOLLO_LED_DISABLED = 0,
    APOLLO_LED_ENABLED  = 1,
    APOLLO_LED_ENABLE_END,
}apollo_led_enable_t;



typedef enum apollo_led_blink_rate_e
{
    APOLLO_LED_BLINKRATE_32MS  = 0,
    APOLLO_LED_BLINKRATE_64MS  = 1,
    APOLLO_LED_BLINKRATE_128MS = 2,
    APOLLO_LED_BLINKRATE_256MS = 3,
    APOLLO_LED_BLINKRATE_512MS = 4,
    APOLLO_LED_BLINKRATE_1024MS= 5,
    APOLLO_LED_BLINKRATE_48MS  = 6,
    APOLLO_LED_BLINKRATE_96MS  = 7,
    APOLLO_LED_BLINKRATE_END,
}apollo_led_blink_rate_t;


/* led type */
typedef enum apollo_led_type_e
{
    APOLLO_LED_TYPE_UTP0   = 0x01,
    APOLLO_LED_TYPE_UTP1   = 0x02,
    APOLLO_LED_TYPE_UTP2   = 0x05,
    APOLLO_LED_TYPE_UTP3   = 0x06,
    APOLLO_LED_TYPE_UTP4   = 0x03,
    APOLLO_LED_TYPE_UTP5   = 0x04,
    APOLLO_LED_TYPE_FIBER  = 0x1A,
    APOLLO_LED_TYPE_PON    = 0x1B,
    APOLLO_LED_TYPE_USB0   = 0x1C,
    APOLLO_LED_TYPE_USB1   = 0x1D,
    APOLLO_LED_TYPE_SATA   = 0x1E,
    APOLLO_LED_TYPE_END
} apollo_led_type_t;



typedef enum apollo_led_force_mode_e
{
    APOLLO_LED_FORCE_OFF   = 0,
    APOLLO_LED_FORCE_ON    = 1,
    APOLLO_LED_FORCE_BLINK = 2,
    APOLLO_LED_FORCE_END
}apollo_led_force_mode_t;


/*
 * Data Declaration
 */
static uint32    led_init = {INIT_NOT_COMPLETED}; 



/*
 * Function Declaration
 */

/* Function Name:
 *      dal_apollo_led_init
 * Description:
 *      Initialize led module.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize led module before calling any led APIs.
 */
int32
dal_apollo_led_init(void)
{
    int     ledIdx;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_LED),"%s",__FUNCTION__);

    led_init = INIT_COMPLETED;
    
    /*disable all parallel LED*/
    for(ledIdx = 0; ledIdx <= APOLLO_PARALLEL_LED_MAX ; ledIdx++)        
    {
        if((ret = dal_apollo_led_parallelEnable_set(ledIdx, DISABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_LED | MOD_DAL), "");
            led_init = INIT_NOT_COMPLETED;
            return ret;
        }
    }
    
    /*set led mode to parallel*/
    if((ret = dal_apollo_led_operation_set(LED_OP_PARALLEL)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED | MOD_DAL), "");
        led_init = INIT_NOT_COMPLETED;
        return ret;
    }
        
    return RT_ERR_OK;
} /* end of dal_apollo_led_init */

/* Function Name:
 *      rtk_led_operation_set
 * Description:
 *      Get Led operation mode
 * Input:
 *      None
 * Output:
 *      pMode - LED operation mode.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API can set Led operation mode.
 *      The modes that can be set are as following:
 *      - LED_OP_PARALLEL 17 led
 *      - LED_OP_SERIAL   32 led 
 */
int32 
dal_apollo_led_operation_get(rtk_led_operation_t *pMode)
{
    int32   ret;
    uint32  data;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_LED),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(led_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(LED_LEDr, LED_SELf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }
    
    if(APOLLO_LED_OP_PARALLEL == data)
        *pMode = LED_OP_PARALLEL;
    else
        *pMode = LED_OP_SERIAL;
    return RT_ERR_OK;
} /* end of dal_apollo_led_operation_get */


/* Function Name:
 *      dal_apollo_led_operation_set
 * Description:
 *      Set Led operation mode
 * Input:
 *      mode - LED operation mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API can set Led operation mode.
 *      The modes that can be set are as following:
 *      - LED_OP_PARALLEL 13 led
 *      - LED_OP_SERIAL   32 led 
 */
int32 
dal_apollo_led_operation_set(rtk_led_operation_t mode)
{
    int32   ret;
    uint32  data;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_LED),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(led_init);

    /* parameter check */
    switch(mode)
    {
        case LED_OP_PARALLEL:
            data = APOLLO_LED_OP_PARALLEL;
            break;
        case LED_OP_SERIAL:
            data = APOLLO_LED_OP_SERIAL;
            break;
        default:
            return RT_ERR_INPUT;
            break;
    }

    if ((ret = reg_field_write(LED_LEDr, LED_SELf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
} /* end of dal_apollo_led_operation_set */


/* Function Name:
 *      dal_apollo_led_serialMode_set
 * Description:
 *      Set Led serial mode active congiuration
 * Input:
 *      None
 * Output:
 *      pActive - high low active mode.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API can set LED serial mode active congiuration.
 */
int32 
dal_apollo_led_serialMode_get(rtk_led_active_t *pActive)
{
    int32   ret;
    uint32  data;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_LED),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(led_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pActive), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(SERI_LED_ACTIVE_LOW_CFGr, SERI_LED_ACTIVE_LOWf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }

    if(APOLLO_LED_ACTIVE_HIGH == data)
        *pActive = LED_ACTIVE_HIGH;
    else
        *pActive = LED_ACTIVE_LOW;
        
    return RT_ERR_OK;
} /* end of dal_apollo_led_serialMode_get */

/* Function Name:
 *      dal_apollo_led_serialMode_set
 * Description:
 *      Set Led serial mode active congiuration
 * Input:
 *      active - high low active mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API can set LED serial mode active congiuration.
 */
int32 
dal_apollo_led_serialMode_set(rtk_led_active_t active)
{
    int32   ret;
    uint32  data;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_LED),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(led_init);

    /* parameter check */
    switch(active)
    {
        case LED_ACTIVE_HIGH:
            data = APOLLO_LED_ACTIVE_HIGH;
            break;
        case LED_ACTIVE_LOW:
            data = APOLLO_LED_ACTIVE_LOW;
            break;
        default:
            return RT_ERR_INPUT;
            break;
    }

    if ((ret = reg_field_write(SERI_LED_ACTIVE_LOW_CFGr, SERI_LED_ACTIVE_LOWf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_apollo_led_serialMode_set */

/* Function Name:
 *      dal_apollo_led_blinkRate_get
 * Description:
 *      Get LED blinking rate
 * Input:
 *      group      - led blinking group
 * Output:
 *      pBlinkRate - blinking rate.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      There are 8 types of LED blinking rates at 32ms, 48ms, 64ms, 96ms, 128ms, 256ms, 512ms, and 1024ms.
 */
int32 
dal_apollo_led_blinkRate_get(rtk_led_blinkGroup_t group, rtk_led_blink_rate_t *pBlinkRate)
{
    int32   ret;
    uint32  data;
    uint32  field;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_LED),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(led_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pBlinkRate), RT_ERR_NULL_POINTER);

    switch(group)
    {
        case LED_BLINK_GROUP_PORT:
            field = SEL_MAC_LED_RATEf;
            break;
        case LED_BLINK_GROUP_USB:
            field = SEL_USB_LED_RATEf;
            break;
        case LED_BLINK_GROUP_SATA:
            field = SEL_SATA_LED_RATEf;
            break;
        case LED_BLINK_GROUP_FORCE_MODE:
            field = SEL_LED_FORCE_RATEf;
            break;
        default:
            return RT_ERR_INPUT;
            break;
    }

    if ((ret = reg_field_read(LED_BLINK_RATE_CFGr, field, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }
    switch(data)
    {
        case APOLLO_LED_BLINKRATE_32MS:
            *pBlinkRate = LED_BLINKRATE_32MS;
            break;
        case APOLLO_LED_BLINKRATE_64MS:
            *pBlinkRate = LED_BLINKRATE_64MS;
            break;
        case APOLLO_LED_BLINKRATE_128MS:
            *pBlinkRate = LED_BLINKRATE_128MS;
            break;
        case APOLLO_LED_BLINKRATE_256MS:
            *pBlinkRate = LED_BLINKRATE_256MS;
            break;
        case APOLLO_LED_BLINKRATE_512MS:
            *pBlinkRate = LED_BLINKRATE_512MS;
            break;
        case APOLLO_LED_BLINKRATE_1024MS:
            *pBlinkRate = LED_BLINKRATE_1024MS;
            break;
        case APOLLO_LED_BLINKRATE_48MS:
            *pBlinkRate = LED_BLINKRATE_48MS;
            break;
        case APOLLO_LED_BLINKRATE_96MS:
            *pBlinkRate = LED_BLINKRATE_96MS;
            break;
        default:
            return RT_ERR_FAILED;
            break;        
    }
    return RT_ERR_OK;
} /* end of dal_apollo_led_blinkRate_get */

/* Function Name:
 *      dal_apollo_led_blinkRate_set
 * Description:
 *      Get LED blinking rate
 * Input:
 *      group      - led blinking group
 *      blinkRate  - blinking rate.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      There are 8 types of LED blinking rates at 32ms, 48ms, 64ms, 96ms, 128ms, 256ms, 512ms, and 1024ms.
 */
int32 
dal_apollo_led_blinkRate_set(rtk_led_blinkGroup_t group, rtk_led_blink_rate_t blinkRate)
{
    int32   ret;
    uint32  data;
    uint32  field;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_LED),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(led_init);

    /* parameter check */
    switch(group)
    {
        case LED_BLINK_GROUP_PORT:
            field = SEL_MAC_LED_RATEf;
            break;
        case LED_BLINK_GROUP_USB:
            field = SEL_USB_LED_RATEf;
            break;
        case LED_BLINK_GROUP_SATA:
            field = SEL_SATA_LED_RATEf;
            break;
        case LED_BLINK_GROUP_FORCE_MODE:
            field = SEL_LED_FORCE_RATEf;
            break;
        default:
            return RT_ERR_INPUT;
            break;
    }

    switch(blinkRate)
    {
        case LED_BLINKRATE_32MS:
            data = APOLLO_LED_BLINKRATE_32MS;
            break;
        case LED_BLINKRATE_64MS:
            data = APOLLO_LED_BLINKRATE_64MS;
            break;
        case LED_BLINKRATE_128MS:
            data = APOLLO_LED_BLINKRATE_128MS;
            break;
        case LED_BLINKRATE_256MS:
            data = APOLLO_LED_BLINKRATE_256MS;
            break;
        case LED_BLINKRATE_512MS:
            data = APOLLO_LED_BLINKRATE_512MS;
            break;
        case LED_BLINKRATE_1024MS:
            data = APOLLO_LED_BLINKRATE_1024MS;
            break;
        case LED_BLINKRATE_48MS:
            data = APOLLO_LED_BLINKRATE_48MS;
            break;
        case LED_BLINKRATE_96MS:
            data = APOLLO_LED_BLINKRATE_96MS;
            break;
        default:
            return RT_ERR_INPUT;
            break;        
    }

    if ((ret = reg_field_write(LED_BLINK_RATE_CFGr, field, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of dal_apollo_led_blinkRate_set */


static int32 _dal_apollo_led_modeEnable_set(uint32 ledIdx, uint32 field, rtk_enable_t enable)
{
    int32   ret;
    uint32  data;
    
    if(ENABLED == enable)    
        data = APOLLO_LED_ENABLED;    
    else
        data = APOLLO_LED_DISABLED;    
    
    if ((ret = reg_array_field_write(DATA_LED_CFGr, REG_ARRAY_INDEX_NONE, ledIdx, field, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }        
    return RT_ERR_OK;
}


static int32 _dal_apollo_led_modeEnable_get(uint32 ledIdx, uint32 field, rtk_enable_t *enable)
{
    int32   ret;
    uint32  data;
    
    if ((ret = reg_array_field_read(DATA_LED_CFGr, REG_ARRAY_INDEX_NONE, ledIdx, field, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }        
    if( APOLLO_LED_DISABLED == data)    
        *enable = DISABLED;    
    else
        *enable = ENABLED;    

    return RT_ERR_OK;
}

static int32 _dal_apollo_led_ponSocConfig_set(uint32 ledIdx, rtk_led_type_t type, rtk_led_config_t *pConfig)
{
    int32   ret;
    uint32  field;
    uint32  isSoC = 0;
    uint32  i;
    
    switch(type)
    {
        case LED_TYPE_PON:
            break;
        case LED_TYPE_USB0:
        case LED_TYPE_USB1:
        case LED_TYPE_SATA:
            isSoC = 1;
            break;
        default:        
            return RT_ERR_INPUT;    
            break;        
    }     
    
    /*error input check*/   
    /*SoC only support LED_CONFIG_SOC_LINK_ACK and LED_CONFIG_FORCE_MODE*/
    /*PON only support LED_CONFIG_PON_LINK and LED_CONFIG_FORCE_MODE*/
    for(i=0;i<LED_CONFIG_END;i++)
    {
        /*skip check force LED*/
        if(LED_CONFIG_FORCE_MODE== i)
            continue;
        /*PON check*/
        if(LED_CONFIG_PON_LINK == i && LED_TYPE_PON == type)
            continue;
        /*SoC check*/
        if(LED_CONFIG_SOC_LINK_ACK == i && isSoC)
            continue;
        if(ENABLED == pConfig->ledEnable[i])    
           return RT_ERR_INPUT;   
    }
   
    /*set force mode*/
    field = CPU_FORCE_MODf;
    if ((ret = _dal_apollo_led_modeEnable_set(ledIdx,field, pConfig->ledEnable[LED_CONFIG_FORCE_MODE])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    } 
    /*set PON link mode*/
    if(LED_TYPE_PON == type)
    {
        field = UTP_SPD1000f;
        if ((ret = _dal_apollo_led_modeEnable_set(ledIdx,field, pConfig->ledEnable[LED_CONFIG_PON_LINK])) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_LED|MOD_DAL), "");
            return ret;
        }         
    }
    /*SoC link Act mode*/
    if(isSoC)
    {
        field = UTP_SPD1000f;
        if ((ret = _dal_apollo_led_modeEnable_set(ledIdx,field, pConfig->ledEnable[LED_CONFIG_SOC_LINK_ACK])) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_LED|MOD_DAL), "");
            return ret;
        }         
    }    
    
    return RT_ERR_OK;    
}


static int32 _dal_apollo_led_ponSocConfig_get(uint32 ledIdx, rtk_led_type_t type, rtk_led_config_t *pConfig)
{
    int32   ret;
    uint32  field;
    uint32  isSoC = 0;
    
    switch(type)
    {
        case LED_TYPE_PON:
            break;
        case LED_TYPE_USB0:
        case LED_TYPE_USB1:
        case LED_TYPE_SATA:
            isSoC = 1;
            break;
        default:        
            return RT_ERR_INPUT;    
            break;        
    }     
    
    /*set force mode*/
    field = CPU_FORCE_MODf;
    if ((ret = _dal_apollo_led_modeEnable_get(ledIdx,field, &pConfig->ledEnable[LED_CONFIG_FORCE_MODE])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    } 
    /*set PON link mode*/
    if(LED_TYPE_PON == type)
    {
        field = UTP_SPD1000f;
        if ((ret = _dal_apollo_led_modeEnable_get(ledIdx,field, &pConfig->ledEnable[LED_CONFIG_PON_LINK])) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_LED|MOD_DAL), "");
            return ret;
        }         
    }
    /*SoC link Act mode*/
    if(isSoC)
    {
        field = UTP_SPD1000f;
        if ((ret = _dal_apollo_led_modeEnable_get(ledIdx,field, &pConfig->ledEnable[LED_CONFIG_SOC_LINK_ACK])) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_LED|MOD_DAL), "");
            return ret;
        }         
    }    
    
    return RT_ERR_OK;    
}



static int32 _dal_apollo_led_portConfig_set(uint32 ledIdx, rtk_led_type_t type, rtk_led_config_t *pConfig)
{
    int32   ret;
    uint32  field;

    switch(type)
    {
        case LED_TYPE_UTP0:
        case LED_TYPE_UTP1:
        case LED_TYPE_UTP2:
        case LED_TYPE_UTP3:
        case LED_TYPE_UTP4:
        case LED_TYPE_UTP5:
        case LED_TYPE_FIBER:
            break;
        default:        
            return RT_ERR_INPUT;    
            break;        
    }     
    
    /*error check*/
    if(ENABLED == pConfig->ledEnable[LED_CONFIG_PON_LINK])    
        return RT_ERR_INPUT;   
    if(ENABLED == pConfig->ledEnable[LED_CONFIG_SOC_LINK_ACK])    
        return RT_ERR_INPUT;   
    if(ENABLED == pConfig->ledEnable[LED_CONFIG_SPD500ACT])    
        return RT_ERR_INPUT;   
    if(ENABLED == pConfig->ledEnable[LED_CONFIG_SPD500])    
        return RT_ERR_INPUT;   
    if(LED_TYPE_FIBER==type)
    {
        if(ENABLED == pConfig->ledEnable[LED_CONFIG_SPD10ACT])    
            return RT_ERR_INPUT;   
        if(ENABLED == pConfig->ledEnable[LED_CONFIG_SPD10])    
            return RT_ERR_INPUT;   
        if(ENABLED == pConfig->ledEnable[LED_CONFIG_COL])    
            return RT_ERR_INPUT;   
        if(ENABLED == pConfig->ledEnable[LED_CONFIG_DUP])    
            return RT_ERR_INPUT;   
    }

    
    /*set force mode*/
    field = CPU_FORCE_MODf;
    if ((ret = _dal_apollo_led_modeEnable_set(ledIdx, field, pConfig->ledEnable[LED_CONFIG_FORCE_MODE])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    } 

    /*set speed 1000 mode*/
    field = UTP_SPD1000f;
    if ((ret = _dal_apollo_led_modeEnable_set(ledIdx, field, pConfig->ledEnable[LED_CONFIG_SPD1000])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    } 

    /*set speed 100 mode*/
    field = UTP_SPD100f;
    if ((ret = _dal_apollo_led_modeEnable_set(ledIdx, field, pConfig->ledEnable[LED_CONFIG_SPD100])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }

    /*set speed 10 mode*/
    field = UTP_SPD10f;
    if ((ret = _dal_apollo_led_modeEnable_set(ledIdx, field, pConfig->ledEnable[LED_CONFIG_SPD10])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }    

    /*set duplex mode*/
    field = UTP_DUPf;
    if ((ret = _dal_apollo_led_modeEnable_set(ledIdx, field, pConfig->ledEnable[LED_CONFIG_DUP])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }

    /*set speed 1000 Act mode*/
    field = UTP_SPD1000_ACTf;
    if ((ret = _dal_apollo_led_modeEnable_set(ledIdx, field, pConfig->ledEnable[LED_CONFIG_SPD1000ACT])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    } 

    /*set speed 100 Act mode*/
    field = UTP_SPD100_ACTf;
    if ((ret = _dal_apollo_led_modeEnable_set(ledIdx, field, pConfig->ledEnable[LED_CONFIG_SPD100ACT])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }

    /*set speed 10 Act mode*/
    field = UTP_SPD10_ACTf;
    if ((ret = _dal_apollo_led_modeEnable_set(ledIdx, field, pConfig->ledEnable[LED_CONFIG_SPD10ACT])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }    

    /*set Rx Act mode*/
    field = UTP_RX_ACTf;
    if ((ret = _dal_apollo_led_modeEnable_set(ledIdx, field, pConfig->ledEnable[LED_CONFIG_RX_ACT])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }   

    /*set Tx Act mode*/
    field = UTP_TX_ACTf;
    if ((ret = _dal_apollo_led_modeEnable_set(ledIdx, field, pConfig->ledEnable[LED_CONFIG_TX_ACT])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    } 

    /*set Collision mode*/
    field = UTP_COLf;
    if ((ret = _dal_apollo_led_modeEnable_set(ledIdx, field, pConfig->ledEnable[LED_CONFIG_COL])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;    
}




static int32 _dal_apollo_led_portConfig_get(uint32 ledIdx, rtk_led_type_t type, rtk_led_config_t *pConfig)
{
    int32   ret;
    uint32  field;

    switch(type)
    {
        case LED_TYPE_UTP0:
        case LED_TYPE_UTP1:
        case LED_TYPE_UTP2:
        case LED_TYPE_UTP3:
        case LED_TYPE_UTP4:
        case LED_TYPE_UTP5:
        case LED_TYPE_FIBER:
            break;
        default:        
            return RT_ERR_INPUT;    
            break;        
    }     
    
   
    /*set force mode*/
    field = CPU_FORCE_MODf;
    if ((ret = _dal_apollo_led_modeEnable_get(ledIdx, field, &pConfig->ledEnable[LED_CONFIG_FORCE_MODE])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    } 

    /*set speed 1000 mode*/
    field = UTP_SPD1000f;
    if ((ret = _dal_apollo_led_modeEnable_get(ledIdx, field, &pConfig->ledEnable[LED_CONFIG_SPD1000])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    } 

    /*set speed 100 mode*/
    field = UTP_SPD100f;
    if ((ret = _dal_apollo_led_modeEnable_get(ledIdx, field, &pConfig->ledEnable[LED_CONFIG_SPD100])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }

    /*set speed 10 mode*/
    field = UTP_SPD10f;
    if ((ret = _dal_apollo_led_modeEnable_get(ledIdx, field, &pConfig->ledEnable[LED_CONFIG_SPD10])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }    

    /*set duplex mode*/
    field = UTP_DUPf;
    if ((ret = _dal_apollo_led_modeEnable_get(ledIdx, field, &pConfig->ledEnable[LED_CONFIG_DUP])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }

    /*set speed 1000 Act mode*/
    field = UTP_SPD1000_ACTf;
    if ((ret = _dal_apollo_led_modeEnable_get(ledIdx, field, &pConfig->ledEnable[LED_CONFIG_SPD1000ACT])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    } 

    /*set speed 100 Act mode*/
    field = UTP_SPD100_ACTf;
    if ((ret = _dal_apollo_led_modeEnable_get(ledIdx, field, &pConfig->ledEnable[LED_CONFIG_SPD100ACT])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }

    /*set speed 10 Act mode*/
    field = UTP_SPD10_ACTf;
    if ((ret = _dal_apollo_led_modeEnable_get(ledIdx, field, &pConfig->ledEnable[LED_CONFIG_SPD10ACT])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }    

    /*set Rx Act mode*/
    field = UTP_RX_ACTf;
    if ((ret = _dal_apollo_led_modeEnable_get(ledIdx, field, &pConfig->ledEnable[LED_CONFIG_RX_ACT])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }   

    /*set Tx Act mode*/
    field = UTP_TX_ACTf;
    if ((ret = _dal_apollo_led_modeEnable_get(ledIdx, field, &pConfig->ledEnable[LED_CONFIG_TX_ACT])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    } 

    /*set Collision mode*/
    field = UTP_COLf;
    if ((ret = _dal_apollo_led_modeEnable_get(ledIdx, field, &pConfig->ledEnable[LED_CONFIG_COL])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;    
}


/* Function Name:
 *      dal_apollo_led_Config_set
 * Description:
 *      Set per group Led to congiuration mode
 * Input:
 *      ledIdx  - LED index id.
 *      type    - LED type
 *      config  - LED configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 */
int32 
dal_apollo_led_config_set(uint32 ledIdx, rtk_led_type_t type, rtk_led_config_t *pConfig)
{
    int32   ret;
    uint32 ledMode;
    uint32 isPortMode = 0;
    uint32 isSoCPonMode = 0;
    int i;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_LED),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(led_init);

    /* parameter check */
    RT_PARAM_CHK((APOLLO_LED_INDEX_MAX < ledIdx), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pConfig), RT_ERR_NULL_POINTER);
    
    
    for(i = 0; i<LED_CONFIG_END ; i++)
    {
        if(pConfig->ledEnable[i] >= RTK_ENABLE_END)    
            return RT_ERR_INPUT;
    }
    
    
    switch(type)
    {
        case LED_TYPE_UTP0:
            ledMode = APOLLO_LED_TYPE_UTP0;
            isPortMode = 1;
            break;

        case LED_TYPE_UTP1:
            ledMode = APOLLO_LED_TYPE_UTP1;
            isPortMode = 1;
            break;

        case LED_TYPE_UTP2:
            ledMode = APOLLO_LED_TYPE_UTP2;
            isPortMode = 1;
            break;

        case LED_TYPE_UTP3:
            ledMode = APOLLO_LED_TYPE_UTP3;
            isPortMode = 1;
            break;
        case LED_TYPE_UTP4:
            ledMode = APOLLO_LED_TYPE_UTP4;
            isPortMode = 1;
            break;
        case LED_TYPE_UTP5:
            ledMode = APOLLO_LED_TYPE_UTP5;
            isPortMode = 1;
            break;
        case LED_TYPE_FIBER:
            ledMode = APOLLO_LED_TYPE_FIBER;
            isPortMode = 1;
            break;
        case LED_TYPE_PON:
            ledMode = APOLLO_LED_TYPE_PON;
            isSoCPonMode = 1;
            break;
        case LED_TYPE_USB0:
            ledMode = APOLLO_LED_TYPE_USB0;
            isSoCPonMode = 1;
            break;

        case LED_TYPE_USB1:
            ledMode = APOLLO_LED_TYPE_USB1;
            isSoCPonMode = 1;
            break;

        case LED_TYPE_SATA:
            ledMode = APOLLO_LED_TYPE_SATA;
            isSoCPonMode = 1;
            break;
        default:
            return RT_ERR_INPUT;    
            break;
    }

    if ((ret = reg_array_field_write(DATA_LED_CFGr, REG_ARRAY_INDEX_NONE, ledIdx, LED_CFGf, &ledMode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_QOS|MOD_DAL), "");
        return ret;
    }

    if(isSoCPonMode)
    {
        if ((ret = _dal_apollo_led_ponSocConfig_set(ledIdx, type, pConfig)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_LED|MOD_DAL), "");
            return ret;
        }
    }
    if(isPortMode)
    {
        if ((ret = _dal_apollo_led_portConfig_set(ledIdx, type, pConfig)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_LED|MOD_DAL), "");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of dal_apollo_led_config_set */

/* Function Name:
 *      dal_apollo_led_groupConfig_set
 * Description:
 *      Set per group Led to congiuration mode
 * Input:
 *      ledIdx  - LED index id.
 * Output:
 *      pType    - LED type
 *      pConfig  - LED configuration
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 */
int32 
dal_apollo_led_config_get(uint32 ledIdx, rtk_led_type_t *pType, rtk_led_config_t *pConfig)
{
    int32   ret;
    uint32 ledMode;
    uint32 isPortMode = 0;
    uint32 isSoCPonMode = 0;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_LED),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(led_init);

    /* parameter check */
    RT_PARAM_CHK((APOLLO_LED_INDEX_MAX < ledIdx), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pType), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pConfig), RT_ERR_NULL_POINTER);

    if ((ret = reg_array_field_read(DATA_LED_CFGr, REG_ARRAY_INDEX_NONE, ledIdx, LED_CFGf, &ledMode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_QOS|MOD_DAL), "");
        return ret;
    }


    switch(ledMode)
    {
        case APOLLO_LED_TYPE_UTP0:
            *pType = LED_TYPE_UTP0;
            isPortMode = 1;
            break;

        case APOLLO_LED_TYPE_UTP1:
            *pType = LED_TYPE_UTP1;
            isPortMode = 1;
            break;

        case APOLLO_LED_TYPE_UTP2:
            *pType = LED_TYPE_UTP2;
            isPortMode = 1;
            break;

        case APOLLO_LED_TYPE_UTP3:
            *pType = LED_TYPE_UTP3;
            isPortMode = 1;
            break;
        case APOLLO_LED_TYPE_UTP4:
            *pType = LED_TYPE_UTP4;
            isPortMode = 1;
            break;
        case APOLLO_LED_TYPE_UTP5:
            *pType = LED_TYPE_UTP5;
            isPortMode = 1;
            break;
        case APOLLO_LED_TYPE_FIBER:
            *pType = LED_TYPE_FIBER;
            isPortMode = 1;
            break;
        case APOLLO_LED_TYPE_PON:
            *pType = LED_TYPE_PON;
            isSoCPonMode = 1;
            break;
        case APOLLO_LED_TYPE_USB0:
            *pType = LED_TYPE_USB0;
            isSoCPonMode = 1;
            break;

        case APOLLO_LED_TYPE_USB1:
            *pType = LED_TYPE_USB1;
            isSoCPonMode = 1;
            break;

        case APOLLO_LED_TYPE_SATA:
            *pType = LED_TYPE_SATA;
            isSoCPonMode = 1;
            break;
        default:
            return RT_ERR_INPUT;    
            break;
    }



    if(isSoCPonMode)
    {
        if ((ret = _dal_apollo_led_ponSocConfig_get(ledIdx, *pType, pConfig)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_LED|MOD_DAL), "");
            return ret;
        }
    }
    if(isPortMode)
    {
        if ((ret = _dal_apollo_led_portConfig_get(ledIdx, *pType, pConfig)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_LED|MOD_DAL), "");
            return ret;
        }
    }
    
    return RT_ERR_OK;
} /* end of dal_apollo_led_config_get */

/* Function Name:
 *      dal_apollo_led_modeForce_get
 * Description:
 *      Get Led group to congiuration force mode
 * Input:
 *      ledIdx - LED index id.
 * Output:
 *      pMode  - LED force mode.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API can get forced Led group mode.
 *      The force modes that can be set are as following:
 *      - LED_FORCE_BLINK,
 *      - LED_FORCE_OFF,
 *      - LED_FORCE_ON.
 *      For LED_OP_SERIAL the max led index is 31  
 *      For LED_OP_PARALLEL the max led index is 16  
 */
int32
dal_apollo_led_modeForce_get(uint32 ledIdx, rtk_led_force_mode_t *pMode)
{
    int32   ret;
    uint32  data;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_LED),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(led_init);

    /* parameter check */
    RT_PARAM_CHK((APOLLO_LED_INDEX_MAX < ledIdx), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);
    
    if ((ret = reg_array_field_read(LED_FORCE_VALUE_CFGr, REG_ARRAY_INDEX_NONE, ledIdx, SEL_LED_FORCE_VALUEf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }       

    switch(data)
    {
        case APOLLO_LED_FORCE_OFF:
            *pMode = LED_FORCE_OFF;
            break;
        case APOLLO_LED_FORCE_ON:
            *pMode = LED_FORCE_ON;
            break;
        case APOLLO_LED_FORCE_BLINK:
            *pMode = LED_FORCE_BLINK;
            break;
        default:
            return RT_ERR_FAILED;
            break;        
        
    }
    return RT_ERR_OK;
} /* end of dal_apollo_led_modeForce_get */

/* Function Name:
 *      dal_apollo_led_modeForce_set
 * Description:
 *      Set Led group to congiuration force mode
 * Input:
 *      ledIdx - LED index id.
 *      mode  - LED force mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API can get forced Led group mode.
 *      The force modes that can be set are as following:
 *      - LED_FORCE_NORMAL,
 *      - LED_FORCE_BLINK,
 *      - LED_FORCE_OFF,
 *      - LED_FORCE_ON.
 *      For LED_OP_SERIAL the max led index is 31  
 *      For LED_OP_PARALLEL the max led index is 16  
 */
int32
dal_apollo_led_modeForce_set(uint32 ledIdx, rtk_led_force_mode_t mode)
{
    int32   ret;
    uint32  data;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_LED),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(led_init);

    /* parameter check */
    RT_PARAM_CHK((APOLLO_LED_INDEX_MAX < ledIdx), RT_ERR_INPUT);
    switch(mode)
    {
        case LED_FORCE_OFF:
            data = APOLLO_LED_FORCE_OFF;
            break;
        case LED_FORCE_ON:
            data = APOLLO_LED_FORCE_ON;
            break;
        case LED_FORCE_BLINK:
            data = APOLLO_LED_FORCE_BLINK;
            break;
        default:
            return RT_ERR_INPUT;
            break;        
    }

    if ((ret = reg_array_field_write(LED_FORCE_VALUE_CFGr, REG_ARRAY_INDEX_NONE, ledIdx, SEL_LED_FORCE_VALUEf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_LED|MOD_DAL), "");
        return ret;
    }   
    return RT_ERR_OK;
} /* end of dal_apollo_led_modeForce_set */



/* Function Name:
 *      dal_apollo_led_parallelEnable_get
 * Description:
 *      Get Led group enable status for parallel mode
 * Input:
 *      ledIdx - LED index id.
 * Output:
 *      pState  - LED parallel enable status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 */
int32
dal_apollo_led_parallelEnable_get(uint32 ledIdx, rtk_enable_t *pState)
{
    int32 ret;
    uint32 data;
   
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_LED), "ledIdx=%d",ledIdx);

    /* check Init status */
    RT_INIT_CHK(led_init);

    /* parameter check */
    RT_PARAM_CHK((APOLLO_PARALLEL_LED_MAX < ledIdx), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pState), RT_ERR_NULL_POINTER);

    /* function body */
    if((ret = reg_field_read(LED_ENr, LED_PARA_P04_ENf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_LED), "");
        return ret;
    }      
    
    if(data & (0x01 << ledIdx))
        *pState = ENABLED;
    else
        *pState = DISABLED;
    
    return RT_ERR_OK;
}   /* end of dal_apollo_led_parallelEnable_get */

/* Function Name:
 *      dal_apollo_led_parallelEnable_set
 * Description:
 *      Set Led group enable status for parallel mode
 * Input:
 *      ledIdx - LED index id.
 *      state  - LED parallel enable status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 */
int32
dal_apollo_led_parallelEnable_set(uint32 ledIdx, rtk_enable_t state)
{
    int32 ret;
    uint32 data;
    uint32 ledIoPin;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_LED), "ledIdx=%d,state=%d",ledIdx, state);

    /* check Init status */
    RT_INIT_CHK(led_init);

    /* parameter check */
    RT_PARAM_CHK((APOLLO_PARALLEL_LED_MAX < ledIdx), RT_ERR_INPUT);
    RT_PARAM_CHK((RTK_ENABLE_END <= state), RT_ERR_INPUT);
    
    switch(ledIdx)
    {
        case 0:
            ledIoPin = APOLLO_LED_IO_LED0;
            break;    
        case 1:
            ledIoPin = APOLLO_LED_IO_LED1;
            break;    
        case 2:
            ledIoPin = APOLLO_LED_IO_LED2;
            break;    
        case 3:
            ledIoPin = APOLLO_LED_IO_LED3;
            break;    
        case 4:
            ledIoPin = APOLLO_LED_IO_LED4;
            break;    
        case 5:
            ledIoPin = APOLLO_LED_IO_LED5;
            break;    
        case 6:
            ledIoPin = APOLLO_LED_IO_LED6;
            break;    
        case 7:
            ledIoPin = APOLLO_LED_IO_LED7;
            break;    
        case 8:
            ledIoPin = APOLLO_LED_IO_LED8;
            break;    
        case 9:
            ledIoPin = APOLLO_LED_IO_LED9;
            break;    
        case 10:
            ledIoPin = APOLLO_LED_IO_LED10;
            break;    
        case 11:
            ledIoPin = APOLLO_LED_IO_LED11;
            break;    
        case 12:
            ledIoPin = APOLLO_LED_IO_LED12;
            break;    
        case 13:
            ledIoPin = APOLLO_LED_IO_LED13;
            break;    
        case 14:
            ledIoPin = APOLLO_LED_IO_LED14;
            break;    
        case 15:
            ledIoPin = APOLLO_LED_IO_LED15;
            break;    
        case 16:
            ledIoPin = APOLLO_LED_IO_LED16;
            break;    
        default:
            return RT_ERR_INPUT;
    }
    
    
    /* function body */
    if(ENABLED==state)
    {
        /*set LED_PARA_P04_EN */   
        if((ret = reg_field_read(LED_ENr, LED_PARA_P04_ENf, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_LED), "");
            return ret;
        }            
        data |= (0x01 << ledIdx);
        if((ret = reg_field_write(LED_ENr, LED_PARA_P04_ENf, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_LED), "");
            return ret;
        }

        /*set IO_LED_ENr*/
        if((ret = reg_read(IO_LED_ENr, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_LED), "");
            return ret;
        }            
        data |= (0x01 << ledIoPin);
        if((ret = reg_write(IO_LED_ENr, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_LED), "");
            return ret;
        }
    }
    else
    {
        /*set LED_PARA_P04_EN */   
        if((ret = reg_field_read(LED_ENr, LED_PARA_P04_ENf, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_LED), "");
            return ret;
        }            
        data &= ~(0x01 << ledIdx);
        if((ret = reg_field_write(LED_ENr, LED_PARA_P04_ENf, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_LED), "");
            return ret;
        }

        /*set IO_LED_ENr*/
        if((ret = reg_read(IO_LED_ENr, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_LED), "");
            return ret;
        }            
        data &= ~(0x01 << ledIoPin);
        if((ret = reg_write(IO_LED_ENr, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_LED), "");
            return ret;
        }        
    }    
    return RT_ERR_OK;
}   /* end of dal_apollo_led_parallelEnable_set */

