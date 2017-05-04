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
 * File:		sc1445x_amb_api.h
 * Purpose:		
 * Created:		Aug 2008
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef sc1445x_amb_API_H
#define sc1445x_amb_API_H
#include <stdio.h>
#include <string.h>

#ifndef SA_CVM
#include <alsa/asoundlib.h>
#endif

#include "sc1445x_alsa_ioctl.h"
#include "sc1445x_mcu_api_defs.h"
 
// #include "../sc1445x_src/common/operation_mode_defs.h"
    
// Audio Management block 
int sc1445x_amb_init(void);
int	sc1445x_amb_sound_open(int bits, int stereo, int rate);
int	sc1445x_amb_sound_close(void);
int sc1445x_amb_get_capture_fd(void);
int sc1445x_amb_get_playback_fd(void);
int sc1445x_amb_sound_read(unsigned char *audioData, int samples);
int sc1445x_amb_sound_write(unsigned char *audioData, int samples);
int sc1445x_amb_select_audio_iface(sc1445x_ae_iface_mode audioIface);
int sc1445x_amb_set_virtual_spk_volume(unsigned short spkgain);
int sc1445x_amb_get_volume_level_count(unsigned short *vol_level_count, unsigned short *gain_level_count);
int sc1445x_amb_get_virtual_spk_volume(void);
int sc1445x_amb_increase_virtual_spk_volume(void);
int sc1445x_amb_decrease_virtual_spk_volume(void);
int sc1445x_amb_set_mute(int flag);
int sc1445x_amb_activate_channel( sc1445x_ae_codec_type encoder,sc1445x_ae_codec_type decoder, unsigned short* ch, int line );
int sc1445x_amb_deactivate_channel( unsigned short ch,int line );
int sc1445x_amb_start_dynamic_tone(sc1445x_ae_std_tone cpTone ,unsigned short line);
int sc1445x_amb_start_periodic_tone(sc1445x_ae_std_tone cpTone ,unsigned short line);

int sc1445x_amb_switch_to_voice(unsigned short channel);
int sc1445x_amb_start_callprogress_tone(sc1445x_ae_std_tone cpTone, unsigned short line);
int sc1445x_amb_stop_callprogress_tone(unsigned short line);
int sc1445x_amb_status_callprogress_tone(unsigned short line);

int sc1445x_amb_play_dtmf(sc1445x_ae_std_tone dtmfKey, int sendToNet, int line);
int sc1445x_amb_stop_dtmf(unsigned short line);
int sc1445x_amb_status_dtmf(unsigned short line );

int sc1445x_amb_set_mode( sc1445x_ae_mode  mode); 
int sc1445x_amb_sidetone_volume(void);
int sc1445x_amb_set_aec(unsigned short onoff);
int sc1445x_amb_internal_connect_lines(unsigned short line1, unsigned short line2);
int sc1445x_amb_internal_disconnect_line(int line);
int sc1445x_amb_cid_hookoff_ind( unsigned short line );
int sc1445x_amb_cid_hookon_ind( unsigned short line );
int sc1445x_amb_cid_first_ring_ind( unsigned short line );
int sc1445x_amb_cid_incoming_call_info( unsigned short line,char* dialled_num, char* caller_id );
  
int sc1445x_amb_init_pcm_line_type(int line, int pcm_type);
  
#endif //sc1445x_amb_API_H
