/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2003  
* All rights reserved.
* 
* Abstract: sram mapping source code.
* Todo: The function is not SMP safe. When we move the data from DRAM to SRAM
* or SRAM to DRAM, other CPUs maybe modify the data at the same time.
* (WARNING)So using the function need to take care of this thing and enhance it!
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
#include <linux/jiffies.h>
#include <asm/rlxbsp.h>

#if defined(CONFIG_RTL8670)
#include "lx4180.h"
#elif defined(CONFIG_RTL8671)
#include "lx5280.h"
#else
#include <bspchip.h>
#endif

#define OK    0
#define NOK   -1

#define Virtual2Physical(x)      	(((unsigned int)x) & 0x1fffffff)
#define Physical2Virtual(x)     	(((unsigned int)x) | 0x80000000)
#define UNCACHE_MASK            (0x20000000)    /* MIPS */
#define UNCACHE(addr)           ((unsigned int)(addr) | (UNCACHE_MASK))

unsigned long  default_maddr = CONFIG_LUNA_USE_SRAM_ADDR;
extern void *__sram;
extern void *__sram_end;

void sramflushdcache(void)
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
static void sram_reset(void){
	REG32(R_C0UMSAR0) = 0x0;
	REG32(R_C0UMSAR1) = 0x0;
	REG32(R_C0UMSAR2) = 0x0;
	REG32(R_C0UMSAR3) = 0x0;
	REG32(R_C0SRAMSAR0) = 0x0;
	REG32(R_C0SRAMSAR1) = 0x0;
	REG32(R_C0SRAMSAR2) = 0x0;
	REG32(R_C0SRAMSAR3) = 0x0;
}
static int sramInit_chk_addr(unsigned long mapping_addr){
	long section_size = 0;

	if (mapping_addr & 0x00007fff){
		/* other conditions */
		printk(KERN_WARNING "\033[1;31m==== Input address(0x%lx) is not 32K aligned ====\033[0m\n", mapping_addr);
		return NOK;
	}
	if (!(mapping_addr & 0x80000000)){
		/* other conditions */
		printk(KERN_WARNING "\033[1;31m==== Input address(0x%lx) is not effective address ====\033[0m\n", mapping_addr);
		return NOK;
	}
	if((REG32(R_C0SRAMSAR0) & 0xffffff) && !(REG32(R_C0SRAMSAR0) & 0x10000000)){
		printk(KERN_WARNING "\033[1;31mFailed! The SRAM is still used. Disable the sram mapping, first!\033[0m\n");
		printk("R_C0SRAMSAR0          :	%x\n",	REG32(R_C0SRAMSAR0));
		printk("R_C0SRAMSAR0 + 0x04   :	%x\n",	REG32(R_C0SRAMSAR0 + 0x04));
		printk("R_C0SRAMSAR0 + 0x08   :	%x\n",	REG32(R_C0SRAMSAR0 + 0x08));
		return NOK;
	}
	/* Just Warning */
	if((unsigned long)&__sram == mapping_addr){
		section_size = (unsigned long)&__sram_end - (unsigned long)&__sram;
		if( section_size == 0){
			printk(KERN_WARNING "==== Warning: The sram_section size(%lu Byte).\n", section_size);
			printk(KERN_WARNING "==== Warning: Check CONFIG_LUNA_USE_SRAM is necessary or not ====\n");
		}else
		if( section_size > BSP_SOC_SRAM_SIZE){
			printk(KERN_WARNING "\033[1;31m==== Warning: The sram_section size(%lu Byte) over physical SRAM_Size(%d Byte) ====\033[0m\n", section_size, BSP_SOC_SRAM_SIZE);
			printk(KERN_WARNING "\033[1;31m==== Warning: The only first %d(Byte) will be mapped onto SRAM ====\033[0m\n", BSP_SOC_SRAM_SIZE);
		}
	}
	
	return OK;
}
/**
 * sramInit(long mapping_addr) - call the sram mapping function
 * @mapping_addr: cpu that just started
 *
 * This function will copy BSP_SOC_SRAM_SIZE Byte memory space from DRAM to SRAM
 * And setting related Register.
 * After calling the fucnction, cpu will access the address on SRAM
 */
