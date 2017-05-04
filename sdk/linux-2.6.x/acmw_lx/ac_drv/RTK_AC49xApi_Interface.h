#ifndef _RTK_AC49xAPI_INTERFACE_
#define	_RTK_AC49xAPI_INTERFACE_

#include "AC49xDrv_Drv.h"
#include "../include/userdef.h"

#define TONE_OFF	0
#define TONE_ON		1

#define CH_CLOSE		0
#define CH_OPEN			1
#define CH_OPEN_FOR_GET_RTCP	2

#ifdef ACMW_PLAYBACK
/* For record playback status */
#define PB_END	0
#define PB_TDM	1
#define PB_NET	2
#define PB_SIL	3

#define G711_FRAME_SIZE	80	/* 80 byte, 10 msec*/
#define G729_FRAME_SIZE	10	/* 10 byte, 10 msec*/
#define G723_FRAME_SIZE	24	/* 24 byte, 30 msec*/
#endif

/* use this list to get the status of each routine */
typedef enum 
{
	AC494_DSP_ERROR_E = -1,
	AC494_DSP_OK_E	= 0,
	AC494_DSP_NO_MODULE_E = -2,
	AC494_DSP_NO_DOWNLOAD_E = -3,
	AC494_DSP_NO_INIT_E = -4
} AC494_DSP_STATUS_ENT;

enum bypass_mode
{
	FAX_BYPASS = 0,
	MODEM_BYPASS = 1
};

typedef struct
{
	char fsk_cid_valid;
	char dtmf_cid_valid;
	char number[25];			/* caller id - number, if caller id is bellcore SDMF number may eq 'P' or 'O' represent private or out-of-area */
	char date[9];				/* caller id - date */
	char cid_name[51];
	char cid_length;
	char called_number[26];			/* for japan DID usage */
	char visual_indicator;			/* 0: indicator absence of waiting messages, 1:indicator presence of waiting messages */
	char number_absence;			/* !=0: indicator private or whatever absence number */
	char name_absence;			/* !=0: indicator private or whatever absence name */
	unsigned char byte_buf[256];		/* the fsk caller id orignal data. user can re verify the data. or re decode */
						/* this byte_buf for new country caller id spec: */
}
TstCidDet;

typedef struct
{
	int channel;
	/* DTMF Detection */
	char dtmf_digit;
	int long_dtmf;		// 1: is long dtmf event, 0: not long dtmf event
	/* Modem/Fax Tone Detection */
	int modem_flag;
	int ced_flag;
	int cng_flag;
	/* IBS Detection */
	int IBS_CP;		// IBS Call Progress detection result
	int Ibs_level;		// Signal level ( in -dBm). (This field represents the level of the signal during the detection frame only)
	/* Energy Detection */
	unsigned short energy_dBm;
	/* Caller ID */
	int fsk_cid_valid;
	int dtmf_cid_valid;
	char *pCidMsg;
	char cid_type;
	char cid_num[25];
	char cid_date_time[9];
	char cid_name[51];
	char num_absence;
	char name_absence;
	char visual_indicator;
	
	/* RTP statistic */
	unsigned int RxRtpStatsCountByte;
	unsigned int RxRtpStatsCountPacket;
	unsigned int RxRtpStatsLostPacket;
	unsigned int TxRtpStatsCountByte;
	unsigned int TxRtpStatsCountPacket;
	
}TeventDetectionResult;

typedef struct
{
	Tac49x3WayConferenceMode acmw_3way_mode_set;
	int acmw_3way_is_set;
}TAcmw3WayState;

