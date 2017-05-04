#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/errno.h>	
#include <linux/types.h>
#include <linux/version.h>
#include <linux/spinlock.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>  	
#include <linux/interrupt.h>	
#include <linux/irqreturn.h>	
#include <linux/ioctl.h>
#include <linux/version.h>

//#include <asm/system.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#elif defined(__ECOS)
#include <cyg/infra/cyg_type.h>
#include <cyg/kernel/kapi.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/io.h>
#include <cyg/io/devtab.h>


#endif

#include "./include/soc_type.h"
#include "./include/aipc_osal.h"
#include "./include/aipc_irq.h"
#include "./include/aipc_intq.h"
#include "./include/aipc_ioctl.h"
#include "./include/aipc_mbox.h"
#include "./include/aipc_debug.h"


/*
*	CPU and DSP common code
*/

void ( *voip_dsp_L2_pkt_rx_trap )(unsigned char* eth_pkt, unsigned long size) = NULL;   // pkshih: eth_pkt content may be modified!!
void voip_dsp_L2_pkt_test(unsigned char* eth_pkt, unsigned long size);

void ( *voip_dsp_L2_pkt_event_trap )(unsigned char* eth_pkt, unsigned long size) = NULL;   // pkshih: eth_pkt content may be modified!!
void voip_dsp_L2_pkt_event_test(unsigned char* eth_pkt, unsigned long size);


#ifdef _AIPC_CPU_
void aipc_cpu_voip_reg_cbk(void);
#else
void aipc_dsp_voip_reg_cbk(void);
#endif

#ifdef _AIPC_CPU_

/*
*	Structure
*/
struct aipc_int_priv{
	char name[INT_NAME_SIZE];
 	struct tasklet_struct	rx_tasklet;
	struct tasklet_struct	event_tasklet;
};

static struct aipc_int_priv aipc_int_priv_dsc;

static irqreturn_t aipc_cpu_ISR( int irq , void *data );
#ifdef CPU_DSR_SUPPORT
void aipc_2cpu_rx_tasklet(unsigned long task_priv);
void aipc_2cpu_event_tasklet(unsigned long task_priv);
#endif

#ifdef CONFIG_RTL8686_IPC_DSP_CONSOLE
#ifdef AIPC_DSP_CONSOLE_USE_TASKLET
	extern void aipc_dsp_console_wakeup_tasklet(void);
#endif
#ifdef AIPC_DSP_CONSOLE_USE_WORKQUEUE
	extern void aipc_dsp_console_wakeup_workqueue(void);
#endif
#endif

irqreturn_t 
aipc_cpu_interrupt(int irq , void *dev_id)
{
	irqreturn_t ret = IRQ_NONE;

#ifdef INT_COUNT
	ASTATS.dsp_t_cpu++;
#endif

	ret = aipc_cpu_ISR(irq , dev_id);

	return ret;
}

int 
aipc_cpu_irq_init(void)	
{
	struct aipc_int_priv* priv = &aipc_int_priv_dsc;
	char strbuf[INT_NAME_SIZE]={0};
	int result=0;

	printk("aipc: start aipc irq init\n");

	memset(priv , 0 , sizeof(struct aipc_int_priv));
	aipc_cpu_voip_reg_cbk();


	sprintf(strbuf, "aipc_irq");	
	strncpy(priv->name , strbuf , strlen(strbuf));

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	#if 0
		result = request_irq(AIPC_IRQ_DSP_T_CPU , aipc_cpu_interrupt , IRQF_DISABLED , 	
				priv->name, priv);// NULL OK
	#else
		result = request_irq(AIPC_IRQ_DSP_T_CPU , aipc_cpu_interrupt , IRQF_DISABLED | IRQF_PERCPU , 	
				priv->name, priv);// NULL OK
	#endif
#else
		result = request_irq(AIPC_IRQ_DSP_T_CPU , aipc_cpu_interrupt , SA_INTERRUPT , 
				priv->name, priv);// NULL OK
#endif

	if (result){
		printk("aipc: Can't request IRQ for IPC\n");
	}
	else{
		printk("aipc: Request IRQ for IPC OK\n");
	}

#ifdef CPU_DSR_SUPPORT
	tasklet_init(&priv->rx_tasklet   , aipc_2cpu_rx_tasklet    , (unsigned long)priv);
	tasklet_init(&priv->event_tasklet, aipc_2cpu_event_tasklet , (unsigned long)priv);
#endif

	return result;
}

