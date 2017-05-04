/**
 * @file voip_manager.h
 * @brief VoIP control API
 */

#ifndef __VOIP_MANAGER_H
#define __VOIP_MANAGER_H

#include <netinet/in.h>

#include "voip_types.h"
#include "voip_flash.h"
#include "voip_params.h"
#include "voip_control.h"
#include "voip_feature.h"

#define MAX_SELF_TONE			8
#define MAX_CUSTOME			4

#define PHONE_ON_HOOK			0
#define PHONE_OFF_HOOK			1
#define PHONE_FLASH_HOOK		2
#define PHONE_STILL_ON_HOOK		3
#define PHONE_STILL_OFF_HOOK		4
#define PHONE_UNKNOWN			5

#define DAA_FLOW_NORMAL			0
#define DAA_FLOW_3WAY_CONFERENCE	1
#define DAA_FLOW_CALL_FORWARD		2

#define DAA_CH				2

#ifdef CONFIG_RTK_VOIP_SRTP
#define SRTP_KEY_LEN 			30
#endif

#define RTK_DISABLE 0
#define RTK_ENABLE 	1

//=================================================
#if 1
#define rtk_line_check rtk_LineCheck         		
#define rtk_Get_PhoneState rtk_GetPhoneState
#define rtk_enablePCM rtk_EnablePcm  		
#define rtk_Set_FXS_OnHook_Trans_PCM_ON rtk_SetFxsOnHookTransPcmOn  		
#define rtk_SetRingFXS rtk_SetRingFxs  			
#define rtk_DisableRingFXS rtk_DisableRingFxs  		
#define rtk_Set_Flash_Hook_Time rtk_SetFlashHookTime  		
#define rtk_Set_Country_Impedance rtk_SetCountryImpedance  		
#define rtk_Set_Impedance rtk_SetImpedance  		
#define rtk_Gen_SLIC_CPC rtk_GenSlicCpc  		
#define rtk_FXO_offhook rtk_FxoOffHook  		 					 
#define rtk_DAA_off_hook rtk_DaaOffHook   		 
#define rtk_DAA_on_hook rtk_DaaOnHook   		
#define rtk_Get_DAA_CallerID rtk_GetDaaCallerID  		 
#define rtk_FXO_onhook rtk_FxoOnHook  		
#define rtk_FXO_RingOn rtk_FxoRingOn  		
#define rtk_FXO_Busy rtk_FxoBusy  		 
#define rtk_Set_FXO_Ring_Detection rtk_SetFxoRingDetection
#define rtk_Set_FXO_Tune rtk_SetFxoTune
#define rtk_Set_DAA_Hybrid rtk_SetDaaHybrid
#define rtk_Dial_PSTN_Call_Forward rtk_DialPstnCallForward  		
#define rtk_Set_flush_fifo rtk_SetFlushFifo  		
#define rtk_print rtk_Print  		
#define rtk_Get_SLIC_Ram_Val rtk_GetSlicRamVal  		
#define rtk_Set_SLIC_Ram_Val rtk_SetSlicRamVal  		 
#define rtk_Get_SLIC_Reg_Val rtk_GetSlicRegVal  		
#define rtk_Set_SLIC_Reg_Val rtk_SetSlicRegVal  		
#define rtk_Set_FXS_FXO_Loopback rtk_SetFxsFxoLoopback
#define rtk_Offhook_Action rtk_OffHookAction
#define rtk_Onhook_Action rtk_OnHookAction

#define rtk_Set_G168_LEC rtk_SetG168Lec  		 
#define rtk_Set_VBD_EC rtk_SetVbdEc  		
#define rtk_Set_echoTail rtk_SetEchoTail  		
#define rtk_Set_Silence_Det_Threshold rtk_SetSilenceDetThreshold  		
#define rtk_Set_Voice_Gain rtk_SetVoiceGain  		 
#define rtk_Set_SPK_AGC rtk_SetSpkAgc  		
#define rtk_Set_SPK_AGC_LVL rtk_SetSpkAgcLvl  		
#define rtk_Set_SPK_AGC_GUP rtk_SetSpkAgcGup  		 
#define rtk_Set_SPK_AGC_GDOWN rtk_SetSpkAgcGdown  		
#define rtk_Set_MIC_AGC rtk_SetMicAgc  		
#define rtk_Set_MIC_AGC_LVL rtk_SetMicAgcLvl  		
#define rtk_Set_MIC_AGC_GUP rtk_SetMicAgcGup  		
#define rtk_Set_MIC_AGC_GDOWN rtk_SetMicAgcGdown   		
#define rtk_Set_Multi_Ring_Cadence rtk_SetMultiRingCadence

#define rtk_Set_Country rtk_SetCountry  		
#define rtk_Set_Country_Tone rtk_SetCountryTone  		
#define rtk_Set_Dis_Tone_Para rtk_SetDisTonePara  		
#define rtk_Set_Custom_Tone rtk_SetCustomTone  		
#define rtk_Use_Custom_Tone rtk_UseCustomTone  		
#define rtk_Set_Update_Tone rtk_SetUpdateTone  		
#define rtk_SIP_INFO_play_tone rtk_SipInfoPlayTone  		
#define rtk_Set_AnswerTone_Det rtk_SetAnswerToneDet  		 

#define rtk_Stop_CID rtk_StopCid  		 
#define rtk_Set_CID_Det_Mode rtk_SetCidDetMode  		
#define rtk_Gen_CID_And_FXS_Ring rtk_GenCidAndFxsRing  		 
#define rtk_Gen_FSK_CID rtk_GenFskCid  		 
#define rtk_Gen_MDMF_FSK_CID rtk_GenMdmfFskCid  		
#define rtk_GetFskCIDState rtk_GetFskCidState 
#define rtk_Gen_FSK_VMWI rtk_GenFskVmwi   
#define rtk_Set_Dtmf_CID_String rtk_SetDtmfCidString  		 
#define rtk_Gen_Dtmf_CID rtk_GenDtmfCid  		 
#define rtk_Set_CID_DTMF_MODE rtk_SetDtmfCidParam  		
#define rtk_Set_DTMF_CFG rtk_SetDtmfCfg  		 
#define rtk_set_dtmf_det_param rtk_SetDtmfDetParam  		
#define rtk_SetDTMFMODE rtk_SetDtmfMode  		
#define rtk_Set_Pulse_Digit_Det rtk_SetPulseDigitDet  		
#define rtk_Gen_Pulse_Dial rtk_GenPulseDial  		 
#define rtk_PulseDial_Gen_Cfg rtk_PulseDialGenCfg  		
#define rtk_Set_Dail_Mode rtk_SetDailMode  		
#define rtk_Get_Dail_Mode rtk_GetDailMode  		
#define rtk_Get_VoIP_Feature rtk_GetVoIPFeature  		 
#define rtk_InitDSP rtk_InitDsp  		
#define rtk_Onhook_Reinit rtk_OnHookReinit  		

#define rtk_Hold_Rtp rtk_HoldRtp  		
#define rtk_qos_set_dscp_priority rtk_QosSetDscpPriority  		 
//#define rtk_BandwidthMgr rtk_Bandwidth_Mgr 		 
//#define rtk_QosSetPortPriority rtk_qos_set_port_priority 		 
//#define rtk_QosResetPortPriority rtk_qos_reset_port_priority 		
//#define rtk_DisableFlowControl rtk_Disable_FlowControl 		
#define rtk_Set_Rtp_Tos rtk_SetRtpTos  		
#define rtk_Set_Rtp_Dscp rtk_SetRtpDscp  		
#define rtk_Set_Sip_Tos rtk_SetSipTos  		
#define rtk_Set_Sip_Dscp rtk_SetSipDscp  		
//#define rtk_DisablePort rtk_Disable_Port 	
	
#define rtk_sip_register rtk_SipRegister   
#define rtk_Set_SLIC_Ring_Cadence rtk_SetSlicRingCadence  
#define rtk_Set_SLIC_Ring_Freq_Amp rtk_SetSlicRingFreqAmp  
#define rtk_Set_SLIC_Line_Voltage rtk_SetSlicLineVoltage  
#define rtk_Set_CID_FSK_GEN_MODE rtk_SetCidFskGenMode   
#define rtk_Gen_FSK_CID_VMWI rtk_GenFskCidVmwi  
#define rtk_DAA_ring rtk_DaaRing   
#define rtk_Set_DAA_Tx_Gain rtk_SetDaaTxGain  
#define rtk_Set_DAA_Rx_Gain rtk_SetDaaRxGain  
#define rtk_Set_DAA_ISR_FLOW rtk_SetDaaIsrFlow  
#define rtk_Get_DAA_ISR_FLOW rtk_GetDaaIsrFlow   
#define rtk_Set_PSTN_HOLD_CFG rtk_SetPstnHoldCfg  
#define rtk_Get_DAA_BusyTone_Status rtk_GetDaaBusyToneStatus  
#define rtk_Get_DAA_Used_By_Which_SLIC rtk_GetDaaUsedByWhichSlic  

#define rtk_Set_FSK_Area rtk_SetFskArea
#define rtk_SetRTPRFC2833 rtk_SetRtpRfc2833
#define rtk_Set_FSK_CLID_Para rtk_SetFskClidPara
#define rtk_Get_FSK_CLID_Para rtk_GetFskClidPara
#define rtk_GetIPPhoneHookStatus rtk_GetIpPhoneHookStatus
#define rtk_VoIP_resource_check rtk_VoipResourceCheck
#define rtk_gpio rtk_GpioControl
#define rtk_Set_LED_Display rtk_SetLedDisplay
#define rtk_Get_LED_Status rtk_GetLEDStatus
#define rtk_Set_SLIC_Relay rtk_SetSlicRelay
#define rtk_Set_PCM_Loop_Mode rtk_SetPcmLoopMode
#define rtk_ENABLEABCDSignal rtk_EnableABCDSignal
#define rtk_GENERATEABCDSignal  rtk_GenerateABCDSignal 
#endif
//=================================================

/**
 * @ingroup VOIP_PHONE
 * Enumeration for SLIC state
 */
typedef enum {
	SLICEVENT_NONE		= 0,
	SLICEVENT_ONHOOK	= 30,
	SLICEVENT_OFFHOOK	= 31,
	SLICEVENT_OFFHOOK_2	= 32,
	SLICEVENT_FLASHHOOK	= 33,
	SLICEVENT_RING_ON	= 34,
	SLICEVENT_RING_OFF	= 35,
} SLICEVENT;

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
 * Enumeration for DAA event
 */
typedef enum
{
	DAAEVENT_NONE		= 0,
	DAAEVENT_RING_ON	= 40,
	DAAEVENT_OFFHOOK_2	= 41,
	DAAEVENT_RING_STOP	= 42,
	DAAEVENT_BUSY_TONE	= 43,
	DAAEVENT_DIST_TONE	= 44,
} DAAEVENT;

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
 * @ingroup TAPI_DSP_FAX
 * Enumeration for fax state
 */
typedef enum
{
    FMS_IDLE = 0,		///< 0 - IDLE
    FMS_FAX_LOW_SPEED,		///< 1 - FAX_CED
    FMS_FAX_HIGH_SPEED,		///< 2 - FAX_DIS_RX
    FMS_MODEM_LOCAL,		///< 3 - MODEM_LOCAL
    FMS_MODEM_REMOTE,		///< 4 - MODEM_REMOTE
    FMS_V21_FLAG,		///< 5 - EVENT_V21FLAG_LOCAL
    FMS_FAX_RUN,                ///<
    FMS_MODEM_RUN,              ///<
    FMS_BEFAX_RUN,              ///<
    FMS_BEMODEM_RUN             ///<
} FAX_MODEM_STATE;

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
    COUNTRY_EX1,
    COUNTRY_EX2,
    COUNTRY_EX3,
    COUNTRY_EX4,
    COUNTRY_CUSTOMER,
    COUNTRY_MAX
}_COUNTRY_;

/**
 * @ingroup TAPI_MEDIA_RTP
 * @brief Structure for RTP handling <br>
 *
 * In RTP, payload type 13 is used for CN. The G.711 main codec will use a PT of "0" for PCMU and "8" for PCMA. <br>
 * RTP packets switch Payload type (PT) between 13 and 0 or 8 for sending VAD and speech packets. In some RFC drafts, a PT of 19 is also considered, and subsequently, a PT of 19 was updated with 13.<p>
 */
typedef struct
{
    uint32 chid;        		///< channel id 
    uint32 sid;  		///< session id. Usually 0 for master session, 1 for slave(conference) session.
    uint32 isTcp;       		///< Not support yet. tcp = 1 or udp = 0
    uint32 remIp;       		///< remote IP address for RTP
    uint16 remPort;		///< remote RTP port number
    uint32 extIp;       		///< local IP address for RTP
    uint16 extPort;		///< local RTP port number
#ifdef SUPPORT_VOICE_QOS
    uint8 tos;			///< TOS field in IP header. Usually defined for QoS.
#endif
   /** @brief local rfc2833 payload type<br>
     * rfc2833_payload_type_locale MUST be set from application to DSP. 
     * If local side doesn't support RFC2833, application should set it to ZERO, otherwise, set it to proper payload type. 
     */
	uint16 rfc2833_payload_type_local;		///< local DTMF rfc2833 payload type
   /** @brief remote rfc2833 payload type<br>
     * rfc2833_payload_type_remote MUST be set from application to DSP.
     * If remote side doesn't support RFC2833, application should set it to ZERO.
     * Application negotiates RFC2833 payload type with remote peer UA.
     *
     */
	uint16 rfc2833_payload_type_remote;		///< remote DTMF rfc2833 payload type
	uint16 rfc2833_fax_modem_pt_local;		///< local Fax/Modem rfc2833 payload type
	uint16 rfc2833_fax_modem_pt_remote;		///< remote Fax/Modem rfc2833 payload type
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
	uint16 rtp_redundant_payload_type_local;		///< Payload type of local redundant rtp. 0 means disable rtp redundant.
	uint16 rtp_redundant_payload_type_remote;	///< Payload type of remote redundant rtp 0 means disable rtp redundant.
	uint8  rtp_redundant_max_Audio;			///< Maximum RTP redundancy number in TX direction (0~2). 0 means no RTP redundancy 
	uint8  rtp_redundant_max_RFC2833;		///< Maximum RFC 2833 redundancy number in TX direction (0~5). 0 means no RFC 2833 redundancy 
#endif
	uint16 SID_payload_type_local;			///< Payload type of local SID (Silence Insertion Description). 13 is recommend.
	uint16 SID_payload_type_remote;			///< Payload type of remote SID (Silence Insertion Description). 13 is recommend.
	uint16 init_randomly;				///< initialize seqno, SSRC and timestamp randomly, so ignore below parameters 
	uint16 init_seqno;				///< initial seqno 
	uint32 init_SSRC;				///< initial SSRC 
	uint32 init_timestamp;				///< initial timestamp 
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
	struct in6_addr remIp6;       		///< remote IPv6 address for RTP
	struct in6_addr extIp6;       		///< local IPv6 address for RTP
	uint32 ipv6;						///< IPv6 flag, 1: IPv6, 0: IPv4
#endif
} rtp_config_t;

//CT_ASSERT(sizeof(rtp_config_t) == sizeof(TstVoipMgrSession));

/**
 * @ingroup TAPI_MEDIA_RTP
 * @brief Structure for RTCP connection  
 */
typedef struct 
{
	uint32 chid;        		///< channel id 
	uint32 sid;  			///< session id. Usually 0 for master session, 1 for slave(conference) session.
	uint32 remIp;       		///< remote IP address for RTP
	uint16 remPort;			///< remote RTP port number
	uint32 extIp;       		///< local IP address for RTP
	uint16 extPort;			///< local RTP port number
#ifdef SUPPORT_VOICE_QOS
	uint8 tos;			///< TOS field in IP header. Usually defined for QoS.
#endif

	uint32 txInterval;		///< RTCP TX interval (ms)
	uint32 enableXR;			///< enable RTCP XR support (RTCP only)

#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
	struct in6_addr remIp6;       		///< remote IPv6 address for RTP
	struct in6_addr extIp6;       		///< local IPv6 address for RTP
	uint32 ipv6;						///< IPv6 flag, 1: IPv6, 0: IPv4
#endif
} rtcp_config_t;

/**
 * @ingroup TAPI_MEDIA_T38
 * @brief Structure for T.38 UDP connection 
 */
