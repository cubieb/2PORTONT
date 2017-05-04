#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include "led-generic.h"
#include "pushbutton.h"
#include "gpio.h"
//#include <asm/mach-realtek/rtl8672/platform.h>
/*linux-2.6.19*/ 
#include <bspchip.h>

#define LOW_ACTIVE	0
#if LOW_ACTIVE

#define GPIO_SET(value, w, op)  do { \
	if (LED_OFF==op) value|=(1<<w); \
	else value&=~(1<<w); \
} while (0);

#define GPIO_READ(w) (gpioRead(w))
		
#else

#define GPIO_SET(value, w, op)  do { \
	if (LED_ON==op) value|=(1<<w); \
	else value&=~(1<<w); \
} while (0);

#define GPIO_READ(w) (!gpioRead(w))

#endif

static unsigned int led_shift_value=0;
static struct timer_list board_led_timer;	
#define NCLKDELAY  (100)    // 100 nanoseconds
#define LED_SHIFT_STR_LEN  	10
#define GPIO_STR_LEN  		32

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

/* warning: this is not a re-entrant function! handle with care!*/
static void gpioShiftOutput(unsigned int shift_value)
{
        int i;
		if( IS_RLE0315 || IS_6166 ) {
	    	REG32(BSP_MISC_PINOCR)&= ~(1<<17 |1<<23);	//NICLEDPIN[1].output as GPIO_D1
	    	REG32(BSP_MISC_PINOCR) |= (1<<18);	//NICLEDPIN[2].output as GPIO_D2
		}
		/* D_1: LED_Data	D_2: LED_CLK	*/	
        gpioConfig(GPIO_D_1, GPIO_FUNC_OUTPUT);
        gpioConfig(GPIO_D_2, GPIO_FUNC_OUTPUT);
        gpioClear(GPIO_D_1);
        gpioClear(GPIO_D_2);

        for (i=0; i<16 ; i++) {
                // Set the clock line low and let it settle.
                gpioClear(GPIO_D_2);
                ndelay(NCLKDELAY);
                
                // Set the data line A to the bits value.  Data line
                // B is always high (tied to 3.3V).
                if (shift_value>>i & 0x1) 
                	gpioSet(GPIO_D_1);
                else 
                    gpioClear(GPIO_D_1);
                
                ndelay(NCLKDELAY);

                // Set the clock line high to latch the LED data
                gpioSet(GPIO_D_2);
                ndelay(NCLKDELAY);
        }
        gpioClear(GPIO_D_1);
        gpioClear(GPIO_D_2);
		//always remember the last behavior
		led_shift_value = shift_value;
}


static int led_shift_read_proc(char *page, char **start, off_t off,
                                   int count, int *eof, void *data)
{
        int len;
        len = sprintf(page, "LED shift is 0x%04X\n", led_shift_value);
        return len;
}


static int led_shift_write_proc(struct file *file, const char *buffer,
                                    unsigned long count, void *data)
{
        char led_shift_str[LED_SHIFT_STR_LEN];

        if (buffer == NULL) {
                printk("led_shift_write_proc: buffer == NULL\n");
                return -EFAULT;
        }
        if (count >= GPIO_STR_LEN) {
                printk("led_shift_write_proc: proc entry too large for conversion string.\n");
                return -EFAULT;
        }
        if (copy_from_user(&led_shift_str, buffer, count)) {
                printk("led_shift_write_proc: copy data from userspace faigpio.\n");
                return -EFAULT;
        }
        if (sscanf(led_shift_str, "0x%x", &led_shift_value) != 1) {
                printk("led_shift_write_proc: sscanf conversion error.\n");
                return -EFAULT;
        }
        if ((led_shift_value < 0) || (led_shift_value > 1<<16)) {
                printk("led_shift_write_proc: led_shift_value=0x%08X out of range.\n",
                       led_shift_value);
                return -EFAULT;
        }

        // Finally set the bit.
		gpioShiftOutput(led_shift_value);

        return count;
}

void PCIE_reset_pin(int *reset){
	*reset = GPIO_D_0;
}
EXPORT_SYMBOL(PCIE_reset_pin);

