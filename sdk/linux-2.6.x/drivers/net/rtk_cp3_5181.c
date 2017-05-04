
#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/netdevice.h>
//#include <linux/compile.h>
#include <linux/init.h>
#include <linux/time.h>
#include<linux/sched.h>

#endif

#include <asm/unistd.h>
#include <asm/processor.h>
#include <asm/uaccess.h>
#include <asm/current.h>

//#define __IRAM_PROFILE		__attribute__ ((section(".iram-profile"))) __attribute__((nomips16)) 
#define __IRAM_PROFILE		__attribute__ ((section(".iram-profile"))) 
#define __DRAM_PROFILE		__attribute__ ((section(".dram-profile"))) 


//manual start counter
unsigned long cp3_cnt0=0;
unsigned long cp3_cnt1=0;
unsigned long cp3_cnt2=0;
unsigned long cp3_cnt3=0;
unsigned long cp3_cnt4=0;
unsigned long cp3_cnt5=0;
unsigned long cp3_cnt6=0;
unsigned long cp3_cnt7=0;

volatile unsigned long currCnt[8]={0};
enum CP3_COUNTER
{
	CP3CNT_CYCLES = 0,
	CP3CNT_NEW_INST_FECTH,
	CP3CNT_NEW_INST_FETCH_CACHE_MISS,
	CP3CNT_NEW_INST_MISS_BUSY_CYCLE,
	CP3CNT_DATA_STORE_INST,
	CP3CNT_DATA_LOAD_INST,
	CP3CNT_DATA_LOAD_OR_STORE_INST,
	CP3CNT_EXACT_RETIRED_INST,
	CP3CNT_RETIRED_INST_FOR_PIPE_A,
	CP3CNT_RETIRED_INST_FOR_PIPE_B,
	CP3CNT_DATA_LOAD_OR_STORE_CACHE_MISS,
	CP3CNT_DATA_LOAD_OR_STORE_MISS_BUSY_CYCLE,
	CP3CNT_RESERVED12,
	CP3CNT_RESERVED13,
	CP3CNT_RESERVED14,
	CP3CNT_RESERVED15,
};

static inline void CP3_COUNTER0_INIT( void )
{
__asm__ __volatile__ \
("  ;\
	mfc0	$8, $12			;\
	la	$9, 0x80000000		;\
	or	$8, $9			;\
	mtc0	$8, $12			;\
");
}

u32 cp3_type_select=
#if 0 /* Inst */
	 /* Counter0 */((0x10|CP3CNT_CYCLES)<< 0) |
	                 /* Counter1 */((0x10|CP3CNT_NEW_INST_FECTH)<< 8) |
	                 /* Counter2 */((0x10|CP3CNT_NEW_INST_FETCH_CACHE_MISS)<<16) |
	                 /* Counter3 */((0x10|CP3CNT_NEW_INST_MISS_BUSY_CYCLE)<<24);
#elif 1 /* Data (LOAD+STORE) */
			    /* Counter0 */((0x10|CP3CNT_CYCLES)<< 0) |
	                 /* Counter1 */((0x10|CP3CNT_DATA_LOAD_OR_STORE_INST)<< 8) |
	                 /* Counter2 */((0x10|CP3CNT_DATA_LOAD_OR_STORE_CACHE_MISS)<<16) |
	                 /* Counter3 */((0x10|CP3CNT_DATA_LOAD_OR_STORE_MISS_BUSY_CYCLE)<<24);
#elif 0 /* Data (STORE) */
			    /* Counter0 */((0x10|CP3CNT_DATA_LOAD_INST)<< 0) |
	                 /* Counter1 */((0x10|CP3CNT_DATA_STORE_INST)<< 8) |
	                 /* Counter2 */((0x10|CP3CNT_DATA_LOAD_OR_STORE_CACHE_MISS)<<16) |
	                 /* Counter3 */((0x10|CP3CNT_DATA_LOAD_OR_STORE_MISS_BUSY_CYCLE)<<24);
#elif 1
			   /* Counter0 */((0x10|CP3CNT_CYCLES)<< 0) |
	                 /* Counter1 */((0x10|CP3CNT_NEW_INST_FECTH)<< 8) |
	                 /* Counter2 */((0x10|CP3CNT_DATA_LOAD_OR_STORE_INST)<<16) |
	                 /* Counter3 */((0x10|CP3CNT_DATA_LOAD_OR_STORE_MISS_BUSY_CYCLE)<<24);
#else
#error
#endif


static inline void CP3_COUNTER0_START( void )
{
// set Count Event Type
//move data from general-purpose register ($8) to CP3 control register ($0)
__asm__ __volatile__ \
("  ;\
	la		$8, cp3_type_select	;\
	lw		$8, 0($8)		;\
	ctc3 	$8, $0				;\
");

}

