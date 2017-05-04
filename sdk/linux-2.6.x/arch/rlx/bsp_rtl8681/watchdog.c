#include <linux/config.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <asm/ptrace.h>

#ifdef CONFIG_RTL8672
//#include <asm/mach-realtek/rtl8672/platform.h>
#include <bspchip.h>
#endif
/*
watchdog commands
*/
#define WDTKICK (WDTCLR|OVSEL_13)  //command to kick watchdog

int watchdog_enable=0;
static int soft_watchdog_last_tick;
static int kick_time=5*HZ;  //default kick duration = 5 sec
static int timeout_time=0;  //for kick cycle time update

/*
reset hw watchdog register
*/
#ifdef CONFIG_RTL8672_SAR
extern void Reset_Sar(void);
extern void Idle(int period);
#endif
void kick_watchdog(void)
{
  int timeslip;
  
	timeslip = jiffies - soft_watchdog_last_tick;
	timeslip = (timeslip>0) ? timeslip : (-timeslip);
	if ((0!=timeout_time)&&(timeslip>timeout_time)) {
	    printk("Watchdog Reset!\n");
	    //reset nic
	   // *((char*)0xB980003B) = 0x01;

#ifdef CONFIG_NET_WIRELESS
	    //reset wlan
	    *((char*)0xBD300037) = 0x04;
#endif	    

#ifdef CONFIG_RTL8672_SAR
	    //reset sar
	    extern void Reset_Sar(void);
	    Reset_Sar();

	    Idle(200000);
#endif

	    REG32(BSP_WDTCNR) = WDTKICK;  //start hw watchdog
	    while (1);  //wait until reboot
	}
}

/*
  Called by watchdog task.
  Return watchdog task sleep time for update kick cycle in task.
*/
int update_watchdog_timer(void)
{
	soft_watchdog_last_tick = jiffies;
	return (kick_time/HZ);
}

/*
get watchdog status
*/
void get_watchdog_status(void)
{
  int timeslip;
  
	timeslip = jiffies - soft_watchdog_last_tick;
	timeslip = (timeslip>0) ? timeslip : (-timeslip);
    printk("Watchdog Status:\n");
    printk("  Enable: %s\n", watchdog_enable?"Yes":"No");
    printk("  Kick Cycle: %d sec\n", kick_time/HZ);
    printk("  Timeout Time: %d sec\n", timeout_time/HZ);
    printk("  %d sec passed since last kick.\n", timeslip/HZ);   
}

/*
set watchdog update time
*/
void set_watchdog_kick_time(int kick_sec)
{
	soft_watchdog_last_tick = jiffies;
    if (kick_sec<2) kick_sec = 2;
	kick_time = kick_sec*HZ;
}

/*
set watchdog timeout time
*/
void set_watchdog_timeout_time(int timeout_sec)
{
	soft_watchdog_last_tick = jiffies;    
    if (timeout_sec<0) timeout_sec = 0;
	timeout_time = timeout_sec*HZ;
}

/*
start watchdog monitor
*/
void start_watchdog(int kick_sec)
{
	//set watchdog kick time
	set_watchdog_kick_time(kick_sec);
	soft_watchdog_last_tick = jiffies;
	watchdog_enable=1;
	printk("watchdog started\n");
	//REG32(WDTCNR) = WDTKICK;
}

/*
stop watchdog monitor
*/
void stop_watchdog(void)
{
	//cli();
	watchdog_enable=0;
	//REG32(WDTCNR) = WDSTOP_PATTERN<<WDTE_OFFSET;	//stop hw watchdog
	//sti();
}




