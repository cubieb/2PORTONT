#ifdef __KERNEL__
#include <linux/kernel.h>	/* printk() */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>	/* copy_*_user */
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
#include <linux/seq_file.h>    // for sequence files
#endif

#include "./include/dram_share.h"
#include "./include/aipc_dev.h"		/* local definitions */
#include "./include/aipc_reg.h"
#include "./include/soc_type.h"
#include "./include/aipc_mem.h"
#include "./include/aipc_debug.h"

#define PROC_AIPC_DEV_DIR                   DEVICE_NAME

/* switch */
#define PROC_AIPC_DEV_DIR_SWITCH            "switch"
#define PROC_AIPC_DEV_DBG_PRINT             "dbg_print"

/* status */
#define PROC_AIPC_DEV_DIR_STATUS            "status"
#define PROC_AIPC_DEV_IPC_COUNTERS          "ipc_counters"
#define PROC_AIPC_DEV_THREAD          		"thread"
#define PROC_AIPC_DEV_REGISTER          	"register"
#define PROC_AIPC_DEV_SHM                   "shm"

#ifdef CONFIG_RTL8686_IPC_RECORD_DSP_LOG
/* dsp */
#define PROC_AIPC_DEV_DIR_DSP               "dsp"

/* dsp log */
#define PROC_AIPC_DEV_DIR_DSP_LOG           "log"
#define PROC_AIPC_DEV_DSP_LOG_ENABLE        "enable"
#define PROC_AIPC_DEV_DSP_LOG_CLEAR         "clear"
#define PROC_AIPC_DEV_DSP_LOG_CONTENTS      "contents"
#define PROC_AIPC_DEV_DSP_LOG_INDEX         "index"
#endif

#ifdef CONFIG_RTL8686_IPC_DSP_CONSOLE
/* dsp console */
#define PROC_AIPC_DEV_DIR_DSP_CONSOLE           "console"

#define PROC_AIPC_DEV_DSP_CONSOLE_ENABLE        "enable"

#define PROC_AIPC_DEV_DSP_CONSOLE_READ_ENABLE   "read_enable"
#define PROC_AIPC_DEV_DSP_CONSOLE_READ_CLEAR    "read_clear"
#define PROC_AIPC_DEV_DSP_CONSOLE_READ_INDEX    "read_index"

#define PROC_AIPC_DEV_DSP_CONSOLE_WRITE_ENABLE   "write_enable"
#define PROC_AIPC_DEV_DSP_CONSOLE_WRITE_CLEAR    "write_clear"
#define PROC_AIPC_DEV_DSP_CONSOLE_WRITE_INDEX    "write_index"
#endif

/* misc */
#define PROC_AIPC_DEV_DIR_MISC              "misc"
#define PROC_AIPC_DEV_OP                    "operations"
#define PROC_AIPC_DEV_PHYADDR               "phymem_addr"
#define PROC_AIPC_DEV_LOGADDR               "logmem_addr"


/* DSL IPC */
#ifdef CONFIG_RTL8686_IPC_DSL_IPC
#define PROC_AIPC_DEV_DIR_DSL               "dsl"
#define PROC_AIPC_DEV_DSL_IPC_COUNTERS      "ipc_counters"
#endif


static struct proc_dir_entry *proc_aipc_dev_dir ;

extern int aipc_ctrl_dump(char *buf);
extern int aipc_intq_dump(char *buf);
extern int aipc_mbox_dump(char *buf);

#ifdef CONFIG_RTL8686_IPC_RECORD_DSP_LOG
extern unsigned int *rec_dsp_log_enable;
extern unsigned int *rec_dsp_log_ins;
extern unsigned int *rec_dsp_log_del;
extern unsigned int *rec_dsp_log_touch;
extern const char   *rec_dsp_log_contents;
extern int aipc_record_dsp_log_empty(void);
extern int aipc_record_dsp_log_full(void);
extern unsigned int aipc_record_dsp_log_contents_use(void);
typedef int (*ft_aipc_dsp_log_add)(char);
extern ft_aipc_dsp_log_add   fp_aipc_dsp_log_add;
#endif

#ifdef CONFIG_RTL8686_IPC_DSL_IPC
extern int aipc_dsl_proc_ctrl_dump (char *buf);
extern int aipc_dsl_proc_eoc_dump  (char *buf);
extern int aipc_dsl_proc_event_dump(char *buf);
#endif

/*
*	switch related
*/
static int 
proc_switch_dbg_print_w (struct file *file, const char *buffer, unsigned long count, void *data)
{
	char tmpbuf[100] = {0};
	int flag = 0;

	if (buffer && !copy_from_user(tmpbuf, buffer, count)) {

		sscanf(tmpbuf, "%d", &flag);
		
		if (flag == 0) // disable
			ACTSW.dbg_mask = 0;
		
		else if(flag > 0)
			ACTSW.dbg_mask = flag;
		
		else
			SDEBUG("wrong number\n");
	}

	printk("\nDebug mask:\n");
	printk("\tdbg_mask=0x%x\n" , ACTSW.dbg_mask);


	return count;
}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int proc_switch_dbg_print_r(struct seq_file *f, void *v)
{
    int n = 0;

	seq_printf( f, "dbg_mask=0x%x\n" , ACTSW.dbg_mask);
	
    return n;
}
#else
static int 
proc_switch_dbg_print_r (char *buf, char **start, off_t off, int count, int *eof, void *data)

{
    int n = 0;

    if( off ) { /* In our case, we write out all data at once. */
        *eof = 1;
        return 0;
    }


	n += sprintf(buf , "dbg_mask=0x%x\n" , ACTSW.dbg_mask);

    *eof = 1;
	
    return n;
}
#endif

