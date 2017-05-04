#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>

#elif defined(__ECOS)

#else
#error "need to define"
#endif

#include "./include/aipc_shm.h"
#include "./include/aipc_debug.h"
#include "./include/aipc_mem.h"
#include "./include/aipc_osal.h"
#include "./include/aipc_swp.h"

#if	defined(REFINE_SHM_NOTIFY) && defined(CONFIG_RTL8686_SHM_NOTIFY)
	const unsigned int *aipc_shm_notify_cpu = (unsigned int *)&(SHM_NOTIFY.notify_cpu);
	const unsigned int *aipc_shm_notify_dsp = (unsigned int *)&(SHM_NOTIFY.notify_dsp);
#elif !defined(REFINE_SHM_NOTIFY) && defined(CONFIG_RTL8686_SHM_NOTIFY)
	const unsigned int *aipc_shm_notify_cpu = (unsigned int *)AIPC_SHM_NOTIFY_CPU;
	const unsigned int *aipc_shm_notify_dsp = (unsigned int *)AIPC_SHM_NOTIFY_DSP;
#endif

extern unsigned int BSP_MHZ, BSP_SYSCLK;

#ifdef CONFIG_RTL8686_IPC_RECORD_DSP_LOG
unsigned int *rec_dsp_log_enable     = (unsigned int *)&(REC_DSP_LOG.enable);
unsigned int *rec_dsp_log_ins        = (unsigned int *)&(REC_DSP_LOG.ins);
unsigned int *rec_dsp_log_del        = (unsigned int *)&(REC_DSP_LOG.del);
unsigned int *rec_dsp_log_touch      = (unsigned int *)&(REC_DSP_LOG.touch);
char         *rec_dsp_log_contents   = (char *)        &(REC_DSP_LOG.contents);

typedef int (*ft_aipc_dsp_log_add)(char);
ft_aipc_dsp_log_add   fp_aipc_dsp_log_add = NULL;


int aipc_record_dsp_log_full(void)
{
	if ((*rec_dsp_log_del + RECORD_DSP_LOG_SIZE - *rec_dsp_log_ins) % RECORD_DSP_LOG_SIZE == 1)
		return TRUE;
	else 
		return FALSE;

}

int aipc_record_dsp_log_empty(void)
{
	if ((*rec_dsp_log_ins + RECORD_DSP_LOG_SIZE - *rec_dsp_log_del) % RECORD_DSP_LOG_SIZE == 0)
		return TRUE;
	else 
		return FALSE;
}

unsigned int aipc_record_dsp_log_contents_use(void)
{
	return ((*rec_dsp_log_ins + RECORD_DSP_LOG_SIZE - *rec_dsp_log_del) % RECORD_DSP_LOG_SIZE);
}
#endif

#ifdef _AIPC_CPU_
int probe_cpu_int_notify_cpu2dsp = 0;        //disalbe by default  
#elif defined(_AIPC_DSP_)
int probe_dsp_int_notify_cpu2dsp = 0;        //disalbe by default  
#endif

#ifdef _AIPC_CPU_

unsigned short CheckDspIfAllSoftwareReady(void)
{
	extern int aipc_dsp_disable_mem(void);
	static int disable_mem = 0;
	unsigned short status = (unsigned short)ACTSW.init;

	if( (status==INIT_OK) && !disable_mem ){
		aipc_dsp_disable_mem();
		disable_mem = 1;
	}

	return status;
}

EXPORT_SYMBOL(CheckDspIfAllSoftwareReady);

