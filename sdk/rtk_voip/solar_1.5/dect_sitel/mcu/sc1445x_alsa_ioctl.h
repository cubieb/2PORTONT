
#if !defined( __SC1445x_ALSA_IOCTL_H )
#define __SC1445x_ALSA_IOCTL_H

#if defined( __KERNEL__ )
#  include "audioengine/sc1445x_audioengine_defs.h"
#else

#define __user

#include <sys/ioctl.h>

/* typedef's copied from sc1445x_audioengine_defs.h */

/* audio engine operating mode */
typedef enum sc1445x_ae_mode_t {
	SC1445x_AE_MODE_NORMAL,                 /* normal operation */
	SC1445x_AE_MODE_LOOPBACK,               /* loopback */
	SC1445x_AE_MODE_AUTOPLAY,               /* playback of stored audio */
	SC1445x_AE_MODE_RAW_PCM,                /* playback of raw PCM audio */
	SC1445x_AE_MODE_INVALID
} sc1445x_ae_mode ;


/* audio engine interface mode */
typedef enum sc1445x_ae_iface_mode_t {
	SC1445x_AE_IFACE_MODE_HANDSET,          /* use handset only */
	SC1445x_AE_IFACE_MODE_OPEN_LISTENING,   /* use ext spk and handset */
	SC1445x_AE_IFACE_MODE_HANDS_FREE,       /* use ext spk and ext mic */
	SC1445x_AE_IFACE_MODE_HEADSET,          /* use headset only */
	SC1445x_AE_IFACE_MODE_WIRELESS,		/* use wireless handset */

	SC1445x_AE_IFACE_MODE_INVALID
} sc1445x_ae_iface_mode ;


/* available flavors of audio codecs */
typedef enum sc1445x_ae_codec_type_t {
	SC1445x_AE_CODEC_UNDEFINED = 0,

	/* G711 flavors */
	SC1445x_AE_CODEC_G711_ALAW,
	SC1445x_AE_CODEC_G711_ULAW,
	SC1445x_AE_CODEC_G711_ALAW_VAD,
	SC1445x_AE_CODEC_G711_ULAW_VAD,

	/* G726 flavors */
	SC1445x_AE_CODEC_G726,
	SC1445x_AE_CODEC_G726_VAD,

	/* G729 flavors */
	SC1445x_AE_CODEC_G729,
	SC1445x_AE_CODEC_G729_VAD,

	/* G722 (wideband) */
	SC1445x_AE_CODEC_G722,

	/* iLBC flavors */
	SC1445x_AE_CODEC_iLBC_20ms,
	SC1445x_AE_CODEC_iLBC_30ms,

	SC1445x_AE_CODEC_INVALID
} sc1445x_ae_codec_type ;


/* available tone frequencies */
typedef enum sc1445x_ae_tone_t {
	SC1445x_AE_TONE_F0 = 0, /* idle */
	SC1445x_AE_TONE_F697,   /*  697 Hz; tone1 for 1, 2, 3, A */
	SC1445x_AE_TONE_F770,   /*  770 Hz; tone1 for 4, 5, 6, B */
	SC1445x_AE_TONE_F852,   /*  852 Hz; tone1 for 7, 8, 9, C */
	SC1445x_AE_TONE_F941,   /*  941 Hz; tone1 for *, 0, #, D */
	SC1445x_AE_TONE_F1209,  /* 1209 Hz; tone2 for 1, 4, 7, * */
	SC1445x_AE_TONE_F1336,  /* 1336 Hz; tone2 for 2, 5, 8, 0 */
	SC1445x_AE_TONE_F1477,  /* 1477 Hz; tone2 for 3, 6, 9, # */
	SC1445x_AE_TONE_F1633,  /* 1633 Hz; tone2 for A, B, C, D */
	SC1445x_AE_TONE_F425,   /*  425 Hz; single tone for busy etc */
	SC1445x_AE_TONE_F440,   /*  440 Hz */
	SC1445x_AE_TONE_INVALID
} sc1445x_ae_tone ;


