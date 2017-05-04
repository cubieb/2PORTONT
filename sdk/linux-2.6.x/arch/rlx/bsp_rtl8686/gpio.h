/*
 * Copyright c                Realtek Semiconductor Corporation, 2002
 * All rights reserved.                                                    
 * 
 * Abstract:
 *
 *   GPIO driver header file for export include.
 *
 */

#ifndef _GPIO_H_
#define _GPIO_H_

//#define CONFIG_JTAG_USABLE
enum GPIO_DATA {
#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE)
	GPIO_RTL8192CD,
#endif //CONFIG_RTL8192CD
	GPIO_DATA_NUM
};

enum GPIO_DEF {
	GPIO_0 = 0, GPIO_1, GPIO_2, GPIO_3, GPIO_4, GPIO_5, GPIO_6, GPIO_7, 
	GPIO_8, GPIO_9, GPIO_10, GPIO_11, GPIO_12, GPIO_13, GPIO_14, GPIO_15, 
	GPIO_16, GPIO_17, GPIO_18, GPIO_19, GPIO_20, GPIO_21, GPIO_22, GPIO_23, 
	GPIO_24, GPIO_25, GPIO_26, GPIO_27, GPIO_28, GPIO_29, GPIO_30, GPIO_31,
	GPIO_32, GPIO_33, GPIO_34, GPIO_35, GPIO_36, GPIO_37, GPIO_38, GPIO_39,
	GPIO_40, GPIO_41, GPIO_42, GPIO_43, GPIO_44, GPIO_45, GPIO_46, GPIO_47,
	GPIO_48, GPIO_49, GPIO_50, GPIO_51, GPIO_52, GPIO_53, GPIO_54, GPIO_55,
	GPIO_56, GPIO_57, GPIO_58, GPIO_59, GPIO_60, GPIO_61, GPIO_62, GPIO_63,
	GPIO_64, GPIO_65, GPIO_66, GPIO_67, GPIO_68, GPIO_69, GPIO_70, GPIO_71,
#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE)
	RTL8192CD_GPIO_0, RTL8192CD_GPIO_1, RTL8192CD_GPIO_2, RTL8192CD_GPIO_3,
	RTL8192CD_GPIO_4, RTL8192CD_GPIO_5, RTL8192CD_GPIO_6, RTL8192CD_GPIO_7,
	RTL8192CD_GPIO_8, RTL8192CD_GPIO_9, RTL8192CD_GPIO_10, RTL8192CD_GPIO_11,
	RTL8192CD_GPIO_12, RTL8192CD_GPIO_13, RTL8192CD_GPIO_14, RTL8192CD_GPIO_15,
	RTL8192CD_92E_ANTSEL_P,
#endif //CONFIG_RTL8192CD
	GPIO_END
};


#undef	GPIO_RESET_BUTTON
// #define GPIO_RESET_BUTTON       1
// #define	GPIO_LED_ADSL_HS	1
#undef	GPIO_LED_ADSL_HS
//#define	GPIO_LED_ADSL_ACT	1
#undef	GPIO_LED_ADSL_ACT
//#define	GPIO_LED_ADSL_ALARM	1
#undef	GPIO_LED_ADSL_ALARM
#undef	GPIO_LED_PPP		




/*real use of GPIO pin
Pin #   0   1 ...   7   8 ...  15
GPIO  PA0 PA1 ... PA7 PB0 ... PB7
*/
#ifdef CONFIG_EXT_SWITCH
// GPIO to simulate MDC/MDIO
// Note: the MDC and MDIO must use the same GPIO port (A or B) in current
// 	software design
#define GPIO_MDC	12	// PB4, 4+8
#define GPIO_MDIO	11	// PB3, 3+8
#endif

#if defined(CONFIG_RTL8670)
#define ADSL_LED	0

// Added by Mason Yu for PPP LED
#elif defined(CONFIG_STD_LED)
/* bitmap of GPIO pin as gpio function
Bit #   0   1 ...   7   8 ...  15
GPIO  PB0 PB1 ... PB7 PA0 ... PA7
*/
#define GPIO_FUNC	0x7820

#define RESET_BUTTON    14
#define ADSL_LED	13
#define ADSL_ACT_LED	11
#define ALARM_LED	12
#define PPP_LED		5
#define PCI_RST		10
#elif  defined(CONFIG_GPIO_LED_CHT_E8B)
#define PPP_RED
#define PPP_GREEN
#define LED_DSL_LINK 
#define LED_WPS_G
#define LED_WPS_R
#define LED_WPS_Y
#define RESET_BUTTON    10

#else // 8672
#define GPIO_FUNC	0xe000

#if 0
#define LED_POWER_GREEN	GPIO_B_6
#define LED_POWER_RED		GPIO_B_7
#define LED_DSL				GPIO_A_5
#define LED_INTERNET_GREEN	GPIO_B_2
#define LED_INTERNET_RED	GPIO_B_5
#endif

#define ADSL_LED	15
#define ADSL_ACT_LED	14
#define ALARM_LED	13
#endif


//WPS MODE
#define GENERIC_MODEL_WPS	0
// Andrew. This define will use GPIO B3(in) for PBC button, and B7(out) for LED
#define BIG_MODEL_WPS 		1
#define ALPHA_MODEL_WPS		2
#define E8B_MODEL_WPS		3
#define BOARD_TYPE 		E8B_MODEL_WPS

#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE)
#if defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1) && defined(CONFIG_WLAN0_5G_WLAN1_2G)
#define WLAN_INDEX	1
#else
#define WLAN_INDEX	0
#endif
#endif 

/*port function definition*/
#define GPIO_FUNC_INPUT 	0x0001  /*data input*/
#define GPIO_FUNC_OUTPUT 	0x0002	/*data output*/
#define GPIO_FUNC_IRQ_FALL 	0x0003	/*falling edge IRQ*/
#define GPIO_FUNC_IRQ_RISE 	0x0004	/*rising edge IRQ*/
#define GPIO_FUNC_IRQ_LEVEL 	0x0005	/*level trigger IRQ*/

extern void resetGPIOSwData(int data);
extern void gpioConfig (int gpio_num, int gpio_func);
//ccwei: 120208-WPS
extern void gpioSetGIMR(int irq_num, int value);
extern void gpioSetIMR(int gpio_num, int value);
extern void gpioClearISR(int gpio_num);
extern int gpioGetBspIRQNum(int gpio_num);
extern unsigned int gpioReadISR(int gpio_num);
//end
extern void gpioSet(int gpio_num);
extern void gpioClear(int gpio_num);
extern int gpioRead(int gpio_num);
extern void gpioHandshaking(int flag);
extern void gpioACT(int flag);
extern void gpioAlarm(int flag);
extern void gpio_LED_PPP(int flag);
#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE)
extern void set_rtl8192cd_gpio(void);
#endif

/* ADSL status */
#if 0
#define C_AMSW_IDLE                0
#define C_AMSW_L3                  1
#define C_AMSW_ACTIVATING          3
#define C_AMSW_INITIALIZING        6
#define C_AMSW_SHOWTIME_L0         9
#define C_AMSW_END_OF_LD     	15
#endif

#endif  /* _GPIO_H_ */