typedef struct 
{
	uint32 chid;        		///< channel id 
	uint32 sid;  			///< session id. Usually 0 for master session, 1 for slave(conference) session.
	uint32 isTcp;       		///< Not support yet. tcp = 1 or udp = 0
	uint32 remIp;       		///< remote IP address for RTP
	uint16 remPort;			///< remote RTP port number
	uint32 extIp;       		///< local IP address for RTP
	uint16 extPort;			///< local RTP port number
#ifdef SUPPORT_VOICE_QOS
	uint8 tos;			///< TOS field in IP header. Usually defined for QoS.
#endif
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
	struct in6_addr remIp6;		///< remote IPv6 address for RTP
	struct in6_addr extIp6;		///< local IPv6 address for RTP
	uint32 ipv6;				///< IPv6 flag, 1: IPv6, 0: IPv4
#endif
} t38udp_config_t;

/**
 * @ingroup TAPI_MEDIA_RTP
 * @brief Structure for RTP payload type <br>
 *
 */
typedef struct
{
    uint32 chid;				///< channel id
    uint32 sid;				///< session id. Usually 0 for master session, 1 for slave(conference) session.

	//RtpPayloadType uPktFormat;		///< Codec of desired voice codec (Legacy optional)
	RtpPayloadType uLocalPktFormat;	///< Codec of desired local(Tx) voice codec
    RtpPayloadType uRemotePktFormat;///< Codec of desired remote(Rx) voice codec
	RtpPayloadType local_pt;		///< Payload type of local voice codec
	RtpPayloadType remote_pt;	///< Payload type of remote voice codec

    RtpPayloadType uPktFormat_vbd;	///< Codec type of voice band data (vbd). Usually used by V.152 FAX.
	RtpPayloadType local_pt_vbd;	///< Payload type of local voice band data (vbd).
	RtpPayloadType remote_pt_vbd;	///< Payload type in remote voice band data (vbd).

   /** @brief G.723.1 rate. There are two bit rates at which G.723.1 can operate.<br>
 	 * Valid values are:<br>
 	 * 0: 6.3 kbit/s (using 24 byte frames) using a MPC-MLQ algorithm (MOS 3.9)<br>
 	 * 1: 5.3 kbit/s (using 20 byte frames) using an ACELP algorithm (MOS 3.62)
 	 */
    int32 nG723Type;			///< see __G7231_RATE__
    //int32 nFramePerPacket;			///< Frame per packet of voice codec (Legacy optional)
    int32 nLocalFramePerPacket;			///< Frame per packet of local (Tx) voice codec 
    int32 nRemoteFramePerPacket;			///< Frame per packet of remote (Rx) voice codec 
    int32 nFramePerPacket_vbd;		///< Frame per packet of voice codec (vbd)

   /** @brief Enable/Disable VAD (Voice activity detection)<br>
 	 * Valid values are:
 	 * 0: Disable
 	 * 1: Enable
 	 */
    int32 bVAD;                     	///< Vad on: 1, off: 0

   /** @brief Enable/Disable PLC (Packet Loss Concealment)<br>
 	 * Valid values are:
 	 * 0: Disable
 	 * 1: Enable
 	 */
	int32 bPLC;

   /** @brief minimun jitter delay (10ms)<br>
     * nJitterDelay is the minimum delay of jitter buffer.<br>
 	 * Valid values are 4 (40ms) to 10 (100ms). Recommended value is 4 (40ms).
 	 */
    uint32 nJitterDelay;				///< min delay
   /** @brief maximum jitter delay (10ms)<br>
     * nJitterDelay is the maximum delay of jitter buffer.<br>
 	 * Valid values are 13 (130ms) to 30 (300ms). Recommended value is 20 (200ms).
 	 */
    uint32 nMaxDelay;				///< max delay
   /** @brief maximum strict jitter delay (10ms)<br>
     * nMaxStrictDelay is the maximum strict delay of jitter buffer.<br>
 	 * Valid values has to be larger than maximum delay, and less than allocated buffer.<br>
 	 * Recommended value is 10000 (use all allocated buffer), <br>
 	 * or choose a value between 30 (300ms) and 60 (600ms) is also not bad. 
 	 */
    uint32 nMaxStrictDelay;			///< max strict delay
   /** @brief optimzation factor of jitter buffer<br>
	 * The optimization factor is the value for adjusting the quality of the voice.  <br>
	 * The higher value of nJitterFactor means the better quality but more delay. <br>
	 * Valild value are 0 to 13. The value 1 is recommended. <br>
	 * Value 0 and 13 are fix delay(fifo). <br>value 0 is for low buffering application. 
	 * value 13 is for FAX or modem only.
 	 */
    uint32 nJitterFactor;			///< optimzation factor of jitter buffer 
   /** @brief G726 packing order<br>
     * Valid values are:<br>
     * 0: Packing none
     * 1: Packing left
     * 2: Packing right
     */
    uint32 nG726Packing;				///< G726 packing order 

    uint32 nG7111Mode;				///< G711.1 Mode. 1: R1, 2: R2a, 3: R2b, 4: R3
	
    /** @brief PCM Mode Setting<br>
     * Valid values are:<br>
     * 0: No action (use previous PCM enable mode)<br> 
     * 1: Auto mode (DSP will change to proper mode according to the voice codec attribute)<br> 
     * 2: Narrow-band Mode<br>
     * 3: Wide-band Mode
     */     

    uint32 nPcmMode;				///< PCM Mode Setting. 
} payloadtype_config_t;

/**
 * @ingroup TAPI_MEDIA_T38
 * @brief Structure for T.38 to be UDP payload type <br>
 *
 */
typedef struct
{
    uint32 chid;					///< channel id
    uint32 sid;					///< session id. Usually 0 for master session, 1 for slave(conference) session.
	
    uint32 bT38ParamEnable;			///< Enable T.38 parameter. If false, default settings are adopted and nT38MaxBuffer and nT38RateMgt are ignored. 

   /** @brief T.38 Max buffer size<br>
     * Valid valures are 200 to 600 (ms) <br>
     * Default is 500 (ms)
 	 */
    uint32 nT38MaxBuffer;			///< T.38 Max buffer size. It can be 200 to 600ms. (default: 500)
   /** @brief T38 Rate management<br>
     * Valid values are:<br>
     * 1: don't pass tcf data
     * 2: pass tcf data (Default)
 	 */
    uint32 nT38RateMgt;				///< T38 Rate management. 1: don't pass tcf data, 2: pass tcf data. (default: 2)
   /** @brief T38 Maximum rate<br>
     * Valid values are:<br>
     * 0: 2400 bps
     * 1: 4800 bps
     * 2: 7200 bps
     * 3: 9600 bps
     * 4: 12000 bps
     * 5: 14400 bps (Default)
 	 */
	uint32 nT38MaxRate;			///< T38 Max reate. 0~5: 2400, 4800, 7200, 9600, 12000, 14400. (default: 5)
   /** @brief T38 ECM<br>
     * Valid values are:<br>
     * 0: disable
     * 1: enable (Default)
 	 */
	uint32 bT38EnableECM;			///< T38 ECM. 1: enable, 0: disable. (default: 1)
   /** @brief T38 ECC signal<br>
     * Valid values are: 0~7<br>
     * Default value: 5
 	 */
	uint32 nT38ECCSignal;			///< T38 ECC signal. 0~7 (default: 5)
   /** @brief T38 ECC data<br>
     * Valid values are: 0~2<br>
     * Default value: 2
 	 */
	uint32 nT38ECCData;			///< T38 ECC data. 0~2 (default: 2)
   /** @brief T38 Spoofing<br>
     * Valid values are:<br>
     * 0: disable 
     * 1: enable (Default)
 	 */
	uint32 bT38EnableSpoof;			///< T38 Spoofing. 1: enable, 0: disable. (default: 1)
   /** @brief T38 Packet Duplicate Num<br>
     * Valid values are: 0~2<br>
     * Default value: 0
 	 */
	uint32 nT38DuplicateNum;			///< T38 DuplicateNum. 0~2 (default: 0)
   /** @brief PCM Mode Setting<br>
     * Valid values are:<br>
     * 0: No action (use previous PCM enable mode)<br>
     * 1: Auto mode (DSP will change to proper mode according to the voice codec attribute)<br>
     * 2: Narrow-band Mode<br>
     * 3: Wide-band Mode
     */    
    uint32 nPcmMode;				///< PCM Mode Setting. 

} t38_payloadtype_config_t;

// +4 for int32 result;
//CT_ASSERT(sizeof(payloadtype_config_t) + 4 == sizeof(TstVoipPayLoadTypeConfig));

/**
 * @ingroup TAPI_DSP_INIT
 * @brief Get VoIP Feature
 * @retval 0 Success
 */
int32 rtk_GetVoIPFeature(void);

/**
 * @ingroup TAPI_DSP_INIT
 * @brief Interface Initialization
 * @param ch The channel number.
 * @note \ref rtk_InitDsp must be called before any use of 
 *       manipulative function of voice
 * @retval 0 Success
 */
int32 rtk_InitDsp(int ch);

/**
 * @ingroup TAPI_DSP_EC
 * @brief Turn on or off LEC 
 * @param chid The channel number.
 * @param support_lec LEC switch <br> 0: Turn off LEC<br> 1: Turn on LEC
 * @retval 0 Success
 */
int32 rtk_SetG168Lec(uint32 chid, uint32 support_lec);

/**
 * @ingroup TAPI_DSP_EC
 * @brief Set the Echo Canceller Mode for VBD 
 * @param chid The channel number.
 * @param vbd_high_ec_mode 
 * 0: EC auto change is off <br> 1: EC auto off <br> 2: EC auto restore with NLP_mute mode
 * @param vbd_low_ec_mode  
 * 0: EC auto change is off <br> 1: EC auto off <br> 2: EC auto restore with NLP_mute mode
 * @param ec_restore_val EC restore value for low/high VBD EC auto restore mode is enable (Available only when vbd_high_ec_mode or vbd_low_ec_mode = 2)<br>
 *   0: Turn off the echo canceller <br> 1: Turn on the echo canceller
 * @note VBD_high_EC_mode/ VBD_low_EC_mode <br>
 * - EC auto change is off <br>Once detected the fax, the echo canceller has not changed its status. <br> 
 * - EC auto off<br>Once detected the fax, the echo canceller will be disabled.<br>
 * - EC auto restore with NLP_mute mode<br>Once detected the fax, the echo canceller enable or disable depends on the ec_restore_val. <br> <br>
 * Whatever the mode, the echo canceller is restored to the web pages configuration when the fax ends.
 * @retval 0 Success
 */
int32 rtk_SetVbdEc(uint32 chid, uint32 vbd_high_ec_mode, uint32 vbd_low_ec_mode, uint32 ec_restore_val);


/**
 * @ingroup TAPI_DSP_EC
 * @brief Set echo tail for LEC
 * @param chid The channel number. 
 * @param echo_tail The echo tail length that is the length of time over which it can cancel echoes. <br>
 * Value range: 1~32 <br> Unit: ms <br> Step: 1 
 * @param nlp Non-linear processor mode <br>
 * 0: NLP off <br> 1: NLP_mute <br> 2: NLP_shift <br> 3: NLP_cng
 * @param at_stepsize NLP Attack time stepsize  <br> Step: 1<br> Resolution:
 *         	   0.001
 * @param rt_stepsize NLP Release time stepsize <br> Step: 1<br> Resolution:
 *         	   0.001
 * @param cng_flag Comfort noise flag<br>
 * 0: Turn off comfort noise in NLP mute mode<br> 1: NLP mute 
 * mode with the comfort noise 
 * @note nlp <br>0: NLP off. Turn off NLP processing. <br>
 * 1: NLP_mute. Removes the nonlinear and residual part of the echo. <br> 
 * 2: NLP_shift. To improve the voice quality when users in a specific environment are some of the voices is discontinuous.<br> 
 * 3: NLP_cng. Not only to remove residual echo, but also generate the comfort noise. 
 * @note cng_flag<br> The comfort noise will add the NLP 
 *       processed signal when cng_flag is enabled. <br>This will
 *       be used only in the NLP_mute mode. <br>
 *		   at_stepsize / rt_stepsize usage:<br>
 *        such as set at_stepsize=5 (in fact corresponds to a value of 0.005)
 * @retval 0 Success
 */
int32 rtk_SetEchoTail(uint32 chid, uint32 echo_tail, uint32 nlp, uint32 at_stepsize, uint32 rt_stepsize, uint32 cng_flag);

/**
 * @ingroup TAPI_OBSOLETE_GROUP
 * @brief Light LED if SIP Register OK
 * @param chid The channel number.
 * @param isOK SIP register status.<br>
 *  0: SIP register failed <br> 1: SIP register OK
 * @retval 0 Success
 */
int rtk_SipRegister(unsigned int chid, unsigned int isOK);

/**
 * @ingroup TAPI_DSP_INIT
 * @brief Reinit DSP for FXS/FXO channel when on hook
 * @param chid The channel number. 
 * @retval 0 Success
 */
int32 rtk_OnHookReinit(uint32 chid);

/**
 * @ingroup TAPI_PHONE_EVENT
 * @brief Get the SLIC state  
 * @param chid The channel number.
 * @param pval The SLIC state. Please refer to \ref SLICEVENT for more detail states.
 * @retval 0 Success
 */  
int32 rtk_GetSlicEvent(uint32 chid, SLICEVENT *pval);

/**
 * @ingroup TAPI_OBSOLETE_GROUP
 * @brief Get the FXS state
 * 
 * This API translates value from rtk_GetSlicEvent(), and <br>
 * does some additional DSP settings. <br>
 * Thus, it can be seen as: 
 * rtk_GetFxsEvent() = rtk_GetSlicEvent() + rtk_GetDtmfEvent() + additional DSP settings 
 * @param chid The channel number.
 * @param pval The phone state. Please refer to \ref SIGNSTATE for more detail states.
 * @retval 0 Success
 * @sa rtk_OffHookAction, rtk_OnHookAction
 */
int32 rtk_GetFxsEvent(uint32 chid, SIGNSTATE *pval);

/**
 * @ingroup TAPI_PHONE_EVENT
 * @brief Process FXS event
 * 
 * This API does some additional DSP settings and translate kernel Event to app Event. <br>
 * Event translater is more like a Event wraper for upper application.
 * @param chid The channel number.
 * @param evt The Voip event ID. Please refer to \ref VoipEventID for more detail states.
 * @param pval The phone state. Please refer to \ref VoipEventID for more detail states.
 * @retval 0 Success
 * @sa rtk_OffHookAction, rtk_OnHookAction
 */
int32 rtk_ProcessFxsEvent(uint32 chid, VoipEventID evt, SIGNSTATE *pval);

/**
 * @ingroup TAPI_PHONE_EVENT
 * @brief Get the DAA state
 * @param chid The channel number.
 * @param pval The FXO state. Please refer to \ref DAAEVENT for more detail states.
 * @retval 0 Success
 */ 
int32 rtk_GetDaaEvent(uint32 chid, DAAEVENT *pval);

/**
 * @ingroup TAPI_OBSOLETE_GROUP 
 * @brief Get the FXO state
 * 
 * This API translates value from rtk_GetDaaEvent(), and <br>
 * does some additional DSP settings. <br>
 * Thus, it can be seen as: 
 * rtk_GetFxoEvent() = rtk_GetDaaEvent() + additional DSP settings 
 * @param chid The channel number.
 * @param pval The FXO state. Please refer to \ref SIGNSTATE for more detail states.
 * @retval 0 Success
 * @sa rtk_OffHookAction, rtk_OnHookAction
 */
int32 rtk_GetFxoEvent(uint32 chid, SIGNSTATE *pval);

/**
 * @ingroup TAPI_PHONE_EVENT
 * @brief Process FXO event
 * 
 * This API does some additional DSP settings and translate kernel Event to app Event. <br>
 * Event translater is more like a Event wraper for upper application.
 * @param chid The channel number.
 * @param evt The Voip event ID. Please refer to \ref VoipEventID for more detail states.
 * @param pval The FXO state. Please refer to \ref VoipEventID for more detail states.
 * @retval 0 Success
 * @sa rtk_OffHookAction, rtk_OnHookAction
 */
int32 rtk_ProcessFxoEvent(uint32 chid, VoipEventID evt, SIGNSTATE *pval);

/**
 * @ingroup TAPI_PHONE_EVENT
 * @brief Get the DECT state
 * @param chid The channel number.
 * @param pval The phone state. Please refer to \ref SIGNSTATE for more detail states.
 * @retval 0 Success
 * @sa rtk_OffHookAction, rtk_OnHookAction
 */
int32 rtk_GetDectEvent(uint32 chid, SIGNSTATE *pval);

