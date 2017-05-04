#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include "led-generic.h"
#include "pushbutton.h"
#include "gpio.h"
#include "bspchip.h"

#include <rtk/led.h>
#include <rtk/init.h>
#include <rtk/i2c.h>
//CONFIG_GPON_VERSION is define in 
//linux-2.6.x/drivers/net/rtl86900/Compiler_Flag
//#define CONFIG_GPON_VERSION 2
//#if CONFIG_GPON_VERSION < 2
//#include <rtk/gpon.h>
//#else
#include <module/gpon/gpon.h>
//#endif


//#define CONFIG_RTL8696_SERIES
#ifdef CONFIG_PON_LED_PROFILE_DEMO_RTL9607
#define CONFIG_RTL9607_SERIES
#elif CONFIG_PON_LED_PROFILE_DEMO_RTL9602B
#define CONFIG_RTL9602_SERIES
#endif
//#define CONFIG_RTL9607P_SERIES

#define DEBUG
#ifdef DEBUG
#define PRINT printk
#else
#define PRINT
#endif

#ifdef CONFIG_APOLLO_MP_TEST
static struct proc_dir_entry *led_test=NULL;
unsigned char led_test_start=0;
unsigned char usbtest=0, ethtest=0;
extern char default_flag;
extern char reboot_flag;
extern char wlan_onoff_flag;
#endif


#define MP_LOW_ACTIVE	1
#if MP_LOW_ACTIVE

#define MP_GPIO_SET(w, op)  do { \
	gpioConfig(w, GPIO_FUNC_OUTPUT); \
	if (LED_ON==op) gpioClear(w); \
	else gpioSet(w); \
} while (0);

#define MP_GPIO_READ(w) (!gpioRead(w))
		
#else

#define MP_GPIO_SET(w, op)  do { \
	gpioConfig(w, GPIO_FUNC_OUTPUT); \
	if (LED_ON==op) gpioSet(w); \
	else gpioClear(w); \
} while (0);

#define MP_GPIO_READ(w) (gpioRead(w))
#endif



typedef enum MP_LED_TYPE
{
	LED_LAN0 = 0,
	LED_LAN1,
	LED_LAN2,
	LED_LAN3,
	LED_LAN4, // 8696/9607P
	LED_LAN5, // 8696/9607P
	LED_PON,
	LED_USB0,
	LED_USB1,
	LED_WPSR,
	LED_WPSY,
	LED_WPSG,
	LED_NET_G,
	LED_NET_R,	
	LED_WAN, // 9607
	LED_LOS, // 9601b
	LED_1G, // 9601b
	LED_SATA, //9607P
	LED_PWR, //9607P
	LED_FX0,
	LED_FX1,
	// wlan0/wlan1/fxs0/fxs1 control by userspace 
}mp_led_type;


struct led_gpio_struct
{
	mp_led_type mp_led_type;
	unsigned int mp_led_gpio_index;
	unsigned int mp_led_gpio_state;
}
#if defined(CONFIG_RTL8696_SERIES)
RTL8696[]={{LED_WAN,58,0},{LED_USB0,21,0},{LED_USB1,22,0},{LED_LAN0,13,1},
			{LED_LAN1,24,1},{LED_LAN2,46,0},{LED_LAN3,48,0},{LED_NET_G,35,0}};
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9607)
RTL9607[]={{LED_WAN,21,0},{LED_USB0,48,0},{LED_USB1,24,1},{LED_LAN0,58,0},
			{LED_LAN1,59,0},{LED_LAN2,46,0},{LED_LAN3,13,0},{LED_NET_G,60,0},
			{LED_PON,22,0}};
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9607_IAD_V00)
RTL9607[]={{LED_LAN0,13,0},{LED_LAN1,24,1},{LED_LAN2,46,0},{LED_LAN3,48,0},
			{LED_USB0,21,0},{LED_NET_G,23,0},{LED_PON,58,0},{LED_LOS,47,0},
			{LED_PWR,10,0},{LED_FX0,31,0}, {LED_FX1,1,0}};
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9607P)
RTL9607P[]={{LED_LAN4,47,0},{LED_USB0,56,0},{LED_USB1,55,0},{LED_LAN0,13,1},
			{LED_LAN1,24,1},{LED_LAN2,46,0},{LED_LAN3,48,0},{LED_NET_G,23,0},
			{LED_PON,58,0},{LED_SATA,57,0}};
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9602B)
RTL9602[]={{LED_LAN1,13,0},{LED_LAN2,24,1},{LED_NET_G,45,0},{LED_PON,22,0}};
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9602C)
RTL9602[]={{LED_LAN0,26,0},{LED_LAN1,39,0},{LED_NET_G,31,0},{LED_NET_R,35,0},{LED_PWR,36,0}, 
			{LED_PON,29,0},{LED_LOS,28,0},{LED_USB0,21,0},{LED_FX0,30,0}};
