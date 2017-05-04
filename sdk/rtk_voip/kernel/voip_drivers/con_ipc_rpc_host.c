#include "voip_types.h"
#include "voip_ipc.h"
#include "voip_debug.h"
#include "con_register.h"
#include "con_mux.h"
#include "con_ipc_mirror.h"

#define FXS_OPS_OFFSET( f )	( ( uint32 )&( ( ( snd_ops_fxs_t * )0 ) ->f ) )
#define DAA_OPS_OFFSET( f )	( ( uint32 )&( ( ( snd_ops_daa_t * )0 ) ->f ) )

static void __ipc_rpc_slic_parser( voip_con_t * p_con, voip_snd_t * p_snd, rpc_content_t * rpc_content )
{
	uint8 *p_data = rpc_content ->data;
	ipc_mirror_priv_data_t * const p_mirror_data = ( ipc_mirror_priv_data_t * )p_con ->ipc.priv;
	ipc_mirror_union_data_t * const p_mirror_udata = &( p_mirror_data ->udata );
	ipc_mirror_help_state_t * const p_mirror_hstate = &( p_mirror_data ->hstate );
	
#if 0
	printk( "RPC slic: %u\n", rpc_content ->ops_offset );
#endif
	
	if( p_snd ->snd_type != SND_TYPE_FXS ) {
		PRINT_Y( "__ipc_rpc_slic_parser: snd type should be FXS!\n" );
		return;
	}
	
	switch( rpc_content ->ops_offset ) {
	case FXS_OPS_OFFSET( enable ):
		p_snd ->fxs_ops ->enable( p_snd, *( ( int * )p_data ) );
		break;
	
	case FXS_OPS_OFFSET( FXS_Ring ):
		p_mirror_hstate ->slic.f_ring_chk = 1;
		p_mirror_hstate ->slic.f_ring_occur = 0;
		p_snd ->fxs_ops ->FXS_Ring( p_snd, *( ( unsigned char * )p_data ) );
		break;
	
	case FXS_OPS_OFFSET( OnHookLineReversal ):
		p_snd ->fxs_ops ->OnHookLineReversal( p_snd, *( ( unsigned char * )p_data ) );
		break;
	
	case FXS_OPS_OFFSET( SLIC_CPC_Gen ):
		p_mirror_hstate ->slic.CPC_pre_linefeed = 		
			p_snd ->fxs_ops ->SLIC_CPC_Gen( p_snd );
		break;
		
	case FXS_OPS_OFFSET( SLIC_CPC_Check ):
		// ignore 'pre_linefeed' in RPC packet 
		p_snd ->fxs_ops ->SLIC_CPC_Check( p_snd, p_mirror_hstate ->slic.CPC_pre_linefeed );
		break;
	
	case FXS_OPS_OFFSET( SendNTTCAR ):
		p_mirror_udata ->slic.SendNTTCAR_chk = 0;
		p_snd ->fxs_ops ->SendNTTCAR( p_snd );
		break;
	
	case FXS_OPS_OFFSET( SendNTTCAR_check ):
		p_mirror_hstate ->slic.f_SendNTTCAR_chk = 1;
		p_mirror_hstate ->slic.SendNTTCAR_check_timeout = *( ( unsigned long * )p_data );
		break;

	case FXS_OPS_OFFSET( SLIC_Set_Power_Save_Mode ):
		p_snd ->fxs_ops ->SLIC_Set_Power_Save_Mode( p_snd);
		break;

	case FXS_OPS_OFFSET( SLIC_Set_FXS_Line_State ):
		p_snd ->fxs_ops ->SLIC_Set_FXS_Line_State( p_snd, *( ( unsigned char * )p_data ) );
		break;

	case FXS_OPS_OFFSET( SLIC_Set_Multi_Ring_Cadence ):		
		p_snd ->fxs_ops ->SLIC_Set_Multi_Ring_Cadence( p_snd, *( ( unsigned short * )p_data ), *( ( unsigned short * )p_data + 1 ), *( ( unsigned short * )p_data + 2 ), *( ( unsigned short * )p_data + 3 ), *( ( unsigned short * )p_data + 4 ), *( ( unsigned short * )p_data + 5 ), *( ( unsigned short * )p_data + 6 ), *( ( unsigned short * )p_data + 7 ));
		break;
	default:
		PRINT_Y( "No handle RPC SLIC ops=%u\n", rpc_content ->ops_offset );
		break;
	}
}

