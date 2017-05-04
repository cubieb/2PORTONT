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
#include "aipc_dsl_ctrl.h"

#ifdef _AIPC_CPU_		//API functions

int   aipc_dsl_ctrl_set( aipc_dsl_ctrl_set_buf_t *set_buf , int sleep );
int   aipc_dsl_ctrl_get( aipc_dsl_ctrl_get_buf_t *get_buf , int sleep );
void  aipc_dsl_ctrl_ret(void);
int   aipc_dsl_ctrl_wait(void);
void *aipc_dsl_ctrl_recv(void);

EXPORT_SYMBOL( aipc_dsl_ctrl_set );
EXPORT_SYMBOL( aipc_dsl_ctrl_get );
EXPORT_SYMBOL( aipc_dsl_ctrl_ret );
EXPORT_SYMBOL( aipc_dsl_ctrl_wait );
EXPORT_SYMBOL( aipc_dsl_ctrl_recv );

#else

int   aipc_dsl_ctrl_set( aipc_dsl_ctrl_set_buf_t *set_buf , int sleep );
void *aipc_dsl_ctrl_recv(void);
void  aipc_dsl_ctrl_ret(void);
int   aipc_dsl_ctrl_wait(void);

EXPORT_SYMBOL( aipc_dsl_ctrl_set );
EXPORT_SYMBOL( aipc_dsl_ctrl_recv );
EXPORT_SYMBOL( aipc_dsl_ctrl_ret  );
EXPORT_SYMBOL( aipc_dsl_ctrl_wait  );

#endif


#if defined(_AIPC_CPU_)
int aipc_dsl_ctrl_set( aipc_dsl_ctrl_set_buf_t  *set_buf , int sleep )
{
	int call_cnt = 0;
	
	#ifdef AIPC_BARRIER
		volatile unsigned int tmp_wr=0;
		tmp_wr = ASTATS.aipc_dsl_ctrl_set;
		tmp_wr++;
		AMB( ASTATS.aipc_dsl_ctrl_set , tmp_wr );
	#else
		ASTATS.aipc_dsl_ctrl_set++;
	#endif 	
	
	while( DSL_CTRL_SHM.own != AIPC_DSL_OWN_CPU ){
		if( sleep )
			AIPC_OSAL_SCHEDULE_TIMEOUT_INTERRUPTIBLE(1);
		else
			AIPC_OSAL_MDELAY(1);

		if( (call_cnt>100) && printk_ratelimit() ){
			SDEBUG("long loop\n");
			call_cnt = 0;
		}
		else
			call_cnt++;
	}

	memcpy( &DSL_CTRL_SHM.data , set_buf , sizeof( aipc_dsl_ctrl_set_buf_t ) );	
	
	#ifdef AIPC_BARRIER
	AMB( DSL_CTRL_SHM.dummy , 0 );
	#endif
	
	#ifdef AIPC_BARRIER
		AMB( DSL_CTRL_SHM.own , AIPC_DSL_OWN_DSP );
	#else
		DSL_CTRL_SHM.own = AIPC_DSL_OWN_DSP;
	#endif
	
	return OK;		
}

int aipc_dsl_ctrl_get( aipc_dsl_ctrl_get_buf_t *get_buf , int sleep )
{
	int call_cnt = 0;
	
	#ifdef AIPC_BARRIER
		volatile unsigned int tmp_wr=0;
		tmp_wr = ASTATS.aipc_dsl_ctrl_get;
		tmp_wr++;
		AMB( ASTATS.aipc_dsl_ctrl_get , tmp_wr );
	#else
		ASTATS.aipc_dsl_ctrl_get++;
	#endif 	
	
	while( DSL_CTRL_SHM.own != AIPC_DSL_OWN_CPU ){
		if( sleep )
			AIPC_OSAL_SCHEDULE_TIMEOUT_INTERRUPTIBLE(1);
		else
			AIPC_OSAL_MDELAY(1);

		if( (call_cnt>100) && printk_ratelimit() ){
			SDEBUG("long loop\n");
			call_cnt = 0;
		}
		else
			call_cnt++;
	}
	
	memcpy( &DSL_CTRL_SHM.data , get_buf , sizeof( aipc_dsl_ctrl_get_buf_t ) );
	
	#ifdef AIPC_BARRIER
	AMB( DSL_CTRL_SHM.dummy , 0 );
	#endif
	
	#ifdef AIPC_BARRIER
		AMB( DSL_CTRL_SHM.own , AIPC_DSL_OWN_DSP );
	#else
		DSL_CTRL_SHM.own = AIPC_DSL_OWN_DSP;
	#endif
	
	while( DSL_CTRL_SHM.own != AIPC_DSL_OWN_CPU ){
		if( sleep )
			AIPC_OSAL_SCHEDULE_TIMEOUT_INTERRUPTIBLE(1);
		else
			AIPC_OSAL_MDELAY(1);

		if( (call_cnt>100) && printk_ratelimit() ){
			SDEBUG("long loop\n");
			call_cnt = 0;
		}
		else
			call_cnt++;
	}
	
	memcpy( get_buf , &DSL_CTRL_SHM.data , sizeof( aipc_dsl_ctrl_get_buf_t ) );
	
	return OK;		
}

