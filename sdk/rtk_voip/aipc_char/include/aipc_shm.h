#ifndef __AIPC_SHM_H__
#define __AIPC_SHM_H__

#include "aipc_global.h"
#include "soc_type.h"
#include "aipc_osal.h"
#include "aipc_define.h"
#include "aipc_buffer.h"
#include "aipc_ioctl.h"
#ifdef AIPC_GOT
#include "aipc_got.h"
#endif

/*****************************************************************************
*   Data Plane Define
*****************************************************************************/
/*
*	Mailbox related define
*/
//	CPU -> DSP
#define MAIL_2DSP_TOTAL 	(64*4)	//jitter buffer*active sessions+1. left 1 for circle queue

#define MB_2DSP_TOTAL (MAIL_2DSP_TOTAL+1)
#define BC_2DSP_TOTAL (MAIL_2DSP_TOTAL+1)

typedef struct _mail_2dsp{
	u8_t	m[MAIL_2DSP_TOTAL][MAIL_2DSP_SIZE];
} mail_2dsp_t;

typedef struct _mb_2dsp{
	void * 				mb[MB_2DSP_TOTAL];	//CPU -> DSP Mailbox Array
	volatile u32_t		ins;
	volatile u32_t		del;
#ifdef QUEUE_OPERATION_CNT
	volatile u32_t		cnt_ins;
	volatile u32_t		cnt_del;
#endif
} mb_2dsp_t;

typedef struct _bc_2dsp{
	void * 				bc[BC_2DSP_TOTAL];	//CPU -> DSP Buffer Circulation Array
	volatile u32_t		ins;
	volatile u32_t		del;
#ifdef QUEUE_OPERATION_CNT
	volatile u32_t		cnt_ins;
	volatile u32_t		cnt_del;
#endif
} bc_2dsp_t;

typedef struct _mbox_2dsp{
	mb_2dsp_t 			mb;
	bc_2dsp_t			bc;
#ifdef QUEUE_OPERATION_CNT
	volatile u32_t		cnt_ins;
	volatile u32_t		cnt_del;
#endif
} mbox_2dsp_t;

//	CPU <- DSP
#define MAIL_2CPU_TOTAL		(8)		//Active session number. left 1 for circle queue

#define MB_2CPU_TOTAL (MAIL_2CPU_TOTAL+1)
#define BC_2CPU_TOTAL (MAIL_2CPU_TOTAL+1)

typedef struct _mail_2cpu{
	u8_t	m[MAIL_2CPU_TOTAL][MAIL_2CPU_SIZE];
} mail_2cpu_t;

typedef struct _mb_2cpu{
	void * 				mb[MB_2CPU_TOTAL];//CPU <- DSP Mailbox Array
	volatile u32_t		ins;
	volatile u32_t		del;
#ifdef QUEUE_OPERATION_CNT
	volatile u32_t		cnt_ins;
	volatile u32_t		cnt_del;
#endif
} mb_2cpu_t;

typedef struct _bc_2cpu{
	void * 				bc[BC_2CPU_TOTAL];//CPU <- DSP Buffer Circulation Array
	volatile u32_t		ins;
	volatile u32_t		del;
#ifdef QUEUE_OPERATION_CNT
	volatile u32_t		cnt_ins;
	volatile u32_t		cnt_del;
#endif
}bc_2cpu_t;

typedef struct _mbox_2cpu{
	mb_2cpu_t 			mb;
	bc_2cpu_t 			bc;
#ifdef QUEUE_OPERATION_CNT
	volatile u32_t		cnt_ins;
	volatile u32_t		cnt_del;
#endif
} mbox_2cpu_t;

/*
*	AIPC interrupt related define
*/
#define SIZE_2DSP_HIQ 		(64)
#define SIZE_2DSP_LOWQ 		(64)
#define SIZE_2CPU_HIQ		(64)
#define SIZE_2CPU_LOWQ		(64)

