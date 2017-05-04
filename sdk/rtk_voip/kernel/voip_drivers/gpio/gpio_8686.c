/*
* Copyright c                  Realtek Semiconductor Corporation, 2012  
* All rights reserved.
* 
* Program : GPIO Driver
* Abstract : 
* Author : 
*/

#include "gpio.h"

extern void gpioConfig (int gpio_num, int gpio_func);
extern void gpioSet(int gpio_num);
extern void gpioClear(int gpio_num);
extern int gpioRead(int gpio_num) ;

int gpio_debug = 0;
extern int spi_dbg;
// Goal : unifiy GPIO functions
//static uint32 _getGpio( enum GPIO_FUNC func, enum GPIO_PORT port, uint32 pin )
static uint32 _getGpio( unsigned char port, uint32 pin )
{
	return 0;
}
//static void _setGpio( enum GPIO_FUNC func, enum GPIO_PORT port, uint32 pin, uint32 data )
static void _setGpio( unsigned char port, uint32 pin, uint32 data )
{
	return 0;
}
//int32 _rtl8686_initGpioPin( uint32 gpioId, enum GPIO_DIRECTION direction, 
//                                           enum GPIO_INTERRUPT_TYPE interruptEnable )
int32 _rtl8686_initGpioPin(uint32 gpioId, unsigned char func)
{
    if(func == 0)
        gpioConfig(gpioId, 0x01);
    else
        gpioConfig(gpioId, 0x02);
}

int32 _rtl8686_getGpioDataBit( uint32 gpioId, uint32* pData )
{
//	unsigned int val;
//	if (gpioId >7)	return 0; //skip non-GPA

//	val = *(volatile int*)GPABCDDATA;

//	*pData = (val >> gpioId) & 0x1 ;
	*pData = gpioRead(gpioId);

}

int32 _rtl8686_setGpioDataBit( uint32 gpioId, uint32 data )
{

    if( data == 1)
        gpioSet(gpioId);
    else
        gpioClear(gpioId);

	return 0;
}


