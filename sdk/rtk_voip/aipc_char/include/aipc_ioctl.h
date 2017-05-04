#ifndef _AIPC_IOCTL_H_
#define _AIPC_IOCTL_H_

/*
 * Buffer size
 */
#define SIZE_1K           (1*1024)
#define SIZE_1M           (1*SIZE_1K*SIZE_1K)
#define SIZE_4M           (4*SIZE_1M)
#define SIZE_5M           (5*SIZE_1M)
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
#define AIPC_BUF_SIZE     (128*SIZE_1K)
#else
#define AIPC_BUF_SIZE     SIZE_1M
#endif


/*
 * Ioctl definitions
 */
#define IOCTL_BASE 		0

typedef enum{
	IOCTL_DST_ADDR 	= IOCTL_BASE+1,
	IOCTL_SRC_ADDR,
	IOCTL_LENGTH,
	IOCTL_COPY,
	IOCTL_COPY_PADDING,

	IOCTL_WF,
	IOCTL_RF,
	IOCTL_WW,
	IOCTL_RW,
	IOCTL_BITOP_AND,
	IOCTL_BITOP_OR,
	IOCTL_BITOP_XOR,
	IOCTL_BITOP_NOT,
	IOCTL_BOOT_DSP,

	IOCTL_CPU_DRAM_UNMAP,
	IOCTL_CPU_SRAM_MAP,
	IOCTL_DSP_DRAM_UNMAP,
	IOCTL_DSP_SRAM_MAP,

	IOCTL_DSP_ENTRY,
	IOCTL_SOC_SRAM_BACKUP,
	IOCTL_ZONE_SET,
	IOCTL_TRIGGER,
	IOCTL_ROM_SET,
	IOCTL_SOC_SRAM_SET,
	IOCTL_INIT_IPC,
	
	IOCTL_RESET_SLIC,
 
//control plane
	//CPU->DSP.
	IOCTL_CTRL_2DSP_SEND,
	IOCTL_CTRL_2DSP_POLL,
	//CPU<-DSP.
	IOCTL_CTRL_2CPU_SEND,
	IOCTL_CTRL_2CPU_POLL,
		
//data plane
	//CPU->DSP. RX direction
	IOCTL_MBOX_2DSP_SEND,	
	IOCTL_MBOX_2DSP_POLL,
	//CPU<-DSP. TX direction
	IOCTL_MBOX_2CPU_SEND,
	IOCTL_MBOX_2CPU_RECV,

//debug functions
	IOCTL_DBG_PRINT,
	IOCTL_DBG_DUMP,

//hw mutex
	IOCTL_IPC_MUTEX_LOCK,
	IOCTL_IPC_MUTEX_TRYLOCK,
	IOCTL_IPC_MUTEX_UNLOCK,
	IOCTL_IPC_MUTEX_OWN,

	IOCTL_MAX
	
} aipc_ioctl;


/*
*	Control Plane
*/
//MBOX CPU->DSP
#define		CTRL_2DSP_SEND_NR		(1<<0)
#define		CTRL_2DSP_SEND_START	(1<<1)
#define		CTRL_2DSP_SEND_STOP		(1<<2)
//MBOX CPU->DSP. Polling
#define		CTRL_2DSP_POLL_NR		(1<<0)
#define		CTRL_2DSP_POLL_START	(1<<1)
#define		CTRL_2DSP_POLL_STOP		(1<<2)
//MBOX CPU<-DSP
#define		CTRL_2CPU_SEND_NR		(1<<0)
#define		CTRL_2CPU_SEND_START	(1<<1)
#define		CTRL_2CPU_SEND_STOP		(1<<2)
//MBOX CPU<-DSP. ISR or DSR
#define		CTRL_2CPU_POLL_NR		(1<<0)
#define		CTRL_2CPU_POLL_START	(1<<1)
#define		CTRL_2CPU_POLL_STOP		(1<<2)

/*
*	Data Plane
*/
//MBOX CPU->DSP
#define		MBOX_2DSP_SEND_NR		(1<<0)
#define		MBOX_2DSP_SEND_START	(1<<1)
#define		MBOX_2DSP_SEND_STOP		(1<<2)
//MBOX CPU->DSP. Polling
#define		MBOX_2DSP_POLL_NR		(1<<0)
#define		MBOX_2DSP_POLL_START	(1<<1)
#define		MBOX_2DSP_POLL_STOP		(1<<2)
//MBOX CPU<-DSP
#define		MBOX_2CPU_SEND_NR		(1<<0)
#define		MBOX_2CPU_SEND_START	(1<<1)
#define		MBOX_2CPU_SEND_STOP		(1<<2)
//MBOX CPU<-DSP. ISR or DSR
#define		MBOX_2CPU_RECV_NR		(1<<0)
#define		MBOX_2CPU_RECV_START	(1<<1)
#define		MBOX_2CPU_RECV_STOP		(1<<2)

/*
*	Debug print level
*/
#ifndef DBG_NONE
#define DBG_NONE		(0)
#define DBG_INTQ		(1<<0)
#define DBG_MBOX		(1<<1)
#define DBG_CTRL		(1<<2)
#define DBG_SYSTEM		(1<<3)
#define DBG_CONT		(1<<4)
#define DBG_DSL_CTRL            (1<<5)
#define DBG_DSL_EOC             (1<<6)
#define DBG_DSL_EVENT           (1<<7)
#define DBG_ALL			(0xFFFFFFFF)
#endif

#ifndef DUMP_CTRL
#define DUMP_NONE		(0)
#define DUMP_INTQ		(1<<0)
#define DUMP_MBOX		(1<<1)
#define DUMP_CTRL		(1<<2)
#endif

#ifndef PROCESSOR_ID_CPU
#define PROCESSOR_ID_CPU		(0)
#define PROCESSOR_ID_DSP		(1)
#endif

#define IOCTL_FLAG_MAX (0xFFFFFFFF)


#endif /* _AIPC_IOCTL_H_ */

