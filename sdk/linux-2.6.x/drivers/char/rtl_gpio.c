/*
 * FILE NAME rtl_gpio.c
 *
 * BRIEF MODULE DESCRIPTION
 *  GPIO For Flash Reload Default
 *
 *  Author: jimmylin@realtek.com.tw
 *
 * Copyright 2005 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE	LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */



#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/reboot.h>
#include <linux/kmod.h>
#include <linux/proc_fs.h>
//#include  "bspchip.h"
#define AUTO_CONFIG

// 2009-0414
//#define	DET_WPS_SPEC
#define USE_INTERRUPT_GPIO

#if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198)
	#define READ_RF_SWITCH_GPIO
#endif

#if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198)
	#include "drivers/net/rtl819x/AsicDriver/rtl865xc_asicregs.h"
/*define the GPIO physical address to customer_gpio.h*/
#if defined(CONFIG_RTL_8196C)	
	#define RESET_PIN_IOBASE PABCD_CNR 	//RTL_GPIO_PABCD_CNR
	#define RESET_PIN_DIRBASE PABCD_DIR	//RTL_GPIO_PABCD_DIR 
	#define RESET_PIN_DATABASE PABCD_DAT //RTL_GPIO_PABCD_DATA
	#define RESET_PIN_NO 0 /*number of the ABCD*/

	#define RESET_LED_IOBASE PABCD_CNR 	//RTL_GPIO_PABCD_CNR
	#define RESET_LED_DIRBASE PABCD_DIR	//RTL_GPIO_PABCD_DIR 
	#define RESET_LED_DATABASE PABCD_DAT //RTL_GPIO_PABCD_DATA
	#define RESET_LED_NO 18 /*number of the ABCD*/

	#define AUTOCFG_LED_IOBASE PABCD_CNR 	//RTL_GPIO_PABCD_CNR
	#define AUTOCFG_LED_DIRBASE PABCD_DIR	//RTL_GPIO_PABCD_DIR 
	#define AUTOCFG_LED_DATABASE PABCD_DAT //RTL_GPIO_PABCD_DATA
	#define AUTOCFG_LED_NO 20 /*number of the ABCD*/

	#define AUTOCFG_PIN_IOBASE PABCD_CNR 	//RTL_GPIO_PABCD_CNR
	#define AUTOCFG_PIN_DIRBASE PABCD_DIR	//RTL_GPIO_PABCD_DIR 
	#define AUTOCFG_PIN_DATABASE PABCD_DAT //RTL_GPIO_PABCD_DATA
	#define AUTOCFG_PIN_NO 1 /*number of the ABCD)*/
	#define AUTOCFG_PIN_IMR PAB_IMR
	
#elif defined(CONFIG_RTL_8198)
	#define RESET_PIN_IOBASE PEFGH_CNR 	//RTL_GPIO_PABCD_CNR
	#define RESET_PIN_DIRBASE PEFGH_DIR	//RTL_GPIO_PABCD_DIR 
	#define RESET_PIN_DATABASE PEFGH_DAT //RTL_GPIO_PABCD_DATA
	#define RESET_PIN_NO 25 /*number of the ABCD*/

	#define RESET_LED_IOBASE PEFGH_CNR 	//RTL_GPIO_PABCD_CNR
	#define RESET_LED_DIRBASE PEFGH_DIR	//RTL_GPIO_PABCD_DIR 
	#define RESET_LED_DATABASE PEFGH_DAT //RTL_GPIO_PABCD_DATA
	#define RESET_LED_NO 27 /*number of the ABCD*/

	#define AUTOCFG_LED_IOBASE PEFGH_CNR 	//RTL_GPIO_PABCD_CNR
	#define AUTOCFG_LED_DIRBASE PEFGH_DIR	//RTL_GPIO_PABCD_DIR 
	#define AUTOCFG_LED_DATABASE PEFGH_DAT //RTL_GPIO_PABCD_DATA
	#define AUTOCFG_LED_NO 20 /*number of the ABCD*/

	#define AUTOCFG_PIN_IOBASE PEFGH_CNR 	//RTL_GPIO_PABCD_CNR
	#define AUTOCFG_PIN_DIRBASE PEFGH_DIR	//RTL_GPIO_PABCD_DIR 
	#define AUTOCFG_PIN_DATABASE PEFGH_DAT //RTL_GPIO_PABCD_DATA
	#define AUTOCFG_PIN_NO 1 /*number of the ABCD)*/
	#define AUTOCFG_PIN_IMR PGH_IMR
	