#else
;
#endif

struct ledstruct
{
	mp_led_type mp_led_type;
	unsigned int mp_led_index;
	unsigned int mp_led_state;
	rtk_led_type_t rtk_led_type;
}
RTL9601b[]={{LED_LAN0,0,0,LED_TYPE_UTP0},{LED_PON,1,0,LED_TYPE_PON},{LED_LOS,2,0,LED_TYPE_PON},{LED_1G,3,0,LED_TYPE_UTP0}};

/*global variable for led/push button test!*/
unsigned int gMPTEST_STATE = 0;

static void print_string(char *str)
{
	printk("%s", str);
}

unsigned int mptest_state(void)
{
	return gMPTEST_STATE;
}
		
void gpioDisable(int gpio_num)
{
        unsigned int reg_num = ((gpio_num>>5)<<2);
        unsigned int mask;

        if ((gpio_num >= 72)||(gpio_num < 0)) return;
        if (gpio_num <= 71)
        {
                mask = 1 << (gpio_num%32);
                REG32((0xBB0000F0 + reg_num)) &= ~mask;
        }
}
	
#ifdef CONFIG_PON_LED_PROFILE_DEMO_RTL9601B				
void mptest_off(struct ledstruct led[],unsigned int total_leds)
{
	int i = 0;
	rtk_led_config_t ledConfig;

	memset(&ledConfig,0x0,sizeof(rtk_led_config_t));
	ledConfig.ledEnable[LED_CONFIG_FORCE_MODE] = ENABLED;
	
	for(i=0;i<total_leds;i++)
	{
		rtk_led_parallelEnable_set (led[i].mp_led_index,ENABLED);
		rtk_led_config_set(led[i].mp_led_index,led[i].rtk_led_type,&ledConfig);
		rtk_led_modeForce_set(led[i].mp_led_index,LED_FORCE_OFF);
	}
	
	gMPTEST_STATE = 0;
}
#else
void mptest_off(struct led_gpio_struct led[],unsigned int total_leds)
{
	int i = 0;
	int OP;
	for(i=0;i<total_leds;i++)
	{
		if(led[i].mp_led_gpio_state) 
			OP = LED_ON;//high active
		else
			OP = LED_OFF;//low active	
		MP_GPIO_SET(led[i].mp_led_gpio_index, led[i].mp_led_gpio_state);
	}
}
#endif

#ifdef CONFIG_PON_LED_PROFILE_DEMO_RTL9601B
void mptest_on(struct ledstruct led[], unsigned int total_leds)
{
	int i = 0;
	rtk_led_config_t ledConfig;
	gMPTEST_STATE = 1;
	memset(&ledConfig,0x0,sizeof(rtk_led_config_t));
	ledConfig.ledEnable[LED_CONFIG_FORCE_MODE] = ENABLED;
	for(i=0;i<total_leds;i++)
	{
		rtk_led_parallelEnable_set (led[i].mp_led_index,ENABLED);
		rtk_led_config_set(led[i].mp_led_index,led[i].rtk_led_type,&ledConfig);
		rtk_led_modeForce_set(led[i].mp_led_index,LED_FORCE_ON);
	}
}
#else
void mptest_on(struct led_gpio_struct led[], unsigned int total_leds)
{
	int i = 0;
	int OP=0;
	gMPTEST_STATE = 1;
	PRINT("%s-%d total_leds=%d\n",__func__,__LINE__,total_leds);
	for(i=0;i<total_leds;i++)
	{
		if(led[i].mp_led_gpio_state) 
			OP = LED_OFF;//high active
		else
			OP = LED_ON;//low active
		PRINT("%s-%d  [%d].index=%d [%d].state=%d\n",__func__,__LINE__,i,led[i].mp_led_gpio_index,i,led[i].mp_led_gpio_state);
		MP_GPIO_SET(led[i].mp_led_gpio_index, OP);
	}
}
#endif

