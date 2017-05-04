/*
 *-----------------------------------------------------------------------------
 *                                                                             
 *               @@@@@@@      *   @@@@@@@@@           *                                     
 *              @       @             @               *                            
 *              @             *       @      ****     *                                 
 *               @@@@@@@      *       @     *    *    *                              
 *        ___---        @     *       @     ******    *                                 
 *  ___---      @       @     *       @     *         *                             
 *   -_          @@@@@@@  _   *       @      ****     *                               
 *     -_                 _ -                                                     
 *       -_          _ -                                                       
 *         -_   _ -        s   e   m   i   c   o   n   d   u   c   t   o   r 
 *           -                                                                    
 *                                                                              
 * (C) Copyright SiTel Semiconductor BV, unpublished work.
 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:		sc1445x_mcu_api_defs.h
 * Purpose:		
 * Created:		Aug 2008
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef sc1445x_mcu_API_DEFS_H
#define sc1445x_mcu_API_DEFS_H

#define SRTP_ENABLED	1

#ifdef MCU_USE_INTERNAL_CONFIG
	#include "../sc1445x_src/common/si_config_file.h"
#endif

//#define REMOTE_LOOPBACK_TEST 
#ifndef REMOTE_LOOPBACK_TEST 
/*	The MCU_SUPPORT_4_CHANNELS definition resides in si_config_file.h. 
		Otherwise it should be explicity defined here */
	//#define MCU_SUPPORT_CHANNELS  4
	//#define MCU_SUPPORTED_PORTS		4
	#define MAX_MCU_ALLOWED_CHANNELS	MCU_SUPPORTED_CHANNELS
	#define MAX_MCU_ALLOWED_PORTS			MCU_SUPPORTED_PORTS
	#define MAX_MCU_SUPPORTED_CALLS		MCU_SUPPORTED_CHANNELS 
#else 
	#define MCU_SUPPORTED_CHANNELS		15
	#define MAX_MCU_SUPPORTED_CALLS		15
	#define MAX_MCU_ALLOWED_PORTS			15
#endif

#define MCU_DTMF_DURATION	750
#define MCU_DTMF_VOLUME		100

#define MCU_JITTER_BUFFER    20

// #define SC1445x_AE_SUPPORT_FAX
#define ALSA_MAX_BYTES MAX_MCU_ALLOWED_CHANNELS * 82  // #define ALSA_MAX_BYTES 408 
  
typedef enum _sc1445x_mcu_dtmf_mode
{
	MCU_DTMF_INBAND_ID,
	MCU_DTMF_2833_ID,
	MCU_DTMF_OUTBAND_ID
}sc1445x_mcu_dtmf_mode;

typedef enum _sc1445x_mcu_key
{
	MCU_KEY_0_ID	= '0', 
	MCU_KEY_1_ID	= '1',
	MCU_KEY_2_ID	= '2',
	MCU_KEY_3_ID	= '3',
	MCU_KEY_4_ID	= '4',
	MCU_KEY_5_ID	= '5',
	MCU_KEY_6_ID	= '6',
	MCU_KEY_7_ID	= '7',
	MCU_KEY_8_ID	= '8',
	MCU_KEY_9_ID	= '9',
	MCU_KEY_A_ID	= 'A',
	MCU_KEY_B_ID	= 'B',
	MCU_KEY_C_ID	= 'C',
	MCU_KEY_D_ID	= 'D',
	MCU_KEY_STAR_ID	= '*',
	MCU_KEY_POUND_ID= '#'
}sc1445x_mcu_key;

