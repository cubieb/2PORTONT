
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
 * File:		 		 sc1445x_amb_api.c
 * Purpose:		 		 
 * Created:		 		 Aug 2008
 * By:		 		 	 YP
 * Country:		 		 Greece
 *
 *-----------------------------------------------------------------------------
 */
/*========================== Include files ==================================*/
#include <ctype.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <values.h>
#include "sc1445x_amb_api.h"
#include "sc1445x_ringplay.h"
#include "sc1445x_mcu_debug.h"
 
#include "sc1445x_alsa.h"
#include <alsa/asoundlib.h>

#include <ortp/ortp.h>
 
#define ALSA_DEVICE "hw" 
#define ALSA_DEVICE_HW "hw" 
 
 /*************************************/ 
static snd_pcm_t *playbackHandler=NULL;
static snd_pcm_t *captureHandler=NULL;
static int CallToneInProgress[MAX_MCU_ALLOWED_PORTS] ;
static int DtmfToneInProgress[MAX_MCU_ALLOWED_PORTS];
int alsaFD[2];   
 
/************************************************/ 
/*		AUDIO MANAGEMNT BLOCK API (AMB API)		*/
/************************************************/ 

////////////////////////////////////////////////////////////////////////////////////////
//
//	Open the Playback and Capture streams.  
////////////////////////////////////////////////////////////////////////////////////////
int AlsaIsOpened = 0;

int sc1445x_amb_init(void)
{
  memset(CallToneInProgress,0,sizeof(CallToneInProgress));
 	memset(DtmfToneInProgress,0,sizeof(DtmfToneInProgress));
  return 0;
}

int sc1445x_amb_sound_open(int bits, int stereo, int rate)
{
 	int err;
	//fix 18 June, 2009
 	PRINTINFO( "[%s] open sound driver \n", __FUNCTION__);
	if (AlsaIsOpened) return 0;// TBH change Alsa Attribute
	// Open a playback stream 
 	if (snd_pcm_open(&playbackHandler, ALSA_DEVICE,SND_PCM_STREAM_PLAYBACK,0/*SND_PCM_NONBLOCK*/) < 0) {
 		ua_stopringplay();
 	 	PRINTINFO( "[%s] try again to open sound driver \n", __FUNCTION__);
		if (snd_pcm_open(&playbackHandler, ALSA_DEVICE,SND_PCM_STREAM_PLAYBACK,0/*SND_PCM_NONBLOCK*/) < 0)  
		{
 			DPRINT( "[%s] UNABLE TO OPEN playback stream \n", __FUNCTION__);
			return -1;
		}
	}
 	if (sc1445x_set_params(ALSA_MAX_BYTES*100 ,playbackHandler)<0){
		snd_pcm_close(playbackHandler);
		DPRINT( "[%s]UNABLE TO set params of playback stream \n", __FUNCTION__);
 		return -1;
	}
 
	err = snd_pcm_prepare(playbackHandler ) ;
	if( err < 0 ) {
 		snd_pcm_close(playbackHandler);
		DPRINT( "[%s]UNABLE TO prepare playback stream \n", __FUNCTION__);
 		return -1 ;
	}
 
	err=snd_pcm_start(playbackHandler);
  
	// Open a capture stream 
	if (snd_pcm_open(&captureHandler, ALSA_DEVICE,SND_PCM_STREAM_CAPTURE,0)<0){ 

		snd_pcm_close(captureHandler);
		if (snd_pcm_open(&captureHandler, ALSA_DEVICE,SND_PCM_STREAM_CAPTURE,0)<0) 
		{
			DPRINT( "[%s]Couldn't open capture stream \n", __FUNCTION__);
  			return -1;
 		}
	}
 
	if (sc1445x_set_params(ALSA_MAX_BYTES*100, captureHandler)<0){
		snd_pcm_close(captureHandler);
		DPRINT( "[%s]Couldn't set params of playback stream  \n", __FUNCTION__);
 		return -1;
	}
 	err = snd_pcm_prepare( captureHandler ) ;
	if (err < 0) {
  		snd_pcm_close(captureHandler);
		DPRINT( "[%s]Couldn't prepare playback stream  \n", __FUNCTION__);
  		return -1 ;
	}
 	snd_pcm_start(captureHandler);
  
	if (!captureHandler || !playbackHandler) {
		sc1445x_amb_sound_close();
  		return -1;
	} 
  
	PRINTINFO( "[%s] pcm streams opened successfully \n", __FUNCTION__);
	AlsaIsOpened=1;
	 
 	return 0;
}
int sc1445x_amb_get_playback_fd(void)
{
	struct pollfd pfd;
	int err;
 	err = snd_pcm_poll_descriptors_count(playbackHandler);
	if ((err <= 0) || (err != 1)) {
        DPRINT( "[%s] Fail to get poll descriptor for playback\n", __FUNCTION__); 
 		return -1;
	}
 	snd_pcm_poll_descriptors(playbackHandler, &pfd, err);
 	return pfd.fd;
}
int sc1445x_amb_get_capture_fd(void)
{
	struct pollfd pfd;
	int err;
 	err = snd_pcm_poll_descriptors_count(captureHandler);
	if ((err <= 0) || (err != 1)) {
        DPRINT( "[%s] Fail to get poll descriptor for capturing\n", __FUNCTION__); 
 		return -1;
	}
 	snd_pcm_poll_descriptors(captureHandler, &pfd, err);
  	return pfd.fd;
 }
 
