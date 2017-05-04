
//
// dspcodec.cpp
//

#include <linux/string.h>
#include <linux/interrupt.h>
#include <net/ip.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
//#include "rtk_voip.h"
#include "dspcodec_0.h"
#include "dspparam.h"
#include "assert.h"
#include "codec_descriptor.h"

//for change country and still play tone.
#include "typedef.h"
extern Flag fPlayTone[][CH_TONE];
#include "rtk_voip.h"//for change country and still play tone.
#include "voip_timer.h"

extern Word16  WhichTone[][CH_TONE];//for change country and still play tone.
#include "voip_init.h"
#include "voip_proc.h"

/* extern variable */
extern CPacketTxBuffer PacketTxBuffer[];
extern CJitterBuffer JitterBuffer[];
extern int dsp_rtk_ss_num;


/* global variable */
#ifdef SUPPORT_DYNAMIC_PAYLOAD
//int DPSize[MAX_SESS_NUM] = {1};

//#define SUPPORT_SYNC_OUT_OF_SEQ
#define SUPPORT_SYNC_BIG_CHANGE

#ifdef DYNAMIC_PAYLOAD_VER1
int DPInitSeq[MAX_DSP_RTK_SS_NUM] = {0};
int DPInitStamp[MAX_DSP_RTK_SS_NUM] = {0};
#else
uint16 DPInSeq[MAX_DSP_RTK_SS_NUM] = {0};		// last internal seq_no 
//uint16 DPOutSeq[MAX_SESS_NUM] = {0};
uint16 DPExpSeq[MAX_DSP_RTK_SS_NUM] = {0};	// expected seq_no from network 
uint32 DPExpTimeStamp[MAX_DSP_RTK_SS_NUM] = {0};	// expected timestamp from network 
uint16 DPNotFirstPacket[MAX_DSP_RTK_SS_NUM] = {0};	// If first packet from network, above two expected values are ignore 
 #ifdef SUPPORT_SYNC_OUT_OF_SEQ
uint16 DPEnableSyncInSeq[MAX_DSP_RTK_SS_NUM] = {0};
uint16 DPSyncInSeq[MAX_DSP_RTK_SS_NUM] = {0};
 #endif
 #ifdef SUPPORT_SYNC_BIG_CHANGE
uint16 BigChangeCount[ MAX_DSP_RTK_SS_NUM ];
uint16 BigChangePreSeq[ MAX_DSP_RTK_SS_NUM ];
uint16 BigChangeDeltaSeq[ MAX_DSP_RTK_SS_NUM ];
 #endif
#endif

#endif	// SUPPORT_DYNAMIC_PAYLOAD

// declarations of constants and static data
//
/*	handsome add function 2005.12.1     */
//int iCountry = DSPCODEC_COUNTRY_USA;
//int dialTone = 0;
//int ringTone = 1;
//int busyTone = 2;
//int waitingTone = 3;
uint32 cust = 0;

// writing states
#if 0
typedef enum
{
	DSPCODEC_WSTATE_NORMALJITTER,
	DSPCODEC_WSTATE_SYNCJITTER
} DSPCODEC_WSTATE;
#endif

#define RTP_SEQ_MOD		(1 << 16)

#ifdef RESERVE_SPACE_FOR_SLOW
enum {	// for nOwner
	R0_OWN = 0,			// ready for R1 
	R1_OWN = 1,			// R0 can place new data
	R0_RESERVE = 2,		// reserved for slow packet (out-of-order)
};
#endif

/* static variable */
//static bool bCreated[MAX_SESS_NUM] = {false};                                       // can create only once

static int over_load_cnt[MAX_DSP_RTK_SS_NUM]={0}, outof_seq_cnt[MAX_DSP_RTK_SS_NUM]={0};    // sandro move from DspcodecWrite()
#ifdef SUPPORT_SYNC_OUT_OF_SEQ
static uint16 outof_seq_prevSeq[MAX_DSP_RTK_SS_NUM] = { 0 };
#endif

//WJF 930806 added, when reached tx_jit_buf_high_threshold, touch_high_threshold will be 1, 
// touch_high_threshold will be 0 when down to tx_jit_buf_low_threshold

#if defined( SUPPORT_ADJUST_JITTER ) && !defined( RESERVE_SPACE_FOR_SLOW )
extern int jbc_max_delay[MAX_DSP_RTK_SS_NUM];
extern int jbc_target_delay[MAX_DSP_RTK_SS_NUM];
#endif
int tx_jit_buf_low_threshold[MAX_DSP_RTK_SS_NUM]={10};
int tx_jit_buf_high_threshold[MAX_DSP_RTK_SS_NUM]={30};

int touch_high_threshold[MAX_DSP_RTK_SS_NUM]={0};

const codec_algo_desc_t *ppNowTranCodecAlgorithmDesc[MAX_DSP_RTK_SS_NUM];
const codec_algo_desc_t *ppNowRecvCodecAlgorithmDesc[MAX_DSP_RTK_SS_NUM];

// pkshih: move to rtk_trap.c 
extern uint32 nRxRtpStatsLostPacket[MAX_DSP_RTK_SS_NUM];

uint32 nRxSilencePacket[MAX_DSP_RTK_SS_NUM];

//static volatile CDspcodecParm* pSendParm;		// the jitter buffer of sending data to RISC1
//static volatile CDspcodecResponse* PacketTxBuffer;	// the jitter buffer of receiving data from 
//#define pSendParm	SEND_ADDR
//#define PacketTxBuffer	RECV_ADDR

typedef struct
{
	//DSPCODEC_ALGORITHM	m_uTranCodingAlgorithm;  	// which coding algorithm used: G711u, G711a, G723.1a53, G723.1a63, G729
	//DSPCODEC_ALGORITHM	m_uRecvCodingAlgorithm;  	// which coding algorithm used: G711u, G711a, G723.1a53, G723.1a63, G729
	//CDspcodecConfig		m_xConfig;				// remember DSP configuration
	//DSPCODEC_INTERFACE	m_nInterface;			// interface to play
	int32				m_nRingType;			// which ring to play

#ifdef SUPPORT_TONE_PROFILE
	int32				m_nBaseTone;			// index of base tone ( dial tone)
#else
	int32				m_nDialTone;			// index of dial tone
	int32				m_nRingTone;			// index of ring tone
	int32				m_nBusyTone;			// index of busy tone
	int32				m_nWaitingTone;			// index of waiting tone
#endif

#if 0
	// data members for writing and reading
#ifndef CLEAN_JITTER_BUFFER_PARAMS
	uint32				m_nPreTick;				// the running tick of RISC1 of the last packet put
	uint32				m_nPreTimeStamp;		// the timestamp of the last packet put
	DSPCODEC_WSTATE	m_nWState;				// the writing state
	int32				m_nInitTd;				// the initial Td (vary according to codec)
	int32				m_nPreTd;				// the time delayed of the last packet put
	int32				m_nAveTd;				// the average Td
	int32				m_nTdOffset;			// the offset of Td
	uint32				m_njSN;					// the sequence no. of the frame to sync jitter
#endif
	// statistics for monitoring or debugging
	//uint32				m_nDuplicate;			// the frame is duplicated
	//uint32				m_nIntNum;				// the number RISC1 has sent interrupts
	//uint32				m_nR1ResetNum;			// the number of reseting RISC1
#endif
} DspcodecData;
static DspcodecData Dsp_data[MAX_DSP_RTK_SS_NUM];

#if 0
#define INIT_LIST_SESS_NUM2		\
	&JitterBuffer[ 0 ].m_nPosDec, 	\
	&JitterBuffer[ 1 ].m_nPosDec
#define INIT_LIST_SESS_NUM4		\
	INIT_LIST_SESS_NUM2,		\
	&JitterBuffer[ 2 ].m_nPosDec, 	\
	&JitterBuffer[ 3 ].m_nPosDec 	
#define INIT_LIST_SESS_NUM6		\
	INIT_LIST_SESS_NUM4,		\
	&JitterBuffer[ 4 ].m_nPosDec, 	\
	&JitterBuffer[ 5 ].m_nPosDec 	
#define INIT_LIST_SESS_NUM8		\
	INIT_LIST_SESS_NUM6,		\
	&JitterBuffer[ 6 ].m_nPosDec, 	\
	&JitterBuffer[ 7 ].m_nPosDec 	
#define INIT_LIST_SESS_NUM10	\
	INIT_LIST_SESS_NUM8,		\
	&JitterBuffer[ 8 ].m_nPosDec, 	\
	&JitterBuffer[ 9 ].m_nPosDec 	
#define INIT_LIST_SESS_NUM12	\
	INIT_LIST_SESS_NUM10,		\
	&JitterBuffer[ 10 ].m_nPosDec, 	\
	&JitterBuffer[ 11 ].m_nPosDec 	
#define INIT_LIST_SESS_NUM14	\
	INIT_LIST_SESS_NUM12,		\
	&JitterBuffer[ 12 ].m_nPosDec,	\
	&JitterBuffer[ 13 ].m_nPosDec
#define INIT_LIST_SESS_NUM16	\
	INIT_LIST_SESS_NUM14,		\
	&JitterBuffer[ 14 ].m_nPosDec,	\
	&JitterBuffer[ 15 ].m_nPosDec
#define INIT_LIST_SESS_NUM32	\
	INIT_LIST_SESS_NUM16,		\
	&JitterBuffer[ 16 ].m_nPosDec,	\
	&JitterBuffer[ 17 ].m_nPosDec,	\
	&JitterBuffer[ 18 ].m_nPosDec,	\
	&JitterBuffer[ 19 ].m_nPosDec,	\
	&JitterBuffer[ 20 ].m_nPosDec,	\
	&JitterBuffer[ 21 ].m_nPosDec,	\
	&JitterBuffer[ 22 ].m_nPosDec,	\
	&JitterBuffer[ 23 ].m_nPosDec,	\
	&JitterBuffer[ 24 ].m_nPosDec,	\
	&JitterBuffer[ 25 ].m_nPosDec,	\
	&JitterBuffer[ 26 ].m_nPosDec,	\
	&JitterBuffer[ 27 ].m_nPosDec,	\
	&JitterBuffer[ 28 ].m_nPosDec,	\
	&JitterBuffer[ 29 ].m_nPosDec,	\
	&JitterBuffer[ 30 ].m_nPosDec,	\
	&JitterBuffer[ 31 ].m_nPosDec	

#ifdef RESERVE_SPACE_FOR_SLOW
extern const uint32 * const p_DspDecWrite[];
const uint32 * const p_DspDecWrite[MAX_DSP_RTK_SS_NUM] = {
 #if MAX_DSP_RTK_SS_NUM == 2
 	INIT_LIST_SESS_NUM2,
 #elif MAX_DSP_RTK_SS_NUM == 4
	INIT_LIST_SESS_NUM4,
 #elif MAX_DSP_RTK_SS_NUM == 6
	INIT_LIST_SESS_NUM6,
 #elif MAX_DSP_RTK_SS_NUM == 8
	INIT_LIST_SESS_NUM8,
 #elif MAX_DSP_RTK_SS_NUM == 10
	INIT_LIST_SESS_NUM10,
 #elif MAX_DSP_RTK_SS_NUM == 12
	INIT_LIST_SESS_NUM12,
 #elif MAX_DSP_RTK_SS_NUM == 14
	INIT_LIST_SESS_NUM14,
 #elif MAX_DSP_RTK_SS_NUM == 16
	INIT_LIST_SESS_NUM16,
 #elif MAX_DSP_RTK_SS_NUM == 32
	INIT_LIST_SESS_NUM32,
#else
#error "Need implememt for pointer p_DspDecWrite for configured MAX_DSP_RTK_SS_NUM"
 #endif
};
#endif
#endif

static inline void printk_off( const char *format, ... ) {}

#define debug_warning		printk_off	/* warning level debug */
#define debug_error			printk		/* error level debug */
#define debug_dev		printk_off	/* development debug */
#define debug_cd			printk		/* codec descriptor */

//
// static functions
//
// compare the sequence numbers
static bool RtpSeqGreater(uint16 a, uint16 b)
{
	if((a > (RTP_SEQ_MOD - 20000) && b < 20000) || (a < 20000 && b > (RTP_SEQ_MOD - 20000)))
		return (a < b);
	else
		return (a > b);
}

// send command to RISC1 and wait for acknowledgement
#if 0
static RESULT SendCommand(uint32 sid)
{
	extern void CmdParser(uint32 sid);

	RESULT retval = DSPCODEC_ERROR_RESPONSE;

	PacketTxBuffer[sid].nOwner = 0;

	CmdParser(sid);

	switch(pSendParm[sid].uCommand)
	{
	case DSPCODEC_COMMAND_INITIALIZE:
		if(PacketTxBuffer[sid].uAck == DSPCODEC_ACK_INITIALIZE_DONE)
			retval = DSPCODEC_SUCCESS;
		break;
	case DSPCODEC_COMMAND_SETCONFIG:
		if(PacketTxBuffer[sid].uAck == DSPCODEC_ACK_SETCONFIG_DONE)
			retval = DSPCODEC_SUCCESS;
		break;
	case DSPCODEC_COMMAND_START:
		if(PacketTxBuffer[sid].uAck == DSPCODEC_ACK_START_DONE)
			retval = DSPCODEC_SUCCESS;
		break;
#if 0
	case DSPCODEC_COMMAND_STARTMIXING:
		if(PacketTxBuffer[sid].uAck == DSPCODEC_ACK_STARTMIXING_DONE)
			retval = DSPCODEC_SUCCESS;
		break;
#endif
	case DSPCODEC_COMMAND_STOP:
		if(PacketTxBuffer[sid].uAck == DSPCODEC_ACK_STOP_DONE)
			retval = DSPCODEC_SUCCESS;
		break;
#if 0
	case DSPCODEC_COMMAND_MUTE:
		if(PacketTxBuffer[sid].uAck == DSPCODEC_ACK_MUTE_DONE)
			retval = DSPCODEC_SUCCESS;
		break;
#endif
	case DSPCODEC_COMMAND_PLAYTONE:
		if(PacketTxBuffer[sid].uAck == DSPCODEC_ACK_PLAYTONE_DONE)
			retval = DSPCODEC_SUCCESS;
		break;
#if 0
	case DSPCODEC_COMMAND_JITTERSYNC:
		if(PacketTxBuffer[sid].uAck == DSPCODEC_ACK_JITTERSYNC_DONE)
			retval = DSPCODEC_SUCCESS;
		break;
#endif
#if 0
	case DSPCODEC_COMMAND_GETVERSION:
		if(PacketTxBuffer[sid].uAck == DSPCODEC_ACK_GETVERSION_DONE)
			retval = DSPCODEC_SUCCESS;
		break;
#endif
#if 0
	case DSPCODEC_COMMAND_DEBUG:
		if(PacketTxBuffer[sid].uAck == DSPCODEC_ACK_DEBUG)
			retval = DSPCODEC_SUCCESS;
		break;
#endif
#if 0
	case DSPCODEC_COMMAND_ALIVE:
		if(PacketTxBuffer[sid].uAck == DSPCODEC_ACK_ALIVE)
			retval = DSPCODEC_SUCCESS;
		break;
#endif
	default:
		retval = DSPCODEC_ERROR_RESPONSE;
		break;
	}

	PacketTxBuffer[sid].nOwner = 0;
	return retval;

}
#endif

#ifndef CLEAN_JITTER_BUFFER_PARAMS
// send JITTERSYNC command to RISC1
static RESULT SyncJitterBuf(uint32 sid, uint32 njSN, int32 nTdOffset)
{
	RESULT retval;

	memset((void *)(&(pSendParm[sid].xParm)), 0, sizeof(CDspcodecJitterSyncParm));
	pSendParm[sid].uCommand = DSPCODEC_COMMAND_JITTERSYNC;
	pSendParm[sid].nOwner = 0;
	pSendParm[sid].xParm.xJitterSyncParm.njSN = njSN;
	pSendParm[sid].xParm.xJitterSyncParm.nTdOffset = nTdOffset;

	retval = SendCommand(sid);

	if(retval != DSPCODEC_SUCCESS)
	{
//		printf("Dspcodec: !!! Send command [JITTERSYNC] fail !!!\n");
	}
	return retval;
}
#endif 

//
// public functions
//

#ifdef SUPPORT_DYNAMIC_PAYLOAD
void DynamicPayloadInit( uint32 sid )
{
	//DPSize[sid] = 1;

 #ifdef DYNAMIC_PAYLOAD_VER1 //tyhuang:111
	DPInitSeq[sid] = 0;
	DPInitStamp[sid] = 0;
 #else
	DPInSeq[sid] = 0;
	//DPOutSeq[sid] = 0;
	DPExpSeq[sid] = 0;
	DPExpTimeStamp[sid] = 0;
	DPNotFirstPacket[sid] = 0;
  #ifdef SUPPORT_SYNC_OUT_OF_SEQ
	DPSyncInSeq[sid] = 0;
	DPEnableSyncInSeq[sid] = 0;
  #endif
  #ifdef SUPPORT_SYNC_BIG_CHANGE
	BigChangeCount[ sid ] = 0;
  #endif
 #endif	
}
#endif /* SUPPORT_DYNAMIC_PAYLOAD */

#if 0	// pkshih: unused code 
void DspcodecInitVar(void)
{
	uint32 sid;
	for(sid=0; sid<dsp_rtk_ss_num; sid++)
	{

#ifdef SUPPORT_DYNAMIC_PAYLOAD
		DynamicPayloadInit( sid );
#endif

		bCreated[sid] = false;					// can create only once

#ifdef SUPPORT_ADJUST_JITTER
 #ifdef RESERVE_SPACE_FOR_SLOW
		tx_jit_buf_high_threshold[sid] = ( BUFFER_NUM_DEC - 1 ) - 5;
		tx_jit_buf_low_threshold[sid] = ( BUFFER_NUM_DEC - 1 ) - 10;
 #else
		tx_jit_buf_high_threshold[sid] = jbc_max_delay[sid] + 5;	/* set high threshold equal max delay add 5*/
		tx_jit_buf_low_threshold[sid] = jbc_target_delay[sid];
 #endif
#else
		tx_jit_buf_low_threshold[sid]=10;
		tx_jit_buf_high_threshold[sid]=30;
#endif
		touch_high_threshold[sid]=0;

		over_load_cnt[sid]=0;
		outof_seq_cnt[sid]=0;
	}

}
#endif

void DspcodecUp(uint32 sid) /* Called in DSP_init */
{
	int i;

#if 0
	if(bCreated[sid])
	{
		printk("Session %d is already DspcodecUp!\n",sid);
		assert(0);
	}
	bCreated[sid] = true;
#endif

	//pSendParm = (CDspcodecParm *)SEND_ADDR;
	//PacketTxBuffer = (CDspcodecResponse *)RECV_ADDR;

	/* initialize member data	config default value
	 */
#if 0
	//Dsp_data[sid].m_xConfig.pToneTable 	= (char*)ToneTable;
	//Dsp_data[sid].m_xConfig.pCtrlParm 		= (char*)&CtrlParam;
	Dsp_data[sid].m_xConfig.bVAD 			= false;
	//Dsp_data[sid].m_xConfig.bAES 			= false;
	//Dsp_data[sid].m_xConfig.bPLC 			= false;
	//Dsp_data[sid].m_xConfig.nVadLevel 		= 0;
	//Dsp_data[sid].m_xConfig.nHangoverTime 	= 70;
	//Dsp_data[sid].m_xConfig.nBGNoiseLevel 	= -30;
	//Dsp_data[sid].m_xConfig.nAttenRange 	= -30;
	//Dsp_data[sid].m_xConfig.nTXGain 		= 0;
	//Dsp_data[sid].m_xConfig.nRXGain 		= 0;
	//Dsp_data[sid].m_xConfig.nTRRatio 		= 0;
	//Dsp_data[sid].m_xConfig.nRTRatio 		= 0;
#endif
	/* other data members */
	//Dsp_data[sid].m_nInterface 	= DSPCODEC_INTERFACE_HANDSET;
	Dsp_data[sid].m_nRingType 	= 0;

	/*	handsome add function 2005.12.1     */
	//DspcodecSetCountry(sid, iCountry);
	DspcodecSetCountry(sid, DSPCODEC_COUNTRY_USA);
	//Dsp_data[sid].m_nIntNum 		= 0;
	//Dsp_data[sid].m_nR1ResetNum 	= 0;

	// clear the owner bits of jitter buffer and receiving buffer
	for(i=0; i<BUFFER_NUM_DEC; i++)/*BUFFER_NUM_DEC = 256*/
		JitterBuffer[sid].xDecBuffer[i].nOwner = 1;
	for(i=0; i<BUFFER_NUM_ENC; i++)
		PacketTxBuffer[sid].xEncOutput[i].nOwner = 0;
	//pSendParm[sid].nOwner = 0;
	JitterBuffer[sid].nLastRecvSeqNo = 0;
	//pSendParm[sid].nFramtPut = 0;
	//PacketTxBuffer[sid].nOwner = 0;
	JitterBuffer[sid].nLastPlaySeqNo = 0;

	// load program of RISC1 to memory and turn on RISC1
	DspcodecResetR1(sid);
}

#if 0
void DspcodecDown(void)
{
}
#endif

RESULT DspcodecInitialize(uint32 sid, DSPCODEC_ALGORITHM uTranCodingAlgorithm, DSPCODEC_ALGORITHM uRecvCodingAlgorithm, const CDspcodecConfig *pConfig)
{
	int i;
	RESULT retval;
	CDspcodecInitializeParm xInitializeParm;

#ifdef Doing_723_decode_play_test
	uTranCodingAlgorithm = DSPCODEC_ALGORITHM_G711U ;
	uRecvCodingAlgorithm = DSPCODEC_ALGORITHM_G711U ;
#endif

	if( ( ppNowTranCodecAlgorithmDesc[ sid ] = GetCodecAlgoDesc( uTranCodingAlgorithm ) ) 
		== NULL )
	{
		printk("Dspcodec: !!! Wrong parameter of uTranCodingAlgorithm %d !!!\n", uTranCodingAlgorithm);
		assert(0);
		return DSPCODEC_ERROR_ALGORITHM;
	}

	if( ( ppNowRecvCodecAlgorithmDesc[ sid ] = GetCodecAlgoDesc( uRecvCodingAlgorithm ) ) 
		== NULL )
	{
		printk("Dspcodec: !!! Wrong parameter of uRecvCodingAlgorithm %d !!!\n", uRecvCodingAlgorithm);
		assert(0);
		return DSPCODEC_ERROR_ALGORITHM;
	}

	// clear the owner bits of jitter buffer and receiving buffer
	for(i=0; i<BUFFER_NUM_DEC; i++)
	{
		JitterBuffer[sid].xDecBuffer[i].nOwner = 1;/* Like a passive sense*/
		JitterBuffer[sid].xDecBuffer[i].nSeqNo = 0;
	}
	for(i=0; i<BUFFER_NUM_ENC; i++)
		PacketTxBuffer[sid].xEncOutput[i].nOwner = 0;/* A active sense */
	//pSendParm[sid].nOwner = 0;
	JitterBuffer[sid].nLastRecvSeqNo = 0;
	//pSendParm[sid].nFramtPut = 0;
	//PacketTxBuffer[sid].nOwner = 0;
	JitterBuffer[sid].nLastPlaySeqNo = 0;
	JitterBuffer[sid].bPutNew = 0;

#if 0
	memset((void *)(&(pSendParm[sid].xParm)), 0, sizeof(CDspcodecInitializeParm));
	Dsp_data[sid].m_uTranCodingAlgorithm = uTranCodingAlgorithm;
	Dsp_data[sid].m_uRecvCodingAlgorithm = uRecvCodingAlgorithm;

	//pSendParm[sid].uCommand = DSPCODEC_COMMAND_INITIALIZE;
	//pSendParm[sid].nOwner = 0;
	pSendParm[sid].xParm.xInitializeParm.uTranCodingAlgorithm 	= Dsp_data[sid].m_uTranCodingAlgorithm;
	pSendParm[sid].xParm.xInitializeParm.uRecvCodingAlgorithm	= Dsp_data[sid].m_uRecvCodingAlgorithm;

	//pSendParm[sid].xParm.xInitializeParm.xConfig.pToneTable 	= Dsp_data[sid].m_xConfig.pToneTable;
	//pSendParm[sid].xParm.xInitializeParm.xConfig.pCtrlParm 		= Dsp_data[sid].m_xConfig.pCtrlParm;
	pSendParm[sid].xParm.xInitializeParm.xConfig.bVAD 			= Dsp_data[sid].m_xConfig.bVAD;
	//pSendParm[sid].xParm.xInitializeParm.xConfig.nVadLevel 		= Dsp_data[sid].m_xConfig.nVadLevel;
	//pSendParm[sid].xParm.xInitializeParm.xConfig.bAES 		= Dsp_data[sid].m_xConfig.bAES;
	//pSendParm[sid].xParm.xInitializeParm.xConfig.nHangoverTime 	= Dsp_data[sid].m_xConfig.nHangoverTime;
	//pSendParm[sid].xParm.xInitializeParm.xConfig.nBGNoiseLevel 	= Dsp_data[sid].m_xConfig.nBGNoiseLevel;
	//pSendParm[sid].xParm.xInitializeParm.xConfig.nAttenRange 	= Dsp_data[sid].m_xConfig.nAttenRange;
	//pSendParm[sid].xParm.xInitializeParm.xConfig.nTXGain 		= Dsp_data[sid].m_xConfig.nTXGain;
	//pSendParm[sid].xParm.xInitializeParm.xConfig.nRXGain 		= Dsp_data[sid].m_xConfig.nRXGain;
	//pSendParm[sid].xParm.xInitializeParm.xConfig.nTRRatio 		= Dsp_data[sid].m_xConfig.nTRRatio;
	//pSendParm[sid].xParm.xInitializeParm.xConfig.nRTRatio 		= Dsp_data[sid].m_xConfig.nRTRatio;
	//pSendParm[sid].xParm.xInitializeParm.xConfig.bPLC 		= Dsp_data[sid].m_xConfig.bPLC;
	pSendParm[sid].xParm.xInitializeParm.xConfig.nJitterDelay	= Dsp_data[sid].m_xConfig.nJitterDelay;
	pSendParm[sid].xParm.xInitializeParm.xConfig.nMaxDelay		= Dsp_data[sid].m_xConfig.nMaxDelay;
	pSendParm[sid].xParm.xInitializeParm.xConfig.nJitterFactor	= Dsp_data[sid].m_xConfig.nJitterFactor;
#else
	xInitializeParm.uTranCodingAlgorithm = uTranCodingAlgorithm;
	xInitializeParm.uRecvCodingAlgorithm = uRecvCodingAlgorithm;
	xInitializeParm.xConfig = *pConfig;
#endif

	//retval = SendCommand(sid);
	retval = DspcodecInitialize_R1( sid, &xInitializeParm );

	// init some data members for writing and reading
	JitterBuffer[sid].m_nPosDec = 0;
	PacketTxBuffer[sid].m_nPosEnc = 0;
	JitterBuffer[sid].m_nPreSeq = 0;
	JitterBuffer[sid].m_bSeqGet = false;
#ifndef CLEAN_JITTER_BUFFER_PARAMS
	Dsp_data[sid].m_nPreTick = 0;
	Dsp_data[sid].m_nPreTimeStamp = 0;
	Dsp_data[sid].m_nWState = DSPCODEC_WSTATE_NORMALJITTER;
#ifdef CONFIG_RTK_VOIP_G7231
	if((Dsp_data[sid].m_uRecvCodingAlgorithm == DSPCODEC_ALGORITHM_G7231A53) ||
		(Dsp_data[sid].m_uRecvCodingAlgorithm == DSPCODEC_ALGORITHM_G7231A63))
	{
		Dsp_data[sid].m_nInitTd = G723PRETD;

#ifndef SUPPORT_ADJUST_JITTER
		tx_jit_buf_high_threshold[sid] = 30 ;
		tx_jit_buf_low_threshold[sid] = 10 ;
#endif
	}
	else 
#endif /* CONFIG_RTK_VOIP_G7231 */
#ifdef CONFIG_RTK_VOIP_G729AB
	if(Dsp_data[sid].m_uRecvCodingAlgorithm == DSPCODEC_ALGORITHM_G729)
	{
		Dsp_data[sid].m_nInitTd = G729PRETD;

#ifndef SUPPORT_ADJUST_JITTER
		tx_jit_buf_high_threshold[sid] = 40 ;
		tx_jit_buf_low_threshold[sid] = 10 ;
#endif
	}
	else
#endif /* CONFIG_RTK_VOIP_G729AB */
	{
		Dsp_data[sid].m_nInitTd = G711PRETD;

#ifndef SUPPORT_ADJUST_JITTER
		tx_jit_buf_high_threshold[sid] = 40 ;
		tx_jit_buf_low_threshold[sid] = 10 ;
#endif
	}
	Dsp_data[sid].m_nPreTd = Dsp_data[sid].m_nInitTd;
	Dsp_data[sid].m_nAveTd = Dsp_data[sid].m_nInitTd;
	Dsp_data[sid].m_nTdOffset = 0;
	Dsp_data[sid].m_njSN = 0;
#endif /* !CLEAN_JITTER_BUFFER_PARAMS */
	JitterBuffer[sid].m_nFrameSent = 0;
	JitterBuffer[sid].m_nEarlyFrame = 0;
	JitterBuffer[sid].m_nLateFrame = 0;
	JitterBuffer[sid].m_nNotOwner = 0;
	//Dsp_data[sid].m_nDuplicate = 0;
	JitterBuffer[sid].m_nFrameRecv = 0;

	if(retval != DSPCODEC_SUCCESS)
	{
		PRINT_R("%s: !!! Send command fail !!!\n", __FUNCTION__);
	}
	return retval;
}

