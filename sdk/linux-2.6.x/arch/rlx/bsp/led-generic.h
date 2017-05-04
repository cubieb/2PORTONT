#ifndef LED_GENERIC_H_
#define LED_GENERIC_H_

#include <linux/timer.h>

enum led_type {
	
	LED_POWER = 1, // do not use use LED_POWER_GREEN insted
	LED_POWER_GREEN,
	LED_POWER_RED,
	LED_DSL,
	LED_INTERNET, // do not use, use LED_INTERNET_GREEN insted
	LED_INTERNET_GREEN,
	LED_INTERNET_RED,
	LED_ALARM,
	LED_HANDSHAKING,
	LED_ACTIVITY,
	LED_PPP_GREEN,
	LED_PPP_RED,
	LED_WPS_GREEN,
	LED_WPS_RED,
	LED_WPS_YELLOW,
#ifdef CONFIG_RTL8672_SW_USB_LED //2010-11-19 paula
	LED_USB_0, 
	LED_USB_1, 
#endif
#ifdef CONFIG_RTK_VOIP
	LED_FXS1,
	LED_FXS2,
#endif
};

#define LED_ON		1
#define LED_OFF		2

struct led_operations {
	char *name;
	void (*handle_init)(void);
	void (*handle_set)(int, int);
};

struct led_struct {
	struct timer_list timer;	
	unsigned short cycle;	// on/off cycle, in 10ms
	unsigned char state;	// current LED state, for flashing on->off transition.
	unsigned char act_state;	// original state, for act. 
	unsigned char led;
	unsigned char backlog;
	unsigned char _counter;
	unsigned char disable;	// enable/disable LED timer function.
};

int led_register_operations(struct led_operations *);

// basic
void led_on(int which);
void led_off(int which);

/* 
*	make led flash, cycle specify the on/off cycle in 10ms..
*/
void led_flash_start(struct led_struct *p, int which, unsigned int cycle);
void led_flash_stop(struct led_struct *p);


/*
*	Activity LED function.
*	which: LED 
*	cycle: flash frequency in ms. 
*	backlog: max. _counter value.
*/
void led_act_start(struct led_struct *p, int which, unsigned int cycle, unsigned char backlog);
void led_act_stop(struct led_struct *p);
static inline void led_act_touch(struct led_struct *p) {
	if (p->_counter < p->backlog)
		p->_counter++;
}

static inline void led_handle_set(int which, int op) { };

#ifdef CONFIG_LED_TR068
void tr068_internet_led_start(void);
void tr068_internet_led_stop(void);
#endif

#endif

