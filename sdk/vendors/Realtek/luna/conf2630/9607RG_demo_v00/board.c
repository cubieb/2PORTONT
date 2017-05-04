#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include "led-generic.h"
#include "pushbutton.h"
#include "gpio.h"

#include <rtk/led.h>
#include <rtk/init.h>
#include <rtk/i2c.h>
//#include <asm/mach-realtek/rtl8672/platform.h>
/*linux-2.6.19*/ 
#include <bspchip.h>
#if (defined(CONFIG_SLOT_0_8812) || defined(CONFIG_SLOT_0_8812AR_VN))  && defined(CONFIG_WLAN0_5G_WLAN1_2G)
extern unsigned int check_slav_on;
#endif
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
	*reset = GPIO_8;
}
EXPORT_SYMBOL(PCIE_reset_pin);

#ifdef CONFIG_USE_PCIE_SLOT_1
void PCIE1_reset_pin(int *reset){
	*reset = GPIO_12;
}
EXPORT_SYMBOL(PCIE1_reset_pin);
#endif

//ccwei: 120208-WPS
#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE)

void get_wifi_wake_pin(int *gpio_num)
{
    *gpio_num = GPIO_45;
}

void get_wifi_wps_pin(int *gpio_num)
{
    *gpio_num = (WPS_PIN - RTL8192CD_GPIO_0);
}
#endif
//end
static void board_01_handle_set(int which, int op) {
	//printk("%s: led %d op %d\n", __FUNCTION__, which, op);
	switch (which) {
#ifdef CONFIG_SW_USB_LED1
	case LED_USB_1:
		GPIO_SET(GPIO_24, op);
		break;
#endif //CONFIG_SW_USB_LED1
#ifdef CONFIG_SW_USB_LED0
	case LED_USB_0:
		GPIO_SET(GPIO_21, op);
		break;
#endif //CONFIG_SW_USB_LED0
#if 0 /* 2012-4-12 krammer add */
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
#else
        case LED_INTERNET_GREEN:
                GPIO_SET(GPIO_23, op);
                break;
#endif /* 2012-4-12 krammer add */

        #if (defined(CONFIG_SLOT_0_8812) || defined(CONFIG_SLOT_0_8812AR_VN))  && defined(CONFIG_WLAN0_5G_WLAN1_2G)
        case LED_WPS_RED:
		if(check_slav_on){
                	if(op == LED_ON)
                	{
                        	REG32(0xBA000044) &= ~(1<<8) ;
                        	REG32(0xBA000044) |= (1<<16);
                	}
                	else
                        	REG32(0xBA000044) |= (1<<8)|(1<<16) ;
		}
                break;
        case LED_WPS_GREEN:
		if(check_slav_on){
                	if(op == LED_ON)
                	{
                        	REG32(0xBA000044) &= ~(1<<12) ;
                        	REG32(0xBA000044) |= (1<<20);
                	}
                	else
                        	REG32(0xBA000044) |= (1<<12)|(1<<20) ;
		}
                break;
        case LED_WPS_YELLOW:
		if(check_slav_on){
                	if(op == LED_ON)
                        	REG32(0xBA00004c) |= (1<<23);
                	else
                        	REG32(0xBA00004C) &= ~(1 <<23);
		}
                break;
        #elif (defined(CONFIG_SLOT_0_8192EE) || defined(CONFIG_SLOT_1_8192EE) )
        case LED_WPS_GREEN:
                GPIO_SET(RTL8192CD_GPIO_4, op);
                break;
        case LED_WPS_RED:
                GPIO_SET(RTL8192CD_GPIO_0, op);
                break;
        case LED_WPS_YELLOW:
                GPIO_SET(RTL8192CD_92E_ANTSEL_P, op);
                break;
        #endif

	default:
		led_handle_set(which, op);
	}
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
	case PB_RESET:
		return GPIO_READ(GPIO_0);

	case PB_WIFISW:
		return GPIO_READ(GPIO_45);
#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE)
        case PB_WPS:
                //120208-WPS
                #if (defined(CONFIG_SLOT_0_8812) || defined(CONFIG_SLOT_0_8812AR_VN))  && defined(CONFIG_WLAN0_5G_WLAN1_2G)
                return (REG32(0xBA000044) & (1<<7)) ? 0 : 1; // 92ER in slave , access the register directly
                #elif (defined(CONFIG_SLOT_0_8192EE) || defined(CONFIG_SLOT_1_8192EE) )
                return GPIO_READ(WPS_PIN);
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

#if (defined CONFIG_USE_UART0) && (defined CONFIG_CPU1_UART0)
#error "ERROR: CONFIG_USE_UART0 and CONFIG_CPU1_UART0 can NOT be enabled together, please run kernel menuconfig to fix it!!!"
#elif (defined CONFIG_USE_UART1) && (defined CONFIG_CPU1_UART1)
#error "ERROR: CONFIG_USE_UART1 and CONFIG_CPU1_UART1 can NOT be enabled together, please run kernel menuconfig to fix it!!!"
#endif /* #if (defined CONFIG_USE_UART0) && (defined CONFIG_CPU1_UART0) */

#if (defined CONFIG_USE_UART1) || (defined CONFIG_CPU1_UART1) || (CONFIG_SERIAL_8250_RUNTIME_UARTS > 1)
void luna_uart1_pinmux(void);
extern unsigned int SOC_ID, SOC_BOND_ID;
#endif /* #if (defined CONFIG_USE_UART1) || (defined CONFIG_CPU1_UART1) */

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


#if 0
	REG32(DATA_LED_CFG_2) = 0x40fff;
	REG32(DATA_LED_CFG_4) = 0x30fff;
	REG32(DATA_LED_CFG_6) = 0x20fff;
	REG32(DATA_LED_CFG_8) = 0x10fff;
	REG32(DATA_LED_CFG_13) = 0x50fff;
	//*((volatile int*)0xbb01e084) = 0x2000;
	REG32(IO_LED_EN) = 0x2154;
	REG32(LED_EN) = 0x42a8;
#endif

    {
        rtk_led_config_t ledConfig;

        rtk_core_init();
        rtk_led_init();
	rtk_i2c_init(0);

        rtk_led_operation_set(LED_OP_PARALLEL);
        rtk_led_parallelEnable_set(2,ENABLED);
        rtk_led_parallelEnable_set(4,ENABLED);
        rtk_led_parallelEnable_set(6,ENABLED);
        rtk_led_parallelEnable_set(8,ENABLED);
        //rtk_led_parallelEnable_set(12,ENABLED);
        //rtk_led_parallelEnable_set(13,ENABLED);
        //rtk_led_parallelEnable_set(16,ENABLED);
            
        memset(&ledConfig,0x0,sizeof(rtk_led_config_t));    
        ledConfig.ledEnable[LED_CONFIG_SPD1000ACT] = ENABLED;
        ledConfig.ledEnable[LED_CONFIG_SPD500ACT] = ENABLED;
        ledConfig.ledEnable[LED_CONFIG_SPD100ACT] = ENABLED;
        ledConfig.ledEnable[LED_CONFIG_SPD10ACT] = ENABLED;
        ledConfig.ledEnable[LED_CONFIG_SPD1000] = ENABLED;
        ledConfig.ledEnable[LED_CONFIG_SPD500] = ENABLED;
        ledConfig.ledEnable[LED_CONFIG_SPD100] = ENABLED;
        ledConfig.ledEnable[LED_CONFIG_SPD10] = ENABLED;

        rtk_led_config_set(2,LED_TYPE_UTP3,&ledConfig);    
        rtk_led_config_set(4,LED_TYPE_UTP2,&ledConfig);    
        rtk_led_config_set(6,LED_TYPE_UTP1,&ledConfig);    
        rtk_led_config_set(8,LED_TYPE_UTP0,&ledConfig);    
        /* Don't config LED, GPON/EPON will config it */
        //rtk_led_config_set(16,LED_TYPE_UTP4,&ledConfig);    

        //memset(&ledConfig,0x0,sizeof(rtk_led_config_t));  
        //ledConfig.ledEnable[LED_CONFIG_SOC_LINK_ACK] = ENABLED;  

        //rtk_led_config_set(6,LED_TYPE_USB1,&ledConfig);    
        //rtk_led_config_set(2,LED_TYPE_USB0,&ledConfig);       
    }

#ifdef CONFIG_LUNA_DUAL_LINUX
	/* Adjust priority */
	long mem_arb = REG32(0xB800101C);
        REG32(0xB8001020) = 0xEEEEEEEE;
        REG32(0xB8001024) = 0x11111111;
        REG32(0xB8001028) = 0xEDEDEDED;
        REG32(0xB800102C) = 0x9B9B9BBB;
        REG32(0xB8001030) = 0x75757555;
        REG32(0xB8001034) = 0xFFFFFFFF;
        REG32(0xB800101C) = mem_arb;
#endif /* #ifdef CONFIG_LUNA_DUAL_LINUX */


	led_register_operations(&board_01_operation);
	pb_register_operations(&board_01_pb_op);

#if (defined CONFIG_USE_UART1) || (defined CONFIG_CPU1_UART1) || (CONFIG_SERIAL_8250_RUNTIME_UARTS > 1)
	luna_uart1_pinmux();
#endif /* #if (defined CONFIG_USE_UART1) || (defined CONFIG_CPU1_UART1) */

	return 0;
}

