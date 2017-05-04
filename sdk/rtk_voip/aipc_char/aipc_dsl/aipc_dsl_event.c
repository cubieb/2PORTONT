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

#include "aipc_shm.h"  
#include "aipc_debug.h"
#include "aipc_swp.h" 
#include "aipc_osal.h" 
#include "aipc_dsl_event.h"  
#include "aipc_api.h"  



#ifdef _AIPC_CPU_
// Please DSL register its own call back
void ( *dsl_pkt_event_trap )(unsigned char* eth_pkt, unsigned long size) = NULL;   
void   aipc_dsl_event_notify(void);
EXPORT_SYMBOL(aipc_dsl_event_notify);

void * aipc_dsl_event_recv(void);
int    aipc_dsl_event_ret(void * dp);
EXPORT_SYMBOL(aipc_dsl_event_recv);
EXPORT_SYMBOL(aipc_dsl_event_ret);
#else
void * aipc_dsl_event_alloc(void);
int    aipc_dsl_event_send(void * dp);
EXPORT_SYMBOL(aipc_dsl_event_alloc);
EXPORT_SYMBOL(aipc_dsl_event_send);
#endif


/*
*	Variables
*/
#ifdef _AIPC_CPU_
#else
static volatile aipc_dsl_event_commit_t  dsl_event_commit;
static volatile u32_t                    event_alloc;
#endif




/** event queue **/
#ifdef _AIPC_DSP_
static int
aipc_dsl_event_queue_full( void )
{
	//attention!! this is count on alloc index!!
	if(((DSL_EVENT_SHM_EQ.del + DSL_EVENT_QUEUE_TOTAL - event_alloc)%DSL_EVENT_QUEUE_TOTAL) == 1){
		return TRUE;
		}
	else{
		return FALSE;
		}
}
#endif

#ifdef _AIPC_CPU_
static int
aipc_dsl_event_queue_empty( void )
{
	if(((DSL_EVENT_SHM_EQ.ins + DSL_EVENT_QUEUE_TOTAL - DSL_EVENT_SHM_EQ.del)%DSL_EVENT_QUEUE_TOTAL) == 0){
		return TRUE;
		}
	else{
		return FALSE;
		}
}
#endif



#ifdef _AIPC_CPU_

//aipc_dsl_event_recv
static void*
aipc_dsl_event_queue_deq( void )
{
	#if 0
	volatile void * retp=NULL;
	#else
	void * retp=NULL;
	#endif

	#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;
	#endif

	#ifdef DSL_EVENT_LOCK
	AIPC_OSAL_INTERRUPT_FLAGS flags;
	AIPC_OSAL_INTERRUPT_DISABLE(flags);
	ADEBUG(DBG_DSL_EVENT , "disable interrupt\n");
	#endif

	#ifdef AIPC_BARRIER
		tmp_wr = ASTATS.aipc_dsl_event_recv;
		tmp_wr++;
		AMB( ASTATS.aipc_dsl_event_recv , tmp_wr );
	#else
		ASTATS.aipc_dsl_event_recv++;
	#endif
	
	if (aipc_dsl_event_queue_empty()){
		retp=NULL;
		ADEBUG(DBG_DSL_EVENT , "event euque is empty\n");
	}
	else{
		retp = &(DSL_EVENT_SHM_EVENT[ DSL_EVENT_SHM_EQ.del ]);
		ADEBUG(DBG_DSL_EVENT, "event pointer=%p\n" , retp);
	}

	#ifdef DSL_EVENT_LOCK
	AIPC_OSAL_INTERRUPT_ENABLE(flags);
	ADEBUG(DBG_DSL_EVENT , "restore interrupt\n");
	#endif
	
	return retp;
}

void *
aipc_dsl_event_recv(void)
{
	return aipc_dsl_event_queue_deq();
}

