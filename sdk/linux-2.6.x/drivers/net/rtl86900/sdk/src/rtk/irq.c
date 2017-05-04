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
 *
 *
 * $Revision: 58276 $
 * $Date: 2015-05-05 18:09:50 +0800 (Tue, 05 May 2015) $
 *
 * Purpose : IRQ API
 *
 * Feature : Provide the APIs to register/deregister IRQ
 *
 */

/*
 * Include Files
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>

#include <bspchip.h>
#if defined(OLD_FPGA_DEFINED)
#include <gpio.h>
#else
#include <bspchip_8686.h>
#endif

#include <common/rt_type.h>
#include <common/rt_error.h>
#include <osal/print.h>
#include <dal/apollo/dal_apollo.h>
#include <dal/apollomp/dal_apollomp.h>
#include <rtk/irq.h>
#include <rtk/gpon.h>
#include <rtk/gpio.h>

#include <dal/apollo/gpon/gpon_platform.h>
#include <rtk/intr.h>

/*
 * Symbol Definition
 */

#if defined(OLD_FPGA_DEFINED)
#define APOLLO_IRQ 16 /* TBD */
#elif defined(FPGA_DEFINED)
#define APOLLO_IRQ BSP_GPIO1_IRQ
#else
#define APOLLO_IRQ BSP_SWITCH_IRQ
#endif

#define GPIO_SET1_IRQ BSP_GPIO_ABCD_IRQ
#define GPIO_SET2_IRQ BSP_GPIO_EFGH_IRQ

/*
 * Data Declaration
 */
static struct net_device switch_dev,gpio_dev,gpio2_dev;
static struct tasklet_struct switch_tasklets,gpio_tasklets,gpio2_tasklets;
static int32 irq_init = {INIT_NOT_COMPLETED};

const int32 gpioIRQId[2]={GPIO_SET1_IRQ,GPIO_SET2_IRQ};

#define GPIO_IMR_NUM 64

static rtk_irq_data_t irq_isr[INTR_TYPE_END-1];

static rtk_irq_data_t gpio_isr[GPIO_IMR_NUM];

static void (*dyinggasp_th_isr)(void);
/*
 * Macro Definition
 */
#define RTL_W32(reg, value)			(*(volatile u32*)(reg)) = (u32)value
#define RTL_R32(reg)				(*(volatile u32*)(reg))

/*
 * Function Declaration
 */


static uint32
irq_clear(int imrId){

	uint32 ret;

	RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_GPON),"\nInterrupt: %d, no isr register or imr is not enabled!\n",imrId);
	if((ret=rtk_intr_ims_clear(imrId)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_GPON | MOD_DAL), "");
	   	return ret;
	}
	return RT_ERR_OK;
}




static void
switch_interrupt_bh(uint32 data)
{

	rtk_enable_t status;
	int32 i,ret;

	RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_GPON),"switch_interrupt_bh 0x%x", data);

	/*loop for check all of interrupt type & isr, explict INTR_TYPE_ALL*/
	for(i=0; i < (INTR_TYPE_END - 1) ; i++)
	{
		/*get status of interrupt type*/
		if((ret=rtk_intr_ims_get(i,&status)) != RT_ERR_OK)
		{
            if(ret!=RT_ERR_INPUT)
            {
                RT_ERR(ret, (MOD_INTR | MOD_DAL), "type=%d",i);
            }
           	continue;
		}
		/*if status enabled and isr is registered*/
		if(status && (irq_isr[i].isr != NULL))
		{
			irq_isr[i].isr();
		}else
		if(status){
			irq_clear(i);
		}
	}

	/* restore all of interrupt */
	if((ret=rtk_intr_imr_restore(data)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_INTR | MOD_DAL), "");
	    return ;
	}

	return;
}



static irqreturn_t
switch_interrupt_th(int irq, void *dev_instance)
{
    uint32 data;
	int32  ret ;
    rtk_enable_t status;

#if defined(OLD_FPGA_DEFINED)
    /* disable interrupt */
    data = 0x3 << (GPIO_B_2*2);
    REG32(GPIO_PAB_IMR) = REG32(GPIO_PAB_IMR)&(~data);

    REG32(0xB8003510) = 0x00000400;      /*  reg.PABCD_ISR : write 1 to clear PORT B pin 2 */

    tasklet_hi_schedule(&switch_tasklets);

    /* enable interrupt */
    data = 0x1 << (GPIO_B_2*2);
    REG32(GPIO_PAB_IMR) = REG32(GPIO_PAB_IMR)|(data);

#else

	/*get current interrupt register*/
    if((ret=rtk_intr_imr_get(INTR_TYPE_ALL,&data)) != RT_ERR_OK)
    {
    	RT_ERR(ret, (MOD_INTR | MOD_DAL), "");
	return IRQ_RETVAL(IRQ_HANDLED);
    }

    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_GPON),"switch_interrupt_bh 0x%x", data);
    switch_tasklets.data = data;

	/* disable all of interrupt */
	if((ret=rtk_intr_imr_set(INTR_TYPE_ALL,DISABLED)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_INTR | MOD_DAL), "");
		return IRQ_RETVAL(IRQ_HANDLED);

	}

    /*get status of dying gasp*/
    if((ret=rtk_intr_ims_get(INTR_TYPE_DYING_GASP,&status)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_INTR | MOD_DAL), "");
        return IRQ_RETVAL(IRQ_HANDLED);
    }
    /*if status enabled and dyinggasp_th_handler registered*/
    if(status && (dyinggasp_th_isr != NULL))
    {
        dyinggasp_th_isr();
    }

    tasklet_hi_schedule(&switch_tasklets);
