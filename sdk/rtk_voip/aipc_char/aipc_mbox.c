#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/version.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
#include <linux/seq_file.h>    // for sequence files
#endif

#elif defined(__ECOS)
#include <cyg/infra/cyg_type.h>
#include <cyg/kernel/kapi.h>


#else
#error "not supported type"
#endif 

#include "./include/aipc_osal.h"
#include "./include/aipc_mem.h"
#include "./include/aipc_mbox.h"
#include "./include/aipc_ioctl.h"
#include "./include/aipc_debug.h"
#include "./include/aipc_swp.h"


/*
*		CPU						DSP
*   ----------------------------------------
*	mbox_2cpu.mb			mbox_2dsp.mb
*
*	mbox_2cpu.bc			mbox_2dsp.bc
*
*	mail_2cpu.m				mail_2dsp.m
*/


#ifdef _AIPC_CPU_
int    aipc_2dsp_mb_enqueue( void *dp );
void * aipc_2dsp_bc_dequeue( void );
void * aipc_2cpu_mb_dequeue( void );
int    aipc_2cpu_bc_enqueue( void *dp );

//export API
void * aipc_data_2dsp_alloc( void )
{
	return aipc_2dsp_bc_dequeue();
}

int    aipc_data_2dsp_send( void *dp )
{
	return aipc_2dsp_mb_enqueue( dp );
}

int    aipc_data_2cpu_ret( void *dp )
{
	return aipc_2cpu_bc_enqueue( dp );
}

void *	aipc_data_2cpu_recv( void )
{
	return aipc_2cpu_mb_dequeue();
}

EXPORT_SYMBOL(aipc_data_2dsp_alloc);
EXPORT_SYMBOL(aipc_data_2dsp_send);
EXPORT_SYMBOL(aipc_data_2cpu_ret);
EXPORT_SYMBOL(aipc_data_2cpu_recv);

#else

int    aipc_2cpu_mb_enqueue( void *dp );
void * aipc_2cpu_bc_dequeue( void );
void * aipc_2dsp_mb_dequeue( void );
int    aipc_2dsp_bc_enqueue( void *dp );

//export API
void * aipc_data_2cpu_alloc( void )
{
	return aipc_2cpu_bc_dequeue();
}

int    aipc_data_2cpu_send( void *dp )
{
	return aipc_2cpu_mb_enqueue( dp );
}

int    aipc_data_2dsp_ret( void *dp )
{
	return aipc_2dsp_bc_enqueue( dp );
}

void * aipc_data_2dsp_recv( void)
{
	return aipc_2dsp_mb_dequeue();
}
#ifdef __KERNEL__
EXPORT_SYMBOL(aipc_data_2cpu_alloc);
EXPORT_SYMBOL(aipc_data_2cpu_send);
EXPORT_SYMBOL(aipc_data_2dsp_ret);
EXPORT_SYMBOL(aipc_data_2dsp_recv);
#endif
#endif

#ifdef AIPC_GOT
#ifdef _AIPC_CPU_
static void 
aipc_cpu_got_init(void)
{
	/* CPU */
	AGOT.aipc_2cpu_mb_dequeue = aipc_2cpu_mb_dequeue;
	AGOT.aipc_2cpu_bc_enqueue = aipc_2cpu_bc_enqueue;
	AGOT.aipc_2dsp_bc_dequeue = aipc_2dsp_bc_dequeue;
	AGOT.aipc_2dsp_mb_enqueue = aipc_2dsp_mb_enqueue;
}

#elif defined(_AIPC_DSP_)
static void 
aipc_dsp_got_init(void)
{
	/* DSP */
	AGOT.aipc_2cpu_mb_enqueue = aipc_2cpu_mb_enqueue;
	AGOT.aipc_2cpu_bc_dequeue = aipc_2cpu_bc_dequeue;
	AGOT.aipc_2dsp_mb_dequeue = aipc_2dsp_mb_dequeue;
	AGOT.aipc_2dsp_bc_enqueue = aipc_2dsp_bc_enqueue;
}
#else	//!_AIPC_CPU_ && !//_AIPC_DSP_
#error "not supported type"
#endif 
#endif

/*
*	CPU
*/
/*
*	Check Queue status
*/
//#ifdef _AIPC_CPU_
int
aipc_2dsp_mb_full( void )
{
	if(((MB_2DSP.del + MB_2DSP_TOTAL - MB_2DSP.ins)%MB_2DSP_TOTAL) == 1){
		ADEBUG(DBG_MBOX , "full\n");
		return TRUE;
		}
	else{
		ADEBUG(DBG_MBOX , "not full\n");
		return FALSE;
		}
}
//#endif

//#ifdef _AIPC_DSP_
int
aipc_2dsp_mb_empty( void )
{
	if(((MB_2DSP.ins + MB_2DSP_TOTAL - MB_2DSP.del)%MB_2DSP_TOTAL) == 0){
		ADEBUG(DBG_MBOX , "empty\n");
		return TRUE;
		}
	else{
		ADEBUG(DBG_MBOX , "not empty\n");
		return FALSE;
		}
}
//#endif

int
aipc_2dsp_bc_full( void )
{
	if(((BC_2DSP.del + BC_2DSP_TOTAL - BC_2DSP.ins)%BC_2DSP_TOTAL) == 1){
		ADEBUG(DBG_MBOX , "full\n");
		return TRUE;
		}
	else{
		ADEBUG(DBG_MBOX , "not full\n");
		return FALSE;
		}
}

//#ifdef _AIPC_CPU_
int
aipc_2dsp_bc_empty( void )
{	
	if(((BC_2DSP.ins + BC_2DSP_TOTAL - BC_2DSP.del)%BC_2DSP_TOTAL) == 0){
		ADEBUG(DBG_MBOX , "empty\n");
		return TRUE;
		}
	else{
		ADEBUG(DBG_MBOX , "not empty\n");
		return FALSE;
		}
}
//#endif

/*
*	DSP
*/
/*
*	Check Queue status
*/
//#ifdef _AIPC_DSP_
int
aipc_2cpu_mb_full( void )
{
	if((MB_2CPU.del + MB_2CPU_TOTAL - MB_2CPU.ins)%MB_2CPU_TOTAL == 1){
		ADEBUG(DBG_MBOX , "full\n");
		return TRUE;
		}
	else{
		ADEBUG(DBG_MBOX , "not full\n");
		return FALSE;
		}
}
//#endif

//#ifdef _AIPC_CPU_
int
aipc_2cpu_mb_empty( void )
{
	if(((MB_2CPU.ins + MB_2CPU_TOTAL - MB_2CPU.del)%MB_2CPU_TOTAL) == 0){
		ADEBUG(DBG_MBOX , "empty\n");
		return TRUE;
		}
	else{
		ADEBUG(DBG_MBOX , "not empty\n");
		return FALSE;
		}
}

int
aipc_2cpu_bc_full( void )
{
	if((BC_2CPU.del + BC_2CPU_TOTAL - BC_2CPU.ins)%BC_2CPU_TOTAL == 1){
		ADEBUG(DBG_MBOX , "full\n");
		return TRUE;
		}
	else{
		ADEBUG(DBG_MBOX , "not full\n");
		return FALSE;
		}
}
//#endif

//#ifdef _AIPC_DSP_
int
aipc_2cpu_bc_empty( void )
{	
	if(((BC_2CPU.ins + BC_2CPU_TOTAL - BC_2CPU.del)%BC_2CPU_TOTAL) == 0){
		ADEBUG(DBG_MBOX , "empty\n");
		return TRUE;
		}
	else{
		ADEBUG(DBG_MBOX , "not empty\n");
		return FALSE;
		}
}
//#endif

