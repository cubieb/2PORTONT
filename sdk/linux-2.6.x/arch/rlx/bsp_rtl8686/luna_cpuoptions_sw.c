 
/*
* ----------------------------------------------------------------
* Copyright Realtek Semiconductor Corporation, 2013  
* All rights reserved.
* 
* Description: luna platform cpu option switch driver(for expriment goal)
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
#include <linux/proc_fs.h>
#include <asm/rlxregs.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/byteorder.h>
#include <asm/cacheflush.h>
#include <bspchip.h>
#include <soc.h>

/* Extended CP0 Registers $4 */
/* BPCTL (LX CP0 Reg4)
    | 31-20 |  19-18   |  17-16  |  15-6 |   5   |   4  |   3   |    2     |   1    |    0     |
    | Rsrvd | WayMask? | BTBSize | Rsrvd | CBPOn | RSOn | BTBOn | CBPInval | Rsrvd  | BTBInval |
*/
/*
#define BP_SET_FUNC_BODY()		{if (1==value) asm_mtlxc0(0x38, BPCTL_REG); else asm_mtlxc0(0x0, BPCTL_REG);}
#define BP_GET_FUNC_BODY()		{*result=(0x38==asm_mflxc0(BPCTL_REG));}
*/

#define read_lxc0_ext()		__read_32bit_lxc0_register($4, 0)
#define write_lxc0_ext(val)	__write_32bit_lxc0_register($4, 0, val)

#define prediction_enable	0x38
#define prediction_disable	0x00
static struct proc_dir_entry *proc_cpu_dir ;
static struct proc_dir_entry *proc_cpu_pred_entry;
static struct proc_dir_entry *proc_cache_entry;
static int proc_bracn_predition_r(char *buf, char **start, off_t offset,
			           int length, int *eof, void *data)
{	
	int pos = 0;
	unsigned int reg_val;	
	
	if(offset > 0)
		return 0;

	reg_val = read_lxc0_ext();
	pos += sprintf(&buf[pos], "Write 1/0 to enable/diable CPU Branch Prediction.\n");
	pos += sprintf(&buf[pos], "BPCTL=0x%x\n", reg_val);
	pos += sprintf(&buf[pos], "[WayMask ]=%d\n", ( (reg_val >> 18) & 0x2));
	pos += sprintf(&buf[pos], "[BTBSize ]=%d\n", ( (reg_val >> 16) & 0x2));
	pos += sprintf(&buf[pos], "[CBPOn   ]=%d\n", ( (reg_val >> 5) & 0x1));
	pos += sprintf(&buf[pos], "[RSOn    ]=%d\n", ( (reg_val >> 4) & 0x1));
	pos += sprintf(&buf[pos], "[BTBOn   ]=%d\n", ( (reg_val >> 3) & 0x1));
	pos += sprintf(&buf[pos], "[CBPInval]=%d\n", ( (reg_val >> 2) & 0x1));
	pos += sprintf(&buf[pos], "[Rsrvd   ]=%d\n", ( (reg_val >> 1) & 0x1));
	pos += sprintf(&buf[pos], "[BTBInval]=%d\n", ( (reg_val >> 0) & 0x1));
	
	return pos;
}

static int proc_bracn_predition_w(struct file *file, const char *buffer,
                                    unsigned long count, void *data)
{
  
  	unsigned char flag;

	if (count < 2)
		return -EFAULT;
	
	if (buffer && !copy_from_user(&flag, buffer, 1)){
		switch(flag) {
		case '1':
                  write_lxc0_ext(prediction_enable);
		  _ehb();
		  break;
		case '0':
		  write_lxc0_ext(prediction_disable);
		  _ehb();
		  break;
		default:
		    printk("Not support setting value(%d)\n", flag);
		}
		return count;
	}
	return -EFAULT;

  
}

/* cache operation */
static inline void luna_flush_cache_all(void){
	if(__flush_cache_all == NULL){
	  printk("flush_cache_all failed.\n");
	}
         __flush_cache_all();
	 printk("flush_cache_all done.\n");
}
static inline void luna_flush_write_buffer(void){
	write_buffer_flush();
	printk("write_buffer_flush done.\n");
}
static int proc_cacheflush_w(struct file *file, const char *buffer,
                                    unsigned long count, void *data)
{
  	unsigned char flag;

	if (count < 2)
		return -EFAULT;
	
	if (buffer && !copy_from_user(&flag, buffer, 1)) {
		switch(flag) {
		case '2':
		      luna_flush_write_buffer();
		      break;
		case '1':
		      luna_flush_cache_all();
		      break;
		case '0':
		default:
		    printk("Not support the input value(%d)\n", flag);
		}
		
		return count;
	}
	return -EFAULT;
  
}
static int proc_cacheflush_r(char *buf, char **start, off_t offset,
			           int length, int *eof, void *data)
{
	int pos = 0;
	
	if(offset > 0)
		return 0;
	pos += sprintf(&buf[pos], "Write 1 to flush all cache\n");
	pos += sprintf(&buf[pos], "Write 2 to flush write buffer\n");
	
	pos += sprintf(&buf[pos], "icache: %dkB/%dB\n", cpu_icache_size >> 10, cpu_icache_line);
	pos += sprintf(&buf[pos], "dcache: %dkB/%dB\n", cpu_dcache_size >> 10, cpu_dcache_line);
	pos += sprintf(&buf[pos], "scache: %dkB/%dB\n", cpu_scache_size >> 10, cpu_scache_line);
	
	return pos;
}
    
static int luna_cpu_bp_proc_init(void)
{

	proc_cpu_dir = proc_mkdir("luna_cpu_sw", NULL);
	if (proc_cpu_dir == NULL) {
		printk("create proc dir: luna_cpu_sw failed!\n");
		return -1;
	}

	if((proc_cpu_pred_entry = create_proc_entry("luna_cpu_prediction", 0644, proc_cpu_dir)) == NULL) {
		printk("create proc: luna_cpu_prediction failed!\n");
		return -1;
	}

	proc_cpu_pred_entry->read_proc  = proc_bracn_predition_r;
	proc_cpu_pred_entry->write_proc  = proc_bracn_predition_w;

	return 0;
}

static int luna_cpu_cache_proc_init(void)
{

	if((proc_cache_entry = create_proc_entry("luna_cacheflush", 0644, proc_cpu_dir)) == NULL) {
		printk("create proc: luna_cpu_prediction failed!\n");
		return -1;
	}

	proc_cache_entry->read_proc  = proc_cacheflush_r;
	proc_cache_entry->write_proc  = proc_cacheflush_w;

        return 0;
}


int __init luna_cpu_switch_init(void)
{

	printk("============= %s ============\n", __func__);
	if(luna_cpu_bp_proc_init() == -1)   return -1;
	if(luna_cpu_cache_proc_init() == -1)   return -1;

	return 0;
}

static void __exit luna_cpu_switch_exit (void)
{
	remove_proc_entry("luna_cpu_prediction", proc_cpu_dir);  
	remove_proc_entry("luna_cpu_sw", NULL);  
}

module_init(luna_cpu_switch_init);
module_exit(luna_cpu_switch_exit);

