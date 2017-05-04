/**
 * @file voip_control.h
 * @brief VoIP control structure
 */

#ifndef VOIP_CONTROL_H
#define VOIP_CONTROL_H

#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
#ifdef __KERNEL__
#include <linux/in6.h>
#endif
#endif

#include "voip_params.h"

#define VOIP_MGR_BASE_CTL				64+1024+64+64+900 //2116

#define VOIP_MGR_IOC_CMD( nr, size )	( ( nr & 0xFFFF ) | ( ( size & 0xFFFF ) << 16 ) )
#define VOIP_MGR_IOC_NR( cmd )			( ( cmd & 0xFFFF ) )
#define VOIP_MGR_IOC_SIZE( cmd )		( ( cmd >> 16 ) & 0xFFFF )

//! VoIP Control ID @ingroup VOIP_CONTROL
//!@{
enum {
	// testing function
	VOIP_MGR_SET_EBL = VOIP_MGR_BASE_CTL+1,
	VOIP_MGR_INIT_GNET,
	VOIP_MGR_INIT_GNET2,
	VOIP_MGR_LOOPBACK,
	VOIP_MGR_GNET,
	VOIP_MGR_SIGNALTEST,
	VOIP_MGR_DSPSETCONFIG,
	VOIP_MGR_DSPCODECSTART,				//2124

	// Protocol - RTP
	//! @ingroup VOIP_PROTOCOL_RTP
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_SET_SESSION,
	VOIP_MGR_UNSET_SESSION,
	VOIP_MGR_SETRTPSESSIONSTATE,
	VOIP_MGR_RTP_CFG,
	VOIP_MGR_HOLD,
	VOIP_MGR_CTRL_RTPSESSION,			//2130
	VOIP_MGR_CTRL_TRANSESSION_ID,
	VOIP_MGR_SETCONFERENCE,
	VOIP_MGR_GET_RTP_RTCP_STATISTICS,
	VOIP_MGR_GET_SESSION_STATISTICS,	//2134

	// Protocol - RTCP
	//! @ingroup VOIP_PROTOCOL_RTCP
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_SET_RTCP_SESSION, 
	VOIP_MGR_UNSET_RTCP_SESSION,		//2136 
	//VOIP_MGR_SET_RTCP_TX_INTERVAL, 	// merge into VOIP_MGR_SET_RTCP_SESSION
	VOIP_MGR_GET_RTCP_LOGGER, 

	// DSP - General
	//! @ingroup VOIP_DSP_GENERAL
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_ON_HOOK_RE_INIT,
	VOIP_MGR_SET_VOICE_GAIN,
	VOIP_MGR_ENERGY_DETECT,
	VOIP_MGR_GET_VOIP_EVENT, 			//2141
	VOIP_MGR_FLUSH_VOIP_EVENT, 

	// DSP - Codec
	//! @ingroup VOIP_DSP_CODEC
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_SETRTPPAYLOADTYPE,
	VOIP_MGR_DSPCODECSTOP,				//2144

	// DSP - FAX and Modem
	//! @ingroup VOIP_DSP_FAXMODEM
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_FAX_OFFHOOK,
	VOIP_MGR_FAX_END_DETECT,
	VOIP_MGR_SET_FAX_MODEM_DET,
	VOIP_MGR_FAX_DIS_DETECT,
	VOIP_MGR_FAX_DIS_TX_DETECT,
	VOIP_MGR_FAX_DIS_RX_DETECT,
	VOIP_MGR_FAX_DCN_DETECT,
	VOIP_MGR_FAX_DCN_TX_DETECT,
	VOIP_MGR_FAX_DCN_RX_DETECT,
	VOIP_MGR_SET_ANSWERTONE_DET,
	VOIP_MGR_SET_FAX_SILENCE_DET,		//2155

	// DSP - LEC
	//! @ingroup VOIP_DSP_LEC
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_SET_ECHO_TAIL_LENGTH,
	VOIP_MGR_SET_G168_LEC_CFG,
	VOIP_MGR_SET_VBD_EC,
	VOIP_MGR_GET_EC_DEBUG, //add by jwsyu, echo canceller debug

	// DSP - DTMF
	//! @ingroup VOIP_DSP_DTMF
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_DTMF_DET_PARAM,			//2160
	VOIP_MGR_DTMF_CFG,
	VOIP_MGR_SET_DTMF_MODE,
	VOIP_MGR_SEND_RFC2833_PKT_CFG,
	VOIP_MGR_SEND_RFC2833_BY_AP,
	VOIP_MGR_PLAY_SIP_INFO,
	VOIP_MGR_LIMIT_MAX_RFC2833_DTMF_DURATION,
	VOIP_MGR_SET_RFC2833_TX_VOLUME,		//2167

	// DSP - Caller ID
	//! @ingroup VOIP_DSP_CALLERID
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_DTMF_CID_GEN_CFG,
	VOIP_MGR_GET_CID_STATE_CFG,
	VOIP_MGR_FSK_CID_GEN_CFG,
	VOIP_MGR_SET_FSK_VMWI_STATE,
	VOIP_MGR_SET_FSK_AREA,
	VOIP_MGR_FSK_ALERT_GEN_CFG,
	VOIP_MGR_SET_CID_DTMF_MODE,
	VOIP_MGR_SET_CID_DET_MODE,
	VOIP_MGR_GET_FSK_CID_STATE_CFG,
	VOIP_MGR_SET_CID_FSK_GEN_MODE,
	VOIP_MGR_FSK_CID_VMWI_GEN_CFG,
	VOIP_MGR_SET_FSK_CLID_PARA,
	VOIP_MGR_GET_FSK_CLID_PARA,
	VOIP_MGR_FSK_CID_MDMF_GEN,			//2181

	// DSP - Tone
	//! @ingroup VOIP_DSP_TONE
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_SETPLAYTONE,
	VOIP_MGR_SET_COUNTRY,
	VOIP_MGR_SET_COUNTRY_IMPEDANCE,
	VOIP_MGR_SET_COUNTRY_TONE,
	VOIP_MGR_SET_TONE_OF_CUSTOMIZE,
	VOIP_MGR_SET_CUST_TONE_PARAM,
	VOIP_MGR_USE_CUST_TONE,
	VOIP_MGR_SET_DIS_TONE_DET,			//2189
	VOIP_MGR_SET_TONE_OF_UPDATE,
	VOIP_MGR_SET_UPDATE_TONE_PARAM,

	// DSP - AGC
	//! @ingroup VOIP_DSP_AGC
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_SET_SPK_AGC,
	VOIP_MGR_SET_SPK_AGC_LVL,
	VOIP_MGR_SET_SPK_AGC_GUP,
	VOIP_MGR_SET_SPK_AGC_GDOWN,
	VOIP_MGR_SET_MIC_AGC,
	VOIP_MGR_SET_MIC_AGC_LVL,
	VOIP_MGR_SET_MIC_AGC_GUP,
	VOIP_MGR_SET_MIC_AGC_GDOWN,			//2197

	// DSP - Pluse Dial
	//! @ingroup VOIP_DSP_PLUSEDIAL
	//! @ingroup VOIP_CONTROL
	//TH: for pulse dial config
	VOIP_MGR_SET_PULSE_DIGIT_DET,
	VOIP_MGR_SET_DIAL_MODE,
	VOIP_MGR_GET_DIAL_MODE,				//2200
	VOIP_MGR_PULSE_DIAL_GEN_CFG,
	VOIP_MGR_GEN_PULSE_DIAL,

	// DSP - IVR
	//! @ingroup VOIP_DSP_IVR
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_PLAY_IVR,
	VOIP_MGR_POLL_IVR,
	VOIP_MGR_STOP_IVR,

	// Driver - PCM ctrl
	//! @ingroup VOIP_DRIVER_PCM
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_PCM_CFG,				//2206
	VOIP_MGR_SET_BUS_DATA_FORMAT,
	VOIP_MGR_SET_PCM_TIMESLOT,
	VOIP_MGR_SET_PCM_LOOP_MODE,

	// Driver - SLIC
	//! @ingroup VOIP_DRIVER_SLIC
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_SLIC_RING,
	VOIP_MGR_SLIC_TONE,
	VOIP_MGR_SLIC_RESTART,
	VOIP_MGR_GET_SLIC_REG_VAL,
	VOIP_MGR_SET_SLIC_TX_GAIN,
	VOIP_MGR_SET_SLIC_RX_GAIN,
	VOIP_MGR_SET_FLASH_HOOK_TIME,
	VOIP_MGR_SET_SLIC_RING_CADENCE,
	VOIP_MGR_SET_SLIC_RING_FRQ_AMP,
	VOIP_MGR_SET_IMPEDANCE,
	VOIP_MGR_SET_SLIC_REG_VAL,			//2220
	VOIP_MGR_GET_SLIC_STAT,
	VOIP_MGR_SLIC_ONHOOK_ACTION, 	// only used in AudioCodes
	VOIP_MGR_SLIC_OFFHOOK_ACTION, 	// only used in AudioCodes
	VOIP_MGR_LINE_CHECK,
	VOIP_MGR_HOOK_FIFO_IN,
	VOIP_MGR_GET_SLIC_RAM_VAL,
	VOIP_MGR_SET_SLIC_RAM_VAL,

	VOIP_MGR_SET_RING_DETECTION,
	VOIP_MGR_SET_FXS_FXO_LOOPBACK,
	VOIP_MGR_SET_SLIC_ONHOOK_TRANS_PCM_START,	//2230

	VOIP_MGR_SET_SLIC_LINE_VOLTAGE,
	VOIP_MGR_GEN_SLIC_CPC,

	// Driver - DAA
	//! @ingroup VOIP_DRIVER_DAA
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_DAA_RING,
	VOIP_MGR_DAA_OFF_HOOK,
	VOIP_MGR_DAA_ON_HOOK,
	VOIP_MGR_GET_DAA_LINE_VOLTAGE,
	VOIP_MGR_SET_DAA_TX_GAIN,
	VOIP_MGR_SET_DAA_RX_GAIN,
	VOIP_MGR_SET_DAA_ISR_FLOW,
	VOIP_MGR_GET_DAA_ISR_FLOW,			//2240
	VOIP_MGR_SET_DAA_PCM_HOLD_CFG,
	VOIP_MGR_GET_DAA_BUSY_TONE_STATUS,
	VOIP_MGR_GET_DAA_CALLER_ID,
	VOIP_MGR_GET_DAA_USED_BY_WHICH_SLIC,
	VOIP_MGR_FXO_ON_HOOK,
	VOIP_MGR_FXO_OFF_HOOK,

	// Driver - GPIO
	//! @ingroup VOIP_DRIVER_GPIO
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_GPIO,
	VOIP_MGR_SET_LED_DISPLAY,
	VOIP_MGR_SET_SLIC_RELAY,

	// Driver - Networking
	//! @ingroup VOIP_DRIVER_NETWORK
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_SET_DSCP_PRIORITY,			//2250
	VOIP_MGR_SET_RTP_TOS,				//2251
	VOIP_MGR_SET_RTP_DSCP,
	VOIP_MGR_SET_SIP_TOS,
	VOIP_MGR_SET_SIP_DSCP,
	VOIP_MGR_GET_PORT_LINK_STATUS,

	// Driver - DECT
	//! @ingroup VOIP_DRIVER_DECT
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_DECT_SET_POWER,
	VOIP_MGR_DECT_GET_POWER,
	VOIP_MGR_DECT_GET_PAGE,
	VOIP_MGR_DECT_SET_LED,

	// Miscellanous
	//! @ingroup VOIP_MISC
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_SIP_REGISTER,	//2260
	VOIP_MGR_GET_FEATURE,
	VOIP_MGR_VOIP_RESOURCE_CHECK,
	VOIP_MGR_SET_FW_UPDATE,

	// IP Phone - keypad, LCM, Codec, LED and etc
	//! @ingroup VOIP_IPPHONE
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_CTL_KEYPAD,
	VOIP_MGR_CTL_LCM,
	VOIP_MGR_CTL_VOICE_PATH,
	VOIP_MGR_CTL_LED,
	VOIP_MGR_CTL_MISC,

	// Debug
	//! @ingroup VOIP_DEBUG
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_DEBUG,
	VOIP_MGR_VOICE_PLAY,	//2270
	VOIP_MGR_GET_T38_PCMIN,
	VOIP_MGR_GET_T38_PACKETIN,
	VOIP_MGR_SET_GETDATA_MODE,
	VOIP_MGR_IPHONE_TEST,
	VOIP_MGR_PRINT,
	VOIP_MGR_COP3_CONIFG,

	// Ethernet DSP
	//! @ingroup VOIP_ETHERNET_DSP
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_SET_DSP_ID_TO_DSP,
	VOIP_MGR_SET_DSP_PHY_ID,
	VOIP_MGR_CHECK_DSP_ALL_SW_READY,
	VOIP_MGR_COMPLETE_DEFER_INIT,//2280

	// DSP - Codec
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_SET_VAD_CNG_THRESHOLD,

	// DSP - DTMF
	//! @ingroup VOIP_DSP_DTMF
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_FAX_MODEM_RFC2833_CFG,
	VOIP_MGR_RFC2833_PKT_INTERVAL_CFG,
	
	// DSP - Caller ID  
	//! @ingroup VOIP_DSP_CALLERID
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_STOP_CID,
	
	// Driver - SLIC  
	//! @ingroup VOIP_DRIVER_SLIC
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_SET_MULTI_RING_CADENCE,
	VOIP_MGR_SET_RTP_PT_CHECKER,
	
	// Driver - DAA  
	//! @ingroup VOIP_DRIVER_DAA
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_SET_DAA_HYBRID, 
	VOIP_MGR_SET_FXO_TUNE,
	
	// Driver - SLIC  
	//! @ingroup VOIP_DRIVER_SLIC
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_SET_PROSLIC_PARAM_STEP1,
	VOIP_MGR_SET_PROSLIC_PARAM_STEP2,//2290
	
	// Driver - Networking  
	//! @addtogroup VOIP_DRIVER_NETWORK
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_QOS_CFG,

	// DSP - DTMF
	//! @ingroup VOIP_DSP_DTMF
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_SET_RFC2833_ENABLE_ABCDSIGNAL,
	VOIP_MGR_SET_RFC2833_GENERATE_ABCDSIGNAL,

	// Driver - SLIC
	//! @addtogroup VOIP_DRIVER_SLIC
	//! @ingroup VOIP_CONTROL
	VOIP_MGR_SET_MERTEPULSE,
	VOIP_MGR_GET_PORTDETECT,
	VOIP_MGR_GET_LINEROH,
	VOIP_MGR_GET_LINEVOLTAGE,
	VOIP_MGR_GET_LINERESFLT,
	VOIP_MGR_SET_OFF_HOOK_TIME,

