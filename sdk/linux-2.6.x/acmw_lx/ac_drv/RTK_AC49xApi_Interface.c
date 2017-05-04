/****************************************************************************
*
*   Company:			Audiocodes Ltd.
*
*   Project:			VoPP SoC
*
*   Hardware Module: 		AC494-EVM
*
*   File Name: 			RTK_AC49xApi_Interface.c
*
*
******************************************************************************
*
* 	DESCRIPTION:		This file is the interface between 
*				RTK VoIP Manager and AC49X API.
*
******************************************************************************/

#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <linux/delay.h>
#include <linux/init.h>
//#include <linux/devfs_fs_kernel.h>
#include <linux/spinlock.h>
//#include <asm/semaphore.h>

#include "rtk_voip.h"
#include "voip_init.h"

#include "RTK_AC49xApi_Interface.h"
#include "AC49xDrv_Api.h"
#include "CidSampleTbl.h"
//#include "si3210init.h"
//#include "Slic_api.h"
#include "snd_mux_slic.h"
#include "dsp_rtk_caller.h"
#include "voip_types.h"
#include "voip_params.h"
#include "voip_control.h"

/* Define the driver structure for holding the dsp device and channels status */
typedef struct dsp_dev_stc {

	//devfs_handle_t 		handle;
	Tac49xSetupChannelAttr 	SetupChannelAttr[ACMW_MAX_NUM_CH]; 	/* the channels status */
	Tac49xSetupDeviceAttr	SetupDeviceAttr; 			/* the device status */
	int			downloadStatus;  			/* download flag, 0=no download */
	int			initStatus;                        	/* init flag, 0=no init */
//	int 			dspCashSegment;   			/* derived from the dinamic allocation for the dsp memory */
	int			numOfChannels;
//	struct semaphore 	sem;

} DSP_DEV;


/* Global Variable */
Tac49xTxPacketParams TxPacketParams;
Tac49xTxPacketParams *pTxPacketParams = &TxPacketParams;
Tac49xRxPacketParams RxPacketParams;
__attribute__ ((aligned (8))) Tac49xPacket ac49xRxPacket;

uint32 nRxRtpStatsCountByte[MAX_VOIP_CH_NUM];
uint32 nRxRtpStatsCountPacket[MAX_VOIP_CH_NUM];
uint32 nRxRtpStatsLostPacket[MAX_VOIP_CH_NUM];
uint32 nTxRtpStatsCountByte[MAX_VOIP_CH_NUM];
uint32 nTxRtpStatsCountPacket[MAX_VOIP_CH_NUM];
uint32 gRtcpStatsUpdOk[MAX_VOIP_CH_NUM] = {0};
uint32 gAcmwChannelState[MAX_VOIP_CH_NUM] = {0};
uint32 gCloseAcmwChAfterGetRtcpStatistic[MAX_VOIP_CH_NUM] = {0}; 
uint32 gSetByassMode[MAX_VOIP_CH_NUM] = {0}; //0: not bypass, 1: fax bypass, 2: modem bypass

/* Static Variable */
//static devfs_handle_t devfs_dir_handle = NULL;
static DSP_DEV  ac_dsp_dev;
static DSP_DEV *dsp_dev = &ac_dsp_dev;
static int pre_service_type[MAX_VOIP_CH_NUM]={0};

static unsigned char fsk_ser_type[MAX_VOIP_CH_NUM] = {0};
static unsigned char fsk_msg_type[MAX_VOIP_CH_NUM] = {0};
static unsigned char fsk_cid[30] = {0};
static unsigned char fsk_date_time[20] = {0};
static unsigned char fsk_name[20] = {0};

#if 0
static unsigned int baseAddr_dspDev;
static int 	    size_dspDev;
static unsigned int baseAddr_setupDevice;
static unsigned int baseAddr_setupChannels;
#endif

/* extern */
extern void Ac49xSetSegB(int segB);
extern void dspStart(void);
extern uint32 rtpConfigOK[];
extern unsigned long gFirstRingOffTimeOut[MAX_VOIP_CH_NUM];
extern unsigned char gRingGenAfterCid[MAX_VOIP_CH_NUM];
extern int gRingCadOff[MAX_VOIP_CH_NUM];
extern char fsk_cid_enable[MAX_VOIP_CH_NUM];
extern char fsk_cid_state[MAX_VOIP_CH_NUM];			


int AcToneTable[][24]=
{
	// USA
	{ CALL_PROGRESS_SIGNAL_TYPE__CONTINUOUS, 350, 440, 10, 0, 0, 35, 15, 10, 13, 13, 0, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Dial
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE, 	 480, 620, 10, 0, 0, 35, 15, 10, 24, 24, 0, 50, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Busy
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    440, 480, 10, 0, 0, 35, 15, 10, 19, 19, 0, 200, 400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Ringback
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    440, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 30, 1000, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}, 	// Call Waiting
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    350, 440, 10, 0, 0, 35, 15, 10, 13, 13, 0, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Stutter Dial Tone

	// UK
	{ CALL_PROGRESS_SIGNAL_TYPE__CONTINUOUS, 350, 440, 10, 0, 0, 35, 15, 10, 13, 13, 0, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Dial
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    400,   0, 10, 0, 0, 35, 15, 10, 24,  0, 0, 38, 38, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Busy
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    400, 450, 10, 0, 0, 35, 15, 10, 19, 19, 0, 35, 20, 40, 20, 40, 200, 0, 0, 0, 0, 0, 0}, // Ringback
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    440, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 30, 3276, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}, 	// Call Waiting
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    400, 0, 10, 0, 0, 35, 15, 10, 13, 0, 0, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Stutter Dial Tone

	// AUSTRALIA
	{ CALL_PROGRESS_SIGNAL_TYPE__CONTINUOUS, 425, 25, 10, 0, 0, 35, 15, 10, 13, 0, 25, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Dial
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    400,  0, 10, 0, 0, 35, 15, 10, 24,  0, 0, 38, 38, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Busy
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    400, 25, 10, 0, 0, 35, 15, 10, 19, 0, 25, 40, 20, 40, 200, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Ringback
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    425, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 20, 20, 20, 440, 0, 0, 0, 0, 1, 0, 0, 0}, 	// Call Waiting
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    425, 0, 10, 0, 0, 35, 15, 10, 13, 0, 0, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Stutter Dial Tone

	// Hong Kong
	{ CALL_PROGRESS_SIGNAL_TYPE__CONTINUOUS, 350, 440, 10, 0, 0, 35, 15, 10, 13, 13, 0, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Dial
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    480, 620, 10, 0, 0, 35, 15, 10, 24, 24, 0, 50, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Busy
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    440, 480, 10, 0, 0, 35, 15, 10, 19, 19, 0, 40, 20, 40, 300, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Ringback
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    440, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 50, 50, 50, 800, 0, 0, 0, 0, 1, 0, 0, 0}, 	// Call Waiting
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    350, 440, 10, 0, 0, 35, 15, 10, 13, 13, 0, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Stutter Dial Tone

	// Japan
	{ CALL_PROGRESS_SIGNAL_TYPE__CONTINUOUS, 400, 0, 10, 0, 0, 35, 15, 10, 13, 0, 0, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Dial
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    400, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 50, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Busy
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    400, 15, 10, 0, 0, 35, 15, 10, 19, 0, 15, 100, 200, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Ringback
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    440, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 30, 3276, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}, 	// Call Waiting //@@?
	{ CALL_PROGRESS_SIGNAL_TYPE__CONTINUOUS, 350, 440, 10, 0, 0, 35, 15, 10, 13, 13, 0, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Stutter Dial Tone

	// Sweden
	{ CALL_PROGRESS_SIGNAL_TYPE__CONTINUOUS, 425, 0, 10, 0, 0, 35, 15, 10, 13, 0, 0, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Dial
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE, 	 425, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 25, 25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Busy
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    425, 0, 10, 0, 0, 35, 15, 10, 19, 0, 0, 100, 500, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Ringback
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    425, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 20, 50, 20, 0, 0, 0, 0, 0, 1, 0, 0, 0}, 	// Call Waiting
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    425, 0, 10, 0, 0, 35, 15, 10, 13, 0, 0, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Stutter Dial Tone

	// Germany
	{ CALL_PROGRESS_SIGNAL_TYPE__CONTINUOUS, 425, 0, 10, 0, 0, 35, 15, 10, 13, 0, 0, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Dial
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    425, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 48, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Busy
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    425, 0, 10, 0, 0, 35, 15, 10, 19, 0, 0, 100, 400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Ringback
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    425, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 20, 20, 20, 500, 0, 0, 0, 0, 1, 0, 0, 0}, 	// Call Waiting
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    350, 440, 10, 0, 0, 35, 15, 10, 13, 13, 0, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Stutter Dial Tone

	// France
	{ CALL_PROGRESS_SIGNAL_TYPE__CONTINUOUS, 440, 0, 10, 0, 0, 35, 15, 10, 13, 0, 0, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Dial
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    440, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 50, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Busy
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    440, 0, 10, 0, 0, 35, 15, 10, 19, 0, 0, 150, 350, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Ringback
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    440, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 30, 1000, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}, 	// Call Waiting
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    440, 0, 10, 0, 0, 35, 15, 10, 13, 13, 0, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Stutter Dial Tone

#if 0
	// Tr57
	{ CALL_PROGRESS_SIGNAL_TYPE__CONTINUOUS, 425, 0, 10, 0, 0, 35, 15, 10, 13, 0, 0, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Dial
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    425, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 20, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Busy
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    425, 0, 10, 0, 0, 35, 15, 10, 19, 0, 0, 150, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Ringback
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    440, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 30, 3276, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}, 	// Call Waiting
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    350, 440, 10, 0, 0, 35, 15, 10, 13, 13, 0, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Stutter Dial Tone
#else
	// Taiwan
	{ CALL_PROGRESS_SIGNAL_TYPE__CONTINUOUS, 350, 440, 10, 0, 0, 35, 15, 10, 13, 13, 0, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Dial
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE, 	 480, 620, 10, 0, 0, 35, 15, 10, 24, 24, 0, 50, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Busy
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    440, 480, 10, 0, 0, 35, 15, 10, 19, 19, 0, 200, 400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Ringback
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    440, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 30, 1000, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}, 	// Call Waiting
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    350, 440, 10, 0, 0, 35, 15, 10, 13, 13, 0, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Stutter Dial Tone
#endif
          
	// Belgium
	{ CALL_PROGRESS_SIGNAL_TYPE__CONTINUOUS, 425, 0, 10, 0, 0, 35, 15, 10, 13, 0, 0, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Dial
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    425, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 50, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Busy
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    425, 0, 10, 0, 0, 35, 15, 10, 19, 0, 0, 100, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Ringback
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    1400, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 18, 18, 18, 350, 0, 0, 0, 0, 1, 0, 0, 0}, 	// Call Waiting
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    440, 0, 10, 0, 0, 35, 15, 10, 13, 0, 0, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Stutter Dial Tone

	// Finland
	{ CALL_PROGRESS_SIGNAL_TYPE__CONTINUOUS, 425, 0, 10, 0, 0, 35, 15, 10, 13, 0, 0, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Dial
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    425, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 30, 30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Busy
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    425, 0, 10, 0, 0, 35, 15, 10, 19, 0, 0, 100, 400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Ringback
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    425, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 15, 15, 15, 800, 0, 0, 0, 0, 1, 0, 0, 0}, 	// Call Waiting
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    425, 0, 10, 0, 0, 35, 15, 10, 13, 0, 0, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Stutter Dial Tone

	// Italy
	{ CALL_PROGRESS_SIGNAL_TYPE__CONTINUOUS, 425, 0, 10, 0, 0, 35, 15, 10, 13, 0, 0, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Dial
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    425, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 50, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Busy
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    425, 0, 10, 0, 0, 35, 15, 10, 19, 0, 0, 100, 400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Ringback
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    425, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 40, 10, 25, 10, 15, 0, 0, 0, 1, 0, 0, 0}, 	// Call Waiting
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    425, 0, 10, 0, 0, 35, 15, 10, 13, 0, 0, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Stutter Dial Tone
	
	// China
	{ CALL_PROGRESS_SIGNAL_TYPE__CONTINUOUS, 450, 0, 10, 0, 0, 35, 15, 10, 13, 0, 0, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Dial
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    450, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 35, 35, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Busy
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    450, 0, 10, 0, 0, 35, 15, 10, 19, 0, 0, 100, 400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Ringback
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    450, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 40, 400, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}, 	// Call Waiting
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    450, 0, 10, 0, 0, 35, 15, 10, 13, 0, 0, 40, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Stutter Dial Tone
	
	// Customer Spec.
	{ CALL_PROGRESS_SIGNAL_TYPE__CONTINUOUS, 350, 440, 10, 0, 0, 35, 15, 10, 13, 13, 0, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Dial
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE, 	 480, 620, 10, 0, 0, 35, 15, 10, 24, 24, 0, 50, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Busy
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    440, 480, 10, 0, 0, 35, 15, 10, 19, 19, 0, 200, 400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 	// Ringback
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    440, 0, 10, 0, 0, 35, 15, 10, 24, 0, 0, 30, 1000, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},	// Call Waiting
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    350, 440, 10, 0, 0, 35, 15, 10, 13, 13, 0, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} 	// Stutter Dial Tone

};

int AcToneTable2[][24]=
{
	{ CALL_PROGRESS_SIGNAL_TYPE__CADENCE,    700, 50, 10, 0, 0, 35, 15, 10, 0, 0, 25, 200, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} 	// Ring for IP phone
	//{ CALL_PROGRESS_SIGNAL_TYPE__SPECIAL_INFORMATION_TONE, 1800, 1000, 10, 1400, 0, 35, 15, 10, 0, 0, 0, 5, 1, 5, 1, 100, 300, 0, 0, 0, 0, 0, 0}
};

/********** Tone Setting Sample Code ***********/
#if 0
	Tac49xCallProgressDeviceConfigurationAttr	*pAttr;
	memset((void*)pAttr, 0, sizeof(Tac49xCallProgressDeviceConfigurationAttr));
	

	/*Dial tone -   425x25  continuous*/
	pAttr->Signal[0].Type	= CALL_PROGRESS_SIGNAL_TYPE__CONTINUOUS;	
	pAttr->Signal[0].ToneAFrequency	= 425;
	pAttr->Signal[0].ToneB_OrAmpModulationFrequency	= 25;
	pAttr->Signal[0].ThirdToneOfTripleBatchDurationTypeFrequency	= 0
	pAttr->Signal[0].AmFactor	= 25;	
	
	pAttr->Signal[0].ToneALevel	= 13;
	pAttr->Signal[0].ToneBLevel	= 0;
	
	pAttr->Signal[0].DetectionTimeOrCadenceFirstOnOrBurstDuration	= 300;

	pAttr->Signal[0].LowEnergyThreshold	= 35;	/* -dbm */
	pAttr->Signal[0].HighEnergyThreshold	= 0;		/* dbm  */	
	pAttr->Signal[0].TwistThreshold	= 10;	
	pAttr->Signal[0].SignalToNoiseRatioThreshold	= 15;
	pAttr->Signal[0].FrequencyDeviationThreshold	= 10;
	
	
	/* Special information tone -   1000/1400/1800   3x0.333 on 1.0 off + announcement */
	
	pAttr->Signal[1].Type	= CALL_PROGRESS_SIGNAL_TYPE__SPECIAL_INFORMATION_TONE;	
	pAttr->Signal[1].ToneAFrequency	= 1000;
	pAttr->Signal[1].ToneB_OrAmpModulationFrequency	= 1400;
	pAttr->Signal[1].ThirdToneOfTripleBatchDurationTypeFrequency	= 1800
	pAttr->Signal[1].AmFactor	= 0;	
	
	pAttr->Signal[1].ToneALevel	= 13;
	pAttr->Signal[1].ToneBLevel	= 0;
	
	pAttr->Signal[1].DetectionTimeOrCadenceFirstOnOrBurstDuration  = 30;/* When the signal is CALL_PROGRESS_SIGNAL_TYPE__SPECIAL_INFORMATION_TONE, this value represent the freq1 on duration. Units are in 10 msec.*/
	pAttr->Signal[1].CadenceFirstOffDuration	= 100;	/* When the signal is CALL_PROGRESS_SIGNAL_TYPE__SPECIAL_INFORMATION_TONE, this value represent the freq1 off duration. Units are in 10 msec.*/
	pAttr->Signal[1].CadenceSecondOnDuration	= 30		/* When the signal is CALL_PROGRESS_SIGNAL_TYPE__SPECIAL_INFORMATION_TONE, this value represent the freq2 on duration. Units are in 10 msec.*/
	pAttr->Signal[1].CadenceSeconedOffDuration	= 100;	/* When the signal is CALL_PROGRESS_SIGNAL_TYPE__SPECIAL_INFORMATION_TONE, this value represent the freq2 off duration. Units are in 10 msec.*/
	pAttr->Signal[1].CadenceThirdOnDuration		= 30		/* When the signal is CALL_PROGRESS_SIGNAL_TYPE__SPECIAL_INFORMATION_TONE, this value represent the freq3 on duration. Units are in 10 msec.*/
	pAttr->Signal[1].CadenceThirdOffDuration	= 100;	/* When the signal is CALL_PROGRESS_SIGNAL_TYPE__SPECIAL_INFORMATION_TONE, this value represent the freq3 off duration. Units are in 10 msec.*/

	pAttr->Signal[1].CadenceVoiceAddedWhileFirstOff	= CONTROL__DISABLE;
	pAttr->Signal[1].CadenceVoiceAddedWhileSecondOff	= CONTROL__DISABLE;
	pAttr->Signal[1].CadenceVoiceAddedWhileThirdOff	= CONTROL__DISABLE;
	
	pAttr->Signal[1].CadenceFourthOnDuration	= 0;
	pAttr->Signal[1].CadenceFourthOffDuration	= 0;
	pAttr->Signal[1].CadenceVoiceAddedWhileFourthOff = CONTROL__DISABLE;

	pAttr->Signal[1].LowEnergyThreshold	= 35;	/* -dbm */
	pAttr->Signal[1].HighEnergyThreshold	= 0;		/* dbm  */	
	pAttr->Signal[1].TwistThreshold	= 10;	
	pAttr->Signal[1].SignalToNoiseRatioThreshold	= 15;
	pAttr->Signal[1].FrequencyDeviationThreshold	= 10;

	{ CALL_PROGRESS_SIGNAL_TYPE__SPECIAL_INFORMATION_TONE, 1000, 1400, 10, 1800, 0, 35, 15, 10, 13, 0, 0, 30, 100, 30, 100, 30, 100, 0, 0, 0, 0, 0, 0}

#endif
/*************************************************/

extern TstDtmfClid dtmf_cid_info[];
// init DTMF caller ID global variable
void dtmf_cid_init(unsigned int chid)
{
	dtmf_cid_info[chid].bBefore1stRing = 1;		// default before 1st Ring
	dtmf_cid_info[chid].bAuto_StartEnd = 0;		// default non-auto
	dtmf_cid_info[chid].start_digit = 'A'-'A';	// default A
	dtmf_cid_info[chid].end_digit = 'C'-'A';	// default C
	memset(dtmf_cid_info[chid].data, 0 , DTMF_CLID_SIZE*sizeof(unsigned char));
}

static void SendToNetwork(int channel, char *vpNITxBuff, S16 NetPayloadSize, Tac49xProtocol portProtocol)
{
	extern void system_process_rtp_tx(unsigned char CH, unsigned int sid, void *ptr_data, unsigned int data_len);

	unsigned int sid;

	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
	{
		printk(AC_FORE_RED "channel %d is not supported: %s:%s:%d\n" AC_RESET, channel, __FILE__, __FUNCTION__, __LINE__);
		return;
	}
			

	if ((vpNITxBuff != NULL) && (NetPayloadSize>0))
	{
		sid = (2*channel+portProtocol-1);
		if (rtpConfigOK[sid] == 1)
			system_process_rtp_tx(channel, sid, vpNITxBuff, NetPayloadSize);
	}


}


static int ACMWPacketCB(char *buff,  U32 PacketSize)
{
	U16 channel;
	Tac49xPacket * pPacket = (Tac49xPacket *)buff;
	
	channel = pPacket->HpiHeader.Channel;
	
	switch(pPacket->HpiHeader.Protocol)
	{
		case PROTOCOL__RTP:
		 	SendToNetwork(channel, &pPacket->u, PacketSize-sizeof(Tac49xHpiPacketHeader), PROTOCOL__RTP);
			break;

		case PROTOCOL__RTCP:
		 	SendToNetwork(channel, &pPacket->u, PacketSize-sizeof(Tac49xHpiPacketHeader), PROTOCOL__RTCP);
		 	break;

	}
	
	return 0;
}


