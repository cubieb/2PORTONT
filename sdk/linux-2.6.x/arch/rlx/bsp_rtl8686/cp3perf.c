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
#include <linux/kernel.h>
#include <asm/unistd.h>
#include <asm/processor.h>
#include <asm/uaccess.h>
#include <asm/rlxregs.h>
#include <linux/proc_fs.h>
#define rtlglue_malloc(size)	kmalloc(size, 0x1f0)
#define rtlglue_free(p)	kfree(p)
#define rtlglue_printf printk

#define CP3PERF_INDEX_MAX 64

struct cp3perf_stat_s {
	char *desc;
	u64 accCycle[4];
	u64 tempCycle[4];
	u32 executedNum;
	u32 hasTempCycle:1; /* true if tempCycle is valid. */
};
typedef struct cp3perf_stat_s cp3perf_stat_t;

#if defined(CONFIG_CPU_RLX5281) || defined(CONFIG_CPU_RLX4281)


#define CP3CNT_NEW_INST_FECTH 						0x01
#define CP3CNT_NEW_INST_FETCH_CACHE_MISS 			0x02
#define CP3CNT_NEW_INST_MISS_BUSY_CYCLE 			0x03
#define CP3CNT_DATA_STORE_INST 						0x04
#define CP3CNT_DATA_LOAD_INST 						0x05
#define CP3CNT_DATA_LOAD_OR_STORE_INST 				0x06
#define CP3CNT_EXACT_RETIRED_INST 					0x07
#define CP3CNT_CYCLES 								0x08
#define CP3CNT_DATA_LOAD_OR_STORE_CACHE_MISS 		0x0a
#define CP3CNT_DATA_LOAD_OR_STORE_MISS_BUSY_CYCLE	0x0b

#else

#define CP3CNT_CYCLES								0x10
#define CP3CNT_NEW_INST_FECTH						0x11
#define CP3CNT_NEW_INST_FETCH_CACHE_MISS			0x12
#define CP3CNT_NEW_INST_MISS_BUSY_CYCLE				0x13
#define CP3CNT_DATA_STORE_INST						0x14
#define CP3CNT_DATA_LOAD_INST						0x15
#define CP3CNT_DATA_LOAD_OR_STORE_INST				0x16
#define CP3CNT_EXACT_RETIRED_INST					0x17
#define CP3CNT_RETIRED_INST_FOR_PIPE_A				0x18
#define CP3CNT_RETIRED_INST_FOR_PIPE_B				0x19
#define CP3CNT_DATA_LOAD_OR_STORE_CACHE_MISS		0x1a
#define CP3CNT_DATA_LOAD_OR_STORE_MISS_BUSY_CYCLE	0x1b
#define CP3CNT_RESERVED12							0x1c
#define CP3CNT_RESERVED13							0x1d
#define CP3CNT_RESERVED14							0x1e
#define CP3CNT_RESERVED15							0x1f

#endif


/* Global variables */
static cp3perf_stat_t cp3PerfStat[CP3PERF_INDEX_MAX];
static u32 cp3perf_inited = 0;
static u32 cp3perf_enable = 1;



__IRAM static void CP3_COUNTER0_INIT( void )
{
__asm__ __volatile__ \
("  ;\
	mfc0	$8, $12			;\
	la		$9, 0x80000000	;\
	or		$8, $9			;\
	mtc0	$8, $12			;\
");
}

__IRAM static void CP3_COUNTER0_START( void )
{
	u32 reg;
#if 1 /* Inst */
	reg = /* Counter0 */((CP3CNT_CYCLES)<< 0) |
	      /* Counter1 */((CP3CNT_NEW_INST_FECTH)<< 8) |
	      /* Counter2 */((CP3CNT_NEW_INST_FETCH_CACHE_MISS)<<16) |
	      /* Counter3 */((CP3CNT_NEW_INST_MISS_BUSY_CYCLE)<<24);
#elif 1 /* Data (LOAD+STORE) */
	reg = /* Counter0 */((CP3CNT_CYCLES)<< 0) |
	      /* Counter1 */((CP3CNT_DATA_LOAD_OR_STORE_INST)<< 8) |
	      /* Counter2 */((CP3CNT_DATA_LOAD_OR_STORE_CACHE_MISS)<<16) |
	      /* Counter3 */((CP3CNT_DATA_LOAD_OR_STORE_MISS_BUSY_CYCLE)<<24);
#elif 1 /* Data (STORE) */
	reg = /* Counter0 */((CP3CNT_DATA_LOAD_INST)<< 0) |
	      /* Counter1 */((CP3CNT_DATA_STORE_INST)<< 8) |
	      /* Counter2 */((CP3CNT_DATA_LOAD_OR_STORE_CACHE_MISS)<<16) |
	      /* Counter3 */((CP3CNT_DATA_LOAD_OR_STORE_MISS_BUSY_CYCLE)<<24);
#else
#error
#endif	
	__asm__ __volatile__ (
	"	ctc3	%0, $0"
	: : "r"(reg)
	);
}

