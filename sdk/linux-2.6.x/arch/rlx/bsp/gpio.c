/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2003  
* All rights reserved.
* 
* Abstract: GPIO driver source code.
*
* ---------------------------------------------------------------
*/

#include <linux/config.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mman.h>
#include <linux/ioctl.h>
#include <linux/fd.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/serial_core.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/byteorder.h>

#if defined(CONFIG_RTL8670)
#include "lx4180.h"
#elif defined(CONFIG_RTL8671)// 8671
#include "lx5280.h"
#else
//#include <platform.h>	//shlee 2.6
/*linux-2.6.19*/
#include <bspchip.h>
#endif
#include "gpio.h"


//#define	CONFIG_LED_TR068	1


//alex
//#define CONFIG_E8BGPIO 1
extern int g_internet_up;
extern int g_ppp_up;
void  gpio_led_set(void);

int led0enable=1; //for rtl8185 driver
static unsigned int GPIOdataReg[GPIO_DATA_NUM];

#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE)
static unsigned char use_rtl8192cd_gpio = 0;

void set_rtl8192cd_gpio(void)
{
	if(use_rtl8192cd_gpio)
		REG32(RTL8192CD_GPIO_PIN_CTRL) = GPIOdataReg[GPIO_RTL8192CD];

	return;
}
EXPORT_SYMBOL(set_rtl8192cd_gpio);
#endif //CONFIG_RTL8192CD


/*
Check BD800000 to identify which GPIO pins can be used
*/
unsigned int get_GPIOMask(void)
{
	unsigned int portMask=0xFFFFFFFF;

#if  !defined(CONFIG_EXT_SWITCH)&& !defined(CONFIG_GPIO_LED_CHT_E8B)
	//portMask &= ~(GPIO_PB3|GPIO_PB4|GPIO_PB5|GPIO_PB6|GPIO_PB7);  //disable B3-B7
#endif

	return portMask;
}

/*
Config one GPIO pin. Release 1 only support output function
number and PIN location map:
Pin	num
PH7  63
:      :
PE0  32
:      :
PB7	15
:	:
PB0	8
PA7	7
:	:
PA0	0
*/
void gpioConfig (int gpio_num, int gpio_func)
{
	unsigned int useablePin;
	unsigned int mask;
  
	//printk( "<<<<<<<<<enter gpioConfig(gpio_num:%d, gpio_func:%d)\n", gpio_num, gpio_func );
	
	if ((gpio_num >= GPIO_END)||(gpio_num < 0)) return;

	if (gpio_num <= GPIO_D_7) {
		useablePin = get_GPIOMask();
		mask = 1 << gpio_num;
		if ((useablePin & mask) == 0) {  //GPIO pins is shared by other modules
			printk("GPIO config Error! PIN %d is used by a hardware module\n",gpio_num);
			return;
		}

		if (GPIO_FUNC_INPUT == gpio_func)
			REG32(GPIO_PABCD_DIR) &= ~mask;
		else
	        REG32(GPIO_PABCD_DIR) |= mask;
	}
	else if (gpio_num <= GPIO_H_7) {
		mask = 1 << (gpio_num - GPIO_E_0);

		if (GPIO_FUNC_INPUT == gpio_func)
			REG32(GPIO_PEFGH_DIR) &= ~mask;
		else
	        REG32(GPIO_PEFGH_DIR) |= mask;
	}
#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE)
	else if (gpio_num <= RTL8192CD_GPIO_7) {
		use_rtl8192cd_gpio = 1;
		mask = 1 << (RTL8192CD_GPIO_IO_SEL_OFFSET + gpio_num - RTL8192CD_GPIO_0);

		if (GPIO_FUNC_INPUT == gpio_func) {
			REG32(RTL8192CD_GPIO_PIN_CTRL) &= ~mask;
			GPIOdataReg[GPIO_RTL8192CD] &= ~mask;
		}
		else {
	        REG32(RTL8192CD_GPIO_PIN_CTRL) |= mask;
			GPIOdataReg[GPIO_RTL8192CD] |= mask;
		}
	}
#endif //CONFIG_RTL8192CD
#ifdef RTL_EXT_GPO
	if (gpio_num >= GPIO_EXT_0 && gpio_num <= GPIO_EXT_3) {
		if (GPIO_FUNC_OUTPUT == gpio_func) {
			REG32(BSP_MISC_CR_BASE) |= BSP_AFED_GPO_SEL; // set as GPO pin
		}
		else {
			// not support input mode.
		}
	}
#endif
}
EXPORT_SYMBOL(gpioConfig);