//aipc_dsl_event_ret
static int
_aipc_dsl_event_ret(void * dp)
{
	int ret=NOK;
	
	#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;
	#endif

	#ifdef DSL_EVENT_LOCK
	AIPC_OSAL_INTERRUPT_FLAGS flags;
	AIPC_OSAL_INTERRUPT_DISABLE(flags);
	ADEBUG(DBG_DSL_EVENT , "disable interrupt\n");
	#endif
	
	#ifdef AIPC_BARRIER
		tmp_wr = DSL_EVENT_SHM_EQ.del;
		tmp_wr = (tmp_wr+1) % DSL_EVENT_QUEUE_TOTAL;	
		AMB( DSL_EVENT_SHM_EQ.del , tmp_wr );
		
		#ifdef QUEUE_OPERATION_CNT
		tmp_wr = DSL_EVENT_SHM_EQ.cnt_del;
		tmp_wr++;
		AMB( DSL_EVENT_SHM_EQ.cnt_del , tmp_wr );
		#endif
		
		tmp_wr = ASTATS.aipc_dsl_event_ret;
		tmp_wr++;
		AMB( ASTATS.aipc_dsl_event_ret , tmp_wr );
	#else
		DSL_EVENT_SHM_EQ.del = (DSL_EVENT_SHM_EQ.del+1)%DSL_EVENT_QUEUE_TOTAL;
		
		#ifdef QUEUE_OPERATION_CNT
		DSL_EVENT_SHM_EQ.cnt_del++;
		#endif

		ASTATS.aipc_dsl_event_ret++;
	#endif

	
	ret=OK;
	
	#ifdef DSL_EVENT_LOCK
	AIPC_OSAL_INTERRUPT_ENABLE(flags);
	ADEBUG(DBG_DSL_EVENT , "restore interrupt\n");
	#endif

	return ret;
}

int
aipc_dsl_event_ret(void * dp)
{
	return _aipc_dsl_event_ret(dp);
}


void aipc_dsl_event_notify(void)
{
	
	void *mp = NULL;
	
	if( (dsl_pkt_event_trap != NULL)  && ((ACTSW.init & INIT_OK) == INIT_OK)){
	
		mp = aipc_dsl_event_recv();
				
		if( mp ){
			(*dsl_pkt_event_trap )( (unsigned char *) mp, DSL_EVENT_SIZE ) ;
			aipc_dsl_event_ret( mp );
		}
	}
}

#if defined(CONFIG_DSL_ON_SLAVE)
extern int xdsl_ipc_2cpu_event_recv(unsigned char* eth_pkt, unsigned long size);
#endif

void dsl_pkt_event_test(unsigned char* eth_pkt, unsigned long size)
{
	ADEBUG(DBG_DSL_EVENT, "eth_pkt=%p\n" , eth_pkt);
	ADEBUG(DBG_DSL_EVENT , "%s\n" , eth_pkt);
}

int
aipc_cpu_dsl_event_queue_init(void)
{
	DSL_EVENT_SHM_EQ.ins = 0;
	DSL_EVENT_SHM_EQ.del = 0;
	
#if defined(CONFIG_DSL_ON_SLAVE)
	dsl_pkt_event_trap = xdsl_ipc_2cpu_event_recv;  
#else
	dsl_pkt_event_trap = NULL;
#endif
	return 0 ;
}



#elif defined(_AIPC_DSP_)

//aipc_dsl_event_alloc
static void *	
_aipc_dsl_event_alloc( void )
{
	void *retp = NULL;
	AIPC_OSAL_INTERRUPT_FLAGS _old;

	#ifdef CTRL_SEND_LIMIT
	int  ctrl_send_limit = 0;
	#endif

	#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;
	#endif

again:
	#ifdef DSL_EVENT_LOCK
	AIPC_OSAL_INTERRUPT_DISABLE(_old);
	#endif
	if (aipc_dsl_event_queue_full()){

		#ifdef STATS_RETRY
		if (ASTATS.aipc_dsl_event_alloc_retry > STATS_CTRL_RETRY_MAX){
				printk( "Over retry maximum %s %d\n" , __FUNCTION__ , __LINE__);
			#ifdef AIPC_BARRIER
				AMB( ASTATS.aipc_dsl_event_alloc_retry , 0);
			#else
				ASTATS.aipc_dsl_event_alloc_retry = 0;
			#endif
			}
		else{
			#ifdef AIPC_BARRIER
				tmp_wr = ASTATS.aipc_dsl_event_alloc_retry;
				tmp_wr++;
				AMB( ASTATS.aipc_dsl_event_alloc_retry , tmp_wr );
			#else
				ASTATS.aipc_dsl_event_alloc_retry++;
			#endif
		}
		#endif

		#ifdef CTRL_SEND_LIMIT
		ctrl_send_limit++;
		#endif
			
		#ifdef DSL_EVENT_LOCK
		AIPC_OSAL_INTERRUPT_ENABLE(_old);
		#endif

		#ifdef CTRL_SEND_LIMIT
		if (ctrl_send_limit > CTRL_SEND_RETRY_MAX){
			AIPC_OSAL_SCHEDULE_TIMEOUT_INTERRUPTIBLE(1);
			return retp;
		}
		#endif		
		
		AIPC_OSAL_SCHEDULE_TIMEOUT_INTERRUPTIBLE(1);
		goto again;
	}

	retp = &(DSL_EVENT_SHM_EVENT[ event_alloc ]);
	dsl_event_commit.commit[ event_alloc ] = 0;
	event_alloc = (event_alloc+1) % DSL_EVENT_QUEUE_TOTAL;

	#ifdef AIPC_BARRIER
		tmp_wr = ASTATS.aipc_dsl_event_alloc;
		tmp_wr++;
		AMB( ASTATS.aipc_dsl_event_alloc , tmp_wr );
	#else
		ASTATS.aipc_dsl_event_alloc++;
	#endif


	#ifdef STATS_RETRY
	#ifdef AIPC_BARRIER
		AMB( ASTATS.aipc_dsl_event_alloc_retry , 0 );
	#else
		ASTATS.aipc_dsl_event_alloc_retry = 0;
	#endif
	#endif

	ADEBUG(DBG_DSL_EVENT , "alloc event buffer %p\n" , retp);
	#ifdef DSL_EVENT_LOCK
	AIPC_OSAL_INTERRUPT_ENABLE(_old);
	#endif
	return retp;
}

