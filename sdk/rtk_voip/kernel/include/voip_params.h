/**
 * @file voip_params.h
 * @brief VoIP control parameters
 */

#ifndef VOIP_PARAMS_H
#define VOIP_PARAMS_H

#include "rtk_voip.h"

/**
 * @ingroup VOIP_DSP_CODEC
 * Enumeration for supported rtp payload (RFC3551)
 */
typedef enum
{
    rtpPayloadUndefined 	= -1,
    rtpPayloadPCMU			= 0,
	// 1~2: Reserved
    rtpPayloadGSM			= 3,
    rtpPayloadG723			= 4,
    rtpPayloadDVI4_8KHz		= 5,
    rtpPayloadDVI4_16KHz	= 6,
    rtpPayloadLPC			= 7,
    rtpPayloadPCMA			= 8,
    rtpPayloadG722			= 9,
    rtpPayloadL16_stereo	= 10,
    rtpPayloadL16_mono		= 11,
    rtpPayloadQCELP			= 12,
    rtpPayloadCN			= 13,
    rtpPayloadMPA			= 14,
    rtpPayloadG728			= 15,
    rtpPayloadDVI4_11KHz	= 16,
    rtpPayloadDVI4_22KHz	= 17,
    rtpPayloadG729			= 18,
	// 19: Reserved
	// 20~24: Unassigned
    rtpPayloadCelB			= 25,
    rtpPayloadJPEG			= 26,
	// 27: Unassigned
    rtpPayloadNV			= 28,
	// 29~30: Unassigned
    rtpPayloadH261			= 31,
    rtpPayloadMPV			= 32,
    rtpPayloadMP2T			= 33,
    rtpPayloadH263			= 34,
	// 35~71: Unassigned
	// 72~76: Reserved for RTCP conflict avoidance
	// 77~95: Unassigned
	// > 96: dynamic payload type
    rtpPayloadDTMF_RFC2833		= 96,
	rtpPayload_iLBC				= 97,
	rtpPayload_iLBC_20ms		= 98,
    rtpPayloadFaxModem_RFC2833	= 101,
    rtpPayloadG726_16			= 102,
    rtpPayloadG726_24			= 103,
    rtpPayloadG726_32 			= 104,
    rtpPayloadG726_40 			= 105,
    rtpPayloadSilence			= 106,	// fake codec
    rtpPayload_PCM_Linear_8K	= 107,
    rtpPayload_PCM_Linear_16K	= 108,
    rtpPayloadT38_Virtual		= 110,
    rtpPayloadV152_Virtual		= 111,
    rtpPayload_SPEEX_NB_RATE8	= 112,
    rtpPayload_SPEEX_NB_RATE2P15= 113,
    rtpPayload_SPEEX_NB_RATE5P95= 114,
    rtpPayload_SPEEX_NB_RATE11	= 115,
    rtpPayload_SPEEX_NB_RATE15	= 116,
    rtpPayload_SPEEX_NB_RATE18P2= 117,
    rtpPayload_SPEEX_NB_RATE24P6= 118,
    rtpPayload_SPEEX_NB_RATE3P95= 119,
    rtpPayloadCiscoRtp			= 120,
    rtpPayloadRtpRedundant		= 121,
    rtpPayloadL16_8k_mono		= 122,
    rtpPayload_AMR_NB			= 123,
    rtpPayloadPCMU_WB			= 124,
    rtpPayloadPCMA_WB			= 125
} RtpPayloadType;

/**
 * @ingroup VOIP_SESSION_RTP
 * Enumeration for rtp session state
 */
typedef enum
{
    rtp_session_unchange = -2,			        ///< RTP Session- Unchanged
    rtp_session_undefined = -1,				///< RTP Session- Undefined
    rtp_session_inactive = 0,                           ///< RTP Session- Inactive
    rtp_session_sendonly = 1,                           ///< RTP Session- Sending only
    rtp_session_recvonly = 2,                           ///< RTP Session- Receiving only
    rtp_session_sendrecv = 3                            ///< RTP Session- Send/Recv
} RtpSessionState;

/**
 * @ingroup VOIP_SESSION_RTP
 * Enumeration for session type
 */
typedef enum
{
    session_type_undefined =  -1,		///< Packet Type - Undefined
    session_type_rtp = 0,               ///< Packet Type - RTP
    session_type_rtcp = 1,              ///< Packet Type - RTCP
    session_type_t38 = 2,               ///< Packet Type - T38
} SessionType;


#ifdef CONFIG_AUDIOCODES_VOIP
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
#if 1
	DSPCODEC_COUNTRY_TW,
#else
	DSPCODEC_COUNTRY_TR,
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
#endif

/**
 * @ingroup VOIP_DSP_TONE
 * Enumeration for which tone to play
 */