#endif

    return IRQ_RETVAL(IRQ_HANDLED);
}


static int32
rtk_switch_irq_init(uint32 irq_id)
{
#if defined(OLD_FPGA_DEFINED)
    uint32 data;
#endif
	int32  ret;

	/*find irq_id is user define or from header file*/
	if(irq_id == IRQ_ID_NO_CARE){
		irq_id = APOLLO_IRQ;
	}
	RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_GPON),"%s: irq_id = %d\n",__FUNCTION__,irq_id);

	memset(&switch_tasklets, 0, sizeof(struct tasklet_struct));
	switch_tasklets.func=(void (*)(unsigned long))switch_interrupt_bh;
	switch_tasklets.data=(unsigned long)NULL;

    /* switch interrupt clear all mask */
	if((ret=rtk_intr_imr_set(INTR_TYPE_ALL,DISABLED)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_INTR | MOD_DAL), "");
        return ret;
	}

    /* clear switch interrupt state */
	if((ret=rtk_intr_ims_clear(INTR_TYPE_ALL)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_INTR | MOD_DAL), "");
        return ret;
	}

#if defined(OLD_FPGA_DEFINED)
	/* switch interrupt polarity set to low */
	if((ret=rtk_intr_polarity_set(INTR_POLAR_LOW)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_INTR | MOD_DAL), "");
        return ret;
	}
#else
    /* switch interrupt polarity set to high */
	if((ret=rtk_intr_polarity_set(INTR_POLAR_HIGH)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_INTR | MOD_DAL), "");
		return ret;
	}
#endif

    /* IRQ register */
    osal_memset(&switch_dev, 0x0, sizeof(switch_dev));
    switch_dev.irq = irq_id;
    osal_strcpy(switch_dev.name,"apl_sw");

    if(request_irq(switch_dev.irq, (irq_handler_t)switch_interrupt_th, IRQF_DISABLED, switch_dev.name, &switch_dev))
    {
     	RT_ERR(ret, (MOD_INTR | MOD_DAL), "");
        return RT_ERR_FAILED;
    }

#if defined(OLD_FPGA_DEFINED)
    /* 利用 GPIO_B_2 (I/O) 做interrupt (以RTL8672 4P+WIFI 為例，這個PIN拉到reset button) */
    /*  reg.PABCD_DIR     : set direction configuration of PORT B pin 2 to b'0
                            (b'0:input pin , b'1:output pin)   */
    gpioConfig(GPIO_B_2,GPIO_FUNC_INPUT);
    /*  reg.PAB_IMR       : set interrupt mode of PORT B pin 2 to  b'01
                            (b'00=Disable interrupt
                             b'01=Enable falling edge interrupt
                             b'11=Enable both falling or rising edge interrupt) */
    data = 0x1 << (GPIO_B_2*2);
    REG32(GPIO_PAB_IMR) = REG32(GPIO_PAB_IMR)|(data);
#endif

	/*initial isr*/
	if((ret=rtk_irq_isr_unregister(INTR_TYPE_ALL)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_INTR | MOD_DAL), "");
		return ret;
	}

    return RT_ERR_OK;
}


static int32
rtk_switch_irq_exit(void)
{
	int32 ret;
    /* IRQ de-register */
    free_irq(switch_dev.irq, &switch_dev);
	/*initial isr*/
	if((ret=rtk_irq_isr_unregister(INTR_TYPE_ALL)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_INTR | MOD_DAL), "");
		return ret;
	}

    return RT_ERR_OK;
}


static void
gpio_interrupt_bh(uint32 data)
{
	rtk_enable_t status;
	int32 i;

	RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_GPON),"%s",__FUNCTION__);

	/*loop for check all of interrupt type & isr*/
	for(i=0; i < 32 ; i++)
	{

		status = (data >> i) & 0x1;

		/*if status enabled and isr is registered*/
		if(status && (gpio_isr[i].isr != NULL))
		{
			gpio_isr[i].isr();
		}
	}

	return;
}

