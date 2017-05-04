#ifndef __SND_MUX_SLIC_H__
#define __SND_MUX_SLIC_H__

#include "voip_types.h"

extern void SLIC_reset( uint32 cch, int codec_law );

extern void FXS_Ring( uint32 cch, unsigned char ring_set );
extern unsigned char FXS_Check_Ring( uint32 cch );

extern void Set_SLIC_Tx_Gain( uint32 cch, int tx_gain );
extern void Set_SLIC_Rx_Gain( uint32 cch, int rx_gain );
extern void SLIC_Set_Ring_Cadence( uint32 cch, unsigned short OnMsec, unsigned short OffMsec );
extern void SLIC_Set_Multi_Ring_Cadence( uint32 cch, 
		unsigned short OnMsec1, unsigned short OffMsec1, unsigned short OnMsec2, unsigned short OffMsec2,
		unsigned short OnMsec3, unsigned short OffMsec3, unsigned short OnMsec4, unsigned short OffMsec4 );
extern void SLIC_Set_Ring_Freq_Amp( uint32 cch, char preset );
extern void SLIC_Set_Impendance_Country( uint32 cch, unsigned short country, unsigned short impd );
extern void SLIC_Set_Impendance( uint32 cch, unsigned short preset );
extern void OnHookLineReversal( uint32 cch, unsigned char bReversal ); //0: Forward On-Hook Transmission, 1: Reverse On-Hook Transmission
extern void SLIC_Set_LineVoltageZero( uint32 cch );
//extern void SLIC_CPC_Gen( uint32 cch, unsigned int time_in_ms_of_cpc_signal );
//extern void SLIC_CPC_CheckAndStop( uint32 cch );	// check in timer
extern unsigned int FXS_Line_Check( uint32 cch );	// Note: this API may cause watch dog timeout. Should it disable WTD?

extern void SendNTTCAR( uint32 cch );
extern unsigned int SendNTTCAR_check( uint32 cch, unsigned long time_out );
extern void disableOscillators( uint32 cch );
extern void SetOnHookTransmissionAndBackupRegister( uint32 cch ); // use for DTMF caller id

extern void RestoreBackupRegisterWhenSetOnHookTransmission( uint32 cch ); // use for DTMF caller id
extern void SLIC_Set_PCM_state( uint32 cch, int enable );
extern unsigned char SLIC_Get_Hook_Status( uint32 cch, int directly );
extern void SLIC_Set_Power_Save_Mode( uint32 cch);
extern void SLIC_Set_FXS_Line_State( uint32 cch, int state);

extern void SLIC_read_reg( uint32 cch, unsigned int num, unsigned char * len, unsigned char *val );
extern void SLIC_write_reg( uint32 cch, unsigned int num, unsigned char * len, unsigned char *val );
extern void SLIC_read_ram( uint32 cch, unsigned short num, unsigned int *val );
extern void SLIC_write_ram( uint32 cch, unsigned short num, unsigned int val );
extern void SLIC_dump_reg( uint32 cch );
extern void SLIC_dump_ram( uint32 cch );
//extern void FXS_FXO_DTx_DRx_Loopback( uint32 cch, unsigned int enable );
extern void FXS_FXO_DTx_DRx_Loopback_greedy( uint32 cch, unsigned int enable );
extern void SLIC_OnHookTrans_PCM_start( uint32 cch );
extern void SLIC_set_param(uint32 cch, unsigned int slic_type, unsigned int param_type, unsigned char* pParam, unsigned int param_size);

#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES
extern int MeteringPulse(uint32 cch, uint16 hz, uint16 onTime, uint16 offTime, uint16 numMeters);
#endif
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST) || defined(CONFIG_RTK_VOIP_DRIVERS_SILAB_SLIC_LINETEST) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LINETEST)
#if 0
extern int PortDetect(uint32 cch, unsigned char *val);
extern int LineROH(uint32 cch, unsigned char *val);
extern int LineVOLTAGE(uint32 cch, unsigned char *val);
#endif
extern int LineTest(uint32 cch, uint16 tID, unsigned char *val);
#endif

extern int GetCalibrationCoeff(uint32 cch, unsigned char *coeff);
extern int SetCalibrationCoeff(uint32 cch, unsigned char *coeff);
extern int DoCalibration(uint32 cch, unsigned char mode);

#endif /* __SND_MUX_SLIC_H__ */

