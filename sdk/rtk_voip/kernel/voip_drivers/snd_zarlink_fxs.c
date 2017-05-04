#include <linux/kernel.h>
#include "voip_types.h"
#include "voip_debug.h"
#include "con_register.h"
#include "snd_define.h"
#include "zarlink_api.h"
#include "snd_zarlink_common.h"
#include "con_ring.h"
// --------------------------------------------------------
// zarlink fxs ops 
// --------------------------------------------------------

extern int con_ch_num;
extern const voip_con_t *get_const_con_ptr( uint32 cch );
static void FXS_Ring_zarlink(voip_snd_t *this, unsigned char ringset )
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	uint32 const chid = (this->con_ptr)->cch;
	int ringset_int = ringset;
	
	if (ringset_int == 1)
	{
	ZarlinkFxsRing(pLine, ringset);
		MultiRingStart(chid);
	}
	else if (ringset_int == 0)
	{
		ZarlinkFxsRing(pLine, ringset);
		MultiRingStop(chid);
	}
	if (ringset_int == (1+MRC_RING_CTRL_OFFSET)) // Only FOR Multi-Ring Cadence
	{
		ringset = ringset - MRC_RING_CTRL_OFFSET;
		ZarlinkFxsRing(pLine, ringset);
	}
	else if (ringset_int == (0+MRC_RING_CTRL_OFFSET))  // Only FOR Multi-Ring Cadence
	{
		ringset = ringset - MRC_RING_CTRL_OFFSET;
		ZarlinkFxsRing(pLine, ringset);
	}
}

static unsigned char FXS_Check_Ring_zarlink(voip_snd_t *this)
{
	unsigned char ringer; //0: ring off, 1: ring on
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;

	ringer = ZarlinkCheckFxsRing(pLine);

	return ringer;
}

static void Adjust_SLIC_Tx_Gain_zarlink(voip_snd_t *this, int tx_gain)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;

	ZarlinkAdjustSlicTxGain(pLine, tx_gain);
}

static void Adjust_SLIC_Rx_Gain_zarlink(voip_snd_t *this, int rx_gain)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;

	ZarlinkAdjustSlicRxGain(pLine, rx_gain);
}

static void SLIC_Set_Ring_Cadence_zarlink(voip_snd_t *this, unsigned short OnMsec, unsigned short OffMsec)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	uint32 cch;
	int multi_ring_set_flag = 0;
	
	for(cch = 0 ; cch < con_ch_num ; cch++){	//used to check if any channel has been set multi ring
		const voip_con_t* con_ptr = get_const_con_ptr(cch);
		if(MultiRingCadenceEnableCheck_con(con_ptr) == 1){
			multi_ring_set_flag = 1;
			break;
		}
	}
	if(multi_ring_set_flag == 1){
		uint32 const chid = (this->con_ptr)->cch;
		
        	MultiRingCadenceEnable(chid, 1, OnMsec, OffMsec, 0, 0, 0, 0, 0, 0);
	}
	else{
	ZarlinkSetRingCadence(pLine, OnMsec, OffMsec);
	}
}

static void SLIC_Set_Multi_Ring_Cadence_zarlink(voip_snd_t *this, unsigned short OnMsec1, unsigned short OffMsec1, unsigned short OnMsec2, unsigned short OffMsec2, unsigned short OnMsec3, unsigned short OffMsec3, unsigned short OnMsec4, unsigned short OffMsec4)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	uint32 const chid = (this->con_ptr)->cch;
	unsigned short max_on, max_off;
	uint32 cch;	
	
	max_on = max_off = 60000;
	ZarlinkSetRingCadence(pLine, max_on, max_off);
	
	MultiRingCadenceEnable(chid, 1, OnMsec1, OffMsec1, OnMsec2, OffMsec2, OnMsec3, OffMsec3, OnMsec4, OffMsec4);
	PRINT_R("%s enable multi ring cadence\n", __FUNCTION__);
	for(cch = 0 ; cch < con_ch_num ; cch++){        //used to check if any channel has not been set by multi ring
                const voip_con_t* con_ptr = get_const_con_ptr(cch);
                if(MultiRingCadenceEnableCheck_con(con_ptr) == 0){
			unsigned short reset_OnMsec = 2000;
			unsigned short reset_OffMsec = 4000;	
			RTKLineObj * const reset_pLine = (RTKLineObj * )(con_ptr->snd_ptr->priv);			
			if(reset_pLine->pDev->cad_on_ms > 0){
				
				reset_OnMsec = reset_pLine->pDev->cad_on_ms;
			}
			if(reset_pLine->pDev->cad_off_ms > 0){
                                
                                reset_OffMsec = reset_pLine->pDev->cad_off_ms;
                        }
                        
                	MultiRingCadenceEnable(cch, 1, reset_OnMsec, reset_OffMsec, 0, 0, 0, 0, 0, 0);
                }
        }
}