void *
aipc_dsl_event_alloc(void)
{
	return _aipc_dsl_event_alloc();
}

//aipc_dsl_event_send
static int	 
aipc_2cpu_event_queue_enq( void *dp )
{
	//extern int aipc_int_send_2cpu( u32_t int_id );

	int ret=NOK;
	//int int_id=AIPC_MGR_2CPU_EVENT_RECV;
	int cur=0;
	int idx=NOK;

	#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;
	#endif

	#ifdef DSL_EVENT_LOCK
	AIPC_OSAL_INTERRUPT_FLAGS _old;
	AIPC_OSAL_INTERRUPT_DISABLE(_old);
	ADEBUG(DBG_DSL_EVENT , "disable interrupt\n");
	#endif

	idx = DSL_EVENT_IDX(dp);

	if(idx!=NOK){
		dsl_event_commit.commit[idx] = 1;
	}
	else{
		#ifdef DSL_EVENT_LOCK
		AIPC_OSAL_INTERRUPT_ENABLE(_old);
		ADEBUG(DBG_DSL_EVENT , "restore interrupt\n");
		#endif
		return ret;
	}

	cur = DSL_EVENT_SHM_EQ.ins;

	while(cur != event_alloc){
		if(dsl_event_commit.commit[cur]==0)
			break;

		cur = (cur+1)%DSL_EVENT_QUEUE_TOTAL;
	}

	if(DSL_EVENT_SHM_EQ.ins != event_alloc){
		#ifdef AIPC_BARRIER
			AMB( DSL_EVENT_SHM_EQ.ins , cur );
		#else
			DSL_EVENT_SHM_EQ.ins = cur;
		#endif

		#ifdef QUEUE_OPERATION_CNT
		#ifdef AIPC_BARRIER
			tmp_wr = DSL_EVENT_SHM_EQ.cnt_ins;
			tmp_wr++;
			AMB( DSL_EVENT_SHM_EQ.cnt_ins , tmp_wr );
		#else
			DSL_EVENT_SHM_EQ.cnt_ins++;
		#endif
		#endif

		//ret = aipc_int_send_2cpu( int_id );
		//if (ret==OK){
		//	ADEBUG(DBG_DSL_EVENT, "enqueue %p OK\n\n" , dp);
		//	//SDEBUG("enqueue %p OK\n\n" , dp);
		//	}
		//else{
		//	ADEBUG(DBG_DSL_EVENT, "enqueue %p failed!!!!!\n\n" , dp);
		//	//SDEBUG("enqueue %p failed!!!!!\n\n" , dp);
		//	}
	}else{
		#ifdef AIPC_BARRIER
			tmp_wr = ASTATS.aipc_dsl_event_exception_send;
			tmp_wr++;
			AMB( ASTATS.aipc_dsl_event_exception_send , tmp_wr );
		#else
			ASTATS.aipc_dsl_event_exception_send++;
		#endif
	}

	#ifdef AIPC_BARRIER
		tmp_wr = ASTATS.aipc_dsl_event_send;
		tmp_wr++;
		AMB( ASTATS.aipc_dsl_event_send , tmp_wr);	
	#else
		ASTATS.aipc_dsl_event_send++;
	#endif	

	#ifdef DSL_EVENT_LOCK
	AIPC_OSAL_INTERRUPT_ENABLE(_old);
	ADEBUG(DBG_DSL_EVENT , "restore interrupt\n");
	#endif

	return ret;	
}