void gpioSetGIMR(int irq_num, int value)
{
	if(value == 0)
		REG32(BSP_GIMR) &= (~(1 << irq_num));
	else
		REG32(BSP_GIMR) |= (1 << irq_num);
}

//ccwei: 120208-WPS (set gpio interrupt mask register)
void gpioSetIMR(int gpio_num, int value)
{
	unsigned int shift;

	if ((gpio_num >= GPIO_END)||(gpio_num < 0)){
		printk("%s(%d): wrong gpio num %d\n", __FUNCTION__, __LINE__,gpio_num);
		return;
	}
	
	value &= 0x3;
	if (gpio_num <= GPIO_B_7){
		shift = ((gpio_num - GPIO_A_0)<<1);
		REG32(GPIO_PAB_IMR) &= (~(EN_BOTH_EDGE_ISR << shift));
		if (value)
			REG32(GPIO_PAB_IMR) |= (value << shift);
	}else if (gpio_num <= GPIO_D_7){
		shift = ((gpio_num - GPIO_C_0)<<1);
		REG32(GPIO_PCD_IMR) &= (~(EN_BOTH_EDGE_ISR << shift));
		if (value)
			REG32(GPIO_PCD_IMR) |= (value << shift);
	}else if (gpio_num <= GPIO_F_7){
		shift = ((gpio_num - GPIO_E_0)<<1);	
		REG32(GPIO_PEF_IMR) &= (~(EN_BOTH_EDGE_ISR << shift));
		if (value)
			REG32(GPIO_PEF_IMR) |= (value << shift);
	}else if (gpio_num <= GPIO_H_7){
		shift = ((gpio_num - GPIO_G_0)<<1);	
		REG32(GPIO_PGH_IMR) &= (~(EN_BOTH_EDGE_ISR << shift));
		if (value)
			REG32(GPIO_PGH_IMR) |= (value << shift);
	}			
}

unsigned int gpioReadISR(int gpio_num)
{
	if ((gpio_num >= GPIO_END)||(gpio_num < 0)){
		printk("%s(%d): wrong gpio num %d\n", __FUNCTION__, __LINE__,gpio_num);
		return (unsigned int)-1;
	}
	
	if (gpio_num <= GPIO_D_7) {
		return REG32(GPIO_PABCD_ISR);
	}else if (gpio_num <= GPIO_H_7){
		return REG32(GPIO_PEFGH_ISR);
	}
	
	return (unsigned int)-1;
}

void gpioClearISR(int gpio_num)
{
	if ((gpio_num >= GPIO_END)||(gpio_num < 0)){
		printk("%s(%d): wrong gpio num %d\n", __FUNCTION__, __LINE__,gpio_num);
		return;
	}
	
	if (gpio_num <= GPIO_D_7){
		REG32(GPIO_PABCD_ISR) |= BIT(gpio_num - GPIO_A_0);
	}else if (gpio_num <= GPIO_H_7){
		REG32(GPIO_PEFGH_ISR) |= BIT(gpio_num - GPIO_E_0);
	}
}