#ifdef __KERNEL__
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
int aipc_mbox_dump (struct seq_file *f)
{
	int n = 0;
	
	seq_printf( f, "Mbox:\n");

	seq_printf( f, "   MB_2DSP:");
	if(aipc_2dsp_mb_empty()){
		seq_printf( f, "empty\n");
	}else if(aipc_2dsp_mb_full()){
		seq_printf( f, "full\n");
	}else{
		seq_printf( f, "middle\n");
	}
	seq_printf( f, "\tindex ins=%d del=%d total=%d\n" , MB_2DSP.ins , MB_2DSP.del , MB_2DSP_TOTAL);
	seq_printf( f, "\tcount ins=%d del=%d\n"          , MB_2DSP.cnt_ins , MB_2DSP.cnt_del );

	seq_printf( f, "   BC_2DSP:");
	if(aipc_2dsp_bc_empty()){
		seq_printf( f, "empty\n");
	}else if(aipc_2dsp_bc_full()){
		seq_printf( f, "full\n");
	}else{
		seq_printf( f, "middle\n");
	}
	seq_printf( f, "\tindex ins=%d del=%d total=%d\n" , BC_2DSP.ins , BC_2DSP.del , BC_2DSP_TOTAL);
	seq_printf( f, "\tcount ins=%d del=%d\n"          , BC_2DSP.cnt_ins , BC_2DSP.cnt_del);

	seq_printf( f, "   MB_2CPU:");
	if(aipc_2cpu_mb_empty()){
		seq_printf( f, "empty\n");
	}else if(aipc_2cpu_mb_full()){
		seq_printf( f, "full\n");
	}else{
		seq_printf( f, "middle\n");
	}
	seq_printf( f, "\tindex ins=%d del=%d total=%d\n" , MB_2CPU.ins , MB_2CPU.del , BC_2CPU_TOTAL);
	seq_printf( f, "\tcount ins=%d del=%d\n"          , MB_2CPU.cnt_ins , MB_2CPU.cnt_del);

	seq_printf( f, "   BC_2CPU:");
	if(aipc_2cpu_bc_empty()){
		seq_printf( f, "empty\n");
	}else if(aipc_2cpu_bc_full()){
		seq_printf( f, "full\n");
	}else{
		seq_printf( f, "middle\n");
	}
	seq_printf( f, "\tindex ins=%d del=%d total=%d\n" , BC_2CPU.ins , BC_2CPU.del , BC_2CPU_TOTAL);
	seq_printf( f, "\tcount ins=%d del=%d\n\n"        , BC_2CPU.cnt_ins , BC_2CPU.cnt_del);


	return n;
}

#else
int aipc_mbox_dump(char *buf)
{
	int n = 0;
	
	n += sprintf(buf , "Mbox:\n");

	n += sprintf(buf+n , "   MB_2DSP:");
	if(aipc_2dsp_mb_empty()){
		n += sprintf(buf+n , "empty\n");
	}else if(aipc_2dsp_mb_full()){
		n += sprintf(buf+n , "full\n");
	}else{
		n += sprintf(buf+n , "middle\n");
	}
	n += sprintf(buf+n , "\tindex ins=%d del=%d total=%d\n" , MB_2DSP.ins , MB_2DSP.del , MB_2DSP_TOTAL);
	n += sprintf(buf+n , "\tcount ins=%d del=%d\n"          , MB_2DSP.cnt_ins , MB_2DSP.cnt_del );

	n += sprintf(buf+n , "   BC_2DSP:");
	if(aipc_2dsp_bc_empty()){
		n += sprintf(buf+n , "empty\n");
	}else if(aipc_2dsp_bc_full()){
		n += sprintf(buf+n , "full\n");
	}else{
		n += sprintf(buf+n , "middle\n");
	}
	n += sprintf(buf+n , "\tindex ins=%d del=%d total=%d\n" , BC_2DSP.ins , BC_2DSP.del , BC_2DSP_TOTAL);
	n += sprintf(buf+n , "\tcount ins=%d del=%d\n"          , BC_2DSP.cnt_ins , BC_2DSP.cnt_del);

	n += sprintf(buf+n , "   MB_2CPU:");
	if(aipc_2cpu_mb_empty()){
		n += sprintf(buf+n , "empty\n");
	}else if(aipc_2cpu_mb_full()){
		n += sprintf(buf+n , "full\n");
	}else{
		n += sprintf(buf+n , "middle\n");
	}
	n += sprintf(buf+n , "\tindex ins=%d del=%d total=%d\n" , MB_2CPU.ins , MB_2CPU.del , BC_2CPU_TOTAL);
	n += sprintf(buf+n , "\tcount ins=%d del=%d\n"          , MB_2CPU.cnt_ins , MB_2CPU.cnt_del);

	n += sprintf(buf+n , "   BC_2CPU:");
	if(aipc_2cpu_bc_empty()){
		n += sprintf(buf+n , "empty\n");
	}else if(aipc_2cpu_bc_full()){
		n += sprintf(buf+n , "full\n");
	}else{
		n += sprintf(buf+n , "middle\n");
	}
	n += sprintf(buf+n , "\tindex ins=%d del=%d total=%d\n" , BC_2CPU.ins , BC_2CPU.del , BC_2CPU_TOTAL);
	n += sprintf(buf+n , "\tcount ins=%d del=%d\n\n"        , BC_2CPU.cnt_ins , BC_2CPU.cnt_del);

	return n;
}
#endif
#endif


#ifdef _AIPC_CPU_
/*
*	Init is done by CPU
*/

static void
aipc_dsp_mbox_init( void )
{
}


static void
aipc_2dsp_mb_init( void )
{
	MB_2DSP.ins= 0;
	MB_2DSP.del= 0;
}

//Modified from DSP code. only for init use
static int
aipc_2dsp_bc_enqueue_init( void *dp )
{
	int ret=NOK;

	if(aipc_2dsp_bc_full()){
		ADEBUG(DBG_MBOX , "enqueue NOK\n");
		ret=NOK;
		}
	else{
		BC_2DSP.bc[ BC_2DSP.ins ] = dp;
		BC_2DSP.ins = (BC_2DSP.ins+1) % BC_2DSP_TOTAL;
		ADEBUG(DBG_MBOX , "enqueue %p OK\n" , dp);
		ret=OK;
		}
	return ret;
}

//Modified from CPU code. only for init use
static int
aipc_2cpu_bc_enqueue_init( void *dp )
{
	int ret=NOK;
	
	if(aipc_2cpu_bc_full()){
		ADEBUG(DBG_MBOX , "enqueue NOK\n");
		ret=NOK;
		}
	else{
		BC_2CPU.bc[ BC_2CPU.ins ] = dp;
		BC_2CPU.ins = (BC_2CPU.ins+1) % MB_2CPU_TOTAL;
		ADEBUG(DBG_MBOX , "enqueue %p OK\n" , dp);
		ret=OK;
		}
	return ret;	
}


static void
aipc_2dsp_bc_init( void )
{
	int i;
	int ret=NOK;
	BC_2DSP.ins = 0;
	BC_2DSP.del = 0;

	for( i=0 ; i<MAIL_2DSP_TOTAL ; i++ ){
		ret = aipc_2dsp_bc_enqueue_init( &MAIL_2DSP.m[i] );
		if(ret==NOK){
			printk( "2DSP BC init failed\n" );
			}
		}

//	SDEBUG("BC_2DSP.ins=%d i=%d\n" , BC_2DSP.ins , i);
}

static void
aipc_2cpu_mb_init( void )
{
	MB_2CPU.ins = 0;
	MB_2CPU.del = 0;
}

static void
aipc_2cpu_bc_init( void )
{
	int i;
	int ret=NOK;
	BC_2CPU.ins = 0;
	BC_2CPU.del = 0;
	
	for( i=0 ; i<MAIL_2CPU_TOTAL ; i++ ){
		ret = aipc_2cpu_bc_enqueue_init( &MAIL_2CPU.m[i] );
		if(ret==NOK){
			printk( "2CPU BC init failed\n" );
			}
		}

//	SDEBUG("BC_2CPU.ins=%d i=%d\n" , BC_2CPU.ins , i);
}

void
aipc_mb_bc_init( void )
{
	aipc_2dsp_mb_init();
	aipc_2dsp_bc_init();
	aipc_2cpu_mb_init();
	aipc_2cpu_bc_init();
	aipc_dsp_mbox_init();
	#ifdef AIPC_GOT
	aipc_cpu_got_init();
	#endif
}

#if !defined(AIPC_FORCE_INTERRUPT_CONTEXT)
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
static DECLARE_MUTEX(cpu_mb_mutex);
#else
static DEFINE_SEMAPHORE(cpu_mb_mutex);
#endif
#endif

/*
*	Queue operations
*/