/*
*	status related
*/
static int 
proc_status_ipc_counters_w (struct file *file, const char *buffer, unsigned long count, void *data)
{

	char tmpbuf[100] = {0};
	int flag = 0;

	if (buffer && !copy_from_user(tmpbuf, buffer, count)) {

		sscanf(tmpbuf, "%d", &flag);
		
		if (flag == 0){ // reset counters
			memset(&ASTATS ,  0 , sizeof(aipc_stats_t));
			
			printk("Clear status counters\n");
		}
		else
			SDEBUG("wrong number\n");
	}

	return count;
}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int aipc_dbg_thread_dump(struct seq_file *f)
{
	int n = 0;

	seq_printf( f, "\nThread counters:\n");
	
	seq_printf( f, "\tctrl_2dsp_counter = %u\n" ,	ATHREAD.ctrl_2dsp_counter);
	seq_printf( f, "\tctrl_2cpu_counter = %u\n" ,	ATHREAD.ctrl_2cpu_counter);
		
	#ifdef	CONFIG_RTL8686_IPC_TEST_THREAD_2
	seq_printf( f, "\tctrl_2dsp_counter_2 = %u\n" , ATHREAD.ctrl_2dsp_counter_2);
	seq_printf( f, "\tctrl_2cpu_counter_2 = %u\n" , ATHREAD.ctrl_2cpu_counter_2);
	#endif
	#ifdef	CONFIG_RTL8686_IPC_TEST_THREAD_3
	seq_printf( f, "\tctrl_2dsp_counter_3 = %u\n" , ATHREAD.ctrl_2dsp_counter_3);
	seq_printf( f, "\tctrl_2cpu_counter_3 = %u\n" , ATHREAD.ctrl_2cpu_counter_3);
	#endif

	
	seq_printf( f, "\tmbox_2dsp_counter = %u\n" ,	ATHREAD.mbox_2dsp_counter);
	seq_printf( f, "\tmbox_2cpu_counter = %u\n" ,	ATHREAD.mbox_2cpu_counter);
	
	#ifdef	CONFIG_RTL8686_IPC_TEST_THREAD_2
	seq_printf( f, "\tmbox_2dsp_counter_2 = %u\n" , ATHREAD.mbox_2dsp_counter_2);
	seq_printf( f, "\tmbox_2cpu_counter_2 = %u\n" , ATHREAD.mbox_2cpu_counter_2);
	#endif

	#ifdef	CONFIG_RTL8686_IPC_TEST_THREAD_3
	seq_printf( f, "\tmbox_2dsp_counter_3 = %u\n" , ATHREAD.mbox_2dsp_counter_3);
	seq_printf( f, "\tmbox_2cpu_counter_3 = %u\n" , ATHREAD.mbox_2cpu_counter_3);
	#endif

	#ifdef CONFIG_RTL8686_READ_DRAM_THREAD
	seq_printf( f, "\tcpu_read_cnt = %u\n" , ATHREAD.cpu_read_cnt);
	seq_printf( f, "\tdsp_read_cnt = %u\n" , ATHREAD.dsp_read_cnt);
	seq_printf( f, "\tall_read_cnt = %u\n" , ATHREAD.all_read_cnt);
	#endif

	return n;
}
#else
static int
aipc_dbg_thread_dump(char *buf)
{
	int n = 0;

	n += sprintf(buf   , "\nThread counters:\n");
	
	n += sprintf(buf+n , "\tctrl_2dsp_counter = %u\n" ,	ATHREAD.ctrl_2dsp_counter);
	n += sprintf(buf+n , "\tctrl_2cpu_counter = %u\n" ,	ATHREAD.ctrl_2cpu_counter);
		
	#ifdef	CONFIG_RTL8686_IPC_TEST_THREAD_2
	n += sprintf(buf+n , "\tctrl_2dsp_counter_2 = %u\n" , ATHREAD.ctrl_2dsp_counter_2);
	n += sprintf(buf+n , "\tctrl_2cpu_counter_2 = %u\n" , ATHREAD.ctrl_2cpu_counter_2);
	#endif
	#ifdef	CONFIG_RTL8686_IPC_TEST_THREAD_3
	n += sprintf(buf+n , "\tctrl_2dsp_counter_3 = %u\n" , ATHREAD.ctrl_2dsp_counter_3);
	n += sprintf(buf+n , "\tctrl_2cpu_counter_3 = %u\n" , ATHREAD.ctrl_2cpu_counter_3);
	#endif

	
	n += sprintf(buf+n , "\tmbox_2dsp_counter = %u\n" ,	ATHREAD.mbox_2dsp_counter);
	n += sprintf(buf+n , "\tmbox_2cpu_counter = %u\n" ,	ATHREAD.mbox_2cpu_counter);
	
	#ifdef	CONFIG_RTL8686_IPC_TEST_THREAD_2
	n += sprintf(buf+n , "\tmbox_2dsp_counter_2 = %u\n" , ATHREAD.mbox_2dsp_counter_2);
	n += sprintf(buf+n , "\tmbox_2cpu_counter_2 = %u\n" , ATHREAD.mbox_2cpu_counter_2);
	#endif

	#ifdef	CONFIG_RTL8686_IPC_TEST_THREAD_3
	n += sprintf(buf+n , "\tmbox_2dsp_counter_3 = %u\n" , ATHREAD.mbox_2dsp_counter_3);
	n += sprintf(buf+n , "\tmbox_2cpu_counter_3 = %u\n" , ATHREAD.mbox_2cpu_counter_3);
	#endif

	#ifdef CONFIG_RTL8686_READ_DRAM_THREAD
	n += sprintf(buf+n , "\tcpu_read_cnt = %u\n" , ATHREAD.cpu_read_cnt);
	n += sprintf(buf+n , "\tdsp_read_cnt = %u\n" , ATHREAD.dsp_read_cnt);
	n += sprintf(buf+n , "\tall_read_cnt = %u\n" , ATHREAD.all_read_cnt);
	#endif

	return n;
}
#endif

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int
aipc_dbg_counter_dump(struct seq_file *f)
{
	int n = 0;

	seq_printf( f, "Statistics:\n");

	/*	  
	*	 data plane
	*/
	seq_printf( f, "   Data plane \n");
	//	  CPU->DSP
	seq_printf( f, "\t (CPU) data CPU->DSP: \n");
	seq_printf( f, "\t aipc_data_2dsp_alloc  = %u	 BC_2DSP.cnt_del = %u\n"  , ASTATS.aipc_data_2dsp_alloc , BC_2DSP.cnt_del);
	seq_printf( f, "\t aipc_data_2dsp_send	 = %u	 MB_2DSP.cnt_ins = %u\n"  , ASTATS.aipc_data_2dsp_send	, MB_2DSP.cnt_ins);
	seq_printf( f, "\t (DSP) data CPU->DSP: \n");
	seq_printf( f, "\t aipc_data_2dsp_recv	 = %u	 MB_2DSP.cnt_del = %u\n"  , ASTATS.aipc_data_2dsp_recv	, MB_2DSP.cnt_del);
	seq_printf( f, "\t aipc_data_2dsp_ret	 = %u	 BC_2DSP.cnt_ins = %u\n"  , ASTATS.aipc_data_2dsp_ret	, BC_2DSP.cnt_ins);

	//	  CPU<-DSP
	seq_printf( f, "\n\t (CPU) data CPU<-DSP: \n");
	seq_printf( f, "\t aipc_data_2cpu_recv	 = %u	 MB_2CPU.cnt_del = %u\n"  , ASTATS.aipc_data_2cpu_recv	, MB_2CPU.cnt_del);
	seq_printf( f, "\t aipc_data_2cpu_ret	 = %u	 BC_2CPU.cnt_ins = %u\n"  , ASTATS.aipc_data_2cpu_ret	, BC_2CPU.cnt_ins);
	seq_printf( f, "\t (DSP) data CPU<-DSP: \n");
	seq_printf( f, "\t aipc_data_2cpu_alloc  = %u	 BC_2CPU.cnt_del = %u\n"  , ASTATS.aipc_data_2cpu_alloc , BC_2CPU.cnt_del);
	seq_printf( f, "\t aipc_data_2cpu_send	 = %u	 MB_2CPU.cnt_ins = %u\n"  , ASTATS.aipc_data_2cpu_send	, MB_2CPU.cnt_ins);

	/*	  
	*	 control plane plane
	*/
	seq_printf( f, "\n	 Control plane \n");	
	//	Control
	seq_printf( f, "\t (CPU) control CPU->DSP: \n");
	seq_printf( f, "\t aipc_ctrl_2dsp_nofbk_alloc  = %u  CMD_2DSP.cnt_del = %u\n"  , 
		ASTATS.aipc_ctrl_2dsp_nofbk_alloc , 	 CMD_2DSP.cnt_del);
	seq_printf( f, "\t aipc_ctrl_2dsp_fbk_alloc    = %u  CMD_LOCAL_2DSP.cnt_del = %u\n"  ,
		ASTATS.aipc_ctrl_2dsp_fbk_alloc   , 	 CMD_LOCAL_2DSP.cnt_del);

#ifdef STATS_RETRY 
	seq_printf( f, "\t aipc_ctrl_2dsp_nofbk_alloc_retry = %u\n" ,
		ASTATS.aipc_ctrl_2dsp_nofbk_alloc_retry);
	seq_printf( f, "\t aipc_ctrl_2dsp_fbk_alloc_retry = %u\n" ,
		ASTATS.aipc_ctrl_2dsp_fbk_alloc_retry);
#endif	
	
	seq_printf( f, "\t aipc_ctrl_2dsp_send		   = %u  CMD_QUEUE_2DSP.cnt_ins = %u\n"  , 
		ASTATS.aipc_ctrl_2dsp_send ,			 CMD_QUEUE_2DSP.cnt_ins);
	seq_printf( f, "\t aipc_ctrl_2dsp_fbk_ret	   = %u  CMD_LOCAL_2DSP.cnt_ins = %u\n"  , 
		ASTATS.aipc_ctrl_2dsp_fbk_ret , 		 CMD_LOCAL_2DSP.cnt_ins);
	
	seq_printf( f, "\t (DSP) control CPU->DSP: \n");
	seq_printf( f, "\t aipc_ctrl_2dsp_recv		   = %u  CMD_QUEUE_2DSP.cnt_del = %u\n"  , 
		ASTATS.aipc_ctrl_2dsp_recv ,			 CMD_QUEUE_2DSP.cnt_del);
	seq_printf( f, "\t aipc_ctrl_2dsp_nofbk_ret    = %u  CMD_2DSP.cnt_ins = %u\n" ,    
		ASTATS.aipc_ctrl_2dsp_nofbk_ret ,		 CMD_2DSP.cnt_ins );
	seq_printf( f, "\t aipc_ctrl_2dsp_fbk_fin	   = %u  CBUF_2DSP.done_cnt = %u\n"  , 
		ASTATS.aipc_ctrl_2dsp_fbk_fin , 		 CBUF_2DSP.done_cnt);

	//	Event
	seq_printf( f, "\n\t (CPU) event CPU<-DSP: \n");	
	seq_printf( f, "\t aipc_ctrl_2cpu_recv	 = %u\n"  ,    ASTATS.aipc_ctrl_2cpu_recv);
	seq_printf( f, "\t aipc_ctrl_2cpu_ret	 = %u  CBUF_EQ_2CPU.cnt_del = %u\n"  , 
		ASTATS.aipc_ctrl_2cpu_ret , 			 CBUF_EQ_2CPU.cnt_del);
	
	seq_printf( f, "\t (DSP) event CPU<-DSP: \n");
	seq_printf( f, "\t aipc_ctrl_2cpu_alloc  = %u\n"  ,    ASTATS.aipc_ctrl_2cpu_alloc);

#ifdef STATS_RETRY 
	seq_printf( f, "\t aipc_ctrl_2cpu_alloc_retry  = %u\n"	, ASTATS.aipc_ctrl_2cpu_alloc_retry);
#endif
	
	seq_printf( f, "\t aipc_ctrl_2cpu_send	 = %u  CBUF_EQ_2CPU.cnt_ins = %u\n"  , 
		ASTATS.aipc_ctrl_2cpu_send ,	   CBUF_EQ_2CPU.cnt_ins);

	/*
	*	 interrupt counters
	*/
	seq_printf( f, "\n	 Interrupt counters \n");
	seq_printf( f, "\t dsp_t_cpu = %u\n" , ASTATS.dsp_t_cpu);
	seq_printf( f, "\t INT_2CPU_HIQ.cnt_ins  = %u INT_2CPU_HIQ.cnt_del =%u \n"	, 
			   INT_2CPU_HIQ.cnt_ins  ,	  INT_2CPU_HIQ.cnt_del);
	seq_printf( f, "\t INT_2CPU_LOWQ.cnt_ins = %u INT_2CPU_LOWQ.cnt_del =%u \n" , 
			   INT_2CPU_LOWQ.cnt_ins ,	  INT_2CPU_LOWQ.cnt_del);

	seq_printf( f, "\t cpu_t_dsp = %u\n" , ASTATS.cpu_t_dsp);
	seq_printf( f, "\t INT_2DSP_HIQ.cnt_ins  = %u INT_2DSP_HIQ.cnt_del =%u \n"	, 
			   INT_2DSP_HIQ.cnt_ins  ,	  INT_2DSP_HIQ.cnt_del);
	seq_printf( f, "\t INT_2DSP_LOWQ.cnt_ins = %u INT_2DSP_LOWQ.cnt_del =%u \n" , 
			   INT_2DSP_LOWQ.cnt_ins ,	  INT_2DSP_LOWQ.cnt_del);


	/*
	*	 shm notify counters
	*/
#ifdef CONFIG_RTL8686_SHM_NOTIFY
	seq_printf( f, "\n	 shm notify counters \n");
	seq_printf( f, "\t shm_notify_cpu = %u shm_notify_dsp = %u\n" , ASTATS.shm_notify_cpu , ASTATS.shm_notify_dsp);
#endif


	/*
	*	 error case
	*/
	seq_printf( f, "\n	 error counters \n");
	seq_printf( f, "\t aipc_data_error = %u aipc_ctrl_error = %u\n"  , ASTATS.aipc_data_error,ASTATS.aipc_ctrl_error);

	/*
	*	exception case
	*/
	seq_printf( f, "\n	 exception counters \n");
	seq_printf( f, "\t aipc_ctrl_2cpu_exception_send = %u\n" ,	  ASTATS.aipc_ctrl_2cpu_exception_send);

	return n;
}