void *aipc_dsl_ctrl_recv(void)
{
	if( DSL_CTRL_SHM.own == AIPC_DSL_OWN_CPU )
		return &DSL_CTRL_SHM.data;
	else
		return NULL;	
}


/*** add by FrankLiao ***/
void aipc_dsl_ctrl_ret(void)
{
	#ifdef AIPC_BARRIER
		AMB( DSL_CTRL_SHM.own , AIPC_DSL_OWN_DSP );
	#else
		DSL_CTRL_SHM.own = AIPC_DSL_OWN_DSP;
	#endif
}

/** add by FrankLiao ***/
int aipc_dsl_ctrl_wait(void)
{
	int call_cnt = 0;
	while( DSL_CTRL_SHM.own != AIPC_DSL_OWN_CPU ){
		AIPC_OSAL_SCHEDULE_TIMEOUT_INTERRUPTIBLE(1);

		if( (call_cnt>100) && printk_ratelimit() ){
			SDEBUG("long loop\n");
			call_cnt = 0;
		}
		else
			call_cnt++;
	}
	return OK;
}

int
aipc_cpu_dsl_ctrl_init(void)
{
	DSL_CTRL_SHM.own = AIPC_DSL_OWN_CPU;
	memset( &DSL_CTRL_SHM.data , 0 , sizeof( aipc_dsl_ctrl_set_buf_t ) );
	return 0 ;
}


#elif defined(_AIPC_DSP_)

void *aipc_dsl_ctrl_recv(void)
{
	#ifdef AIPC_BARRIER
		volatile unsigned int tmp_wr=0;
		tmp_wr = ASTATS.aipc_dsl_ctrl_recv;
		tmp_wr++;
		AMB( ASTATS.aipc_dsl_ctrl_recv , tmp_wr );
	#else
		ASTATS.aipc_dsl_ctrl_recv++;
	#endif 	
	
	if( DSL_CTRL_SHM.own == AIPC_DSL_OWN_DSP )
		return &DSL_CTRL_SHM.data;
	else
		return NULL;	
}

void aipc_dsl_ctrl_ret(void)
{
	#ifdef AIPC_BARRIER
		volatile unsigned int tmp_wr=0;
		tmp_wr = ASTATS.aipc_dsl_ctrl_ret;
		tmp_wr++;
		AMB( ASTATS.aipc_dsl_ctrl_ret , tmp_wr );

		AMB( DSL_CTRL_SHM.own , AIPC_DSL_OWN_CPU );
	#else
		ASTATS.aipc_dsl_ctrl_ret++;
		
		DSL_CTRL_SHM.own = AIPC_DSL_OWN_CPU;
	#endif
}

int aipc_dsl_ctrl_set( aipc_dsl_ctrl_set_buf_t  *set_buf , int sleep)
{
	int call_cnt = 0;
	
	memcpy( &DSL_CTRL_SHM.data , set_buf , sizeof( aipc_dsl_ctrl_set_buf_t ) );	
	

	#ifdef AIPC_BARRIER
	AMB( DSL_CTRL_SHM.dummy , 0 );
	#endif

	return OK;		
}

int aipc_dsl_ctrl_wait(void)
{
	int call_cnt = 0;
	while( DSL_CTRL_SHM.own != AIPC_DSL_OWN_DSP ){
		AIPC_OSAL_SCHEDULE_TIMEOUT_INTERRUPTIBLE(1);

		if( (call_cnt>100) && printk_ratelimit() ){
			SDEBUG("long loop\n");
			call_cnt = 0;
		}
		else
			call_cnt++;
	}
	return OK;
}