static void wps_8812_led_on(void)
{
	PRINT("WPS 8812 ON\n");
	REG32(0xBA000044) &= ~(1<<12) ;
	REG32(0xBA000044) |= (1<<20);
}

static void wps_8812_led_off(void)
{
	PRINT("WPS 8812 OFF\n");
	REG32(0xBA000044) |= (1<<12)|(1<<20) ;
	REG32(0xBA00004C) &= ~(1 <<23);
}

static void wps_8192_led_on(void)
{
#if defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9601B)
#elif defined(CONFIG_RTL8696_SERIES)
	PRINT("WPS 8192 ON\n");
	MP_GPIO_SET(RTL8192CD_GPIO_4, LED_ON);
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9607)
	PRINT("WPS 8192 ON\n");
	MP_GPIO_SET(RTL8192CD_GPIO_4, LED_ON); //wps_G
	MP_GPIO_SET(RTL8192CD_GPIO_0, LED_ON); //wps_R
	MP_GPIO_SET(RTL8192CD_92E_ANTSEL_P, LED_ON); //wps_Y
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9607_IAD_V00)
	PRINT("WPS 8192 ON\n");
	MP_GPIO_SET(RTL8192CD_GPIO_4, LED_ON); //wps_G
	MP_GPIO_SET(RTL8192CD_GPIO_0, LED_ON); //wps_R
	MP_GPIO_SET(RTL8192CD_92E_ANTSEL_P, LED_ON); //wps_Y
#elif defined(CONFIG_CONFIG_PON_LED_PROFILE_DEMO_RTL9607P)
	PRINT("WPS 8192 ON\n");
	/*Dual linux can't find CONFIG_SLOT_1_8192EE in master*/
	//WPS_G on
	PRINT("WPS ON 92E\n");
	REG32(0xB9000044) &= ~(1<<12);
	REG32(0xB9000044) |= (1<<20);
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9602B)
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9602C)
        PRINT("WPS 8192 ON\n");
        MP_GPIO_SET(RTL8192CD_GPIO_4, LED_ON); //wps_G
        MP_GPIO_SET(RTL8192CD_GPIO_0, LED_ON); //wps_R
        MP_GPIO_SET(RTL8192CD_92E_ANTSEL_P, LED_ON); //wps_Y
#endif
}

static void wps_8192_led_off(void)
{

#if defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9601B)
#elif defined(CONFIG_RTL8696_SERIES)
	PRINT("WPS 8192 OFF\n");
	MP_GPIO_SET(RTL8192CD_GPIO_4, LED_OFF);
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9607)
	PRINT("WPS 8192 OFF\n");
	MP_GPIO_SET(RTL8192CD_GPIO_4, LED_OFF); //wps_G
	MP_GPIO_SET(RTL8192CD_GPIO_0, LED_OFF); //wps_R
	MP_GPIO_SET(RTL8192CD_92E_ANTSEL_P, LED_OFF); //wps_Y	
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9607_IAD_V00)
	PRINT("WPS 8192 OFF\n");
	MP_GPIO_SET(RTL8192CD_GPIO_4, LED_OFF); //wps_G
	MP_GPIO_SET(RTL8192CD_GPIO_0, LED_OFF); //wps_R
	MP_GPIO_SET(RTL8192CD_92E_ANTSEL_P, LED_OFF); //wps_Y	
#elif defined(CONFIG_CONFIG_PON_LED_PROFILE_DEMO_RTL9607P)
	//WPS_G off
	PRINT("WPS 8192 OFF\n");
	REG32(0xB9000044) |= (1<<12)|(1<<20) ;
	REG32(0xB900004C) &= ~(1 <<23);
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9602B)
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9602C)
        PRINT("WPS 8192 OFF\n");
        MP_GPIO_SET(RTL8192CD_GPIO_4, LED_OFF); //wps_G
        MP_GPIO_SET(RTL8192CD_GPIO_0, LED_OFF); //wps_R
        MP_GPIO_SET(RTL8192CD_92E_ANTSEL_P, LED_OFF); //wps_Y
#endif

}


static int mptest_read_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;
	return len;
}


