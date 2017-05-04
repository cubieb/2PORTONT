#include <linux/kernel.h>
#include "voip_types.h"
#include "con_register.h"

// retrieve con_ptr, only *mux* can use this!! 
extern voip_con_t *get_voip_con_ptr( uint32 cch );

#define DECLARE_BASIC_SND_FXS( x )			\
	const voip_con_t * const p_con = get_voip_con_ptr( cch );	\
	voip_snd_t * const p_snd = p_con ->snd_ptr;	\
												\
	if( p_snd ->snd_type != SND_TYPE_FXS ) {	\
		CHK_MSG( "snd_type != SND_TYPE_FXS\n" );	\
		return x;								\
	}

#define IMPLEMENT_TYPE0( func, rtype )	\
rtype func(int cch)						\
{										\
	DECLARE_BASIC_SND_FXS( 0 );			\
										\
	return p_snd ->fxs_ops ->func( p_snd);	\
}

#define IMPLEMENT_TYPE0_void( func )	\
void func(int cch)						\
{										\
	DECLARE_BASIC_SND_FXS( ; );			\
										\
	p_snd ->fxs_ops ->func( p_snd);		\
}

#define IMPLEMENT_TYPE1( func, rtype, v1type )	\
rtype func(int cch, v1type var1)		\
{										\
	DECLARE_BASIC_SND_FXS( 0 );			\
										\
	return p_snd ->fxs_ops ->func( p_snd, var1 );	\
}

#define IMPLEMENT_TYPE1_void( func, v1type )	\
void func(int cch, v1type var1)			\
{										\
	DECLARE_BASIC_SND_FXS( ; );			\
										\
	p_snd ->fxs_ops ->func( p_snd, var1 );	\
}

#define IMPLEMENT_TYPE2( func, rtype, v1type, v2type )	\
rtype func(int cch, v1type var1, v2type var2 )	\
{										\
	DECLARE_BASIC_SND_FXS( ; );			\
										\
	p_snd ->fxs_ops ->func( p_snd, var1, var2 );	\
}


#define IMPLEMENT_TYPE2_void( func, v1type, v2type )	\
void func(int cch, v1type var1, v2type var2 )	\
{										\
	DECLARE_BASIC_SND_FXS( ; );			\
										\
	p_snd ->fxs_ops ->func( p_snd, var1, var2 );	\
}

#define IMPLEMENT_TYPE3_void( func, v1type, v2type, v3type)	\
void func(int cch, v1type var1, v2type var2, v3type var3)	\
{										\
	DECLARE_BASIC_SND_FXS( ; );			\
										\
	p_snd ->fxs_ops ->func( p_snd, var1, var2, var3 );	\
}

#define IMPLEMENT_TYPE4( func, rtype, v1type, v2type, v3type, v4type)	\
rtype func(int cch, v1type var1, v2type var2, v3type var3, v4type var4)	\
{										\
	DECLARE_BASIC_SND_FXS( ; );			\
										\
	p_snd ->fxs_ops ->func( p_snd, var1, var2, var3, var4 );	\
}

#define IMPLEMENT_TYPE4_void( func, v1type, v2type, v3type, v4type)	\
void func(int cch, v1type var1, v2type var2, v3type var3, v4type var4)	\
{										\
	DECLARE_BASIC_SND_FXS( ; );			\
										\
	p_snd ->fxs_ops ->func( p_snd, var1, var2, var3, var4 );	\
}

#define IMPLEMENT_TYPE8_void( func, v1type, v2type, v3type, v4type, v5type, v6type, v7type, v8type)	\
void func(int cch, v1type var1, v2type var2, v3type var3, v4type var4, v5type var5, v6type var6, v7type var7, v8type var8)	\
{										\
	DECLARE_BASIC_SND_FXS( ; );			\
										\
	p_snd ->fxs_ops ->func( p_snd, var1, var2, var3, var4, var5, var6, var7, var8 );	\
}