#else
#error "need to check CPU/DSP"
#endif

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int voip_host_cch_invert_table_read_proc(struct seq_file *f)
{
	int n = 0;
	
	seq_printf( f, "Control:\n");
	seq_printf( f, "\tProperty:\n");

	if( DSL_CTRL_SHM.own == AIPC_DSL_OWN_CPU )
		seq_printf( f, "\t own: CPU\n");
	else if ( DSL_CTRL_SHM.own == AIPC_DSL_OWN_DSP )
		seq_printf( f, "\t own: DSP\n");
	else
		seq_printf( f, "\t own: Unknown\n");

	seq_printf( f, "\tAPI:\n");
	seq_printf( f, "\t aipc_dsl_ctrl_set  = %u\n" , ASTATS.aipc_dsl_ctrl_set );
	seq_printf( f, "\t aipc_dsl_ctrl_get  = %u\n" , ASTATS.aipc_dsl_ctrl_get );
	seq_printf( f, "\t aipc_dsl_ctrl_recv = %u\n" , ASTATS.aipc_dsl_ctrl_recv);
	seq_printf( f, "\t aipc_dsl_ctrl_ret  = %u\n" , ASTATS.aipc_dsl_ctrl_ret );
	
	return n;
}
#else
int aipc_dsl_proc_ctrl_dump (char *buf)
{
	int n = 0;
	
	n += sprintf(buf+n , "Control:\n");
	n += sprintf(buf+n , "\tProperty:\n");

	if( DSL_CTRL_SHM.own == AIPC_DSL_OWN_CPU )
		n += sprintf(buf+n , "\t own: CPU\n");
	else if ( DSL_CTRL_SHM.own == AIPC_DSL_OWN_DSP )
		n += sprintf(buf+n , "\t own: DSP\n");
	else
		n += sprintf(buf+n , "\t own: Unknown\n");

	n += sprintf(buf+n , "\tAPI:\n");
	n += sprintf(buf+n , "\t aipc_dsl_ctrl_set  = %u\n" , ASTATS.aipc_dsl_ctrl_set );
	n += sprintf(buf+n , "\t aipc_dsl_ctrl_get  = %u\n" , ASTATS.aipc_dsl_ctrl_get );
	n += sprintf(buf+n , "\t aipc_dsl_ctrl_recv = %u\n" , ASTATS.aipc_dsl_ctrl_recv);
	n += sprintf(buf+n , "\t aipc_dsl_ctrl_ret  = %u\n" , ASTATS.aipc_dsl_ctrl_ret );
	
	return n;
}
#endif

#ifdef CONFIG_RTL8686_IPC_DSL_IPC_TEST

#define CTRL_NAME_SIZE		32

//CPU
#define CPU_DSL_CTRL_SET_NAME       "CpuCtrlSetD"
#define CPU_DSL_CTRL_SET_PERIOD	    ((1)*(MUL))


//DSP
#define DSP_DSL_CTRL_POLL_NAME      "DspCtrlPollD"
#define DSP_DSL_CTRL_POLL_PERIOD    ((1)*(MUL))


#ifdef __KERNEL__
struct ctrl_priv{
	char	name [CTRL_NAME_SIZE];
	struct 	mutex		mutex;
	void *  counter_ptr;
	u32_t   period;
};

