/**
 * @file voip_manager.h
 * @brief VoIP control API
 */

#ifndef __VOIP_MANAGER_H
#define __VOIP_MANAGER_H

#include "voip_types.h"
#include "voip_flash.h"
#include "voip_params.h"
#include "voip_control.h"

#define MAX_SELF_TONE			8
#define MAX_CUSTOME				4

#define PHONE_ON_HOOK			0
#define PHONE_OFF_HOOK			1
#define PHONE_FLASH_HOOK		2
#define PHONE_STILL_ON_HOOK		3
#define PHONE_STILL_OFF_HOOK	4
#define PHONE_UNKNOWN			5

#define DAA_FLOW_NORMAL			0
#define DAA_FLOW_3WAY_CONFERENCE	1
#define DAA_FLOW_CALL_FORWARD		2

#define DAA_CH	2

#ifdef CONFIG_RTK_VOIP_SRTP
#define SRTP_KEY_LEN 30
#endif
/**
 * @ingroup VOIP_PHONE
 * Enumeration for phone state
 */
typedef enum
{
    SIGN_NONE = 0 ,
    SIGN_KEY1 = 1 ,
    SIGN_KEY2 = 2 ,
    SIGN_KEY3 = 3 ,
    SIGN_KEY4 = 4,
    SIGN_KEY5 = 5 ,
    SIGN_KEY6 = 6 ,
    SIGN_KEY7 = 7 ,
    SIGN_KEY8 = 8 ,
    SIGN_KEY9 = 9 ,
    SIGN_KEY0 = 10 ,
    SIGN_STAR = 11,
    SIGN_HASH = 12 ,
    SIGN_ONHOOK = 13 ,
    SIGN_OFFHOOK = 18,
    SIGN_FLASHHOOK = 19,
    SIGN_AUTODIAL = 20,
    SIGN_OFFHOOK_2 = 21,
    SIGN_RING_ON = 22,
    SIGN_RING_OFF = 23
} SIGNSTATE ;

/**
 * @ingroup VOIP_PHONE
 * Enumeration for FXO event
 */
typedef enum
{
    FXOEVENT_NONE = 0 ,
    FXOEVENT_KEY1 = 1 ,
    FXOEVENT_KEY2 = 2 ,
    FXOEVENT_KEY3 = 3 ,
    FXOEVENT_KEY4 = 4,
    FXOEVENT_KEY5 = 5 ,
    FXOEVENT_KEY6 = 6 ,
    FXOEVENT_KEY7 = 7 ,
    FXOEVENT_KEY8 = 8 ,
    FXOEVENT_KEY9 = 9 ,
    FXOEVENT_KEY0 = 10 ,
    FXOEVENT_STAR = 11,
    FXOEVENT_HASH = 12 ,
    FXOEVENT_RING_START = 13 ,
    FXOEVENT_RING_STOP = 14,
    //FXOEVENT_PSTN_OFFHOOK = 15,	// not support yet
    //FXOEVENT_PSTN_ONHOOK = 16,	// not support yet
    FXOEVENT_BUSY_TONE = 17,
    FXOEVENT_DIST_TONE = 18,
    FXOEVENT_POLARITY_REVERSAL = 19,
    FXOEVENT_BAT_DROP_OUT = 20,
    FXOEVENT_MAX = 21
} FXOEVENT;

/**
 * @ingroup VOIP_SESSION_FAX
 * Enumeration for fax state
 */
typedef enum
{
    FAX_IDLE = 0,
    LOW_SPEED_FAX_DETECT ,
    HIGH_SPEED_FAX_DETECT ,
    MODEM_DETECT,
    FAX_RUN,
    MODEM_RUN
} FAXSTATE;


/**
 * @ingroup VOIP_PHONE
 * Enumeration for Country
 */
typedef enum
{
    COUNTRY_USA = 0,
    COUNTRY_UK,
    COUNTRY_AUSTRALIA,
    COUNTRY_HK,
    COUNTRY_JAPAN,
    COUNTRY_SWEDEN,
    COUNTRY_GERMANY,
    COUNTRY_FRANCE,
#if 0
    COUNTRY_TR57,
#else
    COUNTRY_TW,
#endif
    COUNTRY_BELGIUM,
    COUNTRY_FINLAND,
    COUNTRY_ITALY,
    COUNTRY_CHINA,
    COUNTRY_CUSTOMER,
    COUNTRY_MAX
}_COUNTRY_;

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief Structure for RTP handling <br>
 *
 * In RTP, payload type 13 is used for CN. The G.711 main codec will use a PT of "0" for PCMU and "8" for PCMA. RTP packets switch Payload type (PT) between 13 and 0 or 8 for sending VAD and speech packets. In some RFC drafts, a PT of 19 is also considered, and subsequently, a PT of 19 was updated with 13.<p>
 */
typedef struct
{
    uint32 chid;        ///< channel id 
    uint32 sid;  		///< session id. Usually 0 for master session, 1 for slave(conference) session.
    uint32 isTcp;       ///< Not support yet. tcp = 1 or udp = 0
    uint32 remIp;       ///< remote IP address for RTP
    uint16 remPort;		///< remote RTP port number
    uint32 extIp;       ///< local IP address for RTP
    uint16 extPort;		///< local RTP port number
    uint8 tos;		///< TOS field in IP header. Usually defined for QoS.

   /** @brief local rfc2833 payload type<br>
     * rfc2833_payload_type_locale MUST be set from application to DSP. 
     * If local side doesn't support RFC2833, application should set it to ZERO, otherwise, set it to proper payload type. 
 	 */
	uint16 rfc2833_payload_type_local;	// local rfc2833 payload type
   /** @brief remote rfc2833 payload type<br>
     * rfc2833_payload_type_remote MUST be set from application to DSP. 
     * If remote side doesn't support RFC2833, application should set it to ZERO.
     * Application negotiates RFC2833 payload type with remote peer UA.
     *
 	 */
	uint16 rfc2833_payload_type_remote;	// remote rfc2833 payload type
#ifdef CONFIG_RTK_VOIP_SRTP
	unsigned char remoteSrtpKey[SRTP_KEY_LEN]; 	///< remote SRTP key
	unsigned char localSrtpKey[SRTP_KEY_LEN];	///< local SRTP key

   /** @brief Remote crypt algorithm<br>
 	 * Valid values are:<br>
     * @ref NULL_AUTH 
     * @ref UST_TMMHv2 
     * @ref UST_AES_128_XMAC 
     * @ref HMAC_SHA1 
     * @ref STRONGHOLD_AUTH
 	 */
	int remoteCryptAlg;		
   /** @brief local crypt algorithm<br>
 	 * Valid values are:<br>
     * @ref NULL_AUTH 
     * @ref UST_TMMHv2 
     * @ref UST_AES_128_XMAC 
     * @ref HMAC_SHA1 
     * @ref STRONGHOLD_AUTH
 	 */
	int localCryptAlg;		
#endif /*CONFIG_RTK_VOIP_SRTP*/
#ifdef SUPPORT_RTP_REDUNDANT
	uint16 rtp_redundant_payload_type_local;	///< Payload type of local redundant rtp. 0 means disable rtp redundant.
	uint16 rtp_redundant_payload_type_remote;	///< Payload type of remote redundant rtp 0 means disable rtp redundant.
#endif
	uint16 SID_payload_type_local;		///< Payload type of local SID (Silence Insertion Description). 13 is recommend.
	uint16 SID_payload_type_remote;		///< Payload type of remote SID (Silence Insertion Description). 13 is recommend.
	uint32 rtcp_xr_enable;			///< enable RTCP XR support (RTCP only)
	uint16 init_randomly;		///< initialize seqno, SSRC and timestamp randomly, so ignore below parameters 
	uint16 init_seqno;			///< initial seqno 
	uint32 init_SSRC;			///< initial SSRC 
	uint32 init_timestamp;		///< initial timestamp 
} rtp_config_t;

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief Structure for RTP payload type <br>
 *
 */