#ifdef AIPC_MSG_PROBE
#if defined(CONFIG_LUNA_MEMORY_AUTO_DETECTION)
#include <bsp_automem.h>
#endif
void aipc_msg_probe_init( void )
{
	AMSG_PROBE.dsp_mem_offset    = (unsigned long)CONFIG_RTL8686_DSP_MEM_BASE;

#ifdef AIPC_MSG_PROBE_PCM_DMA_USE_SRAM
//	AMSG_PROBE.pcm_dma_sram_conf  = (unsigned long)(0x9fc00000+0x100);  //32bit align
	AMSG_PROBE.pcm_dma_sram_conf  = (unsigned long)(0xbfc00000+0x100);  //32bit align
#endif

#ifdef CONFIG_RTL8686_DSP_LED_TEST
	AMSG_PROBE.dsp_led_test = 1;
#ifdef CONFIG_RTL8686_DSP_LED_GPIO_NUM
	AMSG_PROBE.dsp_led_gpio_num = CONFIG_RTL8686_DSP_LED_GPIO_NUM;
#else
#error "Please set LED GPIO NUM."
#endif
#endif

#ifdef CONFIG_RTL8686_IPC_DSP_CONFIG
#ifdef CONFIG_RTL8686_IPC_DSP_CONFIG_CON_CH_NUM
	AMSG_PROBE.aipc_con_ch_num = CONFIG_RTL8686_IPC_DSP_CONFIG_CON_CH_NUM;
#endif
#ifdef CONFIG_RTL8686_IPC_DSP_CONFIG_MIRROR_SLIC_NR
	AMSG_PROBE.aipc_mirror_slic_nr = CONFIG_RTL8686_IPC_DSP_CONFIG_MIRROR_SLIC_NR;
#endif
#if !defined(CONFIG_RTL8686_IPC_DSP_CONFIG_CON_CH_NUM) || !defined(CONFIG_RTL8686_IPC_DSP_CONFIG_MIRROR_SLIC_NR)
#error "Please check DSP config."
#endif
#endif

#if 1
	AMSG_PROBE.aipc_lx_bus_mhz    =  BSP_MHZ;
	AMSG_PROBE.aipc_lx_bus_clock  =  BSP_SYSCLK;
#ifdef CONFIG_CPU1_UART_BASE_ADDR
	AMSG_PROBE.aipc_uart_base_addr = (unsigned int)CONFIG_CPU1_UART_BASE_ADDR;
#endif

//	SDEBUG("BSP_MHZ %u         BSP_SYSCLK %u\n" ,        BSP_MHZ, BSP_SYSCLK);
//	SDEBUG("aipc_lx_bus_mhz %u aipc_lx_bus_clock %u\n" , AMSG_PROBE.aipc_lx_bus_mhz, AMSG_PROBE.aipc_lx_bus_clock);
#endif

#ifdef CONFIG_RTL8686_IPC_INT_NOTIFY_CPU2DSP
	probe_cpu_int_notify_cpu2dsp = 1;
	AMSG_PROBE.aipc_int_notify_cpu2dsp = 1;
#endif 

}
#endif

#else  //_AIPC_DSP_

unsigned short SetDspAllSoftwareReady(void)
{
	unsigned short status;
	ACTSW.init = INIT_OK;
	status = (unsigned short)ACTSW.init;
	return status;
}

#ifdef CONFIG_RTL8686_IPC_RECORD_DSP_LOG

int aipc_dsp_log_add(char buf)
{
	int ret = NOK;
	unsigned int enable = *rec_dsp_log_enable;

#ifdef AIPC_BARRIER
	volatile unsigned int tmp_wr=0;
#endif
	
	switch (enable) {

	case RECORD_DSP_LOG_DISABLE:
		ret = NOK;
	
	case RECORD_DSP_LOG_ENABLE_SAVE_HISTORY:
		if (!aipc_record_dsp_log_full()){
		#ifdef AIPC_BARRIER
			tmp_wr = *rec_dsp_log_ins;
			AMB( rec_dsp_log_contents[ tmp_wr ] , buf );
			
			tmp_wr = (tmp_wr+1) % RECORD_DSP_LOG_SIZE;
			AMB( *rec_dsp_log_ins , tmp_wr );
		#else			
			rec_dsp_log_contents[ *rec_dsp_log_ins ] = buf;
			*rec_dsp_log_ins = (*rec_dsp_log_ins+1) % RECORD_DSP_LOG_SIZE;
		#endif

			ret = OK;
		}
		break;
	
	case RECORD_DSP_LOG_ENABLE_OVERWRITE_HISTORY:
		#ifdef AIPC_BARRIER
			tmp_wr = *rec_dsp_log_ins;
			AMB( rec_dsp_log_contents[ tmp_wr ] , buf );
			
			tmp_wr = (tmp_wr+1) % RECORD_DSP_LOG_SIZE;
			AMB( *rec_dsp_log_ins , tmp_wr );
		#else		
			rec_dsp_log_contents[ *rec_dsp_log_ins ] = buf;
			*rec_dsp_log_ins = (*rec_dsp_log_ins+1) % RECORD_DSP_LOG_SIZE;
		#endif

			ret = OK;
		break;

	default:
		SDEBUG( "wrong case\n");
	}
	
	return ret;
}