	// Driver - GPIO
	//! @ingroup VOIP_DRIVER_GPIO
	//! @ingroup VOIP_CONTROL	
	VOIP_MGR_GET_LED_STATUS,//2300
	VOIP_MGR_SET_CALIBRATION_COEFFICIENTS,
	VOIP_MGR_GET_CALIBRATION_COEFFICIENTS,
	VOIP_MGR_DO_CALIBRATION,
	VOIP_MGR_GET_LINECAP,
	VOIP_MGR_GET_LINELOOPCURRENT,
	VOIP_MGR_SET_MAX, 	// keep it to be the last one 
};
//!@}

//enum {
//	VOIP_MGR_GET_EBL = VOIP_MGR_BASE_CTL+1,
//	VOIP_MGR_GET_SESSION,
//	VOIP_MGR_GET_MAX, 	// keep it to be the last one
//};

#ifdef CONFIG_RTK_VOIP_SRTP
#define SRTP_KEY_LEN 30
#endif

/**
 * @brief This structure is a member of three way conference
 */
typedef struct
{
	// RTP session info
	uint8 ch_id;				///< Channel ID
	int32 m_id;				///< Media ID
	uint32 ip_src_addr;			///< Source IP address 
	uint32 ip_dst_addr;			///< Destination port 
	uint16 udp_src_port;			///< Source port 
	uint16 udp_dst_port;			///< Destination port 
	uint16 rtcp_src_port;			///< RTCP source port
	uint16 rtcp_dst_port;			///< RTCP destination port
	uint8 protocol;				///< IP protocol number 
	uint8 tos;				///< TOS for IP header
	uint16 rfc2833_payload_type_local;	///< Local payload type of RFC2833
	uint16 rfc2833_payload_type_remote;	///< Remote payload type of RFC2833
	//Payload info
	RtpPayloadType local_pt; 		///< Local payload type
	RtpPayloadType remote_pt; 		///< Remote answer payload type
	//RtpPayloadType uPktFormat; 		///< RTK_VoIP predefined payload type
	RtpPayloadType uLocalPktFormat; 		///< RTK_VoIP predefined local payload type
	RtpPayloadType uRemotePktFormat; 		///< RTK_VoIP predefined remote payload type
	RtpPayloadType local_pt_vbd; 		///< VBD Local payload type
	RtpPayloadType remote_pt_vbd; 		///< VBD Remote answer payload type
	RtpPayloadType uPktFormat_vbd; 		///< VBD RTK_VoIP predefined payload type
	int32 nG723Type;				///< G.723 bitrate
	//int32 nFramePerPacket;			///<  Frames per packet 
	int32 nLocalFramePerPacket;			///<  Local Frames per packet 
	int32 nRemoteFramePerPacket;		///<  Remote Frames per packet 
	int32 nFramePerPacket_vbd;		///<  Frames per packet for VBD
	int32 bVAD;				///< VAD is on or off 
	int32 bPLC;				///< PLC is on or off
	uint32 nJitterDelay;			///< Jitter buffer delay 
	uint32 nMaxDelay;			///< Maximum jitter buffer delay 
	uint32 nMaxStrictDelay;			///< Maximum strict jitter buffer delay (10ms)
	uint32 nJitterFactor;			///< fitter buffer factor 
	// RTP session state
	uint8	state;				///< RTP session state 
	int32 result;				///< The result 
	//int32	ret_val;

#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
	struct in6_addr ip6_src_addr;		///< Source IPv6 address 
	struct in6_addr ip6_dst_addr;		///< Destination IPv6 address
	uint32 ipv6;				///< IPv6 flag, 1: IPv6, 0: IPv4
#endif
}
TstVoipMgrRtpCfg;

/**
 * @brief Structure for three way conference
 * @see do_mgr_VOIP_MGR_SETCONFERENCE()
 */
typedef struct
{
	uint8 ch_id;				///< Channel ID
	uint8 enable;				///< Enable or disable three way conference 
	TstVoipMgrRtpCfg rtp_cfg[2];		///< onference these two sessions 	
	//int32 ret_val;
}
TstVoipMgr3WayCfg;

/**
 * @brief Structure for RTP session 
 * @note When RTP redudant payload type is defined, it fills RTP 
 *       header's payload type field. 
 *       Original set payload types of @ref TstVoipMgrRtpCfg are used
 *       to fill RTP redundant extension header. 
 * @note SID payload type is used by codec whose timestamp clock 
 *       rate is not equal to 8000Hz, such as G.722. 
 *       Its range should be between 96 to 127, and just set it to
 *       be 0 in narrowband codec. 
 * @see do_mgr_VOIP_MGR_SET_SESSION()
 */
typedef struct
{
	uint32 ip_src_addr;								///< Source IP address 
	uint32 ip_dst_addr;                             ///< Destination IP address
	uint16 udp_src_port;                            ///< UDP Source port 
	uint16 udp_dst_port;                            ///< UDP Destination port 
	uint8 protocol;                                 ///< IP protocol number
	uint8 ch_id;                                    ///< Channel ID
	int32 m_id;                                     ///< Media ID
	int32 result;                                   ///< The result of RTP trap register 
	uint8 tos;                                      ///< TOS for IP header 
	uint16 rfc2833_dtmf_pt_local;                   ///< Local payload type of RFC2833
	uint16 rfc2833_dtmf_pt_remote;                  ///< Remote payload type of RFC2833
	uint16 rfc2833_fax_modem_pt_local;              ///< Local payload type of Fax/Modem
	uint16 rfc2833_fax_modem_pt_remote;             ///< Remote payload type of Fax/Modem
#ifdef CONFIG_RTK_VOIP_SRTP
	unsigned char remoteSrtpKey[SRTP_KEY_LEN];	///< Remote SRTP key
	unsigned char localSrtpKey[SRTP_KEY_LEN];	///< Local SRTP key
	int remoteCryptAlg;				///< Remote crypto-algorithm 
	int localCryptAlg;                              ///< Local crypto-algorithm
#endif /*CONFIG_RTK_VOIP_SRTP*/
#ifdef SUPPORT_RTP_REDUNDANT
	uint16 rtp_redundant_payload_type_local;        ///< Local payload type of RTP redundant.
	uint16 rtp_redundant_payload_type_remote;       ///< Remote payload type of RTP redundant 
	uint8  rtp_redundant_max_Audio;                 ///< Maximum RTP redundancy number in TX direction (0~2)
	uint8  rtp_redundant_max_RFC2833;               ///< Maximum RFC 2833 redundancy number in TX direction (0~5)
#endif
	uint16 SID_payload_type_local;                  ///< Local payload type of SID
	uint16 SID_payload_type_remote;                 ///< Remoate payload type of SID 
	uint16 init_randomly;                           ///< Initialize seqno, SSRC and timestamp randomly, so below three parameters are ignored
	uint16 init_seqno;                              ///< Initial seqno
	uint32 init_SSRC;                               ///< SRC
	uint32 init_timestamp;							///< Initial timestamp

#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
	struct in6_addr ip6_src_addr;					///< Source IPv6 address 
	struct in6_addr ip6_dst_addr;					///< Destination IPv6 address
	uint32 ipv6;									///< IPv6 flag, 1: IPv6, 0: IPv4
#endif
	//int32	ret_val;
}
TstVoipMgrSession;

#ifdef PCM_LOOP_MODE_CONTROL
typedef struct
{
	uint8 chid;					///< Channel ID
	uint8 sid;					///< Session ID
	uint8 isLoopBack;                               ///< isLoopBack
	uint32 ip_src_addr;                             ///< Source IP address
	uint32 ip_dst_addr;                             ///< Destination IP address
	uint16 udp_src_port;                            ///< UDP Source port 
	uint16 udp_dst_port;                            ///< UDP Destination port
	//int32	ret_val;
}
TstVoipLoopBcakInfo;
#endif


/**
 * @brief Structure for RTCP session 
 * @see do_mgr_VOIP_MGR_SET_RTCP_SESSION()
 */
typedef struct
{
	uint32 ip_src_addr;				///< Source IP address 
	uint32 ip_dst_addr;                             ///< Destination IP address
	uint16 rtcp_src_port;                           ///< RTCP Source port 
	uint16 rtcp_dst_port;                           ///< RTCP Destination port
	uint8 protocol;                                 ///< IP protocol number
	uint8 ch_id;                                    ///< Channel ID
	int32 m_id;                                     ///< Media ID
	uint32 enableXR;                                ///< Enable XR
	uint32 tx_interval;				///< TX interval
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
	struct in6_addr ip6_src_addr;					///< Source IPv6 address 
	struct in6_addr ip6_dst_addr;					///< Destination IPv6 address
	uint32 ipv6;									///< IPv6 flag, 1: IPv6, 0: IPv4
#endif
#ifdef CONFIG_RTK_VOIP_IPC_ARCH
	int result;					///< The result
#endif
	//int32 ret_val;
}
TstVoipRtcpSession;

/**
 * @brief Structure for RTCP session logger 
 * @see do_mgr_VOIP_MGR_GET_RTCP_LOGGER() 
 */
typedef struct
{
	uint32 ch_id;					///< Channel ID
	uint32 m_id;                                    ///< Media ID 
	uint32 TX_packet_count;                         ///< RTCP TX packet count
	uint32 TX_XR_packet_count;                      ///< RTCP-XR TX packet count
	uint8  TX_loss_rate_max;                        ///< Maximum TX loss rate (RTCP)
	uint8  TX_loss_rate_min;                        ///< Minimum TX loss rate (RTCP)
	uint8  TX_loss_rate_avg;                        ///< Average TX loss rate (RTCP)
	uint8  TX_loss_rate_cur;                        ///< Current TX loss rate (RTCP)
	uint32 TX_jitter_max;                           ///< Maximum TX interarrival jitter (RTCP)
	uint32 TX_jitter_min;                           ///< Minimum TX interarrival jitter (RTCP)
	uint32 TX_jitter_avg;                           ///< Average TX interarrival jitter (RTCP)
	uint32 TX_jitter_cur;                           ///< Current TX interarrival jitter (RTCP)
	uint16 TX_round_trip_max;                       ///< Maximum TX round trip delay (RTCP XR)  
	uint16 TX_round_trip_min;                       ///< Minimum TX round trip delay (RTCP XR)
	uint16 TX_round_trip_avg;                       ///< Average TX round trip delay (RTCP XR)
	uint16 TX_round_trip_cur;                       ///< Current TX round trip delay (RTCP XR)
	uint8  TX_MOS_LQ_max;                           ///< Maximum TX MOS LQ score (RTCP XR)
	uint8  TX_MOS_LQ_min;                           ///< Minimum TX MOS LQ score (RTCP XR)
	uint8  TX_MOS_LQ_avg;                           ///< Average TX MOS LQ score (RTCP XR)
	uint8  TX_MOS_LQ_cur;                           ///< Current TX MOS LQ score (RTCP XR)
	uint16  TX_MOS_LQ_max_x10;                      ///< Maximum TX MOS LQ score*10 (RTCP XR)
	uint16  TX_MOS_LQ_min_x10;                      ///< Minimum TX MOS LQ score*10 (RTCP XR)
	uint16  TX_MOS_LQ_avg_x10;                      ///< Average TX MOS LQ score*10 (RTCP XR)
	uint16  TX_MOS_LQ_cur_x10;	                ///< Current TX MOS LQ score*10 (RTCP XR)
	uint32 RX_packet_count;                         ///< RTCP RX packet count
	uint32 RX_XR_packet_count;                      ///< RTCP-XR RX packet count
	uint8  RX_loss_rate_max;                        ///< Maximum RX loss rate (RTCP)
	uint8  RX_loss_rate_min;                        ///< Minimum RX loss rate (RTCP)
	uint8  RX_loss_rate_avg;                        ///< Average RX loss rate (RTCP)
	uint8  RX_loss_rate_cur;                        ///< Current RX loss rate (RTCP)
	uint32 RX_jitter_max;                           ///< Maximum RX interarrival jitter (RTCP)
	uint32 RX_jitter_min;                           ///< Minimum RX interarrival jitter (RTCP)
	uint32 RX_jitter_avg;                           ///< Average RX interarrival jitter (RTCP)
	uint32 RX_jitter_cur;                           ///< Current RX interarrival jitter (RTCP)
	uint16 RX_round_trip_max;                       ///< Maximum RX round trip delay (RTCP XR)  
	uint16 RX_round_trip_min;                       ///< Minimum RX round trip delay (RTCP XR)
	uint16 RX_round_trip_avg;                       ///< Average RX round trip delay (RTCP XR)
	uint16 RX_round_trip_cur;                       ///< Current RX round trip delay (RTCP XR)
	uint8  RX_MOS_LQ_max;                           ///< Maximum RX MOS LQ score (RTCP XR)
	uint8  RX_MOS_LQ_min;                           ///< Minimum RX MOS LQ score (RTCP XR)
	uint8  RX_MOS_LQ_avg;                           ///< Average RX MOS LQ score (RTCP XR)
	uint8  RX_MOS_LQ_cur;                           ///< Current RX MOS LQ score (RTCP XR)
	uint16 RX_MOS_LQ_avg_x10;                       ///< Maximum RX MOS LQ score*10 (RTCP XR)
}
TstVoipRtcpLogger;


typedef struct
{
	uint32 ch_id;					///< Channel ID
	uint32 m_id;					///< Media ID
	uint32 ssrc;					///< Source being reported
	uint32 fracLost;				///< Fraction lost since last report
	uint32 cumLost;					///< Cumulative packet lost
	uint32 lastSeqRecv;				///< Last seq number received
	uint32 jitter;					///< Interval jitter (timestamp units)
	uint32 lastSRTimeStamp;				///< Last SR (LSR) packet received from ssrc
	uint32 lastSRDelay;				///< Delay since last SR (DLSR) packet
}
TstVoipRtcpReport;

typedef struct
{
	uint32 ch_id;					///< Channel ID
	uint8	enable;                                 ///< Enable flag
	//int32	ret_val;
}
TstVoipTest;

/**
 * @brief Structure for payload type
 * @note nPcmMode PCM Mode Setting. <br> 0: No action(use previous PCM enable mode)<br>
 * 1: Auto mode(DSP will change to proper mode according to the voice codec attribute)<br> 
 * 2: Narrow-band Mode <br> 3: Wide-band Mode
 * @see do_mgr_VOIP_MGR_SETRTPPAYLOADTYPE()
 */