typedef enum
{
	DSPCODEC_TONE_NONE = -1,			///< tone none
	DSPCODEC_TONE_0 = 0,				///< index = 0, DTMF digit '0'
	DSPCODEC_TONE_1,				///< index = 1, DTMF digit '1'
	DSPCODEC_TONE_2,				///< index = 2, DTMF digit '2'
	DSPCODEC_TONE_3,				///< index = 3, DTMF digit '3'
	DSPCODEC_TONE_4,				///< index = 4, DTMF digit '4'
	DSPCODEC_TONE_5,				///< index = 5, DTMF digit '5'
	DSPCODEC_TONE_6,				///< index = 6, DTMF digit '6'
	DSPCODEC_TONE_7,				///< index = 7, DTMF digit '7'
	DSPCODEC_TONE_8,				///< index = 8, DTMF digit '8'
	DSPCODEC_TONE_9,				///< index = 9, DTMF digit '9'
	DSPCODEC_TONE_STARSIGN,			///< index = 10, DTMF digit '*'
	DSPCODEC_TONE_HASHSIGN,			///< index = 11, DTMF digit '#'

#ifdef SUPPORT_TONE_PROFILE

	DSPCODEC_TONE_DIAL,				///< code = 66 in RFC2833, index = 12
	DSPCODEC_TONE_STUTTERDIAL,			///< code = 68 in RFC2833, index = 13
	DSPCODEC_TONE_MESSAGE_WAITING,			///< index = 14
	DSPCODEC_TONE_CONFIRMATION,			///< code = 97 in RFC2833, index = 15
	DSPCODEC_TONE_RINGING, 				///< ring back tone, code = 70 in RFC2833, index = 16
	DSPCODEC_TONE_BUSY,				///< code = 72 in RFC2833, index = 17
	DSPCODEC_TONE_CONGESTION,			///< code = 73 in RFC2833, index = 18
	DSPCODEC_TONE_ROH,				///< code = 88 in RFC2833, index = 19, receiver off-hook (roh)
	DSPCODEC_TONE_DOUBLE_RING,			///< index = 20
	DSPCODEC_TONE_SIT_NOCIRCUIT,			///< code = 74 in RFC2833, index = 21, Special Information Tones (SIT) all circuits busy
	DSPCODEC_TONE_SIT_INTERCEPT,			///< index = 22, number changed or disconnected
	DSPCODEC_TONE_SIT_VACANT,			///< index = 23, unassigned code
	DSPCODEC_TONE_SIT_REORDER,			///< index = 24, incomplete digits
	DSPCODEC_TONE_CALLING_CARD_WITHEVENT,		///< index = 25
	DSPCODEC_TONE_CALLING_CARD,			///< index = 26
	DSPCODEC_TONE_CALL_WAITING,			///< code = 79 in RFC2833, index = 27
	DSPCODEC_TONE_CALL_WAITING_2,			///< index = 28
	DSPCODEC_TONE_CALL_WAITING_3,			///< index = 29
	DSPCODEC_TONE_CALL_WAITING_4,			///< index = 30
	DSPCODEC_TONE_EXTEND_1,				///< extend tone #1, index = 31
	DSPCODEC_TONE_EXTEND_2,				///< extend tone #2, index = 32
	DSPCODEC_TONE_EXTEND_3,				///< extend tone #3, index = 33
	DSPCODEC_TONE_EXTEND_4,				///< extend tone #4, index = 34
	DSPCODEC_TONE_EXTEND_5,				///< extend tone #5, index = 35
	DSPCODEC_TONE_INGRESS_RINGBACK,			///< index = 36

	DSPCODEC_TONE_HOLD,				///< code = 76 in RFC2833, index = 37
	DSPCODEC_TONE_OFFHOOKWARNING,			///< code = 88 in RFC2833, index = 38
	DSPCODEC_TONE_RING,				///< code = 89 in RFC2833, index = 39

#else
	DSPCODEC_TONE_DIAL,				///< code = 66 in RFC2833
	DSPCODEC_TONE_RINGING,				///< code = 70 in RFC2833
	DSPCODEC_TONE_BUSY,				///< code = 72 in RFC2833
	DSPCODEC_TONE_HOLD,				///< code = 76 in RFC2833

	DSPCODEC_TONE_CALL_WAITING,			///< code = 79 in RFC2833
	DSPCODEC_TONE_OFFHOOKWARNING,			///< code = 88 in RFC2833
	DSPCODEC_TONE_RING,				///< code = 89 in RFC2833

#endif // #ifdef SUPPORT_TONE_PROFILE
	DSPCODEC_TONE_RING1,			///< index = 40
	DSPCODEC_TONE_RING2,			///< index = 41
	DSPCODEC_TONE_RING3,			///< index = 42
	DSPCODEC_TONE_RING4,			///< index = 43
	DSPCODEC_TONE_RING5,			///< index = 44
	DSPCODEC_TONE_RING6,			///< index = 45
	DSPCODEC_TONE_RING7,			///< index = 46
	DSPCODEC_TONE_RING8,			///< index = 47
	DSPCODEC_TONE_RING9,			///< index = 48
	DSPCODEC_TONE_RING10,			///< index = 49

	DSPCODEC_TONE_CUSTOM_TONE1,		///< index = 50
	DSPCODEC_TONE_CUSTOM_TONE2,		///< index = 51
	DSPCODEC_TONE_CUSTOM_TONE3,		///< index = 52
	DSPCODEC_TONE_CUSTOM_TONE4,		///< index = 53
	DSPCODEC_TONE_CUSTOM_TONE5,		///< index = 54
	DSPCODEC_TONE_CUSTOM_TONE6,		///< index = 55
	DSPCODEC_TONE_CUSTOM_TONE7,		///< index = 56
	DSPCODEC_TONE_CUSTOM_TONE8,		///< index = 57

#if 1 //def SW_DTMF_CID, always enable
	// hc+ 1124 for DTMF CID =================
	DSPCODEC_TONE_A,					///< DTMF digit A, index = 58
	DSPCODEC_TONE_B,					///< DTMF digit B, index = 59
	DSPCODEC_TONE_C,					///< DTMF digit C, index = 60
	DSPCODEC_TONE_D,					///< DTMF digit D, index = 61
	//=========================================
#endif
	// sandro+ 2006/07/24 for SAS tone
	DSPCODEC_TONE_FSK_SAS,				///< alert signal, index = 62
	// hc+ 1229 for off hook FSK CID
	DSPCODEC_TONE_FSK_ALERT,				///< alert signal, index = 63
	// jwsyu+ 20111208 for off hook FSK CID mute voice
	DSPCODEC_TONE_FSK_MUTE,				///< mute voice when off hook CID, index = 64
	
	DSPCODEC_TONE_NTT_IIT_TONE,			///< for NTT type 2 caller id, index = 65

	// thlin+ continous DTMF tone play for RFC2833
	DSPCODEC_TONE_0_CONT,				///< index = 66
	DSPCODEC_TONE_1_CONT,                   ///< index = 67
	DSPCODEC_TONE_2_CONT,                   ///< index = 68
	DSPCODEC_TONE_3_CONT,                   ///< index = 69
	DSPCODEC_TONE_4_CONT,                   ///< index = 70
	DSPCODEC_TONE_5_CONT,                   ///< index = 71
	DSPCODEC_TONE_6_CONT,                   ///< index = 72
	DSPCODEC_TONE_7_CONT,                   ///< index = 73
	DSPCODEC_TONE_8_CONT,                   ///< index = 74
	DSPCODEC_TONE_9_CONT,                   ///< index = 75
	DSPCODEC_TONE_STARSIGN_CONT,			///< index = 76
	DSPCODEC_TONE_HASHSIGN_CONT,			///< index = 77
	DSPCODEC_TONE_A_CONT,				///< index = 78
	DSPCODEC_TONE_B_CONT,				///< index = 79
	DSPCODEC_TONE_C_CONT,				///< index = 80
	DSPCODEC_TONE_D_CONT,				///< index = 81
	
	DSPCODEC_TONE_VBD_ANS,				///< index = 82
	DSPCODEC_TONE_VBD_ANSBAR,			///< index = 83
	DSPCODEC_TONE_VBD_ANSAM,			///< index = 83
	DSPCODEC_TONE_VBD_ANSAMBAR,			///< index = 84
	DSPCODEC_TONE_VBD_CNG,				///< index = 85
	DSPCODEC_TONE_VBD_CRE,				///< index = 86

	DSPCODEC_TONE_USER_DEFINE1,			///< user define tone #1, index = 87
	DSPCODEC_TONE_USER_DEFINE2,			///< user define tone #2, index = 88
	DSPCODEC_TONE_USER_DEFINE3,			///< user define tone #3, index = 89
	DSPCODEC_TONE_USER_DEFINE4,			///< user define tone #4, index = 90
	DSPCODEC_TONE_USER_DEFINE5,			///< user define tone #5, index = 91
	
	DSPCODEC_TONE_KEY				///< the others key tone, index = 92

} DSPCODEC_TONE;