#endif
	#define WIFI_ONOFF_PIN_IOBASE PABCD_CNR 	//RTL_GPIO_PABCD_CNR
	#define WIFI_ONOFF_PIN_DIRBASE PABCD_DIR	//RTL_GPIO_PABCD_DIR 
	#define WIFI_ONOFF_PIN_DATABASE PABCD_DAT //RTL_GPIO_PABCD_DATA
	#define WIFI_ONOFF_PIN_NO 19/*umber of the ABCD)*/
	#define WIFI_ONOFF_PIN_IMR PAB_IMR

/*
	#define RTL_GPIO_MUX 0xB8000030
	#define RTL_GPIO_MUX_DATA 0x0FC00380//for WIFI ON/OFF and GPIO

	

	#define AUTOCFG_BTN_PIN	AUTOCFG_PIN_NO	// 1
	#define AUTOCFG_LED_PIN		AUTOCFG_LED_NO//20
	#define RESET_LED_PIN		RESET_LED_NO //18
	#define RESET_BTN_PIN		RESET_PIN_NO //0
	#define RTL_GPIO_WIFI_ONOFF	WIFI_ONOFF_PIN_NO
*/
	 #define RTL_GPIO_MUX 0xB8000040
	 #define RTL_GPIO_MUX_DATA 0x00340C00//for WIFI ON/OFF and GPIO
	 #define RTL_GPIO_WIFI_ONOFF     19

#if defined(CONFIG_RTL_8196C)	 
	 #define AUTOCFG_BTN_PIN         3
	 #define AUTOCFG_LED_PIN         4
	 #define RESET_LED_PIN           6
	 #define RESET_BTN_PIN           5
#elif defined(CONFIG_RTL_8198)
	 #define AUTOCFG_BTN_PIN         24
	 #define AUTOCFG_LED_PIN         26
	 #define RESET_LED_PIN           27
	 #define RESET_BTN_PIN           25
#endif	 

#endif



#ifdef USE_INTERRUPT_GPIO
	#define GPIO_IRQ_NUM		1
#endif


// 2009-0414
#ifdef USE_INTERRUPT_GPIO
	#define GPIO_IRQ_NUM		1	
#endif

	#define PROBE_TIME	5


#define PROBE_NULL		0
#define PROBE_ACTIVE	1
#define PROBE_RESET		2
#define PROBE_RELOAD	3
#define RTL_R32(addr)		(*(volatile unsigned long *)(addr))
#define RTL_W32(addr, l)	((*(volatile unsigned long*)(addr)) = (l))
#define RTL_R8(addr)		(*(volatile unsigned char*)(addr))
#define RTL_W8(addr, l)		((*(volatile unsigned char*)(addr)) = (l))

//#define  GPIO_DEBUG
#ifdef GPIO_DEBUG
/* note: prints function name for you */
#  define DPRINTK(fmt, args...) printk("%s: " fmt, __FUNCTION__ , ## args)
#else
#  define DPRINTK(fmt, args...)
#endif

static struct timer_list probe_timer;
static unsigned int    probe_counter;
static unsigned int    probe_state;

static char default_flag='0';
//Brad add for update flash check 20080711
int start_count_time=0;
int Reboot_Wait=0;




//#ifdef CONFIG_RTL865X_AC


#ifdef	USE_INTERRUPT_GPIO
static int wps_button_push = 0;

#endif

#ifdef AUTO_CONFIG
static unsigned int		AutoCfg_LED_Blink;
static unsigned int		AutoCfg_LED_Toggle;

