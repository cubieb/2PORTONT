#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/version.h>
#include <linux/spinlock.h>
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
#include "./include/aipc_shm.h"
#include "./include/aipc_mem.h"
#include "./include/aipc_intq.h"
#include "./include/aipc_ioctl.h"
#include "./include/aipc_debug.h"
#include "./include/aipc_swp.h"

 
/*
*		CPU						DSP
*   ----------------------------------------
*	intq_2cpu.hiq			intq_2dsp.hiq
*
*	intq_2cpu.lowq			intq_2dsp.lowq
*
*/

#ifdef AIPC_GOT
#ifdef _AIPC_CPU_
static
void aipc_cpu_intq_got_init( void )
{
	/* CPU */
	//AGOT.aipc_int_send_2dsp 	= aipc_int_send_2dsp;
	AGOT.aipc_int_2dsp_hiq_enqueue	= aipc_int_2dsp_hiq_enqueue;
	AGOT.aipc_int_2dsp_lowq_enqueue	= aipc_int_2dsp_lowq_enqueue;
	AGOT.aipc_int_2cpu_hiq_dequeue	= aipc_int_2cpu_hiq_dequeue;
	AGOT.aipc_int_2cpu_lowq_dequeue	= aipc_int_2cpu_lowq_dequeue;
	AGOT.aipc_int_2cpu_hiq_empty 	= aipc_int_2cpu_hiq_empty;
	AGOT.aipc_int_2cpu_lowq_empty	= aipc_int_2cpu_lowq_empty;
	AGOT.aipc_int_2dsp_hiq_full 	= aipc_int_2dsp_hiq_full;
	AGOT.aipc_int_2dsp_lowq_full	= aipc_int_2dsp_lowq_full;
}
#elif defined(_AIPC_DSP_)
static
void aipc_dsp_intq_got_init( void )
{
	/* DSP */
	//AGOT.aipc_int_send_2cpu	= aipc_int_send_2cpu;
	AGOT.aipc_int_2cpu_hiq_enqueue	= aipc_int_2cpu_hiq_enqueue;
	AGOT.aipc_int_2cpu_lowq_enqueue = aipc_int_2cpu_lowq_enqueue;
	AGOT.aipc_int_2dsp_hiq_dequeue	= aipc_int_2dsp_hiq_dequeue;
	AGOT.aipc_int_2dsp_lowq_dequeue	= aipc_int_2dsp_lowq_dequeue;
	AGOT.aipc_int_2dsp_hiq_empty	= aipc_int_2dsp_hiq_empty;
	AGOT.aipc_int_2dsp_lowq_empty	= aipc_int_2dsp_hiq_empty;
	AGOT.aipc_int_2cpu_hiq_full	= aipc_int_2cpu_hiq_full;
	AGOT.aipc_int_2cpu_lowq_full	= aipc_int_2cpu_lowq_full;
}

void
aipc_dsp_intq_init( void )
{
	aipc_dsp_intq_got_init();
}

#else	//!_AIPC_CPU_ && !//_AIPC_DSP_
#error "not supported type"
#endif 
#endif

/*
*	CPU
*/
int	
aipc_int_2cpu_hiq_empty( void )
{
#ifdef AIPC_CACHE_FLUSH
	u32_t *p_del , *p_ins;
	p_del = &INT_2CPU_HIQ.del;
	p_ins = &INT_2CPU_HIQ.ins;
	
	apic_flush_dcache(p_del , p_del+sizeof(u32_t));
	apic_flush_dcache(p_ins , p_ins+sizeof(u32_t));
#endif

	if(((INT_2CPU_HIQ.ins + SIZE_2CPU_HIQ - INT_2CPU_HIQ.del)%SIZE_2CPU_HIQ) == 0){
		ADEBUG(DBG_INTQ , "empty\n");
		return TRUE;
		}
	else{
		ADEBUG(DBG_INTQ , "not empty\n");
		return FALSE;
		}
}
	
int
aipc_int_2cpu_lowq_empty( void )
{
#ifdef AIPC_CACHE_FLUSH
	u32_t *p_del , *p_ins;
	p_del = &INT_2CPU_LOWQ.del;
	p_ins = &INT_2CPU_LOWQ.ins;
	
	apic_flush_dcache(p_del , p_del+sizeof(u32_t));
	apic_flush_dcache(p_ins , p_ins+sizeof(u32_t));
#endif

	if(((INT_2CPU_LOWQ.ins + SIZE_2CPU_LOWQ - INT_2CPU_LOWQ.del)%SIZE_2CPU_LOWQ) == 0){
		ADEBUG(DBG_INTQ , "empty\n");
		return TRUE;
		}
	else{
		ADEBUG(DBG_INTQ , "not empty\n");
		return FALSE;
		}
}

