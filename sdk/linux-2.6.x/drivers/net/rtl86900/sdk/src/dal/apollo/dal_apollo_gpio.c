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
 * Purpose : Definition of GPIO API
 *
 * Feature : Provide the APIs to enable and configure GPIO
 *
 */



/*
 * Include Files
 */
#include <dal/apollo/dal_apollo.h>
#include <rtk/gpio.h>
#include <dal/apollo/dal_apollo_gpio.h>


static uint32 gpio_check_supported(uint32 pin)
{
	switch(pin){
	case 0:
	case 6:
	case 8:
	case 9:
	case 10:
	case 11:
	case 15:
	case 16:
	case 17:
	case 18:
	case 31:
	case 27:
	case 29:
	case 28:
	case 25:
	case 26:
	case 33:
	case 32:
	case 30:
	case 37:
	case 38:
	case 34:
	case 35:
	case 36:
	case 39:
	case 40:
	case 58:
	case 59:
	case 62:
	case 63:
	case 64:
	case 65:
	case 66:
	case 67:
	case 68:
	case 69:
		return RT_ERR_OK;
	break;
	default:
		return RT_ERR_CHIP_NOT_SUPPORTED;
	break;
	}
}


#define RT_GPIO_CHK(pin)\
do {\
    if (gpio_check_supported(pin) != RT_ERR_OK) {\
	  osal_printf("pin %d not support!\n",pin); \
        return RT_ERR_CHIP_NOT_SUPPORTED;\
    }\
} while (0)

static uint32    gpio_init = INIT_NOT_COMPLETED;

/* Function Name:
 *      dal_apollo_gpio_init
 * Description:
 *      gpio init function
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *
 */
 extern int32 
dal_apollo_gpio_init(void )
{
      uint32 ret;
	int32 i;
	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_GPIO),"%s",__FUNCTION__);
	gpio_init = INIT_COMPLETED;		
 	for(i = 0 ; i < RTK_GPIO_NUM ; i ++)
      {
		if(gpio_check_supported(i)!=RT_ERR_OK){
			continue;
		}
	  
      		if((ret=dal_apollo_gpio_state_set(i,DISABLED)) != RT_ERR_OK)
      		{
      			RT_ERR(ret, (MOD_DAL|MOD_GPIO), "");	
		      gpio_init = INIT_NOT_COMPLETED;		
			return ret;
      		}
      }
	return RT_ERR_OK;

}



/* Function Name:
 *      dal_apollo_gpio_state_set
 * Description:
 *      enable or disable gpio function
 * Input:
 *      gpioId		- gpio id from 0~71
 *      enable		- enable or disable 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32 
dal_apollo_gpio_state_set(uint32 gpioId, rtk_enable_t enable)
{
    int32 ret;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_GPIO), "gpioId=%d,enable=%d",gpioId, enable);

    /* check Init status */
    RT_INIT_CHK(gpio_init);

    /* parameter check */
    RT_PARAM_CHK((RTK_GPIO_NUM <= gpioId), RT_ERR_INPUT);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    RT_GPIO_CHK(gpioId);
    /* function body */
   if ((ret=reg_array_field_write(GPIO_CTRL_2r, REG_ARRAY_INDEX_NONE, gpioId, EN_GPIOf,&enable)) != RT_ERR_OK)
   {
   	RT_ERR(ret, (MOD_DAL|MOD_GPIO), "");	
	return ret;
   }

    return RT_ERR_OK;
}   /* end of dal_apollo_gpio_state_set */



/* Function Name:
 *      dal_apollo_gpio_state_get
 * Description:
 *      enable or disable gpio function
 * Input:
 *      gpioId		- gpio id from 0~71
 *      enable		- point for get gpio enable or disable 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32 
dal_apollo_gpio_state_get(uint32 gpioId, rtk_enable_t *enable)
{
    int32 ret;
    uint32 value;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_GPIO), "gpioId=%d,enable=%d",gpioId, enable);

    /* check Init status */
    RT_INIT_CHK(gpio_init);

    /* parameter check */
    RT_PARAM_CHK((RTK_GPIO_NUM <= gpioId), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == enable), RT_ERR_INPUT);
    RT_GPIO_CHK(gpioId);

    /* function body */
   if ((ret=reg_array_field_read(GPIO_CTRL_2r, REG_ARRAY_INDEX_NONE, gpioId, EN_GPIOf,&value)) != RT_ERR_OK)
   {
   	RT_ERR(ret, (MOD_DAL|MOD_GPIO), "");	
	return ret;
   }

    *enable = value;
    return RT_ERR_OK;
}   /* end of dal_apollo_gpio_state_get */


