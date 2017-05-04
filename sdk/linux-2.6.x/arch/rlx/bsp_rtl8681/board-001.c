#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include "led-generic.h"
#include "pushbutton.h"
#include "gpio.h"
//#include <asm/mach-realtek/rtl8672/platform.h>
/*linux-2.6.19*/ 
#include <bspchip.h>

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

#if LOW_ACTIVE
#define DefaultGPIODatValue ((1<<GPIO_B_6 | 1<<GPIO_D_3 | 1<<GPIO_D_0 | 1<<GPIO_D_4	\
        	| 1<<GPIO_B_5 | 1<<GPIO_D_1 | 1<<GPIO_D_5 | 1<<GPIO_D_2))
#else
#define DefaultGPIODatValue 0
#endif

void PCIE_reset_pin(int *reset){
	*reset = GPIO_A_5;
}

static void board_05_handle_set(int which, int op) {
	//printk("%s: led %d op %d\n", __FUNCTION__, which, op);
	
	switch (which) {

#if 0
	case LED_POWER_GREEN:
		GPIO_SET(GPIO_B_6, op);
		break;
	case LED_POWER_RED:
		GPIO_SET(GPIO_D_3, op);
		break;
	case LED_DSL:	
		GPIO_SET(GPIO_D_0, op);  
		break;
	// Mason Yu
	case LED_INTERNET_GREEN:
		GPIO_SET(GPIO_D_4, op); 
		break;
	case LED_INTERNET_RED:
		GPIO_SET(GPIO_B_5, op);
		break;		
	case LED_PPP_GREEN:
		GPIO_SET(GPIO_D_4, op); 
		break;
	case LED_PPP_RED:
		GPIO_SET(GPIO_B_5, op);
		break;		
	case LED_WPS_GREEN:
		GPIO_SET(GPIO_D_1, op); 
		break;
	#ifdef CONFIG_SW_USB_LED1 // 2010-11-19 paula
	case LED_USB_1: 
		GPIO_SET(GPIO_D_5, op); 
		break;
	#endif
	//case LED_WPS_RED: // 2010-11-19 paula
	//	GPIO_SET(GPIO_D_5, op); 
	//	break;
	case LED_WPS_YELLOW:
		GPIO_SET(GPIO_D_2, op); 
		break;
#endif		
		
	default:
		led_handle_set(which, op);
	}
}

static void board_05_handle_init(void) {
	board_05_handle_set(LED_POWER_GREEN, LED_OFF);
	board_05_handle_set(LED_POWER_RED, LED_ON);
	board_05_handle_set(LED_DSL, LED_OFF);
	board_05_handle_set(LED_INTERNET_GREEN, LED_OFF);
	board_05_handle_set(LED_INTERNET_RED, LED_OFF);
	board_05_handle_set(LED_WPS_GREEN, LED_OFF);
	board_05_handle_set(LED_WPS_RED, LED_OFF);
	board_05_handle_set(LED_WPS_YELLOW, LED_OFF);
};

static struct led_operations board_05_operation = {
	.name = "board_05",
	.handle_init = board_05_handle_init,
	.handle_set = board_05_handle_set,
	//.handle_flash = board_05_handle_flash,
};


static void board_05_pb_init(void) {
};

static int board_05_pb_is_pushed(int which) {
	switch(which) {
#if 0		
	case PB_RESET:
		if( IS_RLE0315 || IS_6166 ) {
			return GPIO_READ(GPIO_B_2);   
		}
	case PB_WIFISW:
		if( IS_RLE0315 || IS_6166  ) {
			return GPIO_READ(GPIO_A_7);  
		}
	case PB_WPS:
		if( IS_RLE0315 || IS_6166  ) {
			return GPIO_READ(GPIO_B_7); 
		}
#endif
	}
	return 0;
}

static struct pushbutton_operations board_05_pb_op = {
	.handle_init = board_05_pb_init,
	.handle_is_pushed = board_05_pb_is_pushed,
};

static int __init board_05_led_init(void) {
#ifndef CONFIG_JTAG_USABLE
	unsigned int sicr;

	sicr = REG32(BSP_MISC_PINMUX);	
	if ( sicr & BSP_JTAG_GPIO_PINMUX ) {
		printk( "<<<<<<<disable GPIO JTAG function.\n" );
		//REG32(BSP_MISC_PINMUX) = sicr & ~BSP_JTAG_GPIO_PINMUX;
	}
#endif

	if( IS_RLE0315 || IS_6166 ) {
        REG32(BSP_MISC_PINOCR) |= 0x003c0000; //enable GPIO D2, D3, D4, D5
        REG32(BSP_MISC_PINOCR) &= ~((1<<22)|(1<<23));   //enable GPIO D0, D1
	}
	led_register_operations(&board_05_operation);
	pb_register_operations(&board_05_pb_op);
	return 0;
}

static void __exit board_05_led_exit(void) {
}


module_init(board_05_led_init);
module_exit(board_05_led_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPIO driver for Reload default");