/**
 * @ingroup VOIP_DSP_TONE
 * Enumeration for which direction to play tone
 */
typedef enum									
{
	DSPCODEC_TONEDIRECTION_LOCAL,			///< local 
	DSPCODEC_TONEDIRECTION_REMOTE,			///< remote 
	DSPCODEC_TONEDIRECTION_BOTH			///< local and remote
} DSPCODEC_TONEDIRECTION;

/**
 * @ingroup VOIP_DSP_IVR
 * Play Types in IVR
 * We provide many types of input
 */
typedef enum {
	IVR_PLAY_TYPE_TEXT,				///< play a string, such as '192.168.0.0'
	IVR_PLAY_TYPE_G723_63,				///< play a G723 6.3k data (24 bytes per 30 ms)
	IVR_PLAY_TYPE_G729,				///< play a G729 data (10 bytes per 10ms) 
	IVR_PLAY_TYPE_G711A,				///< play a G711 a-law (80 bytes per 10ms) 
	IVR_PLAY_TYPE_LINEAR_8K,				///< play 8k linear (160bytes per 10ms)
	IVR_PLAY_TYPE_LINEAR_16K,			///< play 16k linear (320bytes per 10ms) 
	IVR_PLAY_TYPE_G722,				///< play a G722 (80 bytes per 10ms) 
} IvrPlayType_t;

/**
 * @ingroup VOIP_DSP_IVR
 * IVR play direction (for playing TEXT only) 
 * One can play IVR in local or remote. 
 */
typedef enum {
	IVR_DIR_LOCAL,					///< IVR local direction
	IVR_DIR_REMOTE,					///< IVR remote direction
} IvrPlayDir_t;

/**
 * @ingroup VOIP_DSP_IVR
 * Specified Text Type
 * Execpt to ASCII within 0~127, we define special speech above 128.
 */
enum {
	IVR_TEXT_ID_DHCP		= 128,
	IVR_TEXT_ID_FIX_IP,
	IVR_TEXT_ID_NO_RESOURCE,
	IVR_TEXT_ID_PLZ_ENTER_NUMBER,	
	IVR_TEXT_ID_PLEASE_ENTER_PASSWORD,
	///<<&&ID5&&>>	/* DON'T remove this line, it helps wizard to generate ID. */
	//IVR_TEXT_ID_xxx,	
};

#ifdef CONFIG_RTK_VOIP_IP_PHONE
/**
 * @ingroup VOIP_IPPHONE_KEYPAD 
 * We list keypad control in follow 
 */
typedef enum {
	KEYPAD_CMD_SET_TARGET,
	KEYPAD_CMD_SIG_DEBUG,
	KEYPAD_CMD_READ_KEY,
	KEYPAD_CMD_HOOK_STATUS,
} keypad_cmd_t;

/**
 * @ingroup VOIP_IPPHONE_LCM 
 * We list LCM control in follow 
 */
typedef enum {
	LCM_CMD_DISPLAY_ON_OFF,
	LCM_CMD_MOVE_CURSOR_POS,
	LCM_CMD_DRAW_TEXT,
	LCM_CMD_WRITE_DATA,
	LCM_CMD_WRITE_DATA2,
	LCM_CMD_DIRTY_MMAP,
	LCM_CMD_DIRTY_MMAP2,
} lcm_cmd_t;

/**
 * @ingroup VOIP_IPPHONE_OTHERS 
 * Control voice path 
 */
typedef enum {
	VPATH_MIC1_SPEAKER,
	VPATH_MIC2_MONO,
	VPATH_SPEAKER_ONLY,
	VPATH_MONO_ONLY,
} vpath_t;
#endif /* CONFIG_RTK_VOIP_IP_PHONE */

/**
 * @brief For VoIP resourece parameters
 */
typedef enum 
{
	VOIP_RESOURCE_UNAVAILABLE=0,
	VOIP_RESOURCE_AVAILABLE,
}Voip_reosurce_t;

/**
 * @brief For FSK caller id mode.
 */
typedef enum
{
	FSK_Bellcore = 0,
	FSK_ETSI,
	FSK_BT,
	FSK_NTT
}TfskArea;

/**
 * @brief For FSK caller ID parameters
 */
typedef enum
{
	FSK_PARAM_NULL = 0,				///< 
	FSK_PARAM_DATEnTIME = 0x01,			///< Date and Time
	FSK_PARAM_CLI = 0x02,				///< Calling Line Identify (CLI)
	FSK_PARAM_CLI_ABS = 0x04,			///< Reason for absence of CLI
	FSK_PARAM_CLI_NAME = 0x07,			///< Calling Line Identify (CLI) Name
	FSK_PARAM_CLI_NAME_ABS = 0x08,			///< Reason for absence of (CLI) Name
	FSK_PARAM_MW = 0x0b,				///< Message Waiting

}TfskParaType;

/**
 * @brief For FSK caller ID gain parameters
 */