typedef struct
{
    uint32 chid;					///< channel id
    uint32 sid;						///< session id. Usually 0 for master session, 1 for slave(conference) session.

    RtpPayloadType uPktFormat;		///< Codec of desired voice codec
	RtpPayloadType local_pt;		///< Payload type of local voice codec
	RtpPayloadType remote_pt;		///< Payload type of remote voice codec

    RtpPayloadType uPktFormat_vbd;	///< Codec type of voice band data (vbd). Usually used by V.152 FAX.
	RtpPayloadType local_pt_vbd;	///< Payload type of local voice band data (vbd).
	RtpPayloadType remote_pt_vbd;	///< Payload type in remote voice band data (vbd).

   /** @brief G.723.1 rate. There are two bit rates at which G.723.1 can operate.<br>
 	 * Valid values are:<br>
 	 * 0: 6.3 kbit/s (using 24 byte frames) using a MPC-MLQ algorithm (MOS 3.9)<br>
 	 * 1: 5.3 kbit/s (using 20 byte frames) using an ACELP algorithm (MOS 3.62)
 	 */
    int32 nG723Type;				// see __G7231_RATE__
    int32 nFramePerPacket;			// reserved

   /** @brief Enable/Disable VAD (Voice activity detection)<br>
 	 * Valid values are:
 	 * 0: Disable
 	 * 1: Enable
 	 */
    int32 bVAD;                     // Vad on: 1, off: 0
	
   /** @brief Enable/Disable PLC (Packet Loss Concealment)<br>
 	 * Valid values are:
 	 * 0: Disable
 	 * 1: Enable
 	 */
	int32 bPLC;
	
   /** @brief minimun jitter delay (ms)<br>
     * nJitterDelay is the minimum delay of jitter buffer.<br>
 	 * Valid values are 40ms to 100ms. Recommended value is 40ms.
 	 */
    uint32 nJitterDelay;			// min delay
   /** @brief maximum jitter delay (ms)<br>
     * nJitterDelay is the maximum delay of jitter buffer.<br>
 	 * Valid values are 130ms to 300ms. Recommended value is 130ms.
 	 */
    uint32 nMaxDelay;				// max delay
   /** @brief optimzation factor of jitter buffer<br>
	 * The optimization factor is the value for adjusting the quality of the voice. 
	 * The higher value of nJitterFactor means the better quality but more delay. 
	 * Valild value are 0 to 13. The medium value 7 is recommended. 
	 * Value 0 and 13 are fix delay(fifo). value 0 is for low buffering application. 
	 * value 13 is for FAX or modem only.
 	 */
    uint32 nJitterFactor;			// optimzation factor of jitter buffer 
   /** @brief G726 packing order<br>
     * Valid values are:<br>
     * 0: Packing none 
     * 1: Packing left 
     * 2: Packing right
 	 */
    uint32 nG726Packing;			// G726 packing order 

    uint32 bT38ParamEnable;			///< Enable T.38 parameter. If false, default settings are adopted and nT38MaxBuffer and nT38RateMgt are ignored. 

   /** @brief T.38 Max buffer size<br>
     * Valid valures are 200 to 600 (ms) <br>
     * Default is 500 (ms)
 	 */
    uint32 nT38MaxBuffer;			// T.38 Max buffer size. It can be 200 to 600ms. (default: 500)
   /** @brief T38 Rate management<br>
     * Valid values are:<br>
     * 1: don't pass tcf data 
     * 2: pass tcf data (Default)
 	 */
    uint32 nT38RateMgt;				// T38 Rate management. 1: don't pass tcf data, 2: pass tcf data. (default: 2)
   /** @brief T38 Maximum rate<br>
     * Valid values are:<br>
     * 0: 2400 bps
     * 1: 4800 bps
     * 2: 7200 bps
     * 3: 9600 bps
     * 4: 12000 bps
     * 5: 14400 bps (Default)
 	 */
	uint32 nT38MaxRate;				// T38 Max reate. 0~5: 2400, 4800, 7200, 9600, 12000, 14400. (default: 5)
   /** @brief T38 ECM<br>
     * Valid values are:<br>
     * 0: disable 
     * 1: enable (Default)
 	 */
	uint32 bT38EnableECM;			// T38 ECM. 1: enable, 0: disable. (default: 1)
   /** @brief T38 ECC signal<br>
     * Valid values are: 0~7<br>
     * Default value: 5
 	 */
	uint32 nT38ECCSignal;			// T38 ECC signal. 0~7 (default: 5)
   /** @brief T38 ECC data<br>
     * Valid values are: 0~2<br>
     * Default value: 2
 	 */
	uint32 nT38ECCData;				// T38 ECC data. 0~2 (default: 2)
   /** @brief PCM Mode Setting<br>
     * Valid values are:<br>
     * 0: No action (use previous PCM enable mode)<br> 
     * 1: Auto mode (DSP will change to proper mode according to the voice codec attribute)<br> 
     * 2: Narrow-band Mode<br>
     * 3: Wide-band Mode
 	 */
    uint32 nPcmMode;				// PCM Mode Setting. 
} payloadtype_config_t;

/** 
 * @ingroup VOIP_DSP
 * @brief Get VoIP Feature
 * @retval 0 Success
 */
int32 rtk_Get_VoIP_Feature(void);

/**
 * @ingroup VOIP_DSP
 * @brief Interface Initialization
 * @param ch The channel number.
 * @retval 0 Success
 */
int32 rtk_InitDSP(int ch);

/**
 * @ingroup VOIP_DSP
 * @brief Set echo tail for LEC
 * @param chid The channel number.
 * @param echo_tail The echo tail length (ms). 
 * @param nlp Non-linear Processor 0: off, 1: on 
 * @retval 0 Success
 * @note 1~16ms in G.711, 1~4ms in G.729/G.723 
 */
int32 rtk_Set_echoTail(uint32 chid, uint32 echo_tail, uint32 nlp);

/**
 * @ingroup VOIP_DSP
 * @brief Light LED if SIP Register OK
 * @param chid The channel number.
 * @param isOK 1: SIP register OK, 0: SIP register failed
 * @retval 0 Success
 */
int rtk_sip_register(unsigned int chid, unsigned int isOK);

/**
 * @ingroup VOIP_PHONE
 * @brief Reinit DSP for FXS/FXO channel when on hook
 * @param chid The channel number.
 * @retval 0 Success
 */  
int32 rtk_Onhook_Reinit(uint32 chid);

/**
 * @ingroup VOIP_PHONE
 * @brief Get the FXS state
 * @param chid The channel number.
 * @param pval The phone state.
 * @retval 0 Success
 * @sa rtk_Offhook_Action, rtk_Onhook_Action
 */  
int32 rtk_GetFxsEvent(uint32 chid, SIGNSTATE *pval);

/**
 * @ingroup VOIP_PHONE
 * @brief Get the FXO state
 * @param chid The channel number.
 * @param pval The FXO state.
 * @retval 0 Success
 * @sa rtk_Offhook_Action, rtk_Onhook_Action
 */ 
int32 rtk_GetFxoEvent(uint32 chid, SIGNSTATE *pval);

/**
 * @ingroup VOIP_PHONE
 * @brief Get the DECT state
 * @param chid The channel number.
 * @param pval The phone state.
 * @retval 0 Success
 * @sa rtk_Offhook_Action, rtk_Onhook_Action
 */  
int32 rtk_GetDectEvent(uint32 chid, SIGNSTATE *pval);

/**
 * @ingroup VOIP_PHONE
 * @brief Get the FXO event
 * @param chid The channel number.
 * @param pval The FXO event.
 * @retval 0 Success
 * @sa rtk_Offhook_Action, rtk_Onhook_Action
 */ 
int32 rtk_GetRealFxoEvent(uint32 chid, FXOEVENT *pval);

/** 
 * @ingroup VOIP_FXS
 * @brief Setup the flash hook time
 * @param chid The channel number.
 * @param min_time The min time period for flash hook.
 * @param time The max time period for flash hook.
 * @retval 0 Success
 */
int32 rtk_Set_Flash_Hook_Time(uint32 chid, uint32 min_time, uint32 time);

/**
 * @ingroup VOIP_FXS_RING
 * @brief Enable/Diable FXS Ringing 
 * @param chid The FXS channel number.
 * @param bRinging 1: enable ring, 0: disable ring
 * @retval 0 Success
 */  
int32 rtk_SetRingFXS(uint32 chid, uint32 bRinging);