typedef struct _int_2dsp_hiq{
	volatile u32_t 	hiq[SIZE_2DSP_HIQ]; 	//Interrupt High Priority Queue
	volatile u32_t	ins;
	volatile u32_t	del;
#ifdef QUEUE_OPERATION_CNT
	volatile u32_t	cnt_ins;
	volatile u32_t	cnt_del;
#endif
} int_2dsp_hiq_t;

typedef struct _int_2dsp_lowq{
	volatile u32_t 	lowq[SIZE_2DSP_LOWQ];	//Interrupt Low Priority Queue
	volatile u32_t	ins;
	volatile u32_t	del;
#ifdef QUEUE_OPERATION_CNT
	volatile u32_t	cnt_ins;
	volatile u32_t	cnt_del;
#endif
}  int_2dsp_lowq_t;

//	AIPC Interrupt Queue CPU -> DSP
typedef struct _intq_2dsp{
	int_2dsp_hiq_t 	hiq;
	int_2dsp_lowq_t	lowq;
} intq_2dsp_t;


typedef struct _int_2cpu_hiq{
	volatile u32_t 	hiq[SIZE_2CPU_HIQ]; 	//Interrupt High Priority Queue
	volatile u32_t	ins;
	volatile u32_t	del;
#ifdef QUEUE_OPERATION_CNT
	volatile u32_t	cnt_ins;
	volatile u32_t	cnt_del;
#endif
} int_2cpu_hiq_t;

typedef struct _int_2cpu_lowq{
	volatile u32_t 	lowq[SIZE_2CPU_LOWQ];	//Interrupt Low Priority Queue
	volatile u32_t	ins;
	volatile u32_t	del;
#ifdef QUEUE_OPERATION_CNT
	volatile u32_t	cnt_ins;
	volatile u32_t	cnt_del;
#endif
}  int_2cpu_lowq_t;

//	AIPC Interrupt Queue CPU <- DSP
typedef struct _intq_2cpu{
	int_2cpu_hiq_t 	hiq;
	int_2cpu_lowq_t	lowq;
} intq_2cpu_t;

/*
*	Data plane related structure
*/
//add boot instruction
typedef struct _aipc_boot{
	u32_t cmd[4];	//for boot
} aipc_boot_t; 

//	Data plane related shared memory structure
typedef struct _aipc_mbox{
	mbox_2dsp_t	mbox_2dsp;			//CPU -> DSP mailbox
	mbox_2cpu_t	mbox_2cpu;			//CPU <- DSP mailbox
} aipc_mbox_t; 

typedef struct _aipc_mail{
	mail_2dsp_t	mail_2dsp;			//CPU ->  DSP mail
	mail_2cpu_t	mail_2cpu;			//CPU <-  DSP mail
} aipc_mail_t; 

typedef struct _aipc_intq{
	intq_2dsp_t	intq_2dsp; 			//CPU -> DSP interrupt queue
	intq_2cpu_t	intq_2cpu; 			//CPU <- DSP interrupt queue
} aipc_intq_t; 

/*****************************************************************************
*   Control Plane Define
*****************************************************************************/
/*
*	CPU -> DSP
*/
#define CMD_PHY_TOTAL			(8*4)
#define CMD_LOCAL_PHY_TOTAL		(4*4)

#define CMD_BUF_TOTAL			(CMD_PHY_TOTAL+1)
#define CMD_LOCAL_BUF_TOTAL		(CMD_LOCAL_PHY_TOTAL+1)

#define CMD_QUEUE_TOTAL			(CMD_PHY_TOTAL+CMD_LOCAL_PHY_TOTAL+1)


// Command element
typedef struct{
	u8_t					ce[CMD_SIZE];
} cmd_element_t;

typedef struct{
	u8_t					cle[CMD_SIZE];
	volatile u32_t			done;
} cmd_local_element_t;