int32 Ac49xTxPacketCB( uint32 channel, uint32 mid, void* packet, uint32 pktLen, uint32 flags )
{
	Tac49xTxPacketParams 	TxPacketParams2;
	__attribute__ ((aligned (8))) static Tac49xPacket	ac49xPacket;	
	Tac49xTxPacketParams 	*pTxPacketParams2 = &TxPacketParams2;

	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
	{
		printk(AC_FORE_RED "channel %d is not supported: %s:%s:%d\n" AC_RESET, channel, __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	
	if ((packet == NULL) || pktLen <0)
		return -1;

	memcpy(ac49xPacket.u.Payload, (char *)packet, pktLen);

	pTxPacketParams2->TransferMedium 	= TRANSFER_MEDIUM__HOST;
	pTxPacketParams2->UdpChecksumIncluded 	= CONTROL__DISABLE;
	pTxPacketParams2->FavoriteStream 	= CONTROL__DISABLE;
	pTxPacketParams2->Device	 	= 0;
	pTxPacketParams2->Channel 		= channel;
	pTxPacketParams2->pOutPacket 		= (char *)&ac49xPacket;
	pTxPacketParams2->PacketSize 		= pktLen+sizeof(Tac49xHpiPacketHeader);
	pTxPacketParams2->Protocol 		= mid;

	
	Ac49xTransmitPacket(pTxPacketParams2);

	return 0;
}


int RtkAc49xApiOffhookAction(int chid)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	int res;
	
	(&TxPacketParams)->Channel = chid;
	channel = (&TxPacketParams)->Channel;

	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
	{
		printk(AC_FORE_RED "channel %d is not supported: %s:%s:%d\n" AC_RESET, channel, __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
		
	res = Ac49xAdvancedChannelConfiguration(device, channel, &dsp_dev->SetupChannelAttr[channel].Advanced);
	
	if (res == BUFFER_DESCRIPTOR_FULL_ERROR )
	{
		printk(AC_FORE_RED "Ac49xAdvancedChannelConfiguration: BUFFER_DESCRIPTOR_FULL_ERROR: %s:%s:%d\n" AC_RESET, __FILE__, __FUNCTION__, __LINE__);
		return res;
	}
	else if (res == TX_BD_INDEX_ERROR || res == TX_BAD_PACKER_ADDRESS_ERROR )
	{
		printk(AC_FORE_RED "Ac49xAdvancedChannelConfiguration: Fatal Error: %s:%s:%d\n" AC_RESET, __FILE__, __FUNCTION__, __LINE__);
		return res;
	}
	else if (res == 0)
	{
		//PRINT_MSG("Ac49xAdvancedChannel: OK\n");
	}
		
	res = Ac49xOpenChannelConfiguration(device, channel, &dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel);
	
	if (res == BUFFER_DESCRIPTOR_FULL_ERROR )
	{
		printk(AC_FORE_RED "Ac49xOpenChannelConfiguration: BUFFER_DESCRIPTOR_FULL_ERROR: %s:%s:%d\n" AC_RESET, __FILE__, __FUNCTION__, __LINE__);
	}
	else if (res == TX_BD_INDEX_ERROR || res == TX_BAD_PACKER_ADDRESS_ERROR )
	{
		printk(AC_FORE_RED "Ac49xOpenChannelConfiguration: BUFFER_DESCRIPTOR_FULL_ERROR: %s:%s:%d\n" AC_RESET, __FILE__, __FUNCTION__, __LINE__);
	}
	else if (res == 0)
	{
		//PRINT_MSG("Ac49xOpenChannel: OK\n");
		gAcmwChannelState[channel] = CH_OPEN;
	}
	
	PRINT_MSG("Off-action(%d)\n", chid);
		
	return res;
}

int RtkAc49xApiOnhookAction(int chid)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	int res;
	
	(&TxPacketParams)->Channel = chid;
	channel = (&TxPacketParams)->Channel;

	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
	{
		printk(AC_FORE_RED "channel %d is not supported: %s:%s:%d\n" AC_RESET, channel, __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	
	res = Ac49xCloseChannelConfiguration(device, channel);
	
	if (res == BUFFER_DESCRIPTOR_FULL_ERROR )
	{
		printk(AC_FORE_RED "Ac49xCloseChannelConfiguration: BUFFER_DESCRIPTOR_FULL_ERROR: %s:%s:%d\n" AC_RESET, __FILE__, __FUNCTION__, __LINE__);
	}
	else if (res == TX_BD_INDEX_ERROR || res == TX_BAD_PACKER_ADDRESS_ERROR )
	{
		printk(AC_FORE_RED "Ac49xCloseChannelConfiguration: BUFFER_DESCRIPTOR_FULL_ERROR: %s:%s:%d\n" AC_RESET, __FILE__, __FUNCTION__, __LINE__);
	}
	else if (res == 0)
	{
		//PRINT_MSG("Ac49xCloseChannel: OK\n");
		gAcmwChannelState[channel] = CH_CLOSE;
	}
	
	PRINT_MSG("On-action(%d)\n", chid);
		
	return res;
}



int RtkAc49xApiActiveRegularRtp(int chid, int sid)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	int res;

	(&TxPacketParams)->Channel = chid;
	channel = (&TxPacketParams)->Channel;
	
	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
	{
		printk(AC_FORE_RED "channel %d is not supported: %s:%s:%d\n" AC_RESET, channel, __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
		
	res = Ac49xActivateRegularRtpChannelConfiguration(device,channel, &dsp_dev->SetupChannelAttr[channel].ActivateOrUpdateRtp);
	
	if (res == BUFFER_DESCRIPTOR_FULL_ERROR )
	{
		printk(AC_FORE_RED "Ac49xActivateRegularRtpChannelConfiguration: BUFFER_DESCRIPTOR_FULL_ERROR: %s:%s:%d\n" AC_RESET, __FILE__, __FUNCTION__, __LINE__);
	}
	else if (res == TX_BD_INDEX_ERROR || res == TX_BAD_PACKER_ADDRESS_ERROR )
	{
		printk(AC_FORE_RED "Ac49xActivateRegularRtpChannelConfiguration: BUFFER_DESCRIPTOR_FULL_ERROR: %s:%s:%d\n" AC_RESET, __FILE__, __FUNCTION__, __LINE__);
	}
	else if (res == 0)
	{
		//PRINT_MSG("Ac49xActivateRegularRtp(%d): OK\n", channel);
	}
	else
	{
		PRINT_MSG("Ac49xActivateRegularRtp: return %d\n", res);
	}
	return res;
}

int RtkAc49xApiCloseRegularRtp(int chid, int sid)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	int res;

	(&TxPacketParams)->Channel = chid;
	channel = (&TxPacketParams)->Channel;
	
	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
	{
		printk(AC_FORE_RED "channel %d is not supported: %s:%s:%d\n" AC_RESET, channel, __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	
	res = Ac49xCloseRegularRtpChannelConfiguration(device, channel, &dsp_dev->SetupChannelAttr[channel].CloseRtp);
	
	if (res == BUFFER_DESCRIPTOR_FULL_ERROR )
	{
		printk(AC_FORE_RED "Ac49xCloseRegularRtpChannelConfiguration: BUFFER_DESCRIPTOR_FULL_ERROR: %s:%s:%d\n" AC_RESET, __FILE__, __FUNCTION__, __LINE__);
	}
	else if (res == TX_BD_INDEX_ERROR || res == TX_BAD_PACKER_ADDRESS_ERROR )
	{
		printk(AC_FORE_RED "Ac49xCloseRegularRtpChannelConfiguration: BUFFER_DESCRIPTOR_FULL_ERROR: %s:%s:%d\n" AC_RESET, __FILE__, __FUNCTION__, __LINE__);
	}
	else if (res == 0)
	{
		//PRINT_MSG("Ac49xCloseRegularRtp(%d): OK\n", channel);
	}
	else
	{
		PRINT_MSG("Ac49xCloseRegularRtpChannelConfiguration(%d): return %d\n", channel, res);
	}
		
	return res;
}

int RtkAc49xApiActiveOrDeactive3WayConference(int active_chid, Tac49x3WayConferenceMode mode)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel, mate_channel;
	int ret = 0;
	channel = active_chid;
	
	/*
	_3_WAY_CONFERENCE_MODE__DISABLE,   //ending 3-way confrence connction
 	_3_WAY_CONFERENCE_MODE__1,         //1-PSTN 2-IP
 	_3_WAY_CONFERENCE_MODE__2,         // NOT Supported
 	_3_WAY_CONFERENCE_MODE__3,         //2-PSTN 1-IP
 	_3_WAY_CONFERENCE_MODE__4          // NOT Supported
	*/
  
#if 0
	mate_channel = channel^1;
#else

	mate_channel= (ACMW_MAX_NUM_CH/2)+channel;
	PRINT_MSG("3way (main, mate) = (%d, %d)\n", channel, mate_channel);
	
	// Rule of (main ch, mate ch)
	// ACMW_MAX_NUM_CH = 2, (0, 1)
	// ACMW_MAX_NUM_CH = 4, (0, 2), (1, 3)
	// ACMW_MAX_NUM_CH = 8, (0, 4), (1, 5), (2, 6), (3, 7)
#endif
  
	dsp_dev->SetupChannelAttr[channel].ActivateOrDeactivate3WayConferenceConfiguration.Mode = mode;
	dsp_dev->SetupChannelAttr[channel].ActivateOrDeactivate3WayConferenceConfiguration.ConferenceChannelMate = mate_channel;
	
	if (mode == _3_WAY_CONFERENCE_MODE__1)
	{
		//RtkAc49xApiOffhookAction(channel);  /*SKIP is OK. When main channel phone off-hook, it has been call.*/
		//RtkAc49xApiOffhookAction(mate_channel);  /*MUST. If NOT, Session 1 no sound.*/
		ret = Ac49xActivateOrDeactivate3WayConferenceConfiguration(device, channel, &dsp_dev->SetupChannelAttr[channel].ActivateOrDeactivate3WayConferenceConfiguration);
		//Ac49xActivateRegularRtpChannelConfiguration(device,channel, &dsp_dev->SetupChannelAttr[channel].ActivateOrUpdateRtp); /*MUST. If NOT, Session 0,1 no sound.*/
		//Ac49xActivateRegularRtpChannelConfiguration(device,channel^1, &dsp_dev->SetupChannelAttr[channel^1].ActivateOrUpdateRtp); /*MUST. If NOT, Session 1 no sound.*/

	}
	else if (mode == _3_WAY_CONFERENCE_MODE__3)
	{
		//RtkAc49xApiOffhookAction(channel); /* SKIP is OK */
		RtkAc49xApiOffhookAction(mate_channel); /* SKIP is OK */
		ret = Ac49xActivateOrDeactivate3WayConferenceConfiguration(device, channel, &dsp_dev->SetupChannelAttr[channel].ActivateOrDeactivate3WayConferenceConfiguration);
		//Ac49xActivateRegularRtpChannelConfiguration(device,channel, &dsp_dev->SetupChannelAttr[channel].ActivateOrUpdateRtp); /*MUST. If NOT, loopback no sound.*/
		//Ac49xActivateRegularRtpChannelConfiguration(device,channel^1, &dsp_dev->SetupChannelAttr[channel^1].ActivateOrUpdateRtp); /* SKIP is OK */
	}
	else if (mode == _3_WAY_CONFERENCE_MODE__DISABLE)
	{
		ret = Ac49xActivateOrDeactivate3WayConferenceConfiguration(device, channel, &dsp_dev->SetupChannelAttr[channel].ActivateOrDeactivate3WayConferenceConfiguration);

		//Ac49xCloseRegularRtpChannelConfiguration(device, channel, &dsp_dev->SetupChannelAttr[channel].CloseRtp); // close mate channel RTP
		//Ac49xCloseRegularRtpChannelConfiguration(device, channel^1, &dsp_dev->SetupChannelAttr[channel].CloseRtp); // close mate channel RTP		
		//if (ret != 0)
			//printk("Deactive 3-Way Conference Fail!\n");
	}

	if (ret != 0)
		printk(AC_FORE_RED "(De)active 3-Way Conference Fail!\n" AC_RESET);
	else if (ret == 0)
  		PRINT_MSG("3-Way Set Mode-%d (%d)\n", mode, active_chid);	
  	
	return ret;

}

Tac49x3WayConferenceMode RtkAc49xApiGet3WayMode(int active_chid)
{
	unsigned int channel;
	channel = active_chid;
	PRINT_MSG("Get 3-way = mode-%d (%d)\n",dsp_dev->SetupChannelAttr[channel].ActivateOrDeactivate3WayConferenceConfiguration.Mode, channel);
	
	return dsp_dev->SetupChannelAttr[channel].ActivateOrDeactivate3WayConferenceConfiguration.Mode;
}

int RtkAc49xApiSetRtpChannelConfiguration(int chid, int sid, int payload_type, int g723_type, int bVAD)
{	
	unsigned int channel;
	Tac49xCoder coder = CODER__G711ALAW;

	(&TxPacketParams)->Channel = chid;
	channel = (&TxPacketParams)->Channel;

	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
	{
		printk(AC_FORE_RED "channel %d is not supported: %s:%s:%d\n" AC_RESET, channel, __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	
	dsp_dev->SetupChannelAttr[channel].ActivateOrUpdateRtp[0].RxPayloadType = payload_type;
	dsp_dev->SetupChannelAttr[channel].ActivateOrUpdateRtp[0].TxPayloadType = payload_type;

	PRINT_MSG("Set payload type= ");

	if (payload_type == 8)
	{
		coder = CODER__G711ALAW;
		PRINT_MSG("G.711a");
	}
	else if (payload_type == 0)
	{
		coder = CODER__G711MULAW;
		PRINT_MSG("G.711u");
	}
	else if (payload_type == 18)
	{
		coder = CODER__G729;
		PRINT_MSG("G.729");
	}
	else if (payload_type == 23)
	{
		coder = CODER__G726_16;
		PRINT_MSG("G.726-16k");
	}
	else if (payload_type == 22)
	{
		coder = CODER__G726_24;
		PRINT_MSG("G.726-24k");
	}
	else if (payload_type == 2)
	{
		coder = CODER__G726_32;
		PRINT_MSG("G.726-32k");
	}
	else if (payload_type == 21)
	{
		coder = CODER__G726_40;
		PRINT_MSG("G.726-40k");
	}
	else if (payload_type == 3)
	{
		coder = CODER__GSM_FULL_RATE_13_2 ;
		PRINT_MSG("GSM");
	}
	else if (payload_type == 4 && g723_type == 1)
	{
		coder = CODER__G723LOW;
		PRINT_MSG("G.723-5.3k");
	}
	else if (payload_type == 4 && g723_type == 0)
	{
		coder = CODER__G723HIGH;
		PRINT_MSG("G.723-6.3k");
	}
	
	dsp_dev->SetupChannelAttr[channel].ActivateOrUpdateRtp[0].Encoder = coder;
	dsp_dev->SetupChannelAttr[channel].ActivateOrUpdateRtp[0].Decoder = coder;
	
	/*
	SILENCE_COMPRESSION_MODE__DISABLE   
	SILENCE_COMPRESSION_MODE__ENABLE_NOISE_ADAPTATION_ENABLE   
	SILENCE_COMPRESSION_MODE__ENABLE_NOISE_ADAPTATION_DISABLE   
	*/
	
	dsp_dev->SetupChannelAttr[channel].ActivateOrUpdateRtp[0].SilenceCompressionMode = bVAD;
	PRINT_MSG(", VAD=%d, sid=%d\n", bVAD, sid);
	
	/****** Note ******
	dsp_dev->SetupChannelAttr[channel].ActivateOrUpdateRtp[1].xxx
	[Yishai Gil] it is reserved for mediated RTP channel in more complicated chips. 
	*******************/
	
	return 0;

}

int RtkAc49xApiSetRtcpTxInterval(int chid, unsigned short rtcp_tx_interval)
{	
	unsigned int channel;

	(&TxPacketParams)->Channel = chid;
	channel = (&TxPacketParams)->Channel;

	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
	{
		printk(AC_FORE_RED "channel %d is not supported: %s:%s:%d\n" AC_RESET, channel, __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	
	dsp_dev->SetupChannelAttr[channel].ActivateOrUpdateRtp[0].RtcpMeanTxInterval = rtcp_tx_interval;

	return 0;
}

unsigned char RtkAc49xApiMdmfCidParaParsing(unsigned char *pParaStart /*in*/, TeventDetectionResult *pRes /*out*/)
{
	int i = 0;
	unsigned char para = *pParaStart;
	unsigned char para_len = *(pParaStart+1);
	unsigned char *para_data = pParaStart+2;
	
	//PRINT_MSG("para= %d, para_len= %d\n", para, para_len);
	
	if (para == DATE_AND_TIME)
	{	
		PRINT_MSG("Date and Time: ");	
		for (i = 0; i < para_len; i++)
		{
			pRes->cid_date_time[i] = *(para_data+i);
			PRINT_MSG("%c", pRes->cid_date_time[i]);
		}
		pRes->cid_date_time[i] = 0; //end of the string
		PRINT_MSG("\n");
	}
	else if (para == NUMBER)
	{
		PRINT_MSG("Number: ");
		pRes->num_absence = 0;
		for (i = 0; i < para_len; i++)
		{
			pRes->cid_num[i] = *(para_data+i);
			PRINT_MSG("%c", pRes->cid_num[i]);
		}
		pRes->cid_num[i] = 0; //end of the string
		PRINT_MSG("\n");
	}
	else if (para == NUM_ABS)
	{
		PRINT_MSG("Number Absence\n");
		pRes->num_absence = 1;
	}
	else if (para == NAME)
	{
		PRINT_MSG("Name: ");
		pRes->name_absence = 0;
		for (i = 0; i < para_len; i++)
		{
			pRes->cid_name[i] = *(para_data+i);
			PRINT_MSG("%c", pRes->cid_name[i]);
		}
		pRes->cid_name[i] = 0; //end of the string
		PRINT_MSG("\n");
	}
	else if (para == NAME_ABS)
	{
		PRINT_MSG("Name Absence\n");
		pRes->name_absence = 1;
	}
	else if (para == VMWI_STATUS)
	{
		//PRINT_MSG("*para_data= %x\n", *para_data);
		
		if ((*para_data) == 0xFF)
		{	
			pRes->visual_indicator = 1;	// Active
			PRINT_MSG("VMWI On\n");
		}
		else if ((*para_data) == 0)
		{
			pRes->visual_indicator = 0;	// De-Active
			PRINT_MSG("VMWI Off\n");
		}
	}
	else if (para == CALLED_ID)
	{
		//??
	}
	else
	{
		printk(AC_FORE_RED "Can't Decode CID Parameter Type = 0x%x: %s, %s, %d\n" AC_RESET, para, __FILE__, __FUNCTION__, __LINE__);
		return 0;
	}
	
	return (para_len+2); //2 bytes means: para_type, pran _len
}

void RtkAc49xApiSdmfCidParaParsing(unsigned char *pParaStart /*in*/, unsigned char sdmf_msg_len /*in*/, TeventDetectionResult *pRes /*out*/)
{
	int i = 0;
	unsigned char *pPara_date_time = pParaStart;
	unsigned char *pPara_calling_number = pParaStart+8;
	
	pRes->name_absence = 1;
	
	/* SDMF: Date and Time*/
	
	PRINT_MSG("Date and Time: ");	
	for (i = 0; i < 8; i++)
	{
		pRes->cid_date_time[i] = *(pPara_date_time+i);
		PRINT_MSG("%c", pRes->cid_date_time[i]);
	}
	PRINT_MSG("\n");

	/* SDMF: Calling Number */
	
	if ((*pPara_calling_number) == 0x4F)
	{
		PRINT_MSG("Number Absence: Out of Area\n");
		pRes->num_absence = 1;
	}
	else if ((*pPara_calling_number) == 0x50)
	{
		PRINT_MSG("Number Absence: Private\n");
		pRes->num_absence = 1;
	}
	else
	{
		PRINT_MSG("Number: ");
		for(i=0; i < sdmf_msg_len; i++)
		{
			pRes->cid_num[i] = *(pPara_calling_number+i);
			PRINT_MSG("%c", pRes->cid_num[i]);
		}
			PRINT_MSG("\n");
	}
}

unsigned char RtkAc49xApiNttCidParsing(unsigned char *pParaStart /*in*/, unsigned char cid_len /*in*/, TeventDetectionResult *pRes /*out*/)
{
	
	unsigned char cid_para = 0, residual_len = cid_len;
	unsigned char *pPara_len = NULL, *pPara_data = NULL;
	int i = 0;
	
#if 0	
	int ntt_i=0;
	for (ntt_i = 0; ntt_i < cid_len; ntt_i++)
	{
		PRINT_G("%x ", *(pParaStart+ntt_i));
	}
	printk("\n");
#endif


PARSING:

	cid_para = *pParaStart;
	pPara_len = pParaStart + 1;
	pPara_data = pParaStart + 2;

	switch (cid_para)
	{
		case NTT_DATE_AND_TIME:			
			PRINT_MSG("NTT_DATE_AND_TIME:");
			for (i = 0; i < (*pPara_len); i++)
			{
				pRes->cid_date_time[i] = *(pPara_data+i);
				PRINT_MSG("%c", pRes->cid_date_time[i]);
			}
			pRes->cid_date_time[i] = 0; //end of the string
			PRINT_MSG("\n");
			break;
		case NTT_NUMBER:
			PRINT_MSG("NTT_NUMBER:");
			pRes->num_absence = 0;
			for (i = 0; i < (*pPara_len); i++)
			{
				pRes->cid_num[i] = *(pPara_data+i);
				PRINT_MSG("%c", pRes->cid_num[i]);
			}
			pRes->cid_num[i] = 0; //end of the string
			PRINT_MSG("\n");
			break;
		case NTT_NUM_ABS:
			pRes->num_absence = 1;
			PRINT_MSG("NTT_NUM_ABS.\n");
			break;  
		case NTT_NAME:
			PRINT_MSG("NTT_NAME:");
			pRes->name_absence = 0;
			for (i = 0; i < (*pPara_len); i++)
			{
				pRes->cid_name[i] = *(pPara_data+i);
				PRINT_MSG("%c", pRes->cid_name[i]);
			}
			pRes->cid_name[i] = 0; //end of the string
			PRINT_MSG("\n");
			break;
		case NTT_NAME_ABS:
			pRes->name_absence = 1;
			PRINT_MSG("NTT_NAME_ABS.\n");
			break;
		case NTT_DDI_NUM:
			PRINT_G("NTT_DDI_NUM:");
			for (i = 0; i < (*pPara_len); i++)
			{
				PRINT_MSG("%c", *(pPara_data+i));
			}
			PRINT_MSG("\n");
			PRINT_R("No Realy detected NTT_DDI_NUM to FXS! Need implement.\n");
			break;
		case NTT_ORIGIN_NUM:
			PRINT_G("NTT_ORIGIN_NUM:");
			for (i = 0; i < (*pPara_len); i++)
			{
				PRINT_MSG("%c", *(pPara_data+i));
			}
			PRINT_MSG("\n");
			PRINT_R("No Realy detected NTT_ORIGIN_NUM to FXS! Need implement.\n");
			break;	
		case NTT_NUMBER_EXPAN:
			PRINT_G("NTT_NUMBER_EXPAN:");
			for (i = 0; i < (*pPara_len); i++)
			{
				PRINT_MSG("%c", *(pPara_data+i));
			}
			PRINT_MSG("\n");
			PRINT_R("No Realy detected NTT_NUMBER_EXPAN to FXS! Need implement.\n");
			break;
		case NTT_DDI_NUM_EXPAN:
			PRINT_G("NTT_DDI_NUM_EXPAN:");
			for (i = 0; i < (*pPara_len); i++)
			{
				PRINT_MSG("%c", *(pPara_data+i));
			}
			PRINT_MSG("\n");
			PRINT_R("No Realy detected NTT_DDI_NUM_EXPAN to FXS! Need implement.\n");
			break;
		case NTT_ORIGIN_NUM_EXPAN:
			PRINT_G("NTT_ORIGIN_NUM_EXPAN:");
			for (i = 0; i < (*pPara_len); i++)
			{
				PRINT_MSG("%c", *(pPara_data+i));
			}
			PRINT_MSG("\n");
			PRINT_R("No Realy detected NTT_ORIGIN_NUM_EXPAN to FXS! Need implement.\n");
			break;
		case NTT_PUBLIC_MESSAGE:
			PRINT_G("NTT_PUBLIC_MESSAGE:");
			for (i = 0; i < (*pPara_len); i++)
			{
				PRINT_MSG("%c", *(pPara_data+i));
			}
			PRINT_MSG("\n");
			PRINT_R("No Realy detected NTT_PUBLIC_MESSAGE to FXS! Need implement.\n");
			break;
		
		default:
			PRINT_R("%s line %d:CID parameter 0x%x is not support.\n", __FUNCTION__, __LINE__, cid_para);
			goto RETURN;
			break;
	}
	
	residual_len -= (*pPara_len+2);
	pParaStart += (*pPara_len+2);
	//PRINT_R("residual_len = %d\n", residual_len);
	

	if (residual_len != 0)
	{
		goto PARSING;
	}
RETURN:
	return 0;
}

unsigned char RtkAc49xApiNttCidCidcwParsing(unsigned char *pParaStart /*in*/, TeventDetectionResult *pRes /*out*/)
{
	int i = 0;
	unsigned char para = *pParaStart;
	unsigned char para_len = *(pParaStart+1);
	unsigned char *para_data = pParaStart+2;
	
	//PRINT_MSG("para= %d, para_len= %d\n", para, para_len);
	

	return 0;
}

void RtkAc49xApiFskCidParsing(unsigned char *pcid_msg /*in*/, TeventDetectionResult *res /*out*/)
{

	/* 
	 * Name+number caller ID is called Multiple Data Message Format (MDMF)
	 * Number only caller ID is called Single Data Message Format (SDMF)
	 * Number include date and time
	 */
	
	/*
	=============== ETSI ================ 
	Message Type:
		- Call Setup: MDMF (80h)
		- Visual Waiting: MDMF (82h)
		- Advice of Charge: MDMF (86h)
		- Short Msg Service: : MDMF (89h)
		
	Parameters for Call Setup:
		- Date and Time: 01h
		- Calling Number : 02h
		- Calling Name: 07h
		- Called Line: 03h
		- Number Absence: 04h
		- Name Absence: 08h
		- Visual Indicator: 0Bh
		- ...
	*/	

	/*
	=============== Bellcore ================
	Message Type:
		- Call Setup: MDMF (80h)
		- Visual Waiting: MDMF (82h)
		- Call Setup: SDMF (04h)
		- Visual Waiting: SDMF (06h)
		
	Parameters for Call Setup:
		- Date and Time: 01h
		- Calling Number : 02h
		- Calling Name: 07h
		- Number Absence: 04h
		- Name Absence: 08h
		- Visual Indicator: 0Bh
		- ...
		
	*/

	/*
	=============== NTT ================
	Message Type:
		- CLIP (40h)
		- CIDCW (41h)
		- Auto Select (40h/41h)
		
	Parameters for Call Setup:
		- Calling Number : 02h
		- Calling Name: 07h
		- Number Absence: 04h
		- Name Absence: 08h
		- ...
		
	*/
	
	
	unsigned char cid_len = 0, num_len = 0, date_time_len = 0, name_len = 0, ntt_cid_len = 0;;
	unsigned char msg_type = 0, msg_len = 0;
	unsigned char *pParaEnd = NULL, *pCidEnd = NULL, *pParaIn = NULL, *pNttCidIn = NULL;
	unsigned char para_size = 0;
	
	res->cid_num[0] = 0;
	res->cid_date_time[0] = 0;
	res->cid_name[0] = 0;
	res->num_absence = 0;
	res->name_absence = 0;
	res->visual_indicator = 0;

	/* For ETSI/ Bellcore */
	cid_len = *(pcid_msg+1);
	msg_type = *(pcid_msg+2);
	msg_len = *(pcid_msg+3); // msg len doesn't include 3 bytes: msg type, msg len, check sum
	pCidEnd = (pcid_msg+3) + msg_len;
	
	//PRINT_MSG("cid_len = %d\n", cid_len);
	//PRINT_MSG("msg_type= %x\n", msg_type);
	//PRINT_MSG("msg_len= %d\n", msg_len);

	//PRINT_MSG("pCidEnd= %x\n", pCidEnd);
	
	pParaIn = pcid_msg+4;
	//PRINT_MSG("pParaIn= %x\n", pParaIn);
	
	/* For NTT */
	ntt_cid_len = *(pcid_msg+3);
	pNttCidIn = pcid_msg+4;
	
	if ( msg_type == CALL_SET_UP_MDMF)
	{	
		while(pParaEnd < pCidEnd)
		{
			pParaIn += para_size;
			//PRINT_MSG("pParaIn= %x\n", pParaIn);
			para_size = RtkAc49xApiMdmfCidParaParsing(pParaIn, res);
			if (para_size == 0)
			{
				break;
			}
			pParaEnd = pParaIn + para_size - 1;
			//PRINT_MSG("pParaEnd= %x, para_size=%d\n", pParaEnd, para_size);
			if (pParaEnd == pCidEnd)
			{
				PRINT_MSG("Parsing CID Finish!\n");
			}
		}
	}
	else if (msg_type == MDMF_VMWI)
	{
		while(pParaEnd != pCidEnd)
		{
			pParaIn += para_size;
			//PRINT_MSG("pParaIn= %x\n", pParaIn);
			para_size = RtkAc49xApiMdmfCidParaParsing(pParaIn, res);
			if (para_size == 0)
			{
				break;
			}
			pParaEnd = pParaIn + para_size - 1;
			//PRINT_MSG("pParaEnd= %x, para_size=%d\n", pParaEnd, para_size);
			if (pParaEnd == pCidEnd)
			{
				PRINT_MSG("Parsing CID Finish!\n");
			}
		}
	}
	else if (msg_type == CALL_SET_UP_SDMF)
	{
		RtkAc49xApiSdmfCidParaParsing(pParaIn, msg_len, res);
		PRINT_MSG("Parsing CID Finish!\n");
	}
	else if (msg_type == SDMF_VMWI)
	{
		if (((*pParaIn) == 0x42) && ((*(pParaIn+1)) == 0x42) && ((*(pParaIn+2)) == 0x42))
		{
			res->visual_indicator = 1;	// Active
			PRINT_MSG("VMWI(SDMF) On\n");
		}
		else if (((*pParaIn) == 0x6F) && ((*(pParaIn+1)) == 0x6F) && ((*(pParaIn+2)) == 0x6F))
		{
			res->visual_indicator = 0;	// Deactive
			PRINT_MSG("VMWI(SDMF) Off\n");
		}
		PRINT_MSG("Parsing CID Finish!\n");
	}	
	else if (msg_type == NTT_CLIP)
	{
		//PRINT_G("Get NTT_CLIP type CID!\n");
		RtkAc49xApiNttCidParsing(pNttCidIn, ntt_cid_len, res);
	}
	else if (msg_type == NTT_CIDCW)
	{
		//PRINT_G("Get NTT_CIDCW type CID!\n");
		RtkAc49xApiNttCidParsing(pNttCidIn, ntt_cid_len, res);
	}
	else
	{
		printk(AC_FORE_RED "Not Supported CID Message Type: %s, %s, %d\n" AC_RESET, __FILE__, __FUNCTION__, __LINE__);
	}
}

/* ASCII Table: char 0-9 -> Dec# 48-57
*		char A-D -> Dec# 65-68 
*		char  *  -> Dec# 42 
*		char  #  -> Dec# 35 
*/

#ifdef ACMW_PLAYBACK
#include "../../rtk_voip/voip_dsp/ivr/ivr.h"

static int pb_state[VOIP_CH_NUM] = {0};

/* For Text IVR Play */
int play_g723h_flag[VOIP_CH_NUM] = {0};
int play_g729_flag[VOIP_CH_NUM] = {0};
int play_g711_flag[VOIP_CH_NUM] = {0};
static volatile int play_text_flag[VOIP_CH_NUM] = {0};
static int play_text_idx[VOIP_CH_NUM] = {0};
static int text_play_len[VOIP_CH_NUM] = {0};
static int text_10ms_size = 80;
static unsigned char* pText_play_addr[VOIP_CH_NUM];
static unsigned char IvrTextSpeechBuf[VOIP_CH_NUM][MAX_LEN_OF_IVR_TEXT+1];
static volatile unsigned char IvrTextSpeechBufIdx[VOIP_CH_NUM] = {0};
static volatile int IvrTextSpeechEvent[VOIP_CH_NUM] = {0};

unsigned char SetTextPlayAddr_Len(unsigned char chid, unsigned char* p, unsigned int len)
{
	if ((p == NULL) || (len <= 0))
		return 0;
	pText_play_addr[chid] = p;
	text_play_len[chid] = len;

	return 1;
}

/* For File IVR Play */
#define FILE_PLAY_BUF_LEN 18
#define FRAME_SIZE 80 /* Unit: bytes*/
//#define CHECK_ROLL_BACK

char filePlayBuffer[VOIP_CH_NUM][FILE_PLAY_BUF_LEN*FRAME_SIZE] = {0};
static int filePlay_w[VOIP_CH_NUM] = {0};
static int filePlay_r[VOIP_CH_NUM] = {0};

int IvrPlayBufWrite(unsigned char chid, char* pBuf, int frame_len, int frame_size)
{

	int i=0, len=0, filePlay_w_org = filePlay_w[chid];
	
	if (chid >= VOIP_CH_NUM)
		return -1;
	
	for (i=0; i<frame_len; i++)
	{
		if (((filePlay_w[chid]+1)%FILE_PLAY_BUF_LEN) == filePlay_r[chid])
		{
			//PRINT_Y("(%d,%d)\n", filePlay_r[chid], filePlay_w[chid]);
			break;//Full
		}
		else
		{
			filePlay_w[chid] = (filePlay_w[chid]+1)%FILE_PLAY_BUF_LEN;
			len++;
		}
	}
		
	if ( (filePlay_w_org + len) <= FILE_PLAY_BUF_LEN )
	{
		memcpy(&filePlayBuffer[chid][filePlay_w_org*frame_size], pBuf, frame_size*len);
	}
	else
	{
		memcpy(&filePlayBuffer[chid][filePlay_w_org*frame_size], pBuf, frame_size*(FILE_PLAY_BUF_LEN-filePlay_w_org));
		memcpy(&filePlayBuffer[chid][0], (char*)(pBuf + frame_size*(FILE_PLAY_BUF_LEN-filePlay_w_org)), frame_size*(len-(FILE_PLAY_BUF_LEN-filePlay_w_org)));
	}

	//PRINT_Y("(%d,%d)\n",len, frame_len);
	return len;
}

int IvrPlayBufRead(unsigned char chid, int frame_len, int* index)
{
	int i=0, len=0;

	if (chid >= VOIP_CH_NUM)
		return -1;
	
	*index = filePlay_r[chid];

#ifdef CHECK_ROLL_BACK
	
	for (i=0; i<frame_len; i++)
	{
		if ( filePlay_r[chid] == filePlay_w[chid])
			break;//Empty
		else
		{
			//filePlay_r[chid] = (filePlay_r[chid]+1)%FILE_PLAY_BUF_LEN;
			if ((filePlay_r[chid]+1)%FILE_PLAY_BUF_LEN != 0)
			{
				filePlay_r[chid]++;
				len++;
			}
			else
			{
				filePlay_r[chid] = (filePlay_r[chid]+1)%FILE_PLAY_BUF_LEN;
				len++;
				break;
			}
		}
	}
#else
	for (i=0; i<frame_len; i++)
	{
		if ( filePlay_r[chid] == filePlay_w[chid])
			break;//Empty
		else
		{
			filePlay_r[chid] = (filePlay_r[chid]+1)%FILE_PLAY_BUF_LEN;
			len++;
		}
	}
#endif
	return len;
}

#if 0
int IvrPlayGetBufSize(unsigned int chid)
{
	if (chid >= VOIP_CH_NUM)
		return -1;
		
	if (filePlay_w[chid] > filePlay_r[chid])
		return (filePlay_w[chid] - filePlay_r[chid]);
	else
                return ((FILE_PLAY_BUF_LEN - filePlay_r[chid])+ filePlay_w[chid]);
}

int IvrPlayGetBufAvailable(unsigned int chid)
{
	if (chid >= VOIP_CH_NUM)
		return -1;
		
	return (FILE_PLAY_BUF_LEN - IvrPlayGetBufSize(chid));
}
#endif

void SetTextPlayBuf(unsigned int chid, unsigned char* pText)
{
	strcpy(IvrTextSpeechBuf[chid], pText);				
	IvrTextSpeechEvent[chid] = 1;
	pb_state[chid] = PB_TDM;
	//PRINT_Y("%s\n", IvrTextSpeechBuf[chid]);
}

void CheckAndPlayIvrTextSpeech(void)
{
	int ch;
	unsigned char character, *pText_play = NULL;
	unsigned int ivr_text_len = 0;
	extern const ivr_speech_wave_info_t ivr_speech_wave_info[];
	extern int voip_ch_num;
	
	for (ch=0; ch<voip_ch_num; ch++)
	{
		if (IvrTextSpeechEvent[ch] == 1)
		{
			if (play_text_flag[ch] == 0)
			{
				if ((IvrTextSpeechBufIdx[ch]+1) <= strlen(IvrTextSpeechBuf[ch]))
				{
			
					character = IvrTextSpeechBuf[ch][IvrTextSpeechBufIdx[ch]];
					//PRINT_Y("%c\n", character);
			
				    	if( character >= '0' && character <= '9' )			/* number */
					{
						pText_play = ivr_speech_wave_info[character - '0' + IVR_SPEECH_ID_NUM_0].pSpeechWave;
						ivr_text_len = ivr_speech_wave_info[character - '0' + IVR_SPEECH_ID_NUM_0].length;
					}
					else if( character >= 'a' && character <= 'z' )		/* lower case alphabet */
					{
						pText_play = ivr_speech_wave_info[character - 'a' + IVR_SPEECH_ID_ALPHA_A].pSpeechWave;
						ivr_text_len = ivr_speech_wave_info[character - 'a' + IVR_SPEECH_ID_ALPHA_A].length;
					}
					else if( character >= 'A' && character <= 'Z' )		/* upper case alphabet */
					{
						pText_play = ivr_speech_wave_info[character - 'A' + IVR_SPEECH_ID_ALPHA_A].pSpeechWave;
						ivr_text_len = ivr_speech_wave_info[character - 'A' + IVR_SPEECH_ID_ALPHA_A].length;
					}
					else if( character == '.' )							/* dot */
					{
						pText_play = ivr_speech_wave_info[IVR_SPEECH_ID_DOT].pSpeechWave;
						ivr_text_len = ivr_speech_wave_info[IVR_SPEECH_ID_DOT].length;
					}
					else 
					{
						switch (character)
						{
							case IVR_TEXT_ID_DHCP:
								pText_play = ivr_speech_wave_info[IVR_SPEECH_ID_TXT_DHCP].pSpeechWave;
								ivr_text_len = ivr_speech_wave_info[IVR_SPEECH_ID_TXT_DHCP].length;
								//PRINT_G("IVR_TEXT_ID_DHCP, ch=%d\n", ch);
								break;
					
							case IVR_TEXT_ID_FIX_IP:
								pText_play = ivr_speech_wave_info[IVR_SPEECH_ID_TXT_FIX_IP].pSpeechWave;
								ivr_text_len = ivr_speech_wave_info[IVR_SPEECH_ID_TXT_FIX_IP].length;
								//PRINT_G("IVR_TEXT_ID_FIX_IP, ch=%d\n", ch);
								break;
										
							case IVR_TEXT_ID_NO_RESOURCE:
								pText_play = ivr_speech_wave_info[IVR_SPEECH_ID_TXT_NO_RESOURCE].pSpeechWave;
								ivr_text_len = ivr_speech_wave_info[IVR_SPEECH_ID_TXT_NO_RESOURCE].length;
								//PRINT_G("IVR_TEXT_ID_NO_RESOURCE, ch=%d\n", ch);
								break;
												
							case IVR_TEXT_ID_PLZ_ENTER_NUMBER:
								pText_play = ivr_speech_wave_info[IVR_SPEECH_ID_TXT_PLZ_ENTER_NUMBER].pSpeechWave;
								ivr_text_len = ivr_speech_wave_info[IVR_SPEECH_ID_TXT_PLZ_ENTER_NUMBER].length;
								//PRINT_G("IVR_TEXT_ID_PLZ_ENTER_NUMBER, ch=%d\n", ch);
								break;
					
							default:
								break;
						}
				
					}
					
					
					IvrTextSpeechBufIdx[ch]++;
					
					if (SetTextPlayAddr_Len(ch, pText_play, ivr_text_len))
					{
						extern int play_text_idx[];
						play_text_idx[ch] = 0;
						play_text_flag[ch] = 1;
						RtkAc49xApiPlayIvrTdmStart(ch, CODER__G711ALAW);
						//PRINT_G("Play Text Start!\n");						
					}
					else
						PRINT_R("VOIP_MGR_PLAY_IVR: Play IVR Text Error, ch= %d\n", ch);
					
				}
				else
				{
					RtkAc49xApiPlayIvrEnd(ch);
			 		//PRINT_G("Play Text End!\n");
					IvrTextSpeechEvent[ch] = 0;
					IvrTextSpeechBufIdx[ch] = 0;
				}
					
				
			} // play_text_flag
				
		}
	}

}

#endif // ACMW_PLAYBACK



void RtkAc49xApiEventPolling(TeventDetectionResult *result)
{
	unsigned int ch, pktsize, r=0;
	char digit, event, coder, pcat, opcode;
	unsigned short bypass_flag = 0, type, cidEvent, eventIdx;
	extern char fsk_spec_areas[];

	pktsize = Ac49xReceivePacket(&RxPacketParams);
	result->channel = (&RxPacketParams)->Channel;

#ifdef ACMW_PLAYBACK
	CheckAndPlayIvrTextSpeech();
#endif

	if (PROTOCOL__PLAYBACK_COMMAND == RxPacketParams.Protocol)
	{
		event = ac49xRxPacket.u. PlaybackPayload.u. Command.Opcode; /*should be PLAYBACK_DSP_OPCODE__REQUEST for playback request*/
		coder = ac49xRxPacket.u.PlaybackPayload.u.Command.Coder; /*just an echo of the start playback cmd*/
		
		if (PLAYBACK_DSP_OPCODE__REQUEST == event)
		{
			int AvailableSizeInMsec;
			AvailableSizeInMsec = (ac49xRxPacket.u.PlaybackPayload.u.Command.u.Dsp.AvailableBufferSpace_msec_Msb<<8) | 
					(ac49xRxPacket.u.PlaybackPayload.u.Command.u.Dsp.AvailableBufferSpace_msec_Lsb);
	 		//PRINT_MSG("PlayBack Request, available Size %d msec\n", AvailableSizeInMsec);
	 		PRINT_MSG("PBR, %d msec\n", AvailableSizeInMsec);
#ifdef ACMW_PLAYBACK
			//PRINT_G("PBR, %d msec, %d", AvailableSizeInMsec, dsp_dev->SetupChannelAttr[result->channel].Advanced.PlaybackWaterMark_msec);
			
			if (play_text_flag[result->channel] == 1)
			{			
				/* check if Have play len + will play len > speech len */
				int have_play_len = text_10ms_size*play_text_idx[result->channel];
		 		if (( have_play_len + text_10ms_size*(AvailableSizeInMsec/10) ) > text_play_len[result->channel] )
				{
					AvailableSizeInMsec = 10*(text_play_len[result->channel] - have_play_len)/text_10ms_size;
					r = (text_play_len[result->channel] - have_play_len)%text_10ms_size;
					//PRINT_G("r= %d bytes\n", r);
				}
				else
				{
					r = 0;
				}
	
				//PRINT_Y("s%d\n", text_10ms_size*AvailableSizeInMsec/10 + r);
				RtkAc49xApiPlayIvr(result->channel, pText_play_addr[result->channel] + text_10ms_size*play_text_idx[result->channel], text_10ms_size*AvailableSizeInMsec/10 + r);
				play_text_idx[result->channel] += AvailableSizeInMsec/10;
		 		
		 		if ((text_10ms_size*play_text_idx[result->channel]+r) >=  text_play_len[result->channel] )
		 		{
					play_text_idx[result->channel] = 0;
					play_text_flag[result->channel] = 0;
					//	RtkAc49xApiPlayIvrEnd(result->channel);
		 			//	PRINT_G("Play Text End!\n");
		 		}
	 		}
	 		else if (play_g723h_flag[result->channel] == 1)
	 		{
				int read_len, idx;
				//AvailableSizeInMsec = (AvailableSizeInMsec/10)*10; // make AvailableSizeInMsec is the multiple of 10msec.
				
				//if (IvrPlayGetBufSize(result->channel) < (AvailableSizeInMsec/10))
					//AvailableSizeInMsec = IvrPlayGetBufSize(result->channel)*10;
				//PRINT_R("%d\n", AvailableSizeInMsec/10); // The NO. of frame which we want to read
				
				read_len = IvrPlayBufRead(result->channel, AvailableSizeInMsec/30 /*one frame = 30 ms*/, &idx);
				//PRINT_R("%d\n", read_len);  // The NO. of frames which are read actually
			
			#ifdef CHECK_ROLL_BACK
				RtkAc49xApiPlayIvr(result->channel, (char*)(&filePlayBuffer[result->channel][idx*G723_FRAME_SIZE]), read_len*G723_FRAME_SIZE);
			#else	
				if ((idx + read_len) <= FILE_PLAY_BUF_LEN )
					RtkAc49xApiPlayIvr(result->channel, (char*)(&filePlayBuffer[result->channel][idx*G723_FRAME_SIZE]), read_len*G723_FRAME_SIZE);
				else
				{
					RtkAc49xApiPlayIvr(result->channel, (char*)(&filePlayBuffer[result->channel][idx*G723_FRAME_SIZE]), (FILE_PLAY_BUF_LEN-idx)*G723_FRAME_SIZE);
					RtkAc49xApiPlayIvr(result->channel, (char*)(&filePlayBuffer[result->channel][0]), (read_len-(FILE_PLAY_BUF_LEN-idx))*G723_FRAME_SIZE);
				}
			#endif
				
				if (read_len == 0)
				{
					RtkAc49xApiPlayIvrEnd(result->channel);
					play_g723h_flag[result->channel] = 0;
				}
	 		}
	 		else if (play_g729_flag[result->channel] == 1)
	 		{
				int read_len, idx;
				//AvailableSizeInMsec = (AvailableSizeInMsec/10)*10; // make AvailableSizeInMsec is the multiple of 10msec.
				
				//if (IvrPlayGetBufSize(result->channel) < (AvailableSizeInMsec/10))
					//AvailableSizeInMsec = IvrPlayGetBufSize(result->channel)*10;
				//PRINT_R("%d\n", AvailableSizeInMsec/10); // The NO. of frame which we want to read
				
				read_len = IvrPlayBufRead(result->channel, AvailableSizeInMsec/10 /*one frame = 10 ms*/, &idx);
				//PRINT_R("%d\n", read_len); // The NO. of frames which are read actually
			
			#ifdef CHECK_ROLL_BACK
				RtkAc49xApiPlayIvr(result->channel, (char*)(&filePlayBuffer[result->channel][idx*G729_FRAME_SIZE]), read_len*G729_FRAME_SIZE);
			#else	
				if ((idx + read_len) <= FILE_PLAY_BUF_LEN )
					RtkAc49xApiPlayIvr(result->channel, (char*)(&filePlayBuffer[result->channel][idx*G729_FRAME_SIZE]), read_len*G729_FRAME_SIZE);
				else
				{
					RtkAc49xApiPlayIvr(result->channel, (char*)(&filePlayBuffer[result->channel][idx*G729_FRAME_SIZE]), (FILE_PLAY_BUF_LEN-idx)*G729_FRAME_SIZE);
					RtkAc49xApiPlayIvr(result->channel, (char*)(&filePlayBuffer[result->channel][0]), (read_len-(FILE_PLAY_BUF_LEN-idx))*G729_FRAME_SIZE);
				}
			#endif
				
				if (read_len == 0)
				{
					RtkAc49xApiPlayIvrEnd(result->channel);
					play_g729_flag[result->channel] = 0;
				}

	 		}
	 		else if (play_g711_flag[result->channel] == 1)
	 		{

				int read_len, idx;
				//AvailableSizeInMsec = (AvailableSizeInMsec/10)*10; // make AvailableSizeInMsec is the multiple of 10msec.
				
				//if (IvrPlayGetBufSize(result->channel) < (AvailableSizeInMsec/10))
					//AvailableSizeInMsec = IvrPlayGetBufSize(result->channel)*10;
				//PRINT_R("%d\n", AvailableSizeInMsec/10); // The NO. of frame which we want to read
				
				read_len = IvrPlayBufRead(result->channel, AvailableSizeInMsec/10 /*one frame = 10 ms*/, &idx);
				//PRINT_R("%d\n", read_len);  // The NO. of frames which are read actually
			
			#ifdef CHECK_ROLL_BACK
				RtkAc49xApiPlayIvr(result->channel, (char*)(&filePlayBuffer[result->channel][idx*G711_FRAME_SIZE]), read_len*G711_FRAME_SIZE);
			#else	
				if ((idx + read_len) <= FILE_PLAY_BUF_LEN )
					RtkAc49xApiPlayIvr(result->channel, (char*)(&filePlayBuffer[result->channel][idx*G711_FRAME_SIZE]), read_len*G711_FRAME_SIZE);
				else
				{
					RtkAc49xApiPlayIvr(result->channel, (char*)(&filePlayBuffer[result->channel][idx*G711_FRAME_SIZE]), (FILE_PLAY_BUF_LEN-idx)*G711_FRAME_SIZE);
					RtkAc49xApiPlayIvr(result->channel, (char*)(&filePlayBuffer[result->channel][0]), (read_len-(FILE_PLAY_BUF_LEN-idx))*G711_FRAME_SIZE);
				}
			#endif
				
				if (read_len == 0)
				{
					RtkAc49xApiPlayIvrEnd(result->channel);
					play_g711_flag[result->channel] = 0;
				}

	 		}
#endif // ACMW_PLAYBACK
 		}
 		//printk("\n");
 	}
 		
 	if (PROTOCOL__PROPRIETARY == RxPacketParams.Protocol)
	{
		pcat = ac49xRxPacket.u.ProprietaryPayload.ProprietaryHeader.PacketCategory;
		opcode = ac49xRxPacket.u.ProprietaryPayload.ProprietaryHeader.Opcode;
		event = ac49xRxPacket.u.ProprietaryPayload.u.StatusOrEvent.u.GeneralEvent.EventIndex;
		
		if ((PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET  == pcat) &&
			(STATUS_OR_EVENT_PACKET_OP_CODE__GENERAL_EVENT == opcode) &&
			(GENERAL_EVENT_PACKET_INDEX__PLAYBACK_ENDED == event))
		{
			PRINT_MSG("Play End\n");
		}
	}
	
	/* if pktsize > 0, than there is some information inside  */
	if (pktsize > 0)
	{
		if ((PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET == ac49xRxPacket.u.ProprietaryPayload.ProprietaryHeader.PacketCategory) &&
	            (STATUS_OR_EVENT_PACKET_OP_CODE__IBS_EVENT == ac49xRxPacket.u.ProprietaryPayload.ProprietaryHeader.Opcode))
		{	

			/****** Check DTMF Event ******/
			if( IBS_SYSTEM_CODE__DTMF == ac49xRxPacket.u.ProprietaryPayload.u.StatusOrEvent.u.IbsEvent.System)
			{
				digit = ac49xRxPacket.u.ProprietaryPayload.u.StatusOrEvent.u.IbsEvent.Digit;
			
				/* To map to the realtek voip manager */			
				if (digit == 10) // *
				{
					digit = 42;
				}
				else if (digit == 11) // #
				{
					digit = 35;
				}
				else if ((digit >=0) && (digit<=9)) // 0-9
				{
					digit = digit + 48;
				}
				else if ((digit >=12) && (digit<=15)) // A-D
	         		{
	         			digit = digit + 53;
	         		}
	         	
				if (ac49xRxPacket.u.ProprietaryPayload.u.StatusOrEvent.u.IbsEvent.LongDtmfDetection)
				{
					result->long_dtmf = 1;			
					result->dtmf_digit = 'Z';	
					PRINT_MSG("--> long DTMF ");
				}
				else
				{
					result->dtmf_digit = digit;
					result->Ibs_level = ac49xRxPacket.u.ProprietaryPayload.u.StatusOrEvent.u.IbsEvent.Level;
					PRINT_MSG("|%c| at %ddBm, chid=%d \n", digit, result->Ibs_level, result->channel);
				}
				
				
			
			}
			else if ((IBS_SYSTEM_CODE__CALL_PROGRESS_0_TO_15 == ac49xRxPacket.u.ProprietaryPayload.u.StatusOrEvent.u.IbsEvent.System) ||
				 (IBS_SYSTEM_CODE__CALL_PROGRESS_16_TO_31 == ac49xRxPacket.u.ProprietaryPayload.u.StatusOrEvent.u.IbsEvent.System))
			{
				/*
				IBS Call Progress detection status event is the same as for DTMF detection, Except for the System field.
				The return IBS status packet will contain the following:
				Digit - Call Progress digit as in the call progress table
				System - 6 (Call Progress, digits 0-15) or 7(Call Progress, digits 16-31)
				Level - CP signal level in -dBm

				*/
				result->IBS_CP = ac49xRxPacket.u.ProprietaryPayload.u.StatusOrEvent.u.IbsEvent.Digit;
				result->Ibs_level = ac49xRxPacket.u.ProprietaryPayload.u.StatusOrEvent.u.IbsEvent.Level;
				PRINT_MSG("IBS CP-%d at %ddBm is detected, chid=%d \n", result->IBS_CP, result->Ibs_level, result->channel);
			
			}
		}
		else
		{
			result->dtmf_digit = 'Z';
			result->long_dtmf = 0;
			result->IBS_CP = -1;
			result->Ibs_level = -1;
		}
	
	
		if ((PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET == ac49xRxPacket.u.ProprietaryPayload.ProprietaryHeader.PacketCategory) &&
		    (STATUS_OR_EVENT_PACKET_OP_CODE__FAX_OR_DATA_SIGNAL_EVENT == ac49xRxPacket.u.ProprietaryPayload.ProprietaryHeader.Opcode))
		{	

			/******* Check Fax/Modem Event ******/
			bypass_flag = ac49xRxPacket.u.ProprietaryPayload.u.StatusOrEvent.u. FaxOrDataStatus.FaxBypassFlag;
			//Tac49xFaxOrDataSignalEvent SignalDetectedOnDecoderOutput = ac49xRxPacket.u.ProprietaryPayload.u.StatusOrEvent.u.FaxOrDataStatus.SignalDetectedOnDecoderOutput;
                        Tac49xFaxOrDataSignalEvent SignalDetectedOnEncoderInput = ac49xRxPacket.u.ProprietaryPayload.u.StatusOrEvent.u.FaxOrDataStatus.SignalDetectedOnEncoderInput;
                        
                        if (bypass_flag == 1)
                        {
                        	switch( SignalDetectedOnEncoderInput )
                        	{
                        		case FAX_OR_DATA_SIGNAL_EVENT__FAX_CNG:
						result->cng_flag = 1;
						PRINT_MSG("Get CNG(%d)\n", result->channel);
            				case FAX_OR_DATA_SIGNAL_EVENT__ANS_TONE_2100_FAX_CED_OR_MODEM://here we get standard Fax signals
						result->ced_flag = 1;
						gSetByassMode[result->channel] = 1;
						PRINT_MSG("Get CED(%d)\n", result->channel);
						break;
            				case FAX_OR_DATA_SIGNAL_EVENT__ANS_TONE_WITH_REVERSALS:
            				case FAX_OR_DATA_SIGNAL_EVENT__ANS_TONE_AM:
            				case FAX_OR_DATA_SIGNAL_EVENT__ANS_TONE_AM_REVERSALS://here we get Data signals
            					result->modem_flag = 1;
            					PRINT_MSG("Get Modem tone(%d)\n", result->channel);
						break;

                        	}
                        }

		}
		else
		{
			result->ced_flag = 0;
			result->cng_flag = 0;
			result->modem_flag = 0;
		}
		
	
		if ((PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET == ac49xRxPacket.u.ProprietaryPayload.ProprietaryHeader.PacketCategory) &&
		    (STATUS_OR_EVENT_PACKET_OP_CODE__CALLER_ID == ac49xRxPacket.u.ProprietaryPayload.ProprietaryHeader.Opcode))
		{
			cidEvent = ac49xRxPacket.u.ProprietaryPayload.u.StatusOrEvent.u.CallerId.Event;
			char* pMessage = &(ac49xRxPacket.u.ProprietaryPayload.u.StatusOrEvent.u.CallerId.Message[0]);
			char cid_type = ac49xRxPacket.u.ProprietaryPayload.u.StatusOrEvent.u.CallerId.ServiceType;
			
			/*
			 * CallerIdPayload.Message[0] = 1 =>MESSAGE_TYPE__NONE_DTMF; 0=> MESSAGE_TYPE__DTMF
			 * CallerIdPayload.Message[1] = length of message in bytes
			 * CallerIdPayload.Message[2¡K]= message in chars
			 */


			/* Caller Id Generation */
			if (cidEvent == CALLER_ID_EVENT__GENERATION_COMPLETED_SUCCESSFULLY)
			{
				fsk_cid_state[result->channel] = 0;
				/* Before sending CID, RtkAc49xApiOffhookAction() will be called to open acmw channel. 
				   Here, call RtkAc49xApiOnhookAction() to close acmw channel. 
				   But for type-II CID, it should not be called. */
				if (CALLER_ID_SERVICE_TYPE__1_ON_HOOK == pre_service_type[result->channel])
				{
					RtkAc49xApiOnhookAction(result->channel);// let ch state back to CHANNEL_STATE__IDLE 
				}
				PRINT_MSG("CH%d send FSK CID OK.\n", result->channel);
			}
			else if (cidEvent == CALLER_ID_EVENT__GENERATION_ABORTED_DUE_TO_STOP_COMMAND)
			{
				fsk_cid_state[result->channel] = 0;
				RtkAc49xApiOnhookAction(result->channel);// let ch state back to CHANNEL_STATE__IDLE 
				printk(AC_FORE_RED "CH%d send FSK CID Abort.\n" AC_RESET, result->channel);
			}
			else if (cidEvent == CALLER_ID_EVENT__GENERATION_COMPLETED_OR_ABORTED_DUE_TO_ACKNOWLEDGE_TIMEOUT )
			{
				fsk_cid_state[result->channel] = 0;
				RtkAc49xApiOnhookAction(result->channel);// let ch state back to CHANNEL_STATE__IDLE 
				printk(AC_FORE_RED "CH%d send FSK CID completed or abort (ACK time out).\n" AC_RESET, result->channel);			
			}
			/* Caller Id Detection */
			else if (cidEvent == CALLER_ID_EVENT__RECEIVE_PROCESS_COMPLETED_CHECKSUM_CRC_IS_VALID)
			{
				/* Note: 
				 * If FSK CID detection mode is enable, when FXO receive DTMF CID, 
				 * ACMW will get IBS DTMF event, not DTMF CID event.  
				 */
				if (0 == (*pMessage))
				{
					result->dtmf_cid_valid = 1;
					result->fsk_cid_valid = 0;
					printk("DTMF Caller ID Detected(%d)!\n", result->channel);
				}
				else if (1 == (*pMessage))
				{
					result->dtmf_cid_valid = 0;
					result->fsk_cid_valid = 1;					
					printk("Type-%d FSK Caller ID Detected(%d)!\n", cid_type+1, result->channel);
					RtkAc49xApiFskCidParsing(pMessage, result);
					
				}
				
				result->cid_type = cid_type;
				result->pCidMsg = pMessage;
				
			
			}
			else if (cidEvent == CALLER_ID_EVENT__RECEIVE_PROCESS_COMPLETED_CHECKSUM_CRC_IS_ABSENT)
			{
				result->dtmf_cid_valid = 0;
				result->fsk_cid_valid = 0;
				PRINT_R("CALLER_ID_EVENT__RECEIVE_PROCESS_COMPLETED_CHECKSUM_CRC_IS_ABSENT\n");
			}
			else if (cidEvent == CALLER_ID_EVENT__RECEIVE_PROCESS_COMPLETED_CHECKSUM_CRC_IS_INVALID)
			{
				result->dtmf_cid_valid = 0;
				result->fsk_cid_valid = 0;
				PRINT_R("CALLER_ID_EVENT__RECEIVE_PROCESS_COMPLETED_CHECKSUM_CRC_IS_INVALID\n");
			}
			else if (cidEvent == CALLER_ID_EVENT__RECEIVE_PROCESS_ABORTED_DUE_TO_NO_SIGNAL_TIMEOUT)
			{
				result->dtmf_cid_valid = 0;
				result->fsk_cid_valid = 0;
				PRINT_R("CALLER_ID_EVENT__RECEIVE_PROCESS_ABORTED_DUE_TO_NO_SIGNAL_TIMEOUT\n");
			}
			else if (cidEvent == CALLER_ID_EVENT__RECEIVE_PROCESS_ABORTED_DUE_TO_DISCONNECTION)
			{
				result->dtmf_cid_valid = 0;
				result->fsk_cid_valid = 0;
				PRINT_R("CALLER_ID_EVENT__RECEIVE_PROCESS_ABORTED_DUE_TO_DISCONNECTION\n");
			}
			else if (cidEvent == CALLER_ID_EVENT__RECEIVE_PROCESS_ABORTED_DUE_TO_NO_SIGNAL_END_TIME_OUT)
			{
				result->dtmf_cid_valid = 0;
				result->fsk_cid_valid = 0;
				PRINT_R("CALLER_ID_EVENT__RECEIVE_PROCESS_ABORTED_DUE_TO_NO_SIGNAL_END_TIME_OUT\n");
			}
			else if (cidEvent == CALLER_ID_EVENT__NONE_EVENT)
			{
				result->dtmf_cid_valid = 0;
				result->fsk_cid_valid = 0;
				PRINT_R("CALLER_ID_EVENT__NONE_EVENT\n" AC_RESET);
			}
			else
			{
				result->dtmf_cid_valid = 0;
				result->fsk_cid_valid = 0;
				PRINT_R("Get Undefined Caller Id Event for CH%d\n", result->channel);
			}
		}
		
		if ((PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET == ac49xRxPacket.u.ProprietaryPayload.ProprietaryHeader.PacketCategory) &&
		    (STATUS_OR_EVENT_PACKET_OP_CODE__GENERAL_EVENT == ac49xRxPacket.u.ProprietaryPayload.ProprietaryHeader.Opcode))
		{
			//PRINT_MSG("Get CID event.\n");
			eventIdx = ac49xRxPacket.u.ProprietaryPayload.u.StatusOrEvent.u.GeneralEvent.EventIndex;
			
			extern char dtmf_cid_state[];
			
			if ((eventIdx == GENERAL_EVENT_PACKET_INDEX__IBS_STRING_GENERATION_ENDED) &&
			    (dtmf_cid_state[result->channel] == 1))
			{
				dtmf_cid_state[result->channel] = 0;
				/* Before sending CID, RtkAc49xApiOffhookAction() will be called to open acmw channel. 
				   Here, call RtkAc49xApiOnhookAction() to close acmw channel. */
				   
				RtkAc49xApiOnhookAction(result->channel);// let ch state back to CHANNEL_STATE__IDLE
				/* @@?
				  Run above onhook action here, when phone offhook after CID gen, user can hear a short tone. 
				  But fsk CID won't.
				 */
				PRINT_MSG("CH%d send DTMF CID OK.\n", result->channel);
			}
		}
		
		if ((PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET == ac49xRxPacket.u.ProprietaryPayload.ProprietaryHeader.PacketCategory) &&
		    (STATUS_OR_EVENT_PACKET_OP_CODE__PACKETIZER_STATUS_OR_EVENT == ac49xRxPacket.u.ProprietaryPayload.ProprietaryHeader.Opcode)&&
		    (PACKETIZER_STATUS_OR_EVENT_INDEX__REGULAR_CHANNEL_RECEIVE_RTCP_STATUS_OR_EVENT == ac49xRxPacket.u.ProprietaryPayload.u.StatusOrEvent.u.Packetizer.StatusOrEventIndex))
		{
			Tac49xRtcpReceiveStatus RtcpRxStatus;
			
			RtcpRxStatus = ac49xRxPacket.u.ProprietaryPayload.u.StatusOrEvent.u.Packetizer.u.RtcpReceiveStatus;
			
			if ( RTCP_STATUS_OR_EVENT_REPORT_TYPE__PACKETIZER_STATUS_PACKET_SEND_DUE_TO_GET_RTCP_COMMAND == RtcpRxStatus.ReportType)
			{

				nRxRtpStatsCountByte[result->channel] = 
					(unsigned int)(RtcpRxStatus.ReceiveOctetCount_MswMsb<<24) + 
					(unsigned int)(RtcpRxStatus.ReceiveOctetCount_MswLsb<<16) + 
					(unsigned int)(RtcpRxStatus.ReceiveOctetCount_LswMsb<<8) + 
					(unsigned int)RtcpRxStatus.ReceiveOctetCount_LswLsb;
					

				nRxRtpStatsCountPacket[result->channel] = 
					(unsigned int)(RtcpRxStatus.ReceivePacketCount_MswMsb<<24) + 
					(unsigned int)(RtcpRxStatus.ReceivePacketCount_MswLsb<<16) + 
					(unsigned int)(RtcpRxStatus.ReceivePacketCount_LswMsb<<8) + 
					(unsigned int)RtcpRxStatus.ReceivePacketCount_LswLsb;
				
				/*
				The CumlateLostPackets field is initialized upon receiving again RTP packets 
				after broken connection as you demonstrated. This field is updated when RTCP RR\SR packets 
				are generated, so I expect that if you poll till then you should get other result than zero.
				Please verify you don¡¦t disable the RTCP packets by selecting Mean Tx interval to zero (RtcpMeanTxInterval field). 
				In the Ac49xActivateRegularRtpChannelConfiguration() API.
				*/	

				nRxRtpStatsLostPacket[result->channel] =
					(unsigned int)(RtcpRxStatus.CumulateLostPackets_MswMsb<<24) + 
					(unsigned int)(RtcpRxStatus.CumulateLostPackets_MswLsb<<16) + 
					(unsigned int)(RtcpRxStatus.CumulateLostPackets_LswMsb<<8) + 
					(unsigned int)RtcpRxStatus.CumulateLostPackets_LswLsb;
					

				nTxRtpStatsCountByte[result->channel] =
					(unsigned int)(RtcpRxStatus.TransmitOctetCount_MswMsb<<24) + 
					(unsigned int)(RtcpRxStatus.TransmitOctetCount_MswLsb<<16) + 
					(unsigned int)(RtcpRxStatus.TransmitOctetCount_LswMsb<<8) + 
					(unsigned int)RtcpRxStatus.TransmitOctetCount_LswLsb;
					

				nTxRtpStatsCountPacket[result->channel] =
					(unsigned int)(RtcpRxStatus.TransmitPacketCount_MswMsb<<24) + 
					(unsigned int)(RtcpRxStatus.TransmitPacketCount_MswLsb<<16) + 
					(unsigned int)(RtcpRxStatus.TransmitPacketCount_LswMsb<<8) + 
					(unsigned int)RtcpRxStatus.TransmitPacketCount_LswLsb;
				
				gRtcpStatsUpdOk[result->channel] = 1;
	
				//PRINT_MSG("CH%d-(%d, %d, %d, %d, %d)\n", result->channel, nRxRtpStatsCountByte[result->channel], nRxRtpStatsCountPacket[result->channel],
				//	 nRxRtpStatsLostPacket[result->channel], nTxRtpStatsCountByte[result->channel], nTxRtpStatsCountPacket[result->channel]);
	
				if ((gCloseAcmwChAfterGetRtcpStatistic[result->channel] == 1) && (gAcmwChannelState[result->channel] == CH_OPEN_FOR_GET_RTCP))
				{
					RtkAc49xApiOnhookAction(result->channel); //After getting RTCP statistic, close ACMW channel.
					gCloseAcmwChAfterGetRtcpStatistic[result->channel] = 0;
				}
			
			}
			else if ( RTCP_STATUS_OR_EVENT_REPORT_TYPE__RR_PACKET == RtcpRxStatus.ReportType)
			{
				//PRINT_MSG("CH%d get RTCP_STATUS_OR_EVENT_REPORT_TYPE__RR_PACKET", result->channel);
			}
			else if ( RTCP_STATUS_OR_EVENT_REPORT_TYPE__SR_PACKET == RtcpRxStatus.ReportType)
			{
				//PRINT_MSG("CH%d get RTCP_STATUS_OR_EVENT_REPORT_TYPE__SR_PACKET", result->channel);
			}
				
		}
		
		if ((PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET == ac49xRxPacket.u.ProprietaryPayload.ProprietaryHeader.PacketCategory) &&
		    (STATUS_OR_EVENT_PACKET_OP_CODE__DEVICE_STATUS == ac49xRxPacket.u.ProprietaryPayload.ProprietaryHeader.Opcode))
		{
			/* Frame Energy of the Channel input PCM signal in -dBm */
			extern int energy_in(uint32 ch_id, char input);
			extern int eng_det_flag[];
	
			if (eng_det_flag[result->channel] == 1)
			{
				result->energy_dBm = ac49xRxPacket.u.ProprietaryPayload.u.StatusOrEvent.u.DeviceStatus.ChannelStatusPayload[0].FrameEnergy;
				energy_in(0, result->energy_dBm);
				//PRINT_MSG("CH0-%d  ", result->energy_dBm);
				result->energy_dBm = ac49xRxPacket.u.ProprietaryPayload.u.StatusOrEvent.u.DeviceStatus.ChannelStatusPayload[1].FrameEnergy;
				energy_in(1, result->energy_dBm);
				//PRINT_MSG("CH1-%d\n", result->energy_dBm);
			}
		}

		
	}/*if (pktsize > 0)*/
	else
	{
		result->dtmf_digit = 'Z';
		result->long_dtmf = 0;
		result->ced_flag = 0;
		result->cng_flag = 0;
		result->modem_flag = 0;
		result->dtmf_cid_valid = 0;
		result->fsk_cid_valid = 0;
	}
	
	//printk(AC_FORE_RED "%d, %d\n" AC_RESET, result->fsk_cid_valid, result->channel);
	
	/*** For FSK Caller ID Send with Auto Ring ***/

	//ring_struct ringing;
	unsigned char ring_set;

	for (ch=0; ch < MAX_VOIP_CH_NUM; ch++)
	{
		if ((fsk_cid_state[ch] == 1) && (fsk_cid_enable[ch] == 1))
		{
			if(!(fsk_spec_areas[ch]&0x08)) // after 1st ring
			{
				//ringing.CH = ch;
				if (FXS_Check_Ring(ch) == 0)	//ring off
				{
					printk("==> ch%d, %s, %s, %s\n", ch, fsk_cid, fsk_date_time, fsk_name);
					RtkAc49xApiSendFskCallerId(ch, fsk_ser_type[ch], fsk_msg_type[ch], fsk_cid, fsk_date_time, fsk_name);
					fsk_cid_enable[ch] = 0;
					//PRINT_MSG("\n <RTK> CID = %s\n", stCIDstr.string);
		
					/* Auto 2nd Ring */
					if (gRingGenAfterCid[ch] == 0)
					{
						gFirstRingOffTimeOut[ch] = jiffies + (HZ*gRingCadOff[ch]/1000);
						//printk("=>%d, J=%d\n", gFirstRingOffTimeOut[stCIDstr.ch_id], jiffies);
						gRingGenAfterCid[ch] = 1;
						//printk("1: gRingGenAfterCid[%d] = %d\n", stCIDstr.ch_id, gRingGenAfterCid[stCIDstr.ch_id]);
					}
				}
			}
			else	// before 1st ring
			{
					printk("==> ch%d, %s, %s, %s\n", ch, fsk_cid, fsk_date_time, fsk_name);
				RtkAc49xApiSendFskCallerId(ch, fsk_ser_type[ch], fsk_msg_type[ch], fsk_cid, fsk_date_time, fsk_name);
				fsk_cid_enable[ch] = 0;
				//PRINT_MSG("\n <RTK> CID = %s\n", stCIDstr.string);
	
				/* Auto 1st Ring */
				if (gRingGenAfterCid[ch] == 0)
				{
					gFirstRingOffTimeOut[ch] = jiffies + (HZ*1500/1000);
					//printk("=>%d, J=%d\n", gFirstRingOffTimeOut[stCIDstr.ch_id], jiffies);
					gRingGenAfterCid[ch] = 1;
					//printk("1: gRingGenAfterCid[%d] = %d\n", stCIDstr.ch_id, gRingGenAfterCid[stCIDstr.ch_id]);
				}
			}
		}
	}


}

int RtkAc49xApiSetCountryTone(int toneOfCountry )
{
	Tac49xSetupDeviceAttr *pSetupDeviceAttr = &(dsp_dev->SetupDeviceAttr);
	int i, j;

	//PRINT_MSG("Set Country Tone\n");
	
	for ( i=8; i<12; i++)// 8~11: dial, busy, ringback, call waiting tone
	{
		j = 5*toneOfCountry + i -8;
		pSetupDeviceAttr->CallProgress.Signal[i].Type 						= AcToneTable[j][0];
		pSetupDeviceAttr->CallProgress.Signal[i].ToneAFrequency 				= AcToneTable[j][1];
		pSetupDeviceAttr->CallProgress.Signal[i].ToneB_OrAmpModulationFrequency 		= AcToneTable[j][2];
		pSetupDeviceAttr->CallProgress.Signal[i].TwistThreshold 				= AcToneTable[j][3];
		pSetupDeviceAttr->CallProgress.Signal[i].ThirdToneOfTripleBatchDurationTypeFrequency 	= AcToneTable[j][4];
		pSetupDeviceAttr->CallProgress.Signal[i].HighEnergyThreshold 				= AcToneTable[j][5];
		pSetupDeviceAttr->CallProgress.Signal[i].LowEnergyThreshold 				= AcToneTable[j][6];
		pSetupDeviceAttr->CallProgress.Signal[i].SignalToNoiseRatioThreshold 			= AcToneTable[j][7];
		pSetupDeviceAttr->CallProgress.Signal[i].FrequencyDeviationThreshold 			= AcToneTable[j][8];
		pSetupDeviceAttr->CallProgress.Signal[i].ToneALevel 					= AcToneTable[j][9];
		pSetupDeviceAttr->CallProgress.Signal[i].ToneBLevel 					= AcToneTable[j][10];
		pSetupDeviceAttr->CallProgress.Signal[i].AmFactor 					= AcToneTable[j][11];
		pSetupDeviceAttr->CallProgress.Signal[i].DetectionTimeOrCadenceFirstOnOrBurstDuration 	= AcToneTable[j][12];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceFirstOffDuration 			= AcToneTable[j][13];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceSecondOnDuration 			= AcToneTable[j][14];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceSecondOffDuration 			= AcToneTable[j][15];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceThirdOnDuration 			= AcToneTable[j][16];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceThirdOffDuration 			= AcToneTable[j][17];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceFourthOnDuration 			= AcToneTable[j][18];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceFourthOffDuration 			= AcToneTable[j][19];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceVoiceAddedWhileFirstOff 		= AcToneTable[j][20];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceVoiceAddedWhileSecondOff 		= AcToneTable[j][21];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceVoiceAddedWhileThirdOff 		= AcToneTable[j][22];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceVoiceAddedWhileFourthOff 		= AcToneTable[j][23];
	}                                                                                                             

	for ( i=12; i<13; i++)// 12: Ring tone
	{
		j = 0;

		pSetupDeviceAttr->CallProgress.Signal[i].Type 						= AcToneTable2[j][0];
		pSetupDeviceAttr->CallProgress.Signal[i].ToneAFrequency 				= AcToneTable2[j][1];
		pSetupDeviceAttr->CallProgress.Signal[i].ToneB_OrAmpModulationFrequency 		= AcToneTable2[j][2];
		pSetupDeviceAttr->CallProgress.Signal[i].TwistThreshold 				= AcToneTable2[j][3];
		pSetupDeviceAttr->CallProgress.Signal[i].ThirdToneOfTripleBatchDurationTypeFrequency 	= AcToneTable2[j][4];
		pSetupDeviceAttr->CallProgress.Signal[i].HighEnergyThreshold 				= AcToneTable2[j][5];
		pSetupDeviceAttr->CallProgress.Signal[i].LowEnergyThreshold 				= AcToneTable2[j][6];
		pSetupDeviceAttr->CallProgress.Signal[i].SignalToNoiseRatioThreshold 			= AcToneTable2[j][7];
		pSetupDeviceAttr->CallProgress.Signal[i].FrequencyDeviationThreshold 			= AcToneTable2[j][8];
		pSetupDeviceAttr->CallProgress.Signal[i].ToneALevel 					= AcToneTable2[j][9];
		pSetupDeviceAttr->CallProgress.Signal[i].ToneBLevel 					= AcToneTable2[j][10];
		pSetupDeviceAttr->CallProgress.Signal[i].AmFactor 					= AcToneTable2[j][11];
		pSetupDeviceAttr->CallProgress.Signal[i].DetectionTimeOrCadenceFirstOnOrBurstDuration 	= AcToneTable2[j][12];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceFirstOffDuration 			= AcToneTable2[j][13];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceSecondOnDuration 			= AcToneTable2[j][14];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceSecondOffDuration 			= AcToneTable2[j][15];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceThirdOnDuration 			= AcToneTable2[j][16];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceThirdOffDuration 			= AcToneTable2[j][17];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceFourthOnDuration 			= AcToneTable2[j][18];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceFourthOffDuration 			= AcToneTable2[j][19];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceVoiceAddedWhileFirstOff 		= AcToneTable2[j][20];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceVoiceAddedWhileSecondOff 		= AcToneTable2[j][21];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceVoiceAddedWhileThirdOff 		= AcToneTable2[j][22];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceVoiceAddedWhileFourthOff 		= AcToneTable2[j][23];
	}
	
	for ( i=13; i<14; i++)// 13: Stutter Dial tone
	{
		j = 5*toneOfCountry + 4;		
		pSetupDeviceAttr->CallProgress.Signal[i].Type 						= AcToneTable[j][0];
		pSetupDeviceAttr->CallProgress.Signal[i].ToneAFrequency 				= AcToneTable[j][1];
		pSetupDeviceAttr->CallProgress.Signal[i].ToneB_OrAmpModulationFrequency 		= AcToneTable[j][2];
		pSetupDeviceAttr->CallProgress.Signal[i].TwistThreshold 				= AcToneTable[j][3];
		pSetupDeviceAttr->CallProgress.Signal[i].ThirdToneOfTripleBatchDurationTypeFrequency 	= AcToneTable[j][4];
		pSetupDeviceAttr->CallProgress.Signal[i].HighEnergyThreshold 				= AcToneTable[j][5];
		pSetupDeviceAttr->CallProgress.Signal[i].LowEnergyThreshold 				= AcToneTable[j][6];
		pSetupDeviceAttr->CallProgress.Signal[i].SignalToNoiseRatioThreshold 			= AcToneTable[j][7];
		pSetupDeviceAttr->CallProgress.Signal[i].FrequencyDeviationThreshold 			= AcToneTable[j][8];
		pSetupDeviceAttr->CallProgress.Signal[i].ToneALevel 					= AcToneTable[j][9];
		pSetupDeviceAttr->CallProgress.Signal[i].ToneBLevel 					= AcToneTable[j][10];
		pSetupDeviceAttr->CallProgress.Signal[i].AmFactor 					= AcToneTable[j][11];
		pSetupDeviceAttr->CallProgress.Signal[i].DetectionTimeOrCadenceFirstOnOrBurstDuration 	= AcToneTable[j][12];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceFirstOffDuration 			= AcToneTable[j][13];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceSecondOnDuration 			= AcToneTable[j][14];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceSecondOffDuration 			= AcToneTable[j][15];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceThirdOnDuration 			= AcToneTable[j][16];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceThirdOffDuration 			= AcToneTable[j][17];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceFourthOnDuration 			= AcToneTable[j][18];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceFourthOffDuration 			= AcToneTable[j][19];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceVoiceAddedWhileFirstOff 		= AcToneTable[j][20];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceVoiceAddedWhileSecondOff 		= AcToneTable[j][21];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceVoiceAddedWhileThirdOff 		= AcToneTable[j][22];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceVoiceAddedWhileFourthOff 		= AcToneTable[j][23];
	}
	
	RtkAc49xApiSetupDevice(); // To make tone setting effactive
	
	return 0;
}

int RtkAc49xApiSetCustomTone(TstVoipValue * custom_tone)
{
	Tac49xSetupDeviceAttr *pSetupDeviceAttr = &(dsp_dev->SetupDeviceAttr);
	int i, j;
	extern int CustomToneTable[8][24];
	
	for ( i=8; i<12; i++)// 8~11: dial, busy, ringback, call waiting tone
	{
		if (i == 8)
			j = custom_tone->value1;	//dial
		else if (i == 9)
			j = custom_tone->value3;	//busy
		else if (i == 10)
			j = custom_tone->value2;	//ringback
		else if (i == 11)
			j = custom_tone->value4;	//call waiting
			
		pSetupDeviceAttr->CallProgress.Signal[i].Type 						= CustomToneTable[j][0];
		pSetupDeviceAttr->CallProgress.Signal[i].ToneAFrequency 				= CustomToneTable[j][1];
		pSetupDeviceAttr->CallProgress.Signal[i].ToneB_OrAmpModulationFrequency 		= CustomToneTable[j][2];
		pSetupDeviceAttr->CallProgress.Signal[i].TwistThreshold 				= CustomToneTable[j][3];
		pSetupDeviceAttr->CallProgress.Signal[i].ThirdToneOfTripleBatchDurationTypeFrequency 	= CustomToneTable[j][4];
		pSetupDeviceAttr->CallProgress.Signal[i].HighEnergyThreshold 				= CustomToneTable[j][5];
		pSetupDeviceAttr->CallProgress.Signal[i].LowEnergyThreshold 				= CustomToneTable[j][6];
		pSetupDeviceAttr->CallProgress.Signal[i].SignalToNoiseRatioThreshold 			= CustomToneTable[j][7];
		pSetupDeviceAttr->CallProgress.Signal[i].FrequencyDeviationThreshold 			= CustomToneTable[j][8];
		pSetupDeviceAttr->CallProgress.Signal[i].ToneALevel 					= CustomToneTable[j][9];
		pSetupDeviceAttr->CallProgress.Signal[i].ToneBLevel 					= CustomToneTable[j][10];
		pSetupDeviceAttr->CallProgress.Signal[i].AmFactor 					= CustomToneTable[j][11];
		pSetupDeviceAttr->CallProgress.Signal[i].DetectionTimeOrCadenceFirstOnOrBurstDuration 	= CustomToneTable[j][12];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceFirstOffDuration 			= CustomToneTable[j][13];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceSecondOnDuration 			= CustomToneTable[j][14];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceSecondOffDuration 			= CustomToneTable[j][15];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceThirdOnDuration 			= CustomToneTable[j][16];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceThirdOffDuration 			= CustomToneTable[j][17];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceFourthOnDuration 			= CustomToneTable[j][18];
		pSetupDeviceAttr->CallProgress.Signal[i].CadenceFourthOffDuration 			= CustomToneTable[j][19];

		if ( i == 11)//call waiting tone
		{
			if (pSetupDeviceAttr->CallProgress.Signal[i].DetectionTimeOrCadenceFirstOnOrBurstDuration != 0)
				pSetupDeviceAttr->CallProgress.Signal[i].CadenceVoiceAddedWhileFirstOff = CONTROL__ENABLE;
			else
				pSetupDeviceAttr->CallProgress.Signal[i].CadenceVoiceAddedWhileFirstOff = CONTROL__DISABLE;
			
			if (pSetupDeviceAttr->CallProgress.Signal[i].CadenceSecondOnDuration != 0)
				pSetupDeviceAttr->CallProgress.Signal[i].CadenceVoiceAddedWhileSecondOff = CONTROL__ENABLE;
			else
				pSetupDeviceAttr->CallProgress.Signal[i].CadenceVoiceAddedWhileSecondOff = CONTROL__DISABLE;
			
			if ( pSetupDeviceAttr->CallProgress.Signal[i].CadenceThirdOnDuration != 0)	
				pSetupDeviceAttr->CallProgress.Signal[i].CadenceVoiceAddedWhileThirdOff = CONTROL__ENABLE;
			else
				pSetupDeviceAttr->CallProgress.Signal[i].CadenceVoiceAddedWhileThirdOff = CONTROL__DISABLE;
			
			if (pSetupDeviceAttr->CallProgress.Signal[i].CadenceFourthOnDuration != 0)
				pSetupDeviceAttr->CallProgress.Signal[i].CadenceVoiceAddedWhileFourthOff = CONTROL__ENABLE;
			else
				pSetupDeviceAttr->CallProgress.Signal[i].CadenceVoiceAddedWhileFourthOff = CONTROL__DISABLE;
		}

	}                                                                                                             

	RtkAc49xApiSetupDevice(); // To make tone setting effactive
	
	return 0;
}


int RtkAc49xApiPlayTone(int chid, int sid, int Tone, int ToneOnOff, int ToneDirection)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	Tac49xIbsStringAttr IbsAttr;
	int ret = 0;

	(&TxPacketParams)->Channel = chid;
	channel = (&TxPacketParams)->Channel;

	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
	{
		printk(AC_FORE_RED "channel %d is not supported: %s:%s:%d\n" AC_RESET, channel, __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	
	if ( ToneDirection == DSPCODEC_TONEDIRECTION_LOCAL )
		IbsAttr.GeneratorRedirection = IBS_STRING_GENERATOR_REDIRECTION__INTO_DECODER_OUTPUT;
	else if ( ToneDirection == DSPCODEC_TONEDIRECTION_REMOTE )
		IbsAttr.GeneratorRedirection = IBS_STRING_GENERATOR_REDIRECTION__INTO_ENCODER_INPUT;
		
	IbsAttr.NumberOfDigits = 1;
	IbsAttr.OnDuration = 0xfffffff; // ~20hrs
	IbsAttr.OffDuration = 0;
	IbsAttr.PauseDuration = 0;		
	IbsAttr.Signal[0].Pause = CONTROL__DISABLE;
	
	IbsAttr.Signal[0].SystemCode = IBS_SYSTEM_CODE__CALL_PROGRESS_0_TO_15;

	
	if ( Tone == DSPCODEC_TONE_HOLD )
	{
		IbsAttr.Signal[0].Digit = 7;
		PRINT_MSG("Hold");
	}
	else if ( Tone == DSPCODEC_TONE_DIAL )
	{
		IbsAttr.Signal[0].Digit = 8;
		PRINT_MSG("Dial");
	}
	else if ( Tone == DSPCODEC_TONE_BUSY )
	{
		IbsAttr.Signal[0].Digit = 9;
		PRINT_MSG("Busy");
	}
	else if ( Tone == DSPCODEC_TONE_RINGING )
	{
		IbsAttr.Signal[0].Digit = 10;
		PRINT_MSG("Ringback");
	}
	else if ( Tone == DSPCODEC_TONE_CALL_WAITING )
	{
		IbsAttr.Signal[0].Digit = 11;
		PRINT_MSG("Call waiting");
	}
	else if ( Tone == DSPCODEC_TONE_RING )
	{
		IbsAttr.Signal[0].Digit = 12;
		PRINT_MSG("Ringing");
	}
	else if ( Tone == DSPCODEC_TONE_STUTTERDIAL )
	{
		IbsAttr.Signal[0].Digit = 13;
		PRINT_MSG("Stutter Dial");
	}
	else if ( Tone >= DSPCODEC_TONE_0 && Tone <= DSPCODEC_TONE_HASHSIGN)
	{
		IbsAttr.Signal[0].SystemCode = IBS_SYSTEM_CODE__DTMF;
		
		if (ToneOnOff == TONE_ON)
		{
			ret = RtkAc49xApiGenDtmfTone(chid, Tone, 100, 0, ToneDirection);
		}
		return ret;
	}
	else if (Tone >= DSPCODEC_TONE_A && Tone <= DSPCODEC_TONE_D)
	{
		IbsAttr.Signal[0].SystemCode = IBS_SYSTEM_CODE__DTMF;
		
		if (ToneOnOff == TONE_ON)
		{
			ret = RtkAc49xApiGenDtmfTone(chid, Tone, 100, 0, ToneDirection);
		}
		
		return ret;
	}
	else if ( Tone >= DSPCODEC_TONE_0_CONT && Tone <= DSPCODEC_TONE_HASHSIGN_CONT)
	{
		IbsAttr.Signal[0].SystemCode = IBS_SYSTEM_CODE__DTMF;
		
		if (ToneOnOff == TONE_ON)
		{
			ret = RtkAc49xApiGenDtmfTone(chid, Tone-DSPCODEC_TONE_0_CONT, 100, 0, ToneDirection);
		}
		
		return ret;
	}
	else if ( Tone >= DSPCODEC_TONE_A_CONT && Tone <= DSPCODEC_TONE_D_CONT)
	{
		IbsAttr.Signal[0].SystemCode = IBS_SYSTEM_CODE__DTMF;
		
		if (ToneOnOff == TONE_ON)
		{
			ret = RtkAc49xApiGenDtmfTone(chid, DSPCODEC_TONE_A+(Tone-DSPCODEC_TONE_A_CONT), 100, 0, ToneDirection);
		}
		
		return ret;
	}
	else
	{
		PRINT_MSG("Tone %d", Tone);
	}

	if (ToneOnOff == TONE_ON)
	{
		Ac49xSendIbsStop(device, channel);
		ret = Ac49xSendIbsString(device, channel, &IbsAttr);
		PRINT_MSG(" on(%d)\n", channel);
		if (ret != 0)
			printk(AC_FORE_RED "Play Tone Fail.\n" AC_RESET);
	}
	else if (ToneOnOff == TONE_OFF)
	{
		ret = Ac49xSendIbsStop(device, channel);
		PRINT_MSG(" off(%d)\n", channel);
		if (ret != 0)
			printk(AC_FORE_RED "Stop to Play Tone Fail.\n" AC_RESET);
	}
		
	return ret;
}

int RtkAc49xApiGenDtmfTone(int chid, int dmtf_digit, int on_duration, int off_duration, int direction)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	Tac49xIbsStringAttr IbsAttr;
	int ret = 0;

	(&TxPacketParams)->Channel = chid;
	channel = (&TxPacketParams)->Channel;

	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
	{
		printk(AC_FORE_RED "channel %d is not supported: %s:%s:%d\n" AC_RESET, channel, __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	
	if ( direction == DSPCODEC_TONEDIRECTION_LOCAL )
		IbsAttr.GeneratorRedirection = IBS_STRING_GENERATOR_REDIRECTION__INTO_DECODER_OUTPUT;
	else if ( direction == DSPCODEC_TONEDIRECTION_REMOTE )
		IbsAttr.GeneratorRedirection = IBS_STRING_GENERATOR_REDIRECTION__INTO_ENCODER_INPUT;
	
	IbsAttr.NumberOfDigits = 1;
	IbsAttr.OnDuration = on_duration; // unit: ms
	IbsAttr.OffDuration = off_duration;
	IbsAttr.PauseDuration = 0;		
	IbsAttr.Signal[0].Pause = CONTROL__DISABLE;
	IbsAttr.Signal[0].SystemCode = IBS_SYSTEM_CODE__DTMF;
	
	if (dmtf_digit > DSPCODEC_TONE_HASHSIGN)
	{
		IbsAttr.Signal[0].Digit = 12 + dmtf_digit - DSPCODEC_TONE_A;
		PRINT_MSG("Gen DTMF Tone %s (%d ms)\n", "A"+dmtf_digit-DSPCODEC_TONE_A, on_duration);
	}
	else
	{
		IbsAttr.Signal[0].Digit = dmtf_digit;
		PRINT_MSG("Gen DTMF Tone %d (%d ms)\n", dmtf_digit, on_duration);
	}
	
	Ac49xSendIbsStop(device, channel);
	ret = Ac49xSendIbsString(device, channel, &IbsAttr);
	if (ret != 0)
		printk(AC_FORE_RED "Play DTMF Tone Fail.\n" AC_RESET);

		
	return ret;
}

int RtkAc49xApiSendOutbandDtmfEvent(int chid, int dmtf_digit, int duration) //for IP Phone or AP used.
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	Tac49xIbsStringAttr IbsAttr;
	int ret = 0;

	(&TxPacketParams)->Channel = chid;
	channel = (&TxPacketParams)->Channel;

	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
	{
		printk(AC_FORE_RED "channel %d is not supported: %s:%s:%d\n" AC_RESET, channel, __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	
	IbsAttr.GeneratorRedirection = IBS_STRING_GENERATOR_REDIRECTION__INTO_NETWORK;
	
	IbsAttr.NumberOfDigits = 1;
	IbsAttr.OnDuration = duration; // unit: ms
	IbsAttr.OffDuration = 0;//off_duration;
	IbsAttr.PauseDuration = 0;		
	IbsAttr.Signal[0].Pause = CONTROL__DISABLE;
	IbsAttr.Signal[0].SystemCode = IBS_SYSTEM_CODE__DTMF;
	
	if (dmtf_digit > DSPCODEC_TONE_HASHSIGN)
	{
		IbsAttr.Signal[0].Digit = 12 + dmtf_digit - DSPCODEC_TONE_A;
		PRINT_MSG("Send Outband DTMF Event %s (%d ms)\n", "A"+dmtf_digit-DSPCODEC_TONE_A, duration);
	}
	else
{
		IbsAttr.Signal[0].Digit = dmtf_digit;
		PRINT_MSG("Send Outband DTMF Event %d (%d ms)\n", dmtf_digit, duration);
	}
	
	Ac49xSendIbsStop(device, channel);
	ret = Ac49xSendIbsString(device, channel, &IbsAttr);
	if (ret != 0)
		printk(AC_FORE_RED "Send Outband DTMF event Fail.\n" AC_RESET);

		
	return ret;
}

static int RtkAc49xApiGenNttAlertTone(int chid)
{
	int ret = 0;
	
	ret = RtkAc49xApiGenDtmfTone(chid, DSPCODEC_TONE_C, 80, 0, IBS_STRING_GENERATOR_REDIRECTION__INTO_DECODER_OUTPUT); 	// play C tone for 80 ms
	mdelay(50);							// off 50 ms
	ret += RtkAc49xApiGenDtmfTone(chid, DSPCODEC_TONE_D, 90, 0, IBS_STRING_GENERATOR_REDIRECTION__INTO_DECODER_OUTPUT);	// play D tone for 90 ms
	mdelay(200);							// off 200ms

	if (ret != 0)
		printk(AC_FORE_RED "Gen NTT Alert Tone Fail.\n" AC_RESET);
		
	return ret;
}

int RtkAc49xApiSetEchoCanceller(int chid, Tac49xEchoCancelerMode ec_mode)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;
	
	/*Enumeration values: 
	ECHO_CANCELER_MODE__DISABLE   
	ECHO_CANCELER_MODE__ENABLE   
	ECHO_CANCELER_MODE__FREESE   
	ECHO_CANCELER_MODE__CLEAR 
	*/

	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.EchoCancelerMode = ec_mode;
	Ac49xUpdateChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel));

	return 0;
}

int RtkAc49xApiSetEchoCancellerNLP(int chid, Tac49xEchoCancelerNonLinearProcessor nlp_mode)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;

	/*Enumeration values: 
	ECHO_CANCELER_NON_LINEAR_PROCESSOR_MODE__DISABLE   
	ECHO_CANCELER_NON_LINEAR_PROCESSOR_MODE__LOW_SENSITIVITY   
	ECHO_CANCELER_NON_LINEAR_PROCESSOR_MODE__MEDIUM_SENSITIVITY   
	ECHO_CANCELER_NON_LINEAR_PROCESSOR_MODE__HIGH_SENSITIVITY  
	*/
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.EchoCancelerNonLinearProcessor = nlp_mode;
	Ac49xUpdateChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel));

	return 0;
}

int RtkAc49xApiActiveEchoCancellerNLP(int chid)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;

	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.EchoCancelerMode = ECHO_CANCELER_MODE__ENABLE;
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.EchoCancelerNonLinearProcessor = ECHO_CANCELER_NON_LINEAR_PROCESSOR_MODE__MEDIUM_SENSITIVITY;

	Ac49xUpdateChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel));

	return 0;
}

int RtkAc49xApiUpdateEchoCancellerLength(int chid, Tac49xEchoCancelerLength ec_length)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;
	
	if ( (ec_length < ECHO_CANCELER_LENGTH__4_MSEC ) && (ec_length > ECHO_CANCELER_LENGTH__64_MSEC ))
	{
		printk(AC_FORE_RED "The Selected Echo Canceller Length is NOT Supported.\n" AC_RESET);
		
		return -1;
	}
	
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.EchoCancelerLength = ec_length;
	Ac49xUpdateChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel));
	
	return 0;

}