typedef enum
{
	FSK_CLID_GAIN_8DB_UP = 0,			///< 
	FSK_CLID_GAIN_7DB_UP,                           ///< 
	FSK_CLID_GAIN_6DB_UP,                           ///< 
	FSK_CLID_GAIN_5DB_UP,                           ///< 
	FSK_CLID_GAIN_4DB_UP,                           ///< 
	FSK_CLID_GAIN_3DB_UP,                           ///< 
	FSK_CLID_GAIN_2DB_UP,                           ///< 
	FSK_CLID_GAIN_1DB_UP,                           ///< 
	FSK_CLID_GAIN_0DB,                              ///< 
	FSK_CLID_GAIN_1DB_DOWN,                         ///< 
	FSK_CLID_GAIN_2DB_DOWN,                         ///< 
	FSK_CLID_GAIN_3DB_DOWN,                         ///< 
	FSK_CLID_GAIN_4DB_DOWN,                         ///< 
	FSK_CLID_GAIN_5DB_DOWN,                         ///< 
	FSK_CLID_GAIN_6DB_DOWN,                         ///< 
	FSK_CLID_GAIN_7DB_DOWN,                         ///< 
	FSK_CLID_GAIN_8DB_DOWN,                         ///< 
	FSK_CLID_GAIN_9DB_DOWN,                         ///< 
	FSK_CLID_GAIN_10DB_DOWN,                        ///< 
	FSK_CLID_GAIN_11DB_DOWN,                        ///< 
	FSK_CLID_GAIN_12DB_DOWN,                        ///< 
	FSK_CLID_GAIN_13DB_DOWN,                        ///< 
	FSK_CLID_GAIN_14DB_DOWN,                        ///< 
	FSK_CLID_GAIN_15DB_DOWN,                        ///< 
	FSK_CLID_GAIN_16DB_DOWN,                        ///< 
	FSK_CLID_GAIN_MAX                               ///< 

}TfskClidGain;

/**
 * @brief For LED display
 */
typedef enum {
	LED_OFF,						///< LED turn off
	LED_ON,						///< LED turn On
	LED_BLINKING,			///< LED blinking
	LED_BLINKING_1HZ,			///< LED blinking
	LED_BLINKING_4HZ,			///< LED blinking		
} LedDisplayMode;

/**
 * @ingroup VOIP_PHONE
 * Enumeration for RFC2833 volume mode
 */
typedef enum
{
	RFC2833_VOLUME_DSP_NON_ATUO = 0,		///<
	RFC2833_VOLUME_DSP_ATUO = 1,			///<
	
}RFC2833_VOLUME_MODE;

/**
 * @ingroup VOIP_PHONE
 * Type definition for RFC2833 volume (n indicates -n dBm)
 */
typedef unsigned int RFC2833_VOLUME;

/**
 * @ingroup VOIP_DSP_GENERAL
 * Specified VoIP EVENT type 
 */
typedef enum {
	VET_DTMF	= 0x00010000,	///< bit 16: DTMF event 
	VET_HOOK	= 0x00020000,	///< bit 17: hook event 
	VET_ENERGY	= 0x00040000,	///< bit 18: energy event 
	VET_DECT	= 0x00080000,	///< bit 19: DECT event 
	VET_FAXMDM	= 0x00100000,	///< bit 20: FAX/MODEM event 
	VET_RFC2833	= 0x00200000,	///< bit 21: RFC 2833
	VET_DSP		= 0x00400000,	///< bit 22: other DSP event 
	
	VET_MASK	= 0xFFFF0000,	///< mask bits 16-31
	VET_ALL		= 0xFFFF0000,	///< all event type, bits 16-31
} VoipEventType;

/**
 * @ingroup VOIP_DSP_GENERAL
 * VoIP EVENT mask is used to mask (provide) some events for user 
 */
typedef enum {
	VEM_MID0	= 0x00000001,	///< bit 0: mask media 0 (RFC2833/DSP)
	VEM_MID1	= 0x00000002,	///< bit 1: mask media 1 (RFC2833/DSP)
	VEM_DIRTDM	= 0x00000010,	///< bit 4: mask direction TDM (DTMF) 
	VEM_DIRIP	= 0x00000020,	///< bit 5: mask direction IP (DTMF) 
	
	VEM_ALL		= 0xFFFFFFFF,	///< bit 0-31: mask all 
} VoipEventMask;

/**
 * @ingroup VOIP_DSP_GENERAL
 * Specified VoIP EVENT ID 
 */