//static DEFINE_SPINLOCK(irq_2dsp_lock);

/*******************************************************************************
*	From CPU to DSP		Linux Send function
*******************************************************************************/

int
aipc_int_send_2dsp( u32_t int_id )		//only use 64 
{
	int retval = NOK;
	volatile u32_t  add_hiq = 0;

	#ifdef IRQ_LOCK
	AIPC_OSAL_INTERRUPT_FLAGS state;
	#endif

	add_hiq = !(int_id & 0x01);	//add even to hi, odd to low queue

	//SDEBUG("send %d\n" , int_id);

	if( int_id >= AIPC_MGR_MAX ) 
		return retval;

#ifdef IRQ_LOOP_PROTECT

	if( add_hiq ){
		while(1){
			#ifdef IRQ_LOCK
			AIPC_OSAL_INTERRUPT_DISABLE(state);
			//spin_lock_irqsave( &irq_2dsp_lock , state ); 
			#endif

			if( aipc_int_2dsp_hiq_full() ){
					#ifdef IRQ_LOCK
					AIPC_OSAL_INTERRUPT_ENABLE(state);
					//spin_unlock_irqrestore( &irq_2dsp_lock , state );
					#endif
					continue;			
				}
				else{
					aipc_int_2dsp_hiq_enqueue( int_id );
					aipc_int_assert( T_DSP );
					retval = OK;
					#ifdef IRQ_LOCK
					AIPC_OSAL_INTERRUPT_ENABLE(state);
					//spin_unlock_irqrestore( &irq_2dsp_lock , state );
					#endif
					
					break;
				}
			}		
	}
	else {
		while(1){
			#ifdef IRQ_LOCK
			AIPC_OSAL_INTERRUPT_DISABLE(state);
			//spin_lock_irqsave( &irq_2dsp_lock , state );
			#endif
			
			if( aipc_int_2dsp_lowq_full() ){
				#ifdef IRQ_LOCK
				AIPC_OSAL_INTERRUPT_ENABLE(state);
				//spin_unlock_irqrestore( &irq_2dsp_lock , state );
				#endif
				continue;			
			}
			else{
				aipc_int_2dsp_lowq_enqueue( int_id );
				aipc_int_assert( T_DSP );
				retval = OK;
				#ifdef IRQ_LOCK
				AIPC_OSAL_INTERRUPT_ENABLE(state);
				//spin_unlock_irqrestore( &irq_2dsp_lock , state );
				#endif
				
				break;
			}
		}
	}

#else

	if( add_hiq ){
		if( !aipc_int_2dsp_hiq_full() ){
			aipc_int_2dsp_hiq_enqueue( int_id );
			aipc_int_assert( T_DSP );
			retval = OK;
		}		
	}
	else {
		if( !aipc_int_2dsp_lowq_full() ){
			aipc_int_2dsp_lowq_enqueue( int_id );
			aipc_int_assert( T_DSP );
			retval = OK;
		}
	}

#endif

	return retval;	
}