// Command buffer queue
typedef struct{
	void *					cb[CMD_BUF_TOTAL];
	volatile u32_t			ins;
	volatile u32_t			del;
#ifdef QUEUE_OPERATION_CNT
	volatile u32_t			cnt_ins;
	volatile u32_t			cnt_del;
#endif
} cmd_buf_t;

// Command local buffer queue
typedef struct{
	void *					clb[CMD_LOCAL_BUF_TOTAL];
	volatile u32_t			ins;
	volatile u32_t			del;
#ifdef QUEUE_OPERATION_CNT
	volatile u32_t			cnt_ins;
	volatile u32_t			cnt_del;
#endif
} cmd_local_buf_t;

// Command queue
typedef struct{
	void *					cq[CMD_QUEUE_TOTAL];
	volatile u32_t			ins;
	volatile u32_t			del;
#ifdef QUEUE_OPERATION_CNT
	volatile u32_t			cnt_ins;
	volatile u32_t			cnt_del;
#endif
} cmd_queue_t;

/*
*	CPU <- DSP
*/
#define EVENT_QUEUE_TOTAL		(8*4)

typedef struct{
	u8_t					ee[EVENT_SIZE];
} event_element_t;

typedef struct{
	event_element_t 		event[EVENT_QUEUE_TOTAL];
	volatile u32_t			ins;
	volatile u32_t			del;
//	volatile u32_t			alloc;		//move to DSP cache area
#ifdef QUEUE_OPERATION_CNT
	volatile u32_t			cnt_ins;
	volatile u32_t			cnt_del;
#endif
} event_queue_t;


/*
*	Control plane related structure in shared memory
*/
/*
*	CPU -> DSP
*/
// Combine command element and command local elements. Real buffers.
typedef struct{
	cmd_element_t			cmd_e [CMD_PHY_TOTAL];
	cmd_local_element_t		cmd_le[CMD_LOCAL_PHY_TOTAL];
#ifdef QUEUE_OPERATION_CNT
	volatile u32_t			done_cnt;
#endif
} cbuf_2dsp_t;

// Combine command queue and buffers. Control index. 
typedef struct{
	cmd_buf_t				cmd_b;
	cmd_local_buf_t			cmd_lb;
	cmd_queue_t				cmd_q;
} ctrl_2dsp_t;

/*
*	CPU <- DSP
*/
// Put this only on DSP side. Cache area is OK.
typedef struct{
	volatile u8_t			commit[EVENT_QUEUE_TOTAL];
} event_commit_t;

// Event queue
typedef struct{
	event_queue_t			eq;
} cbuf_2cpu_t;


typedef struct {
	ctrl_2dsp_t 			ctrl_2dsp;			//CPU -> DSP ctrl
} aipc_ctrl_t; 


typedef struct {
	cbuf_2dsp_t 			cbuf_2dsp;			//CPU ->  DSP cbuf
	cbuf_2cpu_t 			cbuf_2cpu;			//CPU <-  DSP cbuf
} aipc_cbuf_t; 

/*****************************************************************************
*   Shared Memory Notify
*****************************************************************************/

//#if	defined(REFINE_SHM_NOTIFY) && defined(CONFIG_RTL8686_SHM_NOTIFY)
typedef struct{
	u32_t      notify_cpu;
	u32_t      notify_dsp;
} aipc_shm_notify_t;
//#endif

/*****************************************************************************
*   Statistics information
*****************************************************************************/