////////////////////////////////////////////////////////////////////////////////////////
//
//  Close an ALSA PCM device 
////////////////////////////////////////////////////////////////////////////////////////

int sc1445x_amb_sound_close(void)
{
	if (captureHandler)	{
		 
		snd_pcm_close(captureHandler);
	}
	if (playbackHandler) {
		 
		snd_pcm_close(playbackHandler);
	}
	AlsaIsOpened=0;
	 
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
//  Read audio frames from a  PCM devive  
////////////////////////////////////////////////////////////////////////////////////////
int sc1445x_amb_sound_read(unsigned char *audioData, int samples)
{
 	int err;
 	snd_pcm_t *dev = captureHandler;
  	err=snd_pcm_readi(dev,audioData,samples); 

	if( -EPIPE == err ) {
 		snd_pcm_prepare( dev ) ;
		snd_pcm_start( dev ) ;
		return err;
	}
	else if( -ESTRPIPE == err ) {
 		return err;
	}
	else if ( err < 0 ) {
 		return err;
	}
	else if( err != ALSA_MAX_BYTES ) {
 	}
 	return err;

   	if (err<0) 
	{
 		return -1;
	}
 	if ((err=sc1445x_alsa_read(dev,audioData,samples))<=0) 
	{
 		return -1;
	}
	return err;
}

////////////////////////////////////////////////////////////////////////////////////////
//
//  Write audio frames to a  PCM devive  
////////////////////////////////////////////////////////////////////////////////////////
int sc1445x_amb_sound_write( unsigned char *audioData, int samples)
{
    	return sc1445x_alsa_write(playbackHandler, audioData, samples);
}

int sc1445x_amb_select_audio_iface(sc1445x_ae_iface_mode audioIface)
{
	snd_hwdep_t *hwdephandle;
	int err;
	sc1445x_ae_iface_mode_t audio_mode;
	audio_mode.mode = audioIface;
 
  		switch(audio_mode.mode)
		{
			case SC1445x_AE_IFACE_MODE_HANDSET:
			case SC1445x_AE_IFACE_MODE_OPEN_LISTENING:
			case SC1445x_AE_IFACE_MODE_HANDS_FREE:
			case SC1445x_AE_IFACE_MODE_HEADSET:
        case SC1445x_AE_IFACE_MODE_WIRELESS:
				if (audio_mode.mode == SC1445x_AE_IFACE_MODE_WIRELESS)
				{
					sc1445x_amb_set_aec((unsigned short)1);
                    DPRINT( "[%s] ====>Enable AEC\n",__FUNCTION__);
				}
				if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
  					DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
	   				return -1;
				}
				if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_SET_IFACE_MODE, &audio_mode))<0)
  					DPRINT( "[%s] Could not set virtual iface mode: %s\n", __FUNCTION__, snd_strerror( err ) );
				else 
					PRINTINFO( "[%s] Virtual Mode selected successfully \n", __FUNCTION__);

				if ((err=snd_hwdep_close(hwdephandle)) < 0) {
					DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
  				}
  				return 0;

			break;
			default:
				DPRINT( "[%s] Wrong audio path setting\n",__FUNCTION__);
  			break;
		}	
	return 0;
}

int sc1445x_amb_set_virtual_spk_volume(unsigned short spkgain)
{
	int err;
	snd_hwdep_t *hwdephandle;
	sc1445x_ae_vspk_vol_t spk_volume;
 
	spk_volume.level = spkgain;
	if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
  		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
		return -1;
	}
	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_SET_VIRTUAL_SPK_VOL, &spk_volume))<0){
		DPRINT( "[%s] Could not set volume: %s\n",__FUNCTION__, snd_strerror(err));
 		snd_hwdep_close(hwdephandle);
 		return -1;
	}else PRINTINFO( "[%s] Virtual Speaker Volume selected successfully \n", __FUNCTION__);
  
	if ((err=snd_hwdep_close(hwdephandle)) < 0) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
  	}
  return 0;
}
//
int sc1445x_amb_get_volume_level_count(unsigned short *vol_level_count, unsigned short *gain_level_count)
{
	int err;
	snd_hwdep_t *hwdephandle;
	sc1445x_vol_gain_count_t volume_level_count;
 	if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
  		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
		return -1;
	}
  
	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_GET_VOL_GAIN_LEVEL_COUNT, &volume_level_count))<0){
		DPRINT( "[%s] Could not get volume level count : %s\n",__FUNCTION__, snd_strerror(err));
 		snd_hwdep_close(hwdephandle);
 		return -1;
	} 

  
	if ((err=snd_hwdep_close(hwdephandle)) < 0) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
  	}
	*vol_level_count = volume_level_count.vol_level_count;  
	*gain_level_count = volume_level_count.gain_level_count;  

	return 0;
}

int sc1445x_amb_get_virtual_spk_volume(void)
{
	int err;
	snd_hwdep_t *hwdephandle;
	sc1445x_ae_vspk_vol_t spk_volume;
 
 	if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
  		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
		return -1;
	} 

	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_GET_VIRTUAL_SPK_VOL, &spk_volume))<0){
		DPRINT("Out of range \n ");
		DPRINT( "[%s] Could not set volume: %s\n",__FUNCTION__, snd_strerror(err));
 		snd_hwdep_close(hwdephandle);
 		return -1;
	}else PRINTINFO( "[%s] Virtual Speaker Volume selected successfully \n", __FUNCTION__);
   
	if ((err=snd_hwdep_close(hwdephandle)) < 0) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
  }
  return (int)(spk_volume.level);
}
 
