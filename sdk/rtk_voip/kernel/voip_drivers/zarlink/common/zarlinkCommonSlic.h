#ifndef __ZARLINKCOMMONsLIC__H__
#define __ZARLINKCOMMONsLIC__H__

#include "vp_api.h"
#include "Ve_profile.h"
#include "zarlinkCommon.h"
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK_ON_NEW_ARCH
#include "snd_define.h"
#else
#include "Slic_api.h"		/* for PCM_MODE */
#endif

/*
 *  Function declarations
 */
//BOOL zarlinkWaitForEvent( VpDevCtxType* pDevCtx, VpEventCategoryType category, uint16 event );

int rtkGetNewChID(void);
VpOptionCodecType getCodecType(BUS_DATA_FORMAT pcm_data_format);

VpStatusType zarlinkSetFxsPcmMode(RTKLineObj *pLine, int pcm_mode);
VpStatusType zarlinkSetRingCadence(RTKLineObj *pLine,
    unsigned short on_msec, unsigned short off_msec );
VpStatusType zarlinkFxsRing(RTKLineObj *pLine, uint8 enable);
VpStatusType zarlinkSendNTTCAR(RTKLineObj *pLine);
VpStatusType zarlinkSetLineState(RTKLineObj *pLine, VpLineStateType state);
VpStatusType zarlinkSetOHT(RTKLineObj *pLine, uint8 reversal);
VpStatusType zarlinkSetLineOpen(RTKLineObj *pLine);
VpStatusType zarlinkSetPcmTxOnly(RTKLineObj *pLine, int enable);
VpStatusType zarlinkSetPcmRxOnly(RTKLineObj *pLine, int enable);
VpStatusType zarlinkDumpDevReg (RTKDevObj *pDev); 
VpStatusType zarlinkDumpDevObj (RTKLineObj *pLine, RTKDevObj *pDev );
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST)
int zarlinkLineTest (	RTKLineObj *pLine, int tID, void *data);

int zarlinkPortDetect (RTKLineObj *pLine, int tID, void *data);
int zarlinkLineROH (	RTKLineObj *pLine, int tID, void *data);
int zarlinkLineVOLTAGE (RTKLineObj *pLine, int tID, void *data);
int zarlinkLineRESFLT (RTKLineObj *pLine, int tID, void *data);
#endif
VpStatusType zarlinkRWDevReg (RTKLineObj *pLine, unsigned int reg, unsigned char *len, char * regdata); 
#if 1 //defined(CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES)
VpStatusType zarlinkInitled (RTKLineObj *pLine);
VpStatusType zarlinkled (RTKLineObj *pLine, int onoff );
#endif
unsigned char zarlinkSendNTTCAR_Check(RTKLineObj *pLine, unsigned long time_out);
unsigned char zarlinkGetFxsHookStatus(RTKLineObj *pLine, int from_polling_timer);
unsigned char zarlinkGetLineState(RTKLineObj *pLine);
unsigned char zarlinkCheckFxsRing(RTKLineObj *pLine);

VpStatusType zarlinkTxGainAdjust(RTKLineObj *pLine, int gain);	
VpStatusType zarlinkRxGainAdjust(RTKLineObj *pLine, int gain);	

VpStatusType zarlinkSetIOState(RTKLineObj *pLine, VPIO IO, int bHigh );
VpStatusType zarlinkGetIOState(RTKLineObj *pLine, VPIO IO, int *bHigh );
VpStatusType zarlinkSetIODir(RTKLineObj *pLine, VPIO IO, int bOut );

#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES
VpStatusType zarlinkStartMeter(RTKLineObj *pLine, uint16 hz, uint16 onTime, uint16 offTime, uint16 numMeters);
#endif

#if 0
VpStatusType zarlinkSetImpedence(RTKLineObj *pLine, uint16 preset);
VpStatusType Ve890SetFxsAcProfileByBand(RTKLineObj *pLine, int pcm_mode);
VpStatusType zarlinkSetRingFreqAmp(RTKLineObj *pLine, uint8 profile);
#endif
VpStatusType zarlinkGetCoeff (RTKLineObj *pLine, void *data);
VpStatusType zarlinkSetCoeff (RTKLineObj *pLine, void *data);
VpStatusType zarlinkDoCalibration(RTKLineObj *pLine, unsigned char mode);

#endif /* __ZARLINKCOMMONsLIC__H__ */