typedef struct
{
	uint32 ch_id;					///< Channel ID
	uint32 m_id;					///< Media ID
	RtpPayloadType local_pt; 			///< Local payload type
	RtpPayloadType remote_pt; 			///< Remote payload type
	//RtpPayloadType uPktFormat; 			///< payload type in rtk_voip
	RtpPayloadType uLocalPktFormat;		///< Local payload type in rtk_voip
	RtpPayloadType uRemotePktFormat;	///< Remote payload type in rtk_voip
	RtpPayloadType local_pt_vbd; 			///< vbd payload type in local
	RtpPayloadType remote_pt_vbd; 			///< vbd payload type in remote answer
	RtpPayloadType uPktFormat_vbd; 			///< vbd payload type in rtk_voip
	int32 nG723Type;					///< G.723 bitrate
	//int32 nFramePerPacket;                          ///<  Frames per packet 
	int32 nLocalFramePerPacket;                          ///<  Local Frames per packet 
	int32 nRemoteFramePerPacket;                          ///<  Remote Frames per packet 
	int32 nFramePerPacket_vbd;                      ///<  Frames per packet for VBD
	int32 bVAD;                                     ///< VAD is on or off 
	int32 bPLC;                                     ///< PLC is on or off
	//int32 result;                                 
	uint32 nJitterDelay;                            ///< Jitter buffer delay (10ms)
	uint32 nMaxDelay;                               ///< Maximum jitter buffer delay (10ms)
	uint32 nMaxStrictDelay;							///< Maximum strict jitter buffer delay (10ms)
	uint32 nJitterFactor;                           ///< fitter buffer factor 
	uint32 nG726Packing;				///< G.726 packing method 
	uint32 bT38ParamEnable;				///< If false, default settings are adopted and nT38MaxBuffer and nT38RateMgt are ignored. 
	uint32 nT38MaxBuffer;				///< T.38 Max buffer size. It can be 200 to 600ms. (default: 500)
	uint32 nT38RateMgt;				///< T.38 Rate management. 1: don't pass tcf data, 2: pass tcf data. (default: 2) 
	uint32 nT38MaxRate; 	                        ///< T.38 Max Rate. 0:2400, 1:4800, 2:7200, 3:9600, 4:12000, 5:14400 (default: 5)
	uint32 bT38EnableECM;	                        ///< T.38 ECM Enabled. 0: disable, 1: enable (default: 1)
	uint32 nT38ECCSignal;	                        ///< T.38 ECC Signal. 0~7 (default: 5)
	uint32 nT38ECCData;				///< T.38 ECC Data. 0~2 (default: 2) 
        uint32 nG7111Mode;				///< G711.1 Mode, 1: R1, 2: R2a, 3: R2b, 4: R3
	uint32 bT38EnableSpoof;				///< T.38 Spoof Default: 1. 1: Enable, 0: Disable 
	uint32 nT38DuplicateNum;			///< T.38 Duplicate 0~2 (default: 0)
	uint32 nPcmMode;				///< PCM Mode Setting. 0: No action 1: Auto mode, 2: Narrow-band Mode, 3: Wide-band Mode
	//int32	ret_val;
}
TstVoipPayLoadTypeConfig;

/**
 * @brief Structure for payload type
 * @see do_mgr_VOIP_MGR_SET_VAD_SID_THRESHOLD()
 */
typedef struct {
	uint32 ch_id;					///< Channel ID
	uint32 m_id;                                    ///< Media ID
	int32 nThresVAD;                                ///< Threshold for VAD average amplitude. default 63. range=1~200
	int32 nThresCNG;                                ///< Maximum CNG amplitude. default 0 (off). range =0~200
	int32 nSIDMode;                                 ///< SID configuration mode (0:Disable configuration, 1:Fixed noise level 2: Adjust noise level)
	int32 nSIDLevel;                                ///< SID noise level, range=0~127, Unit:dBov, default 70
	int32 nSIDGain;					///< SID gain up/down based on SID noise level, range=-127~127, Unit:dB, default:0 (not change)
}
TstVoipThresVadCngConfig;

/**
 * @brief Structure for RTP session state 
 * @see do_mgr_VOIP_MGR_SETRTPSESSIONSTATE()
 */
typedef struct
{
	uint32 ch_id;					///< Channel ID
	uint32 m_id;					///< Media ID
	uint8	state;					///< RTP state
	//int32	ret_val;
}
TstVoipRtpSessionState;

/**
 * @brief Structure for playing tone 
 * @see do_mgr_VOIP_MGR_SETPLAYTONE()
 */
typedef struct
{
	uint32 ch_id;					///< Channel ID
	uint32 m_id;					///< Media ID
	DSPCODEC_TONE nTone;                            ///< Tone ID
	uint32 bFlag;                                   ///< Play or stop a tone
	DSPCODEC_TONEDIRECTION path;			///<  Play tone locally or remotely 
	//int32	ret_val;
}
TstVoipPlayToneConfig;

/**
 * @brief Structure for general purpose control. 
 * @see do_mgr_VOIP_MGR_RTP_CFG() do_mgr_VOIP_MGR_UNSET_SESSION()
 *      do_mgr_VOIP_MGR_HOLD() do_mgr_VOIP_MGR_CTRL_RTPSESSION()
 *      do_mgr_VOIP_MGR_CTRL_TRANSESSION_ID() do_mgr_VOIP_MGR_UNSET_RTCP_SESSION()
 *      do_mgr_VOIP_MGR_ON_HOOK_RE_INIT() do_mgr_VOIP_MGR_FAX_MODEM_PASS_CFG()
 *      do_mgr_VOIP_MGR_FAX_OFFHOOK() do_mgr_VOIP_MGR_FAX_END_DETECT()
 *      do_mgr_VOIP_MGR_SET_FAX_MODEM_DET() do_mgr_VOIP_MGR_FAX_DIS_DETECT()
 *		do_mgr_VOIP_MGR_SET_FAX_SILENCE_DET()
 *      do_mgr_VOIP_MGR_SET_G168_LEC_CFG()
 *      do_mgr_VOIP_MGR_DTMF_DET_THRESHOLD()
 *      do_mgr_VOIP_MGR_DTMF_CFG() do_mgr_VOIP_MGR_SET_DTMF_MODE()
 *      do_mgr_VOIP_MGR_SEND_RFC2833_BY_AP() do_mgr_VOIP_MGR_SET_CID_DET_MODE()
 *      do_mgr_VOIP_MGR_SET_CID_FSK_GEN_MODE() do_mgr_VOIP_MGR_SET_PULSE_DIGIT_DET()
 *      do_mgr_VOIP_MGR_SET_DIAL_MODE() do_mgr_VOIP_MGR_GET_DIAL_MODE()
 *      do_mgr_VOIP_MGR_PCM_CFG() do_mgr_VOIP_MGR_GET_SLIC_STAT()
 *      do_mgr_VOIP_MGR_SLIC_ONHOOK_ACTION() do_mgr_VOIP_MGR_SLIC_OFFHOOK_ACTION()
 *      do_mgr_VOIP_MGR_HOOK_FIFO_IN() do_mgr_VOIP_MGR_DAA_RING()
 *      do_mgr_VOIP_MGR_DAA_OFF_HOOK() do_mgr_VOIP_MGR_DAA_ON_HOOK()
 *      do_mgr_VOIP_MGR_FXO_ON_HOOK() do_mgr_VOIP_MGR_FXO_OFF_HOOK()
 *      do_mgr_VOIP_MGR_SIP_REGISTER() do_mgr_VOIP_MGR_GET_FEATURE()
 *      do_mgr_VOIP_MGR_VOIP_RESOURCE_CHECK()
 */
typedef struct
{
	uint32 ch_id;					///< Channel ID
	uint32 m_id;                                    ///< Media ID
	uint32 t_id;                                    ///< Transaction (active) ID
	uint32 enable; 					///< Enable or disable,  0-> off, 1->ON  
	uint32 cfg;                                     ///< Configuration value 
	uint32 cfg2;                                    ///< Configuration value2
	uint32 cfg3;                                    ///< Configuration value3
	//int32	ret_val;
}
TstVoipCfg;

/**
 * @brief Structure for general purpose control.
 * @see do_mgr_VOIP_MGR_GET_FEATURE()
 */
typedef struct {
	uint8 feature_raw[ 8 ];				///< use memcpy() to exchange VoIP feature
}
TstVoipFeature;


/**
 * @brief Structure for general purpose control. 
 * @see do_mgr_VOIP_MGR_SET_RTCP_TX_INTERVAL() do_mgr_VOIP_MGR_SET_VOICE_GAIN()
 *      do_mgr_VOIP_MGR_ENERGY_DETECT() do_mgr_VOIP_MGR_DSPCODECSTOP()
 *      do_mgr_VOIP_MGR_SET_ECHO_TAIL_LENGTH() do_mgr_VOIP_MGR_DTMF_GET()
 *      do_mgr_VOIP_MGR_PLAY_SIP_INFO() do_mgr_VOIP_MGR_SET_TONE_OF_COUNTRY()
 *      do_mgr_VOIP_MGR_SET_TONE_OF_CUSTOMIZE() do_mgr_VOIP_MGR_USE_CUST_TONE()
 *      do_mgr_VOIP_MGR_SET_TONE_OF_UPDATE()
 *      do_mgr_VOIP_MGR_SET_SPK_AGC() do_mgr_VOIP_MGR_SET_SPK_AGC_LVL()
 *      do_mgr_VOIP_MGR_SET_SPK_AGC_GUP() do_mgr_VOIP_MGR_SET_SPK_AGC_GDOWN()
 *      do_mgr_VOIP_MGR_SET_MIC_AGC() do_mgr_VOIP_MGR_SET_MIC_AGC_LVL()
 *      do_mgr_VOIP_MGR_SET_MIC_AGC_GUP() do_mgr_VOIP_MGR_SET_MIC_AGC_GDOWN()
 *      do_mgr_VOIP_MGR_PULSE_DIAL_GEN_CFG() do_mgr_VOIP_MGR_GEN_PULSE_DIAL()
 *      do_mgr_VOIP_MGR_SET_PCM_LOOP_MODE() do_mgr_VOIP_MGR_SET_SLIC_TX_GAIN()
 *      do_mgr_VOIP_MGR_SET_SLIC_RX_GAIN() do_mgr_VOIP_MGR_SET_SLIC_RING_CADENCE()
 *      do_mgr_VOIP_MGR_FLUSH_FIFO() do_mgr_VOIP_MGR_LINE_CHECK()
 *      do_mgr_VOIP_MGR_SET_RING_DETECTION() do_mgr_VOIP_MGR_SET_DAA_TX_GAIN()
 *      do_mgr_VOIP_MGR_SET_DAA_RX_GAIN() do_mgr_VOIP_MGR_SET_DAA_ISR_FLOW()
 *      do_mgr_VOIP_MGR_GET_DAA_ISR_FLOW() do_mgr_VOIP_MGR_SET_DAA_PCM_HOLD_CFG()
 *      do_mgr_VOIP_MGR_GET_DAA_BUSY_TONE_STATUS() do_mgr_VOIP_MGR_GET_DAA_USED_BY_WHICH_SLIC()
 *      do_mgr_VOIP_MGR_DEBUG() 
 */
typedef struct
{
	uint32 ch_id;					 ///< Channel ID
	uint32 m_id;                                     ///< Media ID
	int8	value;                                   ///< Configuration value 
	int8	value1;                                  ///< Configuration value1
	int8	value2;                                  ///< Configuration value2
	int8	value3;                                  ///< Configuration value3
	int8	value4;                                  ///< Configuration value4
	int16	value5;                                  ///< Configuration value5
	int16	value6;                                  ///< Configuration value6
	int16  value7;                                   ///< Configuration value7
	//int32	ret_val;
}
TstVoipValue;

/**
 * @brief Structure for retrieving VoIP event  
 * @see do_mgr_VOIP_MGR_GET_VOIP_EVENT()
 */
typedef struct 
{
	uint32			ch_id;		///< [in] Channel ID
	VoipEventType	type;		///< [in] Event type filter
	VoipEventMask	mask;		///< [in/out] Mask session for RFC2833 and DSP, or in/out for DTMF 
	VoipEventID		id;			///< [out] return event ID 
	uint32			p0;			///< [out] additional parameter 0 for the event 
	uint32			p1;			///< [out] additional parameter 1 for the event 
	uint32			time;		///< [out] when does the event takes place? (in unit of ms)
}
TstVoipEvent;

/**
 * @brief Structure for flushing VoIP event  
 * @see do_mgr_VOIP_MGR_FLUSH_VOIP_EVENT()
 */
typedef struct 
{
	uint32			ch_id;		///< [in] Channel ID
	VoipEventType	type;		///< [in] Event type filter
	VoipEventMask	mask;		///< [in] Mask session for RFC2833 and DSP, or in/out for DTMF 
}
TstFlushVoipEvent;

/**
 * @brief Structure for retrieving hook status 
 * @see do_mgr_VOIP_MGR_SLIC_HOOK()
 */
typedef struct
{
	unsigned char ch_id;          			///< Channel ID
	unsigned char change;         			///< Hook status is changed , 1: Change. 0: No-Change
	unsigned char hook_status;    			///< Current hook status, 1: Off-Hook, 0: On-Hook
	//int32	ret_val;
}
TstVoipSlicHook;

/**
 * @brief Structure for SLIC ring 
 * @see do_mgr_VOIP_MGR_SLIC_RING()
 */
typedef struct
{
	uint32 ch_id;          			///< Channel ID
	uint32 ring_set;       			///< Ring_ON: ring_set = 1 , Ring_OFF: ring_set = 0
	//int32	ret_val;
}
TstVoipSlicRing;

/**
 * @brief Structure for SLIC to generate tone 
 * @warning Now, we use DSP to generate tone. 
 * @see do_mgr_VOIP_MGR_SLIC_TONE() VOIP_DSP_TONE
 */
typedef struct
{
	unsigned char ch_id;				///< Channel ID
	unsigned int tone2play;                          ///< The tone to play 
	//int32	ret_val;
}
TstVoipSlicTone;

/**
 * @brief Structure for SLIC restart 
 * @see do_mgr_VOIP_MGR_SLIC_RESTART() 
 */
typedef struct
{
	unsigned char ch_id;				///< Channel ID
	unsigned int codec_law; 				///< Codec mode (It suggests a-law (1)), linear: 0, a_law: 1, u_law: 2
	//int32	ret_val;
}
TstVoipSlicRestart;