static int mptest_write_proc(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	char flag;
	static unsigned int regValue;	
	if (buffer && !copy_from_user(&flag, buffer, 1)) 
	{
		if (flag == '0')
		{
#if defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9601B)
			print_string("LED OFF\n");
			mptest_off(RTL9601b,sizeof(RTL9601b)/sizeof(RTL9601b[0]));
#elif defined(CONFIG_RTL8696_SERIES)
			print_string("LED OFF\n");
			mptest_off(RTL8696,sizeof(RTL8696)/sizeof(RTL8696[0]));
	#if (defined(CONFIG_SLOT_0_8812) || defined(CONFIG_SLOT_0_8812AR_VN))  && defined(CONFIG_WLAN0_5G_WLAN1_2G)
			wps_8812_led_off();
	#elif (defined(CONFIG_SLOT_0_8192EE) || defined(CONFIG_SLOT_1_8192EE) )
			wps_8192_led_off();
	#endif
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9607)
			print_string("LED OFF\n");
			mptest_off(RTL9607,sizeof(RTL9607)/sizeof(RTL9607[0]));
	#if (defined(CONFIG_SLOT_0_8812) || defined(CONFIG_SLOT_0_8812AR_VN))  && defined(CONFIG_WLAN0_5G_WLAN1_2G)
			wps_8812_led_off();
	#elif (defined(CONFIG_SLOT_0_8192EE) || defined(CONFIG_SLOT_1_8192EE) )
			wps_8192_led_off();
	#endif	
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9607_IAD_V00)
			print_string("RTL9607 LED OFF\n");
			if(0 != REG32(0xBB023014))
				regValue = REG32(0xBB023014); //record current status
			REG32(0xBB023014) = 0;
			mptest_off(RTL9607,sizeof(RTL9607)/sizeof(RTL9607[0]));
	#if (defined(CONFIG_SLOT_0_8812) || defined(CONFIG_SLOT_0_8812AR_VN))  && defined(CONFIG_WLAN0_5G_WLAN1_2G)
			wps_8812_led_off();
	#elif (defined(CONFIG_SLOT_0_8192EE) || defined(CONFIG_SLOT_1_8192EE) )
			wps_8192_led_off();
	#endif		
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9602B)	
			print_string("LED OFF\n");
			mptest_off(RTL9602,sizeof(RTL9602)/sizeof(RTL9602[0]));
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9602C)	
			print_string("RTL9602 LED OFF\n");
			if(0 != REG32(0xBB023014))
				regValue = REG32(0xBB023014); //record current status
			REG32(0xBB023014) = 0;
			mptest_off(RTL9602,sizeof(RTL9602)/sizeof(RTL9602[0]));
	#if (defined(CONFIG_SLOT_0_8812) || defined(CONFIG_SLOT_0_8812AR_VN))  && defined(CONFIG_WLAN0_5G_WLAN1_2G)
			wps_8812_led_off();
	#elif (defined(CONFIG_SLOT_0_8192EE) || defined(CONFIG_SLOT_1_8192EE) )
			wps_8192_led_off();
	#endif
#elif defined(CONFIG_CONFIG_PON_LED_PROFILE_DEMO_RTL9607P)
			print_string("LED OFF\n");
			mptest_off(RTL9607P,sizeof(RTL9607P)/sizeof(RTL9607P[0]));
	#if (defined(CONFIG_SLOT_0_8812) || defined(CONFIG_SLOT_0_8812AR_VN))  && defined(CONFIG_WLAN0_5G_WLAN1_2G)
			wps_8812_led_off();
	#elif (defined(CONFIG_SLOT_0_8192EE) || defined(CONFIG_SLOT_1_8192EE) )
			wps_8192_led_off();
	#endif		
