
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
 * (C) Copyright SiTel Semiconductor BV, unpublished work.file: 

 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:		 		 sc1445x_media.c
 * Purpose:		 		 
 * Created:		 		 Dec 2007
 * By:		 		 	 YP
 * Country:		 		 Greece
 *
 *-----------------------------------------------------------------------------
 */
/*========================== Include files ==================================*/
#include <ortp/ortp.h>

#include <ortp/telephonyevents.h>
 
#include "sc1445x_alsa_ioctl.h"
#include "sc1445x_media.h"
#include "sc1445x_mcu_block.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
 
#define TYPE(val) type: (val)
#define CLOCK_RATE(val) clock_rate: (val)
#define BYTES_PER_SAMPLE(val) bits_per_sample: (val)
#define ZERO_PATTERN(val)   zero_pattern: (val)
#define PATTERN_LENGTH(val) pattern_length: (val)
#define NORMAL_BITRATE(val) normal_bitrate: (val)
#define MIME_TYPE(val) mime_type: (val)
/*
#define TYPE(val)  (val)
#define CLOCK_RATE(val)   (val)
#define BYTES_PER_SAMPLE(val)  (val)
#define ZERO_PATTERN(val)    (val)
#define PATTERN_LENGTH(val)  (val)
#define NORMAL_BITRATE(val)  (val)
#define MIME_TYPE(val)   (val)
*/
char Aoffset127=127; 
char Aoffset0xD5=(char)0xD5; 
 
PayloadType	telephone_event101={
	PAYLOAD_AUDIO_PACKETIZED, /*type */
	8000,	/*clock rate */
	0,		/* bytes per sample N/A */
	NULL,	/* zero pattern N/A*/
	0,		/*pattern_length N/A */
	0,		/*	normal_bitrate */
	MIME_TYPE ("telephone-event")
 };
PayloadType  pcmu8000={
	TYPE( PAYLOAD_AUDIO_PACKETIZED),//PAYLOAD_AUDIO_CONTINUOUS
	CLOCK_RATE( 8000),
	BYTES_PER_SAMPLE(8),
  ZERO_PATTERN(NULL),//	ZERO_PATTERN( &Aoffset127),
	  PATTERN_LENGTH(0), //PATTERN_LENGTH( 1),
	NORMAL_BITRATE( 64000),
	MIME_TYPE ("PCMU")
};

PayloadType  pcma8000={
	TYPE( PAYLOAD_AUDIO_PACKETIZED),
	CLOCK_RATE(8000),
	BYTES_PER_SAMPLE(8),
	 ZERO_PATTERN(NULL), //ZERO_PATTERN( &Aoffset0xD5),
	  PATTERN_LENGTH(0),//PATTERN_LENGTH( 1),
	NORMAL_BITRATE( 64000),
	MIME_TYPE ("PCMA")
};
 
  PayloadType ilbc =
{
  TYPE( PAYLOAD_AUDIO_PACKETIZED),
  CLOCK_RATE(8000),
  BYTES_PER_SAMPLE( 0),
  ZERO_PATTERN(NULL),
  PATTERN_LENGTH( 0),
  NORMAL_BITRATE( 13330),
  MIME_TYPE ("ILBC")
};

  PayloadType ilbc_20 =
{
  TYPE( PAYLOAD_AUDIO_PACKETIZED),
  CLOCK_RATE(8000),
  BYTES_PER_SAMPLE( 0),
  ZERO_PATTERN(NULL),
  PATTERN_LENGTH( 0),
  NORMAL_BITRATE( 15200), //FIX FRIDAY 
  MIME_TYPE ("ILBC")
};
//ILBC FIX
/*  PayloadType ilbc =
{
  TYPE( PAYLOAD_AUDIO_PACKETIZED),
  CLOCK_RATE(8000),
  BYTES_PER_SAMPLE( 0),
  ZERO_PATTERN(NULL),
  PATTERN_LENGTH( 0),
  NORMAL_BITRATE( 13330),
  MIME_TYPE ("ILBC")
};*/