/*******************************************************************************
*	From DSP to CPU		Linux ISR function
*******************************************************************************/
irqreturn_t 
aipc_cpu_ISR( int irq , void *data )
{
	int n = MAX_HANDLE_CNT;
	u32_t fid=0;

	#ifndef CONFIG_RTL8686_SHM_NOTIFY
	volatile u32_t status=0;
	#endif

#if defined(IRQ_LOOP_PROTECT)

	aipc_int_mask( irq );
	aipc_int_deassert( T_CPU );

	while(1) {
		if( !aipc_int_2cpu_hiq_empty() ){	//handle elements in hi queue
#ifdef INT_QUEUE_FLUSH
			for(n=MAX_HANDLE_CNT ; n>0 ; n--)
#else
			for( ; n>0 ; n-- )
#endif
			{
				if( !aipc_int_2cpu_hiq_empty() ){  //For hi queue	
					//Get element from hi queue
					if( OK == aipc_int_2cpu_hiq_dequeue( &fid ))
						aipc_exe_callback( fid , data );  //Run callback
					else
						break;
				}
			}
		}

		if( !aipc_int_2cpu_lowq_empty() ){	//handle elements in low queue
#ifdef INT_QUEUE_FLUSH
			for(n=MAX_HANDLE_CNT ; n>0 ; n--)
#else
		#ifdef SCAN_INT_LOW_QUEUE
			if(n<=0){ 
				n=SIZE_2CPU_LOWQ/2;
			}
		#endif
			for( ; n>0 ; n-- )
#endif
			{
				if( !aipc_int_2cpu_lowq_empty() ){  //For low queue	
					//Get element from low queue
					if( OK == aipc_int_2cpu_lowq_dequeue( &fid ) )
						aipc_exe_callback( fid , data );  //Run callback
					else
						break;
				}
			}
		}

	#ifdef CONFIG_RTL8686_SHM_NOTIFY
		break;
	#else
		status = REG32(R_GISR0_0);

		if( status & DSP_T_CPU_IP )
			aipc_int_deassert( T_CPU );
		else
			break;
	#endif
	}

	aipc_int_unmask( irq );

#else

	aipc_int_mask( irq );
	aipc_int_deassert( T_CPU );

	for( ; n>0 ; n-- )
	{
		if( !aipc_int_2cpu_hiq_empty() ){  //For hi queue	
			//Get element from hi queue
			if( OK == aipc_int_2cpu_hiq_dequeue( &fid ))
				aipc_exe_callback( fid , data );  //Run callback
			else
				break;
		}
	}

	for( ; n>0 ; n-- )
	{
		if( !aipc_int_2cpu_lowq_empty() ){	//For low queue 
			//Get element from low queue
			if( OK == aipc_int_2cpu_lowq_dequeue( &fid ) )
				aipc_exe_callback( fid , data );  //Run callback
			else
				break;
		}
	}

	status = REG32(R_GISR0_0);

	if( status & DSP_T_CPU_IP )
		aipc_int_deassert( T_CPU );

	aipc_int_unmask( irq );

#endif

#ifdef CONFIG_RTL8686_IPC_DSP_CONSOLE
#ifdef AIPC_DSP_CONSOLE_USE_TASKLET
	aipc_dsp_console_wakeup_tasklet();
#endif
#ifdef AIPC_DSP_CONSOLE_USE_WORKQUEUE
	aipc_dsp_console_wakeup_workqueue();
#endif
#endif

	return IRQ_HANDLED;
}



#ifdef CPU_DSR_SUPPORT 	//recieve packet in DSR
void aipc_2cpu_rx_tasklet(unsigned long task_priv)
{
	//struct aipc_int_priv *priv = (struct aipc_int_priv *)task_priv;
	void *mp = NULL;
	int i = 0;
	
	ADEBUG(DBG_CONT , "trigger 2cpu rx tasklet.\n");
#ifdef CPU_DSR_RX_CNT
	for(i=0 ; i< CPU_DSR_RX_CNT ; i++){	
		mp = aipc_data_2cpu_recv();
		 
		if(mp){
			if (voip_dsp_L2_pkt_rx_trap){
				(*voip_dsp_L2_pkt_rx_trap )((unsigned char *)mp, MAIL_2CPU_SIZE) ;
				aipc_data_2cpu_ret(mp);
				}
			}
		else{
			//if(i==0)
			//	printk("more data tasklet %s(%d)\n" , __FUNCTION__ , __LINE__ );
			//else
			//	printk("data tasklet i=%d %s(%d)\n" , i , __FUNCTION__ , __LINE__ );
			
			break;
		}
	}
#else
	mp = aipc_data_2cpu_recv();
		 
	if(mp){
		if (voip_dsp_L2_pkt_rx_trap){
			(*voip_dsp_L2_pkt_rx_trap )((unsigned char *)mp, MAIL_2CPU_SIZE) ;
			aipc_data_2cpu_ret(mp);
			}
		}
#endif
}


