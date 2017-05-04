/*
 * Copyright (C) 2011 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 *
 *
 * Purpose : watchdog timeout kernel thread
 *
 * Feature : Use kernel thread to perodically kick the watchdog
 *
 */

 
/*
  * Include Files
  */
#include <linux/module.h> 
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/init.h> 
#include <linux/proc_fs.h>
#include <linux/time.h>
#include <linux/spinlock.h>
#include <bspchip.h>
#include <bspchip_8686.h>

/*
 * Data Declaration
 */
unsigned int watchdog_flag = 0;
unsigned int kick_wdg_time = 5;

unsigned int wdg_timeout = CONFIG_WDT_PH1_TO;
unsigned int wdg_timeout_ph2 = CONFIG_WDT_PH2_TO;
unsigned int wdg_clk_sc = CONFIG_WDT_CLK_SC;
unsigned int wdg_rest_mode = CONFIG_WDT_RESET_MODE;

struct proc_dir_entry *watchdog_proc_dir = NULL;
struct proc_dir_entry *watchdog_flag_entry = NULL;
struct proc_dir_entry *wdg_timeout_entry = NULL;
struct proc_dir_entry *kick_wdg_time_entry = NULL;
struct proc_dir_entry *wdg_register_entry = NULL;
#define DEBUG_LOCK_TEST
#ifdef DEBUG_LOCK_TEST
struct proc_dir_entry *wdt_lock_entry = NULL;
DEFINE_SPINLOCK(wdt_lock);
#endif

struct task_struct *pWatchdogTask;

#define RTK_WDG_EN() (*((volatile unsigned int *)(BSP_WDTCTRLR)))|=WDT_E
#define RTK_WDG_DIS() (*((volatile unsigned int *)(BSP_WDTCTRLR)))&=(~WDT_E)
#define RTK_WDG_KICK() (*((volatile unsigned int *)(BSP_WDCNTRR)))|= WDT_KICK

#define _DEBUG_RTK_WDT_ALWAYS_   0xffffffff

#define DEBUG_RTK_WDT
#ifdef DEBUG_RTK_WDT
static unsigned int _debug_rtk_wdt_ = (_DEBUG_RTK_WDT_ALWAYS_);//|DEBUG_READ|DEBUG_WRITE|DEBUG_ERASE);
static int debug_mask = 0;
#define DEBUG_RTK_WDT_PRINT(mask, string) \
			if ((_debug_rtk_wdt_ & mask) || (mask == _DEBUG_RTK_WDT_ALWAYS_)) \
			printk string
#else
#define DEBUG_RTK_WDT_PRINT(mask, string)
#endif

struct timeval start;
struct timeval end;

static void show_usage(void)
{
	unsigned int reg_val = REG32(BSP_WDTCTRLR);	
	printk("************ Watchdog Setting ****************\n");
	printk("WDT_E=%d, (1-enable, 0-disable)\n", ( (reg_val >> 31) & 0x1));
	printk("LX(MHz)=%u\n", BSP_MHZ);
	printk("WDT_CLK_SC=%d\n", ( (reg_val >> 29) & 0x3));
	printk("PH1_TO=%d\n", ( (reg_val >> 22) & 0x1F));
	printk("PH2_TO=%d\n", ( (reg_val >> 15) & 0x1F));
	printk("WDT_RESET_MODE=%d\n", ( (reg_val ) & 0x3));
	printk("**********************************************\n");
}

//for LX 200M Hz
void set_rtk_wdt_ph1_threshold(int sec)
{
	unsigned int reg=0;

	reg = REG32(BSP_WDTCTRLR);
	reg &= ~(WDT_PH12_TO_MSK << WDT_PH1_TO_SHIFT);
	
	reg &= ~(WDT_PH12_TO_MSK << WDT_PH2_TO_SHIFT);
	reg |= (wdg_timeout_ph2 << WDT_PH2_TO_SHIFT);
	
	reg |= (wdg_clk_sc << WDT_CLK_SC_SHIFT);
	reg |= wdg_rest_mode;
	
	if(sec ==0){
			REG32(BSP_WDTCTRLR) = reg | (31 << WDT_PH1_TO_SHIFT);
	}else{
			REG32(BSP_WDTCTRLR) = reg | (sec << WDT_PH1_TO_SHIFT);
	}
	show_usage();
}



void kick_rtk_watchdog(void)
{
	if (_debug_rtk_wdt_ & debug_mask){
  	do_gettimeofday(&end);
		printk("tv_sec:%u\t",(unsigned int)end.tv_sec);
		printk("tv_usec:%u\n",(unsigned int)end.tv_usec);
	}
	RTK_WDG_KICK();
}