__IRAM static void CP3_COUNTER0_STOP( void )
{
__asm__ __volatile__ \
("	;\
	ctc3 	$0, $0			;\
");
}

__IRAM static u64 CP3_COUNTER0_GET( void )
{
	u64	counter;
__asm__ __volatile__ (			
	"	mfc3	$9, $8			\n"
	"	sw		$9, 4(%0)		\n"
	"	mfc3	$9, $9			\n"
	"	sw		$9, 0(%0)		\n"
	:
	: "r"(&counter)
);
	return counter;
}

__IRAM u32 test_cp3(void) {
	u32 c;
__asm__ __volatile__ (			
	"	mfc3	$9, $8			\n"
	"	sw		$9, %0			\n"	
	: "=m" (c)
);
	return c;
}

__IRAM static void CP3_COUNTER0_GET_ALL( u64 ptr[4] )
{
	__asm__ (
	"	mfc3	$9,	$9		\n"
	"	sw		$9,	0(%0)	\n"
	"	mfc3	$9,	$8		\n"
	"	sw		$9,	4(%0)	\n"
	"	mfc3	$9,	$11		\n"
	"	sw		$9,	0(%1)	\n"
	"	mfc3	$9,	$10		\n"
	"	sw		$9,	4(%1)	\n"
	"	mfc3	$9,	$13		\n"
	"	sw		$9,	0(%2)	\n"
	"	mfc3	$9,	$12		\n"
	"	sw		$9,	4(%2)	\n"
	"	mfc3	$9,	$15		\n"
	"	sw		$9,	0(%3)	\n"
	"	mfc3	$9,	$14		\n"
	"	sw		$9,	4(%3)	\n"
	:
	: "r"(&ptr[0]), "r"(&ptr[1]), "r"(&ptr[2]), "r"(&ptr[3]) 
	);
}

int cp3perfInit(void)
{
	int i;
	CP3_COUNTER0_INIT();
	CP3_COUNTER0_START();

	cp3perf_inited = 1;
	cp3perf_enable = 1;
	memset( &cp3PerfStat, 0, sizeof( cp3PerfStat ) );

	for (i = 0; i < CP3PERF_INDEX_MAX; i++) {
		cp3PerfStat[i].desc = "";
	}
	return 0;
}

int cp3perfReset(void)
{
	cp3perfInit();

	return 0;
}

int cp3perfPause( void )
{
	if ( !cp3perf_inited ) return -1;

	cp3perf_enable = 0;

	/* Louis patch: someone will disable CP3 in somewhere. */
	CP3_COUNTER0_INIT();

	CP3_COUNTER0_STOP();

	return 0;
}

int cp3perfResume( void )
{
	if ( cp3perf_inited == 0 ) return -1;

	cp3perf_enable = 1;

	/* Louis patch: someone will disable CP3 in somewhere. */
	CP3_COUNTER0_INIT();

	CP3_COUNTER0_START();

	return 0;
}

__IRAM int cp3perfEnter( u32 index )
{
	if ( !cp3perf_inited ||
	     !cp3perf_enable ) return -1;
	if ( index >= (sizeof(cp3PerfStat)/sizeof(cp3perf_stat_t)) )
		return -1;

	/* Louis patch: someone will disable CP3 in somewhere. */
	CP3_COUNTER0_INIT();

	CP3_COUNTER0_STOP();
	CP3_COUNTER0_GET_ALL(cp3PerfStat[index].tempCycle);
	cp3PerfStat[index].hasTempCycle = 1;
	CP3_COUNTER0_START();

	return 0;
}
EXPORT(cp3perfEnter);

__IRAM int cp3perfExit( u32 index )
{
	u64 currCnt[4];
	
	if ( !cp3perf_inited ||
	     !cp3perf_enable ) return -1;
	if ( index >= (sizeof(cp3PerfStat)/sizeof(cp3perf_stat_t)) )
		return -1;
	if ( cp3PerfStat[index].hasTempCycle == 0 )
		return -1;

	/* Louis patch: someone will disable CP3 in somewhere. */
	CP3_COUNTER0_INIT();

	CP3_COUNTER0_STOP();
	CP3_COUNTER0_GET_ALL(currCnt);
	cp3PerfStat[index].accCycle[0] += currCnt[0]-cp3PerfStat[index].tempCycle[0];
	cp3PerfStat[index].accCycle[1] += currCnt[1]-cp3PerfStat[index].tempCycle[1];
	cp3PerfStat[index].accCycle[2] += currCnt[2]-cp3PerfStat[index].tempCycle[2];
	cp3PerfStat[index].accCycle[3] += currCnt[3]-cp3PerfStat[index].tempCycle[3];
	cp3PerfStat[index].hasTempCycle = 0;
	cp3PerfStat[index].executedNum++;
	CP3_COUNTER0_START();

	return 0;
}
EXPORT(cp3perfExit);