int
aipc_dsl_event_send(void * dp)
{
	return aipc_2cpu_event_queue_enq(dp);
}


static int 
aipc_dsp_dsl_event_index_init(void)
{
	/*
	*	init event queue index
	*/
	event_alloc = 0;		//this only can be done in DSP
	DSL_EVENT_SHM_EQ.ins = 0;	//this can be done in CPU
	DSL_EVENT_SHM_EQ.del = 0;	//this can be done in CPU

	/*
	*	init event queue buffer
	*/
	memset(&dsl_event_commit , 0 , sizeof(aipc_dsl_event_commit_t)); //this only can be done in DSP
	return 0;
}

core_initcall( aipc_dsp_dsl_event_index_init );

#else
#error "need to check CPU/DSP"
#endif


#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int voip_host_cch_invert_table_read_proc(struct seq_file *f, void *v)
{
	int n = 0;
#ifdef _AIPC_DSP_
	int i = 0;
#endif

	//Event queue
	seq_printf( f, "Event:\n");
	seq_printf( f, "\tDSL_EVENT_SHM_EQ:\n");
	seq_printf( f, "\tindex ins=%d del=%d total=%d\n" , DSL_EVENT_SHM_EQ.ins     , DSL_EVENT_SHM_EQ.del , DSL_EVENT_QUEUE_TOTAL);
	seq_printf( f, "\tcount ins=%d del=%d\n"          , DSL_EVENT_SHM_EQ.cnt_ins , DSL_EVENT_SHM_EQ.cnt_del);

#ifdef _AIPC_CPU_
	seq_printf( f, "\tDSL_EVENT_SHM_EQ event_alloc and event_commit are in DSP side. \n\n");
#else
	seq_printf( f, "\tevent_alloc = %u\n" , event_alloc );
	seq_printf( f, "\tevent_commit:\n" );
	for(i = 0 ; i<DSL_EVENT_QUEUE_TOTAL ; i++ )
		seq_printf( f, "%u " , dsl_event_commit.commit[i]);	
		
	n += sprintf( buf+n , "\n");	
#endif
	
	
	//	Event
    seq_printf( f, "\t(CPU) event CPU<-DSP: \n");    
    seq_printf( f, "\taipc_dsl_event_recv   = %u\n"  ,      ASTATS.aipc_dsl_event_recv);
    seq_printf( f, "\taipc_dsl_event_ret    = %u  DSL_EVENT_SHM_EQ.cnt_del = %u\n" , 
                      ASTATS.aipc_dsl_event_ret , DSL_EVENT_SHM_EQ.cnt_del);
    
    seq_printf( f, "\t(DSP) event CPU<-DSP: \n");
    seq_printf( f, "\taipc_dsl_event_alloc  = %u\n"  ,       ASTATS.aipc_dsl_event_alloc);
    seq_printf( f, "\taipc_dsl_event_alloc_retry  = %u\n"  , ASTATS.aipc_dsl_event_alloc_retry);
	seq_printf( f, "\taipc_dsl_event_send   = %u  DSL_EVENT_SHM_EQ.cnt_ins = %u\n"  , 
                      ASTATS.aipc_dsl_event_send ,       DSL_EVENT_SHM_EQ.cnt_ins);

	return n;
}
#else
int
aipc_dsl_proc_event_dump(char *buf)
{
	int n = 0;
#ifdef _AIPC_DSP_
	int i = 0;
#endif

	//Event queue
	n += sprintf(buf+n , "Event:\n");
	n += sprintf(buf+n , "\tDSL_EVENT_SHM_EQ:\n");
	n += sprintf(buf+n , "\tindex ins=%d del=%d total=%d\n" , DSL_EVENT_SHM_EQ.ins     , DSL_EVENT_SHM_EQ.del , DSL_EVENT_QUEUE_TOTAL);
	n += sprintf(buf+n , "\tcount ins=%d del=%d\n"          , DSL_EVENT_SHM_EQ.cnt_ins , DSL_EVENT_SHM_EQ.cnt_del);

#ifdef _AIPC_CPU_
	n += sprintf(buf+n , "\tDSL_EVENT_SHM_EQ event_alloc and event_commit are in DSP side. \n\n");
#else
	n += sprintf(buf+n , "\tevent_alloc = %u\n" , event_alloc );
	n += sprintf(buf+n , "\tevent_commit:\n" );
	for(i = 0 ; i<DSL_EVENT_QUEUE_TOTAL ; i++ )
		n += sprintf(buf+n , "%u " , dsl_event_commit.commit[i]);	
		
	n += sprintf( buf+n , "\n");	
#endif
	
	
	//	Event
    n += sprintf(buf+n , "\t(CPU) event CPU<-DSP: \n");    
    n += sprintf(buf+n , "\taipc_dsl_event_recv   = %u\n"  ,      ASTATS.aipc_dsl_event_recv);
    n += sprintf(buf+n , "\taipc_dsl_event_ret    = %u  DSL_EVENT_SHM_EQ.cnt_del = %u\n" , 
                      ASTATS.aipc_dsl_event_ret , DSL_EVENT_SHM_EQ.cnt_del);
    
    n += sprintf(buf+n , "\t(DSP) event CPU<-DSP: \n");
    n += sprintf(buf+n , "\taipc_dsl_event_alloc  = %u\n"  ,       ASTATS.aipc_dsl_event_alloc);
    n += sprintf(buf+n , "\taipc_dsl_event_alloc_retry  = %u\n"  , ASTATS.aipc_dsl_event_alloc_retry);
	n += sprintf(buf+n , "\taipc_dsl_event_send   = %u  DSL_EVENT_SHM_EQ.cnt_ins = %u\n"  , 
                      ASTATS.aipc_dsl_event_send ,       DSL_EVENT_SHM_EQ.cnt_ins);

	return n;
}
#endif


