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
#define WPS_PIN RTL8192CD_GPIO_7

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
	*reset = GPIO_12;
}
EXPORT_SYMBOL(PCIE_reset_pin);

#ifdef CONFIG_USE_PCIE_SLOT_1
void PCIE1_reset_pin(int *reset){
	*reset = GPIO_59;
}
EXPORT_SYMBOL(PCIE1_reset_pin);
#endif

static void board_slave_handle_set(int which, int op) {
}

static void board_slave_handle_init(void) {

};

static struct led_operations board_slave_operation = {
	.name = "board_slave",
	.handle_init = board_slave_handle_init,
	.handle_set = board_slave_handle_set,
};


static void board_slave_pb_init(void) {
};

static int board_slave_pb_is_pushed(int which) {
		return 0;
}

static struct pushbutton_operations board_slave_pb_op = {
	.handle_init = board_slave_pb_init,
	.handle_is_pushed = board_slave_pb_is_pushed,
};

static int __init board_slave_led_init(void) {
	return 0;
}

static void __exit board_slave_led_exit(void) {
}


module_init(board_slave_led_init);
module_exit(board_slave_led_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPIO driver for Reload default");