/**
 * @ingroup TAPI_OBSOLETE_GROUP
 * @brief Get the FXO event
 * @param chid The channel number.
 * @param pval The FXO event.
 * @note rtk_GetRealFxoEvent() can get more detail FXO event than rtk_GetFxoEvent().
 * @retval 0 Success
 * @sa rtk_OffHookAction, rtk_OnHookAction
 */
int32 rtk_GetRealFxoEvent(uint32 chid, FXOEVENT *pval);

/**
 * @ingroup TAPI_PHONE_EVENT
 * @brief Get the VoIP event
 * @param stVoipEvent \ref TstVoipEvent to know detail.
 * @retval 0 Success
 */
int32 rtk_GetVoIPEvent( TstVoipEvent *stVoipEvent );

/**
 * @ingroup TAPI_PHONE_EVENT
 * @brief Flush the VoIP event
 * @param stFlushVoipEvent \ref TstFlushVoipEvent to know detail.
 * @retval 0 Success
 */
int32 rtk_FlushVoIPEvent( TstFlushVoipEvent *stFlushVoipEvent );

/**
 * @ingroup TAPI_PHONE_FXS
 * @brief Setup the flash hook time
 *
 * Flash hook or hookflash simulates quickly hanging up then picking up again. <br>
 * This api is used to set a time period of a quick off-hook/on-hook/off-hook cycle. <br>
 * <br>
 * This action can signal the telephone exchange to do something.  <br>A common use of hook 
 * flash is to switch to another incoming call with the call waiting service. Another use 
 * is to indicate a request for voice conferencing. <br>
 * <br>
 * If on-hook time period between min_time and max_time detected by FXS will be treated as 
 * a flash-hook event. <br>If time period is large than max_time, it will triggers on_hook event 
 * to upper application. 
 *
 * @param chid The channel number.
 * @param min_time The min hang up time period for flash hook. (Unit: ms)
 * @param max_time The max hang up time period for flash hook. (Unit: ms)
 * @retval 0 Success
 */
int32 rtk_SetFlashHookTime(uint32 chid, uint32 min_time, uint32 max_time);

/**
 * @ingroup TAPI_PHONE_FXS
 * @brief Setup the off hook time
 * @param chid The channel number.
 * @param offhook_time The min time period for off hook detection. (Unit: ms)
 * @retval 0 Success
 */
int32 rtk_SetOffHookTime(uint32 chid, uint32 offhook_time);

/**
 * @ingroup TAPI_PHONE_FXS
 * @brief Enable/Diable FXS Ringing
 * 
 * Enable/Disable to Start/Stop Ringer on a specific FXS line.
 *
 * @param chid The FXS channel number.
 * @param bRinging 
 *  0: Disable the FXS ringing. <br> 1: Enable the FXS ringing.
 * @retval 0 Success
 */
int32 rtk_SetRingFxs(uint32 chid, uint32 bRinging);

/**
 * @ingroup TAPI_DSP_VAD
 * @brief Set VAD/CNG threshold and SID noise level
 * 		 configuration
 *
 * The comfort noise payload, which is also known as a Silence Insertion
 * Descriptor (SID) frame and the CN payload format are defined in RFC 3389. <br> 
 * VAD detects current voice energy to determine the SID noise level. <br>
 * @param chid The channel number.
 * @param mid The media session number.
 * @param nThresVAD VAD Threshold
 * @param nThresCNG CNG Threshold
 * @param nModeSID SID Configuration Mode
 * \htmlonly  <font color=#000000> <br> <br>
 * <table border=1 width=230 style="border-collapse:collapse;" borderColor=black > 
 * <td align=center><b>SID Configuration Mode</b></td> <td align=center><b>Index</b></td> <tr>  
 * <td align=center>Disable  Configuration</td> <td align=center>0 </td>  <tr>  
 * <td align=center>Fixed noise level </td> <td align=center>1</td> <tr>  
 * <td align=center>Adjust noise level  </td> <td align=center>2</td> <tr>  
 * </table> </font> \endhtmlonly 
 * <br> Disable configuration: The SID noise level is determined by VAD. <br> 
 * Fixed noise level: The SID noise level is configured by user. <br> 
 * Adjust noise level: The SID noise level is determined by VAD and the user's settings.<br>
 * @param nLevelSID SID Noise Level (Available only when the nModeSID=1) <br> 
 * Available range: 0~127 (0dBoV ~ -127dBoVm, 0dBov is maximum value). <br> Unit: dBoV <br>
 * @param nGainSID SID Gain Adjustment (Available only when the nModeSID=2) <br>
 * Available range: +127~-127, 0(not change), the positive/negative value is gain up/down. <br> Unit: dBoV <br> 
 * @retval 0 Success
 */    
int32 rtk_SetThresholdVadCng( int32 chid, int32 mid, int32 nThresVAD, int32 nThresCNG, int32 nModeSID, int32 nLevelSID, int32 nGainSID );

/**
 * @ingroup TAPI_PHONE_FXS
 * @brief Disable all FXS Ring
 * 
 * When this api is invoked with parameter bDisable = 1, all request from rtk_SetRingFxs() 
 * will be permanently ignored until this api is being invoked again with parameter
 * bDisable = 0. <br>
 * rtk_DisableRingFxs() has higher priority than rtk_SetRingFxs().<br>
 *
 * @param bDisable flag to disable all FXS Ring. <br>
 * 0: No action. <br> 1: Disable all FXS ringing.
 * @retval 0 Success
 * @note Test Issue.
 */
int32 rtk_DisableRingFxs(int bDisable);

/**
 * @ingroup TAPI_PHONE_FXS
 * @brief Set the ring cadence to a FXS line.
 * @param chid The FXS channel number.
 * @param cad_on_msec The time period of Cadence-On (Unit: ms).
 * @param cad_off_msec The time period of Cadence-Off (Unit: ms).
 * @retval 0 Success
 */
int32 rtk_SetSlicRingCadence(uint32 chid, uint16 cad_on_msec, uint16 cad_off_msec);

/**
 * @ingroup TAPI_PHONE_FXS
 * @brief Set the multi-ring cadence to a FXS line.
 * @param chid The FXS channel number.
 * @param pRingCad The pointer of Ring Candence parameters, it support up to 4 cadence.
 * @retval 0 Success
 */
int32 rtk_SetMultiRingCadence(uint32 chid, TstVoipCadence* pRingCad);

/**
 * @ingroup TAPI_PHONE_FXS
 * @brief Set the ring frequency and amplitude of FXS
 *
 * Many of ring frequencies and amplitudes have presents that are available for easily using.<br> 
 * You can select theindex from the preset table. 
 * 
 * @param chid The FXS channel number.
 * @param preset The Ring frequency, amplitude preset table index. 
 * @note The preset table depends on various SLIC vendor.
 * @retval 0 Success
 */
int32 rtk_SetSlicRingFreqAmp(uint32 chid, uint8 preset);

/**
 * @ingroup TAPI_PHONE_FXS
 * @brief Set SLIC line voltage
 * @param chid The channel number 
 * @param flag SLIC line voltage flag. <br>
 *  0: zero voltage. SLIC port is deny of service. <br> 
 *  1: normal voltage.  Put SLIC to On-Hook tranmission state. <br> 
 *  2: reverse voltage. Put SLIC to Polarity Reversal On-Hook tranmission state.
 * @retval 0 Success
 */
int32 rtk_SetSlicLineVoltage(uint32 chid, uint8 flag);

/**
 * @ingroup TAPI_PHONE_FXS
 * @brief Generate SLIC CPC signal
 *
 * CPC (Calling Party Control) or Open Loop Disconnect is a signal sent by a FXS device 
 * and received by a FXO device, to indicate that the calling party has hung up.
 *
 * A wink function slowly ramps down the TIP-RING voltage (VTR) to zero followed by a 
 * return to the original on-hook open-circuit dc voltage value. (from Si32178 datasheet)
 *
 * Don't confuse CPC with Battery Reversal.
 *
 * @param chid The channel number 
 * @param cpc_ms SLIC CPC signal period <br> Unit: ms
 * @note only supported by Silab SLIC currently.
 * @retval 0 Success
 */
int32 rtk_GenSlicCpc(uint32 chid, uint32 cpc_ms);


/**
 * @ingroup TAPI_PHONE_FXO
 * @brief Set the Ring Detection Parameters to FXO
 *
 * This api is used to define FXO ring cadence for FXO ring detection. <br>
 * Low layer state machine reports VEID_HOOK_FXO_RING_ON and VEID_HOOK_FXO_RING_OFF 
 * when low layer detects the ring cadence matched the setting.
 *
 * @param ring_on_msec The time period of the ring On.
 * @param first_ringoff_msec The time period of the first ring Off.
 * @param ring_off_msec The time period of the ring Off.
 * @retval 0 Success
 */
int32 rtk_SetFxoRingDetection(uint16 ring_on_msec, uint16 first_ringoff_msec, uint16 ring_off_msec);

/**
 * @ingroup TAPI_DSP_CID
 * @brief Stop Caller ID Generation
 * @param chid The FXS channel number.
 * @param cid_mode Caller ID mode. <br>
 * 0: DTMF Caller ID <br> 1: FSK Caller ID<br> 2: Both DTMF and FSK Caller ID <br>
 * @note If stop CID generation, we can't receive end event. 
 * @retval 0 Success
 */ 
int32 rtk_StopCid(uint32 chid, char cid_mode);

/**
 * @ingroup TAPI_DSP_CID_DTMF
 * @brief Set Caller ID string for DTMF generation 
 * @param chid The FXS channel number.
 * @param str_cid The Caller ID string.
 * @note This API is only set caller ID string, so application should enable PCM and check complete. <br>
 * An easy way to achieve this generation, one can use 
 * integration API named \ref rtk_GenDtmfCid. 
 * @retval 0 Success
 */  
int32 rtk_SetDtmfCidString(uint32 chid, const char *str_cid);

/**
 * @ingroup TAPI_DSP_CID_DTMF
 * @brief Generate DTMF Caller ID
 * @param chid The FXS channel number.
 * @param str_cid The Caller ID string.
 * @note When application calls this API to send DTMF caller ID, DSP ring the phone automatically after sending caller ID. <br>
 * So, there is no need to ring the phone by application. <br> 
 * If caller ID is configured to send between 1st and 2nd ring, DSP also can handle it with ring cadence automatically.
 * @retval 0 Success
 */  
int32 rtk_GenDtmfCid(uint32 chid, const char *str_cid);

/**
 * @ingroup TAPI_DSP_CID_DTMF
 * @brief Set the parameters of DTMF Caller ID Generation
 * @param chid The FXS channel number.
 * @param  start_digit DTMF Caller ID start digit
 * @param  end_digit DTMF Caller ID end digit
 * @param  auto_ctrl DSP automatic action <br>
 * Bit 0: Auto start/end digit send <br>
 * Bit 1: Auto SLIC Ring <br>
 * Bit 2: Before 1st Ring<br>
 * Bit 3: Auto SLIC action (SLIC action : such as SLIC hook statue check, line polarity change (not include SLIC Ringing))<br>
 * \htmlonly  <font color=#000000> <br> <br>
 * <table border=1 width=900 style="border-collapse:collapse;" borderColor=black > 
 * <td align=center><b>value</b></td> <td align=center><b>Bit 0</b></td> <td align=center><b>Bit 1</b></td> <td align=center><b>Bit 2</b></td> <td align=center><b>Bit 3</b></td><tr>  
 * <td align=center>0</td> <td align=center>Non-Auto mode<br></td> <td align=center>Non-auto mode<br>(DSP send caller ID string only. If caller ID need start/end digits,<br> developer should add them to caller ID strings)</td> <td align=center>After 1st Ring</td> <td align=center>Non-auto mode</td><tr>  
 * <td align=center>1</td> <td align=center>Auto mode<br></td> <td align=center>Auto mode</td> <td align=center>Before 1st Ring</td> <td align=center>Auto mode</td><tr>  
 * </table> </font> \endhtmlonly 
  * @param dtmf_on_ms DTMF digits on duration (msec, must be multiple of 10msec)
 * @param dtmf_pause_ms Inter DTMF digits pause duration (msec, must be multiple of 10msec)
 * @param dtmf_pre_silence The silence duration before DTMF caller ID (msec, must be multiple of 10msec)
 * @param dtmf_end_silence The silence duration after DTMF caller ID (msec, must be multiple of 10msec)
 * @retval 0 Success
 */
int32 rtk_SetDtmfCidParam(uint32 chid, DTMF_DIGIT start_digit, DTMF_DIGIT end_digit, uint32 auto_ctrl, uint32 dtmf_on_ms, uint32 dtmf_pause_ms, uint32 dtmf_pre_silence, uint32 dtmf_end_silence);

/**
 * @ingroup TAPI_DSP_CID_FSK
 * @brief FSK Caller ID Generation
 * @param chid The FXS channel number.
 * @param str_cid The Caller ID
 * @param str_date The Caller Date
 * @param str_cid_name The Caller ID Name
 * @param mode Mode <br>
 * 0: type I<br> 1: type II
 * @note When application calls this API to send FSK caller ID, DSP ring the phone automatically after sending caller ID. <br>
 * So, there is no need to ring the phone by application. <br>If caller ID is configured to sendbetween 1st and 2nd ring, DSP also can handle it with
 * ring cadence automatically.
 * @retval 0 Success
 * @retval -2 Busy state 
 */
int32 rtk_GenFskCid(uint32 chid, char *str_cid, char *str_date, char *str_cid_name, char mode);

/**
 * @ingroup TAPI_DSP_CID_FSK
 *
 *  This API provides to generate Multiple Data Message Format
 *  (MDMF) which is Name+number caller ID.
 * @brief FSK Caller ID Generation
 * @param chid The FXS channel number.
 * @param pClid The pointer of FSK Caller ID Data
 * @param num_clid_element The number of FSK Caller ID element
 * @note When application calls this API to send FSK caller ID, DSP ring the phone automatically after sending caller ID.<br> 
 * So, there is no need to ring the phone by application. <br>
 * If caller ID is configured to send between 1st and 2nd ring, DSP also can handle it with ring cadence automatically.
 * @retval 0 Success
 * @retval -2 Busy state
 */ 
int32 rtk_GenMdmfFskCid(uint32 chid, TstFskClid* pClid, uint32 num_clid_element);

/**
 * @ingroup TAPI_DSP_CID
 * @brief Caller ID generation and FXS Ring
 * @param chid The FXS channel number.
 * @param cid_mode The mode of Caller ID <br>
 * 0:DTMF<br> 1:FSK
 * @param str_cid The Caller ID
 * @param str_date The Caller Date
 * @param str_cid_name The Caller ID Name
 * @param fsk_type FSK type <br>
 * 0: type I<br> 1: type II
 * @param bRinging 
 * 0: Disable Ringing<br> 1: Enable Ringing
 * @note When str_cid is NULL(no CID number), this API won't send CID, and then just perform disable/enable FXS Ring. <br>
 * When application calls this API to send FSK caller ID, DSPring the phone automatically after sending caller ID
 * even bRinging is equal to zero. <br>So, there is no need to ring the phone by application. <br>
 * If caller ID is configured to send between 1st and 2nd ring, DSP also can handle it with ring cadence automatically. <br>
 * If str_cid[0] is equal to zero, caller ID won't be send. In this situation, this API will ring on/off the phone according to the parameter bRinging.
 * @retval 0 Success
 */
int32 rtk_GenCidAndFxsRing(uint32 chid, char cid_mode, char *str_cid, char *str_date, char *str_cid_name, char fsk_type, uint32 bRinging);

/**
 * @ingroup TAPI_DSP_CID_FSK
 * @brief Set the FSK Area
 * @param chid The FXS channel number.
 * @param area The area of FSK.   <br>
 *  Bit 0-2: Area  <br> - 0: Bellcore <br> - 1: ETSI<br> - 2: BT<br> - 3: NTT <br>
 *  Bit 3: Caller ID Prior First Ring <br> - 0: Between 1st ring and 2nd ring  <br> - 1: Send Caller ID before 1st ring <br>
 *  Bit 4: Dual Tone before Caller ID (Fsk Alert Tone) <br> - 0: False<br> - 1: True <br>
 *  Bit 5: Short Ring before Caller ID <br> - 0: False<br> - 1: True <br>
 *  Bit 6: Reverse Polarity before Caller ID (Line Reverse) <br> - 0: Forward<br> - 1: Reverse <br>
 *  Bit 7: FSK Date & Time Sync and Display Name <br> - 0: False<br> - 1: True <br>
 *  Bit 8: Auto SLIC Action <br> - 0: Disable<br> - 1: Enable <br> SLIC action, such as SLIC Ringing, SLIC hook statue check, line polarity change
 * @retval 0 Success
 */