typedef enum {
	VEID_MASK_ID	= 0x0000FFFF,	///< evnet ID mask 0-15
	VEID_MASK_TYPE	= VET_MASK,		///< event type mask 16-31
	
	VEID_NONE		= 0x00000000,	///< no event 
	
	// DTMF (0x00010000)
	VEID_DTMF_WILDCARD		= VET_DTMF,		///< to combine with ASCII '0'-'9' '*' '#'
	VEID_DTMF_DIGIT_MASK	= 0x000000FF,	///< mask to retrieve ASCII digit 
	
	VEID_DTMF_0		= VET_DTMF | '0',	///< DTMF '0'
	VEID_DTMF_1		= VET_DTMF | '1',	///< DTMF '1'
	VEID_DTMF_2		= VET_DTMF | '2',	///< DTMF '2'
	VEID_DTMF_3		= VET_DTMF | '3',	///< DTMF '3'
	VEID_DTMF_4		= VET_DTMF | '4',	///< DTMF '4'
	VEID_DTMF_5		= VET_DTMF | '5',	///< DTMF '5'
	VEID_DTMF_6		= VET_DTMF | '6',	///< DTMF '6'
	VEID_DTMF_7		= VET_DTMF | '7',	///< DTMF '7'
	VEID_DTMF_8		= VET_DTMF | '8',	///< DTMF '8'
	VEID_DTMF_9		= VET_DTMF | '9',	///< DTMF '9'
	VEID_DTMF_STAR	= VET_DTMF | '*',	///< DTMF '*'
	VEID_DTMF_SHARP	= VET_DTMF | '#',	///< DTMF '#'
	VEID_DTMF_A		= VET_DTMF | 'A',	///< DTMF 'A'
	VEID_DTMF_B		= VET_DTMF | 'B',	///< DTMF 'B'
	VEID_DTMF_C		= VET_DTMF | 'C',	///< DTMF 'C'
	VEID_DTMF_D		= VET_DTMF | 'D',	///< DTMF 'D'
	VEID_DTMF_ENERGY= VET_DTMF | 'E',	///< indicate 'E'nergy
	
	// Hook (0x00020000)
	VEID_HOOK_PHONE_ON_HOOK = VET_HOOK,	///< On hook 
	VEID_HOOK_PHONE_OFF_HOOK,			///< Off hook
	VEID_HOOK_PHONE_FLASH_HOOK,			///< Flash hook
	VEID_HOOK_PHONE_STILL_ON_HOOK,		///< Still on hook 
	VEID_HOOK_PHONE_STILL_OFF_HOOK,		///< Still off hook 
	//VEID_HOOK_PHONE_UNKNOWN,			///< Unknown??
	VEID_HOOK_FXO_ON_HOOK,				///< FXO on hook
	VEID_HOOK_FXO_OFF_HOOK,				///< FXO off hook
	VEID_HOOK_FXO_FLASH_HOOK,			///< FXO flash hook 
	VEID_HOOK_FXO_STILL_ON_HOOK,		///< FXO still on hook 
	VEID_HOOK_FXO_STILL_OFF_HOOK,		///< FXO still off hook
	VEID_HOOK_FXO_RING_ON,				///< FXO ring on 
	VEID_HOOK_FXO_RING_OFF,				///< FXO ring off 
	VEID_HOOK_FXO_BUSY_TONE,			///< FXO busy tone 
	VEID_HOOK_FXO_CALLER_ID,			///< FXO caller ID 
	VEID_HOOK_FXO_RING_TONE_ON,			///< FXO ring tone on 
	VEID_HOOK_FXO_RING_TONE_OFF,		///< FXO ring tone off 
	VEID_HOOK_FXO_POLARITY_REVERSAL,	///< FXO resersal 
	VEID_HOOK_FXO_CURRENT_DROP,			///< FXO current drop 
	VEID_HOOK_FXO_DIS_TONE,				///< FXO disconnect tone 
	//VEID_HOOK_FXO_UNKNOWN,			///< unknown?? 
	VEID_HOOK_OUTBAND_FLASH_EVENT,		///< RTP outband flash event 
	
	// Energy (0x00040000)
	VEID_ENERGY = VET_ENERGY,			///< Energy 
	
	// DECT (0x00080000)
	VEID_DECT_BUTTON_PAGE = VET_DECT, 	///< Page button 
	VEID_DECT_BUTTON_REGISTRATION_MODE,	///< Register button 
	VEID_DECT_BUTTON_DEL_HS,			///< Delete button 
	VEID_DECT_BUTTON_NOT_DEFINED,		///< Undefined button 
	
	// FAXMDM (0x00100000)
	VEID_FAXMDM_AUDIOCODES_FAX		= VET_FAXMDM | 1,	///< FAX event (Audiocodes)
	VEID_FAXMDM_AUDIOCODES_MODEM	= VET_FAXMDM | 2,	///< Modem event (Audiocodes)
	VEID_FAXMDM_LEC_AUTO_RESTORE	= VET_FAXMDM | 20,	///< AUTO Restore LEC config event \ref rtk_SetVbdEc
	VEID_FAXMDM_LEC_AUTO_ON			= VET_FAXMDM | 21,	///< AUTO Turn ON LEC event \ref rtk_SetVbdEc
	VEID_FAXMDM_LEC_AUTO_OFF		= VET_FAXMDM | 22,	///< AUTO Turn OFF LEC event \ref rtk_SetVbdEc
	VEID_FAXMDM_FAX_CED				= VET_FAXMDM | 30,	///< FAX CED 2100HZ event
	VEID_FAXMDM_FAX_DIS_TX			= VET_FAXMDM | 31,	///< FAX DIS event in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_FAX_DIS_RX			= VET_FAXMDM | 32,	///< FAX DIS event in receive path(TDM/PCM/Local)
	VEID_FAXMDM_FAX_DCN_TX			= VET_FAXMDM | 33,	///< FAX DCN event in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_FAX_DCN_RX			= VET_FAXMDM | 34,	///< FAX DCN event in receive path(TDM/PCM/Local)
	VEID_FAXMDM_MODEM_LOCAL			= VET_FAXMDM | 35,	///< MODEM /ASNam tone in receive path(TDM/PCM/Local)
	VEID_FAXMDM_MODEM_LOCAL_DELAY	= VET_FAXMDM | 36,	///< This event means that DSP need to check if DIS_RX event occur within 4~6 sec after modem tone event is detected.
	VEID_FAXMDM_MODEM_REMOTE		= VET_FAXMDM | 37,	///< MODEM_REMOTE must right after MODEM_LOCAL_DELAY. see ced.c
	VEID_FAXMDM_ANSTONE_CNG_LOCAL	= VET_FAXMDM | 40,	///< FAX CNG tone in receive path(TDM/PCM/Local)
	VEID_FAXMDM_ANSTONE_ANS_LOCAL	= VET_FAXMDM | 41,	///< FAX CED/ANS tone in receive path(TDM/PCM/Local)
	VEID_FAXMDM_ANSTONE_ANSAM_LOCAL	= VET_FAXMDM | 42,	///< MODEM ANSam tone in receive path(TDM/PCM/Local)
	VEID_FAXMDM_ANSTONE_ANSBAR_LOCAL	= VET_FAXMDM | 43,	///< MODEM /ANS tone in receive path(TDM/PCM/Local)
	VEID_FAXMDM_ANSTONE_ANSAMBAR_LOCAL	= VET_FAXMDM | 44,	///< MODEM /ANSam tone in receive path(TDM/PCM/Local)
	VEID_FAXMDM_ANSTONE_BELLANS_LOCAL	= VET_FAXMDM | 45,	///< MODEM BELL Answer tone in receive path(TDM/PCM/Local)
	VEID_FAXMDM_ANSTONE_V22_LOCAL		= VET_FAXMDM | 46,	///< MODEM V22 tone in receive path(TDM/PCM/Local)
	VEID_FAXMDM_ANSTONE_V8BIS_LOCAL	= VET_FAXMDM | 47,	///< MODEM V8bis tone in receive path(TDM/PCM/Local)
	VEID_FAXMDM_V21FLAG_LOCAL		= VET_FAXMDM | 48,	///< FAX V21FLAG (DIS preamble) in receive path(TDM/PCM/Local)
	VEID_FAXMDM_HS_FAX_SEND_V21FLAG_LOCAL = VET_FAXMDM | 49,	///< Super G3, High speed FAX: itu v.8 sender, V.21 CM in receive path(TDM/PCM/Local)
	VEID_FAXMDM_HS_FAX_RECV_V21FLAG_LOCAL = VET_FAXMDM | 50,	///< Super G3, High speed FAX: itu v.8 receiver, V.21 JM in receive path(TDM/PCM/Local)
	VEID_FAXMDM_ANSTONE_OFF_LOCAL		= VET_FAXMDM | 51,	///< the FAX or MODEM tone off event
	VEID_FAXMDM_ANSTONE_V21C2_LOCAL		= VET_FAXMDM | 52,	///< MODEM V21 channel 2 tone in receive path(TDM/PCM/Local)
	VEID_FAXMDM_ANSTONE_V21C1_LOCAL		= VET_FAXMDM | 53,	///< MODEM V21 channel 1 tone in receive path(TDM/PCM/Local)
	VEID_FAXMDM_ANSTONE_V23_LOCAL		= VET_FAXMDM | 54,	///< MODEM V23 tone in receive path(TDM/PCM/Local)
	VEID_FAXMDM_ANSTONE_BELL202ANS_LOCAL	= VET_FAXMDM | 55,	///< MODEM BELL 202 Answer tone in receive path(TDM/PCM/Local)
	VEID_FAXMDM_ANSTONE_BELL202CP_LOCAL	= VET_FAXMDM | 56,	///< MODEM BELL 202 CP tone in receive path(TDM/PCM/Local)

	                                                       
	VEID_FAXMDM_ANSTONE_CNG_REMOTE	= VET_FAXMDM | 60,		///< FAX CNG tone in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_ANSTONE_ANS_REMOTE	= VET_FAXMDM | 61,		///< FAX CED/ANS tone in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_ANSTONE_ANSAM_REMOTE	= VET_FAXMDM | 62,	///< MODEM ANSam tone in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_ANSTONE_ANSBAR_REMOTE	= VET_FAXMDM | 63,	///< MODEM /ANS tone in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_ANSTONE_ANSAMBAR_REMOTE	= VET_FAXMDM | 64,	///< MODEM /ANSam tone in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_ANSTONE_BELLANS_REMOTE	= VET_FAXMDM | 65,	///< MODEM BELL Answer tone in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_ANSTONE_V22_REMOTE	= VET_FAXMDM | 66,		///< MODEM V22 tone in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_ANSTONE_V8BIS_REMOTE	= VET_FAXMDM | 67,	///< MODEM V8bis tone in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_V21FLAG_REMOTE		= VET_FAXMDM | 68,		///< FAX V21FLAG (DIS preamble) in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_HS_FAX_SEND_V21FLAG_REMOTE	= VET_FAXMDM | 69,	///< Super G3, High speed FAX: itu v.8 sender, V.21 CM in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_HS_FAX_RECV_V21FLAG_REMOTE	= VET_FAXMDM | 70,	///< Super G3, High speed FAX: itu v.8 receiver, V.21 JM in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_ANSTONE_OFF_REMOTE	= VET_FAXMDM | 71,		///< the FAX or MODEM tone off event
	VEID_FAXMDM_ANSTONE_V21C2_REMOTE	= VET_FAXMDM | 72,	///< MODEM V21 channel 2 tone in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_ANSTONE_V21C1_REMOTE	= VET_FAXMDM | 73,	///< MODEM V21 channel 1 tone in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_ANSTONE_V23_REMOTE		= VET_FAXMDM | 74,	///< MODEM V23 tone in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_ANSTONE_BELL202ANS_REMOTE	= VET_FAXMDM | 75,	///< MODEM BELL 202 Answer tone in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_ANSTONE_BELL202CP_REMOTE	= VET_FAXMDM | 76,	///< MODEM BELL 202 CP tone in transmit path(IP/RTP/Remote)

	VEID_FAXMDM_FAST_FAXTONE_LOCAL		= VET_FAXMDM | 80,	///< Fast detection FAX tone in receive path(TDM/PCM/Local), may mis-detect
	VEID_FAXMDM_FAST_MODEMTONE_LOCAL	= VET_FAXMDM | 81,	///< Fast detection MODEM tone in receive path(TDM/PCM/Local), may mis-detect
	VEID_FAXMDM_FAST_FAXTONE_REMOTE		= VET_FAXMDM | 85,	///< Fast detection FAX tone in transmit path(IP/RTP/Remote), may mis-detect
	VEID_FAXMDM_FAST_MODEMTONE_REMOTE	= VET_FAXMDM | 86,	///< Fast detection MODEM tone in transmit path(IP/RTP/Remote), may mis-detect
	                                                       
	VEID_FAXMDM_V152_RTP_VBD		= VET_FAXMDM | 90,	///< Switch to V.152 VBD due to receive VBD RTP
	VEID_FAXMDM_V152_RTP_AUDIO		= VET_FAXMDM | 91,	///< Switch to V.152 voice due to receive voice RTP
	VEID_FAXMDM_V152_SIG_CED		= VET_FAXMDM | 92,	///< Switch to V.152 VBD due to CED signal 
	VEID_FAXMDM_V152_TDM_SIG_END	= VET_FAXMDM | 93,	///< Switch to V.152 voice due to END signal 
	VEID_FAXMDM_V152_BI_SILENCE		= VET_FAXMDM | 94,	///< Switch to V.152 voice due to silence 
	VEID_FAXMDM_V150_SIG_MODEM		= VET_FAXMDM | 96,	///< Switch to V.150 voice due to modem tone
	                                                       
	VEID_FAXMDM_SILENCE_TDM		= VET_FAXMDM | 100,	///< Silence in TDM side 
	VEID_FAXMDM_SILENCE_IP		= VET_FAXMDM | 101,	///< Silence in IP side 
	VEID_FAXMDM_SILENCE_TDM_IP	= VET_FAXMDM | 102,	///< Silence in both sides 
	// ECM Mode Events
	VEID_FAXMDM_ECM_DCN_LOCAL	= VET_FAXMDM | 110,	///< FAX ECM Mode real DCN event in receive path(TDM/PCM/Local)
	VEID_FAXMDM_ECM_DCN_REMOTE	= VET_FAXMDM | 111,	///< FAX ECM Mode real DCN event in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_PPS_MPS_LOCAL	= VET_FAXMDM | 112,	///< FAX PPS MPS event in receive path(TDM/PCM/Local)
	VEID_FAXMDM_PPS_MPS_REMOTE	= VET_FAXMDM | 113,     ///< FAX PPS MPS event in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_PPS_EOP_LOCAL	= VET_FAXMDM | 114,     ///< FAX PPS EOP event in receive path(TDM/PCM/Local)
	VEID_FAXMDM_PPS_EOP_REMOTE	= VET_FAXMDM | 115,     ///< FAX PPS EOP event in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_PPS_PRI_MPS_LOCAL	= VET_FAXMDM | 116,     ///< FAX PPS PRI-MPS event in receive path(TDM/PCM/Local)
	VEID_FAXMDM_PPS_PRI_MPS_REMOTE	= VET_FAXMDM | 117,     ///< FAX PPS PRI-MPS event in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_PPS_PRI_EOP_LOCAL	= VET_FAXMDM | 118,	///< FAX PPS PRI-EOP event in receive path(TDM/PCM/Local)
	VEID_FAXMDM_PPS_PRI_EOP_REMOTE	= VET_FAXMDM | 119,	///< FAX PPS PRI-EOP event in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_ECM_FAILURE_LOCAL	= VET_FAXMDM | 120,	///< FAX ECM mode failure in receive path(TDM/PCM/Local)
	VEID_FAXMDM_ECM_FAILURE_REMOTE	= VET_FAXMDM | 121,	///< FAX ECM mode failure in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_ECM_RNR_LOCAL	= VET_FAXMDM | 122,	///< FAX ECM mode RNR in receive path(TDM/PCM/Local)
	VEID_FAXMDM_ECM_RNR_REMOTE	= VET_FAXMDM | 123,	///< FAX ECM mode RNR in transmit path(IP/RTP/Remote)
	VEID_FAXMDM_ECM_RR_LOCAL	= VET_FAXMDM | 124,	///< FAX ECM mode RR in receive path(TDM/PCM/Local)
	VEID_FAXMDM_ECM_RR_REMOTE	= VET_FAXMDM | 125,	///< FAX ECM mode RR in transmit path(IP/RTP/Remote)
	
	// T.38 Event
	VEID_FAXMDM_T38_FAX_END	= VET_FAXMDM | 130,	///< T.38 FAX End Event
	
	// RFC2833 RTP RX (Receive RFC2833 from RTP packet) (0x00200000)
	VEID_RFC2833_RX_WILDCARD	= VET_RFC2833,		///< To combine with 8 bits evnet encoding 
	VEID_RFC2833_RX_MASK		= 0x000000FF,		///< To obtain 8 bits event encoding 
	
	VEID_RFC2833_RX_DTMF_0		= VET_RFC2833 | 0,	///< RFC2833 - DTMF 0
	VEID_RFC2833_RX_DTMF_1,							///< RFC2833 - DTMF 1
	VEID_RFC2833_RX_DTMF_2,							///< RFC2833 - DTMF 2
	VEID_RFC2833_RX_DTMF_3,							///< RFC2833 - DTMF 3
	VEID_RFC2833_RX_DTMF_4,							///< RFC2833 - DTMF 4
	VEID_RFC2833_RX_DTMF_5,							///< RFC2833 - DTMF 5
	VEID_RFC2833_RX_DTMF_6,							///< RFC2833 - DTMF 6
	VEID_RFC2833_RX_DTMF_7,							///< RFC2833 - DTMF 7
	VEID_RFC2833_RX_DTMF_8,							///< RFC2833 - DTMF 8
	VEID_RFC2833_RX_DTMF_9,							///< RFC2833 - DTMF 9
	VEID_RFC2833_RX_DTMF_STAR,						///< RFC2833 - DTMF *
	VEID_RFC2833_RX_DTMF_POUND,						///< RFC2833 - DTMF #
	VEID_RFC2833_RX_DTMF_A,							///< RFC2833 - DTMF A
	VEID_RFC2833_RX_DTMF_B,							///< RFC2833 - DTMF B
	VEID_RFC2833_RX_DTMF_C,							///< RFC2833 - DTMF C
	VEID_RFC2833_RX_DTMF_D,							///< RFC2833 - DTMF D
	VEID_RFC2833_RX_DTMF_FLASH,						///< RFC2833 - DTMF flash 

	VEID_RFC2833_RX_DATA_FAX_ANS = VET_RFC2833 | 32,	///< RFC2833 - Data FAX ANS
	VEID_RFC2833_RX_DATA_FAX_ANSBAR,				///< RFC2833 - Data FAX /ANS
	VEID_RFC2833_RX_DATA_FAX_ANSAM,					///< RFC2833 - Data FAX ANS AM
	VEID_RFC2833_RX_DATA_FAX_ANSAMBAR,				///< RFC2833 - Data FAX /SNA AM
	VEID_RFC2833_RX_DATA_FAX_CNG,					///< RFC2833 - Data FAX CNG
	VEID_RFC2833_RX_DATA_FAX_V21_CH1_B0,			///< RFC2833 - Data FAX V21 CH1 B0
	VEID_RFC2833_RX_DATA_FAX_V21_CH1_B1,			///< RFC2833 - Data FAX V21 CH1 B1
	VEID_RFC2833_RX_DATA_FAX_V21_CH2_B0,			///< RFC2833 - Data FAX V21 CH2 B0
	VEID_RFC2833_RX_DATA_FAX_V21_CH2_B1,			///< RFC2833 - Data FAX V21 CH2 B1 
	VEID_RFC2833_RX_DATA_FAX_CRDI,					///< RFC2833 - Data FAX CRDI
	VEID_RFC2833_RX_DATA_FAX_CRDR,					///< RFC2833 - Data FAX CRDR
	VEID_RFC2833_RX_DATA_FAX_CRE,					///< RFC2833 - Data FAX CRE
	VEID_RFC2833_RX_DATA_FAX_ESI,					///< RFC2833 - Data FAX ESI
	VEID_RFC2833_RX_DATA_FAX_ESR,					///< RFC2833 - Data FAX ESR 
	VEID_RFC2833_RX_DATA_FAX_MRDI,					///< RFC2833 - Data FAX MRDI 
	VEID_RFC2833_RX_DATA_FAX_MRDR,					///< RFC2833 - Data FAX MRDR 
	VEID_RFC2833_RX_DATA_FAX_MRE,					///< RFC2833 - Data FAX MRE 
	VEID_RFC2833_RX_DATA_FAX_CT,					///< RFC2833 - Data FAX CT 
	
	// DSP Event (0x00400000)
	VEID_DSP_DTMF_CLID_END			= VET_DSP | 10,	///< DTMF CLID END Event 
	VEID_DSP_FSK_CLID_END			= VET_DSP | 11,	///< FSK CLID END Event 
	VEID_DSP_FSK_CLID_TYPE2_NO_ACK	= VET_DSP | 12,	///< FSK TYPE2 CLID NO ACK Event
	VEID_DSP_LOCAL_TONE_END			= VET_DSP | 20,	///< Local tone stop event when the tone duration expired. 
	VEID_DSP_REMOTE_TONE_END		= VET_DSP | 21,	///< Remote tone stop event when the tone duration expired. 
	VEID_DSP_LOCAL_TONE_USER_STOP	= VET_DSP | 22,	///< Local tone stop event by user
	VEID_DSP_REMOTE_TONE_USER_STOP	= VET_DSP | 23,	///< Remote tone stop event by user	
	VEID_DSP_RTP_PAYLOAD_MISMATCH	= VET_DSP | 24,	///< Unexpect RTP payload type from remote
	
} VoipEventID;