int sc1445x_amb_increase_virtual_spk_volume(void)
{
	int err;
	snd_hwdep_t *hwdephandle;
	sc1445x_ae_vspk_vol_t spk_volume;
 
 	if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
  		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
		return -1;
	}
		 
	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_GET_VIRTUAL_SPK_VOL, &spk_volume))<0){
		DPRINT("Out of range \n ");
		DPRINT( "[%s] Could not set volume: %s\n",__FUNCTION__, snd_strerror(err));
 		snd_hwdep_close(hwdephandle);
 		return -1;
	}else PRINTINFO( "[%s] Virtual Speaker Volume selected successfully \n", __FUNCTION__);
 	spk_volume.level++; 

	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_SET_VIRTUAL_SPK_VOL, &spk_volume))<0){
		DPRINT( "[%s] Could not set volume: %s\n",__FUNCTION__, snd_strerror(err));
 		snd_hwdep_close(hwdephandle);
 		return -1;
	}else PRINTINFO( "[%s] Virtual Speaker Volume selected successfully \n", __FUNCTION__);

  
	if ((err=snd_hwdep_close(hwdephandle)) < 0) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
  }
  return 0;
}
 
int sc1445x_amb_decrease_virtual_spk_volume(void)
{
	int err;
	snd_hwdep_t *hwdephandle;
	sc1445x_ae_vspk_vol_t spk_volume;
 	if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
  		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
		return -1;
	}
	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_GET_VIRTUAL_SPK_VOL, &spk_volume))<0){
		DPRINT( "[%s] Could not set volume: %s\n",__FUNCTION__, snd_strerror(err));
 		snd_hwdep_close(hwdephandle);
 		return -1;
	}else PRINTINFO( "[%s] Virtual Speaker Volume selected successfully \n", __FUNCTION__);
	if (spk_volume.level)
		spk_volume.level--; 

	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_SET_VIRTUAL_SPK_VOL, &spk_volume))<0){
		DPRINT( "[%s] Could not set volume: %s\n",__FUNCTION__, snd_strerror(err));
 		snd_hwdep_close(hwdephandle);
 		return -1;
	}else PRINTINFO( "[%s] Virtual Speaker Volume selected successfully \n", __FUNCTION__);
 
	if ((err=snd_hwdep_close(hwdephandle)) < 0) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
  	}
  return 0;
}

 
int sc1445x_amb_set_mute(int flag)
{
 	int err  ;  
	snd_hwdep_t *hwdephandle;
	
	if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
  		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
		return -1;
	}
	if (flag==MCU_MUTE_ON)
	{
		if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_MUTE_MIC, 0))<0){
			DPRINT( "[%s] Could not set mute: %s\n",__FUNCTION__, snd_strerror(err));
 			snd_hwdep_close(hwdephandle);
 			return -1;
		} 
	}else if (flag==MCU_MUTE_OFF)
	{
		if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_UNMUTE_MIC, 0))<0){
			DPRINT( "[%s] Could not set mute: %s\n",__FUNCTION__, snd_strerror(err));
 			snd_hwdep_close(hwdephandle);
 			return -1;
		} 
 	}

	if ((err=snd_hwdep_close(hwdephandle)) < 0) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
  	}

	 return 0;	
}

////////////////////////////////////////////////////////////////////////////////////////
//
//  Activate a free channel and assign a specific codec to it.
////////////////////////////////////////////////////////////////////////////////////////

int sc1445x_amb_activate_channel( sc1445x_ae_codec_type encoder,
			sc1445x_ae_codec_type decoder, unsigned short* ch, int line )
{
 	int err = 0 ;
 	sc1445x_connect_to_pcm_line_t cpl;
  snd_hwdep_t *hwdephandle;
	unsigned short pcmline = (unsigned short) line  ;

 	sc1445x_start_audio_channel_t sac ;
	 
  sac.enc_codec = encoder;
	sac.dec_codec = decoder;
  if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
  	DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
 		return err;
	}
  	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_START_AUDIO_CHANNEL, &sac ))<0){
		DPRINT( "[%s] Could not activate audio : %s\n", __FUNCTION__, snd_strerror( err ) );
  		snd_hwdep_close(hwdephandle);
 
		return err ;
	}else PRINTINFO( "[%s] Audio Channel %d selected successfully \n", __FUNCTION__, sac.activated_channel);
 	*ch = sac.activated_channel ;

 	if ((line < MAX_MCU_ALLOWED_PORTS) && (line & 0x80000000))
	{
		cpl.pcm_line = (unsigned short)pcmline ;
		cpl.channel_index = sac.activated_channel;
		if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_CONNECT_TO_PCM_LINE, &cpl ))<0){
			DPRINT( "[%s] Could not connect channel and pcm line : %s\n", __FUNCTION__, snd_strerror( err ) );
			snd_hwdep_close(hwdephandle);
			return err ;
		}else PRINTINFO("[%s] PCM Line %d connected successfully with channel %d \n", __FUNCTION__, cpl.pcm_line, cpl.channel_index);
 
	}

 	if ((err=snd_hwdep_close(hwdephandle)) < 0) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
  }
 
 	return err ;
}
  