#else
static int
aipc_dbg_counter_dump(char *buf)
{
	int n = 0;

	n += sprintf(buf , "Statistics:\n");

    /*    
    *    data plane
    */
	n += sprintf(buf+n , "   Data plane \n");
    //    CPU->DSP
	n += sprintf(buf+n , "\t (CPU) data CPU->DSP: \n");
	n += sprintf(buf+n , "\t aipc_data_2dsp_alloc  = %u    BC_2DSP.cnt_del = %u\n"  , ASTATS.aipc_data_2dsp_alloc , BC_2DSP.cnt_del);
	n += sprintf(buf+n , "\t aipc_data_2dsp_send   = %u    MB_2DSP.cnt_ins = %u\n"  , ASTATS.aipc_data_2dsp_send  , MB_2DSP.cnt_ins);
	n += sprintf(buf+n , "\t (DSP) data CPU->DSP: \n");
	n += sprintf(buf+n , "\t aipc_data_2dsp_recv   = %u    MB_2DSP.cnt_del = %u\n"  , ASTATS.aipc_data_2dsp_recv  , MB_2DSP.cnt_del);
	n += sprintf(buf+n , "\t aipc_data_2dsp_ret    = %u    BC_2DSP.cnt_ins = %u\n"  , ASTATS.aipc_data_2dsp_ret   , BC_2DSP.cnt_ins);

    //    CPU<-DSP
	n += sprintf(buf+n , "\n\t (CPU) data CPU<-DSP: \n");
	n += sprintf(buf+n , "\t aipc_data_2cpu_recv   = %u    MB_2CPU.cnt_del = %u\n"  , ASTATS.aipc_data_2cpu_recv  , MB_2CPU.cnt_del);
	n += sprintf(buf+n , "\t aipc_data_2cpu_ret    = %u    BC_2CPU.cnt_ins = %u\n"  , ASTATS.aipc_data_2cpu_ret   , BC_2CPU.cnt_ins);
	n += sprintf(buf+n , "\t (DSP) data CPU<-DSP: \n");
	n += sprintf(buf+n , "\t aipc_data_2cpu_alloc  = %u    BC_2CPU.cnt_del = %u\n"  , ASTATS.aipc_data_2cpu_alloc , BC_2CPU.cnt_del);
	n += sprintf(buf+n , "\t aipc_data_2cpu_send   = %u    MB_2CPU.cnt_ins = %u\n"  , ASTATS.aipc_data_2cpu_send  , MB_2CPU.cnt_ins);

    /*    
    *    control plane plane
    */
	n += sprintf(buf+n , "\n   Control plane \n");    
    //	Control
	n += sprintf(buf+n , "\t (CPU) control CPU->DSP: \n");
	n += sprintf(buf+n , "\t aipc_ctrl_2dsp_nofbk_alloc  = %u  CMD_2DSP.cnt_del = %u\n"  , 
		ASTATS.aipc_ctrl_2dsp_nofbk_alloc ,      CMD_2DSP.cnt_del);
	n += sprintf(buf+n , "\t aipc_ctrl_2dsp_fbk_alloc    = %u  CMD_LOCAL_2DSP.cnt_del = %u\n"  ,
		ASTATS.aipc_ctrl_2dsp_fbk_alloc   ,      CMD_LOCAL_2DSP.cnt_del);

	#ifdef STATS_RETRY 
	n += sprintf(buf+n , "\t aipc_ctrl_2dsp_nofbk_alloc_retry = %u\n" ,
		ASTATS.aipc_ctrl_2dsp_nofbk_alloc_retry);
	n += sprintf(buf+n , "\t aipc_ctrl_2dsp_fbk_alloc_retry = %u\n" ,
		ASTATS.aipc_ctrl_2dsp_fbk_alloc_retry);
	#endif	
	
	n += sprintf(buf+n , "\t aipc_ctrl_2dsp_send         = %u  CMD_QUEUE_2DSP.cnt_ins = %u\n"  , 
		ASTATS.aipc_ctrl_2dsp_send ,             CMD_QUEUE_2DSP.cnt_ins);
   	n += sprintf(buf+n , "\t aipc_ctrl_2dsp_fbk_ret      = %u  CMD_LOCAL_2DSP.cnt_ins = %u\n"  , 
		ASTATS.aipc_ctrl_2dsp_fbk_ret ,          CMD_LOCAL_2DSP.cnt_ins);
	
    n += sprintf(buf+n , "\t (DSP) control CPU->DSP: \n");
    n += sprintf(buf+n , "\t aipc_ctrl_2dsp_recv         = %u  CMD_QUEUE_2DSP.cnt_del = %u\n"  , 
		ASTATS.aipc_ctrl_2dsp_recv ,             CMD_QUEUE_2DSP.cnt_del);
    n += sprintf(buf+n , "\t aipc_ctrl_2dsp_nofbk_ret    = %u  CMD_2DSP.cnt_ins = %u\n" ,    
		ASTATS.aipc_ctrl_2dsp_nofbk_ret ,        CMD_2DSP.cnt_ins );
    n += sprintf(buf+n , "\t aipc_ctrl_2dsp_fbk_fin      = %u  CBUF_2DSP.done_cnt = %u\n"  , 
		ASTATS.aipc_ctrl_2dsp_fbk_fin ,          CBUF_2DSP.done_cnt);

	//	Event
    n += sprintf(buf+n , "\n\t (CPU) event CPU<-DSP: \n");    
    n += sprintf(buf+n , "\t aipc_ctrl_2cpu_recv   = %u\n"  ,    ASTATS.aipc_ctrl_2cpu_recv);
    n += sprintf(buf+n , "\t aipc_ctrl_2cpu_ret    = %u  CBUF_EQ_2CPU.cnt_del = %u\n"  , 
		ASTATS.aipc_ctrl_2cpu_ret ,              CBUF_EQ_2CPU.cnt_del);
    
    n += sprintf(buf+n , "\t (DSP) event CPU<-DSP: \n");
    n += sprintf(buf+n , "\t aipc_ctrl_2cpu_alloc  = %u\n"  ,    ASTATS.aipc_ctrl_2cpu_alloc);

	#ifdef STATS_RETRY 
    n += sprintf(buf+n , "\t aipc_ctrl_2cpu_alloc_retry  = %u\n"  , ASTATS.aipc_ctrl_2cpu_alloc_retry);
	#endif
	
	n += sprintf(buf+n , "\t aipc_ctrl_2cpu_send   = %u  CBUF_EQ_2CPU.cnt_ins = %u\n"  , 
		ASTATS.aipc_ctrl_2cpu_send ,       CBUF_EQ_2CPU.cnt_ins);

    /*
    *    interrupt counters
    */
	n += sprintf(buf+n , "\n   Interrupt counters \n");
	n += sprintf(buf+n , "\t dsp_t_cpu = %u\n" , ASTATS.dsp_t_cpu);
	n += sprintf(buf+n , "\t INT_2CPU_HIQ.cnt_ins  = %u INT_2CPU_HIQ.cnt_del =%u \n"  , 
		       INT_2CPU_HIQ.cnt_ins  ,    INT_2CPU_HIQ.cnt_del);
	n += sprintf(buf+n , "\t INT_2CPU_LOWQ.cnt_ins = %u INT_2CPU_LOWQ.cnt_del =%u \n" , 
		       INT_2CPU_LOWQ.cnt_ins ,    INT_2CPU_LOWQ.cnt_del);

	n += sprintf(buf+n , "\t cpu_t_dsp = %u\n" , ASTATS.cpu_t_dsp);
	n += sprintf(buf+n , "\t INT_2DSP_HIQ.cnt_ins  = %u INT_2DSP_HIQ.cnt_del =%u \n"  , 
		       INT_2DSP_HIQ.cnt_ins  ,    INT_2DSP_HIQ.cnt_del);
	n += sprintf(buf+n , "\t INT_2DSP_LOWQ.cnt_ins = %u INT_2DSP_LOWQ.cnt_del =%u \n" , 
		       INT_2DSP_LOWQ.cnt_ins ,    INT_2DSP_LOWQ.cnt_del);


    /*
    *    shm notify counters
    */
#ifdef CONFIG_RTL8686_SHM_NOTIFY
	n += sprintf(buf+n , "\n   shm notify counters \n");
	n += sprintf(buf+n , "\t shm_notify_cpu = %u shm_notify_dsp = %u\n" , ASTATS.shm_notify_cpu , ASTATS.shm_notify_dsp);
#endif


	/*
	*	 error case
	*/
	n += sprintf(buf+n , "\n   error counters \n");
	n += sprintf(buf+n , "\t aipc_data_error = %u aipc_ctrl_error = %u\n"  , ASTATS.aipc_data_error,ASTATS.aipc_ctrl_error);

	/*
	*	exception case
	*/
	n += sprintf(buf+n , "\n   exception counters \n");
	n += sprintf(buf+n , "\t aipc_ctrl_2cpu_exception_send = %u\n" ,    ASTATS.aipc_ctrl_2cpu_exception_send);

	return n;
}
#endif

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int aipc_thread_switch_dump(struct seq_file *f)
{
	int n = 0;
	
	seq_printf( f, "\nControl Switch:\n");

//control plane
	//CPU->DSP. RX direction
	seq_printf( f, "\tctrl_2dsp_send = 0x%x\n" , ATHREAD.ctrl_2dsp_send);
	seq_printf( f, "\tctrl_2dsp_poll = 0x%x\n" , ATHREAD.ctrl_2dsp_poll);
		
	//CPU<-DSP. TX direction
	seq_printf( f, "\tctrl_2cpu_send = 0x%x\n" , ATHREAD.ctrl_2cpu_send);
	seq_printf( f, "\tctrl_2cpu_poll = 0x%x\n" , ATHREAD.ctrl_2cpu_poll);
			
//data plane
	//CPU->DSP. RX direction
	seq_printf( f, "\tmbox_2dsp_send = 0x%x\n" , ATHREAD.mbox_2dsp_send);
	seq_printf( f, "\tmbox_2dsp_poll = 0x%x\n" , ATHREAD.mbox_2dsp_poll);
			
	//CPU<-DSP. TX direction
	seq_printf( f, "\tmbox_2cpu_send = 0x%x\n" , ATHREAD.mbox_2cpu_send);
	seq_printf( f, "\tmbox_2cpu_recv = 0x%x\n" , ATHREAD.mbox_2cpu_recv);

	return n;
}
#else
static int
aipc_thread_switch_dump(char *buf)
{
	int n = 0;
	
	n += sprintf(buf , "\nControl Switch:\n");

//control plane
	//CPU->DSP. RX direction
	n += sprintf(buf+n , "\tctrl_2dsp_send = 0x%x\n" , ATHREAD.ctrl_2dsp_send);
	n += sprintf(buf+n , "\tctrl_2dsp_poll = 0x%x\n" , ATHREAD.ctrl_2dsp_poll);
		
	//CPU<-DSP. TX direction
	n += sprintf(buf+n , "\tctrl_2cpu_send = 0x%x\n" , ATHREAD.ctrl_2cpu_send);
	n += sprintf(buf+n , "\tctrl_2cpu_poll = 0x%x\n" , ATHREAD.ctrl_2cpu_poll);
			
//data plane
	//CPU->DSP. RX direction
	n += sprintf(buf+n , "\tmbox_2dsp_send = 0x%x\n" , ATHREAD.mbox_2dsp_send);
	n += sprintf(buf+n , "\tmbox_2dsp_poll = 0x%x\n" , ATHREAD.mbox_2dsp_poll);
			
	//CPU<-DSP. TX direction
	n += sprintf(buf+n , "\tmbox_2cpu_send = 0x%x\n" , ATHREAD.mbox_2cpu_send);
	n += sprintf(buf+n , "\tmbox_2cpu_recv = 0x%x\n" , ATHREAD.mbox_2cpu_recv);

	return n;
}
#endif

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int 
proc_status_ipc_counters_r (struct seq_file *f)

{
	int n = 0;

	{

		aipc_mbox_dump(f);

		aipc_intq_dump(f);

		aipc_ctrl_dump(f);

		aipc_dbg_counter_dump(f);
	}
	
	return n;
}

#else
static int 
proc_status_ipc_counters_r (char *buf, char **start, off_t off, int count, int *eof, void *data)

{
	int n = 0;

	switch(off){
	case 0:
		n += aipc_mbox_dump(buf);

		n += aipc_intq_dump(buf+n);

		n += aipc_ctrl_dump(buf+n);

		break;
		
	case 1:
		n += aipc_dbg_counter_dump(buf+n);

		break;
	}

	*start = (char *)1; 

	if (n==0)
	    *eof = 1;
	
	return n;
}
#endif
static int 
proc_status_thread_w (struct file *file, const char *buffer, unsigned long count, void *data)
{
	char tmpbuf[100] = {0};
	int flag = 0;

	if (buffer && !copy_from_user(tmpbuf, buffer, count)) {

		sscanf(tmpbuf, "%d", &flag);

		if (flag == 0){
			memset(&ATHREAD , 0 , sizeof(aipc_thread_t));
			
			printk("Clear thread counters\n");
		}
		else
			printk("wrong number\n");
	}

	return count;
}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int proc_status_thread_r(struct seq_file *f, void *v)
{
	int n = 0;

	aipc_thread_switch_dump(f);
	
	aipc_dbg_thread_dump(f);

	return n;
}
#else
static int 
proc_status_thread_r (char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int n = 0;

    if( off ) { /* In our case, we write out all data at once. */
        *eof = 1;
        return 0;
    }

	n += aipc_thread_switch_dump(buf);
	
	n += aipc_dbg_thread_dump(buf+n);

    *eof = 1;
	
	return n;
}
#endif