int	
aipc_int_2dsp_hiq_full( void )
{
#ifdef AIPC_CACHE_FLUSH
	u32_t *p_del , *p_ins;
	p_del = &INT_2DSP_HIQ.del;
	p_ins = &INT_2DSP_HIQ.ins;
	
	apic_flush_dcache(p_del , p_del+sizeof(u32_t));
	apic_flush_dcache(p_ins , p_ins+sizeof(u32_t));
#endif

	if(((INT_2DSP_HIQ.del + SIZE_2DSP_HIQ - INT_2DSP_HIQ.ins)%SIZE_2DSP_HIQ) == 1){
		ADEBUG(DBG_INTQ , "full\n");
		return TRUE;
		}
	else{
		ADEBUG(DBG_INTQ , "not full\n");
		return FALSE;
		}
}

int
aipc_int_2dsp_lowq_full( void )
{
#ifdef AIPC_CACHE_FLUSH
	u32_t *p_del , *p_ins;
	p_del = &INT_2DSP_LOWQ.del;
	p_ins = &INT_2DSP_LOWQ.ins;
	
	apic_flush_dcache(p_del , p_del+sizeof(u32_t));
	apic_flush_dcache(p_ins , p_ins+sizeof(u32_t));
#endif

	if(((INT_2DSP_LOWQ.del + SIZE_2DSP_LOWQ - INT_2DSP_LOWQ.ins)%SIZE_2DSP_LOWQ) == 1){
		ADEBUG(DBG_INTQ , "full\n");
		return TRUE;
		}
	else{
		ADEBUG(DBG_INTQ , "not full\n");
		return FALSE;
		}
}

/*
*	DSP
*/

int 
aipc_int_2dsp_hiq_empty( void )
{
#ifdef AIPC_CACHE_FLUSH
	u32_t *p_del , *p_ins;
	p_del = &INT_2DSP_HIQ.del;
	p_ins = &INT_2DSP_HIQ.ins;
	
	apic_flush_dcache(p_del , p_del+sizeof(u32_t));
	apic_flush_dcache(p_ins , p_ins+sizeof(u32_t));
#endif
	if(((INT_2DSP_HIQ.ins + SIZE_2DSP_HIQ - INT_2DSP_HIQ.del)%SIZE_2DSP_HIQ) == 0){
		ADEBUG(DBG_INTQ , "empty\n");
		return TRUE;
		}
	else{
		ADEBUG(DBG_INTQ , "not empty\n");
		return FALSE;
		}
}

int
aipc_int_2dsp_lowq_empty( void )
{
#ifdef AIPC_CACHE_FLUSH
	u32_t *p_del , *p_ins;
	p_del = &INT_2DSP_LOWQ.del;
	p_ins = &INT_2DSP_LOWQ.ins;
	
	apic_flush_dcache(p_del , p_del+sizeof(u32_t));
	apic_flush_dcache(p_ins , p_ins+sizeof(u32_t));
#endif

	if(((INT_2DSP_LOWQ.ins + SIZE_2DSP_LOWQ - INT_2DSP_LOWQ.del)%SIZE_2DSP_LOWQ) == 0){
		ADEBUG(DBG_INTQ , "empty\n");
		return TRUE;
		}
	else{
		ADEBUG(DBG_INTQ , "not empty\n");
		return FALSE;
		}
}

int	
aipc_int_2cpu_hiq_full( void )
{
#ifdef AIPC_CACHE_FLUSH
	u32_t *p_del , *p_ins;
	p_del = &INT_2CPU_HIQ.del;
	p_ins = &INT_2CPU_HIQ.ins;
	
	apic_flush_dcache(p_del , p_del+sizeof(u32_t));
	apic_flush_dcache(p_ins , p_ins+sizeof(u32_t));
#endif

	if(((INT_2CPU_HIQ.del + SIZE_2CPU_HIQ - INT_2CPU_HIQ.ins)%SIZE_2CPU_HIQ) == 1){
		ADEBUG(DBG_INTQ , "full\n");
		return TRUE;
		}
	else{
		ADEBUG(DBG_INTQ , "not full\n");
		return FALSE;
		}
}

int
aipc_int_2cpu_lowq_full( void )
{
#ifdef AIPC_CACHE_FLUSH
	u32_t *p_del , *p_ins;
	p_del = &INT_2CPU_LOWQ.del;
	p_ins = &INT_2CPU_LOWQ.ins;
	
	apic_flush_dcache(p_del , p_del+sizeof(u32_t));
	apic_flush_dcache(p_ins , p_ins+sizeof(u32_t));
#endif

	if(((INT_2CPU_LOWQ.del + SIZE_2CPU_LOWQ - INT_2CPU_LOWQ.ins)%SIZE_2CPU_LOWQ) == 1){
		ADEBUG(DBG_INTQ , "full\n");
		return TRUE;
		}
	else{
		ADEBUG(DBG_INTQ , "not full\n");
		return FALSE;
		}
}