/* standard DTMF and other tones */
typedef enum sc1445x_ae_std_tone_t {
	SC1445x_AE_STD_TONE_0,		/* digit 0 */
	SC1445x_AE_STD_TONE_1,		/* digit 1 */
	SC1445x_AE_STD_TONE_2,		/* digit 2 */
	SC1445x_AE_STD_TONE_3,		/* digit 3 */
	SC1445x_AE_STD_TONE_4,		/* digit 4 */
	SC1445x_AE_STD_TONE_5,		/* digit 5 */
	SC1445x_AE_STD_TONE_6,		/* digit 6 */
	SC1445x_AE_STD_TONE_7,		/* digit 7 */
	SC1445x_AE_STD_TONE_8,		/* digit 8 */
	SC1445x_AE_STD_TONE_9,		/* digit 9 */
	SC1445x_AE_STD_TONE_STAR,	/* key * */
	SC1445x_AE_STD_TONE_HASH,	/* key # */
	SC1445x_AE_STD_TONE_A,		/* key A */
	SC1445x_AE_STD_TONE_B,		/* key B */
	SC1445x_AE_STD_TONE_C,		/* key C */
	SC1445x_AE_STD_TONE_D,		/* key D */
	SC1445x_AE_STD_TONE_BUSY,
	SC1445x_AE_STD_TONE_CONGESTION,
	SC1445x_AE_STD_TONE_DIAL,
	SC1445x_AE_STD_TONE_DISCONNECT1,
	SC1445x_AE_STD_TONE_DISCONNECT2,
	SC1445x_AE_STD_TONE_RINGING,
	SC1445x_AE_STD_TONE_SPECIAL_DIAL,
	SC1445x_AE_STD_TONE_WAITING,
	SC1445x_AE_STD_TONE_INVALID
} sc1445x_ae_std_tone ;


/* parameters for generating a custom tone */
typedef struct sc1445x_ae_custom_tone_params_t {
	/* ARGCOS for the 3 frequencies, narrowband */
	unsigned short narrow_argcos[3] ;

	/* ARGCOS for the 3 frequencies, narrowband */
	unsigned short narrow_argsin[3] ;

	/* ARGCOS for the 3 frequencies, wideband */
	unsigned short wide_argcos[3] ;

	/* ARGCOS for the 3 frequencies, wideband */
	unsigned short wide_argsin[3] ;

	/* amplitudes for the 3 frequencies */
	unsigned short amplitude[3] ;
} sc1445x_ae_custom_tone_params ;


/* one-tone part of a tone sequence */
typedef struct sc1445x_ae_tone_sequence_part_t {
	sc1445x_ae_custom_tone_params tone_params ;
	unsigned short on_duration ;
	unsigned short off_duration ;
} sc1445x_ae_tone_sequence_part ;


/* audio statistics */
typedef struct sc1445x_ae_audio_stats_t {
	/* frame type 0 (no packet) */
	unsigned empty_to_dsp ;
	unsigned empty_from_dsp ;
	unsigned empty_to_os ;
	unsigned empty_from_os ;

	/* frame type 1 (normal) */
	unsigned normal_to_dsp ;
	unsigned normal_from_dsp ;
	unsigned normal_to_os ;
	unsigned normal_from_os ;

	/* frame type 2 (silence) */
	unsigned sid_to_dsp ;
	unsigned sid_from_dsp ;
	unsigned sid_to_os ;
	unsigned sid_from_os ;
} sc1445x_ae_audio_stats ;


/* ATA/DECT line type */
typedef enum sc1445x_ae_line_type_t {
	SC1445x_AE_LINE_TYPE_ATA = 0,			/* ATA */

	SC1445x_AE_LINE_TYPE_CVM_DECT_NARROW,		/* narrowband */
							/* CVM DECT */

	SC1445x_AE_LINE_TYPE_CVM_DECT_WIDE_8KHZ_ALAW,	/* wideband CVM DECT */
							/* a-law compressed */
							/* to 8kHz */

	SC1445x_AE_LINE_TYPE_CVM_DECT_WIDE_8KHZ_ULAW,	/* wideband CVM DECT */
							/* u-law compressed */
							/* to 8kHz */

	SC1445x_AE_LINE_TYPE_CVM_DECT_WIDE_16KHZ,	/* wideband CVM DECT */
							/* at 16kHz */

	SC1445x_AE_LINE_TYPE_NATIVE_DECT_NARROW,	/* narrowband */
							/* native DECT */

	SC1445x_AE_LINE_TYPE_NATIVE_DECT_WIDE,		/* wideband */
							/* native DECT */

	SC1445x_AE_LINE_TYPE_INVALID
} sc1445x_ae_line_type ;

#endif