static int
proc_status_register_w(struct file *file, const char *buffer, unsigned long count, void *data)
{
	return count;
}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int proc_status_register_r(struct seq_file *f, void *v)
{
	int n = 0;

	seq_printf( f, "\nMemory Registers:\n");
	
	seq_printf( f, "\tC0DOR0  = 0x%x\n" , REG32(C0DOR0));
	seq_printf( f, "\tC0DMAR0 = 0x%x\n" , REG32(C0DMAR0));
	seq_printf( f, "\tC0DOR1  = 0x%x\n" , REG32(C0DOR1));
	seq_printf( f, "\tC0DMAR1 = 0x%x\n" , REG32(C0DMAR1));
	seq_printf( f, "\tC0DOR2  = 0x%x\n" , REG32(C0DOR2));
	seq_printf( f, "\tC0DMAR2 = 0x%x\n" , REG32(C0DMAR2));
	seq_printf( f, "\tC0RCR   = 0x%x\n" , REG32(C0RCR));
	seq_printf( f, "\tC0ILAR  = 0x%x\n" , REG32(C0ILAR));

	seq_printf( f, "\tC1DOR0  = 0x%x\n" , REG32(C1DOR0));
	seq_printf( f, "\tC1DMAR0 = 0x%x\n" , REG32(C1DMAR0));
	seq_printf( f, "\tC1DOR1  = 0x%x\n" , REG32(C1DOR1));
	seq_printf( f, "\tC1DMAR1 = 0x%x\n" , REG32(C1DMAR1));
	seq_printf( f, "\tC1DOR2  = 0x%x\n" , REG32(C1DOR2));
	seq_printf( f, "\tC1DMAR2 = 0x%x\n" , REG32(C1DMAR2));
	seq_printf( f, "\tC1RCR   = 0x%x\n" , REG32(C1RCR));
	seq_printf( f, "\tC1ILAR  = 0x%x\n" , REG32(C1ILAR));

	return n;
}
#else
static int
proc_status_register_r(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int n = 0;

    if( off ) { /* In our case, we write out all data at once. */
        *eof = 1;
        return 0;
    }

	n += sprintf(buf , "\nMemory Registers:\n");
	
	n += sprintf(buf+n , "\tC0DOR0  = 0x%x\n" , REG32(C0DOR0));
	n += sprintf(buf+n , "\tC0DMAR0 = 0x%x\n" , REG32(C0DMAR0));
	n += sprintf(buf+n , "\tC0DOR1  = 0x%x\n" , REG32(C0DOR1));
	n += sprintf(buf+n , "\tC0DMAR1 = 0x%x\n" , REG32(C0DMAR1));
	n += sprintf(buf+n , "\tC0DOR2  = 0x%x\n" , REG32(C0DOR2));
	n += sprintf(buf+n , "\tC0DMAR2 = 0x%x\n" , REG32(C0DMAR2));
	n += sprintf(buf+n , "\tC0RCR   = 0x%x\n" , REG32(C0RCR));
	n += sprintf(buf+n , "\tC0ILAR  = 0x%x\n" , REG32(C0ILAR));

	n += sprintf(buf+n , "\tC1DOR0  = 0x%x\n" , REG32(C1DOR0));
	n += sprintf(buf+n , "\tC1DMAR0 = 0x%x\n" , REG32(C1DMAR0));
	n += sprintf(buf+n , "\tC1DOR1  = 0x%x\n" , REG32(C1DOR1));
	n += sprintf(buf+n , "\tC1DMAR1 = 0x%x\n" , REG32(C1DMAR1));
	n += sprintf(buf+n , "\tC1DOR2  = 0x%x\n" , REG32(C1DOR2));
	n += sprintf(buf+n , "\tC1DMAR2 = 0x%x\n" , REG32(C1DMAR2));
	n += sprintf(buf+n , "\tC1RCR   = 0x%x\n" , REG32(C1RCR));
	n += sprintf(buf+n , "\tC1ILAR  = 0x%x\n" , REG32(C1ILAR));

	*eof = 1;

	return n;
}
#endif

static int
proc_status_shm_w(struct file *file, const char *buffer, unsigned long count, void *data)
{
	return count;
}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int proc_status_shm_r(struct seq_file *f, void *v)
{
	int n = 0;
#ifdef CONFIG_RTL8686_SHM_NOTIFY
	extern const unsigned int *aipc_shm_notify_cpu;
	extern const unsigned int *aipc_shm_notify_dsp;
#endif

	seq_printf( f, "\nShared Memory Status:\n");
	
	seq_printf( f, "\nShared Memory Data Size:\n");
	seq_printf( f, "\t aipc_dram_t size  = 0x%x\n" , sizeof(aipc_dram_t));
	seq_printf( f, "\t aipc_sram_t size  = 0x%x\n" , sizeof(aipc_sram_t));

#ifdef CONFIG_RTL8686_SHM_NOTIFY
	seq_printf( f, "\nShared Memory Notification:\n");
	seq_printf( f, "\t aipc_shm_notify_cpu  = 0x%p\n" , aipc_shm_notify_cpu);
	seq_printf( f, "\t aipc_shm_notify_dsp  = 0x%p\n" , aipc_shm_notify_dsp);
#endif

	return n;
}
#else
static int
proc_status_shm_r(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int n = 0;
#ifdef CONFIG_RTL8686_SHM_NOTIFY
	extern const unsigned int *aipc_shm_notify_cpu;
	extern const unsigned int *aipc_shm_notify_dsp;
#endif

    if( off ) { /* In our case, we write out all data at once. */
        *eof = 1;
        return 0;
    }

	n += sprintf(buf , "\nShared Memory Status:\n");
	
	n += sprintf(buf+n , "\nShared Memory Data Size:\n");
	n += sprintf(buf+n , "\t aipc_dram_t size  = 0x%x\n" , sizeof(aipc_dram_t));
	n += sprintf(buf+n , "\t aipc_sram_t size  = 0x%x\n" , sizeof(aipc_sram_t));

#ifdef CONFIG_RTL8686_SHM_NOTIFY
	n += sprintf(buf+n , "\nShared Memory Notification:\n");
	n += sprintf(buf+n , "\t aipc_shm_notify_cpu  = 0x%p\n" , aipc_shm_notify_cpu);
	n += sprintf(buf+n , "\t aipc_shm_notify_dsp  = 0x%p\n" , aipc_shm_notify_dsp);
#endif

	*eof = 1;

	return n;
}
#endif

#ifdef CONFIG_RTL8686_IPC_RECORD_DSP_LOG
static int
proc_dsp_log_enable_w (struct file *file, const char *buffer, unsigned long count, void *data)
{
	char tmpbuf[100];
	int flag = 0;

	if (buffer && !copy_from_user(tmpbuf, buffer, count)) {

		sscanf(tmpbuf, "%d", &flag);

		if (flag == RECORD_DSP_LOG_DISABLE){
			*rec_dsp_log_enable = RECORD_DSP_LOG_DISABLE;
			printk("Disable record dsp log\n");
		}
		else if (flag == RECORD_DSP_LOG_ENABLE_SAVE_HISTORY){
			*rec_dsp_log_enable = RECORD_DSP_LOG_ENABLE_SAVE_HISTORY;
			printk("Enable record dsp log and save history\n");			
		}
		else if (flag == RECORD_DSP_LOG_ENABLE_OVERWRITE_HISTORY){
			*rec_dsp_log_enable = RECORD_DSP_LOG_ENABLE_OVERWRITE_HISTORY;
			printk("Enable record dsp log and overwrite history\n");			
		}
		else
			printk("wrong number\n");
	}

	return count;
}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int proc_dsp_log_enable_r(struct seq_file *f, void *v)
{
	int n = 0;

	seq_printf( f, "record dsp log = %d\n" , *rec_dsp_log_enable);	
	
	return n;
}
#else
static int
proc_dsp_log_enable_r (char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int n = 0;

    if( off ) { /* In our case, we write out all data at once. */
        *eof = 1;
        return 0;
    }

	n += sprintf(buf , "record dsp log = %d\n" , *rec_dsp_log_enable);	
	
    *eof = 1;

	return n;
}
#endif

static int
proc_dsp_log_clear_w (struct file *file, const char *buffer, unsigned long count, void *data)
{
	char tmpbuf[100];
	int flag = 0;

	if (buffer && !copy_from_user(tmpbuf, buffer, count)) {

		sscanf(tmpbuf, "%d", &flag);

		if (flag == 0){
			*rec_dsp_log_del   = *rec_dsp_log_ins;
			*rec_dsp_log_touch = 0;
			
			printk("Clear record dsp log\n");
		}
		else
			printk("wrong number\n");
	}
	
	return count;
}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int proc_dsp_log_clear_r(struct seq_file *f, void *v)
{
	int n = 0;
	return n;
}
#else
static int
proc_dsp_log_clear_r (char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int n = 0;

    *eof = 1;

	return n;
}
#endif

static int
proc_dsp_log_contents_w (struct file *file, const char *buffer, unsigned long count, void *data)
{
	return count;
}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)

static int proc_dsp_log_contents_r(struct seq_file *f, void *v)
{
	int n = 0;
	unsigned int dsp_log_del = 0;
	int remain = 0;

	dsp_log_del = *rec_dsp_log_del;
	remain = aipc_record_dsp_log_contents_use();

	while( remain > 0 ){
		seq_printf(f, "%c", rec_dsp_log_contents[ dsp_log_del ]);
		dsp_log_del = (dsp_log_del+1) % RECORD_DSP_LOG_SIZE;
		remain--;
	}
	
	//printk("[%s][%d] remain=%d\n" ,__FUNCTION__, __LINE__ , remain);

	return n;
}
#else
static int
proc_dsp_log_contents_r (char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int n = 0;
	static unsigned int dsp_log_del = 0;
	static int remain = 0;

	if (*rec_dsp_log_enable == RECORD_DSP_LOG_DISABLE) {
        *eof = 1;
        n += sprintf( buf , "Record DSP log is disabled\n" );
        return n;
    }

	if (aipc_record_dsp_log_empty()) {
        *eof = 1;
        n += sprintf( buf , "Record DSP log is empty\n" );
        return n;
    }
    
    if (off==0){
    	dsp_log_del = *rec_dsp_log_del;
		remain = aipc_record_dsp_log_contents_use();
    }

	while( remain > 0 ){
		buf[n] = rec_dsp_log_contents[ dsp_log_del ];
		n++;
		dsp_log_del = (dsp_log_del+1) % RECORD_DSP_LOG_SIZE;
		remain--;

		if (n >= count)
			break;
	}

	*start = (char *)1;

	if (n==0)
	    *eof = 1;	

	//printk("count = %d remain = %u n = %d\n" , count , remain , n);

	return n;
}
#endif

