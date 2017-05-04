#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#elif defined(__ECOS)

#else
#error "need to define"
#endif

#include "aipc_global.h"
#include "aipc_shm.h"  
#include "aipc_debug.h"
#include "aipc_swp.h"
#include "aipc_osal.h"  
#include "aipc_dsl_eoc.h"

#ifdef _AIPC_CPU_		//API functions

int   aipc_dsl_eoc_set( aipc_dsl_eoc_set_buf_t  *set_buf );
int   aipc_dsl_eoc_get( aipc_dsl_eoc_get_buf_t  *get_buf );
EXPORT_SYMBOL( aipc_dsl_eoc_set );
EXPORT_SYMBOL( aipc_dsl_eoc_get );

#else

void *aipc_dsl_eoc_recv_set(void);
void *aipc_dsl_eoc_recv_get(void);
void  aipc_dsl_eoc_ret_set(void) ;
void  aipc_dsl_eoc_ret_get(void) ;
EXPORT_SYMBOL( aipc_dsl_eoc_recv_set );
EXPORT_SYMBOL( aipc_dsl_eoc_recv_get );
EXPORT_SYMBOL( aipc_dsl_eoc_ret_set  );
EXPORT_SYMBOL( aipc_dsl_eoc_ret_get  );

#endif

#if defined(_AIPC_CPU_)
int aipc_dsl_eoc_set( aipc_dsl_eoc_set_buf_t  *set_buf )
{
	int call_cnt = 0;
	#ifdef AIPC_BARRIER
		volatile unsigned int tmp_wr=0;
		tmp_wr = ASTATS.aipc_dsl_eoc_set;
		tmp_wr++;
		AMB( ASTATS.aipc_dsl_eoc_set , tmp_wr );
	#else
		ASTATS.aipc_dsl_eoc_set++;
	#endif 
	
	while( DSL_EOC_SHM_SET.own != AIPC_DSL_OWN_CPU ){
		AIPC_OSAL_SCHEDULE_TIMEOUT_INTERRUPTIBLE(1);
		if( (call_cnt>100) && printk_ratelimit() ){
			SDEBUG("long loop\n");
			call_cnt = 0;
		}
		else
			call_cnt++;
	}

	memcpy( &DSL_EOC_SHM_SET.data , set_buf , sizeof( aipc_dsl_eoc_set_buf_t ) );	
	
	#ifdef AIPC_BARRIER
	AMB( DSL_EOC_SHM_SET.dummy , 0 );
	#endif
	
	#ifdef AIPC_BARRIER
		AMB( DSL_EOC_SHM_SET.own , AIPC_DSL_OWN_DSP );
	#else
		DSL_EOC_SHM_SET.own = AIPC_DSL_OWN_DSP;
	#endif
	
	return OK;		
}

int aipc_dsl_eoc_get( aipc_dsl_eoc_get_buf_t *get_buf )
{
	int call_cnt = 0;

	#ifdef AIPC_BARRIER
		volatile unsigned int tmp_wr=0;
		tmp_wr = ASTATS.aipc_dsl_eoc_get;
		tmp_wr++;
		AMB( ASTATS.aipc_dsl_eoc_get , tmp_wr );
	#else
		ASTATS.aipc_dsl_eoc_get++;
	#endif 
	
	while( DSL_EOC_SHM_GET.own != AIPC_DSL_OWN_CPU ){
		AIPC_OSAL_SCHEDULE_TIMEOUT_INTERRUPTIBLE(1);

		if( (call_cnt>100) && printk_ratelimit() ){
			SDEBUG("long loop\n");
			call_cnt = 0;
		}
		else
			call_cnt++;
	}
	
	memcpy( &DSL_EOC_SHM_GET.data , get_buf , sizeof( aipc_dsl_eoc_get_buf_t ) );
	
	#ifdef AIPC_BARRIER
	AMB( DSL_EOC_SHM_GET.dummy , 0 );
	#endif
	
	#ifdef AIPC_BARRIER
		AMB( DSL_EOC_SHM_GET.own , AIPC_DSL_OWN_DSP );
	#else
		DSL_EOC_SHM.own = AIPC_DSL_OWN_DSP;
	#endif
	
	while( DSL_EOC_SHM_GET.own != AIPC_DSL_OWN_CPU ){
		AIPC_OSAL_SCHEDULE_TIMEOUT_INTERRUPTIBLE(1);

		if( (call_cnt>100) && printk_ratelimit() ){
			SDEBUG("long loop\n");
			call_cnt = 0;
		}
		else
			call_cnt++;
	}
	memcpy( get_buf , &DSL_EOC_SHM_GET.data , sizeof( aipc_dsl_eoc_get_buf_t ) );
	
	return OK;		
}