#ifdef __KERNEL__
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
int aipc_intq_dump(struct seq_file *f)
{
	int n = 0;

	seq_printf( f, "IntQ:\n");

	seq_printf( f, "   INT_2DSP_HIQ:");
    if(aipc_int_2dsp_hiq_empty()){
        seq_printf( f, "empty\n");
    }else if(aipc_int_2dsp_hiq_full()){
        seq_printf( f, "full\n");
    }else{
        seq_printf( f, "middle\n");
    }
	seq_printf( f, "\tindex ins=%d del=%d total=%d\n" , INT_2DSP_HIQ.ins ,  INT_2DSP_HIQ.del ,  SIZE_2DSP_HIQ);
	seq_printf( f, "\tcount ins=%d del=%d\n"          , INT_2DSP_HIQ.cnt_ins ,  INT_2DSP_HIQ.cnt_del);


	seq_printf( f, "   INT_2DSP_LOWQ:");
    if(aipc_int_2dsp_lowq_empty()){
        seq_printf( f, "empty\n");
    }else if(aipc_int_2dsp_lowq_full()){
        seq_printf( f, "full\n");
    }else{
        seq_printf( f, "middle\n");
    }
	seq_printf( f, "\tindex ins=%d del=%d total=%d\n" , INT_2DSP_LOWQ.ins , INT_2DSP_LOWQ.del , SIZE_2DSP_LOWQ);
	seq_printf( f, "\tcount ins=%d del=%d\n"          , INT_2DSP_LOWQ.cnt_ins , INT_2DSP_LOWQ.cnt_del);


	seq_printf( f, "   INT_2CPU_HIQ:");
    if(aipc_int_2cpu_hiq_empty()){
        seq_printf( f, "empty\n");
    }else if(aipc_int_2cpu_hiq_full()){
        seq_printf( f, "full\n");
    }else{
        seq_printf( f, "middle\n");
    }
	seq_printf( f, "\tindex ins=%d del=%d total=%d\n" , INT_2CPU_HIQ.ins ,  INT_2CPU_HIQ.del ,  SIZE_2CPU_HIQ);
	seq_printf( f, "\tcount ins=%d del=%d\n"          , INT_2CPU_HIQ.cnt_ins ,  INT_2CPU_HIQ.cnt_del);


	seq_printf( f, "   INT_2CPU_LOWQ:");
    if(aipc_int_2cpu_lowq_empty()){
        seq_printf( f, "empty\n");
    }else if(aipc_int_2cpu_lowq_full()){
        seq_printf( f, "full\n");
    }else{
        seq_printf( f, "middle\n");
    }
	seq_printf( f, "\tindex ins=%d del=%d total=%d\n" , INT_2CPU_LOWQ.ins , INT_2CPU_LOWQ.del , SIZE_2CPU_LOWQ);
	seq_printf( f, "\tcount ins=%d del=%d\n\n"        , INT_2CPU_LOWQ.cnt_ins , INT_2CPU_LOWQ.cnt_del);


	return n;
}

