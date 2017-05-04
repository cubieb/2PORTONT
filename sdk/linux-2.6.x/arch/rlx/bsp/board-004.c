#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include "led-generic.h"
#include "pushbutton.h"
#include "gpio.h"

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

static void iad_handle_set(int which, int op) {
	//printk("%s: led %d op %d\n", __FUNCTION__, which, op);
	
	switch (which) {
	case LED_POWER_GREEN:
		GPIO_SET(GPIO_B_6, op);
		break;
	case LED_POWER_RED:
		GPIO_SET(GPIO_B_7, op);
		break;
	case LED_DSL:		
		GPIO_SET(GPIO_D_7, op);
		break;
	case LED_INTERNET_GREEN:
		GPIO_SET(GPIO_B_2, op);
		break;
	case LED_INTERNET_RED:
		GPIO_SET(GPIO_B_5, op);
		break;
	case LED_PPP_GREEN:
		GPIO_SET(GPIO_B_2, op);
		break;
	case LED_PPP_RED:
		GPIO_SET(GPIO_B_5, op);
		break;		
	case LED_WPS_GREEN:
		GPIO_SET(GPIO_C_7, op);		
		break;
	case LED_WPS_RED:
		GPIO_SET(GPIO_B_3, op);		
		break;
	case LED_WPS_YELLOW:
		GPIO_SET(GPIO_D_4, op);		
		break;
#ifdef CONFIG_RTK_VOIP
	case LED_FXS1:
		GPIO_SET(GPIO_D_0, op);		
		break;
	case LED_FXS2:
		GPIO_SET(GPIO_D_3, op);		
		break;
#endif	
	default:
		led_handle_set(which, op);
	}
}

static void iad_handle_init(void) {
	iad_handle_set(LED_POWER_GREEN, LED_OFF);
	iad_handle_set(LED_POWER_RED, LED_ON);
	iad_handle_set(LED_DSL, LED_OFF);
	iad_handle_set(LED_INTERNET_GREEN, LED_OFF);
	iad_handle_set(LED_INTERNET_RED, LED_OFF);
#ifdef CONFIG_RTK_VOIP
	iad_handle_set(LED_FXS1, LED_OFF);
	iad_handle_set(LED_FXS2, LED_OFF);
#endif

};

static struct led_operations iad_operation = {
	.name = "iad",
	.handle_init = iad_handle_init,
	.handle_set = iad_handle_set,
	//.handle_flash = iad_handle_flash,
};


static void iad_pb_init(void) {
};

static int iad_pb_is_pushed(int which) {
	switch(which) {
		
	case PB_RESET:
		//printk("PB: %d\n", GPIO_READ(GPIO_A_6));
		return GPIO_READ(GPIO_A_6);
	case PB_WPS:
		return GPIO_READ(GPIO_A_7) ;
	case PB_WIFISW:
		return GPIO_READ(GPIO_D_5);
	}
	return 0;
}

static struct pushbutton_operations iad_pb_op = {
	.handle_init = iad_pb_init,
	.handle_is_pushed = iad_pb_is_pushed,
};

static int __init iad_led_init(void) {
#ifndef CONFIG_JTAG_USABLE
	unsigned int sicr;

	sicr = REG32(BSP_MISC_PINMUX);	
	if ( sicr & BSP_JTAG_GPIO_PINMUX ) {
		printk( "<<<<<<<disable GPIO JTAG function.\n" );
		REG32(BSP_MISC_PINMUX) = sicr & ~BSP_JTAG_GPIO_PINMUX;
	}
#endif

	led_register_operations(&iad_operation);
	pb_register_operations(&iad_pb_op);
	return 0;
}

static void __exit iad_led_exit(void) {
}


module_init(iad_led_init);
module_exit(iad_led_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPIO driver for Reload default");



