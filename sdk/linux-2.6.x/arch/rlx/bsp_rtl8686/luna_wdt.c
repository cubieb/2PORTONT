/*
* ----------------------------------------------------------------
* Copyright Realtek Semiconductor Corporation, 2014  
* All rights reserved.
* 
* Description: luna platform watchdog driver
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

extern unsigned int BSP_MHZ, BSP_SYSCLK;
static u32 WDTCTRLR_reg_val; 
#define UPDATE_WDT_REG_MIRROR (WDTCTRLR_reg_val = REG32(0xb8003268))
#define WRITEBACK_WDT_REG_MIRROR (REG32(0xb8003268) = WDTCTRLR_reg_val)
static inline void kick_wdt(void) {
	REG32(0xb8003260) = (0x1 << 31);
}

static int proc_wdt_status_r(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{	
	int pos = 0;
	unsigned int reg_val;	
	
	if(offset > 0)
		return 0;

	reg_val = WDTCTRLR_reg_val;
	pos += sprintf(&buf[pos], "WDT_E=%d, (1-enable, 0-disable)\n", ( (reg_val >> 31) & 0x1));
	pos += sprintf(&buf[pos], "LX(MHz)=%u\n", BSP_MHZ);
	pos += sprintf(&buf[pos], "WDT_CLK_SC=%d\n", ( (reg_val >> 29) & 0x3));
	pos += sprintf(&buf[pos], "PH1_TO=%d\n", ( (reg_val >> 22) & 0x1F));
	pos += sprintf(&buf[pos], "PH2_TO=%d\n", ( (reg_val >> 15) & 0x1F));
	pos += sprintf(&buf[pos], "WDT_RESET_MODE=%d\n", ( (reg_val ) & 0x3));

	return pos;
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

static void show_usage(void)
{
	printk("echo <enable> [<clock_overflow_scale> [<phase1_timer_out> [<phase2_time-out>]]] > /proc/wdt/enable\n");
	printk("<enable>: 0-disable, 1-enable\n");
	printk("<clock_overflow_scale>: 0:2^25, 1:2^26, 2:2^27, 3:2^28\n");
	printk("<phase1_time_out>: 0~31\n");
	printk("<phase2_time_out>: 0~31\n");
}

#define PROCFS_MAX_SIZE		64
static int proc_wdt_enable_w(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char procfs_buffer[PROCFS_MAX_SIZE];
	unsigned long procfs_buffer_size = 0;
	unsigned int wdt_cntl; /* 0: disable, 1: disable */
	u32 clk_sc, ph1_to, ph2_to;
	u32 argc;

	/* get buffer size */
	procfs_buffer_size = count;
	if (procfs_buffer_size > PROCFS_MAX_SIZE ) {
		procfs_buffer_size = PROCFS_MAX_SIZE;
	}
	
	/* write data to the buffer */
	if ( copy_from_user(procfs_buffer, buffer, procfs_buffer_size) ) {
		return -EFAULT;
	}

	argc = sscanf(procfs_buffer, "%u %u %u %u", &wdt_cntl, &clk_sc, &ph1_to, &ph2_to);
	switch(argc) {
	case 4:
		if(ph2_to <= 31) {
		//if((ph2_to >= 0) && (ph2_to <= 31)) {
			//update ph2 to
			WDTCTRLR_reg_val &= ~(0x1F << 15);
			WDTCTRLR_reg_val |= (ph2_to << 15);
		} else {
			show_usage();
			break;
		} 
	case 3:
		if(ph1_to <= 31) {
			//update ph1 to
			WDTCTRLR_reg_val &= ~(0x1F << 22);
			WDTCTRLR_reg_val |= (ph1_to << 22);
		} else {
			show_usage();
			break;
		} 
	case 2:
		if(clk_sc <= 4) {
			//update ph1 to
			WDTCTRLR_reg_val &= ~(0x3<< 29);
			WDTCTRLR_reg_val |= (clk_sc << 29);
		} else {
			show_usage();
			break;
		} 
	case 1:
		switch(wdt_cntl) {
		case 0:
			WDTCTRLR_reg_val &= ~(0x1 << 31);
			break;
		case 1: 
			WDTCTRLR_reg_val |= (0x1 << 31);
			break;
		default:
			show_usage();
			break;
		} 
		WRITEBACK_WDT_REG_MIRROR;
		break;

	default:
		show_usage();
		break;
	}
	
	
	return procfs_buffer_size;
}

static int proc_wdt_keepalive_w(struct file *file, const char *buffer, unsigned long count, void *data)
{
	kick_wdt();
	printk("WDT kicked!\n");
	
	return count;
}


static struct proc_dir_entry *proc_wdt_dir ;
static void luna_wdt_proc_init(void)
{
	struct proc_dir_entry *entry;
	
	/*
	*	create root directory
	*/
	proc_wdt_dir = proc_mkdir("wdt", NULL);
	if (proc_wdt_dir == NULL) {
		printk("create proc root failed!\n");
		return;
	}

	if((entry = create_proc_entry("status", 0444, proc_wdt_dir)) == NULL) {
		printk("create proc wdt/status failed!\n");
		return;
	}
	entry->read_proc  = proc_wdt_status_r;

	if((entry = create_proc_entry("registers", 0444, proc_wdt_dir)) == NULL) {
		printk("create proc wdt/registers failed!\n");
		return;
	}
	entry->read_proc  = proc_wdt_registers_r;

	if((entry = create_proc_entry("enable", 0200, proc_wdt_dir)) == NULL) {
		printk("create proc wdt/enable failed!\n");
		return;
	}
	entry->write_proc = proc_wdt_enable_w;
	entry->read_proc  = proc_wdt_status_r;

	if((entry = create_proc_entry("keepalive", 0200, proc_wdt_dir)) == NULL) {
		printk("create proc wdt/keepalive failed!\n");
		return;
	}
	entry->write_proc = proc_wdt_keepalive_w;

}

int __init luna_wdt_init(void)
{
	printk("=================================\n");
	printk("%s\n", __FUNCTION__);
	printk("=================================\n");
	/* Always kick WDT in case that it is enabled and takes a long time to travel to here for some reason to avoid some unexpect reboot */
	kick_wdt();
	luna_wdt_proc_init();

	UPDATE_WDT_REG_MIRROR;

	return 0;
}

static void __exit luna_wdt_exit (void)
{
}

module_init(luna_wdt_init);
module_exit(luna_wdt_exit);