#else
int aipc_intq_dump(char *buf)
{
	int n = 0;

	n += sprintf(buf , "IntQ:\n");

	n += sprintf(buf+n , "   INT_2DSP_HIQ:");
    if(aipc_int_2dsp_hiq_empty()){
        n += sprintf(buf+n , "empty\n");
    }else if(aipc_int_2dsp_hiq_full()){
        n += sprintf(buf+n , "full\n");
    }else{
        n += sprintf(buf+n , "middle\n");
    }
	n += sprintf(buf+n , "\tindex ins=%d del=%d total=%d\n" , INT_2DSP_HIQ.ins ,  INT_2DSP_HIQ.del ,  SIZE_2DSP_HIQ);
	n += sprintf(buf+n , "\tcount ins=%d del=%d\n"          , INT_2DSP_HIQ.cnt_ins ,  INT_2DSP_HIQ.cnt_del);


	n += sprintf(buf+n , "   INT_2DSP_LOWQ:");
    if(aipc_int_2dsp_lowq_empty()){
        n += sprintf(buf+n , "empty\n");
    }else if(aipc_int_2dsp_lowq_full()){
        n += sprintf(buf+n , "full\n");
    }else{
        n += sprintf(buf+n , "middle\n");
    }
	n += sprintf(buf+n , "\tindex ins=%d del=%d total=%d\n" , INT_2DSP_LOWQ.ins , INT_2DSP_LOWQ.del , SIZE_2DSP_LOWQ);
	n += sprintf(buf+n , "\tcount ins=%d del=%d\n"          , INT_2DSP_LOWQ.cnt_ins , INT_2DSP_LOWQ.cnt_del);


	n += sprintf(buf+n , "   INT_2CPU_HIQ:");
    if(aipc_int_2cpu_hiq_empty()){
        n += sprintf(buf+n , "empty\n");
    }else if(aipc_int_2cpu_hiq_full()){
        n += sprintf(buf+n , "full\n");
    }else{
        n += sprintf(buf+n , "middle\n");
    }
	n += sprintf(buf+n , "\tindex ins=%d del=%d total=%d\n" , INT_2CPU_HIQ.ins ,  INT_2CPU_HIQ.del ,  SIZE_2CPU_HIQ);
	n += sprintf(buf+n , "\tcount ins=%d del=%d\n"          , INT_2CPU_HIQ.cnt_ins ,  INT_2CPU_HIQ.cnt_del);


	n += sprintf(buf+n , "   INT_2CPU_LOWQ:");
    if(aipc_int_2cpu_lowq_empty()){
        n += sprintf(buf+n , "empty\n");
    }else if(aipc_int_2cpu_lowq_full()){
        n += sprintf(buf+n , "full\n");
    }else{
        n += sprintf(buf+n , "middle\n");
    }
	n += sprintf(buf+n , "\tindex ins=%d del=%d total=%d\n" , INT_2CPU_LOWQ.ins , INT_2CPU_LOWQ.del , SIZE_2CPU_LOWQ);
	n += sprintf(buf+n , "\tcount ins=%d del=%d\n\n"        , INT_2CPU_LOWQ.cnt_ins , INT_2CPU_LOWQ.cnt_del);

	return n;
}
#endif
#endif



#ifdef _AIPC_CPU_
//static DEFINE_SPINLOCK(intq_lock);

/*
*	Init is done by CPU
*/

static void
aipc_int_2cpu_hiq_init( void )
{
	INT_2CPU_HIQ.ins = 0;
	INT_2CPU_HIQ.del = 0;
}

static void
aipc_int_2cpu_lowq_init( void )
{
	INT_2CPU_LOWQ.ins = 0;
	INT_2CPU_LOWQ.del = 0;
}


static void
aipc_int_2dsp_hiq_init( void )
{
	INT_2DSP_HIQ.ins = 0;
	INT_2DSP_HIQ.del = 0;
}

static void
aipc_int_2dsp_lowq_init( void )
{
	INT_2DSP_LOWQ.ins = 0;
	INT_2DSP_LOWQ.del = 0;
}


void
aipc_intq_init(void)
{
	aipc_int_2cpu_hiq_init();
	aipc_int_2cpu_lowq_init();
	aipc_int_2dsp_hiq_init();
	aipc_int_2dsp_lowq_init();
	#ifdef AIPC_GOT
	aipc_cpu_intq_got_init();
	#endif
}

/*
*	CPU
*/
int
aipc_int_2dsp_hiq_enqueue( u32_t int_id )
{
#ifdef AIPC_CACHE_FLUSH
	u32_t *p_del , *p_ins , *p_qe;
#endif

#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;
#endif

	int  ret=NOK;

#ifdef INTQ_LOCK
	AIPC_OSAL_INTERRUPT_FLAGS state;
	AIPC_OSAL_INTERRUPT_DISABLE(state);
	//spin_lock_irqsave( &intq_lock , state );  
#endif

#ifdef AIPC_CACHE_FLUSH
	p_del = &INT_2DSP_HIQ.del;
	p_ins = &INT_2DSP_HIQ.ins;

	apic_flush_dcache(p_del , p_del+sizeof(u32_t));
	apic_flush_dcache(p_ins , p_ins+sizeof(u32_t));
	p_qe  = &INT_2DSP_HIQ.hiq[INT_2DSP_HIQ.ins];
#endif

	if( !aipc_int_2dsp_hiq_full() ){
#ifdef AIPC_BARRIER
		tmp_wr = INT_2DSP_HIQ.ins;
		AMB( INT_2DSP_HIQ.hiq[ tmp_wr ] , int_id );
		
		tmp_wr = (tmp_wr+1) % SIZE_2DSP_HIQ;
		AMB( INT_2DSP_HIQ.ins , tmp_wr );
		
		#ifdef QUEUE_OPERATION_CNT
		tmp_wr = INT_2DSP_HIQ.cnt_ins;
		tmp_wr++;
		AMB( INT_2DSP_HIQ.cnt_ins , tmp_wr );
		#endif
#else
		INT_2DSP_HIQ.hiq[INT_2DSP_HIQ.ins] = int_id;
		INT_2DSP_HIQ.ins = (INT_2DSP_HIQ.ins+1)%SIZE_2DSP_HIQ;
		
		#ifdef QUEUE_OPERATION_CNT
		INT_2DSP_HIQ.cnt_ins++;
		#endif
#endif

#ifdef AIPC_CACHE_FLUSH
		apic_flush_dcache(p_qe  , p_qe +sizeof(u32_t));
		apic_flush_dcache(p_ins , p_ins+sizeof(u32_t));
#endif
		ret=OK;
		ADEBUG(DBG_INTQ , "enqueue %u OK\n" , int_id);
		}
	else{
		ret=NOK;
		ADEBUG(DBG_INTQ , "enqueue %u NOK\n" , int_id);
		}

#ifdef INTQ_LOCK
	AIPC_OSAL_INTERRUPT_ENABLE(state);
	//spin_unlock_irqrestore( &intq_lock , state );
#endif

	return ret;
}