typedef struct{
	//	CPU data plane
	u32_t aipc_data_2dsp_alloc;
	u32_t aipc_data_2dsp_send;
	u32_t aipc_data_2cpu_ret;
	u32_t aipc_data_2cpu_recv;

	//	DSP data plane
	u32_t aipc_data_2cpu_alloc;
	u32_t aipc_data_2cpu_send;
	u32_t aipc_data_2dsp_ret;
	u32_t aipc_data_2dsp_recv;

	/*
	*	Control CPU->DSP
	*/
	/** CPU **/
	u32_t aipc_ctrl_2dsp_nofbk_alloc;
	u32_t aipc_ctrl_2dsp_fbk_alloc;
	#ifdef STATS_RETRY
	u32_t aipc_ctrl_2dsp_nofbk_alloc_retry;
	u32_t aipc_ctrl_2dsp_fbk_alloc_retry;
	#endif
	u32_t aipc_ctrl_2dsp_send;
	u32_t aipc_ctrl_2dsp_fbk_ret;
	
	/** DSP **/
	u32_t aipc_ctrl_2dsp_recv;
	u32_t aipc_ctrl_2dsp_nofbk_ret;
	u32_t aipc_ctrl_2dsp_fbk_fin;

	/*
	*	Event CPU<-DSP
	*/
	/** CPU **/
	u32_t aipc_ctrl_2cpu_recv;
	u32_t aipc_ctrl_2cpu_ret;
	
	/** DSP **/
	u32_t aipc_ctrl_2cpu_alloc;
	#ifdef STATS_RETRY
	u32_t aipc_ctrl_2cpu_alloc_retry;
	#endif
	u32_t aipc_ctrl_2cpu_send;

	//interrupt statistics
	u32_t cpu_t_dsp;
	u32_t dsp_t_cpu;

	#ifdef CONFIG_RTL8686_SHM_NOTIFY
	//shm notify
	u32_t shm_notify_cpu;
	u32_t shm_notify_dsp;
	#endif

	//error case counter
	u32_t aipc_data_error;
	u32_t aipc_ctrl_error;

	//exception case
	u32_t aipc_ctrl_2cpu_exception_send;
	
	//DSL IPC ===========================================
	//DSL CTRL
	u32_t aipc_dsl_ctrl_set ;
	u32_t aipc_dsl_ctrl_get ;
	u32_t aipc_dsl_ctrl_recv;
	u32_t aipc_dsl_ctrl_ret ;
	
	//DSL EOC
	u32_t aipc_dsl_eoc_set;
	u32_t aipc_dsl_eoc_get;
	u32_t aipc_dsl_eoc_recv_set;
	u32_t aipc_dsl_eoc_recv_get;
	u32_t aipc_dsl_eoc_ret_set;
	u32_t aipc_dsl_eoc_ret_get;
	
	//DSL Event
	u32_t aipc_dsl_event_recv;
	u32_t aipc_dsl_event_ret;
	u32_t aipc_dsl_event_alloc;
	u32_t aipc_dsl_event_alloc_retry;
	u32_t aipc_dsl_event_send;
	u32_t aipc_dsl_event_exception_send;

} aipc_stats_t;


/*****************************************************************************
*   Test Threads
*****************************************************************************/

typedef struct{
//data plane
	//CPU->DSP. RX direction
	u32_t	mbox_2dsp_send; 
	u32_t	mbox_2dsp_poll;
	
	u32_t	mbox_2dsp_counter;
	u32_t	mbox_2dsp_counter_2;
	u32_t	mbox_2dsp_counter_3;


	//CPU<-DSP. TX direction
	u32_t	mbox_2cpu_send;
	u32_t	mbox_2cpu_recv;
	
	u32_t	mbox_2cpu_counter;
	u32_t	mbox_2cpu_counter_2;
	u32_t	mbox_2cpu_counter_3;

//control plane
	//CPU->DSP. RX direction
	u32_t	ctrl_2dsp_send; 
	u32_t	ctrl_2dsp_poll;
	
	u32_t	ctrl_2dsp_counter;
	u32_t	ctrl_2dsp_counter_2;
	u32_t	ctrl_2dsp_counter_3;

	//CPU<-DSP. TX direction
	u32_t	ctrl_2cpu_send;
	u32_t	ctrl_2cpu_poll;
	
	u32_t	ctrl_2cpu_counter;
	u32_t	ctrl_2cpu_counter_2;
	u32_t	ctrl_2cpu_counter_3;

	//#ifdef	CONFIG_RTL8686_READ_DRAM_THREAD
	//u32_t   cpu_read_cnt;
	//u32_t   dsp_read_cnt;
	//u32_t   all_read_cnt;
	//#endif


//DSL IPC
	//Control CPU->DSP
	u32_t	dsl_ctrl_set;
	u32_t	dsl_ctrl_get;
	//Control CPU<-DSP
	u32_t	dsl_ctrl_recv;
	u32_t	dsl_ctrl_ret;

	//EOC CPU->DSP
	u32_t	dsl_eoc_set;
	u32_t	dsl_eoc_get;
	//EOC CPU<-DSP
	u32_t	dsl_eoc_recv_set;
	u32_t	dsl_eoc_recv_get;
	u32_t	dsl_eoc_ret_set;
	u32_t	dsl_eoc_ret_get;

	//Event CPU->DSP	
	u32_t	dsl_event_alloc;
	u32_t	dsl_event_send;
	//Event CPU<-DSP
	u32_t	dsl_event_recv;
	u32_t	dsl_event_ret;	
} aipc_thread_t;


