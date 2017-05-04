#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include "led-generic.h"
#include "pushbutton.h"
#include "gpio.h"

/**********************************************************************
NOTICE:
Because GPIO is not enough, GPIO_A_7 is shared by WPS and WIFI on-off button
and GPIO_B_7 is used to be WPS green LED. There is no GPIO_D pins on v6 board.
***********************************************************************/


#define LOW_ACTIVE	1
#if LOW_ACTIVE

#define GPIO_SET(w, op)  do { \
	gpioConfig(w, GPIO_FUNC_OUTPUT); \
	if (LED_ON==op) gpioClear(w); \
	else gpioSet(w); \
} while (0);

#define GPIO_READ(w) (!gpioRead(w))
		
#else

#define GPIO_SET(w, op)  do { \
	gpioConfig(w, GPIO_FUNC_OUTPUT); \
	if (LED_ON==op) gpioSet(w); \
	else gpioClear(w); \
} while (0);

#define GPIO_READ(w) (gpioRead(w))

#endif

#ifdef FWUPGRADE_LED
//jiunming, for telefonica, Recoverable Error during Firmware upgrade
extern unsigned char fwupgradeerrstate;
unsigned char flag_led_power_green;
unsigned char flag_led_power_red;
unsigned char flag_led_dsl;
unsigned char flag_led_internet_green;
unsigned char flag_led_internet_red;
unsigned char flag_led_wps_green;
unsigned char flag_led_wps_red;
unsigned char flag_led_wps_yellow;
#endif

static void board_03_v6_handle_set(int which, int op) {
	//printk("%s: led %d op %d\n", __FUNCTION__, which, op);
	
	switch (which) {
	case LED_POWER_GREEN:
		GPIO_SET(GPIO_B_6, op);
		break;
/*		
	case LED_POWER_RED:
		GPIO_SET(GPIO_B_7, op);
		break;
*/		
	case LED_DSL:		
		GPIO_SET(GPIO_A_5, op);
		break;	
	case LED_PPP_GREEN:
		GPIO_SET(GPIO_B_2, op);
		break;
	case LED_PPP_RED:
		GPIO_SET(GPIO_B_5, op);
		break;		
	case LED_WPS_GREEN:
		GPIO_SET(GPIO_B_7, op);		
		break;
/*
	case LED_WPS_GREEN:
		GPIO_SET(GPIO_D_3, op);		
		break;
	case LED_WPS_RED:
		GPIO_SET(GPIO_D_4, op);		
		break;
	case LED_WPS_YELLOW:
		GPIO_SET(GPIO_D_6, op);		
		break;
*/		
		
	default:
		led_handle_set(which, op);
	}
}

static void board_03_v6_handle_init(void) {
	board_03_v6_handle_set(LED_POWER_GREEN, LED_OFF);
	board_03_v6_handle_set(LED_POWER_RED, LED_ON);
	board_03_v6_handle_set(LED_DSL, LED_OFF);
	board_03_v6_handle_set(LED_INTERNET_GREEN, LED_OFF);
	board_03_v6_handle_set(LED_INTERNET_RED, LED_OFF);
};

static struct led_operations board_03_v6_operation = {
	.name = "board_03_v6",
	.handle_init = board_03_v6_handle_init,
	.handle_set = board_03_v6_handle_set,
	//.handle_flash = board_03_v6_handle_flash,
};


static void board_03_v6_pb_init(void) {
};

static int board_03_v6_pb_is_pushed(int which) {
	switch(which) {
		
	case PB_RESET:
		//printk("PB: %d\n", GPIO_READ(GPIO_A_6));
		return GPIO_READ(GPIO_A_6);
	case PB_WIFISW:
		//return	GPIO_READ(GPIO_D_2);
	case PB_WPS:
		return GPIO_READ(GPIO_A_7);
	}
	return 0;
}

static struct pushbutton_operations board_03_v6_pb_op = {
	.handle_init = board_03_v6_pb_init,
	.handle_is_pushed = board_03_v6_pb_is_pushed,
};

static int __init board_03_v6_led_init(void) {
#ifndef CONFIG_JTAG_USABLE
	unsigned int sicr;

	sicr = REG32(BSP_MISC_PINMUX);	
	if ( sicr & BSP_JTAG_GPIO_PINMUX ) {
		printk( "<<<<<<<disable GPIO JTAG function.\n" );
		REG32(BSP_MISC_PINMUX) = sicr & ~BSP_JTAG_GPIO_PINMUX;
	}
#endif

	led_register_operations(&board_03_v6_operation);
	pb_register_operations(&board_03_v6_pb_op);
	return 0;
}

static void __exit board_03_v6_led_exit(void) {
}


module_init(board_03_v6_led_init);
module_exit(board_03_v6_led_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPIO driver for Reload default");