/**
 * @brief Structure for caller ID 
 *
 * cid_dtmf_mode Caller ID in DTMF mode. <br>
 *        0-1 bits for starting digit, and 2-3 bits for ending digit. <br>
 *        00: A, 01: B, 02: C, 03: D
 * @see do_mgr_VOIP_MGR_DTMF_CID_GEN_CFG() do_mgr_VOIP_MGR_GET_CID_STATE_CFG()
 *      do_mgr_VOIP_MGR_FSK_CID_GEN_CFG() do_mgr_VOIP_MGR_SET_FSK_VMWI_STATE()
 *      do_mgr_VOIP_MGR_SET_FSK_AREA() do_mgr_VOIP_MGR_FSK_ALERT_GEN_CFG()
 *      do_mgr_VOIP_MGR_SET_CID_DTMF_MODE() do_mgr_VOIP_MGR_GET_FSK_CID_STATE_CFG()
 *      do_mgr_VOIP_MGR_FSK_CID_VMWI_GEN_CFG() do_mgr_VOIP_MGR_GET_DAA_CALLER_ID()
 */
typedef struct
{
	unsigned char ch_id;				///< Channel ID
	unsigned char daa_id;				///< DAA ID
	unsigned char cid_state;                        ///< Caller ID state 
	unsigned char cid_mode;                         ///< Caller ID mode 
	unsigned char cid_gain;                         ///< Caller ID gain 
	unsigned char cid_msg_type;                     ///< Caller ID message type 
	char string[21];                                ///< Caller ID - Phonenumber  
	char string2[9];					///< Caller ID - Date and time. For date & time sync
	char cid_name[51];                               ///< Caller ID - Name
	unsigned char cid_dtmf_mode;                    ///< Caller ID in DTMF mode.
	unsigned int cid_dtmf_start;			///< DTMF Caller ID start digit
	unsigned int cid_dtmf_end;			///< DTMF Caller ID end digit
	unsigned int cid_dtmf_auto;			///< DTMF Caller ID auto control for auto Ringing, auto SLIC action, auto Start/End digit sending, etc.
	unsigned int cid_dtmf_on_ms;
	unsigned int cid_dtmf_pause_ms;
	unsigned int cid_dtmf_pre_silence_ms;
	unsigned int cid_dtmf_end_silence_ms;
	//int32	ret_val;
}
TstVoipCID;//For dtmf cid generation

/**
 * @brief For fsk type2 caller id detect
 */
typedef struct
{
	unsigned long time_out;				///< Time out
	unsigned char cid_setup;                        ///< CID setup
	unsigned char cid_mode;				///< 0:don't need ack 'D', 1:ETSI need ack 'D' 
	unsigned char cid_states;			///< CID state
	unsigned char cid_complete;			///< CID completed or not
	char d_play_cnt;					///< play count
	//int32	ret_val;
}
TstVoipFskT2Cid_det;

/**
 * @brief For fsk caller id detect
 */
typedef struct
{
	unsigned long time_out;				///< Time out
	unsigned char cid_setup;                        ///< CID setup
	unsigned char cid_mode;                         ///< CID mode
	unsigned char cid_msg_type;                     ///< CID message type
	unsigned char cid_states;                       ///< CID state
	unsigned char cid_complete;                     ///< CID completed or not
	char cid_number[21];                             ///< CID phone number
	char cid_date[9];				///< CID Date and time. For date & time sync
	char cid_name[51];				///< CID - Name
	//int32	ret_val;
}
TstVoipFskCid;

/**
 * @brief For fsk caller id gen para setting
 */
typedef struct
{
	unsigned char ch_id;				///< Channel ID
	TfskArea area;					///< [ Area ] 0:Bellcore 1:ETSI 2:BT 3:NTT 
	unsigned int CS_cnt;				///< channel seizure signal bit count 
	unsigned int mark_cnt;				///< mark signal bit count 
	TfskClidGain mark_gain;				///< unit in dB 
	TfskClidGain space_gain;			///< unit in dB 
	unsigned int type2_expected_ack_tone;		///< DTMF A, or B, or C, or D
	unsigned int delay_after_1st_ring;		///< unit in ms 
	unsigned int delay_before_2nd_ring;		///< unit in ms 
	unsigned int silence_before_sas;		///< unit in ms 
	unsigned int sas_time;				///< unit in ms 
	unsigned int delay_after_sas;			///< unit in ms 
	unsigned int cas_time;				///< unit in ms 
	unsigned int type1_delay_after_cas;		///< unit in ms 
	unsigned int ack_waiting_time;			///< unit in ms 
	unsigned int delay_after_ack_recv;		///< unit in ms 
	unsigned int delay_after_type2_fsk;		///< unit in ms, range 40~640ms 
	unsigned int RPAS_Duration;			///< The duration of RP-AS 200ms~300ms (may extend to 450ms):ETSI
	unsigned int RPAS2FSK_Period;			///< From end of RP-AS to start of FSK data transmission (T3:ETSI)
	//int32	ret_val;
}
TstVoipFskPara;

// The 2 definition should be the same to the definitions in fskcid_gen.h
#define MAX_CLID_DATA_SIZE	80
#define FSK_MDMF_SIZE		5

/**
 * @brief This structure should be the same to the structure TstFskClidData in fskcid_gen.h
 */
typedef struct
{
        TfskParaType type;				///< FSK parameter type
        char data[MAX_CLID_DATA_SIZE];                   ///< FSK call ID data
}
TstFskClid_Data;

typedef struct
{
        unsigned char ch_id;				///< Channel ID
        unsigned char service_type;                     ///< Service type
        TstFskClid_Data cid_data[FSK_MDMF_SIZE];		///< FSK call ID data
        //int32	ret_val;
}
TstFskClid;


/**
 * @brief Structure for sending RFC2833 packet 
 * @see do_mgrVOIP_MGR_SEND_RFC2833_PKT_CFG()
 */
typedef struct
{
	unsigned char ch_id;				///< Channel ID
	unsigned char m_id;				///< Media ID
	unsigned char digit;                            ///< A digit to send 
	unsigned int duration;                          ///< DTMF duration 
	unsigned char bEnable;                          ///< Enable bool value
	unsigned int volume;                            ///< DTMF tone volume
	//int32	ret_val;
}
TstVoip2833;//For sending RFC2833 packet

/**
 * @brief Structure for reading and writing SLIC register
 * @see do_mgr_VOIP_MGR_GET_SLIC_REG_VAL() do_mgr_VOIP_MGR_SET_SLIC_REG_VAL()
 */
typedef struct
{
	unsigned char ch_id;				///< Channel ID
	unsigned int reg_num;				///< Register number 
	unsigned char reg_len;                          ///< Number of Register bytes (Max 16) 
	unsigned char reg_ary[16];                      ///< for Register value
	//int32	ret_val;
}
TstVoipSlicReg;

/**
 * @brief Structure for reading and writing SLIC RAM
 * @see do_mgr_VOIP_MGR_GET_SLIC_RAM_VAL() do_mgr_VOIP_MGR_SET_SLIC_RAM_VAL()
 */
typedef struct
{
	unsigned char ch_id;				///< Channel ID
	unsigned short reg_num;                         ///< RAM number 
	unsigned int reg_val;                           ///< RAM value
	//int32	ret_val;
}
TstVoipSlicRam;

#if 0
typedef struct
{
	unsigned char ch_id;				///< Channel ID
	unsigned char s_id;                             ///< Session ID
	RtpPayloadType uPktFormat;                      ///< RTP payload type
	unsigned char channel_enable;			///< Enable bool value
	//int32	ret_val;
}
TstTwoChannelCfg; //for two channel simplify g729
#endif

/**
 * @brief Structure for setting flash hook time
 * @see do_mgr_VOIP_MGR_SET_FLASH_HOOK_TIME()
 */
typedef struct
{
	unsigned char ch_id;				///< Channel ID
	unsigned int  flash_time;                       ///< Upper bound of flash hook time (ms) 
	unsigned int  flash_time_min;                   ///< Lower bound of flash hook time (ms) 
	unsigned int  dummy;				///< Dummy
	//int32	ret_val;
}
TstVoipHook;

/**
 * @brief Structure for setting off hook time
 * @see do_mgr_VOIP_MGR_SET_OFF_HOOK_TIME()
 */
typedef struct
{
	unsigned char ch_id;				///< Channel ID
	unsigned int  offhook_min;				///< offhook_minimum time 
	//int32	ret_val;
}
TstVoipOffHook;

/**
 * @brief Structure for setting or getting calibration coefficients
 * @see do_mgr_VOIP_MGR_SET_CALIBRATION_COEFFICIENTS() do_mgr_VOIP_MGR_GET_CALIBRATION_COEFFICIENTS()
 */
typedef struct
{
	unsigned char ch_id;				///< Channel ID
	unsigned char coeff[52];				///< calibration parameters, le89116 is 52bytes 
	int result;
}
TstVoipCalibrationCoeff;

/**
 * @brief Structure for do calibration coefficients
 * @see do_mgr_VOIP_MGR_DO_CALIBRATION()
 */
typedef struct
{
	unsigned char ch_id;				///< Channel ID
	unsigned char mode;				///< 0 : VpCalLine, 1 : VpCal
	int result;
}
TstVoipDoCalibration;

/**
 * @brief Structure for tone configuration
 * @see do_mgr_VOIP_MGR_SET_CUST_TONE_PARAM() do_mgr_VOIP_MGR_SET_UPDATE_TONE_PARAM()
 */