static void __ipc_rpc_daa_parser( voip_con_t * p_con, voip_snd_t * p_snd, rpc_content_t * rpc_content )
{
	uint8 *p_data = rpc_content ->data;
	ipc_mirror_priv_data_t * const p_mirror_data = ( ipc_mirror_priv_data_t * )p_con ->ipc.priv;
	ipc_mirror_union_data_t * const p_mirror_udata = &( p_mirror_data ->udata );
	ipc_mirror_help_state_t * const p_mirror_hstate = &( p_mirror_data ->hstate );	
#if 0
	printk( "RPC daa: %u\n", rpc_content ->ops_offset );
#endif
	
	if( p_snd ->snd_type != SND_TYPE_DAA ) {
		PRINT_Y( "__ipc_rpc_daa_parser: snd type should be DAA!\n" );
		return;
	}
	
	switch( rpc_content ->ops_offset ) {
	case DAA_OPS_OFFSET( enable ):
		p_snd ->daa_ops ->enable( p_snd, *( ( int * )p_data ) );
		break;
		
	case DAA_OPS_OFFSET( DAA_Set_PulseDial ):
		p_snd ->daa_ops ->DAA_Set_PulseDial( p_snd, *( ( unsigned int * )p_data ) );
		break;
	
	case DAA_OPS_OFFSET( DAA_OnHook_Line_Monitor_Enable ):
		p_snd ->daa_ops ->DAA_OnHook_Line_Monitor_Enable( p_snd );
		break;

	case DAA_OPS_OFFSET( DAA_Off_Hook ):
		p_snd ->daa_ops ->DAA_Off_Hook( p_snd );
		break;

	case DAA_OPS_OFFSET( DAA_On_Hook ):
		p_snd ->daa_ops ->DAA_On_Hook( p_snd );
		break;		

	case DAA_OPS_OFFSET( DAA_Check_Line_State ):
		p_snd ->daa_ops ->DAA_Check_Line_State( p_snd );
		break;
		
	case DAA_OPS_OFFSET( DAA_Get_Polarity ):
		p_snd ->daa_ops ->DAA_Get_Polarity( p_snd );
		break;			
		
	case DAA_OPS_OFFSET( DAA_Hook_Status ):
		p_mirror_hstate ->daa.hook_status =  
			p_snd ->daa_ops ->DAA_Hook_Status( p_snd, *( ( unsigned int * )p_data ) );
		break;				
		
	default:
		PRINT_Y( "No handle RPC DAA ops=%u\n", rpc_content ->ops_offset );
		break;
	}
}

void ipc_rpc_parser( ipc_ctrl_pkt_t *ipc_ctrl )
{
	// retrieve con_ptr, only *mux* can use this!! 
	extern voip_con_t *get_voip_con_ptr( uint32 cch );
	
	extern int ipcSentRpcAckPacket( unsigned short category, uint16 seq_no, unsigned int host_cch, void* rpc_ack_data, unsigned short rpc_ack_len );
	
	rpc_content_t * const rpc_content = ( rpc_content_t * )ipc_ctrl ->content;
	const int32 host_cch = API_get_Host_CH( ipc_ctrl ->dsp_cpuid, rpc_content ->cch );	
	voip_con_t * p_con;
	voip_snd_t * p_snd;

	if (host_cch < 0)
	{
		PRINT_R("Error, %s, line%d, host_cch < 0\n", __FUNCTION__, __LINE__);
		return;
	}

	p_con = get_voip_con_ptr( host_cch );
	p_snd = p_con ->snd_ptr;


	// send ACK packet 
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_ISSUE_PURE_ACK
	ipcSentRpcAckPacket( ipc_ctrl ->category, ipc_ctrl ->sequence, host_cch, NULL, 0 );
#endif
	
	switch( ipc_ctrl ->category ) {
	case IPC_RPC_SLIC:
		__ipc_rpc_slic_parser( p_con, p_snd, rpc_content );
		break;
	case IPC_RPC_DAA:
		__ipc_rpc_daa_parser( p_con, p_snd, rpc_content );
		break;
	}
}

