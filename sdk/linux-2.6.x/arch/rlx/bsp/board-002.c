#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/semaphore.h>

#include "led-generic.h"
#include "gpio.h"


#define LED_SET(w, op) if (LED_ON==op) spio_set(w); else spio_clear(w)

/*
#define SPIO_POWER_GREEN            0
#define SPIO_POWER_RED                1
#define SPIO_PPP_GREEN                  2
#define SPIO_PPP_RED              3               
#define SPIO_DSL_LINK             4 
#define SPIO_WPS_G                 5
#define SPIO_WPS_R                 6
#define SPIO_WPS_Y                 7
*/

#define LED_CLK     GPIO_B_6
#define LED_DATA  GPIO_B_7

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

static unsigned char spio_register = 0;
static struct semaphore spio_sem;


static void spio_refresh(void) {
	int idx;
	down_interruptible(&spio_sem);
	gpioConfig(LED_DATA, GPIO_FUNC_OUTPUT);
	gpioConfig(LED_CLK, GPIO_FUNC_OUTPUT);

	for (idx=7; idx >= 0; idx++) {
		if (spio_register & (1 << idx))
			gpioClear(LED_DATA);	
		else
			gpioSet(LED_DATA);	

		gpioClear(LED_CLK);  
		gpioSet(LED_CLK);
	}
	up(&spio_sem);
}


static void spio_set(int which) {
	spio_register |= (1 << which);
	spio_refresh();
}

static void spio_clear(int which) {
	spio_register &= ~(1 << which);
	spio_refresh();
}

//static struct led_flash led_flash_dsl;
//static struct led_flash led_internet_act;

static void e8b_handle_set(int which, int op) {
	//printk("%s: led %d op %d\n", __FUNCTION__, which, op);
	
	switch (which) {
	case LED_POWER_GREEN:
		LED_SET(0, op);
		break;
	case LED_POWER_RED:
		LED_SET(1, op);
		break;
	case LED_PPP_GREEN:		
		LED_SET(2, op);
		break;
	case LED_PPP_RED:		
		LED_SET(3, op);
		break;
	case LED_DSL:		
		LED_SET(4, op);
		break;
	case LED_WPS_GREEN:		
		LED_SET(5, op);
		break;
	case LED_WPS_RED:
		LED_SET(6, op);
		break;
	case LED_WPS_YELLOW:		
		LED_SET(7, op);
		break;
	
	
	
	default:
		led_handle_set(which, op);
	}
}

static void e8b_handle_init(void) {
	e8b_handle_set(LED_POWER_GREEN, LED_OFF);
	e8b_handle_set(LED_POWER_RED, LED_ON);
	e8b_handle_set(LED_DSL, LED_OFF);
	e8b_handle_set(LED_INTERNET_GREEN, LED_OFF);
	e8b_handle_set(LED_INTERNET_RED, LED_OFF);
};

static struct led_operations e8b_operation = {
	.name = "led-e8b",
	.handle_init = e8b_handle_init,
	.handle_set = e8b_handle_set,
	//.handle_flash = combo_handle_flash,
};

static int __init e8b_led_init(void) {
#ifndef CONFIG_JTAG_USABLE
	unsigned int sicr;

	sicr = REG32(BSP_MISC_PINMUX);	
	if ( sicr & BSP_JTAG_GPIO_PINMUX ) {
		printk( "<<<<<<<disable GPIO JTAG function.\n" );
		REG32(BSP_MISC_PINMUX) = sicr & ~BSP_JTAG_GPIO_PINMUX;
	}
#endif

	led_register_operations(&e8b_operation);
	init_MUTEX(&spio_sem);
	
	return 0;
}

static void __exit e8b_led_exit(void) {
}


module_init(e8b_led_init);
module_exit(e8b_led_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPIO driver for Reload default");