/*
*	CPU send data to DSP
*	1. Get free buffer through aipc_2dsp_bc_dequeue()
*	2. Add data in buffer
*	3. Send to DSP through aipc_2dsp_mb_enqueue()
*	Usage
*	1. Use these APIs in kernel thread to generate data
*/
int
aipc_2dsp_mb_enqueue( void *dp )
{
	#ifdef INT_NOTIFY_DATA_2DSP
	extern int probe_cpu_int_notify_cpu2dsp;
	extern int aipc_int_send_2dsp( u32_t int_id );
	u32_t int_id = AIPC_MGR_2DSP_MBOX_RECV;
	#endif
	int ret=NOK;
	
	#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;
	#endif

	#ifdef MBOX_LOCK
	#ifdef AIPC_FORCE_INTERRUPT_CONTEXT
	AIPC_OSAL_INTERRUPT_FLAGS flags;
	AIPC_OSAL_INTERRUPT_DISABLE(flags);
	#else
	AIPC_OSAL_MUTEX_LOCK(&cpu_mb_mutex);
	#endif
	ADEBUG(DBG_MBOX , "in mutex\n");
	#endif

	#ifdef AIPC_BARRIER
		tmp_wr = ASTATS.aipc_data_2dsp_send;
		tmp_wr++;
		AMB( ASTATS.aipc_data_2dsp_send , tmp_wr);
	#else
		ASTATS.aipc_data_2dsp_send++;
	#endif

	if(aipc_2dsp_mb_full()){
		ret=NOK;
		ADEBUG(DBG_MBOX , "enqueue NOK\n");
		}
	else{
		#ifdef AIPC_BARRIER
			tmp_wr = MB_2DSP.ins;
			AMB( MB_2DSP.mb[ tmp_wr ] , dp );
			
			tmp_wr = (tmp_wr+1) % MB_2DSP_TOTAL;
			AMB( MB_2DSP.ins , tmp_wr );
		
			#ifdef QUEUE_OPERATION_CNT
			tmp_wr = MB_2DSP.cnt_ins;
			tmp_wr++;
			AMB( MB_2DSP.cnt_ins , tmp_wr );
			#endif
		#else
			MB_2DSP.mb[MB_2DSP.ins] = dp;
			MB_2DSP.ins = (MB_2DSP.ins+1) % MB_2DSP_TOTAL;
		
			#ifdef QUEUE_OPERATION_CNT
			MB_2DSP.cnt_ins++;
			#endif
		#endif

		#ifdef INT_NOTIFY_DATA_2DSP
		if( probe_cpu_int_notify_cpu2dsp == 1 )
			ret = aipc_int_send_2dsp( int_id );
		else
			ret = OK;

		if (ret==OK){
			ADEBUG(DBG_MBOX , "enqueue %p OK\n\n" , dp);
			//SDEBUG("enqueue %p OK\n\n" , dp);
			}
		else{
			ADEBUG(DBG_MBOX , "enqueue %p failed!!!!!\n\n" , dp);
			//SDEBUG("enqueue %p failed!!!!!\n\n" , dp);
			}
		#else
		ret = OK;
		#endif
		
		ADEBUG(DBG_MBOX , "enqueue OK\n");
		}

	#ifdef MBOX_LOCK
	#ifdef AIPC_FORCE_INTERRUPT_CONTEXT
	AIPC_OSAL_INTERRUPT_ENABLE(flags);
	#else
	AIPC_OSAL_MUTEX_UNLOCK(&cpu_mb_mutex);
	#endif
	ADEBUG(DBG_MBOX , "out mutex\n");
	#endif	

	return ret;
}

void *
aipc_2dsp_bc_dequeue( void )
{
	void *retp;
	unsigned long now = 0;

	#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;
	#endif

	#ifdef CHECK_ALLOC
	static unsigned long last_alloc_time=0;
	static unsigned long try_alloc_cnt=0;
	unsigned long past_tick=0;
	#endif

	#ifdef MBOX_LOCK
	#ifdef AIPC_FORCE_INTERRUPT_CONTEXT
	AIPC_OSAL_INTERRUPT_FLAGS flags;
	AIPC_OSAL_INTERRUPT_DISABLE(flags);
	#else
	AIPC_OSAL_MUTEX_LOCK(&cpu_mb_mutex);
	#endif
	ADEBUG(DBG_MBOX , "in mutex\n");
	#endif

	now = jiffies;
	
	#ifdef CHECK_ALLOC
	try_alloc_cnt++;
	#endif
	
	#ifdef AIPC_BARRIER
	tmp_wr = ASTATS.aipc_data_2dsp_alloc;
	tmp_wr++;
	AMB( ASTATS.aipc_data_2dsp_alloc , tmp_wr );
	#else
	ASTATS.aipc_data_2dsp_alloc++;
	#endif

	if(aipc_2dsp_bc_empty()){
		retp = NULL;  //Buffer Circulation is Empty
		ADEBUG(DBG_MBOX , "dequeue NOK\n");
		
		#ifdef CHECK_ALLOC
		past_tick = now - last_alloc_time;
		if (past_tick >= CHECK_TIME_ALLOC){
			printk("try allocate %lu in past %lu(ms). average %lu %s(%d)\n" , 
			try_alloc_cnt , past_tick , (try_alloc_cnt/past_tick) , __FUNCTION__ , __LINE__);
		}
		#endif
		}
	else{
		#ifdef AIPC_BARRIER
			tmp_wr = BC_2DSP.del;
			retp = BC_2DSP.bc[ tmp_wr ];
			tmp_wr = (tmp_wr+1) % BC_2DSP_TOTAL;
			AMB( BC_2DSP.del , tmp_wr );
	
			#ifdef CHECK_ALLOC
			last_alloc_time = now;
			try_alloc_cnt = 0;
			#endif
	
			#ifdef QUEUE_OPERATION_CNT
			tmp_wr = BC_2DSP.cnt_del;
			tmp_wr++;
			AMB( BC_2DSP.cnt_del , tmp_wr );
			#endif
		#else
			retp = BC_2DSP.bc[ BC_2DSP.del ];
			BC_2DSP.del = (BC_2DSP.del+1) % BC_2DSP_TOTAL;
	
			#ifdef CHECK_ALLOC
			last_alloc_time = now;
			try_alloc_cnt = 0;
			#endif
	
			#ifdef QUEUE_OPERATION_CNT
			BC_2DSP.cnt_del++;
			#endif
		#endif
	
		ADEBUG(DBG_MBOX , "dequeue %p OK\n" , retp);
		}

	//check usage
	#if 0
	unsigned long cur_left  = 0;
	unsigned long cur_use   = 0;

	cur_left = (BC_2DSP.ins + BC_2DSP_TOTAL - BC_2DSP.del)%BC_2DSP_TOTAL;
	cur_use  = BC_2DSP_TOTAL - cur_left;
	
	if(cur_use >= 10){
		printk("use %ld left %ld. time %lx %s(%d)\n" , 
			cur_use , cur_left , now , __FUNCTION__ , __LINE__);
	}
	#endif

	#ifdef MBOX_LOCK
	#ifdef AIPC_FORCE_INTERRUPT_CONTEXT
	AIPC_OSAL_INTERRUPT_ENABLE(flags);
	#else
	AIPC_OSAL_MUTEX_UNLOCK(&cpu_mb_mutex);
	#endif
	ADEBUG(DBG_MBOX , "out mutex\n");
	#endif	

	return retp;
}


/*
*	CPU receive data from DSP
*	1. Get data from aipc_2cpu_mb_dequeue()
*	2. Get data in buffer
*	3. Return buffer to DSP through aipc_2cpu_bc_enqueue()
*	Usage
*	1. Use these APIs in ISR or DSR for receiving data
*/
void *	
aipc_2cpu_mb_dequeue( void )
{
	void *retp;

	#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;
	#endif

	#ifdef MBOX_LOCK
	#ifdef ISR_DSR_LOCK 		//tasklet
	AIPC_OSAL_INTERRUPT_FLAGS flags;
	AIPC_OSAL_INTERRUPT_DISABLE(flags);
	ADEBUG(DBG_MBOX , "disable interrupt\n");
	#endif
	#endif

	#ifdef AIPC_BARRIER
	tmp_wr = ASTATS.aipc_data_2cpu_recv;
	tmp_wr++;
	AMB( ASTATS.aipc_data_2cpu_recv , tmp_wr );
	#else
	ASTATS.aipc_data_2cpu_recv++;
	#endif
	
	if(aipc_2cpu_mb_empty()){
		retp=NULL;
		ADEBUG(DBG_MBOX , "dequeue NOK\n");
		}
	else{
		#ifdef AIPC_BARRIER
			tmp_wr = MB_2CPU.del;
			retp   = MB_2CPU.mb[ tmp_wr ];
			tmp_wr = (tmp_wr+1) % MB_2CPU_TOTAL;
			AMB( MB_2CPU.del , tmp_wr );
	
			#ifdef QUEUE_OPERATION_CNT
			tmp_wr = MB_2CPU.cnt_del;
			tmp_wr++;
			AMB( MB_2CPU.cnt_del , tmp_wr );
			#endif
		#else
			retp = MB_2CPU.mb[ MB_2CPU.del ];
			MB_2CPU.del = (MB_2CPU.del+1) % MB_2CPU_TOTAL;
	
			#ifdef QUEUE_OPERATION_CNT
			MB_2CPU.cnt_del++;
			#endif
		#endif
		
		ADEBUG(DBG_MBOX , "dequeue %p OK\n" , retp);
		}
	
	#ifdef MBOX_LOCK
	#ifdef ISR_DSR_LOCK 		//tasklet
	AIPC_OSAL_INTERRUPT_ENABLE(flags);
	ADEBUG(DBG_MBOX , "restore interrupt\n");
	#endif	
	#endif

	return retp;
}