static inline void CP3_COUNTER0_STOP( void )
{

//must stop counting, before read the counter.

__asm__ __volatile__ \
("	;\
	ctc3 	$0, $0			;\
");


}

static inline void CP3_COUNTER0_GET_ALL( void )
{

__asm__ __volatile__ \
("	;\
	la		$4, currCnt	;\
	mfc3	$9, $9			;\
	nop				;\
	nop				;\
	sw		$9, 0x00($4)	;\
	mfc3	$9, $8			;\
	nop				;\
	nop				;\
	sw		$9, 0x04($4)	;\
	mfc3	$9, $11			;\
	nop				;\
	nop				;\
	sw		$9, 0x08($4)	;\
	mfc3	$9, $10			;\
	nop				;\
	nop				;\
	sw		$9, 0x0C($4)	;\
	mfc3	$9, $13			;\
	nop				;\
	nop				;\
	sw		$9, 0x10($4)	;\
	mfc3	$9, $12			;\
	nop				;\
	nop				;\
	sw		$9, 0x14($4)	;\
	mfc3	$9, $15			;\
	nop				;\
	nop				;\
	sw		$9, 0x18($4)	;\
	mfc3	$9, $14			;\
	nop				;\
	nop				;\
	sw		$9, 0x1C($4)	;\
");

}





inline int inline_strlen(char *buf)
{
	int i=0;
	while(1)
	{
		if(buf[i]==0) return i;
		i++;
	}
}

inline void inline_memcpy(u8 *dest, u8 *src, int len)
{
	int i;
	for(i=0;i<len;i++)
		dest[i]=src[i];
}


inline int inline_memcmp(u8 *str1, u8 *str2, int len)
{
	int i;
	int not_eq=0;
	for(i=0;i<len;i++)
		if(str1[i]!=str2[i])
		{
			if(str1[i]>str2[i]) not_eq=1;
			else	not_eq=-1;
			break;
		}
	return not_eq;
}


// proc file-system


struct timer_list autoStop;
unsigned long autoStopData;

int cp3_manual_stop(void)
{
	CP3_COUNTER0_STOP();
	CP3_COUNTER0_GET_ALL();
	cp3_cnt0=currCnt[1];
	cp3_cnt1=currCnt[3];
	cp3_cnt2=currCnt[5];
	cp3_cnt3=currCnt[7];
	cp3_cnt4=currCnt[0];
	cp3_cnt5=currCnt[2];
	cp3_cnt6=currCnt[4];
	cp3_cnt7=currCnt[6];
	printk("CP3 manual stop...\n");
	printk("***%lu,%lu,%lu,%lu\n",cp3_cnt0,cp3_cnt1,cp3_cnt2,cp3_cnt3);
	return 0;	
}

void cp3_stop(unsigned long data)
{
	//del_timer(&autoStop);
	//cp3_manual_stop();	
	printk("5secs\n");
}



int cp3_manual_start(void)
{
	

	CP3_COUNTER0_INIT();
	CP3_COUNTER0_STOP();

	__asm__ __volatile__ \
	("	;\
		mtc3	$zero, $9			;\
		mtc3	$zero, $8			;\
		mtc3	$zero, $11			;\
		mtc3	$zero, $10			;\
		mtc3	$zero, $13			;\
		mtc3	$zero, $12			;\
		mtc3	$zero, $15			;\
		mtc3	$zero, $14			;\
	");


	printk("CP3 manual start...\n");	

	init_timer(&autoStop);
	autoStop.data=autoStopData;
	autoStop.function = cp3_stop;	
	autoStop.expires=jiffies+500;	
	add_timer(&autoStop);	
	CP3_COUNTER0_START();

	return 0;
}

struct proc_dir_entry *proc_rg;

void profile_proc_init(void)
{

	CP3_COUNTER0_INIT();

	if(proc_rg==NULL)
		proc_rg = proc_mkdir("rg", NULL); 



	if ( create_proc_read_entry ("cp3_start", 0644, proc_rg,
			(read_proc_t *)cp3_manual_start, (void *)NULL) == NULL ) {
		printk("create proc rg/cp3_start failed!\n");
		return -1;
	}


	if ( create_proc_read_entry ("cp3_stop", 0644, proc_rg,
			(read_proc_t *)cp3_manual_stop, (void *)NULL) == NULL ) {
		printk("create proc rg/cp3_stop failed!\n");
		return -1;
	}


}


int __init rtk_rg_profile_module_init(void)
{

	profile_proc_init();
	return 0;
}

void __exit rtk_rg_profile_module_exit(void)
{
}

module_init(rtk_rg_profile_module_init);
module_exit(rtk_rg_profile_module_exit);
 

