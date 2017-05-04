
//
// dspcodec.h
//

#ifndef _DSPCODEC_H_
#define _DSPCODEC_H_

#ifdef __ECOS
#include <pkgconf/system.h>
#include <pkgconf/io.h>
#include <cyg/io/io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/io/devtab.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_io.h>
#endif

#include "rtk_voip.h"
#include "voip_types.h"
#include "dspcodec.h"
#include "voip_params.h"

//#define G711PRETD           (500*80)			// (500*160)
//#define G723PRETD           (500*240)
//#define G729PRETD           (500*80)			// (500*160)
//#define TDOFFSET_PO         64000               // 8sec
//#define TDOFFSET_NE         (-64000)            // -8sec

// country tone set
typedef enum
{
	DSPCODEC_COUNTRY_USA,
	DSPCODEC_COUNTRY_UK,
	DSPCODEC_COUNTRY_AUSTRALIA,
	DSPCODEC_COUNTRY_HK,
	DSPCODEC_COUNTRY_JP,
	DSPCODEC_COUNTRY_SE,
	DSPCODEC_COUNTRY_GR,
	DSPCODEC_COUNTRY_FR,
#if 0	
	DSPCODEC_COUNTRY_TR,
#else
	DSPCODEC_COUNTRY_TW,
#endif
	DSPCODEC_COUNTRY_BE,
	DSPCODEC_COUNTRY_FL,
	DSPCODEC_COUNTRY_IT,
	DSPCODEC_COUNTRY_CN,
	DSPCODEC_COUNTRY_EX1,		///< extend country #1
	DSPCODEC_COUNTRY_EX2,		///< extend country #2
	DSPCODEC_COUNTRY_EX3,		///< extend country #3
	DSPCODEC_COUNTRY_EX4,		///< extend country #4
#ifdef COUNTRY_TONE_RESERVED
	DSPCODEC_COUNTRY_RESERVE,
#endif
	DSPCODEC_COUNTRY_CUSTOME
} DSPCODEC_COUNTRY;

// input/output interfaces
#if 0
typedef enum
{
	DSPCODEC_INTERFACE_HANDSET,
	DSPCODEC_INTERFACE_HANDFREE,
	DSPCODEC_INTERFACE_HEADSET
} DSPCODEC_INTERFACE;
#endif

