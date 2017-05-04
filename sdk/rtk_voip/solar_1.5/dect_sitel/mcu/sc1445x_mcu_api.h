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
 * File:		sc1445x_mcu_api.h
 * Purpose:		
 * Created:		Aug 2008
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef sc1445x_mcu_API_H
#define sc1445x_mcu_API_H
 
#include "sc1445x_mcu_block.h"
#include "sc1445x_mcu_api_defs.h"
int sc1445x_mcu_terminate(sc1445x_mcu_instance_t *pMcuInstance);
int sc1445x_mcu_create(void);
 
 int sc1445x_mcu_media_open(unsigned short *pHandle, const sc1445x_mcu_media_type mediaType);
sc1445x_mcu_instance_t *sc1445x_get_mcu_instance(unsigned short handle);
int sc1445x_mcu_configure(void* pHandler,const sc1445x_mcu_media_ctrl_mode mediaMode, sc1445x_mcu_audio_media_stream_t *pMediaStream);
int sc1445x_mcu_set_tone(sc1445x_mcu_tone_bmp toneBMP, sc1445x_mcu_key keyDtmf,unsigned short line );
int sc1445x_mcu_set_mode(sc1445x_mcu_audio_mode audMode);
int sc1445x_mcu_set_volume(unsigned short gain);
int sc1445x_mcu_get_volume_level_count(unsigned short *vol_level_count, unsigned short *gain_level_count);

int sc1445x_mcu_get_virtual_spk_volume(void);
int sc1445x_mcu_increase_volume(void);
int sc1445x_mcu_decrease_volume(void);

int sc1445x_mcu_mute(int muteFlag);
int sc1445x_mcu_set_sidetone(void);
int sc1445x_mcu_set_aec(unsigned short state);

void sc1445x_mcu_send_dtmf(const sc1445x_mcu_key key , unsigned short line);
void sc1445x_mcu_send_dtmf_rfc2833(const sc1445x_mcu_key key  );
void sc1445x_mcu_send_dtmf_inband(const sc1445x_mcu_key key, unsigned short line);
sc1445x_ae_std_tone convertDtmf(const sc1445x_mcu_key key);
 


#endif //sc1445x_mcu_API_H