int
aipc_cpu_dsl_eoc_init(void)
{
	DSL_EOC_SHM_SET.own = AIPC_DSL_OWN_CPU;
	memset( &DSL_EOC_SHM_SET.data , 0 , sizeof( aipc_dsl_eoc_set_buf_t ) );
	
	DSL_EOC_SHM_GET.own = AIPC_DSL_OWN_CPU;
	memset( &DSL_EOC_SHM_GET.data , 0 , sizeof( aipc_dsl_eoc_get_buf_t ) );
	return 0;
}

#elif defined(_AIPC_DSP_)

void *aipc_dsl_eoc_recv_set(void)
{	
	#ifdef AIPC_BARRIER
		volatile unsigned int tmp_wr=0;
		tmp_wr = ASTATS.aipc_dsl_eoc_recv_set;
		tmp_wr++;
		AMB( ASTATS.aipc_dsl_eoc_recv_set , tmp_wr );
	#else
		ASTATS.aipc_dsl_eoc_recv_set++;
	#endif 
	
	if( DSL_EOC_SHM_SET.own == AIPC_DSL_OWN_DSP )
		return &DSL_EOC_SHM_SET.data;
	else
		return NULL;	
}

void aipc_dsl_eoc_ret_set(void)
{
	#ifdef AIPC_BARRIER
		volatile unsigned int tmp_wr=0;
		tmp_wr = ASTATS.aipc_dsl_eoc_ret_set;
		tmp_wr++;
		AMB( ASTATS.aipc_dsl_eoc_ret_set , tmp_wr );

		AMB( DSL_EOC_SHM_SET.own , AIPC_DSL_OWN_CPU );
	#else
		ASTATS.aipc_dsl_eoc_ret_set++;

		DSL_EOC_SHM_SET.own = AIPC_DSL_OWN_CPU;
	#endif 
}

void *aipc_dsl_eoc_recv_get(void)
{
	#ifdef AIPC_BARRIER
		volatile unsigned int tmp_wr=0;
		tmp_wr = ASTATS.aipc_dsl_eoc_recv_get;
		tmp_wr++;
		AMB( ASTATS.aipc_dsl_eoc_recv_get , tmp_wr );
	#else
		ASTATS.aipc_dsl_eoc_recv_get++;
	#endif 
	
	if( DSL_EOC_SHM_GET.own == AIPC_DSL_OWN_DSP )
		return &DSL_EOC_SHM_GET.data;
	else
		return NULL;	
}

void aipc_dsl_eoc_ret_get(void)
{
	#ifdef AIPC_BARRIER
		volatile unsigned int tmp_wr=0;
		tmp_wr = ASTATS.aipc_dsl_eoc_ret_get;
		tmp_wr++;
		AMB( ASTATS.aipc_dsl_eoc_ret_get , tmp_wr );

		AMB( DSL_EOC_SHM_GET.own , AIPC_DSL_OWN_CPU );
	#else
		ASTATS.aipc_dsl_eoc_ret_get++;

		DSL_EOC_SHM_GET.own = AIPC_DSL_OWN_CPU;
	#endif 
}