/* Function Prototype */
int RtkAc49xApiOffhookAction(int chid);
int RtkAc49xApiOnhookAction(int chid);
int RtkAc49xApiActiveRegularRtp(int chid, int sid);
int RtkAc49xApiCloseRegularRtp(int chid, int sid);
void RtkAc49xApiEventPolling(TeventDetectionResult *result);
int RtkAc49xApiSetCountryTone(int toneOfCountry);
int RtkAc49xApiPlayTone(int chid, int sid, int Tone, int ToneOnOff, int ToneDirection);
int RtkAc49xApiGenDtmfTone(int chid, int dmtf_digit, int on_duration, int off_duration, int direction);
int RtkAc49xApiSendOutbandDtmfEvent(int chid, int dmtf_digit, int duration);
int RtkAc49xApiSetRtpChannelConfiguration(int chid, int sid, int payload_type, int g723_type, int bVAD);
int RtkAc49xApiSetEchoCanceller(int chid, Tac49xEchoCancelerMode ec_mode);
int RtkAc49xApiSetEchoCancellerNLP(int chid, Tac49xEchoCancelerNonLinearProcessor nlp_mode);
int RtkAc49xApiActiveEchoCancellerNLP(int chid);
int RtkAc49xApiUpdateEchoCancellerLength(int chid, Tac49xEchoCancelerLength ec_length);
int RtkAc49xApi_LEC_enable(int chid);
int RtkAc49xApi_LEC_disable(int chid);
int RtkAc49xApiSetVoiceGain(int chid, Tac49xVoiceGain in_gain, Tac49xVoiceGain out_gain);
int RtkAc49xApiSetFaxAndCallerIdLevel(int chid, Tac49xFaxAndCallerIdLevel level);
int RtkAc49xApiSetCallerIdDetection(int chid, Tac49xControl on_hook_st, Tac49xControl off_hook_st, Tac49xCallerIdStandard standard);
int RtkAc49xApiSetVoiceJBDelay(int chid, int max_delay, int min_delay, Tac49xDynamicJitterBufferOptimizationFactor jb_factor);
int RtkAc49xApiActiveOrDeactive3WayConference(int active_chid, Tac49x3WayConferenceMode mode);
Tac49x3WayConferenceMode RtkAc49xApiGet3WayMode(int active_chid);
int RtkAc49xApiAgcDeviceConfig(int min_gain, int max_gain);
int RtkAc49xApiAgcConfig(int chid, Tac49xControl agc_enable, Tac49xAgcLocation agc_location);
int RtkAc49xApiAgcEnergySlope(int chid, Tac49xAgcTargetEnergy agc_target_energy, Tac49xAgcGainSlope agc_slope);
int RtkAc49xApiSetIbsTransferMode(int chid, Tac49xIbsTransferMode ibs_mode);
int RtkAc49xApiSetIBSDetectionRedirection(int chid, Tac49xIbsDetectionRedirection ibs_redirection);
int RtkAc49xApiSetCallProgressIBSDetection(int chid, Tac49xControl enable);
int RtkAc49xApiSetDtmfDetection(int chid, Tac49xControl enable);
int RtkAc49xApiSetLongDtmfDetection(int chid, Tac49xControl enable);
int RtkAc49xApiSetDtmfErasureMode(int chid, Tac49xDtmfErasureMode dtmf_erasure_mode);
Tac49xDriverVersionInfo RtkAc49xApiGetDriverVersionInfo(void);
int RtkAc49xApiSetFaxTransferMethod(int chid, Tac49xFaxModemTransferMethod  fax_modem_trans_mode);
Tac49xFaxModemTransferMethod RtkAc49xApiGetFaxTransferMethod(int chid);
int RtkAc49xApiSetCedTransferMode(int chid, Tac49xCedTransferMode ced_trans_mode);
int RtkAc49xApiSetCngRelay(int chid, Tac49xControl enable);
int RtkAc49xApiSetIbsLevel(int chid, int ibs_level_minus_dbm);
int RtkAc49xApiSetFskCallerIdParam(int chid, unsigned char service_type, unsigned char msg_type, unsigned char *pCallerID, unsigned char *pDate_time, unsigned char *pName);
int RtkAc49xApiSendFskCallerId(int chid, unsigned char service_type, unsigned char msg_type, unsigned char *pCallerID, unsigned char *pDate_time, unsigned char *pName);
int RtkAc49xApiSendVmwi(int chid, unsigned char vmwi_msg_type, char *pIndicator);
int RtkAc49xApiSendDtmfCallerId(int chid, unsigned char type, unsigned char *pMessage);
int RtkAc49xApiResetRtpStatistics(int chid);
int RtkAc49xApiGetRtpStatistics(int chid);
int RtkAc49xApiSetupDevice(void);
int RtkAc49xApiSetRtcpTxInterval(int chid, unsigned short rtcp_tx_interval);
unsigned short RtkAc49xGetChannelState(int chid);
int RtkAc49xApiSetInputGainLocation(int chid, Tac49xInputGainLocation in_gain_location);
int RtkAc49xApiDetectNetIbsPackets(int chid, Tac49xControl enable);
int RtkAc49xApiSetIntoBypassMode(int chid, int bypass);
int RtkAc49xApiPlayIvrTdmStart(int chid, Tac49xCoder coder);
int RtkAc49xApiPlayIvrNetworkStart(int chid, Tac49xCoder coder);
int RtkAc49xApiPlaySilence(int chid, int silence_msec);
int RtkAc49xApiPlayIvr(int chid, char *pBuf, int BufSize);
int RtkAc49xApiPollIvr(int chid);
int RtkAc49xApiPlayIvrEnd(int chid);
int RtkAc49xApiPlayBackSetting(int chid, int water_mark_msec, Tac49xControl Request_enable);
unsigned char RtkAc49xApiMdmfCidParaParsing(unsigned char *pParaStart, TeventDetectionResult *pRes);
void RtkAc49xApiSdmfCidParaParsing(unsigned char *pParaStart, unsigned char sdmf_msg_len, TeventDetectionResult *pRes);
unsigned char RtkAc49xApiNttCidParsing(unsigned char *pParaStart, unsigned char cid_len, TeventDetectionResult *pRes);
unsigned char RtkAc49xApiNttCidCidcwParsing(unsigned char *pParaStart, TeventDetectionResult *pRes);
void RtkAc49xApiFskCidParsing(unsigned char *pcid_msg, TeventDetectionResult *res);
#ifdef ACMW_PLAYBACK
unsigned char SetTextPlayAddr(unsigned char chid, unsigned char* p, unsigned int len);
int IvrPlayBufWrite(unsigned char chid, char* pBuf, int frame_len, int frame_size);
int IvrPlayBufRead(unsigned char chid, int frame_len, int* index);
#endif

#endif