//void SLIC_reset(int CH, int codec_law)
IMPLEMENT_TYPE1_void( SLIC_reset, int );
//void FXS_Ring( uint32 cch, unsigned char ring_set )
IMPLEMENT_TYPE1_void( FXS_Ring, unsigned char );
//unsigned char FXS_Check_Ring( uint32 cch )
IMPLEMENT_TYPE0( FXS_Check_Ring, unsigned char );

//void Set_SLIC_Tx_Gain( uint32 cch, unsigned char tx_gain )
IMPLEMENT_TYPE1_void( Set_SLIC_Tx_Gain, int );
//void Set_SLIC_Rx_Gain( uint32 cch, unsigned char rx_gain )
IMPLEMENT_TYPE1_void( Set_SLIC_Rx_Gain, int );
//void SLIC_Set_Ring_Cadence( uint32 cch, unsigned short OnMsec, unsigned short OffMsec )
IMPLEMENT_TYPE2_void( SLIC_Set_Ring_Cadence, unsigned short, unsigned short );
//void SLIC_Set_Multi_Ring_Cadence( uint32 cch, unsigned short OnMsec1, unsigned short OffMsec1, unsigned short OnMsec2, unsigned short OffMsec2,
//	unsigned short OnMsec3, unsigned short OffMsec3, unsigned short OnMsec4, unsigned short OffMsec4)
IMPLEMENT_TYPE8_void( SLIC_Set_Multi_Ring_Cadence, unsigned short, unsigned short, unsigned short, unsigned short, unsigned short, unsigned short, unsigned short, unsigned short );
//void SLIC_Set_Ring_Freq_Amp( uint32 cch, char preset )
IMPLEMENT_TYPE1_void( SLIC_Set_Ring_Freq_Amp, char );
//void SLIC_Set_Impendance_Country( uint32 cch, unsigned short country, unsigned short impd )
IMPLEMENT_TYPE2_void( SLIC_Set_Impendance_Country, unsigned short, unsigned short );
//void SLIC_Set_Impendance( uint32 cch, unsigned short preset )
IMPLEMENT_TYPE1_void( SLIC_Set_Impendance, unsigned short );
//void OnHookLineReversal( uint32 cch, unsigned char bReversal ) //0: Forward On-Hook Transmission, 1: Reverse On-Hook Transmission
IMPLEMENT_TYPE1_void( OnHookLineReversal, unsigned char );
//void SLIC_Set_LineVoltageZero( uint32 cch )
IMPLEMENT_TYPE0_void( SLIC_Set_LineVoltageZero );
//void SLIC_CPC_Gen( uint32 cch, unsigned int time_in_ms_of_cpc_signal )
//IMPLEMENT_TYPE1_void( SLIC_CPC_Gen, unsigned int );
//void SLIC_CPC_CheckAndStop( uint32 cch )	// check in timer
//IMPLEMENT_TYPE0_void( SLIC_CPC_CheckAndStop );
//unsigned int FXS_Line_Check( uint32 cch )	// Note: this API may cause watch dog timeout. Should it disable WTD?
IMPLEMENT_TYPE0( FXS_Line_Check, unsigned int );

//void SendNTTCAR( uint32 cch )
IMPLEMENT_TYPE0_void( SendNTTCAR );
//unsigned int SendNTTCAR_check( uint32 cch, unsigned long time_out )
IMPLEMENT_TYPE1( SendNTTCAR_check, unsigned int, unsigned long );
//void disableOscillators( uint32 cch )
IMPLEMENT_TYPE0_void( disableOscillators );

//void SetOnHookTransmissionAndBackupRegister( uint32 cch ) // use for DTMF caller id
IMPLEMENT_TYPE0_void( SetOnHookTransmissionAndBackupRegister );
//void RestoreBackupRegisterWhenSetOnHookTransmission( uint32 cch ) // use for DTMF caller id
IMPLEMENT_TYPE0_void( RestoreBackupRegisterWhenSetOnHookTransmission );