////////////////////////////////////////////////////////////////////////////////////////
//
//  Deactivate a channel 
////////////////////////////////////////////////////////////////////////////////////////
int sc1445x_amb_deactivate_channel(unsigned short ch, int line)
{
	snd_hwdep_t *hwdephandle;
	unsigned short pcmline = (unsigned short)(line);
    sc1445x_disconnect_pcm_line_t dpl;
 	int err = 0 ;
	sc1445x_stop_audio_channel_t sac ;

	if (ch<0) return -1;
 
	if ((pcmline<0) || (line>=MAX_MCU_ALLOWED_PORTS)) {
 		DPRINT( "[%s] FATAL ERROR line too big\n", __FUNCTION__ );
  	return -1;
	}
  	if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
   		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
  		return err;
	}
  	sac.channel_index = (unsigned short)ch ;
	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_STOP_AUDIO_CHANNEL, &sac ))<0){
		 DPRINT( "[%s] Could not stop audio channel : %s\n", __FUNCTION__, snd_strerror( err ) );
   		 snd_hwdep_close(hwdephandle);
 		 return err ;
	}else 
	 {
	 	PRINTINFO( "[%s] Audio Channel %d deactivated successfully \n", __FUNCTION__, sac.channel_index);
	}

 	if ((line<MAX_MCU_ALLOWED_PORTS) && (line & 0x80000000))
	{
		dpl.pcm_line = (unsigned short)pcmline ;
		if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_DISCONNECT_PCM_LINE, &dpl))<0){
			 DPRINT( "[%s] Could not disconnect line : %s\n", __FUNCTION__, snd_strerror( err ) );
 			 snd_hwdep_close(hwdephandle);
			return err ;
		}else PRINTINFO( "[%s] PCM line %d disconnected successfully with channel %d \n", __FUNCTION__, dpl.pcm_line, sac.channel_index);
	}
 
 	if ((err=snd_hwdep_close(hwdephandle)) < 0) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
  }
  
  return err ;
}

////////////////////////////////////////////////////////////////////////////////////////
//
//  Play a dynamic call progress tone 
////////////////////////////////////////////////////////////////////////////////////////
int sc1445x_amb_start_dynamic_tone(sc1445x_ae_std_tone cpTone, unsigned short line)
{
	int err;
	sc1445x_tone_t dynTone;
 	snd_hwdep_t *hwdephandle;
 	sc1445x_tone_vol_t tVolume;

 	tVolume.vol=(unsigned short)0x2300;
	if (line > MAX_MCU_ALLOWED_PORTS) line=0;
	tVolume.tg_mod =(unsigned short) line;

	if (line >= MAX_MCU_ALLOWED_PORTS) return -1;
	if (CallToneInProgress[line]) {
		sc1445x_amb_stop_callprogress_tone(line);
		CallToneInProgress[line]=(int)0;
	}
    
 	if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
  		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
 		return -1;
	}
 
	dynTone.tg_mod= line;
 	dynTone.tone1 = cpTone; 	 
 	dynTone.tone2 = SC1445x_AE_TONE_F0; 	 
 	dynTone.tone3 = SC1445x_AE_TONE_F0; 	 
 	dynTone.repeat = 0;
 	dynTone.dur_on = 1000;
 	dynTone.dur_off = 1000;

#ifndef USE_PCM
 	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_DONT_SEND_TONES,0 )) < 0) {
		DPRINT( "[%s] Could not pass  SNDRV_SC1445x_DONT_SEND_TONES ioctl: %s\n", __FUNCTION__, snd_strerror( err ) );
		snd_hwdep_close(hwdephandle);
		return -1;
	}else PRINTINFO( "[%s] SendToNet flag passed successfully \n", __FUNCTION__);

	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_SET_TONE_VOL, &tVolume) ) < 0) {
		DPRINT( "[%s] Could not set tone volume: %s\n", __FUNCTION__, snd_strerror( err ) );
		snd_hwdep_close(hwdephandle);
		return -1;
	}else PRINTINFO( "[%s] Tone Volume passed successfully \n", __FUNCTION__);	
#endif
		 
	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_START_TONE, &dynTone )) < 0) {
 		DPRINT( "[%s] Could not pass call progress tone  : %s\n", __FUNCTION__, snd_strerror( err ) );
		snd_hwdep_close(hwdephandle);
		return -1;
	} else PRINTINFO( "[%s] Call Progress Tone passed successfully at port %d \n", __FUNCTION__, line);	
 
	if ((err=snd_hwdep_close(hwdephandle)) < 0) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
  	}
   	return 0;

}
 
////////////////////////////////////////////////////////////////////////////////////////
//
//  Play a periodic call progress tone 
////////////////////////////////////////////////////////////////////////////////////////
int sc1445x_amb_start_periodic_tone(sc1445x_ae_std_tone cpTone ,unsigned short line)
{
	int err;
	sc1445x_tone_t dynTone;
 	snd_hwdep_t *hwdephandle;
 	sc1445x_tone_vol_t tVolume;
 	tVolume.vol=(unsigned short)0x2300;
	if (line> MAX_MCU_ALLOWED_PORTS) line=0;
	tVolume.tg_mod =(unsigned short) line;

	if (line >= MAX_MCU_ALLOWED_PORTS) return -1;
 // 	if (CallToneInProgress[line]) return (int)0;
  //	CallToneInProgress[line]=(int)1;
    
	CallToneInProgress[line]=(int)1;

  if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
  	DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
 		return -1;
	}
 
	dynTone.tg_mod= line;
 	dynTone.tone1 = cpTone; 	 
 	dynTone.tone2 = cpTone; 	 
 	dynTone.tone3 = cpTone; 	 
 	dynTone.repeat = 1;
 	dynTone.dur_on = 1000;
 	dynTone.dur_off = 5000;