/*	handsome add function 2005.12.1     */
typedef struct aspToneCfgParam
{
	uint32	toneType;
	uint16	cycle;

	uint16	cadNUM;

	uint32	CadOn0;
	uint32	CadOff0;
	uint32	CadOn1;
	uint32	CadOff1;
	uint32	CadOn2;
	uint32	CadOff2;
	uint32	CadOn3;
	uint32	CadOff3;

	uint32 PatternOff;
	uint32 ToneNUM;

	uint32	Freq0;
	uint32	Freq1;
	uint32	Freq2;
	uint32	Freq3;

	int32 Gain0;
	int32 Gain1;
	int32 Gain2;
	int32 Gain3;

	uint32	C1_Freq0;
	uint32	C1_Freq1;
	uint32	C1_Freq2;
	uint32	C1_Freq3;

	int32 C1_Gain0;
	int32 C1_Gain1;
	int32 C1_Gain2;
	int32 C1_Gain3;

	uint32	C2_Freq0;
	uint32	C2_Freq1;
	uint32	C2_Freq2;
	uint32	C2_Freq3;

	int32 C2_Gain0;
	int32 C2_Gain1;
	int32 C2_Gain2;
	int32 C2_Gain3;


	uint32	C3_Freq0;
	uint32	C3_Freq1;
	uint32	C3_Freq2;
	uint32	C3_Freq3;

	int32 C3_Gain0;
	int32 C3_Gain1;
	int32 C3_Gain2;
	int32 C3_Gain3;

	uint32	CadOn4;
	uint32	CadOff4;
	uint32	CadOn5;
	uint32	CadOff5;
	uint32	CadOn6;
	uint32	CadOff6;
	uint32	CadOn7;
	uint32	CadOff7;

	uint32	C4_Freq0;
	uint32	C4_Freq1;
	uint32	C4_Freq2;
	uint32	C4_Freq3;

	int32 C4_Gain0;
	int32 C4_Gain1;
	int32 C4_Gain2;
	int32 C4_Gain3;

	uint32	C5_Freq0;
	uint32	C5_Freq1;
	uint32	C5_Freq2;
	uint32	C5_Freq3;

	int32 C5_Gain0;
	int32 C5_Gain1;
	int32 C5_Gain2;
	int32 C5_Gain3;

	uint32	C6_Freq0;
	uint32	C6_Freq1;
	uint32	C6_Freq2;
	uint32	C6_Freq3;

	int32 C6_Gain0;
	int32 C6_Gain1;
	int32 C6_Gain2;
	int32 C6_Gain3;

	uint32	C7_Freq0;
	uint32	C7_Freq1;
	uint32	C7_Freq2;
	uint32	C7_Freq3;

	int32 C7_Gain0;
	int32 C7_Gain1;
	int32 C7_Gain2;
	int32 C7_Gain3;

	uint32	CadOn8;
	uint32	CadOff8;
	uint32	CadOn9;
	uint32	CadOff9;
	uint32	CadOn10;
	uint32	CadOff10;
	uint32	CadOn11;
	uint32	CadOff11;

	uint32	C8_Freq0;
	uint32	C8_Freq1;
	uint32	C8_Freq2;
	uint32	C8_Freq3;

	int32 C8_Gain0;
	int32 C8_Gain1;
	int32 C8_Gain2;
	int32 C8_Gain3;

	uint32	C9_Freq0;
	uint32	C9_Freq1;
	uint32	C9_Freq2;
	uint32	C9_Freq3;

	int32 C9_Gain0;
	int32 C9_Gain1;
	int32 C9_Gain2;
	int32 C9_Gain3;

	uint32	C10_Freq0;
	uint32	C10_Freq1;
	uint32	C10_Freq2;
	uint32	C10_Freq3;

	int32 C10_Gain0;
	int32 C10_Gain1;
	int32 C10_Gain2;
	int32 C10_Gain3;

	uint32	C11_Freq0;
	uint32	C11_Freq1;
	uint32	C11_Freq2;
	uint32	C11_Freq3;

	int32 C11_Gain0;
	int32 C11_Gain1;
	int32 C11_Gain2;
	int32 C11_Gain3;

	uint32	CadOn12;
	uint32	CadOff12;
	uint32	CadOn13;
	uint32	CadOff13;
	uint32	CadOn14;
	uint32	CadOff14;
	uint32	CadOn15;
	uint32	CadOff15;

	uint32	C12_Freq0;
	uint32	C12_Freq1;
	uint32	C12_Freq2;
	uint32	C12_Freq3;

	int32 C12_Gain0;
	int32 C12_Gain1;
	int32 C12_Gain2;
	int32 C12_Gain3;

	uint32	C13_Freq0;
	uint32	C13_Freq1;
	uint32	C13_Freq2;
	uint32	C13_Freq3;

	int32 C13_Gain0;
	int32 C13_Gain1;
	int32 C13_Gain2;
	int32 C13_Gain3;

	uint32	C14_Freq0;
	uint32	C14_Freq1;
	uint32	C14_Freq2;
	uint32	C14_Freq3;

	int32 C14_Gain0;
	int32 C14_Gain1;
	int32 C14_Gain2;
	int32 C14_Gain3;

	uint32	C15_Freq0;
	uint32	C15_Freq1;
	uint32	C15_Freq2;
	uint32	C15_Freq3;

	int32 C15_Gain0;
	int32 C15_Gain1;
	int32 C15_Gain2;
	int32 C15_Gain3;

	uint32	CadOn16;
	uint32	CadOff16;
	uint32	CadOn17;
	uint32	CadOff17;
	uint32	CadOn18;
	uint32	CadOff18;
	uint32	CadOn19;
	uint32	CadOff19;

	uint32	C16_Freq0;
	uint32	C16_Freq1;
	uint32	C16_Freq2;
	uint32	C16_Freq3;

	int32 C16_Gain0;
	int32 C16_Gain1;
	int32 C16_Gain2;
	int32 C16_Gain3;

	uint32	C17_Freq0;
	uint32	C17_Freq1;
	uint32	C17_Freq2;
	uint32	C17_Freq3;

	int32 C17_Gain0;
	int32 C17_Gain1;
	int32 C17_Gain2;
	int32 C17_Gain3;

	uint32	C18_Freq0;
	uint32	C18_Freq1;
	uint32	C18_Freq2;
	uint32	C18_Freq3;

	int32 C18_Gain0;
	int32 C18_Gain1;
	int32 C18_Gain2;
	int32 C18_Gain3;

	uint32	C19_Freq0;
	uint32	C19_Freq1;
	uint32	C19_Freq2;
	uint32	C19_Freq3;

	int32 C19_Gain0;
	int32 C19_Gain1;
	int32 C19_Gain2;
	int32 C19_Gain3;

	uint32	CadOn20;
	uint32	CadOff20;
	uint32	CadOn21;
	uint32	CadOff21;
	uint32	CadOn22;
	uint32	CadOff22;
	uint32	CadOn23;
	uint32	CadOff23;

	uint32	C20_Freq0;
	uint32	C20_Freq1;
	uint32	C20_Freq2;
	uint32	C20_Freq3;

	int32 C20_Gain0;
	int32 C20_Gain1;
	int32 C20_Gain2;
	int32 C20_Gain3;

	uint32	C21_Freq0;
	uint32	C21_Freq1;
	uint32	C21_Freq2;
	uint32	C21_Freq3;

	int32 C21_Gain0;
	int32 C21_Gain1;
	int32 C21_Gain2;
	int32 C21_Gain3;

	uint32	C22_Freq0;
	uint32	C22_Freq1;
	uint32	C22_Freq2;
	uint32	C22_Freq3;

	int32 C22_Gain0;
	int32 C22_Gain1;
	int32 C22_Gain2;
	int32 C22_Gain3;

	uint32	C23_Freq0;
	uint32	C23_Freq1;
	uint32	C23_Freq2;
	uint32	C23_Freq3;

	int32 C23_Gain0;
	int32 C23_Gain1;
	int32 C23_Gain2;
	int32 C23_Gain3;

	uint32	CadOn24;
	uint32	CadOff24;
	uint32	CadOn25;
	uint32	CadOff25;
	uint32	CadOn26;
	uint32	CadOff26;
	uint32	CadOn27;
	uint32	CadOff27;

	uint32	C24_Freq0;
	uint32	C24_Freq1;
	uint32	C24_Freq2;
	uint32	C24_Freq3;

	int32 C24_Gain0;
	int32 C24_Gain1;
	int32 C24_Gain2;
	int32 C24_Gain3;

	uint32	C25_Freq0;
	uint32	C25_Freq1;
	uint32	C25_Freq2;
	uint32	C25_Freq3;

	int32 C25_Gain0;
	int32 C25_Gain1;
	int32 C25_Gain2;
	int32 C25_Gain3;

	uint32	C26_Freq0;
	uint32	C26_Freq1;
	uint32	C26_Freq2;
	uint32	C26_Freq3;

	int32 C26_Gain0;
	int32 C26_Gain1;
	int32 C26_Gain2;
	int32 C26_Gain3;

	uint32	C27_Freq0;
	uint32	C27_Freq1;
	uint32	C27_Freq2;
	uint32	C27_Freq3;

	int32 C27_Gain0;
	int32 C27_Gain1;
	int32 C27_Gain2;
	int32 C27_Gain3;

	uint32	CadOn28;
	uint32	CadOff28;
	uint32	CadOn29;
	uint32	CadOff29;
	uint32	CadOn30;
	uint32	CadOff30;
	uint32	CadOn31;
	uint32	CadOff31;

	uint32	C28_Freq0;
	uint32	C28_Freq1;
	uint32	C28_Freq2;
	uint32	C28_Freq3;

	int32 C28_Gain0;
	int32 C28_Gain1;
	int32 C28_Gain2;
	int32 C28_Gain3;

	uint32	C29_Freq0;
	uint32	C29_Freq1;
	uint32	C29_Freq2;
	uint32	C29_Freq3;

	int32 C29_Gain0;
	int32 C29_Gain1;
	int32 C29_Gain2;
	int32 C29_Gain3;

	uint32	C30_Freq0;
	uint32	C30_Freq1;
	uint32	C30_Freq2;
	uint32	C30_Freq3;

	int32 C30_Gain0;
	int32 C30_Gain1;
	int32 C30_Gain2;
	int32 C30_Gain3;

	uint32	C31_Freq0;
	uint32	C31_Freq1;
	uint32	C31_Freq2;
	uint32	C31_Freq3;

	int32 C31_Gain0;
	int32 C31_Gain1;
	int32 C31_Gain2;
	int32 C31_Gain3;

} aspToneCfgParam_t;