PayloadType amr={
  TYPE( PAYLOAD_AUDIO_PACKETIZED),
  CLOCK_RATE(8000),
  BYTES_PER_SAMPLE(0),
  ZERO_PATTERN(NULL),
  PATTERN_LENGTH(0),
  NORMAL_BITRATE(12000),    /* set mode MR 122 as default */
  MIME_TYPE ("AMR")
};

 PayloadType amr_wb={
  TYPE( PAYLOAD_AUDIO_PACKETIZED),
  CLOCK_RATE(16000),
  BYTES_PER_SAMPLE(320),
  ZERO_PATTERN(NULL),
  PATTERN_LENGTH(0),
  NORMAL_BITRATE(24000),   /* set mode 2385 as default */
  MIME_TYPE ("AMR-WB")
};

PayloadType cng_8={
  TYPE( PAYLOAD_AUDIO_PACKETIZED),
  CLOCK_RATE(8000),
  BYTES_PER_SAMPLE(0),
  ZERO_PATTERN(NULL),
  PATTERN_LENGTH(0),
  NORMAL_BITRATE(8000),
  MIME_TYPE ("CN")
};

  PayloadType cng_16={
  TYPE( PAYLOAD_AUDIO_PACKETIZED),
  CLOCK_RATE(16000),
  BYTES_PER_SAMPLE(0),
  ZERO_PATTERN(NULL),
  PATTERN_LENGTH(0),
  NORMAL_BITRATE(8000),
  MIME_TYPE ("CN")
};

  PayloadType mspeex_nb={
  TYPE( PAYLOAD_AUDIO_PACKETIZED),
  CLOCK_RATE(8000),
  BYTES_PER_SAMPLE(320),
  ZERO_PATTERN(NULL),
  PATTERN_LENGTH(0),
  NORMAL_BITRATE(15000),    /* set mode MR 122 as default */
  MIME_TYPE ("SPEEX")
};
 PayloadType mspeex_wb={
  TYPE( PAYLOAD_AUDIO_PACKETIZED),
  CLOCK_RATE(16000),
  BYTES_PER_SAMPLE(0),
  ZERO_PATTERN(NULL),
  PATTERN_LENGTH(0),
  NORMAL_BITRATE(28000),
  MIME_TYPE ("SPEEX")
};

 PayloadType g72632={
  PAYLOAD_AUDIO_PACKETIZED,
  CLOCK_RATE(8000),
  BYTES_PER_SAMPLE(0),
  ZERO_PATTERN(NULL),
  PATTERN_LENGTH(0),
  NORMAL_BITRATE(32000),
  MIME_TYPE ("G726-32")
};

 PayloadType g722={
  PAYLOAD_AUDIO_PACKETIZED,
  CLOCK_RATE(8000),
  BYTES_PER_SAMPLE(0),
  ZERO_PATTERN(NULL),
  PATTERN_LENGTH(0),
  NORMAL_BITRATE(64000), //YANNIS TBD
  MIME_TYPE ("G722")
};

 PayloadType g729={
  PAYLOAD_AUDIO_PACKETIZED,
  CLOCK_RATE(8000),
  BYTES_PER_SAMPLE(0),
  ZERO_PATTERN(NULL),
  PATTERN_LENGTH(0),
  NORMAL_BITRATE(64000),
  MIME_TYPE ("G729")
};

int sc1445x_media_init(void)
{
 	sc1445x_media_vprofile_init();
	return 0;
 }
 // extern RtpProfile av_profile;
 // RtpProfile av_profile1;

 RtpProfile * sc1445x_media_get_av_profile( void )
{
         return &av_profile;
} 

PayloadType * sc1445x_media_get_telephone_event( void )
{
        return &payload_type_telephone_event;
}