#ifndef USE_PCM
 	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_DONT_SEND_TONES,0 )) < 0) {
		DPRINT( "[%s] Could not pass  SNDRV_SC1445x_DONT_SEND_TONES ioctl: %s\n", __FUNCTION__, snd_strerror( err ) );
		snd_hwdep_close(hwdephandle);
		return -1;
	}else PRINTINFO( "[%s] SendToNet flag passed successfully \n", __FUNCTION__);

	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_SET_TONE_VOL, &tVolume) ) < 0) {
		DPRINT( "[%s] Could not set tone volume: %s\n", __FUNCTION__, snd_strerror( err ) );
		snd_hwdep_close(hwdephandle);
		return -1;
	}else PRINTINFO( "[%s] Tone Volume passed successfully \n", __FUNCTION__);	
#endif
		 
	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_START_TONE, &dynTone )) < 0) {
 		DPRINT( "[%s] Could not pass call progress tone  : %s\n", __FUNCTION__, snd_strerror( err ) );
		snd_hwdep_close(hwdephandle);
		return -1;
	} else PRINTINFO( "[%s] Call Progress Tone passed successfully at port %d \n", __FUNCTION__, line);	
 
	if ((err=snd_hwdep_close(hwdephandle)) < 0) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
  	}
   	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
//  Play a standard call progress tone 
////////////////////////////////////////////////////////////////////////////////////////
int sc1445x_amb_start_callprogress_tone(sc1445x_ae_std_tone cpTone, unsigned short line)
{
	int err;
 	snd_hwdep_t *hwdephandle;
	sc1445x_std_tone_t st ;
 	sc1445x_tone_vol_t tVolume;
	 
 	tVolume.vol=(unsigned short)0x2300;
	if (line > MAX_MCU_ALLOWED_PORTS) line=0;
	 
 	tVolume.tg_mod =(unsigned short) line;

	if (line >= MAX_MCU_ALLOWED_PORTS) return -1;
  	
  	if (CallToneInProgress[line]) return (int)0;
  	
	CallToneInProgress[line]=(int)1;
    
   	if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
  		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
 		return -1;
	}
	st.tone = cpTone; 	 
	st.tg_mod= line;

#ifndef USE_PCM
 	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_DONT_SEND_TONES,0 )) < 0) {
		DPRINT( "[%s] Could not pass  SNDRV_SC1445x_DONT_SEND_TONES ioctl: %s\n", __FUNCTION__, snd_strerror( err ) );
		snd_hwdep_close(hwdephandle);
		return -1;
	}else PRINTINFO( "[%s] SendToNet flag passed successfully \n", __FUNCTION__);

	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_SET_TONE_VOL, &tVolume) ) < 0) {
		DPRINT( "[%s] Could not set tone volume: %s\n", __FUNCTION__, snd_strerror( err ) );
		snd_hwdep_close(hwdephandle);
		return -1;
	}else PRINTINFO( "[%s] Tone Volume passed successfully \n", __FUNCTION__);	
#endif

  	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_PLAY_STD_TONE, &st )) < 0) {
 		DPRINT( "[%s] Could not pass call progress tone  : %s\n", __FUNCTION__, snd_strerror( err ) );
 		snd_hwdep_close(hwdephandle);
		return -1;
	}else PRINTINFO( "[%s] Call Progress Tone passed successfully at port %d\n", __FUNCTION__, line);	
	
 
	if ((err=snd_hwdep_close(hwdephandle)) < 0) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
  	}
   	return 0;
}
int sc1445x_amb_status_callprogress_tone(unsigned short line)
{
	if (line >= MAX_MCU_ALLOWED_PORTS) return -1;
 	return CallToneInProgress[line];
}
////////////////////////////////////////////////////////////////////////////////////////
//
//  Stop a standard call progress tone 
////////////////////////////////////////////////////////////////////////////////////////
int sc1445x_amb_stop_callprogress_tone(unsigned short line)
{
	int err;
	sc1445x_stop_tone_t st;
	snd_hwdep_t *hwdephandle;
 
	if (line >= MAX_MCU_ALLOWED_PORTS) return -1;

	if (!CallToneInProgress[line]) return (int)0;
    CallToneInProgress[line]=(int)0; 
 
	if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
  		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
 		return -1;
	}
 	st.tg_mod = line;
  	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_STOP_TONE, &st )) < 0) {
		DPRINT( "[%s] Could not set stop call progress tone: %s\n", __FUNCTION__, snd_strerror( err ) );
  		snd_hwdep_close(hwdephandle);
		return -1;
	}else PRINTINFO( "[%s] Call Progress Tone stopped successfully \n", __FUNCTION__);	

	if ((err=snd_hwdep_close(hwdephandle)) < 0) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
  	}
  	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//  Play a Dtmf tone 
////////////////////////////////////////////////////////////////////////////////////////

int sc1445x_amb_status_dtmf(unsigned short line)
{
	return DtmfToneInProgress[line];
}