int
aipc_2cpu_bc_enqueue( void *dp )
{
	int ret=NOK;
	
	#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;
	#endif
	
	#ifdef MBOX_LOCK
	#ifdef ISR_DSR_LOCK			//tasklet
	AIPC_OSAL_INTERRUPT_FLAGS flags;
	AIPC_OSAL_INTERRUPT_DISABLE(flags);
	ADEBUG(DBG_MBOX , "disable interrupt\n");
	#endif
	#endif

	#ifdef AIPC_BARRIER
	tmp_wr = ASTATS.aipc_data_2cpu_ret;
	tmp_wr++;
	AMB( ASTATS.aipc_data_2cpu_ret , tmp_wr );
	#else
	ASTATS.aipc_data_2cpu_ret++;
	#endif

	if(aipc_2cpu_bc_full()){
		ret=NOK;
		ADEBUG(DBG_MBOX , "enqueue NOK\n");
		}
	else{
		#ifdef AIPC_BARRIER
			tmp_wr = BC_2CPU.ins;
			AMB( BC_2CPU.bc[ tmp_wr ] , dp );
			tmp_wr = (tmp_wr+1) % BC_2CPU_TOTAL;
			AMB( BC_2CPU.ins , tmp_wr );
	
			#ifdef QUEUE_OPERATION_CNT
			tmp_wr = BC_2CPU.cnt_ins;
			tmp_wr++;
			AMB( BC_2CPU.cnt_ins , tmp_wr );
			#endif
		#else
			BC_2CPU.bc[ BC_2CPU.ins ] = dp;
			BC_2CPU.ins = (BC_2CPU.ins+1) % BC_2CPU_TOTAL;
	
			#ifdef QUEUE_OPERATION_CNT
			BC_2CPU.cnt_ins++;
			#endif
		#endif
		
		ret=OK;
		ADEBUG(DBG_MBOX , "enqueue %p OK\n" , dp);
		}

	#ifdef MBOX_LOCK
	#ifdef ISR_DSR_LOCK 		//tasklet
	AIPC_OSAL_INTERRUPT_ENABLE(flags);
	ADEBUG(DBG_MBOX , "restore interrupt\n");
	#endif	
	#endif

	return ret;	
}



#else   // defined(_AIPC_DSP_)
extern int dsp_ctrl_event_task_init(void);

#ifdef __KERNEL__

#if !defined(AIPC_FORCE_INTERRUPT_CONTEXT)
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
static DECLARE_MUTEX(dsp_mb_mutex);
#else
static DEFINE_SEMAPHORE(dsp_mb_mutex);
#endif
#endif

#elif defined(__ECOS)
static cyg_mutex_t dsp_mb_mutex;
#endif

void aipc_dsp_mb_init(void)
{
#ifdef __ECOS
	cyg_mutex_init(&dsp_mb_mutex);
#endif
}

/*
*	Queue operations
*/
/*
*	DSP send data to CPU
*	1. Get free buffer through aipc_2cpu_bc_dequeue()
*	2. Add data in buffer
*	3. Send to DSP through aipc_2cpu_mb_enqueue()
*	Usage
*	1. Use these APIs in kernel thread to generate data
*/
int
aipc_2cpu_mb_enqueue( void *dp )
{
	#ifdef INT_NOTIFY_DATA_2CPU 
	extern int aipc_int_send_2cpu( u32_t int_id );
	u32_t int_id = AIPC_MGR_2CPU_MBOX_RECV;
	#endif
	int   ret=NOK;

	#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;
	#endif

	#ifdef MBOX_LOCK
	#ifdef AIPC_FORCE_INTERRUPT_CONTEXT
	AIPC_OSAL_INTERRUPT_FLAGS flags;
	AIPC_OSAL_INTERRUPT_DISABLE(flags);
	#else
	AIPC_OSAL_MUTEX_LOCK( &dsp_mb_mutex );
	#endif
	ADEBUG(DBG_MBOX , "in mutex\n");
	#endif

	#ifdef AIPC_BARRIER
		tmp_wr = ASTATS.aipc_data_2cpu_send;
		tmp_wr++;
		AMB( ASTATS.aipc_data_2cpu_send , tmp_wr );
	#endif


	if(aipc_2cpu_mb_full()){
		ret=NOK;
		ADEBUG(DBG_MBOX , "enqueue NOK\n");
		}
	else{
		#ifdef AIPC_BARRIER
			tmp_wr = MB_2CPU.ins;
			AMB( MB_2CPU.mb[ tmp_wr ] , dp );
			
			tmp_wr = (tmp_wr+1) % MB_2CPU_TOTAL;
			AMB( MB_2CPU.ins , tmp_wr );
	
			#ifdef QUEUE_OPERATION_CNT
			tmp_wr = MB_2CPU.cnt_ins;
			tmp_wr++;
			AMB( MB_2CPU.cnt_ins , tmp_wr );
			#endif		
		#else
			MB_2CPU.mb[ MB_2CPU.ins ] = dp;
			MB_2CPU.ins = (MB_2CPU.ins+1) % MB_2CPU_TOTAL;
	
			#ifdef QUEUE_OPERATION_CNT
			MB_2CPU.cnt_ins++;
			#endif
		#endif
		
		#ifdef INT_NOTIFY_DATA_2CPU
		ret = aipc_int_send_2cpu( int_id );
		if (ret==OK){
			ADEBUG(DBG_MBOX , "enqueue %p OK\n\n" , dp);
			//SDEBUG("enqueue %p OK\n\n" , dp);
			}
		else{
			ADEBUG(DBG_MBOX , "enqueue %p failed!!!!!\n\n" , dp);
			//SDEBUG("enqueue %p failed!!!!!\n\n" , dp);
			}
		#else
		ret=OK;
		#endif
		}

	#ifdef MBOX_LOCK
	#ifdef AIPC_FORCE_INTERRUPT_CONTEXT
	AIPC_OSAL_INTERRUPT_ENABLE(flags);
	#else
	AIPC_OSAL_MUTEX_UNLOCK(&dsp_mb_mutex);
	#endif
	ADEBUG(DBG_MBOX , "out mutex\n");
	#endif

	return ret;
}

void *
aipc_2cpu_bc_dequeue( void )
{
	void *retp;

	#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;
	#endif

	#ifdef MBOX_LOCK
	#ifdef AIPC_FORCE_INTERRUPT_CONTEXT
	AIPC_OSAL_INTERRUPT_FLAGS flags;
	AIPC_OSAL_INTERRUPT_DISABLE(flags);
	#else
	AIPC_OSAL_MUTEX_LOCK( &dsp_mb_mutex );
	#endif
	ADEBUG(DBG_MBOX , "in mutex\n");
	#endif

	#ifdef AIPC_BARRIER
		tmp_wr = ASTATS.aipc_data_2cpu_alloc;
		tmp_wr++;
		AMB( ASTATS.aipc_data_2cpu_alloc , tmp_wr );
	#else
		ASTATS.aipc_data_2cpu_alloc++;
	#endif

	if(aipc_2cpu_bc_empty()){
		retp = NULL;  //Buffer Circulation is Empty
		ADEBUG(DBG_MBOX , "dequeue NOK\n");
		}
	else{
		#ifdef AIPC_BARRIER
			tmp_wr = BC_2CPU.del;
			retp   = BC_2CPU.bc[ tmp_wr ];
			tmp_wr = (tmp_wr+1) % BC_2CPU_TOTAL;
			AMB( BC_2CPU.del , tmp_wr );
	
			#ifdef QUEUE_OPERATION_CNT
			tmp_wr = BC_2CPU.cnt_del;
			tmp_wr++;
			AMB( BC_2CPU.cnt_del , tmp_wr );
			#endif
		
		#else
			retp = BC_2CPU.bc[ BC_2CPU.del ];
			BC_2CPU.del = (BC_2CPU.del+1) % BC_2CPU_TOTAL;
	
			#ifdef QUEUE_OPERATION_CNT
			BC_2CPU.cnt_del++;
			#endif
		#endif
	
		ADEBUG(DBG_MBOX , "dequeue %p OK\n" , retp);
		}

	#ifdef MBOX_LOCK
	#ifdef AIPC_FORCE_INTERRUPT_CONTEXT
	AIPC_OSAL_INTERRUPT_ENABLE(flags);
	#else
	AIPC_OSAL_MUTEX_UNLOCK(&dsp_mb_mutex);
	#endif
	ADEBUG(DBG_MBOX , "out mutex\n");
	#endif

	return retp;
}