RESULT DspcodecSetConfig(uint32 sid/*, CDspcodecConfig* pConfig*/)
{
	RESULT retval;

#if 0
	assert(pConfig);

	memset((void *)(&(pSendParm[sid].xParm)), 0, sizeof(CDspcodecSetConfigParm));
	Dsp_data[sid].m_xConfig = *pConfig;
	//pSendParm[sid].uCommand = DSPCODEC_COMMAND_SETCONFIG;
	//pSendParm[sid].nOwner = 0;
	//pSendParm[sid].xParm.xSetConfigParm.xConfig.pToneTable 	= Dsp_data[sid].m_xConfig.pToneTable;
	//pSendParm[sid].xParm.xSetConfigParm.xConfig.pCtrlParm 	= Dsp_data[sid].m_xConfig.pCtrlParm;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.bVAD 	= Dsp_data[sid].m_xConfig.bVAD;
	//pSendParm[sid].xParm.xSetConfigParm.xConfig.nVadLevel 	= Dsp_data[sid].m_xConfig.nVadLevel;
/*	pSendParm[sid].xParm.xSetConfigParm.xConfig.bAES 	= Dsp_data[sid].m_xConfig.bAES;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nHangoverTime = Dsp_data[sid].m_xConfig.nHangoverTime;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nBGNoiseLevel = Dsp_data[sid].m_xConfig.nBGNoiseLevel;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nAttenRange = Dsp_data[sid].m_xConfig.nAttenRange;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nTXGain 	= Dsp_data[sid].m_xConfig.nTXGain;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nRXGain 	= Dsp_data[sid].m_xConfig.nRXGain;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nTRRatio 	= Dsp_data[sid].m_xConfig.nTRRatio;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nRTRatio 	= Dsp_data[sid].m_xConfig.nRTRatio;
*/	//pSendParm[sid].xParm.xSetConfigParm.xConfig.bPLC 	= Dsp_data[sid].m_xConfig.bPLC;
#endif

	//retval = SendCommand(sid);
	retval = DspcodecSetConfig_R1( sid );

	if(retval != DSPCODEC_SUCCESS)
	{
		PRINT_R("%s: !!! Send command fail !!!\n", __FUNCTION__);
	}
	return retval;
}

#if 0
RESULT DspcodecGetConfig(uint32 sid, CDspcodecConfig* pConfig)
{
	assert(pConfig);

	//pConfig->pToneTable		= Dsp_data[sid].m_xConfig.pToneTable;
	//pConfig->pCtrlParm 		= Dsp_data[sid].m_xConfig.pCtrlParm;
	pConfig->bVAD 			= Dsp_data[sid].m_xConfig.bVAD;
	//pConfig->nVadLevel 		= Dsp_data[sid].m_xConfig.nVadLevel;
/*	pConfig->bAES 			= Dsp_data[sid].m_xConfig.bAES;
	pConfig->nHangoverTime 		= Dsp_data[sid].m_xConfig.nHangoverTime;
	pConfig->nBGNoiseLevel 		= Dsp_data[sid].m_xConfig.nBGNoiseLevel;
	pConfig->nAttenRange 		= Dsp_data[sid].m_xConfig.nAttenRange;
	pConfig->nTXGain 		= Dsp_data[sid].m_xConfig.nTXGain;
	pConfig->nRXGain 		= Dsp_data[sid].m_xConfig.nRXGain;
	pConfig->nTRRatio 		= Dsp_data[sid].m_xConfig.nTRRatio;
	pConfig->nRTRatio 		= Dsp_data[sid].m_xConfig.nRTRatio;
*/	//pConfig->bPLC 			= Dsp_data[sid].m_xConfig.bPLC;
	return DSPCODEC_SUCCESS;
}
#endif

#if 0
DSPCODEC_ALGORITHM DspcodecGetAlgorithm(uint32 sid)
{
	/* DSPCODEC_ALGORITHM_UNKNOW can be used, only if open source part. */
	if(sid >= dsp_rtk_ss_num)
		return DSPCODEC_ALGORITHM_UNKNOW;

	return Dsp_data[sid].m_uCodingAlgorithm;
}
#endif

RESULT DspcodecStart(uint32 sid, DSPCODEC_ACTION uAction)
{
	RESULT retval;
	int i;
	CDspcodecStartParm xStartParm;

	if(uAction > DSPCODEC_ACTION_DECODE)
	{
		printk("Dspcodec: !!! Wrong parameter of uAction %d !!!\n", uAction);
		assert(uAction <= DSPCODEC_ACTION_DECODE);
		return DSPCODEC_ERROR_ACTION;
	}

	memset((void *)(&(xStartParm)), 0, sizeof(CDspcodecStartParm));
	//pSendParm[sid].uCommand = DSPCODEC_COMMAND_START;
	//pSendParm[sid].nOwner = 0;
	/*pSendParm[sid].xParm.*/xStartParm.uCodingAction = uAction;

	//retval = SendCommand(sid);
	retval = DspcodecStart_R1( sid, &xStartParm );

	// re-initialize local variables
	if(uAction == DSPCODEC_ACTION_DECODE)	// only write()
	{
		for(i=0; i<BUFFER_NUM_ENC; i++)
			PacketTxBuffer[sid].xEncOutput[i].nOwner = 0;
		//PacketTxBuffer[sid].nOwner = 0;
		JitterBuffer[sid].nLastPlaySeqNo = 0;

		PacketTxBuffer[sid].m_nPosEnc = 0;
		JitterBuffer[sid].m_nFrameRecv = 0;
	}
	else if(uAction == DSPCODEC_ACTION_ENCODE)	// only read()
	{
		for(i=0; i<BUFFER_NUM_DEC; i++)
		{
			JitterBuffer[sid].xDecBuffer[i].nOwner = 1;
			JitterBuffer[sid].xDecBuffer[i].nSeqNo = 0;
		}
		//pSendParm[sid].nOwner = 0;
		JitterBuffer[sid].nLastRecvSeqNo = 0;
		//pSendParm[sid].nFramtPut = 0;

		JitterBuffer[sid].m_nPosDec = 0;
		JitterBuffer[sid].m_nPreSeq = 0;
		JitterBuffer[sid].m_bSeqGet = false;
#ifndef CLEAN_JITTER_BUFFER_PARAMS
		Dsp_data[sid].m_nPreTick = 0;
		Dsp_data[sid].m_nPreTimeStamp = 0;
		Dsp_data[sid].m_nWState = DSPCODEC_WSTATE_NORMALJITTER;
#ifdef CONFIG_RTK_VOIP_G7231
		if((Dsp_data[sid].m_uCodingAlgorithm == DSPCODEC_ALGORITHM_G7231A53) ||
			(Dsp_data[sid].m_uCodingAlgorithm == DSPCODEC_ALGORITHM_G7231A63))
		{
			Dsp_data[sid].m_nInitTd = G723PRETD;
		} else 
#endif
#ifdef CONFIG_RTK_VOIP_G729AB
		if(Dsp_data[sid].m_uCodingAlgorithm == DSPCODEC_ALGORITHM_G729)
		{
			Dsp_data[sid].m_nInitTd = G729PRETD;
		} else
#endif
		{
			Dsp_data[sid].m_nInitTd = G711PRETD;
		}

		Dsp_data[sid].m_nPreTd = Dsp_data[sid].m_nInitTd;
		Dsp_data[sid].m_nAveTd = Dsp_data[sid].m_nInitTd;
		Dsp_data[sid].m_nTdOffset = 0;
		Dsp_data[sid].m_njSN = 0;
#endif /* !CLEAN_JITTER_BUFFER_PARAMS */
		JitterBuffer[sid].m_nFrameSent = 0;
		JitterBuffer[sid].m_nEarlyFrame = 0;
		JitterBuffer[sid].m_nLateFrame = 0;
		JitterBuffer[sid].m_nNotOwner = 0;
		//Dsp_data[sid].m_nDuplicate = 0;
	}

	return retval;
}

#ifndef CLEAN_JITTER_BUFFER_PARAMS
void reset_jitter_buffer(uint32 sid)
{
	int i ;

	// clear the owner bits of jitter buffer and receiving buffer
	for(i=0; i<BUFFER_NUM_DEC; i++)
	{
		pSendParm[sid].xDecBuffer[i].nOwner = 1;
		pSendParm[sid].xDecBuffer[i].nSeqNo = 0;
	}
	for(i=0; i<BUFFER_NUM_ENC; i++)
		PacketTxBuffer[sid].xEncOutput[i].nOwner = 0;
	pSendParm[sid].nOwner = 0;
	pSendParm[sid].nSeqNo = 0;
	pSendParm[sid].nFramtPut = 0;
	PacketTxBuffer[sid].nOwner = 0;
	PacketTxBuffer[sid].nSeqNo = 0;
}
#endif

RESULT DspcodecStop(uint32 sid)
{
	RESULT retval;

	//pSendParm[sid].uCommand = DSPCODEC_COMMAND_STOP;
	//pSendParm[sid].nOwner = 0;
	//retval = SendCommand(sid);
	retval = DspcodecStop_R1( sid );

	if(retval != DSPCODEC_SUCCESS)
	{
	}
	return retval;
}

#if 0
RESULT DspcodecMute(uint32 sid, DSPCODEC_MUTEDIRECTION uDirection)
{
	RESULT retval;

	if(uDirection > DSPCODEC_MUTEDIRECTION_NONE)
	{
		printk("Dspcodec: !!! Wrong parameter of uDirection %d !!!\n", uDirection);
		assert(uDirection <= DSPCODEC_MUTEDIRECTION_NONE);
		return DSPCODEC_ERROR_MUTEDIRECTION;
	}

	memset((void *)(&(pSendParm[sid].xParm)), 0, sizeof(CDspcodecMuteParm));
	pSendParm[sid].uCommand = DSPCODEC_COMMAND_MUTE;
	pSendParm[sid].nOwner = 0;
	pSendParm[sid].xParm.xMuteParm.uMuteDirection = uDirection;
	retval = SendCommand(sid);

	if(retval != DSPCODEC_SUCCESS)
	{
	}
	return retval;
}
#endif

RESULT DspcodecPlayTone(uint32 sid, DSPCODEC_TONE nTone, bool bFlag, DSPCODEC_TONEDIRECTION path)
{
	RESULT retval;
	uint8 value;
	extern void TonePhaseRevSet(int sid, int flag);
	CDspcodecPlayToneParm xPlayToneParm;
	
	if((nTone < DSPCODEC_TONE_0) || (nTone > DSPCODEC_TONE_KEY))
	{
		assert((nTone >= DSPCODEC_TONE_0) && (nTone <= DSPCODEC_TONE_KEY));
		return DSPCODEC_ERROR_TONE;
	}

	if(path > DSPCODEC_TONEDIRECTION_BOTH)
	{
		assert(path <= DSPCODEC_TONEDIRECTION_BOTH);
		return DSPCODEC_ERROR_PARAMETER;
	}

	TonePhaseRevSet(sid, 0);

	memset((void *)(&(xPlayToneParm)), 0, sizeof(CDspcodecPlayToneParm));
	//pSendParm[sid].uCommand = DSPCODEC_COMMAND_PLAYTONE;
	//pSendParm[sid].nOwner = 0;

	if((nTone >= DSPCODEC_TONE_0) && (nTone <= DSPCODEC_TONE_HASHSIGN))
	{
		/*pSendParm[sid].xParm.*/xPlayToneParm.nTone = nTone;
//#ifndef CONFIG_RTK_VOIP_IP_PHONE	// pkshih: remove these 'debug' code 
//		if( dtmf_mode[0]==2/*_inband*/ )	// hc$ note, must follow chid
//			path = DSPCODEC_TONEDIRECTION_BOTH ;	//WJF 931122 debug
//#endif
		// play DIGIT_0, DIGIT_1, DIGIT_2, DIGIT_3, DIGIT_4, DIGIT_5
		//      DIGIT_6, DIGIT_7, DIGIT_8, DIGIT_9, DIGIT_STAR, DIGIT_PONDA
	}
	else if ((nTone >= DSPCODEC_TONE_0_CONT) && (nTone <= DSPCODEC_TONE_D_CONT))
	{	// thlin+ continous DTMF tone play for RFC2833
		/*pSendParm[sid].xParm.*/xPlayToneParm.nTone = nTone-DSPCODEC_TONE_0_CONT+DIGIT_0_CONT;
		// play DIGIT_0_CONT, DIGIT_1_CONT, DIGIT_2_CONT, DIGIT_3_CONT, DIGIT_4_CONT, DIGIT_5_CONT,
		//      DIGIT_6_CONT, DIGIT_7_CONT, DIGIT_8_CONT, DIGIT_9_CONT, DIGIT_STAR_CONT, DIGIT_PONDA_CONT,
		//      DIGIT_A_CONT, DIGIT_B_CONT, DIGIT_C_CONT, DIGIT_D_CONT
	}
	else
	{
		switch(nTone)
		{
#ifdef SUPPORT_TONE_PROFILE

		case DSPCODEC_TONE_DIAL:
		case DSPCODEC_TONE_STUTTERDIAL:
		case DSPCODEC_TONE_MESSAGE_WAITING:
		case DSPCODEC_TONE_CONFIRMATION:
		case DSPCODEC_TONE_RINGING:
		case DSPCODEC_TONE_BUSY:
		case DSPCODEC_TONE_CONGESTION:
		case DSPCODEC_TONE_ROH:
		case DSPCODEC_TONE_DOUBLE_RING:
		case DSPCODEC_TONE_SIT_NOCIRCUIT:
		case DSPCODEC_TONE_SIT_INTERCEPT:
		case DSPCODEC_TONE_SIT_VACANT:
		case DSPCODEC_TONE_SIT_REORDER:
		case DSPCODEC_TONE_CALLING_CARD_WITHEVENT:
		case DSPCODEC_TONE_CALLING_CARD:
		case DSPCODEC_TONE_CALL_WAITING:
		case DSPCODEC_TONE_CALL_WAITING_2:
		case DSPCODEC_TONE_CALL_WAITING_3:
		case DSPCODEC_TONE_CALL_WAITING_4:
		case DSPCODEC_TONE_EXTEND_1:
		case DSPCODEC_TONE_EXTEND_2:
		case DSPCODEC_TONE_EXTEND_3:
		case DSPCODEC_TONE_EXTEND_4:
		case DSPCODEC_TONE_EXTEND_5:
		case DSPCODEC_TONE_INGRESS_RINGBACK:
			/*pSendParm[sid].xParm.*/xPlayToneParm.nTone = Dsp_data[sid].m_nBaseTone + nTone - DSPCODEC_TONE_DIAL;
			// play country DIAL, STUTTERDIAL, MESSAGE_WAITING, CONFIRMATION, RING, BUSY, CONGESTION, ROH, DOUBLE_RING,
			//              SIT_NOCIRCUIT, SIT_INTERCEPT, SIT_VACANT, SIT_REORDER, CALLING_CARD_WITHEVENT, CALLING_CARD, 
			//              CALL_WAITING_1, CALL_WAITING_2, CALL_WAITING_3, CALL_WAITING_4, EXTEND_1, EXTEND_2, EXTEND_3, 
			//              EXTEND_4, EXTEND_5, INGRESS_RINGBACK,
			break;
		case DSPCODEC_TONE_HOLD:
			/*pSendParm[sid].xParm.*/xPlayToneParm.nTone = HOLDING;
			break;
		case DSPCODEC_TONE_OFFHOOKWARNING:
			/*pSendParm[sid].xParm.*/xPlayToneParm.nTone = OFFHOOKWARING;
			break;
		case DSPCODEC_TONE_RING:
			/*pSendParm[sid].xParm.*/xPlayToneParm.nTone = RING_1 + Dsp_data[sid].m_nRingType;
			break;
#else
		case DSPCODEC_TONE_DIAL:
			pSendParm[sid].xParm.xPlayToneParm.nTone = Dsp_data[sid].m_nDialTone;
			break;
		case DSPCODEC_TONE_RINGING:
			pSendParm[sid].xParm.xPlayToneParm.nTone = Dsp_data[sid].m_nRingTone;
			break;
		case DSPCODEC_TONE_BUSY:
			pSendParm[sid].xParm.xPlayToneParm.nTone = Dsp_data[sid].m_nBusyTone;
			break;
		case DSPCODEC_TONE_HOLD:
			pSendParm[sid].xParm.xPlayToneParm.nTone = HOLDING;
			break;
		case DSPCODEC_TONE_CALL_WAITING:
			pSendParm[sid].xParm.xPlayToneParm.nTone = Dsp_data[sid].m_nWaitingTone;
			break;
		case DSPCODEC_TONE_OFFHOOKWARNING:
			pSendParm[sid].xParm.xPlayToneParm.nTone = OFFHOOKWARING;
			break;
		case DSPCODEC_TONE_RING:
			pSendParm[sid].xParm.xPlayToneParm.nTone = RING_1 + Dsp_data[sid].m_nRingType;
			break;
#endif	// #ifdef SUPPORT_TONE_PROFILE
		case DSPCODEC_TONE_RING1:
		case DSPCODEC_TONE_RING2:
		case DSPCODEC_TONE_RING3:
		case DSPCODEC_TONE_RING4:
		case DSPCODEC_TONE_RING5:
		case DSPCODEC_TONE_RING6:
		case DSPCODEC_TONE_RING7:
		case DSPCODEC_TONE_RING8:
		case DSPCODEC_TONE_RING9:
		case DSPCODEC_TONE_RING10:
			/*pSendParm[sid].xParm.*/xPlayToneParm.nTone = RING_1 + (nTone-DSPCODEC_TONE_RING1);
			// play RING_1, RING_2, RING_3, RING_4, RING_5,
			//      RING_6, RING_7, RING_8, RING_9, RING_10,
			break;

		case DSPCODEC_TONE_CUSTOM_TONE1:
		case DSPCODEC_TONE_CUSTOM_TONE2:
		case DSPCODEC_TONE_CUSTOM_TONE3:
		case DSPCODEC_TONE_CUSTOM_TONE4:
		case DSPCODEC_TONE_CUSTOM_TONE5:
		case DSPCODEC_TONE_CUSTOM_TONE6:
		case DSPCODEC_TONE_CUSTOM_TONE7:
		case DSPCODEC_TONE_CUSTOM_TONE8:
			/*pSendParm[sid].xParm.*/xPlayToneParm.nTone = CUSTOM_TONE1 + (nTone-DSPCODEC_TONE_CUSTOM_TONE1);
			// play CUSTOM_TONE1, CUSTOM_TONE2, CUSTOM_TONE3, CUSTOM_TONE4, 
			//      CUSTOM_TONE5, CUSTOM_TONE6, CUSTOM_TONE7, CUSTOM_TONE8, 
			break;

#if 1 //def SW_DTMF_CID	// hc+ 1124 for DTMF CID, always enable
		case DSPCODEC_TONE_A:
		case DSPCODEC_TONE_B:
		case DSPCODEC_TONE_C:
		case DSPCODEC_TONE_D:		
			/*pSendParm[sid].xParm.*/xPlayToneParm.nTone = DIGIT_A + (nTone-DSPCODEC_TONE_A);	
			// play DIGIT_A, DIGIT_B , DIGIT_C, DIGIT_D
			break;
#endif

		case DSPCODEC_TONE_FSK_SAS:		// sandro+ 2006/07/24 for call waiting tone
			/*pSendParm[sid].xParm.*/xPlayToneParm.nTone = FSK_SAS;
			break;
			
		case DSPCODEC_TONE_FSK_ALERT:	// hc+ 1229 for off hook FSK CID
			/*pSendParm[sid].xParm.*/xPlayToneParm.nTone = FSK_ALERT;
			break;

		case DSPCODEC_TONE_FSK_MUTE:	// jwsyu+ 20111208 for off hook FSK CID mute voice
			/*pSendParm[sid].xParm.*/xPlayToneParm.nTone = FSK_MUTE;
			break;

		case DSPCODEC_TONE_NTT_IIT_TONE:
			/*pSendParm[sid].xParm.*/xPlayToneParm.nTone = NTT_IIT_TONE;
			//printk("ntt_tone, %d\n",NTT_IIT_TONE);
			break;
		
		case DSPCODEC_TONE_VBD_ANS:
			/*pSendParm[sid].xParm.*/xPlayToneParm.nTone = VBD_ANS;
			break;
		
		case DSPCODEC_TONE_VBD_ANSBAR:
			/*pSendParm[sid].xParm.*/xPlayToneParm.nTone = VBD_ANS_BAR;
			TonePhaseRevSet(sid, 1);
			break;
		
		case DSPCODEC_TONE_VBD_ANSAM:
			/*pSendParm[sid].xParm.*/xPlayToneParm.nTone = VBD_ANSAM;
			break;
		
		case DSPCODEC_TONE_VBD_ANSAMBAR:
			/*pSendParm[sid].xParm.*/xPlayToneParm.nTone = VBD_ANSAM_BAR;
			TonePhaseRevSet(sid, 1);
			break;
		
		case DSPCODEC_TONE_VBD_CNG:
			/*pSendParm[sid].xParm.*/xPlayToneParm.nTone = VBD_CNG;
			break;

		case DSPCODEC_TONE_VBD_CRE:
			/*pSendParm[sid].xParm.*/xPlayToneParm.nTone = VBD_CRE;
			break;
		
		case DSPCODEC_TONE_USER_DEFINE1:
		case DSPCODEC_TONE_USER_DEFINE2:
		case DSPCODEC_TONE_USER_DEFINE3:
		case DSPCODEC_TONE_USER_DEFINE4:
		case DSPCODEC_TONE_USER_DEFINE5:
			/*pSendParm[sid].xParm.*/xPlayToneParm.nTone = USER_DEFINE1 + (nTone-DSPCODEC_TONE_USER_DEFINE1);
			break;

		case DSPCODEC_TONE_KEY:
			/*pSendParm[sid].xParm.*/xPlayToneParm.nTone = DIGIT_PONDA;
			break;

		default:
			/* should never go here */
			assert(0);
		}
	}
	/*pSendParm[sid].xParm.*/xPlayToneParm.bFlag = bFlag;
	/*pSendParm[sid].xParm.*/xPlayToneParm.uToneDirection = path;

	//retval = SendCommand(sid);
	retval = DspcodecPlayTone_R1( sid, &xPlayToneParm );

	if(retval != DSPCODEC_SUCCESS)
	{
		printk("Playtone command not successful\n");
	}
	value = value;	// remove compiling warning
	return retval;
}

//WJF 930806 added, when reached tx_jit_buf_high_threshold, touch_high_threshold will be 1, 
// touch_high_threshold will be 0 when down to tx_jit_buf_low_threshold
//int tx_jit_buf_low_threshold=10, tx_jit_buf_high_threshold=30, touch_high_threshold=0 ;
///static char local_buf[640] ;
//WJF 930806, return 1 if tx jitter buffer over loaded, DSP not fast enough to play the voice
int check_if_jitter_buffer_over_loaded(uint32 sid, int k, int r, int tx_jitter_buffer_load)
{
#ifndef RESERVE_SPACE_FOR_SLOW
	int i, n;
	
	tx_jitter_buffer_load=0;
	n = (k+1) % BUFFER_NUM_DEC ;	//next position

	if( pSendParm[sid].xDecBuffer[k].nOwner == 1 )
	{
		touch_high_threshold[sid] = 0 ;
		return touch_high_threshold[sid] ;
	}

	k = (n+BUFFER_NUM_DEC-tx_jit_buf_high_threshold[sid]) % BUFFER_NUM_DEC ;	//previous 30 position
	for(i=0; i<tx_jit_buf_high_threshold[sid]; i++)
	{
		if( pSendParm[sid].xDecBuffer[k].nOwner == 0 )
			++tx_jitter_buffer_load ;
		k = (k+1) % BUFFER_NUM_DEC ;	//previous 30 + next i position
	}
#endif	// !RESERVE_SPACE_FOR_SLOW

	if( touch_high_threshold[sid] )
	{
		if( tx_jitter_buffer_load < tx_jit_buf_low_threshold[sid] )
			touch_high_threshold[sid] = 0 ;
	}
	else if( tx_jitter_buffer_load >= tx_jit_buf_high_threshold[sid] )
	{
		touch_high_threshold[sid] = 1 ;
	}

	return touch_high_threshold[sid];
}