int sc1445x_amb_play_dtmf(sc1445x_ae_std_tone dtmfKey, int sendToNet, int line)
{
	int err;
 	snd_hwdep_t *hwdephandle ;
	sc1445x_std_tone_t st ;
 	sc1445x_tone_vol_t tVolume;
	sc1445x_send_tone_ex_t ex_t;

#ifdef USE_PCM
	ex_t.tg_mod=(unsigned short)line;
#else
	ex_t.tg_mod=0;
#endif

 	tVolume.vol=(unsigned short)0x4000;
	tVolume.tg_mod = (unsigned short)line;

#ifndef USE_PCM
	// Now supported in ATA also
	if (line > 0 ) return 0; //v1.0.1.1 not supported in case of ATA
#endif

  if (DtmfToneInProgress[line]) return (int)0;
 	DtmfToneInProgress[line]=(int)1;
/*Vagelis
  if (CallToneInProgress[line]){
		si_alsa_stop_callprogress_tone((unsigned short)line);
	}
*/
 if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
		return -1;
	}

	st.tone = (sc1445x_ae_std_tone)(dtmfKey); 	 
  st.tg_mod = (unsigned short)line; 
	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_SET_TONE_VOL, &tVolume )) < 0) {
		DPRINT( "[%s] Could not set tone volume: %s\n", __FUNCTION__, snd_strerror( err ) );
  	snd_hwdep_close(hwdephandle);
		return -1;
	}else PRINTINFO( "[%s] Tone Volume passed successfully \n", __FUNCTION__);	 

	if (sendToNet)
	{	
		if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_SEND_TONES_EX, &ex_t )) < 0) {
			DPRINT( "[%s] Could not set tone: %s\n", __FUNCTION__, snd_strerror( err ) );
  		snd_hwdep_close(hwdephandle);
			return -1;
		}else PRINTINFO( "[%s] SendtoNet Flag passed successfully \n", __FUNCTION__);	
	}
	else
	{
		if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_DONT_SEND_TONES_EX, &ex_t )) < 0) {
			DPRINT( "[%s] Could not pass SNDRV_SC1445x_DONT_SEND_TONES ioctl : %s\n", __FUNCTION__, snd_strerror( err ) );
  		snd_hwdep_close(hwdephandle);
			return -1;
		}else PRINTINFO( "[%s] Dont SendtoNet Flag passed successfully for line %d \n", __FUNCTION__, line);	
 	}
	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_PLAY_STD_TONE, &st )) < 0) {
		DPRINT( "[%s] Could not play tone : %s\n", __FUNCTION__, snd_strerror( err ) );
 		snd_hwdep_close(hwdephandle);
		return -1;
	}else PRINTINFO( "[%s] DTMF passed successfully \n", __FUNCTION__);	

	if ((err=snd_hwdep_close(hwdephandle)) < 0) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
  }

#ifdef USE_PCM
	DtmfToneInProgress[line]=(int)0;
#endif

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
//  Play a Dtmf tone 
////////////////////////////////////////////////////////////////////////////////////////
int sc1445x_amb_stop_dtmf(unsigned short line)
{
  int err;
	sc1445x_stop_tone_t st;
	snd_hwdep_t *hwdephandle;
  if (!DtmfToneInProgress[line]) return (int)0;
		DtmfToneInProgress[line]=(int)0; 
 	if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
  		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
		return -1;
	}
 	st.tg_mod = line;
	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_STOP_TONE, &st )) < 0) {
		DPRINT( "[%s] Could not stop tone : %s\n", __FUNCTION__, snd_strerror( err ) ); 
		snd_hwdep_close(hwdephandle);
		return -1;
	}else PRINTINFO( "[%s] DTMF stopped successfully \n", __FUNCTION__);	
	if ((err=snd_hwdep_close(hwdephandle)) < 0) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
  	}
  	return 0;
}
 
////////////////////////////////////////////////////////////////////////////////////////
//
//  Set the mode of a PCM devive  
////////////////////////////////////////////////////////////////////////////////////////
int sc1445x_amb_set_mode( sc1445x_ae_mode  mode) 
{
 	snd_hwdep_t *hwdephandle;
	int err  ;
	sc1445x_audio_mode_t am ;

	am.mode = mode ;
	if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
  		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
		return err;
	}
 	switch( am.mode ) {
		case SC1445x_AE_MODE_NORMAL:
 		case SC1445x_AE_MODE_LOOPBACK:
 		case SC1445x_AE_MODE_AUTOPLAY:
 		case SC1445x_AE_MODE_RAW_PCM:
			break;
 		default:
 			DPRINT( "[%s] Invalid Mode \n", __FUNCTION__);
   			goto _setmodeexit;
	}
	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_SET_MODE, &am ))<0) {
		DPRINT( "[%s] Could not set mode : %s\n", __FUNCTION__, snd_strerror( err ) );
  		snd_hwdep_close(hwdephandle);
 	}else PRINTINFO( "[%s] PCM Mode passed successfully \n", __FUNCTION__);	
_setmodeexit:
	if ((err=snd_hwdep_close(hwdephandle)) < 0) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
  	}
 	return err ; 
}

int sc1445x_amb_sidetone_volume(void)
{
	snd_hwdep_t *hwdephandle;
	sc1445x_sidetone_vol_t sd;
	int err;

	sd.vol=0x1000;

	if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
  		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
 		return -1;
	}

 	if ((err = snd_hwdep_ioctl(hwdephandle, SNDRV_SC1445x_SET_SIDETONE_VOL, &sd))<0){ 
 		DPRINT( "[%s] Could not set sidetone gain: %s\n", __FUNCTION__, snd_strerror( err ) );
 	    snd_hwdep_close(hwdephandle);
 		return -1;
	}else PRINTINFO( "[%s] Sidetone passed successfully \n", __FUNCTION__);	

	if ((err=snd_hwdep_close(hwdephandle)) < 0) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
  	}

	return 0;
}