/* 
 * The global variable "support_lec_g168[]" is a flag defined by Realtek for LEC on/off used. 
 * The API RtkAc49xApiActiveEchoCancellerNLP() is for LEC setting inside ACMW.
 */
unsigned char support_lec_g168[MAX_VOIP_CH_NUM] = {[0 ... MAX_VOIP_CH_NUM-1]=1};// 0: LEC disable  1: LEC enable

int RtkAc49xApi_LEC_enable(int chid)
{
        if (chid > MAX_VOIP_CH_NUM)
	{
		printk("Enable LEC Fail, Wrong chid=%d\n", chid);
		return -1;
	}
		
	support_lec_g168[chid] = 1;
	
	return 0;
}

int RtkAc49xApi_LEC_disable(int chid)
{
        if (chid > MAX_VOIP_CH_NUM)
	{
		printk("Enable LEC Fail, Wrong chid=%d\n", chid);
		return -1;
	}
		
	support_lec_g168[chid] = 0;
	
	return 0;
}

int RtkAc49xApiSetVoiceGain(int chid, Tac49xVoiceGain in_gain, Tac49xVoiceGain out_gain)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;
	
	/*
	 * Note: Voice output gain is effective only when RTP is active(i.e. during talking time).
	 *       Input gain has diff location. It may before echo canceller or may after echo canceller and before encoder.
	 *	 See RtkAc49xApiSetInputGainLocation() API for detail.
	 */
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.InputGain = in_gain;	
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.VoiceOutputGain = out_gain;
	Ac49xUpdateChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel));
	
	return 0;
}