/* Function Name:
 *      dal_apollo_gpio_mode_set
 * Description:
 *     set gpio to input or output mode
 * Input:
 *      gpioId 		-gpio id from 0 to 71
 *	  mode		-gpio mode, input or output mode	
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32 
dal_apollo_gpio_mode_set(uint32 gpioId, rtk_gpio_mode_t mode)
{

    int32 ret;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_GPIO), "gpioId=%d,mode=%d",gpioId, mode);

    /* check Init status */
    RT_INIT_CHK(gpio_init);

    /* parameter check */
    RT_PARAM_CHK((RTK_GPIO_NUM <=gpioId), RT_ERR_INPUT);
    RT_PARAM_CHK((GPIO_MODE_END <=mode), RT_ERR_INPUT);
    RT_GPIO_CHK(gpioId);

    /* function body */
   if ((ret=reg_array_field_write(GPIO_CTRL_4r, REG_ARRAY_INDEX_NONE, gpioId, SEL_GPIOf,&mode)) != RT_ERR_OK)
   {
   	RT_ERR(ret, (MOD_DAL|MOD_GPIO), "");	
	return ret;
   }

    return RT_ERR_OK;
}   /* end of dal_apollo_gpio_mode_set */


/* Function Name:
 *      dal_apollo_gpio_mode_get
 * Description:
 *     get gpio to input or output mode
 * Input:
 *      gpioId 		-gpio id from 0 to 71
 *	  mode		-point for get gpio mode	
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32 
dal_apollo_gpio_mode_get(uint32 gpioId, rtk_gpio_mode_t *mode)
{

    int32 ret;
    uint32 value;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_GPIO), "gpioId=%d,mode=%d",gpioId, mode);

    /* check Init status */
    RT_INIT_CHK(gpio_init);

    /* parameter check */
    RT_PARAM_CHK((RTK_GPIO_NUM <=gpioId), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == mode), RT_ERR_NULL_POINTER);
    RT_GPIO_CHK(gpioId);

    /* function body */
   if ((ret=reg_array_field_read(GPIO_CTRL_4r, REG_ARRAY_INDEX_NONE, gpioId, SEL_GPIOf,&value)) != RT_ERR_OK)
   {
   	RT_ERR(ret, (MOD_DAL|MOD_GPIO), "");	
	return ret;
   }

   *mode = value;

    return RT_ERR_OK;
}   /* end of dal_apollo_gpio_mode_get */




/* Function Name:
 *      dal_apollo_gpio_databit_get
 * Description:
 *     read gpio data
 * Input:
 *      gpioId 		-gpio id from 0 to 71
 *	  data		-point for read data from gpio	
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32 
dal_apollo_gpio_databit_get(uint32 gpioId, uint32 *data)
{
    int32 ret;
    uint32 value;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_GPIO), "gpioId=%d",gpioId);

    /* check Init status */
    RT_INIT_CHK(gpio_init);

    /* parameter check */
    RT_PARAM_CHK((RTK_GPIO_NUM <=gpioId), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == data), RT_ERR_NULL_POINTER);
    RT_GPIO_CHK(gpioId);

    /*check gpio is enable*/
    if((ret=dal_apollo_gpio_state_get(gpioId,&value)) != RT_ERR_OK)
    {
 	RT_ERR(ret, (MOD_DAL|MOD_GPIO), "");	
	return ret;
    }
    if(value != ENABLED)
    {
      ret  = RT_ERR_NOT_ALLOWED;
    	RT_ERR(ret, (MOD_DAL|MOD_GPIO), "");	
	return ret;
    }
    /*check gpio is input mode*/
    if((ret=dal_apollo_gpio_mode_get(gpioId,&value)) != RT_ERR_OK)
    {
 	RT_ERR(ret, (MOD_DAL|MOD_GPIO), "");	
	return ret;
    }
	
    if(value!=GPIO_INPUT)
    {
      ret = RT_ERR_NOT_ALLOWED;
    	RT_ERR(ret, (MOD_DAL|MOD_GPIO), "");	
	return ret;
    }	

    /* function body */
   if ((ret=reg_array_field_read(GPIO_CTRL_1r, REG_ARRAY_INDEX_NONE, gpioId, STS_GPIOf,&value)) != RT_ERR_OK)
   {
   	RT_ERR(ret, (MOD_DAL|MOD_GPIO), "");	
	return ret;
   }

   *data = value;

    return RT_ERR_OK;
}   /* end of dal_apollo_gpio_databit_get */