/*	handsome add function 2005.12.1     */
#if 0
typedef struct gDSP_aspTone
{
	uint32 custID;
	aspToneCfgParam_t* DSP_aspToneCfg;
} gDSP_aspTone_t;
#endif


/*	handsome add function 2005.12.1     */
#if 0
typedef struct aspToneCountryCfgParam
{
	uint8		CountryId;
	uint8		iDial;
	uint8		iRing;
	uint8		iBusy;
	uint8		iWaiting;

} gDSP_aspToneCountry_t;
#endif

typedef unsigned int	RESULT;

// DSP interface functions
//void   DspcodecInitVar(void);		// pkshih: unused code 

void   DspcodecUp(uint32 sid);

//void   DspcodecDown(void);

RESULT DspcodecInitialize(uint32 sid, DSPCODEC_ALGORITHM uTranCodingAlgorithm, DSPCODEC_ALGORITHM uRecvCodingAlgorithm, const CDspcodecConfig *pConfig);
RESULT DspcodecInitialize_R1(uint32 sid, const CDspcodecInitializeParm *pInitializeParm );

RESULT DspcodecSetConfig(uint32 sid/*, CDspcodecConfig* pConfig*/);
RESULT DspcodecSetConfig_R1(uint32 sid );

//RESULT DspcodecGetConfig(uint32 sid, CDspcodecConfig* pConfig);