#ifdef SUPPORT_SYNC_OUT_OF_SEQ
static void SyncDynamicPayload( uint32 sid, uint16 seq )
{
	DPNotFirstPacket[ sid ] = 0;
	DPSyncInSeq[ sid ] = seq;
	DPEnableSyncInSeq[ sid ] = 1;
	
	printk( "Synchronize to %d after 30 out-of-seq(%d).\n", seq, sid );
}
#endif /* SUPPORT_SYNC_OUT_OF_SEQ */

//#define debug_in_console
//WJF 930806 notice :
// The nSeq written into the xDecBuffer[] better be in sequence
//  otherwise, the DSP could induce some delay, even stop to play and no voice heard
///static uint32 nSeqPrev=0;
int32 DspcodecWrite(uint32 chid, uint32 sid, uint8* pBuf, int32 nSize, uint32 nSeq, uint32 nTimestamp, RtpFramesType DecFrameType )
{
	extern Word16 jBufPtr[];
	uint32 jbuf_ri;
	int jbuf_load;

	uint32 n = 0, retval = 0;
#ifndef CLEAN_JITTER_BUFFER_PARAMS
	uint32 nowTick;
	int32  diffTimeStamp, Td = 0;
#endif
	
#ifdef RESERVE_SPACE_FOR_SLOW
	uint16 seqDelta, seqRes;
	uint16 i;
	uint16 seq;
#endif

#if 0
	if( sid == 0 )
		printk( "%d@%d ", nSeq & 0x3F, nTimestamp & 0xFFF ); 
#endif

	if((pBuf == NULL) || (nSize == 0))
		return 0;


	if(!JitterBuffer[sid].m_bSeqGet)		// the very first frame
	{
		if( !JitterBuffer[sid].bPutNew )
			return 0;	/* wait for jbc ready */
	
		JitterBuffer[sid].m_nFrameSent++;
		JitterBuffer[sid].nLastRecvSeqNo = nSeq;
		//pSendParm[sid].nFramtPut = Dsp_data[sid].m_nFrameSent;
		JitterBuffer[sid].xDecBuffer[0].nSeqNo = nSeq;
		JitterBuffer[sid].xDecBuffer[0].nTimeStamp = nTimestamp;
		JitterBuffer[sid].xDecBuffer[0].nSize = nSize;
#ifndef CLEAN_JITTER_BUFFER_PARAMS
		pSendParm[sid].xDecBuffer[0].nTd = Dsp_data[sid].m_nInitTd;
#endif
		memcpy((void *)JitterBuffer[sid].xDecBuffer[0].pBuffer, pBuf, nSize);
		JitterBuffer[sid].xDecBuffer[0].nDecFrameType = DecFrameType;
		JitterBuffer[sid].xDecBuffer[0].nArrive = timetick;
		JitterBuffer[sid].xDecBuffer[0].nOwner = 0;
#ifdef NEW_JITTER_BUFFER_WI_DESIGN
		JitterBuffer[sid].m_nPosDec = 1;
#endif
		retval = nSize;
		JitterBuffer[sid].m_nPreSeq = nSeq;
		JitterBuffer[sid].m_bSeqGet = true;
#ifndef CLEAN_JITTER_BUFFER_PARAMS
		Dsp_data[sid].m_nPreTick = PacketTxBuffer[sid].nRunTick;
		Dsp_data[sid].m_nPreTimeStamp = nTimestamp;
		Dsp_data[sid].m_nAveTd = (32703 * (short)(Dsp_data[sid].m_nAveTd / 128) + 65 * (short)(Dsp_data[sid].m_nInitTd / 128)) >> 8;
#endif
	}
#ifdef RESERVE_SPACE_FOR_SLOW
	else if( RtpSeqGreater(nSeq, JitterBuffer[sid].m_nPreSeq) )		// normal frames
#else
	else if( RtpSeqGreater(nSeq, Dsp_data[sid].m_nPreSeq) || (outof_seq_cnt[sid]>30) )		// normal frames
#endif
	{

#ifdef RESERVE_SPACE_FOR_SLOW
		jbuf_ri = jBufPtr[ sid ];
		
		// k is written index; r index is not read yet
		if( JitterBuffer[sid].m_nPosDec == jbuf_ri ) {
  #ifdef NEW_JITTER_BUFFER_WI_DESIGN
  			jbuf_load = 0;	// init state.
  #else
			if( JitterBuffer[sid].xDecBuffer[jbuf_ri].nOwner == 1 )
				jbuf_load = 0;	// init state.
			else
				jbuf_load = 1;
  #endif /* NEW_JITTER_BUFFER_WI_DESIGN */
  #ifndef NEW_JITTER_BUFFER_WI_DESIGN
		} else if( ( ( Dsp_data[sid].m_nPosDec + 1 ) % BUFFER_NUM_DEC ) == jbuf_ri )
		{	// out of buffer 
			// In our design, buffer will not be full.
			jbuf_load = 0;
  #endif /* !NEW_JITTER_BUFFER_WI_DESIGN */
		} else if( jbuf_ri > JitterBuffer[sid].m_nPosDec ) {
  #ifdef NEW_JITTER_BUFFER_WI_DESIGN
			jbuf_load = JitterBuffer[sid].m_nPosDec + BUFFER_NUM_DEC - jbuf_ri;
  #else
			jbuf_load = Dsp_data[sid].m_nPosDec + 1 + BUFFER_NUM_DEC - jbuf_ri;
  #endif 
		} else {
  #ifdef NEW_JITTER_BUFFER_WI_DESIGN
  			jbuf_load = JitterBuffer[sid].m_nPosDec - jbuf_ri;
  #else
			jbuf_load = Dsp_data[sid].m_nPosDec + 1 - jbuf_ri;
  #endif 
		}
#else
		jbuf_load = 0;	// unused if RESERVE_SPACE_FOR_SLOW not defined
#endif //RESERVE_SPACE_FOR_SLOW
		
		//WJF 930806 added, discard the packet if this jitter buffer is over loaded
		if( check_if_jitter_buffer_over_loaded(sid, JitterBuffer[sid].m_nPosDec, jbuf_ri, jbuf_load) )
		{
			if( ++over_load_cnt[sid] > 30 )	//WJF 931106 needed to do, reset buffer if over loaded too long
			{
			}	
			
			switch( over_load_cnt[sid] ) {
			case 1:
				debug_error("D%d ", sid);
				break;
			case 10:
				debug_error("Da ");
				break;
			}
			
			JitterBuffer[sid].m_nPreSeq = nSeq;
			JitterBuffer[sid].m_nLateFrame++;
		}
		else if( JitterBuffer[sid].bPutNew )
		{			
			over_load_cnt[sid] = 0 ;
#ifdef NEW_JITTER_BUFFER_WI_DESIGN
			n = JitterBuffer[sid].m_nPosDec;
#else
			n = (Dsp_data[sid].m_nPosDec+1)%BUFFER_NUM_DEC ;
#endif

#ifdef RESERVE_SPACE_FOR_SLOW
			seqDelta = ( uint16 )( nSeq - JitterBuffer[sid].m_nPreSeq - 1 );
			
			if( outof_seq_cnt[sid] > 30 ) {
				// too many out-of-seq, don't fill SKIP
			} else if( seqDelta ) {
				/* 
				 * Maximum reserved space is high threshold -5, because
				 * it receives many frames per packet.
				 */
				if( tx_jit_buf_high_threshold[ sid ] - 5 <= jbuf_load )
					seqRes = 0;
				else if( tx_jit_buf_high_threshold[ sid ] - 5 <= jbuf_load + seqDelta ) {
					seqRes = tx_jit_buf_high_threshold[ sid ] - 5 - jbuf_load - 1;	// 1 for arrive packet
				} else
					seqRes = seqDelta;

				seq = nSeq - seqRes;
					
				for( i = 0; i < seqRes; i ++ ) {
					// fill seq# to reserved space only
					JitterBuffer[sid].xDecBuffer[n].nSeqNo = seq ++;
					JitterBuffer[sid].xDecBuffer[n].nTimeStamp = 0;
					JitterBuffer[sid].xDecBuffer[n].nSize = 0;
  #ifndef CLEAN_JITTER_BUFFER_PARAMS
					JitterBuffer[sid].xDecBuffer[n].nTd = 0;
  #endif
					JitterBuffer[sid].xDecBuffer[n].nOwner = R0_RESERVE; // 2
				
					n = (n+1)%BUFFER_NUM_DEC;
				}
			}
#endif // RESERVE_SPACE_FOR_SLOW

			JitterBuffer[sid].m_nPreSeq = nSeq;
#ifndef NEW_JITTER_BUFFER_WI_DESIGN
			Dsp_data[sid].m_nPosDec = n;
#endif

#ifndef CLEAN_JITTER_BUFFER_PARAMS
			nowTick = PacketTxBuffer[sid].nRunTick;
			diffTimeStamp = (nTimestamp > Dsp_data[sid].m_nPreTimeStamp) ? (nTimestamp - Dsp_data[sid].m_nPreTimeStamp) :
				(0xffffffff - Dsp_data[sid].m_nPreTimeStamp + nTimestamp + 1);
#endif
#ifndef CLEAN_JITTER_BUFFER_PARAMS
			Td = (int32)(nowTick - Dsp_data[sid].m_nPreTick) - diffTimeStamp + Dsp_data[sid].m_nPreTd;
			//WJF 931121 added, check Td to avoid unreasonable situation
			if( (Td > (Dsp_data[sid].m_nInitTd+16000)) || (Td < (Dsp_data[sid].m_nInitTd-16000)) )
				Td = Dsp_data[sid].m_nInitTd;
#endif
			if(JitterBuffer[sid].xDecBuffer[n].nOwner == 1)
				JitterBuffer[sid].m_nFrameSent++;
			else
				JitterBuffer[sid].m_nNotOwner++;

			JitterBuffer[sid].nLastRecvSeqNo = nSeq;
			//pSendParm[sid].nFramtPut = Dsp_data[sid].m_nFrameSent;
			JitterBuffer[sid].xDecBuffer[n].nSeqNo = nSeq;
			JitterBuffer[sid].xDecBuffer[n].nTimeStamp = nTimestamp;
			JitterBuffer[sid].xDecBuffer[n].nSize = nSize;

#ifndef CLEAN_JITTER_BUFFER_PARAMS
			if((Dsp_data[sid].m_nWState == DSPCODEC_WSTATE_SYNCJITTER) && RtpSeqGreater(Dsp_data[sid].m_njSN, nSeq))
				JitterBuffer[sid].xDecBuffer[n].nTd = Td + Dsp_data[sid].m_nTdOffset;
			else
				JitterBuffer[sid].xDecBuffer[n].nTd = Td;
#endif

			memcpy((void *)JitterBuffer[sid].xDecBuffer[n].pBuffer, pBuf, nSize);
			JitterBuffer[sid].xDecBuffer[n].nDecFrameType = DecFrameType;
			JitterBuffer[sid].xDecBuffer[n].nArrive = timetick;
			JitterBuffer[sid].xDecBuffer[n].nOwner = 0;
			retval = nSize;
#ifndef CLEAN_JITTER_BUFFER_PARAMS
			Dsp_data[sid].m_nPreTick = nowTick;
			Dsp_data[sid].m_nPreTimeStamp = nTimestamp;
			Dsp_data[sid].m_nPreTd = Td;
#endif

#ifdef NEW_JITTER_BUFFER_WI_DESIGN
			JitterBuffer[sid].m_nPosDec = (n+1)%BUFFER_NUM_DEC;
#endif

#ifndef CLEAN_JITTER_BUFFER_PARAMS
			if(Dsp_data[sid].m_nWState == DSPCODEC_WSTATE_NORMALJITTER)
			{
				Dsp_data[sid].m_nAveTd = (32703 * (short)(Dsp_data[sid].m_nAveTd / 128) + 65 * (short)(Td / 128)) >> 8;
				if((Dsp_data[sid].m_nAveTd > (TDOFFSET_PO + Dsp_data[sid].m_nInitTd)) ||
					(Dsp_data[sid].m_nAveTd < (TDOFFSET_NE + Dsp_data[sid].m_nInitTd)))
				{
					Dsp_data[sid].m_nTdOffset = (Dsp_data[sid].m_nAveTd > (TDOFFSET_PO + Dsp_data[sid].m_nInitTd))?
   						TDOFFSET_PO : TDOFFSET_NE;
					Dsp_data[sid].m_njSN = nSeq;
					SyncJitterBuf(sid, Dsp_data[sid].m_njSN, Dsp_data[sid].m_nTdOffset);
					Dsp_data[sid].m_nPreTd -= Dsp_data[sid].m_nTdOffset;
					Dsp_data[sid].m_nWState = DSPCODEC_WSTATE_SYNCJITTER;
				}
			}
			else
			{
				if(RtpSeqGreater(PacketTxBuffer[sid].nSeqNo, Dsp_data[sid].m_njSN))
				{
					Dsp_data[sid].m_nAveTd -= Dsp_data[sid].m_nTdOffset;
					Dsp_data[sid].m_nTdOffset = 0;
					Dsp_data[sid].m_nWState = DSPCODEC_WSTATE_NORMALJITTER;
				}
			}
#endif /* !CLEAN_JITTER_BUFFER_PARAMS */
		}
		else
		{
			JitterBuffer[sid].m_nEarlyFrame++;
		}

		outof_seq_cnt[sid] = 0;
	}
	else	// out of sequence packet, recover sequence if 30 continuous outofseq happenned
	{
#ifdef RESERVE_SPACE_FOR_SLOW
		bool bReservedSpaceFound = false;
		
		jbuf_ri = jBufPtr[ sid ];

		n = JitterBuffer[sid].m_nPosDec;

  #ifdef NEW_JITTER_BUFFER_WI_DESIGN
		if( n != jbuf_ri ) 
  #else			
		if( ( ( n + 1 ) % BUFFER_NUM_DEC ) != jbuf_ri ) 
  #endif 
		{
			// If not empty, search for reserved space
#ifndef CLEAN_JITTER_BUFFER_PARAMS
			nowTick = PacketTxBuffer[sid].nRunTick;
#endif
			
			for( i = 0; i < tx_jit_buf_high_threshold[ sid ]; i ++ ) {

  #ifdef NEW_JITTER_BUFFER_WI_DESIGN
				n = (n+BUFFER_NUM_DEC-1)%BUFFER_NUM_DEC;	// n --;
  #endif
				
				if( JitterBuffer[sid].xDecBuffer[n].nSeqNo == nSeq ) {
					if( JitterBuffer[sid].xDecBuffer[n].nOwner == 0 ) {
						// special to deal with RTP redunant 
						bReservedSpaceFound = true;
						break;
					}

					if( JitterBuffer[sid].xDecBuffer[n].nOwner == R0_RESERVE ) {
											
						// fill reserved space
						JitterBuffer[sid].xDecBuffer[n].nSeqNo = nSeq;
						JitterBuffer[sid].xDecBuffer[n].nTimeStamp = nTimestamp;
						JitterBuffer[sid].xDecBuffer[n].nSize = nSize;

  #ifndef CLEAN_JITTER_BUFFER_PARAMS			
						if((Dsp_data[sid].m_nWState == DSPCODEC_WSTATE_SYNCJITTER) && RtpSeqGreater(Dsp_data[sid].m_njSN, nSeq))
							JitterBuffer[sid].xDecBuffer[n].nTd = Td + Dsp_data[sid].m_nTdOffset;
						else
							JitterBuffer[sid].xDecBuffer[n].nTd = Td;
  #endif
						
						memcpy((void *)JitterBuffer[sid].xDecBuffer[n].pBuffer, pBuf, nSize);
						JitterBuffer[sid].xDecBuffer[n].nDecFrameType = DecFrameType;
						JitterBuffer[sid].xDecBuffer[n].nArrive = timetick;
						JitterBuffer[sid].xDecBuffer[n].nOwner = 0;
						retval = nSize;
  #ifndef CLEAN_JITTER_BUFFER_PARAMS
						Dsp_data[sid].m_nPreTick = nowTick;
						Dsp_data[sid].m_nPreTimeStamp = nTimestamp;
						Dsp_data[sid].m_nPreTd = Td;						
  #endif
					
						outof_seq_cnt[sid] = 0;
						bReservedSpaceFound = true;
						break;
					}
				} 
	
				if( jbuf_ri == n )	// meet read index
					break;
				
  #ifndef NEW_JITTER_BUFFER_WI_DESIGN
				n = (n+BUFFER_NUM_DEC-1)%BUFFER_NUM_DEC;	// n --;
  #endif
			}
		}
		
		if( !bReservedSpaceFound )
#endif // RESERVE_SPACE_FOR_SLOW
		{
			//debug_warning("%d:OOSD(%d,%d) ", sid, Dsp_data[sid].m_nPreSeq, nSeq);
			debug_warning("OSD%d ", sid);

#ifdef SUPPORT_SYNC_OUT_OF_SEQ			
			if( ( uint16 )( outof_seq_prevSeq[ sid ] + 1 ) == ( uint16 )nSeq ) {
				if( ++ outof_seq_cnt[ sid ] > 30 ) {
					SyncDynamicPayload( sid, JitterBuffer[sid].m_nPreSeq );
					outof_seq_cnt[ sid ] = 0;
				}
			} else
				outof_seq_cnt[ sid ] = 0;
			
			outof_seq_prevSeq[ sid ] = nSeq;
#else
			++outof_seq_cnt[sid] ;
#endif /* SUPPORT_SYNC_OUT_OF_SEQ */
			
			JitterBuffer[sid].m_nLateFrame++;
		}
	}

	return retval;
}

int32 DspcodecRead(uint32 chid, uint32 sid, uint8* pBuf, int32 nSize)
{
	int length = 0;
	uint32 ri;

	if((pBuf == NULL) || (nSize == 0))
		return 0;
	
	ri = PacketTxBuffer[sid].m_nPosEnc;
	
	if(PacketTxBuffer[sid].xEncOutput[ ri ].nOwner == 1)
	{
		if(PacketTxBuffer[sid].xEncOutput[ ri ].nSize == 0)
			length = -1;	// invalid length to distinguish that it is silence
		else
		{
			length = (nSize >= PacketTxBuffer[sid].xEncOutput[ ri ].nSize)?
				PacketTxBuffer[sid].xEncOutput[ ri ].nSize : nSize;

			memcpy(pBuf, (void *)PacketTxBuffer[sid].xEncOutput[ ri ].pBuffer, length);
			JitterBuffer[sid].m_nFrameRecv++;
		}
		PacketTxBuffer[sid].xEncOutput[ ri ].nOwner = 0;
		
		// increase ri 
		ri ++;
		if( ri == BUFFER_NUM_ENC)
			 ri = 0;
		
		PacketTxBuffer[sid].m_nPosEnc = ri;
	}

	return length;
}

#if 0
RESULT DspcodecSetVolume(int32 nVal)
{
#ifdef __ECOS
	uint8 vol, value;
	if((nVal < 0) || (nVal > 10))
	{
		printk("Dspcodec: !!! Wrong parameter of nVal %d !!!\n", nVal);
		assert((nVal >= 0) && (nVal <= 10));
		return DSPCODEC_ERROR_PARAMETER;
	}
	vol = RxGainMap[nVal];
	value = CodecRead(0x02);
	value = (value & 0xf0) | vol;
	CodecWrChk(0x02, value);
//	printf("Dspcodec: Set volume gain: [%ddB]\n", (int)vol*2-28);
#endif
	return DSPCODEC_SUCCESS;
}
#endif

#if 0
RESULT DspcodecSetEncGain(int32 nVal)
{
#ifdef __ECOS
	uint8 vol, value;
	if((nVal < 0) || (nVal > 10))
	{
		assert((nVal >= 0) && (nVal <= 10));
		return DSPCODEC_ERROR_PARAMETER;
	}
	vol = TxGainMap[nVal];
#ifdef NEW_CODEC_TX_GAIN
	CodecWrChk(0x03, vol);
#else
	value = CodecRead(0x03);
	value = (value & 0xf0) | vol;
	CodecWrChk(0x03, value);
#endif
#endif
	return DSPCODEC_SUCCESS;
}
#endif

#if 0
RESULT DspcodecSetSidetoneGain(int32 nVal)
{
#ifdef __ECOS
	uint8 vol, value;
	if((nVal < 0) || (nVal > 10))
	{
		assert((nVal >= 0) && (nVal <= 10));
		return DSPCODEC_ERROR_PARAMETER;
	}
	vol = STGainMap[nVal];
	value = CodecRead(0x02);
	value = (value & 0x0f) | (vol << 4);
	CodecWrChk(0x02, value);
#endif
	return DSPCODEC_SUCCESS;
}
#endif

#if 0
RESULT DspcodecSidetoneSwitch(bool bFlag)
{
#ifdef __ECOS
	uint8 value;

	Dsp_data.m_bSidetoneOn = bFlag;
	if(bFlag)
	{
		if(Dsp_data.m_nMediaOn)
		{
			// turn on sidetone
			value = CodecRead(0x00);
			value = value & 0xef;
			CodecWrChk(0x00, value);
		}
	}
	else
	{
		if(Dsp_data.m_nMediaOn)
		{
			// turn off sidetone
			value = CodecRead(0x00);
			value = value | 0x10;
			CodecWrChk(0x00, value);
		}
	}
#endif
	return DSPCODEC_SUCCESS;
}
#endif