/*****************************************************************************
*   Record DSP Logs
*****************************************************************************/

//#ifdef CONFIG_RTL8686_IPC_RECORD_DSP_LOG
typedef struct{
	volatile u32_t      enable;	//0: disable , non-zero: enable
								//1: enable , save history 
								//2: enable , overwrite history.
	volatile u32_t      ins;
	volatile u32_t      del;
	char                contents[ RECORD_DSP_LOG_SIZE ];
	volatile u32_t      touch;
} aipc_dsp_log_t;
//#endif


/*****************************************************************************
*   DSP Console
*****************************************************************************/

//#ifdef CONFIG_RTL8686_IPC_DSP_CONSOLE
typedef struct{
	volatile u32_t      	enable;
	volatile u32_t      	ins;
	volatile u32_t      	del;
	char                	read_buf[ DSP_CONSOLE_READ_BUF_SIZE ];
	
	//debug counters
	volatile u32_t      	eq_try;
	volatile u32_t      	eq_ok;
	volatile u32_t      	dq_try;
	volatile u32_t      	dq_ok;
} aipc_dsp_console_read_t;

typedef struct{
	volatile u32_t      	enable;
	volatile u32_t      	ins;
	volatile u32_t    		del;
	char                	write_buf[ DSP_CONSOLE_WRITE_BUF_SIZE ];

	//debug counters
	volatile u32_t      	eq_try;
	volatile u32_t      	eq_ok;
	volatile u32_t      	dq_try;
	volatile u32_t      	dq_ok;
} aipc_dsp_console_write_t;

typedef struct{
	volatile u32_t       		enable;
	aipc_dsp_console_read_t     dsp_console_read;
	aipc_dsp_console_write_t    dsp_console_write;
} aipc_dsp_console_t;
//#endif


/*****************************************************************************
*   IPC message
*****************************************************************************/
//#ifdef AIPC_MSG_PROBE
typedef struct{
	//#ifdef AIPC_MSG_PROBE_DSP_MEM_OFFSET
	unsigned long dsp_mem_offset;
	//#endif
	//#ifdef AIPC_MSG_PROBE_PCM_DMA_USE_SRAM
	unsigned long pcm_dma_sram_conf ;
	//#endif
	//#ifdef CONFIG_RTL8686_DSP_LED_TEST
	int dsp_led_test;
	int dsp_led_gpio_num;
	//#endif
	//#ifdef CONFIG_RTL8686_IPC_DSP_CONFIG
	int aipc_con_ch_num;
	int aipc_mirror_slic_nr;
	//#endif
	//CPU get lx bus mhz and clock. DSP probe lx info during boot up.
	int aipc_lx_bus_mhz;
	int aipc_lx_bus_clock;
	// UART setting for the other CPU
	int aipc_uart_base_addr;
	//CPU notify DSP is interrupt or polling mode.
	int aipc_int_notify_cpu2dsp;
} aipc_msg_probe_t;
//#endif