/**
 * @ingroup VOIP_DRIVER_PCM
 * Bus data format 
 */
typedef enum
{
	AP_BUSDATFMT_PCM_LINEAR,			///< PCM linear 
	AP_BUSDATFMT_PCM_ALAW,				///< PCM A law  
	AP_BUSDATFMT_PCM_ULAW,				///< PCM u law
	AP_BUSDATFMT_PCM_WIDEBAND_LINEAR,	///< PCM wideband - linear
	AP_BUSDATFMT_PCM_WIDEBAND_ALAW,		///< PCM wideband - A law
	AP_BUSDATFMT_PCM_WIDEBAND_ULAW,		///< PCM wideband - u law
	AP_BUSDATFMT_PCM_UNKNOWN,			///< Unknown PCM mode 
} AP_BUS_DATA_FORMAT;

/**
 * @ingroup VOIP_DSP_GENERAL
 * VoIP FXO tune setting mask
 */
typedef enum {
	FXO_TUNE_BUSY_SET_MASK	= 0x00010000,
	FXO_TUNE_DURA_SET_MASK	= 0x00020000,
	FXO_TUNE_BUSY_VAL_MASK	= 0x0000ffff,
	FXO_TUNE_DURA_VAL_MASK	= 0x0000ffff,
} VoipFxoTuneMask;