/**
 * @ingroup VOIP_DSP
 * @brief Set VAD/CNG Threshold
 * @param chid The channel number.
 * @param mid The media session number.
 * @param nThresVAD VAD Threshold
 * @param nThresCNG CNG Threshold
 * @param nModeCNG CNG Configuration Mode
 * @param nLevelCNG CNG Noise Level
 * @param nGainCNG CNG Gain Adjustment
 * @retval 0 Success
 */  
int32 rtk_SetThresholdVadCng( int32 chid, int32 mid, int32 nThresVAD, int32 nThresCNG, int32 nModeCNG, int32 nLevelCNG, int32 nGainCNG );

/*
 * @ingroup VOIP_FXS_RING
 * @brief Disable all FXS Ring
 * @param bDisable flag to disable all FXS Ring
 * @retval 0 Success
 * @note Test Issue.
 */  
int32 rtk_DisableRingFXS(int bDisable);

/**
 * @ingroup VOIP_FXS_RING
 * @brief Set the Ring Cadence of FXS
 * @param chid The FXS channel number.
 * @param cad_on_msec The time period of Cadence-On.
 * @param cad_off_msec The time period of Cadence-Off.
 * @retval 0 Success
 */  
int32 rtk_Set_SLIC_Ring_Cadence(uint32 chid, uint16 cad_on_msec, uint16 cad_off_msec);

/**
 * @ingroup VOIP_FXS_RING
 * @brief Set the Ring frequency and amputide of FXS
 * @param chid The FXS channel number.
 * @param preset The Ring frequency, amputide preset table index.
 * @retval 0 Success
 */  
int32 rtk_Set_SLIC_Ring_Freq_Amp(uint32 chid, uint8 preset);

/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Set SLIC line voltage
 * @param TstVoipValue.ch_id Channel ID
 * @param TstVoipValue.value SLIC line voltage flag. 0: zero voltage, 1: normal voltage, 2: reverse voltage
 * @retval 0 Success
 */
int32 rtk_Set_SLIC_Line_Voltage(uint32 chid, uint8 flag);

/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Generate SLIC CPC signal
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.cfg SLIC CPC signal period (unit in ms)
 * @retval 0 Success
 */
int32 rtk_Gen_SLIC_CPC(uint32 chid, uint32 cpc_ms);


/**
 * @ingroup VOIP_FXO
 * @brief Set the Ring Detection Parameters of FXO
 * @param chid The FXO channel number.
 * @param ring_on_msec The time threshold of the Ring On.
 * @param first_ringoff_msec The time threshold of the First Ring Off.
 * @param ring_off_msec The time threshold of the Ring Off.
 * @retval 0 Success
 */
int32 rtk_Set_FXO_Ring_Detection(uint16 ring_on_msec, uint16 first_ringoff_msec, uint16 ring_off_msec);

/**
 * @ingroup VOIP_FXS_CALLERID
 * @brief Caller ID generation via DTMF
 * @param chid The FXS channel number.
 * @param str_cid The Caller ID
 * @retval 0 Success
 */  
int32 rtk_Gen_Dtmf_CID(uint32 chid, char *str_cid);

/**
 * @ingroup VOIP_FXS_CALLERID
 * @brief Set the DTMF Caller ID Generation Mode
 * @param chid The FXS channel number.
 * @param area set the before ring or after ring send cid
 *                  bit 0, 1: Area -> 0: Bellcore, 1: ETSI, 2: BT, 3: NTT 
 *                  bit 3: Caller ID Prior First Ring, [DTMF & FSK, other bit used in FSK mode only]
 *                  bit 4: Dual Tone before Caller ID (Fsk Alert Tone), 
 *		    bit 5: Short Ring before Caller ID, 
 *                  bit 6: Reverse Polarity before Caller ID (Line Reverse),
 *		    bit 7: FSK Date & Time Sync and Display Name
 * @param cid_dtmf_mode set the caller id start/end digit
 *                  bit 0, 1: DTMF START DIGIT -> 0: A, 1: B, 2: C, 3: D.
 *                  bit 2, 3: DTMF END DIGIT -> 0: A, 1: B, 2: C, 3: D.
 *		    bit 4: Auto start/end digit send
 *			0 -> auto mode: DSP will send start/end digit according to the bit 0-3 setting automatically
 *			1 -> non-auto mode: DSP send caller ID string only. If caller ID need start/end digits, developer should add them to caller ID strings.
 * @retval 0 Success
 */
int32 rtk_Set_CID_DTMF_MODE(uint32 chid, char area, char cid_dtmf_mode);

/**
 * @ingroup VOIP_FXS_CALLERID
 * @brief FSK Caller ID Generation
 * @param chid The FXS channel number.
 * @param str_cid The Caller ID
 * @param str_date The Date
 * @param str_cid_name The Caller ID Name
 * @param mode 0: type I, 1: type II
 * @retval 0:Success, -2: Busy state
 */  
int32 rtk_Gen_FSK_CID(uint32 chid, char *str_cid, char *str_date, char *str_cid_name, char mode);

/**
 * @ingroup VOIP_FXS_CALLERID
 * @brief FSK Caller ID Generation
 * @param chid The FXS channel number.
 * @param pClid The pointer of FSK Caller ID Data
 * @param num_clid_element The number of FSK Caller ID element
 * @retval 0:Success, -2: Busy state
 */ 
int32 rtk_Gen_MDMF_FSK_CID(uint32 chid, TstFskClid* pClid, uint32 num_clid_element);

/**
 * @ingroup VOIP_FXS_CALLERID
 * @brief Caller ID generation and FXS Ring
 * @param chid The FXS channel number.
 * @param cid_mode The mode of Caller ID 0:DTMF, 1:FSK
 * @param str_cid The Caller ID
 * @param str_date The Date
 * @param str_cid_name The Caller ID Name
 * @param fsk_type 0: type I, 1: type II
 * @param bRinging 0: disable Ring, 1: enable Ring
 * @Note When str_cid is NULL(no cid number), this API won't send cid, and then just perform disable/enable FXS Ring
 * @retval 0 Success
 */  
int32 rtk_Gen_CID_And_FXS_Ring(uint32 chid, char cid_mode, char *str_cid, char *str_date, char *str_cid_name, char fsk_type, uint32 bRinging);

/**
 * @ingroup VOIP_FXS_CALLERID
 * @brief Set the FSK Area
 * @param chid The FXS channel number.
 * @param area The area of FSK. (see _CID_AREA_),
 *                  bit 0, 1: Area -> 0: Bellcore, 1: ETSI, 2: BT, 3: NTT 
 *                  bit 3: Caller ID Prior First Ring, 
 *                  bit 4: Dual Tone before Caller ID (Fsk Alert Tone),
 *		    bit 5: Short Ring before Caller ID, 
 *                  bit 6: Reverse Polarity before Caller ID (Line Reverse),
 *		    bit 7: FSK Date & Time Sync and Display Name
 * @retval 0 Success
 * @sa _CID_AREA_
 */
int32 rtk_Set_FSK_Area(uint32 chid, char area);

/**
 * @ingroup VOIP_FXS_CALLERID
 * @brief Set the FSK Caller ID Parameters
 * @param chid The FXS channel number.
 * @param para The variable pointer of FSK Caller ID Parameters
 * @retval 0 Success
 */
int32 rtk_Set_FSK_CLID_Para(TstVoipFskPara* para);

/**
 * @ingroup VOIP_FXS_CALLERID
 * @brief Get the FSK Caller ID Parameters
 * @param chid The FXS channel number.
 * @param para The variable pointer to save current FSK Caller ID Parameters
 * @retval 0 Success
 */
int32 rtk_Get_FSK_CLID_Para(TstVoipFskPara* para);

/**
 * @ingroup VOIP_FXS_CALLERID
 * @brief get the fsk caller id send complete or not.
 * @param chid The FXS channel number.
 * @param cid_state 0:fsk cid send complete 1:sending fsk cid
 */
int32 rtk_GetFskCIDState(uint32 chid, uint32 *cid_state);

/**
 * @ingroup VOIP_FXS_CALLERID
 * @brief Set the FSK Caller ID Generation Mode,
 * @param chid The FXS channel number.
 * @param isOK 0: hardward gen fsk caller id, 1: software gen fsk caller id
 * @retval 0 Success
 */