int RtkAc49xApiSetFaxAndCallerIdLevel(int chid, Tac49xFaxAndCallerIdLevel level)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;
	
	/*
	 * Fax and Caller ID output level. Default = FAX_AND_CALLER_ID_LEVEL__minus_12_DBM
	 * Range: -6 ~ -21 dBm
	 */
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.FaxAndCallerIdLevel = level;	
	Ac49xUpdateChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel));
	
	return 0;
}

int RtkAc49xApiSetCallerIdDetection(int chid, Tac49xControl on_hook_st, Tac49xControl off_hook_st, Tac49xCallerIdStandard standard)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;
	
	/*
	 * If CallerIdOnHookService or CallerIdOffHookService are enabled,
	 * the cid rxmodem will be enabled and will wait for preamble according to the standard and type.
	 * (i.e. seizure signal, dt-as, dtmf a,b ¡K).
	 */
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.CallerIdStandard = standard;	
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.CallerIdOnHookService = on_hook_st;
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.CallerIdOffHookService= off_hook_st;
	Ac49xUpdateChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel));
	
	return 0;
}

int RtkAc49xApiSetInputGainLocation(int chid, Tac49xInputGainLocation in_gain_location)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;

	/* enum Tac49xInputGainLocation{
	 * INPUT_GAIN_LOCATION__BEFORE_ECHO_CANCELER   
	 * INPUT_GAIN_LOCATION__AFTER_ECHO_CANCELER_AND_BOFORE_ENCODER  
	 * }
	 */
	dsp_dev->SetupChannelAttr[channel].Advanced.InputGainLocation = in_gain_location;
	Ac49xAdvancedChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].Advanced));

	return 0;
}

