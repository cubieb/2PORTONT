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
 * File:		 		 sc1445x_mcu_api.c
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
#include <ortp/ortp.h>
#include "sc1445x_amb_api.h"
#include "sc1445x_media.h"
#include "sc1445x_ortp.h"
#include "sc1445x_mcu_api.h"
#include "sc1445x_mcu_debug.h"
//19 June, 2009
extern pthread_mutex_t McuProtectMutex;                                               
 
int sc1445x_mcu_create(void)
{
	sc1445x_mcu_init();
 
	return 0;
}

int sc1445x_mcu_terminate(sc1445x_mcu_instance_t *pMcuInstance)
{
	 sc1445x_mcu_instance_free(pMcuInstance);
 	return 0;
}

int sc1445x_mcu_media_open(unsigned short *pHandle, const sc1445x_mcu_media_type mediaType)
 
{	
	switch (mediaType)
 	{
	    case MCU_MTYPE_AUDIO_ID:
             if (pHandle) 
            {
                *pHandle = (unsigned short)sc1445x_mcu_instance_allocate();
                return 0;
            }
            else
                DPRINT( "[%s] Err:pHandle NULL!\n", __FUNCTION__);
  
			break;
  		case MCU_MTYPE_VIDEO_ID:
		case MCU_MTYPE_APP_ID:
		case MCU_MTYPE_GDATA_ID:
		case MCU_MTYPE_IMG_ID:
			DPRINT( "[%s] Media Type  %d not supported\n", __FUNCTION__, mediaType);
 			break;
 	}
	return -1;
}

int sc1445x_mcu_configure(void* pMcuInstance,const sc1445x_mcu_media_ctrl_mode mediaMode, sc1445x_mcu_audio_media_stream_t *pMediaStream)
{
	int ret;
	sc1445x_mcu_instance_t* pInstance = (sc1445x_mcu_instance_t*)pMcuInstance;

 	if (!pMcuInstance) return -1;

 	switch (mediaMode)
	{
		case MCU_MCTRL_START_ID:
			if ( !pMediaStream) return -1;
			//fix 19 June 2009
			pthread_mutex_lock (&McuProtectMutex);
	   		ret= sc1445x_mcu_instance_start(pMcuInstance,pMediaStream );
			if (ret==-1) pthread_mutex_unlock (&McuProtectMutex);
 			return ret;
 		case MCU_MCTRL_MODIFY_ID:
 			if ( !pMediaStream) return -1;
  			return sc1445x_mcu_instance_modify(pMcuInstance,pMediaStream );
 			break;
 		case MCU_MCTRL_STOP_ID:
			// fix May 14/05/2009
 			if(/*pInstance->state != MCU_CALL_STATE_ALLOCATED &&*/ pInstance->state != MCU_CALL_STATE_IDLE)
			{	
				//fix 19 June 2009
				pthread_mutex_lock (&McuProtectMutex);
			    ret= sc1445x_mcu_instance_destroy(pMcuInstance);
				if (ret==-1) pthread_mutex_unlock (&McuProtectMutex);
				return ret;
			}
            else
                return -1;

 		case MCU_MCTRL_LOOP_ID:
			if ( !pMediaStream) return -1;
   	  		pInstance->mediaStream.mediaAttr = pMediaStream->mediaAttr;
			if (pInstance->mediaStream.mediaAttr == MCU_MATTR_BOTH_ID) 
			{	
 				return sc1445x_mcu_instance_start_loop(pInstance,pMediaStream );
			}
			else {
 	  			return sc1445x_mcu_instance_destroy_loop(pInstance);
			}
			break;
 	}
	return 0;
}

int sc1445x_mcu_set_tone(sc1445x_mcu_tone_bmp toneBMP, sc1445x_mcu_key keyDtmf,unsigned short line )
{
	int ret;
  	if (toneBMP == MCU_TONE_NA_BT) {
		ret = sc1445x_amb_status_callprogress_tone(line);

		if (sc1445x_amb_status_dtmf(line)) 
			sc1445x_amb_stop_dtmf(line);
			
		if (ret)
			 sc1445x_amb_stop_callprogress_tone(line);
	}
	else if (toneBMP == MCU_TONE_DT_BT)
	{
  		sc1445x_mcu_send_dtmf(keyDtmf,line);
	}
	else{ 
		//fix May 13/05/2009
		if (sc1445x_amb_status_callprogress_tone(line))
			sc1445x_amb_stop_callprogress_tone(line);  
  		return sc1445x_amb_start_callprogress_tone( convertTone(toneBMP), line);
 	}

  return 0;
}	

int sc1445x_mcu_set_mode(sc1445x_mcu_audio_mode audMode)
{
   	return sc1445x_amb_select_audio_iface(convertAudioIface(audMode));
}
int sc1445x_mcu_set_volume(unsigned short level)
{
   	return sc1445x_amb_set_virtual_spk_volume(level);

}
int sc1445x_mcu_get_volume_level_count(unsigned short *vol_level_count, unsigned short *gain_level_count)
{
   	return sc1445x_amb_get_volume_level_count(vol_level_count,gain_level_count);

}

int sc1445x_mcu_get_virtual_spk_volume(void)
{
	return sc1445x_amb_get_virtual_spk_volume();
}

int sc1445x_mcu_increase_volume(void)
{
   	return sc1445x_amb_increase_virtual_spk_volume();

}
 int sc1445x_mcu_decrease_volume(void)
{
   	return sc1445x_amb_decrease_virtual_spk_volume();

}

int sc1445x_mcu_mute(int muteFlag)
{
    return sc1445x_amb_set_mute(muteFlag);
}
int sc1445x_mcu_set_sidetone(void)
{
	return sc1445x_amb_sidetone_volume();
}

int sc1445x_mcu_set_aec(unsigned short state)
{
	return sc1445x_amb_set_aec((unsigned short) state);
}
 