int gpioGetBspIRQNum(int gpio_num)
{
	if ((gpio_num >= GPIO_END)||(gpio_num < 0)){
		printk("%s(%d): wrong gpio num %d\n", __FUNCTION__, __LINE__,gpio_num);
		return -1;
	}
	
	if (gpio_num <= GPIO_D_7) {
		return BSP_GPIO_ABCD_IRQ;
	}else if (gpio_num <= GPIO_H_7){
		return BSP_GPIO_EFGH_IRQ;
	}
	
	return -1;
}
//end
/*set GPIO pins on*/
void gpioSet(int gpio_num)
{
	unsigned int portMask=0;
	unsigned int pins;
 
	//printk( "<<<<<<<<<enter gpioSet( gpio_num:%d )\n", gpio_num );  
	if ((gpio_num >= GPIO_END)||(gpio_num < 0)) return;

	if (gpio_num <= GPIO_D_7) {
		pins = 1 << gpio_num;
		portMask = get_GPIOMask();
		pins &= portMask;  //mask out disable pins
		if (pins == 0) return;  //no pins to set 

		GPIOdataReg[GPIO_ABCD] |= pins;  //set pins
		//write out
		REG32(GPIO_PABCD_DAT) = GPIOdataReg[GPIO_ABCD];
	}
	else if (gpio_num <= GPIO_H_7) {
		pins = 1 << (gpio_num - GPIO_E_0);
		GPIOdataReg[GPIO_EFGH] |= pins;
		REG32(GPIO_PEFGH_DAT) = GPIOdataReg[GPIO_EFGH];
	}
#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE)
	else if (gpio_num <= RTL8192CD_GPIO_7) {
		use_rtl8192cd_gpio = 1;
		pins = 1 << (RTL8192CD_GPIO_OUT_OFFSET + gpio_num - RTL8192CD_GPIO_0);
		GPIOdataReg[GPIO_RTL8192CD] |= pins;
		//ccwei: 120208-WPS
		REG32(RTL8192CD_GPIO_PIN_CTRL) = (REG32(RTL8192CD_GPIO_PIN_CTRL) & 0xFF000000) |
			GPIOdataReg[GPIO_RTL8192CD];
	}
#endif //CONFIG_RTL8192CD
#ifdef RTL_EXT_GPO
	if (gpio_num >= GPIO_EXT_0 && gpio_num <= GPIO_EXT_3) {
		pins = 1 << (gpio_num - GPIO_EXT_0);
		GPIOdataReg[GPIO_EXT] |= pins;
		REG32(BSP_MISC_PINOCR) = REG32(BSP_MISC_PINOCR) | 
			GPIOdataReg[GPIO_EXT] << BSP_GPO_AFD_SHIFT;
	}
#endif
}
EXPORT_SYMBOL(gpioSet);

/*set GPIO pins off*/
void gpioClear(int gpio_num)
{
	unsigned int portMask=0;
	unsigned int pins;

//	printk( "<<<<<<<<<enter gpioClear( gpio_num:%d )\n", gpio_num );      
	if ((gpio_num >= GPIO_END)||(gpio_num < 0)) return;

	if (gpio_num <= GPIO_D_7) {
		pins = 1 << gpio_num;
		portMask = get_GPIOMask();
		pins &= portMask;  //mask out disable pins
		if (pins==0) return;  //no pins to reset    
	        
		GPIOdataReg[GPIO_ABCD] &= ~pins;  //reset pins
		//write out
		REG32(GPIO_PABCD_DAT) = GPIOdataReg[GPIO_ABCD];
	}
	else if (gpio_num <= GPIO_H_7) {
		pins = 1 << (gpio_num - GPIO_E_0);
		GPIOdataReg[GPIO_EFGH] &= ~pins;
		REG32(GPIO_PEFGH_DAT) = GPIOdataReg[GPIO_EFGH];
	}
#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE)
	else if (gpio_num <= RTL8192CD_GPIO_7) {
		use_rtl8192cd_gpio = 1;
		pins = 1 << (RTL8192CD_GPIO_OUT_OFFSET + gpio_num - RTL8192CD_GPIO_0);
		GPIOdataReg[GPIO_RTL8192CD] &= ~pins;
		//ccwei: 120208
		REG32(RTL8192CD_GPIO_PIN_CTRL) = (REG32(RTL8192CD_GPIO_PIN_CTRL) & 0xFF000000) |
			GPIOdataReg[GPIO_RTL8192CD];
	}
#endif //CONFIG_RTL8192CD
#ifdef RTL_EXT_GPO
	if (gpio_num >= GPIO_EXT_0 && gpio_num <= GPIO_EXT_3) {
		pins = 1 << (gpio_num - GPIO_EXT_0);
		GPIOdataReg[GPIO_EXT] &= ~pins;
		REG32(BSP_MISC_PINOCR) &= ~(pins << BSP_GPO_AFD_SHIFT);
	}
#endif
}
EXPORT_SYMBOL(gpioClear);