/*
*	DSP receive data from CPU
*	1. Get data from aipc_2dsp_mb_dequeue()
*	2. Get data in buffer
*	3. Return buffer to DSP through aipc_2dsp_bc_enqueue()
*	Usage
*	1. Use these APIs in kernel thread for receiving data
*/
void *	
aipc_2dsp_mb_dequeue( void )
{
	void *retp;

	#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;
	#endif

	#ifdef MBOX_LOCK
	AIPC_OSAL_INTERRUPT_FLAGS _old;
	AIPC_OSAL_INTERRUPT_DISABLE(_old);
	ADEBUG(DBG_MBOX , "disable interrupt\n");
	#endif
	
	#ifdef AIPC_BARRIER
		tmp_wr = ASTATS.aipc_data_2dsp_recv;
		tmp_wr++;
		AMB( ASTATS.aipc_data_2dsp_recv , tmp_wr );
	#else
		ASTATS.aipc_data_2dsp_recv++;
	#endif


	if(aipc_2dsp_mb_empty()){
		retp = NULL;  //mail box empty
		ADEBUG(DBG_MBOX , "dequeue NOK\n");
		}
	else{
		#ifdef AIPC_BARRIER
			tmp_wr = MB_2DSP.del;
			retp = MB_2DSP.mb[ tmp_wr ];
			tmp_wr = (tmp_wr+1) %  MB_2DSP_TOTAL;
			AMB( MB_2DSP.del , tmp_wr );
	
			#ifdef QUEUE_OPERATION_CNT
			tmp_wr = MB_2DSP.cnt_del;
			tmp_wr++;
			AMB( MB_2DSP.cnt_del , tmp_wr );
			#endif
		#else
			retp = MB_2DSP.mb[ MB_2DSP.del ];
			MB_2DSP.del = (MB_2DSP.del+1) % MB_2DSP_TOTAL;
	
			#ifdef QUEUE_OPERATION_CNT
			MB_2DSP.cnt_del++;
			#endif
		#endif
		
		ADEBUG(DBG_MBOX , "dequeue %p OK\n" , retp);
		}

	#ifdef MBOX_LOCK
	AIPC_OSAL_INTERRUPT_ENABLE(_old);
	ADEBUG(DBG_MBOX , "restore interrupt\n");
	#endif

	return retp;
}

int
aipc_2dsp_bc_enqueue( void *dp )
{
	int ret=NOK;
	
	#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;
	#endif

	#ifdef MBOX_LOCK
	AIPC_OSAL_INTERRUPT_FLAGS _old;
	AIPC_OSAL_INTERRUPT_DISABLE(_old);
	ADEBUG(DBG_MBOX , "disable interrupt\n");
	#endif

	#ifdef AIPC_BARRIER
		tmp_wr = ASTATS.aipc_data_2dsp_ret;
		tmp_wr++;
		AMB( ASTATS.aipc_data_2dsp_ret , tmp_wr );
	#else
		ASTATS.aipc_data_2dsp_ret++;
	#endif

	if(aipc_2dsp_bc_full()){
		ret=NOK;
		ADEBUG(DBG_MBOX , "enqueue NOK\n");
		}
	else{
		#ifdef AIPC_BARRIER
			tmp_wr = BC_2DSP.ins;
			AMB( BC_2DSP.bc[ tmp_wr ] , dp );
			
			tmp_wr = (tmp_wr+1) % BC_2DSP_TOTAL;
			AMB( BC_2DSP.ins , tmp_wr );
	
			#ifdef QUEUE_OPERATION_CNT
			tmp_wr = BC_2DSP.cnt_ins;
			tmp_wr++;
			AMB( BC_2DSP.cnt_ins , tmp_wr );
			#endif
		#else
			BC_2DSP.bc[ BC_2DSP.ins ] = dp;
			BC_2DSP.ins = (BC_2DSP.ins+1) % BC_2DSP_TOTAL;
	
			#ifdef QUEUE_OPERATION_CNT
			BC_2DSP.cnt_ins++;
			#endif
		#endif
		
		ret=OK;
		ADEBUG(DBG_MBOX , "enqueue %p OK\n" , dp);
		}

	#ifdef MBOX_LOCK
	AIPC_OSAL_INTERRUPT_ENABLE(_old);
	ADEBUG(DBG_MBOX , "restore interrupt\n");
	#endif
	
	return ret;
}

#endif 

/*
 *
 *	IPC test program
 *
 */

#ifdef CONFIG_RTL8686_IPC_TEST_THREAD
#ifdef __KERNEL__
struct mbox_priv{
	char	name [CPU_SEND_THREAD_NAME_SIZE];
	struct	mutex		mutex;
	void *  counter_ptr;
	u32_t   period;
};

#if 0
struct mbox_2dsp_send_priv{
	char	name [CPU_SEND_THREAD_NAME_SIZE];
	struct	mutex		mutex;
	void *  counter_ptr;
	u32_t   period;
};

struct mbox_2cpu_send_priv{
	char	name [DSP_SEND_THREAD_NAME_SIZE];
	struct	mutex		mutex;
	void *  counter_ptr;
	u32_t   period;
};

struct mbox_2dsp_poll_priv{
	char	name [DSP_RECV_THREAD_NAME_SIZE];
	struct	mutex		mutex;
	void *  counter_ptr;
	u32_t   period;
};
#endif

static int 
mbox_task_init(
	struct task_struct         * task_ptr,
	struct mbox_priv           * thread_priv,
	void * thread_fn,
	char * thread_name,
	void * counter_ptr , 
	u32_t  period
	)
{
	if (!thread_priv || !thread_fn || !thread_name || !counter_ptr){
		SDEBUG("Wrong thread init\n");
		return NOK;
	}

	memset(thread_priv , 0 , sizeof(struct mbox_priv));
	
	thread_priv->counter_ptr = counter_ptr;
	thread_priv->period      = period;

	strncpy(thread_priv->name, thread_name , sizeof(thread_priv->name));
	mutex_init(&thread_priv->mutex);

	task_ptr = kthread_run(
				thread_fn,
				thread_priv,
				thread_priv->name);

	if (!IS_ERR(task_ptr)){
		//printk("%s create successfully!\n" , thread_priv->name);
		return OK;
	}

	printk("%s create failed!\n" , thread_priv->name);
	return NOK;
}

int
aipc_dsp_task_init(
	struct task_struct         * task_ptr,
	void * thread_fn,
	char * thread_name)
{
	if (!thread_fn || !thread_name){
		SDEBUG("Wrong thread init\n");
		return NOK;
	}
	
	task_ptr = kthread_run(
				thread_fn,
				NULL,
				thread_name);

	if (!IS_ERR(task_ptr)){
		//printk("%s create successfully!\n" , thread_name);
		return OK;
	}

	printk("%s create failed!\n" , thread_name);
	return NOK;
}


#elif defined(__ECOS)
#if 0
struct aipc_dsp_send_priv{
	char	name [DSP_SEND_THREAD_NAME_SIZE];
	cyg_mutex_t mutex;
	void *  counter_ptr;
	u32_t   period;
};

struct aipc_dsp_poll_priv{
	char	name [DSP_RECV_THREAD_NAME_SIZE];
	cyg_mutex_t mutex;
	void *  counter_ptr;
	u32_t   period;
};
#endif
#ifdef CONFIG_RTL8686_READ_DRAM_THREAD
struct cpu_read_priv{
	char	name [32];
	struct	mutex		mutex;
	void *  counter_ptr;
	u32_t   period;
};
#endif
#endif
#endif 

#ifdef CONFIG_RTL8686_IPC_TEST_THREAD
#ifdef _AIPC_CPU_
#ifdef CONFIG_RTL8686_IPC_TEST_THREAD_2	
static struct task_struct *             mbox_2dsp_send_task_2=NULL;
static struct mbox_priv                 mbox_2dsp_send_dsc_2;
#endif
#ifdef CONFIG_RTL8686_IPC_TEST_THREAD_3	
static struct task_struct *             mbox_2dsp_send_task_3=NULL;
static struct mbox_priv                 mbox_2dsp_send_dsc_3;
#endif

static struct task_struct *             mbox_2dsp_send_task=NULL;
static struct mbox_priv                 mbox_2dsp_send_dsc;

