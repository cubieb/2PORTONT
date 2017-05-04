

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/netdevice.h>
//#include <linux/compile.h>
#include <linux/init.h>
//#undef PROFILE
//#define PROFILE int profile_idx; do { profile_idx=profile(__FUNCTION__); }while(0)
//#define PROFILE_END do { profile_end(profile_idx); }while(0)
#endif

#ifdef CONFIG_RG_PROFILE


#include <asm/unistd.h>
#include <asm/processor.h>
#include <asm/uaccess.h>
//#include <asm/mipsregs.h>


#include <asm/current.h>
#include<linux/sched.h>


//#define __IRAM_PROFILE		__attribute__ ((section(".iram-profile"))) __attribute__((nomips16)) 
#define __IRAM_PROFILE		__attribute__ ((section(".iram-profile"))) 
#define __DRAM_PROFILE		__attribute__ ((section(".dram-profile"))) 

#define MAX_HASH_IDX 8192	//must power of 2
#define MAX_HASH_IDX_BIT 13
#define MAX_HASH_WAY_BIT 4	/* 16 way */
#define FUNC_LEN	42

char funcs[MAX_HASH_IDX][FUNC_LEN]={{0}};
int lines[MAX_HASH_IDX]={0};
int lines_exit[MAX_HASH_IDX]={0};
int stop_cnt=1;
#define CPU_TYPE_5281 5281
#define CPU_TYPE_5181 5181
#define CPU_TYPE CPU_TYPE_5281

//rtl8651_romeperf_stat_t romePerfStat[ROMEPERF_INDEX_MAX];
unsigned long long cp3_count0[MAX_HASH_IDX]={0};
unsigned long long cp3_count1[MAX_HASH_IDX]={0};
unsigned long long cp3_count2[MAX_HASH_IDX]={0};
unsigned long long cp3_count3[MAX_HASH_IDX]={0};
unsigned long cp3_cnt0_temp[MAX_HASH_IDX]={0};
unsigned long cp3_cnt1_temp[MAX_HASH_IDX]={0};
unsigned long cp3_cnt2_temp[MAX_HASH_IDX]={0};
unsigned long cp3_cnt3_temp[MAX_HASH_IDX]={0};
#if (CPU_TYPE==CPU_TYPE_5281)
unsigned long long cp3_count4[MAX_HASH_IDX]={0};
unsigned long long cp3_count5[MAX_HASH_IDX]={0};
unsigned long long cp3_count6[MAX_HASH_IDX]={0};
unsigned long long cp3_count7[MAX_HASH_IDX]={0};
unsigned long cp3_cnt4_temp[MAX_HASH_IDX]={0};
unsigned long cp3_cnt5_temp[MAX_HASH_IDX]={0};
unsigned long cp3_cnt6_temp[MAX_HASH_IDX]={0};
unsigned long cp3_cnt7_temp[MAX_HASH_IDX]={0};
#endif

//manual start counter
unsigned long cp3_cnt0=0;
unsigned long cp3_cnt1=0;
unsigned long cp3_cnt2=0;
unsigned long cp3_cnt3=0;
unsigned long cp3_cnt4=0;
unsigned long cp3_cnt5=0;
unsigned long cp3_cnt6=0;
unsigned long cp3_cnt7=0;

#ifdef DEBUG_PROFILE
unsigned long cp3_cnt20=0;
unsigned long cp3_cnt21=0;
unsigned long cp3_cnt22=0;
unsigned long cp3_cnt23=0;
unsigned long cp3_cnt24=0;
unsigned long cp3_cnt25=0;
unsigned long cp3_cnt26=0;
unsigned long cp3_cnt27=0;
#endif

int init_cnt=0;

