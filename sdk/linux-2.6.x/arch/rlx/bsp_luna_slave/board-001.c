#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include "led-generic.h"
#include "pushbutton.h"
#include "gpio.h"
//#include <asm/mach-realtek/rtl8672/platform.h>
/*linux-2.6.19*/ 
#include <bspchip.h>

//ccwei: 120208-WPS
#define WPS_PIN RTL8192CD_GPIO_2

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

void PCIE_reset_pin(int *reset){
	*reset = GPIO_62;
}
EXPORT_SYMBOL(PCIE_reset_pin);

#ifdef CONFIG_USE_PCIE_SLOT_1
void PCIE1_reset_pin(int *reset){
	*reset = GPIO_68;
}
EXPORT_SYMBOL(PCIE1_reset_pin);
#endif

//ccwei: 120208-WPS
#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE)

void get_wifi_wps_pin(int *gpio_num)
{
    *gpio_num = (WPS_PIN - RTL8192CD_GPIO_0);
}
#endif
//end
static void board_01_handle_set(int which, int op) {
	//printk("%s: led %d op %d\n", __FUNCTION__, which, op);
	
#if 0 /* 2012-4-12 krammer add */
	switch (which) {
#ifdef CONFIG_SW_USB_LED1
	case LED_USB_1:
		GPIO_SET(GPIO_F_6, op);
		break;
#endif //CONFIG_SW_USB_LED1
	case LED_DSL:	
		GPIO_SET(GPIO_H_3, op);
		break;
#ifdef CONFIG_SW_USB_LED0
	case LED_USB_0:
		GPIO_SET(GPIO_F_5, op);
		break;
#endif //CONFIG_SW_USB_LED0
	case LED_POWER_GREEN:
		GPIO_SET(GPIO_B_4, op);
		break;
	case LED_POWER_RED:
		GPIO_SET(GPIO_B_5, op);
		break;
	case LED_INTERNET_RED:
	case LED_PPP_RED:
		GPIO_SET(GPIO_B_6, op);
		break;
	case LED_WPS_GREEN:
		GPIO_SET(GPIO_C_3, op);
		break;
	case LED_WPS_RED:
		GPIO_SET(GPIO_C_4, op);
		break;
	case LED_WPS_YELLOW:
		GPIO_SET(GPIO_C_5, op);
		break;
	case LED_INTERNET_GREEN:
	case LED_PPP_GREEN:
		GPIO_SET(GPIO_D_1, op);
		break;
	default:
		led_handle_set(which, op);
	}
#else
#endif /* 2012-4-12 krammer add */
}

static void board_01_handle_init(void) {
	board_01_handle_set(LED_POWER_GREEN, LED_OFF);
	board_01_handle_set(LED_POWER_RED, LED_ON);
	board_01_handle_set(LED_DSL, LED_OFF);
	board_01_handle_set(LED_INTERNET_GREEN, LED_OFF);
	board_01_handle_set(LED_INTERNET_RED, LED_OFF);
	board_01_handle_set(LED_WPS_GREEN, LED_OFF);
	board_01_handle_set(LED_WPS_RED, LED_OFF);
	board_01_handle_set(LED_WPS_YELLOW, LED_OFF);
#ifdef CONFIG_SW_USB_LED1
	board_01_handle_set(LED_USB_1, LED_OFF);
#endif //CONFIG_SW_USB_LED1
#ifdef CONFIG_SW_USB_LED0
	board_01_handle_set(LED_USB_0, LED_OFF);
#endif //CONFIG_SW_USB_LED0

};

static struct led_operations board_01_operation = {
	.name = "board_01",
	.handle_init = board_01_handle_init,
	.handle_set = board_01_handle_set,
};


static void board_01_pb_init(void) {
};

static int board_01_pb_is_pushed(int which) {
	switch(which) {
#if 0//now we don't have WPS botton on hw
#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE)
	case PB_WPS:
		//120208-WPS
		return GPIO_READ(WPS_PIN);
		//return GPIO_READ(RTL8192CD_GPIO_2);
#endif
#endif
	default:
		return 0;
	}
}

static struct pushbutton_operations board_01_pb_op = {
	.handle_init = board_01_pb_init,
	.handle_is_pushed = board_01_pb_is_pushed,
};

static int __init board_01_led_init(void) {
#if 0//krammer mark first
#ifndef CONFIG_JTAG_USABLE
	unsigned int sicr;

	sicr = REG32(BSP_MISC_PINMUX);	
	if ( sicr & BSP_JTAG_GPIO_PINMUX ) {
		printk( "<<<<<<<disable GPIO JTAG function.\n" );
		REG32(BSP_MISC_PINMUX) = sicr & ~BSP_JTAG_GPIO_PINMUX;
	}
#endif //CONFIG_JTAG_USABLE
	REG32(BSP_MISC_PINMUX) &= ~(BSP_NANDF_GPIO
#ifdef CONFIG_SW_USB_LED1
		| BSP_USBLED1_GPF6
#endif //CONFIG_SW_USB_LED1
#ifdef CONFIG_SW_USB_LED0
		| BSP_USBLED0_GPF5
#endif //CONFIG_SW_USB_LED0
	);
	REG32(BSP_MISC_PINMUX) |= (BSP_LEDS1_GPH5 | BSP_LEDS2_GPH6 | BSP_LEDS3_GPH7 | BSP_LEDP0_GPF0);	//setup switch LED
	if (IS_8676V2)
		REG32(BSP_RGMII_PAD_CTRL) &= ~0x40000000; // Set to GPIO mode (USBLED1)
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8676
	REG32(BSP_IP_SEL) |= (BSP_EN_PCM | BSP_EN_VOIPACC);
	REG32(BSP_MISC_PINMUX) |= BSP_VOIP_GPIO_PINMUX_G1;
#ifdef CONFIG_RTK_VOIP_8676_SPI_GPIO
	REG32(BSP_PCM_SPI_IND_CTRL) &= ~BSP_SPI_GPIO_G1; //G1 GPIO-simulated SPI
	REG32(BSP_VOIP_SPI_CS_IND_CTRL) &= ~BSP_VOIP_SPI_CS1; //GPIO-simulated SPI CS1
	REG32(BSP_VOIP_SPI_CS_IND_CTRL) &= ~BSP_VOIP_SPI_CS2; //GPIO-simulated SPI CS2
//#elif defined (CONFIG_RTK_VOIP_8676_ISI_ZSI)
//	REG32(BSP_MISC_PINMUX) |= (BSP_ZSI_ISI_PINMUX_C0 | BSP_ZSI_ISI_PINMUX_C1);
#endif
#endif
#endif
#ifdef CONFIG_PCIE_POWER_SAVING
	REG32(IO_MODE_EN_REG) |= IO_MODE_INTRPT1_EN; // ENABLE interrupt 1 
#endif
	
	led_register_operations(&board_01_operation);
	pb_register_operations(&board_01_pb_op);
	return 0;
}

static void __exit board_01_led_exit(void) {
}


module_init(board_01_led_init);
module_exit(board_01_led_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPIO driver for Reload default");