static void
gpio_interrupt2_bh(uint32 data)
{
	rtk_enable_t status;
	int32 i;

	RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_GPON),"%s",__FUNCTION__);

	/*loop for check all of interrupt type & isr*/
	for(i=0; i < 32 ; i++)
	{
		status = (data >> i) & 0x1;

		/*if status enabled and isr is registered*/
		if(status && (gpio_isr[i+32].isr != NULL))
		{
			gpio_isr[i+32].isr();
		}
	}
	return;
}





static irqreturn_t
gpio_interrupt_th(int irq, void *dev_instance)
{
	int32  ret,gpioId;
	uint32 value;
	struct tasklet_struct *pTasklet;
	RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_GPON),"%s",__FUNCTION__);


	gpioId 	 = (irq==GPIO_SET1_IRQ) ? RTK_GPIO_INTR_SET1 : RTK_GPIO_INTR_SET2;
	pTasklet = (irq==GPIO_SET1_IRQ) ? &gpio_tasklets : &gpio2_tasklets;

	if((ret=rtk_gpio_intrStatus_get(gpioId,&value)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_INTR | MOD_DAL), "");
        return ret;
	}
	pTasklet->data = value;

	if((ret=rtk_gpio_intrStatus_clean(gpioId)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_INTR | MOD_DAL), "");
        return ret;
	}

	tasklet_hi_schedule(pTasklet);

    return IRQ_RETVAL(IRQ_HANDLED);
}



static int32
rtk_gpio_irq_init(void)
{
	int32  ret;

	/*find irq_id is user define or from header file*/
	RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_GPON),"%s: irq_id = %d and %d\n",__FUNCTION__,GPIO_SET1_IRQ,GPIO_SET2_IRQ);


    /* gpio interrupt default disable all */
	if((ret=rtk_gpio_intr_set(RTK_GPIO_INTR_ALL,GPIO_INTR_DISABLE)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_INTR | MOD_DAL), "");
        return ret;
	}

	/* clean all gpio interrupt status*/
	if((ret=rtk_gpio_intrStatus_clean(RTK_GPIO_INTR_ALL)) != RT_ERR_OK)
	{
		 RT_ERR(ret, (MOD_INTR | MOD_DAL), "");
		 return ret;
	}
	/* IRQ register */
	osal_memset(&gpio_tasklets, 0, sizeof(struct tasklet_struct));
	gpio_tasklets.func=(void (*)(unsigned long))gpio_interrupt_bh;
	gpio_tasklets.data=(unsigned long)NULL;

	/* IRQ register */
	osal_memset(&gpio2_tasklets, 0, sizeof(struct tasklet_struct));
	gpio2_tasklets.func=(void (*)(unsigned long))gpio_interrupt2_bh;
	gpio2_tasklets.data=(unsigned long)NULL;

	/*register interruot irq */
	osal_memset(&gpio_dev, 0x0, sizeof(gpio_dev));
	gpio_dev.irq = GPIO_SET1_IRQ;

	osal_strcpy(gpio_dev.name,"apl_gpio");
	if(request_irq(gpio_dev.irq, (irq_handler_t)gpio_interrupt_th, IRQF_DISABLED, gpio_dev.name, &gpio_dev))
	{
	 	RT_ERR(ret, (MOD_INTR | MOD_DAL), "");
	    return RT_ERR_FAILED;
	}
	osal_memset(&gpio2_dev,0,sizeof(gpio2_dev));
	gpio2_dev.irq = GPIO_SET2_IRQ;
	osal_strcpy(gpio2_dev.name,"apl_gpio2");
	if(request_irq(gpio2_dev.irq, (irq_handler_t)gpio_interrupt_th, IRQF_DISABLED, gpio2_dev.name, &gpio2_dev))
	{
		RT_ERR(ret, (MOD_INTR | MOD_DAL), "");
		return RT_ERR_FAILED;
	}

	/*initial isr*/
	if((ret=rtk_irq_gpioISR_unregister(RTK_GPIO_INTR_ALL)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_INTR | MOD_DAL), "");
		return ret;
	}

	return RT_ERR_OK;
}

static int32
rtk_gpio_irq_exit(void)
{
	int32 ret,i;
    /* IRQ de-register */
	for(i=0;i<2;i++)
	{
    	free_irq(gpio_dev.irq, &gpio_dev);
	}
	/*initial isr*/
	if((ret=rtk_irq_gpioISR_unregister(GPIO_IMR_NUM)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_INTR | MOD_DAL), "");
		return ret;
	}

    return RT_ERR_OK;
}