typedef struct
{
	unsigned long	toneType;	///< see _TONE_TYPE_
	unsigned short	cycle;		///< "<0": illegal value, "0": represent "continuous tone", ">0": cycle number

	unsigned short	cadNUM;		///< Cadence number (in SUCC and SUCC_ADD mode, it represent repeat number of sequence, 
	                            ///<  FOUR_FREQ support 32 cad)	
	unsigned long	CadOn0;		///< Cadence On0 time (ms)
	unsigned long	CadOff0;		///< Cadence Off0 time (ms)
	unsigned long	CadOn1;		///< Cadence On1 time (ms)
	unsigned long	CadOff1;		///< Cadence Off1 time (ms)
	unsigned long	CadOn2;		///< Cadence On2 time (ms)
	unsigned long	CadOff2;		///< Cadence Off2 time (ms)
	unsigned long	CadOn3;		///< Cadence On3 time (ms)
	unsigned long	CadOff3;		///< Cadence Off3 time (ms)

	unsigned long PatternOff;	///< pattern Off time (ms)
	unsigned long ToneNUM;		///< tone number (1..4)

	unsigned long	Freq0;		///< Freq0 (Hz)
	unsigned long	Freq1;		///< Freq1 (Hz)
	unsigned long	Freq2;		///< Freq2 (Hz)
	unsigned long	Freq3;		///< Freq3 (Hz)

	long Gain0;			///< Gain0 (db)
	long Gain1;			///< Gain1 (db)
	long Gain2;			///< Gain2 (db)
	long Gain3;			///< Gain3 (db)

	unsigned long	C1_Freq0;		///< C1_Freq0 (Hz)
	unsigned long	C1_Freq1;		///< C1_Freq1 (Hz)
	unsigned long	C1_Freq2;		///< C1_Freq2 (Hz)
	unsigned long	C1_Freq3;		///< C1_Freq3 (Hz)

	long C1_Gain0;			///< C1_Gain0 (db)
	long C1_Gain1;			///< C1_Gain1 (db)
	long C1_Gain2;			///< C1_Gain2 (db)
	long C1_Gain3;			///< C1_Gain3 (db)

	unsigned long	C2_Freq0;		///< C2_Freq0 (Hz)
	unsigned long	C2_Freq1;		///< C2_Freq1 (Hz)
	unsigned long	C2_Freq2;		///< C2_Freq2 (Hz)
	unsigned long	C2_Freq3;		///< C2_Freq3 (Hz)

	long C2_Gain0;			///< C2_Gain0 (db)
	long C2_Gain1;			///< C2_Gain1 (db)
	long C2_Gain2;			///< C2_Gain2 (db)
	long C2_Gain3;			///< C2_Gain3 (db)

	unsigned long	C3_Freq0;		///< C3_Freq0 (Hz)
	unsigned long	C3_Freq1;		///< C3_Freq1 (Hz)
	unsigned long	C3_Freq2;		///< C3_Freq2 (Hz)
	unsigned long	C3_Freq3;		///< C3_Freq3 (Hz)

	long C3_Gain0;			///< C3_Gain0 (db)
	long C3_Gain1;			///< C3_Gain1 (db)
	long C3_Gain2;			///< C3_Gain2 (db)
	long C3_Gain3;			///< C3_Gain3 (db)

	unsigned long	CadOn4;		///< Cadence On4 time (ms)
	unsigned long	CadOff4;		///< Cadence Off4 time (ms)
	unsigned long	CadOn5;		///< Cadence On5 time (ms)
	unsigned long	CadOff5;		///< Cadence Off5 time (ms)
	unsigned long	CadOn6;		///< Cadence On6 time (ms)
	unsigned long	CadOff6;		///< Cadence Off6 time (ms)
	unsigned long	CadOn7;		///< Cadence On7 time (ms)
	unsigned long	CadOff7;		///< Cadence Off7 time (ms)

	unsigned long	C4_Freq0;		///< C_4Freq0 (Hz)
	unsigned long	C4_Freq1;		///< C_4Freq1 (Hz)
	unsigned long	C4_Freq2;		///< C_4Freq2 (Hz)
	unsigned long	C4_Freq3;		///< C_4Freq3 (Hz)

	long C4_Gain0;			///< C_4Gain0 (db)
	long C4_Gain1;			///< C_4Gain1 (db)
	long C4_Gain2;			///< C_4Gain2 (db)
	long C4_Gain3;			///< C_4Gain3 (db)

	unsigned long	C5_Freq0;		///< C5_Freq0 (Hz)
	unsigned long	C5_Freq1;		///< C5_Freq1 (Hz)
	unsigned long	C5_Freq2;		///< C5_Freq2 (Hz)
	unsigned long	C5_Freq3;		///< C5_Freq3 (Hz)

	long C5_Gain0;			///< C5_Gain0 (db)
	long C5_Gain1;			///< C5_Gain1 (db)
	long C5_Gain2;			///< C5_Gain2 (db)
	long C5_Gain3;			///< C5_Gain3 (db)

	unsigned long	C6_Freq0;		///< C6_Freq0 (Hz)
	unsigned long	C6_Freq1;		///< C6_Freq1 (Hz)
	unsigned long	C6_Freq2;		///< C6_Freq2 (Hz)
	unsigned long	C6_Freq3;		///< C6_Freq3 (Hz)

	long C6_Gain0;			///< C6_Gain0 (db)
	long C6_Gain1;			///< C6_Gain1 (db)
	long C6_Gain2;			///< C6_Gain2 (db)
	long C6_Gain3;			///< C6_Gain3 (db)

	unsigned long	C7_Freq0;		///< C7_Freq0 (Hz)
	unsigned long	C7_Freq1;		///< C7_Freq1 (Hz)
	unsigned long	C7_Freq2;		///< C7_Freq2 (Hz)
	unsigned long	C7_Freq3;		///< C7_Freq3 (Hz)

	long C7_Gain0;			///< C7_Gain0 (db)
	long C7_Gain1;			///< C7_Gain1 (db)
	long C7_Gain2;			///< C7_Gain2 (db)
	long C7_Gain3;			///< C7_Gain3 (db)

	unsigned long	CadOn8;		///< Cadence On8 time (ms)
	unsigned long	CadOff8;		///< Cadence Off8 time (ms)
	unsigned long	CadOn9;		///< Cadence On9 time (ms)
	unsigned long	CadOff9;		///< Cadence Off9 time (ms)
	unsigned long	CadOn10;		///< Cadence On10 time (ms)
	unsigned long	CadOff10;		///< Cadence Off10 time (ms)
	unsigned long	CadOn11;		///< Cadence On11 time (ms)
	unsigned long	CadOff11;		///< Cadence Off11 time (ms)

	unsigned long	C8_Freq0;		///< C8_Freq0 (Hz)
	unsigned long	C8_Freq1;		///< C8_Freq1 (Hz)
	unsigned long	C8_Freq2;		///< C8_Freq2 (Hz)
	unsigned long	C8_Freq3;		///< C8_Freq3 (Hz)

	long C8_Gain0;			///< C8_Gain0 (db)
	long C8_Gain1;			///< C8_Gain1 (db)
	long C8_Gain2;			///< C8_Gain2 (db)
	long C8_Gain3;			///< C8_Gain3 (db)

	unsigned long	C9_Freq0;		///< C9_Freq0 (Hz)
	unsigned long	C9_Freq1;		///< C9_Freq1 (Hz)
	unsigned long	C9_Freq2;		///< C9_Freq2 (Hz)
	unsigned long	C9_Freq3;		///< C9_Freq3 (Hz)

	long C9_Gain0;			///< C9_Gain0 (db)
	long C9_Gain1;			///< C9_Gain1 (db)
	long C9_Gain2;			///< C9_Gain2 (db)
	long C9_Gain3;			///< C9_Gain3 (db)

	unsigned long	C10_Freq0;		///< C10_Freq0 (Hz)
	unsigned long	C10_Freq1;		///< C10_Freq1 (Hz)
	unsigned long	C10_Freq2;		///< C10_Freq2 (Hz)
	unsigned long	C10_Freq3;		///< C10_Freq3 (Hz)

	long C10_Gain0;			///< C10_Gain0 (db)
	long C10_Gain1;			///< C10_Gain1 (db)
	long C10_Gain2;			///< C10_Gain2 (db)
	long C10_Gain3;			///< C10_Gain3 (db)

	unsigned long	C11_Freq0;		///< C11_Freq0 (Hz)
	unsigned long	C11_Freq1;		///< C11_Freq1 (Hz)
	unsigned long	C11_Freq2;		///< C11_Freq2 (Hz)
	unsigned long	C11_Freq3;		///< C11_Freq3 (Hz)

	long C11_Gain0;			///< C11_Gain0 (db)
	long C11_Gain1;			///< C11_Gain1 (db)
	long C11_Gain2;			///< C11_Gain2 (db)
	long C11_Gain3;			///< C11_Gain3 (db)

	unsigned long	CadOn12;		///< Cadence On12 time (ms)
	unsigned long	CadOff12;		///< Cadence Off12 time (ms)
	unsigned long	CadOn13;		///< Cadence On13 time (ms)
	unsigned long	CadOff13;		///< Cadence Off13 time (ms)
	unsigned long	CadOn14;		///< Cadence On14 time (ms)
	unsigned long	CadOff14;		///< Cadence Off14 time (ms)
	unsigned long	CadOn15;		///< Cadence On15 time (ms)
	unsigned long	CadOff15;		///< Cadence Off15 time (ms)

	unsigned long	C12_Freq0;		///< C12_Freq0 (Hz)
	unsigned long	C12_Freq1;		///< C12_Freq1 (Hz)
	unsigned long	C12_Freq2;		///< C12_Freq2 (Hz)
	unsigned long	C12_Freq3;		///< C12_Freq3 (Hz)

	long C12_Gain0;			///< C12_Gain0 (db)
	long C12_Gain1;			///< C12_Gain1 (db)
	long C12_Gain2;			///< C12_Gain2 (db)
	long C12_Gain3;			///< C12_Gain3 (db)

	unsigned long	C13_Freq0;		///< C13_Freq0 (Hz)
	unsigned long	C13_Freq1;		///< C13_Freq1 (Hz)
	unsigned long	C13_Freq2;		///< C13_Freq2 (Hz)
	unsigned long	C13_Freq3;		///< C13_Freq3 (Hz)

	long C13_Gain0;			///< C13_Gain0 (db)
	long C13_Gain1;			///< C13_Gain1 (db)
	long C13_Gain2;			///< C13_Gain2 (db)
	long C13_Gain3;			///< C13_Gain3 (db)

	unsigned long	C14_Freq0;		///< C14_Freq0 (Hz)
	unsigned long	C14_Freq1;		///< C14_Freq1 (Hz)
	unsigned long	C14_Freq2;		///< C14_Freq2 (Hz)
	unsigned long	C14_Freq3;		///< C14_Freq3 (Hz)

	long C14_Gain0;			///< C14_Gain0 (db)
	long C14_Gain1;			///< C14_Gain1 (db)
	long C14_Gain2;			///< C14_Gain2 (db)
	long C14_Gain3;			///< C14_Gain3 (db)

	unsigned long	C15_Freq0;		///< C15_Freq0 (Hz)
	unsigned long	C15_Freq1;		///< C15_Freq1 (Hz)
	unsigned long	C15_Freq2;		///< C15_Freq2 (Hz)
	unsigned long	C15_Freq3;		///< C15_Freq3 (Hz)

	long C15_Gain0;			///< C15_Gain0 (db)
	long C15_Gain1;			///< C15_Gain1 (db)
	long C15_Gain2;			///< C15_Gain2 (db)
	long C15_Gain3;			///< C15_Gain3 (db)

	unsigned long	CadOn16;		///< Cadence On16 time (ms)
	unsigned long	CadOff16;		///< Cadence Off16 time (ms)
	unsigned long	CadOn17;		///< Cadence On17 time (ms)
	unsigned long	CadOff17;		///< Cadence Off17 time (ms)
	unsigned long	CadOn18;		///< Cadence On18 time (ms)
	unsigned long	CadOff18;		///< Cadence Off18 time (ms)
	unsigned long	CadOn19;		///< Cadence On19 time (ms)
	unsigned long	CadOff19;		///< Cadence Off19 time (ms)

	unsigned long	C16_Freq0;		///< C16_Freq0 (Hz)
	unsigned long	C16_Freq1;		///< C16_Freq1 (Hz)
	unsigned long	C16_Freq2;		///< C16_Freq2 (Hz)
	unsigned long	C16_Freq3;		///< C16_Freq3 (Hz)

	long C16_Gain0;			///< C16_Gain0 (db)
	long C16_Gain1;			///< C16_Gain1 (db)
	long C16_Gain2;			///< C16_Gain2 (db)
	long C16_Gain3;			///< C16_Gain3 (db)

	unsigned long	C17_Freq0;		///< C17_Freq0 (Hz)
	unsigned long	C17_Freq1;		///< C17_Freq1 (Hz)
	unsigned long	C17_Freq2;		///< C17_Freq2 (Hz)
	unsigned long	C17_Freq3;		///< C17_Freq3 (Hz)

	long C17_Gain0;			///< C17_Gain0 (db)
	long C17_Gain1;			///< C17_Gain1 (db)
	long C17_Gain2;			///< C17_Gain2 (db)
	long C17_Gain3;			///< C17_Gain3 (db)

	unsigned long	C18_Freq0;		///< C18_Freq0 (Hz)
	unsigned long	C18_Freq1;		///< C18_Freq1 (Hz)
	unsigned long	C18_Freq2;		///< C18_Freq2 (Hz)
	unsigned long	C18_Freq3;		///< C18_Freq3 (Hz)

	long C18_Gain0;			///< C18_Gain0 (db)
	long C18_Gain1;			///< C18_Gain1 (db)
	long C18_Gain2;			///< C18_Gain2 (db)
	long C18_Gain3;			///< C18_Gain3 (db)

	unsigned long	C19_Freq0;		///< C19_Freq0 (Hz)
	unsigned long	C19_Freq1;		///< C19_Freq1 (Hz)
	unsigned long	C19_Freq2;		///< C19_Freq2 (Hz)
	unsigned long	C19_Freq3;		///< C19_Freq3 (Hz)

	long C19_Gain0;			///< C19_Gain0 (db)
	long C19_Gain1;			///< C19_Gain1 (db)
	long C19_Gain2;			///< C19_Gain2 (db)
	long C19_Gain3;			///< C19_Gain3 (db)

	unsigned long	CadOn20;		///< Cadence On20 time (ms)
	unsigned long	CadOff20;		///< Cadence Off20 time (ms)
	unsigned long	CadOn21;		///< Cadence On21 time (ms)
	unsigned long	CadOff21;		///< Cadence Off21 time (ms)
	unsigned long	CadOn22;		///< Cadence On22 time (ms)
	unsigned long	CadOff22;		///< Cadence Off22 time (ms)
	unsigned long	CadOn23;		///< Cadence On23 time (ms)
	unsigned long	CadOff23;		///< Cadence Off23 time (ms)

	unsigned long	C20_Freq0;		///< C20_Freq0 (Hz)
	unsigned long	C20_Freq1;		///< C20_Freq1 (Hz)
	unsigned long	C20_Freq2;		///< C20_Freq2 (Hz)
	unsigned long	C20_Freq3;		///< C20_Freq3 (Hz)

	long C20_Gain0;			///< C20_Gain0 (db)
	long C20_Gain1;			///< C20_Gain1 (db)
	long C20_Gain2;			///< C20_Gain2 (db)
	long C20_Gain3;			///< C20_Gain3 (db)

	unsigned long	C21_Freq0;		///< C21_Freq0 (Hz)
	unsigned long	C21_Freq1;		///< C21_Freq1 (Hz)
	unsigned long	C21_Freq2;		///< C21_Freq2 (Hz)
	unsigned long	C21_Freq3;		///< C21_Freq3 (Hz)

	long C21_Gain0;			///< C21_Gain0 (db)
	long C21_Gain1;			///< C21_Gain1 (db)
	long C21_Gain2;			///< C21_Gain2 (db)
	long C21_Gain3;			///< C21_Gain3 (db)

	unsigned long	C22_Freq0;		///< C22_Freq0 (Hz)
	unsigned long	C22_Freq1;		///< C22_Freq1 (Hz)
	unsigned long	C22_Freq2;		///< C22_Freq2 (Hz)
	unsigned long	C22_Freq3;		///< C22_Freq3 (Hz)

	long C22_Gain0;			///< C22_Gain0 (db)
	long C22_Gain1;			///< C22_Gain1 (db)
	long C22_Gain2;			///< C22_Gain2 (db)
	long C22_Gain3;			///< C22_Gain3 (db)

	unsigned long	C23_Freq0;		///< C23_Freq0 (Hz)
	unsigned long	C23_Freq1;		///< C23_Freq1 (Hz)
	unsigned long	C23_Freq2;		///< C23_Freq2 (Hz)
	unsigned long	C23_Freq3;		///< C23_Freq3 (Hz)

	long C23_Gain0;			///< C23_Gain0 (db)
	long C23_Gain1;			///< C23_Gain1 (db)
	long C23_Gain2;			///< C23_Gain2 (db)
	long C23_Gain3;			///< C23_Gain3 (db)

	unsigned long	CadOn24;		///< Cadence On24 time (ms)
	unsigned long	CadOff24;		///< Cadence Off24 time (ms)
	unsigned long	CadOn25;		///< Cadence On25 time (ms)
	unsigned long	CadOff25;		///< Cadence Off25 time (ms)
	unsigned long	CadOn26;		///< Cadence On26 time (ms)
	unsigned long	CadOff26;		///< Cadence Off26 time (ms)
	unsigned long	CadOn27;		///< Cadence On27 time (ms)
	unsigned long	CadOff27;		///< Cadence Off27 time (ms)

	unsigned long	C24_Freq0;		///< C24_Freq0 (Hz)
	unsigned long	C24_Freq1;		///< C24_Freq1 (Hz)
	unsigned long	C24_Freq2;		///< C24_Freq2 (Hz)
	unsigned long	C24_Freq3;		///< C24_Freq3 (Hz)

	long C24_Gain0;			///< C24_Gain0 (db)
	long C24_Gain1;			///< C24_Gain1 (db)
	long C24_Gain2;			///< C24_Gain2 (db)
	long C24_Gain3;			///< C24_Gain3 (db)

	unsigned long	C25_Freq0;		///< C25_Freq0 (Hz)
	unsigned long	C25_Freq1;		///< C25_Freq1 (Hz)
	unsigned long	C25_Freq2;		///< C25_Freq2 (Hz)
	unsigned long	C25_Freq3;		///< C25_Freq3 (Hz)

	long C25_Gain0;			///< C25_Gain0 (db)
	long C25_Gain1;			///< C25_Gain1 (db)
	long C25_Gain2;			///< C25_Gain2 (db)
	long C25_Gain3;			///< C25_Gain3 (db)

	unsigned long	C26_Freq0;		///< C26_Freq0 (Hz)
	unsigned long	C26_Freq1;		///< C26_Freq1 (Hz)
	unsigned long	C26_Freq2;		///< C26_Freq2 (Hz)
	unsigned long	C26_Freq3;		///< C26_Freq3 (Hz)

	long C26_Gain0;			///< C26_Gain0 (db)
	long C26_Gain1;			///< C26_Gain1 (db)
	long C26_Gain2;			///< C26_Gain2 (db)
	long C26_Gain3;			///< C26_Gain3 (db)

	unsigned long	C27_Freq0;		///< C27_Freq0 (Hz)
	unsigned long	C27_Freq1;		///< C27_Freq1 (Hz)
	unsigned long	C27_Freq2;		///< C27_Freq2 (Hz)
	unsigned long	C27_Freq3;		///< C27_Freq3 (Hz)

	long C27_Gain0;			///< C27_Gain0 (db)
	long C27_Gain1;			///< C27_Gain1 (db)
	long C27_Gain2;			///< C27_Gain2 (db)
	long C27_Gain3;			///< C27_Gain3 (db)

	unsigned long	CadOn28;		///< Cadence On28 time (ms)
	unsigned long	CadOff28;		///< Cadence Off28 time (ms)
	unsigned long	CadOn29;		///< Cadence On29 time (ms)
	unsigned long	CadOff29;		///< Cadence Off29 time (ms)
	unsigned long	CadOn30;		///< Cadence On30 time (ms)
	unsigned long	CadOff30;		///< Cadence Off30 time (ms)
	unsigned long	CadOn31;		///< Cadence On31 time (ms)
	unsigned long	CadOff31;		///< Cadence Off31 time (ms)

	unsigned long	C28_Freq0;		///< C28_Freq0 (Hz)
	unsigned long	C28_Freq1;		///< C28_Freq1 (Hz)
	unsigned long	C28_Freq2;		///< C28_Freq2 (Hz)
	unsigned long	C28_Freq3;		///< C28_Freq3 (Hz)

	long C28_Gain0;			///< C28_Gain0 (db)
	long C28_Gain1;			///< C28_Gain1 (db)
	long C28_Gain2;			///< C28_Gain2 (db)
	long C28_Gain3;			///< C28_Gain3 (db)

	unsigned long	C29_Freq0;		///< C29_Freq0 (Hz)
	unsigned long	C29_Freq1;		///< C29_Freq1 (Hz)
	unsigned long	C29_Freq2;		///< C29_Freq2 (Hz)
	unsigned long	C29_Freq3;		///< C29_Freq3 (Hz)

	long C29_Gain0;			///< C29_Gain0 (db)
	long C29_Gain1;			///< C29_Gain1 (db)
	long C29_Gain2;			///< C29_Gain2 (db)
	long C29_Gain3;			///< C29_Gain3 (db)

	unsigned long	C30_Freq0;		///< C30_Freq0 (Hz)
	unsigned long	C30_Freq1;		///< C30_Freq1 (Hz)
	unsigned long	C30_Freq2;		///< C30_Freq2 (Hz)
	unsigned long	C30_Freq3;		///< C30_Freq3 (Hz)

	long C30_Gain0;			///< C30_Gain0 (db)
	long C30_Gain1;			///< C30_Gain1 (db)
	long C30_Gain2;			///< C30_Gain2 (db)
	long C30_Gain3;			///< C30_Gain3 (db)

	unsigned long	C31_Freq0;		///< C31_Freq0 (Hz)
	unsigned long	C31_Freq1;		///< C31_Freq1 (Hz)
	unsigned long	C31_Freq2;		///< C31_Freq2 (Hz)
	unsigned long	C31_Freq3;		///< C31_Freq3 (Hz)

	long C31_Gain0;			///< C31_Gain0 (db)
	long C31_Gain1;			///< C31_Gain1 (db)
	long C31_Gain2;			///< C31_Gain2 (db)
	long C31_Gain3;			///< C31_Gain3 (db)

	//int32	ret_val;
}
TstVoipToneCfg;