#if 0
RESULT DspcodecSetRingType(uint32 sid, int32 nRing)
{
	if((nRing < 0) || (nRing > 9))
	{
//		printk("Dspcodec: !!! Wrong parameter of nRing %d !!!\n", nRing);
		assert((nRing >= 0) && (nRing <= 9));
		return DSPCODEC_ERROR_PARAMETER;
	}

	Dsp_data[sid].m_nRingType = nRing;
	return DSPCODEC_SUCCESS;
}
#endif
static int tone_country;
RESULT DspcodecSetCountry(uint32 sid, int32 nCountry)
{
	int k;
#ifdef SUPPORT_TONE_PROFILE
	int pre_country_tone;

	pre_country_tone = Dsp_data[sid].m_nBaseTone;
#else
	int pre_m_nDialTone;
	int pre_m_nRingTone;
	int pre_m_nBusyTone;
	int pre_m_nWaitingTone;

	pre_m_nDialTone = Dsp_data[sid].m_nDialTone;
	pre_m_nRingTone = Dsp_data[sid].m_nRingTone;
	pre_m_nBusyTone = Dsp_data[sid].m_nBusyTone;
	pre_m_nWaitingTone = Dsp_data[sid].m_nWaitingTone;
#endif //#ifdef SUPPORT_TONE_PROFILE
	tone_country=nCountry;
	switch(nCountry)
	{
#ifdef SUPPORT_TONE_PROFILE
		case DSPCODEC_COUNTRY_USA:
			Dsp_data[sid].m_nBaseTone = USA_DIAL;
			break;
		case DSPCODEC_COUNTRY_UK:
			Dsp_data[sid].m_nBaseTone = UK_DIAL;
			break;
		case DSPCODEC_COUNTRY_AUSTRALIA:
			Dsp_data[sid].m_nBaseTone = AUSTRALIA_DIAL;
			break;
		case DSPCODEC_COUNTRY_HK:
			Dsp_data[sid].m_nBaseTone = HK_DIAL;
			break;
		case DSPCODEC_COUNTRY_JP:
			Dsp_data[sid].m_nBaseTone = JP_DIAL;
			break;
		case DSPCODEC_COUNTRY_SE:
			Dsp_data[sid].m_nBaseTone = SE_DIAL;
			break;
		case DSPCODEC_COUNTRY_GR:
			Dsp_data[sid].m_nBaseTone = GR_DIAL;
			break;
		case DSPCODEC_COUNTRY_FR:
			Dsp_data[sid].m_nBaseTone = FR_DIAL;
			break;
#if 0
		case DSPCODEC_COUNTRY_TR:
			Dsp_data[sid].m_nBaseTone = TR_DIAL;
			break;
#else
		case DSPCODEC_COUNTRY_TW:
			Dsp_data[sid].m_nBaseTone = TW_DIAL;
			break;
#endif

		case DSPCODEC_COUNTRY_BE:
			Dsp_data[sid].m_nBaseTone = BE_DIAL;
			break;
		case DSPCODEC_COUNTRY_FL:
			Dsp_data[sid].m_nBaseTone = FL_DIAL;
			break;
		case DSPCODEC_COUNTRY_IT:
			Dsp_data[sid].m_nBaseTone = IT_DIAL;
			break;
		case DSPCODEC_COUNTRY_CN:
			Dsp_data[sid].m_nBaseTone = CN_DIAL;
			break;
		case DSPCODEC_COUNTRY_EX1:
			Dsp_data[sid].m_nBaseTone = EX1_DIAL;
			break;
		case DSPCODEC_COUNTRY_EX2:
			Dsp_data[sid].m_nBaseTone = EX2_DIAL;
			break;
		case DSPCODEC_COUNTRY_EX3:
			Dsp_data[sid].m_nBaseTone = EX3_DIAL;
			break;
		case DSPCODEC_COUNTRY_EX4:
			Dsp_data[sid].m_nBaseTone = EX4_DIAL;
			break;
#ifdef COUNTRY_TONE_RESERVED
		case DSPCODEC_COUNTRY_RESERVE:
			Dsp_data[sid].m_nBaseTone = RESERVE_DIAL;
			break;
#endif
		case DSPCODEC_COUNTRY_CUSTOME:
			Dsp_data[sid].m_nBaseTone = CUSTOM_DIAL;
			break;
#else
		case DSPCODEC_COUNTRY_USA:
			Dsp_data[sid].m_nDialTone = USA_DIAL;
			Dsp_data[sid].m_nRingTone = USA_RING;
			Dsp_data[sid].m_nBusyTone = USA_BUSY;
			Dsp_data[sid].m_nWaitingTone = USA_WAITING;
			break;
		case DSPCODEC_COUNTRY_UK:
			Dsp_data[sid].m_nDialTone = UK_DIAL;
			Dsp_data[sid].m_nRingTone = UK_RING;
			Dsp_data[sid].m_nBusyTone = UK_BUSY;
			Dsp_data[sid].m_nWaitingTone = USA_WAITING;
			break;
		case DSPCODEC_COUNTRY_AUSTRALIA:
			Dsp_data[sid].m_nDialTone = AUSTRALIA_DIAL;
			Dsp_data[sid].m_nRingTone = AUSTRALIA_RING;
			Dsp_data[sid].m_nBusyTone = AUSTRALIA_BUSY;
			Dsp_data[sid].m_nWaitingTone = AUSTRALIA_WAITING;
			break;
		case DSPCODEC_COUNTRY_HK:
			Dsp_data[sid].m_nDialTone = HK_DIAL;
			Dsp_data[sid].m_nRingTone = HK_RING;
			Dsp_data[sid].m_nBusyTone = HK_BUSY;
			Dsp_data[sid].m_nWaitingTone = HK_WAITING;
			break;
		case DSPCODEC_COUNTRY_JP:
			Dsp_data[sid].m_nDialTone = JP_DIAL;
			Dsp_data[sid].m_nRingTone = JP_RING;
			Dsp_data[sid].m_nBusyTone = JP_BUSY;
			Dsp_data[sid].m_nWaitingTone = JP_WAITING;
			break;
		case DSPCODEC_COUNTRY_SE:
			Dsp_data[sid].m_nDialTone = SE_DIAL;
			Dsp_data[sid].m_nRingTone = SE_RING;
			Dsp_data[sid].m_nBusyTone = SE_BUSY;
			Dsp_data[sid].m_nWaitingTone = SE_WAITING;
			break;
		case DSPCODEC_COUNTRY_GR:
			Dsp_data[sid].m_nDialTone = GR_DIAL;
			Dsp_data[sid].m_nRingTone = GR_RING;
			Dsp_data[sid].m_nBusyTone = GR_BUSY;
			Dsp_data[sid].m_nWaitingTone = GR_WAITING;
			break;
		case DSPCODEC_COUNTRY_FR:
			Dsp_data[sid].m_nDialTone = FR_DIAL;
			Dsp_data[sid].m_nRingTone = FR_RING;
			Dsp_data[sid].m_nBusyTone = FR_BUSY;
			Dsp_data[sid].m_nWaitingTone = FR_WAITING;
			break;
		case DSPCODEC_COUNTRY_TR:
			Dsp_data[sid].m_nDialTone = TR_DIAL;
			Dsp_data[sid].m_nRingTone = TR_RING;
			Dsp_data[sid].m_nBusyTone = TR_BUSY;
			Dsp_data[sid].m_nWaitingTone = TR_WAITING;
			break;
		case DSPCODEC_COUNTRY_BE:
			Dsp_data[sid].m_nDialTone = BE_DIAL;
			Dsp_data[sid].m_nRingTone = BE_RING;
			Dsp_data[sid].m_nBusyTone = BE_BUSY;
			Dsp_data[sid].m_nWaitingTone = BE_WAITING;
			break;
		case DSPCODEC_COUNTRY_FL:
			Dsp_data[sid].m_nDialTone = FL_DIAL;
			Dsp_data[sid].m_nRingTone = FL_RING;
			Dsp_data[sid].m_nBusyTone = FL_BUSY;
			Dsp_data[sid].m_nWaitingTone = FL_WAITING;
			break;
		case DSPCODEC_COUNTRY_IT:
			Dsp_data[sid].m_nDialTone = IT_DIAL;
			Dsp_data[sid].m_nRingTone = IT_RING;
			Dsp_data[sid].m_nBusyTone = IT_BUSY;
			Dsp_data[sid].m_nWaitingTone = IT_WAITING;
			break;
		case DSPCODEC_COUNTRY_CN:
			Dsp_data[sid].m_nDialTone = CN_DIAL;
			Dsp_data[sid].m_nRingTone = CN_RING;
			Dsp_data[sid].m_nBusyTone = CN_BUSY;
			Dsp_data[sid].m_nWaitingTone = CN_WAITING;
			break;
		case DSPCODEC_COUNTRY_CUSTOME:
			Dsp_data[sid].m_nDialTone = dialTone+CUSTOM_TONE1;
			Dsp_data[sid].m_nRingTone = ringTone+CUSTOM_TONE1;
			Dsp_data[sid].m_nBusyTone = busyTone+CUSTOM_TONE1;
			Dsp_data[sid].m_nWaitingTone = waitingTone+CUSTOM_TONE1;
			break;
#endif
		default:
//			printf("Dspcodec: !!! Wrong parameter of nCountry %d !!!\n", nCountry);
			assert(0);
			return DSPCODEC_ERROR_PARAMETER;
	}
#ifdef SUPPORT_TONE_PROFILE

	if(pre_country_tone !=Dsp_data[sid].m_nBaseTone)//change country
	{
		for (k=0; k<CH_TONE; k++)
		{
			if (fPlayTone[sid][k] == 1
			   &&(WhichTone[sid][k]>= pre_country_tone)
			   &&(WhichTone[sid][k]<= (pre_country_tone+DSPCODEC_TONE_INGRESS_RINGBACK-DSPCODEC_TONE_DIAL)) )
			{
				printk("change tone when change country,sid=%d",sid);
				WhichTone[sid][k]=WhichTone[sid][k]+Dsp_data[sid].m_nBaseTone-pre_country_tone;
			}

		}
	}

#else
	if(  (pre_m_nDialTone != Dsp_data[sid].m_nDialTone)
	   ||(pre_m_nRingTone != Dsp_data[sid].m_nRingTone)
	   ||(pre_m_nBusyTone != Dsp_data[sid].m_nBusyTone)
	   ||(pre_m_nWaitingTone != Dsp_data[sid].m_nWaitingTone)  )//change country
	{
		for (k=0; k<CH_TONE; k++)
		{
			if (fPlayTone[sid][k] == 1 )
			{
				switch(WhichTone[sid][k])
				{
					case pre_m_nDialTone:
						WhichTone[sid][k] = Dsp_data[sid].m_nDialTone;
						printk("change tone when change country");
						break;
					case pre_m_nRingTone:
						WhichTone[sid][k] = Dsp_data[sid].m_nRingTone;
						printk("change tone when change country");
						break;
					case pre_m_nBusyTone:
						WhichTone[sid][k] = Dsp_data[sid].m_nBusyTone;
						printk("change tone when change country");
						break;
					case pre_m_nWaitingTone:
						WhichTone[sid][k] = Dsp_data[sid].m_nWaitingTone;
						printk("change tone when change country");
						break;
				}
			}

	}

#endif

	return DSPCODEC_SUCCESS;
}

#if 0
RESULT DspcodecSetInterface(DSPCODEC_INTERFACE nInterface)
{
#ifdef __ECOS
	uint8 value;

	if(nInterface == DSPCODEC_INTERFACE_HANDSET)
	{
		Dsp_data[chid].m_nInterface = nInterface;
		value = CodecRead(0x00);
		value = value & 0xfc;
		CodecWrChk(0x00, value);
	}
	else if(nInterface == DSPCODEC_INTERFACE_HANDFREE)
	{
		Dsp_data[chid].m_nInterface = nInterface;
		value = CodecRead(0x00);
		value = (value & 0xfc) | 0x01;
		CodecWrChk(0x00, value);
	}
	else if(nInterface == DSPCODEC_INTERFACE_HEADSET)
	{
		Dsp_data[chid].m_nInterface = nInterface;
		value = CodecRead(0x00);
		value = (value & 0xfc) | 0x02;
		CodecWrChk(0x00, value);
	}
	else
	{
//		printf("Dspcodec: !!! No such interface %d !!!\n", nInterface);
		assert(0);
		return DSPCODEC_ERROR_PARAMETER;
	}
#endif
	return DSPCODEC_SUCCESS;
}
#endif

#if 0
int32 DspcodecGetVersion(char *pBuf, int32 nLen)
{
	int n = 0;
	RESULT retval;

	if(!pBuf || !nLen)
		return 0;

#ifdef __ECOS
	sem_wait(&Dsp_data.m_semDspAccess);
	pSendParm->uCommand = DSPCODEC_COMMAND_GETVERSION;
	pSendParm->nOwner = 0;
	retval = SendCommand(sid);
	if(retval == DSPCODEC_SUCCESS)
	{
		n = (nLen > (int)strlen((char*)(PacketTxBuffer->pPrintBuff)))?
			strlen((char*)(PacketTxBuffer->pPrintBuff)) : (nLen - 1);
		strncpy(pBuf, (char*)(PacketTxBuffer->pPrintBuff), n + 1);
	}
	sem_post(&Dsp_data.m_semDspAccess);

	if(retval != DSPCODEC_SUCCESS)
	{
//		printf("Dspcodec: !!! Send command [GETVERSION] fail !!!\n");
	}
#else
	strncpy(pBuf, "N/A", 4);
	n = 3;
#endif
	retval = retval;	// remove compiling warning
	return n;
}
#endif

void DspcodecResetR1(uint32 sid)
{
	RESULT retval;
	// send config to RISC1
#if 0
	memset((void *)(&(pSendParm[sid].xParm)), 0, sizeof(CDspcodecSetConfigParm));
	//pSendParm[sid].uCommand = DSPCODEC_COMMAND_SETCONFIG;
	//pSendParm[sid].nOwner = 0;
	//pSendParm[sid].xParm.xSetConfigParm.xConfig.pToneTable 	= Dsp_data[sid].m_xConfig.pToneTable;
	//pSendParm[sid].xParm.xSetConfigParm.xConfig.pCtrlParm 	= Dsp_data[sid].m_xConfig.pCtrlParm;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.bVAD 		= Dsp_data[sid].m_xConfig.bVAD;
	//pSendParm[sid].xParm.xSetConfigParm.xConfig.nVadLevel 	= Dsp_data[sid].m_xConfig.nVadLevel;
	//pSendParm[sid].xParm.xSetConfigParm.xConfig.bAES 		= Dsp_data[sid].m_xConfig.bAES;
	//pSendParm[sid].xParm.xSetConfigParm.xConfig.nHangoverTime = Dsp_data[sid].m_xConfig.nHangoverTime;
	//pSendParm[sid].xParm.xSetConfigParm.xConfig.nBGNoiseLevel = Dsp_data[sid].m_xConfig.nBGNoiseLevel;
	//pSendParm[sid].xParm.xSetConfigParm.xConfig.nAttenRange = Dsp_data[sid].m_xConfig.nAttenRange;
	//pSendParm[sid].xParm.xSetConfigParm.xConfig.nTXGain 	= Dsp_data[sid].m_xConfig.nTXGain;
	//pSendParm[sid].xParm.xSetConfigParm.xConfig.nRXGain 	= Dsp_data[sid].m_xConfig.nRXGain;
	//pSendParm[sid].xParm.xSetConfigParm.xConfig.nTRRatio 	= Dsp_data[sid].m_xConfig.nTRRatio;
	//pSendParm[sid].xParm.xSetConfigParm.xConfig.nRTRatio 	= Dsp_data[sid].m_xConfig.nRTRatio;
	//pSendParm[sid].xParm.xSetConfigParm.xConfig.bPLC 		= Dsp_data[sid].m_xConfig.bPLC;
#endif

	//retval = SendCommand(sid);
	retval = DspcodecSetConfig_R1( sid );

	if(retval != DSPCODEC_SUCCESS)
	{
	}

}

#if 0
void DspcodecDebugR1(uint32 sid)
{
	RESULT retval;
	int index=0, wait_cnt=0, length ;
	char * p_buf ;

	pSendParm[sid].uCommand = DSPCODEC_COMMAND_DEBUG;
	pSendParm[sid].nOwner = 0;
	retval = SendCommand(sid);

	if(retval != DSPCODEC_SUCCESS)
	{
	}

	while(1)
	{
		if(PacketTxBuffer[sid].xEncOutput[index].nOwner == 1)
		{
			if(PacketTxBuffer[sid].xEncOutput[index].nSize == 0)
				break ;
			else
			{
				length = PacketTxBuffer[sid].xEncOutput[index].nSize;
				p_buf = (char*)(PacketTxBuffer[sid].xEncOutput[index].pBuffer);
				PacketTxBuffer[sid].xEncOutput[index].nOwner = 0;
				++index ;
				if(index == BUFFER_NUM_ENC)
					index = 0;
			}
			wait_cnt = 0 ;
		}
		if( ++wait_cnt > 1000 ) break ;
	}
}
#endif

#ifdef SUPPORT_DYNAMIC_PAYLOAD
int32 DPDspcodecWrite(uint32 chid, uint32 sid, uint8* pBuf, int32 nSize, uint32 nSeq, uint32 nTimestamp,
						uint32 bRedundancy)
{
	//int nLen;
	int voice_frames_num, k;
	uint16 in_seq;			// internal seq_no + frames' ones 
	uint32 timestamp;		// current packet's timestamp + frames' ones (internal use this too)
	int TempLen = 0;		// voice frame length 
	int TempStamp = 0;		// voice frame timestamp 
 #ifdef SUPPORT_APPENDIX_SID
	int TempLenSID = 4;		// SID frame length (give a special value to prevent that we don't initalize this value in some codec.)
	int bAppendixSID = 0;	// SID exists in this packet 
 #endif
 	uint8 *pSrcBuf;		// point to pBuf. Increase this value in decomposition loop. 
 #ifdef SUPPORT_ADJUST_JITTER
	extern unsigned short rx_frames_per_packet[MAX_DSP_RTK_SS_NUM];
	extern void InitAdjustJitterOfJBC( uint32 ssid, unsigned short nRxFramePerPacket );
 #endif
	extern void AnnouncePacketRecivce_OnTime( uint32 sid );
	extern void AnnouncePacketRecivce_Slow( uint32 sid );
	extern void AnnouncePacketRecivce_Loss( uint32 sid );
 #ifdef SUPPORT_RTP_REDUNDANT
	extern uint32 RtpRedundantTimestamp_local[ DSP_RTK_SS_NUM ];
 #endif
 #ifdef SUPPORT_V152_VBD
	extern int V152_CheckSessionEnabled( int sid );
 #endif

 	const codec_algo_desc_t *pCurrentCodecDescriptor;
 	FN_GetFrameInfo_FrameLength pfnGetFrameLength;
 	int bVolatileFrameSize;
 	int bFrameSizeinbit;
 #if 1//def CONFIG_RTK_VOIP_RTCP_XR
 	extern void JbcDiscard_SetFrames( uint32 ssid, uint32 n );
	int discard = 0;
  #define INC_DISCARD_IF0		discard += !	// increase discard if 0 
 #else
  #define INC_DISCARD_IF0
 #endif
 	
 	enum {
 		ARRCASE_NORMAL,	// normal          ( cur == exp )
 		ARRCASE_OOSEQ,	// out of sequence ( cur <  exp )
 		ARRCASE_LOSS,	// loss            ( cur >  exp )
 	} arrive_case;

	//printk( "x%d ", nSize );
	
#if 0 	
	if( sid == 0 )
		printk( "%d@%d ", nSeq & 0x3F, nTimestamp & 0xFFF ); 
#endif
	
	/*
	 * Step 1. According to codec algorithm, frame payload length, 
	 *         frame timestamp and SID payload length are given.
	 */
	pCurrentCodecDescriptor = ppNowRecvCodecAlgorithmDesc[ sid ];
	
 	if( pCurrentCodecDescriptor == NULL ) {
 		printk("\n warning; (%d)codec algorithm is not defined, but for r1 writting!", sid );
		return 0;
	}
	
	pfnGetFrameLength = pCurrentCodecDescriptor ->fnGetR0FrameLength;
		
  #ifdef SUPPORT_APPENDIX_SID
	TempLenSID = ( *pCurrentCodecDescriptor ->fnGetR0SidLength )( nSize );
  #endif
  	TempLen = ( *pfnGetFrameLength )( pBuf );
  	bVolatileFrameSize = pCurrentCodecDescriptor ->bR0VolatileFrameLength;
  	TempStamp = pCurrentCodecDescriptor ->nR0FrameTimestamp;
	bFrameSizeinbit = pCurrentCodecDescriptor ->bR0FrameLengthinbit;
	
	/*
	 * Step 2. Modify source ptr and size by PayloadShift 
	 */
	
	if ( (pCurrentCodecDescriptor ->bPayloadShift) == 1 )
	{
		pBuf = (uint8*)pBuf + ( pCurrentCodecDescriptor ->nPayloadShiftByte );
		nSize = nSize - ( pCurrentCodecDescriptor ->nPayloadShiftByte );
	}

	/*
	 * Step 3. Calculate number of frames in this packet.
	 */
	
	//nLen = nSize;

	voice_frames_num = ( *pCurrentCodecDescriptor ->fnGetNumberOfFramesInCurrentPacket )
			( pBuf, nSize, TempLen, TempLenSID, &bAppendixSID );

 #ifdef SUPPORT_V152_VBD	// prevent V.152 codec handover issue 
	if( voice_frames_num >= 4 && rx_frames_per_packet[ sid ] > 0 && rx_frames_per_packet[ sid ] <= 2 && 
		V152_CheckSessionEnabled( sid ) ) 
	{
		// Ignore this packet!! 
		//printk( "I" );
		return 0;
	}
#endif
 
	//if(i>0)
	//	DPSize[sid] = i;

	/*
	 * Step 4. Save & process depends on frames number 
	 */	
	if( bAppendixSID )
		nRxSilencePacket[ sid ] ++;

 #ifdef SUPPORT_ADJUST_JITTER
	if( rx_frames_per_packet[ sid ] < voice_frames_num )
		InitAdjustJitterOfJBC( sid, voice_frames_num );
 #endif // SUPPORT_ADJUST_JITTER
 #ifdef SUPPORT_RTP_REDUNDANT
	if( RtpRedundantTimestamp_local[ sid ] == 0 ) {
		RtpRedundantTimestamp_local[ sid ] = voice_frames_num * TempStamp;
	}
 #endif
	/*
	 * Step 5. Check packet arrive case 
	 */
#ifdef DYNAMIC_PAYLOAD_VER1	//tyhuang:111
	in_seq = DPInitSeq[sid];
	timestamp = DPInitStamp[sid];
	for(k=0;k<voice_frames_num;k++)
	{
		INC_DISCARD_IF0
		DspcodecWrite(chid, sid, pBuf+TempLen*k, TempLen, in_seq, timestamp);???
		in_seq++;
		timestamp +=TempStamp;
	}
	DPInitSeq[sid] = in_seq;
	DPInitStamp[sid] = timestamp;
#else // ! DYNAMIC_PAYLOAD_VER1
	//DPOutSeq[sid] = nSeq;
	timestamp = nTimestamp;
	in_seq = DPInSeq[sid] + 1;
	pSrcBuf = pBuf;
	
	if( DPNotFirstPacket[ sid ] == 0 ) {
		// sync. first packet or resume phone call.
		DPExpSeq[sid] = nSeq;
		DPNotFirstPacket[ sid ] = 1;
		
 #ifdef SUPPORT_SYNC_OUT_OF_SEQ
 		if( DPEnableSyncInSeq[ sid ] ) {
 			in_seq = DPInSeq[ sid ] = DPSyncInSeq[ sid ] + 1;
 			DPEnableSyncInSeq[ sid ] = 0;
 		}
 #endif
 #ifdef SUPPORT_SYNC_BIG_CHANGE
		DPExpTimeStamp[sid] = nTimestamp;	// for checking only.  
 #endif

		//debug_dev( "First packet seq#(%d): %d\n", sid, nSeq );
	}

#ifdef SUPPORT_SYNC_BIG_CHANGE
	if( BigChangeCount[ sid ] == 0 ) {
	
		int16 delta_seq;
		int32 delta_timestamp;
	
		delta_seq = ( int16 )( DPExpSeq[ sid ] - ( uint16 )nSeq );
		delta_timestamp = ( int32 )( DPExpTimeStamp[sid] - nTimestamp );
		
#if 1	// exclude VAD case (consider VAD case and little packet loss )
		if( delta_seq < 5 && delta_seq > -5 && 	// 5 sequence 
			delta_timestamp < 30 * 8000 && delta_timestamp > -30 * 8000 ) // 30 seconds 
		{
			//if( delta_timestamp )
			//	printk( "A: delta_seq=%d delta_timestamp=%d\n", delta_seq, delta_timestamp );
		} else 
#endif
		if( delta_seq > 50 || delta_seq < -50 ||
			delta_timestamp > 50 * 160 || delta_timestamp < -50 * 160 ) 
		{
			//printk( "B: delta_seq=%d delta_timestamp=%d\n", delta_seq, delta_timestamp );
			PRINT_MSG( "B: DS=%d DT=%d\n", delta_seq, delta_timestamp );
			
			BigChangeCount[ sid ] ++;
			BigChangePreSeq[ sid ] = nSeq;
			
			discard = rx_frames_per_packet[ sid ];
			
			BigChangeDeltaSeq[ sid ] = ((delta_seq >= 0) ? delta_seq : (-delta_seq) );
		
			goto label_no_update_variables;
		}
	
	} else {
	
		if(  BigChangePreSeq[ sid ] + 1 == nSeq ) {
			
			BigChangeCount[ sid ] ++;
			BigChangePreSeq[ sid ] = nSeq;
			
			if( BigChangeCount[ sid ] > 30 ) {
			
				extern void DspcodecWriteSnyc( uint32 sid );
				
				DspcodecWriteSnyc( sid );
				
				PRINT_MSG( "sync big change!!\n" );
				
				nRxRtpStatsLostPacket[ sid ] += BigChangeDeltaSeq[ sid ];
				
				BigChangeCount[ sid ] = 0;
				BigChangeDeltaSeq[ sid ] = 0;
				
			}
			
			discard = rx_frames_per_packet[ sid ];
			
			goto label_no_update_variables;	
		} else {
			
			BigChangePreSeq[ sid ] = 0;
		}
	}
#endif // SUPPORT_SYNC_BIG_CHANGE
	
	if(DPExpSeq[sid] == nSeq) 	//arrival on time
	{
		AnnouncePacketRecivce_OnTime( sid );
		
		arrive_case = ARRCASE_NORMAL;
		goto label_start_write_frames;
	}
	//else if(DPExpSeq[sid] > nSeq)			//slow (out of order)
	else if( RtpSeqGreater( DPExpSeq[sid], nSeq ) )	
	{
 #if 0
		//debug_dev("slow?\n");//drop it?
		return 0;
 #else
  #ifdef RESERVE_SPACE_FOR_SLOW
  		// assign correct seq#
   		in_seq = DPInSeq[sid] - ( DPExpTimeStamp[sid] - nTimestamp )/TempStamp + 1;
  #else
		//syn
		DPExpSeq[sid] = nSeq;
  #endif
		
  		AnnouncePacketRecivce_Slow( sid );
  		
  		arrive_case = ARRCASE_OOSEQ;
  		goto label_start_write_frames;
  		
 #endif	// if 0
	}
	else								//lose?
	{
		/*
		 * How to deal with packet loss after SID? JBC will solve it.
		 * We give a scenario to describe packet loss:
		 *      Pv  Psid	NTx		NTx		loss	Pv
		 * seq# 0	1						2		3
		 * time 0	80		(160)	(240)	320		400
		 *     << After Dynamic Payload >>
		 * seq#	0	1								?
		 * time	0	80								400 
		 * 				--> (400-80)/80=4 --> seq# = 5 ><
		 */
		uint16 lost_pkt;
		int32 lost_payload;
		
		lost_pkt = ( uint16 )nSeq - DPExpSeq[sid];	//lost packet no		
		lost_payload = ( int32 )(nTimestamp - DPExpTimeStamp[sid])/TempStamp;	//lost payload
		//debug_warning("%d:L%d(%d, %d, %d) ",sid, lost_payload, lost_pkt, nSeq, DPExpSeq[sid]);
		debug_warning("%d:L%d ",sid, lost_pkt);
		
		//printk("%d:L%u,%d ",sid, lost_pkt, lost_payload);
		
		if( lost_pkt > 200 || 
			lost_payload > 200 || lost_payload < 0 ) 
		{
			// simple *HUGE* change check.

			// seen as no loss 
			lost_pkt = 0;
			lost_payload = 0;
		}
		
		debug_warning("%d:L%d,%d ",sid, lost_pkt, lost_payload);
		in_seq += lost_payload;
		nRxRtpStatsLostPacket[ sid ] += lost_pkt;
		//debug_dev("DPSeq %d->%d\n", DPInSeq[sid], seq);
		
		if( lost_pkt )
 			AnnouncePacketRecivce_Loss( sid );

//	debug_dev("lost?\n");
 		
 		arrive_case = ARRCASE_LOSS;
 		goto label_start_write_frames;
	}
	
	// ----------------------------------------------------- 
	// Step 6. write frames to DspcodecWrite
	//
	// parameters:
	//  voice_frames_num [i ] frames number 
	//  bAppendixSID     [i ] contain exactly one SID frame 
	//  pSrcBuf      [im] source payload ptr 
	//  bFrameSizeinbit    [i ] different case to know frame size 
	//  bVolatileFrameSize [i ] different case to know frame size 
	//  in_seq    [io] seq_no to jitter buffer (based on previous writing)
	//  timestamp [io] timstamp to jitter buffer (based on current packet) 
	//  TempLen   [i ] voice frame length 
	//  TempStamp [i ] voice frame timestamp 
	//
label_start_write_frames:

#if 0	
	if( sid == 0 )
		printk( "C%d ", arrive_case );
#endif

#if 0
	if( sid == 0 )
		printk( "[" );
#endif
	
	for( k = 0; k < voice_frames_num; k ++ )
	{
		int TemLen_inbit;	/* the packet length in bit */
		unsigned char Temp_buf[62];
		if ( bFrameSizeinbit )
		{
			TemLen_inbit = 4;
			if ( ((TemLen_inbit & 0x7)*k)& 0x7)
			{
				int bit_shift = ((TemLen_inbit & 0x7)*k)& 0x7;
				int temp_index;
				for ( temp_index = 0; temp_index<TempLen ; temp_index++)
				{
					Temp_buf[temp_index]=0;
					Temp_buf[temp_index]= (pSrcBuf[temp_index-1]<<bit_shift) | (pSrcBuf[temp_index]>>bit_shift);
				}

			}
			else
			{
				int temp_index;
				for ( temp_index = 0; temp_index<TempLen ; temp_index++)
				{
					Temp_buf[temp_index]= pSrcBuf[temp_index];
				}
			
			}

			INC_DISCARD_IF0
			DspcodecWrite(chid, sid, Temp_buf, TempLen, in_seq, timestamp, RTP_FRAME_NORMAL);
			in_seq ++;
			timestamp +=TempStamp;
			pSrcBuf += TempLen;

			if ( (((TemLen_inbit & 0x7)*k) & (~0x7))  != (((TemLen_inbit & 0x7)*(k+1)) & (~0x7)))
				pSrcBuf--;
		}
		else
		{
			if( bVolatileFrameSize )
				TempLen = ( *pfnGetFrameLength )( pSrcBuf );
			
	
			INC_DISCARD_IF0
			DspcodecWrite(chid, sid, pSrcBuf, TempLen, in_seq, timestamp, RTP_FRAME_NORMAL);
			in_seq ++;
			timestamp +=TempStamp;
			pSrcBuf += TempLen;
		}
	}
 #ifdef SUPPORT_APPENDIX_SID
 	if( bAppendixSID ) {
  #if 1
        /* In a very special case, callee sent CNG in period of 40 */
        extern bool RtpTimeStampGreater(uint32 a, uint32 b);
        
        if( arrive_case == ARRCASE_NORMAL &&
        	voice_frames_num == 0 && 
        	RtpTimeStampGreater( DPExpTimeStamp[sid], nTimestamp ) )
        {
            // Don't update variables related to dynamic paylaod
            DPExpSeq[sid] = nSeq + 1;
            //printk( "C%d\n", nSeq );
            return nSize;
        }
  #endif
		INC_DISCARD_IF0
		DspcodecWrite(chid, sid, pSrcBuf, TempLenSID, in_seq, timestamp, RTP_FRAME_SID);
		in_seq ++;
		timestamp +=TempStamp;
	}
 #endif	

#if 0
	if( sid == 0 )
		printk( "]" );
#endif

	// end of frames write 
	// ----------------------------------------------------- 

	/*
	 * Step 7. Save internal and expected information for next packet 
	 */

 #ifdef RESERVE_SPACE_FOR_SLOW
	if( arrive_case == ARRCASE_OOSEQ ) {
  		// Don't update variables related to dynamic paylaod 
  		//return nSize;
  		goto label_no_update_variables;
  	}
 #endif

	DPInSeq[sid] = in_seq - 1;
	DPExpSeq[sid] = nSeq + 1;
	DPExpTimeStamp[sid] = timestamp;

label_no_update_variables:
#endif // DYNAMIC_PAYLOAD_VER1

	if( discard && !bRedundancy ) {
#ifdef CONFIG_RTK_VOIP_RTCP_XR
		extern void RtpRx_IncDiscard( uint32 sid );
		RtpRx_IncDiscard( sid );
#endif
		JbcDiscard_SetFrames( sid, discard );
	}

	return nSize;
}
#endif // SUPPORT_DYNAMIC_PAYLOAD

void DspcodecWriteSnyc( uint32 sid )
{
#if defined( SUPPORT_DYNAMIC_PAYLOAD ) && !defined( DYNAMIC_PAYLOAD_VER1 )
	DPNotFirstPacket[ sid ] = 0;
#endif // SUPPORT_DYNAMIC_PAYLOAD && !DYNAMIC_PAYLOAD_VER1
}

void DspcodecWriteSkipSeqNo( uint32 sid, uint32 nSeq )
{
	//uint32 diff;

	//if( diff = ( nSeq - DPExpSeq[ sid ] ) )
	//	debug_dev( "LS:%d\n", diff );
	
	DPExpSeq[ sid ] = nSeq + 1;
}

void ResetSessionRxStatistics( uint32 sid )
{
	nRxSilencePacket[ sid ] = 0;
}

// ------------------------------------------------------------------
// proc 
// ------------------------------------------------------------------
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int voip_rtp_write_read_proc(struct seq_file *f, void *v)
{
	int ss;
	int n = 0;

	if( IS_CH_PROC_DATA( v ) ) {
		//ch = CH_FROM_PROC_DATA( data );
		//n = sprintf( buf, "channel=%d\n", ch );
	} else {
		ss = SS_FROM_PROC_DATA( v );
		
		seq_printf( f, "session=%d\n", ss );
		
		seq_printf( f, "DPInSeq=%u DPExpSeq=%u "
						"DPExpTimeStamp=%u DPNotFirstPacket=%u\n", 
						DPInSeq[ ss ], DPExpSeq[ ss ], 
						DPExpTimeStamp[ ss ], DPNotFirstPacket[ ss ] );
	}

	return n;
}
#else
static int voip_rtp_write_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int ss;
	int n = 0;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	if( IS_CH_PROC_DATA( data ) ) {
		//ch = CH_FROM_PROC_DATA( data );
		//n = sprintf( buf, "channel=%d\n", ch );
	} else {
		ss = SS_FROM_PROC_DATA( data );
		
		n = sprintf( buf, "session=%d\n", ss );
		
		n += sprintf( buf + n, "DPInSeq=%u DPExpSeq=%u "
						"DPExpTimeStamp=%u DPNotFirstPacket=%u\n", 
						DPInSeq[ ss ], DPExpSeq[ ss ], 
						DPExpTimeStamp[ ss ], DPNotFirstPacket[ ss ] );
	}
	
	*eof = 1;
	return n;
}
#endif

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int rtp_write_open(struct inode *inode, struct file *file)
{
	return single_open(file, voip_rtp_write_read_proc, NULL);
}

