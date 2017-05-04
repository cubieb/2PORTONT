/*
* ----------------------------------------------------------------
* Copyright Realtek Semiconductor Corporation, 2013  
* All rights reserved.
* 
* Description: luna platform clock management unit driver
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

#include <bspchip.h>

#include <soc.h>

unsigned int cmu_cpu0_sleep_cnt = 0;
static int proc_cmu_status_print_r(char *buf, char **start, off_t offset,
			           int length, int *eof, void *data)
{	
	int pos = 0;
	unsigned int reg_val;	
	
	if(offset > 0)
		return 0;

	reg_val = REG32(0xb8000308);
	pos += sprintf(&buf[pos], "OTTO_CPU_CYCLE=%u\n", OTTO_CPU_CYCLE());
	pos += sprintf(&buf[pos], "REG_CPU0_WK_INDI=%d\n", ( (reg_val >> 31) & 0x1));
	pos += sprintf(&buf[pos], "REG_CPU1_WK_INDI=%d\n", ( (reg_val >> 30) & 0x1));
	pos += sprintf(&buf[pos], "REG_LX0_WK_INDI =%d\n", ( (reg_val >> 29) & 0x1));
	pos += sprintf(&buf[pos], "REG_LX1_WK_INDI =%d\n", ( (reg_val >> 28) & 0x1));
	pos += sprintf(&buf[pos], "REG_LX2_WK_INDI =%d\n", ( (reg_val >> 27) & 0x1));
	pos += sprintf(&buf[pos], "REG_LXP_WK_INDI =%d\n", ( (reg_val >> 26) & 0x1));
	pos += sprintf(&buf[pos], "Freq_Div_Index: 1-F/2, 2-F/4, 3-F/8, 4-F/16, 5-F/32, 6-F/64, 7-F/128\n");
	pos += sprintf(&buf[pos], "REG_CPU0_Freq_Div_Index =%d\n", ( (reg_val >> 23) & 0x7));
	pos += sprintf(&buf[pos], "REG_CPU1_Freq_Div_Index =%d\n", ( (reg_val >> 20) & 0x7));
	pos += sprintf(&buf[pos], "REG_LX0_Freq_Div_Index  =%d\n", ( (reg_val >> 17) & 0x7));
	pos += sprintf(&buf[pos], "REG_LX1_Freq_Div_Index  =%d\n", ( (reg_val >> 14) & 0x7));
	pos += sprintf(&buf[pos], "REG_LX2_Freq_Div_Index  =%d\n", ( (reg_val >> 11) & 0x7));
	pos += sprintf(&buf[pos], "REG_LXP_Freq_Div_Index  =%d\n", ( (reg_val >>  8) & 0x7));
	pos += sprintf(&buf[pos], "REG_CMU_MD  =%d (0:disable, 1:fixed mode, 2:dynamic mode)\n", ( (reg_val >>  6) & 0x3));
	pos += sprintf(&buf[pos], "cf_oc0_slow_dram=%d\n", ( (reg_val >> 4) & 0x1));
	pos += sprintf(&buf[pos], "cf_oc1_slow_dram=%d\n", ( (reg_val >> 3) & 0x1));
	pos += sprintf(&buf[pos], "cf_lx0_slow_dram=%d\n", ( (reg_val >> 2) & 0x1));
	pos += sprintf(&buf[pos], "cf_lx1_slow_dram=%d\n", ( (reg_val >> 1) & 0x1));
	pos += sprintf(&buf[pos], "cf_lx2_slow_dram=%d\n", ( (reg_val >> 0) & 0x1));
	
	pos += sprintf(&buf[pos], "cmu_cpu0_sleep_cnt=%d\n", cmu_cpu0_sleep_cnt);

	return pos;
}


static int proc_cmu_registers_print_r(char *buf, char **start, off_t offset,
			              int length, int *eof, void *data)
{	
	int pos = 0;
	int i;

	for(i=0; i<4; i++) {
		pos += sprintf(&buf[pos], "REG32(0x%08x)=0x%08x\n", (0xb8000300+(i*4)), REG32((0xb8000300+(i*4))));
		//printk("REG32(0x%08x)=0x%08x\n", (0xb8000300+(i*4)), REG32((0xb8000300+(i*4))));
	}

	return pos;
}

static struct proc_dir_entry *proc_cmu_dir ;
static void luna_cmu_proc_init(void)
{
	struct proc_dir_entry *entry;
	
	/*
	*	create root directory
	*/
	proc_cmu_dir = proc_mkdir("cmu", NULL);
	if (proc_cmu_dir == NULL) {
		printk("create proc root failed!\n");
		return;
	}

	if((entry = create_proc_entry("status", 0644, proc_cmu_dir)) == NULL) {
		printk("create proc cmu/status failed!\n");
		return;
	}
	//entry->write_proc = proc_cmu_status_print_w;
	entry->read_proc  = proc_cmu_status_print_r;

	if((entry = create_proc_entry("registers", 0644, proc_cmu_dir)) == NULL) {
		printk("create proc cmu/registers failed!\n");
		return;
	}
	//entry->write_proc = proc_cmu_registers_print_w;
	entry->read_proc  = proc_cmu_registers_print_r;

}

int __init luna_cmu_init(void)
{
	/* TODO: move cmu init from prom.c to here */

	printk("=================================\n");
	printk("%s\n", __FUNCTION__);
	printk("=================================\n");

	luna_cmu_proc_init();

	return 0;
}

static void __exit luna_cmu_exit (void)
{
}

module_init(luna_cmu_init);
module_exit(luna_cmu_exit);

