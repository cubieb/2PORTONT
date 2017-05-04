
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
 * File:		sc1445x_mcu_block.h
 * Purpose:		
 * Created:		Aug 2008
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef sc1445x_mcu_BLOCK_H
#define sc1445x_mcu_BLOCK_H
  
#include "sc1445x_amb_api.h"
#include "sc1445x_mcu_api_defs.h"

typedef enum _sc1445x_mcu_call_state
{
	MCU_CALL_STATE_IDLE					=0x0000000,	
	MCU_CALL_STATE_ALLOCATED			=0x0000001,
	MCU_CALL_STATE_CONNECTED			=0x0000010,
	MCU_CALL_STATE_CONNECTED_RX			=0x0000100,
	MCU_CALL_STATE_CONNECTED_TX			=0x0001000,
	MCU_CALL_STATE_CONNECTED_RX_TX		=0x0001110,
	MCU_CALL_STATE_RELEASED				=0x1000000	
}sc1445x_mcu_call_state;

 
int sc1445x_mcu_init(void);
unsigned short sc1445x_mcu_instance_allocate(void);
void sc1445x_mcu_instance_free(sc1445x_mcu_instance_t *pMcuInstance);

int sc1445x_mcu_scheduler_instance_reopen(sc1445x_mcu_instance_t *pMcuInstance,sc1445x_mcu_audio_media_stream_t *pMediaStream );
int sc1445x_mcu_instance_start(sc1445x_mcu_instance_t *pMcuInstance, sc1445x_mcu_audio_media_stream_t *pMediaStream ) ;
int sc1445x_mcu_instance_destroy(sc1445x_mcu_instance_t *pMcuInstance);
int sc1445x_mcu_instance_modify(sc1445x_mcu_instance_t *pMcuInstance, sc1445x_mcu_audio_media_stream_t *pMediaStream ) ;
 
int sc1445x_mcu_instance_start_loop(sc1445x_mcu_instance_t *pMcuInstance, sc1445x_mcu_audio_media_stream_t *pMediaStream ) ;
int sc1445x_mcu_instance_destroy_loop(sc1445x_mcu_instance_t *pMcuInstance);

int sc1445x_mcu_audio_channel_activation(sc1445x_mcu_instance_t *pHandler);
int sc1445x_mcu_set_telephone_event_profile(int type);
sc1445x_ae_std_tone convertTone(const sc1445x_mcu_tone_bmp toneBMP);
sc1445x_ae_std_tone convertDtmf(const sc1445x_mcu_key dtmfkey);
sc1445x_ae_iface_mode convertAudioIface(const sc1445x_mcu_audio_mode audioMode);

void sc1445x_mcu_getAddress(unsigned char* address);
void sc1445x_mcu_getAddress_v6(unsigned char* address);
void sc1445x_mcu_endpoint_initiate(int , audio_frames_buffer_t* pAmbBufferHandler, int rxPTime,int txPTime);
#endif //sc1445x_mcu_BLOCK_H