static int sramInit(unsigned long mapping_addr)
{
	unsigned long flags;

	preempt_disable();
	local_irq_save(flags);

	sram_reset();

 	printk("%s: SRAM Mapping begin_addr(%lx), mapping size(%d) \n", __FUNCTION__, mapping_addr, BSP_SOC_SRAM_SIZE);

	/* SRAM Mapping enable
	 *DRAM_SIZE_32KB : 0x08, 32KB*/

	/*CPU0 Unmapped Memory Segment Address Register 0*/
	REG32(R_C0UMSAR0) = (Virtual2Physical(0x9f000000))|1;
	REG32(R_C0UMSAR0 + 0x04) = DRAM_SIZE_32KB;
	/*SRAM Segment Address Register 0*/
	REG32(R_C0SRAMSAR0 + 0x08) = BSP_SOC_SRAM_BASE;
	REG32(R_C0SRAMSAR0 + 0x04) = DRAM_SIZE_32KB;
	REG32(R_C0SRAMSAR0) = (Virtual2Physical(0x9f000000))|1;
	

	/*Restore the original content*/
	memcpy((char *)(UNCACHE(0x9f000000)), (char *)mapping_addr, BSP_SOC_SRAM_SIZE);
	REG32(R_C0UMSAR0) = (Virtual2Physical(mapping_addr))|1;
	REG32(R_C0SRAMSAR0) = (Virtual2Physical(mapping_addr))|1;

	sramflushdcache();

	local_irq_restore(flags);
	preempt_enable();
	return OK;
	
}
inline static int sram_restore_chk(void){
  
	if((REG32(R_C0SRAMSAR0) & 0x01) == 0){
		printk(KERN_WARNING "\033[1;31mNo necessary! The SRAM is not used.\033[0m\n");
		printk("R_C0SRAMSAR0          :	%x\n",	REG32(R_C0SRAMSAR0));
		printk("R_C0SRAMSAR0 + 0x04   :	%x\n",	REG32(R_C0SRAMSAR0 + 0x04));
		printk("R_C0SRAMSAR0 + 0x08   :	%x\n",	REG32(R_C0SRAMSAR0 + 0x08));
		return NOK;
	}
	
	return OK;
}
/**
 * luna_sram_restore() - restore the data from SRAM to DRAM to sure the data is consistent
 * If the SRAM control Register has been enabled, the data should come from
 * DRAM and map a virtual address.
 * So someone want to use the SRAM, they should call the fucnction to restore the data to
 * DRAM(The data on SRAM maybe be modified)
 */
void luna_sram_restore(void){

	char *sram_begin;
	unsigned char *code_buf;
	unsigned long flags;

	if(sram_restore_chk() == NOK){
	  return;
	}
	sram_begin =(char *)Physical2Virtual(REG32(R_C0SRAMSAR0)&0xFFFFFFFE);

	code_buf = (unsigned char *)kmalloc(BSP_SOC_SRAM_SIZE, GFP_KERNEL);
	if(code_buf == NULL){
		printk(KERN_WARNING "==== sram unmapping failed(No mem). ====\n");
		return;
	}
	preempt_disable();
	local_irq_save(flags);
	memcpy(code_buf, sram_begin, BSP_SOC_SRAM_SIZE);
	sram_reset();
	memcpy(sram_begin, code_buf, BSP_SOC_SRAM_SIZE);
	sramflushdcache();
	local_irq_restore(flags);
	preempt_enable();
	kfree(code_buf);
	printk(KERN_WARNING "==== SRAM unmapping success ====\n");
  
}
/**
 * luna_sram_mapping(maddr) - call the function to mapping memory space from DRAM to SRAM
 * If the maddr is zero, will use the default_maddr to replace it.
 * 
 */
int luna_sram_mapping(unsigned long maddr){
  	if( maddr == 0x0 ){
		maddr = default_maddr;
	}
	if(sramInit_chk_addr(maddr) == NOK){
		return NOK;
	}
	return sramInit(maddr);
}