#ifdef CONFIG_RTL8686_IPC_DSL_IPC_TEST

#ifdef _AIPC_DSP_		//Event Test

#define CTRL_NAME_SIZE		32

#define DSL_EVENT_SEND_NAME		"DspEventSendD"
#define DSL_EVENT_SEND_PERIOD	((1)*(MUL))

static int dsl_event_send_thread(void *p);

struct dsl_event_priv{
	char	name [CTRL_NAME_SIZE];
	struct 	mutex		mutex;
	void *  counter_ptr;
	u32_t   period;
};

static int 
event_task_init(
	struct task_struct    * task_ptr,
	struct dsl_event_priv * thread_priv,
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
	SDEBUG("thread_name = %s\n" , thread_name );
	SDEBUG("period      = %u\n" , period      );

	memset(thread_priv , 0 , sizeof(struct dsl_event_priv));
	
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

static struct task_struct *         dsl_event_send_task=NULL;
static struct dsl_event_priv        dsl_event_send_dsc;


static int 
dsl_event_send_thread(void *p)
{
	struct dsl_event_priv * tp = (struct dsl_event_priv *)p;
	void *mp;

	SDEBUG("\n");

    set_current_state(TASK_INTERRUPTIBLE);

	do { 
		msleep_interruptible( DSL_EVENT_SEND_PERIOD );
#if 1
		if((ACTSW.init & INIT_OK) == INIT_OK){
			mp = aipc_dsl_event_alloc();

			if(mp){
				memset(mp , 0 , DSL_EVENT_SIZE);

				snprintf(mp , DSL_EVENT_SIZE, "%s counter=%u\n" , 
					tp->name , (*(u32_t*)(tp->counter_ptr))++);

				ADEBUG(DBG_DSL_EVENT , "%s\n" , (char*)mp);

				aipc_dsl_event_send( mp );			
			}
			else{
				ADEBUG(DBG_DSL_EVENT , "no buffers\n");
			}
		}
#endif
	} while (!kthread_should_stop());

	return OK;
}

int 
dsl_event_send_task_init(void)
{
	int ret = NOK;
	SDEBUG("\n");
	ret = event_task_init(
		dsl_event_send_task , 
		&dsl_event_send_dsc ,
		dsl_event_send_thread,
		DSL_EVENT_SEND_NAME,
		&(ATHREAD.dsl_event_send),
		DSL_EVENT_SEND_PERIOD
	);
	SDEBUG("&dsl_event_send_dsc = %p \n"    , &dsl_event_send_dsc );
	SDEBUG(" dsl_event_send_thread = %p \n" , dsl_event_send_thread  );


	return ret;
}

module_init( dsl_event_send_task_init );

#endif  // _AIPC_DSP_
#endif  // CONFIG_RTL8686_IPC_DSL_IPC_TEST