int32 rtk_Set_CID_FSK_GEN_MODE(unsigned int chid, unsigned int isOK);

/**
 * @ingroup VOIP_FXS_CALLERID
 * @brief Generate the VMWI via FSK
 * @param chid The FXS channel number.
 * @param state The address of value to set VMWI state. (0: off, 1: on)
 * @param mode 0: type I, 1: type II
 * @retval 0:Success, -2: Busy state
 */  
int32 rtk_Gen_FSK_VMWI(uint32 chid, char *state, char mode);

/**
 * @ingroup VOIP_FXS_CALLERID
 * @brief Caller ID and VMWI generation via FSK
 * @param chid The FXS channel number.
 * @param str_cid The Caller ID or VMWI
 * @param str_date The Date
 * @param str_cid_name The Caller ID Name
 * @param mode 0: type I, 1: type II
 * @param msg_type FSK_MSG_CALLSETUP:cid or FSK_MSG_MWSETUP:vmwi
 * @retval 0:Success, -2: Busy state
 */
int32 rtk_Gen_FSK_CID_VMWI(uint32 chid, char *str_cid, char *str_date, char *str_cid_name, char mode, char msg_type);

/**
 * @ingroup VOIP_DSP
 * @brief Set the speaker and mic voice gain
 * @param chid The channel number.
 * @param spk_gain -32~31 dB (-32: mute, default is 0 dB)
 * @param mic_gain -32~31 dB (-32: mute, default is 0 dB)
 * @retval 0 Success
 */
int32 rtk_Set_Voice_Gain(uint32 chid, int spk_gain, int mic_gain);

/**
 * @ingroup VOIP_DSP
 * @brief Set answet tone detection
 * @param chid The channel number.
 * @param conifg bit0: CNG_TDM, bit1:ANS_TDM, bit2:ANSAM_TDM, bit3:ANSBAR_TDM,
 *               bit4: ANSAMBAR_TDM, bit5: BELLANS_TDM, bit6: V22_TDM, bit7: V8bisCre_TDM
 *               bit8: CNG_IP, bit9:ANS_IP, bit10:ANSAM_IP, bit11:ANSBAR_IP,
 *               bit12: ANSAMBAR_IP, bit13: BELLANS_IP, bit14: V22_IP, bit15: V8bisCre_IP
 *               bit16: V21flag_TDM, bit17: V21flag_IP
 * @retval 0 Success
 */
int32 rtk_Set_AnswerTone_Det(uint32 chid, uint32 config);

/**
 * @ingroup VOIP_DSP
 * @brief rtk_Set_SPK_AGC
 * @param chid The channel number.
 * @param support_gain support_gain
 * @retval 0 Success
 */
int32 rtk_Set_SPK_AGC(uint32 chid, uint32 support_gain);

/**
 * @ingroup VOIP_DSP
 * @brief rtk_Set_SPK_AGC_LVL
 * @param chid The channel number.
 * @param level level
 * @retval 0 Success
 */
int32 rtk_Set_SPK_AGC_LVL(uint32 chid, uint32 level);

/**
 * @ingroup VOIP_DSP
 * @brief rtk_Set_SPK_AGC_GUP
 * @param chid The channel number.
 * @param gain gain
 * @retval 0 Success
 */
int32 rtk_Set_SPK_AGC_GUP(uint32 chid, uint32 gain);

/**
 * @ingroup VOIP_DSP
 * @brief rtk_Set_SPK_AGC_GDOWN
 * @param chid The channel number.
 * @param gain gain
 * @retval 0 Success
 */
int32 rtk_Set_SPK_AGC_GDOWN(uint32 chid, uint32 gain);

/**
 * @ingroup VOIP_DSP
 * @brief rtk_Set_MIC_AGC
 * @param chid The channel number.
 * @param support_gain support_gain
 * @retval 0 Success
 */
int32 rtk_Set_MIC_AGC(uint32 chid, uint32 support_gain);

/**
 * @ingroup VOIP_DSP
 * @brief rtk_Set_MIC_AGC_LVL
 * @param chid The channel number.
 * @param level level
 * @retval 0 Success
 */
int32 rtk_Set_MIC_AGC_LVL(uint32 chid, uint32 level);

/**
 * @ingroup VOIP_DSP
 * @brief rtk_Set_MIC_AGC_GUP
 * @param chid The channel number.
 * @param gain gain
 * @retval 0 Success
 */
int32 rtk_Set_MIC_AGC_GUP(uint32 chid, uint32 gain);

/**
 * @ingroup VOIP_DSP
 * @brief rtk_Set_MIC_AGC_GDOWN
 * @param chid The channel number.
 * @param gain gain
 * @retval 0 Success
 */
int32 rtk_Set_MIC_AGC_GDOWN(uint32 chid, uint32 gain);

/**
 * @ingroup VOIP_FXO_VIRTUAL_DAA
 * @param chid The FXS channel number
 * @brief Switch virtual DAA realy to PSTN line
 * @retval 1 Success, 0xff Line not connect or busy or not support
 * @note This API is only used for Virtual DAA
 */
int rtk_DAA_off_hook(uint32 chid);

/* Obsolete API (replaced by rtk_GetFxoEvent)
 * @ingroup VOIP_FXO
 * @param chid The specified FXS channel for FXO
 * @brief Check Ringing via PSTN line
 * @retval 1 PSTN Ringing
 */
int rtk_DAA_ring(uint32 chid);

/**
 * @ingroup VOIP_FXO
 * @param chid The FXO channel number
 * @brief On-Hook in PSTN line
 * @retval 0 Success
 */
int rtk_DAA_on_hook(uint32 chid);

/* Obsolete API (replaced by rtk_Set_Voice_Gain)
 * @ingroup VOIP_FXO_GAIN
 * @brief Set the Tx Gain of FXO
 * @param gain The gain value (1..10).
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int32 rtk_Set_DAA_Tx_Gain(uint32 gain);

/* Obsolete API (replaced by rtk_Set_Voice_Gain)
 * @ingroup VOIP_FXO_GAIN
 * @brief Set the Rx Gain of FXO
 * @param gain The gain value (1..10).
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int32 rtk_Set_DAA_Rx_Gain(uint32 gain);

/** 
 * @ingroup VOIP_FXO_CALLERID
 * @brief Get the FXO Detected Caller ID
 * @param chid The FXO channel number
 * @param str_cid The Caller ID String
 * @param str_date The Date String
 * @param str_name The Caller ID Name
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int32 rtk_Get_DAA_CallerID(uint32 chid, char *str_cid, char *str_date, char *str_name);

/** 
 * @ingroup VOIP_FXO_CALLERID
 * @brief Set the FXO Caller ID Detection Mode
 * @param chid The FXO channel number
 * @param auto_det 0: Disable Caller ID Auto Detection,
 *                 1: Enable Caller ID Auto Detection (NTT Support),
 *                 2: Enable Caller ID Auto Detection (NTT Not Support)
 * @param cid_det_mode  The Caller ID Mode for Caller ID Detection
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int32 rtk_Set_CID_Det_Mode(uint32 chid, int auto_det, int cid_det_mode);

/* Obsolete API
 * @ingroup VOIP_FXO
 * @brief Set the DAA ISR Flow
 * @param chid The channel number
 * @param sid The session number. Usually 0 for master session, 1 for slave(conference) session.
 * @param flow The DAA ISR Flow => 0: Normal, 1: PSTN 3-way conference, 2: PSTN call forward
 * @retval 1: success
 * @retval 0xff: line not connect or busy or not support
 * @note Virtual DAA not support
 */
int32 rtk_Set_DAA_ISR_FLOW(unsigned int chid, unsigned int sid, unsigned int flow);

/* Obsolete API
 * @ingroup VOIP_FXO
 * @brief Get the DAA ISR Flow
 * @param chid The channel number
 * @param sid The session number. Usually 0 for master session, 1 for slave(conference) session.
 * @retval 0: Normal
 * @retval 1: PSTN 3-way conference
 * @retval 2: PSTN call forward
 * @note Virtual DAA not support
 */
int32 rtk_Get_DAA_ISR_FLOW(unsigned int chid, unsigned int sid);