static int
proc_dsp_log_index_w (struct file *file, const char *buffer, unsigned long count, void *data)
{
	return count;
}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int proc_dsp_log_index_r(struct seq_file *f, void *v)
{
	int n = 0;
	int full  = 0 ;
	int empty = 0;
	unsigned int use  = 0;
	unsigned int enable  = 0;	

	use    = aipc_record_dsp_log_contents_use();
	full   = aipc_record_dsp_log_full();
	empty  = aipc_record_dsp_log_empty();
	enable = *rec_dsp_log_enable;
	
	seq_printf( f, "\nIndex:\n");
	
	seq_printf( f, "\t enable      = %u "       , enable);
	if (enable == RECORD_DSP_LOG_DISABLE)
		seq_printf( f, "   %s\n"      , "disable");
	else if (enable == RECORD_DSP_LOG_ENABLE_SAVE_HISTORY)
		seq_printf( f, "   %s\n"      , "save history");
	else if (enable == RECORD_DSP_LOG_ENABLE_OVERWRITE_HISTORY)
		seq_printf( f, "   %s\n"      , "overwrite history");
	else 
		seq_printf( f, "   %s\n"      , "others");
	
	seq_printf( f, "\t touch       = %u\n"       , *rec_dsp_log_touch);
	seq_printf( f, "\t ins         = %u\n"       , *rec_dsp_log_ins);
	seq_printf( f, "\t del         = %u\n"       , *rec_dsp_log_del);
	seq_printf( f, "\t use         = %u\n"       ,  use);
	
	if (full) {
		seq_printf( f, "\t full        = 1\n");
	}
	else if (empty) {
		seq_printf( f, "\t empty       = 1\n");
	}
	else if (use) {
		seq_printf( f, "\t middle      = 1\n");
	}
	else {
		seq_printf( f, "\t unexpected queue status \n");
	}
	
	seq_printf( f, "\t conts addr  = %p\n"       ,  rec_dsp_log_contents);
	seq_printf( f, "\t log size    = %u\n"       ,  RECORD_DSP_LOG_SIZE);

	return n;
}
#else
static int
proc_dsp_log_index_r (char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int n = 0;
	int full  = 0 ;
	int empty = 0;
	unsigned int use  = 0;
	unsigned int enable  = 0;	

    if( off ) { /* In our case, we write out all data at once. */
        *eof = 1;
        return 0;
    }
	
	use    = aipc_record_dsp_log_contents_use();
	full   = aipc_record_dsp_log_full();
	empty  = aipc_record_dsp_log_empty();
	enable = *rec_dsp_log_enable;
	
	n += sprintf(buf , "\nIndex:\n");
	
	n += sprintf(buf+n , "\t enable      = %u "       , enable);
	if (enable == RECORD_DSP_LOG_DISABLE)
		n += sprintf(buf+n , "   %s\n"      , "disable");
	else if (enable == RECORD_DSP_LOG_ENABLE_SAVE_HISTORY)
		n += sprintf(buf+n , "   %s\n"      , "save history");
	else if (enable == RECORD_DSP_LOG_ENABLE_OVERWRITE_HISTORY)
		n += sprintf(buf+n , "   %s\n"      , "overwrite history");
	else 
		n += sprintf(buf+n , "   %s\n"      , "others");
	
	n += sprintf(buf+n , "\t touch       = %u\n"       , *rec_dsp_log_touch);
	n += sprintf(buf+n , "\t ins         = %u\n"       , *rec_dsp_log_ins);
	n += sprintf(buf+n , "\t del         = %u\n"       , *rec_dsp_log_del);
	n += sprintf(buf+n , "\t use         = %u\n"       ,  use);
	
	if (full) {
		n += sprintf(buf+n , "\t full        = 1\n");
	}
	else if (empty) {
		n += sprintf(buf+n , "\t empty       = 1\n");
	}
	else if (use) {
		n += sprintf(buf+n , "\t middle      = 1\n");
	}
	else {
		n += sprintf(buf+n , "\t unexpected queue status \n");
	}
	
	n += sprintf(buf+n , "\t conts addr  = %p\n"       ,  rec_dsp_log_contents);
	n += sprintf(buf+n , "\t log size    = %u\n"       ,  RECORD_DSP_LOG_SIZE);

	*eof = 1;
	return n;
}
#endif

#endif

#ifdef CONFIG_RTL8686_IPC_DSP_CONSOLE
extern          int aipc_dsp_console_read_buf_full(void);
extern          int aipc_dsp_console_read_buf_empty(void);
extern unsigned int aipc_dsp_console_read_buf_use(void);

extern          int aipc_dsp_console_write_buf_full(void);
extern          int aipc_dsp_console_write_buf_empty(void);
extern unsigned int aipc_dsp_console_write_buf_use(void);  


static int
proc_dsp_console_enable_w (struct file *file, const char *buffer, unsigned long count, void *data)
{
	char tmpbuf[100];
	int flag = 0;

	if (buffer && !copy_from_user(tmpbuf, buffer, count)) {

		sscanf(tmpbuf, "%d", &flag);

		if (flag == 0){
			ADSP_CONSOLE.enable = 0;
			printk("disable dsp console\n");
		}
		else if (flag == 1){
			ADSP_CONSOLE.enable = 1;
			printk("ensable dsp console\n");			
		}
		else
			printk("wrong number\n");
	}

	return count;
}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int proc_dsp_console_enable_r(struct seq_file *f, void *v)
{
	int n = 0;

	seq_printf( f, "dsp console enable = %d\n" , ADSP_CONSOLE.enable);	

	return n;
}
#else
static int
proc_dsp_console_enable_r (char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int n = 0;

    if( off ) { /* In our case, we write out all data at once. */
        *eof = 1;
        return 0;
    }

	n += sprintf(buf , "dsp console enable = %d\n" , ADSP_CONSOLE.enable);	
	
    *eof = 1;

	return n;
}
#endif

/*
*	read queue
*/
static int
proc_dsp_console_read_queue_enable_w (struct file *file, const char *buffer, unsigned long count, void *data)
{
	char tmpbuf[100];
	int flag = 0;

	if (buffer && !copy_from_user(tmpbuf, buffer, count)) {

		sscanf(tmpbuf, "%d", &flag);

		if (flag == 0){
			ADSP_CONSOLE_RD.enable = 0;
			printk("disable dsp read queue\n");
		}
		else if (flag == 1){
			ADSP_CONSOLE_RD.enable = 1;
			printk("ensable dsp read queue\n");			
		}
		else
			printk("wrong number\n");
	}

	return count;
}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int proc_dsp_console_read_queue_enable_r(struct seq_file *f, void *v)
{
	int n = 0;

	seq_printf( f, "dsp read queue enable = %d\n" , ADSP_CONSOLE_RD.enable);	
	
	return n;
}
#else
static int
proc_dsp_console_read_queue_enable_r (char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int n = 0;

    if( off ) { /* In our case, we write out all data at once. */
        *eof = 1;
        return 0;
    }

	n += sprintf(buf , "dsp read queue enable = %d\n" , ADSP_CONSOLE_RD.enable);	
	
    *eof = 1;

	return n;
}
#endif

static int
proc_dsp_console_read_queue_index_w (struct file *file, const char *buffer, unsigned long count, void *data)
{
	return count;
}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int proc_dsp_console_read_queue_index_r(struct seq_file *f, void *v)
{
	int n = 0;
	unsigned int use  = 0;
	int full  = 0 ;
	int empty = 0;
	
	use   = aipc_dsp_console_read_buf_use();
	full  = aipc_dsp_console_read_buf_full();
	empty = aipc_dsp_console_read_buf_empty();
	
	seq_printf( f, "\nIndex:\n");
	
	seq_printf( f, "\t enable      = %u\n"       ,  ADSP_CONSOLE_RD.enable);
	seq_printf( f, "\t eq_try      = %u\n"       ,  ADSP_CONSOLE_RD.eq_try);
	seq_printf( f, "\t eq_ok       = %u\n"       ,  ADSP_CONSOLE_RD.eq_ok);
	seq_printf( f, "\t dq_try      = %u\n"       ,  ADSP_CONSOLE_RD.dq_try);
	seq_printf( f, "\t dq_ok       = %u\n"       ,  ADSP_CONSOLE_RD.dq_ok);
	seq_printf( f, "\t ins         = %u\n"       ,  ADSP_CONSOLE_RD.ins);
	seq_printf( f, "\t del         = %u\n"       ,  ADSP_CONSOLE_RD.del);
	seq_printf( f, "\t use         = %u\n"       ,  use);
	
	if (full) {
		seq_printf( f, "\t full        = 1\n");
	}
	else if (empty) {
		seq_printf( f, "\t empty       = 1\n");
	}
	else if (use) {
		seq_printf( f, "\t middle      = 1\n");
	}
	else {
		seq_printf( f, "\t unexpected queue status \n");
	}
	
	seq_printf( f, "\t buf addr   = %p\n"       ,  &ADSP_CONSOLE_RD.read_buf);
	seq_printf( f, "\t queue size = %u\n"       ,  DSP_CONSOLE_READ_BUF_SIZE);

	return n;
}
#else
static int
proc_dsp_console_read_queue_index_r (char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int n = 0;
	unsigned int use  = 0;
	int full  = 0 ;
	int empty = 0;

    if( off ) { /* In our case, we write out all data at once. */
        *eof = 1;
        return 0;
    }
	
	use   = aipc_dsp_console_read_buf_use();
	full  = aipc_dsp_console_read_buf_full();
	empty = aipc_dsp_console_read_buf_empty();
	
	n += sprintf(buf , "\nIndex:\n");
	
	n += sprintf(buf+n , "\t enable      = %u\n"       ,  ADSP_CONSOLE_RD.enable);
	n += sprintf(buf+n , "\t eq_try      = %u\n"       ,  ADSP_CONSOLE_RD.eq_try);
	n += sprintf(buf+n , "\t eq_ok       = %u\n"       ,  ADSP_CONSOLE_RD.eq_ok);
	n += sprintf(buf+n , "\t dq_try      = %u\n"       ,  ADSP_CONSOLE_RD.dq_try);
	n += sprintf(buf+n , "\t dq_ok       = %u\n"       ,  ADSP_CONSOLE_RD.dq_ok);
	n += sprintf(buf+n , "\t ins         = %u\n"       ,  ADSP_CONSOLE_RD.ins);
	n += sprintf(buf+n , "\t del         = %u\n"       ,  ADSP_CONSOLE_RD.del);
	n += sprintf(buf+n , "\t use         = %u\n"       ,  use);
	
	if (full) {
		n += sprintf(buf+n , "\t full        = 1\n");
	}
	else if (empty) {
		n += sprintf(buf+n , "\t empty       = 1\n");
	}
	else if (use) {
		n += sprintf(buf+n , "\t middle      = 1\n");
	}
	else {
		n += sprintf(buf+n , "\t unexpected queue status \n");
	}
	
	n += sprintf(buf+n , "\t buf addr   = %p\n"       ,  &ADSP_CONSOLE_RD.read_buf);
	n += sprintf(buf+n , "\t queue size = %u\n"       ,  DSP_CONSOLE_READ_BUF_SIZE);

	*eof = 1;
	return n;
}
#endif