int32 rtk_SetFskArea(uint32 chid, uint32 area);

/**
 * @ingroup TAPI_DSP_CID_FSK
 * @brief Set the FSK Caller ID Parameters
 * @param para The variable pointer of FSK Caller ID Parameters
 * @retval 0 Success
 */
int32 rtk_SetFskClidPara(TstVoipFskPara* para);

/**
 * @ingroup TAPI_DSP_CID_FSK
 * @brief Get the FSK Caller ID Parameters
 * @param para The variable pointer to save current FSK Caller ID Parameters
 * @retval 0 Success
 */
int32 rtk_GetFskClidPara(TstVoipFskPara* para);

/**
 * @ingroup TAPI_DSP_CID_FSK
 * @brief Get the FSK caller ID send completed or not
 * @param chid The FXS channel number.
 * @param cid_state CID state <br>
 * 0: FSK CID transmission is completed. <br> 1: Sending FSK CID.
 * @retval 0 Success
 */
int32 rtk_GetFskCidState(uint32 chid, uint32 *cid_state);

/**
 * @ingroup TAPI_OBSOLETE_GROUP
 * @brief Set the FSK Caller ID Generation Mode
 * @param chid The FXS channel number.
 * @param isOK 
 * 0: Hardware generates FSK caller ID <br> 
 * 1: Software generates FSK caller ID
 * @retval 0 Success
 */
int32 rtk_SetCidFskGenMode(unsigned int chid, unsigned int isOK);

/**
 * @ingroup TAPI_DSP_CID_FSK
 * @brief Generate the VMWI via FSK
 * @param chid The FXS channel number.
 * @param pVmwi The pointer of VMWI Data
 * @param num_vmwi_element The number of VMWI element
 * @retval 0 Success
 * @retval -2 Busy state
 */  
int32 rtk_GenFskVmwi(uint32 chid, TstFskClid* pVmwi, uint32 num_vmwi_element);

/**
 * @ingroup TAPI_OBSOLETE_GROUP
 * @brief Generate Caller ID and VMWI via FSK
 * @param chid The FXS channel number.
 * @param str_cid The caller ID string or VMWI
 * @param str_date The caller date
 * @param str_cid_name The caller ID name
 * @param mode Mode <br>
 * 0: type I<br> 1: type II
 * @param msg_type FSK_MSG_CALLSETUP:cid or FSK_MSG_MWSETUP:vmwi
 * @retval 0 Success
 * @retval -2 Bus state
 */
int32 rtk_GenFskCidVmwi(uint32 chid, char *str_cid, char *str_date, char *str_cid_name, char mode, char msg_type);

/**
 * @ingroup TAPI_DSP_AGC
 * @brief Set the speaker and mic voice gain
 * @param chid The channel number.
 * @param spk_gain speaker gain <br>
 * Available range: -32~31  <br> Mute: -32 <br> Default: 0dB <br> Unit:dB <br>Step:1dB 
 * @param mic_gain microphone gain <br>
 * Available range: -32~31  <br> Mute: -32 <br> Default: 0dB <br> Unit:dB <br>Step:1dB 
 * @note The speaker gain/Mic gain means that the direction of the voice to PCM/RTP.
 * @retval 0 Success
 */
int32 rtk_SetVoiceGain(uint32 chid, int spk_gain, int mic_gain);

/**
 * @ingroup TAPI_DSP_TONE
 * @brief Set the answer tone detection
 * @param chid The channel number.
 * @param config_1 The answer tone detection mode, 0:disable 
 *     	   1:enable <br>
 * Bit0: CNG_TDM<br> 
 * Bit1: ANS_TDM <br>
 * Bit2: ANSAM_TDM<br>
 * Bit3: ANSBAR_TDM<br> 
 * Bit4: ANSAMBAR_TDM<br> 
 * Bit5: BELLANS_TDM<br> 
 * Bit6: V22_TDM<br> 
 * Bit7: V8bisCre_TDM<br>
 * Bit8: CNG_IP<br> 
 * Bit9: ANS_IP<br> 
 * Bit10: ANSAM_IP<br>
 * Bit11: ANSBAR_IP<br> 
 * Bit12: ANSAMBAR_IP<br> 
 * Bit13: BELLANS_IP<br> 
 * Bit14: V22_IP<br> 
 * Bit15: V8bisCre_IP <br>
 * Bit16: V21flag_TDM <br> 
 * Bit17: V21flag_IP<br>
 * Bit18: V21DIS_TDM <br> 
 * Bit19: V21DIS_IP<br>
 * Bit20: V21DCN_TDM <br> 
 * Bit21: V21DCN_IP<br>
 * Bit22: V21Channel_2_TDM<br>
 * Bit23: V21Channel_2_IP<br>
 * Bit24: V21Channel_1_TDM<br>
 * Bit25: V21Channel_1_IP<br>
 * Bit26: V23_TDM<br>
 * Bit27: V23_IP<br>
 * Bit28: BELL202ANS_TDM<br>
 * Bit29: BELL202ANS_IP<br>
 * Bit30: BELL202CP_TDM<br>
 * Bit31: BELL202CP_IP<br>
 * @param config_2 The ECM mode setting, bitmask 0:disable 1:enable <br>
 * Bit0: ECM PPS-MPS_TDM <br> 
 * Bit1: ECM PPS-MPS_IP <br> 
 * Bit2: ECM PPS-EOP_TDM <br> 
 * Bit3: ECM PPS-EOP_IP <br> 
 * Bit4: ECM RNR_TDM <br> 
 * Bit5: ECM RNR_IP <br> 
 * @param threshold The answer tone detecting threshold.
 * Available range: 0 ~ 13<br>0:  0dB<br> 1: -3dB<br> 2:  -6dB<br>3: -9dB<br> 4: -12dB<br> 5: -15dB<br>
 * 6: -18dB<br> 7: -21dB<br> 8: -24dB<br> 9: -27dB<br> 10:-30dB (Default setting)<br>
 * 11: -33dB<br> 12:-36dB<br> 13:-39dB.
 * @retval 0 Success
 */
int32 rtk_SetAnswerToneDet(uint32 chid, uint32 config_1, uint32 config_2, int32 threshold);

/**
 * @ingroup TAPI_DSP_VAD
 * @brief Set silence detection threshold 
 * @param chid The channel number.
 * @param energy Seen as silence if less than this energy.  <br>
 * Available range: 0 ~ 127 (indicates 0dB ~ -127dB)<br> 
 * Default: 55<br> Unit: dB
 * @param period Silence period in unit of ms <br>Default: 10000
 * @note It may be better to use (T.30 T2 = 6 +- 1 sec) as silence period. 
 * @retval 0 Success
 */
int32 rtk_SetSilenceDetThreshold(uint32 chid, uint32 energy, uint32 period);

/**
 * @ingroup TAPI_DSP_AGC
 * @brief Set the speaker AGC(Automatic gain control) enable/disable
 * @param chid The channel number.
 * @param support_gain  The support_gain <br>
 * 0: Disable <br> 1: Enable <br>Default: 0 
 * @param adaptive_threshold The adaptive threshold <br> 
 * Available range: 0 to 70 (0 ~ -70dB) <br>Unit: dB<br> Default: 55 
 * @retval 0 Success
 */
int32 rtk_SetSpkAgc(uint32 chid, uint32 support_gain, uint32 adaptive_threshold);

/**
 * @ingroup TAPI_DSP_AGC
 * @brief  Set the speaker AGC(Automatic gain control) target level. 
 *
 * The AGC will apply an appropriate gain to the input signal in order to achieve this desired output level.<BR>
 * Target level: This is the desired average level of the output signal. (9 target levels are available)<BR>
 * The AGC aims to keep a constant recording or playing volume irrespective of the input signal level. <BR>
 * This is achieved by setting the AGC target level, gain-up and gain-down. <BR>
 * If the input is loud, the gain (gain-down) is turned down. <BR>
 * If the input is soft, the gain (gain-up) is turned up.
 * @param chid The channel number.
 * @param level The target level of the speaker <br>
 *  Available range: 0~8 <br> Unit: dB <br> Step: 3dB
 *  <br> Default: 0
 *  
 <pre>  
      level  target level
        0       -25 dB
        1       -22 dB
        2       -19 dB
        3       -16 dB
        4       -13 dB
        5       -10 dB
        6        -7 dB
        7        -4 dB
        8        -1 dB
 </pre> 
 * @note
 * It is necessary to target level for the threshold when the signal level after the gain-up compensation is greater than the target level.<br>
 * It is necessary to target level for the threshold when the signal level after the gain-down compensation is less than the target level. <br>
 * For example: <br>
 * One signal level(-15dB), gain-up=5dB, target level=-13dB <br>
 * After gain compensation, the processed signal should be -10dB but obviously it is greater than the target level. <br>
 * Because the maximum level is the target level, the the processed signal level will not greater than the target level.
 * @retval 0 Success
 */
int32 rtk_SetSpkAgcLvl(uint32 chid, uint32 level);

/**
 * @ingroup TAPI_DSP_AGC
 * @brief Set the speaker AGC(Automatic gain control) max gain up 
 * @param chid The channel number.
 * @param gain The max gain up of speaker <br>
 * Available range: 0~8 <br> Unit: dB <br> Step: 1dB <br>
 * Default: 5
 *
 <pre>
       gain   gain-up
        0      1 dB
        1      2 dB
        2      3 dB
        3      4 dB
        4      5 dB
        5      6 dB
        6      7 dB
        7      8 dB
        8      9 dB
 </pre> 
 * @retval 0 Success
 */
int32 rtk_SetSpkAgcGup(uint32 chid, uint32 gain);

/**
 * @ingroup TAPI_DSP_AGC
 * @brief Set the speaker AGC(Automatic gain control) max gain down
 * @param chid The channel number.
 * @param gain The max gain down of speaker <br>
 * Available range: 0~8 <br> Unit: dB <br> Step: 1dB <br>
 * Default: 5
 *
 <pre>
       gain  gain-down
        0      -1 dB
        1      -2 dB
        2      -3 dB
        3      -4 dB
        4      -5 dB
        5      -6 dB
        6      -7 dB
        7      -8 dB
        8      -9 dB
 </pre> 
 * @retval 0 Success
 */
int32 rtk_SetSpkAgcGdown(uint32 chid, uint32 gain);

/**
 * @ingroup TAPI_DSP_AGC
 * @brief Set the MIC AGC(Automatic gain control) enable/disable
 * @param chid The channel number.
 * @param support_gain The support_gain <br>
 *  0: Disable <br> 1: Enable <br> Default: 0
 * @param adaptive_threshold The adaptive threshold <br> 
 * Available range: 0 to 70 (0 ~ -70dB) <br>Unit: dB<Br> Default: 55 
 * @retval 0 Success
 */
int32 rtk_SetMicAgc(uint32 chid, uint32 support_gain, uint32 adaptive_threshold);

/**
 * @ingroup TAPI_DSP_AGC
 * @brief Set the MIC AGC(Automatic gain control) target level. 
 *
 * The AGC will apply an appropriate gain to the input signal in order to achieve this desired output level.<BR>
 * Target level: This is the desired average level of the output
 * signal. (9 target levels are available) <br>
 * The AGC aims to keep a constant recording or playing volume irrespective of the input signal level. <BR>
 * This is achieved by setting the AGC target level, gain-up and gain-down. <BR>
 * If the input is loud, the gain (gain-down) is turned down. <BR>
 * If the input is soft, the gain (gain-up) is turned up.
 * @param chid The channel number.
 * @param level The target level of the microphone <br>
 *  Available range: 0~8 <br> Unit: dB <br> Step: 3dB
 *  <br> Default: 0
 *  
  <pre>  
      level  target level
        0       -25 dB
        1       -22 dB
        2       -19 dB
        3       -16 dB
        4       -13 dB
        5       -10 dB
        6        -7 dB
        7        -4 dB
        8        -1 dB
 </pre> 
 * @note
 * It is necessary to target level for the threshold when the signal level after the gain-up compensation is greater than the target level.<br>
 * It is necessary to target level for the threshold when the signal level after the gain-down compensation is less than the target level. <br>
 * For example: <br>
 * One signal level(-15dB), gain-up=5dB, target level=-13dB <br>
 * After gain compensation, the processed signal should be -10dB but obviously it is greater than the target level. <br>
 * Because the maximum level is the target level, the the processed signal level will not greater than the target level.
 * @retval 0 Success
 */
int32 rtk_SetMicAgcLvl(uint32 chid, uint32 level);

/**
 * @ingroup TAPI_DSP_AGC
 * @brief Set the MIC AGC(Automatic gain control) maximum gain up
 * @param chid The channel number.
 * @param gain The max gain up of microphone <br>
 * Available range: 0~8 <br> Unit: dB <br> Step: 1dB <br>
 * Default: 5
 <pre>
       gain   gain-up
        0      1 dB
        1      2 dB
        2      3 dB
        3      4 dB
        4      5 dB
        5      6 dB
        6      7 dB
        7      8 dB
        8      9 dB
 </pre>  
 * @retval 0 Success
 */
int32 rtk_SetMicAgcGup(uint32 chid, uint32 gain);

/**
 * @ingroup TAPI_DSP_AGC
 * @brief Set the MIC AGC(Automatic gain control) maximum gain down
 * @param chid The channel number.
 * @param gain The max gain down of microphone <br>
 * Available range: 0~8 <br> Unit: dB <br> Step: 1dB <br>
   Default: 5
 <pre>
       gain  gain-down
        0      -1 dB
        1      -2 dB
        2      -3 dB
        3      -4 dB
        4      -5 dB
        5      -6 dB
        6      -7 dB
        7      -8 dB
        8      -9 dB
 </pre>
 * @retval 0 Success
 */
int32 rtk_SetMicAgcGdown(uint32 chid, uint32 gain);

/**
 * @ingroup TAPI_OBSOLETE_GROUP
 * @brief Switch virtual DAA relay to PSTN line (replaced by
 		  \ref rtk_FxoOffHook)
 *
 * @param chid The FXS channel number 
 * @note This API is only used for Virtual DAA
 * @retval 1 Success
 * @retval 0xFF Line not connect or busy or not support
 */
int rtk_DaaOffHook(uint32 chid);

/**
 * @ingroup TAPI_PHONE_FXO
 * @brief Check Ringing via PSTN line
 *
 * Check is DAA(FXO) ringing or not.
 *
 * @param chid The specified FXS channel for FXO
 * @retval 1 PSTN Ringing
 */
int rtk_DaaRing(uint32 chid);

/**
 * @ingroup TAPI_OBSOLETE_GROUP
 * @brief On-Hook in PSTN line. (replaced by
 		  \ref rtk_FxoOnHook)
 *
 * @param chid The FXO channel number 
 * @retval 0 Success
 */
int rtk_DaaOnHook(uint32 chid);

/**
 * @ingroup TAPI_PHONE_FXO
 * @brief Set the Tx Gain of FXO.
 * @param gain The gain value (0..11) (0 ~ -11 dB)
 * Available range: 0 ~ -11 <br> Unit: dB <br> Step: 1dB <br>
 <pre>
       gain  gain-down
        0       0 dB
        1      -1 dB
        2      -2 dB
        3      -3 dB
        4      -4 dB
        5      -5 dB
        6      -6 dB
        7      -7 dB
        8      -8 dB
        9      -9 dB
       10     -10 dB
       11     -11 dB
 </pre>
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int32 rtk_SetDaaTxGain(uint32 gain);

/**
 * @ingroup TAPI_PHONE_FXO
 * @brief Set the Rx Gain of FXO
 * @param gain The gain value (0..11) (0 ~ -11 dB)
 * Available range: 0 ~ -11 <br> Unit: dB <br> Step: 1dB <br>
 <pre>
       gain  gain-down
        0       0 dB
        1      -1 dB
        2      -2 dB
        3      -3 dB
        4      -4 dB
        5      -5 dB
        6      -6 dB
        7      -7 dB
        8      -8 dB
        9      -9 dB
       10     -10 dB
       11     -11 dB
 </pre>
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int32 rtk_SetDaaRxGain(uint32 gain);

/**
 * @ingroup TAPI_PHONE_FXO
 * @brief Get the FXO Detected Caller ID
 * @param chid The FXO channel number
 * @param str_cid The Caller ID String
 * @param str_date The Date String
 * @param str_name The Caller ID Name (If str_date is a null string, ignore this parameter.)
 * @retval 0 Success
 * @note  How to get FXO Caller ID: <br> 
 * 1.	When application get FXO Ring event, then it's safe to get Caller ID.<br> 
 * 2.	If Caller ID is set to send between 1st ring and 2nd ring, then DSP report FXO Ring event till 2nd ring,
 * to let application safe to get Caller ID. <br> 
 * 3.	If no Caller ID is detected, then str_cid[0] = str_date[0] = 0.<br>
 * Virtual DAA not support.
 */