#if (CPU_TYPE==CPU_TYPE_5181)
volatile unsigned long long currCnt[4];
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
#elif (CPU_TYPE==CPU_TYPE_5281)
volatile unsigned long currCnt[8];
enum CP3_COUNTER
{
	CP3CNT_TOTAL_CYCLES = 8,
	CP3CNT_INST_FETCHS = 1,
	CP3CNT_ICACHE_MISS_TIMES = 2,
	CP3CNT_ICACHE_MISS_CYCLES = 3,
	CP3CNT_DATA_STORE_INST=4,
	CP3CNT_DATA_LOAD_INST=5,
	CP3CNT_DATA_LOAD_OR_STORE_INST=6,
	CP3CNT_EXACT_RETIRED_INST=7,
	CP3CNT_DCACHE_MISS_TIMES=0xa,
	CP3CNT_DCACHE_MISS_CYCLES=0xb,
        CP3CNT_EXCEPTION=0x2d,
        CP3CNT_INTERRUPT=0x2e,
        CP3CNT_DCACHE_HIT=0x4a,
};
#endif

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

#if (CPU_TYPE==CPU_TYPE_5181)
#define DUAL_COUNTER_MODE	0 //don't modify this value, 5181 do not have dual counter mode
u32 cp3_type_select=
#if 1 /* Inst */
	 /* Counter0 */((0x10|CP3CNT_CYCLES)<< 0) |
	                 /* Counter1 */((0x10|CP3CNT_NEW_INST_FECTH)<< 8) |
	                 /* Counter2 */((0x10|CP3CNT_NEW_INST_FETCH_CACHE_MISS)<<16) |
	                 /* Counter3 */((0x10|CP3CNT_NEW_INST_MISS_BUSY_CYCLE)<<24);
#elif 0 /* Data (LOAD+STORE) */
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

#elif (CPU_TYPE==CPU_TYPE_5281)
#define DUAL_COUNTER_MODE	1

#if 1
u32 cp3_type_select= /* Counter0 */((CP3CNT_TOTAL_CYCLES)<< 0) |
	                 /* Counter1 */((CP3CNT_INST_FETCHS)<< 8) |
	                 /* Counter2 */((CP3CNT_ICACHE_MISS_TIMES)<<16) |
	                 /* Counter3 */((CP3CNT_ICACHE_MISS_CYCLES)<<24);
#else
u32 cp3_type_select= /* Counter0 */((CP3CNT_DCACHE_MISS_TIMES)<< 0) |
			 /* Counter1 */((CP3CNT_DCACHE_MISS_CYCLES)<< 8) |
			 /* Counter2 */((CP3CNT_DCACHE_HIT)<<16) |
			 /* Counter3 */((CP3CNT_DATA_LOAD_OR_STORE_INST)<<24);

#endif


#if DUAL_COUNTER_MODE
u32 cp3_dual_counter=0xaa0f;

#if 1
u32 cp3_type_select2= /* Counter0 */((CP3CNT_DCACHE_MISS_TIMES)<< 0) |
			 /* Counter1 */((CP3CNT_DCACHE_MISS_CYCLES)<< 8) |
			 /* Counter2 */((CP3CNT_DCACHE_HIT)<<16) |
			 /* Counter3 */((CP3CNT_DATA_LOAD_OR_STORE_INST)<<24);
#else
u32 cp3_type_select2= /* Counter0 */((CP3CNT_DCACHE_MISS_TIMES)<< 0) |
			 /* Counter1 */((CP3CNT_DCACHE_MISS_CYCLES)<< 8) |
			 /* Counter2 */((CP3CNT_EXCEPTION)<<16) |
			 /* Counter3 */((CP3CNT_INTERRUPT)<<24);
#endif

#else //not dual counter mode
u32 cp3_dual_counter=0xaa00;

#endif

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

#if (CPU_TYPE==CPU_TYPE_5281)

#if DUAL_COUNTER_MODE
__asm__ __volatile__ \
("  ;\
	la		$8, cp3_type_select2	;\
	lw		$8, 0($8)		;\
	ctc3 	$8, $1				;\
");
#endif

__asm__ __volatile__ \
("	;\
	la		$8, cp3_dual_counter	;\
	lw		$8, 0($8)		;\
	ctc3	$8, $2				;\
");
	
#endif
}