/** 
 * @ingroup VOIP_FXO
 * @brief Dial PSTN Number for PSTN Call Forward
 * @param chid The FXO channel number
 * @param sid  The seesion number. Usually 0 for master session, 1 for slave(conference) session.
 * @param *cf_no_str The Pointer of the PSTN Number String
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int32 rtk_Dial_PSTN_Call_Forward(uint32 chid, uint32 sid, char *cf_no_str);

/* Obsolete API
 * @ingroup VOIP_FXO
 * @brief (Un)Hold PSTN Line
 * @param slic_chid The slic channel number
 * @param daa_chid The daa channel number
 * @param config  The configuration of Hold(config=1) or Un-Hold(config=0)
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int32 rtk_Set_PSTN_HOLD_CFG(unsigned int slic_chid, unsigned int daa_chid, unsigned int config);

/* Obsolete API (replaced by rtk_GetFxoEvent)
 * @ingroup VOIP_FXO
 * @brief Get if DAA detect the busy tone
 * @param daa_chid The daa channel number
 * @retval busy_flag Busy tone is  1: Detected, 0: NOT detected
 * @note Virtual DAA not support
 */
int32 rtk_Get_DAA_BusyTone_Status(unsigned int daa_chid);

/* Obsolete API
 * @ingroup VOIP_FXO
 * @brief Check which FXS channel is replaced by FXO 
 * @param chid The FXS channel number
 * @retval 1: TRUE, 0: FALSE
 * @note Virtual DAA not support
 */
int rtk_Get_DAA_Used_By_Which_SLIC(uint32 chid);

/**
 * @ingroup VOIP_SESSION
 * @brief Assign the active session.
 * @param chid The channel number.
 * @param sid The session number. Usually 0 for master session, 1 for slave(conference) session.
 * @retval 0 Success
 */  
int32 rtk_SetTranSessionID(uint32 chid, uint32 sid);

/*
 * @ingroup VOIP_SESSION
 * @brief Get active session by channel number
 * @param chid The specified hannel number.
 * @param psid The session number. Usually 0 for master session, 1 for slave(conference) session.
 * @retval 0 Success
 */  
int32 rtk_GetTranSessionID(uint32 chid, uint32* psid);

/**
 * @ingroup VOIP_SESSION_TONE
 * @brief Play/Stop the assigned Tone 
 * @param chid The channel number.
 * @param sid The session number. Usually 0 for master session, 1 for slave(conference) session.
 * @param nTone The tone type.
 * @param bFlag 1: play, 0: stop
 * @param Path The tone direction.
 * @retval 0 Success
 */  
int32 rtk_SetPlayTone(uint32 chid, uint32 sid, DSPCODEC_TONE nTone, uint32 bFlag, 
	DSPCODEC_TONEDIRECTION Path);

/**
 * @ingroup VOIP_SESSION_TONE
 * @brief Set the Tone of Country
 * @param voip_ptr The configuration of VoIP.
 * @retval 0 Success
 * @note The parameter is depend on the flash layout.
 */
int32 rtk_Set_Country(voipCfgParam_t *voip_ptr);

/**
 * @ingroup VOIP_PHONE
 * @brief Set the SLIC impedance of Country
 * @param country The selected country
 * @retval 0 Success
 */
int32 rtk_Set_Country_Impedance(_COUNTRY_ country);

/**
 * @ingroup VOIP_SESSION_TONE
 * @brief Set the Tone of Country
 * @param country The selected country
 * @retval 0 Success
 */
int32 rtk_Set_Country_Tone(_COUNTRY_ country);

/**
 * @ingroup VOIP_PHONE
 * @brief Set the SLIC impedance from preset table
 * @param preset The preset table index
 * @retval 0 Success
 */
int32 rtk_Set_Impedance(uint16 preset);

/**
 * @ingroup VOIP_SESSION_TONE
 * @brief Set the disconnect Tone det param
 * @param voip_ptr The configuration of VoIP.
 * @retval 0 Success
 * @note The parameter is depend on the flash layout.
 */
int32 rtk_Set_Dis_Tone_Para(voipCfgParam_t *voip_ptr);

/**
 * @ingroup VOIP_SESSION_TONE
 * @brief Set the custom tone
 * @param custom The custom index.
 * @param pstToneCfgParam The custom tone configuration.
 * @retval 0 Success
 */  
int32 rtk_Set_Custom_Tone(uint8 custom, st_ToneCfgParam *pstToneCfgParam);

/**
 * @ingroup VOIP_SESSION_TONE
 * @brief Use the custom tone to be dial, ringback, busy, and call waiting tone
 * @param dial Customer dial tone 
 * @param ringback Customer ringing tone 
 * @param busy Customer busy tone 
 * @param waiting Customer call waiting tone 
 * @retval 0 Success
 */  
int32 rtk_Use_Custom_Tone(uint8 dial, uint8 ringback, uint8 busy, uint8 waiting);

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief Set RTP configuration <br>
 * Invoke rtk_SetRtpConfig() when creating a RTP session or modifying RTP parameters.
 * @param cfg The RTP configuration.
 * @retval 0 Success
 */
int32 rtk_SetRtpConfig(rtp_config_t* cfg);

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief Set RTCP configuration <br>
 * Invoke rtk_SetRtcpConfig() when creating a RTCP session or modifying RTCP parameters.
 * @param cfg The RTCP configuration.
 * @param rtcp_tx_interval The RTCP TX Interval.
 *  (ms)
 * @retval 0 Success
 */
int32 rtk_SetRtcpConfig(rtp_config_t *cfg, unsigned short rtcp_tx_interval);

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief Get RTCP logger 
 * @param chid Channel ID.
 * @param sid Session ID.
 * @param logger RTCP logger data.
 * @retval 0 Success
 */
int32 rtk_GetRtcpLogger( uint32 chid, uint32 sid, TstVoipRtcpLogger *logger );

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief Set RTP payload type <br>
 * @param cfg The RTP payload configuration.
 * @retval 0 Success
 */  
int32 rtk_SetRtpPayloadType(payloadtype_config_t* cfg);

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief Set RTP Session State <br>
 * This API used to drop packets which generated by DSP or received from remote peer. 
 * If state is rtp_session_sendonly, RTP module continuely sends RTP to remote peer 
 * but drops packets which recieved from remote peer.<p> 
 *
 * If state is rtp_session_recvonly,
 * RTP module continuely receives packets from remote peer but drops RTP packets 
 * which generated from DSP. <p>
 *
 * If state is rtp_session_inactive, RTP module drops all packets from remote paeer 
 * and local DSP. This state inactive both RTP and RTCP sessions.<p>
 * 
 * If state is rtp_session_sendrecv, RTP module forwards RTP packets to remote peer 
 * and receives RTP from remote peer.<p>
 *
 * @param chid The channel number.
 * @param sid The session number. Usually 0 for master session, 1 for slave(conference) session.
 * @param state The RTP Session State.
 * @retval 0 Success
 */  
int32 rtk_SetRtpSessionState(uint32 chid, uint32 sid, RtpSessionState state);

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief Enable/Disable conference.<br>
 * This API used to enable or disable a 3-way conference session.
 * @param stVoipMgr3WayCfg The conference settings for two RTP sessions. 
 * DSP has ability to join different codec streams into a conference session.
 * @retval 0 Success
 */  
int32 rtk_SetConference(TstVoipMgr3WayCfg *stVoipMgr3WayCfg);

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief Hold/Resume RTP
 * @param chid The channel number.
 * @param sid The session number. Usually 0 for master session, 1 for slave(conference) session.
 * @param enable 1: Hold, 0: Resume
 * @retval 0 Success
 */  
int32 rtk_Hold_Rtp(uint32 chid, uint32 sid, uint32 enable);

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief Get RTP statistics according to channel id.<br>
 * Provided statistics are:<br>
 * nRxRtpStatsCountByte 	Number of Rx RTP bytes<br>
 * nRxRtpStatsCountPacket 	Number of Rx RTP packets<br>
 * nRxRtpStatsLostPacket 	Number of Rx packet loss<br>
 * nTxRtpStatsCountByte 	Number of Tx RTP bytes<br>
 * nTxRtpStatsCountPacket 	Number of Tx RTP packets
 * @param chid The channel number.
 * @param bReset Reset all statistics to zero, if bReset is set.
 * @param pstVoipRtpStatistics RTP statistics. 
 */