typedef enum _sc1445x_mcu_tone_bmp
{
	MCU_TONE_NA_BT	=0,		// Silence
	MCU_TONE_DL_BT	=1<<1,	// Dial tone bit is set
	MCU_TONE_BZ_BT	=1<<2,	// Busy tone bit is set
	MCU_TONE_WR_BT	=1<<3,	// Warning tone bit is set
	MCU_TONE_DT_BT	=1<<4,	// Digit tone bit is set
	MCU_TONE_RG_BT	=1<<5,	// Ring tone bit is set
	MCU_TONE_RB_BT	=1<<6,	// Ring back tone bit is set	
	MCU_TONE_CW_BT	=1<<7,	// Call waiting tone bit is set
	MCU_TONE_CF_BT	=1<<8,	// Confirmation tone bit is set
	MCU_TONE_DL2_BT	=1<<9,	// Second dial tone bit is set	
  MCU_TONE_CNG_BT =1<<10, // Congestion tone bit is set
  MCU_TONE_DISCN1_BT =1<<11, // Disconnect1 tone bit is set
  MCU_TONE_DISCN2_BT =1<<12, // Disconnect2 tone bit is set
  MCU_TONE_INVALID
}sc1445x_mcu_tone_bmp;

typedef enum _sc1445x_mcu_codec_type
{
	MCU_CDC_G711A_ID,
	MCU_CDC_G711U_ID,
	MCU_CDC_G729_ID,
	MCU_CDC_G723_ID,
	MCU_CDC_G726_ID,
	MCU_CDC_H261_ID,
	MCU_CDC_G722_ID,
	MCU_CDC_iLBC_ID,
	MCU_CDC_iLBC20_ID
}sc1445x_mcu_codec_type;

typedef enum _sc1445x_mcu_media_type
{
	MCU_MTYPE_AUDIO_ID,
	MCU_MTYPE_VIDEO_ID,
	MCU_MTYPE_APP_ID,
	MCU_MTYPE_GDATA_ID,
	MCU_MTYPE_IMG_ID,
	MCU_MTYPE_FAX_ID 
}sc1445x_mcu_media_type;

typedef enum _sc1445x_mcu_audio_mode
{
	MCU_AUD_HANDSET_ID,
	MCU_AUD_HANDSFREE_ID,
	MCU_AUD_HEADSET_ID,
	MCU_AUD_BLUETOOTH_ID,
	MCU_AUD_OPENLISTEN_ID,
	MCU_AUD_DECTHEADSET_ID

}sc1445x_mcu_audio_mode;

typedef enum _sc1445x_mcu_media_format
{
	MCU_MFMT_RTPAVP_ID,
	MCU_MFMT_UDP_ID,
	MCU_MFMT_T38_ID
}sc1445x_mcu_media_format;
 
#define		MCU_SRTP_BYPASS						0 
#define		MCU_SRTP_ENCRYPTION_AUTHENTICATION  1
#define	 	MCU_SRTP_NO_ENCRYPTION				2
#define		MCU_SRTP_NO_AUTHENTICATION			4
 

typedef enum _sc1445x_mcu_media_attr
{
	MCU_MATTR_IDLE_ID,
	MCU_MATTR_RCV_ID,
	MCU_MATTR_SND_ID,
	MCU_MATTR_BOTH_ID,
	MCU_MATTR_INA_ID,
	MCU_MATTR_RLOOP_ID
}sc1445x_mcu_media_attr;

typedef enum _sc1445x_mcu_media_ctrl_mode
{
	MCU_MCTRL_START_ID,
	MCU_MCTRL_MODIFY_ID,
	MCU_MCTRL_STOP_ID,
	MCU_MCTRL_LOOP_ID 

}sc1445x_mcu_media_ctrl_mode;

typedef enum _sc1445x_mcu_mute_flag
{
	MCU_MUTE_ON,
	MCU_MUTE_OFF
}sc1445x_mcu_mute_flag;

#define MAX_CRYPTO_KEY_SIZE			32

#define MAX_CRYPTO_KEY_64_SIZE	64
#define MAX_CRYPTO_SUITE_SIZE	64