void aipc_2cpu_event_tasklet(unsigned long task_priv)
{
	//struct aipc_int_priv *priv = (struct aipc_int_priv *)task_priv;
	void *mp = NULL;
	int i = 0;

	ADEBUG(DBG_CONT , "trigger 2cpu event tasklet.\n");

#ifdef CPU_DSR_RX_CNT
	for(i=0 ; i< CPU_DSR_RX_CNT ; i++){	
		mp = aipc_ctrl_2cpu_recv();

		if(mp){
			if (voip_dsp_L2_pkt_event_trap){
				(*voip_dsp_L2_pkt_event_trap)((unsigned char *)mp, EVENT_SIZE);
				aipc_ctrl_2cpu_ret(mp);
				}
			}
		else{
			//if(i==0)
			//	printk("more event tasklet %s(%d)\n" , __FUNCTION__ , __LINE__ );
			//else
			//	printk("event tasklet i=%d %s(%d)\n" , i , __FUNCTION__ , __LINE__ );

			break;
		}
	}
#else
	mp = aipc_ctrl_2cpu_recv();

	if(mp){
		if (voip_dsp_L2_pkt_event_trap){
			(*voip_dsp_L2_pkt_event_trap)((unsigned char *)mp, EVENT_SIZE);
			aipc_ctrl_2cpu_ret(mp);
			}
		}
#endif
}


#endif

void aipc_cpu_voip_reg_cbk(void)
{
	voip_dsp_L2_pkt_rx_trap    = voip_dsp_L2_pkt_test;
	voip_dsp_L2_pkt_event_trap = voip_dsp_L2_pkt_event_test;
}

#elif defined(_AIPC_DSP_)
//static cyg_uint32
//aipc_dsp_ISR(cyg_vector_t vector , cyg_addrword_t data);
AIPC_OSAL_ISR_RET_TYPE
aipc_dsp_ISR( int vector , void *data );

//static void
//aipc_dsp_DSR(cyg_vector_t vector , cyg_ucount32 count, cyg_addrword_t data);
void aipc_2dsp_rx_tasklet(unsigned long task_priv);
void aipc_2dsp_event_tasklet(unsigned long task_priv);

#if 0
cyg_uint32 aipc_dsp_interrupt( cyg_vector_t vector, cyg_addrword_t data )
#else
AIPC_OSAL_ISR_RET_TYPE aipc_dsp_interrupt(int irq , void *dev_id)
#endif
{
	AIPC_OSAL_ISR_RET_TYPE ret;

#ifdef INT_COUNT
	ASTATS.cpu_t_dsp++;
#endif

	ret = aipc_dsp_ISR( irq , dev_id );

    return ret;
}

void
aipc_dsp_irq_init(void)
{
	//aipc_dsp_voip_reg_cbk();	
}

#ifdef __ECOS
/*
*	Interrupt Structure Define
*/
#if 0
typedef struct aipc_int_info {
    CYG_ADDRWORD   base;
    CYG_WORD       int_num;
	cyg_priority_t int_prio;
    cyg_handle_t   interrupt_handle; 
    cyg_interrupt  interrupt;
} aipc_int_info_t;



/*
*	Variable Define
*/
//static aipc_int_info_t dsp_int_info;

#if 0
extern cyg_devio_table_t cyg_io_serial_devio;
static bool aipc_dsp_irq_init(struct cyg_devtab_entry *tab);
static Cyg_ErrNo aipc_dsp_dev_lookup(struct cyg_devtab_entry **tab,
                  struct cyg_devtab_entry *sub_tab,
                  const char *name);

#if 0
DEVIO_TABLE(   aipc_dsp_devio,
	           cyg_devio_cwrite,
	           cyg_devio_cread,
	           cyg_devio_select,
	           cyg_devio_get_config,
	           cyg_devio_set_config                   
    );
#endif

DEVTAB_ENTRY(    aipc_dsp_dev_entry0,
	             "/dev/aipc_dev",
	             0,
	             &cyg_io_serial_devio,
	             aipc_dsp_irq_init, 
	             aipc_dsp_dev_lookup,
	             &dsp_int_info
    );