static int 
mbox_2dsp_send_thread(void *p)
{
	struct mbox_priv * tp = (struct mbox_priv *)p;

	void *mp;  

    set_current_state(TASK_INTERRUPTIBLE);

	do {                    
		msleep_interruptible(tp->period);

		if(ATHREAD.mbox_2dsp_send & MBOX_2DSP_SEND_START){
			
			ADEBUG(DBG_MBOX , "\n");
	
			mp = aipc_data_2dsp_alloc();
			
			if(mp){
				memset(mp , 0 , MAIL_2DSP_SIZE);

				snprintf(mp , MAIL_2DSP_SIZE , "%s counter=%u\n" , tp->name , (*(u32_t*)(tp->counter_ptr))++);

				ADEBUG(DBG_CONT , "%s\n" , (char*)mp);
				
				aipc_data_2dsp_send( mp );
				}
			else{
				ADEBUG(DBG_CONT , "BC_2DSP no buffers\n");
				}
			}
	} while (!kthread_should_stop());

	return OK;
}

#if 0
static int 
mbox_2dsp_send_task_init(
	struct task_struct         * task_ptr,
	struct mbox_2dsp_send_priv * thread_priv,
	void * thread_fn,
	char * thread_name,
	void * counter_ptr , 
	u32_t  period
	)
{
	if (!thread_priv || !thread_fn || !thread_name || !counter_ptr || !period){
		printk("Wrong thread init\n");
		SDEBUG("\n");
		return NOK;
	}

	memset(thread_priv , 0 , sizeof(struct mbox_2dsp_send_priv));
	
	thread_priv->counter_ptr = counter_ptr;
	thread_priv->period      = period;

	strncpy(thread_priv->name, thread_name , sizeof(thread_priv->name));
	mutex_init(&thread_priv->mutex);

	task_ptr = kthread_run(
				thread_fn,
				thread_priv,
				thread_priv->name);

	if (!IS_ERR(task_ptr)){
		//printk("%s create successfully!\n" , thread_priv->name);
		return OK;
	}

	printk("%s create failed!\n" , thread_priv->name);
	return NOK;
}
#endif


static int 
mbox_2dsp_task_start(void)
{
	mbox_task_init(
		mbox_2dsp_send_task,
		&mbox_2dsp_send_dsc,
		mbox_2dsp_send_thread,
		CPU_SEND_THREAD_NAME,
		&(ATHREAD.mbox_2dsp_counter),
		CPU_SEND_PERIOD);

#ifdef CONFIG_RTL8686_IPC_TEST_THREAD_2	
	mbox_task_init(
		mbox_2dsp_send_task_2,
		&mbox_2dsp_send_dsc_2,
		mbox_2dsp_send_thread,
		CPU_SEND_THREAD_NAME_2,
		&(ATHREAD.mbox_2dsp_counter_2),
		CPU_SEND_PERIOD_2);
#endif

#ifdef CONFIG_RTL8686_IPC_TEST_THREAD_3	
	mbox_task_init(
		mbox_2dsp_send_task_3,
		&mbox_2dsp_send_dsc_3,
		mbox_2dsp_send_thread,
		CPU_SEND_THREAD_NAME_3,
		&(ATHREAD.mbox_2dsp_counter_3),
		CPU_SEND_PERIOD_3);
#endif
	return OK;
}

int mbox_2dsp_send_task_exit(struct task_struct * task_ptr)
{
	if(task_ptr)
		kthread_stop(task_ptr);
	
	return OK;
}


int mbox_2dsp_task_init(void)
{
	mbox_2dsp_task_start();

	return OK;
}

#else  // _AIPC_DSP_

#ifdef __KERNEL__

// CPU->DSP Send Thread
#ifdef CONFIG_RTL8686_IPC_TEST_THREAD_2
static struct task_struct *             mbox_2cpu_send_task_2=NULL;
static struct mbox_priv                 mbox_2cpu_send_dsc_2;
#endif
#ifdef CONFIG_RTL8686_IPC_TEST_THREAD_3
static struct task_struct *             mbox_2cpu_send_task_3=NULL;
static struct mbox_priv                 mbox_2cpu_send_dsc_3;
#endif

static struct task_struct *             mbox_2cpu_send_task=NULL;
static struct mbox_priv                 mbox_2cpu_send_dsc;

static int
mbox_2cpu_send_thread(void *p)
{
	struct mbox_priv * tp = (struct mbox_priv *)p;

    void  *mp;

    set_current_state(TASK_INTERRUPTIBLE);

    do {
        msleep_interruptible(tp->period);
        
        if(ATHREAD.mbox_2cpu_send & MBOX_2CPU_SEND_START){
            ADEBUG(DBG_MBOX , "\n");

            mp = aipc_data_2cpu_alloc();

            if( mp!=NULL ){
                memset(mp , 0 , MAIL_2CPU_SIZE);

                snprintf(mp , MAIL_2CPU_SIZE , "%s counter=%u\n" , tp->name , (*(u32_t*)(tp->counter_ptr))++);

                ADEBUG(DBG_CONT , "%s\n" , (char*)mp);

                aipc_data_2cpu_send( mp );
            }
            else{
                ADEBUG(DBG_CONT , "BC_2CPU no buffers\n");
            }
        }
    } while (!kthread_should_stop());

    return OK;
}


static int
mbox_2cpu_send_task_init_2(void)
{
    mbox_task_init(
        mbox_2cpu_send_task_2,
        &mbox_2cpu_send_dsc_2,
        mbox_2cpu_send_thread,
        CPU_SEND_THREAD_NAME_2,
        &(ATHREAD.mbox_2cpu_counter_2),
        DSP_SEND_PERIOD_2);
        
    return OK;
}

static int
mbox_2cpu_send_task_init_3(void)
{
    mbox_task_init(
        mbox_2cpu_send_task_3,
        &mbox_2cpu_send_dsc_3,
        mbox_2cpu_send_thread,
        CPU_SEND_THREAD_NAME_3,
        &(ATHREAD.mbox_2cpu_counter_3),
        DSP_SEND_PERIOD_3);

    return OK;
}



static int
mbox_2cpu_send_task_init(void)
{
    mbox_task_init(
        mbox_2cpu_send_task,
        &mbox_2cpu_send_dsc,
        mbox_2cpu_send_thread,
        CPU_SEND_THREAD_NAME,
        &(ATHREAD.mbox_2cpu_counter),
        DSP_SEND_PERIOD);

    return OK;
}


// CPU->DSP Poll Thread
static struct task_struct *             mbox_2dsp_poll_task=NULL;
static struct mbox_priv                 mbox_2dsp_poll_dsc;


static int
mbox_2dsp_poll_thread(void *p)
{
	struct mbox_priv * tp = (struct mbox_priv *)p;
	
    void *mp = NULL;

    set_current_state(TASK_INTERRUPTIBLE);

    do {
        msleep_interruptible(tp->period);

        if(ATHREAD.mbox_2dsp_poll & MBOX_2DSP_POLL_START){
            ADEBUG(DBG_MBOX , "\n");

            mp = aipc_data_2dsp_recv();

            if( mp!=NULL ){
                ADEBUG(DBG_CONT , "%s\n" , (char *)mp);
                aipc_data_2dsp_ret( mp );
            }
            else{
                ADEBUG(DBG_CONT , "BC 2DSP is not run out\n");
            }
        }
    } while (!kthread_should_stop());

    return OK;
}

static int
mbox_2dsp_poll_task_init(void)
{
    mbox_task_init(
        mbox_2dsp_poll_task,
        &mbox_2dsp_poll_dsc,
        mbox_2dsp_poll_thread,
        DSP_POLL_THREAD_NAME,
        &(ATHREAD.mbox_2dsp_counter),
        DSP_POLL_PERIOD);

    return OK;
}



#elif defined(__ECOS)

#ifdef CONFIG_RTL8686_IPC_TEST_THREAD_2

static int	mbox_2cpu_send_task_init_2(void);

#define MBOX_2CPU_SEND_THREAD_PRIORITY_2 	29
#define MBOX_2CPU_SEND_THREAD_STACK_SIZE_2	2048 

static int mbox_2cpu_send_thread_stack_2[ MBOX_2CPU_SEND_THREAD_STACK_SIZE_2 ];
static cyg_handle_t mbox_2cpu_send_thread_handle_2;
static cyg_thread   mbox_2cpu_send_thread_obj_2;

static void 
mbox_2cpu_send_thread_2(cyg_addrword_t index)
{
	void  *mp;
	
	while(1)
	{
		cyg_thread_delay( DSP_SEND_PERIOD_2 );
		
		if(ATHREAD.mbox_2cpu_send & MBOX_2CPU_SEND_START){
			ADEBUG(DBG_MBOX , "\n");

			mp = aipc_data_2cpu_alloc();
			
			if( mp!=NULL ){
				memset(mp , 0 , MAIL_2CPU_SIZE);

				snprintf(mp , MAIL_2CPU_SIZE , 
					"%s mbox_2cpu_counter_2=%u\n" , "Data CPU<-DSP Thread 2" , (ATHREAD.mbox_2cpu_counter_2)++);

				ADEBUG(DBG_CONT , "%s\n" , (char*)mp);

				aipc_data_2cpu_send( mp );
			}
			else{
				ADEBUG(DBG_CONT , "BC_2CPU no buffers\n");
			}
		}
	}
}