void autoconfig_gpio_init(void)
{

	RTL_W32(AUTOCFG_PIN_IOBASE,(RTL_R32(AUTOCFG_PIN_IOBASE)&(~(1 << AUTOCFG_BTN_PIN))));
	RTL_W32(AUTOCFG_LED_IOBASE,(RTL_R32(AUTOCFG_LED_IOBASE)&(~(1 << AUTOCFG_LED_PIN))));


	// Set GPIOA pin 1 as input pin for auto config button
	RTL_W32(AUTOCFG_PIN_DIRBASE, (RTL_R32(AUTOCFG_PIN_DIRBASE) & (~(1 << AUTOCFG_BTN_PIN))));

	// Set GPIOA ping 3 as output pin for auto config led
	RTL_W32(AUTOCFG_LED_DIRBASE, (RTL_R32(AUTOCFG_LED_DIRBASE) | (1 << AUTOCFG_LED_PIN)));

	// turn off auto config led in the beginning
	RTL_W32(AUTOCFG_LED_DATABASE, (RTL_R32(AUTOCFG_LED_DATABASE) | (1 << AUTOCFG_LED_PIN)));
}
#ifdef CONFIG_RTL_8196C_GW_MP

void all_led_on(void)
{
	//printk("Into MP GPIO");
	 #ifdef CONFIG_RTL_8196C_GW_MP
	RTL_W32(0xB8000030, (RTL_R32(0xB8000030) | 0x00F00F80 ));
	RTL_W32(PABCD_CNR, (RTL_R32(PABCD_CNR) & (~(1 << PS_LED_GREEN_PIN))));
	RTL_W32(PABCD_CNR, (RTL_R32(PABCD_CNR) & (~(1 << PS_LED_ORANGE_PIN))));

	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) | (1 << PS_LED_GREEN_PIN)));
	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) | (1 << PS_LED_ORANGE_PIN)));
	
	RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << PS_LED_GREEN_PIN)));
	RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << PS_LED_ORANGE_PIN))));

	/* inet_led init setting */
	RTL_W32(PABCD_CNR, (RTL_R32(PABCD_CNR) & (~(1 << INET_LED_GREEN_PIN))));
	RTL_W32(PABCD_CNR, (RTL_R32(PABCD_CNR) & (~(1 << INET_LED_ORANGE_PIN))));

	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) | (1 << INET_LED_GREEN_PIN)));
	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) | (1 << INET_LED_ORANGE_PIN)));
	
	RTL_W32(AUTOCFG_LED_DIRBASE, (RTL_R32(AUTOCFG_LED_DIRBASE) & (~(1 << AUTOCFG_LED_PIN_MP))));

	RTL_W32(PABCD_DIR, (RTL_R32(PABCD_DIR) | (1 << AUTOCFG_LED_PIN_MP)));

	//RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << INET_LED_GREEN_PIN))));
	//RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | (1 << INET_LED_ORANGE_PIN)));
	RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << PS_LED_GREEN_PIN))));	
	RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << PS_LED_ORANGE_PIN))));
	RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << INET_LED_GREEN_PIN))));
	RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << INET_LED_ORANGE_PIN))));
	RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~(1 << AUTOCFG_LED_PIN_MP))));
	RTL_W32(RESET_LED_DATABASE, (RTL_R32(RESET_LED_DATABASE) & (~(1 << RESET_LED_PIN_MP))));
	#endif

}
#endif
void autoconfig_gpio_off(void)
{
	RTL_W32(AUTOCFG_LED_DATABASE, (RTL_R32(AUTOCFG_LED_DATABASE) | (1 << AUTOCFG_LED_PIN)));
	AutoCfg_LED_Blink = 0;
}


void autoconfig_gpio_on(void)
{
	RTL_W32(AUTOCFG_LED_DATABASE, (RTL_R32(AUTOCFG_LED_DATABASE) & (~(1 << AUTOCFG_LED_PIN))));
	AutoCfg_LED_Blink = 0;
}