int
aipc_int_2cpu_hiq_dequeue( u32_t *fid )
{
	int  ret=NOK;

#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;
#endif

#ifdef INTQ_LOCK
	AIPC_OSAL_INTERRUPT_FLAGS state;
	AIPC_OSAL_INTERRUPT_DISABLE(state);
	//spin_lock_irqsave( &intq_lock , state );	
#endif


#ifdef AIPC_CACHE_FLUSH
	u32_t *p_del , *p_ins , *p_qe;
	p_del = &INT_2CPU_HIQ.del;
	p_ins = &INT_2CPU_HIQ.ins;
		
	apic_flush_dcache(p_del , p_del+sizeof(u32_t));
	apic_flush_dcache(p_ins , p_ins+sizeof(u32_t));
	p_qe  = &INT_2CPU_HIQ.hiq[INT_2CPU_HIQ.del];
#endif

	if( !aipc_int_2cpu_hiq_empty() ){
#ifdef AIPC_BARRIER
		tmp_wr =  INT_2CPU_HIQ.del;
		*fid = INT_2CPU_HIQ.hiq[ tmp_wr ];
		tmp_wr =  (INT_2CPU_HIQ.del+1) % SIZE_2CPU_HIQ;
		 AMB(  INT_2CPU_HIQ.del , tmp_wr );

		#ifdef QUEUE_OPERATION_CNT
		tmp_wr = INT_2CPU_HIQ.cnt_del;
		tmp_wr++;
		AMB( INT_2CPU_HIQ.cnt_del , tmp_wr );
		#endif
#else
		*fid = INT_2CPU_HIQ.hiq[ INT_2CPU_HIQ.del ];
		INT_2CPU_HIQ.del = (INT_2CPU_HIQ.del+1)%SIZE_2CPU_HIQ;

		#ifdef QUEUE_OPERATION_CNT
		INT_2CPU_HIQ.cnt_del++;
		#endif
#endif
		
#ifdef AIPC_CACHE_FLUSH
		apic_flush_dcache(p_qe	, p_qe +sizeof(u32_t));
		apic_flush_dcache(p_del , p_del+sizeof(u32_t));
#endif
		ret = OK;
		ADEBUG(DBG_INTQ , "dequeue %u OK\n" , *fid);
		}
	else{
		ret = NOK;
		ADEBUG(DBG_INTQ , "dequeue %u NOK\n" , *fid);
		}

#ifdef INTQ_LOCK
	AIPC_OSAL_INTERRUPT_ENABLE(state);
	//spin_unlock_irqrestore( &intq_lock , state );
#endif

	return ret;

}