/* for proc/led_shift can retrieve the latest led value */
extern unsigned int led_shift_value;
//extern void gpioShiftOutput(unsigned int shift_value);

static void board_06_handle_set(int which, int op) {
	unsigned int led_value = led_shift_value;
	//printk("%s: led %d op %d\n", __FUNCTION__, which, op);
	switch (which) {
	case LED_POWER_GREEN:
		GPIO_SET(led_value, 15, op);
		break;
	case LED_POWER_RED:
		GPIO_SET(led_value, 14, op);
		break;
	case LED_DSL:	
		GPIO_SET(led_value, 13, op);  
		break;

	case LED_INTERNET_GREEN:	//use user_defined0
		GPIO_SET(led_value, 2, op); 
		break;
	case LED_INTERNET_RED:		//use user_define1
		GPIO_SET(led_value, 3, op);
		break;		

	case LED_PPP_GREEN:
		GPIO_SET(led_value, 12, op); 
		break;
	case LED_PPP_RED:
		GPIO_SET(led_value, 11, op);
		break;		
	case LED_WPS_GREEN:
		GPIO_SET(led_value, 7, op); 
		break;
	case LED_WPS_RED:
		GPIO_SET(led_value, 6, op); 
		break;
	case LED_WPS_YELLOW:
		GPIO_SET(led_value, 5, op); 
		break;				
	default:
		led_handle_set(which, op);
	}
		
	gpioShiftOutput(led_value);
}

static void board_06_handle_init(void) {
	board_06_handle_set(LED_POWER_GREEN, LED_OFF);
	board_06_handle_set(LED_POWER_RED, LED_ON);
	board_06_handle_set(LED_DSL, LED_OFF);
	board_06_handle_set(LED_INTERNET_GREEN, LED_OFF);
	board_06_handle_set(LED_INTERNET_RED, LED_OFF);
	board_06_handle_set(LED_WPS_GREEN, LED_OFF);
	board_06_handle_set(LED_WPS_RED, LED_OFF);
	board_06_handle_set(LED_WPS_YELLOW, LED_OFF);
};

static struct led_operations board_06_operation = {
	.name = "board_06",
	.handle_init = board_06_handle_init,
	.handle_set = board_06_handle_set,
	//.handle_flash = board_06_handle_flash,
};


static void board_06_pb_init(void) {
};

static int board_06_pb_is_pushed(int which) {
	switch(which) {
		
	case PB_RESET:
		if( IS_RLE0315 || IS_6166 ) {
			return GPIO_READ(GPIO_B_5);   
		}
	case PB_WIFISW:
		if( IS_RLE0315 || IS_6166 ) {
			return GPIO_READ(GPIO_B_6);  
		}
	case PB_WPS:
		if( IS_RLE0315 || IS_6166 ) {
			return GPIO_READ(GPIO_B_7); 
		}
	}
	return 0;
}

static struct pushbutton_operations board_06_pb_op = {
	.handle_init = board_06_pb_init,
	.handle_is_pushed = board_06_pb_is_pushed,
};

static int __init board_06_led_init(void) {
#ifndef CONFIG_JTAG_USABLE
	unsigned int sicr;

	sicr = REG32(BSP_MISC_PINMUX);	
	if ( sicr & BSP_JTAG_GPIO_PINMUX ) {
		printk( "<<<<<<<disable GPIO JTAG function.\n" );
		REG32(BSP_MISC_PINMUX) = sicr & ~BSP_JTAG_GPIO_PINMUX;
	}
#endif
	if( IS_RLE0315 || IS_6166 ) {
        REG32(BSP_MISC_PINOCR) |= 0x003c0000; //enable GPIO D1, D2 ,D3, D4, D5
        REG32(BSP_MISC_PINOCR) &= ~(1<<22);   //enable GPIO D0
	}
	init_timer (&board_led_timer);
	board_led_timer.data = 0;
	board_led_timer.function = gpioShiftOutput;

	led_register_operations(&board_06_operation);
	pb_register_operations(&board_06_pb_op);

	return 0;
}

static void __exit board_06_led_exit(void) {
}


module_init(board_06_led_init);
module_exit(board_06_led_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("LED driver for RL6166 IAD");