typedef struct sc1445x_audio_mode {
	/* app <-> driver */
	sc1445x_ae_mode mode ;

	/* app -> driver */
	union {
#if 0
		struct {
			sc1445x_ae_codec_type encoder ;
			sc1445x_ae_codec_type decoder ;
		} normal ;

		struct {
			sc1445x_ae_codec_type encoder ;
			sc1445x_ae_codec_type decoder ;
		} loopback ;
#endif

		struct {
			unsigned short nchannels ;
			unsigned short __user** pointers ;
		} auto_play ;

		struct {
		} raw_pcm ;
	} ;
} sc1445x_audio_mode_t ;


typedef struct sc1445x_start_audio_channel {
	/* app -> driver */
	sc1445x_ae_codec_type enc_codec ;	/* encoder (capture) type */
	sc1445x_ae_codec_type dec_codec ;	/* decoder (playback) type */

	/* driver -> app */
	unsigned short activated_channel ;	/* which channel was activated */
} sc1445x_start_audio_channel_t ;


typedef struct sc1445x_stop_audio_channel {
	/* app -> driver */
	unsigned short channel_index ;
} sc1445x_stop_audio_channel_t ;


#if 0
typedef struct sc1445x_spk_vol {
	/* app -> driver */
	unsigned short spk_index ;

	/* app <-> driver */
	unsigned short vol ;
}  sc1445x_spk_vol_t ;


typedef struct sc1445x_mic_gain {
	/* app -> driver */
	unsigned short mic_index ;

	/* app <-> driver */
	unsigned short gain ;
}  sc1445x_mic_gain_t ;
#endif


typedef struct sc1445x_tone {
	/* app -> driver */
	unsigned short tg_mod ;		/* for PCM/DECT, use 0 for Phone */
	sc1445x_ae_tone tone1 ;
	sc1445x_ae_tone tone2 ;
	sc1445x_ae_tone tone3 ;
	unsigned short dur_on ;		/* in msecs */
	unsigned short dur_off ;	/* in msecs */
	short repeat ;
} sc1445x_tone_t ;


typedef struct sc1445x_stop_tone {
	/* app -> driver */
	unsigned short tg_mod ;		/* for PCM/DECT, use 0 for Phone */
} sc1445x_stop_tone_t ;


typedef struct sc1445x_std_tone {
	/* app -> driver */
	unsigned short tg_mod ;		/* for PCM/DECT, use 0 for Phone */
	sc1445x_ae_std_tone tone ;
} sc1445x_std_tone_t ;


typedef struct sc1445x_tone_vol {
	/* app -> driver */
	unsigned short tg_mod ;		/* for PCM/DECT, use 0 for Phone */
	/* app <-> driver */
	unsigned short vol ;
} sc1445x_tone_vol_t ;


#if 0
typedef struct sc1445x_digital_vol {
	/* app -> driver */
	unsigned short vol ;
} sc1445x_digital_vol_t ;
#endif


typedef struct sc1445x_sidetone_vol {
	/* app <-> driver */
	unsigned short vol ;
} sc1445x_sidetone_vol_t ;


#if 0
typedef struct sc1445x_ext_spk_vol {
	/* app -> driver */
	unsigned short vol ;
} sc1445x_ext_spk_vol_t ;
#endif


typedef struct sc1445x_audio_stats {
	/* app -> driver */
	unsigned short channel_index ;

	/* driver -> app */
	sc1445x_ae_audio_stats stats ;
} sc1445x_audio_stats_t ;


typedef struct sc1445x_ae_version {
	/* driver -> app */
	char id[8] ;
} sc1445x_ae_version_t ;


typedef struct sc1445x_ae_iface_mode {
	/* app <-> driver */
	sc1445x_ae_iface_mode mode ;
} sc1445x_ae_iface_mode_t ;


typedef struct sc1445x_ae_vspk_vol {
	/* app <-> driver */
	unsigned short level ;
} sc1445x_ae_vspk_vol_t ;


typedef struct sc1445x_ae_vmic_gain {
	/* app <-> driver */
	unsigned short level ;
} sc1445x_ae_vmic_gain_t ;


typedef struct sc1445x_connect_to_line {
	/* app -> driver */
	unsigned short channel_index ;
	union {
		unsigned short pcm_line ;
		unsigned short line ;
	} ;
} sc1445x_connect_to_line_t ;


typedef struct sc1445x_connect_lines {
	/* app -> driver */
	union {
		unsigned short pcm_line1 ;
		unsigned short line1 ;
	} ;
	union {
		unsigned short pcm_line2 ;
		unsigned short line2 ;
	} ;
} sc1445x_connect_lines_t ;