void autoconfig_gpio_blink(void)
{
	RTL_W32(AUTOCFG_LED_DATABASE, (RTL_R32(AUTOCFG_LED_DATABASE) & (~(1 << AUTOCFG_LED_PIN))));

	AutoCfg_LED_Blink = 1;
	AutoCfg_LED_Toggle = 1;

}



#endif // AUTO_CONFIG




static void rtl_gpio_timer(unsigned long data)
{
	unsigned int pressed=1;


#if  defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198)
	if (RTL_R32(RESET_PIN_DATABASE) & (1 << RESET_BTN_PIN))
#endif
	{
		pressed = 0;

		//turn off LED0
		#ifndef CONFIG_RTL_8196C_GW_MP
		RTL_W32(RESET_LED_DATABASE, (RTL_R32(RESET_LED_DATABASE) | ((1 << RESET_LED_PIN))));
		#endif
	}
	else
	{
		DPRINTK("Key pressed %d!\n", probe_counter+1);
	}

	if (probe_state == PROBE_NULL)
	{
		if (pressed)
		{
			probe_state = PROBE_ACTIVE;
			probe_counter++;
		}
		else
			probe_counter = 0;
	}
	else if (probe_state == PROBE_ACTIVE)
	{
		if (pressed)
		{
			probe_counter++;

			if ((probe_counter >=2 ) && (probe_counter <=PROBE_TIME))
			{


				DPRINTK("2-5 turn on led\n");
				//turn on LED0
				RTL_W32(RESET_LED_DATABASE, (RTL_R32(RESET_LED_DATABASE) & (~(1 << RESET_LED_PIN))));


			}
			else if (probe_counter >= PROBE_TIME)
			{

				// sparkling LED0
				DPRINTK(">5 \n");

			if (probe_counter & 1)
					RTL_W32(RESET_LED_DATABASE, (RTL_R32(RESET_LED_DATABASE) | ((1 << RESET_LED_PIN))));
				else
					RTL_W32(RESET_LED_DATABASE, (RTL_R32(RESET_LED_DATABASE) & (~(1 << RESET_LED_PIN))));


			}
		}
		else
		{
			#if defined(CONFIG_RTL865X_SC)
			if (probe_counter < 5)
			#else
			if (probe_counter < 2)
			#endif
			{
				probe_state = PROBE_NULL;
				probe_counter = 0;
				DPRINTK("<2 \n");
				#if defined(CONFIG_RTL865X_SC)
				ResetToAutoCfgBtn = 1;
				#endif
			}
			else if (probe_counter >= PROBE_TIME)
			{


				//reload default
				default_flag = '1';

				//kernel_thread(reset_flash_default, (void *)1, SIGCHLD);
				return;

			}
			else
			{
				DPRINTK("2-5 reset\n");
			#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
				kill_proc(1,SIGTERM,1);
			#else
				kill_pid(1,SIGTERM,1);
			#endif
				//kernel_thread(reset_flash_default, 0, SIGCHLD);
				return;
			}
		}
	}

#ifdef AUTO_CONFIG
	if (AutoCfg_LED_Blink==1)
	{
		if (AutoCfg_LED_Toggle) {
			RTL_W32(AUTOCFG_LED_DATABASE, (RTL_R32(AUTOCFG_LED_DATABASE) | (1 << AUTOCFG_LED_PIN)));
		}
		else {
			RTL_W32(AUTOCFG_LED_DATABASE, (RTL_R32(AUTOCFG_LED_DATABASE) & (~(1 << AUTOCFG_LED_PIN))));
		}
		AutoCfg_LED_Toggle = AutoCfg_LED_Toggle? 0 : 1;
	}
#endif


	mod_timer(&probe_timer, jiffies + 100);

}

#ifdef AUTO_CONFIG
#if defined(USE_INTERRUPT_GPIO)
static void gpio_interrupt_isr(int irq, void *dev_instance, struct pt_regs *regs)
{
	wps_button_push = 1;   	  	
  	RTL_W32(PABCD_ISR, RTL_R32(PABCD_ISR)); 	
}
#endif

static int read_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;
	char flag;