int32 rtk_Get_Rtp_Statistics( uint32 chid, uint32 bReset, TstVoipRtpStatistics *pstVoipRtpStatistics );

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief set RTP packet with TOS. Usually defined for QoS.<br>
 * @param rtp_tos tos of rtp.
 *
 */
int32 rtk_Set_Rtp_Tos(int32 rtp_tos);

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief set RTP packet with Dscp. Usually defined for QoS.<br>
 * @param rtp_dscp dscp of rtp.
 */
int32 rtk_Set_Rtp_Dscp(int32 rtp_dscp);


/**
 * @ingroup VOIP_SESSION_RTP
 * @brief inform kernel the tos value of sip signaling. Usually defined for QoS.<br>
 * @param sip_tos tos of sip.
 */
int32 rtk_Set_Sip_Tos(int32 sip_tos);

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief inform kernel the dscp value of sip signalin. Usually defined for QoS.<br>
 * @param sip_dscp dscp of sip.
 */
int32 rtk_Set_Sip_Dscp(int32 sip_dscp);


/**
 * @ingroup VOIP_SESSION_DTMF
 * @brief Set the DTMF mode
 * @param chid The channel number.
 * @param mode The DTMF mode. (see __DTMF_TYPE__)
 * @retval 0 Success
 * @sa __DTMF_TYPE__ 
 */  
int32 rtk_SetDTMFMODE(uint32 chid, uint32 mode);

/**
 * @ingroup VOIP_SESSION_DTMF
 * @brief Set RFC2833 send by AP or DSP 
 * @param chid The channel number
 * @param config RFC2833 is send by 0: DSP, 1: AP
 * @retval 0 Success
 */  
int32 rtk_SetRFC2833SendByAP(uint32 chid, uint32 config);

/**
 * @ingroup VOIP_SESSION_DTMF
 * @brief Set RFC2833 TX configuration
 * @param chid The channel number
 * @param volume The RFC2833 volume. 0 to 31 (0dBm to -31 dBm)
 * @param tx_mode RFC2833 is send by 0: DSP, 1: AP
 * @retval 0 Success
 */  
int32 rtk_SetRFC2833TxConfig(uint32 chid, uint32 volume, uint32 tx_mode);

/**
 * @ingroup VOIP_SESSION_DTMF
 * @brief Send DTMF via RFC2833 
 * @param chid The channel number.
 * @param sid The session number. Usually 0 for master session, 1 for slave(conference) session.
 * @param digit The digit of user input. (0..11, means 0..9,*,#)
 * @param duration The RFC2833 event duration (ms)
 * @retval 0 Success
 */  
int32 rtk_SetRTPRFC2833(uint32 chid, uint32 sid, uint32 digit, unsigned int duration);

/**
 * @ingroup VOIP_SESSION_DTMF
 * @brief Limit the Max. RFC2833 DTMF Duration
 * @param chid The channel number.
 * @param duration The limited Max. DTMF duration (ms)
 * @param bEnable 0: disable , 1: enable Max. limitation for DTMF duration
 * @retval 0 Success
 */  
int32 rtk_LimitMaxRfc2833DtmfDuration(uint32 chid, uint32 duration_in_ms, uint8 bEnable);

/**
 * @ingroup VOIP_SESSION_DTMF
 * @brief Get The Received RFC2833 Event
 * @param chid The channel number.
 * @param mid  The media channel ID
 * @param pent The received RFC2833 event, -1 means no event
 * @retval 0 Success
 */  
int32 rtk_GetRfc2833RxEvent(uint32 chid, uint32 mid, RFC2833_EVENT *pent);

/**
 * @ingroup VOIP_SESSION_DTMF
 * @brief FAX/Modem RFC2833 configuration
 * @param chid The channel number.
  * @param relay_flag 0: Doesn't send, 1: Send -- the RFC2833 packets when DSP detect TDM Fax/Modem tone
 * @param removal_flag 0: Doesn't removal, 1: Removal -- the inband RTP packet when Fax/Modem RFC2833 packets are sending
 * @param tone_gen_flag 0: Doesn't play, 1: Play -- the Fax/Modem tone when receiving Fax/Modem RFC2833 packets
 */
int32 rtk_SetFaxModemRfc2833(uint32 chid, uint32 relay_flag, uint32 removal_flag, uint32 tone_gen_flag);

/** 
 * @ingroup VOIP_SESSION_DTMF
 * @brief Play tone when receive SIP INFO
 * @param chid The channel number
 * @param sid The session number. Usually 0 for master session, 1 for slave(conference) session.
 * @param tone The tone need to play
 * @param duration The tone duration (ms)
 * @retval 0 Success
 */
int32 rtk_SIP_INFO_play_tone(unsigned int chid, unsigned int sid, DSPCODEC_TONE tone, unsigned int duration);

/**
 * @ingroup VOIP_SESSION_FAX
 * @brief Fax/Modem detection
 * @param chid The channel number.
 * @param pval The fax state.
 * @param flush 1: flush FIFO, 0: do nothing
 * @retval 0 Success
 */  
int32 rtk_GetFaxModemEvent(uint32 chid, uint32 *pval, uint32 flush);

/** 
 * @ingroup VOIP_SESSION_FAX
 * @brief Fax/Modem detection config
 * @param chid The channel number.
 * @param mode The fax/Modem det config: 0:auto. 1:fax. 2:modem.
 * @retval 0 Success
 */  
int32 rtk_SetFaxModemDet(uint32 chid, uint32 mode);

/** 
 * @ingroup VOIP_SESSION_FAX
 * @brief Get Fax end detection result
 * @param chid The channel number.
 * @param *pval 0: No Fax end 1: Fax end detected
 * @retval 0 Success
 */  
int32 rtk_GetFaxEndDetect(uint32 chid, uint32 *pval);

/** 
 * @ingroup VOIP_SESSION_FAX
 * @brief Get Fax DIS detection result
 * @param chid The channel number.
 * @param *pval 0: No Fax DIS 1: Fax DIS detected
 * @retval 0 Success
 */  
int32 rtk_GetFaxDisDetect(uint32 chid, uint32 *pval);

/** 
 * @ingroup VOIP_SESSION_FAX
 * @brief Get Fax Dis TX event detection result
 * @param chid The channel number.
 * @param *pval 0: No Fax DIS TX 1: Fax DIS TX detected
 * @retval 0 Success
 */  
int32 rtk_GetFaxDisTxDetect(uint32 chid, uint32 *pval);

/** 
 * @ingroup VOIP_SESSION_FAX
 * @brief Get Fax Dis RX event detection result
 * @param chid The channel number.
 * @param *pval 0: No Fax DIS RX 1: Fax DIS RX detected
 * @retval 0 Success
 */  
int32 rtk_GetFaxDisRxDetect(uint32 chid, uint32 *pval);

/** 
 * @ingroup VOIP_FXO_REAL_DAA
 * @brief Get FXO Line Voltage
 * @param chid The channel number.
 * @param *pval FXO Line Voltage
 * @retval 0 Success
 */ 
int32 rtk_GetFxoLineVoltage(uint32 chid, uint32 *pval);

/** 
 * @ingroup VOIP_IVR
 * @brief Play a text speech
 * @param chid The channel number.
 * @param dir The playout directions.
 * @param pszText2Speech The text to speech.
 * @retval Playing interval in unit of 10ms.
 */
int rtk_IvrStartPlaying( unsigned int chid, IvrPlayDir_t dir, unsigned char *pszText2Speech );

/** 
 * @ingroup VOIP_IVR
 * @brief Play a G.723 6.3k voice 
 * @param chid The channel number.
 * @param nFrameCount The number of frame to be decoded. 
 * @param pData Point to data. 
 * @retval Copied frames, so less or equal to nFrameCount. 
 */
int rtk_IvrStartPlayG72363( unsigned int chid, unsigned int nFrameCount, const unsigned char *pData );

/** 
 * @ingroup VOIP_IVR
 * @brief Poll whether it is playing or not
 * @param chid The channel number.
 * @retval 1 Playing
 * @retval 0 Stopped 
 */