typedef struct _sc1445x_mcu_srtp_params_t
{
 	char 			tx_key[MAX_CRYPTO_KEY_64_SIZE];
	unsigned int	tx_key_len;
	char 			tx_suite[MAX_CRYPTO_SUITE_SIZE];
	unsigned int	tx_suite_len;
 	unsigned int	cr_au_flag;
  	char 			rx_key[MAX_CRYPTO_KEY_64_SIZE];
	unsigned int 	rx_key_len;
	char 			rx_suite[MAX_CRYPTO_SUITE_SIZE];
	unsigned int	rx_suite_len;
	
}sc1445x_mcu_srtp_params_t;

typedef struct _sc1445x_mcu_audio_media_params
{
	unsigned int lport;
	unsigned int rport;
#ifdef SIP_IPv6 
	char rAddress[64];
#else
	unsigned char rAddress[4];
#endif	
	sc1445x_mcu_codec_type matchCodec;
 	unsigned char ptype;
 	unsigned short rxRate;
	unsigned short txRate;
	sc1445x_mcu_dtmf_mode dtmfMode;
	unsigned char dtmfPtype;
	int vad;
 	//void *pRtpSession;
    int fxsLine;
	int Jitter;
	int tos;
	unsigned char rtp_ptype;

	#ifdef SRTP_ENABLED
		sc1445x_mcu_srtp_params_t srtp_params;
	#endif
 }sc1445x_mcu_audio_media_params_t;

typedef struct _sc1445x_mcu_audio_media_stream
{
	sc1445x_mcu_media_type	mediaType;
	sc1445x_mcu_media_format mediaFormat;
	sc1445x_mcu_media_attr mediaAttr;
 	sc1445x_mcu_audio_media_params_t mediaParams;
}sc1445x_mcu_audio_media_stream_t;

#define SC1445x_MAX_AUDIO_HEADER					2
#define SC1445x_MAX_PTIME							6								 
#define SC1445x_MAX_AUDIO_PACKET					80 
	 
#define SC1445x_CHANNEL_STATE_SILENCE			0
#define SC1445x_CHANNEL_STATE_ACTIVE			1
#define SC1445x_CHANNEL_STATE_SID					2


#define SC1445x_TALK_STATE_ACTIVE					0
#define SC1445x_TALK_STATE_SILENCE				1
#define SC1445x_TALK_STATE_SID						2

typedef struct _audio_frames_buffer 
{
	unsigned char audioPacket[(SC1445x_MAX_AUDIO_PACKET+SC1445x_MAX_AUDIO_HEADER) * SC1445x_MAX_PTIME];
	unsigned char rtpPacket[(SC1445x_MAX_AUDIO_PACKET  +SC1445x_MAX_AUDIO_HEADER) * SC1445x_MAX_PTIME];
	unsigned char SID[16];

	unsigned char SidSize;  
	unsigned char txTicks;
//	unsigned char rxTicks;
//	unsigned char codecType;

 	unsigned char txStoredSamples ;
  	unsigned char txPacketization;
  	unsigned char txPacketizationTime;
 	unsigned char rxMorePackets;

	unsigned char rxPacketization;
	unsigned char rxPacketizationTime;
//	unsigned char reserved[2];

 	int rxAudioSamplePos ;
	int rxPacketSize;
	int rxTimestamp;

  	int txAudioPacketPos;
	int txAudioSamplesSize ;
	int txTimestamp;
}audio_frames_buffer_t; 

typedef struct _sc1445x_mcu_instance_t
{
	int state;
 	void* pRtpSession;
 	unsigned short alsaChannel;
	int codecSampleSize;
	audio_frames_buffer_t audioFramesBuffer;
	sc1445x_mcu_audio_media_stream_t mediaStream;
	unsigned short handle;  
	#ifdef SRTP_ENABLED
		void *pSrtpSession; 
	#endif
}sc1445x_mcu_instance_t;

#endif //sc1445x_mcu_API_DEFS_H