int watchdog_timeout_thread(void *data)
{


    while(!kthread_should_stop())
    {
    	//gettimeofday(&start,NULL);
    	/* No need to wake up earlier */
    	set_current_state(TASK_UNINTERRUPTIBLE);
    	schedule_timeout(kick_wdg_time * HZ);    
    	if(watchdog_flag){
			kick_rtk_watchdog();
		}
    }
	return 0;
}

static void wdt_thread_maintain(int flag){

		if(flag){
			if( pWatchdogTask == NULL){
		    pWatchdogTask = kthread_create(watchdog_timeout_thread, NULL, "watchdog");
		    if(IS_ERR(pWatchdogTask))
		    {
		        printk("[Kthread : watchdog] init failed %ld!\n", PTR_ERR(pWatchdogTask));
		    }
		    else
		    {
		        wake_up_process(pWatchdogTask);
		        printk("[Kthread : watchdog ] init complete!\n");
		    }
			}
	}else{
			if( pWatchdogTask != NULL){
    		kthread_stop(pWatchdogTask);
    		pWatchdogTask = NULL;
			}		
	}

}

/* Kthread watchdog */
static int kick_wdg_time_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "[kick_wdg_time = %d sec]\n", kick_wdg_time);
	len += sprintf(page + len, "set watchdog kick time to 1~31 sec\n");
	return len;
}

static int kick_wdg_time_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	unsigned char tmpBuf[16] = {0};
	int len = (count > 15) ? 15 : count;

	if (buffer && !copy_from_user(tmpBuf, buffer, len))
	{
		kick_wdg_time = simple_strtoul(tmpBuf, NULL, 10);
		printk("write kick_wdg_time to %d\n", kick_wdg_time);
		//set_wdt_ph1_threshold(wdg_timeout);
		return count;
	}
	return -EFAULT;
}

/* ƒÜ	Watch Dog Timer Control Register: PH1 */
static int wdg_timeout_read(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
	int pos = 0;
	unsigned int reg_val;	
	
	if(offset > 0)
		return 0;

	reg_val = REG32(BSP_WDTCTRLR);
	pos += sprintf(&buf[pos], "WDT_E=%d, (1-enable, 0-disable)\n", ( (reg_val >> 31) & 0x1));
	pos += sprintf(&buf[pos], "LX(MHz)=%u\n", BSP_MHZ);
	pos += sprintf(&buf[pos], "WDT_CLK_SC=%d\n", ( (reg_val >> 29) & 0x3));
	pos += sprintf(&buf[pos], "PH1_TO=%d\n", ( (reg_val >> 22) & 0x1F));
	pos += sprintf(&buf[pos], "PH2_TO=%d\n", ( (reg_val >> 15) & 0x1F));
	pos += sprintf(&buf[pos], "WDT_RESET_MODE=%d\n", ( (reg_val ) & 0x3));
	return pos;
}

static int wdg_timeout_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	unsigned char tmpBuf[16] = {0};
	int len = (count > 15) ? 15 : count;

	if (buffer && !copy_from_user(tmpBuf, buffer, len))
	{
		wdg_timeout = simple_strtoul(tmpBuf, NULL, 10);
		printk("write wdg_timeout to value(%d)\n", wdg_timeout);
		set_rtk_wdt_ph1_threshold(wdg_timeout);
		return count;
	}
	return -EFAULT;
}


static int watchdog_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "[watchdog_flag = 0x%08x]\n", watchdog_flag);
	len += sprintf(page + len, "0x1: enable hw watchdog timeout\n");
	len += sprintf(page + len, "0x3: enable hw watchdog timeout debug message\n");


	return len;
}

static int watchdog_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	unsigned char tmpBuf[16] = {0};
	int len = (count > 15) ? 15 : count;

	if (buffer && !copy_from_user(tmpBuf, buffer, len))
	{
		watchdog_flag = simple_strtoul(tmpBuf, NULL, 10);
		printk("write watchdog_flag to 0x%08x\n", watchdog_flag);
		if(watchdog_flag){
			wdt_thread_maintain(1);
			RTK_WDG_EN();
			RTK_WDG_KICK();
		}else{
			RTK_WDG_DIS();
			wdt_thread_maintain(0); //stop thread
		}
			
		if(watchdog_flag & 0x02)
			debug_mask = 1;
			
		return count;
	}
	return -EFAULT;
}

static int proc_wdt_registers_r(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{	
	int pos = 0;
	int i;

	for(i=0; i<3; i++) {
		pos += sprintf(&buf[pos], "REG32(0x%08x)=0x%08x\n", (0xb8003260+(i*4)), REG32((0xb8003260+(i*4))));
	}

	return pos;
}

#ifdef DEBUG_LOCK_TEST
static int wdt_lock_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "[watchdog_flag = 0x%08x]\n", watchdog_flag);

	return len;
}
static void wdt_lock_func(void){
	spin_lock(&wdt_lock);
	printk("[Watchdog Timeout Testing]Dead lock forever.");
	while(1);
		
}