int RtkAc49xApiSetVoiceJBDelay(int chid, int max_delay, int min_delay, Tac49xDynamicJitterBufferOptimizationFactor jb_factor)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;
	
	if ( min_delay > max_delay )
	{
		PRINT_MSG("Error! JB Min.dealy > Max.dealy\n");
		return -1;
	}	
	
	if (max_delay > 320)
	{
		max_delay = 320;
		PRINT_MSG("JB Max.dealy > 320 msec, set to 320 msec\n");
	}
	else if (max_delay == 0)
	{
		max_delay = 150;
		PRINT_MSG("JB Max.dealy = 0, set to 150 msec\n");
	}
	else
		PRINT_MSG("Set JB Max.dealy = %d\n\n", max_delay);

	if (min_delay > 280)
	{
		min_delay = 280;
		PRINT_MSG("JB Max.dealy > 280 msec, set to 280 msec\n");
	}
	else if (min_delay == 0)
	{
		min_delay = 35;
		PRINT_MSG("JB Max.dealy = 0, set to 35 msec\n");
	}
	else
		PRINT_MSG("Set JB Min.dealy = %d\n\n", min_delay);
		
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.VoiceJitterBufferMaxDelay = max_delay;	
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.VoiceJitterBufferMinDelay = min_delay;
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.DynamicJitterBufferOptimizationFactor = jb_factor;
	Ac49xUpdateChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel));

	return 0;
	
	/* ------ Note for Jitter Buffer Parameters ------
	Min delay ¡V The jitter buffer delay is initially set to this delay. The jitter buffer delay will never go bellow this delay.
	Max Delay ¡V The maximal Jitter buffer delay. The dynamic jitter buffer delay will never go above this delay.
        Optimization Factor ¡V the jitter buffer delay is adjusted to the network jitter in the following way: 
        			when there are underrun (the jitter buffer is empty and the decoder does not have any packet to decode), 
        			the jitter buffer delay increases. When a substantial time passes and the network jitter is within the jitter buffer delay, 
        			the jitter buffer delay decreases. The optimization factor parameter defines the increase rate and the decrease rate.
        Dynamic jitter buffer optimized for voice quality = 12, Dynamic jitter buffer optimized for delay = 1, value of 7 is recommended for typical network conditions. 
        Optimization Factor = 13 this mode is suitable for fax and modem transmission. In this mode the jitter buffer delay is fixed and 
        			there are no drift corrections until there are underruns or overruns.
	*/
}