static int
proc_dsp_console_read_queue_clear_w (struct file *file, const char *buffer, unsigned long count, void *data)
{
	char tmpbuf[100];
	int flag = 0;

	if (buffer && !copy_from_user(tmpbuf, buffer, count)) {

		sscanf(tmpbuf, "%d", &flag);

		if (flag == 0){
			ADSP_CONSOLE_RD.del   = ADSP_CONSOLE_RD.ins;
			ADSP_CONSOLE_RD.eq_try  = 0;
			ADSP_CONSOLE_RD.eq_ok   = 0;
			ADSP_CONSOLE_RD.dq_try  = 0;
			ADSP_CONSOLE_RD.dq_ok   = 0;
			
			printk("clear dsp console read queue\n");
		}
		else
			printk("wrong number\n");
	}
	
	return count;
}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int proc_dsp_console_read_queue_clear_r(struct seq_file *f, void *v)
{
	int n = 0;

	return n;
}
#else
static int
proc_dsp_console_read_queue_clear_r (char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int n = 0;

    *eof = 1;

	return n;
}
#endif

/*
*	write queue
*/
static int
proc_dsp_console_write_queue_enable_w (struct file *file, const char *buffer, unsigned long count, void *data)
{
	char tmpbuf[100];
	int flag = 0;

	if (buffer && !copy_from_user(tmpbuf, buffer, count)) {

		sscanf(tmpbuf, "%d", &flag);

		if (flag == 0){
			ADSP_CONSOLE_WR.enable = 0;
			printk("disable dsp write queue\n");
		}
		else if (flag == 1){
			ADSP_CONSOLE_WR.enable = 1;
			printk("ensable dsp write queue\n");			
		}
		else
			printk("wrong number\n");
	}

	return count;
}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int proc_dsp_console_write_queue_enable_r(struct seq_file *f, void *v)
{
	int n = 0;

	seq_printf( f, "dsp write queue enable = %d\n" , ADSP_CONSOLE_WR.enable);	

	return n;
}
#else
static int
proc_dsp_console_write_queue_enable_r (char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int n = 0;

    if( off ) { /* In our case, we write out all data at once. */
        *eof = 1;
        return 0;
    }

	n += sprintf(buf , "dsp write queue enable = %d\n" , ADSP_CONSOLE_WR.enable);	
	
    *eof = 1;

	return n;
}
#endif

static int
proc_dsp_console_write_queue_index_w (struct file *file, const char *buffer, unsigned long count, void *data)
{
	return count;
}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int proc_dsp_console_write_queue_index_r(struct seq_file *f, void *v)
{
	int n = 0;
	unsigned int use  = 0;
	int full  = 0 ;
	int empty = 0;

	use   = aipc_dsp_console_write_buf_use();
	full  = aipc_dsp_console_write_buf_full();
	empty = aipc_dsp_console_write_buf_empty();
	
	seq_printf( f, "\nIndex:\n");
	
	seq_printf( f, "\t enable      = %u\n"       ,  ADSP_CONSOLE_WR.enable);
	seq_printf( f, "\t eq_try      = %u\n"       ,  ADSP_CONSOLE_WR.eq_try);
	seq_printf( f, "\t eq_ok       = %u\n"       ,  ADSP_CONSOLE_WR.eq_ok);
	seq_printf( f, "\t dq_try      = %u\n"       ,  ADSP_CONSOLE_WR.dq_try);
	seq_printf( f, "\t dq_ok       = %u\n"       ,  ADSP_CONSOLE_WR.dq_ok);
	seq_printf( f, "\t ins         = %u\n"       ,  ADSP_CONSOLE_WR.ins);
	seq_printf( f, "\t del         = %u\n"       ,  ADSP_CONSOLE_WR.del);
	seq_printf( f, "\t use         = %u\n"       ,  use);
	
	if (full) {
		seq_printf( f, "\t full        = 1\n");
	}
	else if (empty) {
		seq_printf( f, "\t empty       = 1\n");
	}
	else if (use) {
		seq_printf( f, "\t middle      = 1\n");
	}
	else {
		seq_printf( f, "\t unexpected queue status \n");
	}
	
	seq_printf( f, "\t buf addr   = %p\n"       ,  &ADSP_CONSOLE_WR.write_buf);
	seq_printf( f, "\t queue size = %u\n"       ,  DSP_CONSOLE_WRITE_BUF_SIZE);

	return n;	
}
#else
static int
proc_dsp_console_write_queue_index_r (char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int n = 0;
	unsigned int use  = 0;
	int full  = 0 ;
	int empty = 0;

    if( off ) { /* In our case, we write out all data at once. */
        *eof = 1;
        return 0;
    }
	
	use   = aipc_dsp_console_write_buf_use();
	full  = aipc_dsp_console_write_buf_full();
	empty = aipc_dsp_console_write_buf_empty();
	
	n += sprintf(buf , "\nIndex:\n");
	
	n += sprintf(buf+n , "\t enable      = %u\n"       ,  ADSP_CONSOLE_WR.enable);
	n += sprintf(buf+n , "\t eq_try      = %u\n"       ,  ADSP_CONSOLE_WR.eq_try);
	n += sprintf(buf+n , "\t eq_ok       = %u\n"       ,  ADSP_CONSOLE_WR.eq_ok);
	n += sprintf(buf+n , "\t dq_try      = %u\n"       ,  ADSP_CONSOLE_WR.dq_try);
	n += sprintf(buf+n , "\t dq_ok       = %u\n"       ,  ADSP_CONSOLE_WR.dq_ok);
	n += sprintf(buf+n , "\t ins         = %u\n"       ,  ADSP_CONSOLE_WR.ins);
	n += sprintf(buf+n , "\t del         = %u\n"       ,  ADSP_CONSOLE_WR.del);
	n += sprintf(buf+n , "\t use         = %u\n"       ,  use);
	
	if (full) {
		n += sprintf(buf+n , "\t full        = 1\n");
	}
	else if (empty) {
		n += sprintf(buf+n , "\t empty       = 1\n");
	}
	else if (use) {
		n += sprintf(buf+n , "\t middle      = 1\n");
	}
	else {
		n += sprintf(buf+n , "\t unexpected queue status \n");
	}
	
	n += sprintf(buf+n , "\t buf addr   = %p\n"       ,  &ADSP_CONSOLE_WR.write_buf);
	n += sprintf(buf+n , "\t queue size = %u\n"       ,  DSP_CONSOLE_WRITE_BUF_SIZE);

	*eof = 1;
	return n;

}
#endif

static int
proc_dsp_console_write_queue_clear_w (struct file *file, const char *buffer, unsigned long count, void *data)
{
	char tmpbuf[100];
	int flag = 0;

	if (buffer && !copy_from_user(tmpbuf, buffer, count)) {

		sscanf(tmpbuf, "%d", &flag);

		if (flag == 0){
			ADSP_CONSOLE_WR.del   = ADSP_CONSOLE_WR.ins;
			ADSP_CONSOLE_WR.eq_try  = 0;
			ADSP_CONSOLE_WR.eq_ok   = 0;
			ADSP_CONSOLE_WR.dq_try  = 0;
			ADSP_CONSOLE_WR.dq_ok   = 0;
			
			printk("clear dsp console write queue\n");
		}
		else
			printk("wrong number\n");
	}
	
	return count;
}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int proc_dsp_console_write_queue_clear_r(struct seq_file *f, void *v)
{
	int n = 0;

	return n;
}
#else
static int
proc_dsp_console_write_queue_clear_r (char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int n = 0;

    *eof = 1;

	return n;
}
#endif
#endif

#if 0
static int 
proc_read_phymem_addr(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	return sprintf(buf, "%08lx\n", __pa(dst_addr));
}

static int 
proc_read_logmem_addr(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	return sprintf(buf, "%08lx\n", (ul32_t)dst_addr);
}

static struct file_operations aipc_dev_proc_ops = {
    .owner   = THIS_MODULE,
    .open    = NULL,
    .read    = NULL,
	.write	 = NULL,
    .llseek  = NULL,
    .release = NULL
};
#endif


#ifdef CONFIG_RTL8686_IPC_DSL_IPC
static int 
proc_dsl_ipc_counters_w (struct file *file, const char *buffer, unsigned long count, void *data)
{

	char tmpbuf[100] = {0};
	int flag = 0;

	if (buffer && !copy_from_user(tmpbuf, buffer, count)) {

		sscanf(tmpbuf, "%d", &flag);
		
		if (flag == 0){ // reset counters
			memset(&ASTATS ,  0 , sizeof(aipc_stats_t));
			
			printk("Clear status counters\n");
		}
		else
			SDEBUG("wrong number\n");
	}

	return count;
}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int proc_dsl_ipc_counters_r(struct seq_file *f, void *v)
{
	int n = 0;

	switch(off){
	case 0:
		aipc_dsl_proc_ctrl_dump ( f );
		aipc_dsl_proc_eoc_dump  ( f );
		break;

	case 1:
		aipc_dsl_proc_event_dump( f );
		break;
		
	default:
		break;
	}
	
	return n;
}
#else
static int 
proc_dsl_ipc_counters_r (char *buf, char **start, off_t off, int count, int *eof, void *data)

{
	int n = 0;

	switch(off){
	case 0:
		n += aipc_dsl_proc_ctrl_dump ( buf + n );
		n += aipc_dsl_proc_eoc_dump  ( buf + n );
		break;

	case 1:
		n += aipc_dsl_proc_event_dump( buf + n );
		break;
		
	default:
		break;
	}

	*start = (char *)1; 

	if (n==0)
	    *eof = 1;
	
	return n;
}
#endif
#endif

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int aipc_dbg_open(struct inode *inode, struct file *file){
	return single_open(file, proc_switch_dbg_print_r, NULL);
}

static ssize_t aipc_dbg_write(struct file *file, const char __user * userbuf, size_t count, loff_t * off) {
        return proc_switch_dbg_print_w(file, userbuf, count, NULL);
}

struct file_operations proc_aipc_dbg_ops = {
	.owner	= THIS_MODULE,
	.open	= aipc_dbg_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = aipc_dbg_write,};
	
static int aipc_counter_open(struct inode *inode, struct file *file){
	return single_open(file, proc_status_ipc_counters_r, NULL);
}

static ssize_t aipc_counter_write(struct file *file, const char __user * userbuf, size_t count, loff_t * off) {
        return proc_status_ipc_counters_w(file, userbuf, count, NULL);
}

struct file_operations proc_aipc_ipc_counter_ops = {
	.owner	= THIS_MODULE,
	.open	= aipc_counter_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = aipc_counter_write,};	
	
static int aipc_thread_open(struct inode *inode, struct file *file){
	return single_open(file, proc_status_thread_r, NULL);
}

static ssize_t aipc_thread_write(struct file *file, const char __user * userbuf, size_t count, loff_t * off) {
        return proc_status_thread_w(file, userbuf, count, NULL);
}

struct file_operations proc_aipc_thread_ops = {
	.owner	= THIS_MODULE,
	.open	= aipc_thread_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = aipc_thread_write,};
	
static int aipc_register_open(struct inode *inode, struct file *file){
	return single_open(file, proc_status_register_r, NULL);
}

static ssize_t aipc_register_write(struct file *file, const char __user * userbuf, size_t count, loff_t * off) {
        return proc_status_register_w(file, userbuf, count, NULL);
}

struct file_operations proc_aipc_register_ops = {
	.owner	= THIS_MODULE,
	.open	= aipc_register_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = aipc_register_write,};
	
static int aipc_shm_open(struct inode *inode, struct file *file){
	return single_open(file, proc_status_shm_r, NULL);
}

static ssize_t aipc_shm_write(struct file *file, const char __user * userbuf, size_t count, loff_t * off) {
        return proc_status_shm_w(file, userbuf, count, NULL);
}