void sc1445x_media_vprofile_init(void)
{
//	int i;
	RtpProfile *profile;
   	
	profile = sc1445x_media_get_av_profile();
	rtp_profile_clear_all(profile);

	profile->name="AV profile";
 
	rtp_profile_set_payload(profile,PH_MEDIA_PCMU_PAYLOAD,&pcmu8000);
	rtp_profile_set_payload(profile,PH_MEDIA_PCMA_PAYLOAD,&pcma8000);
	rtp_profile_set_payload(profile,PH_MEDIA_G729_PAYLOAD, &g729);

//	rtp_profile_set_payload(profile,PH_MEDIA_ILBC_PAYLOAD, &ilbc);
//	rtp_profile_set_payload(profile,PH_MEDIA_ILBC_PAYLOAD_20ms, &ilbc_20);

	rtp_profile_set_payload(profile,PH_MEDIA_G72632_PAYLOAD, &g72632);
	rtp_profile_set_payload(profile,PH_MEDIA_DTMF_PAYLOAD, sc1445x_media_get_telephone_event());
	rtp_profile_set_payload(profile,PH_MEDIA_G722_PAYLOAD, &g722);
	rtp_profile_set_payload(profile,PH_MEDIA_CN_PAYLOAD, &cng_8);

	return; 
 }
 
int sc1445x_media_find_samplesize(int payload) 	 
{
	int samplesize = 0;
 	switch(payload)
	{
		case PH_MEDIA_PCMU_PAYLOAD:	
  		case PH_MEDIA_PCMA_PAYLOAD:	
			samplesize = 80;
		break;
		case PH_MEDIA_G72632_PAYLOAD:
			samplesize = 40;	
 		break;
		case PH_MEDIA_G729_PAYLOAD:
			samplesize = 10;
		break;
		case PH_MEDIA_G722_PAYLOAD:
			samplesize = 80;
 		break;
		case PH_MEDIA_ILBC_PAYLOAD:
			samplesize = 50;
		break;
		case PH_MEDIA_ILBC_PAYLOAD_20ms:
			samplesize = 38;//FIX FRIDAY 
	 
		break;
	}
 
	return samplesize;
}

int sc1445x_media_get_ptime(int payload) 	 
{
  	switch(payload)
	{
		case PH_MEDIA_PCMU_PAYLOAD:	
  		case PH_MEDIA_PCMA_PAYLOAD:	
 		case PH_MEDIA_G72632_PAYLOAD:
 		case PH_MEDIA_G729_PAYLOAD:
 		case PH_MEDIA_G722_PAYLOAD:
			return 0;
		break;
 		case PH_MEDIA_ILBC_PAYLOAD:
			return 30;
		break;
		case PH_MEDIA_ILBC_PAYLOAD_20ms:
			return 20;
		break;
	}
 
	return 0;
}

unsigned short sc1445x_media_find_codec_type(int payload, int vad) 	 
{
	unsigned short type = 0;
  	switch(payload)
	{
		case PH_MEDIA_PCMU_PAYLOAD:	
			if (vad) type = SC1445x_AE_CODEC_G711_ULAW_VAD;
			else type = SC1445x_AE_CODEC_G711_ULAW;
		break;
 		case PH_MEDIA_PCMA_PAYLOAD:	
			if (vad) type = SC1445x_AE_CODEC_G711_ALAW_VAD;
			else type = SC1445x_AE_CODEC_G711_ALAW;
		break;
		case PH_MEDIA_G72632_PAYLOAD:
			if (vad) type = SC1445x_AE_CODEC_G726_VAD;
			else type = SC1445x_AE_CODEC_G726;
		break;
		case PH_MEDIA_G729_PAYLOAD:
			if (vad) type = SC1445x_AE_CODEC_G729_VAD;
			else type = SC1445x_AE_CODEC_G729;
		break;
		case PH_MEDIA_G722_PAYLOAD:
 
 			type = SC1445x_AE_CODEC_G722;
		break;
		case PH_MEDIA_ILBC_PAYLOAD:
			 type = SC1445x_AE_CODEC_iLBC_30ms;
		break;
		case PH_MEDIA_ILBC_PAYLOAD_20ms:
 			type = SC1445x_AE_CODEC_iLBC_20ms;
		break;
		default://case PH_MEDIA_PCMU_PAYLOAD:	
			if (vad) type = SC1445x_AE_CODEC_G711_ULAW_VAD;
			else type = SC1445x_AE_CODEC_G711_ULAW;
	 

	}
 
	return type;
}
 