int gpioRead(int gpio_num) 
{
	unsigned int val;
	int ret = 0;
	
	if ((gpio_num >= GPIO_END)||(gpio_num < 0)) 
		return 0;

	if (gpio_num <= GPIO_D_7) {
		val = REG32(GPIO_PABCD_DAT);
		ret = (val & (1 << gpio_num)) ? 1:0;
	}
	else if (gpio_num <= GPIO_H_7) {
		val = REG32(GPIO_PEFGH_DAT);
		ret = (val & (1 << (gpio_num - GPIO_E_0))) ? 1:0;
	}
#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE)
	else if (gpio_num <= RTL8192CD_GPIO_7) {
		use_rtl8192cd_gpio = 1;
		val = REG32(RTL8192CD_GPIO_PIN_CTRL);
		ret = (val & (1 << (gpio_num - RTL8192CD_GPIO_0))) ? 1:0;
	}
#endif //CONFIG_RTL8192CD
#ifdef RTL_EXT_GPO
	if (gpio_num >= GPIO_EXT_0 && gpio_num <= GPIO_EXT_3) {
		val = REG32(BSP_MISC_PINOCR);
		ret = (val & (1 << (BSP_GPO_AFD_SHIFT + gpio_num - GPIO_EXT_0))) ? 1:0;
	}
#endif

	return ret;
}

// Added by Mason Yu for New map LED
void gpioHandshaking(int flag)
{
	#ifdef GPIO_LED_ADSL_HS
	gpioConfig(ADSL_LED, GPIO_FUNC_OUTPUT);		

	// on
	if ( flag == 1) {
		gpioClear(ADSL_LED);
		//gpioSet(ADSL_LED);
		return;
	}	
	
	// off
	if ( flag == 0) {
		gpioSet(ADSL_LED); 
		//gpioClear(ADSL_LED);
		return;
	}
	#elif   defined(CONFIG_GPIO_LED_CHT_E8B)
	      gpio_set_dsl_link(flag);
	#else
	return;
	#endif

}

// Added by Mason Yu for New map LED
void gpioACT(int flag)
{
	#ifdef GPIO_LED_ADSL_ACT
	gpioConfig(ADSL_ACT_LED, GPIO_FUNC_OUTPUT);
	
	// on
	if ( flag == 1) {
		gpioClear(ADSL_ACT_LED);
		return;
	}	
	
	// off
	if ( flag == 0) {
		gpioSet(ADSL_ACT_LED); 
		return;
	}
	#else
	return;
	#endif

}


// Added by Mason Yu for New map LED
void gpioAlarm(int flag)
{
	#ifdef GPIO_LED_ADSL_ALARM
	gpioConfig(ALARM_LED, GPIO_FUNC_OUTPUT);
	
	// on
	if ( flag == 1) {
		gpioClear(ALARM_LED);
		return;
	}	
	
	// off
	if ( flag == 0) {
		gpioSet(ALARM_LED); 
		return;
	}
	#else
	return;
	#endif

}

#if defined(CONFIG_STD_LED)
void gpio_LED_PPP(int flag)
{
	#ifdef GPIO_LED_PPP
	gpioConfig(PPP_LED, GPIO_FUNC_OUTPUT);
	
	// on
	if ( flag == 1) {
		gpioClear(PPP_LED);
		return;
	}	
	
	// off
	if ( flag == 0) {
		gpioSet(PPP_LED);
		return;
	}
	#else
	return;
	#endif
}
#elif defined(CONFIG_GPIO_LED_CHT_E8B)
void gpio_LED_PPP(int flag)
{
   gpio_set_ppp_g(flag);

}


#endif
void gpio_internet(int flag)
{
//	gpioAlarm(flag);

	#ifdef GPIO_LED_TR068_INTERNET
	gpioConfig(INTERNET_LED, GPIO_FUNC_OUTPUT);
	
	// on
	if ( flag == 1) {
		gpioClear(INTERNET_LED);
		return;
	}	
	
	// off
	if ( flag == 0) {
		gpioSet(INTERNET_LED);
		return;
	}
	#elif defined(CONFIG_GPIO_LED_CHT_E8B)
                  
          gpio_set_ppp_g(flag);
	
	#else
	return;
	#endif
}



#ifdef CONFIG_ETHWAN
extern void *__sram;
extern void *__sram_end;

void sramflushdcache()
{
	int garbage_tmp;
	__asm__ __volatile__(
         ".set\tnoreorder\n\t"
         ".set\tnoat\n\t"
         "mfc0 %0, $20\n\t"
         "nop\n\t"
         "nop\n\t"
         "andi %0, 0xFDFF\n\t"
         "mtc0 %0, $20\n\t"
         "nop\n\t"
         "nop\n\t"
         "ori %0, 0x200\n\t"
         "mtc0 %0, $20\n\t"
         "nop\n\t"
         "nop\n\t"
         ".set\tat\n\t"
         ".set\treorder"
         : "=r" (garbage_tmp));
}