int
aipc_int_2dsp_lowq_enqueue( u32_t int_id )
{
#ifdef AIPC_CACHE_FLUSH
	u32_t *p_del , *p_ins , *p_qe;
#endif
	int  ret=NOK;
	
#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;
#endif

#ifdef INTQ_LOCK
	AIPC_OSAL_INTERRUPT_FLAGS state;
	AIPC_OSAL_INTERRUPT_DISABLE(state);
	//spin_lock_irqsave( &intq_lock , state ); 
#endif
	
#ifdef AIPC_CACHE_FLUSH
	p_del = &INT_2DSP_LOWQ.del;
	p_ins = &INT_2DSP_LOWQ.ins;
		
	apic_flush_dcache(p_del , p_del+sizeof(u32_t));
	apic_flush_dcache(p_ins , p_ins+sizeof(u32_t));
	p_qe  = &INT_2DSP_LOWQ.lowq[INT_2DSP_LOWQ.ins];
#endif
	if( !aipc_int_2dsp_lowq_full() ){
#ifdef AIPC_BARRIER
		tmp_wr = INT_2DSP_LOWQ.ins;
		AMB( INT_2DSP_LOWQ.lowq[ tmp_wr ] , int_id );
		tmp_wr = (tmp_wr+1) % SIZE_2DSP_LOWQ;
		AMB( INT_2DSP_LOWQ.ins , tmp_wr );

		#ifdef QUEUE_OPERATION_CNT
		tmp_wr = INT_2DSP_LOWQ.cnt_ins;
		tmp_wr++;
		AMB( INT_2DSP_LOWQ.cnt_ins , tmp_wr );
		#endif
#else		
		INT_2DSP_LOWQ.lowq[INT_2DSP_LOWQ.ins] = int_id;
		INT_2DSP_LOWQ.ins = (INT_2DSP_LOWQ.ins+1)%SIZE_2DSP_LOWQ;

		#ifdef QUEUE_OPERATION_CNT
		INT_2DSP_LOWQ.cnt_ins++;
		#endif
#endif

#ifdef AIPC_CACHE_FLUSH
		apic_flush_dcache(p_qe	, p_qe +sizeof(u32_t));
		apic_flush_dcache(p_ins , p_ins+sizeof(u32_t));
#endif
		ret=OK;
		ADEBUG(DBG_INTQ , "enqueue %u OK\n" , int_id);
		}
	else{
		ret=NOK;
		ADEBUG(DBG_INTQ , "enqueue %u NOK\n" , int_id);
		}

#ifdef INTQ_LOCK
	AIPC_OSAL_INTERRUPT_ENABLE(state);
	//spin_unlock_irqrestore( &intq_lock , state );
#endif

	return ret;	
}

int
aipc_int_2cpu_lowq_dequeue( u32_t *fid )
{
	int  ret=NOK;
	
#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;
#endif
	
#ifdef INTQ_LOCK
	AIPC_OSAL_INTERRUPT_FLAGS state;
	AIPC_OSAL_INTERRUPT_DISABLE(state);
	//spin_lock_irqsave( &intq_lock , state ); 
#endif

#ifdef AIPC_CACHE_FLUSH
	u32_t *p_del , *p_ins , *p_qe;
	p_del = &INT_2CPU_LOWQ.del;
	p_ins = &INT_2CPU_LOWQ.ins;
		
	apic_flush_dcache(p_del , p_del+sizeof(u32_t));
	apic_flush_dcache(p_ins , p_ins+sizeof(u32_t));
	p_qe  = &INT_2CPU_LOWQ.lowq[INT_2CPU_LOWQ.del];
#endif
	if( !aipc_int_2cpu_lowq_empty() ){
#ifdef AIPC_BARRIER
		tmp_wr = INT_2CPU_LOWQ.del;
		*fid = INT_2CPU_LOWQ.lowq[ tmp_wr ];
		tmp_wr = (tmp_wr+1) % SIZE_2CPU_LOWQ;
		AMB( INT_2CPU_LOWQ.del , tmp_wr );

		#ifdef QUEUE_OPERATION_CNT
		tmp_wr = INT_2CPU_LOWQ.cnt_del;
		tmp_wr++;
		AMB( INT_2CPU_LOWQ.cnt_del , tmp_wr );
		#endif
#else
		*fid = INT_2CPU_LOWQ.lowq[ INT_2CPU_LOWQ.del ];
		INT_2CPU_LOWQ.del = (INT_2CPU_LOWQ.del+1)%SIZE_2CPU_LOWQ;

		#ifdef QUEUE_OPERATION_CNT
		INT_2CPU_LOWQ.cnt_del++;
		#endif
#endif

#ifdef AIPC_CACHE_FLUSH
		apic_flush_dcache(p_qe	, p_qe +sizeof(u32_t));
		apic_flush_dcache(p_del , p_del+sizeof(u32_t));
#endif
		ret = OK;
		ADEBUG(DBG_INTQ , "dequeue %u OK\n" , *fid);
		}
	else{
		ret = NOK;
		ADEBUG(DBG_INTQ , "dequeue %u NOK\n" , *fid);
		}

#ifdef INTQ_LOCK
	AIPC_OSAL_INTERRUPT_ENABLE(state);
	//spin_unlock_irqrestore( &intq_lock , state );
#endif

	return ret; 
}


#elif defined(_AIPC_DSP_)
/*
*	DSP
*/