int32 rtk_GetDaaCallerID(uint32 chid, char *str_cid, char *str_date, char *str_name);

/** 
 * @ingroup TAPI_DSP_CID
 * @brief Set the FXO Caller ID Detection Mode
 * @param chid The FXO channel number
 * @param auto_det The FXO CID auto detection mode <br>
 * 0: Disable Caller ID Auto Detection <br> 1: Enable Caller ID Auto Detection (NTT Support). <br> 2: Enable
 * Caller ID Auto Detection (NTT Not Support) <br> Default: 2
 * @param cid_det_mode The Caller ID Mode for Caller ID Detection (available only when the auto_det=0) <br>
 * Available range: 0 ~ 4 <br>0: Bellcore FSK <br> 1: ETSI FSK <br> 2: BT FSK <br> 3: NTT FSK <br> 4: DTMF <br> 
 * Default: 4
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int32 rtk_SetCidDetMode(uint32 chid, int auto_det, int cid_det_mode);

/**
 * @ingroup TAPI_OBSOLETE_GROUP
 * @brief Set the DAA ISR Flow
 * @param chid The channel number
 * @param sid The session number. Usually 0 for master session, 1 for slave(conference) session.
 * @param flow The DAA ISR Flow <br>
 * 0: Normal<br> 1: PSTN 3-way conference <br> 2: PSTN call forward
 * @retval 1 Success
 * @retval 0xFF Line not connect or busy or not support
 * @note Virtual DAA not support
 */
int32 rtk_SetDaaIsrFlow(unsigned int chid, unsigned int sid, unsigned int flow);

/**
 * @ingroup TAPI_OBSOLETE_GROUP
 * @brief Get the DAA ISR Flow
 * @param chid The channel number
 * @param sid The session number. Usually 0 for master session, 1 for slave(conference) session.
 * @retval 0 Normal
 * @retval 1 PSTN 3-way conference
 * @retval 2 PSTN call forward
 * @note Virtual DAA not support
 */
int32 rtk_GetDaaIsrFlow(unsigned int chid, unsigned int sid);

/**
 * @ingroup TAPI_PHONE_FXO
 * @brief Dial PSTN Number for PSTN Call Forward
 *
 * This api dials phone number to PSTN(Central Office) via DAA(FXO).
 *
 * @param chid The FXO channel number
 * @param sid  The seesion number. Usually 0 for master session, 1 for slave(conference) session.
 * @param *cf_no_str The Pointer of the PSTN Number String
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int32 rtk_DialPstnCallForward(uint32 chid, uint32 sid, char *cf_no_str);

/**
 * @ingroup TAPI_OBSOLETE_GROUP
 * @brief (Un)Hold PSTN Line
 * @param slic_chid The slic channel number
 * @param daa_chid The daa channel number
 * @param config  The configuration of Hold(config=1) or Un-Hold(config=0)
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int32 rtk_SetPstnHoldCfg(unsigned int slic_chid, unsigned int daa_chid, unsigned int config);

/**
 * @ingroup TAPI_OBSOLETE_GROUP
 * @brief Get if DAA detect the busy tone (replaced by rtk_GetFxoEvent)
 * @param daa_chid The daa channel number
 * @retval busy_flag Busy tone is  1: Detected, 0: NOT detected
 * @note Virtual DAA not support
 */
int32 rtk_GetDaaBusyToneStatus(unsigned int daa_chid);

/**
 * @ingroup TAPI_OBSOLETE_GROUP 
 * @brief Check which FXS channel is replaced by FXO 
 * @param chid The FXS channel number
 * @retval 1: TRUE, 0: FALSE
 * @note Virtual DAA not support
 */
int rtk_GetDaaUsedByWhichSlic(uint32 chid);

/** 
 * @ingroup TAPI_PHONE_FXO
 * @brief Set the DAA Hybrid (Only for Silicon Lab Si3050 DAA)
 * @param chid The DAA channel number
 * @param index The DAA Hybrid index. For the detail parameters for each index, developer need the check the hybrid constants of DAA driver.
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int32 rtk_SetDaaHybrid(uint32 chid, uint8 index);

/** 
 * @ingroup TAPI_PHONE_FXO
 * @brief To tune the impedance for FXO port to match to the central office (Only for Silicon Lab Si3050 DAA)
 * @param chid The DAA channel number
 * @param control The FXO tune control parameter<br>
 * 0x0: Stop FXO tune Timer<br> 0x1: Start FXO tune timer<br> 0x2: Init FXO tune<br>
 * 0x3: Enable FXO tune<br> 0x4: Disable FXO tune<br>
 * \htmlonly 0x10000|(16-bit val): Set FXO tune busy timeout (msec), DSP default is 10000 msec. EX: 0x203E8 means for 1000 msec<br><br>
 * <i><span style="padding-left:30px"> "busy timeout" means that duration between test DMTF dial and busy tone from CO. </span><br>
 * <span style="padding-left:30px">When DAA off-hook, DSP will generate a DTMF tone to CO, and CO stop dial tone for a duration and then play busy tone.</span><br>
 * <span style="padding-left:30px">This duration is the "busy timeout" need to set to DSP. DSP will test in this duration.</span><br><br></i>\endhtmlonly
 * 0x20000|(16-bit val): Set FXO tune test tone duration (msec), DSP default is 1000 msec.<br>
 * @retval 0 Success
 * @note  The call flow for FXO tune:
 * \htmlonly  <i> <br> <br>
 * 1. Start FXO tune timer.<br>
 * 2. Init FXO tune.<br>
 * 3. Set the "busy timeout" and "test tone" duration if it is required.<br>
 * 4. Enable FXO tune. <br>
 * 5. Check the FXO tune status (see below).<br>
 * 6. If FXO tune is done, there is no need to stop the FXO tune. DSP will stop and set the impedance of best ERL automaitcally.  If FXO tune is not done, you can stop the FXO tune to terminate FXO tune process.
 * </i> \endhtmlonly  
 * @note  Developer can "cat /proc/voip/ch2/fxo_tune" to check the FXO tune status
 * \htmlonly  <font color=blue><i> <br> <br>
 # cat /proc/voip/ch2/fxo_tune<br>
 * *FXO tune status, ch=2<br>
 *  - tuning state: 0<br>
 *  - busy timeout setting: 10000 msec<br>
 *  - current tuning index: 72<br>
 *  - test tone duration: 1000 msec<br>
 *  - current test tone power: 8301562<br>
 *  - <font color=red>test done: Yes</font> <br>
 *  - <font color=red>best ERL 33 dB </font>(index = 17)<br>
 * *Note: echo 0xff into fox_tune to dump ERL value of each tuning index<br><br>
 * </font></i> \endhtmlonly  
 * "test done: Yes" means that FXO tuning is done.<br>
 * "best ERL 33 dB" show the best echo return loss in dB.<br>
 */
int32 rtk_SetFxoTune(uint32 chid, uint32 control);

/**
 * @ingroup TAPI_MEDIA_MISC
 * @brief Assign the active session.
 * @param chid The channel number.
 * @param sid The session number. Usually 0 for master session, 1 for slave(conference) session.
 * @retval 0 Success
 */
int32 rtk_SetTranSessionID(uint32 chid, uint32 sid);

/**
 * @ingroup TAPI_MEDIA_MISC
 * @brief Get active session by channel number
 * @param chid The specified hannel number.
 * @param psid The session number. Usually 0 for master session, 1 for slave(conference) session.
 * @retval 0 Success
 */
int32 rtk_GetTranSessionID(uint32 chid, uint32* psid);

/**
 * @ingroup TAPI_DSP_TONE
 * @brief Play/Stop the assigned tone. 
 * @param chid The channel number.
 * @param sid The session number. Usually 0 for master session, 1 for slave(conference) session.
 * @param nTone The tone type, please refer to the \ref DSPCODEC_TONE "DSPCODEC_TONE" for more detail information.
 * @param bFlag The play flag <br>
 * 0: Stop <br> 1: Play 
 * @param Path The tone direction <br>
 * Please refer to \ref DSPCODEC_TONEDIRECTION
 * "DSPCODEC_TONEDIRECTION".
 * @retval 0 Success
 * @note The tone is various in different countries. 
 */ 
int32 rtk_SetPlayTone(uint32 chid, uint32 sid, DSPCODEC_TONE nTone, uint32 bFlag,
	DSPCODEC_TONEDIRECTION Path);

/**
 * @ingroup TAPI_DSP_TONE
 * @brief Set the tone, busy tone detection parametes and SLIC impedance according to the country.
 * @param voip_ptr The configuration of VoIP
 * @retval 0 Success
 * @note The parameter is depend on the flash layout.
 */
int32 rtk_SetCountry(voipCfgParam_t *voip_ptr);

/**
 * @ingroup TAPI_PHONE_FXS
 * @brief Set the SLIC impedance only according to the country.
 * @param country The selected country.
 * @note \ref rtk_SetCountryImpedance provides the 
 *       country-specific impedance setting (Not include tone).
 * @retval 0 Success
 */
int32 rtk_SetCountryImpedance(_COUNTRY_ country);

/**
 * @ingroup TAPI_DSP_TONE
 * @brief Set the based tone only according to the country.
 * @param country The selected country, please refer to \ref _COUNTRY_ "COUNTRY" setting.
 * @retval 0 Success 
 */
int32 rtk_SetCountryTone(_COUNTRY_ country);

/**
 * @ingroup TAPI_PHONE_FXS
 * @brief Set the SLIC impedance from the preset table.
 * @param preset The preset table index.
 * \htmlonly  <font color=#000000> <br> <br>
 * <table border=1 width=330 style="border-collapse:collapse;"borderColor=black >  
 * <td align=center><b>preset index</b></td> <td align=center><b>AC Impedance</b></td> <tr>  
 * <td align=center>0</td> <td align=center>600 ohm</td> <tr>  
 * <td align=center>1</td> <td align=center>900 ohm</td> <tr>  
 * <td align=center>2</td> <td align=center>250 ohm + (750 ohm  || 150nf)</td> <tr>  
 * <td align=center>3</td> <td align=center>320 ohm + (1150 ohm || 230nf)</td> <tr>  
 * <td align=center>4</td> <td align=center>350 ohm + (1000 ohm || 210nf)</td> <tr>  
 * </table>
 * </font> \endhtmlonly  
 * @note \ref rtk_SetImpedance provides five fixed AC impedance.
 * @retval 0 Success
 */
int32 rtk_SetImpedance(uint16 preset);

/**
 * @ingroup TAPI_DSP_TONE
 * @brief Set the disconnect tone detection parameters
 * @param voip_ptr The configuration of VoIP. 
 * @note The parameter is depend on the flash layout.
 * @retval 0 Success
 */
int32 rtk_SetDisTonePara(voipCfgParam_t *voip_ptr);	

/**
 * @ingroup TAPI_DSP_TONE
 * @brief Set the custom tone
 * @param custom The custom index.
 * @param pstToneCfgParam The custom tone configuration.
 *
 * A structure of the pstToneCfgParam please refer to @ref TstVoipToneCfg "st_ToneCfgParam". <br> 
 * @note Set the tone parameters when you are ready to create your own ringtones set.
 * @retval 0 Success
 */
int32 rtk_SetCustomTone(uint8 custom, st_ToneCfgParam *pstToneCfgParam);

/**
 * @ingroup TAPI_DSP_TONE
 * @brief Use the custom tone to set the dial, ringback, busy, and call waiting tone.
 * @param dial Customer dial tone 
 * @param ringback Customer ringing tone 
 * @param busy Customer busy tone 
 * @param waiting Customer call waiting tone 
 * @note Before perform \ref rtk_UseCustomTone, the customer 
 *       tone should have been set by rtk_SetCustomTone().
 * @retval 0 Success
 */  
int32 rtk_UseCustomTone(uint8 dial, uint8 ringback, uint8 busy, uint8 waiting);

/**
 * @ingroup TAPI_DSP_TONE
 * @brief Set the update tone
 * @param update_country The update country \ref DSPCODEC_COUNTRY.
 * @param update_tone The update tone \ref DSPCODEC_TONE.
 * @param pstToneCfgParam The update tone configuration.
 *
 * A structure of the pstToneCfgParam please refer to @ref TstVoipToneCfg "st_ToneCfgParam". <br> 
 * @note Set the tone parameters when you are ready to create your own tones set.
 * @retval 0 Success
 */
int32 rtk_SetUpdateTone(uint8 update_country, uint8 update_tone, st_ToneCfgParam *pstToneCfgParam);

/**
 * @ingroup TAPI_MEDIA_RTP
 * @brief Set RTP configuration
 *
 * Invoke rtk_SetRtpConfig() when creating a RTP session or modifying RTP parameters.
 * @param cfg The RTP configuration. 
 * @retval 0 Success
 */
int32 rtk_SetRtpConfig(rtp_config_t* cfg);

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief Set QoS configuration
 *
 * Invoke rtk_SetQoSConfig() after creating a RTP session or modifying RTP parameters.
 * @param chid The channel number.
 * @param sid The session number. Usually 0 for master session, 1 for slave(conference) session.
 * @param type The Session Type (session_type_rtp/session_type_rtcp/session_type_t38)
 * @param cfg The QoS configuration for dscpRemark vlanPriorityRemark quenuNum
 * @retval 0 Success
 */
int32 rtk_SetQoSConfig(int chid, int sid, SessionType type, TstQosCfg *cfg);

/**
 * @ingroup TAPI_MEDIA_T38
 * @brief Set T.38 UDP configuration 
 *
 * Invoke rtk_SetT38UdpConfig() when creating a T.38 session or modifying RTP parameters.
 * @param cfg The T.38 UDP configuration.
 * @retval 0 Success
 */
int32 rtk_SetT38UdpConfig(t38udp_config_t* cfg);

/**
 * @ingroup TAPI_MEDIA_RTP
 * @brief Set RTCP configuration 
 *
 * Invoke rtk_SetRtcpConfig() when creating a RTCP session or modifying RTCP parameters. 
 * @param cfg The RTCP configuration. 
 * @retval 0 Success
 */
int32 rtk_SetRtcpConfig(rtcp_config_t *cfg);

/**
 * @ingroup TAPI_MEDIA_RTP
 * @brief Get RTCP logger 
 * @param chid The channel number .
 * @param sid Session ID.
 * @param logger RTCP logger data.
 * @retval 0 Success
 */
int32 rtk_GetRtcpLogger( uint32 chid, uint32 sid, TstVoipRtcpLogger *logger );

/**
 * @ingroup TAPI_DSP_MEDIA
 * @brief Set RTP payload type 
 * @param cfg The RTP payload configuration.
 * @note Please refer to payloadtype_config_t for RTP payload 
 *    	type settings.
 * @retval 0 Success
 */
int32 rtk_SetRtpPayloadType(payloadtype_config_t* cfg);

/**
 * @ingroup TAPI_DSP_MEDIA
 * @brief Set T.38 to be UDP payload type.
 * @param cfg The T.38 parameters configuration.
 * @retval 0 Success
 */    
int32 rtk_SetT38PayloadType(t38_payloadtype_config_t *cfg);

/**
 * @ingroup TAPI_MEDIA_RTP
 * @brief Set RTP Session State 
 *
 * This API used to drop packets which generated by DSP or received from remote peer. 
 * If state is rtp_session_sendonly, RTP module continuely sends RTP to remote peer 
 * but drops packets which recieved from remote peer.<p> 
 * If state is rtp_session_recvonly, RTP module continuely receives packets from remote peer but drops RTP packets 
 * which generated from DSP. <br> 
 * If state is rtp_session_inactive, RTP module drops all packets from remote paeer 
 * and local DSP. This state inactive both RTP and RTCP sessions.<br> 
 * If state is rtp_session_sendrecv, RTP module forwards RTP packets to remote peer 
 * and receives RTP from remote peer. 
 * @param chid The channel number.
 * @param sid The session number. Usually 0 for master session, 1 for slave(conference) session.
 * @param state The RTP Session State.
 * @retval 0 Success
 */
int32 rtk_SetRtpSessionState(uint32 chid, uint32 sid, RtpSessionState state);