/**
 * @brief Structure for disconnection tone detection
 * @see do_mgr_VOIP_MGR_SET_DIS_TONE_DET
 */
typedef struct
{
	unsigned int distone_num;	///< the dis-connect tone number 0, 1, 2, 3, 4

	unsigned int tone1_accuracy;	///< defaut:16, larger value more accuracy , lesser value more tolerance in frequency
	unsigned int tone1_level;	///< default:800, larger valre need more amplitude
	unsigned int tone1_frequency1; ///< the first dis-connect tone detect frequency unit: Hz. samling rate = 8000hz, allow 1~ 3999
	unsigned int tone1_frequency2; ///< the second dis-connect tone detect frequency unit: Hz. samling rate = 8000hz, allow 0~ 3999
	                         /*  0 for only one frequency. */
	int tone1_distone_on_low_limit; ///< the lower limit of dis-tone ON time unit:10ms, ex 0.5s=500ms = 50unit
	int tone1_distone_on_up_limit;  ///< the upper limit of dis-tone ON time unit:10ms
	int tone1_distone_off_low_limit; ///< the lower limit of dis-tone OFF time unit:10ms
	int tone1_distone_off_up_limit;  ///< the upper limit of dis-tone OFF time unit:10ms

	unsigned int tone2_accuracy;	///< defaut:16, larger value more accuracy , lesser value more tolerance in frequency
	unsigned int tone2_level;	///< default:800, larger valre need more amplitude
	unsigned int tone2_frequency1; ///< the first dis-connect tone detect frequency unit: Hz. samling rate = 8000hz, allow 1~ 3999
	unsigned int tone2_frequency2; ///<the second dis-connect tone detect frequency unit: Hz. samling rate = 8000hz, allow 0~ 3999
	                         /*  0 for only one frequency. */
	int tone2_distone_on_low_limit; ///< the lower limit of dis-tone ON time unit:10ms, ex 0.5s=500ms = 50unit
	int tone2_distone_on_up_limit;  ///< the upper limit of dis-tone ON time unit:10ms
	int tone2_distone_off_low_limit; ///< the lower limit of dis-tone OFF time unit:10ms
	int tone2_distone_off_up_limit;  ///< the upper limit of dis-tone OFF time unit:10ms

	unsigned int tone3_accuracy;	///< defaut:16, larger value more accuracy , lesser value more tolerance in frequency
	unsigned int tone3_level;	///< default:800, larger valre need more amplitude
	unsigned int tone3_frequency1; ///< the first dis-connect tone detect frequency unit: Hz. samling rate = 8000hz, allow 1~ 3999
	unsigned int tone3_frequency2; ///<the second dis-connect tone detect frequency unit: Hz. samling rate = 8000hz, allow 0~ 3999
	                         /*  0 for only one frequency. */
	int tone3_distone_on_low_limit; ///< the lower limit of dis-tone ON time unit:10ms, ex 0.5s=500ms = 50unit
	int tone3_distone_on_up_limit;  ///< the upper limit of dis-tone ON time unit:10ms
	int tone3_distone_off_low_limit; ///< the lower limit of dis-tone OFF time unit:10ms
	int tone3_distone_off_up_limit;  ///< the upper limit of dis-tone OFF time unit:10ms

	unsigned int tone4_accuracy;	///< defaut:16, larger value more accuracy , lesser value more tolerance in frequency
	unsigned int tone4_level;	///< default:800, larger valre need more amplitude
	unsigned int tone4_frequency1; ///< the first dis-connect tone detect frequency unit: Hz. samling rate = 8000hz, allow 1~ 3999
	unsigned int tone4_frequency2; ///<the second dis-connect tone detect frequency unit: Hz. samling rate = 8000hz, allow 0~ 3999
	                         /*  0 for only one frequency. */
	int tone4_distone_on_low_limit; ///< the lower limit of dis-tone ON time unit:10ms, ex 0.5s=500ms = 50unit
	int tone4_distone_on_up_limit;  ///< the upper limit of dis-tone ON time unit:10ms
	int tone4_distone_off_low_limit; ///< the lower limit of dis-tone OFF time unit:10ms
	int tone4_distone_off_up_limit;  ///< the upper limit of dis-tone OFF time unit:10ms

	//int32	ret_val;
}
TstVoipdistonedet_parm;

#define MAX_LEN_OF_IVR_TEXT		40
/**
 * @brief Structure for playing textual IVR
 * @see TstVoipPlayIVR_Header
 */
typedef struct {
	unsigned char ch_id;					///< Channel ID 
	unsigned char m_id;					///< Media ID 
	IvrPlayType_t type;					///< Type of codec == IVR_PLAY_TYPE_TEXT 
	IvrPlayDir_t dir;					///< Play IVR locally or remotely 
	unsigned int playing_period_10ms;			///< How much data in buffer [output]
	unsigned int ret1_unused;				///< Unused parameter [output]
	/* --- Above part should be identical to TstVoipPlayIVR_Header --- */
	unsigned char szText2speech[ MAX_LEN_OF_IVR_TEXT + 1 ];	///< String for playing 
	//int32	ret_val;
} TstVoipPlayIVR_Text;

#define MAX_FRAMES_OF_G72363	10
/**
 * @brief Structure for playing G.723-6.3k IVR
 * @note nRetCopiedFrames should be smaller to equal to nFramesCount
 * @see TstVoipPlayIVR_Header TstVoipPlayIVR_Header_v2
 */
typedef struct {
	unsigned char ch_id;					///< Channel ID 
	unsigned char m_id;					///< Media ID 
	IvrPlayType_t type;					///< Type of codec == IVR_PLAY_TYPE_G723_63 
	IvrPlayDir_t dir;					///< Play IVR locally or remotely 
	unsigned int playing_period_10ms;		   	///< How much data in buffer [output]
	unsigned int nRetCopiedFrames;				///< Number of frames are copied, so the residual has to be given again [output] 
	/* --- Above part should be identical to TstVoipPlayIVR_Header --- */
	unsigned int  nFramesCount;				///< Number of frames in data buffer 
	unsigned char data[ 24 * MAX_FRAMES_OF_G72363 ];		///< G.723 6.3k data 
	/* --- Above part should be identical to TstVoipPlayIVR_Header_v2 --- */
	//int32	ret_val;
} TstVoipPlayIVR_G72363;

#define MAX_FRAMES_OF_G729	10
/**
 * @brief Structure for playing G.729 IVR
 * @note nRetCopiedFrames should be smaller to equal to nFramesCount
 * @see TstVoipPlayIVR_Header TstVoipPlayIVR_Header_v2
 */
typedef struct {
	unsigned char ch_id;					///< Channel ID 
	unsigned char m_id;					///< Media ID 
	IvrPlayType_t type;					///< Type of codec == IVR_PLAY_TYPE_G729 
	IvrPlayDir_t dir;					///< Play IVR locally or remotely 
	unsigned int playing_period_10ms;	                ///< How much data in buffer [output]
	unsigned int nRetCopiedFrames;		                ///< Number of frames are copied, so the residual has to be given again [output]
	/* --- Above part should be identical to TstVoipPlayIVR_Header --- */
	unsigned int  nFramesCount;				///< Number of frames in data buffer
	unsigned char data[ 10 * MAX_FRAMES_OF_G729 ];		///< G.729 data 
	/* --- Above part should be identical to TstVoipPlayIVR_Header_v2 --- */
	//int32	ret_val;
} TstVoipPlayIVR_G729;

#define MAX_FRAMES_OF_G711	10
/**
 * @brief Structure for playing G.711 IVR
 * @note nRetCopiedFrames should be smaller to equal to nFramesCount
 * @see TstVoipPlayIVR_Header TstVoipPlayIVR_Header_v2
 */
typedef struct {
	unsigned char ch_id;					///< Channel ID 
	unsigned char m_id;					///< Media ID 
	IvrPlayType_t type;					///< Type of codec == IVR_PLAY_TYPE_G711A
	IvrPlayDir_t dir;					///< Play IVR locally or remotely 
	unsigned int playing_period_10ms;	                ///< How much data in buffer [output]
	unsigned int nRetCopiedFrames;		                ///< Number of frames are copied, so the residual has to be given again [output]
	/* --- Above part should be identical to TstVoipPlayIVR_Header --- */
	unsigned int  nFramesCount;				///< Number of frames in data buffer
	unsigned char data[ 80 * MAX_FRAMES_OF_G711 ];          ///< G.711 data 
	/* --- Above part should be identical to TstVoipPlayIVR_Header_v2 --- */
	//int32	ret_val;
} TstVoipPlayIVR_G711;

#define MAX_FRAMES_OF_G722	10
/**
 * @brief Structure for playing G.722 IVR
 * @note nRetCopiedFrames should be smaller to equal to nFramesCount
 * @see TstVoipPlayIVR_Header TstVoipPlayIVR_Header_v2
 */
typedef struct {
	unsigned char ch_id;					///< Channel ID 
	unsigned char m_id;					///< Media ID 
	IvrPlayType_t type;					///< Type of codec == IVR_PLAY_TYPE_G722
	IvrPlayDir_t dir;					///< Play IVR locally or remotely 
	unsigned int playing_period_10ms;	                ///< How much data in buffer [output]
	unsigned int nRetCopiedFrames;		                ///< Number of frames are copied, so the residual has to be given again [output]
	/* --- Above part should be identical to TstVoipPlayIVR_Header --- */
	unsigned int  nFramesCount;				///< Number of frames in data buffer
	unsigned char data[ 80 * MAX_FRAMES_OF_G722 ];          ///< G.722 data
	/* --- Above part should be identical to TstVoipPlayIVR_Header_v2 --- */
	//int32	ret_val;
} TstVoipPlayIVR_G722;

#define MAX_FRAMES_OF_LINEAR_8K	10
/**
 * @brief Structure for playing Linear 8k IVR
 * @note nRetCopiedFrames should be smaller to equal to nFramesCount
 * @see TstVoipPlayIVR_Header TstVoipPlayIVR_Header_v2
 */
typedef struct {
	unsigned char ch_id;					///< Channel ID                                                                 	
	unsigned char m_id;					///< Media ID 
	IvrPlayType_t type;					///< Type of codec == IVR_PLAY_TYPE_LINEAR_8K
	IvrPlayDir_t dir;					///< Play IVR locally or remotely 
	unsigned int playing_period_10ms;	                ///< How much data in buffer [output]
	unsigned int nRetCopiedFrames;		                ///< Number of frames are copied, so the residual has to be given again [output]
	/* --- Above part should be identical to TstVoipPlayIVR_Header --- */
	unsigned int  nFramesCount;				///< Number of frames in data buffer	
	unsigned char data[ 160 * MAX_FRAMES_OF_LINEAR_8K ];    ///< linear 8k data 
	/* --- Above part should be identical to TstVoipPlayIVR_Header_v2 --- */
	//int32	ret_val;
} TstVoipPlayIVR_8K;

#define MAX_FRAMES_OF_LINEAR_16K	10
/**
 * @brief Structure for playing Linear 16k IVR
 * @note nRetCopiedFrames should be smaller to equal to nFramesCount
 * @see TstVoipPlayIVR_Header TstVoipPlayIVR_Header_v2
 */
