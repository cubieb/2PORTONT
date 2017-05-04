#include <linux/string.h>
#include <linux/version.h>
#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_init.h"
#include "voip_ipc.h"
#include "voip_debug.h"

#include "aipc_api.h"
#include "aipc_buffer.h"

typedef enum {
	ASHM_2DSP_CTRL,
	ASHM_2CPU_EVENT,
	ASHM_2DSP_MBOX,
	ASHM_2CPU_MBOX,
} aipc_shm_t;

static inline aipc_shm_t get_aipc_shm_type( uint8 protocol )
{
	switch( protocol )
	{
#ifdef CONFIG_RTK_VOIP_COPROCESS_DSP_IS_HOST
	case IPC_PROT_CTRL:					// a) host --> dsp 
	case IPC_PROT_ACK:					// b) host --> dsp 
	case IPC_PROT_MIRROR_ACK:			// c) host --> dsp
	case IPC_PROT_RPC_ACK:				// d) host --> dsp	
		return ASHM_2DSP_CTRL;
		
	case IPC_PROT_VOICE_TO_DSP:			//    host --> dsp 
	case IPC_PROT_T38_TO_DSP:			//    host --> dsp 
	case IPC_PROT_MIRROR:				// c) host <-- dsp
	case IPC_PROT_RPC:					// d) host <-- dsp
	default:
		return ASHM_2DSP_MBOX;
		
#elif defined( CONFIG_RTK_VOIP_COPROCESS_DSP_IS_DSP )
	case IPC_PROT_RESP:					// a) host <-- dsp 
	case IPC_PROT_EVENT:				// b) host <-- dsp 
		return ASHM_2CPU_EVENT;

	case IPC_PROT_VOICE_TO_HOST:		//    host <-- dsp 
	case IPC_PROT_T38_TO_HOST:			//    host <-- dsp 
	default:
		return ASHM_2CPU_MBOX;
#endif
	}
}

ipc_ctrl_pkt_t *coprocessor_dsp_tx_allocate( unsigned int *len, 
											void **ipc_priv, uint8 protocol )
{
	// ipc_priv will pass to coprocessor_start_xmit() 
	unsigned int buffer_size = 0;
	
	switch( get_aipc_shm_type( protocol ) ) {
#ifdef CONFIG_RTK_VOIP_COPROCESS_DSP_IS_HOST
	case ASHM_2DSP_CTRL:
		buffer_size = CMD_SIZE;
		
		if( ( *ipc_priv = aipc_ctrl_2dsp_nofbk_alloc() ) == NULL )
			return NULL;
			
		break;
		
	case ASHM_2DSP_MBOX:
		buffer_size = MAIL_2DSP_SIZE;
		
		if( ( *ipc_priv = aipc_data_2dsp_alloc() ) == NULL )
			return NULL;
		
		break;
#else
	case ASHM_2CPU_EVENT:
		buffer_size = EVENT_SIZE;
		
		if( ( *ipc_priv = aipc_ctrl_2cpu_alloc() ) == NULL )
			return NULL;
			
		break;
		
	case ASHM_2CPU_MBOX:
		buffer_size = MAIL_2CPU_SIZE;
		
		if( ( *ipc_priv = aipc_data_2cpu_alloc() ) == NULL )
			return NULL;
			
		break;
#endif
	
	default:
		return NULL;
	}
	
	// check allocated size 
	if( buffer_size < *len + 4 ) {
		PRINT_R( "IPC allocated size is too small (%u < %u).\n", buffer_size, *len + 4 );
	}
	
	// preserve 4 bytes for length 
	*( ( unsigned long * )( *ipc_priv ) ) = *len;
	
	return ( ipc_ctrl_pkt_t * )( ( unsigned char * )( *ipc_priv ) + 4 );
}

void coprocessor_dsp_fill_tx_header( ipc_ctrl_pkt_t *ipc_pkt,
								const TstTxPktCtrl* txCtrl, uint8 protocol )
{
#if 0	// nothing! 
	switch( get_aipc_shm_type( protocol ) ) {
#ifdef CONFIG_RTK_VOIP_COPROCESS_DSP_IS_HOST
	case ASHM_2DSP_CTRL:
		break;
	case ASHM_2DSP_MBOX:
		break;
#else
	case ASHM_2CPU_EVENT:
		break;
	case ASHM_2CPU_MBOX:
		break;
#endif
	}
#endif 
}

void coprocessor_start_xmit( void *ipc_priv, uint8 protocol )
{
	switch( get_aipc_shm_type( protocol ) ) {
#ifdef CONFIG_RTK_VOIP_COPROCESS_DSP_IS_HOST
	case ASHM_2DSP_CTRL:
		aipc_ctrl_2dsp_send( ipc_priv, 0 );
		break;
		
	case ASHM_2DSP_MBOX:
		aipc_data_2dsp_send( ipc_priv );
		break;
#else
	case ASHM_2CPU_EVENT:
		aipc_ctrl_2cpu_send( ipc_priv );
		break;
		
	case ASHM_2CPU_MBOX:
		aipc_data_2cpu_send( ipc_priv );
		break;
#endif

	default:
		break;
	}
}