int rtk_IvrPollPlaying( unsigned int chid );

/** 
 * @ingroup VOIP_IVR
 * @brief Stop playing immediately
 * @param chid The channel number.
 * @retval 0 Success
 */
int rtk_IvrStopPlaying( unsigned int chid );

/** 
 * @ingroup VOIP_PHONE
 * @brief OffHook Action function. 
 * @param chid The channel number.
 * @retval 0 Success
 * @Note Call it always after Off-Hook action start
 * @sa rtk_GetFxsEvent, rtk_GetFxoEvent
 */
int32 rtk_Offhook_Action(uint32 chid);

/** 
 * @ingroup VOIP_PHONE
 * @brief OnHook Action function. 
 * @param chid The channel number.
 * @retval 0 Success
 * @Note Call it always before On-Hook action done
 * @sa rtk_GetFxsEvent, rtk_GetFxoEvent
 */
int32 rtk_Onhook_Action(uint32 chid);

/**
 * @ingroup VOIP_NET
 * @brief Set QOS priority for DSCP
 * @param dscp for set priority
 * @param priority (0-7, 7 is the highest priority)
 * @Note packet with high priority will be first processed by HW
 * @retval 0 Success
 */
int32 rtk_qos_set_dscp_priority(int32 dscp, int32 priority);

/**
 * @ingroup VOIP_NET
 * @brief reset QOS priority for DSCP to 0
 * @retval 0 Success
 */
int32 rtk_qos_set_dscp_priority(int32 sip, int32 rtp);

#ifdef CONFIG_RTK_VOIP_WAN_VLAN
/**
 * @ingroup VOIP_VLAN
 * @brief setup 3 VLANs for Voice, Data, and Video traffic. Note: Traffic from/to Video Port (Port 3) are tagged with Video VLAN TAG, Traffic from/to LAN/WLAN are tagged with Data VLAN TAG, and other traffic are tagged with Voice VLAN TAG.
 * @param voip_ptr voip configuration
 * @retval 0 Success
 */
int rtk_switch_wan_3_vlan(voipCfgParam_t *voip_ptr);

/**
 * @internal use
 * @brief setup 2 VLANs for Voice and Data traffic. Note: Traffic from/to LAN/WLAN are tagged with Data VLAN TAG, and other traffic are tagged with Voice VLAN TAG.
 * @param voip_ptr voip configuration
 * @retval 0 Success
 */
int rtk_switch_wan_2_vlan(voipCfgParam_t *voip_ptr);

/*
 * internal use
 * @brief rtk_switch_wan_vlan
 * @param voip_ptr voip configuration
 * @retval 0 Success
 */
int rtk_switch_wan_vlan(voipCfgParam_t *voip_ptr);
#endif // CONFIG_RTK_VOIP_WAN_VLAN

/*
 * internal use
 * @brief rtk_8305_switch
 * @param phy phy
 * @param reg reg
 * @param value value
 * @param r_w r_w
 * @retval 0 Success
 */
int rtk_8305_switch(unsigned short phy,unsigned short reg,unsigned short value,unsigned short r_w);


/* add by Tim 1/8/2007
 * @brief WAN Clone MAC 
 * @param MAC address ptr
 * @retval 0 Success
 */
int rtk_WAN_Clone_MAC(unsigned char* MAC);

/**
 * @ingroup VOIP_NET
 * @brief Bandwitdh Mgr
 * @param port port
 * @param dir direction (0: ingress 1:egress)
 * @param ban bandwidth (0: unlimit, egress range: 0~16383, egress unit: 64kbps, ingress range: 0~65535, ingress unit: 16kbps)
 * @retval 0 Success
 */
int rtk_Bandwidth_Mgr(	unsigned int port, unsigned int dir, unsigned int ban);

/**
 * @ingroup VOIP_NET
 * @brief Disable Port
 * @param port port
 * @param disable (0: enable 1: disable)
 * @retval 0 Success
 */
int rtk_Disable_Port(unsigned int port, unsigned int disable);

/**
 * @ingroup VOIP_NET
 * @brief set port priority
 * @param port port
 * @param priority (0-7, 7 is the highest priority)
 * @retval 0 Success
 */
int rtk_qos_set_port_priority(  unsigned int port, unsigned int priority);

/**
 * @ingroup VOIP_NET
 * @brief reset all port priority to 0
 * @retval 0 Success
 */
int rtk_qos_reset_port_priority( void );


/**
 * @ingroup VOIP_NET
 * @brief Disable FlowControl
 * @param port port
 * @param disable disable_fc (0: enable 1:disable)
 * @retval 0 Success
 */
int rtk_Disable_FlowControl( unsigned int port,unsigned int disable);

/**
 * @ingroup VOIP_NET
 * @brief Get Port link Status
 * @param pstatus status
 * @retval 0 Success
 */
int32 rtk_GetNetlinkStatus( uint32 *pstatus );

/**
 * @ingroup VOIP_
 * @brief Enable PCM channel
 * @param chid Channel ID 
 * @param val 0: disable, 1: enable PCM as narrow-band, 2: enable PCM as wide-band
 * @retval 0 Success
 */
int32 rtk_eanblePCM(uint32 chid, uint32 val);

 /**
 * @ingroup VOIP_DSP_DTMF
 * @brief Set DTMF Configuration
 * @param TstVoipCfg.ch_id Channel ID 
 * @param TstVoipCfg.enable 0: disable, 1: enable
 * @param TstVoipCfg.cfg DTMF detection direction, 0: TDM, 1: IP
 * @retval 0 Success
 */
int32 rtk_Set_DTMF_CFG(uint32 chid, int32 bEnable, uint32 dir);

 /**
 * @ingroup VOIP_DSP_DTMF
 * @brief Set DTMF detection threshold
 * @param TstVoipCfg.ch_id Channel ID 
 * @param TstVoipCfg.cfg DTMF detection threshold, 0 to 40 (range from 0 t0 -40 dBm)
 * @param TstVoipCfg.cfg2 DTMF detection direction, 0: TDM, 1: IP
 * @retval 0 Success
 */
int32 rtk_set_dtmf_det_threshold(uint32 chid, int32 threshold, uint32 dir);

/**
 * @ingroup VOIP_IP_PHONE
 * @brief Get hook status of IP Phone
 * @param pHookStatus Hook status. 1: on-hook, 0: off-hook
 */
extern int rtk_GetIPPhoneHookStatus( uint32 *pHookStatus );

/**
* @ingroup VOIP_RESOURCE_CHECK
* @brief Get the VoIP Middleware Resource status
* @param chid The target channel open. (Help to find DSP chip)
* @param payload_type The RTP payoad type (0, 4, 8, 18...) for check resource
* @retval 1: resource available, 0: resource un-available
*/
unsigned short rtk_VoIP_resource_check(uint32 chid, int payload_type);

/**
 * @ingroup VOIP_PHONE
 * @brief Get the phone state only (don't read fifo)
 * @param pstVoipCfg The config
 * @retval 0 Success
 * @sa rtk_GetFxsEvent
 */  
int32 rtk_Set_GetPhoneStat(TstVoipCfg* pstVoipCfg);

/**
 * @ingroup VOIP_PHONE
 * @brief Flush the VoIP kernel used fifo.
 * @param chid The channel number.
 * @retval 0 Success
 */
int rtk_Set_flush_fifo(uint32 chid);
 
/**
 * @ingroup VOIP_PHONE
 * @brief check line status
 * @param chid The channel number.
 * @retval For FXS: 0: Phone dis-connect, 1: Phone connect, 2: Phone off-hook, 3: Check time out ( may connect too many phone set => view as connect), 4: Can not check, Linefeed should be set to active state first.
 * @retval For FXO: 0: PSTN Line connect, 1: PSTN Line not connect, 2: PSTN Line busy
 */
int rtk_line_check(uint32 chid);

/**
 * @ingroup VOIP_FXO_REAL_DAA
 * @brief Let DAA off-hook
 * @param chid The FXO channel number.
 * @retval 1 Success
 * @note This API is only used for real DAA
 */
int rtk_FXO_offhook(uint32 chid);

