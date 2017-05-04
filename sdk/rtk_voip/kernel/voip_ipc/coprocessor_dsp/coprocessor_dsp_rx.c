#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_init.h"
#include "voip_ipc.h"
#include "voip_timer.h"

#include "aipc_shm.h"
#include "aipc_api.h"

#include "ipc_arch_rx.h"

static void coprocessor_dsp_rx( void *shm_pkt, unsigned long size)
{
	// 'size' is useless in coprocessor, so 
	// we add four byte length in shm_pkt. 
	
	ipc_pkt_rx_entry( ( ipc_ctrl_pkt_t * )( ( unsigned char * )( shm_pkt ) + 4 ), 
						*( ( unsigned long * )( shm_pkt ) ) );
}

static void coprocessor_rx_timer_timer( unsigned long data )
{
	// do polling action 
	
#ifdef CONFIG_RTK_VOIP_COPROCESS_DSP_IS_HOST
	
	
	// process mbox packet from dsp to cpu 
	//aipc_tocpu_mbox_recv();
	
	// process event packet from dsp to cpu 
	//aipc_event_proc();
#elif defined( CONFIG_RTK_VOIP_COPROCESS_DSP_IS_DSP )
	void *mp;
	
	// process mbox packet from cpu to dsp 
	while( ( mp = aipc_data_2dsp_recv() ) ) {
		
		coprocessor_dsp_rx( mp, 0 );
		
		aipc_data_2dsp_ret( mp );
	}
		
	// process ctrl packet from cpu to dsp 
	while( ( mp = aipc_ctrl_2dsp_recv() ) ) {
		
		coprocessor_dsp_rx( mp, 0 );
		
		// this marco will check feedback or not automatically. 
		RETURN_CTRL( mp );
		//aipc_ctrl_2dsp_ret( mp );
	}
#endif	
	
}
	
static int __init coprocessor_dsp_rx_init( void )
{
#ifdef CONFIG_RTK_VOIP_COPROCESS_DSP_IS_HOST
	extern void ( *voip_dsp_L2_pkt_rx_trap )(unsigned char* eth_pkt, unsigned long size);
	extern void ( *voip_dsp_L2_pkt_event_trap )(unsigned char* eth_pkt, unsigned long size);
	
	voip_dsp_L2_pkt_rx_trap = coprocessor_dsp_rx;
	voip_dsp_L2_pkt_event_trap = coprocessor_dsp_rx;
#elif defined( CONFIG_RTK_VOIP_COPROCESS_DSP_IS_DSP )
	extern int probe_dsp_int_notify_cpu2dsp;
	if( probe_dsp_int_notify_cpu2dsp == 1 )
	{
		extern void ( *voip_dsp_L2_pkt_rx_trap )(unsigned char* eth_pkt, unsigned long size);
		extern void ( *voip_dsp_L2_pkt_event_trap )(unsigned char* eth_pkt, unsigned long size);
		
		voip_dsp_L2_pkt_rx_trap = coprocessor_dsp_rx;
		voip_dsp_L2_pkt_event_trap = coprocessor_dsp_rx;	
	}
		
	printk("%s(%d) probe_dsp_int_notify_cpu2dsp=%d\n" , __FUNCTION__ , __LINE__ , 
			probe_dsp_int_notify_cpu2dsp);
		
#else
	???
#endif

#ifdef CONFIG_RTK_VOIP_COPROCESS_DSP_IS_HOST
	register_timer_10ms( ( fn_timer_t )coprocessor_rx_timer_timer, NULL );
#elif defined( CONFIG_RTK_VOIP_COPROCESS_DSP_IS_DSP )
	if( probe_dsp_int_notify_cpu2dsp == 0 )
		register_timer_10ms( ( fn_timer_t )coprocessor_rx_timer_timer, NULL );
#endif	

	return 0;
}

voip_initcall( coprocessor_dsp_rx_init );