static void SLIC_Set_Ring_Freq_Amp_zarlink(voip_snd_t *this, char preset)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	ZarlinkSetRingFreqAmp(pLine, preset);
}

static void SLIC_Set_Impendance_Country_zarlink(voip_snd_t *this, unsigned short country, unsigned short impd)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	ZarlinkSetImpedenceCountry(pLine, (unsigned char)country);
}

static void SLIC_Set_Impendance_zarlink(voip_snd_t *this, unsigned short preset)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	ZarlinkSetImpedence(pLine, preset);
}

#if 0
static void SLIC_GenProcessTone(unsigned int chid, genTone_struct *gen_tone)
{
}
#endif


static void OnHookLineReversal_zarlink(voip_snd_t *this, unsigned char bReversal) //0: Forward On-Hook Transmission, 1: Reverse On-Hook Transmission
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	ZarlinkSetOHT(pLine, bReversal);
	
	//printk("--> OnHookLineReversal_zarlink.\n");
}

static void SLIC_Set_LineVoltageZero_zarlink(voip_snd_t *this)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	ZarlinkSetLineOpen(pLine);
	//ZarlinkSetLineState(pLine, VP_LINE_DISCONNECT);
}

static uint8 SLIC_CPC_Gen_zarlink(voip_snd_t *this)
{
#if 0	// con_polling.c: SLIC_CPC_Gen_cch() do this 
	extern void HookPollingDisable(int cch);

	if (slic_cpc[chid].cpc_start != 0)
	{
		PRINT_R("SLIC CPC gen not stop, ch=%d\n", chid);
		return;
	}
#endif

	uint8 pre_linefeed;
	
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	pre_linefeed = ZarlinkGetLineState( pLine ); // save current linefeed status

	ZarlinkSetLineOpen( pLine );
	//ZarlinkSetLineState( pLine, VP_LINE_DISCONNECT );

#if 0	// con_polling.c: SLIC_CPC_Gen_cch() do this 
	slic_cpc[chid].cpc_timeout = jiffies + (HZ*time_in_ms_of_cpc_signal/1000);
	slic_cpc[chid].cpc_start = 1;
	slic_cpc[chid].cpc_stop = 0;
	HookPollFlag[chid] = 0; // disable hook pooling
#endif
	
	return pre_linefeed;
}

static void SLIC_CPC_Check_zarlink(voip_snd_t *this, uint8 pre_linefeed)	// check in timer
{
#if 0	// con_polling.c: ENTRY_SLIC_CPC_Polling() do this 
	extern void HookPollingEnable(int cch);
	
	if (slic_cpc[chid].cpc_start == 0)
		return;
#endif

	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	// Stop wink function
#if 0
	if ((slic_cpc[chid].cpc_stop == 0) && (timetick_after(timetick, slic_cpc[chid].cpc_timeout)))
#endif
	{

		//printk("set linefeed=0x%x\n", slic_cpc[chid].pre_linefeed);
		ZarlinkSetLineState(pLine, pre_linefeed);

#if 0	// con_polling.c: ENTRY_SLIC_CPC_Polling() do this 
		slic_cpc[chid].cpc_timeout2 = jiffies + (HZ*200/1000);
		slic_cpc[chid].cpc_stop = 1;
#endif

	}
	
#if 0	// con_polling.c: ENTRY_SLIC_CPC_Polling() do this 
	if ((slic_cpc[chid].cpc_stop == 1) && (timetick_after(timetick, slic_cpc[chid].cpc_timeout2)))
	{
		slic_cpc[chid].cpc_start = 0;
		//HookPollFlag[chid] = 1; // enable hook pooling
		HookPollingEnable( chid );
	}
#endif
}

/*  return value:
	0: Phone dis-connect, 
	1: Phone connect, 
	2: Phone off-hook, 
	3: Check time out ( may connect too many phone set => view as connect),
	4: Can not check, Linefeed should be set to active state first.
*/
//static inline unsigned char SLIC_Get_Hook_Status( int chid );