/* Function Name:
 *      rtk_irq_isr_register
 * Description:
 *      Register isr handler
 * Input:
 *      intr            - interrupt type
 *      fun            - function pointer of isr hander
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32
rtk_irq_isr_register(rtk_intr_type_t intr, void (*fun)(void))
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(irq_init);

	/* parameter check */
	RT_PARAM_CHK((NULL == fun), RT_ERR_NULL_POINTER);
	RT_PARAM_CHK((INTR_TYPE_END <= intr), RT_ERR_OUT_OF_RANGE);

	/* hook function point to isr_mapper*/
	irq_isr[intr].isr = fun;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_irq_isr_unregister
 * Description:
 *      Register isr handler
 * Input:
 *      intr            - interrupt type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32
rtk_irq_isr_unregister(rtk_intr_type_t intr){

	int32   i;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(irq_init);

	/* parameter check */
	RT_PARAM_CHK((INTR_TYPE_END <= intr), RT_ERR_OUT_OF_RANGE);

	/* un-hook function point to isr_mapper*/
	if(INTR_TYPE_ALL == intr)
	{
		for(i = 0; i < INTR_TYPE_ALL; i++)
        {
			irq_isr[i].isr = NULL;
        }
	}
	else
	{
			irq_isr[intr].isr = NULL;
	}

    return RT_ERR_OK;

}



/* Function Name:
 *      rtk_irq_gpioISR_register
 * Description:
 *      Register isr handler
 * Input:
 *      gpioId       - gpio id for interrupt ISR
 *      fun            - function pointer of isr hander
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32
rtk_irq_gpioISR_register(uint32 gpioId, void (*fun)(void))
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(irq_init);

	/* parameter check */
	RT_PARAM_CHK((NULL == fun), RT_ERR_NULL_POINTER);
	RT_PARAM_CHK((GPIO_IMR_NUM < gpioId), RT_ERR_OUT_OF_RANGE);

	/* hook function point to isr_mapper*/
	gpio_isr[gpioId].isr = fun;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_irq_gpioISR_unregister
 * Description:
 *      Register isr handler
 * Input:
 *      gpioId   - gpio id for interrupt ISR
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32
rtk_irq_gpioISR_unregister(uint32 gpioId){

	int32   i;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(irq_init);

	/* parameter check */
	RT_PARAM_CHK(((GPIO_IMR_NUM) < gpioId), RT_ERR_OUT_OF_RANGE);

	/* un-hook function point to isr_mapper*/
	if(RTK_GPIO_INTR_ALL == gpioId)
	{
		for(i = 0; i < GPIO_IMR_NUM; i++)
        {
			gpio_isr[i].isr = NULL;
        }
	}
	else
	{
		gpio_isr[gpioId].isr = NULL;
	}

    return RT_ERR_OK;

}

/* Function Name:
 *      rtk_irq_isrDgTh_register
 * Description:
 *      Register Dying gasp top half isr handler
 * Input:
 *      intr            - interrupt type
 *      fun            - function pointer of isr hander
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32
rtk_irq_isrDgTh_register(void (*fun)(void))
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(irq_init);

	/* parameter check */
	RT_PARAM_CHK((NULL == fun), RT_ERR_NULL_POINTER);

	/* hook function point to isr_mapper*/
	dyinggasp_th_isr = fun;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_irq_isrDgTh_unregister
 * Description:
 *      Register  Dying gasp top half isr handler
 * Input:
 *      intr            - interrupt type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32
rtk_irq_isrDgTh_unregister(void){


    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(irq_init);

	/* un-hook function point to isr_mapper*/
    dyinggasp_th_isr = NULL;

    return RT_ERR_OK;

}


/* Function Name:
 *      rtk_irq_init
 * Description:
 *      Register isr handler
 * Input:
 *      intr            - interrupt type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32
rtk_irq_init(void){

	int32	ret;

	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);
	irq_init  = INIT_COMPLETED;

	if((ret = rtk_switch_irq_init(IRQ_ID_NO_CARE))!=RT_ERR_OK)
	{
		irq_init = INIT_NOT_COMPLETED;
	}
#if !(defined (FPGA_DEFINED))
	if((ret = rtk_gpio_irq_init())!=RT_ERR_OK)
	{
		irq_init = INIT_NOT_COMPLETED;
	}
#endif
	return ret;
}


/* Function Name:
 *      rtk_irq_exit
 * Description:
 *      Register isr handler
 * Input:
 *      intr            - interrupt type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32
rtk_irq_exit(void){

	int32	ret;

	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);
	if( INIT_COMPLETED == irq_init)
    {
		ret = rtk_switch_irq_exit();
		ret = rtk_gpio_irq_exit();
		irq_init = INIT_NOT_COMPLETED;
	}
	return RT_ERR_OK;
}