/**
 * @ingroup VOIP_DSP_GENERAL
 * Enumeration for DTMF Digit
 */
typedef enum
{
	DTMF_DIGIT_0 = 0 ,
	DTMF_DIGIT_1,
	DTMF_DIGIT_2,
	DTMF_DIGIT_3,
	DTMF_DIGIT_4,
	DTMF_DIGIT_5,
	DTMF_DIGIT_6,
	DTMF_DIGIT_7,
	DTMF_DIGIT_8,
	DTMF_DIGIT_9,
	DTMF_DIGIT_STAR,
	DTMF_DIGIT_HASH,
	DTMF_DIGIT_A,
	DTMF_DIGIT_B,
	DTMF_DIGIT_C,
	DTMF_DIGIT_D
    
} DTMF_DIGIT;

/**
 * @brief For Silab proslic type
 */
typedef enum
{
	PROSLIC_TYPE_SI3226 = 0,		///< Proslic type Si3226
	PROSLIC_TYPE_SI3217X = 1,		///< Proslic type Si3217x
	PROSLIC_TYPE_SI3226X = 2,		///< Proslic type Si3226x
	PROSLIC_TYPE_MAX = 3			///< Proslic type max

}ProslicType;

/**
 * @brief For Silab proslic parameter type
 */
typedef enum
{
	PROSLIC_PARAM_TYPE_RING = 0,			///< Proslic param. type Ring
	PROSLIC_PARAM_TYPE_DCFEED = 1,			///< Proslic param. type DC feed
	PROSLIC_PARAM_TYPE_IMPEDANCE = 2,		///< Proslic param. type Impedance
	PROSLIC_PARAM_TYPE_MAX = 3			///< Proslic param. type max

}ProslicParamType;