int aipc_record_dsp_log_init(void)
{
	int status = NOK;
	REC_DSP_LOG.enable = (unsigned int)RECORD_DSP_LOG_ENABLE_OVERWRITE_HISTORY;
	
	if( !fp_aipc_dsp_log_add ){
		fp_aipc_dsp_log_add = aipc_dsp_log_add;
		//diag_printf("%p %p %s(%d)\n" , fp_aipc_dsp_log_add , aipc_dsp_log_add , __FUNCTION__, __LINE__);
	}

	status = OK;
	return status;
}
#endif

#ifdef AIPC_MSG_PROBE_DSP_MEM_OFFSET
void aipc_dsp_init_msg_probe(void)
{
#ifdef __ECOS
	extern unsigned long physical_addr_offset;
	
	if( AMSG_PROBE.dsp_mem_offset )
		physical_addr_offset = AMSG_PROBE.dsp_mem_offset;
	else
		printk("msg probe dsp mem offset is empty!");	
#endif

#ifdef AIPC_MSG_PROBE_PCM_DMA_USE_SRAM
	extern unsigned long pcm_dma_sram_addr;

	if( AMSG_PROBE.pcm_dma_sram_conf )
		 pcm_dma_sram_addr = AMSG_PROBE.pcm_dma_sram_conf;
	else
		printk("msg probe dsp mem offset is empty!");
#endif

#ifdef CONFIG_RTL8686_DSP_LED_TEST
	extern int thread_dsp_led_test;
	extern int thread_dsp_led_gpio_num;
	
	if( AMSG_PROBE.dsp_led_test ) {
		thread_dsp_led_test = 1;
#ifdef CONFIG_RTL8686_DSP_LED_GPIO_NUM
		thread_dsp_led_gpio_num = CONFIG_RTL8686_DSP_LED_GPIO_NUM;
#else
#error "Please set LED GPIO NUM."
#endif
	}
#endif

//#ifdef CONFIG_RTL8686_IPC_INT_NOTIFY_CPU2DSP
	if( AMSG_PROBE.aipc_int_notify_cpu2dsp == 1 )
		probe_dsp_int_notify_cpu2dsp = 1;
//#endif
}
#endif

#ifdef CONFIG_RTL8686_IPC_DSP_CONFIG
#ifdef __ECOS
void aipc_msg_probe_set_dsp_config(void)
{
	extern int con_ch_num;
	
	if(AMSG_PROBE.aipc_con_ch_num){
		con_ch_num = AMSG_PROBE.aipc_con_ch_num;
	}
}
#endif
#endif

void
aipc_dsp_ipc_init(void)
{
	extern void	aipc_dsp_cm_init(void);
	extern void aipc_dsp_mb_init(void);
#ifdef CONFIG_RTL8686_IPC_RECORD_DSP_LOG
	extern int aipc_record_dsp_log_init( void );
#endif
#ifdef CONFIG_RTL8686_IPC_TEST_THREAD
	extern void aipc_dsp_test_thread_entry( void );
#endif

#ifdef AIPC_GOT
	extern void aipc_dsp_intq_init( void );
	aipc_dsp_intq_init();
#endif

	aipc_dsp_cm_init();
	aipc_dsp_mb_init();
	
#ifdef CONFIG_RTL8686_IPC_RECORD_DSP_LOG
	aipc_record_dsp_log_init();
#endif

#ifdef AIPC_MSG_PROBE_DSP_MEM_OFFSET
	aipc_dsp_init_msg_probe();
#endif                      

#ifdef CONFIG_RTL8686_IPC_TEST_THREAD
	aipc_dsp_test_thread_entry();
#endif
}

#ifdef __KERNEL__
extern void aipc_dev_create_proc(void);

int aipc_dsp_ipc_initcall_init(void)
{
	aipc_dsp_ipc_init();
	
	aipc_dev_create_proc();
	
	return 0;
}
core_initcall(aipc_dsp_ipc_initcall_init);
//module_init(aipc_dsp_ipc_initcall_init);
#endif

#endif