int RtkAc49xApiAgcDeviceConfig(int min_gain, int max_gain)
{
	unsigned int device = (&TxPacketParams)->Device;
	Tac49xAgcDeviceConfigurationAttr Agc_Attr;
	
	Agc_Attr.MinGain = min_gain;
        Agc_Attr.MaxGain = max_gain;
        PRINT_MSG("Set this device AGC min Gain=-%d dB, max Gain=%d dB\n", min_gain, max_gain);
        
        /* Set Default Value*/
        Agc_Attr.FastAdaptationGainSlope = AGC_GAIN_SLOPE__10_00_DB_SEC;
        Agc_Attr.FastAdaptationDuration = 1500;
        Agc_Attr.SilenceDuration = 120;
        Agc_Attr.DetectionDuration = 15;
        Agc_Attr.SignalingOverhang = 500;

	Ac49xAgcDeviceConfiguration(device, &Agc_Attr);
	
	return 0;
}

int RtkAc49xApiAgcConfig(int chid, Tac49xControl agc_enable, Tac49xAgcLocation agc_location)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;
	
	/*
	location:
	AGC_LOCATION__AT_ENCODER_INPUT   
	AGC_LOCATION__AT_DECODER_OUTPUT  
	*/
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.AgcEnable = agc_enable;	
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.AgcLocation = agc_location;
	Ac49xUpdateChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel));
	
	PRINT_MSG("CH%d AGC enable=%d ", channel, agc_enable);
	if (agc_location == 1)
		PRINT_MSG("location: SPK.\n");
	else if (agc_location == 0)
		PRINT_MSG("location: MIC.\n");
	
	return 0;
}

int RtkAc49xApiAgcEnergySlope(int chid, Tac49xAgcTargetEnergy agc_target_energy, Tac49xAgcGainSlope agc_slope)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;
	
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.AgcTargetEnergy = agc_target_energy;
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.AgcSlowAdaptationGainSlope = agc_slope;
	
	Ac49xUpdateChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel));
	
	PRINT_MSG("Set the CH%d AGC Target Energy= -%d dBm\n", channel, agc_target_energy);
	
	return 0;
}

