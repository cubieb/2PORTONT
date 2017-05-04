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

static void combo_handle_set(int which, int op) {
	//printk("%s: led %d op %d\n", __FUNCTION__, which, op);
	
	switch (which) {
	case LED_POWER_GREEN:
		GPIO_SET(GPIO_B_6, op);
		break;
	case LED_POWER_RED:
		GPIO_SET(GPIO_B_7, op);
		break;
	case LED_DSL:		
		GPIO_SET(GPIO_A_5, op);
		break;
	case LED_INTERNET_GREEN:
		GPIO_SET(GPIO_B_2, op);
		break;
	case LED_INTERNET_RED:
		GPIO_SET(GPIO_B_5, op);
		break;
	default:
		led_handle_set(which, op);
	}
}

static void combo_handle_init(void) {
	combo_handle_set(LED_POWER_GREEN, LED_OFF);
	combo_handle_set(LED_POWER_RED, LED_ON);
	combo_handle_set(LED_DSL, LED_OFF);
	combo_handle_set(LED_INTERNET_GREEN, LED_OFF);
	combo_handle_set(LED_INTERNET_RED, LED_OFF);
};

static struct led_operations combo_operation = {
	.name = "combo",
	.handle_init = combo_handle_init,
	.handle_set = combo_handle_set,
	//.handle_flash = combo_handle_flash,
};


static void combo_pb_init(void) {
};

static int combo_pb_is_pushed(int which) {
	switch(which) {
		
	case PB_RESET:
		//printk("PB: %d\n", GPIO_READ(GPIO_A_6));
		return GPIO_READ(GPIO_A_6);
	}
	return 0;
}

static struct pushbutton_operations combo_pb_op = {
	.handle_init = combo_pb_init,
	.handle_is_pushed = combo_pb_is_pushed,
};

static int __init combo_led_init(void) {
#ifndef CONFIG_JTAG_USABLE
	unsigned int sicr;

	sicr = REG32(BSP_MISC_PINMUX);	
	if ( sicr & BSP_JTAG_GPIO_PINMUX ) {
		printk( "<<<<<<<disable GPIO JTAG function.\n" );
		REG32(BSP_MISC_PINMUX) = sicr & ~BSP_JTAG_GPIO_PINMUX;
	}
#endif

	led_register_operations(&combo_operation);
	pb_register_operations(&combo_pb_op);
	return 0;
}

static void __exit combo_led_exit(void) {
}


module_init(combo_led_init);
module_exit(combo_led_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPIO driver for Reload default");