int cp3perfDump( int start, int end )
{

	int i;

	cp3perf_stat_t* statSnapShot = rtlglue_malloc(sizeof(cp3perf_stat_t) * (end - start + 1) );
	if( statSnapShot == NULL )
	{
		rtlglue_printf("statSnapShot mem alloc failed\n");
		return -1;
	}

	rtlglue_printf( "index %30s %12s %8s %10s\n", "description", "accCycle", "totalNum", "Average" );

	for( i = start; i <= end; i++ )
	{
		int j;
		for( j =0; j < sizeof(cp3PerfStat[i].accCycle)/sizeof(cp3PerfStat[i].accCycle[0]); j++ )
		{
			statSnapShot[i].accCycle[j]  = cp3PerfStat[i].accCycle[j];
			statSnapShot[i].tempCycle[j] = cp3PerfStat[i].tempCycle[j];
		}
		statSnapShot[i].executedNum  = cp3PerfStat[i].executedNum;
		statSnapShot[i].hasTempCycle = cp3PerfStat[i].hasTempCycle;
	}

	for( i = start; i <= end; i++ )
	{
		if ( statSnapShot[i].executedNum == 0 )
		{
			rtlglue_printf( "[%3d] %30s %12s %8s %10s\n", i, cp3PerfStat[i].desc, "--", "--", "--" );
		}
		else
		{
			int j;
			rtlglue_printf( "[%3d] %30s ", i, cp3PerfStat[i].desc );
			for( j =0; j < sizeof(statSnapShot[i].accCycle)/sizeof(statSnapShot[i].accCycle[0]); j++ )
			{
				u32 *pAccCycle = (u32*)&statSnapShot[i].accCycle[j];
				u32 avrgCycle = /* Hi-word */ (pAccCycle[0]*(0xffffffff/statSnapShot[i].executedNum)) +
				                /* Low-word */(pAccCycle[1]/statSnapShot[i].executedNum);
				rtlglue_printf( "%12llu %8u %10u\n",
				statSnapShot[i].accCycle[j],
				statSnapShot[i].executedNum,
				avrgCycle );
				rtlglue_printf( " %3s  %30s ", "", "" );
			}
			rtlglue_printf( "\r" );
		}
	}

	rtlglue_free(statSnapShot);

	return 0;
}

static int cp3_start=0, cp3_end=9;
static int read_perf_dump(char *page, char **start, off_t off, int count, int *eof, void *data) 
 {
	unsigned long flags;
	printk("Dump range %d-%d\n", cp3_start, cp3_end);
	local_irq_save(flags);
	cp3perfDump(cp3_start, cp3_end);
	local_irq_restore(flags);
	return 0;
}

static int flush_perf_dump(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	unsigned long flags;
	char cmd[32];
	int val1, val2=0, n;
	
	if (copy_from_user(cmd, buffer, count > sizeof(cmd) ? sizeof(cmd) : count)) {
		return -EFAULT;
	}
	
	if (!strncmp(cmd, "reset", 5)) {
		local_irq_save(flags);
		cp3perfReset();
		local_irq_restore(flags);
	} else if ((n=sscanf(cmd, "%d %d",&val1,&val2))&&(n>0)) {	
		switch(n) {
		case 2:
			if ((val2>CP3PERF_INDEX_MAX)||(val2<0)) return -EINVAL;			
		case 1:
			if ((val1>CP3PERF_INDEX_MAX)||(val1<0)) return -EINVAL;
			if (val2 > val1) {
				cp3_start = val1; 
				cp3_end   = val2;
			} else {
				cp3_start = val2; 
				cp3_end   = val1;
			}
			break;
		default:
			return -EINVAL;
		}
	}
	return count;
};


static int __init cp3perf_init(void) {
	struct proc_dir_entry *pe;
	
	cp3perfInit();
	
	pe = create_proc_entry("cp3perf", S_IRUSR |S_IWUSR | S_IRGRP | S_IROTH, NULL);
	if (!pe) {
		return -EINVAL;
	}
	
	pe->read_proc  = read_perf_dump;
	pe->write_proc = flush_perf_dump;
	return 0;
}

static void __exit cp3perf_exit(void) {
}

module_init(cp3perf_init); 
module_exit(cp3perf_exit); 