/*****************************************************************************
*   Control switch variable for both CPU/DSP
*****************************************************************************/
//Init status
#define		INIT_NR				(1<<0)
#define		INIT_OK				(1<<1)

typedef struct{
	u32_t				init;				//IPC init done
	u32_t   			dbg_mask;			//debug print mask
	aipc_stats_t 		stats;
	aipc_thread_t		thread;
}aipc_ctsw_t;



typedef struct{
	volatile unsigned char gap_0[ 128 ];
	volatile unsigned int  wr[4];
	volatile unsigned char gap_1[ 128 ];
}aipc_dummy_t;

/*****************************************************************************
*   DSL IPC
*****************************************************************************/
typedef enum AIPC_DSL_OWN {
	AIPC_DSL_OWN_CPU ,
	AIPC_DSL_OWN_DSP ,
	AIPC_DSL_OWN_MAX
} AIPC_DSL_OWN_T;
 
//Control
typedef struct {
	u8_t data[ DSL_CTRL_BUF_SIZE ];	
} aipc_dsl_ctrl_data_t;

typedef struct {
	volatile AIPC_DSL_OWN_T   own;
	aipc_dsl_ctrl_data_t      data;
	u32_t                     dummy;
} aipc_dsl_ctrl_shm_t;

//EOC
typedef struct {
	u8_t data[ DSL_CTRL_BUF_SIZE ];	
} aipc_dsl_eoc_data_t;

typedef struct {
	volatile AIPC_DSL_OWN_T   own;
	aipc_dsl_eoc_data_t       data;
	u32_t                     dummy;
} aipc_dsl_eoc_shm_t;

//Event
#define DSL_EVENT_QUEUE_TOTAL		(8)

typedef struct{
	u8_t					ee[DSL_EVENT_SIZE];
} aipc_dsl_event_element_t;

typedef struct{
	aipc_dsl_event_element_t  event[DSL_EVENT_QUEUE_TOTAL];
	volatile u32_t			  ins;
	volatile u32_t			  del;
//	volatile u32_t			  alloc;		//move to DSP cache area
#ifdef QUEUE_OPERATION_CNT    
	volatile u32_t			  cnt_ins;
	volatile u32_t			  cnt_del;
#endif
} aipc_dsl_event_queue_t;

// Put this only on DSP side. Cache area is OK.
typedef struct{
	volatile u8_t            commit[DSL_EVENT_QUEUE_TOTAL];
} aipc_dsl_event_commit_t;

// DSL event queue
typedef struct{
	aipc_dsl_event_queue_t   eq;
} aipc_dsl_event_shm_t;

/*****************************************************************************
*   SRAM/DRAM data structure define
*****************************************************************************/
typedef struct _aipc_sram{
#ifdef AIPC_BOOT
	aipc_boot_t  boot;
#endif
#ifdef AIPC_GOT
	aipc_got_t	 got;
#endif
	aipc_ctrl_t  ctrl;
	aipc_mbox_t	 mbox;
	aipc_intq_t	 intq;
//#ifdef AIPC_BARRIER
	aipc_dummy_t		dum;
//#endif
} aipc_sram_t;

typedef struct _aipc_dram{
	aipc_cbuf_t  cbuf;
	aipc_mail_t	 mail;
	aipc_ctsw_t	 ctsw;
//#if	defined(REFINE_SHM_NOTIFY) && defined(CONFIG_RTL8686_SHM_NOTIFY)
	aipc_shm_notify_t 	shm_notify;
//#endif
//#ifdef CONFIG_RTL8686_IPC_RECORD_DSP_LOG
	aipc_dsp_log_t		dsp_log;
//#endif
//#ifdef CONFIG_RTL8686_IPC_DSP_CONSOLE
	aipc_dsp_console_t  dsp_console;
//#endif
//#ifdef AIPC_MSG_PROBE
	aipc_msg_probe_t	msg_probe;
//#endif

//DSL IPC==================================
	aipc_dsl_ctrl_shm_t   dsl_ctrl;
	aipc_dsl_eoc_shm_t    dsl_eoc[2];  // 0: set , 1 get
	aipc_dsl_event_shm_t  dsl_event;

} aipc_dram_t;