typedef struct {
	unsigned char ch_id;					///< Channel ID                                                                 	
	unsigned char m_id;					///< Media ID 
	IvrPlayType_t type;					///< Type of codec == IVR_PLAY_TYPE_LINEAR_16K
	IvrPlayDir_t dir;					///< Play IVR locally or remotely 
	unsigned int playing_period_10ms;	                ///< How much data in buffer [output]
	unsigned int nRetCopiedFrames;		                ///< Number of frames are copied, so the residual has to be given again [output]
	/* --- Above part should be identical to TstVoipPlayIVR_Header --- */
	unsigned int  nFramesCount;				///< Number of frames in data buffer	
	unsigned char data[ 320 * MAX_FRAMES_OF_LINEAR_16K ];   ///< linear 16k data 
	/* --- Above part should be identical to TstVoipPlayIVR_Header_v2 --- */
	//int32	ret_val;
} TstVoipPlayIVR_16K;

/**
 * @brief General structure header for playing IVR
 * @see do_mgr_VOIP_MGR_PLAY_IVR()
 */
typedef struct {
	unsigned char ch_id;					///< Channel ID    
	unsigned char m_id;					///< Media ID 
	IvrPlayType_t type;                                     ///< Type of codec 
	IvrPlayDir_t dir;					///< Play IVR locally or remotely 
	unsigned int playing_period_10ms;			///< How much data in buffer [output]
	unsigned int ret1;					///< Used by various codec [output], each codec has different meaning 
	//int32	ret_val;
} TstVoipPlayIVR_Header;

/**
 * @brief General structure header version 2 for playing IVR
 * @note nRetCopiedFrames should be smaller to equal to nFramesCount
 * @see do_mgr_VOIP_MGR_PLAY_IVR() TstVoipPlayIVR_Header
 */
typedef struct {
	unsigned char ch_id;					///< Channel ID    
	unsigned char m_id;					///< Media ID 
	IvrPlayType_t type;                                     ///< Type of codec 
	IvrPlayDir_t dir;					///< Play IVR locally or remotely 
	unsigned int playing_period_10ms;	                ///< How much data in buffer [output]
	unsigned int nRetCopiedFrames;				///< Number of frames are copied, so the residual has to be given again [output]
	/* --- Above part should be identical to TstVoipPlayIVR_Header --- */
	unsigned int  nFramesCount;				///< Number of frames in data buffer 
	unsigned char data[ 1 ];					///< Data for each codec 
} TstVoipPlayIVR_Header_v2;

/**
 * @brief Structure for polling IVR
 * @see do_mgr_VOIP_MGR_POLL_IVR()
 */
typedef struct {
	unsigned char ch_id;					///< Channel ID   
	unsigned char m_id;					///< Media ID 
	unsigned char bPlaying;					///< If it is playing, 1 is given. Otherwise, 0 is given 
	//int32	ret_val;
} TstVoipPollIVR;


/**
 * @brief Structure for stoping IVR
 * @see do_mgr_VOIP_MGR_STOP_IVR()
 */
typedef struct {
	unsigned char ch_id;					///< Channel ID   
	unsigned char m_id;					///< Media ID 
	//int32	ret_val;
} TstVoipStopIVR;

/**
 * @brief Structure for T38 PCM-in debug 
 * @note These code are comment! 
 * @see do_mgr_VOIP_MGR_GET_T38_PCMIN()
 */
typedef struct {
	unsigned short pcmIn[ 480 ];				///< Input PCM data 
	unsigned int snPcm;                                     ///< Sequence number of PCM data 
	unsigned int ret;                                       ///< Number of retrieved PCM data 
	//int32	ret_val;
} TstT38PcmIn;

/**
 * @brief Structure for T38 packet-in debug 
 * @note These code are comment! 
 * @see do_mgr_VOIP_MGR_GET_T38_PACKETIN()
 */
typedef struct {
	unsigned short packetIn[ 300 ];				///< Input packet data 
	unsigned int nSize;                                     ///< Size of packet 
	unsigned int snPcm;                                     ///< Sequence number of PCM data used to keep input timing 
	//int32	ret_val;
} TstT38PacketIn;

/**
 * @brief Structure for voice record debug 
 * @note mode Dump mode whose format is<br>
 *        DUMP_MODE_3 | [RECORD_PCM_HANDLER | RECORD_TX_SINE_RX_ECHO | RECORD_PCM_INTERFACE | RECORD_PCM_LEC]<br>
 *        Where, DUMP_MODE_3: 16, RECORD_PCM_HANDLER: 1, RECORD_TX_SINE_RX_ECHO: 2, RECORD_PCM_INTERFACE: 4, RECORD_PCM_LEC: 8 <br>  
 *        Using this debug function, one has to uncomment RTK_VOICE_RECORD in rtk_voip.h and <br>
 *       VOIPBOX_VOICERECORD in voipbox.
 * @see do_mgr_VOIP_MGR_SET_GETDATA_MODE()
 */
typedef struct
{
	long ch_id;						///< Channel ID 
	long write_enable;                                      ///< Initial writing. (0: initial, 1: do nothing)
	long length;                                            ///< Retrieved voice data length 
	long mode;                               		///< Dump mode whose format 
	char buf[1120];                                         ///< Retrieved voice data 
	//int32	ret_val;
} TstVoipdataget_o;

typedef struct
{
	long tx_readindex;					///< TX data read index
	long tx_writeindex;					///< Rx data write index     
	long write_enable;                                      ///< Write enable
	long rx_readindex;                                      ///< RX data read index  
	long rx_writeindex;                                     ///< Rx data write index 
	long rx_readindex2;                                     ///< RX data read index2   
	long rx_writeindex2;                                    ///< Rx data write index2 
	char *txbuf;						///< tx buffer
	char *rxbuf;						///< rx buffer
	char *rxbuf2;						///< rx2 buffer
	//int32	ret_val;
}
TstVoipdataget;


typedef struct
{
	long ch_id;						///< Channel ID 
	long write_enable;					///< Write enable
	long length;						///< Data length
	long mode;						///< Mode
	int32	ret_val;
	char buf[1120];						///< output buffer
} TstVoipdataput_o;

typedef struct
{
	long readindex;						///< data read index
	long writeindex;						///< data write index
	long write_enable;                                      ///< Write enable
	char *txbuf;						///< tx buffer
	//int32	ret_val;
}
TstVoipdataput;

/**
 * @brief Structure for retrieving RTP statistics 
 */
typedef struct {
	unsigned char ch_id;					 ///< [in] Channel ID 
	unsigned char bResetStatistics;			         ///< [in] Reset statistics 
	/* follows are output */
	unsigned long nRxRtpStatsCountByte;			 ///< [out] Number of Rx RTP bytes 
	unsigned long nRxRtpStatsCountPacket;                    ///< [out] Number of Rx RTP packets 
	unsigned long nRxRtpStatsLostPacket;                     ///< [out] Number of Rx packet loss
	unsigned long nTxRtpStatsCountByte;                      ///< [out] Number of Tx RTP bytes 
	unsigned long nTxRtpStatsCountPacket;                    ///< [out] Number of Tx RTP packets 
	//int32	ret_val;
} TstVoipRtpStatistics;

/**
 * @brief Structure for retrieving statistics by session 
 */
typedef struct {
	unsigned char ch_id;						///< [input] Channel ID 
	unsigned char m_id;						///< [input] Media ID 
	unsigned char bResetStatistics;					///< [input] Reset statistics 
	/* follows are output */
	unsigned long nRxSilencePacket;                                 ///< Number of silence packet 
	unsigned long nTxSilencePacket;                                 ///< Number of silence packet 
	unsigned long nAvgPlayoutDelay;                                 ///< Average playout delay (in unit of 1ms)
	unsigned long nCurrentJitterBuf;                                ///< Current jitter buffering packets 
	unsigned long nEarlyPacket;                                     ///< Early packet for playout
	unsigned long nLatePacket;                                      ///< Late packet for playout 
	unsigned long nSilenceSpeech;                                   ///< Count of playout a silence speech (in unit of 10ms)
	//int32	ret_val;
} TstVoipSessionStatistics;

/**
 * @brief Structure for retrieving RTP/RTCP statistics
 * @see do_mgr_VOIP_MGR_GET_RTP_RTCP_STATISTICS()
 */
typedef struct
{
	unsigned char	ch_id;					///< [input] Channel ID 
	unsigned char	m_id;					///< [input] Media ID 
	unsigned char	bResetStatistics;			///< [input] Reset statistics 
	/* follows are output */
	unsigned long	nTxPkts;           			///< packets sent
	unsigned long	nTxBytes;          			///< octets sent
	unsigned long	nRxPkts;           			///< packets received
	unsigned long	nRxBytes;          			///< octets received
	unsigned long 	nLost;             			///< Number of packets lost
	unsigned long 	nMaxFractionLost;             		///< Max. Fraction packets lost
	unsigned long 	nMinFractionLost;             		///< Min. Fraction packets lost
	unsigned long 	nAvgFractionLost;             		///< Avg. Fraction packets lost
	unsigned long 	nCurFractionLost;             		///< Current Fraction packets lost
	unsigned long	nDiscarded;        			///< discarded packets
	unsigned long	nTxRtcpPkts;       			///< Number of ingress RTCP packets
	unsigned long	nRxRtcpPkts;       			///< Number of egress RTCP packets
	unsigned short	nTxRtcpXrPkts;     			///< Number of egress RTCP XR packets
	unsigned short	nRxRtcpXrPkts;     			///< Number of egress RTCP XR packets
	unsigned short	nOverRuns;         			///< Number of jitter buffer overruns
	unsigned short	nUnderRuns;        			///< Number of jitter buffer underruns
	unsigned long	nMaxJitter;           			///< Max. Interarrival jitter estimate from RTP service (in ms)
	unsigned long	nMinJitter;           			///< Min. Interarrival jitter estimate from RTP service (in ms)
	unsigned long	nAvgJitter;           			///< Avg. Interarrival jitter estimate from RTP service (in ms)
	unsigned long	nCurJitter;           			///< Current Interarrival jitter estimate from RTP service (in ms)
	unsigned long	nMaxRtcpTime;	  			///< maximum time between RTCP packets
	//int32	ret_val;
}TstRtpRtcpStatistics;


typedef struct
{
	unsigned int event;						///< event
	unsigned int marker;                                            ///< marker
	unsigned int duration;                                          ///< duration
	unsigned int volume;                                            ///< volume
	unsigned int edge;                                              ///< edge
	unsigned int play;      						///< for check if play or not  
	//int32	ret_val;
}
TstRfc2833event;

typedef struct
{
	Voip_reosurce_t resource;					///< VoIP resource
	TstVoipMgr3WayCfg _3way_session_info;                           ///< 3-way session information
	//int32	ret_val;
} TstVoipResourceCheck;

/**
 * @brief Structure for port link status.
 * @param status Port link status, and its value is resolved by following:<br>
 *        PORT_LINK_LAN_ALL		0x0000000F	<br>
 *        PORT_LINK_LAN0		0x00000001	<br>
 *        PORT_LINK_LAN1		0x00000002	<br>
 *        PORT_LINK_LAN2		0x00000004	<br>
 *        PORT_LINK_LAN3		0x00000008	<br>
 *        PORT_LINK_RESERVED1	0x000000F0	<br>
 *        PORT_LINK_WAN			0x00000100	<br>
 * @see do_mgr_VOIP_MGR_GET_PORT_LINK_STATUS()
 */
 
/**
 * @brief Structure for VoIP port link status.
 */ 
typedef struct {
	uint32 status;							///< Status
} TstVoipPortLinkStatus;

/**
 * @brief Structure for GPIO read, write and initialization
 * @see do_mgr_VOIP_MGR_GPIO()
 */
// rock 2008.1.16
typedef struct {
	unsigned int action;						///< init=0, read=1, write=2
	unsigned int pid;						///< GPIO id = (gpio_group << 16) | (gpip_pin & 0xffff)
	/** @brief 
	 * For GPIO read/write:<br>
     	 * -Bit 0: Disable = 0, Enable = 1<br>
     	 * For GPIO init:<br>
     	 * -Bit0~1: GPIO_PERIPHERAL			<br>
     	 * -Bit2: GPIO_DIRECTION				<br>
     	 * -Bit3: GPIO_INTERRUPT_TYPE			<br> 
 	 */	
	unsigned int value;						///< GPIO value
	unsigned int result;						///< 0: Success, 1: Failed
	//int32	ret_val;
} TstVoipGPIO;

/**
 * @brief Structure for LED display  
 * @see do_mgr_VOIP_MGR_SET_LED_DISPLAY() LedDisplayMode
 */
typedef struct {
	uint32 ch_id;							///< Channel ID
	uint32 led_id;							///< Each channel has at most 2 LED, and we use 0 normally. 
	LedDisplayMode mode;                                            ///< LED display mode. It may be off/on/blinking 
} TstVoipLedDisplay;

/**
 * @brief Structure for SLIC relay 
 * @see do_mgr_VOIP_MGR_SET_SLIC_RELAY()
 */
typedef struct {
	uint32 ch_id;							///< Channel ID 
	uint32 close;							///< A boolean value to indicate circuit between SLIC and phone is close(1) or open(0)
} TstVoipSlicRelay;

/**
 * @brief Structure for bus data format 
 * @see do_mgr_VOIP_MGR_SET_BUS_DATA_FORMAT()
 */
typedef struct {
	uint32 ch_id;							///< Channel ID
	AP_BUS_DATA_FORMAT format;					///< Data format 
} TstVoipBusDataFormat;

/**
 * @brief Structure for PCM timeslot 
 * @see do_mgr_VOIP_MGR_SET_PCM_TIMESLOT()
 */
typedef struct {
	uint32 ch_id;							///< Channel ID 
	uint32 timeslot1;						///< Timeslot for narrowband 
	uint32 timeslot2;						///< Combine with timeslot1 to be wideband 
									///< (narrowband ignore this one)
} TstVoipPcmTimeslot;

/**
 * @brief Structure for single value ioctl
 * @see do_mgr_VOIP_MGR_DECT_SET_POWER()
 * @see do_mgr_VOIP_MGR_DECT_GET_POWER()
 * @see do_mgr_VOIP_MGR_DECT_GET_PAGE()
*/
typedef struct
{
	uint32 value;							///< Value
} TstVoipSingleValue;