static int 
ctrl_task_init(
	struct task_struct    * task_ptr,
	struct ctrl_priv      * thread_priv,
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

	memset(thread_priv , 0 , sizeof(struct ctrl_priv));
	
	thread_priv->counter_ptr = counter_ptr;
	thread_priv->period      = period;

	strncpy(thread_priv->name, thread_name , strlen(thread_priv->name));
	//mutex_init(&thread_priv->mutex);


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
static struct task_struct *			dsl_ctrl_set_task=NULL;
static struct ctrl_priv				dsl_ctrl_set_dsc;


static int 
dsl_ctrl_set_thread(void *p)
{
	struct ctrl_priv * tp = (struct ctrl_priv *)p;
	int i=0;

	aipc_dsl_ctrl_set_buf_t set_buf;
	aipc_dsl_ctrl_get_buf_t get_buf;
	

    set_current_state(TASK_INTERRUPTIBLE);

	do {                    
		msleep_interruptible(tp->period);

#if 1
		if((ACTSW.init & INIT_OK) == INIT_OK){
			for(i=0 ; i<5; i++){	
				msleep_interruptible(tp->period);
	
				memset( &set_buf , 0 , sizeof( aipc_dsl_ctrl_set_buf_t ) );
	
				snprintf( set_buf.data  , DSL_CTRL_BUF_SIZE , "%s counter=%u\n" , 
					tp->name , (*(u32_t*)(tp->counter_ptr))++);
				
				aipc_dsl_ctrl_set( &set_buf , 1 );
			}
	
			for(i=0 ; i<5; i++){	
				msleep_interruptible(tp->period);
	
				memset( &get_buf , 0 , sizeof( aipc_dsl_ctrl_get_buf_t ) );
	
				snprintf( get_buf.data  , DSL_CTRL_BUF_SIZE , "%s counter=%u\n" , 
					tp->name , (*(u32_t*)(tp->counter_ptr))++);
				
				aipc_dsl_ctrl_get( &get_buf , 1 );
				
				ADEBUG( DBG_CONT , "%s\n" , get_buf.data );
			}
		}
#endif

	} while (!kthread_should_stop());

	return OK;
}


static int 
cpu_ctrl_task_start(void)
{
	int ret = NOK;

	ret = ctrl_task_init(
		dsl_ctrl_set_task , 
		&dsl_ctrl_set_dsc ,
		dsl_ctrl_set_thread,
		CPU_DSL_CTRL_SET_NAME,
		&(ATHREAD.dsl_ctrl_set),
		CPU_DSL_CTRL_SET_PERIOD
	);
	
	return ret;
}

int 
cpu_ctrl_task_init(void)
{
	return cpu_ctrl_task_start();
}

static struct task_struct *aipc_startup_task;
int cpu_dsl_test_thread_entry(void);

extern int cpu_eoc_task_init(void);

#define CPU_STARTUP_THREAD "IpcDslStartD"

static int 
aipc_startup_thread(void *p)
{
	extern int	cpu_ctrl_task_init(void);

	static int init_done  = 0;

	SDEBUG("%s start\n" , CPU_STARTUP_THREAD);
	
	set_current_state(TASK_INTERRUPTIBLE);
	do {					
		msleep_interruptible(10);
		//ADEBUG(DBG_SYSTEM, "%s executing ACTSW.init=%x addr=%p\n" , 
		//CPU_STARTUP_THREAD , ACTSW.init , &(ACTSW));

		if( !init_done ){
			if((ACTSW.init & INIT_OK)){
				//control & data test thread init
				cpu_ctrl_task_init();
				cpu_eoc_task_init();
				init_done = 1;
				break;
				}
			}
	} while (!kthread_should_stop());

	ADEBUG(DBG_SYSTEM, "%s exit\n" , CPU_STARTUP_THREAD);
	kthread_stop(aipc_startup_task);
	
	return OK;
}

int
cpu_dsl_test_thread_entry(void)
{
	aipc_startup_task = kthread_run(
					aipc_startup_thread,
					NULL, 
					CPU_STARTUP_THREAD);

	if (!IS_ERR(aipc_startup_task)){
		SDEBUG("%s create successfully!\n" , CPU_STARTUP_THREAD);
		return OK;
	}

	printk("%s create failed!\n" , CPU_STARTUP_THREAD);
	return NOK;
}

module_init( cpu_dsl_test_thread_entry );

#elif defined(_AIPC_DSP_)

static struct task_struct *         dsl_ctrl_poll_task=NULL;
static struct ctrl_priv             dsl_ctrl_poll_dsc;

extern void *aipc_dsl_eoc_recv_set(void);
extern void *aipc_dsl_eoc_recv_get(void);
extern void  aipc_dsl_eoc_ret_set(void) ;
extern void  aipc_dsl_eoc_ret_get(void) ;


static int
dsl_ctrl_poll_thread(void *p)
{
	void *mp;

	SDEBUG("\n");

    set_current_state(TASK_INTERRUPTIBLE);

	do { 
		msleep_interruptible( DSP_DSL_CTRL_POLL_PERIOD );
#if 1
		if((ACTSW.init & INIT_OK) == INIT_OK){

	// Control		
			mp = aipc_dsl_ctrl_recv();
			if(mp){				
				ADEBUG(DBG_CONT , "%s\n" , (char*)mp);
	
				snprintf(mp , DSL_CTRL_BUF_SIZE , "dummy test info\n");
				
				aipc_dsl_ctrl_ret();
			}
	
	// EOC
			mp = aipc_dsl_eoc_recv_set();
			if(mp){				
				ADEBUG(DBG_CONT , "%s\n" , (char*)mp);
	
				snprintf(mp , DSL_CTRL_BUF_SIZE , "dummy test info\n");
				
				aipc_dsl_eoc_ret_set();
			}
	
			mp = aipc_dsl_eoc_recv_get();
			if(mp){				
				ADEBUG(DBG_CONT , "%s\n" , (char*)mp);
	
				snprintf(mp , DSL_CTRL_BUF_SIZE , "dummy test info\n");
				
				aipc_dsl_eoc_ret_get();			
			}
		}
#endif
			
	} while (!kthread_should_stop());
	
	return OK;
}

static int 
dsl_ctrl_poll_task_init(void)
{
	int ret = NOK;
	ret = ctrl_task_init(
		dsl_ctrl_poll_task , 
		&dsl_ctrl_poll_dsc ,
		dsl_ctrl_poll_thread,
		DSP_DSL_CTRL_POLL_NAME,
		&(ATHREAD.dsl_ctrl_recv),
		DSP_DSL_CTRL_POLL_PERIOD
	);

	return ret;
}

int
dsp_dsl_test_thread_entry(void)
{
	SDEBUG("\n");
	dsl_ctrl_poll_task_init();
	
	return OK;
}

module_init( dsp_dsl_test_thread_entry );

#endif //!_AIPC_CPU_ && !_AIPC_DSP_

#endif // CONFIG_RTL8686_IPC_DSL_IPC_TEST