int sc1445x_amb_set_aec(unsigned short onoff)
{
	snd_hwdep_t *hwdephandle;

	int err;
 
	if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
  		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
		return -1;
	}

	if (onoff==1){
		if ((err = snd_hwdep_ioctl(hwdephandle, SNDRV_SC1445x_TURN_AEC_ON, 0))<0){
			DPRINT( "[%s] Could not set AEC : %s\n", __FUNCTION__, snd_strerror( err ) );
			snd_hwdep_close(hwdephandle);
			return -1;
		}else PRINTINFO( "[%s] PAEC opened successfully \n", __FUNCTION__);	

	 }else if (onoff==0){ 
			if ((err = snd_hwdep_ioctl(hwdephandle, SNDRV_SC1445x_TURN_AEC_OFF, 0))<0){
				DPRINT( "[%s] Could not set AEC : %s\n", __FUNCTION__, snd_strerror( err ) );
				snd_hwdep_close(hwdephandle);
				return -1;
			}else PRINTINFO( "[%s] PAEC closed successfully \n", __FUNCTION__);	
	}

	if ((err=snd_hwdep_close(hwdephandle)) < 0) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
  }

	return 0;
}


void sc1445x_amb_alsa_statistics(snd_pcm_t *pPlayHandler, snd_pcm_t *pCaptureHandler)
{
	static snd_output_t *log;
	snd_pcm_status_t *status;
	int err;

	snd_output_stdio_attach(&log, stderr, 0);
 	snd_pcm_status_alloca(&status);
   
	snd_pcm_dump(pCaptureHandler, log);
	if ((err = snd_pcm_status(pCaptureHandler, status))<0) {
		DPRINT( "[%s] Could not get capture status : %s\n", __FUNCTION__, snd_strerror( err ) );
		return;
	}
 	snd_pcm_status_dump(status, log);
   
	snd_pcm_dump(pPlayHandler, log);
	if ((err = snd_pcm_status(pPlayHandler, status))<0) {
		DPRINT( "[%s] Could not get playback status : %s\n", __FUNCTION__, snd_strerror( err ) );
 		return;
	}
 	snd_pcm_status_dump(status, log);
 }

int sc1445x_amb_internal_connect_lines(unsigned short line1, unsigned short line2 )
{
	snd_hwdep_t *hwdephandle;
	int err;
	sc1445x_connect_pcm_lines_t pcmlines;

	if ((line1 >= MAX_MCU_ALLOWED_PORTS) || (line2 >= MAX_MCU_ALLOWED_PORTS)) return -1;

	pcmlines.pcm_line1 = line1;
	pcmlines.pcm_line2 = line2;
	 
	if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
  		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
		return -1;
	}

	if ((err = snd_hwdep_ioctl(hwdephandle, SNDRV_SC1445x_CONNECT_PCM_LINES, &pcmlines))<0){
 		DPRINT( "[%s] Could connect pcm lines : %s\n", __FUNCTION__, snd_strerror( err ) );
  	snd_hwdep_close(hwdephandle);
		return -1;
	}else PRINTINFO("[%s] PCM lines line1 %d line2 %d connected successfully\n", __FUNCTION__, line1, line2);	
	 
	if ((err=snd_hwdep_close(hwdephandle)) < 0) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
  	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
//  Deactivate a pcm internal connect channel 
////////////////////////////////////////////////////////////////////////////////////////
int sc1445x_amb_internal_disconnect_line(int line)
{
	snd_hwdep_t *hwdephandle;
  sc1445x_disconnect_pcm_line_t dpl;
 	int err = 0 ;
 
	if ((line < 0) || (line >= MAX_MCU_ALLOWED_PORTS)) {
 		DPRINT( "[%s] FATAL ERROR pcm port out of range \n", __FUNCTION__ );
  	return -1;
	}
  if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
  	return err;
	}
  	
	dpl.pcm_line = (unsigned short)line ;
	if ((err = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_DISCONNECT_PCM_LINE, &dpl))<0){
		DPRINT( "[%s] Could not disconnect line : %s\n", __FUNCTION__, snd_strerror( err ) );
		snd_hwdep_close(hwdephandle);
		return err ;
	}else PRINTINFO( "[%s] PCM line %d disconnected successfully \n", __FUNCTION__, dpl.pcm_line);

 	if ((err=snd_hwdep_close(hwdephandle)) < 0) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
  }
  
  return err ;
}


int sc1445x_amb_switch_to_fax(unsigned short channel);
int sc1445x_amb_switch_to_fax(unsigned short channel)
{
	snd_hwdep_t *hwdephandle;
	int err;
    sc1445x_audio_to_fax_t tfax;
  	 
	tfax.channel_index = channel;
	if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
  		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
		return -1;
	}

	if ((err = snd_hwdep_ioctl(hwdephandle, SNDRV_SC1445x_SWITCH_AUDIO_TO_FAX, &tfax))<0){
 		 
  		snd_hwdep_close(hwdephandle);
		return -1;
	} 
	 
	if ((err=snd_hwdep_close(hwdephandle)) < 0) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
  	}
	return 0;
}
int sc1445x_amb_cid_hookoff_ind( unsigned short line )
{
	snd_hwdep_t* h ;
	int err ;
	sc1445x_cid_ind_t ci ;
 	err = snd_hwdep_open( &h, ALSA_DEVICE_HW, O_RDWR ) ;
	if( err < 0 ) {

		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
		return -1;
	}
 	ci.pcm_line = line ;
	err = snd_hwdep_ioctl( h, SNDRV_SC1445x_CID_IND_OFF_HOOK, &ci ) ;
	if( err < 0 ) {
		DPRINT ("[%s] : %s\n", __FUNCTION__, snd_strerror( err ) );
		return -1 ;
	}
 	err = snd_hwdep_close( h ) ;
	if( err < 0 ) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
	}
 	return 0 ;
}