static inline unsigned int FXS_Line_Check_zarlink( voip_snd_t *this )	// Note: this API may cause watch dog timeout. Should it disable WTD?
{
	//unsigned long flags;
	//unsigned int v_tip, v_ring, tick=0;
	//unsigned int v_tip, v_ring, tick = 0;
	//unsigned int connect_flag = 0, time_out_flag = 0;
	//unsigned char linefeed, rev_linefeed;

	if ( 1 == this ->fxs_ops ->SLIC_Get_Hook_Status( this, 1 ) )
	{
		//PRINT_MSG("%s: Phone 0ff-hook\n",__FUNCTION__);
		return 2;
	}

	return 4;
}


static void SendNTTCAR_zarlink( voip_snd_t *this )
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	ZarlinkSendNTTCAR(pLine);
}

static unsigned int SendNTTCAR_check_zarlink(voip_snd_t *this, unsigned long time_out)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	return ZarlinkSendNTTCAR_Check(pLine, time_out);
}

static void disableOscillators_zarlink(voip_snd_t *this)
{
	printk("Not implemented!\n");
}

static void SetOnHookTransmissionAndBackupRegister_zarlink(voip_snd_t *this) // use for DTMF caller id
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	ZarlinkSetOHT(pLine, 0);
}

static inline void RestoreBackupRegisterWhenSetOnHookTransmission_zarlink(voip_snd_t *this) // use for DTMF caller id
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	ZarlinkGetLineState(pLine);//thlin test
}

#define PCMLAW_OFFSET	3

CT_ASSERT( BUSDATFMT_PCM_WIDEBAND_LINEAR - BUSDATFMT_PCM_LINEAR == PCMLAW_OFFSET );
CT_ASSERT( BUSDATFMT_PCM_WIDEBAND_ALAW - BUSDATFMT_PCM_ALAW == PCMLAW_OFFSET );
CT_ASSERT( BUSDATFMT_PCM_WIDEBAND_ULAW - BUSDATFMT_PCM_ULAW == PCMLAW_OFFSET );

static void SLIC_Set_PCM_state_zarlink(voip_snd_t *this, int enable)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	/* By the limition of API-II, LE89 series, PCM tx/rx can not be mute at the same time. */
	ZarlinkSetPcmTxOnly(pLine, ( enable ? 0 : 1 ));// mute phone SPK
	//ZarlinkSetPcmRxOnly(chid, ( enable ? 0 : 1 ));// mute phone MIC
}

static unsigned char SLIC_Get_Hook_Status_zarlink(voip_snd_t *this, int directly)
{
	unsigned char status;
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;

	status = ZarlinkGetFxsHookStatus(pLine, directly);
	
	return status;
}

static void SLIC_Set_Power_Save_Mode_zarlink(voip_snd_t *this)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	ZarlinkSetLineState(pLine, VP_LINE_STANDBY);
	
	//printk("--> SLIC_Set_Power_Save_Mode_zarlink.\n");
}

/* state: 
	0: line in-active state
	1: line active state
	2: power save state
	3: OHT
	4: OHT polrev
	5: Ring
*/
static void SLIC_Set_FXS_Line_State_zarlink(voip_snd_t *this, int state)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	switch (state)
	{
		case 0:
			state = VP_LINE_TIP_OPEN;
			break;
		case 1:
			state = VP_LINE_ACTIVE;
			break;
		case 2:
			state = VP_LINE_STANDBY;
			break;
		case 3:
			state = VP_LINE_OHT;
			break;
		case 4:
			state = VP_LINE_OHT_POLREV;
			break;
		case 5:
			state = VP_LINE_RINGING;
			break;
		case 6:
			state = VP_LINE_TALK_POLREV;
			break;
		case 7:
			state = VP_LINE_TALK;
			break;
		default:
			printk("Warnning! Error case, set to OHT state. in %s, line%d\n", __FUNCTION__, __LINE__);
			state = VP_LINE_OHT;
			break;
	}
	
	ZarlinkSetLineState(pLine, state);
}

static void SLIC_read_reg_zarlink(voip_snd_t *this, unsigned int num, unsigned char *len, unsigned char *val)
{
	RTKLineObj * pLine = (RTKLineObj * )this ->priv;

	/* check if dump all */
	if (num == 890 || num == 880 || num == 886) {
		*len = 0;
		ZarlinkDumpDevReg(pLine);

	} else  if (num%2==0) {
		/* Zarlink user odd number if register as read register */
		*len = 0; 
		return;

	}else{
		ZarlinkRWDevReg(pLine,num,len,val);
	}

	return;
}