void  sramInit()
{
	unsigned char *sram_begin;
	unsigned char *sram_end;
	unsigned char *code_buf;
	int sram_size;
	int __iram_size;
	int __sram_size_type;
	unsigned long flags;

	__iram_size = 32768;
	__sram_size_type = 8; /*32KB*/

	sram_begin = (unsigned char*)&__sram ;
	sram_end = (unsigned char*)&__sram_end ;
	if(sram_begin == sram_end){
		printk("sram begin & end = %x, don't init sram\n", sram_begin);
		return;
	}
	
	sram_size = 128 * (1 << __sram_size_type);
	code_buf = (unsigned char *)kmalloc(sram_size,GFP_KERNEL);
	memcpy(code_buf, sram_begin, sram_size);

 	local_irq_save(flags);		
//	REG32(0xb8001300) = (((unsigned int )sram_begin)&(0x1ffffffe))|1;
//	printk("__sram_size_type %x \n",__sram_size_type);
//	REG32(0xb8001304) = __sram_size_type;
//	local_irq_enable();
//	printk("sram_begin %x \n",sram_begin);
//	REG32(0xb8001300) = (((unsigned int )sram_begin)&(0x1ffffffe))|1;
#if 1
	/*jim 20100210 interrupt should be disabled, to sure no sram-mapped code will be executed*/

	//SRAM Mapping enable
	
	//printk("%s(%d):\n",__FUNCTION__,__LINE__);
	REG32(0xb8001300) = (((unsigned int )sram_begin)&(0x1ffffffe))|1;
	printk(" sram_begin %x \n",sram_begin);
	REG32(0xb8001304) = __sram_size_type;
	printk("__sram_size_type %x \n",__sram_size_type);
	REG32(0xb8004000) = (((unsigned int )sram_begin)&(0x1ffffffe))|1;
//	printk("__sram_size_type 3%x \n",__sram_size_type);
	REG32(0xb8004004) = __sram_size_type;
	/*jim 20100210 lexra_memcpy is located in sram mapping region*/
	//memcpy(sram_begin, code_buf, sram_size);
//	printk("%s(%d):\n",__FUNCTION__,__LINE__);
#if 1
   	{
		int i;
		for(i=0; i<sram_size; i++)
		{
			*(sram_begin+i) = *(code_buf+i);
		}
	}
#endif

#endif
	/*jim 20100210 we should flush dcache to sure no problem*/
	sramflushdcache();

	local_irq_restore(flags);
	
	kfree(code_buf);
}
#endif //CONFIG_ETHWAN


#ifdef CONFIG_RTL8672_PWR_SAVE

static int lowp_proc_w(struct file *file, const char *buffer,
				unsigned long count, void *data)
{	
	char tmpbuf[100];
	int num = 0;
	//u32 saved,status;
	//u32 gimr, flags;
	extern void rtl8672_low_power(int);
#if 0
	printk("Entering SLEEP...\n");
	local_irq_save(flags);
	// mask all interrupt except for UART
	status = read_c0_status();
	gimr = REG32(GIMR);
	REG32(GIMR) = (1<<12) | (1<<13);
	write_c0_status(status & 0xFFFF0800);
	__asm__ __volatile__ (
		"sleep;"
	);
	printk("Leaving SLEEP...\n");
	write_c0_status(status);
	REG32(GIMR) = gimr;

	local_irq_restore(flags);
#endif	

	
#if 1
	if (buffer && !copy_from_user(tmpbuf, buffer, count)) {
		sscanf(tmpbuf, "%d", &num);
		rtl8672_low_power(num);					
    }
#endif	
	return count;
}