typedef struct sc1445x_disconnect_line {
	/* app -> driver */
	union {
		unsigned short pcm_line ;
		unsigned short line ;
	} ;
} sc1445x_disconnect_line_t ;


typedef struct sc1445x_audio_to_fax {
	/* app -> driver */
	unsigned short channel_index ;
} sc1445x_audio_to_fax_t ;


typedef struct sc1445x_fax_to_audio {
	/* app -> driver */
	unsigned short channel_index ;
	//sc1445x_ae_codec_type enc_codec ;	/* encoder (capture) type */
	//sc1445x_ae_codec_type dec_codec ;	/* decoder (playback) type */
} sc1445x_fax_to_audio_t ;


typedef struct sc1445x_set_cid_info {
	/* app -> driver */
	union {
		unsigned short pcm_line ;
		unsigned short line ;
	} ;
	unsigned char month ;		/* 1 - 12 */
	unsigned char day ;		/* 1 - 31 */
	unsigned char hour ;		/* 0 - 23 */
	unsigned char minutes ;		/* 0 - 59 */
	char number[81] ;		/* string, empty if number not avail. */
	char name[101] ;		/* string, empty if number not avail. */
} sc1445x_set_cid_info_t ;


typedef struct sc1445x_cid_ind {
	/* app -> driver */
	union {
		unsigned short pcm_line ;
		unsigned short line ;
	} ;
} sc1445x_cid_ind_t ;


typedef struct sc1445x_line_type {
	/* app -> driver */
	unsigned short line ;

	/* app <-> driver */
	sc1445x_ae_line_type type ;
} sc1445x_line_type_t ;

typedef struct sc1445x_vol_gain_count {
	/* driver -> app */
	unsigned short vol_level_count ;	/* number of spk vol levels */
	unsigned short gain_level_count ;	/* number of mic gain levels */
} sc1445x_vol_gain_count_t ;

typedef struct sc1445x_fax_init_params {
	/* app -> driver */
	unsigned short line ;
	unsigned short p0DBIN ;      	/* reference input level for 0 dBm  */
	unsigned short p0DBOUT ;    	/* reference output level for 0 dBm */
	unsigned short pCEDLength ;  	/* duration of reconstituted CED, 0-3000ms */
	unsigned short pMDMCmd ;	/* Modem Command */
} sc1445x_fax_init_params_t ;

typedef struct sc1445x_custom_tone {
	/* app -> driver */
	unsigned short tg_mod ;		/* for PCM/DECT, use 0 for Phone */

	sc1445x_ae_custom_tone_params params ;

	unsigned short dur_on ;		/* in msecs */
	unsigned short dur_off ;	/* in msecs */

	short repeat ;
} sc1445x_custom_tone_t ;

typedef struct sc1445x_tone_seq {
	/* app -> driver */
	unsigned short tg_mod ;		/* for PCM/DECT, use 0 for Phone */

	unsigned short seq_len ;
	sc1445x_ae_tone_sequence_part* seq ;
	short repeat_seq ;		/* ignored by */
					/* SNDRV_SC1445x_EXPAND_TONE_SEQUENCE */
} sc1445x_tone_seq_t ;


typedef struct sc1445x_send_tone_ex {
	/* app -> driver */
	unsigned short tg_mod ;		/* for PCM/DECT, use 0 for Phone */
} sc1445x_send_tone_ex_t ;


enum sc1445x_platform_type {
	SC14450_3_CHANNELS,
	SC14452_3_CHANNELS,
	SC14452_4_CHANNELS
} ;

typedef struct sc1445x_platform_info {
	/* driver -> app */
	enum sc1445x_platform_type type ;
} sc1445x_platform_info_t ;