/**
 * @ingroup TAPI_MEDIA
 * @brief Set Sessions to be Inactive 
 *
 * This API used to inactive specified session, so all connections (RTP/RTCP/T.38)
 * will be released and codec will stop.  
 * @param chid The channel number.
 * @param sid The session number. Usually 0 for master session, 1 for slave(conference) session.
 * @retval 0 Success
 */
int32 rtk_SetSessionInactive( uint32 chid, uint32 sid );

/**
 * @ingroup TAPI_MEDIA_RTP
 * @brief Enable/Disable conference.
 *
 * This API used to enable or disable a 3-way conference session.
 * @param stVoipMgr3WayCfg The conference settings for two RTP sessions. 
 * @note DSP has ability to join different codec streams into a conference session.
 * @retval 0 Success
 */ 
int32 rtk_SetConference(TstVoipMgr3WayCfg *stVoipMgr3WayCfg);

/**
 * @ingroup TAPI_MEDIA_RTP
 * @brief Hold/Resume RTP
 * @param chid The channel number.
 * @param sid The session number. Usually 0 for master session, 1 for slave(conference) session.
 * @param enable Enable <br>
 * 0: Resume <br> 1: Hold
 * @retval 0 Success
 */
int32 rtk_HoldRtp(uint32 chid, uint32 sid, uint32 enable);

/**
 * @ingroup TAPI_MEDIA_RTP
 * @brief Get RTP/RTCP statistics
 * @param chid The channel number.
 * @param mid The media session ID.
 * @param bReset Reset partial statistics to zero, if bReset is set to 1. Some statistic will be auto reset when VoIP session is closed/opened.
 * @param pstRtpRtcpStatistics RTP/RTCP statistics. <br>
 * Provided statistics are: <br>
 *  - nTxPkts: Number of Tx RTP packets
 *  - nTxBytes: Number of Tx RTP bytes<br>
 *  - nRxPkts: Number of Rx RTP packets<br>
 *  - nRxBytes: Number of Rx RTP bytes<br>
 *  - nLost: Number of Rx packet loss <br>
 *  - nMaxFractionLost: Maximun Rx packet fraction loss (Follow RFC3550 A.3)<br>
 *  - nMinFractionLost: Minimun Rx packet fraction loss (Follow RFC3550 A.3)<br>
 *  - nAvgFractionLost: Average Rx packet fraction loss (Follow RFC3550 A.3)<br>
 *  - nCurFractionLost: Current Rx packet fraction loss (Follow RFC3550 A.3)<br>
 *  - nDiscarded: Number of Rx discarded RTP packets
 *  - nTxRtcpPkts: Number of Tx RTCP packets (including RTCP RX packet)
 *  - nRxRtcpPkts: Number of Rx RTCP packets (including RTCP RX packet)
 *  - nTxRtcpXrPkts: Number of Tx RTCP XR packets
 *  - nRxRtcpXrPkts: Number of Rx RTCP XR packets
 *  - nOverRuns: Number of times jitter buffer overruns
 *  - nUnderRuns: Number of times jitter buffer underruns
 *  - nMaxJitter: Maximun interarrival jitter estimate from RTP service (in ms)
 *  - nMinJitter: Minimun interarrival jitter estimate from RTP service (in ms)
 *  - nAvgJitter: Average interarrival jitter estimate from RTP service (in ms)
 *  - nCurJitter: Current interarrival jitter estimate from RTP service (in ms)
 *  - nMaxRtcpTime: Maximum time between RTCP packets (in ms)
 * @retval 0 Success
 */
int32 rtk_GetRtpRtcpStatistics( uint32 chid, uint32 mid, uint32 bReset, TstRtpRtcpStatistics *pstRtpRtcpStatistics );

/**
 * @ingroup TAPI_NETWORK_QOS
 * @brief Set TOS value of RTP packets in VoIP session. Usually defined for QoS.
 * @param rtp_tos TOS of RTP.
 *
 */
int32 rtk_SetRtpTos(int32 rtp_tos);

/**
 * @ingroup TAPI_NETWORK_QOS
 * @brief Set DSCP value of RTP packets in VoIP session. Usually defined for QoS.
 * @param rtp_dscp DSCP of RTP.
 */
int32 rtk_SetRtpDscp(int32 rtp_dscp);


/**
 * @ingroup TAPI_NETWORK_QOS
 * @brief Inform kernel the TOS value of SIP packets.
 * @param sip_tos TOS of SIP.
 */
int32 rtk_SetSipTos(int32 sip_tos);

/**
 * @ingroup TAPI_NETWORK_QOS
 * @brief Inform kernel the DSCP value of SIP packets.
 * @param sip_dscp DSCP of SIP.
 */
int32 rtk_SetSipDscp(int32 sip_dscp);

/**
 * @ingroup TAPI_NETWORK_QOS
 * @brief Set priority referenced by swCore for packets marked with special dscp value
 * @param dscp dscp value. 
 * @param priority priority referenced by swCore for dscp value <br>
 * Available range: 0 to 7 (7 is the highest priority).
 * @note Packet with higher priority will be first processed by swCore.
 * @retval 0 Success
 */
int32 rtk_QosSetDscpPriority(int32 dscp, int32 priority);

/**
 * @ingroup TAPI_NETWORK_QOS
 * @brief set priority of dscp value for packets to zero 
 * @retval 0 Success
 */
int32 rtk_qos_reset_dscp_priority(void);

/**
 * @ingroup TAPI_DSP_CID_DTMF
 * @brief Set the DTMF mode
 *
 * For DTMF mode, there are three types of DTMF signaling methods.
 * @param chid The channel number.
 * @param mode The DTMF mode. 
 * \htmlonly  <font color=#000000> <br> <br>
 * <table border=1 width=230 style="border-collapse:collapse;" borderColor=black > 
 * <td align=center><b>DTMF mode</b></td> <td align=center><b>Index</b></td> <tr>
 * <td align=left>DTMF_RFC2833 </td> <td align=center>0    </td> <tr>
 * <td align=left>DTMF_SIPINFO </td> <td align=center>1    </td> <tr>
 * <td align=left>DTMF_INBAND  </td> <td align=center>2    </td> <tr> 
 * </table> </font> \endhtmlonly 
 * @retval 0 Success
 */ 
int32 rtk_SetDtmfMode(uint32 chid, uint32 mode);

/**
 * @ingroup TAPI_MEDIA_RFC2833
 * @brief To set RFC2833 to be sent by AP or DSP
 * @param chid The channel number
 * @param config RFC2833 is sent by DSP or AP 
 * \htmlonly  <font color=#008040>  <br>
 * <table border=1 width=150 style="border-collapse:collapse;" borderColor=black >
 * <td align=center><b>Sent by</b></td> <td align=center><b>config</b></td> <tr>
 * <td align=center>DSP   </td> <td align=center>0    </td> <tr>
 * <td align=center>AP    </td> <td align=center>1    </td> <tr>
 * </table> </font> \endhtmlonly  
 * @retval 0 Success
 */
int32 rtk_SetRFC2833SendByAP(uint32 chid, uint32 config);

/**
 * @ingroup TAPI_MEDIA_RFC2833
 * @brief RFC2833 TX configuration
 * @param chid The channel number
 * @param tx_mode RFC2833 is sent by DSP or AP
 * @param volume_mode The RFC2833 volume mode(only for DTMF RFC2833, fax/modem RFC2833 is always mode 0).
 * @param volume The RFC2833 volume.
 * \htmlonly  <font color=#008040>  <br><br>
 * <table border=1 width=800 style="border-collapse:collapse;" borderColor=black > 
  * <td rowspan=3 align=center>tx_mode</td>
 * <td align=center><b>Mode</b></td> <td align=center><b>Value</b> </td> <td align=center><b>Description</b> </td><tr> 
 * <td align=center>DSP</td> <td align=center> 0</td> <td align=left> RFC2833 is sent by DSP automatically once DTMF key is detected, and finished util DTMF key is released</td><tr>
 * <td align=center>AP</td> <td align=center> 1</td> <td align=left> RFC2833 is sent by application. Developer need to assign the duration, volume of RFC2833 packets fisrt</td><tr>
 * <td rowspan=3 align=center>volume_mode</td>
 * <td align=center><b>Mode</b></td> <td align=center><b>Value</b> </td> <td align=center><b>Description</b> </td><tr> 
 * <td align=center>DSP</td> <td align=center> 1</td> <td align=left> Volume is detected and set by DSP for the DTMF/FAX/Modem tone</td><tr>
 * <td align=center>AP</td> <td align=center> 0</td> <td align=left> Volume is set by application</td><tr>
 * <td rowspan=3 align=center>volume</td>
 * <td align=center><b>Volume range</b></td> <td align=center>0 to 31 </td> <tr>
 * <td align=center><b>Unit</b></td> <td align=center>dBm </td> <tr>
 * <td align=center><b>Step</b></td> <td align=center> -1dBm</td> <tr> <tr>
 * </table> </font> \endhtmlonly
 * @note
 * \htmlonly  <font color=#008040>  <br>
 * <table border=1 width=900 style="border-collapse:collapse;" borderColor=black >
 * <td align=center><b></b></td> <td align=center><b>tx_mode = 0(by DSP)</b></td> <td align=center><b>tx_mode = 1(by AP)</b></td><tr>
 * <td align=center><b>volume_mode = 0 (by AP)</b></td> <td align=center>  The transmit RFC2833 volume will be set according to the application setting  </td> <td align=center>  The transmit RFC2833 volume will be set according to the application setting  </td><tr>
 * <td align=center><b>volume_mode = 1 (by DSP)</b></td> <td align=center> The transmit RFC2833 volume will be set according to the results of DTMF detection</td> <td align=center>  The transmit RFC2833 volume will be set according to the application setting  </td><tr>
 * </table> </font> \endhtmlonly  
 * @retval 0 Success
 */

int32 rtk_SetRFC2833TxConfig(uint32 chid, uint32 tx_mode, RFC2833_VOLUME_MODE volume_mode, RFC2833_VOLUME volume);

/**
 * @ingroup TAPI_MEDIA_RFC2833
 * @brief Send DTMF via RFC2833
 *
 * The event duration described in the RFC2833 Section 3.5 <br>
 * Duration: Duration of this digit, in timestamp units. Thus, the event began at the instant identified by the RTP timestamp <br>
 * and has so far lasted as long as indicated by this parameter. The event may or may not have ended. <br> <br>  
 * For a sampling rate of 8000 Hz, this field is sufficient to express event durations of up to approximately 8 seconds. <br>
 * @param chid The channel number.
 * @param sid The session number. Usually 0 for master session, 1 for slave(conference) session.
 * @param digit The digit of user input. (0..11, means 0..9,*,#)
 * \htmlonly  <font color=#008040>  <br>
 * <table border=1 width=200 style="border-collapse:collapse;" borderColor=black >
  <td align=center><b>Digit value</b></td> <td align=center><b>Symbol</b></td> <tr> 
  <td align=center>0</td> <td align=center> 0</td> <tr>
  <td align=center>1</td> <td align=center> 1</td> <tr>
  <td align=center>2</td> <td align=center> 2</td> <tr>
  <td align=center>3</td> <td align=center> 3</td> <tr>
  <td align=center>4</td> <td align=center> 4</td> <tr>
  <td align=center>5</td> <td align=center> 5</td> <tr>
  <td align=center>6</td> <td align=center> 6</td> <tr>
  <td align=center>7</td> <td align=center> 7</td> <tr>
  <td align=center>8</td> <td align=center> 8</td> <tr>
  <td align=center>9</td> <td align=center> 9</td> <tr>
  <td align=center>10</td> <td align=center> *</td> <tr>
  <td align=center>11</td> <td align=center> #</td> <tr>  
  </table> <br>
 * </font> \endhtmlonly  
 * @param duration The RFC2833 event duration (Unit:ms)
 * @retval 0 Success
 */
int32 rtk_SetRtpRfc2833(uint32 chid, uint32 sid, uint32 digit, unsigned int duration);

/**
 * @ingroup TAPI_MEDIA_RFC2833
 * @brief Limit the maximum duration of RFC2833 DTMF.
 * @param chid The channel number.
 * @param duration_in_ms The limited Max. DTMF duration (Unit:ms)
 * @param bEnable Max limitation for DTMF duration <br>
 * 0: Disable <br> 1: Enable 
 * @note rtk_LimitMaxRfc2833DtmfDuration() is used to limit the packet duration which is sent by RFC2833 and the
 * received DTMF playback duration. <br>
 * If you set to limit to 3 seconds, DSP will send RFC2833 packet 3 seconds if the DTMF keypad is pressed and hold more than 3 seconds. <br> 
 * And DSP only play 3 seconds of DTMF tone if the duration of received RFC2833 packet is more than 3 seconds.
 * @retval 0 Success
 */
int32 rtk_LimitMaxRfc2833DtmfDuration(uint32 chid, uint32 duration_in_ms, uint8 bEnable);

/**
 * @ingroup TAPI_MEDIA_RFC2833
 * @brief Get The received RFC2833 event.
 * @param chid The channel number.
 * @param mid  The media channel ID. Usually 0 for master session, 1 for slave(conference) session.
 * @param pent The received RFC2833 event. 
 *             Use \ref VEID_RFC2833_RX_MASK to obtain event coding in packet. 
 * @note pent = VEID_NONE means no event.
 * @retval 0 Success
 */
int32 rtk_GetRfc2833RxEvent(uint32 chid, uint32 mid, VoipEventID *pent);

/**
 * @ingroup TAPI_MEDIA_RFC2833
 * @brief FAX/Modem RFC2833 configuration
 * @param chid The channel number.
 * @param relay_flag The flag control to send the RFC2833 packets when DSP detects TDM Fax/Modem tone. <br>
 * 0: No send <br> 1: Send
 * @param removal_flag  The flag control to remove the inband RTP packet when Fax/Modem RFC2833 packets are sending. <br>
 * 0: No remove <br> 1: Remove 
 * @param tone_gen_flag The flag control to generate the Fax/Modem tone when receiving Fax/Modem RFC2833 packets. <br>
 * 0: No generate <br> 1: Generate
 */
int32 rtk_SetFaxModemRfc2833(uint32 chid, uint32 relay_flag, uint32 removal_flag, uint32 tone_gen_flag);

/**
 * @ingroup TAPI_MEDIA_RFC2833
 * @brief RFC2833 Packet Interval configuration
 * @param chid The channel number.
 * @param mid Media ID. Usually 0 for master session, 1 for slave(conference) session
 * @param type Type <br>
 * 0: DTMF RFC2833<br> 1: Fax/Modem RFC2833
 * @param interval Packet Interval (Unit: msec), must be multiple of 10 msec
 */
int32 rtk_SetRfc2833PacketInterval(uint32 chid, uint32 mid, uint32 type, uint32 interval);

/** 
 * @ingroup TAPI_DSP_TONE
 * @brief Play tone when receive SIP INFO
 * @param chid The channel number
 * @param sid The session number. Usually 0 for master session, 1 for slave (conference) session.
 * @param tone The tone need to play. This parameter need be configured before use.
 * @param duration The tone duration (Unit: ms) 
 * @retval 0 Success
 */
int32 rtk_SipInfoPlayTone(unsigned int chid, unsigned int sid, DSPCODEC_TONE tone, unsigned int duration);

/**
 * @ingroup TAPI_DSP_FAX
 * @brief Fax/Modem detection, report CED tone , ANSam tone, V.21 DIS event.
 * @param chid The channel number.
 * @param pval The translated fax state, see \ref FAX_MODEM_STATE. 
 * @param pevent The raw fax state. (0 means no event)
 * @param flush Flush <br>
 * 1: Flush the FIFO <br> 0: Do nothing
 * @retval 0 Success
 */    
int32 rtk_GetFaxModemEvent(uint32 chid, uint32 *pval, VoipEventID *pevent, uint32 flush);

/** 
 * @ingroup TAPI_DSP_FAX
 * @brief Fax/Modem detect configuration
 * @param chid The channel number.
 * @param mode The Fax/Modem detect mode.
 * \htmlonly  <font color=#000000> <br> <br>
 * <table border=1 width=200 style="border-collapse:collapse;" borderColor=black >
 * <td align=center><b>Detect mode</b></td> 
 * <td align=center><b>Index</b></td> <tr>
 * <td align=center>Auto-Hi-Speed-Fax </td> 
 * <td align=center>0</td> <tr> 
 * <td align=center>Fax</td>  <td align=center>1</td> <tr>
 * <td align=center>Modem   </td>  <td align=center>2</td> <tr>
 * <td align=center>Auto-Low-Speed-Fax   </td>
 * <td align=center>3</td> <tr> </table> </font> \endhtmlonly
 * @note the mode default is 0 
 * @retval 0 Success
 */  