static int lowp_proc_r(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{	
	int pos = 0;

	if (length >= 128) {
		pos += sprintf(&buf[pos], "length=%d CLK=%u\n", length, SYSCLK);
		pos += sprintf(&buf[pos], "b8002000=%02x\n", REG8(0xb8002000));
		if( IS_RLE0315 || IS_6166 ) {
			pos += sprintf(&buf[pos], "b8003200=%08x\n", REG32(0xb8003200));
			pos += sprintf(&buf[pos], "b800332C=%08x\n", REG32(0xb800332C));
		}
	}

	//pos += sprintf(&buf[pos], "%d", wifi_debug_level);
	//pos += sprintf(&buf[pos], "\n");

	return pos;
}
#endif

int __init gpio_init(void)
{
	struct proc_dir_entry *proc_file1;
	int i;

	for(i=0; i<GPIO_DATA_NUM; i++)
		GPIOdataReg[i] = 0;

	#ifdef CONFIG_RTL8672_PWR_SAVE
	proc_file1 = create_proc_entry("low_power",	0644, NULL);
	if(proc_file1 == NULL)
	{
		printk("can't create proc: low_power\r\n");

	} else {
		proc_file1->write_proc = lowp_proc_w;
		proc_file1->read_proc  = lowp_proc_r;
	}
	#endif
	
#if 0 //move to board-xxx.c
	unsigned int sicr;

	sicr = REG32(BSP_MISC_PINMUX);	
	if( sicr&BSP_JTAG_GPIO_PINMUX )
	{
		printk( "<<<<<<<disable GPIO JTAG function.\n" );
		REG32(BSP_MISC_PINMUX) = sicr& ~BSP_JTAG_GPIO_PINMUX;
	}
#endif

	//init GPIO_PABCD_CNR=0
	REG32(GPIO_PABCD_CNR) = 0x0;
	//init GPIO_PABCD_PTYPE=0
	REG32(GPIO_PABCD_PTYPE) = 0x0;


#ifdef CONFIG_GPIO_LED_CHT_E8B	
         E8GPIO_init();	 
	  gpioHandshaking(0);
         gpio_internet(0);
#endif

#ifdef CONFIG_ETHWAN
	sramInit();
#endif //CONFIG_ETHWAN

	return 0;
  
}


#ifdef CONFIG_GPIO_LED_CHT_E8B
// In the charge of LED blinking control timer
struct timer_list FlashLedTimer;


 
void FlashLED(unsigned long status)
{
	static int CtrlFlag = 0;

	{
	switch((unsigned char)status){
		case C_AMSW_IDLE:
			FlashLedTimer.expires=jiffies+HZ/2;
			gpioHandshaking(CtrlFlag);
			gpio_internet(0);

			break;
		case C_AMSW_L3:			
			gpioHandshaking(1);
			gpio_internet(0);
		
			break;
		case C_AMSW_ACTIVATING:
			FlashLedTimer.expires=jiffies+HZ/2;
		
			gpioHandshaking(CtrlFlag);
			gpio_internet(0);
			break;
		case C_AMSW_INITIALIZING:			
			FlashLedTimer.expires=jiffies+HZ/4;
			gpioHandshaking(CtrlFlag);
			gpio_internet(0);
		
			break;
		case C_AMSW_SHOWTIME_L0:	
			FlashLedTimer.expires=jiffies+HZ/4;

		
			// Added by Mason Yu for PPP ACT LED
//			printk("\ng_internet_up=%d\n",g_internet_up);

			if(IsTrafficOnAtm()) {		
				if(g_internet_up == 1)
					gpio_internet(CtrlFlag);
				else
					gpio_internet(0);
				#ifdef GPIO_LED_PPP_BLK
				if ( g_ppp_up >= 1)				
					gpio_LED_PPP(!CtrlFlag);
				#endif
			}			
			else {	
				if(g_internet_up == 1)
				{
					CtrlFlag = 1;
					gpio_internet(CtrlFlag);
				
				}
				else
					gpio_internet(0);
				#ifdef GPIO_LED_PPP_BLK
				if ( g_ppp_up >= 1)				
					gpio_LED_PPP(1);
				#endif
			}		
			 gpioHandshaking(1);
                      break;
	   
		case C_AMSW_END_OF_LD:
			
			gpioHandshaking(0);	
			gpio_internet(0);
			break;
	}

	if(CtrlFlag)
		CtrlFlag = 0;
	else
		CtrlFlag = 1;	

	FlashLedTimer.data = status;	
	FlashLedTimer.function=(void (*)(unsigned long))FlashLED;
	mod_timer(&FlashLedTimer, FlashLedTimer.expires);	

	}
	
}	



/*
 * ADSL_state : Called by DSP driver while ADSL status changing
 */

void ADSL_state(unsigned char LEDstate)
{
	static unsigned char LastStatus = 255;

	if(LastStatus != LEDstate){
		LastStatus = LEDstate;
		FlashLED((unsigned long)LEDstate);
	}
}	
#endif

//if 128 PIN board 
//PA5 PA6 PA7 PB0 PB1 PB2 PB3 PB4 PB5 PB6 PB7
//0        1    2     3     4     5    6      7      8    9    10
//74164 used

//if 208 PIN board
#define LED_CLK    14    //GPIO_B_6
#define LED_DATA  15    //GPIO_B_7


#define POWER_GREEN            0
#define POWER_RED                1
#define PPP_GREEN                  2
#define PPP_RED                      3               
#define LED_DSL_LINK             4 
/* moved to gpio.h
#define LED_WPS_G                 5
#define LED_WPS_R                 6
#define LED_WPS_Y                 7
*/
static int   power_red_on    ;
static int   power_green_on   ;
static int   ppp_red_on ;
static int   ppp_green_on ;
static int   led_dsl_link_on  ;
static int   led_wps_g_on  = 0 ;
static int   led_wps_r_on = 0;
static int   led_wps_y_on = 0;

void gpio_set_power_g(int flag)
{
    if(flag) //on  
       power_green_on = 1;
    else 
       power_green_on = 0;		
    gpio_led_set();
}

void gpio_set_power_r(int flag)
{
    if(flag)
       power_red_on = 1;
    else
       power_red_on = 0;		 
    gpio_led_set();
}

void gpio_set_ppp_g(int flag)
{                
     if(flag)
	 ppp_green_on = 1;
     else
	 ppp_green_on = 0;
     gpio_led_set();	 
     	 
}

void gpio_set_ppp_r(int flag)
{
     if(flag)
	 ppp_red_on = 1;
     else
	 ppp_red_on = 0;
     gpio_led_set();	 
     	 
}

void gpio_set_dsl_link(int flag)
{
     if(flag)
	led_dsl_link_on  = 1;
     else
	 led_dsl_link_on = 0;
     gpio_led_set();	 
     	 
}
void  gpio_led_set(void) {
	gpioConfig(LED_DATA, GPIO_FUNC_OUTPUT);
	gpioConfig(LED_CLK, GPIO_FUNC_OUTPUT);

   
       if(led_wps_y_on){
	 	gpioClear(LED_DATA);	
       }
	else {
		gpioSet(LED_DATA);
	}
	
	gpioClear(LED_CLK);  
	gpioSet(LED_CLK); 	   //clock 1
	if(led_wps_r_on)
	{
          gpioClear(LED_DATA);
	}
	else 
	{
		gpioSet(LED_DATA);
	}
	gpioClear(LED_CLK)	;
	gpioSet(LED_CLK);     //clock  2  

	if(led_wps_g_on){
            gpioClear(LED_DATA);
	}	
	else {
         gpioSet(LED_DATA);       	
	}

	gpioClear(LED_CLK)	;  
	gpioSet(LED_CLK);     //clock  3
	if(ppp_red_on)
	{
             gpioClear(LED_DATA);

	}
	else
	{
             gpioSet(LED_DATA);

	}
	gpioClear(LED_CLK)	;
	gpioSet(LED_CLK);     //clock  4
	if(ppp_green_on){
         gpioClear(LED_DATA);   
	}
	else
	{
            gpioSet(LED_DATA);

	}
	gpioClear(LED_CLK)	;  
	gpioSet(LED_CLK);     //clock   5 
	if(led_dsl_link_on)
	{
           gpioClear(LED_DATA);

	}
	else 
	{
	  gpioSet(LED_DATA);
	}
	  gpioClear(LED_CLK)	;
	  gpioSet(LED_CLK);     //clock   6
	if(power_red_on)
	{
	  gpioClear(LED_DATA);
	}
	else
	{
	    
	   gpioSet(LED_DATA);
	}
	gpioClear(LED_CLK)	;
	gpioSet(LED_CLK);     //clock    7
	if(power_green_on)
	{
              gpioClear(LED_DATA);
	
	}
	else
	{
             gpioSet(LED_DATA);
	}
	gpioClear(LED_CLK)	;
	gpioSet(LED_CLK);     //clock     8 
     


}
 #ifdef CONFIG_GPIO_LED_CHT_E8B 
void E8GPIO_init()
{

	init_timer(&FlashLedTimer);	

}
#endif

static void __exit gpio_exit (void)
{
}

module_init(gpio_init);
module_exit(gpio_exit);