static inline void CP3_COUNTER0_STOP( void )
{

//must stop counting, before read the counter.

__asm__ __volatile__ \
("	;\
	ctc3 	$0, $0			;\
");


#if (CPU_TYPE==CPU_TYPE_5281)
__asm__ __volatile__ \
("	;\
	ctc3 	$0, $1			;\
");
#endif

}

static inline void CP3_COUNTER0_GET_ALL( void )
{
#if (CPU_TYPE==CPU_TYPE_5181)
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
#elif (CPU_TYPE==CPU_TYPE_5281)
__asm__ __volatile__ \
("	;\
	la		$8, currCnt	;\
	mfc3	$9, $9			;\
	sw		$9, 0x00($8)	;\
	mfc3	$9, $8			;\
	sw		$9, 0x04($8)	;\
	mfc3	$9, $11			;\
	sw		$9, 0x08($8)	;\
	mfc3	$9, $10			;\
	sw		$9, 0x0C($8)	;\
	mfc3	$9, $13			;\
	sw		$9, 0x10($8)	;\
	mfc3	$9, $12			;\
	sw		$9, 0x14($8)	;\
	mfc3	$9, $15			;\
	sw		$9, 0x18($8)	;\
	mfc3	$9, $14			;\
	sw		$9, 0x1C($8)	;\
");
#endif
}

#if 0
void cp3_start(void)
{
	CP3_COUNTER0_INIT();
	CP3_COUNTER0_STOP();
	CP3_COUNTER0_GET_ALL();
#if (CPU_TYPE==CPU_TYPE_5181)	
	cp3_cnt0 = currCnt[0];
	cp3_cnt1 = currCnt[1];
	cp3_cnt2 = currCnt[2];
	cp3_cnt3 = currCnt[3];
#endif	
	CP3_COUNTER0_START();	
}

void cp3_stop(void)
{
	CP3_COUNTER0_STOP();
	CP3_COUNTER0_GET_ALL();	
	printk("### CP3 #### %lu,%lu,%lu,%lu\n",currCnt[0]-cp3_cnt0,currCnt[1]-cp3_cnt1,currCnt[2]-cp3_cnt2,currCnt[3]-cp3_cnt3);

}
#endif

#if 0
int __init cp3_init (void)
{
	struct proc_dir_entry *proc_root=NULL;
	proc_root = proc_mkdir("cp3", NULL);
	
	if ( create_proc_read_entry ("start", 0644, proc_root,
			(read_proc_t *)cp3_start, (void *)NULL) == NULL ) {
		printk("create proc cp3/start failed!\n");
		return -1;
	}
	if ( create_proc_read_entry ("stop", 0644, proc_root,
			(read_proc_t *)cp3_stop, (void *)NULL) == NULL ) {
		printk("create proc cp3/stop failed!\n");
		return -1;
	}
	return 0;	
}
module_init(cp3_init);
#endif