#ifdef DEBUG_SRAM
__SRAM_DATA
 unsigned int sram_addr[128]={1,2,3,4,5,6,7,8,9,10, 11,12,13,14,15,16,17,18,19,20,1,2,3,4,5,6,7,8,9,10, 11,12,13,14,15,16,17,18,19,20,1,2,3,4,5,6,7,8,9,10, 11,12,13,14,15,16,17,18,19,20,1,2,3,4,5,6,7,8,9,10, 11,12,13,14,15,16,17,18,19,20};


__SRAM_TEXT void sram_stress_test(void){
      int x;
      int *ptr;
      printk("%s: test start(%lu)\n", __FUNCTION__, jiffies);
      for(x=0;x<102400;x++){
      sram_addr[15]++;
      sram_addr[65]++;
      sram_addr[2]++;
      }
      print_hex_dump(KERN_INFO, "sram_data: ", DUMP_PREFIX_ADDRESS, 32, 4, sram_addr, 128*4, true);
      printk("%s: test finish(%lu)!\n", __FUNCTION__, jiffies);
      ptr=(unsigned int *)UNCACHE(sram_addr);
      printk("sram_addr[0] =%x to sram_addr[127] =%x\n",*ptr, *(ptr+127));
      printk("sram_addr[2] =%x , %x\n",*(ptr+2), sram_addr[2]);
      printk("sram_addr[15] =%x , %x\n",*(ptr+15),sram_addr[15]);
      printk("sram_addr[65] =%x , %x\n",*(ptr+65),sram_addr[65]);
      
}

void sram_mapping_info(void){
printk("\n"); 
printk("R_C0UMSAR0          :	%x\n",	REG32(R_C0UMSAR0)); 
printk("R_C0UMSAR0 + 0x04   :	%x\n",	REG32(R_C0UMSAR0 + 0x04));

/*SRAM Segment Address Register 0*/
printk("R_C0SRAMSAR0          :	%x\n",	REG32(R_C0SRAMSAR0)); 
printk("R_C0SRAMSAR0 + 0x04   :	%x\n",	REG32(R_C0SRAMSAR0 + 0x04));
printk("R_C0SRAMSAR0 + 0x08   :	%x\n",	REG32(R_C0SRAMSAR0 + 0x08)); 

}

static struct proc_dir_entry *proc_sram_entry = NULL;
static int sramtest_read_proc(char *buf,char **start,off_t offset,int count,int *eof,void *data )
{
	int len=0;

	preempt_disable();
	sram_stress_test();
	preempt_enable();
	return len;
}

static int sramtest_write_proc(struct file *file, const char *buffer,
                                    unsigned long count, void *data)
{
	char naddr[16]="";
	unsigned long newma;
	char sramtest_flag='n';
	char maddr[12];
	if (buffer && !copy_from_user(naddr, buffer, sizeof(naddr) -1 )) {  
		sscanf(naddr,"%c %s\n",&sramtest_flag, maddr);
		switch(sramtest_flag) {
		case 'n':
			break;
		case 'm':
			luna_sram_mapping(0);
			break;
		case 'r':
			luna_sram_restore();
			break;
		case 's':
			sram_mapping_info();
			break;
		case 'a':
			newma = simple_strtoul(maddr, NULL, 16);
			luna_sram_mapping(newma);
			break;
		}
		return count;
	}
	return -EFAULT;
}

static void create_sram_test_proc(void){

  proc_sram_entry = create_proc_entry("sram_test_proc", 0666, NULL);
  if (!proc_sram_entry)
  {
    printk(KERN_INFO "Error creating proc entry\n");
    return ;
  }

  proc_sram_entry->read_proc = sramtest_read_proc ;
  proc_sram_entry->write_proc = sramtest_write_proc;
  printk(KERN_INFO "proc_sram_entry initialized!\n");
  
  
  return;
}

#endif  //debug

int __init sram_init(void)
{
  printk("=========== SRAM Mapping Init ===============\n");
#ifdef DEBUG_SRAM
  create_sram_test_proc();
#endif
  if(default_maddr == 0){
    default_maddr = (unsigned long)&__sram;
  }
  luna_sram_mapping(default_maddr);
  printk("=============================================\n");
  return OK;
}
static void __exit  sram_exit(void){
    
}
module_init(sram_init);
module_exit(sram_exit);