/**
 * @ingroup VOIP_FXO
 * @brief FXO on-hook
 * @param chid The FXO channel number.
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int rtk_FXO_onhook(uint32 chid);

/**
 * @ingroup VOIP_FXO
 * @brief FXO Ring
 * @param chid The FXO channel number.
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int rtk_FXO_RingOn(uint32 chid);

/**
 * @ingroup VOIP_FXO
 * @brief FXO Busy
 * @param chid The FXO channel number.
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int rtk_FXO_Busy(uint32 chid);

/**
 * @ingroup VOIP_GPIO
 * @brief GPIO control
 * @param action 0: gpio init, 1: read, 2: write
 * @param pid pid = (gpio_group << 16) | (gpio_pin & 0xffff)
 * @param value 
 *     GPIO read/write:
 *			- bit 0: disable = 0, enable = 1
 *     GPIO init:
 *			- bit0~1: GPIO_PERIPHERAL
 *			- bit2: GPIO_DIRECTION
 *			- bit3: GPIO_INTERRUPT_TYPE
 * @param ret_value read result
 * @retval 0 Success
 */
int rtk_gpio(unsigned long action, unsigned long pid, unsigned long value, unsigned long *ret_value);

/**
 * @ingroup VOIP_GPIO
 * @brief Set LED Display mode 
 * @param chid Channel number 
 * @param LED_ID LED ID. Each channel has at most 2 LED, and we use 0 normally.
 * @param mode Display mode. @ref LedDisplayMode
 * @retval 0 Success
 */
int rtk_Set_LED_Display( uint32 chid, uint32 LED_ID, LedDisplayMode mode );

/**
 * @ingroup VOIP_GPIO
 * @brief Set SLIC Relay mode 
 * @param chid Channel number 
 * @param close A boolean value to indicate circuit between SLIC and phone is close(1) or open(0)
 * @retval 0 Success
 * @note Because kernel will switch relay to correct state,
 *       application doesn't need to control relay normally.  
 */
int rtk_Set_SLIC_Relay( uint32 chid, uint32 close );

/**
 * @ingroup VOIP_DSP
 * @brief The variables contaions the VoIP Feature
 * @note call rtk_Get_VoIP_Feature first
 */
extern uint64 g_VoIP_Feature;

/**
 * @ingroup VOIP_FXO
 * @brief Set FXO dial mode
 * @param chid The FXO channel number.
 * @param mode  0:disable, 1:enable Pulse Dial for FXO
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int rtk_Set_Dail_Mode(uint32 chid, uint32 mode);

/**
 * @ingroup VOIP_FXO
 * @brief Get FXO dial mode: 0-disable, 1-enable Pulse Dial for FXO
 * @param chid The FXO channel number.
 * @retval 0: disable 1: enable Pulse dial
 * @note Virtual DAA not support
 */
int rtk_Get_Dail_Mode(uint32 chid);

 /**
 * @ingroup VOIP_FXO
 * @brief Generate pulse dial for FXO
 * @param chid The FXO channel number.
 * @param digit The pulse dial digit(0~9)
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int rtk_Gen_Pulse_Dial(uint32 chid, char digit);

 /**
 * @ingroup VOIP_FXO
 * @brief Pulse dial generation config for FXO
 * @param pps The pulse dial gen speed(Pulse per second).
 * @param make_duration	Make duration of the pulse digit, unit:10 msec
 * @param interdigit_duration The pause time between the pulse digit, unit:10 msec
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int rtk_PulseDial_Gen_Cfg(char pps, short make_duration, short interdigit_duration);

/**
 * @ingroup VOIP_FXS
 * @brief Set FXS pulse detection
 * @param chid The FXS channel number.
 * @param enable  0:disable, 1:enable Pulse detection for FXS
 * @param pause_time The threshold of the pause duration of the adjacent pulse digit(msec)
 * @param min_break_ths The threshold of min break time(msec)
 * @param max_break_ths The threshold of max break time(msec)
 * ex. If the pause duration of two pulse trains(digit 3 and digit 4) is smaller than PULSE_DIAL_PAUSE_TIME,
 * then the detection result will be digit 7 (3+4).
 * @retval 0 Success
 */
int rtk_Set_Pulse_Digit_Det(uint32 chid, uint32 enable, uint32 pause_time, uint32 min_break_ths, uint32 max_break_ths);

/**
 * @ingroup VOIP_FXS
 * @brief Get SLIC 3226 ram value.
 * @param chid The SLIC channel number.
 * @param reg  Ram number.
 * @retval The value of the ram.
 */
uint32 rtk_Get_SLIC_Ram_Val(uint8 chid, uint16 reg);

/**
 * @ingroup VOIP_FXS
 * @brief Set SLIC 3226 ram value.
 * @param chid The SLIC channel number.
 * @param reg  Ram number.
 * @param value The value of the ram you want to set.
 * @retval 0 Success
 */
int rtk_Set_SLIC_Ram_Val(uint8 chid, uint16 reg, uint32 value);

/**
 * @ingroup VOIP_NET
 * @brief Get port link status
 * @param pstatus Port link status holder.
 * @retval 0 Success
 */
int32 rtk_GetPortLinkStatus( uint32 *pstatus );

/**
 * @ingroup VOIP_PCM
 * @brief Set PCM Loop Mode
 * @param mode 0- Not loop mode, 1- loop mode, 2- loop mode with VoIP
 * @param main_ch If the mode is 2, main channel will create VoIP session.
 * @param mate_ch If the mode is 2, mate channel will NOT create VoIP session.
 * @retval 0 Success
 */
int rtk_Set_PCM_Loop_Mode(char group, char mode, char main_ch, char mate_ch);

/**
 * @ingroup VOIP_FXS
 * @brief Set FXS FXO Loopback Mode
 * @param chid SLIC channel ID
 * @param enable  Enable(1) or disable(0) FXS FXO loopback
 * @retval 0 Success
 */
int rtk_Set_FXS_FXO_Loopback(unsigned int chid, unsigned int enable);

/**
 * @ingroup VOIP_FXS
 * @brief Set FXS On Hook Transmission and PCM On
 * @param chid SLIC channel ID
 * @retval 0 Success
 */
int rtk_Set_FXS_OnHook_Trans_PCM_ON(unsigned int chid);

/**
 * @ingroup VOIP_DEBUG
 * @brief print message via kernel
 * @param level debug level (0: error, 1: warning, 2: info, 3: trace)
 * @param module module name
 * @param msg debug message
 * @retval 0 Success
 */
int rtk_print(int level, char *module, char *msg);
int rtk_cp3_measure(st_CP3_VoIP_param* cp3_voip_param);

#ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT
/*
 * @ingroup VOIP_DECT
 * @brief Set DECT power 
 * @param power DECT power. 0: active, 1: inactive.
 * @retval 0 Success
 */
int32 rtk_SetDectPower( uint32 power );

/*
 * @ingroup VOIP_DECT
 * @brief Get DECT power 
 * @retval DECT power. 0: active, 1: inactive.
 * @note It need delay between power off and on.
 * @note IMPORTANT!! do some delay after power on. 
 */
int32 rtk_GetDectPower( void );

/*
 * @ingroup VOIP_DECT
 * @brief Get DECT page button 
 * @retval DECT page. 0: active, 1: inactive.
 */
int32 rtk_GetDectPage( void );

/*
 * @ingroup VOIP_DECT
 * @brief Get DECT button event
 * @retval DECT button event. -1: no event, 0x1: page, 0x2: registration mode, 0x3: delete registered HS, 0xff: not defined
 */
int32 rtk_GetDectButtonEvent( void );

/*
 * @ingroup VOIP_DECT
 * @brief Get HS ID which occupy line
 * @retval HS ID
 */
int32 rtk_GetLineOccupyHS(uint32 line_id);

/*
 * @ingroup VOIP_DECT
 * @brief Put DECT event into event FIFO
 * @retval 0 Success
 */
int dect_event_in(uint32 line_id, uint32 hs_id, char input);

/*
 * @ingroup VOIP_DECT
 * @brief Set DECT LED
 * @param state 0: LED off, 1: LED on, 2: LED blinking
 * @retval 0 Success
 */
int32 rtk_SetDectLED( int chid, char state );
#endif // CONFIG_RTK_VOIP_DRIVERS_ATA_DECT

#endif // __VOIP_MANAGER_H