int profile_init_one(char *a, char **b, off_t c,  int d,  int *e, void *f)
{
//	struct proc_dir_entry *proc_root=NULL;
	CP3_COUNTER0_INIT();
	CP3_COUNTER0_STOP();
	stop_cnt=1;
	memset(lines_exit,0,MAX_HASH_IDX*sizeof(int));		
	memset(cp3_count0,0,MAX_HASH_IDX*sizeof(unsigned long long));
	memset(cp3_count1,0,MAX_HASH_IDX*sizeof(unsigned long long));
	memset(cp3_count2,0,MAX_HASH_IDX*sizeof(unsigned long long));	
	memset(cp3_count3,0,MAX_HASH_IDX*sizeof(unsigned long long));
#if (CPU_TYPE==CPU_TYPE_5281)
	memset(cp3_count4,0,MAX_HASH_IDX*sizeof(unsigned long long));
	memset(cp3_count5,0,MAX_HASH_IDX*sizeof(unsigned long long));
	memset(cp3_count6,0,MAX_HASH_IDX*sizeof(unsigned long long));
	memset(cp3_count7,0,MAX_HASH_IDX*sizeof(unsigned long long));
#endif
	memset(lines,0,MAX_HASH_IDX*sizeof(int));
	memset(funcs,0,MAX_HASH_IDX*FUNC_LEN);
	printk("*** profile reset ***\n");
	CP3_COUNTER0_INIT();
	CP3_COUNTER0_START();	
	printk("*** profile START ***\n");		
	stop_cnt=0;
	init_cnt++;


	
	return 0;

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


#define GIMR1		(0xb8003000)
#define GIMR2		(0xb8003004)
#define REG32(reg) 	(*((volatile unsigned int *)(reg)))


//__IRAM_PROFILE
//fastcall void profile_end(int idx)
__IRAM_PROFILE 
void profile_end(int idx)
{
	if(stop_cnt) return;	
	CP3_COUNTER0_INIT();	//20150918LUKE: enable CU of CP0.status to prevent "Coprocessor Usage exception, 7000002c"
	CP3_COUNTER0_STOP();
	++lines_exit[idx];

	//unsigned long int flags;
	//spinlock_t *lock;	
	//unsigned int backup_GIMR1=REG32(GIMR1);
	//unsigned int backup_GIMR2=REG32(GIMR2); 
	//REG32(GIMR1)=0; 
	//REG32(GIMR2)=0; 	
	//spin_lock_irqsave(&lock,flags);
	
	CP3_COUNTER0_GET_ALL();
	//for fixing compiler warnning
	{
		
#if (CPU_TYPE==CPU_TYPE_5181)
		long temp_var;
		if(cp3_count0[idx]) cp3_count0[idx] += currCnt[0];
		if(cp3_count1[idx]) cp3_count1[idx] += currCnt[1];
		if(cp3_count2[idx]) cp3_count2[idx] += currCnt[2];
		if(cp3_count3[idx]) cp3_count3[idx] += currCnt[3];		
#elif (CPU_TYPE==CPU_TYPE_5281)
#if 1
		if(cp3_cnt0_temp[idx]<=currCnt[1])
			cp3_count0[idx]+=(unsigned long long)(currCnt[1]-cp3_cnt0_temp[idx]);
		else
			cp3_count0[idx]+=(unsigned long long)(currCnt[1]+0x1000000-cp3_cnt0_temp[idx]);

		if(cp3_cnt1_temp[idx]<=currCnt[3])
			cp3_count1[idx]+=(unsigned long long)(currCnt[3]-cp3_cnt1_temp[idx]);
		else
			cp3_count1[idx]+=(unsigned long long)(currCnt[3]+0x1000000-cp3_cnt1_temp[idx]);
		
		if(cp3_cnt2_temp[idx]<=currCnt[5])
			cp3_count2[idx]+=(unsigned long long)(currCnt[5]-cp3_cnt2_temp[idx]);
		else
			cp3_count2[idx]+=(unsigned long long)(currCnt[5]+0x1000000-cp3_cnt2_temp[idx]);

		if(cp3_cnt3_temp[idx]<=currCnt[7])
			cp3_count3[idx]+=(unsigned long long)(currCnt[7]-cp3_cnt3_temp[idx]);
		else
			cp3_count3[idx]+=(unsigned long long)(currCnt[7]+0x1000000-cp3_cnt3_temp[idx]);

		if(cp3_cnt4_temp[idx]<=currCnt[0])
			cp3_count4[idx]+=(unsigned long long)(currCnt[0]-cp3_cnt4_temp[idx]);
		else
			cp3_count4[idx]+=(unsigned long long)(currCnt[0]+0x1000000-cp3_cnt4_temp[idx]);

		if(cp3_cnt5_temp[idx]<=currCnt[2])
			cp3_count5[idx]+=(unsigned long long)(currCnt[2]-cp3_cnt5_temp[idx]);
		else
			cp3_count5[idx]+=(unsigned long long)(currCnt[2]+0x1000000-cp3_cnt5_temp[idx]);

		if(cp3_cnt6_temp[idx]<=currCnt[4])
			cp3_count6[idx]+=(unsigned long long)(currCnt[4]-cp3_cnt6_temp[idx]);
		else
			cp3_count6[idx]+=(unsigned long long)(currCnt[4]+0x1000000-cp3_cnt6_temp[idx]);

		if(cp3_cnt7_temp[idx]<=currCnt[6])
			cp3_count7[idx]+=(unsigned long long)(currCnt[6]-cp3_cnt7_temp[idx]);
		else
			cp3_count7[idx]+=(unsigned long long)(currCnt[6]+0x1000000-cp3_cnt7_temp[idx]);	
#endif
#endif
		//spin_unlock_irqrestore(&lock,flags);
		//REG32(GIMR1)=backup_GIMR1;
		//REG32(GIMR2)=backup_GIMR2;
		CP3_COUNTER0_START();
	}
}


//__IRAM_PROFILE fastcall int profile(char *func)
__IRAM_PROFILE
int profile(const char *func)
{
	CP3_COUNTER0_INIT();
	CP3_COUNTER0_STOP();
	if(stop_cnt) return MAX_HASH_IDX-1;

	//for fixing compiler warnning
	{
		int idx;
		int func_len;
		int j;
		//unsigned long int flags;
		//spinlock_t *lock;
		//unsigned int backup_GIMR1=REG32(GIMR1);
		//unsigned int backup_GIMR2=REG32(GIMR2);	
		//REG32(GIMR1)=0;	
		//REG32(GIMR2)=0;				
		//spin_lock_irqsave(&lock,flags);
		
		func_len=inline_strlen((char *)func);
		idx=((
			     ((unsigned int)(func[func_len-1]-'_'+1)|
		             (unsigned int)((func[func_len-2]-'_'+1)<<8)|
		             (unsigned int)((func[func_len-3]-'_'+1)<<16) )
	     	             *
	     	             0x9e370001
	     	             *
			      ((unsigned int)(func[0]-'_'+1)|
		             (unsigned int)((func[1]-'_'+1)<<8)|
		             (unsigned int)((func[2]-'_'+1)<<16) )     	             
	     	          )>>(32-MAX_HASH_IDX_BIT+MAX_HASH_WAY_BIT))<<MAX_HASH_WAY_BIT;

		for(j=0;j<(1<<MAX_HASH_WAY_BIT);j++)
		{
			if(lines[idx+j]==0)
			{
				++lines[idx+j];
				inline_memcpy(funcs[idx+j],(char *)func,FUNC_LEN-1);
				goto out;
			}
			else
			{			
				if(inline_memcmp((u8*)func,funcs[idx+j],(func_len<(FUNC_LEN-1))?func_len:(FUNC_LEN-1))==0)
				{
					++lines[idx+j];
					goto out;
				}			
			}
		}

		if(j==(1<<MAX_HASH_WAY_BIT))
		{
			if(init_cnt>=2)
				printk("hash table FULL! func=%s can't count in profile!\n",func);
			return MAX_HASH_IDX-1;
		}

	out:
		idx+=j;
		//spin_unlock_irqrestore(&lock,flags);
		//REG32(GIMR1)=backup_GIMR1;
		//REG32(GIMR2)=backup_GIMR2;
		CP3_COUNTER0_GET_ALL();

#if (CPU_TYPE==CPU_TYPE_5181)
		cp3_count0[idx] -= currCnt[0];
		cp3_count1[idx] -= currCnt[1];
		cp3_count2[idx] -= currCnt[2];
		cp3_count3[idx] -= currCnt[3];
#elif (CPU_TYPE==CPU_TYPE_5281)	
		cp3_cnt0_temp[idx]=currCnt[1];
		cp3_cnt1_temp[idx]=currCnt[3];
		cp3_cnt2_temp[idx]=currCnt[5];
		cp3_cnt3_temp[idx]=currCnt[7];
		cp3_cnt4_temp[idx]=currCnt[0];
		cp3_cnt5_temp[idx]=currCnt[2];
		cp3_cnt6_temp[idx]=currCnt[4];
		cp3_cnt7_temp[idx]=currCnt[6];	
#endif
		CP3_COUNTER0_START();	
		return idx;
	}
}

//void test_imem(void);
int test_imem(char *a, char **b, off_t c,  int d,  int *e, void *f);


int profile_dump(char *a, char **b, off_t c,  int d,  int *e, void *f)
{
    //char aaa,*bbb;
	//bbb=test_imem(&aaa);
	CP3_COUNTER0_INIT();	
	CP3_COUNTER0_STOP();
	CP3_COUNTER0_GET_ALL();		
	stop_cnt=1;

	//for fixing compiler warnning
	{
		int i,j=0;	
		for(i=0;i<MAX_HASH_IDX;i++)
		{
			if(lines[i]!=0)
			{
				++j;

				printk("[%5d],%10d, %s",i,lines[i],&funcs[i][0]);
				printk(",%d",lines_exit[i]);
				if(lines[i]==lines_exit[i])
				{
#if (CPU_TYPE==CPU_TYPE_5181)			
					printk(",%llu,%llu,%llu,%llu",cp3_count0[i],cp3_count1[i],cp3_count2[i],cp3_count3[i]);
#elif (CPU_TYPE==CPU_TYPE_5281)
					printk(",%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu",cp3_count0[i],cp3_count1[i],cp3_count2[i],cp3_count3[i],cp3_count4[i],cp3_count5[i],cp3_count6[i],cp3_count7[i]);
#endif
				}
				else
				{
#if (CPU_TYPE==CPU_TYPE_5181)			
					printk(",0,0,0,0");
#elif (CPU_TYPE==CPU_TYPE_5281)
					printk(",0,0,0,0,0,0,0,0");
#endif
				}
				
				printk("\n");
			}
		}
		printk("Total %d functions\n",j);

		stop_cnt=0;
		CP3_COUNTER0_START();
		
		return 0;
	}

}


// proc file-system
#include "rtk_rg_struct.h"
#include "rtk_rg_internal.h"
#include <linux/time.h>

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
	printk("***%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n",cp3_cnt0,cp3_cnt1,cp3_cnt2,cp3_cnt3,cp3_cnt4,cp3_cnt5,cp3_cnt6,cp3_cnt7);
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

int profile_exeCounter_get(char *buf, char **start, off_t offset,int count, int *eof, void *data)
{
	printk("CP3 packet counter:%d\n",rg_kernel.cp3_execute_count);
	return 0;
}

int profile_exeCounter_set(struct file *file, const char *buffer, unsigned long count, void *data)
{
   rg_kernel.cp3_execute_count=_rtk_rg_pasring_proc_string_to_integer(buffer,count);
   profile_exeCounter_get(NULL,NULL,0,0,NULL,NULL);
   profile_init_one(NULL, NULL, 0, 0, NULL, NULL);
   return count;
}


#define IMEMOff (1 << 5)
#define IMEMOn (1 << 6)

#define _TO_STR(_s) #_s
#define TO_STR(s) _TO_STR(s)

#define __asm_mfc0(mfc_reg, mfc_sel) ({	\
    unsigned int __ret;\
    __asm__ __volatile__ (\
        "mfc0 %0," TO_STR(mfc_reg) "," TO_STR(mfc_sel) ";\n\t"\
        : "=r" (__ret));\
        __ret;})

