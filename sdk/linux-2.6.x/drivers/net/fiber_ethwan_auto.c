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
 * Purpose : auto switch fiber wan / ethernet wan kernel thread
 *
 * Feature : Use kernel thread to perodically probe fiber/ethernet wan switch
 *
 */



#include <linux/module.h> 
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/init.h> 
#include <linux/proc_fs.h>
#include <bspchip.h>
/*
 * Data Declaration
 */
__DRAM unsigned int AutoSwitch = 1;

struct proc_dir_entry *fiber_proc_dir = NULL;
struct proc_dir_entry *fiber_flag_entry = NULL;

struct task_struct *pFiberAutoTask;
static int fib_state = 0xff,fib_cur_state = 0;


#define _DEBUG_FIBER_ALWAYS_   0xffffffff

#define DEBUG_FIBER
#ifdef DEBUG_FIBER
static unsigned int _debug_fiber_ = (_DEBUG_FIBER_ALWAYS_);//|DEBUG_READ|DEBUG_WRITE|DEBUG_ERASE);
#define DEBUG_FIBER_PRINT(mask, string) \
			if ((_debug_fiber_ & mask) || (mask == _DEBUG_FIBER_ALWAYS_)) \
			printk string
#else
#define DEBUG_FIBER_PRINT(mask, string)
#endif

#define UTP_FIBER_AUTODET 0xBB0001A8
	#define _SEL_FIB 	(1 << 9)
	#define _PHY4_SDET	(1 << 8)
	#define _FIB_SDET 	(1 << 7)
	#define _SDS_RX_DISABLE (1 << 3)
#define CHIP_INF_SEL	  0xBB000170
	#define _PHY4_EN (1 << 1)

#define	WSDS_ANA_00		0xBB022000
	#define _REG_CDR_RESET_MANUAL (1 << 4)

void fiber_auto_switch(void)
{
	unsigned int reg_fib;
	reg_fib = REG32(UTP_FIBER_AUTODET);

	if(reg_fib & _FIB_SDET)
		fib_cur_state = 1;
	else
		fib_cur_state = 0;

	if(fib_state == fib_cur_state)
		return;
	
	if(fib_cur_state){
		//fiber detected
		printk("fiber detected\n");
		REG32(CHIP_INF_SEL) &= ~(_PHY4_EN);
		REG32(UTP_FIBER_AUTODET) &= ~(_SDS_RX_DISABLE);
		REG32(UTP_FIBER_AUTODET) |= (_SEL_FIB);
		//toogle bit 3 sedes rx reset
		REG32(WSDS_ANA_00) |= (_REG_CDR_RESET_MANUAL);
		REG32(WSDS_ANA_00) &= ~(_REG_CDR_RESET_MANUAL);
		fib_state = 1;
	}else{
		//eth wan detedted
		printk("ethwan detected\n");
		REG32(UTP_FIBER_AUTODET) &= ~(_SEL_FIB);
		REG32(UTP_FIBER_AUTODET) |= _SDS_RX_DISABLE; 
		REG32(CHIP_INF_SEL) |= (_PHY4_EN);
		fib_state = 0;
	}
}

int fiber_auto_thread(void *data)
{


    while(!kthread_should_stop())
    {
    	/* No need to wake up earlier */
    	set_current_state(TASK_UNINTERRUPTIBLE);
    	schedule_timeout(5 * HZ / 10); //500ms to wake up   
    	if(AutoSwitch){
			fiber_auto_switch();
		}
    }
	return 0;
}

static int fiber_auto_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "[auto_switch_flag = 0x%08x]\n", AutoSwitch);
	len += sprintf(page + len, "0x1: enable fiber/ethernet wan auto switch func.\n");
	len += sprintf(page + len, "0x3: enable fiber/ethernet wan auto switch debug message\n");


	return len;
}

static int fiber_auto_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	unsigned char tmpBuf[16] = {0};
	int len = (count > 15) ? 15 : count;

	if (buffer && !copy_from_user(tmpBuf, buffer, len))
	{
		AutoSwitch = simple_strtoul(tmpBuf, NULL, 16);
		//printk("write watchdog_flag to 0x%08x\n", AutoSwitch);
		return count;
	}
	return -EFAULT;
}


static void fiber_dbg_init(void)
{
    /* Create proc debug commands */
    if(NULL == fiber_proc_dir)
    {
        fiber_proc_dir = proc_mkdir("fiber", NULL);
		//printk("[%s-%d] \n", __FILE__, __LINE__);
    }
    if(fiber_proc_dir)
    {
   		//printk("[%s-%d] \n", __FILE__, __LINE__);
    	fiber_flag_entry = create_proc_entry("AutoSwitch", 0, fiber_proc_dir);
    	if(fiber_flag_entry){
    		fiber_flag_entry->read_proc = fiber_auto_read;
    		fiber_flag_entry->write_proc = fiber_auto_write;
			//printk("[%s-%d] \n", __FILE__, __LINE__);
    	}
    }
}

static void fiber_dbg_exit(void)
{
    /* Remove proc debug commands */
    if(fiber_flag_entry)
    {
    	remove_proc_entry("AutoSwitch", fiber_flag_entry);
        fiber_flag_entry = NULL;
    }
    if(fiber_proc_dir)
    {
        remove_proc_entry("fiber", NULL);
        fiber_proc_dir = NULL;
    }
}



int __init fiber_auto_init(void)
{
    pFiberAutoTask = kthread_create(fiber_auto_thread, NULL, "fiberauto");
    if(IS_ERR(pFiberAutoTask))
    {
        printk("[%s-%d] init failed %ld!\n", __FILE__, __LINE__, PTR_ERR(pFiberAutoTask));
    }
    else
    {
        wake_up_process(pFiberAutoTask);
        printk("[%s-%d] init complete!\n", __FILE__, __LINE__);
    }
    fiber_dbg_init();

    return 0;
}

void __exit fiber_auto_exit(void)
{
	printk("[%s-%d] exit!\n", __FILE__, __LINE__);
    kthread_stop(pFiberAutoTask);
    fiber_dbg_exit();
}



MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RealTek Fiber/Ethernet WAN auto switch module");
MODULE_AUTHOR("David Chen <david_cw@realtek.com>");
module_init(fiber_auto_init);
module_exit(fiber_auto_exit);





