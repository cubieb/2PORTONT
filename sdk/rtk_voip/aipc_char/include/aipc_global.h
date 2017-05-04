#ifndef _AIPC_GLOBAL_H_
#define _AIPC_GLOBAL_H_

/*
*	Reserve SRAM 128KB for GPON use
*/
#define GPON_RESV		1

/*
*	IPC control plane and data plane test
*/
//#define AIPC_CACHE_FLUSH	1

#define QUEUE_OPERATION_CNT	1

/*
*	IPC module parameters
*/
#if !defined(CONFIG_RTL8686_IPC_SINGLE_CPU)
#define AIPC_MODULE_INIT_ZONE_ENTRY		1
#endif
#define AIPC_MODULE_VOIP_IP_ENABLE		1
#define AIPC_MODULE_VOIP_SET_PCM_FS		1
//#define AIPC_MODULE_DISALBE_WDOG		1


/*
*	IPC context
*/
#define AIPC_FORCE_INTERRUPT_CONTEXT   1

/*
*	IPC control plane and data plane test
*/
#ifdef CONFIG_RTL8686_IPC_TEST_THREAD
//#define CPU_CTRL_TEST	1
//#define CPU_DATA_TEST	1
//#define DSP_CTRL_TEST	1
//#define DSP_DATA_TEST	1

#define CPU_SEND_NOFBK	1
#define CPU_SEND_FBK	1
#endif

#define INT_NOTIFY_DATA_2CPU	1
#define INT_NOTIFY_DATA_2DSP	1

/*
*	DSP use DSR for handling
*/
#define CPU_DSR_SUPPORT		1
#define DSP_DSR_SUPPORT		1


/*
*	Parameter definitions
*/
//#define MUL		0		//Multiplication of thread execution period time
//#define MUL		1		//Multiplication of thread execution period time
#define MUL		10		//Multiplication of thread execution period time
//#define MUL		100		//Use 100 for print message test. 10 for high speed test



/*
*	Boot DSP delay
*/
//#define BOOT_DSP_DELAY
#ifdef BOOT_DSP_DELAY
#define BOOT_DSP_DELAY_TIME	3000
#endif

#define KICK_DSP_DELAY_TIME	4
#define RESET_SLIC_DELAY_TIME 5
//#define KERNEL_BOOT_DELAY_TIME 5000

/*
*	Concurrent lock for IPC
*/
#define MBOX_LOCK
#define INTQ_LOCK
#define IRQ_LOCK
#define CTRL_LOCK
//#define ISR_DSR_LOCK

//#define DSL_CTRL_LOCK
//#define DSL_EOC_LOCK
#define DSL_EVENT_LOCK

#define IRQ_LOOP_PROTECT
//#define CTRL_MORE_THREADS
#define CTRL_WAIT_PROC

#define INT_COUNT

/*
*	Reboot Test Thread
*/
//#define REBOOT_THREAD

/*
*	Statistics
*/
#define	STATS_RETRY
#ifdef  STATS_RETRY
#define STATS_CTRL_RETRY_MAX    0xffff
#endif

/*
*	CTRL Limit
*/
#define CTRL_SEND_LIMIT
#ifdef  CTRL_SEND_LIMIT
#define CTRL_SEND_RETRY_MAX     0xffff
#endif

/*
*	HW MUTEX
*/
//#define IPC_HW_MUTEX_ASM	1
//#define IPC_HW_MUTEX_CCODE	1



/*
*	Refine
*/
//#define INT_QUEUE_FLUSH			1
#define SCAN_INT_LOW_QUEUE		1

#ifdef CONFIG_RTL8686_SHM_NOTIFY
#define REFINE_SHM_NOTIFY		1
#endif

//#ifdef CONFIG_RTL8686_SHM_NOTIFY
#define CPU_DSR_RX_CNT			0x20
//#endif

/*
*	VoIP config
*/
#define ROM_MAP_ADDR	0x94000000

/*
*	Check alloc
*/
//#define CHECK_ALLOC
#define CHECK_TIME_ALLOC			1


/*
*	DSP log
*/
//#ifdef CONFIG_RTL8686_IPC_RECORD_DSP_LOG
#define RECORD_DSP_LOG_DISABLE                          0
#define RECORD_DSP_LOG_ENABLE_SAVE_HISTORY              1
#define RECORD_DSP_LOG_ENABLE_OVERWRITE_HISTORY         2
#define RECORD_DSP_LOG_SIZE                             (128*1024)
#define MAX_PRINT_BUF_SIZE                              (3*1024)
//#endif

/*
*	DSP console
*/
//#ifdef CONFIG_RTL8686_IPC_DSP_CONSOLE
#define DSP_CONSOLE_MAX_USE_COUNT       1
#define DSP_CONSOLE_READ_BUF_SIZE		(16*1024)
#define DSP_CONSOLE_WRITE_BUF_SIZE		(128)
//#endif

//#define AIPC_DSP_CONSOLE_USE_TASKLET	1
#define AIPC_DSP_CONSOLE_USE_WORKQUEUE	1

#if defined(AIPC_DSP_CONSOLE_USE_TASKLET) && defined(AIPC_DSP_CONSOLE_USE_WORKQUEUE)
#error "conflict define"
#endif

          
/*
*	IPC Message
*/
#define AIPC_MSG_PROBE			1
#ifdef  AIPC_MSG_PROBE
#define AIPC_MSG_PROBE_DSP_MEM_OFFSET	1
//#define AIPC_MSG_PROBE_PCM_DMA_USE_SRAM	1
#endif

/*
*	eCos Related define
*/
#ifdef __ECOS
#endif

/*
*	AIPC Barrier
*/
#ifdef _AIPC_DSP_
#define AIPC_FORCE_CHECK_BRR
#endif

#if defined(CONFIG_RTL8685) || defined(AIPC_FORCE_CHECK_BRR)
#define AIPC_BARRIER		1
#endif

#endif /* _AIPC_GLOBAL_H_ */