//void SLIC_Set_PCM_state( uint32 cch, int enable )
IMPLEMENT_TYPE1_void( SLIC_Set_PCM_state, int );
//unsigned char SLIC_Get_Hook_Status( uint32 cch )
IMPLEMENT_TYPE1( SLIC_Get_Hook_Status, unsigned char, int );
//void SLIC_Set_Power_Save_Mode(int chid);
IMPLEMENT_TYPE0_void( SLIC_Set_Power_Save_Mode );
//void SLIC_Set_FXS_Line_State(int chid, int state);
IMPLEMENT_TYPE1_void( SLIC_Set_FXS_Line_State, int );

//void SLIC_read_reg( uint32 cch, unsigned int num, unsigned char len, unsigned char *val )
IMPLEMENT_TYPE3_void( SLIC_read_reg, unsigned int, unsigned char *, unsigned char * );
//void SLIC_write_reg( uint32 cch, unsigned char num, unsigned char len, unsigned char val )
IMPLEMENT_TYPE3_void( SLIC_write_reg, unsigned int, unsigned char *, unsigned char * );
//void SLIC_read_ram( uint32 cch, unsigned short num, unsigned int *val )
IMPLEMENT_TYPE2_void( SLIC_read_ram, unsigned int, unsigned int * );
//void SLIC_write_ram( uint32 cch, unsigned short num, unsigned int val )
IMPLEMENT_TYPE2_void( SLIC_write_ram, unsigned int, unsigned int );
//void SLIC_dump_reg( uint32 cch )
IMPLEMENT_TYPE0_void( SLIC_dump_reg );
//void SLIC_dump_ram( uint32 cch )
IMPLEMENT_TYPE0_void( SLIC_dump_ram );
//void FXS_FXO_DTx_DRx_Loopback( uint32 cch, unsigned int enable )
IMPLEMENT_TYPE2_void( FXS_FXO_DTx_DRx_Loopback, voip_snd_t *, unsigned int );
void FXS_FXO_DTx_DRx_Loopback_greedy( uint32 cch, unsigned int enable )
{
	voip_snd_t * daa_snd;
	
	DECLARE_BASIC_SND_FXS( ; );
	
	daa_snd = p_snd + 1;		// assume FXS/DAA instance is continual 
	
	if( daa_snd ->snd_type != SND_TYPE_DAA )
		return;
	
	p_snd ->fxs_ops ->FXS_FXO_DTx_DRx_Loopback( p_snd, daa_snd, enable );
}

//void SLIC_OnHookTrans_PCM_start( uint32 cch )
IMPLEMENT_TYPE0_void( SLIC_OnHookTrans_PCM_start );

//void SLIC_set_param( uint32 cch, unsigned int slic_type, unsigned int param_type, unsigned char* pParam, unsigned int param_size );
IMPLEMENT_TYPE4_void( SLIC_set_param, unsigned int, unsigned int, unsigned char*, unsigned int );

#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES
//static int SLIC_MerteringPulse_zarlink(voip_snd_t *this, uint16 hz, uint16 onTime, uint16 offTime, uint16 numMeters)
IMPLEMENT_TYPE4(MeteringPulse, int, uint16, uint16, uint16, uint16);
#endif

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST) || defined(CONFIG_RTK_VOIP_DRIVERS_SILAB_SLIC_LINETEST) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LINETEST)
//int SLIC_PortDetect_zarlink(voip_snd_t *this)
#if 0
IMPLEMENT_TYPE1( PortDetect, int, unsigned char *);
IMPLEMENT_TYPE1( LineROH, int, unsigned char *);
IMPLEMENT_TYPE1( LineVOLTAGE, int, unsigned char *);
IMPLEMENT_TYPE1( LineRESFLT, int, unsigned char *);
#endif
IMPLEMENT_TYPE2( LineTest, int, uint16, unsigned char *);

#endif

IMPLEMENT_TYPE1( GetCalibrationCoeff, int, unsigned char *);
IMPLEMENT_TYPE1( SetCalibrationCoeff, int, unsigned char *);
IMPLEMENT_TYPE1( DoCalibration, int, unsigned char);