int RtkAc49xApiSetIbsTransferMode(int chid, Tac49xIbsTransferMode ibs_mode)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;
	
	/*
	DTMF is detected on the PCM on the TDM interface, then according to DTMF transfer mode in the open channel AP, DtmfTransferMode attribute:
	typedef enum
	{
    		IBS_TRANSFER_MODE__RELAY_DISABLE_VOICE_MUTE,
    		IBS_TRANSFER_MODE__RELAY_ENABLE_VOICE_DISABLE,
    		IBS_TRANSFER_MODE__TRANSPARENT_THROUGH_VOICE,
    		IBS_TRANSFER_MODE__RELAY_ENABLE_VOICE_MUTE
    	} Tac49xIbsTransferMode;
    	If relay option is selected than RFC2833 is generated automatically.
	*/
	
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.DtmfTransferMode  = ibs_mode;
	//dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.CasRelay = CONTROL__ENABLE;
	
	Ac49xUpdateChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel));
	
	if (ibs_mode == IBS_TRANSFER_MODE__TRANSPARENT_THROUGH_VOICE)
		PRINT_MSG("Set CH%d Transfer Mode to IBS_TRANSFER_MODE__TRANSPARENT_THROUGH_VOICE\n", channel);
	else if (ibs_mode == IBS_TRANSFER_MODE__RELAY_ENABLE_VOICE_MUTE)
		PRINT_MSG("Set CH%d Transfer Mode to IBS_TRANSFER_MODE__RELAY_ENABLE_VOICE_MUTE\n", channel);
	else if (ibs_mode == IBS_TRANSFER_MODE__RELAY_ENABLE_VOICE_DISABLE)
		PRINT_MSG("Set CH%d Transfer Mode to IBS_TRANSFER_MODE__RELAY_ENABLE_VOICE_DISABLE\n", channel);
	else if (ibs_mode == IBS_TRANSFER_MODE__RELAY_DISABLE_VOICE_MUTE)
		PRINT_MSG("Set CH%d Transfer Mode to IBS_TRANSFER_MODE__RELAY_DISABLE_VOICE_MUTE\n", channel);

	
	return 0;
}


int RtkAc49xApiSetFaxTransferMethod(int chid, Tac49xFaxModemTransferMethod  fax_modem_trans_mode)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;
	
	/*
	FAX_MODEM_TRANSFER_METHOD__DISABLE  = 0 (default)
	FAX_MODEM_TRANSFER_METHOD__RELAY    = 1
	FAX_MODEM_TRANSFER_METHOD__BYPASS   = 2
	*/
	
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.FaxTransferMethod = fax_modem_trans_mode;
	
	Ac49xUpdateChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel));
	
	return 0;
}


Tac49xFaxModemTransferMethod RtkAc49xApiGetFaxTransferMethod(int chid)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;
	
	/*
	FAX_MODEM_TRANSFER_METHOD__DISABLE  = 0 (default)
	FAX_MODEM_TRANSFER_METHOD__RELAY    = 1
	FAX_MODEM_TRANSFER_METHOD__BYPASS   = 2
	*/
	return dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.FaxTransferMethod;
}

int RtkAc49xApiSetCedTransferMode(int chid, Tac49xCedTransferMode ced_trans_mode)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;
	
	/*
	CED_TRANSFER_MODE__BY_FAX_RELAY    (default)
	CED_TRANSFER_MODE__IN_VOICE_OR_PCM_BYPASS_MODE_TRIGGER_FAX_BY_V21_PREAMBLE  
	*/
	
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.CedTransferMode = ced_trans_mode;
	
	Ac49xUpdateChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel));
	
	return 0;
}

int RtkAc49xApiSetCngRelay(int chid, Tac49xControl enable)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;
	
	/*
	CONTROL__DISABLE  	
	CONTROL__ENABLE     
	*/
	
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.CngRelayEnable  = enable;
	
	Ac49xUpdateChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel));
	
	return 0;
}

int RtkAc49xApiSetIBSDetectionRedirection(int chid, Tac49xIbsDetectionRedirection ibs_redirection)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;

	/*
	ibs_redirection:
	IBS_DETECTION_REDIRECTION__ON_INPUT_STREAM_ENCODER_INPUT  (Default)
	IBS_DETECTION_REDIRECTION__ON_OUTPUT_STREAM_DECODER_OUTPUT
	Note: if IBS_DETECTION_REDIRECTION__ON_OUTPUT_STREAM_DECODER_OUTPUT is set, it can not detect DTMF tone from phone set.
	*/
		
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.IbsDetectionRedirection = ibs_redirection;
	
	Ac49xUpdateChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel));
	
	return 0;
}

int RtkAc49xApiSetCallProgressIBSDetection(int chid, Tac49xControl enable)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;

	/*
	enable:
	CONTROL__DISABLE = 0
	CONTROL__ENABLE = 1
	*/
		
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.DetectCallProgress = enable;
	
	Ac49xUpdateChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel));
	
	PRINT_MSG("CH%d CP-IBS Det enable=%d\n", chid, enable);
	
	return 0;
}

int RtkAc49xApiSetDtmfDetection(int chid, Tac49xControl enable)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;

	/*
	CONTROL__DISABLE = 0
	CONTROL__ENABLE = 1
	*/
		
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.DetectDTMFRegister = enable;
	
	Ac49xUpdateChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel));
	
	PRINT_MSG("CH%d DTME Det enable=%d\n", chid, enable);
	
	return 0;
}

int RtkAc49xApiSetLongDtmfDetection(int chid, Tac49xControl enable)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;

	/*
	CONTROL__DISABLE = 0
	CONTROL__ENABLE = 1
	*/
		
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.LongDtmfDetectionEnable = enable;
	
	Ac49xUpdateChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel));
	
	PRINT_MSG("CH%d Long DTME Det enable=%d\n", chid, enable);
	
	return 0;
}

int RtkAc49xApiSetDtmfErasureMode(int chid, Tac49xDtmfErasureMode dtmf_erasure_mode)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;
	
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.DtmfErasureMode = dtmf_erasure_mode;
	
	Ac49xUpdateChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel));
	
	return 0;
}


int RtkAc49xApiDetectNetIbsPackets(int chid, Tac49xControl enable)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;
	
	/* Enables the detection of network IBS packets (RFC 2833). */
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.DetectNetIbsPackets  = enable;
	
	Ac49xUpdateChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel));
	
	return 0;
}

int RtkAc49xApiSetIbsLevel(int chid, int ibs_level_minus_dbm)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;
	
	/*DTMF and MF gain (in steps of -dBm) during Relay or IBS string generation. Default = -9 dBm.*/
	dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.IbsLevel_minus_dbm = ibs_level_minus_dbm;
	
	Ac49xUpdateChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel));
	
	return 0;
}


Tac49xDriverVersionInfo RtkAc49xApiGetDriverVersionInfo(void)
{
	Tac49xDriverVersionInfo DriverVersionInfo;
	Tac49xDriverVersionInfo *pDriverVersionInfo = &DriverVersionInfo;
	
	Ac49xGetDriverVersionInfo(pDriverVersionInfo);
	
	return DriverVersionInfo;
}


int RtkAc49xApiSetFskCallerIdParam(int chid, unsigned char service_type, unsigned char msg_type, unsigned char *pCallerID, unsigned char *pDate_time, unsigned char *pName)
{
	fsk_ser_type[chid] = service_type;
	fsk_msg_type[chid] = msg_type;
	strcpy(fsk_cid, pCallerID);
	strcpy(fsk_date_time, pDate_time);
	strcpy(fsk_name, pName);

	return 0;
}

int RtkAc49xApiSendFskCallerId(int chid, unsigned char service_type, unsigned char msg_type, unsigned char *pCallerID, unsigned char *pDate_time, unsigned char *pName)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int ret = 0, channel, cid_len, date_time_len, name_len, i;
	unsigned char msg_len;
	unsigned long flags;
	Tac49xCallerIdAttr		callerIdAttr;
	Tac49xCallerIdAsciMessage 	callerIdAsciMessage;
	Tac49xCallerIdAttr 		*pCallerIdAttr;
	Tac49xCallerIdAsciMessage 	*pCallerIdAsciMessage;
	//ring_struct ringing;
	unsigned char ring_set;
	const uint32 cch = chid;
	
	channel = chid;
	pCallerIdAttr = &callerIdAttr;
	pCallerIdAsciMessage = &callerIdAsciMessage;
	
	extern char fsk_spec_areas[];

	// bit0-2: FSK Type - 0:Bellcore 1:ETSI 2:BT 3:NTT 
	// bit 3: Caller ID Prior First Ring 
	// bit 4: Dual Tone before Caller ID (Fsk Alert Tone)
	// bit 5: Short Ring before Caller ID  
	// bit 6: Reverse Polarity before Caller ID (Line Reverse)
	// bit 7: FSK Date & Time Sync and Display Name
        unsigned char standard_flag[MAX_VOIP_CH_NUM];
        unsigned char dual_tone_flag[MAX_VOIP_CH_NUM];
        unsigned char date_time_name_flag[MAX_VOIP_CH_NUM];
        unsigned char reverse_polar_flag[MAX_VOIP_CH_NUM];
        unsigned char short_ring_flag[MAX_VOIP_CH_NUM];
        unsigned char prior_ring_flag[MAX_VOIP_CH_NUM];
	
	save_flags(flags); cli();
	
	standard_flag[chid] = fsk_spec_areas[chid]&0x07;
	prior_ring_flag[chid] = (fsk_spec_areas[chid]&0x08)>>3;
	date_time_name_flag[chid] = (fsk_spec_areas[chid]&0x80)>>7;
	reverse_polar_flag[chid] = (fsk_spec_areas[chid]&0x40)>>6;
	short_ring_flag[chid] = (fsk_spec_areas[chid]&0x20)>>5;
	dual_tone_flag[chid] = (fsk_spec_areas[chid]&0x10)>>4;
	//PRINT_MSG("%d, %d, %d, %d\n", standard_flag[chid], dual_tone_flag[chid], date_time_name_flag[chid], reverse_polar_flag[chid]);
		
	// service_type: 0-OnHook, 1-OffHook
	// CALLER_ID_SERVICE_TYPE__1_ON_HOOK   
	// CALLER_ID_SERVICE_TYPE__2_OFF_HOOK 
	
	// msg_type: CALL_SET_UP, CND, SDMF_VMWI, MDMF_VMWI, NTT, NTT_II

	// fsk_spec_areas[chid]&0x10:
	// ETSI_ON_HOOK_METHOD__NO_PRECEDING_DUAL_TONE_ALERT_SIGNAL   
	// ETSI_ON_HOOK_METHOD__WITH_PRECEDING_DUAL_TONE_ALERT_SIGNAL  
	   
	pCallerIdAttr->ChecksumByteAbsent = CONTROL__DISABLE;
	pCallerIdAttr->Error = CONTROL__DISABLE;
	pCallerIdAttr->Event = CALLER_ID_EVENT__NONE_EVENT;	
	pCallerIdAsciMessage->Title.Type.Value = (char)CALLER_ID_SEGMENTATION_MESSAGE_TYPE__NONE_DTMF;

	if (standard_flag[chid] == 0)
	{
		pCallerIdAttr->Standard = CALLER_ID_STANDARD__TELCORDIA_BELLCORE;
		PRINT_MSG("Bellcore..");
	}
	else if ((standard_flag[chid] == 1) || (standard_flag[chid] == 2))
	{
		pCallerIdAttr->Standard = CALLER_ID_STANDARD__ETSI;
		PRINT_MSG("ETSI(BT)..");
	}
	else if (standard_flag[chid] == 3)
	{
		pCallerIdAttr->Standard = CALLER_ID_STANDARD__NTT;
		PRINT_MSG("NTT..");
	}
	pCallerIdAttr->ServiceType = service_type;
	pre_service_type[chid] = service_type;
	PRINT_MSG("Type %d\n", service_type+1);
	pCallerIdAttr->EtsiOnhookMethod = dual_tone_flag[chid];
	
	cid_len = strlen(pCallerID);
	date_time_len = strlen(pDate_time);
	name_len = strlen(pName);
	//PRINT_MSG("(%d, %d, %d)\n", cid_len, date_time_len, name_len);
	//PRINT_MSG("(%s, %s, %s)\n", pCallerID, pDate_time, pName);
	
	if (date_time_name_flag[chid] == 1)
	{
		pCallerIdAsciMessage->Title.Length.Value = (char)(2+(2+date_time_len)+(2+cid_len)+(2+name_len	));
	}
	else
	{
		pCallerIdAsciMessage->Title.Length.Value = (char)(2+(2+cid_len));	
	}
	msg_len = pCallerIdAsciMessage->Title.Length.Value - 2; 
	//PRINT_MSG("(%d, %d)\n", pCallerIdAsciMessage->Title.Length.Value, msg_len);
	
	if(standard_flag[chid] == 3)//NTT Caller ID
	{
		if (service_type == 0)	// type 1
			msg_type = NTT_CLIP; // 0x40
		else if (service_type == 1) // type 2
			msg_type = NTT_CIDCW;  // 0x41
	}
	pCallerIdAsciMessage->Data[0].Value = msg_type;
	pCallerIdAsciMessage->Data[1].Value = msg_len; //Message Length
	
	// set date, time, and name parameter
	if (date_time_name_flag[chid] == 1)
	{
		pCallerIdAsciMessage->Data[2].Value = DATE_AND_TIME;
		pCallerIdAsciMessage->Data[3].Value = date_time_len;
		memcpy(&pCallerIdAsciMessage->Data[4], pDate_time, date_time_len);
				
		pCallerIdAsciMessage->Data[2+2+date_time_len+2+cid_len+1-1].Value = NAME;
		pCallerIdAsciMessage->Data[2+2+date_time_len+2+cid_len+2-1].Value = name_len;
		memcpy(&pCallerIdAsciMessage->Data[2+2+date_time_len+2+cid_len+2], pName, name_len);
	}
	
	// set caller id parameter
	if (date_time_name_flag[chid] == 1)
	{
		if (strcmp(pCallerID, "-") == 0)
		{
			pCallerIdAsciMessage->Data[2+2+date_time_len+1-1].Value = NUM_ABS;
			cid_len = 1;
			pCallerID = "P";
		}
		else
		{
			pCallerIdAsciMessage->Data[2+2+date_time_len+1-1].Value = NUMBER;			
		}
		
		pCallerIdAsciMessage->Data[2+2+date_time_len+2-1].Value = cid_len;
		memcpy(&pCallerIdAsciMessage->Data[2+2+date_time_len+2], pCallerID, cid_len);
	}
	else
	{
		if (strcmp(pCallerID, "-") == 0)
		{
			pCallerIdAsciMessage->Data[2].Value = NUM_ABS;
			cid_len = 1;
			pCallerID = "P";
		}
		else
		{
			pCallerIdAsciMessage->Data[2].Value = NUMBER;
		}
		pCallerIdAsciMessage->Data[3].Value = cid_len;
		memcpy(&pCallerIdAsciMessage->Data[4], pCallerID, cid_len);
	}
	
	if(standard_flag[chid] == 3)//NTT Caller ID
	{
		if(service_type == 0) // on-hook
		{
			OnHookLineReversal(cch, 1);
			SendNTTCAR(cch);//gavin
		}
	}
	else
	{
		if (service_type == 0) // on-hook
		{
			// Reverse Polarity before Caller ID (Line Reverse)
			if(reverse_polar_flag[chid] == 1)
			{
				OnHookLineReversal(cch, 1);
				mdelay(150);
			}
			else
			{
				OnHookLineReversal(cch, 0);
			}
		
			// Short Ring (250ms) before Caller ID
			if(short_ring_flag[chid] == 1) 
			{	
				//ringing.CH = chid;
				//ringing.ring_set = 1;
				//FXS_Ring(&ringing);
				FXS_Ring( cch, 1 );
				mdelay(250);
				
				if(reverse_polar_flag[chid] == 1)
				{
					OnHookLineReversal(cch, 1);
				}
				else
				{
					OnHookLineReversal(cch, 0);
				}
			}
			
			// Caller ID Prior First Ring
			if(prior_ring_flag[chid] == 0)
			{
				//ringing.CH = chid;
				//ringing.ring_set = 1;
				//FXS_Ring(&ringing);
				FXS_Ring( cch, 1 );
				mdelay(10);
		
				//ringing.CH = chid;
				while(FXS_Check_Ring(chid/*&ringing*/))
					mdelay(10);

				if(reverse_polar_flag[chid] == 1)
				{
					OnHookLineReversal(cch, 1);
				}
				else
				{
					OnHookLineReversal(cch, 0);
				}
			}
		}
	
	}
	
	restore_flags(flags);
	
	if( (standard_flag[chid] == 3) && (service_type == 1)) // NTT Type II
	{
		RtkAc49xApiGenNttAlertTone(chid);
	}

#if 1
	for(i=0; i < pCallerIdAsciMessage->Title.Length.Value; i++)
		PRINT_MSG("%X ", pCallerIdAsciMessage->Data[i].Value);
	PRINT_MSG("\n");
#endif	
	
	if (service_type == 0) // On-hook
	{
		RtkAc49xApiOffhookAction(channel); // Open ACMW Channel for sending CID
	}


	ret = Ac49xSendCallerIdMessage(device, channel, pCallerIdAttr, pCallerIdAsciMessage);
	
	if (ret != 0)
		printk(AC_FORE_RED "Send Caller ID Error for CH%d\n" AC_RESET, chid);
	
	return ret;
}


int RtkAc49xApiSendVmwi(int chid, unsigned char vmwi_msg_type, char *pIndicator)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int ret = 0, channel, i;
	Tac49xCallerIdAttr		callerIdAttr;
	Tac49xCallerIdAsciMessage 	callerIdAsciMessage;
	Tac49xCallerIdAttr 		*pCallerIdAttr;
	Tac49xCallerIdAsciMessage 	*pCallerIdAsciMessage;
	const uint32 cch = chid;
	
	channel = chid;
	pCallerIdAttr = &callerIdAttr;
	pCallerIdAsciMessage = &callerIdAsciMessage;
	
	pCallerIdAttr->ChecksumByteAbsent = CONTROL__DISABLE;
	pCallerIdAttr->Error = CONTROL__DISABLE;
	pCallerIdAttr->Event = CALLER_ID_EVENT__NONE_EVENT;	
	pCallerIdAsciMessage->Title.Type.Value = (char)CALLER_ID_SEGMENTATION_MESSAGE_TYPE__NONE_DTMF;
	pCallerIdAttr->Standard = CALLER_ID_STANDARD__ETSI;
	pCallerIdAttr->ServiceType = CALLER_ID_SERVICE_TYPE__1_ON_HOOK;
	pCallerIdAttr->EtsiOnhookMethod = ETSI_ON_HOOK_METHOD__NO_PRECEDING_DUAL_TONE_ALERT_SIGNAL;
	
	pCallerIdAsciMessage->Title.Length.Value = 5;
	pCallerIdAsciMessage->Data[0].Value = MDMF_VMWI;	//Message Type
	pCallerIdAsciMessage->Data[1].Value = 0x03; 		//Message Length
	pCallerIdAsciMessage->Data[2].Value = VMWI_STATUS;	//Paramater Type
	pCallerIdAsciMessage->Data[3].Value = 0x01;		//Paramater Length
	pCallerIdAsciMessage->Data[4].Value = (*pIndicator) ? 0xff : 0x0;//Indicator on/off

	
#if 1
	for(i=0; i < pCallerIdAsciMessage->Title.Length.Value; i++)
		PRINT_MSG("%X ", pCallerIdAsciMessage->Data[i].Value);
	PRINT_MSG("\n");
#endif		
	OnHookLineReversal(cch, 0);
	
	RtkAc49xApiOffhookAction(channel);

	ret = Ac49xSendCallerIdMessage(device, channel, pCallerIdAttr, pCallerIdAsciMessage);

	if (ret != 0)
		printk(AC_FORE_RED "Send VMWI Error for CH%d\n" AC_RESET, chid);
	
	return ret;
}


int RtkAc49xApiSendDtmfCallerId(int chid, unsigned char type, unsigned char *pMessage)
{
	unsigned int device = (&TxPacketParams)->Device;
	unsigned int i, ret, channel, cid_len;
	Tac49xCallerIdAttr 		callerIdAttr;
	Tac49xCallerIdAsciMessage 	callerIdAsciMessage;
	Tac49xCallerIdAttr 		*pCallerIdAttr;
	Tac49xCallerIdAsciMessage 	*pCallerIdAsciMessage;
	extern char cid_dtmf_mode[];
	extern char fsk_spec_areas[];
	//ring_struct ringing;
	unsigned char ring_set;
	const uint32 cch = chid;
	
	channel = chid;
	pCallerIdAttr = &callerIdAttr;
	pCallerIdAsciMessage = &callerIdAsciMessage;
	
	if (strcmp(pMessage, "-") == 0)
	{
		PRINT_MSG("Not Support 'Private' DTMF Caller ID(ch=%d).\n", chid);
		return -1;
	}
	
	/////////// SET CALLER ID ETSI DTMF TYPE 1 ATTRIBUTES //////////

	pCallerIdAttr->Standard 		= CALLER_ID_STANDARD__DTMF_CLIP_ETSI;
	pCallerIdAttr->ServiceType 		= CALLER_ID_SERVICE_TYPE__1_ON_HOOK;
	pCallerIdAttr->EtsiOnhookMethod 	= ETSI_ON_HOOK_METHOD__NO_PRECEDING_DUAL_TONE_ALERT_SIGNAL;	
	pCallerIdAttr->ChecksumByteAbsent 	= CONTROL__DISABLE;
	pCallerIdAttr->Event 			= CALLER_ID_EVENT__NONE_EVENT;	
	
	/////////// SET CALLER ID MESSAGE //////////
	
	pCallerIdAsciMessage->Title.Type.Value = (char)CALLER_ID_SEGMENTATION_MESSAGE_TYPE__DTMF;
	
	//CALLER ID MESSAGE LENGTH (in BYTE)
	cid_len = strlen(pMessage);
	pCallerIdAsciMessage->Title.Length.Value = cid_len+2;
	PRINT_MSG("cid len=%d\n", cid_len);
	
	pCallerIdAsciMessage->Data[0].Value = (U8)('A'); 	 //start preambel ('A' or 'D')
	pCallerIdAsciMessage->Data[cid_len+1].Value = (U8)('A'); //stop preambel ('C')
	pCallerIdAsciMessage->Data[0].Value += cid_dtmf_mode[chid]&0x03;
	pCallerIdAsciMessage->Data[cid_len+1].Value += (cid_dtmf_mode[chid]>>2)&0x03;
	//PRINT_MSG("%d, %d\n",pCallerIdAsciMessage->Data[0].Value, pCallerIdAsciMessage->Data[cid_len+1].Value);
	
	for(i=0; i<cid_len; i++)
	{
		pCallerIdAsciMessage->Data[i+1].Value = (U8)(*(pMessage+i));
	}
	
	// Caller ID Prior First Ring
	if(!(fsk_spec_areas[channel]&0x08))
	{
		//ringing.CH = chid;
		//ringing.ring_set = 1;
		//FXS_Ring(&ringing);
		FXS_Ring( cch, 1 );
		mdelay(10);
		
		//ringing.CH = chid;
		while(FXS_Check_Ring(chid/*&ringing*/))
			mdelay(10);
	}

	OnHookLineReversal(cch, 0);	
	RtkAc49xApiOffhookAction(channel); // Open ACMW Channel for sending CID
	
	ret = Ac49xSendCallerIdMessage(device, channel, pCallerIdAttr, pCallerIdAsciMessage);
	//PRINT_MSG("Send DTMF CID.\n");
	
	if (ret != 0)
		printk(AC_FORE_RED "Send Caller ID Error for CH%d\n" AC_RESET, chid);
	
	return ret;
}