struct file_operations proc_rtp_write_fops = {
	.owner	= THIS_MODULE,
	.open	= rtp_write_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
//read:   voip_rtp_write_read_proc
};
#endif


static int __init voip_rtp_write_proc_init( void )
{
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	create_voip_session_proc_read_entry( "rtp_write", &proc_rtp_write_fops );
#else
	create_voip_session_proc_read_entry( "rtp_write", voip_rtp_write_read_proc );
#endif

	return 0;
}

static void __exit voip_rtp_write_proc_exit( void )
{
	remove_voip_session_proc_entry( "rtp_write" );
}

voip_initcall_proc( voip_rtp_write_proc_init );
voip_exitcall( voip_rtp_write_proc_exit );

// ------------------------------------------------------------------
// ------------------------------------------------------------------

#ifdef SUPPORT_TONE_PROFILE
RESULT DspcodecSetCustomTone(DSPCODEC_TONE nTone, ToneCfgParam_t *pToneCfg)
{
	uint32 idx = 0;
	ToneCfgParam_t *pToneTable = NULL;

	if((nTone < DSPCODEC_TONE_DIAL) || (nTone > DSPCODEC_TONE_INGRESS_RINGBACK))
		return DSPCODEC_ERROR_PARAMETER;
#if 1
	idx = nTone - DSPCODEC_TONE_DIAL;
	pToneTable = (ToneCfgParam_t *)&ToneTable[CUSTOM_DIAL+idx];
	pToneTable->ToneType = pToneCfg->ToneType;
	pToneTable->cycle = pToneCfg->cycle;
	pToneTable->cadNUM = pToneCfg->cadNUM;
	pToneTable->CadOn0 = pToneCfg->CadOn0;
	pToneTable->CadOff0 = pToneCfg->CadOff0;
	pToneTable->CadOn1 = pToneCfg->CadOn1;
	pToneTable->CadOff1 = pToneCfg->CadOff1;
	pToneTable->CadOn2 = pToneCfg->CadOn2;
	pToneTable->CadOff2 = pToneCfg->CadOff2;
	pToneTable->CadOn3 = pToneCfg->CadOn3;
	pToneTable->CadOff3 = pToneCfg->CadOff3;
	pToneTable->PatternOff = pToneCfg->PatternOff;
	pToneTable->ToneNUM = pToneCfg->ToneNUM;
	pToneTable->Freq0 = pToneCfg->Freq0;
	pToneTable->Freq1 = pToneCfg->Freq1;
	pToneTable->Freq2 = pToneCfg->Freq2;
	pToneTable->Freq3 = pToneCfg->Freq3;
	pToneTable->Gain0 = pToneCfg->Gain0;
	pToneTable->Gain1 = pToneCfg->Gain1;
	pToneTable->Gain2 = pToneCfg->Gain2;
	pToneTable->Gain3 = pToneCfg->Gain3;
	pToneTable->C1_Freq0 = pToneCfg->C1_Freq0;
	pToneTable->C1_Freq1 = pToneCfg->C1_Freq1;
	pToneTable->C1_Freq2 = pToneCfg->C1_Freq2;
	pToneTable->C1_Freq3 = pToneCfg->C1_Freq3;
	pToneTable->C1_Gain0 = pToneCfg->C1_Gain0;
	pToneTable->C1_Gain1 = pToneCfg->C1_Gain1;
	pToneTable->C1_Gain2 = pToneCfg->C1_Gain2;
	pToneTable->C1_Gain3 = pToneCfg->C1_Gain3;
	pToneTable->C2_Freq0 = pToneCfg->C2_Freq0;
	pToneTable->C2_Freq1 = pToneCfg->C2_Freq1;
	pToneTable->C2_Freq2 = pToneCfg->C2_Freq2;
	pToneTable->C2_Freq3 = pToneCfg->C2_Freq3;
	pToneTable->C2_Gain0 = pToneCfg->C2_Gain0;
	pToneTable->C2_Gain1 = pToneCfg->C2_Gain1;
	pToneTable->C2_Gain2 = pToneCfg->C2_Gain2;
	pToneTable->C2_Gain3 = pToneCfg->C2_Gain3;
	pToneTable->C3_Freq0 = pToneCfg->C3_Freq0;
	pToneTable->C3_Freq1 = pToneCfg->C3_Freq1;
	pToneTable->C3_Freq2 = pToneCfg->C3_Freq2;
	pToneTable->C3_Freq3 = pToneCfg->C3_Freq3;
	pToneTable->C3_Gain0 = pToneCfg->C3_Gain0;
	pToneTable->C3_Gain1 = pToneCfg->C3_Gain1;
	pToneTable->C3_Gain2 = pToneCfg->C3_Gain2;
	pToneTable->C3_Gain3 = pToneCfg->C3_Gain3;

	pToneTable->CadOn4 = pToneCfg->CadOn4;
	pToneTable->CadOff4 = pToneCfg->CadOff4;
	pToneTable->CadOn5 = pToneCfg->CadOn5;
	pToneTable->CadOff5 = pToneCfg->CadOff5;
	pToneTable->CadOn6 = pToneCfg->CadOn6;
	pToneTable->CadOff6 = pToneCfg->CadOff6;
	pToneTable->CadOn7 = pToneCfg->CadOn7;
	pToneTable->CadOff7 = pToneCfg->CadOff7;
	pToneTable->C4_Freq0 = pToneCfg->C4_Freq0;
	pToneTable->C4_Freq1 = pToneCfg->C4_Freq1;
	pToneTable->C4_Freq2 = pToneCfg->C4_Freq2;
	pToneTable->C4_Freq3 = pToneCfg->C4_Freq3;
	pToneTable->C4_Gain0 = pToneCfg->C4_Gain0;
	pToneTable->C4_Gain1 = pToneCfg->C4_Gain1;
	pToneTable->C4_Gain2 = pToneCfg->C4_Gain2;
	pToneTable->C4_Gain3 = pToneCfg->C4_Gain3;
	pToneTable->C5_Freq0 = pToneCfg->C5_Freq0;
	pToneTable->C5_Freq1 = pToneCfg->C5_Freq1;
	pToneTable->C5_Freq2 = pToneCfg->C5_Freq2;
	pToneTable->C5_Freq3 = pToneCfg->C5_Freq3;
	pToneTable->C5_Gain0 = pToneCfg->C5_Gain0;
	pToneTable->C5_Gain1 = pToneCfg->C5_Gain1;
	pToneTable->C5_Gain2 = pToneCfg->C5_Gain2;
	pToneTable->C5_Gain3 = pToneCfg->C5_Gain3;
	pToneTable->C6_Freq0 = pToneCfg->C6_Freq0;
	pToneTable->C6_Freq1 = pToneCfg->C6_Freq1;
	pToneTable->C6_Freq2 = pToneCfg->C6_Freq2;
	pToneTable->C6_Freq3 = pToneCfg->C6_Freq3;
	pToneTable->C6_Gain0 = pToneCfg->C6_Gain0;
	pToneTable->C6_Gain1 = pToneCfg->C6_Gain1;
	pToneTable->C6_Gain2 = pToneCfg->C6_Gain2;
	pToneTable->C6_Gain3 = pToneCfg->C6_Gain3;
	pToneTable->C7_Freq0 = pToneCfg->C7_Freq0;
	pToneTable->C7_Freq1 = pToneCfg->C7_Freq1;
	pToneTable->C7_Freq2 = pToneCfg->C7_Freq2;
	pToneTable->C7_Freq3 = pToneCfg->C7_Freq3;
	pToneTable->C7_Gain0 = pToneCfg->C7_Gain0;
	pToneTable->C7_Gain1 = pToneCfg->C7_Gain1;
	pToneTable->C7_Gain2 = pToneCfg->C7_Gain2;
	pToneTable->C7_Gain3 = pToneCfg->C7_Gain3;

	pToneTable->CadOn8 = pToneCfg->CadOn8;
	pToneTable->CadOff8 = pToneCfg->CadOff8;
	pToneTable->CadOn9 = pToneCfg->CadOn9;
	pToneTable->CadOff9 = pToneCfg->CadOff9;
	pToneTable->CadOn10 = pToneCfg->CadOn10;
	pToneTable->CadOff10 = pToneCfg->CadOff10;
	pToneTable->CadOn11 = pToneCfg->CadOn11;
	pToneTable->CadOff11 = pToneCfg->CadOff11;
	pToneTable->C8_Freq0 = pToneCfg->C8_Freq0;
	pToneTable->C8_Freq1 = pToneCfg->C8_Freq1;
	pToneTable->C8_Freq2 = pToneCfg->C8_Freq2;
	pToneTable->C8_Freq3 = pToneCfg->C8_Freq3;
	pToneTable->C8_Gain0 = pToneCfg->C8_Gain0;
	pToneTable->C8_Gain1 = pToneCfg->C8_Gain1;
	pToneTable->C8_Gain2 = pToneCfg->C8_Gain2;
	pToneTable->C8_Gain3 = pToneCfg->C8_Gain3;
	pToneTable->C9_Freq0 = pToneCfg->C9_Freq0;
	pToneTable->C9_Freq1 = pToneCfg->C9_Freq1;
	pToneTable->C9_Freq2 = pToneCfg->C9_Freq2;
	pToneTable->C9_Freq3 = pToneCfg->C9_Freq3;
	pToneTable->C9_Gain0 = pToneCfg->C9_Gain0;
	pToneTable->C9_Gain1 = pToneCfg->C9_Gain1;
	pToneTable->C9_Gain2 = pToneCfg->C9_Gain2;
	pToneTable->C9_Gain3 = pToneCfg->C9_Gain3;
	pToneTable->C10_Freq0 = pToneCfg->C10_Freq0;
	pToneTable->C10_Freq1 = pToneCfg->C10_Freq1;
	pToneTable->C10_Freq2 = pToneCfg->C10_Freq2;
	pToneTable->C10_Freq3 = pToneCfg->C10_Freq3;
	pToneTable->C10_Gain0 = pToneCfg->C10_Gain0;
	pToneTable->C10_Gain1 = pToneCfg->C10_Gain1;
	pToneTable->C10_Gain2 = pToneCfg->C10_Gain2;
	pToneTable->C10_Gain3 = pToneCfg->C10_Gain3;
	pToneTable->C11_Freq0 = pToneCfg->C11_Freq0;
	pToneTable->C11_Freq1 = pToneCfg->C11_Freq1;
	pToneTable->C11_Freq2 = pToneCfg->C11_Freq2;
	pToneTable->C11_Freq3 = pToneCfg->C11_Freq3;
	pToneTable->C11_Gain0 = pToneCfg->C11_Gain0;
	pToneTable->C11_Gain1 = pToneCfg->C11_Gain1;
	pToneTable->C11_Gain2 = pToneCfg->C11_Gain2;
	pToneTable->C11_Gain3 = pToneCfg->C11_Gain3;

	pToneTable->CadOn12 = pToneCfg->CadOn12;
	pToneTable->CadOff12 = pToneCfg->CadOff12;
	pToneTable->CadOn13 = pToneCfg->CadOn13;
	pToneTable->CadOff13 = pToneCfg->CadOff13;
	pToneTable->CadOn14 = pToneCfg->CadOn14;
	pToneTable->CadOff14 = pToneCfg->CadOff14;
	pToneTable->CadOn15 = pToneCfg->CadOn15;
	pToneTable->CadOff15 = pToneCfg->CadOff15;
	pToneTable->C12_Freq0 = pToneCfg->C12_Freq0;
	pToneTable->C12_Freq1 = pToneCfg->C12_Freq1;
	pToneTable->C12_Freq2 = pToneCfg->C12_Freq2;
	pToneTable->C12_Freq3 = pToneCfg->C12_Freq3;
	pToneTable->C12_Gain0 = pToneCfg->C12_Gain0;
	pToneTable->C12_Gain1 = pToneCfg->C12_Gain1;
	pToneTable->C12_Gain2 = pToneCfg->C12_Gain2;
	pToneTable->C12_Gain3 = pToneCfg->C12_Gain3;
	pToneTable->C13_Freq0 = pToneCfg->C13_Freq0;
	pToneTable->C13_Freq1 = pToneCfg->C13_Freq1;
	pToneTable->C13_Freq2 = pToneCfg->C13_Freq2;
	pToneTable->C13_Freq3 = pToneCfg->C13_Freq3;
	pToneTable->C13_Gain0 = pToneCfg->C13_Gain0;
	pToneTable->C13_Gain1 = pToneCfg->C13_Gain1;
	pToneTable->C13_Gain2 = pToneCfg->C13_Gain2;
	pToneTable->C13_Gain3 = pToneCfg->C13_Gain3;
	pToneTable->C14_Freq0 = pToneCfg->C14_Freq0;
	pToneTable->C14_Freq1 = pToneCfg->C14_Freq1;
	pToneTable->C14_Freq2 = pToneCfg->C14_Freq2;
	pToneTable->C14_Freq3 = pToneCfg->C14_Freq3;
	pToneTable->C14_Gain0 = pToneCfg->C14_Gain0;
	pToneTable->C14_Gain1 = pToneCfg->C14_Gain1;
	pToneTable->C14_Gain2 = pToneCfg->C14_Gain2;
	pToneTable->C14_Gain3 = pToneCfg->C14_Gain3;
	pToneTable->C15_Freq0 = pToneCfg->C15_Freq0;
	pToneTable->C15_Freq1 = pToneCfg->C15_Freq1;
	pToneTable->C15_Freq2 = pToneCfg->C15_Freq2;
	pToneTable->C15_Freq3 = pToneCfg->C15_Freq3;
	pToneTable->C15_Gain0 = pToneCfg->C15_Gain0;
	pToneTable->C15_Gain1 = pToneCfg->C15_Gain1;
	pToneTable->C15_Gain2 = pToneCfg->C15_Gain2;
	pToneTable->C15_Gain3 = pToneCfg->C15_Gain3;

	pToneTable->CadOn16 = pToneCfg->CadOn16;
	pToneTable->CadOff16 = pToneCfg->CadOff16;
	pToneTable->CadOn17 = pToneCfg->CadOn17;
	pToneTable->CadOff17 = pToneCfg->CadOff17;
	pToneTable->CadOn18 = pToneCfg->CadOn18;
	pToneTable->CadOff18 = pToneCfg->CadOff18;
	pToneTable->CadOn19 = pToneCfg->CadOn19;
	pToneTable->CadOff19 = pToneCfg->CadOff19;
	pToneTable->C16_Freq0 = pToneCfg->C16_Freq0;
	pToneTable->C16_Freq1 = pToneCfg->C16_Freq1;
	pToneTable->C16_Freq2 = pToneCfg->C16_Freq2;
	pToneTable->C16_Freq3 = pToneCfg->C16_Freq3;
	pToneTable->C16_Gain0 = pToneCfg->C16_Gain0;
	pToneTable->C16_Gain1 = pToneCfg->C16_Gain1;
	pToneTable->C16_Gain2 = pToneCfg->C16_Gain2;
	pToneTable->C16_Gain3 = pToneCfg->C16_Gain3;
	pToneTable->C17_Freq0 = pToneCfg->C17_Freq0;
	pToneTable->C17_Freq1 = pToneCfg->C17_Freq1;
	pToneTable->C17_Freq2 = pToneCfg->C17_Freq2;
	pToneTable->C17_Freq3 = pToneCfg->C17_Freq3;
	pToneTable->C17_Gain0 = pToneCfg->C17_Gain0;
	pToneTable->C17_Gain1 = pToneCfg->C17_Gain1;
	pToneTable->C17_Gain2 = pToneCfg->C17_Gain2;
	pToneTable->C17_Gain3 = pToneCfg->C17_Gain3;
	pToneTable->C18_Freq0 = pToneCfg->C18_Freq0;
	pToneTable->C18_Freq1 = pToneCfg->C18_Freq1;
	pToneTable->C18_Freq2 = pToneCfg->C18_Freq2;
	pToneTable->C18_Freq3 = pToneCfg->C18_Freq3;
	pToneTable->C18_Gain0 = pToneCfg->C18_Gain0;
	pToneTable->C18_Gain1 = pToneCfg->C18_Gain1;
	pToneTable->C18_Gain2 = pToneCfg->C18_Gain2;
	pToneTable->C18_Gain3 = pToneCfg->C18_Gain3;
	pToneTable->C19_Freq0 = pToneCfg->C19_Freq0;
	pToneTable->C19_Freq1 = pToneCfg->C19_Freq1;
	pToneTable->C19_Freq2 = pToneCfg->C19_Freq2;
	pToneTable->C19_Freq3 = pToneCfg->C19_Freq3;
	pToneTable->C19_Gain0 = pToneCfg->C19_Gain0;
	pToneTable->C19_Gain1 = pToneCfg->C19_Gain1;
	pToneTable->C19_Gain2 = pToneCfg->C19_Gain2;
	pToneTable->C19_Gain3 = pToneCfg->C19_Gain3;

	pToneTable->CadOn20 = pToneCfg->CadOn20;
	pToneTable->CadOff20 = pToneCfg->CadOff20;
	pToneTable->CadOn21 = pToneCfg->CadOn21;
	pToneTable->CadOff21 = pToneCfg->CadOff21;
	pToneTable->CadOn22 = pToneCfg->CadOn22;
	pToneTable->CadOff22 = pToneCfg->CadOff22;
	pToneTable->CadOn23 = pToneCfg->CadOn23;
	pToneTable->CadOff23 = pToneCfg->CadOff23;
	pToneTable->C20_Freq0 = pToneCfg->C20_Freq0;
	pToneTable->C20_Freq1 = pToneCfg->C20_Freq1;
	pToneTable->C20_Freq2 = pToneCfg->C20_Freq2;
	pToneTable->C20_Freq3 = pToneCfg->C20_Freq3;
	pToneTable->C20_Gain0 = pToneCfg->C20_Gain0;
	pToneTable->C20_Gain1 = pToneCfg->C20_Gain1;
	pToneTable->C20_Gain2 = pToneCfg->C20_Gain2;
	pToneTable->C20_Gain3 = pToneCfg->C20_Gain3;
	pToneTable->C21_Freq0 = pToneCfg->C21_Freq0;
	pToneTable->C21_Freq1 = pToneCfg->C21_Freq1;
	pToneTable->C21_Freq2 = pToneCfg->C21_Freq2;
	pToneTable->C21_Freq3 = pToneCfg->C21_Freq3;
	pToneTable->C21_Gain0 = pToneCfg->C21_Gain0;
	pToneTable->C21_Gain1 = pToneCfg->C21_Gain1;
	pToneTable->C21_Gain2 = pToneCfg->C21_Gain2;
	pToneTable->C21_Gain3 = pToneCfg->C21_Gain3;
	pToneTable->C22_Freq0 = pToneCfg->C22_Freq0;
	pToneTable->C22_Freq1 = pToneCfg->C22_Freq1;
	pToneTable->C22_Freq2 = pToneCfg->C22_Freq2;
	pToneTable->C22_Freq3 = pToneCfg->C22_Freq3;
	pToneTable->C22_Gain0 = pToneCfg->C22_Gain0;
	pToneTable->C22_Gain1 = pToneCfg->C22_Gain1;
	pToneTable->C22_Gain2 = pToneCfg->C22_Gain2;
	pToneTable->C22_Gain3 = pToneCfg->C22_Gain3;
	pToneTable->C23_Freq0 = pToneCfg->C23_Freq0;
	pToneTable->C23_Freq1 = pToneCfg->C23_Freq1;
	pToneTable->C23_Freq2 = pToneCfg->C23_Freq2;
	pToneTable->C23_Freq3 = pToneCfg->C23_Freq3;
	pToneTable->C23_Gain0 = pToneCfg->C23_Gain0;
	pToneTable->C23_Gain1 = pToneCfg->C23_Gain1;
	pToneTable->C23_Gain2 = pToneCfg->C23_Gain2;
	pToneTable->C23_Gain3 = pToneCfg->C23_Gain3;

	pToneTable->CadOn24 = pToneCfg->CadOn24;
	pToneTable->CadOff24 = pToneCfg->CadOff24;
	pToneTable->CadOn25 = pToneCfg->CadOn25;
	pToneTable->CadOff25 = pToneCfg->CadOff25;
	pToneTable->CadOn26 = pToneCfg->CadOn26;
	pToneTable->CadOff26 = pToneCfg->CadOff26;
	pToneTable->CadOn27 = pToneCfg->CadOn27;
	pToneTable->CadOff27 = pToneCfg->CadOff27;
	pToneTable->C24_Freq0 = pToneCfg->C24_Freq0;
	pToneTable->C24_Freq1 = pToneCfg->C24_Freq1;
	pToneTable->C24_Freq2 = pToneCfg->C24_Freq2;
	pToneTable->C24_Freq3 = pToneCfg->C24_Freq3;
	pToneTable->C24_Gain0 = pToneCfg->C24_Gain0;
	pToneTable->C24_Gain1 = pToneCfg->C24_Gain1;
	pToneTable->C24_Gain2 = pToneCfg->C24_Gain2;
	pToneTable->C24_Gain3 = pToneCfg->C24_Gain3;
	pToneTable->C25_Freq0 = pToneCfg->C25_Freq0;
	pToneTable->C25_Freq1 = pToneCfg->C25_Freq1;
	pToneTable->C25_Freq2 = pToneCfg->C25_Freq2;
	pToneTable->C25_Freq3 = pToneCfg->C25_Freq3;
	pToneTable->C25_Gain0 = pToneCfg->C25_Gain0;
	pToneTable->C25_Gain1 = pToneCfg->C25_Gain1;
	pToneTable->C25_Gain2 = pToneCfg->C25_Gain2;
	pToneTable->C25_Gain3 = pToneCfg->C25_Gain3;
	pToneTable->C26_Freq0 = pToneCfg->C26_Freq0;
	pToneTable->C26_Freq1 = pToneCfg->C26_Freq1;
	pToneTable->C26_Freq2 = pToneCfg->C26_Freq2;
	pToneTable->C26_Freq3 = pToneCfg->C26_Freq3;
	pToneTable->C26_Gain0 = pToneCfg->C26_Gain0;
	pToneTable->C26_Gain1 = pToneCfg->C26_Gain1;
	pToneTable->C26_Gain2 = pToneCfg->C26_Gain2;
	pToneTable->C26_Gain3 = pToneCfg->C26_Gain3;
	pToneTable->C27_Freq0 = pToneCfg->C27_Freq0;
	pToneTable->C27_Freq1 = pToneCfg->C27_Freq1;
	pToneTable->C27_Freq2 = pToneCfg->C27_Freq2;
	pToneTable->C27_Freq3 = pToneCfg->C27_Freq3;
	pToneTable->C27_Gain0 = pToneCfg->C27_Gain0;
	pToneTable->C27_Gain1 = pToneCfg->C27_Gain1;
	pToneTable->C27_Gain2 = pToneCfg->C27_Gain2;
	pToneTable->C27_Gain3 = pToneCfg->C27_Gain3;

	pToneTable->CadOn28 = pToneCfg->CadOn28;
	pToneTable->CadOff28 = pToneCfg->CadOff28;
	pToneTable->CadOn29 = pToneCfg->CadOn29;
	pToneTable->CadOff29 = pToneCfg->CadOff29;
	pToneTable->CadOn30 = pToneCfg->CadOn30;
	pToneTable->CadOff30 = pToneCfg->CadOff30;
	pToneTable->CadOn31 = pToneCfg->CadOn31;
	pToneTable->CadOff31 = pToneCfg->CadOff31;
	pToneTable->C28_Freq0 = pToneCfg->C28_Freq0;
	pToneTable->C28_Freq1 = pToneCfg->C28_Freq1;
	pToneTable->C28_Freq2 = pToneCfg->C28_Freq2;
	pToneTable->C28_Freq3 = pToneCfg->C28_Freq3;
	pToneTable->C28_Gain0 = pToneCfg->C28_Gain0;
	pToneTable->C28_Gain1 = pToneCfg->C28_Gain1;
	pToneTable->C28_Gain2 = pToneCfg->C28_Gain2;
	pToneTable->C28_Gain3 = pToneCfg->C28_Gain3;
	pToneTable->C29_Freq0 = pToneCfg->C29_Freq0;
	pToneTable->C29_Freq1 = pToneCfg->C29_Freq1;
	pToneTable->C29_Freq2 = pToneCfg->C29_Freq2;
	pToneTable->C29_Freq3 = pToneCfg->C29_Freq3;
	pToneTable->C29_Gain0 = pToneCfg->C29_Gain0;
	pToneTable->C29_Gain1 = pToneCfg->C29_Gain1;
	pToneTable->C29_Gain2 = pToneCfg->C29_Gain2;
	pToneTable->C29_Gain3 = pToneCfg->C29_Gain3;
	pToneTable->C30_Freq0 = pToneCfg->C30_Freq0;
	pToneTable->C30_Freq1 = pToneCfg->C30_Freq1;
	pToneTable->C30_Freq2 = pToneCfg->C30_Freq2;
	pToneTable->C30_Freq3 = pToneCfg->C30_Freq3;
	pToneTable->C30_Gain0 = pToneCfg->C30_Gain0;
	pToneTable->C30_Gain1 = pToneCfg->C30_Gain1;
	pToneTable->C30_Gain2 = pToneCfg->C30_Gain2;
	pToneTable->C30_Gain3 = pToneCfg->C30_Gain3;
	pToneTable->C31_Freq0 = pToneCfg->C31_Freq0;
	pToneTable->C31_Freq1 = pToneCfg->C31_Freq1;
	pToneTable->C31_Freq2 = pToneCfg->C31_Freq2;
	pToneTable->C31_Freq3 = pToneCfg->C31_Freq3;
	pToneTable->C31_Gain0 = pToneCfg->C31_Gain0;
	pToneTable->C31_Gain1 = pToneCfg->C31_Gain1;
	pToneTable->C31_Gain2 = pToneCfg->C31_Gain2;
	pToneTable->C31_Gain3 = pToneCfg->C31_Gain3;

#endif
	return DSPCODEC_SUCCESS;
}
#endif	/* #ifdef SUPPORT_TONE_PROFILE */
//user space tone(DSPCODEC_TONE) to kernel space tone(TONES) convert table
static TONES tone_convert_table[]={
	DIGIT_0,
	DIGIT_1,
	DIGIT_2,
	DIGIT_3,
	DIGIT_4,
	DIGIT_5,
	DIGIT_6,
	DIGIT_7,
	DIGIT_8,
	DIGIT_9,
	DIGIT_STAR,
	DIGIT_PONDA,
#ifdef SUPPORT_TONE_PROFILE
	USA_DIAL,
	USA_STUTTERDIAL,
	USA_MESSAGE_WAITING,
	USA_CONFIRMATION,
	USA_RING,
	USA_BUSY,
	USA_CONGESTION,
	USA_ROH,
	USA_DOUBLE_RING,
	USA_SIT_NOCIRCUIT,
	USA_SIT_INTERCEPT,
	USA_SIT_VACANT,
	USA_SIT_REORDER,
	USA_CALLING_CARD_WITHEVENT,
	USA_CALLING_CARD,
	USA_CALL_WAITING_1,
	USA_CALL_WAITING_2,
	USA_CALL_WAITING_3,
	USA_CALL_WAITING_4,
	USA_EXTEND_1,
	USA_EXTEND_2,
	USA_EXTEND_3,
	USA_EXTEND_4,
	USA_EXTEND_5,
	USA_INGRESS_RINGBACK,

	HOLDING,
	OFFHOOKWARING,
	RING_1,
#else
  #error "only support tone profile"
#endif
	RING_1,
	RING_2,
	RING_3,
	RING_4,
	RING_5,
	RING_6,
	RING_7,
	RING_8,
	RING_9,
	RING_10,
	CUSTOM_TONE1,
	CUSTOM_TONE2,
	CUSTOM_TONE3,
	CUSTOM_TONE4,
	CUSTOM_TONE5,
	CUSTOM_TONE6,
	CUSTOM_TONE7,
	CUSTOM_TONE8,
	DIGIT_A,
	DIGIT_B,
	DIGIT_C,
	DIGIT_D,
	FSK_SAS,
	FSK_ALERT,
	FSK_MUTE,
	NTT_IIT_TONE,
	DIGIT_0_CONT,
	DIGIT_1_CONT,
	DIGIT_2_CONT,
	DIGIT_3_CONT,
	DIGIT_4_CONT,
	DIGIT_5_CONT,
	DIGIT_6_CONT,
	DIGIT_7_CONT,
	DIGIT_8_CONT,
	DIGIT_9_CONT,
	DIGIT_STAR_CONT,
	DIGIT_PONDA_CONT,
	DIGIT_A_CONT,
	DIGIT_B_CONT,
	DIGIT_C_CONT,
	DIGIT_D_CONT,
	VBD_ANS,
	VBD_ANS_BAR,
	VBD_ANSAM,
	VBD_ANSAM_BAR,
	VBD_CNG,
	VBD_CRE,
	USER_DEFINE1,
	USER_DEFINE2,
	USER_DEFINE3,
	USER_DEFINE4,
	USER_DEFINE5,
	DIGIT_PONDA,
};

