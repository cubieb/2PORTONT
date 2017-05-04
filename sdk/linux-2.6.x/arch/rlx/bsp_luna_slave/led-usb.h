#ifndef LED_USB_H_
#define LED_USB_H_

extern void usb_act_func(unsigned char which, unsigned char LEDstate); 

enum USB_LED_STATE
{
	USB_LED_INPROG_BLINK=0,
	USB_LED_STOP_BLINK=1
};

#include "led-generic.h"

#endif