int RtkAc49xApiResetRtpStatistics(int chid)
{
	unsigned int device = (&TxPacketParams)->Device, indx=0, para_value=0;
	Tac49xSetRtcpParametersAttr Rtcp_Attr;
	
	/*Data Fields:
		Tac49xRtcpParameterIndex ParameterIndex 
		U32 ParameterValue 
		
		ParameterIndex:
		Index of the modified parameter. 
		Default = RTCP_PARAMETER_INDEX__TRANSMIT_PACKET_COUNT
		
		RTCP_PARAMETER_INDEX__TRANSMIT_PACKET_COUNT   
		RTCP_PARAMETER_INDEX__TRANSMIT_OCTETS_COUNT   
		RTCP_PARAMETER_INDEX__RECEIVE_PACKET_COUNT   
		RTCP_PARAMETER_INDEX__RECEIVE_OCTETS_COUNT   
		RTCP_PARAMETER_INDEX__JITTER   
		RTCP_PARAMETER_INDEX__PACKET_LOSS   
		RTCP_PARAMETER_INDEX__ROUND_TRIP_DELAY  
		
		ParameterValue:
		The value of the parameter. If the value size is less then 32 bits, 
		then it is located in the LS bits.

	*/

	if (gAcmwChannelState[chid] == CH_CLOSE)
	{
		RtkAc49xApiOffhookAction(chid);
		gCloseAcmwChAfterGetRtcpStatistic[chid] = 1;
		gAcmwChannelState[chid] = CH_OPEN_FOR_GET_RTCP;
	}
	
	for (indx = 0; indx <= RTCP_PARAMETER_INDEX__ROUND_TRIP_DELAY; indx++)
	{
		Rtcp_Attr.ParameterIndex = indx;
		Rtcp_Attr.ParameterValue = para_value;
		Ac49xSetRegularRtcpParameters(device, chid, &Rtcp_Attr);
	}

	//PRINT_MSG("Restset RTCP Statistic.\n");
	
	if ((gCloseAcmwChAfterGetRtcpStatistic[chid] == 1) && (gAcmwChannelState[chid] == CH_OPEN_FOR_GET_RTCP))	
		RtkAc49xApiOnhookAction(chid);
		
	return 0;
}

int RtkAc49xApiGetRtpStatistics(int chid)
{
	//This API will cause the middleware to send RTCP event packet 
	//with the required parameters.
	
	unsigned int device = (&TxPacketParams)->Device;
	
	if (gAcmwChannelState[chid] == CH_CLOSE)
	{
		RtkAc49xApiOffhookAction(chid);// Must Open ACMW Channel and then RTCP statistic can be got.
		gAcmwChannelState[chid] = CH_OPEN_FOR_GET_RTCP;
		gCloseAcmwChAfterGetRtcpStatistic[chid] = 1;
	}
		
	Ac49xGetRegularRtcpParameters(device, chid);
	
	return 0;
}

unsigned short RtkAc49xGetChannelState(int chid)
{
	/* thlin+
	 * In current implementation, 
	 * when phone is onhook, acmw channel state is in CHANNEL_STATE__IDLE.
	 * when phone is offhook, acmw channel state is in CHANNEL_STATE__ACTIVE_NONE.
	 * when VoIP call is estiablished, acmw channel state is in CHANNEL_STATE__ACTIVE_RTP.
	 * when one FXS port is in mode-1 3-way conference, the both acmw channel state are CHANNEL_STATE__ACTIVE_RTP.
	 * when one FXS port hold one far-end VoIP user A, and talk with another far-end VoIP user B, 
	 * then the one acmw channel is in CHANNEL_STATE__IDLE, and another channel state is in CHANNEL_STATE__ACTIVE_RTP.
	 * Note, when calling RtkAc49xApiOffhookAction(), channel state will change to CHANNEL_STATE__ACTIVE_NONE.
	 *       when calling RtkAc49xApiOnhookAction(), channel state will change to CHANNEL_STATE__IDLE.
	 *       So, during sending type-I caller ID, the channel state will be 
	 *	 CHANNEL_STATE__IDLE-> CHANNEL_STATE__ACTIVE_NONE -> Send CID, and send OK ->CHANNEL_STATE__IDLE
	 */
	unsigned short ch_state;
	ch_state = ac49xRxPacket.u.ProprietaryPayload.u.StatusOrEvent.u.DeviceStatus.ChannelStatusPayload[chid].ChannelState ;
	//PRINT_MSG("ch%d state = %d\n", chid, ch_state);
	
	return ch_state;
}


// This API update channel state, so it should be call when RTP close, channel open state.
int RtkAc49xApiSetIntoBypassMode(int chid, int bypass)
{
	unsigned int device = (&TxPacketParams)->Device;

	gSetByassMode[chid] = 0;

	/* Set Jitter Buffer to static delay for bypass mode */
	dsp_dev->SetupChannelAttr[chid].OpenOrUpdateChannel.DynamicJitterBufferOptimizationFactor = DYNAMIC_JITTER_BUFFER_OPTIMIZATION_FACTOR__STATIC_DELAY_FOR_BYPASS_MODE;
	
	/* Turn off VAD/CNG */
	dsp_dev->SetupChannelAttr[chid].ActivateOrUpdateRtp[0].SilenceCompressionMode = SILENCE_COMPRESSION_MODE__DISABLE;

	/* Turn off LEC/NLP for modem bypass, turn off NLP only for fax bypass */
	if (bypass == MODEM_BYPASS)// modem bypass
	{
		dsp_dev->SetupChannelAttr[chid].OpenOrUpdateChannel.EchoCancelerMode = ECHO_CANCELER_MODE__DISABLE;
	}
	dsp_dev->SetupChannelAttr[chid].OpenOrUpdateChannel.EchoCancelerNonLinearProcessor = ECHO_CANCELER_NON_LINEAR_PROCESSOR_MODE__DISABLE;

	Ac49xUpdateChannelConfiguration(device , chid, &(dsp_dev->SetupChannelAttr[chid].OpenOrUpdateChannel));

	//printk("-----> Go into bypass mode\n");
	return 0;
}

#ifdef ACMW_PLAYBACK
int RtkAc49xApiPlayIvrTdmStart(int chid, Tac49xCoder coder)
{
	unsigned int device = (&TxPacketParams)->Device;
	int ret;
	
	ret = Ac49xPlaybackTdmStart(device, chid, coder);
	
	if (ret == BUFFER_DESCRIPTOR_FULL_ERROR)
	{
		printk(AC_FORE_RED "BUFFER_DESCRIPTOR_FULL_ERROR: %s:%s:%d\n" AC_RESET, __FILE__, __FUNCTION__, __LINE__);
	}
	else if (ret != 0) // 0 is success
	{
		//printk(AC_FORE_RED "Fatal ERROR: %s:%s:%d\n" AC_RESET, __FILE__, __FUNCTION__, __LINE__);
	}
	
	pb_state[chid] = PB_TDM;
	
	return ret; 
}

int RtkAc49xApiPlayIvrNetworkStart(int chid, Tac49xCoder coder)
{
	unsigned int device = (&TxPacketParams)->Device;
	int ret;
	
	ret = Ac49xPlaybackNetworkStart(device, chid, coder);

	if (ret == BUFFER_DESCRIPTOR_FULL_ERROR)
	{
		printk(AC_FORE_RED "BUFFER_DESCRIPTOR_FULL_ERROR: %s:%s:%d\n" AC_RESET, __FILE__, __FUNCTION__, __LINE__);
	}
	else if (ret != 0) // 0 is success
	{
		//printk(AC_FORE_RED "Fatal ERROR: %s:%s:%d\n" AC_RESET, __FILE__, __FUNCTION__, __LINE__);
	}
	
	pb_state[chid] = PB_NET;
	
	return ret; 
}

/*
	Note: Silence playback steps:
	1. RtkAc49xApiPlayIvrTdmStart or RtkAc49xApiPlayIvrNetworkStart
	2. RtkAc49xApiPlaySilence
	3. RtkAc49xApiPlayIvrEnd
	Once you called PlaybackEnd it will activate the silence
	(as long as the time you set it to)and then return to the original IBS tone.
*/

int RtkAc49xApiPlaySilence(int chid, int silence_msec)
{
	unsigned int device = (&TxPacketParams)->Device;
	int ret;
	
	/* silence_msec: 
	 *	in msec resolution; 
	 *	the duration is rounded to the nearest multiple of the
         *	coder duration; the range is 0-16000 seconds. 
         */	
	ret = Ac49xPlaybackPlaySilence(device, chid, silence_msec);
	
	if (ret == BUFFER_DESCRIPTOR_FULL_ERROR)
	{
		printk(AC_FORE_RED "BUFFER_DESCRIPTOR_FULL_ERROR: %s:%s:%d\n" AC_RESET, __FILE__, __FUNCTION__, __LINE__);
	}
	else if (ret != 0) // 0 is success
	{
		//printk(AC_FORE_RED "Fatal ERROR: %s:%s:%d\n" AC_RESET, __FILE__, __FUNCTION__, __LINE__);
	}
	
	pb_state[chid] = PB_SIL;
	
	return ret; 
}

int RtkAc49xApiPlayIvr(int chid, char *pBuf, int BufSize)
{
	/* BufSize:
	 *	Size of the voice payload. 
	 *	It must be smaller than the received AvailableBufferSpace_msec. 
	 *	Refer to Tac49xPlaybackDspCommand.
	 */
	unsigned int device = (&TxPacketParams)->Device;
	int ret;
	
	ret = Ac49xPlaybackPlayVoice( device, chid, pBuf, BufSize);

	if (ret == BUFFER_DESCRIPTOR_FULL_ERROR)
	{
		printk(AC_FORE_RED "BUFFER_DESCRIPTOR_FULL_ERROR: %s:%s:%d\n" AC_RESET, __FILE__, __FUNCTION__, __LINE__);
	}
	else if (ret != 0) // 0 is success
	{
		//printk(AC_FORE_RED "Fatal ERROR: %s:%s:%d\n" AC_RESET, __FILE__, __FUNCTION__, __LINE__);
	}
	
	return ret; 
}

int RtkAc49xApiPollIvr(int chid)
{
	return pb_state[chid];
}

int RtkAc49xApiPlayIvrEnd(int chid)
{
	unsigned int device = (&TxPacketParams)->Device;
	int ret;
	
	ret = Ac49xPlaybackEnd( device, chid);

	if (ret == BUFFER_DESCRIPTOR_FULL_ERROR)
	{
		printk(AC_FORE_RED "BUFFER_DESCRIPTOR_FULL_ERROR: %s:%s:%d\n" AC_RESET, __FILE__, __FUNCTION__, __LINE__);
	}
	else if (ret != 0) // 0 is success
	{
		//printk(AC_FORE_RED "Fatal ERROR: %s:%s:%d\n" AC_RESET, __FILE__, __FUNCTION__, __LINE__);
	}
	
	pb_state[chid] = PB_END;
	
	IvrTextSpeechEvent[chid] = 0;
	IvrTextSpeechBufIdx[chid] = 0;
	play_text_idx[chid] = 0;
	play_text_flag[chid] = 0;
	text_play_len[chid] = 0;
	play_g723h_flag[chid] = 0;
	play_g729_flag[chid] = 0;
	play_g711_flag[chid] = 0;
	
	//PRINT_G("IVR Play End, ch=%d!\n", chid);
	
	return ret; 
}

int RtkAc49xApiPlayBackSetting(int chid, int water_mark_msec, Tac49xControl Request_enable)
{

	unsigned int device = (&TxPacketParams)->Device;
	unsigned int channel;
	channel = chid;

	dsp_dev->SetupChannelAttr[channel].Advanced.PlaybackWaterMark_msec = water_mark_msec;
	dsp_dev->SetupChannelAttr[channel].Advanced.PlaybackRequestEnable = Request_enable;
	Ac49xAdvancedChannelConfiguration(device , channel, &(dsp_dev->SetupChannelAttr[channel].Advanced));

	pb_state[chid] = 0;

	return 0;
}
#endif

static void RtkAc49xApiSetInitialConfig(void)
{
	int chid;
	extern int Is_DAA_Channel(int chid);
	
	for (chid=0; chid < AC49X_NUMBER_OF_CHANNELS; chid++)
	{
		if (Is_DAA_Channel(chid) == 1)
		{
			RtkAc49xApiOffhookAction(chid);
		}
		RtkAc49xApiSetFaxTransferMethod(chid, FAX_MODEM_TRANSFER_METHOD__BYPASS); // set to auto-bypass mode
		//RtkAc49xApiSetCngRelay(chid, CONTROL__ENABLE);
		RtkAc49xApiSetCedTransferMode(chid, CED_TRANSFER_MODE__IN_VOICE_OR_PCM_BYPASS_MODE_TRIGGER_FAX_BY_V21_PREAMBLE);
		RtkAc49xApiDetectNetIbsPackets(chid, CONTROL__ENABLE);
		RtkAc49xApiSetInputGainLocation(chid, INPUT_GAIN_LOCATION__AFTER_ECHO_CANCELER_AND_BOFORE_ENCODER);
		RtkAc49xApiActiveEchoCancellerNLP(chid);//for ACMW
		RtkAc49xApi_LEC_enable(chid);//for RTK defined flag
		RtkAc49xApiSetCallProgressIBSDetection(chid, CONTROL__ENABLE);
		RtkAc49xApiSetIBSDetectionRedirection(chid, IBS_DETECTION_REDIRECTION__ON_INPUT_STREAM_ENCODER_INPUT);
#ifdef ACMW_PLAYBACK
		RtkAc49xApiPlayBackSetting(chid, 80, CONTROL__ENABLE);
#endif
		if (Is_DAA_Channel(chid) == 1)
		{
			RtkAc49xApiSetDtmfDetection(chid, CONTROL__ENABLE);
		}
		else
		{
			RtkAc49xApiSetDtmfDetection(chid, CONTROL__DISABLE);
		}
	}
}

int RtkAc49xApiSetupDevice(void)
{
	extern Tac49xSetupDeviceStatus Ac49xSetupDevice(int Device, Tac49xSetupDeviceAttr *pSetupDeviceAttr);
	
	Ac49xSetupDevice(0 /* Device */, &dsp_dev->SetupDeviceAttr);
	
	/* Important and Must! 
	   Becasue re-setup device, so re-init config */
	RtkAc49xApiSetInitialConfig();
	
	return 0;
}

/****************************************************************************
*
*	Name:		dspDev_setDefaults
*----------------------------------------------------------------------------
*	Abstract:	set the dsp device and channels to defaults values
*----------------------------------------------------------------------------
*	Input:		none
*----------------------------------------------------------------------------
*  	Output:		none
*----------------------------------------------------------------------------
*	Returns: 	case of success = AC494_DSP_OK_E
******************************************************************************/

static int dspDev_setDefaults(void)
{
	extern Tac49xSetupDeviceStatus Ac49xSetupDevice(int Device, Tac49xSetupDeviceAttr *pSetupDeviceAttr);
	extern void Ac49xSetDefaultSetupDeviceAttr(Tac49xSetupDeviceAttr *pSetupDeviceAttr);
	extern void Ac49xSetDefaultSetupChannelAttr(Tac49xSetupChannelAttr *pSetupChannelAttr);
	extern int Ac49xInitDriver(Tac49xControl DebugMode);
	int channel;

	/* set device defaults */
	Ac49xSetDefaultSetupDeviceAttr(&(dsp_dev->SetupDeviceAttr));

	dsp_dev->SetupDeviceAttr.Open.NumberOfChannels = dsp_dev->numOfChannels;

	/* set the channels over the device */
	for(channel=0; channel< (dsp_dev->SetupDeviceAttr.Open.NumberOfChannels) ; channel++)
	{
		dsp_dev->SetupDeviceAttr.Open.Channel[channel].Slot = channel;
		dsp_dev->SetupDeviceAttr.Open.Channel[channel].InterconnectedSlot = channel+4;
	}

	/* set channels defaults */
	for(channel=0; channel< (dsp_dev->SetupDeviceAttr.Open.NumberOfChannels) ; channel++)
		Ac49xSetDefaultSetupChannelAttr(&(dsp_dev->SetupChannelAttr[channel]));


#if 0
	/* by default connect channel 0 to the handset */
	dsp_dev->SetupChannelAttr[0].CodecConfiguration.HandsetInputToOutput = CONTROL__ENABLE;
	dsp_dev->SetupChannelAttr[0].CodecConfiguration.HandsetInputSelect = CONTROL__ENABLE;
	dsp_dev->SetupChannelAttr[0].CodecConfiguration.HandsetOutputSelect = CONTROL__ENABLE;
#endif
	Ac49xInitDriver(CONTROL__ENABLE);
	Ac49xResetDeviceBufferDescriptors(0);
	
	
	for(channel=0; channel< (dsp_dev->SetupDeviceAttr.Open.NumberOfChannels) ; channel++) 
	{
		/* Init Tx Packet */
		//pTxPacketParams = &TxPacketParams;
		//pTxPacketParams->Channel = channel;
		(&TxPacketParams)->Device = 0;
		
		/* Init Rx Packet */
		(&RxPacketParams)->Device = 0;
		(&RxPacketParams)->TransferMedium = TRANSFER_MEDIUM__HOST ;
		(&RxPacketParams)->pInPacket  = (char *)&ac49xRxPacket;
	}

	Ac49xInitializePacketReceive(ACMWPacketCB); 
	Ac49xSetupDevice(0 /* Device */, &dsp_dev->SetupDeviceAttr);

	return AC494_DSP_OK_E;

} /* end of dspDev_setDefaults() */
/******************************************************************************/



/****************************************************************************
*
*	Name:		dspDev_init
*----------------------------------------------------------------------------
*	Abstract:	initialize the device (call with insmod)
*----------------------------------------------------------------------------
*	Input:		none
*----------------------------------------------------------------------------
*  	Output:		none
*----------------------------------------------------------------------------
*	Returns: 	case of success = 0, else -1
******************************************************************************/
int __init	dspDev_init(void)
{
	int i;
	extern int voip_ch_num;
	Tac49xDriverVersionInfo DriverVersionInfo;
	dsp_dev->numOfChannels = AC49X_NUMBER_OF_CHANNELS;
	dsp_dev->downloadStatus = 0;
	dsp_dev->initStatus = 0;

	printk(AC_FORE_YELLOW "<<<<<<<<<<<<<<<< AudioCodes DSP Init >>>>>>>>>>>>>>>>>>>>\n");
	dspDev_setDefaults();
	DriverVersionInfo = RtkAc49xApiGetDriverVersionInfo();
	printk("AC49X DSP Driver Version Info:\n");
	printk("MandatoryVersion: %s\n", DriverVersionInfo.MandatoryVersionPrefix);
	printk("SubVersion: %s\n", DriverVersionInfo.SubVersion );
	printk("Date: %s\n", DriverVersionInfo.Date);
	printk("Comments : %s\n", DriverVersionInfo.sComments );
	printk("<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>\n" AC_RESET);

	//extern void Init_Event_Polling_Use_Timer(void);
	//Init_Event_Polling_Use_Timer();
	RtkAc49xApiSetInitialConfig();
	for (i=0; i < voip_ch_num; i++)
		dtmf_cid_init(i);

	return (0);

} /* end of dspDev_init() */
/******************************************************************************/


void __exit dspDev_exit(void)
{
	//devfs_unregister(devfs_dir_handle);
	kfree(dsp_dev);
}



voip_initcall(dspDev_init);
voip_exitcall(dspDev_exit);
//module_init(dspDev_init);
//module_exit(dspDev_exit);