/* ioctls */
#define SNDRV_SC1445x_SET_MODE			_IOW( 'H', 0x10, sc1445x_audio_mode_t )
#define SNDRV_SC1445x_GET_MODE			_IOR( 'H', 0x11, sc1445x_audio_mode_t )
#define SNDRV_SC1445x_START_AUDIO_CHANNEL	_IOWR( 'H', 0x12, sc1445x_start_audio_channel_t )
#define SNDRV_SC1445x_STOP_AUDIO_CHANNEL	_IOW( 'H', 0x13, sc1445x_stop_audio_channel_t )
//#define SNDRV_SC1445x_SET_SPK_VOL		_IOW( 'H', 0x14, sc1445x_spk_vol_t )
//#define SNDRV_SC1445x_GET_SPK_VOL		_IOWR( 'H', 0x15, sc1445x_spk_vol_t )
//#define SNDRV_SC1445x_SET_MIC_GAIN		_IOW( 'H', 0x16, sc1445x_mic_gain_t )
//#define SNDRV_SC1445x_GET_MIC_GAIN		_IOWR( 'H', 0x17, sc1445x_mic_gain_t )
#define SNDRV_SC1445x_START_TONE		_IOW( 'H', 0x18, sc1445x_tone_t )
#define SNDRV_SC1445x_STOP_TONE			_IOW( 'H', 0x19, sc1445x_stop_tone_t )
#define SNDRV_SC1445x_PLAY_STD_TONE		_IOW( 'H', 0x1a, sc1445x_std_tone_t )
#define SNDRV_SC1445x_SET_TONE_VOL		_IOW( 'H', 0x1b, sc1445x_tone_vol_t )
#define SNDRV_SC1445x_GET_TONE_VOL		_IOWR( 'H', 0x1c, sc1445x_tone_vol_t )
#define SNDRV_SC1445x_MUTE_SPK			_IO( 'H', 0x1d )
#define SNDRV_SC1445x_UNMUTE_SPK		_IO( 'H', 0x1e )
#define SNDRV_SC1445x_MUTE_MIC			_IO( 'H', 0x1f )
#define SNDRV_SC1445x_UNMUTE_MIC		_IO( 'H', 0x20 )
//#define SNDRV_SC1445x_SELECT_HANDSET		_IO( 'H', 0x21 )
//#define SNDRV_SC1445x_SELECT_HEADSET		_IO( 'H', 0x22 )
//#define SNDRV_SC1445x_SELECT_EXTERNAL		_IO( 'H', 0x23 )
#define SNDRV_SC1445x_SEND_TONES		_IO( 'H', 0x24 )
#define SNDRV_SC1445x_DONT_SEND_TONES		_IO( 'H', 0x25 )
//#define SNDRV_SC1445x_SET_DIGITAL_VOL		_IOW( 'H', 0x26, sc1445x_digital_vol_t )
#define SNDRV_SC1445x_SET_SIDETONE_VOL		_IOW( 'H', 0x27, sc1445x_sidetone_vol_t )
#define SNDRV_SC1445x_GET_SIDETONE_VOL		_IOR( 'H', 0x28, sc1445x_sidetone_vol_t )
//#define SNDRV_SC1445x_SEL_OUTPUT_TO_CODEC	_IO( 'H', 0x29 )
//#define SNDRV_SC1445x_SEL_OUTPUT_TO_EXT_SPK	_IO( 'H', 0x2a )
//#define SNDRV_SC1445x_SET_EXT_SPK_VOL		_IOW( 'H', 0x2b, sc1445x_ext_spk_vol_t )
#define SNDRV_SC1445x_TURN_AEC_ON		_IO( 'H', 0x2c )
#define SNDRV_SC1445x_TURN_AEC_OFF		_IO( 'H', 0x2d )
#define SNDRV_SC1445x_GET_AUDIO_CHANNEL_STATS	_IOWR( 'H', 0x2e, sc1445x_audio_stats_t )
#define SNDRV_SC1445x_RESET_AUDIO_STATS		_IO( 'H', 0x2f )
#define SNDRV_SC1445x_GET_AUDIO_DRIVER_VERSION	_IOR( 'H', 0x30, sc1445x_ae_version_t )
#define SNDRV_SC1445x_GET_DSP_FIRMWARE_VERSION	_IOR( 'H', 0x31, sc1445x_ae_version_t )
#define SNDRV_SC1445x_SET_IFACE_MODE		_IOW( 'H', 0x32, sc1445x_ae_iface_mode_t )
#define SNDRV_SC1445x_GET_IFACE_MODE		_IOR( 'H', 0x33, sc1445x_ae_iface_mode_t )
#define SNDRV_SC1445x_SET_VIRTUAL_SPK_VOL	_IOW( 'H', 0x34, sc1445x_ae_vspk_vol_t )
#define SNDRV_SC1445x_GET_VIRTUAL_SPK_VOL	_IOR( 'H', 0x35, sc1445x_ae_vspk_vol_t )
#define SNDRV_SC1445x_SET_VIRTUAL_MIC_GAIN	_IOW( 'H', 0x36, sc1445x_ae_vmic_gain_t )
#define SNDRV_SC1445x_GET_VIRTUAL_MIC_GAIN	_IOR( 'H', 0x37, sc1445x_ae_vmic_gain_t )
#define SNDRV_SC1445x_MUTE_VIRTUAL_MIC		_IO( 'H', 0x38 )
#define SNDRV_SC1445x_UNMUTE_VIRTUAL_MIC	_IO( 'H', 0x39 )
#define SNDRV_SC1445x_CONNECT_TO_LINE		_IOW( 'H', 0x3a, sc1445x_connect_to_line_t )
#define SNDRV_SC1445x_CONNECT_LINES		_IOW( 'H', 0x3b, sc1445x_connect_lines_t )
#define SNDRV_SC1445x_DISCONNECT_LINE		_IOW( 'H', 0x3c, sc1445x_disconnect_line_t )
#define SNDRV_SC1445x_GET_DSP2_FIRMWARE_VERSION	_IOR( 'H', 0x3d, sc1445x_ae_version_t )
#define SNDRV_SC1445x_SWITCH_AUDIO_TO_FAX	_IOW( 'H', 0x3e, sc1445x_audio_to_fax_t )
#define SNDRV_SC1445x_SWITCH_FAX_TO_AUDIO	_IOW( 'H', 0x3f, sc1445x_fax_to_audio_t )
#define SNDRV_SC1445x_SET_CID_INFO		_IOW( 'H', 0x40, sc1445x_set_cid_info_t )
#define SNDRV_SC1445x_CID_IND_FIRST_RING	_IOW( 'H', 0x41, sc1445x_cid_ind_t )
#define SNDRV_SC1445x_CID_IND_OFF_HOOK		_IOW( 'H', 0x42, sc1445x_cid_ind_t )
#define SNDRV_SC1445x_CID_IND_ON_HOOK		_IOW( 'H', 0x43, sc1445x_cid_ind_t )
#define SNDRV_SC1445x_SET_LINE_TYPE		_IOW( 'H', 0x44, sc1445x_line_type_t )
#define SNDRV_SC1445x_GET_LINE_TYPE		_IOR( 'H', 0x45, sc1445x_line_type_t )
#define SNDRV_SC1445x_GET_VOL_GAIN_LEVEL_COUNT	_IOR( 'H', 0x46, sc1445x_vol_gain_count_t )
#define SNDRV_SC1445x_FAX_INIT			_IOW( 'H', 0x47, sc1445x_fax_init_params_t )
#define SNDRV_SC1445x_START_CUSTOM_TONE		_IOW( 'H', 0x48, sc1445x_custom_tone_t )
#define SNDRV_SC1445x_START_TONE_SEQUENCE	_IOW( 'H', 0x49, sc1445x_tone_seq_t )
#define SNDRV_SC1445x_EXPAND_TONE_SEQUENCE	_IOW( 'H', 0x4a, sc1445x_tone_seq_t )
#define SNDRV_SC1445x_SEND_TONES_EX		_IOW( 'H', 0x4b, sc1445x_send_tone_ex_t )
#define SNDRV_SC1445x_DONT_SEND_TONES_EX	_IOW( 'H', 0x4c, sc1445x_send_tone_ex_t )
#define SNDRV_SC1445x_GET_PLATFORM_INFO		_IOR( 'H', 0x4d, sc1445x_platform_info_t )

#define SNDRV_SC1445x_AE_USE_OLD_IOCTL_NAMES
#if defined( SNDRV_SC1445x_AE_USE_OLD_IOCTL_NAMES )
/* deprecated names */
#  define SNDRV_SC1445x_CONNECT_TO_PCM_LINE	SNDRV_SC1445x_CONNECT_TO_LINE
#  define SNDRV_SC1445x_CONNECT_PCM_LINES		SNDRV_SC1445x_CONNECT_LINES
#  define SNDRV_SC1445x_DISCONNECT_PCM_LINE	SNDRV_SC1445x_DISCONNECT_LINE

#  define sc1445x_connect_to_pcm_line_t		sc1445x_connect_to_line_t
#  define sc1445x_connect_pcm_lines_t		sc1445x_connect_lines_t
#  define sc1445x_disconnect_pcm_line_t		sc1445x_disconnect_line_t
#endif

#endif  /* __SC1445x_ALSA_IOCTL_H */