#endif
		}
		else if (flag == '1')
		{
#if defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9601B)
			PRINT("LED ON\n");
			mptest_on(RTL9601b,sizeof(RTL9601b)/sizeof(RTL9601b[0]));
#elif defined(CONFIG_RTL8696_SERIES)
			PRINT("RTL8696 LED ON\n");
			regValue = REG32(0xBB023014); //record current status
			REG32(0xBB023014) = 0;
			mptest_on(RTL8696,sizeof(RTL8696)/sizeof(RTL8696[0]));
	#if (defined(CONFIG_SLOT_0_8812) || defined(CONFIG_SLOT_0_8812AR_VN))  && defined(CONFIG_WLAN0_5G_WLAN1_2G)
			wps_8812_led_on();
	#elif (defined(CONFIG_SLOT_0_8192EE) || defined(CONFIG_SLOT_1_8192EE))
			wps_8192_led_on();
	#endif
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9607)
			PRINT("RTL9607 LED ON\n");
			regValue = REG32(0xBB023014); //record current status
			REG32(0xBB023014) = 0;
			mptest_on(RTL9607,sizeof(RTL9607)/sizeof(RTL9607[0]));
	#if (defined(CONFIG_SLOT_0_8812) || defined(CONFIG_SLOT_0_8812AR_VN))  && defined(CONFIG_WLAN0_5G_WLAN1_2G)
			wps_8812_led_on();
	#elif (defined(CONFIG_SLOT_0_8192EE) || defined(CONFIG_SLOT_1_8192EE))
			wps_8192_led_on();
	#endif	
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9607_IAD_V00)
			PRINT("RTL9607 LED ON\n");
			if(0 != REG32(0xBB023014))
				regValue = REG32(0xBB023014); //record current status
			REG32(0xBB023014) = 0;
			mptest_on(RTL9607,sizeof(RTL9607)/sizeof(RTL9607[0]));
	#if (defined(CONFIG_SLOT_0_8812) || defined(CONFIG_SLOT_0_8812AR_VN))  && defined(CONFIG_WLAN0_5G_WLAN1_2G)
			wps_8812_led_on();
	#elif (defined(CONFIG_SLOT_0_8192EE) || defined(CONFIG_SLOT_1_8192EE))
			wps_8192_led_on();
	#endif	
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9602B)	
			PRINT("RTL9602 LED ON\n");
			regValue = REG32(0xBB023014); //record current status
			REG32(0xBB023014) = 0;
			mptest_on(RTL9602,sizeof(RTL9602)/sizeof(RTL9602[0]));
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9602C)
			PRINT("RTL9602 LED ON\n");
			if(0 != REG32(0xBB023014))
				regValue = REG32(0xBB023014); //record current status
			REG32(0xBB023014) = 0;
			mptest_on(RTL9602,sizeof(RTL9602)/sizeof(RTL9602[0]));
	#if (defined(CONFIG_SLOT_0_8812) || defined(CONFIG_SLOT_0_8812AR_VN))  && defined(CONFIG_WLAN0_5G_WLAN1_2G)
			wps_8812_led_on();
	#elif (defined(CONFIG_SLOT_0_8192EE) || defined(CONFIG_SLOT_1_8192EE))
			wps_8192_led_on();
	#endif	
#elif defined(CONFIG_CONFIG_PON_LED_PROFILE_DEMO_RTL9607P)	
			PRINT("RTL9607P LED ON\n");
			regValue = REG32(0xBB023014); //record current status
			REG32(0xBB023014) = 0;
			mptest_on(RTL9607P,sizeof(RTL9607P)/sizeof(RTL9607P[0]));
	#if ((defined(CONFIG_SLOT_0_8812) || defined(CONFIG_SLOT_0_8812AR_VN)) && defined(CONFIG_WLAN0_5G_WLAN1_2G))
			wps_8812_led_on();		
	#else
			wps_8192_led_on();
	#endif	
#endif
		}else if (flag == '2'){
			//restore status
			REG32(0xBB023014) = regValue;
			gMPTEST_STATE = 0;
		}
		
		return count;
	}
	return -EFAULT;
}