static void SLIC_write_reg_zarlink(voip_snd_t *this, unsigned int num, unsigned char *len, unsigned char *val)
{
	RTKLineObj * pLine = (RTKLineObj * )this ->priv;

	/* Zarlink user even number of register as write register */
	if (num%2==1) {
		*len = 0;
		return;
	}

	ZarlinkRWDevReg(pLine,num,len,val);
	return;
}

static void SLIC_read_ram_zarlink(voip_snd_t *this, unsigned short num, unsigned char len, unsigned int *val)
{
	printk("%s(%d)Not support yet!\n",__FUNCTION__,__LINE__);
}

static void SLIC_write_ram_zarlink(voip_snd_t *this, unsigned short num, unsigned int val)
{
	printk("%s(%d)Not support yet!\n",__FUNCTION__,__LINE__);
}

static void SLIC_dump_reg_zarlink(voip_snd_t *this)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	ZarlinkDumpDevReg(pLine);
}

static void SLIC_dump_ram_zarlink(voip_snd_t *this)
{
	printk("%s(%d)Not support yet!\n",__FUNCTION__,__LINE__);
}

static void FXS_FXO_DTx_DRx_Loopback_zarlink(voip_snd_t *this, voip_snd_t *daa_snd, unsigned int enable)
{
	printk( "Not implement FXS_FXO loopback\n" );
}

static void SLIC_OnHookTrans_PCM_start_zarlink(voip_snd_t *this)
{
	this ->fxs_ops ->SLIC_Set_PCM_state(this, SLIC_PCM_ON);
	this ->fxs_ops ->OnHookLineReversal(this, 0);		//Forward On-Hook Transmission
	PRINT_MSG("SLIC_OnHookTrans_PCM_start, ch = %d\n", this ->sch);
}

static int enable_zarlink( voip_snd_t *this, int enable )
{
	SOLAC_PCMSetup_priv_ops( this, enable );
	this ->fxs_ops ->SLIC_Set_PCM_state( this, enable );
	
	return 0;
}

static void SLIC_set_param_zarlink(voip_snd_t *this, unsigned int slic_type, unsigned int param_type, unsigned char* pParam, unsigned int param_size)
{
	printk("%s(%d)Not support yet!\n",__FUNCTION__,__LINE__);
}

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST)
#if 0
static int SLIC_PortDetect_zarlink(voip_snd_t *this, unsigned int *val)
{
	int res = 0;
	RTKLineObj * pLine = (RTKLineObj * )this ->priv;
	res = ZarlinkPortDetection(pLine, val);
	return res;
}

static int SLIC_LineROH_zarlink(voip_snd_t *this, unsigned int *val)
{
	int res = 0;
	RTKLineObj * pLine = (RTKLineObj * )this ->priv;
	res = ZarlinkLTROH(pLine, val);
	return res;
}

static int SLIC_LineVOLTAGE_zarlink(voip_snd_t *this, unsigned int *val)
{
	int res = 0;
	RTKLineObj * pLine = (RTKLineObj * )this ->priv;
	res = ZarlinkLTLINEVOLTAGE(pLine, val);
	return res;
}

static int SLIC_LineRESFLT_zarlink(voip_snd_t *this, unsigned int *val)
{
	int res = 0;
	RTKLineObj * pLine = (RTKLineObj * )this ->priv;
	res = ZarlinkLTRESFLT(pLine, val);
	return res;
}
#endif

static int SLIC_LineTest_zarlink(voip_snd_t *this, uint16 tID, unsigned char *val)
{
	int res = 0;
	RTKLineObj * pLine = (RTKLineObj * )this ->priv;
	res = ZarlinkLineTest(pLine, tID, val);
	return res;
}


#endif

static int SLIC_GetCalibrationCoeff_zarlink(voip_snd_t *this, unsigned char *coeff)
{
	unsigned char status;
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;

	status = ZarlinkGetCalibrationCoeff(pLine, coeff);
	
	return status;
}

static int SLIC_SetCalibrationCoeff_zarlink(voip_snd_t *this, unsigned char *coeff)
{
	unsigned char status;
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;

	status = ZarlinkSetCalibrationCoeff(pLine, coeff);
	
	return status;
}

static int SLIC_DoCalibration_zarlink(voip_snd_t *this, unsigned char mode)
{
	unsigned char status;
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;

	status = ZarlinkDoCalibration(pLine, mode);
	
	return status;
}