#if  defined(USE_INTERRUPT_GPIO)
// 2009-0414		
	if (wps_button_push) {
		flag = '1';
		wps_button_push = 0;		
	}
	else{
		if (RTL_R32(AUTOCFG_PIN_DATABASE) & (1 << AUTOCFG_BTN_PIN)){
			flag = '0';
		}else{
			//panic_printk("wps button be held \n");
			flag = '1';
		}

	}
// 2009-0414		
#else

	if (RTL_R32(AUTOCFG_PIN_DATABASE) & (1 << AUTOCFG_BTN_PIN))
		flag = '0';
	else {
		flag = '1';
	}
#endif // CONFIG_RTL865X_KLD		
		
	len = sprintf(page, "%c\n", flag);


	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;
	return len;
}



#ifdef CONFIG_RTL_KERNEL_MIPS16_CHAR
__NOMIPS16
#endif 
static int write_proc(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	char flag[20];
//Brad add for update flash check 20080711

	char start_count[10], wait[10];

	if (count < 2)
		return -EFAULT;

	DPRINTK("file: %08x, buffer: %s, count: %lu, data: %08x\n",
		(unsigned int)file, buffer, count, (unsigned int)data);

	if (buffer && !copy_from_user(&flag, buffer, 1)) {
		if (flag[0] == 'E') {
			autoconfig_gpio_init();
			#ifdef CONFIG_RTL865X_CMO
			extra_led_gpio_init();
			#endif
		}
		else if (flag[0] == '0')
			autoconfig_gpio_off();
		else if (flag[0] == '1')
			autoconfig_gpio_on();
		else if (flag[0] == '2')
			autoconfig_gpio_blink();
#ifdef CONFIG_RTL_8196C_GW_MP
		else if (flag[0] == '9') // disable system led
                {
			all_led_on();
		}
#endif	

#ifdef CONFIG_RTL865X_CMO
		else if (flag[0] == '3')
			wep_wpa_led_on();
		else if (flag[0] == '5')
			wep_wpa_led_off();
		else if (flag[0] == '6')
			mac_ctl_led_on();
		else if (flag[0] == '7')
			mac_ctl_led_off();
		else if (flag[0] == '8')
			bridge_repeater_led_on();
		else if (flag[0] == '9')
			bridge_repeater_led_off();
		else if (flag[0] == 'A')
			system_led_on();
//		else if (flag[0] == 'B')
//			system_led_off();
		else if (flag[0] == 'C')
			lan_led_on();
		else if (flag[0] == 'D')
			lan_led_off();
		else if (flag[0] == 'Z')
			printk("gpio test test\n");
//		else if (flag[0] == '9')
//			system_led_blink = 2;
#endif

//Brad add for update flash check 20080711

		else if (flag[0] == '4'){
			start_count_time= 1;
			sscanf(buffer, "%s %s", start_count, wait);
			Reboot_Wait = (simple_strtol(wait,NULL,0))*100;
		}


		else
			{}

		return count;
	}
	else
		return -EFAULT;
}
#endif // AUTO_CONFIG

static int default_read_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "%c\n", default_flag);
	if (len <= off+count) *eof = 1;
	  *start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	  return len;
}

#ifdef CONFIG_RTL_KERNEL_MIPS16_CHAR
__NOMIPS16
#endif 
static int default_write_proc(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	if (count < 2)
		return -EFAULT;
	if (buffer && !copy_from_user(&default_flag, buffer, 1)) {
		return count;
	}
	return -EFAULT;
}

#ifdef READ_RF_SWITCH_GPIO
static int rf_switch_read_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;
	char flag;

	if (RTL_R32(WIFI_ONOFF_PIN_DATABASE) & (1<<WIFI_ONOFF_PIN_NO)){
		flag = '1';
	}else{
		flag = '0';
	}
	len = sprintf(page, "%c\n", flag);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;
	return len;
}
#endif