int sc1445x_amb_switch_to_voice(unsigned short channel)
{
	snd_hwdep_t *hwdephandle;
	int err;
    sc1445x_audio_to_fax_t tfax;
  	 
	tfax.channel_index = channel;
	if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
  		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
		return -1;
	}

	if ((err = snd_hwdep_ioctl(hwdephandle, SNDRV_SC1445x_SWITCH_FAX_TO_AUDIO, &tfax))<0){
 		 
  		snd_hwdep_close(hwdephandle);
		return -1;
	} 
	 
	if ((err=snd_hwdep_close(hwdephandle)) < 0) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
  	}
	return 0;
}  

int sc1445x_amb_cid_hookon_ind( unsigned short line )
{
	snd_hwdep_t* h ;
	int err ;
	sc1445x_cid_ind_t ci ;
 	err = snd_hwdep_open( &h, ALSA_DEVICE_HW, O_RDWR ) ;
	if( err < 0 ) {
		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
		return -1;
	}
 	ci.pcm_line = line ;
	err = snd_hwdep_ioctl( h, SNDRV_SC1445x_CID_IND_ON_HOOK, &ci ) ;
	if( err < 0 ) {
		DPRINT( "[%s] : %s\n", __FUNCTION__, snd_strerror( err ) );
		return -1 ;
	}
 	err = snd_hwdep_close( h ) ;
	if( err < 0 ) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
	}
	return 0 ;
}

 
int sc1445x_amb_cid_first_ring_ind( unsigned short line )
{
	snd_hwdep_t* h ;
	int err ;
	sc1445x_cid_ind_t ci ;
 	err = snd_hwdep_open( &h, ALSA_DEVICE_HW, O_RDWR ) ;
	if( err < 0 ) {
		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
		return -1;
	}
 	ci.pcm_line = line ;
	err = snd_hwdep_ioctl( h, SNDRV_SC1445x_CID_IND_FIRST_RING, &ci ) ;
	if( err < 0 ) {
		DPRINT( "[%s] : %s\n", __FUNCTION__, snd_strerror( err ) );
		return -1 ;
	}
  	err = snd_hwdep_close( h ) ;
	if( err < 0 ) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
	}
 	return 0 ;
}

int sc1445x_amb_cid_incoming_call_info( unsigned short line,char* dialled_num, char* caller_id )
{
 	snd_hwdep_t* h;
	int err ;
 	sc1445x_set_cid_info_t sci ;
 	err = snd_hwdep_open( &h, ALSA_DEVICE_HW, O_RDWR ) ;
	if( err < 0 ) {
		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
		return -1;
	}
 	sci.pcm_line =  line ;
 	//TODO: get real date
	sci.month = 1 ;
	sci.day = 8 ;
	sci.hour = 18 ;
	sci.minutes = 11 ;
  	if (dialled_num)
	{
 		if (strlen(dialled_num)<sizeof(sci.number ))
		{
			strcpy( sci.number, dialled_num) ;
   		}else
		{
			strncpy( sci.number,  dialled_num, sizeof(sci.number ) ) ;
			sci.number[sizeof(sci.number )] = '\0' ;  // be safe 
 		}
	}else sci.number[0] = '\0' ;  // be safe 

	if (caller_id) 
	{
		if (strlen(caller_id)<sizeof( sci.name ))
		{
			strcpy( sci.name, caller_id) ;
  	 	}else
		{
			strncpy( sci.name,  caller_id, sizeof(sci.name ) ) ;
			sci.name[sizeof(sci.name )] = '\0' ;  // be safe 
 		}
	}else sci.name[0] = '\0' ;  // be safe 
 	err = snd_hwdep_ioctl( h, SNDRV_SC1445x_SET_CID_INFO, &sci ) ;
	if( err < 0 ) {
		DPRINT( "[%s] :Could not pass set cid info %s\n", __FUNCTION__, snd_strerror( err ) );
		return -1 ;
	}
 	err = snd_hwdep_close( h ) ;
	if( err < 0 ) {
		DPRINT( "[%s] Could not close ALSA Device : %s\n", __FUNCTION__, snd_strerror( err ) );
	}
  	return 0 ;
}
  
int sc1445x_amb_init_pcm_line_type(int line, int pcm_type)
{
	int err = 0 ;
	snd_hwdep_t *hwdephandle;
	sc1445x_line_type_t line_type;

	if ((line<0) || (line>MAX_MCU_ALLOWED_PORTS-1)) return -1;
	if ((err = snd_hwdep_open(&hwdephandle, ALSA_DEVICE_HW, O_RDWR)) < 0) {
		DPRINT( "[%s] couldn't open alsa driver %s\n",__FUNCTION__ , snd_strerror(err) );
		return err;
	}

	line_type.line = (unsigned short)line;
	line_type.type = pcm_type;
	if ((err = snd_hwdep_ioctl(hwdephandle, SNDRV_SC1445x_SET_LINE_TYPE, &line_type ))<0){
	  DPRINT( "[%s] Could not initialize pcm port [%d] with line type [%d] : %s\n", __FUNCTION__, line, line_type.type, snd_strerror( err ) );
	  snd_hwdep_close(hwdephandle);
	  return err ;
	}else PRINTINFO( "[%s] PCM [%d] line type successfully initialized to [%d]\n", __FUNCTION__, line, line_type.type );

	return err;
}

 