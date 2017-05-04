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
 * File:		 		 sc1445x_alsa.c
 * Purpose:		 		 
 * Created:		 		 Jan 2008
 * By:		 		 	 YP
 * Country:		 		 Greece
 *
 *-----------------------------------------------------------------------------
 */
/*========================== Include files ==================================*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <values.h>
#include "sc1445x_alsa.h"
 
#include <alsa/asoundlib.h>
#include <ortp/ortp.h>
 
  
 /*************************************/ 
static snd_pcm_uframes_t period_size ;
static snd_pcm_uframes_t buffer_size ;
  
//  ALSA DRIVER API
  int sc1445x_alsa_can_read(snd_pcm_t *dev, int frames){
	if (dev==NULL) return -1;
	return snd_pcm_avail_update(dev)>=frames;
}

  int sc1445x_alsa_read(snd_pcm_t *handle,unsigned char *buf,int nsamples)
{
	int err;
	err=snd_pcm_readi(handle,buf,nsamples);

	if (err==-EPIPE){
		snd_pcm_prepare(handle ) ;
		snd_pcm_start( handle ) ;
	}
	return err;
}

 
int sc1445x_alsa_write(snd_pcm_t *handle,unsigned char *buf,int nsamples)
{
	int err;
	snd_pcm_sframes_t avail ;
#if 0
	static int first_time = 1 ;
#endif

	if (handle==NULL) return -1;

#if 0
	if( first_time ) 
	{
		err=snd_pcm_start(handle);
		if (err<0){
			printf("write snd_pcm_start() failed: %s\n", snd_strerror(err));
		}
		first_time = 0 ;
	} 
#endif

	avail = snd_pcm_avail_update( handle ) ;
	if( avail < nsamples ) {
//		 snd_pcm_start( handle ) ;
		 //printf( "avail for playback is %d\n", avail ) ;
	}

	if ((err=snd_pcm_writei(handle,buf,nsamples))<0){
 
		//telis 22 Feb 2008
// 			printf(  "snd_pcm_writei() XRUN "				"-- recovery?  \n" ) ;
			err = snd_pcm_prepare( handle ) ;
			if( err < 0 ) {
//				printf( "FATAL ERROR ----- could not recover from XRUN: \n\n\n\n\n" ) ;
				return -1;
			}
			snd_pcm_start( handle ) ;
			err =snd_pcm_writei(handle,buf,nsamples);
// 			if( err != period_size )
//				printf( "(re)snd_pcm_writei --> %d\n", res ) ;
		 
/*
		if (err==-EPIPE){
			snd_pcm_prepare(handle);
			err=snd_pcm_writei(handle,buf,nsamples);
			if (err<0) printf("alsa_card_write: Error writing sound buffer \n");
		}else if (err!=-EWOULDBLOCK){
			printf("alsa_card_write: snd_pcm_writei() failed \n");
		}
*/

	}else if (err!=nsamples) {
;//		printf("Only  ... samples written instead of ");
	}

	return err;
}
int sc1445x_set_params(int rate , snd_pcm_t *pcm_handle)
 {
	int res = 0 ;
 	snd_pcm_hw_params_t* hwparams = NULL ;
	snd_pcm_sw_params_t* swparams = NULL ;
 	unsigned exact_rate ;

//	rate = ALSA_MAX_BYTES*100 ;  /* 3 * (2 + 80) bytes per 10 msec */
	period_size = rate / 100 ;
	buffer_size = 1 * period_size ;//v1.0.1.1

	/* set the hardware parameters */
	snd_pcm_hw_params_alloca( &hwparams ) ;
	snd_pcm_hw_params_any( pcm_handle, hwparams ) ;

	res = snd_pcm_hw_params_set_access( pcm_handle, hwparams,
					SND_PCM_ACCESS_RW_INTERLEAVED ) ;
	if( res < 0 ) {
		fprintf( stderr, "could not set access: %s\n",
							snd_strerror( res ) ) ;
		return res ;
	}

	res = snd_pcm_hw_params_set_format( pcm_handle, hwparams,
						SND_PCM_FORMAT_S8 ) ;
	if( res < 0 ) {
		fprintf( stderr, "could not set format: %s\n",
							snd_strerror( res ) ) ;
		return res ;
	}

	res = snd_pcm_hw_params_set_channels( pcm_handle, hwparams, 1 ) ;
	if( res < 0 ) {
		fprintf( stderr, "could not set channels: %s\n",
							snd_strerror( res ) ) ;
		return res ;
	}

	exact_rate = rate ;
	res = snd_pcm_hw_params_set_rate_near( pcm_handle, hwparams,
							&exact_rate, NULL ) ;
	if( res < 0 ) {
		fprintf( stderr, "could not set rate: %s\n",
							snd_strerror( res ) ) ;
		return res ;
	}
	if( exact_rate != rate ) {
		printf( "could not set rate to %u Hz; it was set to %u"		" instead\n", rate, exact_rate ) ;
	}

	res = snd_pcm_hw_params_set_period_size_near( pcm_handle, hwparams,
							&period_size, NULL ) ;
	if( res < 0 ) {
		fprintf( stderr, "could not set period size: %s\n",		snd_strerror( res ) ) ;
		return res ;
	}

	res = snd_pcm_hw_params_set_buffer_size_near( pcm_handle, hwparams,
								&buffer_size ) ;
	if( res < 0 ) {
		fprintf( stderr, "could not set buffer size: %s\n",
							snd_strerror( res ) ) ;
		return res ;
	}

	res = snd_pcm_hw_params( pcm_handle, hwparams ) ;
	if( res < 0 ) {
		fprintf( stderr, "could not set new hw params: %s\n",
							snd_strerror( res ) ) ;
		return res ;
	}


	snd_pcm_hw_params_get_period_size( hwparams, &period_size, 0 ) ;
	snd_pcm_hw_params_get_buffer_size( hwparams, &buffer_size ) ;

//	printf( "hw params set: rate=%uHz, period_size=%u bytes, "	"buffer_size=%u bytes\n", exact_rate,period_size, buffer_size ) ;

	/* set the software parameters */
	snd_pcm_sw_params_alloca( &swparams ) ;
	snd_pcm_sw_params_current( pcm_handle, swparams ) ;

	res = snd_pcm_sw_params_set_start_threshold( pcm_handle, swparams,
								MAXINT/*period_size*/ ) ;
	if( res < 0 ) {
		fprintf( stderr, "could not set start threshold: %s\n",
							snd_strerror( res ) ) ;
		return res ;
	}

	res = snd_pcm_sw_params_set_avail_min( pcm_handle, swparams,
								period_size ) ;
	if( res < 0 ) {
		fprintf( stderr, "could not set start min available: %s\n",
							snd_strerror( res ) ) ;
		return res ;
	}

#if 1 
	/* turn off stopping due to too many available frames */
	res = snd_pcm_sw_params_set_stop_threshold( pcm_handle, swparams,
								MAXINT ) ;
	if( res < 0 ) {
		fprintf( stderr, "could not set stop threshold: %s\n",
							snd_strerror( res ) ) ;
		return res ;
	}
#endif

	res = snd_pcm_sw_params_set_xfer_align( pcm_handle, swparams, 1 ) ;
	if( res < 0 ) {
		fprintf( stderr, "could not set xfer align: %s\n",
							snd_strerror( res ) ) ;
		return res ;
	}

	res = snd_pcm_sw_params( pcm_handle, swparams ) ;
	if( res < 0 ) {
		fprintf( stderr, "could not set new sw params: %s\n",
							snd_strerror( res ) ) ;
		return res ;
	}

	/* print out the configuration */
//	snd_pcm_sw_params_dump( swparams, log ) ;
//	snd_pcm_dump( pcm_handle, log ) ;

	return res ;
}




  