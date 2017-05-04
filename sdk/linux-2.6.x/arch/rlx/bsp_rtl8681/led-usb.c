#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include <asm/processor.h>
#include <bspchip.h>
//#include "led-generic.h"
#include "pushbutton.h"
#include "led-usb.h"

static struct led_struct led_usb0;
static struct led_struct led_usb1;


static void usb_led_act_timer_func(unsigned long data) {
	struct led_struct *p = (struct led_struct *)data;
	
	if (p->_counter>0) {

		p->state = !p->state;
		if (p->state) 
			led_on(p->led);
		else
			led_off(p->led);
		
		p->_counter--;
		mod_timer(&(p->timer), jiffies + p->cycle);
	}		
	else 
		led_on(p->led);
}


void usb_led_act_start(struct led_struct *p, unsigned int cycle, unsigned char counter) {

	p->cycle = cycle;
	p->_counter = counter;
	p->timer.expires = jiffies + p->cycle;
	mod_timer(&(p->timer), p->timer.expires);
}

void usb_act_func(unsigned char which, unsigned char LEDstate) {
	struct led_struct *p = (which==LED_USB_0)? &led_usb0: &led_usb1;

	switch (LEDstate)
	{
		case USB_LED_INPROG_BLINK:
			usb_led_act_start(p, HZ/4, 8);
			break;
		case USB_LED_STOP_BLINK:
			led_act_stop(p);
			led_off(p->led);
			break;
		default:
			break;
	}
		
}

static void usb_led_init_func(struct led_struct *p, unsigned char which){
	p->led = which;
	init_timer(&p->timer);
	p->timer.function = usb_led_act_timer_func;
	p->timer.data = (unsigned long) p;
}


static int __init led_usb_init(void) {
#ifdef CONFIG_SW_USB_LED0
	usb_led_init_func(&led_usb0, LED_USB_0);
#endif //CONFIG_SW_USB_LED0
#ifdef CONFIG_SW_USB_LED1
	usb_led_init_func(&led_usb1, LED_USB_1);
#endif	//CONFIG_SW_USB_LED1
	return 0;
}

static void __exit led_usb_exit(void) {
}


module_init(led_usb_init);
module_exit(led_usb_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPIO driver for Reload default");