#ifdef CONFIG_APOLLO_MP_TEST
#ifdef CONFIG_PON_LED_PROFILE_DEMO_RTL9601B
#define P_ABLTY 0xBB0000EC
#define PORT_NUM 1
#else
#define P_ABLTY 0xBB0000BC
#define PORT_NUM 4
#endif
static int led_test_read(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{	  
	int len=0;

	if (ethtest) {
		int LinkStatus, portnum;
		rtk_gpon_fsm_status_t state;
	
		for (portnum=0; portnum<PORT_NUM; portnum++) {
			LinkStatus = ((*(volatile unsigned int *)(P_ABLTY + (portnum<<2))) >> 4)&0x1;
			len += sprintf(page+len, "Test Switch LAN PORT %d %s\n",
					portnum+1, LinkStatus?"UP":"DOWN");
		}
		rtk_gponapp_ponStatus_get(&state);
		if(state == RTK_GPONMAC_FSM_STATE_O5)
			LinkStatus = 1;
		else
			LinkStatus = 0;
		len += sprintf(page+len, "Test Switch PON PORT %s\n", LinkStatus?"UP":"DOWN");
		ethtest = 0;
	}
	else if (led_test_start) {
		if ('0' != default_flag)
			len += sprintf(page+len, "RESET success!\n");
		/*FIXME. where should I get the wps button info? */
		if (pb_is_pushed(PB_WPS) == 1)
			len += sprintf(page+len, "WPS success!\n");
#ifdef CONFIG_WLAN_ON_OFF_BUTTON		
		if ('0' != wlan_onoff_flag)
			len += sprintf(page+len, "WLAN success!\n");
#endif		
	}

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	return len;
}

static int led_set_single_light(mp_led_type mp_led_type)
{
	int i;
	rtk_led_config_t ledConfig;
	
#if defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9601B)
	int total_leds = (sizeof(RTL9601b)/sizeof(RTL9601b[0]));
	memset(&ledConfig,0x0,sizeof(rtk_led_config_t));
	ledConfig.ledEnable[LED_CONFIG_FORCE_MODE] = ENABLED;
	//printk("total_leds = %d\n",total_leds);

	for(i=0;i<total_leds;i++)
	{
		if(RTL9601b[i].mp_led_type == mp_led_type){
			rtk_led_parallelEnable_set (RTL9601b[i].mp_led_index,ENABLED);
			rtk_led_config_set(RTL9601b[i].mp_led_index,RTL9601b[i].rtk_led_type,&ledConfig);
			rtk_led_modeForce_set(RTL9601b[i].mp_led_index,LED_FORCE_ON);
			return 0;
		}
	}
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9607_IAD_V00)
	int total_leds = (sizeof(RTL9607)/sizeof(RTL9607[0]));
	int OP=0;

	for(i=0;i<total_leds;i++)
	{
		if(RTL9607[i].mp_led_type == mp_led_type)
		{ 
			if(RTL9607[i].mp_led_gpio_state) 
				OP = LED_OFF;//high active
			else
				OP = LED_ON;//low active
			PRINT("%s-%d  [%d].index=%d [%d].state=%d\n",__func__,__LINE__,i,RTL9607[i].mp_led_gpio_index,i,RTL9607[i].mp_led_gpio_state);
			MP_GPIO_SET(RTL9607[i].mp_led_gpio_index, OP);
		}
	}

#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9602C)
	int total_leds = (sizeof(RTL9602)/sizeof(RTL9602[0]));
	int OP=0;

	for(i=0;i<total_leds;i++)
	{
		if(RTL9602[i].mp_led_type == mp_led_type)
		{ 
			if(RTL9602[i].mp_led_gpio_state) 
				OP = LED_OFF;//high active
			else
				OP = LED_ON;//low active
			PRINT("%s-%d  [%d].index=%d [%d].state=%d\n",__func__,__LINE__,i,RTL9602[i].mp_led_gpio_index,i,RTL9602[i].mp_led_gpio_state);
			MP_GPIO_SET(RTL9602[i].mp_led_gpio_index, OP);
		}
	}
#endif
	return -1;
}

void led_set_all_led_on(void)
{
#if defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9601B)
PRINT("LED ON\n");
mptest_on(RTL9601b,sizeof(RTL9601b)/sizeof(RTL9601b[0]));
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9607_IAD_V00)
			PRINT("RTL9607 LED ON\n");
			mptest_on(RTL9607,sizeof(RTL9607)/sizeof(RTL9607[0]));
	#if (defined(CONFIG_SLOT_0_8812) || defined(CONFIG_SLOT_0_8812AR_VN))  && defined(CONFIG_WLAN0_5G_WLAN1_2G)
			wps_8812_led_on();
	#elif (defined(CONFIG_SLOT_0_8192EE) || defined(CONFIG_SLOT_1_8192EE))
			wps_8192_led_on();
	#endif
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9602C)
			PRINT("RTL9602C LED ON\n");
			mptest_on(RTL9602,sizeof(RTL9602)/sizeof(RTL9602[0]));
	#if (defined(CONFIG_SLOT_0_8812) || defined(CONFIG_SLOT_0_8812AR_VN))  && defined(CONFIG_WLAN0_5G_WLAN1_2G)
			wps_8812_led_on();
	#elif (defined(CONFIG_SLOT_0_8192EE) || defined(CONFIG_SLOT_1_8192EE))
			wps_8192_led_on();
	#endif		
#endif
/*TODO: any other led should be add here*/
}

