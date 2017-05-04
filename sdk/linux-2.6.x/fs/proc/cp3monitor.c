/*
 *  Performance Profiling routines
 *
 *  $Id: romeperf.c,v 1.1 2009/11/06 12:26:48 victoryman Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/types.h>
#include <asm/processor.h>
#include <asm/uaccess.h>
#include <asm/rlxregs.h>
#include <asm/cp_unit.h>

#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/jiffies.h>
#include <linux/string.h>


#define debug_printf(enable, arg...) \
    if(enable) printk(arg)

struct cp3monitor_stat_s {
/*
    u64 acc_cp3_con0;
    u64 acc_cp3_con1;
    u64 acc_cp3_con2;
    u64 acc_cp3_con3;
*/
    u32 acc_cp3_con0_lo;
    u32 acc_cp3_con1_lo;
    u32 acc_cp3_con2_lo;
    u32 acc_cp3_con3_lo;
    u32 cp0_count;
};
typedef struct cp3monitor_stat_s cp3monitor_stat_t;


/* Global variables for read_proc & write_proc*/
u32 _cp3_count_event;
u64 pre_cp3_con0, pre_cp3_con1, pre_cp3_con2, pre_cp3_con3;
u64 pre_cp3_con0_lo, pre_cp3_con1_lo, pre_cp3_con2_lo, pre_cp3_con3_lo;


static int read_perf(char *buffer, char **start, off_t offset, int len, int *eof, void *data)
{

    u64 currCnt[4];
    get_all_cp3_counters(currCnt[0], currCnt[1], currCnt[2], currCnt[3]);
    debug_printf(0, " %llu,%llu,%llu,%llu\n", currCnt[0], currCnt[1], currCnt[2], currCnt[3]);

   	int ret = 0;

	if (offset > 0) {
    	/* we have finished to read, return 0 */
    	ret  = 0;
	} else {
	    ret += sprintf(buffer+ret, "%llu,%llu,%llu,%llu\n",
            currCnt[0] - pre_cp3_con0, currCnt[1] - pre_cp3_con1,
            currCnt[2] - pre_cp3_con2, currCnt[3] - pre_cp3_con3);

        get_all_cp3_counters(pre_cp3_con0, pre_cp3_con1, pre_cp3_con2, pre_cp3_con3);
	}

    return ret;
}

static int write_perf(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	char cmd[64];
    char *strptr, *arg;
    int unsigned interval=1, times=10;
    int auto_probe = 0, counter = 0, exeIndex = 0;
    u32 curr_count, prev_count;

	if (copy_from_user(cmd, buffer, count > sizeof(cmd) ? sizeof(cmd) : count)) {
		return -EFAULT;
	}
    cmd[count] = '\0';

    _cp3_count_event = 0;
#ifdef CONFIG_LUNA_PROC_CP3_MONITOR_EXCLUDE_IDLE
    _system_idle_wait = 0;
    _count_cpu_idle = 0;
#endif
    strptr=cmd;
    while ((arg = strsep(&strptr, " ")) != NULL) {
#ifdef CONFIG_LUNA_PROC_CP3_MONITOR_EXCLUDE_IDLE
         if (strcmp(arg, "-idle") == 0) {
            arg = strsep(&strptr, " ");
            if (arg) {
                _system_idle_wait = simple_strtol(arg, NULL, 0);
                debug_printf(0, "\n === In system idle mode (argu = %d) === \n", _system_idle_wait);
            }
        } else
#endif
        if (strcmp(arg, "-p") == 0) {
            arg = strsep(&strptr, " ");
            if (arg) {
                interval = simple_strtol(arg, NULL, 0);
                auto_probe = 1;
            }
        }
        else if (strcmp(arg, "-t") == 0) {
            arg = strsep(&strptr, " ");
            if (arg) {
                times = simple_strtol(arg, NULL, 0);
                auto_probe = 1;
            }
        } else {
            _cp3_count_event |= ( simple_strtol(arg, NULL, 0) << (counter*8));
            counter ++;
            debug_printf(0, "reg: 0x%x\n", _cp3_count_event);
        }
    }
    debug_printf(0, "\nStart to probe count event 0x%x for %d sec * %d times...\n", _cp3_count_event, interval, times);

    if (!_cp3_count_event)
        return count;

    cp3monitor_stat_t cp3State[times];

    get_all_cp3_counters_lo(pre_cp3_con0_lo, pre_cp3_con1_lo, pre_cp3_con2_lo, pre_cp3_con3_lo);
    debug_printf(0, "1st: %llu,%llu,%llu,%llu\n", pre_cp3_con0, pre_cp3_con1, pre_cp3_con2, pre_cp3_con3);

    _asm_resume_cp3_events(_cp3_count_event);
    prev_count = _asm_get_cp0_count();

    if (auto_probe){

        while (exeIndex < times) {

            set_current_state(TASK_INTERRUPTIBLE);
            schedule_timeout(HZ * interval);

            get_all_cp3_counters_lo(cp3State[exeIndex].acc_cp3_con0_lo, cp3State[exeIndex].acc_cp3_con1_lo,
                                cp3State[exeIndex].acc_cp3_con2_lo, cp3State[exeIndex].acc_cp3_con3_lo);
            curr_count = _asm_get_cp0_count();

            cp3State[exeIndex].acc_cp3_con0_lo-= pre_cp3_con0_lo;
            cp3State[exeIndex].acc_cp3_con1_lo-= pre_cp3_con1_lo;
            cp3State[exeIndex].acc_cp3_con2_lo-= pre_cp3_con2_lo;
            cp3State[exeIndex].acc_cp3_con3_lo-= pre_cp3_con3_lo;

            cp3State[exeIndex].cp0_count = curr_count - prev_count;
            exeIndex ++;

            prev_count = curr_count;
            get_all_cp3_counters_lo(pre_cp3_con0_lo, pre_cp3_con1_lo, pre_cp3_con2_lo, pre_cp3_con3_lo);
            debug_printf(0, "%2d,%llu,%llu,%llu,%llu\n", exeIndex,
                pre_cp3_con0, pre_cp3_con1, pre_cp3_con2, pre_cp3_con3);
        }

        debug_printf(0, "CP3 event: %x\n", _asm_freeze_cp3_events());
#ifdef CONFIG_LUNA_PROC_CP3_MONITOR_EXCLUDE_IDLE
        printk("idle cycle,%d,idle times,%d\n", _system_idle_wait, _count_cpu_idle);
#endif
        for (exeIndex = 0; exeIndex < times; exeIndex++){
            printk("%d,%u,%u,%u,%u,%u\n", exeIndex, cp3State[exeIndex].cp0_count,
              cp3State[exeIndex].acc_cp3_con0_lo, cp3State[exeIndex].acc_cp3_con1_lo,
              cp3State[exeIndex].acc_cp3_con2_lo, cp3State[exeIndex].acc_cp3_con3_lo);
        }
    }

	return count;
};

static int __init cp3monitor_init(void) {
	struct proc_dir_entry *pe;

	pe = create_proc_entry("cp3monitor", S_IRUSR |S_IWUSR | S_IRGRP | S_IROTH, NULL);
	if (!pe) {
		return -EINVAL;
	}

	pe->read_proc  = read_perf;
	pe->write_proc = write_perf;
    _asm_enable_cp3();
	return 0;
}

static void __exit cp3monitor_exit(void) {
}

module_init(cp3monitor_init);
module_exit(cp3monitor_exit);