static void tone_convert_table_update_country(DSPCODEC_COUNTRY country)
{
	int start;
	int i;

	switch ( country )
	{
		default:
		case DSPCODEC_COUNTRY_USA:
			start = USA_DIAL;
			break;
		case DSPCODEC_COUNTRY_UK:
			start = UK_DIAL;
			break;
		case DSPCODEC_COUNTRY_AUSTRALIA:
			start = AUSTRALIA_DIAL;
			break;
		case DSPCODEC_COUNTRY_HK:
			start = HK_DIAL;
			break;
		case DSPCODEC_COUNTRY_JP:
			start = JP_DIAL;
			break;
		case DSPCODEC_COUNTRY_SE:
			start = SE_DIAL;
			break;
		case DSPCODEC_COUNTRY_GR:
			start = GR_DIAL;
			break;
		case DSPCODEC_COUNTRY_FR:
			start = FR_DIAL;
			break;
		case DSPCODEC_COUNTRY_TW:
			start = TW_DIAL;
			break;
		case DSPCODEC_COUNTRY_BE:
			start = BE_DIAL;
			break;
		case DSPCODEC_COUNTRY_FL:
			start = FL_DIAL;
			break;
		case DSPCODEC_COUNTRY_IT:
			start = IT_DIAL;
			break;
		case DSPCODEC_COUNTRY_CN:
			start = CN_DIAL;
			break;
		case DSPCODEC_COUNTRY_EX1:
			start = EX1_DIAL;
			break;
		case DSPCODEC_COUNTRY_EX2:
			start = EX2_DIAL;
			break;
		case DSPCODEC_COUNTRY_EX3:
			start = EX3_DIAL;
			break;
		case DSPCODEC_COUNTRY_EX4:
			start = EX4_DIAL;
			break;
#ifdef COUNTRY_TONE_RESERVED
		case DSPCODEC_COUNTRY_RESERVE:
			start = RESERVE_DIAL;
			break;
#endif
		case DSPCODEC_COUNTRY_CUSTOME:
			start = CUSTOM_DIAL;
			break;
	}
	for (i=0 ; i<25 ; i++) {
		tone_convert_table[DSPCODEC_TONE_DIAL+i]=start+i;
	}
}



/*	handsome add function 2005.12.1     */
int32 setTone(DSPCODEC_COUNTRY country, DSPCODEC_TONE tone, aspToneCfgParam_t* pToneCfg)//thlin modify
{
	ToneCfgParam_t *pToneTable;
	unsigned long flags;
#ifdef SUPPORT_TONE_PROFILE
	tone_convert_table_update_country( country );
	if((tone < DSPCODEC_TONE_0) || (tone > DSPCODEC_TONE_KEY)){
		tone = DSPCODEC_TONE_0;
	}
	pToneTable = (ToneCfgParam_t *)&ToneTable[tone_convert_table[tone]];
//disable interrupt at update tone parameter.
	//ToneCfgParam_t *pToneTable = (ToneCfgParam_t *)&ToneTable[CUSTOM_TONE1+cust];
	local_irq_save(flags);
	pToneTable->ToneType = pToneCfg->toneType;
	pToneTable->cycle = pToneCfg->cycle;
	pToneTable->cadNUM = pToneCfg->cadNUM;
	pToneTable->CadOn0 = pToneCfg->CadOn0;
	pToneTable->CadOff0 = pToneCfg->CadOff0;
	pToneTable->CadOn1 = pToneCfg->CadOn1;
	pToneTable->CadOff1 = pToneCfg->CadOff1;
	pToneTable->CadOn2 = pToneCfg->CadOn2;
	pToneTable->CadOff2 = pToneCfg->CadOff2;
	pToneTable->CadOn3 = pToneCfg->CadOn3;
	pToneTable->CadOff3 = pToneCfg->CadOff3;
	pToneTable->PatternOff = pToneCfg->PatternOff;
	pToneTable->ToneNUM = pToneCfg->ToneNUM;
	pToneTable->Freq0 = pToneCfg->Freq0;
	pToneTable->Freq1 = pToneCfg->Freq1;
	pToneTable->Freq2 = pToneCfg->Freq2;
	pToneTable->Freq3 = pToneCfg->Freq3;
	pToneTable->Gain0 = pToneCfg->Gain0;
	pToneTable->Gain1 = pToneCfg->Gain1;
	pToneTable->Gain2 = pToneCfg->Gain2;
	pToneTable->Gain3 = pToneCfg->Gain3;
	pToneTable->C1_Freq0 = pToneCfg->C1_Freq0;
	pToneTable->C1_Freq1 = pToneCfg->C1_Freq1;
	pToneTable->C1_Freq2 = pToneCfg->C1_Freq2;
	pToneTable->C1_Freq3 = pToneCfg->C1_Freq3;
	pToneTable->C1_Gain0 = pToneCfg->C1_Gain0;
	pToneTable->C1_Gain1 = pToneCfg->C1_Gain1;
	pToneTable->C1_Gain2 = pToneCfg->C1_Gain2;
	pToneTable->C1_Gain3 = pToneCfg->C1_Gain3;
	pToneTable->C2_Freq0 = pToneCfg->C2_Freq0;
	pToneTable->C2_Freq1 = pToneCfg->C2_Freq1;
	pToneTable->C2_Freq2 = pToneCfg->C2_Freq2;
	pToneTable->C2_Freq3 = pToneCfg->C2_Freq3;
	pToneTable->C2_Gain0 = pToneCfg->C2_Gain0;
	pToneTable->C2_Gain1 = pToneCfg->C2_Gain1;
	pToneTable->C2_Gain2 = pToneCfg->C2_Gain2;
	pToneTable->C2_Gain3 = pToneCfg->C2_Gain3;
	pToneTable->C3_Freq0 = pToneCfg->C3_Freq0;
	pToneTable->C3_Freq1 = pToneCfg->C3_Freq1;
	pToneTable->C3_Freq2 = pToneCfg->C3_Freq2;
	pToneTable->C3_Freq3 = pToneCfg->C3_Freq3;
	pToneTable->C3_Gain0 = pToneCfg->C3_Gain0;
	pToneTable->C3_Gain1 = pToneCfg->C3_Gain1;
	pToneTable->C3_Gain2 = pToneCfg->C3_Gain2;
	pToneTable->C3_Gain3 = pToneCfg->C3_Gain3;

	pToneTable->CadOn4 = pToneCfg->CadOn4;
	pToneTable->CadOff4 = pToneCfg->CadOff4;
	pToneTable->CadOn5 = pToneCfg->CadOn5;
	pToneTable->CadOff5 = pToneCfg->CadOff5;
	pToneTable->CadOn6 = pToneCfg->CadOn6;
	pToneTable->CadOff6 = pToneCfg->CadOff6;
	pToneTable->CadOn7 = pToneCfg->CadOn7;
	pToneTable->CadOff7 = pToneCfg->CadOff7;
	pToneTable->C4_Freq0 = pToneCfg->C4_Freq0;
	pToneTable->C4_Freq1 = pToneCfg->C4_Freq1;
	pToneTable->C4_Freq2 = pToneCfg->C4_Freq2;
	pToneTable->C4_Freq3 = pToneCfg->C4_Freq3;
	pToneTable->C4_Gain0 = pToneCfg->C4_Gain0;
	pToneTable->C4_Gain1 = pToneCfg->C4_Gain1;
	pToneTable->C4_Gain2 = pToneCfg->C4_Gain2;
	pToneTable->C4_Gain3 = pToneCfg->C4_Gain3;
	pToneTable->C5_Freq0 = pToneCfg->C5_Freq0;
	pToneTable->C5_Freq1 = pToneCfg->C5_Freq1;
	pToneTable->C5_Freq2 = pToneCfg->C5_Freq2;
	pToneTable->C5_Freq3 = pToneCfg->C5_Freq3;
	pToneTable->C5_Gain0 = pToneCfg->C5_Gain0;
	pToneTable->C5_Gain1 = pToneCfg->C5_Gain1;
	pToneTable->C5_Gain2 = pToneCfg->C5_Gain2;
	pToneTable->C5_Gain3 = pToneCfg->C5_Gain3;
	pToneTable->C6_Freq0 = pToneCfg->C6_Freq0;
	pToneTable->C6_Freq1 = pToneCfg->C6_Freq1;
	pToneTable->C6_Freq2 = pToneCfg->C6_Freq2;
	pToneTable->C6_Freq3 = pToneCfg->C6_Freq3;
	pToneTable->C6_Gain0 = pToneCfg->C6_Gain0;
	pToneTable->C6_Gain1 = pToneCfg->C6_Gain1;
	pToneTable->C6_Gain2 = pToneCfg->C6_Gain2;
	pToneTable->C6_Gain3 = pToneCfg->C6_Gain3;
	pToneTable->C7_Freq0 = pToneCfg->C7_Freq0;
	pToneTable->C7_Freq1 = pToneCfg->C7_Freq1;
	pToneTable->C7_Freq2 = pToneCfg->C7_Freq2;
	pToneTable->C7_Freq3 = pToneCfg->C7_Freq3;
	pToneTable->C7_Gain0 = pToneCfg->C7_Gain0;
	pToneTable->C7_Gain1 = pToneCfg->C7_Gain1;
	pToneTable->C7_Gain2 = pToneCfg->C7_Gain2;
	pToneTable->C7_Gain3 = pToneCfg->C7_Gain3;

	pToneTable->CadOn8 = pToneCfg->CadOn8;
	pToneTable->CadOff8 = pToneCfg->CadOff8;
	pToneTable->CadOn9 = pToneCfg->CadOn9;
	pToneTable->CadOff9 = pToneCfg->CadOff9;
	pToneTable->CadOn10 = pToneCfg->CadOn10;
	pToneTable->CadOff10 = pToneCfg->CadOff10;
	pToneTable->CadOn11 = pToneCfg->CadOn11;
	pToneTable->CadOff11 = pToneCfg->CadOff11;
	pToneTable->C8_Freq0 = pToneCfg->C8_Freq0;
	pToneTable->C8_Freq1 = pToneCfg->C8_Freq1;
	pToneTable->C8_Freq2 = pToneCfg->C8_Freq2;
	pToneTable->C8_Freq3 = pToneCfg->C8_Freq3;
	pToneTable->C8_Gain0 = pToneCfg->C8_Gain0;
	pToneTable->C8_Gain1 = pToneCfg->C8_Gain1;
	pToneTable->C8_Gain2 = pToneCfg->C8_Gain2;
	pToneTable->C8_Gain3 = pToneCfg->C8_Gain3;
	pToneTable->C9_Freq0 = pToneCfg->C9_Freq0;
	pToneTable->C9_Freq1 = pToneCfg->C9_Freq1;
	pToneTable->C9_Freq2 = pToneCfg->C9_Freq2;
	pToneTable->C9_Freq3 = pToneCfg->C9_Freq3;
	pToneTable->C9_Gain0 = pToneCfg->C9_Gain0;
	pToneTable->C9_Gain1 = pToneCfg->C9_Gain1;
	pToneTable->C9_Gain2 = pToneCfg->C9_Gain2;
	pToneTable->C9_Gain3 = pToneCfg->C9_Gain3;
	pToneTable->C10_Freq0 = pToneCfg->C10_Freq0;
	pToneTable->C10_Freq1 = pToneCfg->C10_Freq1;
	pToneTable->C10_Freq2 = pToneCfg->C10_Freq2;
	pToneTable->C10_Freq3 = pToneCfg->C10_Freq3;
	pToneTable->C10_Gain0 = pToneCfg->C10_Gain0;
	pToneTable->C10_Gain1 = pToneCfg->C10_Gain1;
	pToneTable->C10_Gain2 = pToneCfg->C10_Gain2;
	pToneTable->C10_Gain3 = pToneCfg->C10_Gain3;
	pToneTable->C11_Freq0 = pToneCfg->C11_Freq0;
	pToneTable->C11_Freq1 = pToneCfg->C11_Freq1;
	pToneTable->C11_Freq2 = pToneCfg->C11_Freq2;
	pToneTable->C11_Freq3 = pToneCfg->C11_Freq3;
	pToneTable->C11_Gain0 = pToneCfg->C11_Gain0;
	pToneTable->C11_Gain1 = pToneCfg->C11_Gain1;
	pToneTable->C11_Gain2 = pToneCfg->C11_Gain2;
	pToneTable->C11_Gain3 = pToneCfg->C11_Gain3;

	pToneTable->CadOn12 = pToneCfg->CadOn12;
	pToneTable->CadOff12 = pToneCfg->CadOff12;
	pToneTable->CadOn13 = pToneCfg->CadOn13;
	pToneTable->CadOff13 = pToneCfg->CadOff13;
	pToneTable->CadOn14 = pToneCfg->CadOn14;
	pToneTable->CadOff14 = pToneCfg->CadOff14;
	pToneTable->CadOn15 = pToneCfg->CadOn15;
	pToneTable->CadOff15 = pToneCfg->CadOff15;
	pToneTable->C12_Freq0 = pToneCfg->C12_Freq0;
	pToneTable->C12_Freq1 = pToneCfg->C12_Freq1;
	pToneTable->C12_Freq2 = pToneCfg->C12_Freq2;
	pToneTable->C12_Freq3 = pToneCfg->C12_Freq3;
	pToneTable->C12_Gain0 = pToneCfg->C12_Gain0;
	pToneTable->C12_Gain1 = pToneCfg->C12_Gain1;
	pToneTable->C12_Gain2 = pToneCfg->C12_Gain2;
	pToneTable->C12_Gain3 = pToneCfg->C12_Gain3;
	pToneTable->C13_Freq0 = pToneCfg->C13_Freq0;
	pToneTable->C13_Freq1 = pToneCfg->C13_Freq1;
	pToneTable->C13_Freq2 = pToneCfg->C13_Freq2;
	pToneTable->C13_Freq3 = pToneCfg->C13_Freq3;
	pToneTable->C13_Gain0 = pToneCfg->C13_Gain0;
	pToneTable->C13_Gain1 = pToneCfg->C13_Gain1;
	pToneTable->C13_Gain2 = pToneCfg->C13_Gain2;
	pToneTable->C13_Gain3 = pToneCfg->C13_Gain3;
	pToneTable->C14_Freq0 = pToneCfg->C14_Freq0;
	pToneTable->C14_Freq1 = pToneCfg->C14_Freq1;
	pToneTable->C14_Freq2 = pToneCfg->C14_Freq2;
	pToneTable->C14_Freq3 = pToneCfg->C14_Freq3;
	pToneTable->C14_Gain0 = pToneCfg->C14_Gain0;
	pToneTable->C14_Gain1 = pToneCfg->C14_Gain1;
	pToneTable->C14_Gain2 = pToneCfg->C14_Gain2;
	pToneTable->C14_Gain3 = pToneCfg->C14_Gain3;
	pToneTable->C15_Freq0 = pToneCfg->C15_Freq0;
	pToneTable->C15_Freq1 = pToneCfg->C15_Freq1;
	pToneTable->C15_Freq2 = pToneCfg->C15_Freq2;
	pToneTable->C15_Freq3 = pToneCfg->C15_Freq3;
	pToneTable->C15_Gain0 = pToneCfg->C15_Gain0;
	pToneTable->C15_Gain1 = pToneCfg->C15_Gain1;
	pToneTable->C15_Gain2 = pToneCfg->C15_Gain2;
	pToneTable->C15_Gain3 = pToneCfg->C15_Gain3;

	pToneTable->CadOn16 = pToneCfg->CadOn16;
	pToneTable->CadOff16 = pToneCfg->CadOff16;
	pToneTable->CadOn17 = pToneCfg->CadOn17;
	pToneTable->CadOff17 = pToneCfg->CadOff17;
	pToneTable->CadOn18 = pToneCfg->CadOn18;
	pToneTable->CadOff18 = pToneCfg->CadOff18;
	pToneTable->CadOn19 = pToneCfg->CadOn19;
	pToneTable->CadOff19 = pToneCfg->CadOff19;
	pToneTable->C16_Freq0 = pToneCfg->C16_Freq0;
	pToneTable->C16_Freq1 = pToneCfg->C16_Freq1;
	pToneTable->C16_Freq2 = pToneCfg->C16_Freq2;
	pToneTable->C16_Freq3 = pToneCfg->C16_Freq3;
	pToneTable->C16_Gain0 = pToneCfg->C16_Gain0;
	pToneTable->C16_Gain1 = pToneCfg->C16_Gain1;
	pToneTable->C16_Gain2 = pToneCfg->C16_Gain2;
	pToneTable->C16_Gain3 = pToneCfg->C16_Gain3;
	pToneTable->C17_Freq0 = pToneCfg->C17_Freq0;
	pToneTable->C17_Freq1 = pToneCfg->C17_Freq1;
	pToneTable->C17_Freq2 = pToneCfg->C17_Freq2;
	pToneTable->C17_Freq3 = pToneCfg->C17_Freq3;
	pToneTable->C17_Gain0 = pToneCfg->C17_Gain0;
	pToneTable->C17_Gain1 = pToneCfg->C17_Gain1;
	pToneTable->C17_Gain2 = pToneCfg->C17_Gain2;
	pToneTable->C17_Gain3 = pToneCfg->C17_Gain3;
	pToneTable->C18_Freq0 = pToneCfg->C18_Freq0;
	pToneTable->C18_Freq1 = pToneCfg->C18_Freq1;
	pToneTable->C18_Freq2 = pToneCfg->C18_Freq2;
	pToneTable->C18_Freq3 = pToneCfg->C18_Freq3;
	pToneTable->C18_Gain0 = pToneCfg->C18_Gain0;
	pToneTable->C18_Gain1 = pToneCfg->C18_Gain1;
	pToneTable->C18_Gain2 = pToneCfg->C18_Gain2;
	pToneTable->C18_Gain3 = pToneCfg->C18_Gain3;
	pToneTable->C19_Freq0 = pToneCfg->C19_Freq0;
	pToneTable->C19_Freq1 = pToneCfg->C19_Freq1;
	pToneTable->C19_Freq2 = pToneCfg->C19_Freq2;
	pToneTable->C19_Freq3 = pToneCfg->C19_Freq3;
	pToneTable->C19_Gain0 = pToneCfg->C19_Gain0;
	pToneTable->C19_Gain1 = pToneCfg->C19_Gain1;
	pToneTable->C19_Gain2 = pToneCfg->C19_Gain2;
	pToneTable->C19_Gain3 = pToneCfg->C19_Gain3;

	pToneTable->CadOn20 = pToneCfg->CadOn20;
	pToneTable->CadOff20 = pToneCfg->CadOff20;
	pToneTable->CadOn21 = pToneCfg->CadOn21;
	pToneTable->CadOff21 = pToneCfg->CadOff21;
	pToneTable->CadOn22 = pToneCfg->CadOn22;
	pToneTable->CadOff22 = pToneCfg->CadOff22;
	pToneTable->CadOn23 = pToneCfg->CadOn23;
	pToneTable->CadOff23 = pToneCfg->CadOff23;
	pToneTable->C20_Freq0 = pToneCfg->C20_Freq0;
	pToneTable->C20_Freq1 = pToneCfg->C20_Freq1;
	pToneTable->C20_Freq2 = pToneCfg->C20_Freq2;
	pToneTable->C20_Freq3 = pToneCfg->C20_Freq3;
	pToneTable->C20_Gain0 = pToneCfg->C20_Gain0;
	pToneTable->C20_Gain1 = pToneCfg->C20_Gain1;
	pToneTable->C20_Gain2 = pToneCfg->C20_Gain2;
	pToneTable->C20_Gain3 = pToneCfg->C20_Gain3;
	pToneTable->C21_Freq0 = pToneCfg->C21_Freq0;
	pToneTable->C21_Freq1 = pToneCfg->C21_Freq1;
	pToneTable->C21_Freq2 = pToneCfg->C21_Freq2;
	pToneTable->C21_Freq3 = pToneCfg->C21_Freq3;
	pToneTable->C21_Gain0 = pToneCfg->C21_Gain0;
	pToneTable->C21_Gain1 = pToneCfg->C21_Gain1;
	pToneTable->C21_Gain2 = pToneCfg->C21_Gain2;
	pToneTable->C21_Gain3 = pToneCfg->C21_Gain3;
	pToneTable->C22_Freq0 = pToneCfg->C22_Freq0;
	pToneTable->C22_Freq1 = pToneCfg->C22_Freq1;
	pToneTable->C22_Freq2 = pToneCfg->C22_Freq2;
	pToneTable->C22_Freq3 = pToneCfg->C22_Freq3;
	pToneTable->C22_Gain0 = pToneCfg->C22_Gain0;
	pToneTable->C22_Gain1 = pToneCfg->C22_Gain1;
	pToneTable->C22_Gain2 = pToneCfg->C22_Gain2;
	pToneTable->C22_Gain3 = pToneCfg->C22_Gain3;
	pToneTable->C23_Freq0 = pToneCfg->C23_Freq0;
	pToneTable->C23_Freq1 = pToneCfg->C23_Freq1;
	pToneTable->C23_Freq2 = pToneCfg->C23_Freq2;
	pToneTable->C23_Freq3 = pToneCfg->C23_Freq3;
	pToneTable->C23_Gain0 = pToneCfg->C23_Gain0;
	pToneTable->C23_Gain1 = pToneCfg->C23_Gain1;
	pToneTable->C23_Gain2 = pToneCfg->C23_Gain2;
	pToneTable->C23_Gain3 = pToneCfg->C23_Gain3;

	pToneTable->CadOn24 = pToneCfg->CadOn24;
	pToneTable->CadOff24 = pToneCfg->CadOff24;
	pToneTable->CadOn25 = pToneCfg->CadOn25;
	pToneTable->CadOff25 = pToneCfg->CadOff25;
	pToneTable->CadOn26 = pToneCfg->CadOn26;
	pToneTable->CadOff26 = pToneCfg->CadOff26;
	pToneTable->CadOn27 = pToneCfg->CadOn27;
	pToneTable->CadOff27 = pToneCfg->CadOff27;
	pToneTable->C24_Freq0 = pToneCfg->C24_Freq0;
	pToneTable->C24_Freq1 = pToneCfg->C24_Freq1;
	pToneTable->C24_Freq2 = pToneCfg->C24_Freq2;
	pToneTable->C24_Freq3 = pToneCfg->C24_Freq3;
	pToneTable->C24_Gain0 = pToneCfg->C24_Gain0;
	pToneTable->C24_Gain1 = pToneCfg->C24_Gain1;
	pToneTable->C24_Gain2 = pToneCfg->C24_Gain2;
	pToneTable->C24_Gain3 = pToneCfg->C24_Gain3;
	pToneTable->C25_Freq0 = pToneCfg->C25_Freq0;
	pToneTable->C25_Freq1 = pToneCfg->C25_Freq1;
	pToneTable->C25_Freq2 = pToneCfg->C25_Freq2;
	pToneTable->C25_Freq3 = pToneCfg->C25_Freq3;
	pToneTable->C25_Gain0 = pToneCfg->C25_Gain0;
	pToneTable->C25_Gain1 = pToneCfg->C25_Gain1;
	pToneTable->C25_Gain2 = pToneCfg->C25_Gain2;
	pToneTable->C25_Gain3 = pToneCfg->C25_Gain3;
	pToneTable->C26_Freq0 = pToneCfg->C26_Freq0;
	pToneTable->C26_Freq1 = pToneCfg->C26_Freq1;
	pToneTable->C26_Freq2 = pToneCfg->C26_Freq2;
	pToneTable->C26_Freq3 = pToneCfg->C26_Freq3;
	pToneTable->C26_Gain0 = pToneCfg->C26_Gain0;
	pToneTable->C26_Gain1 = pToneCfg->C26_Gain1;
	pToneTable->C26_Gain2 = pToneCfg->C26_Gain2;
	pToneTable->C26_Gain3 = pToneCfg->C26_Gain3;
	pToneTable->C27_Freq0 = pToneCfg->C27_Freq0;
	pToneTable->C27_Freq1 = pToneCfg->C27_Freq1;
	pToneTable->C27_Freq2 = pToneCfg->C27_Freq2;
	pToneTable->C27_Freq3 = pToneCfg->C27_Freq3;
	pToneTable->C27_Gain0 = pToneCfg->C27_Gain0;
	pToneTable->C27_Gain1 = pToneCfg->C27_Gain1;
	pToneTable->C27_Gain2 = pToneCfg->C27_Gain2;
	pToneTable->C27_Gain3 = pToneCfg->C27_Gain3;

	pToneTable->CadOn28 = pToneCfg->CadOn28;
	pToneTable->CadOff28 = pToneCfg->CadOff28;
	pToneTable->CadOn29 = pToneCfg->CadOn29;
	pToneTable->CadOff29 = pToneCfg->CadOff29;
	pToneTable->CadOn30 = pToneCfg->CadOn30;
	pToneTable->CadOff30 = pToneCfg->CadOff30;
	pToneTable->CadOn31 = pToneCfg->CadOn31;
	pToneTable->CadOff31 = pToneCfg->CadOff31;
	pToneTable->C28_Freq0 = pToneCfg->C28_Freq0;
	pToneTable->C28_Freq1 = pToneCfg->C28_Freq1;
	pToneTable->C28_Freq2 = pToneCfg->C28_Freq2;
	pToneTable->C28_Freq3 = pToneCfg->C28_Freq3;
	pToneTable->C28_Gain0 = pToneCfg->C28_Gain0;
	pToneTable->C28_Gain1 = pToneCfg->C28_Gain1;
	pToneTable->C28_Gain2 = pToneCfg->C28_Gain2;
	pToneTable->C28_Gain3 = pToneCfg->C28_Gain3;
	pToneTable->C29_Freq0 = pToneCfg->C29_Freq0;
	pToneTable->C29_Freq1 = pToneCfg->C29_Freq1;
	pToneTable->C29_Freq2 = pToneCfg->C29_Freq2;
	pToneTable->C29_Freq3 = pToneCfg->C29_Freq3;
	pToneTable->C29_Gain0 = pToneCfg->C29_Gain0;
	pToneTable->C29_Gain1 = pToneCfg->C29_Gain1;
	pToneTable->C29_Gain2 = pToneCfg->C29_Gain2;
	pToneTable->C29_Gain3 = pToneCfg->C29_Gain3;
	pToneTable->C30_Freq0 = pToneCfg->C30_Freq0;
	pToneTable->C30_Freq1 = pToneCfg->C30_Freq1;
	pToneTable->C30_Freq2 = pToneCfg->C30_Freq2;
	pToneTable->C30_Freq3 = pToneCfg->C30_Freq3;
	pToneTable->C30_Gain0 = pToneCfg->C30_Gain0;
	pToneTable->C30_Gain1 = pToneCfg->C30_Gain1;
	pToneTable->C30_Gain2 = pToneCfg->C30_Gain2;
	pToneTable->C30_Gain3 = pToneCfg->C30_Gain3;
	pToneTable->C31_Freq0 = pToneCfg->C31_Freq0;
	pToneTable->C31_Freq1 = pToneCfg->C31_Freq1;
	pToneTable->C31_Freq2 = pToneCfg->C31_Freq2;
	pToneTable->C31_Freq3 = pToneCfg->C31_Freq3;
	pToneTable->C31_Gain0 = pToneCfg->C31_Gain0;
	pToneTable->C31_Gain1 = pToneCfg->C31_Gain1;
	pToneTable->C31_Gain2 = pToneCfg->C31_Gain2;
	pToneTable->C31_Gain3 = pToneCfg->C31_Gain3;
	local_irq_restore(flags);
#if 0   //thlin
	if(cust == dialTone)
		DspcodecSetCustomTone(DSPCODEC_TONE_DIAL, (ToneCfgParam_t *)pToneTable);
	if(cust == ringTone)
		DspcodecSetCustomTone(DSPCODEC_TONE_RINGING, (ToneCfgParam_t *)pToneTable);
	if(cust == busyTone)
		DspcodecSetCustomTone(DSPCODEC_TONE_BUSY, (ToneCfgParam_t *)pToneTable);
	if(cust == waitingTone)
		DspcodecSetCustomTone(DSPCODEC_TONE_CALL_WAITING, (ToneCfgParam_t *)pToneTable);
#endif

#else
	#error "deprecated, jwsyu 20121024 "
	ToneTable[CUSTOM_TONE1+cust][0]=pToneCfg->toneType;
	ToneTable[CUSTOM_TONE1+cust][1]=pToneCfg->cycle;
	ToneTable[CUSTOM_TONE1+cust][2]=pToneCfg->cadNUM;
	ToneTable[CUSTOM_TONE1+cust][3]=pToneCfg->CadOn0;
	ToneTable[CUSTOM_TONE1+cust][4]=pToneCfg->CadOn1;
	ToneTable[CUSTOM_TONE1+cust][5]=pToneCfg->CadOn2;
	ToneTable[CUSTOM_TONE1+cust][6]=pToneCfg->CadOn3;
	ToneTable[CUSTOM_TONE1+cust][7]=pToneCfg->CadOff0;
	ToneTable[CUSTOM_TONE1+cust][8]=pToneCfg->CadOff1;
	ToneTable[CUSTOM_TONE1+cust][9]=pToneCfg->CadOff2;
	ToneTable[CUSTOM_TONE1+cust][10]=pToneCfg->CadOff3;
	ToneTable[CUSTOM_TONE1+cust][11]=pToneCfg->PatternOff;
	ToneTable[CUSTOM_TONE1+cust][12]=pToneCfg->ToneNUM;
	ToneTable[CUSTOM_TONE1+cust][13]=pToneCfg->Gain0;
	ToneTable[CUSTOM_TONE1+cust][14]=pToneCfg->Gain1;
	ToneTable[CUSTOM_TONE1+cust][15]=pToneCfg->Gain2;
	ToneTable[CUSTOM_TONE1+cust][16]=pToneCfg->Gain3;
	ToneTable[CUSTOM_TONE1+cust][17]=pToneCfg->Freq0;
	ToneTable[CUSTOM_TONE1+cust][18]=pToneCfg->Freq1;
	ToneTable[CUSTOM_TONE1+cust][19]=pToneCfg->Freq2;
	ToneTable[CUSTOM_TONE1+cust][20]=pToneCfg->Freq3;
#endif
	return SUCCESS;
}