int
aipc_int_2cpu_hiq_enqueue( u32_t int_id )
{
#ifdef AIPC_CACHE_FLUSH
	u32_t *p_del , *p_ins , *p_qe;
#endif

	int ret=NOK;
	
#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;
#endif

#ifdef INTQ_LOCK
	AIPC_OSAL_INTERRUPT_FLAGS _old;
	AIPC_OSAL_INTERRUPT_DISABLE(_old);
#endif

#ifdef AIPC_CACHE_FLUSH
	p_del = &INT_2CPU_HIQ.del;
	p_ins = &INT_2CPU_HIQ.ins;
		
	apic_flush_dcache(p_del , p_del+sizeof(u32_t));
	apic_flush_dcache(p_ins , p_ins+sizeof(u32_t));
	p_qe  = &INT_2CPU_HIQ.hiq[INT_2CPU_HIQ.ins];
#endif

	if( !aipc_int_2cpu_hiq_full() ){
#ifdef AIPC_BARRIER
		tmp_wr = INT_2CPU_HIQ.ins;
		AMB( INT_2CPU_HIQ.hiq[ tmp_wr ] , int_id );
		
		tmp_wr = (tmp_wr+1) % SIZE_2CPU_HIQ;
		AMB( INT_2CPU_HIQ.ins , tmp_wr );

		#ifdef QUEUE_OPERATION_CNT
		tmp_wr = INT_2CPU_HIQ.cnt_ins;
		tmp_wr++;
		AMB( INT_2CPU_HIQ.cnt_ins , tmp_wr );
		#endif
#else		
		INT_2CPU_HIQ.hiq[INT_2CPU_HIQ.ins] = int_id;
		INT_2CPU_HIQ.ins = (INT_2CPU_HIQ.ins+1)%SIZE_2CPU_HIQ;

		#ifdef QUEUE_OPERATION_CNT
		INT_2CPU_HIQ.cnt_ins++;
		#endif
#endif

#ifdef AIPC_CACHE_FLUSH
		apic_flush_dcache(p_qe	, p_qe +sizeof(u32_t));
		apic_flush_dcache(p_ins , p_ins+sizeof(u32_t));
#endif
		ret=OK;
		ADEBUG(DBG_INTQ , "enqueue OK\n");
		}
	else{
		ret=NOK;
		ADEBUG(DBG_INTQ , "enqueue NOK\n");
		}
	
#ifdef INTQ_LOCK
	AIPC_OSAL_INTERRUPT_ENABLE(_old);
#endif
	return ret;
}

int
aipc_int_2dsp_hiq_dequeue( u32_t *fid )
{
#ifdef AIPC_CACHE_FLUSH
	u32_t *p_del , *p_ins , *p_qe;
#endif

	int ret=NOK;
	
#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;
#endif

#ifdef INTQ_LOCK
	AIPC_OSAL_INTERRUPT_FLAGS _old;
	AIPC_OSAL_INTERRUPT_DISABLE(_old);
#endif

#ifdef AIPC_CACHE_FLUSH
	p_del = &INT_2DSP_HIQ.del;
	p_ins = &INT_2DSP_HIQ.ins;
				
	apic_flush_dcache(p_del , p_del+sizeof(u32_t));
	apic_flush_dcache(p_ins , p_ins+sizeof(u32_t));
	p_qe  = &INT_2DSP_HIQ.hiq[INT_2DSP_HIQ.ins];
#endif

	if( !aipc_int_2dsp_hiq_empty() ){
#ifdef AIPC_BARRIER
		tmp_wr = INT_2DSP_HIQ.del;
		*fid   = INT_2DSP_HIQ.hiq[ tmp_wr ];
		tmp_wr = (tmp_wr+1) % SIZE_2DSP_HIQ;
		AMB( INT_2DSP_HIQ.del , tmp_wr );

		#ifdef QUEUE_OPERATION_CNT
		tmp_wr = INT_2DSP_HIQ.cnt_del;
		tmp_wr++;
		AMB( INT_2DSP_HIQ.cnt_del , tmp_wr );
		#endif
#else
		*fid = INT_2DSP_HIQ.hiq[ INT_2DSP_HIQ.del ];
		INT_2DSP_HIQ.del = (INT_2DSP_HIQ.del+1)%SIZE_2DSP_HIQ;

		#ifdef QUEUE_OPERATION_CNT
		INT_2DSP_HIQ.cnt_del++;
		#endif
#endif

#ifdef AIPC_CACHE_FLUSH
		apic_flush_dcache(p_qe	, p_qe +sizeof(u32_t));
		apic_flush_dcache(p_del , p_del+sizeof(u32_t));
#endif
		ret = OK;
		ADEBUG(DBG_INTQ , "dequeue OK\n");
		}
	else{
		ret = NOK;
		ADEBUG(DBG_INTQ , "dequeue NOK\n");
		}
	
#ifdef INTQ_LOCK
	AIPC_OSAL_INTERRUPT_ENABLE(_old);
#endif

	return ret;
}

