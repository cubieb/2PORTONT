
#include <linux/types.h>
#include "rtk_voip.h"
#ifdef FXO_BUSY_TONE_DET
#include "voip_types.h"
#include "voip_control.h"
#include "tone_det_i.h"
#endif
#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
#include "resampler.h"
#endif

/*
   ISO C99 standard
   int8_t, int16_t, int32_t, int64_t,
   uint8_t, uint16_t, uint32_t, uint64_t
 */

int rtcp_sid_offset = RTCP_SID_OFFSET;

//#ifdef CONFIG_RTK_VOIP_IPC_ARCH
//int rtkVoipIsEthernetDsp = 1;
//#else
//int rtkVoipIsEthernetDsp = 0;
//#endif

#ifndef DTMF_DET_DURATION_HIGH_ACCURACY
uint32_t dtmf_data1[2/*dir*/][DSP_RTK_CH_NUM*480] __attribute__((aligned(8)));
#endif

typedef struct
{
	int data0 __attribute__((aligned(16))); /* aligned in a cache line */
	int data1;
	int data2;
	int data3;
	int data4;
	int data5;
	int data6;
	int data7;

	int data8;
	int data9;
	int data10;
	int data11;
	int data12;
	int data13;
	int data14;
}
TstVoipbusytone_det_data;

TstVoipbusytone_det_data stVoipbusytone_det_data[DSP_RTK_CH_NUM];

extern void* pstVoipbusytone_det[];


int dsp_init_var(void)
{
	int chid;

#ifdef FXO_BUSY_TONE_DET
	for (chid=0 ; chid<DSP_RTK_CH_NUM ; chid++)
	{
		pstVoipbusytone_det[chid] = &stVoipbusytone_det_data[chid];
		busy_tone_det_init(chid);
	}

	if (busy_tone_size() > sizeof(TstVoipbusytone_det_data) )
		printk("ERROR size not correct\n");
#endif

	for (chid=0 ; chid<DSP_RTK_CH_NUM ; chid++)
	{

#ifdef SUPPORT_FAX_V21_DETECT
		extern void init_fax_v21(unsigned int chid);
		init_fax_v21(chid);
#endif


#if 0
		frequency_echo_state_init(chid, 80, 1024);
		/* 80 is frame sample unit:, 1024 is tap length = 128ms */
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
		extern LecObj_t RtkLecObj;
		extern void NR_init(unsigned int chid);
		RtkLecObj.EC_G168NlpInit(stVoipCfg.ch_id, 5, 5, 0);
		NR_init(chid);
#endif
	}


#if defined(SUPPORT_G722_TYPE_WN) || defined(CONFIG_RTK_VOIP_DRIVERS_IIS) || defined(CONFIG_RTK_VOIP_WIDEBAND_SUPPORT)

	int err_up, err_down;

#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
	resampler_init(MAX_NB_CHANNELS, 8000, 16000, 1, &err_up);
	resampler_init(MAX_NB_CHANNELS, 16000, 8000, 1, &err_down);
#else	
	resampler_init(DSP_RTK_SS_NUM, 8000, 16000, 1, &err_up);
	resampler_init(DSP_RTK_SS_NUM, 16000, 8000, 1, &err_down);
#endif
	
	if ((err_up !=0) || (err_down !=0))
	{
		PRINT_R("Resampler init error!\n");
	}

#endif

	// Change V.21 0x7e flag detedtion duration threshold, default is 4 (~0.1 sec)
	extern int g_hdlc_cnt;
	g_hdlc_cnt = 8; // ~ 0.2 sec

	printk("INIT VAR OK\n");


	return 0;
}



