#if (defined CONFIG_USE_UART1) || (defined CONFIG_CPU1_UART1) || (CONFIG_SERIAL_8250_RUNTIME_UARTS > 1)
void luna_uart1_pinmux(void)
{
    printk("=============Enter: %s=============\n", __FUNCTION__);
    if (SOC_ID==0x0371) {
        REG32(SOC_0371_UART1_PINMUX) = 0;
        //SDEBUG("SOC_0371_UART1_PINMUX 0x%08x\n" , REG32(SOC_0371_UART1_PINMUX));
    }

    printk("SOC_BOND_ID=0x%08x\n", SOC_BOND_ID);
    switch(SOC_BOND_ID) {
        case CHIP_901   :  
            printk("ERROR: UART1 is not supported, please run kernel menuconfig to turn off enabling UART1.\n");
            break;

        /* UTX1D1 is pinmuxed with  GPIO13/LED8(P0LED0) */
        /* URX1D1 is pinmuxed with  GPIO24/LED6(P1LED0) */
        case CHIP_906_1 :
        case CHIP_906_2 :
        case CHIP_902   :
        case CHIP_903   :
        case CHIP_907   :  
        case CHIP_96    :  
        case CHIP_2510  :  
        case CHIP_98B   :
        case CHIP_DBG   :
        case CHIP_9607P :
            /* Turn off GPIO 13/24 */
            REG32(GPIO_CRTL_2_REG_A) &= ~(GPIO13_EN | GPIO24_EN);
            /* Turn off LED 6/8 */
            REG32(SOC_6266_IO_LED_EN_REG_A) &= ~(LED6_EN | LED8_EN);
            /* Turn on UART1 */
            REG32(SOC_6266_UART1_PINMUX) |= (1<<4);
            printk("GPIO    setting/0x%08x 0x%08x\n" ,  GPIO_CRTL_2_REG_A, REG32(GPIO_CRTL_2_REG_A));
            printk("LED     setting/0x%08x 0x%08x\n" ,  SOC_6266_IO_LED_EN_REG_A, REG32(SOC_6266_IO_LED_EN_REG_A));
            printk("IO mode setting/0x%08x 0x%08x\n" ,  SOC_6266_UART1_PINMUX, REG32(SOC_6266_UART1_PINMUX));
            break;

        default: printk( "unknown bond id\n");
    }

    printk("Disabled GPIO/LED pin-mux with UART1.\n");
    printk("=============Exit: %s=============\n", __FUNCTION__);
}
#endif /* #if (defined CONFIG_USE_UART1) || (defined CONFIG_CPU1_UART1)  */



static void __exit board_01_led_exit(void) {
}


module_init(board_01_led_init);
module_exit(board_01_led_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPIO driver for Reload default");