static Cyg_ErrNo
aipc_dsp_dev_lookup(struct cyg_devtab_entry **tab,
                  struct cyg_devtab_entry *sub_tab,
                  const char *name)
{
	aipc_dsp_irq_init(NULL);
	return ENOERR;
}
#endif


#if 0
static cyg_uint32 
aipc_dsp_interrupt( cyg_vector_t vector, cyg_addrword_t data )
#else
AIPC_OSAL_ISR_RET_TYPE
aipc_dsp_interrupt(int irq , void *dev_id)
#endif
{
	AIPC_OSAL_ISR_RET_TYPE ret;

#ifdef INT_COUNT
	ASTATS.cpu_t_dsp++;
#endif

	ret = aipc_dsp_ISR( vector , data );

    return ret;
}

static bool
aipc_dsp_irq_init(struct cyg_devtab_entry *tab)
{
	aipc_int_info_t *int_info = &dsp_int_info;

	if (int_info==NULL)
		return NOK;

	int_info->int_num 	= AIPC_IRQ_CPU_T_DSP;
	int_info->int_prio	= AIPC_INT_PRIORITY;
	
	printk("%d %s int_num=%d int_prio=%d\n" , __LINE__ , __FUNCTION__ ,
		int_info->int_num,	int_info->int_prio);

	aipc_dsp_voip_reg_cbk();

#ifdef DSP_DSR_SUPPORT
	cyg_drv_interrupt_create(
		int_info->int_num,
		int_info->int_prio,
		(cyg_addrword_t)int_info,
		aipc_dsp_interrupt,
		aipc_dsp_DSR,
		&int_info->interrupt_handle,
		&int_info->interrupt);
#else
	cyg_drv_interrupt_create(
		int_info->int_num,
		int_info->int_prio,
		(cyg_addrword_t)int_info,
		aipc_dsp_interrupt,
		NULL,
		&int_info->interrupt_handle,
		&int_info->interrupt);
#endif
		
	 cyg_drv_interrupt_attach(int_info->interrupt_handle);
	 cyg_drv_interrupt_unmask(int_info->int_num);

	 return true;
}
#endif

#endif

/*******************************************************************************
*	From DSP to CPU		eCos Send function
*******************************************************************************/
int
aipc_int_send_2cpu( u32_t int_id )		//only use 64 
{
	int retval = NOK;
	volatile u32_t add_hiq = 0;	

	#ifdef IRQ_LOCK
	AIPC_OSAL_INTERRUPT_FLAGS _old;
	#endif

	add_hiq = !(int_id & 0x01);	//add even to hi, odd to low queue

	ADEBUG(DBG_INTQ , "send %d\n" , int_id);
	
	if( int_id >= AIPC_MGR_MAX ) 
		return retval;

#ifdef IRQ_LOOP_PROTECT
	if( add_hiq ){
		while(1){
			#ifdef IRQ_LOCK
			AIPC_OSAL_INTERRUPT_DISABLE(_old);
			#endif

			if( aipc_int_2cpu_hiq_full() ){
				#ifdef IRQ_LOCK
				AIPC_OSAL_INTERRUPT_ENABLE(_old);
				#endif
				continue;			
			}
			else{
				aipc_int_2cpu_hiq_enqueue( int_id );
				aipc_int_assert( T_CPU );
				retval = OK;
				#ifdef IRQ_LOCK
				AIPC_OSAL_INTERRUPT_ENABLE(_old);
				#endif
				
				break;
			}
		}
	}
	else{
		while(1){
			#ifdef IRQ_LOCK
			AIPC_OSAL_INTERRUPT_DISABLE(_old);
			#endif

			if( aipc_int_2cpu_lowq_full() ){
				#ifdef IRQ_LOCK
				AIPC_OSAL_INTERRUPT_ENABLE(_old);
				#endif
				continue;			
			}
			else{
				aipc_int_2cpu_lowq_enqueue( int_id );
				aipc_int_assert( T_CPU );
				retval = OK;
				#ifdef IRQ_LOCK
				AIPC_OSAL_INTERRUPT_ENABLE(_old);
				#endif
				
				break;
			}
		}
	}

#else		

	if( add_hiq ){
		if( !aipc_int_2cpu_hiq_full() ){
			aipc_int_2cpu_hiq_enqueue( int_id );
			aipc_int_assert( T_CPU );
			retval = OK;
		}
	}
	else{
		if( !aipc_int_2cpu_lowq_full() ){
			aipc_int_2cpu_lowq_enqueue( int_id );
			aipc_int_assert( T_CPU );
			retval = OK;
		}
	}

#endif

	return retval;
}