struct file_operations proc_aipc_shm_ops = {
	.owner	= THIS_MODULE,
	.open	= aipc_shm_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = aipc_shm_write,};	
	
static int aipc_log_enable_open(struct inode *inode, struct file *file){
	return single_open(file, proc_dsp_log_enable_r, NULL);
}

static ssize_t aipc_log_enable_write(struct file *file, const char __user * userbuf, size_t count, loff_t * off) {
        return proc_dsp_log_enable_w(file, userbuf, count, NULL);
}

struct file_operations proc_aipc_log_enable_ops = {
	.owner	= THIS_MODULE,
	.open	= aipc_log_enable_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = aipc_log_enable_write,};		
	
	
static int aipc_log_clear_open(struct inode *inode, struct file *file){
	return single_open(file, proc_dsp_log_clear_r, NULL);
}

static ssize_t aipc_log_clear_write(struct file *file, const char __user * userbuf, size_t count, loff_t * off) {
        return proc_dsp_log_clear_w(file, userbuf, count, NULL);
}

struct file_operations proc_aipc_log_clear_ops = {
	.owner	= THIS_MODULE,
	.open	= aipc_log_clear_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = aipc_log_clear_write,};		

	
static int aipc_log_contents_open(struct inode *inode, struct file *file){
	return single_open(file, &proc_dsp_log_contents_r, NULL);
}

static ssize_t aipc_log_contents_write(struct file *file, const char __user * userbuf, size_t count, loff_t * off) {
        return proc_dsp_log_contents_w(file, userbuf, count, NULL);
}

struct file_operations proc_aipc_log_contents_ops = {
	.owner	= THIS_MODULE,
	.open	= aipc_log_contents_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = aipc_log_contents_write
};			
	
static int aipc_log_index_open(struct inode *inode, struct file *file){
	return single_open(file, proc_dsp_log_index_r, NULL);
}

static ssize_t aipc_log_index_write(struct file *file, const char __user * userbuf, size_t count, loff_t * off) {
        return proc_dsp_log_index_w(file, userbuf, count, NULL);
}

struct file_operations proc_aipc_log_index_ops = {
	.owner	= THIS_MODULE,
	.open	= aipc_log_index_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = aipc_log_index_write
};	
	
static int aipc_console_enable_open(struct inode *inode, struct file *file){
	return single_open(file, proc_dsp_console_enable_r, NULL);
}

static ssize_t aipc_console_enable_write(struct file *file, const char __user * userbuf, size_t count, loff_t * off) {
        return proc_dsp_console_enable_w(file, userbuf, count, NULL);
}

struct file_operations proc_aipc_console_enable_ops = {
	.owner	= THIS_MODULE,
	.open	= aipc_console_enable_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = aipc_console_enable_write,};	
	
static int aipc_console_read_enable_open(struct inode *inode, struct file *file){
	return single_open(file, proc_dsp_console_read_queue_enable_r, NULL);
}

static ssize_t aipc_console_read_enable_write(struct file *file, const char __user * userbuf, size_t count, loff_t * off) {
        return proc_dsp_console_read_queue_enable_w(file, userbuf, count, NULL);
}

struct file_operations proc_aipc_console_read_enable_ops = {
	.owner	= THIS_MODULE,
	.open	= aipc_console_read_enable_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = aipc_console_read_enable_write,};		
	
static int aipc_console_read_clear_open(struct inode *inode, struct file *file){
	return single_open(file, proc_dsp_console_read_queue_clear_r, NULL);
}

static ssize_t aipc_console_read_clear_write(struct file *file, const char __user * userbuf, size_t count, loff_t * off) {
        return proc_dsp_console_read_queue_clear_w(file, userbuf, count, NULL);
}

struct file_operations proc_aipc_console_read_clear_ops = {
	.owner	= THIS_MODULE,
	.open	= aipc_console_read_clear_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = aipc_console_read_clear_write,};		
	
static int aipc_console_read_index_open(struct inode *inode, struct file *file){
	return single_open(file, proc_dsp_console_read_queue_index_r, NULL);
}

static ssize_t aipc_console_read_index_write(struct file *file, const char __user * userbuf, size_t count, loff_t * off) {
        return proc_dsp_console_read_queue_index_w(file, userbuf, count, NULL);
}

struct file_operations proc_aipc_console_read_index_ops = {
	.owner	= THIS_MODULE,
	.open	= aipc_console_read_index_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = aipc_console_read_index_write,};			
	
	
static int aipc_console_write_enable_open(struct inode *inode, struct file *file){
	return single_open(file, proc_dsp_console_write_queue_enable_r, NULL);
}

static ssize_t aipc_console_write_enable_write(struct file *file, const char __user * userbuf, size_t count, loff_t * off) {
        return proc_dsp_console_write_queue_enable_w(file, userbuf, count, NULL);
}

struct file_operations proc_aipc_console_write_enable_ops = {
	.owner	= THIS_MODULE,
	.open	= aipc_console_write_enable_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = aipc_console_write_enable_write,};		
	
	
static int aipc_console_write_clear_open(struct inode *inode, struct file *file){
	return single_open(file, proc_dsp_console_write_queue_clear_r, NULL);
}

static ssize_t aipc_console_write_clear_write(struct file *file, const char __user * userbuf, size_t count, loff_t * off) {
	return proc_dsp_console_write_queue_clear_w(file, userbuf, count, NULL);
}

struct file_operations proc_aipc_console_write_clear_ops = {
	.owner	= THIS_MODULE,
	.open	= aipc_console_write_clear_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = aipc_console_write_clear_write,};		
	
static int aipc_console_write_index_open(struct inode *inode, struct file *file){
	return single_open(file, proc_dsp_console_write_queue_index_r, NULL);
}

static ssize_t aipc_console_write_index_write(struct file *file, const char __user * userbuf, size_t count, loff_t * off) {
        return proc_dsp_console_write_queue_index_w(file, userbuf, count, NULL);
}

struct file_operations proc_aipc_console_write_index_ops = {
	.owner	= THIS_MODULE,
	.open	= aipc_console_write_index_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = aipc_console_write_index_write,};	
	
#ifdef CONFIG_RTL8686_IPC_DSL_IPC
static int aipc_dsl_ipc_counter_open(struct inode *inode, struct file *file){
	return single_open(file, proc_dsl_ipc_counters_r, NULL);
}

static ssize_t aipc_dsl_ipc_counter_write(struct file *file, const char __user * userbuf, size_t count, loff_t * off) {
        return proc_dsl_ipc_counters_w(file, userbuf, count, NULL);
}

struct file_operations proc_aipc_dsl_ipc_counter_ops = {
	.owner	= THIS_MODULE,
	.open	= aipc_dsl_ipc_counter_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = aipc_dsl_ipc_counter_write,};		
#endif	
#endif

