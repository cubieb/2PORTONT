#include <linux/jiffies.h>
#include "led-generic.h"

static struct led_operations *led_op = 0;

void (*led_handshaking_func)(int) = 0;
void (*led_alarm_func)(int) = 0;
void (*led_act_func)(int) = 0;
void (*led_tr068_internet_act_func)(void) = 0;

#define DBG(fmt, ...) //printk(fmt, __VA_ARGS__)

static void led_flash_timer_func(unsigned long data) {
	struct led_struct *p = (struct led_struct *)data;

	if (!p->disable) {
	p->state = !p->state;//invert. maybe array is better.
	if (p->state)
		led_on(p->led);
	else
		led_off(p->led);
	}
	mod_timer(&(p->timer), jiffies + p->cycle);
}

static void led_generic_timer_start(struct led_struct *p,  void(*func)(unsigned long)) {
	init_timer(&p->timer);
	p->timer.function = func;
	p->timer.data = (unsigned long) p;
	p->timer.expires = jiffies + p->cycle;
	mod_timer(&(p->timer), p->timer.expires);
}

static void led_handle_flash_generic(struct led_struct *p,  unsigned short cycle) {
	del_timer(&(p->timer)); // stop it first.
	p->cycle = cycle;	
	led_generic_timer_start(p, led_flash_timer_func);
}



static void led_act_timer_func(unsigned long data) {
	struct led_struct *p = (struct led_struct *)data;
	
	if (p->_counter) {
		//printk(".");
		p->state = !p->state;//invert. maybe array is better.
		if (p->state) 
			led_on(p->led);
		else
			led_off(p->led);
		
		if (p->state == p->act_state)
			p->_counter--;
	}			
	mod_timer(&(p->timer), jiffies + p->cycle);
}
/*
void led_handle_act_start(struct led_struct *p, unsigned int cycle) {
	//p->backlog = 0;
	//p->cycle = HZ / 20;
	p->cycle = cycle;
	init_timer(&p->timer);
	p->state = p->act_state;
	p->timer.function = led_act_timer_func;
	p->timer.data = (unsigned long) p;
	p->timer.expires = jiffies + p->cycle;
	mod_timer(&(p->timer), p->timer.expires);
}

void led_handle_act_stop(struct led_struct *p) {
	p->_counter = 0;
	del_timer(&(p->timer)); 
}
*/

static void led_dummy_set(int which, int on) {
}

int led_register_operations(struct led_operations *ops) {
	if (led_op) {
		printk(KERN_ERR "LED device already registered by %s\n", led_op->name);
		return -1;
	}
	led_op = ops;
	if (!ops->handle_set)
		ops->handle_set = led_dummy_set;
	
	ops->handle_init();
	
	return 0;
}

void led_on(int which) {
	DBG("%s: %d\n", __FUNCTION__, which);
	if (led_op)
		led_op->handle_set(which, LED_ON);
}

void led_off(int which) {
	DBG("%s: %d\n", __FUNCTION__, which);
	if (led_op)
		led_op->handle_set(which, LED_OFF);
}

void led_flash_start(struct led_struct *p, int which, unsigned int cycle) {
	DBG("%s: led %d, cycle %d * 10ms\n", __FUNCTION__, which, cycle);
	p->led = which;
	led_handle_flash_generic(p, cycle);
}

void led_flash_stop(struct led_struct *p) {
	DBG("%s: led %d\n", __FUNCTION__, p->led);
	del_timer(&(p->timer)); 
}

void led_act_start(struct led_struct *p, int which, unsigned int cycle, unsigned char backlog) {
	led_act_stop(p);
	p->cycle = cycle;
	p->led = which;
	p->backlog = backlog;
	init_timer(&p->timer);
	p->state = p->act_state = 1;
	p->timer.function = led_act_timer_func;
	p->timer.data = (unsigned long) p;
	p->timer.expires = jiffies + p->cycle;
	mod_timer(&(p->timer), p->timer.expires);
}

void led_act_stop(struct led_struct *p) {
	p->_counter = 0;
	del_timer(&(p->timer));
}


// for adsl, actually the name should be LEDHandshaking.. GPIO does not give propriate abstraction..
/*
void gpioHandshaking(int flag) {
	if (led_handshaking_func) {
		led_handshaking_func(flag);
		return;
	}
		
	if (flag == 1) 
		led_on(LED_DSL);
	else
		led_off(LED_DSL);
}

void gpioACT(int flag) {
	led_act(LED_ACTIVITY);
}

void gpioAlarm(int flag) {
	if (flag)
		led_on(LED_ALARM);
	else
		led_off(LED_ALARM);	
}
*/