int32 rtk_SetFaxModemDet(uint32 chid, uint32 mode);

/**
 * @ingroup TAPI_DSP_FAX
 * @brief Get Fax end detection result
 * @param chid The channel number.
 * @param *pval It will return the result of the fax end detection. <br>
 * 0: No Fax end <br> 1: Fax end detected
 * @retval 0 Success
 */
int32 rtk_GetFaxEndDetect(uint32 chid, uint32 *pval);

/**
 * @ingroup TAPI_DSP_FAX
 * @brief Get Fax DIS detection result in all path
 * @param chid The channel number.
 * @param *pval It will return the result of the DIS detection <br>
 * 0: No Fax DIS. <br> 1: Fax DIS detected 
 * @retval 0 Success
 */
int32 rtk_GetFaxDisDetect(uint32 chid, uint32 *pval);

/**
 * @ingroup TAPI_DSP_FAX
 * @brief Get Fax Dis TX event detection result
 * @param chid The channel number.
 * @param *pval It will return the result of DIS Tx detection, transmit path(IP/RTP/Remote). <br>
 * 0: No Fax DIS TX. <br> 1: Fax DIS TX detected
 * @retval 0 Success
 */
int32 rtk_GetFaxDisTxDetect(uint32 chid, uint32 *pval);

/**
 * @ingroup TAPI_DSP_FAX
 * @brief Get Fax Dis RX event detection result
 * @param chid The channel number.
 * @param *pval It will return the result of DIS RX detection, receive path(TDM/PCM/Local). <br>
 * 0: No Fax DIS RX. <br> 1: Fax DIS RX detected.
 * @retval 0 Success
 */
int32 rtk_GetFaxDisRxDetect(uint32 chid, uint32 *pval);

/**
 * @ingroup TAPI_PHONE_FXO
 * @brief Get FXO Line Voltage
 * @param chid The channel number.
 * @param *pval FXO Line Voltage
 * @retval 0 Success
 */
int32 rtk_GetFxoLineVoltage(uint32 chid, uint32 *pval);

/**
 * @ingroup TAPI_DSP_IVR
 * @brief Play a text speech
 * @param chid The channel number. 
 * @param sid The session number.
 * @param dir The playout directions.
 * @param pszText2Speech The text to speech.
 * @retval Playing interval in Unit of 10ms.
 */
int rtk_IvrStartPlaying( unsigned int chid, unsigned int sid, IvrPlayDir_t dir, char *pszText2Speech );

/**
 * @ingroup TAPI_DSP_IVR
 * @brief Play a G.723 6.3k voice
 * @param chid The channel number. 
 * @param sid The session number. 
 * @param dir The playout directions. 
 * @param nFrameCount The number of frame to be decoded.
 * @param pData Point to data.
 * @retval Copied frames, so less or equal to nFrameCount.
 */
int rtk_IvrStartPlayG72363( unsigned int chid, unsigned int sid, IvrPlayDir_t dir, unsigned int nFrameCount, const unsigned char *pData );

/**
 * @ingroup TAPI_DSP_IVR
 * @brief Poll whether it is playing or not.
 * @param chid The channel number. 
 * @param sid The session number.
 * @retval 1 Playing
 * @retval 0 Stopped
 */
int rtk_IvrPollPlaying( unsigned int chid, unsigned int sid );

/**
 * @ingroup TAPI_DSP_IVR
 * @brief Stop playing immediately.
 * @param chid The channel number. 
 * @param sid The session number.
 * @retval 0 Success
 */
int rtk_IvrStopPlaying( unsigned int chid, unsigned int sid );

/**
 * @ingroup TAPI_PHONE_MISC
 * @brief OffHook Action function. Call this TAPI always when Off-Hook event is got.
 * @param chid The channel number.
 * @retval 0 Success
 * @sa rtk_GetFxsEvent, rtk_GetFxoEvent <br>
 */
int32 rtk_OffHookAction(uint32 chid);

/**
 * @ingroup TAPI_PHONE_MISC
 * @brief OnHook Action function. Call this TAPI always when On-Hook event is got.
 * @param chid The channel number.
 * @retval 0 Success
 * @sa rtk_GetFxsEvent, rtk_GetFxoEvent
 */
int32 rtk_OnHookAction(uint32 chid);

/**
 * @ingroup TAPI_PHONE_PCM
 * @brief To disable or enable PCM channel as Narrowband/Wideband
 * @param chid The channel number 
 * @param val Config value
 * \htmlonly  <font color=#008040>  <br>
 * <table border=1 width=350 style="border-collapse:collapse;" borderColor=black >
 * <td align=center><b>Config value</b></td> <td align=center><b>Description</b></td> <tr>
 * <td align=center>0  </td> <td align=left>Disable PCM    </td> <tr>
 * <td align=center>1  </td> <td align=left>Enable PCM as Narrowband    </td> <tr>
 * <td align=center>2  </td> <td align=left>Enable PCM as Wideband    </td> <tr>
 * </table> </font> \endhtmlonly
 * @retval 0 Success
 */
int32 rtk_EnablePcm(uint32 chid, uint32 val);

/**
 * @ingroup TAPI_PHONE_PCM
 * @brief Set bus format 
 * @param chid The channel number 
 * @param format Bus data format, refer to @ref AP_BUS_DATA_FORMAT
 * @retval 0 Success
 */
int32 rtk_SetBusFormat(uint32 chid, AP_BUS_DATA_FORMAT format);

/**
 * @ingroup TAPI_PHONE_PCM
 * @brief Set PCM timeslot 
 * @param chid The channel number 
 * @param timeslot1 PCM timeslot 1 <br>
 *  Available range: 0 to 31. 
 * @param timeslot2 PCM timeslot 2 (Wideband channel only) <br>
 *  Available range: 0 to 31. 
 * @note
 * Some informations for setting timeslot1 and timeslot2 are listed in table below.<br>
 * Other slot limitations may depend on the specifications of SLIC/DAA or other PCM slave device.<br>
 * \htmlonly  <font color=#008040>  <br>
 * <table border=1 width=500 style="border-collapse:collapse;" borderColor=black >
 * <td align=center><b>Bus Format</b></td> <td align=center><b>Slot number required</b></td> <td align=center><b>Slot limitation</b></td> <tr>
 * <td align=center>NB Liner  </td> <td align=center>2  </td> <td align=left>Slot must set to even number </td><tr>
 * <td align=center>NB A-law  </td> <td align=center>1  </td> <td align=left>NO </td><tr>
 * <td align=center>NB A-law  </td> <td align=center>1  </td> <td align=left>NO </td><tr>
 * <td align=center>WB Liner  </td> <td align=center>4  </td> <td align=left>Slot must set to even number </td><tr>
 * <td align=center>WB A-law  </td> <td align=center>2  </td> <td align=left>NO </td><tr>
 * <td align=center>WB A-law  </td> <td align=center>2  </td> <td align=left>NO </td><tr>
 * </table> </font> \endhtmlonly
 * @retval 0 Success
 */
int32 rtk_SetPcmTimeslot(uint32 chid, uint32 timeslot1, uint32 timeslot2);

/**
 * @ingroup TAPI_DSP_DTMF_DET
 * @brief Enable/Disable DTMF detection
 *
 * The detection examines the energy of one of the two tones from an incoming   
 * signal at eight different DTMF frequencies to determine which DTMF frequency is present. <br> 
 * Since maximum energy corresponds to DTMF frequency, this procedure enables us to detect the DTMF frequency. <br>
 * @param chid The channel number  
 * @param bEnable
 * 0: Disable <br> 1: Enable
 * @param dir DTMF detection direction <br> 
 * 0: TDM <br> 1: IP <br> 2: For DTMF Echo Suppression Function
 * @retval 0 Success
 */
int32 rtk_SetDtmfCfg(uint32 chid, int32 bEnable, uint32 dir);

 /**
 * @ingroup TAPI_DSP_DTMF_DET
 * @brief Set DTMF detection parameters
 *
 * This TAPI can not enable/disable DTMF det. freq. offset support. You can set it by calling rtk_SetDtmfDetParamUpdate().
 * @param chid The channel number  
 * @param dir DTMF detection direction <br>
 * 0: TDM <br> 1: IP
 * @param threshold DTMF detection threshold <br>
 * Available range: 0 to 40 ( 0 ~ -40 dBm)<br> Unit: dBm 
 * @param on_time_10ms DTMF detection minimum on time,
 * on_time_10ms. <br> Unit: 10ms
 * @param fore_twist DTMF detection acceptable fore-twist threshold (unit: dB, for twist is positive value)
 * Available range: 1 to 12 <br> Unit: dB 
 * @param rev_twist DTMF detection acceptable rev-twist threshold (unit: dB, for twist is negative value)
 * Available range: 1 to 12 <br> Unit: dB
 * @retval 0 Success
 * @sa rtk_SetDtmfDetParamUpdate
 */
int32 rtk_SetDtmfDetParam(uint32 chid, uint32 dir, int32 threshold, uint32 on_time_10ms, uint32 fore_twist, uint32 rev_twist);

 /**
 * @ingroup TAPI_DSP_DTMF_DET
 * @brief Set DTMF detection parameters
 *
 * Invoke rtk_SetDtmfDetParamUpdate(), you can select which parameter(s) that you want to change/update.
 * For not update filed(s), you can ignore it(them).
 * @param stDtmfDetPara The DTMF detection parameters
 * @retval 0 Success
 * @sa rtk_SetDtmfDetParam
 */
int32 rtk_SetDtmfDetParamUpdate(TstDtmfDetPara* stDtmfDetPara);

/**
 * @ingroup TAPI_DSP_CID_DTMF
 * @brief Retrieve detected DTMF digits, energy, and duration
 * @param chid Channel ID
 * @param dir 0: TDM side, 1: IP side DTMF Event
 * @param *pEvent A DTMF digit. If digit is 'z', it means FIFO is empty.<br>
 * If digit is 'E', it means END of DTMF digit, and DTMF digit engegy and duration can be got.
 * @param *pEnergy The detected DTMF digit energy (unit dBFS range:0~-45)
 * @param *pDuration The detected DTMF digit duration (unit ms)
 * @retval 0 Success
 */
int32 rtk_GetDtmfEvent(uint32 chid, uint32 dir, int8 *pEvent, int8 *pEnergy, int16 *pDuration);

/**
 * @ingroup TAPI_PHONE_MISC
 * @brief Get hook status of IP Phone.
 * @param pHookStatus Hook status <br>
 * 0: Off-hook <br> 1: On-hook 
 */
extern int rtk_GetIPPhoneHookStatus( uint32 *pHookStatus );

/**
* @ingroup TAPI_MEDIA_MISC
* @brief Get the VoIP Middleware Resource status.
* @param chid The target channel open. (Help to find DSP chip)
* @param payload_type The RTP payoad type (0, 4, 8, 18...) for check resource
* @retval 1 Resource available
* @retval 0 Resource un-available
*/
unsigned short rtk_VoipResourceCheck(uint32 chid, int payload_type);

/**
 * @ingroup TAPI_PHONE_EVENT
 * @brief Retrieve DSP Event, such as tone end event, caller ID end event
 * @param chid Channel ID
 * @param mid Media ID
 * @param *pEvent The DSP Event.  
 * @param *pData The DSP Data. If event is '0', it means data is NOT meaningful.
 * @retval 0 Success
 */
int32 rtk_GetDspEvent(uint32 chid, uint32 mid, VoipEventID *pEvent, uint32 *pData);

/**
 * @ingroup TAPI_PHONE_STATE
 * @brief Get the FXS/FXO hook state (This TAPI will NOT read event FIFO)
 * @param pstVoipCfg
 * \htmlonly  <font color=#008040>  <br>
 * <table border=1 width=350 style="border-collapse:collapse;" borderColor=black >
 * <td align=center><b>pstVoipCfg</b></td> <td align=center><b>Description</b></td> <tr>
 * <td align=left>pstVoipCfg->ch_id (In) </td> <td align=left>FXS/FXO Channel number</td> <tr>
 * <td align=left>pstVoipCfg->cfg (Out) </td> <td align=left>Phone state result </td> <tr>
 * </table> </font> \endhtmlonly
 * @retval 0 Success
 * @sa rtk_GetFxsEvent, rtk_GetFxoEvent
 */
int32 rtk_GetPhoneState(TstVoipCfg* pstVoipCfg);

/**
 * @ingroup TAPI_PHONE_EVENT
 * @brief Flush the VoIP kernel used FIFO.
 * @param chid The channel number.
 * @retval 0 Success
 */
int rtk_SetFlushFifo(uint32 chid);

/**
 * @ingroup TAPI_PHONE_STATE
 * @brief Check FXS/FXO line state
 * @param chid The FXS/FXO channel number.
 * @retval For FXS <br>
 * \htmlonly  <font color=#008040>  <br>
 * <table border=1 width=750 style="border-collapse:collapse;" borderColor=black >
 * <td align=center><b>Return value</b></td> <td align=center><b>State</b></td> <td align=center><b>SLIC Supported</b></td><tr>
 * <td align=center>0 </td> <td align=left>Phone dis-connected </td> <td align=center>Silab </td><tr>
 * <td align=center>1 </td> <td align=left>Phone connected </td> <td align=center>Silab </td><tr>
 * <td align=center>2 </td> <td align=left>Phone off-hook </td> <td align=center>Silab/Zarlink </td><tr>
 * <td align=center>3 </td> <td align=left>Check time out ( may connect too many phone set => view as connect) </td> <td align=center>Silab </td><tr>
 * <td align=center>4 </td> <td align=left>Can not check, Linefeed should be set to active state first </td> <td align=center>Silab/Zarlink </td><tr>
 * </table> </font> \endhtmlonly
 * @retval For FXO  <br>
 * \htmlonly  <font color=#008040>  <br>
 * <table border=1 width=550 style="border-collapse:collapse;" borderColor=black >
 * <td align=center><b>Return value</b></td> <td align=center><b>State</b></td> <td align=center><b>SLIC Supported</b></td> <tr>
 * <td align=center>0 </td> <td align=left>PSTN Line connected </td> <td align=center>Silab/Zarlink </td><tr>
 * <td align=center>1 </td> <td align=left>PSTN Line dis-connected </td> <td align=center>Silab/Zarlink </td><tr>
 * <td align=center>2 </td> <td align=left>PSTN Line busy </td> <td align=center>Silab </td><tr>
 * </table> </font> \endhtmlonly
 * @note The FXS line state is not always reliable. It may depend on SLIC, PCB layout, length of RJ-11 line and number of Phone sets. It's just for reference. <br>
 * However, FXO line state is reliable.
 */
int rtk_LineCheck(uint32 chid);

/**
 * @ingroup TAPI_PHONE_FXO
 * @brief Let DAA off-hook
 *
 * @param chid The FXO channel number.
 * @retval 1 Success
 * @note This API is only used for real DAA
 */
int rtk_FxoOffHook(uint32 chid);

/**
 * @ingroup TAPI_PHONE_FXO
 * @brief Let FXO on-hook
 *
 * FXO line will be on-hook and set to OHT state. <br>
 * OHT: ON Hook Transmission. It is ready for caller id transmission.
 *
 * @param chid The FXO channel number.
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int rtk_FxoOnHook(uint32 chid);

/**
 * @ingroup TAPI_PHONE_FXO
 * @brief Let FXO Ringing
 * 
 * This api force kernel to report VEID_HOOK_FXO_RING_ON to user application. <br>
 * It is defined for a special case that needs low layer's event to trigger upper layer's state machine. <br>
 * This api is unnecessary for normal cases.
 *
 * @param chid The FXO channel number.
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int rtk_FxoRingOn(uint32 chid);

/**
 * @ingroup TAPI_PHONE_FXO
 * @brief FXO Busy
 *
 * This api force kernel to report VEID_HOOK_FXO_BUSY_TONE to user application. <br>
 * It is defined for a special case that needs low layer's event to trigger upper layer's state machine. <br>
 * This api is unnecessary for normal cases.
 *
 * @param chid The FXO channel number.
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int rtk_FxoBusy(uint32 chid);

/**
 * @ingroup TAPI_PHONE_DEBUG
 * @brief GPIO control
 * @param action Action <br>
 * 0: GPIO init <br> 1: Read <br> 2: Write
 * @param pid pid = (gpio_group << 16) | (gpio_pin & 0xFFFF)
 * @param value Value <br>
 * For GPIO Read/Write: <br>
 * Bit 0: Disable = 0, Enable = 1 <br>
 * For GPIO init: <br>
 * Bit 0-1: GPIO_PERIPHERAL <br>
 * Bit 2: GPIO_DIRECTION <br>
 * Bit 3: GPIO_INTERRUPT_TYPE
 * @param ret_value Read result
 * @retval 0 Success
 */