static int 
mbox_2cpu_send_task_init_2(void)
{
	cyg_thread_create(
		MBOX_2CPU_SEND_THREAD_PRIORITY_2,
		mbox_2cpu_send_thread_2,
		0,
		DSP_SEND_THREAD_NAME_2,
		&mbox_2cpu_send_thread_stack_2,
		MBOX_2CPU_SEND_THREAD_STACK_SIZE_2,
		&mbox_2cpu_send_thread_handle_2,
		&mbox_2cpu_send_thread_obj_2 );

		// Let the thread run when the scheduler starts.
		cyg_thread_resume( mbox_2cpu_send_thread_handle_2 );

	return OK;
}
#endif //CONFIG_RTL8686_IPC_TEST_THREAD_2

#ifdef CONFIG_RTL8686_IPC_TEST_THREAD_3
static int	mbox_2cpu_send_task_init_3(void);

#define MBOX_2CPU_SEND_THREAD_PRIORITY_3 	29
#define MBOX_2CPU_SEND_THREAD_STACK_SIZE_3	2048 

static int mbox_2cpu_send_thread_stack_3[ MBOX_2CPU_SEND_THREAD_STACK_SIZE_3 ];
static cyg_handle_t mbox_2cpu_send_thread_handle_3;
static cyg_thread   mbox_2cpu_send_thread_obj_3;

static void 
mbox_2cpu_send_thread_3(cyg_addrword_t index)
{
	void  *mp;
	
	while(1)
	{
		cyg_thread_delay( DSP_SEND_PERIOD_3 );
		

		if(ATHREAD.mbox_2cpu_send & MBOX_2CPU_SEND_START){
			ADEBUG(DBG_MBOX , "\n");

			mp = aipc_data_2cpu_alloc();
			
			if( mp!=NULL ){
				memset(mp , 0 , MAIL_2CPU_SIZE);

				snprintf(mp , MAIL_2CPU_SIZE , 
					"%s mbox_2cpu_counter_3=%u\n" , "Data CPU<-DSP Thread 2" , (ATHREAD.mbox_2cpu_counter_3)++);

				ADEBUG(DBG_CONT , "%s\n" , (char*)mp);

				aipc_data_2cpu_send( mp );
			}
			else{
				ADEBUG(DBG_CONT , "BC_2CPU no buffers\n");
			}
		}
	}
}

static int 
mbox_2cpu_send_task_init_3(void)
{
	cyg_thread_create(
		MBOX_2CPU_SEND_THREAD_PRIORITY_3,
		mbox_2cpu_send_thread_3,
		0,
		DSP_SEND_THREAD_NAME_3,
		&mbox_2cpu_send_thread_stack_3,
		MBOX_2CPU_SEND_THREAD_STACK_SIZE_3,
		&mbox_2cpu_send_thread_handle_3,
		&mbox_2cpu_send_thread_obj_3 );

		// Let the thread run when the scheduler starts.
		cyg_thread_resume( mbox_2cpu_send_thread_handle_3 );

	return OK;
}
#endif  //CONFIG_RTL8686_IPC_TEST_THREAD_3


#define MBOX_2CPU_SEND_THREAD_PRIORITY 		29
#define MBOX_2CPU_SEND_THREAD_STACK_SIZE	2048 

static int mbox_2cpu_send_thread_stack[ MBOX_2CPU_SEND_THREAD_STACK_SIZE ];
static cyg_handle_t mbox_2cpu_send_thread_handle;
static cyg_thread   mbox_2cpu_send_thread_obj;

static void 
mbox_2cpu_send_thread(cyg_addrword_t index)
{
	void  *mp;
	
	while(1)
	{
		cyg_thread_delay( DSP_SEND_PERIOD );
		
		if(ATHREAD.mbox_2cpu_send & MBOX_2CPU_SEND_START){
			ADEBUG(DBG_MBOX , "\n");

			mp = aipc_data_2cpu_alloc();
			
			if( mp!=NULL ){
				memset(mp , 0 , MAIL_2CPU_SIZE);

				snprintf(mp , MAIL_2CPU_SIZE , "%s mbox_2cpu_counter=%u\n" , 	"Data CPU<-DSP Thread 1" , (ATHREAD.mbox_2cpu_counter)++);

				ADEBUG(DBG_CONT , "%s\n" , (char*)mp);

				aipc_data_2cpu_send( mp );
			}
			else{
				ADEBUG(DBG_CONT , "BC_2CPU no buffers\n");
			}
		}
	}
}

static int 
mbox_2cpu_send_task_init(void)
{
	cyg_thread_create(
		MBOX_2CPU_SEND_THREAD_PRIORITY,
		mbox_2cpu_send_thread,
		0,
		DSP_SEND_THREAD_NAME,
		&mbox_2cpu_send_thread_stack,
		MBOX_2CPU_SEND_THREAD_STACK_SIZE,
		&mbox_2cpu_send_thread_handle,
		&mbox_2cpu_send_thread_obj );

		// Let the thread run when the scheduler starts.
		cyg_thread_resume( mbox_2cpu_send_thread_handle );

	return OK;
}

int 
mbox_2cpu_send_task_exit(cyg_handle_t handle)
{
	cyg_thread_kill(handle);
	cyg_thread_delete(handle);
	
	return OK;
}


#define MBOX_2DSP_POLL_THREAD_PRIORITY  	29 
#define MBOX_2DSP_POLL_THREAD_STACK_SIZE 	2048 

static int mbox_2dsp_poll_thread_stack[ MBOX_2DSP_POLL_THREAD_STACK_SIZE ];
static cyg_handle_t mbox_2dsp_poll_thread_handle;
static cyg_thread   mbox_2dsp_poll_thread_obj;

static void 
mbox_2dsp_poll_thread(cyg_addrword_t index)
{
	void *mp = NULL;
	
	while(1)
	{
		cyg_thread_delay( DSP_POLL_PERIOD );
		if(ATHREAD.mbox_2dsp_poll & MBOX_2DSP_POLL_START){
			ADEBUG(DBG_MBOX , "\n");
			
			mp = aipc_data_2dsp_recv();
		
			if( mp!=NULL ){
				ADEBUG(DBG_CONT , "%s\n" , (char *)mp);
				aipc_data_2dsp_ret( mp );
			}	
			else{
				ADEBUG(DBG_CONT , "BC 2DSP is not run out\n");
			}
		}
	}
}

static int 
mbox_2dsp_poll_task_init(void)
{
	cyg_thread_create(
		MBOX_2DSP_POLL_THREAD_PRIORITY,
		mbox_2dsp_poll_thread,
		0,
		DSP_POLL_THREAD_NAME,
		&mbox_2dsp_poll_thread_stack,
		MBOX_2DSP_POLL_THREAD_STACK_SIZE,
		&mbox_2dsp_poll_thread_handle,
		&mbox_2dsp_poll_thread_obj );

		// Let the thread run when the scheduler starts.
		cyg_thread_resume( mbox_2dsp_poll_thread_handle );

	return OK;
}


int 
mbox_2dsp_poll_task_exit(cyg_handle_t handle)
{
	cyg_thread_kill(handle);
	cyg_thread_delete(handle);
	return OK;	
}

#endif  //__ECOS


// Common code
static int 
mbox_2cpu_task_start(void)
{
	//CPU->DSP
#ifndef	INT_NOTIFY_DATA_2DSP
	mbox_2dsp_poll_task_init();
#endif
	
	//CPU<-DSP
	mbox_2cpu_send_task_init();
#ifdef CONFIG_RTL8686_IPC_TEST_THREAD_2
	mbox_2cpu_send_task_init_2();
#endif
#ifdef CONFIG_RTL8686_IPC_TEST_THREAD_3
	mbox_2cpu_send_task_init_3();
#endif
	return OK;
}

int 
mbox_2cpu_task_init(void)
{
	mbox_2cpu_task_start();
	return OK;
}

#ifdef __KERNEL__
static struct task_struct *             startup_thread_task=NULL;