int __init rtl_gpio_init(void)
{
	struct proc_dir_entry *res=NULL;

	printk("Realtek GPIO Driver for Flash Reload Default\n");

	// Set GPIOA pin 10(8181)/0(8186) as input pin for reset button

#if  defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198)
	RTL_W32(RTL_GPIO_MUX, (RTL_R32(RTL_GPIO_MUX) | (RTL_GPIO_MUX_DATA)));
#if defined(CONFIG_RTL_8198)
	RTL_W32(RTL_GPIO_MUX, (RTL_R32(RTL_GPIO_MUX) | 0xf));
#endif
	RTL_W32(RESET_PIN_IOBASE, (RTL_R32(RESET_PIN_IOBASE) & (~(1 << RESET_BTN_PIN))));
	RTL_W32(RESET_PIN_DIRBASE, (RTL_R32(RESET_PIN_DIRBASE) & (~(1 << RESET_BTN_PIN))));
#if defined(READ_RF_SWITCH_GPIO)
	RTL_W32(WIFI_ONOFF_PIN_DIRBASE, (RTL_R32(WIFI_ONOFF_PIN_DIRBASE) & ( ~(1<<RTL_GPIO_WIFI_ONOFF))));
	RTL_W32(WIFI_ONOFF_PIN_DIRBASE, (RTL_R32(WIFI_ONOFF_PIN_DIRBASE) & (~(1<<RTL_GPIO_WIFI_ONOFF))));
	RTL_W32(WIFI_ONOFF_PIN_DATABASE, (RTL_R32(WIFI_ONOFF_PIN_DATABASE) & (~(1<<RTL_GPIO_WIFI_ONOFF))));

#endif // #if defined(READ_RF_SWITCH_GPIO)
#endif // #if defined(CONFIG_RTL865X)

	// Set GPIOA ping 2 as output pin for reset led
	RTL_W32(RESET_LED_DIRBASE, (RTL_R32(RESET_LED_DIRBASE) | ((1 << RESET_LED_PIN))));


#ifdef AUTO_CONFIG
	res = create_proc_entry("gpio", 0, NULL);
	if (res) {
		res->read_proc = read_proc;
		res->write_proc = write_proc;
	}
	else {
		printk("Realtek GPIO Driver, create proc failed!\n");
	}

// 2009-0414		
#if defined(USE_INTERRUPT_GPIO)
#if defined(CONFIG_RTL_8198)
	RTL_R32(AUTOCFG_PIN_IMR) |= (0x01 << (AUTOCFG_BTN_PIN-16)*2); // enable interrupt in falling-edge		
#else
	RTL_R32(AUTOCFG_PIN_IMR) |= (0x01 << AUTOCFG_BTN_PIN*2); // enable interrupt in falling-edge	
#endif
	if (request_irq(GPIO_IRQ_NUM, gpio_interrupt_isr, IRQF_SHARED, "rtl_gpio", NULL)) {	  
		//panic_printk("gpio request_irq(%d) error!\n", GPIO_IRQ_NUM);		
   	}
#endif
// 2009-0414		
#endif

	res = create_proc_entry("load_default", 0, NULL);
	if (res) {
		res->read_proc = default_read_proc;
		res->write_proc = default_write_proc;
	}

#ifdef READ_RF_SWITCH_GPIO
	res = create_proc_entry("rf_switch", 0, NULL);
	if (res) {
		res->read_proc = rf_switch_read_proc;
		res->write_proc = NULL;
	}
#endif



	init_timer(&probe_timer);
	probe_counter = 0;
	probe_state = PROBE_NULL;
	probe_timer.expires = jiffies + 100;
	probe_timer.data = (unsigned long)NULL;
	probe_timer.function = &rtl_gpio_timer;
	mod_timer(&probe_timer, jiffies + 100);

#ifdef CONFIG_RTL865X_CMO
	extra_led_gpio_init();
#endif
	return 0;
}


static void __exit rtl_gpio_exit(void)
{
	printk("Unload Realtek GPIO Driver \n");
	del_timer_sync(&probe_timer);
}


module_exit(rtl_gpio_exit);
module_init(rtl_gpio_init);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPIO driver for Reload default");