int rtk_GpioControl(unsigned long action, unsigned long pid, unsigned long value, unsigned long *ret_value);

/**
 * @ingroup TAPI_PHONE_MISC
 * @brief Set LED display mode 
 * @param chid Channel number 
 * @param LED_ID LED ID. Each channel has at most 2 LED, and we use 0 normally.
 * @param mode Display mode. Please refer to /ref 
 *             LedDisplayMode.
 * @retval 0 Success
 */
int rtk_SetLedDisplay( uint32 chid, uint32 LED_ID, LedDisplayMode mode );

/**
 * @ingroup TAPI_PHONE_MISC
 * @brief Get LED display status 
 * @param chid Channel number 
 * @param LED_ID LED ID. Each channel has at most 2 LED, and we use 0 normally.
 * @param mode Result of display mode. Please refer to /ref 
 *             LedDisplayMode.
 * @retval 0 Success
 */
int rtk_GetLEDStatus( uint32 chid, uint32 LED_ID, LedDisplayMode *mode );

/**
 * @ingroup TAPI_PHONE_DEBUG
 * @brief Set SLIC relay mode 
 * @param chid Channel number 
 * @param close A boolean value to indicate circuit between SLIC 
 *    			 and phone.<br>
 * 0: Open <br> 1: Close 
 * @retval 0 Success
 * @note Because kernel will switch relay to correct state,
 *       application doesn't need to control relay normally.  
 */
int rtk_SetSlicRelay( uint32 chid, uint32 close );

/**
 * @ingroup VOIP_MISC
 * @brief The variables contaions the VoIP Feature
 * @note call rtk_GetVoIPFeature first
 */
extern VoipFeature_t g_VoIP_Feature;

/**
 * @ingroup VOIP_MISC
 * @brief The variables retrieved from feature and to provide efficient access
 * @note call rtk_GetVoIPFeature first
 */
extern uint32 g_VoIP_Ports;

/**
 * @ingroup VOIP_MISC
 * @brief This variable is the fd of ioctl, and can be used to select() to wait 
 *        for events. 
 */
extern int g_VoIP_Mgr_FD;

/**
 * @ingroup TAPI_DSP_PULSEDIAL
 * @brief Set FXO dial mode
 * @param chid The FXO channel number.
 * @param mode  Set mode to enable/disable Pulse Dial for FXO<br>
 * \htmlonly  <font color=#008040>  <br>
 * <table border=1 width=150 style="border-collapse:collapse;" borderColor=black >
 * <td align=center><b>Mode</b></td> <td align=center><b>mode</b></td> <tr>
 * <td align=center>Disable  </td> <td align=center>0    </td> <tr>
 * <td align=center>Enable  </td> <td align=center>1    </td> <tr>
 * </table> </font> \endhtmlonly  
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int rtk_SetDailMode(uint32 chid, uint32 mode);

/**
 * @ingroup TAPI_DSP_PULSEDIAL
 * @brief Get FXO dial mode
 * @param chid The FXO channel number.
 * @retval 0 Pulse dial is disabled
 * @retval 1 Pulse dial is enabled
 * @note Virtual DAA not support
 */
int rtk_GetDailMode(uint32 chid);

 /**
 * @ingroup TAPI_DSP_PULSEDIAL
 * @brief Generate pulse dial for FXO
 * @param chid The FXO channel number.
 * @param digit The pulse dial digit (0~9)
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int rtk_GenPulseDial(uint32 chid, char digit);

 /**
 * @ingroup TAPI_DSP_PULSEDIAL
 * @brief Pulse dial generation config for FXO
 * @param pps The pulse dial gen speed(Pulse per second).
 * @param make_duration	Make duration of the pulse digit <br> Unit: 10 msec
 * @param interdigit_duration The pause time between the pulse digit <br> Unit: 10 msec
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int rtk_PulseDialGenCfg(char pps, short make_duration, short interdigit_duration);

/**
 * @ingroup TAPI_DSP_PULSEDIAL
 * @brief Set FXS pulse detection
 * @param chid The FXS channel number.
 * @param enable Enable flag. <br>  
 * 0: Disable Pulse detection for FXS <br> 1: Enable Pulse detection for FXS
 * @param pause_time The threshold of the pause duration of the adjacent pulse digit. <br> Unit: msec
 * @param min_break_ths The threshold of min break time. <br> Unit: msec
 * @param max_break_ths The threshold of max break time. <br> Unit: msec <br>
 * ex. If the pause duration of two pulse trains (digit 3 and digit 4) is smaller than PULSE_DIAL_PAUSE_TIME,
 * then the detection result will be digit 7 (3+4).
 * @retval 0 Success
 */
int rtk_SetPulseDigitDet(uint32 chid, uint32 enable, uint32 pause_time, uint32 min_break_ths, uint32 max_break_ths);

/**
 * @ingroup TAPI_PHONE_DEBUG
 * @brief Get Silicon Lab SLIC RAM value.
 * @param chid The SLIC channel number.
 * @param reg  SLIC RAM number.
 * @retval The value of the RAM.
 */
uint32 rtk_GetSlicRamVal(uint8 chid, uint16 reg);

/**
 * @ingroup TAPI_PHONE_DEBUG
 * @brief Set Silicon Lab SLIC RAM value.
 * @param chid The SLIC channel number.
 * @param reg  SLIC RAM number.
 * @param value The value which write to the RAM.
 * @retval 0 Success
 */
int rtk_SetSlicRamVal(uint8 chid, uint16 reg, uint32 value);

/**
 * @ingroup TAPI_PHONE_DEBUG
 * @brief Get SLIC(DAA) register value. (Supported for Silicon Lab and Zarlink)
 * @param chid The SLIC(DAA) channel number.
 * @param reg  SLIC(DAA) register number.
 * @param regdata The value of the SLIC(DAA) register.
 * @retval The length of the SLIC(DAA) register (unit: byte).
 */
uint32 rtk_GetSlicRegVal(uint32 chid, uint32 reg, uint8 *regdata);

/**
 * @ingroup TAPI_PHONE_DEBUG
 * @brief Set SLIC(DAA) register value. (Supported for Silicon Lab and Zarlink)
 * @param chid The SLIC(DAA) channel number.
 * @param reg  SLIC(DAA) register number.
 * @param len The length of SLIC(DAA) register (unit: byte).
 * @param regdata The value of the SLIC(DAA) register.
 * @retval 0 Success
 */
int rtk_SetSlicRegVal(int chid, int reg, int len, char *regdata);

/**
 * @ingroup TAPI_NETWORK_MISC
 * @brief Get port link status
 * @param pstatus Port link status holder.
 * @retval 0 Success
 */
int32 rtk_GetPortLinkStatus( uint32 *pstatus );

/**
 * @ingroup TAPI_PHONE_DEBUG
 * @brief Set PCM Loop Mode
 * @param group Group. <br>Channel is in gruop "group", if group
 *    			 <0, means not in pcm loop mode
 * @param mode Mode <br>
 * 0: Not loop mode <br> 1: Loop mode <br> 2: Loop mode with VoIP
 * @param main_ch Main channel  <br>  
 * If the mode is 2, main channel will create VoIP session.
 * @param mate_ch Mate channel <br>  
 * If the mode is 2, mate channel will NOT create VoIP session.
 * @retval 0 Success
 */
int rtk_SetPcmLoopMode(char group, char mode, char main_ch, char mate_ch);

/**
 * @ingroup TAPI_PHONE_MISC
 * @brief Set FXS/FXO into Loopback Mode (Hardtware Loopback)
 * @param chid The FXS channel number
 * @param enable  Enable flag <br> 
 * \htmlonly  <font color=#008040>  <br>
 * <table border=1 width=550 style="border-collapse:collapse;" borderColor=black >
 * <td align=center><b>Enable flag</b></td> <td align=center><b>Description</b></td> <td align=center><b>SLIC Supported</b></td> <tr>
 * <td align=center>0 </td> <td align=center>Disable FXS/FXO loopback </td> <td align=center>Silab SI32178 </td><tr>
 * <td align=center>1 </td> <td align=center>Enable FXS/FXO loopback </td> <td align=center>Silab SI32178 </td><tr>
 * </table> </font> \endhtmlonly
 * @retval 0 Success
 */
int rtk_SetFxsFxoLoopback(unsigned int chid, unsigned int enable);

/**
 * @ingroup TAPI_PHONE_FXS
 * @brief Set FXS On Hook Transmission and turn On PCM channel for data transfer
 * <br>
 * This API has two actions. <br>
 * 1. Set FXS line to on hook data transmission state. <br>
 * 2. Enabling pcm channel according to chid. <br>
 * <br>
 * Analogue on-hook data transmission is a technique which enables information 
 * to be transmitted to the called party from the exchange(FXS).  
 *
 * @param chid SLIC channel ID
 * @retval 0 Success
 */
int rtk_SetFxsOnHookTransPcmOn(unsigned int chid);

/**
 * @ingroup VOIP_DSP
 * @brief Set RTP PT Checker
 * @param enable Enable RTP PT checker <br> 
 * @param sync When RTP checker found PT is mis-matched, DSP docoder auto sync.
 * @param report When RTP checker found PT is mis-matched, report to user space
 * @param pkt_cnt_thres The mis-match PT packet count threshold for RTP checker 
 * @retval 0 Success
 */
int32 rtk_Set_RTP_PT_checker(uint32 chid, uint32 enable, uint32 sync, uint32 report, uint32 pkt_cnt_thres);

/**
 * @ingroup TAPI_PHONE_DEBUG
 * @brief Print message via kernel
 * @param level Debug level <br>
 * \htmlonly  <font color=#008040>  <br>
 * <table border=1 width=150 style="border-collapse:collapse;" borderColor=black >
 * <td align=center><b>Debug level</b></td> <td align=center><b>level</b></td> <tr>
 * <td align=center>Error   </td> <td align=center>0    </td> <tr>
 * <td align=center>Warning    </td> <td align=center>1    </td> <tr>
 * <td align=center>Info    </td> <td align=center>2    </td> <tr>
 * <td align=center>Trace    </td> <td align=center>3    </td> <tr>
 * </table> </font> \endhtmlonly  
 * @param module Module name
 * @param msg Debug message
 * @retval 0 Success
 */
int rtk_Print(int level, char *module, char *msg);
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
 * @retval DECT power. <br> 0: Active <br> 1: Inactive.
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
 * @retval DECT button event. 
 * @sa VEID_DECT_BUTTON_PAGE VEID_DECT_BUTTON_REGISTRATION_MODE
 * @sa VEID_DECT_BUTTON_DEL_HS VEID_DECT_BUTTON_NOT_DEFINED
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

/**
 * @ingroup TAPI_IPC
 * @brief Set DSP CPUID in EthernetDSP
 * @param cpuid DSP CPUID
 * @retval 0 Success
 */
int32 rtk_SetDspIdToDsp(unsigned char cpuid);

/**
 * @ingroup TAPI_IPC
 * @brief Set DSP PhyID to control
 * @param cpuid DSP CPUID
 * @retval 0 Success
 */
int32 rtk_SetDspPhyId(unsigned char cpuid);

/**
 * @ingroup TAPI_IPC
 * @brief Check if DSP is ready
 * @param cpuid DSP CPUID
 * @retval 0 Not ready 
 * @retval >= 0 Ready 
 * @retval < 0 Fail
 */
int32 rtk_CheckDspAllSoftwareReady(unsigned char cpuid);

/**
 * @ingroup TAPI_IPC
 * @brief Complete defer initialization
 * @retval 0 Success
 */
uint8 rtk_CompleteDeferInitialzation( void );

/**
 * @ingroup TAPI_PHONE_FXS
 * @brief Set the Silab Proslic parameters, such as Ring, DC Feed, Impedance, etc.
 * @param chid The FXS channel number.
 * @param slic_type The SLIC type, ex: Si3226x, Si3217x. See \ref ProslicType.
 * @param param_type The parameter type, ex: Ring, Impedance. See \ref ProslicParamType.
 * @param pParam The Proslic paramter pointer.
 * @param param_size The paramters size in byte.
 * @note Currently, it only supports SLIC type Si3226x, and paramter type: Ring, DC Feed, and impedance.
 * @retval 0 Success
 */
int32 rtkSetProslicParam(uint32 chid, ProslicType slic_type, ProslicParamType param_type, void* pParam, uint32 param_size);

/**
 * @ingroup TAPI_MEDIA_RFC2833
 * @brief To enable transmit or receive RFC2833 trunk event
 * @param chid The channel number
 * @param bEnable It is present that 0 is diable, 1 is enable
 * @retval 0 Success
 */
int32 rtk_EnableABCDSignal(uint32 chid, int bEnable);

/**
 * @ingroup TAPI_MEDIA_RFC2833
 * @brief To generate RFC2833 ABCD signal
 * @param chid The channel number
 * @param signal The ABCD signal event
 * @retval 0 Success
 */
int32 rtk_GenerateABCDSignal (uint32 chid, unsigned char signal);


/**
 * @ingroup TAPI_PHONE_FXS
 * @brief To generate Metering Pulse signal
 * @param chid The channel number
 * @param hz Choose 12K or 16K, Please input 12 or 16
 * @param onTime Pulse on time in 10ms increments
 * @param offTime Pulse off time in 10ms increments
 * @param numMeters Number of meter cycles to perform
 * @retval 0 Success
 */
int32 rtk_GenMeteringPulse(uint32 chid, uint16 hz, uint16 onTime, uint16 offTime, uint16 numMeters);

/**
 * @ingroup TAPI_PHONE_FXS
 * @brief To get port status
 * @param chid The channel number
 * @param isPlugged R1 is plugged 
 * @retval 0 Success
 */
int32 rtk_GetPortStatus(uint32 chid);

/**
 * @ingroup TAPI_PHONE_FXS
 * @brief To get port status
 * @param chid The channel number
 * @param roh The ROH result
 * @retval 0 Success
 */
int32 rtk_GetLineRoh(uint32 chid, TstVoipSLICLineRoh *roh);

/**
 * @ingroup TAPI_PHONE_FXS
 * @brief To get AC/DC voltage
 * @param chid The channel number
 * @param voltage The AC/DC voltage value
 * @retval 0 Success
 */
int32 rtk_GetLineVoltage(uint32 chid, TstVoipSLICLineVoltage *voltage);

/**
 * @ingroup TAPI_PHONE_FXS
 * @brief To get capacitance value
 * @param chid The channel number
 * @param cap The result from CAP testing
 * @retval 0 Success
 */
int32 rtk_GetLineCap(uint32 chid, TstVoipSLICLineCap *cap);

/**
 * @ingroup TAPI_PHONE_FXS
 * @brief To get loop current value
 * @param chid The channel number
 * @param loopcurrent The result from loop current testing
 * @retval 0 Success
 */
int32 rtk_GetLineLoopCurrent(uint32 chid, TstVoipSLICLineLoopCurrent *loopcurrent);

/**
 * @ingroup TAPI_PHONE_FXS
 * @brief To get ResFlt result
 * @param chid The channel number
 * @param Res The result from ResFLT testing
 * @retval 0 Success
 */
int32 rtk_GetLineResflt(uint32 chid, TstVoipSLICLineResFlt *flt);

/**
 * @ingroup TAPI_PHONE_FXS
 * @brief To get Calibration Coefficients
 * @param ch_id The channel number
 * @param coeff The 52 bytes coefficients
 * @param result
 * @retval 0 Success
 */
int32 rtk_GetCalibrationCoeff(uint32 chid, TstVoipCalibrationCoeff *stVoipCalibrationCoeff );

/**
 * @ingroup TAPI_PHONE_FXS
 * @brief To set Calibration Coefficients
 * @param ch_id The channel number
 * @param coeff The 52 bytes coefficients
 * @param result This function wouldn't return this.
 * @retval 0 Success
 */
int32 rtk_SetCalibrationCoeff(uint32 chid, TstVoipCalibrationCoeff *stVoipCalibrationCoeff );

/**
 * @ingroup TAPI_PHONE_FXS
 * @brief To do Calibration
 * @param ch_id The channel number
 * @param mode 1:Call VpCalLine, 0:Call VpCal
 * @param result
 * @retval 0 Success
 */
int32 rtk_DoCalibration(uint32 chid, TstVoipDoCalibration *stVoipDoCalibration);

#endif // __VOIP_MANAGER_H