void led_set_all_led_off(void)
{
	static unsigned int regValue;	
#if defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9601B)
				print_string("LED OFF\n");
				mptest_off(RTL9601b,sizeof(RTL9601b)/sizeof(RTL9601b[0]));
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9607_IAD_V00)
			print_string("RTL9607 LED OFF\n");
			mptest_off(RTL9607,sizeof(RTL9607)/sizeof(RTL9607[0]));
	#if (defined(CONFIG_SLOT_0_8812) || defined(CONFIG_SLOT_0_8812AR_VN))  && defined(CONFIG_WLAN0_5G_WLAN1_2G)
			wps_8812_led_off();
	#elif (defined(CONFIG_SLOT_0_8192EE) || defined(CONFIG_SLOT_1_8192EE) )
			wps_8192_led_off();
	#endif
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9602C)	
			print_string("RTL9602 LED OFF\n");
			mptest_off(RTL9602,sizeof(RTL9602)/sizeof(RTL9602[0]));
	#if (defined(CONFIG_SLOT_0_8812) || defined(CONFIG_SLOT_0_8812AR_VN))  && defined(CONFIG_WLAN0_5G_WLAN1_2G)
			wps_8812_led_off();
	#elif (defined(CONFIG_SLOT_0_8192EE) || defined(CONFIG_SLOT_1_8192EE) )
			wps_8192_led_off();
	#endif	
#endif			
				/*TODO: any other led should be add here*/

}

static int led_test_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	rtk_led_config_t ledConfig;
	char	tmpbuf[512];
	char	*strptr, *tokptr;
	static unsigned int regValue;	

	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len] = '\0';
		
		strptr=tmpbuf;

		if(strlen(strptr)==0)
			goto errout;
		tokptr = strsep(&strptr," ");
		if (tokptr == NULL)
			goto errout;
		
		/*parse command*/
		if(strncmp(tokptr, "start", 5) == 0)
		{
			led_test_start = 1;
			if(0 != REG32(0xBB023014))
				regValue = REG32(0xBB023014); //record current status
			REG32(0xBB023014) = 0;			
		}
		else if(strncmp(tokptr, "stop", 4) == 0)
		{
			led_test_start = 0;
			/* clear all button trace */
#ifdef CONFIG_WLAN_ON_OFF_BUTTON			
			wlan_onoff_flag = '0';
#endif
			default_flag = '0';
			REG32(0xBB023014) = regValue;

			/*TODO, we should add wps button later*/
		}
		else if (strncmp(tokptr, "reset", 5) == 0)
		{
			if (0 == led_test_start)
				goto errout;
			default_flag = '1';
		}
#ifdef CONFIG_USB_SUPPORT		
		else if(strncmp(tokptr, "usbtest", 7) == 0)
		{
		}
