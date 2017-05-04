#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include <asm/processor.h>
//#include <platform.h>
/*linux-2.6.19*/
#include <bspchip.h> 
#include "led-generic.h"

static struct led_struct led_flash_dsl;
static struct led_struct led_internet_act;

#ifdef CONFIG_RTK_VOIP
static struct led_struct led_fxs1;
static struct led_struct led_fxs2;
#endif

void tr068_internet_traffic(void) {
	led_act_touch(&led_internet_act);
}


void tr068_internet_led_start(void) {
	//printk("%s\n", __FUNCTION__);
	led_act_start(&led_internet_act, LED_INTERNET_GREEN, HZ / 20, 10);
	//led_flash_start(&led_internet_act, tr069_internet_flash_func);
}


void tr068_internet_led_stop(void) {
	//printk("%s\n", __FUNCTION__);
	led_act_stop(&led_internet_act);
	//led_internet_act._counter = 0;
	//del_timer(&(led_internet_act.timer));
}

char adsl_showtime = 0;

void ADSL_state(unsigned char LEDstate)
{
	static unsigned char LastStatus = 255;
	static char prev_adsl_showtime = 0;
	if(LastStatus == LEDstate)
		return;
		
	LastStatus = LEDstate;
	adsl_showtime = 0;
	switch((unsigned char)LEDstate){
		case C_AMSW_IDLE:
		case C_AMSW_L3:
		case C_AMSW_ACTIVATING:
		case C_AMSW_END_OF_LD:
			led_off(LED_INTERNET_GREEN);
			led_off(LED_INTERNET_RED);
			tr068_internet_led_stop();
			led_flash_start(&led_flash_dsl, LED_DSL, HZ / 4);
			break;
					
		case C_AMSW_INITIALIZING:			
			led_flash_start(&led_flash_dsl, LED_DSL, HZ / 8);		
			break;
			
		case C_AMSW_SHOWTIME_L0:
			led_flash_stop(&led_flash_dsl); 
			led_on(LED_DSL);
			adsl_showtime = 1;
			break;
	}	
	if (prev_adsl_showtime != adsl_showtime) {
		extern void internet_led_check(void);
		prev_adsl_showtime = adsl_showtime;
		internet_led_check();
		#if 0
		if (adsl_showtime)
			blocking_notifier_call_chain(&dsl_chain,
						NETDEV_UP, 0);
		else
			blocking_notifier_call_chain(&dsl_chain,
						NETDEV_DOWN, 0);
		#endif
	}
}	
#ifdef CONFIG_RTK_VOIP
typedef enum {
    VOIP_LED_OFF, // registration not OK
    VOIP_LED_ON, // registration OK
    VOIP_LED_START_BLINK, // Offhook
    VOIP_LED_STOP_BLINK_RESTORE, //Onhook
} _LED_STATE;

static int led_write_proc(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
    char led_control[2];

    if (buffer && !copy_from_user(&led_control, buffer, sizeof(led_control)))
	{
		if (led_control[0] == 0) // FXS 1
		{
			switch (led_control[1])
			{
				case VOIP_LED_OFF:
				led_off (LED_FXS1);
				break;	
				case VOIP_LED_ON:
				led_on (LED_FXS1);
				break;	
				case VOIP_LED_START_BLINK:
				led_flash_start(&led_fxs1, LED_FXS1, HZ/2);
				break;	
				case VOIP_LED_STOP_BLINK_RESTORE:
				led_flash_stop(&led_fxs1);
				break;	
			}
		}

		if (led_control[0] == 1) // FXS 2
		{
			switch (led_control[1])
			{
				case VOIP_LED_OFF:
				led_off (LED_FXS2);
				break;	
				case VOIP_LED_ON:
				led_on (LED_FXS2);
				break;	
				case VOIP_LED_START_BLINK:
				led_flash_start(&led_fxs2, LED_FXS2, HZ/2);
				break;	
				case VOIP_LED_STOP_BLINK_RESTORE:
				led_flash_stop(&led_fxs2);
				break;	
			}
		}

	}
	return count;
}
#endif


static int __init led_tr068_init(void) {
	
	//led_flash_dsl.led = LED_DSL;			
	//led_flash_dsl.state = 0; // off
	
	led_internet_act.led = LED_INTERNET_GREEN;
	led_internet_act.act_state = 1; // Default is ON
	led_internet_act.cycle = HZ /20;
	led_internet_act.backlog = 10;
	
#ifdef CONFIG_RTK_VOIP
	struct proc_dir_entry *entry=NULL;
	entry = create_proc_entry("led_fxs", 0, NULL);
	if (entry) {
		entry->write_proc = led_write_proc;
	}
	else {
		printk("Realtek Driver, create proc for FXS-Led failed!\n");
	}
#endif
	
	return 0;
}

static void __exit led_tr068_exit(void) {
}


module_init(led_tr068_init);
module_exit(led_tr068_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPIO driver for Reload default");