int
aipc_int_2cpu_lowq_enqueue( u32_t int_id )
{
#ifdef AIPC_CACHE_FLUSH
	u32_t *p_del , *p_ins , *p_qe;
#endif

	int ret=NOK;

#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;
#endif

#ifdef INTQ_LOCK
	AIPC_OSAL_INTERRUPT_FLAGS _old;
	AIPC_OSAL_INTERRUPT_DISABLE(_old);
#endif

#ifdef AIPC_CACHE_FLUSH
	p_del = &INT_2CPU_LOWQ.del;
	p_ins = &INT_2CPU_LOWQ.ins;
			
	apic_flush_dcache(p_del , p_del+sizeof(u32_t));
	apic_flush_dcache(p_ins , p_ins+sizeof(u32_t));
	p_qe  = &INT_2CPU_LOWQ.lowq[INT_2CPU_LOWQ.ins];
#endif

	if( !aipc_int_2cpu_lowq_full() ){
#ifdef AIPC_BARRIER
		tmp_wr = INT_2CPU_LOWQ.ins;
		AMB( INT_2CPU_LOWQ.lowq[ tmp_wr ] , int_id );
		
		tmp_wr = (tmp_wr+1)%SIZE_2CPU_LOWQ;
		AMB( INT_2CPU_LOWQ.ins , tmp_wr );

		#ifdef QUEUE_OPERATION_CNT
		tmp_wr = INT_2CPU_LOWQ.cnt_ins;
		tmp_wr++;
		AMB( INT_2CPU_LOWQ.cnt_ins , tmp_wr );
		#endif
#else
		INT_2CPU_LOWQ.lowq[INT_2CPU_LOWQ.ins] = int_id;
		INT_2CPU_LOWQ.ins = (INT_2CPU_LOWQ.ins+1)%SIZE_2CPU_LOWQ;

		#ifdef QUEUE_OPERATION_CNT
		INT_2CPU_LOWQ.cnt_ins++;
		#endif
#endif

#ifdef AIPC_CACHE_FLUSH
		apic_flush_dcache(p_qe	, p_qe +sizeof(u32_t));
		apic_flush_dcache(p_ins , p_ins+sizeof(u32_t));
#endif
		ret=OK;
		ADEBUG(DBG_INTQ , "enqueue OK\n");
		}	
	else{
		ret=NOK;
		ADEBUG(DBG_INTQ , "enqueue NOK\n");
		}
	
#ifdef INTQ_LOCK
	AIPC_OSAL_INTERRUPT_ENABLE(_old);
#endif

	return ret;
}

int
aipc_int_2dsp_lowq_dequeue( u32_t *fid )
{
#ifdef AIPC_CACHE_FLUSH
	u32_t *p_del , *p_ins , *p_qe;
#endif

	int ret=NOK;
	
#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;
#endif

#ifdef INTQ_LOCK
	AIPC_OSAL_INTERRUPT_FLAGS _old;
	AIPC_OSAL_INTERRUPT_DISABLE(_old);
#endif

#ifdef AIPC_CACHE_FLUSH
	p_del = &INT_2DSP_LOWQ.del;
	p_ins = &INT_2DSP_LOWQ.ins;
		
	apic_flush_dcache(p_del , p_del+sizeof(u32_t));
	apic_flush_dcache(p_ins , p_ins+sizeof(u32_t));
	p_qe  = &INT_2DSP_LOWQ.lowq[INT_2DSP_LOWQ.del];
#endif

	if( !aipc_int_2dsp_lowq_empty() ){
#ifdef AIPC_BARRIER
		tmp_wr = INT_2DSP_LOWQ.del;
		*fid   = INT_2DSP_LOWQ.lowq[ tmp_wr ];
		tmp_wr = (tmp_wr+1)%SIZE_2DSP_LOWQ;
		AMB( INT_2DSP_LOWQ.del , tmp_wr );

		#ifdef QUEUE_OPERATION_CNT
		tmp_wr = INT_2DSP_LOWQ.cnt_del;
		tmp_wr++;
		AMB( INT_2DSP_LOWQ.cnt_del , tmp_wr );
		#endif
#else
		*fid = INT_2DSP_LOWQ.lowq[ INT_2DSP_LOWQ.del ];
		INT_2DSP_LOWQ.del = (INT_2DSP_LOWQ.del+1)%SIZE_2DSP_LOWQ;

		#ifdef QUEUE_OPERATION_CNT
		INT_2DSP_LOWQ.cnt_del++;
		#endif
#endif

#ifdef AIPC_CACHE_FLUSH
		apic_flush_dcache(p_qe	, p_qe +sizeof(u32_t));
		apic_flush_dcache(p_del , p_del+sizeof(u32_t));
#endif
		ret = OK;
		ADEBUG(DBG_INTQ , "dequeue OK\n");
		}
	else{
		ret = NOK;
		ADEBUG(DBG_INTQ , "dequeue NOK\n");
		}

#ifdef INTQ_LOCK
	AIPC_OSAL_INTERRUPT_ENABLE(_old);
#endif

	return ret;
}

#else	//!_AIPC_CPU_ && !//_AIPC_DSP_
#error "not supported type"
#endif 