#else
#error "need to check CPU/DSP"
#endif

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int voip_host_cch_invert_table_read_proc(struct seq_file *f)
{
	int n = 0;
	
	seq_printf( f, "EOC:\n");
	seq_printf( f, "\tProperty Set:\n");

	if( DSL_EOC_SHM_SET.own == AIPC_DSL_OWN_CPU )
		seq_printf( f, "\t own: CPU\n");
	else if ( DSL_EOC_SHM_SET.own == AIPC_DSL_OWN_DSP )
		seq_printf( f, "\t own: DSP\n");
	else
		seq_printf( f, "\t own: Unknown\n");

	seq_printf( f, "\tProperty Get:\n");

	if( DSL_EOC_SHM_GET.own == AIPC_DSL_OWN_CPU )
		seq_printf( f, "\t own: CPU\n");
	else if ( DSL_EOC_SHM_GET.own == AIPC_DSL_OWN_DSP )
		seq_printf( f, "\t own: DSP\n");
	else
		seq_printf( f, "\t own: Unknown\n");

	seq_printf( f, "\tAPI:\n");
	seq_printf( f, "\t aipc_dsl_eoc_set      = %u\n" , ASTATS.aipc_dsl_eoc_set     );
	seq_printf( f, "\t aipc_dsl_eoc_get      = %u\n" , ASTATS.aipc_dsl_eoc_get     );
	seq_printf( f, "\t aipc_dsl_eoc_recv_set = %u\n" , ASTATS.aipc_dsl_eoc_recv_set);
	seq_printf( f, "\t aipc_dsl_eoc_recv_get = %u\n" , ASTATS.aipc_dsl_eoc_recv_get);
	seq_printf( f, "\t aipc_dsl_eoc_ret_set  = %u\n" , ASTATS.aipc_dsl_eoc_ret_set );
	seq_printf( f, "\t aipc_dsl_eoc_ret_get  = %u\n" , ASTATS.aipc_dsl_eoc_ret_get );
	
	return n;
}
#else
int aipc_dsl_proc_eoc_dump (char *buf)
{
	int n = 0;
	
	n += sprintf(buf+n , "EOC:\n");
	n += sprintf(buf+n , "\tProperty Set:\n");

	if( DSL_EOC_SHM_SET.own == AIPC_DSL_OWN_CPU )
		n += sprintf(buf+n , "\t own: CPU\n");
	else if ( DSL_EOC_SHM_SET.own == AIPC_DSL_OWN_DSP )
		n += sprintf(buf+n , "\t own: DSP\n");
	else
		n += sprintf(buf+n , "\t own: Unknown\n");

	n += sprintf(buf+n , "\tProperty Get:\n");

	if( DSL_EOC_SHM_GET.own == AIPC_DSL_OWN_CPU )
		n += sprintf(buf+n , "\t own: CPU\n");
	else if ( DSL_EOC_SHM_GET.own == AIPC_DSL_OWN_DSP )
		n += sprintf(buf+n , "\t own: DSP\n");
	else
		n += sprintf(buf+n , "\t own: Unknown\n");

	n += sprintf(buf+n , "\tAPI:\n");
	n += sprintf(buf+n , "\t aipc_dsl_eoc_set      = %u\n" , ASTATS.aipc_dsl_eoc_set     );
	n += sprintf(buf+n , "\t aipc_dsl_eoc_get      = %u\n" , ASTATS.aipc_dsl_eoc_get     );
	n += sprintf(buf+n , "\t aipc_dsl_eoc_recv_set = %u\n" , ASTATS.aipc_dsl_eoc_recv_set);
	n += sprintf(buf+n , "\t aipc_dsl_eoc_recv_get = %u\n" , ASTATS.aipc_dsl_eoc_recv_get);
	n += sprintf(buf+n , "\t aipc_dsl_eoc_ret_set  = %u\n" , ASTATS.aipc_dsl_eoc_ret_set );
	n += sprintf(buf+n , "\t aipc_dsl_eoc_ret_get  = %u\n" , ASTATS.aipc_dsl_eoc_ret_get );
	
	return n;
}
#endif

#ifdef CONFIG_RTL8686_IPC_DSL_IPC_TEST

#define CTRL_NAME_SIZE		32

//CPU
#define CPU_DSL_EOC_SET_NAME		"CpuEocSetD"
#define CPU_DSL_EOC_SET_PERIOD	    ((1)*(MUL))

#define CPU_DSL_EOC_GET_NAME		"CpuEocGetD"
#define CPU_DSL_EOC_GET_PERIOD	    ((1)*(MUL))

#ifdef __KERNEL__
struct eoc_priv{
	char	name [CTRL_NAME_SIZE];
	struct 	mutex		mutex;
	void *  counter_ptr;
	u32_t   period;
};