/*******************************************************************************
*	From CPU to DSP		eCos ISR function
*******************************************************************************/
AIPC_OSAL_ISR_RET_TYPE
aipc_dsp_ISR( int vector , void *data )
//aipc_dsp_ISR(cyg_vector_t vector , cyg_addrword_t data)
{
	int n = MAX_HANDLE_CNT;
	u32_t fid=0;
	volatile u32_t status = 0;

	//SDEBUG("irq=%d data=%u\n" , vector , data);

#ifdef IRQ_LOOP_PROTECT

	#if 1
	aipc_int_mask( vector );		
	#else
	HAL_INTERRUPT_MASK( vector ); 
	//HAL_INTERRUPT_ACKNOWLEDGEMENT( vector );
	#endif

	aipc_int_deassert( T_DSP );

	while(1) {
		if( !aipc_int_2dsp_hiq_empty() ){	//handle elements in hi queue
			for( ; n>0 ; n-- )
			{
				if( !aipc_int_2dsp_hiq_empty() ){  //For hi queue
					//Get element from hi queue
					if( OK == aipc_int_2dsp_hiq_dequeue( &fid ))
						aipc_exe_callback( fid , (void *)data );  //Run callback
					else
						break;
				}
			}
		}
		if( !aipc_int_2dsp_lowq_empty() ){	//handle elements in low queue
			for( ; n>0 ; n-- )
			{
				if( !aipc_int_2dsp_lowq_empty() ){	//For low queue 
					//Get element from low queue
					if( OK == aipc_int_2dsp_lowq_dequeue( &fid ))
						aipc_exe_callback( fid , (void *)data );  //Run callback
					else
						break;
				}
			}
		}
		status = REG32(R_GISR1_0);

		if( status & CPU_T_DSP_IP )
			aipc_int_deassert( T_DSP );
		else
			break;
	}
	
	#if 1
	aipc_int_unmask( vector );
	#else
	HAL_INTERRUPT_UNMASK( vector );
	#endif

#else

	aipc_int_mask( vector );		

	aipc_int_deassert( T_DSP );

	for( ; n>0 ; n-- )
	{
		if( !aipc_int_2dsp_hiq_empty() ){  //For hi queue
			//Get element from hi queue
			if( OK == aipc_int_2dsp_hiq_dequeue( &fid ))
				aipc_exe_callback( fid , (void *)data );  //Run callback
			else
				break;
		}
	}

	for( ; n>0 ; n-- )
	{
		if( !aipc_int_2dsp_lowq_empty() ){  //For low queue	
			//Get element from low queue
			if( OK == aipc_int_2dsp_lowq_dequeue( &fid ))
				aipc_exe_callback( fid , (void *)data );  //Run callback
			else
				break;
		}
	}

	status = REG32(R_GISR1_0);

	if( status & CPU_T_DSP_IP )
		aipc_int_deassert( T_DSP );
	
	aipc_int_unmask( vector );

#endif

#if 0
	#ifdef __KERNEL__
		return AIPC_OSAL_ISR_HANDLE;
	#elif defined(__ECOS)
		#ifdef DSP_DSR_SUPPORT
			return AIPC_OSAL_ISR_HANDLE | AIPC_OSAL_ISR_CALLDSR;
		#else
			return AIPC_OSAL_ISR_HANDLE;
		#endif
	#endif
#else

	return AIPC_OSAL_ISR_HANDLE;

#endif

}