void 
aipc_dev_create_proc(void)
{
	struct proc_dir_entry *entry;
	
	/*
	*	create root directory
	*/
	proc_aipc_dev_dir = proc_mkdir(PROC_AIPC_DEV_DIR, NULL);
	if (proc_aipc_dev_dir == NULL) {
		printk("create proc root failed!\n");
		return;
	}

	/*
	*	create switch directory
	*/
	proc_aipc_dev_dir = proc_mkdir(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_SWITCH , NULL);
	if (proc_aipc_dev_dir == NULL) {
		printk("create proc aipc_dev/switch failed!\n");
		return;
	}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	if((entry = proc_create(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_SWITCH "/" PROC_AIPC_DEV_DBG_PRINT, 0644, NULL, &proc_aipc_dbg_ops)) == NULL) {
		printk("create proc aipc_dev/switch/dbg_print failed!\n");
		return;
	}
#else	
	if((entry = create_proc_entry(PROC_AIPC_DEV_DBG_PRINT, 0644, proc_aipc_dev_dir)) == NULL) {
		printk("create proc aipc_dev/switch/dbg_print failed!\n");
		return;
	}
	entry->write_proc = proc_switch_dbg_print_w;
	entry->read_proc  = proc_switch_dbg_print_r;
#endif

	/*
	*	create status directory
	*/
	proc_aipc_dev_dir = proc_mkdir(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_STATUS, NULL);
	if (proc_aipc_dev_dir == NULL) {
		printk("create proc aipc_dev/status failed!\n");
		return;
	}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	if((entry = proc_create(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_STATUS "/" PROC_AIPC_DEV_IPC_COUNTERS, 0644, NULL, &proc_aipc_ipc_counter_ops)) == NULL) {
		printk("create proc aipc_dev/status/ipc_counters failed!\n");
		return;
	}
#else
	if((entry = create_proc_entry(PROC_AIPC_DEV_IPC_COUNTERS, 0644, proc_aipc_dev_dir)) == NULL) {
		printk("create proc aipc_dev/status/ipc_counters failed!\n");
		return;
	}
	entry->write_proc = proc_status_ipc_counters_w;
	entry->read_proc  = proc_status_ipc_counters_r;
#endif	

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	if((entry = proc_create(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_STATUS "/" PROC_AIPC_DEV_THREAD, 0644, NULL, &proc_aipc_thread_ops)) == NULL) {
		printk("create proc aipc_dev/status/thread failed!\n");
		return;
	}
#else
	if((entry = create_proc_entry(PROC_AIPC_DEV_THREAD, 0644, proc_aipc_dev_dir)) == NULL) {
		printk("create proc aipc_dev/status/thread failed!\n");
		return;
	}
	entry->write_proc = proc_status_thread_w;
	entry->read_proc  = proc_status_thread_r;
#endif	

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	if((entry = proc_create(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_STATUS "/" PROC_AIPC_DEV_REGISTER, 0644, NULL, &proc_aipc_register_ops)) == NULL) {
		printk("create proc aipc_dev/status/register failed!\n");
		return;
	}
#else
	if((entry = create_proc_entry(PROC_AIPC_DEV_REGISTER, 0644, proc_aipc_dev_dir)) == NULL) {
		printk("create proc aipc_dev/status/register failed!\n");
		return;
	}
	entry->write_proc = proc_status_register_w;
	entry->read_proc  = proc_status_register_r;
#endif	

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	if((entry = proc_create(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_STATUS "/" PROC_AIPC_DEV_SHM, 0644, NULL, &proc_aipc_shm_ops)) == NULL) {
		printk("create proc aipc_dev/status/shm failed!\n");
		return;
	}
#else
	if((entry = create_proc_entry(PROC_AIPC_DEV_SHM, 0644, proc_aipc_dev_dir)) == NULL) {
		printk("create proc aipc_dev/status/shm failed!\n");
		return;
	}
	entry->write_proc = proc_status_shm_w;
	entry->read_proc  = proc_status_shm_r;
#endif		

#ifdef CONFIG_RTL8686_IPC_RECORD_DSP_LOG
	/*
	*	create dsp/log directory
	*/
	proc_aipc_dev_dir = proc_mkdir(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_DSP , NULL);
	if (proc_aipc_dev_dir == NULL) {
		printk("create proc aipc_dev/dsp failed!\n");
		return;
	}
	
	proc_aipc_dev_dir = proc_mkdir(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_DSP "/" PROC_AIPC_DEV_DIR_DSP_LOG , NULL);
	if (proc_aipc_dev_dir == NULL) {
		printk("create proc aipc_dev/dsp/log failed!\n");
		return;
	}
	
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	if((entry = proc_create(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_DSP "/" PROC_AIPC_DEV_DIR_DSP_LOG "/" PROC_AIPC_DEV_DSP_LOG_ENABLE, 0644, NULL, &proc_aipc_log_enable_ops)) == NULL) {
		printk("create proc aipc_dev/dsp/log/enable failed!\n");
		return;
	}
#else	
	if((entry = create_proc_entry(PROC_AIPC_DEV_DSP_LOG_ENABLE, 0644, proc_aipc_dev_dir)) == NULL) {
		printk("create proc aipc_dev/dsp/log/enable failed!\n");
		return;
	}
	entry->write_proc = proc_dsp_log_enable_w;
	entry->read_proc  = proc_dsp_log_enable_r;
#endif	

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	if((entry = proc_create(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_DSP "/" PROC_AIPC_DEV_DIR_DSP_LOG "/" PROC_AIPC_DEV_DSP_LOG_CLEAR, 0644, NULL, &proc_aipc_log_clear_ops)) == NULL) {
		printk("create proc aipc_dev/dsp/log/clear failed!\n");
		return;
	}
#else	
	if((entry = create_proc_entry(PROC_AIPC_DEV_DSP_LOG_CLEAR, 0644, proc_aipc_dev_dir)) == NULL) {
		printk("create proc aipc_dev/dsp/log/clear failed!\n");
		return;
	}
	entry->write_proc = proc_dsp_log_clear_w;
	entry->read_proc  = proc_dsp_log_clear_r;
#endif	

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	if((entry = proc_create(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_DSP "/" PROC_AIPC_DEV_DIR_DSP_LOG "/" PROC_AIPC_DEV_DSP_LOG_CONTENTS, 0644, NULL, &proc_aipc_log_contents_ops)) == NULL) {
		printk("create proc aipc_dev/dsp/log/contents failed!\n");
		return;
	}
#else	
	if((entry = create_proc_entry(PROC_AIPC_DEV_DSP_LOG_CONTENTS, 0644, proc_aipc_dev_dir)) == NULL) {
		printk("create proc aipc_dev/dsp/log/contents failed!\n");
		return;
	}
	entry->write_proc = proc_dsp_log_contents_w;
	entry->read_proc  = proc_dsp_log_contents_r;
#endif		

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	if((entry = proc_create(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_DSP "/" PROC_AIPC_DEV_DIR_DSP_LOG "/" PROC_AIPC_DEV_DSP_LOG_INDEX, 0644, NULL, &proc_aipc_log_index_ops)) == NULL) {
		printk("create proc aipc_dev/dsp/log/index failed!\n");
		return;
	}
#else
	if((entry = create_proc_entry(PROC_AIPC_DEV_DSP_LOG_INDEX, 0644, proc_aipc_dev_dir)) == NULL) {
		printk("create proc aipc_dev/dsp/log/index failed!\n");
		return;
	}
	entry->write_proc = proc_dsp_log_index_w;
	entry->read_proc  = proc_dsp_log_index_r;
#endif	
#endif

#ifdef CONFIG_RTL8686_IPC_DSP_CONSOLE
	/*
	*	create dsp/console directory
	*/
	proc_aipc_dev_dir = proc_mkdir(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_DSP "/" PROC_AIPC_DEV_DIR_DSP_CONSOLE , NULL);
	if (proc_aipc_dev_dir == NULL) {
		printk("create proc aipc_dev/dsp/console failed!\n");
		return;
	}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	if((entry = proc_create(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_DSP "/" PROC_AIPC_DEV_DIR_DSP_CONSOLE "/" PROC_AIPC_DEV_DSP_CONSOLE_ENABLE, 0644, NULL, &proc_aipc_console_enable_ops)) == NULL) {
		printk("create proc aipc_dev/dsp/enable failed!\n");
		return;
	}
#else	
	if((entry = create_proc_entry(PROC_AIPC_DEV_DSP_CONSOLE_ENABLE, 0644, proc_aipc_dev_dir)) == NULL) {
		printk("create proc aipc_dev/dsp/enable failed!\n");
		return;
	}
	entry->write_proc = proc_dsp_console_enable_w;
	entry->read_proc  = proc_dsp_console_enable_r;	
#endif

	/*
	*	read queue
	*/	
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	if((entry = proc_create(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_DSP "/" PROC_AIPC_DEV_DIR_DSP_CONSOLE "/" PROC_AIPC_DEV_DSP_CONSOLE_READ_ENABLE, 0644, NULL, &proc_aipc_console_read_enable_ops)) == NULL) {
		printk("create proc aipc_dev/dsp/log/read_enable failed!\n");
		return;
	}
#else		
	if((entry = create_proc_entry(PROC_AIPC_DEV_DSP_CONSOLE_READ_ENABLE, 0644, proc_aipc_dev_dir)) == NULL) {
		printk("create proc aipc_dev/dsp/log/read_enable failed!\n");
		return;
	}
	entry->write_proc = proc_dsp_console_read_queue_enable_w;
	entry->read_proc  = proc_dsp_console_read_queue_enable_r;
#endif	

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	if((entry = proc_create(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_DSP "/" PROC_AIPC_DEV_DIR_DSP_CONSOLE "/" PROC_AIPC_DEV_DSP_CONSOLE_READ_CLEAR, 0644, NULL, &proc_aipc_console_read_clear_ops)) == NULL) {
		printk("create proc aipc_dev/dsp/log/read_clear failed!\n");
		return;
	}
#else	
	if((entry = create_proc_entry(PROC_AIPC_DEV_DSP_CONSOLE_READ_CLEAR, 0644, proc_aipc_dev_dir)) == NULL) {
		printk("create proc aipc_dev/dsp/log/read_clear failed!\n");
		return;
	}
	entry->write_proc = proc_dsp_console_read_queue_clear_w;
	entry->read_proc  = proc_dsp_console_read_queue_clear_r;
#endif	

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	if((entry = proc_create(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_DSP "/" PROC_AIPC_DEV_DIR_DSP_CONSOLE "/" PROC_AIPC_DEV_DSP_CONSOLE_READ_INDEX, 0644, NULL, &proc_aipc_console_read_index_ops)) == NULL) {
		printk("create proc aipc_dev/dsp/log/index failed!\n");
		return;
	}
#else	
	if((entry = create_proc_entry(PROC_AIPC_DEV_DSP_CONSOLE_READ_INDEX, 0644, proc_aipc_dev_dir)) == NULL) {
		printk("create proc aipc_dev/dsp/log/index failed!\n");
		return;
	}
	entry->write_proc = proc_dsp_console_read_queue_index_w;
	entry->read_proc  = proc_dsp_console_read_queue_index_r;
#endif	
	/*
	*	write queue
	*/	
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	if((entry = proc_create(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_DSP "/" PROC_AIPC_DEV_DIR_DSP_CONSOLE "/" PROC_AIPC_DEV_DSP_CONSOLE_WRITE_ENABLE, 0644, NULL, &proc_aipc_console_write_enable_ops)) == NULL) {
		printk("create proc aipc_dev/dsp/log/write_enable failed!\n");
		return;
	}
#else		
	if((entry = create_proc_entry(PROC_AIPC_DEV_DSP_CONSOLE_WRITE_ENABLE, 0644, proc_aipc_dev_dir)) == NULL) {
		printk("create proc aipc_dev/dsp/log/write_enable failed!\n");
		return;
	}
	entry->write_proc = proc_dsp_console_write_queue_enable_w;
	entry->read_proc  = proc_dsp_console_write_queue_enable_r;
#endif	

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	if((entry = proc_create(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_DSP "/" PROC_AIPC_DEV_DIR_DSP_CONSOLE "/" PROC_AIPC_DEV_DSP_CONSOLE_WRITE_CLEAR, 0644, NULL, &proc_aipc_console_write_clear_ops)) == NULL) {
		printk("create proc aipc_dev/dsp/log/write_clear failed!\n");
		return;
	}
#else	
	if((entry = create_proc_entry(PROC_AIPC_DEV_DSP_CONSOLE_WRITE_CLEAR, 0644, proc_aipc_dev_dir)) == NULL) {
		printk("create proc aipc_dev/dsp/log/write_clear failed!\n");
		return;
	}
	entry->write_proc = proc_dsp_console_write_queue_clear_w;
	entry->read_proc  = proc_dsp_console_write_queue_clear_r;
#endif	

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	if((entry = proc_create(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_DSP "/" PROC_AIPC_DEV_DIR_DSP_CONSOLE "/" PROC_AIPC_DEV_DSP_CONSOLE_WRITE_INDEX, 0644, NULL, &proc_aipc_console_write_index_ops)) == NULL) {
		printk("create proc aipc_dev/dsp/log/write_index failed!\n");
		return;
	}
#else
	if((entry = create_proc_entry(PROC_AIPC_DEV_DSP_CONSOLE_WRITE_INDEX, 0644, proc_aipc_dev_dir)) == NULL) {
		printk("create proc aipc_dev/dsp/log/write_index failed!\n");
		return;
	}
	entry->write_proc = proc_dsp_console_write_queue_index_w;
	entry->read_proc  = proc_dsp_console_write_queue_index_r;
#endif	
#endif

#ifdef CONFIG_RTL8686_IPC_DSL_IPC
	proc_aipc_dev_dir = proc_mkdir(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_DSL , NULL);
	if (proc_aipc_dev_dir == NULL) {
		printk("create proc aipc_dev/dsl failed!\n");
		return;
	}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	if((entry = proc_create(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_DSL "/" PROC_AIPC_DEV_DSL_IPC_COUNTERS, 0644, NULL, &proc_aipc_dsl_ipc_counter_ops)) == NULL) {
		printk("create proc aipc_dev/dsl/ipc_counters failed!\n");
		return;
	}
#else
	if((entry = create_proc_entry(PROC_AIPC_DEV_DSL_IPC_COUNTERS, 0644, proc_aipc_dev_dir)) == NULL) {
		printk("create proc aipc_dev/dsl/ipc_counters failed!\n");
		return;
	}
	entry->write_proc = proc_dsl_ipc_counters_w;
	entry->read_proc  = proc_dsl_ipc_counters_r;
#endif	
#endif

	/*
	*	create misc directory
	*/
#if 0
	proc_aipc_dev_dir = proc_mkdir(PROC_AIPC_DEV_DIR "/" PROC_AIPC_DEV_DIR_MISC , NULL);
	if (proc_aipc_dev_dir == NULL) {
		printk("create proc aipc_dev/misc failed!\n");
		return;
	}

	if((entry = create_proc_entry(PROC_AIPC_DEV_OP, 0644, proc_aipc_dev_dir)) == NULL){
		printk("create proc aipc_dev/misc/operations failed!\n");
		return;
	}
	entry->proc_fops = &aipc_dev_proc_ops;

	create_proc_read_entry( PROC_AIPC_DEV_PHYADDR , 0644 , 
		proc_aipc_dev_dir , proc_read_phymem_addr , NULL );
	create_proc_read_entry( PROC_AIPC_DEV_LOGADDR , 0644 , 
		proc_aipc_dev_dir , proc_read_logmem_addr , NULL );
#endif
}

#endif