/*****************************************************************************
*   Abbreviation Define
*****************************************************************************/

#if	defined(_AIPC_CPU_)||defined(_AIPC_DSP_)
#define AIPC_SHM_BASE       (0xB0000000)

#if 0  // 1.DRAM 2.SRAM

/* DRAM */
#define AIPC_DRAM_BASE		(AIPC_SHM_BASE)
//#define AIPC_DRAM_LENGTH	(0x0E0000)
#define AIPC_DRAM_LENGTH	((sizeof(aipc_dram_t) + 0x10) & (0xfffffff0))

/* SRAM */
#ifdef CONFIG_RTL8686_IPC_IDX_USE_SRAM
	#if   defined(_AIPC_CPU_)
	#define AIPC_SRAM_BASE		(DSP_BOOT_INS_SRAM_MAPPING_ADDR+0x20)
	#elif defined(_AIPC_DSP_)
	#define AIPC_SRAM_BASE		(0xbfc00000+0x20)
	#endif
#else
	#define AIPC_SRAM_BASE		(AIPC_DRAM_BASE+AIPC_DRAM_LENGTH)
	//#define AIPC_SRAM_LENGTH	(0x005000)
	#define AIPC_SRAM_LENGTH	((sizeof(aipc_sram_t) + 0x10) & (0xfffffff0))
	
#endif

#else // 1.SRAM 2.DRAM

/* SRAM */
#ifdef CONFIG_RTL8686_IPC_IDX_USE_SRAM
	#if   defined(_AIPC_CPU_)
	#define AIPC_SRAM_BASE		(DSP_BOOT_INS_SRAM_MAPPING_ADDR+0x20)
	#elif defined(_AIPC_DSP_)
	#define AIPC_SRAM_BASE		(0xbfc00000+0x20)
	#endif
#else
	#define AIPC_SRAM_BASE		(AIPC_SHM_BASE)
	#define AIPC_SRAM_LENGTH	((sizeof(aipc_sram_t) + 0x10) & (0xfffffff0))
#endif

/* DRAM */
#define AIPC_DRAM_BASE		(AIPC_SRAM_BASE+AIPC_SRAM_LENGTH)
#define AIPC_DRAM_LENGTH	((sizeof(aipc_dram_t) + 0x10) & (0xfffffff0))

#endif

/* SHM_NOTIFY */
#if	!defined(REFINE_SHM_NOTIFY) && defined(CONFIG_RTL8686_SHM_NOTIFY)
#define AIPC_SHM_NOTIFY_BASE 	(AIPC_SRAM_BASE+AIPC_SRAM_LENGTH)
#define AIPC_SHM_NOTIFY_CPU		(AIPC_SHM_NOTIFY_BASE+0x0)
#define AIPC_SHM_NOTIFY_DSP		(AIPC_SHM_NOTIFY_CPU +0x0)
#endif


/*	DRAM variable define*/
#define ADRAM 			((aipc_dram_t *)AIPC_DRAM_BASE)
/*	SRAM variable define*/
#define ASRAM 			((aipc_sram_t *)AIPC_SRAM_BASE)

//Interrupt Queue
#define INT_2CPU_HIQ 	((ASRAM)->intq.intq_2cpu.hiq)
#define INT_2CPU_LOWQ 	((ASRAM)->intq.intq_2cpu.lowq)
#define INT_2DSP_HIQ 	((ASRAM)->intq.intq_2dsp.hiq)
#define INT_2DSP_LOWQ 	((ASRAM)->intq.intq_2dsp.lowq)