RESULT DspcodecStart(uint32 sid, DSPCODEC_ACTION uAction);
RESULT DspcodecStart_R1(uint32 sid, /*const*/ CDspcodecStartParm *pStartParm);

RESULT DspcodecStop(uint32 sid);
RESULT DspcodecStop_R1(uint32 sid);

//RESULT DspcodecMute(uint32 sid, DSPCODEC_MUTEDIRECTION uDirection);

RESULT  voip_tone_stop_event(uint32 sid, uint32 path);                                                                
RESULT  voip_tone_userstop_event(uint32 sid, uint32 path);  
RESULT voip_rtp_payload_mismatch_event( uint32 sid, RtpPayloadType payload );

RESULT DspcodecPlayTone(uint32 sid, DSPCODEC_TONE nTone, bool bFlag, DSPCODEC_TONEDIRECTION nPath);
RESULT DspcodecPlayTone_R1( uint32 sid, const CDspcodecPlayToneParm *pPlayToneParm );

int32  DspcodecWrite(uint32 chid, uint32 sid, uint8* pBuf, int32 nSize, uint32 nSeq, uint32 nTimestamp, RtpFramesType DecFrameType);

int32  DspcodecRead(uint32 chid, uint32 sid, uint8* pBuf, int32 nSize);

//RESULT DspcodecSetVolume(int32 nVal);

//RESULT DspcodecSetEncGain(int32 nVal);

//RESULT DspcodecSetSidetoneGain(int32 nVal);

//RESULT DspcodecSidetoneSwitch(bool bFlag);

//RESULT DspcodecSetRingType(uint32 sid, int32 nRing);

RESULT DspcodecSetCountry(uint32 sid, int32 nCountry);

//RESULT DspcodecSetInterface(DSPCODEC_INTERFACE nInterface);

//int32  DspcodecGetVersion(char *pBuf, int32 nLen);

void   DspcodecResetR1(uint32 sid);

//void   DspcodecDebugR1(uint32 sid);

int32 setTone(DSPCODEC_COUNTRY country, DSPCODEC_TONE tone, aspToneCfgParam_t* pToneCfg); // thlin modify

//int32 DSP_SetToneInfo(uint32* data);
//int32 setToneCountry(gDSP_aspToneCountry_t* pCfg);					/*	handsome add function 2005.12.1     */

#endif	// _DSPCODEC_H_