/*	handsome add function 2005.12.1     */
#if 0
int32 setToneCountry(gDSP_aspToneCountry_t* pCfg)
{
	uint32 sid;
#ifdef SUPPORT_TONE_PROFILE
	short *pToneTable = NULL;
#endif

	iCountry = pCfg->CountryId;

	if(dialTone != pCfg->iDial)
	{
	dialTone = pCfg->iDial;
#ifdef SUPPORT_TONE_PROFILE
		pToneTable = (short*)&ToneTable[CUSTOM_TONE1+dialTone];
		DspcodecSetCustomTone(DSPCODEC_TONE_DIAL, (ToneCfgParam_t *)pToneTable);
#endif
	}
	if(ringTone != pCfg->iRing)
	{
 	ringTone = pCfg->iRing;
#ifdef SUPPORT_TONE_PROFILE
		pToneTable = (short*)&ToneTable[CUSTOM_TONE1+dialTone];
		DspcodecSetCustomTone(DSPCODEC_TONE_RINGING, (ToneCfgParam_t *)pToneTable);
#endif
	}
	if(busyTone != pCfg->iBusy)
	{
	busyTone = pCfg->iBusy;
#ifdef SUPPORT_TONE_PROFILE
		pToneTable = (short*)&ToneTable[CUSTOM_TONE1+dialTone];
		DspcodecSetCustomTone(DSPCODEC_TONE_BUSY, (ToneCfgParam_t *)pToneTable);
#endif
	}
	if(waitingTone != pCfg->iWaiting)
	{
	waitingTone = pCfg->iWaiting;
#ifdef SUPPORT_TONE_PROFILE
		pToneTable = (short*)&ToneTable[CUSTOM_TONE1+dialTone];
		DspcodecSetCustomTone(DSPCODEC_TONE_CALL_WAITING, (ToneCfgParam_t *)pToneTable);
#endif
	}

//#ifdef SUPPORT_TONE_PROFILE
	for(sid=0; sid<dsp_rtk_ss_num; sid++)
		DspcodecSetCountry(sid, iCountry);
//#endif

	return SUCCESS;
}
#endif

char* country_table_str[]={"COUNTRY_USA", "COUNTRY_UK", "COUNTRY_AUSTRALIA",
                           "COUNTRY_HK",  "COUNTRY_JP", "COUNTRY_SE",
                           "COUNTRY_GR", "COUNTRY_FR",  "COUNTRY_TW",
                           "COUNTRY_BE", "COUNTRY_FL","COUNTRY_IT", "COUNTRY_CN",
                           "COUNTRY_EX1", "COUNTRY_EX2", "COUNTRY_EX3", "COUNTRY_EX4",
#ifdef COUNTRY_TONE_RESERVED
                            "COUNTRY_RESERVE", "COUNTRY_CUSTOME", "COUNTRY_MAX"
#else
                            "COUNTRY_CUSTOME", "COUNTRY_MAX"
#endif
                              };
			    
			    
char* tone_type_str[]={"ADDITIVE", "MODULATED", "SUCC", "SUCC_ADD", "RING_SUCC", "RING_SUCC_ADD", "FOUR_FREQ", "STEP_INC", "TWO_STEP"};

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int voip_tone_read(struct seq_file *f, void *v)
{
	int len;
	char tmp[1024] = {0};
	int index = 0;
	ToneCfgParam_t *pToneTable = NULL;

	seq_printf( f, "{%d} tone country: %d(%s) \n", TONE_ENTRY_MAX,
	                                tone_country, country_table_str[tone_country]);
	pToneTable = (ToneCfgParam_t *)&ToneTable[Dsp_data[0].m_nBaseTone];
	seq_printf( f, "dial tone: type(%s) \n", 
	                  tone_type_str[pToneTable->ToneType]);
	seq_printf( f, "          freq num(%d)= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                pToneTable->ToneNUM, pToneTable->Freq0, pToneTable->Gain0, pToneTable->Freq1,
	                pToneTable->Gain1, pToneTable->Freq2, pToneTable->Gain2, pToneTable->Freq3,
	                pToneTable->Gain3);
	seq_printf( f, "          cad num(%d)= %don/%doff, %don/%doff, %don/%doff, %don/%doff \n",
	                pToneTable->cadNUM, pToneTable->CadOn0, pToneTable->CadOff0, pToneTable->CadOn1,
	                pToneTable->CadOff1, pToneTable->CadOn2, pToneTable->CadOff2, pToneTable->CadOn3,
	                pToneTable->CadOff3);
	pToneTable = (ToneCfgParam_t *)&ToneTable[Dsp_data[0].m_nBaseTone+1];
	seq_printf( f, "stutter dial tone: type(%s) \n", 
	                  tone_type_str[pToneTable->ToneType]);
	seq_printf( f, "          freq num(%d)= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                pToneTable->ToneNUM, pToneTable->Freq0, pToneTable->Gain0, pToneTable->Freq1,
	                pToneTable->Gain1, pToneTable->Freq2, pToneTable->Gain2, pToneTable->Freq3,
	                pToneTable->Gain3);
	seq_printf( f, "          cad num(%d)= %don/%doff, %don/%doff, %don/%doff, %don/%doff \n",
	                pToneTable->cadNUM, pToneTable->CadOn0, pToneTable->CadOff0, pToneTable->CadOn1,
	                pToneTable->CadOff1, pToneTable->CadOn2, pToneTable->CadOff2, pToneTable->CadOn3,
	                pToneTable->CadOff3);
	pToneTable = (ToneCfgParam_t *)&ToneTable[Dsp_data[0].m_nBaseTone+4];
	seq_printf( f, "ring tone: type(%s) \n", 
	                  tone_type_str[pToneTable->ToneType]);
	seq_printf( f, "          freq num(%d)= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                pToneTable->ToneNUM, pToneTable->Freq0, pToneTable->Gain0, pToneTable->Freq1,
	                pToneTable->Gain1, pToneTable->Freq2, pToneTable->Gain2, pToneTable->Freq3,
	                pToneTable->Gain3);
	seq_printf( f, "          cad num(%d)= %don/%doff, %don/%doff, %don/%doff, %don/%doff \n",
	                pToneTable->cadNUM, pToneTable->CadOn0, pToneTable->CadOff0, pToneTable->CadOn1,
	                pToneTable->CadOff1, pToneTable->CadOn2, pToneTable->CadOff2, pToneTable->CadOn3,
	                pToneTable->CadOff3);
	pToneTable = (ToneCfgParam_t *)&ToneTable[Dsp_data[0].m_nBaseTone+5];
	seq_printf( f, "busy tone: type(%s) \n", 
	                  tone_type_str[pToneTable->ToneType]);
	seq_printf( f, "          freq num(%d)= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                pToneTable->ToneNUM, pToneTable->Freq0, pToneTable->Gain0, pToneTable->Freq1,
	                pToneTable->Gain1, pToneTable->Freq2, pToneTable->Gain2, pToneTable->Freq3,
	                pToneTable->Gain3);
	seq_printf( f, "          cad num(%d)= %don/%doff, %don/%doff, %don/%doff, %don/%doff \n",
	                pToneTable->cadNUM, pToneTable->CadOn0, pToneTable->CadOff0, pToneTable->CadOn1,
	                pToneTable->CadOff1, pToneTable->CadOn2, pToneTable->CadOff2, pToneTable->CadOn3,
	                pToneTable->CadOff3);
	pToneTable = (ToneCfgParam_t *)&ToneTable[Dsp_data[0].m_nBaseTone+15];
	seq_printf( f, "call waiting tone: type(%s) \n", 
	                  tone_type_str[pToneTable->ToneType]);
	seq_printf( f, "          freq num(%d)= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                pToneTable->ToneNUM, pToneTable->Freq0, pToneTable->Gain0, pToneTable->Freq1,
	                pToneTable->Gain1, pToneTable->Freq2, pToneTable->Gain2, pToneTable->Freq3,
	                pToneTable->Gain3);
	seq_printf( f, "          cad num(%d)= %don/%doff, %don/%doff, %don/%doff, %don/%doff \n",
	                pToneTable->cadNUM, pToneTable->CadOn0, pToneTable->CadOff0, pToneTable->CadOn1,
	                pToneTable->CadOff1, pToneTable->CadOn2, pToneTable->CadOff2, pToneTable->CadOn3,
	                pToneTable->CadOff3);

	//seq_printf( f, page,tmp);

	return len;
}
#else
static int voip_tone_read(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{
	int len;
	char tmp[1024] = {0};
	int index = 0;
	ToneCfgParam_t *pToneTable = NULL;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}

	index += sprintf(tmp+index, "{%d} tone country: %d(%s) \n", TONE_ENTRY_MAX,
	                                tone_country, country_table_str[tone_country]);
	pToneTable = (ToneCfgParam_t *)&ToneTable[Dsp_data[0].m_nBaseTone];
	index += sprintf(tmp+index, "dial tone: type(%s) \n", 
	                  tone_type_str[pToneTable->ToneType]);
	index += sprintf(tmp+index, "          freq num(%d)= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                pToneTable->ToneNUM, pToneTable->Freq0, pToneTable->Gain0, pToneTable->Freq1,
	                pToneTable->Gain1, pToneTable->Freq2, pToneTable->Gain2, pToneTable->Freq3,
	                pToneTable->Gain3);
	index += sprintf(tmp+index, "          cad num(%d)= %don/%doff, %don/%doff, %don/%doff, %don/%doff \n",
	                pToneTable->cadNUM, pToneTable->CadOn0, pToneTable->CadOff0, pToneTable->CadOn1,
	                pToneTable->CadOff1, pToneTable->CadOn2, pToneTable->CadOff2, pToneTable->CadOn3,
	                pToneTable->CadOff3);
	pToneTable = (ToneCfgParam_t *)&ToneTable[Dsp_data[0].m_nBaseTone+1];
	index += sprintf(tmp+index, "stutter dial tone: type(%s) \n", 
	                  tone_type_str[pToneTable->ToneType]);
	index += sprintf(tmp+index, "          freq num(%d)= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                pToneTable->ToneNUM, pToneTable->Freq0, pToneTable->Gain0, pToneTable->Freq1,
	                pToneTable->Gain1, pToneTable->Freq2, pToneTable->Gain2, pToneTable->Freq3,
	                pToneTable->Gain3);
	index += sprintf(tmp+index, "          cad num(%d)= %don/%doff, %don/%doff, %don/%doff, %don/%doff \n",
	                pToneTable->cadNUM, pToneTable->CadOn0, pToneTable->CadOff0, pToneTable->CadOn1,
	                pToneTable->CadOff1, pToneTable->CadOn2, pToneTable->CadOff2, pToneTable->CadOn3,
	                pToneTable->CadOff3);
	pToneTable = (ToneCfgParam_t *)&ToneTable[Dsp_data[0].m_nBaseTone+4];
	index += sprintf(tmp+index, "ring tone: type(%s) \n", 
	                  tone_type_str[pToneTable->ToneType]);
	index += sprintf(tmp+index, "          freq num(%d)= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                pToneTable->ToneNUM, pToneTable->Freq0, pToneTable->Gain0, pToneTable->Freq1,
	                pToneTable->Gain1, pToneTable->Freq2, pToneTable->Gain2, pToneTable->Freq3,
	                pToneTable->Gain3);
	index += sprintf(tmp+index, "          cad num(%d)= %don/%doff, %don/%doff, %don/%doff, %don/%doff \n",
	                pToneTable->cadNUM, pToneTable->CadOn0, pToneTable->CadOff0, pToneTable->CadOn1,
	                pToneTable->CadOff1, pToneTable->CadOn2, pToneTable->CadOff2, pToneTable->CadOn3,
	                pToneTable->CadOff3);
	pToneTable = (ToneCfgParam_t *)&ToneTable[Dsp_data[0].m_nBaseTone+5];
	index += sprintf(tmp+index, "busy tone: type(%s) \n", 
	                  tone_type_str[pToneTable->ToneType]);
	index += sprintf(tmp+index, "          freq num(%d)= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                pToneTable->ToneNUM, pToneTable->Freq0, pToneTable->Gain0, pToneTable->Freq1,
	                pToneTable->Gain1, pToneTable->Freq2, pToneTable->Gain2, pToneTable->Freq3,
	                pToneTable->Gain3);
	index += sprintf(tmp+index, "          cad num(%d)= %don/%doff, %don/%doff, %don/%doff, %don/%doff \n",
	                pToneTable->cadNUM, pToneTable->CadOn0, pToneTable->CadOff0, pToneTable->CadOn1,
	                pToneTable->CadOff1, pToneTable->CadOn2, pToneTable->CadOff2, pToneTable->CadOn3,
	                pToneTable->CadOff3);
	pToneTable = (ToneCfgParam_t *)&ToneTable[Dsp_data[0].m_nBaseTone+15];
	index += sprintf(tmp+index, "call waiting tone: type(%s) \n", 
	                  tone_type_str[pToneTable->ToneType]);
	index += sprintf(tmp+index, "          freq num(%d)= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                pToneTable->ToneNUM, pToneTable->Freq0, pToneTable->Gain0, pToneTable->Freq1,
	                pToneTable->Gain1, pToneTable->Freq2, pToneTable->Gain2, pToneTable->Freq3,
	                pToneTable->Gain3);
	index += sprintf(tmp+index, "          cad num(%d)= %don/%doff, %don/%doff, %don/%doff, %don/%doff \n",
	                pToneTable->cadNUM, pToneTable->CadOn0, pToneTable->CadOff0, pToneTable->CadOn1,
	                pToneTable->CadOff1, pToneTable->CadOn2, pToneTable->CadOff2, pToneTable->CadOn3,
	                pToneTable->CadOff3);

	len =  sprintf(page,tmp);

	*eof = 1;
	return len;
}
#endif
extern char* playtone_whichtone_str[];
int tone_test;

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int voip_tone_write(struct file *file, const char *buffer, 
                               unsigned long count, void *data)
{
	char tmp[1024*4]={0};
	int index=0;
	if (count < 2)
		return -EFAULT;

	if (buffer && !copy_from_user(tmp, buffer, 128)) {
		sscanf(tmp, "%d", &tone_test);
	}
	index += sprintf(tmp+index, "Which tone : %d(%s) \n", tone_test,
	                                playtone_whichtone_str[tone_test]);
		index += sprintf( tmp + index, "type(%s) cycle(%d) \n",  tone_type_str[ToneTable[tone_test].ToneType], ToneTable[tone_test].cycle);
	index += sprintf(tmp+index, "          cad num(%d)= %don/%doff, %don/%doff, %don/%doff, %don/%doff unit:mSec\n",
	                ToneTable[tone_test].cadNUM, ToneTable[tone_test].CadOn0, ToneTable[tone_test].CadOff0, ToneTable[tone_test].CadOn1,
	                ToneTable[tone_test].CadOff1, ToneTable[tone_test].CadOn2, ToneTable[tone_test].CadOff2, ToneTable[tone_test].CadOn3,
	                ToneTable[tone_test].CadOff3);
		index += sprintf( tmp + index, "PatternOff(%d) \n",  ToneTable[tone_test].PatternOff);
	index += sprintf(tmp+index, "          freq num(%d)= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].ToneNUM, ToneTable[tone_test].Freq0, ToneTable[tone_test].Gain0, ToneTable[tone_test].Freq1,
	                ToneTable[tone_test].Gain1, ToneTable[tone_test].Freq2, ToneTable[tone_test].Gain2, ToneTable[tone_test].Freq3,
	                ToneTable[tone_test].Gain3);

		if (ToneTable[tone_test].ToneType>=6) { /* FOUR_FREQ or STEP_INC or TWO_STEP */
	index += sprintf(tmp+index, "        cad1/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C1_Freq0, ToneTable[tone_test].C1_Gain0, ToneTable[tone_test].C1_Freq1,
	                ToneTable[tone_test].C1_Gain1, ToneTable[tone_test].C1_Freq2, ToneTable[tone_test].C1_Gain2, ToneTable[tone_test].C1_Freq3,
	                ToneTable[tone_test].C1_Gain3);
			
	index += sprintf(tmp+index, "        cad2/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C2_Freq0, ToneTable[tone_test].C2_Gain0, ToneTable[tone_test].C2_Freq1,
	                ToneTable[tone_test].C2_Gain1, ToneTable[tone_test].C2_Freq2, ToneTable[tone_test].C2_Gain2, ToneTable[tone_test].C2_Freq3,
	                ToneTable[tone_test].C2_Gain3);
	index += sprintf(tmp+index, "        cad3/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C3_Freq0, ToneTable[tone_test].C3_Gain0, ToneTable[tone_test].C3_Freq1,
	                ToneTable[tone_test].C3_Gain1, ToneTable[tone_test].C3_Freq2, ToneTable[tone_test].C3_Gain2, ToneTable[tone_test].C3_Freq3,
	                ToneTable[tone_test].C3_Gain3);

			}
	if (ToneTable[tone_test].cadNUM>4) {
		
	index += sprintf(tmp+index, "        cad4/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C4_Freq0, ToneTable[tone_test].C4_Gain0, ToneTable[tone_test].C4_Freq1,
	                ToneTable[tone_test].C4_Gain1, ToneTable[tone_test].C4_Freq2, ToneTable[tone_test].C4_Gain2, ToneTable[tone_test].C4_Freq3,
	                ToneTable[tone_test].C4_Gain3);

	index += sprintf(tmp+index, "        cad5/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C5_Freq0, ToneTable[tone_test].C5_Gain0, ToneTable[tone_test].C5_Freq1,
	                ToneTable[tone_test].C5_Gain1, ToneTable[tone_test].C5_Freq2, ToneTable[tone_test].C5_Gain2, ToneTable[tone_test].C5_Freq3,
	                ToneTable[tone_test].C5_Gain3);

	index += sprintf(tmp+index, "        cad6/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C6_Freq0, ToneTable[tone_test].C6_Gain0, ToneTable[tone_test].C6_Freq1,
	                ToneTable[tone_test].C6_Gain1, ToneTable[tone_test].C6_Freq2, ToneTable[tone_test].C6_Gain2, ToneTable[tone_test].C6_Freq3,
	                ToneTable[tone_test].C6_Gain3);

	index += sprintf(tmp+index, "        cad7/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C7_Freq0, ToneTable[tone_test].C7_Gain0, ToneTable[tone_test].C7_Freq1,
	                ToneTable[tone_test].C7_Gain1, ToneTable[tone_test].C7_Freq2, ToneTable[tone_test].C7_Gain2, ToneTable[tone_test].C7_Freq3,
	                ToneTable[tone_test].C7_Gain3);

	}
	if (ToneTable[tone_test].cadNUM>8) {
		
	index += sprintf(tmp+index, "        cad8/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C8_Freq0, ToneTable[tone_test].C8_Gain0, ToneTable[tone_test].C8_Freq1,
	                ToneTable[tone_test].C8_Gain1, ToneTable[tone_test].C8_Freq2, ToneTable[tone_test].C8_Gain2, ToneTable[tone_test].C8_Freq3,
	                ToneTable[tone_test].C8_Gain3);

	index += sprintf(tmp+index, "        cad9/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C9_Freq0, ToneTable[tone_test].C9_Gain0, ToneTable[tone_test].C9_Freq1,
	                ToneTable[tone_test].C9_Gain1, ToneTable[tone_test].C9_Freq2, ToneTable[tone_test].C9_Gain2, ToneTable[tone_test].C9_Freq3,
	                ToneTable[tone_test].C9_Gain3);

	index += sprintf(tmp+index, "        cad10/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C10_Freq0, ToneTable[tone_test].C10_Gain0, ToneTable[tone_test].C10_Freq1,
	                ToneTable[tone_test].C10_Gain1, ToneTable[tone_test].C10_Freq2, ToneTable[tone_test].C10_Gain2, ToneTable[tone_test].C10_Freq3,
	                ToneTable[tone_test].C10_Gain3);

	index += sprintf(tmp+index, "        cad11/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C11_Freq0, ToneTable[tone_test].C11_Gain0, ToneTable[tone_test].C11_Freq1,
	                ToneTable[tone_test].C11_Gain1, ToneTable[tone_test].C11_Freq2, ToneTable[tone_test].C11_Gain2, ToneTable[tone_test].C11_Freq3,
	                ToneTable[tone_test].C11_Gain3);

	}
	printk("%s", tmp);
	tmp[0]=0;
	index=0;
	if (ToneTable[tone_test].cadNUM>12) {
		
	index += sprintf(tmp, "        cad12/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C12_Freq0, ToneTable[tone_test].C12_Gain0, ToneTable[tone_test].C12_Freq1,
	                ToneTable[tone_test].C12_Gain1, ToneTable[tone_test].C12_Freq2, ToneTable[tone_test].C12_Gain2, ToneTable[tone_test].C12_Freq3,
	                ToneTable[tone_test].C12_Gain3);

	index += sprintf(tmp+index, "        cad13/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C13_Freq0, ToneTable[tone_test].C13_Gain0, ToneTable[tone_test].C13_Freq1,
	                ToneTable[tone_test].C13_Gain1, ToneTable[tone_test].C13_Freq2, ToneTable[tone_test].C13_Gain2, ToneTable[tone_test].C13_Freq3,
	                ToneTable[tone_test].C13_Gain3);

	index += sprintf(tmp+index, "        cad14/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C14_Freq0, ToneTable[tone_test].C14_Gain0, ToneTable[tone_test].C14_Freq1,
	                ToneTable[tone_test].C14_Gain1, ToneTable[tone_test].C14_Freq2, ToneTable[tone_test].C14_Gain2, ToneTable[tone_test].C14_Freq3,
	                ToneTable[tone_test].C14_Gain3);

	index += sprintf(tmp+index, "        cad15/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C15_Freq0, ToneTable[tone_test].C15_Gain0, ToneTable[tone_test].C15_Freq1,
	                ToneTable[tone_test].C15_Gain1, ToneTable[tone_test].C15_Freq2, ToneTable[tone_test].C15_Gain2, ToneTable[tone_test].C15_Freq3,
	                ToneTable[tone_test].C15_Gain3);

	}

	if (ToneTable[tone_test].cadNUM>16) {
		
	index += sprintf(tmp+index, "        cad16/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C16_Freq0, ToneTable[tone_test].C16_Gain0, ToneTable[tone_test].C16_Freq1,
	                ToneTable[tone_test].C16_Gain1, ToneTable[tone_test].C16_Freq2, ToneTable[tone_test].C16_Gain2, ToneTable[tone_test].C16_Freq3,
	                ToneTable[tone_test].C16_Gain3);

	index += sprintf(tmp+index, "        cad17/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C17_Freq0, ToneTable[tone_test].C17_Gain0, ToneTable[tone_test].C17_Freq1,
	                ToneTable[tone_test].C17_Gain1, ToneTable[tone_test].C17_Freq2, ToneTable[tone_test].C17_Gain2, ToneTable[tone_test].C17_Freq3,
	                ToneTable[tone_test].C17_Gain3);

	index += sprintf(tmp+index, "        cad18/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C18_Freq0, ToneTable[tone_test].C18_Gain0, ToneTable[tone_test].C18_Freq1,
	                ToneTable[tone_test].C18_Gain1, ToneTable[tone_test].C18_Freq2, ToneTable[tone_test].C18_Gain2, ToneTable[tone_test].C18_Freq3,
	                ToneTable[tone_test].C18_Gain3);

	index += sprintf(tmp+index, "        cad19/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C19_Freq0, ToneTable[tone_test].C19_Gain0, ToneTable[tone_test].C19_Freq1,
	                ToneTable[tone_test].C19_Gain1, ToneTable[tone_test].C19_Freq2, ToneTable[tone_test].C19_Gain2, ToneTable[tone_test].C19_Freq3,
	                ToneTable[tone_test].C19_Gain3);

	}

	if (ToneTable[tone_test].cadNUM>20) {
		
	index += sprintf(tmp+index, "        cad20/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C20_Freq0, ToneTable[tone_test].C20_Gain0, ToneTable[tone_test].C20_Freq1,
	                ToneTable[tone_test].C20_Gain1, ToneTable[tone_test].C20_Freq2, ToneTable[tone_test].C20_Gain2, ToneTable[tone_test].C20_Freq3,
	                ToneTable[tone_test].C20_Gain3);

	index += sprintf(tmp+index, "        cad21/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C21_Freq0, ToneTable[tone_test].C21_Gain0, ToneTable[tone_test].C21_Freq1,
	                ToneTable[tone_test].C21_Gain1, ToneTable[tone_test].C21_Freq2, ToneTable[tone_test].C21_Gain2, ToneTable[tone_test].C21_Freq3,
	                ToneTable[tone_test].C21_Gain3);

	index += sprintf(tmp+index, "        cad22/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C22_Freq0, ToneTable[tone_test].C22_Gain0, ToneTable[tone_test].C22_Freq1,
	                ToneTable[tone_test].C22_Gain1, ToneTable[tone_test].C22_Freq2, ToneTable[tone_test].C22_Gain2, ToneTable[tone_test].C22_Freq3,
	                ToneTable[tone_test].C22_Gain3);

	index += sprintf(tmp+index, "        cad23/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C23_Freq0, ToneTable[tone_test].C23_Gain0, ToneTable[tone_test].C23_Freq1,
	                ToneTable[tone_test].C23_Gain1, ToneTable[tone_test].C23_Freq2, ToneTable[tone_test].C23_Gain2, ToneTable[tone_test].C23_Freq3,
	                ToneTable[tone_test].C23_Gain3);

	}
	printk("%s", tmp);
	tmp[0]=0;
	index=0;
	if (ToneTable[tone_test].cadNUM>24) {
		
	index += sprintf(tmp+index, "        cad24/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C24_Freq0, ToneTable[tone_test].C24_Gain0, ToneTable[tone_test].C24_Freq1,
	                ToneTable[tone_test].C24_Gain1, ToneTable[tone_test].C24_Freq2, ToneTable[tone_test].C24_Gain2, ToneTable[tone_test].C24_Freq3,
	                ToneTable[tone_test].C24_Gain3);

	index += sprintf(tmp+index, "        cad25/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C25_Freq0, ToneTable[tone_test].C25_Gain0, ToneTable[tone_test].C25_Freq1,
	                ToneTable[tone_test].C25_Gain1, ToneTable[tone_test].C25_Freq2, ToneTable[tone_test].C25_Gain2, ToneTable[tone_test].C25_Freq3,
	                ToneTable[tone_test].C25_Gain3);

	index += sprintf(tmp+index, "        cad26/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C26_Freq0, ToneTable[tone_test].C26_Gain0, ToneTable[tone_test].C26_Freq1,
	                ToneTable[tone_test].C26_Gain1, ToneTable[tone_test].C26_Freq2, ToneTable[tone_test].C26_Gain2, ToneTable[tone_test].C26_Freq3,
	                ToneTable[tone_test].C26_Gain3);

	index += sprintf(tmp+index, "        cad27/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C27_Freq0, ToneTable[tone_test].C27_Gain0, ToneTable[tone_test].C27_Freq1,
	                ToneTable[tone_test].C27_Gain1, ToneTable[tone_test].C27_Freq2, ToneTable[tone_test].C27_Gain2, ToneTable[tone_test].C27_Freq3,
	                ToneTable[tone_test].C27_Gain3);

	}

	if (ToneTable[tone_test].cadNUM>28) {
		
	index += sprintf(tmp+index, "        cad28/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C28_Freq0, ToneTable[tone_test].C28_Gain0, ToneTable[tone_test].C28_Freq1,
	                ToneTable[tone_test].C28_Gain1, ToneTable[tone_test].C28_Freq2, ToneTable[tone_test].C28_Gain2, ToneTable[tone_test].C28_Freq3,
	                ToneTable[tone_test].C28_Gain3);

	index += sprintf(tmp+index, "        cad29/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C29_Freq0, ToneTable[tone_test].C29_Gain0, ToneTable[tone_test].C29_Freq1,
	                ToneTable[tone_test].C29_Gain1, ToneTable[tone_test].C29_Freq2, ToneTable[tone_test].C29_Gain2, ToneTable[tone_test].C29_Freq3,
	                ToneTable[tone_test].C29_Gain3);

	index += sprintf(tmp+index, "        cad30/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C30_Freq0, ToneTable[tone_test].C30_Gain0, ToneTable[tone_test].C30_Freq1,
	                ToneTable[tone_test].C30_Gain1, ToneTable[tone_test].C30_Freq2, ToneTable[tone_test].C30_Gain2, ToneTable[tone_test].C30_Freq3,
	                ToneTable[tone_test].C30_Gain3);

	index += sprintf(tmp+index, "        cad31/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C31_Freq0, ToneTable[tone_test].C31_Gain0, ToneTable[tone_test].C31_Freq1,
	                ToneTable[tone_test].C31_Gain1, ToneTable[tone_test].C31_Freq2, ToneTable[tone_test].C31_Gain2, ToneTable[tone_test].C31_Freq3,
	                ToneTable[tone_test].C31_Gain3);

	}

	printk("%s", tmp);

	return count;
}