#ifdef DSP_DSR_SUPPORT
#ifdef __ECOS
void
aipc_dsp_DSR(cyg_vector_t vector , cyg_ucount32 count, cyg_addrword_t data)
{
	void *mp = NULL;
	//SDEBUG( "trigger 2dsp rx DSR. vector=%d count=%u data=%u \n" , vector , count , data );
	
	mp = aipc_data_2dsp_recv();

	if(mp){
		if (voip_dsp_L2_pkt_rx_trap){
			(*voip_dsp_L2_pkt_rx_trap )((unsigned char *)mp, MAIL_2DSP_SIZE);
			aipc_data_2dsp_ret(mp);
			}
		}
}
#endif


void aipc_2dsp_rx_tasklet(unsigned long task_priv)
{
	void *mp;

#if 1
	while( mp = aipc_data_2dsp_recv() )
	{	 
		if (voip_dsp_L2_pkt_rx_trap){
			(*voip_dsp_L2_pkt_rx_trap )((unsigned char *)mp, MAIL_2DSP_SIZE) ;
			aipc_data_2dsp_ret(mp);
			}
	}
#else
	extern void coprocessor_dsp_rx( void *shm_pkt, unsigned long size);
	// process mbox packet from cpu to dsp 
	while( ( mp = aipc_data_2dsp_recv() ) ) {
		
		coprocessor_dsp_rx( mp, 0 );
		
		aipc_data_2dsp_ret( mp );
	}
#endif
}

void aipc_2dsp_event_tasklet(unsigned long task_priv)
{
	void *mp;
	
#if 1
	while( mp = aipc_ctrl_2dsp_recv() )
	{	 
		if (voip_dsp_L2_pkt_event_trap){
			(*voip_dsp_L2_pkt_event_trap )((unsigned char *)mp, CMD_SIZE) ;
			RETURN_CTRL(mp);
			}
	}
#else	
	extern void coprocessor_dsp_rx( void *shm_pkt, unsigned long size);
	// process ctrl packet from cpu to dsp 
	while( ( mp = aipc_ctrl_2dsp_recv() ) ) {
		
		coprocessor_dsp_rx( mp, 0 );
		
		// this marco will check feedback or not automatically. 
		RETURN_CTRL( mp );
		//aipc_ctrl_2dsp_ret( mp );
	}
#endif

}
#endif

void aipc_dsp_voip_reg_cbk(void)
{
	voip_dsp_L2_pkt_rx_trap    = voip_dsp_L2_pkt_test;
	voip_dsp_L2_pkt_event_trap = voip_dsp_L2_pkt_event_test;
}

#endif


/*
*	Interrupt callback functions
*/

#ifdef _AIPC_CPU_
/*
*	For CPU<-DSP sending RTP packet case, DSP needs to send interrupt to notify CPU
*	about packet arrival. CPU receive RTP packet in ISR context.
*	In this case, this function should be called in AIPC ISR.
*/
int
aipc_2cpu_mbox_recv( u32_t int_id , void *data )
{
#ifdef CPU_DSR_SUPPORT
	struct aipc_int_priv* priv = &aipc_int_priv_dsc;

	tasklet_hi_schedule(&priv->rx_tasklet);

#else	//recieve packet in ISR

	void *mp = NULL;

	mp = aipc_data_2cpu_recv();

	if(mp){
		ADEBUG(DBG_CONT , "mbox recv from ISR\n");

		if (voip_dsp_L2_pkt_rx_trap){
			(*voip_dsp_L2_pkt_rx_trap)((unsigned char *)mp, MAIL_2CPU_SIZE);
			aipc_data_2cpu_ret(mp);
		}
	}

#endif

	return 0;
}

int
aipc_2cpu_event_recv( u32_t int_id , void *data )
{
#ifdef CPU_DSR_SUPPORT
	struct aipc_int_priv* priv = &aipc_int_priv_dsc;

	tasklet_hi_schedule(&priv->event_tasklet);

#else	//recieve packet in ISR

	void *mp = NULL;

	mp = aipc_ctrl_2cpu_recv();

	if(mp){
		ADEBUG(DBG_CONT , "event recv from ISR\n");

		if (voip_dsp_L2_pkt_event_trap){
			(*voip_dsp_L2_pkt_event_trap)((unsigned char *)mp, EVENT_SIZE);
			aipc_ctrl_2cpu_ret(mp);
		}
	}

#endif
	return 0;
}