#endif		
		else if (strncmp(tokptr, "ethstatus", 9) == 0)
		{
			ethtest = 1;
		}
		else if (strncmp(tokptr, "set", 3) == 0)
		{
			if (0 == led_test_start)
				goto errout;
			
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
				goto errout;
			
			if ( !strncmp(tokptr, "power", 5)) {
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
					goto errout;

				if ( !strncmp(tokptr, "green", 5)) {
					led_on(LED_POWER_GREEN);
				}
				else if ( !strncmp(tokptr, "red", 3)) {
					led_on(LED_POWER_RED);
				}
			}
			else if ( !strncmp(tokptr, "internet", 8)) {
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
					goto errout;

				if ( !strncmp(tokptr, "green", 5)) {
					led_on(LED_INTERNET_GREEN);
				}
				else if ( !strncmp(tokptr, "red", 3)) {
					led_on(LED_INTERNET_RED);
				}
			}
			else if ( !strncmp(tokptr, "lan", 3)) {
				int port;
				
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
					goto errout;
				port = simple_strtol(tokptr, NULL, 0);
#if defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9601B)
				if ((port>1) || (port<1))
#else
				if ((port>4) || (port<1))
#endif					
					goto errout;
				printk("port:%d\n",port);
				switch(port){
					case 1:
						led_set_single_light(LED_LAN0);
						break;
					case 2:
						led_set_single_light(LED_LAN1);
						break;
					case 3:
						led_set_single_light(LED_LAN2);
						break;
					case 4:
						led_set_single_light(LED_LAN3);
						break;
					default:
						printk("Error Port=%d\n", port);
						break;
				}
				
			}
			else if( !strncmp(tokptr, "pon", 3)) {
				led_set_single_light(LED_PON);
			}
			else if( !strncmp(tokptr, "los", 3)) {
				led_set_single_light(LED_LOS);
			}
#ifdef CONFIG_RTK_VOIP
			else if( !strncmp(tokptr, "fxs", 3)) {
				int port;
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
					goto errout;
				port = simple_strtol(tokptr, NULL, 0);
				switch(port)
				{
					case 1:
						led_set_single_light(LED_FX0);
						break;
					case 2:
						led_set_single_light(LED_FX1);
						break;
					default:
						printk("Error Port=%d\n", port);
						break;
				}
			}				
#endif			
#if defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9601B)
			else if( !strncmp(tokptr, "1G", 2)) {
				led_set_single_light(LED_1G);
			}
#endif	
			else if ( !strncmp(tokptr, "dsl", 3)) {
				//TODO
			}
#ifndef CONFIG_PON_LED_PROFILE_DEMO_RTL9601B			
			else if ( !strncmp(tokptr, "wlan0", 5)) {
#if defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9607_IAD_V00)
				MP_GPIO_SET(RTL8192CD_GPIO_5, LED_ON); //wps_G
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9602C)	
				MP_GPIO_SET(RTL8192CD_GPIO_5, LED_ON); //wps_G
#endif
				led_on(LED_WLAN_2_4G);
			}
			else if ( !strncmp(tokptr, "wlan1", 5)) {
				led_on(LED_WLAN_5G);
			}
			else if ( !strncmp(tokptr, "wps", 2)) {
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
					goto errout;

				if ( !strncmp(tokptr, "green", 5)) {
					led_on(LED_WPS_GREEN);
				}
			}
#ifdef CONFIG_USB_SUPPORT			
			else if ( !strncmp(tokptr, "usbhost", 7)) {
#ifdef CONFIG_SW_USB_LED0
				led_on(LED_USB_0);
#endif
#ifdef CONFIG_SW_USB_LED1
				led_on(LED_USB_1);
#endif
			}
#endif			
#endif /*CONFIG_RTL9601B_SERIES*/			
		}
		else if (strncmp(tokptr, "allledon", 8) == 0) {
			if (0 == led_test_start)
				goto errout;
			led_set_all_led_on();

		}
		else if (strncmp(tokptr, "allledoff", 9) == 0) {
			if (0 == led_test_start)
				goto errout;
			led_set_all_led_off();
		}
		else if (strncmp(tokptr, "allgreenon", 10) == 0) {
			if (0 == led_test_start)
				goto errout;
			printk("Not Support yet!\n");
		}
		else if (strncmp(tokptr, "allgreenoff", 11) == 0) {
			if (0 == led_test_start)
				goto errout;
			printk("Not Support yet!\n");

		}
		else if (strncmp(tokptr, "allredon", 8) == 0) {
			printk("Not Support yet!\n");

/*
			led_on(LED_POWER_RED);
			led_on(LED_INTERNET_RED);
*/			
		}
		else if (strncmp(tokptr, "allredoff", 9) == 0) {
			printk("Not Support yet!\n");			
/*
			led_off(LED_POWER_RED);
			led_off(LED_INTERNET_RED);
*/			
		}
		else if(strncmp(tokptr, "clean", 5) == 0)
		{
			default_flag = '0';
#ifdef CONFIG_WLAN_ON_OFF_BUTTON			
			wlan_onoff_flag = '0';			
#endif
		}
		else
		{
			goto errout;
		}
	}
	else
	{
errout:
		printk("err command\n");
	}

	return len;
}


#endif


static int __init mptest_init(void) {

	struct proc_dir_entry *entry=NULL;	
	entry = create_proc_entry("mptest", 0, NULL);
	if (entry) {
		entry->read_proc = mptest_read_proc;
		entry->write_proc = mptest_write_proc;
	}
	else {
		printk("MP Test driver, create proc failed!\n");
	}
#ifdef CONFIG_APOLLO_MP_TEST
	led_test = create_proc_entry("led_test", 0, NULL);
	if(led_test != NULL)
	{
		led_test->read_proc = led_test_read;
		led_test->write_proc= led_test_write;
	}
#endif

	return 0;
}

static void __exit mptest_exit(void) {
}


module_init(mptest_init);
module_exit(mptest_exit);


MODULE_LICENSE("GPL");