#else
static int voip_tone_write(struct file *file, const char *buffer, 
                               unsigned long count, void *data)
{
	char tmp[1024*4]={0};
	int index=0;
	if (count < 2)
		return -EFAULT;

	if (buffer && !copy_from_user(tmp, buffer, 128)) {
		sscanf(tmp, "%d", &tone_test);
	}
	index += sprintf(tmp+index, "Which tone : %d(%s) \n", tone_test,
	                                playtone_whichtone_str[tone_test]);
		index += sprintf( tmp + index, "type(%s) cycle(%d) \n",  tone_type_str[ToneTable[tone_test].ToneType], ToneTable[tone_test].cycle);
	index += sprintf(tmp+index, "          cad num(%d)= %don/%doff, %don/%doff, %don/%doff, %don/%doff unit:mSec\n",
	                ToneTable[tone_test].cadNUM, ToneTable[tone_test].CadOn0, ToneTable[tone_test].CadOff0, ToneTable[tone_test].CadOn1,
	                ToneTable[tone_test].CadOff1, ToneTable[tone_test].CadOn2, ToneTable[tone_test].CadOff2, ToneTable[tone_test].CadOn3,
	                ToneTable[tone_test].CadOff3);
		index += sprintf( tmp + index, "PatternOff(%d) \n",  ToneTable[tone_test].PatternOff);
	index += sprintf(tmp+index, "          freq num(%d)= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].ToneNUM, ToneTable[tone_test].Freq0, ToneTable[tone_test].Gain0, ToneTable[tone_test].Freq1,
	                ToneTable[tone_test].Gain1, ToneTable[tone_test].Freq2, ToneTable[tone_test].Gain2, ToneTable[tone_test].Freq3,
	                ToneTable[tone_test].Gain3);

		if (ToneTable[tone_test].ToneType>=6) { /* FOUR_FREQ or STEP_INC or TWO_STEP */
	index += sprintf(tmp+index, "        cad1/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C1_Freq0, ToneTable[tone_test].C1_Gain0, ToneTable[tone_test].C1_Freq1,
	                ToneTable[tone_test].C1_Gain1, ToneTable[tone_test].C1_Freq2, ToneTable[tone_test].C1_Gain2, ToneTable[tone_test].C1_Freq3,
	                ToneTable[tone_test].C1_Gain3);
			
	index += sprintf(tmp+index, "        cad2/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C2_Freq0, ToneTable[tone_test].C2_Gain0, ToneTable[tone_test].C2_Freq1,
	                ToneTable[tone_test].C2_Gain1, ToneTable[tone_test].C2_Freq2, ToneTable[tone_test].C2_Gain2, ToneTable[tone_test].C2_Freq3,
	                ToneTable[tone_test].C2_Gain3);
	index += sprintf(tmp+index, "        cad3/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C3_Freq0, ToneTable[tone_test].C3_Gain0, ToneTable[tone_test].C3_Freq1,
	                ToneTable[tone_test].C3_Gain1, ToneTable[tone_test].C3_Freq2, ToneTable[tone_test].C3_Gain2, ToneTable[tone_test].C3_Freq3,
	                ToneTable[tone_test].C3_Gain3);

			}
	if (ToneTable[tone_test].cadNUM>4) {
		
	index += sprintf(tmp+index, "        cad4/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C4_Freq0, ToneTable[tone_test].C4_Gain0, ToneTable[tone_test].C4_Freq1,
	                ToneTable[tone_test].C4_Gain1, ToneTable[tone_test].C4_Freq2, ToneTable[tone_test].C4_Gain2, ToneTable[tone_test].C4_Freq3,
	                ToneTable[tone_test].C4_Gain3);

	index += sprintf(tmp+index, "        cad5/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C5_Freq0, ToneTable[tone_test].C5_Gain0, ToneTable[tone_test].C5_Freq1,
	                ToneTable[tone_test].C5_Gain1, ToneTable[tone_test].C5_Freq2, ToneTable[tone_test].C5_Gain2, ToneTable[tone_test].C5_Freq3,
	                ToneTable[tone_test].C5_Gain3);

	index += sprintf(tmp+index, "        cad6/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C6_Freq0, ToneTable[tone_test].C6_Gain0, ToneTable[tone_test].C6_Freq1,
	                ToneTable[tone_test].C6_Gain1, ToneTable[tone_test].C6_Freq2, ToneTable[tone_test].C6_Gain2, ToneTable[tone_test].C6_Freq3,
	                ToneTable[tone_test].C6_Gain3);

	index += sprintf(tmp+index, "        cad7/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C7_Freq0, ToneTable[tone_test].C7_Gain0, ToneTable[tone_test].C7_Freq1,
	                ToneTable[tone_test].C7_Gain1, ToneTable[tone_test].C7_Freq2, ToneTable[tone_test].C7_Gain2, ToneTable[tone_test].C7_Freq3,
	                ToneTable[tone_test].C7_Gain3);

	}
	if (ToneTable[tone_test].cadNUM>8) {
		
	index += sprintf(tmp+index, "        cad8/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C8_Freq0, ToneTable[tone_test].C8_Gain0, ToneTable[tone_test].C8_Freq1,
	                ToneTable[tone_test].C8_Gain1, ToneTable[tone_test].C8_Freq2, ToneTable[tone_test].C8_Gain2, ToneTable[tone_test].C8_Freq3,
	                ToneTable[tone_test].C8_Gain3);

	index += sprintf(tmp+index, "        cad9/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C9_Freq0, ToneTable[tone_test].C9_Gain0, ToneTable[tone_test].C9_Freq1,
	                ToneTable[tone_test].C9_Gain1, ToneTable[tone_test].C9_Freq2, ToneTable[tone_test].C9_Gain2, ToneTable[tone_test].C9_Freq3,
	                ToneTable[tone_test].C9_Gain3);

	index += sprintf(tmp+index, "        cad10/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C10_Freq0, ToneTable[tone_test].C10_Gain0, ToneTable[tone_test].C10_Freq1,
	                ToneTable[tone_test].C10_Gain1, ToneTable[tone_test].C10_Freq2, ToneTable[tone_test].C10_Gain2, ToneTable[tone_test].C10_Freq3,
	                ToneTable[tone_test].C10_Gain3);

	index += sprintf(tmp+index, "        cad11/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C11_Freq0, ToneTable[tone_test].C11_Gain0, ToneTable[tone_test].C11_Freq1,
	                ToneTable[tone_test].C11_Gain1, ToneTable[tone_test].C11_Freq2, ToneTable[tone_test].C11_Gain2, ToneTable[tone_test].C11_Freq3,
	                ToneTable[tone_test].C11_Gain3);

	}
	printk("%s", tmp);
	tmp[0]=0;
	index=0;
	if (ToneTable[tone_test].cadNUM>12) {
		
	index += sprintf(tmp, "        cad12/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C12_Freq0, ToneTable[tone_test].C12_Gain0, ToneTable[tone_test].C12_Freq1,
	                ToneTable[tone_test].C12_Gain1, ToneTable[tone_test].C12_Freq2, ToneTable[tone_test].C12_Gain2, ToneTable[tone_test].C12_Freq3,
	                ToneTable[tone_test].C12_Gain3);

	index += sprintf(tmp+index, "        cad13/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C13_Freq0, ToneTable[tone_test].C13_Gain0, ToneTable[tone_test].C13_Freq1,
	                ToneTable[tone_test].C13_Gain1, ToneTable[tone_test].C13_Freq2, ToneTable[tone_test].C13_Gain2, ToneTable[tone_test].C13_Freq3,
	                ToneTable[tone_test].C13_Gain3);

	index += sprintf(tmp+index, "        cad14/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C14_Freq0, ToneTable[tone_test].C14_Gain0, ToneTable[tone_test].C14_Freq1,
	                ToneTable[tone_test].C14_Gain1, ToneTable[tone_test].C14_Freq2, ToneTable[tone_test].C14_Gain2, ToneTable[tone_test].C14_Freq3,
	                ToneTable[tone_test].C14_Gain3);

	index += sprintf(tmp+index, "        cad15/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C15_Freq0, ToneTable[tone_test].C15_Gain0, ToneTable[tone_test].C15_Freq1,
	                ToneTable[tone_test].C15_Gain1, ToneTable[tone_test].C15_Freq2, ToneTable[tone_test].C15_Gain2, ToneTable[tone_test].C15_Freq3,
	                ToneTable[tone_test].C15_Gain3);

	}

	if (ToneTable[tone_test].cadNUM>16) {
		
	index += sprintf(tmp+index, "        cad16/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C16_Freq0, ToneTable[tone_test].C16_Gain0, ToneTable[tone_test].C16_Freq1,
	                ToneTable[tone_test].C16_Gain1, ToneTable[tone_test].C16_Freq2, ToneTable[tone_test].C16_Gain2, ToneTable[tone_test].C16_Freq3,
	                ToneTable[tone_test].C16_Gain3);

	index += sprintf(tmp+index, "        cad17/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C17_Freq0, ToneTable[tone_test].C17_Gain0, ToneTable[tone_test].C17_Freq1,
	                ToneTable[tone_test].C17_Gain1, ToneTable[tone_test].C17_Freq2, ToneTable[tone_test].C17_Gain2, ToneTable[tone_test].C17_Freq3,
	                ToneTable[tone_test].C17_Gain3);

	index += sprintf(tmp+index, "        cad18/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C18_Freq0, ToneTable[tone_test].C18_Gain0, ToneTable[tone_test].C18_Freq1,
	                ToneTable[tone_test].C18_Gain1, ToneTable[tone_test].C18_Freq2, ToneTable[tone_test].C18_Gain2, ToneTable[tone_test].C18_Freq3,
	                ToneTable[tone_test].C18_Gain3);

	index += sprintf(tmp+index, "        cad19/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C19_Freq0, ToneTable[tone_test].C19_Gain0, ToneTable[tone_test].C19_Freq1,
	                ToneTable[tone_test].C19_Gain1, ToneTable[tone_test].C19_Freq2, ToneTable[tone_test].C19_Gain2, ToneTable[tone_test].C19_Freq3,
	                ToneTable[tone_test].C19_Gain3);

	}

	if (ToneTable[tone_test].cadNUM>20) {
		
	index += sprintf(tmp+index, "        cad20/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C20_Freq0, ToneTable[tone_test].C20_Gain0, ToneTable[tone_test].C20_Freq1,
	                ToneTable[tone_test].C20_Gain1, ToneTable[tone_test].C20_Freq2, ToneTable[tone_test].C20_Gain2, ToneTable[tone_test].C20_Freq3,
	                ToneTable[tone_test].C20_Gain3);

	index += sprintf(tmp+index, "        cad21/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C21_Freq0, ToneTable[tone_test].C21_Gain0, ToneTable[tone_test].C21_Freq1,
	                ToneTable[tone_test].C21_Gain1, ToneTable[tone_test].C21_Freq2, ToneTable[tone_test].C21_Gain2, ToneTable[tone_test].C21_Freq3,
	                ToneTable[tone_test].C21_Gain3);

	index += sprintf(tmp+index, "        cad22/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C22_Freq0, ToneTable[tone_test].C22_Gain0, ToneTable[tone_test].C22_Freq1,
	                ToneTable[tone_test].C22_Gain1, ToneTable[tone_test].C22_Freq2, ToneTable[tone_test].C22_Gain2, ToneTable[tone_test].C22_Freq3,
	                ToneTable[tone_test].C22_Gain3);

	index += sprintf(tmp+index, "        cad23/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C23_Freq0, ToneTable[tone_test].C23_Gain0, ToneTable[tone_test].C23_Freq1,
	                ToneTable[tone_test].C23_Gain1, ToneTable[tone_test].C23_Freq2, ToneTable[tone_test].C23_Gain2, ToneTable[tone_test].C23_Freq3,
	                ToneTable[tone_test].C23_Gain3);

	}
	printk("%s", tmp);
	tmp[0]=0;
	index=0;
	if (ToneTable[tone_test].cadNUM>24) {
		
	index += sprintf(tmp+index, "        cad24/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C24_Freq0, ToneTable[tone_test].C24_Gain0, ToneTable[tone_test].C24_Freq1,
	                ToneTable[tone_test].C24_Gain1, ToneTable[tone_test].C24_Freq2, ToneTable[tone_test].C24_Gain2, ToneTable[tone_test].C24_Freq3,
	                ToneTable[tone_test].C24_Gain3);

	index += sprintf(tmp+index, "        cad25/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C25_Freq0, ToneTable[tone_test].C25_Gain0, ToneTable[tone_test].C25_Freq1,
	                ToneTable[tone_test].C25_Gain1, ToneTable[tone_test].C25_Freq2, ToneTable[tone_test].C25_Gain2, ToneTable[tone_test].C25_Freq3,
	                ToneTable[tone_test].C25_Gain3);

	index += sprintf(tmp+index, "        cad26/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C26_Freq0, ToneTable[tone_test].C26_Gain0, ToneTable[tone_test].C26_Freq1,
	                ToneTable[tone_test].C26_Gain1, ToneTable[tone_test].C26_Freq2, ToneTable[tone_test].C26_Gain2, ToneTable[tone_test].C26_Freq3,
	                ToneTable[tone_test].C26_Gain3);

	index += sprintf(tmp+index, "        cad27/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C27_Freq0, ToneTable[tone_test].C27_Gain0, ToneTable[tone_test].C27_Freq1,
	                ToneTable[tone_test].C27_Gain1, ToneTable[tone_test].C27_Freq2, ToneTable[tone_test].C27_Gain2, ToneTable[tone_test].C27_Freq3,
	                ToneTable[tone_test].C27_Gain3);

	}

	if (ToneTable[tone_test].cadNUM>28) {
		
	index += sprintf(tmp+index, "        cad28/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C28_Freq0, ToneTable[tone_test].C28_Gain0, ToneTable[tone_test].C28_Freq1,
	                ToneTable[tone_test].C28_Gain1, ToneTable[tone_test].C28_Freq2, ToneTable[tone_test].C28_Gain2, ToneTable[tone_test].C28_Freq3,
	                ToneTable[tone_test].C28_Gain3);

	index += sprintf(tmp+index, "        cad29/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C29_Freq0, ToneTable[tone_test].C29_Gain0, ToneTable[tone_test].C29_Freq1,
	                ToneTable[tone_test].C29_Gain1, ToneTable[tone_test].C29_Freq2, ToneTable[tone_test].C29_Gain2, ToneTable[tone_test].C29_Freq3,
	                ToneTable[tone_test].C29_Gain3);

	index += sprintf(tmp+index, "        cad30/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C30_Freq0, ToneTable[tone_test].C30_Gain0, ToneTable[tone_test].C30_Freq1,
	                ToneTable[tone_test].C30_Gain1, ToneTable[tone_test].C30_Freq2, ToneTable[tone_test].C30_Gain2, ToneTable[tone_test].C30_Freq3,
	                ToneTable[tone_test].C30_Gain3);

	index += sprintf(tmp+index, "        cad31/= %dHz/%ddB, %dHz/%ddB, %dHz/%ddB, %dHz/%ddB \n",
	                ToneTable[tone_test].C31_Freq0, ToneTable[tone_test].C31_Gain0, ToneTable[tone_test].C31_Freq1,
	                ToneTable[tone_test].C31_Gain1, ToneTable[tone_test].C31_Freq2, ToneTable[tone_test].C31_Gain2, ToneTable[tone_test].C31_Freq3,
	                ToneTable[tone_test].C31_Gain3);

	}

	printk("%s", tmp);

	return count;
}
#endif

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int codec1_open(struct inode *inode, struct file *file)
{
	return single_open(file, voip_tone_read, NULL);
}

static ssize_t codec1_write(struct file *file, const char __user * userbuf, size_t count, loff_t * off) {
        return voip_tone_write(file, userbuf, count, NULL);
}

struct file_operations proc_codec1_fops = {
	.owner	= THIS_MODULE,
	.open	= codec1_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = codec1_write,
//read:   voip_tone_read,
//write:	voip_tone_write
};
#endif

int __init voip_proc_tone_init( void )
{
	struct proc_dir_entry *voip_tone_proc;

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	voip_tone_proc = create_voip_proc_rw_entry(PROC_VOIP_DIR "/show_tone_config", 0644, NULL,
														 &proc_codec1_fops);
#else
	voip_tone_proc = create_voip_proc_rw_entry(PROC_VOIP_DIR "/show_tone_config", 0644, NULL,
	                                                         voip_tone_read, voip_tone_write);
#endif
	if (voip_tone_proc == NULL ) {
		printk("voip_tone_proc NULL!! \n;");
		return -1;
	}

	return 0;
}

void __exit voip_proc_tone_exit( void )
{
	remove_proc_entry( PROC_VOIP_DIR "/show_tone_config", NULL );
}

voip_initcall_proc( voip_proc_tone_init );
voip_exitcall( voip_proc_tone_exit );