static int wdt_lock_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	unsigned char tmpBuf[16] = {0};
	int len = (count > 15) ? 15 : count;
	int lock_flag = 0;
	if (buffer && !copy_from_user(tmpBuf, buffer, len))
	{
		lock_flag = simple_strtoul(tmpBuf, NULL, 10);
		printk("write lock_flag to 0x%08x\n", lock_flag);
		if(lock_flag){
			wdt_lock_func();
		}
		return count;
	}
	return -EFAULT;
}

#endif

static void watchdog_dbg_init(void)
{
    /* Create proc debug commands */
    if(NULL == watchdog_proc_dir)
    {
        watchdog_proc_dir = proc_mkdir("watchdog", NULL);
    }
    if(watchdog_proc_dir)
    {
    	watchdog_flag_entry = create_proc_entry("watchdog_flag", 0, watchdog_proc_dir);
    	if(watchdog_flag_entry){
    		watchdog_flag_entry->read_proc = watchdog_read;
    		watchdog_flag_entry->write_proc = watchdog_write;
    	}
    	wdg_timeout_entry = create_proc_entry("wdg_timeout", 0, watchdog_proc_dir);
    	if(wdg_timeout_entry){
    		wdg_timeout_entry->read_proc = wdg_timeout_read;
    		wdg_timeout_entry->write_proc = wdg_timeout_write;
    	}
    	kick_wdg_time_entry = create_proc_entry("kick_wdg_time", 0, watchdog_proc_dir);
    	if(kick_wdg_time_entry){
    		kick_wdg_time_entry->read_proc = kick_wdg_time_read;
    		kick_wdg_time_entry->write_proc = kick_wdg_time_write;
    	}
    	
    	wdg_register_entry = create_proc_entry("registers", 0, watchdog_proc_dir);
     	if(wdg_register_entry){
    		wdg_register_entry->read_proc = proc_wdt_registers_r;
    	}
    	
    	
#ifdef DEBUG_LOCK_TEST
			wdt_lock_entry = create_proc_entry("wdt_lock_test", 0, watchdog_proc_dir);
    	if(wdt_lock_entry){
    		wdt_lock_entry->read_proc = wdt_lock_read;
    		wdt_lock_entry->write_proc = wdt_lock_write;
    	}
#endif
    }
}

static void watchdog_dbg_exit(void)
{
    /* Remove proc debug commands */
    if(watchdog_flag_entry)
    {
    	remove_proc_entry("watchdog_flag", watchdog_flag_entry);
        watchdog_flag_entry = NULL;
    }
    if(wdg_timeout_entry)
    {
    	remove_proc_entry("wdg_timeout", wdg_timeout_entry);
        wdg_timeout_entry = NULL;
    }
    if(kick_wdg_time_entry)
    {
    	remove_proc_entry("kick_wdg_time", kick_wdg_time_entry);
        kick_wdg_time_entry = NULL;
    }
    if(wdg_register_entry)
    {
    	remove_proc_entry("registers", wdg_register_entry);
        wdg_register_entry = NULL;
    }
#ifdef DEBUG_LOCK_TEST
    if(wdt_lock_entry)
    {
    	remove_proc_entry("wdt_lock_test", wdt_lock_entry);
        wdt_lock_entry = NULL;
    }
#endif
    if(watchdog_proc_dir)
    {
        remove_proc_entry("watchdog", NULL);
        watchdog_proc_dir = NULL;
    }
}



int __init watchdog_timeout_init(void)
{
		int i = 0;
		if(CONFIG_WDT_ENABLE){
			wdt_thread_maintain(1);
			set_rtk_wdt_ph1_threshold(wdg_timeout);
			RTK_WDG_EN();
			watchdog_flag = 1;
		}
    watchdog_dbg_init();
		//set_rtk_wdt_ph1_threshold(wdg_timeout);
	printk("=================================\n");
	printk("%s\n", __FUNCTION__);
	for(i=0; i<3; i++) {
		 printk("REG32(0x%08x)=0x%08x\n", (0xb8003260+(i*4)), REG32((0xb8003260+(i*4))));
	}
	printk("=================================\n");
    return 0;
}

void __exit watchdog_timeout_exit(void)
{
	printk("[%s-%d] exit!\n", __FILE__, __LINE__);
	RTK_WDG_DIS();
    wdt_thread_maintain(0);
    watchdog_dbg_exit();
}


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RealTek watchdog timeout module");
MODULE_AUTHOR("David Chen <david_cw@realtek.com>");
module_init(watchdog_timeout_init);
module_exit(watchdog_timeout_exit);