#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES
static int SLIC_MerteringPulse_zarlink(voip_snd_t *this, uint16 hz, uint16 onTime, uint16 offTime, uint16 numMeters)
{
	int res = 0;
	RTKLineObj * pLine = (RTKLineObj * )this ->priv;
	res = ZarlinkMeteringPulse(pLine, hz, onTime, offTime, numMeters);
	return res;
}
#endif

// --------------------------------------------------------
// channel mapping architecture 
// --------------------------------------------------------

const snd_ops_fxs_t snd_zarlink_fxs_ops = {
	// common operation 
	.enable = enable_zarlink,
	
	// for each snd_type 
	//.SLIC_reset = SLIC_reset_zarlink,
	.FXS_Ring = FXS_Ring_zarlink,
	.FXS_Check_Ring = FXS_Check_Ring_zarlink,
	.FXS_Line_Check = FXS_Line_Check_zarlink,	// Note: this API may cause watch dog timeout. Should it disable WTD?
	.SLIC_Set_PCM_state = SLIC_Set_PCM_state_zarlink,
	.SLIC_Get_Hook_Status = SLIC_Get_Hook_Status_zarlink,
	.SLIC_Set_Power_Save_Mode = SLIC_Set_Power_Save_Mode_zarlink,
	.SLIC_Set_FXS_Line_State = SLIC_Set_FXS_Line_State_zarlink,
	
	.Set_SLIC_Tx_Gain = Adjust_SLIC_Tx_Gain_zarlink,
	.Set_SLIC_Rx_Gain = Adjust_SLIC_Rx_Gain_zarlink,
	.SLIC_Set_Ring_Cadence = SLIC_Set_Ring_Cadence_zarlink,
	.SLIC_Set_Multi_Ring_Cadence = SLIC_Set_Multi_Ring_Cadence_zarlink,
	.SLIC_Set_Ring_Freq_Amp = SLIC_Set_Ring_Freq_Amp_zarlink,
	.SLIC_Set_Impendance_Country = SLIC_Set_Impendance_Country_zarlink, 
	.SLIC_Set_Impendance = SLIC_Set_Impendance_zarlink,
	.OnHookLineReversal = OnHookLineReversal_zarlink,	//0: Forward On-Hook Transmission, 1: Reverse On-Hook Transmission
	.SLIC_Set_LineVoltageZero = SLIC_Set_LineVoltageZero_zarlink,
	
	.SLIC_CPC_Gen = SLIC_CPC_Gen_zarlink,
	.SLIC_CPC_Check = SLIC_CPC_Check_zarlink,	// check in timer
	
	.SendNTTCAR = SendNTTCAR_zarlink,
	.SendNTTCAR_check = SendNTTCAR_check_zarlink,
	
	.disableOscillators = disableOscillators_zarlink,
	
	.SetOnHookTransmissionAndBackupRegister = SetOnHookTransmissionAndBackupRegister_zarlink,	// use for DTMF caller id
	.RestoreBackupRegisterWhenSetOnHookTransmission = RestoreBackupRegisterWhenSetOnHookTransmission_zarlink,	// use for DTMF caller id
	
	.FXS_FXO_DTx_DRx_Loopback = FXS_FXO_DTx_DRx_Loopback_zarlink,
	.SLIC_OnHookTrans_PCM_start = SLIC_OnHookTrans_PCM_start_zarlink,
	.SLIC_set_param = SLIC_set_param_zarlink,
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST)
#if 0
	.PortDetect = SLIC_PortDetect_zarlink,
	.LineROH = SLIC_LineROH_zarlink,
	.LineVOLTAGE = SLIC_LineVOLTAGE_zarlink,
	.LineRESFLT = SLIC_LineRESFLT_zarlink,
#endif	
	.LineTest = SLIC_LineTest_zarlink,
#endif
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES
	.MeteringPulse = SLIC_MerteringPulse_zarlink,
#endif
	.GetCalibrationCoeff	= SLIC_GetCalibrationCoeff_zarlink,
	.SetCalibrationCoeff	= SLIC_SetCalibrationCoeff_zarlink,
	.DoCalibration			= SLIC_DoCalibration_zarlink,
	
	// read/write register/ram
	.SLIC_read_reg = SLIC_read_reg_zarlink,
	.SLIC_write_reg = SLIC_write_reg_zarlink,
	.SLIC_read_ram = SLIC_read_ram_zarlink,
	.SLIC_write_ram = SLIC_write_ram_zarlink,
	.SLIC_dump_reg = SLIC_dump_reg_zarlink,
	.SLIC_dump_ram = SLIC_dump_ram_zarlink,
	
	//.SLIC_show_ID = ??
};

