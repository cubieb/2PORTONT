
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
 * File:		sc1445x_alsa.h
 * Purpose:		
 * Created:		Dec 2008
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef sc1445x_ALSA_H
#define sc1445x_ALSA_H

#ifndef SA_CVM
#include <alsa/asoundlib.h>

#include "sc1445x_alsa_ioctl.h"
  
//Alsa API
 int sc1445x_alsa_can_read(snd_pcm_t *dev, int frames);
 int sc1445x_alsa_read(snd_pcm_t *handle,unsigned char *buf,int nsamples);
 int sc1445x_alsa_write(snd_pcm_t *handle,unsigned char *buf,int nsamples); 
 int sc1445x_set_params(int rate, snd_pcm_t *pcm_handle);
#endif // SA_CVM

#endif //sc1445x_ALSA_H