//Mail box
#define MB_2DSP  		((ASRAM)->mbox.mbox_2dsp.mb)
#define BC_2DSP  		((ASRAM)->mbox.mbox_2dsp.bc)
#define MB_2CPU  		((ASRAM)->mbox.mbox_2cpu.mb)
#define BC_2CPU	 		((ASRAM)->mbox.mbox_2cpu.bc)

#define MAIL_2CPU		((ADRAM)->mail.mail_2cpu)
#define MAIL_2DSP		((ADRAM)->mail.mail_2dsp)

//boot instruction
#ifdef AIPC_BOOT
#define ABOOT			((ASRAM)->boot)
#endif

//GOT function
#ifdef AIPC_GOT
#define AGOT  			((ASRAM)->got)
#endif


//Control plane
//Command CPU->DSP
#define CMD_2DSP				((ASRAM)->ctrl.ctrl_2dsp.cmd_b)
#define CMD_LOCAL_2DSP			((ASRAM)->ctrl.ctrl_2dsp.cmd_lb)
#define CMD_QUEUE_2DSP			((ASRAM)->ctrl.ctrl_2dsp.cmd_q)

#define CBUF_2DSP				((ADRAM)->cbuf.cbuf_2dsp)
#define CBUF_CMD_2DSP			((ADRAM)->cbuf.cbuf_2dsp.cmd_e)
#define CBUF_CMD_LOCAL_2DSP		((ADRAM)->cbuf.cbuf_2dsp.cmd_le)

//Event CPU<-DSP
#define CBUF_EQ_2CPU			((ADRAM)->cbuf.cbuf_2cpu.eq)
#define CBUF_EVENT_2CPU			((ADRAM)->cbuf.cbuf_2cpu.eq.event)

//Shared Memory Notify
#if	defined(REFINE_SHM_NOTIFY) && defined(CONFIG_RTL8686_SHM_NOTIFY)
#define SHM_NOTIFY		((ADRAM)->shm_notify)
#endif

//Control switch for both CPU/DSP
#define ACTSW			((ADRAM)->ctsw)

//Statistics
#define ASTATS			((ADRAM)->ctsw.stats)

//Thread control
#define ATHREAD			((ADRAM)->ctsw.thread)

//Record DSP logs
//#ifdef CONFIG_RTL8686_IPC_RECORD_DSP_LOG
#define REC_DSP_LOG		((ADRAM)->dsp_log)
//#endif

//DSP console
//#ifdef CONFIG_RTL8686_IPC_DSP_CONSOLE
#define ADSP_CONSOLE        ((ADRAM)->dsp_console)
#define ADSP_CONSOLE_RD     ((ADRAM)->dsp_console.dsp_console_read)
#define ADSP_CONSOLE_WR	    ((ADRAM)->dsp_console.dsp_console_write)
//#endif

//MSG Probe
#define AMSG_PROBE			((ADRAM)->msg_probe)

//Dummy read/write
#define ADUM				((ASRAM)->dum)

//===========================================================================
//DSL IPC
//Control
#define DSL_CTRL_SHM            ((ADRAM)->dsl_ctrl)

//EOC
#define DSL_EOC_SHM             ((ADRAM)->dsl_eoc)
#define DSL_EOC_SHM_SET         ((ADRAM)->dsl_eoc[0])
#define DSL_EOC_SHM_GET         ((ADRAM)->dsl_eoc[1])

//Event CPU<-DSP
#define DSL_EVENT_SHM_EQ        ((ADRAM)->dsl_event.eq)
#define DSL_EVENT_SHM_EVENT     ((ADRAM)->dsl_event.eq.event)

//===========================================================================

#else	//!_AIPC_CPU_ && !//_AIPC_DSP_
#error "not supported type"
#endif

#endif