/**
 * @ingroup VOIP_PORT_LINK_STATUS
 * The bits definition related to TstVoipPortLinkStatus.status
 */
#define PORT_LINK_LAN_ALL	0x0000000F	/* bits 0-3, active LAN */
#define PORT_LINK_LAN0		0x00000001	
#define PORT_LINK_LAN1		0x00000002
#define PORT_LINK_LAN2		0x00000004
#define PORT_LINK_LAN3		0x00000008
#define PORT_LINK_RESERVED1	0x000000F0	/* bit 4-7: reserved for more LAN */
#define PORT_LINK_WAN		0x00000100	/* bit 8, active WAN */

typedef enum {
    LT_TID_RSVD1,           /* Reserved TID */
    LT_TID_LINE_V,          /* Line Voltage Test */
    LT_TID_ROH,             /* Receiver Off-Hook indication */
    LT_TID_RINGERS,         /* Ringers test per FCC Part 68 REN def.*/
    LT_TID_RES_FLT,         /* Resistive Fault */
    LT_TID_MSOCKET,         /* Master Socket detection */
    LT_TID_XCONNECT,        /* Cross Connect detection */
    LT_TID_CAP,             /* Capacitance measurement test */
    LT_TID_ALL_GR_909,      /* All GR-909 fault tests in predefined order*/
    LT_TID_LOOPBACK,        /* Loopback test */
    LT_TID_DC_FEED_ST,      /* DC Feed Self Test */
    LT_TID_RD_LOOP_COND,    /* Read Loop Condition */
    LT_TID_DC_VOLTAGE,      /* DC VOLTAGE Test */
    LT_TID_RINGING_ST,      /* Ringing Self Test */
    LT_TID_ON_OFF_HOOK_ST,  /* On/Off hook Self Test */
    LT_TID_RD_BAT_COND,     /* Read battery conditions */
    LT_TID_PRE_LINE_V,      /* Pre Line Voltage Test */
    LT_TID_FLT_DSCRM,       /* Fault Discrimination Test */
    LT_NUM_TID_CODES
} LtTestIdType;

typedef enum {
	LT_SILAB_TID_REN,
	LT_SILAB_TID_ROH,
	LT_SILAB_TID_RES_FLT,
	LT_SILAB_TID_VOLTAGE,
	LT_SILAB_TID_CAPACITANCE,
	LT_SILAB_TID_CAPACITIVE_FAULT
} LtTestSiLabIdType;

#endif