#define __asm_mtc0(value, mtc_reg, mtc_sel) ({\
    unsigned int __value=(value);\
    __asm__ __volatile__ (\
        "mtc0 %0, " TO_STR(mtc_reg) "," TO_STR(mtc_sel) ";\n\t"\
        : : "r" (__value)); })

uint32_t soc_read_cctl_sel0(void) {
	return __asm_mfc0($20, 0);
}

uint32_t soc_read_cctl_sel1(void) {
	return __asm_mfc0($20, 1);
}

static void soc_write_cctl_sel0(uint32_t val) {
	__asm_mtc0(val, $20, 0);
	return;
}

static void soc_write_cctl_sel1(uint32_t val) {
	__asm_mtc0(val, $20, 1);
	return;
}

void soc_disable_imem(void) {
	volatile uint32_t cctl_val;

	/* For IMEM0 */
	/* read cp0 cctl reg. */
	cctl_val = soc_read_cctl_sel0();

	/* set on&off = 0 */
	cctl_val &= ~(IMEMOn|IMEMOff);
	soc_write_cctl_sel0(cctl_val);

	/* write off = 1 to disable */
	cctl_val |= IMEMOff;
	soc_write_cctl_sel0(cctl_val);


	/* For IMEM1 */
	/* read cp0 cctl reg. */
	cctl_val = soc_read_cctl_sel1();

	/* set on&off = 0 */
	cctl_val &= ~(IMEMOn|IMEMOff);
	soc_write_cctl_sel1(cctl_val);

	/* write off = 1 to disable */
	cctl_val |= IMEMOff;
	soc_write_cctl_sel1(cctl_val);
	
	return;
}