/**
 * @brief Structure for DTMF detection parameter ioctl
 * @see do_mgr_VOIP_MGR_DTMF_CFG()
 * @see do_mgr_VOIP_MGR_DTMF_DET_PARAM()
*/
typedef struct
{
	unsigned int ch_id;				///< Channel ID
	unsigned int m_id;				///< Reserved field for future used (Media ID)
	unsigned int t_id;				///< Reserved field for future used (Transaction (active) ID)
	unsigned int enable;			///< DTMF det enable or disable,  0-> off, 1->ON
	unsigned int dir;				///< DTMF det direction, 0-> TDM, 1-> IP
	unsigned int thres_upd;			///< Update DTMF det threshold flag, 0-> no update, 1-> update
	unsigned int thres;				///< DTMF det threshold, Available range: 0 to 40 ( 0 ~ -40 dBm)
	unsigned int on_time_upd;		///< Update DTMF det on time flag, 0-> no update, 1-> update
	unsigned int on_time;			///< DTMF det on time (uint: 10ms)
	unsigned int twist_upd;			///< Update DTMF det acceptable twist flag, 0-> no update, 1-> updates
	unsigned int fore_twist;		///< DTMF det acceptable fore-twist (dB), Available range: 1 to 12 dB
	unsigned int rev_twist;			///< DTMF det acceptable rev-twist (dB), Available range: 1 to 12 dB
	unsigned int freq_offset_upd;	///< Update DTMF freq. offset flag, 0-> no update, 1-> update
	unsigned int freq_offset;		///< DTMF freq. offset support flag, 0-> not support, 1-> support

} TstDtmfDetPara;

/**
 * @brief Structure for cadence parameter ioctl
 * @see do_mgr_VOIP_MGR_SET_MULTI_RING_CADENCE()
*/
typedef struct
{
	uint32  ch_id;		///< Channel ID
	uint32  m_id;		///< Media ID
	uint16	cadon1;		///< Cadence On1 time (ms)
	uint16	cadoff1;	///< Cadence On1 time (ms)
	uint16	cadon2;		///< Cadence On2 time (ms)
	uint16	cadoff2;	///< Cadence On2 time (ms)
	uint16	cadon3;		///< Cadence On3 time (ms)
	uint16	cadoff3;	///< Cadence On3 time (ms)
	uint16	cadon4;		///< Cadence On4 time (ms)
	uint16  cadoff4;	///< Cadence On4 time (ms)
}
TstVoipCadence;

#ifdef CONFIG_RTK_VOIP_IP_PHONE
 #ifdef __KERNEL__
  #include <linux/types.h>	// pid_t
 #else
  #include <sys/types.h>	// pid_t
 #endif

/* ------- We list keypad control in follow ------- */
typedef unsigned short wkey_t;	/* wide key */

/**
 * @brief General structure header for keypad
 * @see TstKeypadCtl do_mgr_VOIP_MGR_CTL_KEYPAD()
 */
typedef struct TstKeypadGeneral_s {
	keypad_cmd_t cmd;						///< Command type
} TstKeypadGeneral;

/**
 * @brief Structure for giving a target process
 * @see TstKeypadCtl TstKeypadGeneral
 */
typedef struct TstKeypadSetTarget_s {
	keypad_cmd_t	cmd;						///< Command type, and it has to be KEYPAD_CMD_SET_TARGET
	pid_t		pid;						///< Target process ID
} TstKeypadSetTarget;

/**
 * @brief Structure for debuging keypad function
 * @see TstKeypadCtl TstKeypadGeneral
 */
typedef struct TstKeypadSignalDebug_s {
	keypad_cmd_t	cmd;						// Command type, and it has to be KEYPAD_CMD_SIG_DEBUG
	wkey_t		wkey;						// Key ID. This ID will be put into key buffer, so another function can read it normally.
} TstKeypadSignalDebug;

/**
 * @brief Structure for debuging keypad function
 * @param cmd Command type, and it has to be KEYPAD_CMD_READ_KEY
 * @param wkey Key ID
 * @param validKey Determine whether wkey field is valid or not
 * @see TstKeypadCtl TstKeypadGeneral
 */
typedef struct TstKeypadReadKey_s {
	keypad_cmd_t	cmd;	// = KEYPAD_CMD_READ_KEY
	wkey_t			wkey;
	unsigned char	validKey;
} TstKeypadReadKey;

/**
 * @brief Structure for debuging keypad function
 * @param cmd Command type, and it has to be KEYPAD_CMD_HOOK_STATUS
 * @param wkey status Get hook status
 * @see TstKeypadCtl TstKeypadGeneral
 */
typedef struct TstKeypadHookStatus_s {
	keypad_cmd_t	cmd;	// = KEYPAD_CMD_HOOK_STATUS
	unsigned char	status;	/* 0: on-hook, 1: off-hook */
} TstKeypadHookStatus;

/**
 * @brief Structure for keypad
 * @see do_mgr_VOIP_MGR_CTL_KEYPAD()
 */
typedef union {
	TstKeypadGeneral		General;		//!< General header
	TstKeypadSetTarget		SetTarget;		//!< Set application target PID
	TstKeypadSignalDebug	SignalDebug;	//!< Debug purpose. Put a key into buffer.
	TstKeypadReadKey		ReadKey;		//!< Read a key
	TstKeypadHookStatus		HookStatus;		//!< Read hook status
} TstKeypadCtl;

/* ------- We list LCM control in follow ------- */
/**
 * @brief Structure for LCM display on or off
 * @param cmd Command type. It has to be LCD_CMD_DISPLAY_ON_OFF.
 * @param bDisplayOnOff Display on or off. (1: on, 0: off)
 * @param bCursorOnOff Cursor on or on. (1: on, 0: off)
 * @param bCursorBlink Cursor blink. (1: on, 0: off)
 * @see TstLcmCtl TstLcmGeneral
 */
typedef struct TstLcmDisplayOnOff_s {
	lcm_cmd_t		cmd;	// = LCD_CMD_DISPLAY_ON_OFF
	unsigned char	bDisplayOnOff;	/* 1: on, 0: off, others: invalid */
	unsigned char	bCursorOnOff;	/* 1: on, 0: off, others: invalid */
	unsigned char	bCursorBlink;	/* 1: on, 0: off, others: invalid */
} TstLcmDisplayOnOff;

/**
 * @brief Structure for LCM cursor position
 * @param cmd Command type. It has to be LCM_CMD_MOVE_CURSOR_POS.
 * @param x Cursor position in X-axis
 * @param y Cursor position in Y-axis
 * @note Character mode only
 * @see TstLcmCtl TstLcmGeneral
 */
typedef struct TstLcmMoveCursorPosition_s {
	lcm_cmd_t		cmd;	// = LCM_CMD_MOVE_CURSOR_POS
	int				x;
	int				y;
} TstLcmMoveCursorPosition;

#define MAX_LEN_OF_DRAW_TEXT	16	/* NOT include null-terminator */
/**
 * @brief Structure for drawing text
 * @param cmd Command type. It has to be LCM_CMD_DRAW_TEXT.
 * @param x Text position in X-axis
 * @param y Text position in Y-axis
 * @param szText Text buffer
 * @param len Text length in buffer
 * @note Character mode only
 * @see TstLcmCtl TstLcmGeneral
 */
typedef struct TstLcmDrawText_s {
	lcm_cmd_t		cmd;	// = LCM_CMD_DRAW_TEXT
	int				x;
	int				y;
	unsigned char 	szText[ MAX_LEN_OF_DRAW_TEXT ];
	int				len;
} TstLcmDrawText;

#define MAX_LEN_OF_WRITE_DATA	160	/* for 80x16 bits LCD */
/**
 * @brief Structure for writing data
 * @param cmd Command type. It has to be LCM_CMD_WRITE_DATA.
 * @param start Start position of VRAM
 * @param pixels Pixels data
 * @param len Length in pixels parameter
 * @note Graphic mode only
 * @see TstLcmCtl TstLcmGeneral
 */
typedef struct TstLcmWriteData_s {
	lcm_cmd_t		cmd;	// = LCM_CMD_WRITE_DATA
	int				start;
	unsigned char 	pixels[ MAX_LEN_OF_WRITE_DATA ];
	int				len;
} TstLcmWriteData;

#define MAX_LEN_OF_WRITE_DATA2	132	/* for 132*65 bits LCD */
/**
 * @brief Structure for writing data (mode 2)
 * @param cmd Command type. It has to be LCM_CMD_WRITE_DATA2.
 * @param page Start page
 * @param col Start column
 * @param pixels Pixels data
 * @param len Length in pixels parameter
 * @note Graphic mode only
 * @see TstLcmCtl TstLcmGeneral
 */
typedef struct TstLcmWriteData2_s {
	lcm_cmd_t		cmd;	// = LCM_CMD_WRITE_DATA2
	int				page;
	int				col;
	unsigned char 	pixels[ MAX_LEN_OF_WRITE_DATA2 ];
	int				len;
} TstLcmWriteData2;

/**
 * @brief Structure for LCM refresh from MMAP
 * @param cmd Command type. It has to be LCM_CMD_DIRTY_MMAP.
 * @param start Start position of VRAM
 * @param len Refresh length after start position
 * @note Graphic mode only
 * @note Application has to use MMAP as its VRAM first.
 * @see TstLcmCtl TstLcmGeneral
 */
typedef struct TstLcmDirtyMmap_s {
	lcm_cmd_t		cmd;	// = LCM_CMD_DIRTY_MMAP
	int				start;
	int				len;
} TstLcmDirtyMmap;

/**
 * @brief Structure for LCM refresh from MMAP (mode 2)
 * @param cmd Command type. It has to be LCM_CMD_DIRTY_MMAP2.
 * @param page Start page
 * @param col Start column
 * @param len Refresh length after start position
 * @param rows Refresh rows number
 * @note Graphic mode only
 * @note Application has to use MMAP as its VRAM first.
 * @see TstLcmCtl TstLcmGeneral
 */
typedef struct TstLcmDirtyMmap2_s {
	lcm_cmd_t		cmd;	// = LCM_CMD_DIRTY_MMAP2
	int				page;
	int				col;
	int				len;
	int				rows;
} TstLcmDirtyMmap2;

/**
 * @brief General structure header for LCM
 * @param cmd Command type.
 * @see TstLcmCtl TstLcmGeneral
 */
typedef struct TstLcmGeneral_s {
	lcm_cmd_t		cmd;
} TstLcmGeneral;

/**
 * @brief Structure for LCM
 * @see do_mgr_VOIP_MGR_CTL_LCM()
 */
typedef union TstLcmCtl_s {
	TstLcmGeneral				General;			//!< General header
	TstLcmDisplayOnOff			DisplayOnOff;		//!< Display on/off
	TstLcmMoveCursorPosition	MoveCursorPosition;	//!< Move cursor (textual mode)
	TstLcmDrawText				DrawText;			//!< Draw text (textual mode)
	TstLcmWriteData				WriteData;			//!< Write data (graphic mode)
	TstLcmWriteData2			WriteData2;			//!< Write data (graphic mode)
	TstLcmDirtyMmap				DirtyMmap;			//!< Refresh from MMAP (graphic mode)
	TstLcmDirtyMmap2			DirtyMmap2;			//!< Refresh from MMAP (graphic mode)
} TstLcmCtl;

/* ------- Control voice path ------- */
/**
 * @brief Structure for voice patch control
 * @param vpath Voice path
 * @see do_mgr_VOIP_MGR_CTL_VOICE_PATH()
 */
typedef struct TstVoicePath_s {
	vpath_t	vpath;
} TstVoicePath_t;

/* ------- Control LED display ------- */
/**
 * @brief Structure for LED display
 * @param led Each bit represents a LED
 * @see do_mgr_VOIP_MGR_CTL_LED()
 */
typedef struct TstLedCtl_s {
	unsigned long led;
} TstLedCtl;

/* ------- IP phone miscellanous options ------- */
/**
 * @brief Structure for retrieving miscellanous options
 * @param buildno Build number
 * @param builddate Build date
 * @see do_mgr_VOIP_MGR_CTL_MISC()
 */
typedef struct TstMiscCtl_s {
	unsigned long buildno;
	unsigned long builddate;
} TstMiscCtl_t;
#endif /* CONFIG_RTK_VOIP_IP_PHONE */

#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
/**
 * @brief Structure for IP phone debug
 * @param function_type Function type. It can be <br>
 *        0: codec write, 1: codec read, <br>
 *        2: codec loopback on, 3: codec loopback off, <br>
 *        4: LED test
 * @param reg Register number
 * @param value Register value
 * @note Use 'iphone_test' to debug
 * @see do_mgr_VOIP_MGR_IPHONE_TEST()
 */
typedef struct
{
	unsigned int function_type;
	unsigned int reg;
	unsigned int value;
} IPhone_test;
#endif

typedef struct
{
	long mode;	// 0:init_to lec, 1:echo canceller, 2:init_to aec
	short buf1[128];	//input,
	short buf2[128];	//echo,
	//int32	ret_val;
}
TstVoipEcDebug;

typedef struct
{
	uint8 dscpRemark;		///< DSCP Remark value
	uint8 vlanPriorityRemark;	///< VLAN Priority Remakr value
	uint8 queueNum;			///< Queue Number setting
}
TstQosCfg;

typedef struct
{
	uint8 chid;					///< Channel ID
	uint8 sid;					///< Session ID
	SessionType sessiontype;	///< Session type
	TstQosCfg qos;				///< QoS Config
}
TstRtpQosRemark;

typedef struct
{
	uint8 chid;					///< Channel ID
	uint16 hz;					///< Choose 12K or 16K, Please input 12 or 16
	uint16 onTime;				///< Pulse on time in 10ms increments
	uint16 offTime;				///< Pulse off time in 10ms increments
	uint16 numMeters;			///< Number of meter cycles to perform
}
TstVoipSLICMeterPulse;

typedef struct
{
	uint8 chid;					///< Channel ID
	uint16 status;				///< return value : 1 is connected, 0 is disconnected
}
TstVoipSLICPortDetect;

typedef struct
{
	uint8 chid;					///< Channel ID
	int fltMask;
	int measStatus;	
	int rLoop1;
	int rLoop2;
}
TstVoipSLICLineRoh;

typedef struct
{
	uint8 chid;					///< Channel ID
	int vAcDiff;
	int vAcRing;
	int vAcTip;
	int vDcDiff;
	int vDcRing;
	int vDcTip;
	int fltMask;
	int measStatus;	
}
TstVoipSLICLineVoltage;

typedef struct
{
	uint8 chid;					///< Channel ID
	int rGnd;	
	int rrg;	
	int rtg;
	int rtr;
	int fltMask;	
	int measStatus;
}
TstVoipSLICLineResFlt;

typedef struct
{
	uint8 chid;					///< Channel ID
	int crg;	
	int ctg;	
	int ctr;
	int fltMask;	
	int measStatus;
}
TstVoipSLICLineCap;

typedef struct
{
	uint8 chid;					///< Channel ID
	int iTestLoad;	
	int rTestLoad;	
	int vTestLoad;
	int fltMask;	
	int measStatus;
}
TstVoipSLICLineLoopCurrent;


#endif