static int
startup_thread(void *p)
{
	extern int dsp_ctrl_task_init(void);

	static int init_done  = 0;
	printk( "enter startup_thread\n" );
	
	set_current_state(TASK_INTERRUPTIBLE);

	do { 
		msleep_interruptible(10);

		if(!init_done){
			if((ACTSW.init & INIT_OK)){				
				/*
				*	DSP IPC Init function. Need to be called in DSP init procedure.
				*/
				//aipc_dsp_ipc_init();	//Move to cyg_user_start();

				dsp_ctrl_task_init();
				mbox_2cpu_task_init();
				init_done = 1;
				}
			}
	} while (!kthread_should_stop());
	
	printk( "exit startup_thread\n" );
	
	return OK;
}

void aipc_dsp_test_thread_entry(void)
{
	aipc_dsp_task_init( 
		startup_thread_task , 
		startup_thread,
		"StartupThread"
		);
}

#elif defined(__ECOS)

void startup_thread(cyg_addrword_t data_nt)
{
	extern int dsp_ctrl_task_init(void);

	static int init_done  = 0;
	printk( "enter startup_thread\n" );
	
	while(1){
		//cyg_thread_delay(DSP_CHECK_START_PERIOD);
		cyg_thread_delay(10);
		//SDEBUG("ACTSW.init=%x addr=%p \n" , ACTSW.init , &(ACTSW));

		if(!init_done){
			if((ACTSW.init & INIT_OK)){				
				/*
				*	DSP IPC Init function. Need to be called in DSP init procedure.
				*/
				//aipc_dsp_ipc_init();	//Move to cyg_user_start();

				dsp_ctrl_task_init();
				mbox_2cpu_task_init();
				init_done = 1;
				}
			}
		}
	
	printk( "exit startup_thread\n" );
}
#endif  // __ECOS
#endif  // _AIPC_DSP_
#endif  // CONFIG_RTL8686_IPC_TEST_THREAD





#ifdef CONFIG_RTL8686_READ_DRAM_THREAD

#ifdef _AIPC_CPU_
static struct task_struct *             cpu_read_task=NULL;
static struct cpu_read_priv             cpu_read_dsc;

static struct task_struct *             cpu_read_task_1=NULL;
static struct cpu_read_priv             cpu_read_dsc_1;

static struct task_struct *             cpu_read_task_2=NULL;
static struct cpu_read_priv             cpu_read_dsc_2;

static aipc_sram_t cpu_sram;
static aipc_dram_t cpu_dram;


static int 
cpu_read_thread(void *p)
{
	struct cpu_read_priv * tp = (struct cpu_read_priv *)p;

#if 0
	static u32_t cpu_read = 0;
	static u32_t all_read = 0;
#endif
	int i;

    set_current_state(TASK_INTERRUPTIBLE);

	do {
		msleep_interruptible(1);
#if 1
		for(i=0 ; i<99999 ; i++){
			memcpy(&cpu_sram , ASRAM,  sizeof(aipc_sram_t));
			memcpy(&cpu_dram , ADRAM,  sizeof(aipc_dram_t));
			
			memcpy(ASRAM  , &cpu_sram , sizeof(aipc_sram_t));
			memcpy(ADRAM  , &cpu_dram , sizeof(aipc_dram_t));
		}
#else
		for(i=0 ; i<99999 ; i++){
			cpu_read = ATHREAD.cpu_read_cnt++;
			all_read = ATHREAD.all_read_cnt++;
		}
#endif

	} while (!kthread_should_stop());

	return OK;
}



static int 
cpu_read_task_init(
	struct task_struct         * task_ptr,
	struct cpu_read_priv       * thread_priv,
	void * thread_fn,
	char * thread_name,
	void * counter_ptr , 
	u32_t  period
	)
{
	if (!thread_priv || !thread_fn || !thread_name){
		printk("Wrong thread init\n");
		SDEBUG("\n");
		return NOK;
	}

	memset(thread_priv , 0 , sizeof(struct cpu_read_priv));
	
	thread_priv->period      = period;

	strncpy(thread_priv->name, thread_name , sizeof(thread_priv->name));
	mutex_init(&thread_priv->mutex);

	task_ptr = kthread_run(
				thread_fn,
				thread_priv,
				thread_priv->name);

	if (!IS_ERR(task_ptr)){
		printk("%s create successfully!\n" , thread_priv->name);
		return OK;
	}

	printk("%s create failed!\n" , thread_priv->name);
	return NOK;
}


int
cpu_read_task_start(void)
{
	cpu_read_task_init(
		cpu_read_task,
		&cpu_read_dsc, 
		cpu_read_thread,
		"CpuReadD0",
		NULL,
		0);

	cpu_read_task_init(
		cpu_read_task_1,
		&cpu_read_dsc_1, 
		cpu_read_thread,
		"CpuReadD1",
		NULL,
		0);

	cpu_read_task_init(
		cpu_read_task_2,
		&cpu_read_dsc_2, 
		cpu_read_thread,
		"CpuReadD2",
		NULL,
		0);

	return OK;
}

#else

/*
*	DSP read thread 0
*/
#define DSP_READ_THREAD_PRIORITY  	29 
#define DSP_READ_THREAD_STACK_SIZE 	2048 

static int dsp_read_thread_stack[ DSP_READ_THREAD_STACK_SIZE ];
static cyg_handle_t dsp_read_thread_handle;
static cyg_thread   dsp_read_thread_obj;

static aipc_sram_t dsp_sram;
static aipc_dram_t dsp_dram;


static void 
dsp_read_thread(cyg_addrword_t index)
{
#if 0
	static u32_t dsp_read = 0;
	static u32_t all_read = 0;
#endif	
	int i = 0;

	while(1)
	{
		cyg_thread_yield();
#if 1
		for(i=0 ; i<99999 ; i++){
			memcpy(&dsp_sram , ASRAM,  sizeof(aipc_sram_t));
			memcpy(&dsp_dram , ADRAM,  sizeof(aipc_dram_t));
			
			memcpy(ASRAM  , &dsp_sram , sizeof(aipc_sram_t));
			memcpy(ADRAM  , &dsp_dram , sizeof(aipc_dram_t));
		}

#else
		dsp_read = ATHREAD.dsp_read_cnt++;
		all_read = ATHREAD.all_read_cnt++;
#endif
	}
}

static int 
dsp_read_task_init(void)
{
	cyg_thread_create(
		DSP_READ_THREAD_PRIORITY,
		dsp_read_thread,
		0,
		"DspReadD0",
		&dsp_read_thread_stack,
		DSP_READ_THREAD_STACK_SIZE,
		&dsp_read_thread_handle,
		&dsp_read_thread_obj );

		// Let the thread run when the scheduler starts.
		cyg_thread_resume( dsp_read_thread_handle );

	return OK;
}

/*
*	DSP read thread 1
*/
static int dsp_read_thread_stack_1[ DSP_READ_THREAD_STACK_SIZE ];
static cyg_handle_t dsp_read_thread_handle_1;
static cyg_thread   dsp_read_thread_obj_1;

static int 
dsp_read_task_init_1(void)
{
	cyg_thread_create(
		DSP_READ_THREAD_PRIORITY,
		dsp_read_thread,
		0,
		"DspReadD1",
		&dsp_read_thread_stack_1,
		DSP_READ_THREAD_STACK_SIZE,
		&dsp_read_thread_handle_1,
		&dsp_read_thread_obj_1 );

		// Let the thread run when the scheduler starts.
		cyg_thread_resume( dsp_read_thread_handle_1 );

	return OK;
}

/*
*	DSP read thread 2
*/
static int dsp_read_thread_stack_2[ DSP_READ_THREAD_STACK_SIZE ];
static cyg_handle_t dsp_read_thread_handle_2;
static cyg_thread   dsp_read_thread_obj_2;

static int 
dsp_read_task_init_2(void)
{
	cyg_thread_create(
		DSP_READ_THREAD_PRIORITY,
		dsp_read_thread,
		0,
		"DspReadD2",
		&dsp_read_thread_stack_2,
		DSP_READ_THREAD_STACK_SIZE,
		&dsp_read_thread_handle_2,
		&dsp_read_thread_obj_2 );

		// Let the thread run when the scheduler starts.
		cyg_thread_resume( dsp_read_thread_handle_2);

	return OK;
}

static int 
dsp_read_task_start(void)
{
	dsp_read_task_init();

	dsp_read_task_init_1();

	dsp_read_task_init_2();
}

void dsp_read_startup_thread(cyg_addrword_t data_nt)
{
	static int init_done  = 0;
	printf( "enter startup_thread\n" );
	
	while(1){
		cyg_thread_delay(10);

		if(!init_done){
			if((ACTSW.init & INIT_OK)){

				dsp_read_task_start();

				init_done = 1;
				}
			}
		}
	
	printf( "exit startup_thread\n" );
}


#endif
#endif //CONFIG_RTL8686_READ_DRAM_THREAD