//CAREFUL!! Here just turn on hardware setting, memory mapping should be done before this!
void soc_enable_imem(void) {
	volatile uint32_t cctl_val;

	/* For IMEM0 */
	/* read cp0 cctl reg. */
	cctl_val = soc_read_cctl_sel0();

	/* set on&off = 0 */
	cctl_val &= ~(IMEMOn|IMEMOff);
	soc_write_cctl_sel0(cctl_val);

	/* write on = 1 to enable */
	cctl_val |= IMEMOn;
	soc_write_cctl_sel0(cctl_val);


	/* For IMEM1 */
	/* read cp0 cctl reg. */
	cctl_val = soc_read_cctl_sel1();

	/* set on&off = 0 */
	cctl_val &= ~(IMEMOn|IMEMOff);
	soc_write_cctl_sel1(cctl_val);

	/* write off = 1 to enable */
	cctl_val |= IMEMOn;
	soc_write_cctl_sel1(cctl_val);
	
	return;
}


int profile_imem_get(char *buf, char **start, off_t offset,int count, int *eof, void *data)
{
	printk("imem0:%02x, imem1:%02x\n",(soc_read_cctl_sel0()&0x60)>>5,(soc_read_cctl_sel1()&0x60)>>5);
	return 0;
}

int profile_imem_set(struct file *file, const char *buffer, unsigned long count, void *data)
{
	int i=_rtk_rg_pasring_proc_string_to_integer(buffer,count);
	if(i==0){
		soc_disable_imem();
		printk("imem0&imem1 disable now...\n");
	}else{
		soc_enable_imem();
		printk("imem0&imem1 enable now...\n");
	}
	profile_imem_get(NULL,NULL,0,0,NULL,NULL);
	return count;
}