static int 
eoc_task_init(
	struct task_struct    * task_ptr,
	struct eoc_priv      * thread_priv,
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

	SDEBUG("thread_priv = %p\n" , thread_priv );
	SDEBUG("thread_fn   = %p\n" , thread_fn   );
	SDEBUG("thread_name = %p\n" , thread_name );
	SDEBUG("period      = %u\n" , period      );

	memset(thread_priv , 0 , sizeof(struct eoc_priv));
	
	thread_priv->counter_ptr = counter_ptr;
	thread_priv->period      = period;

	strncpy(thread_priv->name, thread_name , sizeof(thread_priv->name));
	mutex_init(&thread_priv->mutex);


	task_ptr = kthread_run(
				thread_fn,
				thread_priv,
				thread_priv->name);

	if (!IS_ERR(task_ptr)){
		SDEBUG("%s create successfully!\n" , thread_priv->name);
		return OK;
	}

	printk("%s create failed!\n" , thread_priv->name);
	return NOK;
}
#endif

#ifdef _AIPC_CPU_
static struct task_struct *			dsl_eoc_set_task=NULL;
static struct eoc_priv				dsl_eoc_set_dsc;

static struct task_struct *			dsl_eoc_get_task=NULL;
static struct eoc_priv				dsl_eoc_get_dsc;

static int 
dsl_eoc_set_thread(void *p)
{
	struct eoc_priv * tp = (struct eoc_priv *)p;
	int i=0;

	aipc_dsl_eoc_set_buf_t set_buf;

    set_current_state(TASK_INTERRUPTIBLE);

	do {                    
		msleep_interruptible(tp->period);
#if 1
		if((ACTSW.init & INIT_OK) == INIT_OK){
			for(i=0 ; i<5; i++){	
				msleep_interruptible(tp->period);
	
				memset( &set_buf , 0 , sizeof( aipc_dsl_eoc_set_buf_t ) );
	
				snprintf( set_buf.data  , DSL_EOC_BUF_SIZE , "%s counter=%u\n" , 
					tp->name , (*(u32_t*)(tp->counter_ptr))++);
				
				aipc_dsl_eoc_set( &set_buf );
				
				ADEBUG( DBG_CONT , "%s\n" , set_buf.data );
			}
		}
#endif
	} while (!kthread_should_stop());

	return OK;
}

static int 
dsl_eoc_get_thread(void *p)
{
	struct eoc_priv * tp = (struct eoc_priv *)p;
	int i=0;

	aipc_dsl_eoc_get_buf_t get_buf;
	

    set_current_state(TASK_INTERRUPTIBLE);

	do {                    
		msleep_interruptible(tp->period);
#if 1
		if((ACTSW.init & INIT_OK) == INIT_OK){
			for(i=0 ; i<5; i++){	
				msleep_interruptible(tp->period);
	
				memset( &get_buf , 0 , sizeof( aipc_dsl_eoc_get_buf_t ) );
	
				snprintf( get_buf.data  , DSL_EOC_BUF_SIZE , "%s counter=%u\n" , 
					tp->name , (*(u32_t*)(tp->counter_ptr))++);
				
				aipc_dsl_eoc_get( &get_buf );
				
				ADEBUG( DBG_CONT , "%s\n" , get_buf.data );
			}
		}
#endif
	} while (!kthread_should_stop());

	return OK;
}

static int 
cpu_eoc_task_start(void)
{
	int ret = NOK;

	ret = eoc_task_init(
		dsl_eoc_set_task , 
		&dsl_eoc_set_dsc ,
		dsl_eoc_set_thread,
		CPU_DSL_EOC_SET_NAME,
		&(ATHREAD.dsl_eoc_set),
		CPU_DSL_EOC_SET_PERIOD
	);
	
	if( ret != OK ){
		return ret;
	}

	ret = eoc_task_init(
		dsl_eoc_get_task , 
		&dsl_eoc_get_dsc ,
		dsl_eoc_get_thread,
		CPU_DSL_EOC_GET_NAME,
		&(ATHREAD.dsl_eoc_get),
		CPU_DSL_EOC_GET_PERIOD
	);	
	
	return ret;
}

int 
cpu_eoc_task_init(void)
{
	return cpu_eoc_task_start();
}

#elif defined(_AIPC_DSP_)

#endif //!_AIPC_CPU_ && !_AIPC_DSP_

#endif // CONFIG_RTL8686_IPC_DSL_IPC_TEST