/* Function Name:
 *      dal_apollo_gpio_databit_set
 * Description:
 *     write data to gpio
 * Input:
 *      gpioId 		-gpio id from 0 to 71
 *	  data		-write data to gpio	
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32 
dal_apollo_gpio_databit_set(uint32 gpioId, uint32 data)
{

    int32 ret;
    uint32 value;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_GPIO), "gpioId=%d",gpioId);

    /* check Init status */
    RT_INIT_CHK(gpio_init);

    /* parameter check */
    RT_PARAM_CHK((RTK_GPIO_NUM <=gpioId), RT_ERR_INPUT);
    RT_PARAM_CHK((2 <=data), RT_ERR_INPUT);
    RT_GPIO_CHK(gpioId);


    /*check gpio is enable*/
    if((ret=dal_apollo_gpio_state_get(gpioId,&value)) != RT_ERR_OK)
    {
 	RT_ERR(ret, (MOD_DAL|MOD_GPIO), "");	
	return ret;
    }
    if(value != ENABLED)
    {
      ret  = RT_ERR_NOT_ALLOWED;
    	RT_ERR(ret, (MOD_DAL|MOD_GPIO), "");	
	return ret;
    }

     /*check gpio is input mode*/
    if((ret=dal_apollo_gpio_mode_get(gpioId,&value)) != RT_ERR_OK)
    {
 	RT_ERR(ret, (MOD_DAL|MOD_GPIO), "");	
	return ret;
    }
	
    if(value!=GPIO_OUTPUT)
    {
      ret = RT_ERR_NOT_ALLOWED;
    	RT_ERR(ret, (MOD_DAL|MOD_GPIO), "");	
	return ret;
    }	
	
    /* function body */
   if ((ret=reg_array_field_write(GPIO_CTRL_0r, REG_ARRAY_INDEX_NONE, gpioId, CTRL_GPIOf,&data)) != RT_ERR_OK)
   {
   	RT_ERR(ret, (MOD_DAL|MOD_GPIO), "");	
	return ret;
   }


    return RT_ERR_OK;
}   /* end of dal_apollo_gpio_databit_set */



/* Function Name:
 *      dal_apollo_gpio_intr_set
 * Description:
 *     write data to gpio
 * Input:
 *      intrId: id 0 for gpio pin 63, id 1 for gpio pin 65
 *      state: point of state, enable or disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
extern int32 
dal_apollo_gpio_intr_set(uint32 intrId,rtk_enable_t state)
{
    int32 ret;
    uint32 value;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_GPIO), "state=%d",state);

    /* check Init status */
    RT_INIT_CHK(gpio_init);

    /* parameter check */
    RT_PARAM_CHK((RTK_ENABLE_END<=state), RT_ERR_INPUT);
    RT_PARAM_CHK((RTK_GPIO_INTR_NUM<=intrId), RT_ERR_INPUT);
 

    /* get current status first*/
   if ((ret=reg_field_read(IO_MODE_ENr, INTRPT_ENf,&value)) != RT_ERR_OK)
   {
   	RT_ERR(ret, (MOD_DAL|MOD_GPIO), "");	
	return ret;
   }

    if(intrId == 0 ){
   	value = state | (value & 0x2); 
   }else{
	value = (state << 1) | (value & 0x1); 
   }

    /* function body */
   if ((ret=reg_field_write(IO_MODE_ENr, INTRPT_ENf,&value)) != RT_ERR_OK)
   {
   	RT_ERR(ret, (MOD_DAL|MOD_GPIO), "");	
	return ret;
   }
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_apollo_gpio_intr_get
 * Description:
 *     write data to gpio
 * Input:
 *      intrId: id 0 for gpio pin 63, id 1 for gpio pin 65
 *      state: point of state, enable or disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
extern int32 
dal_apollo_gpio_intr_get(uint32 intrId,rtk_enable_t *state)
{
    int32 ret;
    uint32 value;

    /* check Init status */
    RT_INIT_CHK(gpio_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == state), RT_ERR_INPUT);
    RT_PARAM_CHK((RTK_GPIO_INTR_NUM<=intrId), RT_ERR_INPUT);

    
    /* function body */
   if ((ret=reg_field_read(IO_MODE_ENr, INTRPT_ENf,&value)) != RT_ERR_OK)
   {
   	RT_ERR(ret, (MOD_DAL|MOD_GPIO), "");	
	return ret;
   }

   if(intrId == 0 ){
   	*state = (value & 0x1); 
   }else{
	*state = ((value & 0x2) >> 1 );
   }
   
    return RT_ERR_OK;
}