void profile_proc_init(void)
{
	struct proc_dir_entry *p;
	CP3_COUNTER0_INIT();

	if(rg_kernel.proc_rg==NULL)
		rg_kernel.proc_rg = proc_mkdir("rg", NULL); 

	if ( create_proc_read_entry ("profileReset", 0644, rg_kernel.proc_rg,
			profile_init_one, (void *)NULL) == NULL ) {
		printk("create proc reset failed!\n");
		return;
	}
	if ( create_proc_read_entry ("profileDump", 0644, rg_kernel.proc_rg,
			profile_dump, (void *)NULL) == NULL ) {
		printk("create proc dump failed!\n");
		return;
	}
	
	p = create_proc_entry("profilePktCounter", 0644, rg_kernel.proc_rg); 
	if (p){ 	
		p->write_proc = (void *)profile_exeCounter_set;
		p->read_proc = (void *)profile_exeCounter_get;
	}else{		
		printk("create proc rg/pktCounter failed!\n");	
		return;
	}

	p = create_proc_entry("profileImem", 0644, rg_kernel.proc_rg); 
	if (p){ 	
		p->write_proc = (void *)profile_imem_set;
		p->read_proc = (void *)profile_imem_get;
	}else{		
		printk("create proc rg/profileImem failed!\n");	
		return;
	}

	if ( create_proc_read_entry ("cp3_start", 0644, rg_kernel.proc_rg,
			(read_proc_t *)cp3_manual_start, (void *)NULL) == NULL ) {
		printk("create proc rg/cp3_start failed!\n");
		return;
	}


	if ( create_proc_read_entry ("cp3_stop", 0644, rg_kernel.proc_rg,
			(read_proc_t *)cp3_manual_stop, (void *)NULL) == NULL ) {
		printk("create proc rg/cp3_stop failed!\n");
		return;
	}
	

#ifdef DEBUG_PROFILE
	if ( create_proc_read_entry ("test", 0644, rg_kernel.proc_rg,
			test_imem, (void *)NULL) == NULL ) {
		printk("create proc dump failed!\n");
		return;
	}
#endif	
	
	profile_init_one(NULL,NULL,0,0,NULL,NULL);


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
 
#ifdef DEBUG_PROFILE
__IRAM_PROFILE 
int test_imem(char *aa, char **b, off_t c,  int d,  int *e, void *f)
{
	//volatile int dmem_test_a=0,temp0=1,temp1=2;
	unsigned long flags;
	unsigned int backup_GIMR1=REG32(GIMR1);
	unsigned int backup_GIMR2=REG32(GIMR2);	
	REG32(GIMR1)=0;	
	REG32(GIMR2)=0;		
	local_irq_save(flags);

	{PROFILE;PROFILE_END;}
	{PROFILE;PROFILE_END;}
	{PROFILE;PROFILE_END;}
	{PROFILE;PROFILE_END;}
	{PROFILE;PROFILE_END;}
	{PROFILE;PROFILE_END;}
	{PROFILE;PROFILE_END;}
	{PROFILE;PROFILE_END;}
	{PROFILE;PROFILE_END;}
	{PROFILE;PROFILE_END;}
	
	CP3_COUNTER0_INIT();
	CP3_COUNTER0_STOP();

#if 1

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

#endif

	CP3_COUNTER0_GET_ALL();
	cp3_cnt0=currCnt[1];
	cp3_cnt1=currCnt[3];
	cp3_cnt2=currCnt[5];
	cp3_cnt3=currCnt[7];
	cp3_cnt4=currCnt[0];
	cp3_cnt5=currCnt[2];
	cp3_cnt6=currCnt[4];
	cp3_cnt7=currCnt[6];	
	CP3_COUNTER0_START();

    //put your code here to measure.

	CP3_COUNTER0_STOP();
	CP3_COUNTER0_GET_ALL();
	cp3_cnt20=currCnt[1];
	cp3_cnt21=currCnt[3];
	cp3_cnt22=currCnt[5];
	cp3_cnt23=currCnt[7];
	cp3_cnt24=currCnt[0];
	cp3_cnt25=currCnt[2];
	cp3_cnt26=currCnt[4];
	cp3_cnt27=currCnt[6];

	printk("1***%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n",cp3_cnt0,cp3_cnt1,cp3_cnt2,cp3_cnt3,cp3_cnt4,cp3_cnt5,cp3_cnt6,cp3_cnt7);
	printk("2***%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n",cp3_cnt20,cp3_cnt21,cp3_cnt22,cp3_cnt23,cp3_cnt24,cp3_cnt25,cp3_cnt26,cp3_cnt27);
	printk("3***%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n",cp3_cnt20-cp3_cnt0,cp3_cnt21-cp3_cnt1,cp3_cnt22-cp3_cnt2,cp3_cnt23-cp3_cnt3
		,cp3_cnt24-cp3_cnt4,cp3_cnt25-cp3_cnt5,cp3_cnt26-cp3_cnt6,cp3_cnt27-cp3_cnt7);

	local_irq_restore(flags);
	REG32(GIMR1)=backup_GIMR1;
	REG32(GIMR2)=backup_GIMR2;	
	CP3_COUNTER0_START();
	return 0;

}

#endif

#endif