#elif defined(_AIPC_DSP_)

int
aipc_2dsp_mbox_recv( u32_t int_id , void *data )
{
#ifdef CONFIG_RTK_VOIP
	extern void	aipc_hook_call_rx_tasklet(void);
	aipc_hook_call_rx_tasklet();
#endif

	return 0;
}

int
aipc_2dsp_event_recv( u32_t int_id , void *data )
{
#ifdef CONFIG_RTK_VOIP
	extern void	aipc_hook_call_event_tasklet(void);
	aipc_hook_call_event_tasklet();
#endif

	return 0;
}

#endif


#ifdef _AIPC_CPU_

//Define callback functions here
static aipc_entry_t aipc_callback_table[] = {
	//DSP send RTP packet to CPU using interrupt
	{ AIPC_MGR_2CPU_MBOX_RECV 	, aipc_2cpu_mbox_recv  	},	//high	queue
	{ AIPC_MGR_2CPU_EVENT_RECV	, aipc_2cpu_event_recv	},	//low	queue
	{ AIPC_MGR_2DSP_MBOX_RECV 	, NULL					},	//high	queue
	{ AIPC_MGR_2DSP_EVENT_RECV  , NULL					},  //low   queue
	{ AIPC_MGR_MAX , NULL }
};

#elif defined(_AIPC_DSP_)

//Define callback functions here
static aipc_entry_t aipc_callback_table[] = {
	//DSP send RTP packet to CPU using interrupt
	{ AIPC_MGR_2CPU_MBOX_RECV 	, NULL  				},	//high	queue
	{ AIPC_MGR_2CPU_EVENT_RECV	, NULL					},	//low	queue
	{ AIPC_MGR_2DSP_MBOX_RECV 	, aipc_2dsp_mbox_recv  	},	//high	queue
	{ AIPC_MGR_2DSP_EVENT_RECV  , aipc_2dsp_event_recv  },  //low   queue
	{ AIPC_MGR_MAX , NULL }
};

#endif

#ifndef __ECOS
#ifdef CONFIG_RTL8686_SHM_NOTIFY
extern const unsigned int *aipc_shm_notify_cpu;

int aipc_shm_notify(void)
{
	irqreturn_t ret = IRQ_NONE;

#ifdef _AIPC_CPU_
	int irq = 0;
	void *dev_id = NULL;
	
	if ((*((volatile unsigned int *)aipc_shm_notify_cpu))==1){
#ifdef INT_COUNT
		ASTATS.shm_notify_cpu++;
#endif
		ret = aipc_cpu_ISR(irq , dev_id);
	}
	
#elif defined(_AIPC_DSP_)
	ret = AIPC_OSAL_ISR_HANDLE;
#endif

	return ret;
}
#endif
#endif

int
aipc_exe_callback( u32_t cmd , void *data )
{
	int index=0;
	int retval=0;
	aipc_entry_t *p_entry; 

	//printk("aipc: cmd=%u data=%p\n" , cmd , data);
	
	if( cmd <= AIPC_MGR_BASE || cmd >= AIPC_MGR_MAX ){
		return NOK;	
	}
	
	index = cmd - ( AIPC_MGR_BASE + 1 );
	
	p_entry = &aipc_callback_table[ index ];
	
	if( p_entry ->do_mgr ) {
		retval = p_entry ->do_mgr( cmd, data );
	}
	return retval;
}

void voip_dsp_L2_pkt_test(unsigned char* eth_pkt, unsigned long size)
{
	ADEBUG(DBG_MBOX , "eth_pkt=%p\n" , eth_pkt);
	ADEBUG(DBG_CONT , "%s\n" , eth_pkt);
}

void voip_dsp_L2_pkt_event_test(unsigned char* eth_pkt, unsigned long size)
{
	ADEBUG(DBG_CTRL, "eth_pkt=%p\n" , eth_pkt);
	ADEBUG(DBG_CONT , "%s\n" , eth_pkt);
}



