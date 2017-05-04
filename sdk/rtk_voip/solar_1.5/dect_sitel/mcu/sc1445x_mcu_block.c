
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
 * File:		 		 sc1445x_mcu_block.c
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
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <net/if.h>
#ifdef SIP_IPv6
#include <ifaddrs.h>
#endif
#include <ortp/ortp.h>
  
#include "sc1445x_mcu_block.h"
#include "sc1445x_media.h"
#include "sc1445x_ortp.h"
#include "sc1445x_amb_api.h"
#include "sc1445x_mcu_scheduler.h"
#include "sc1445x_t38layer.h"

#ifdef SRTP_ENABLED
	#include "sc1445x_srtp.h"
#endif

#include "sc1445x_mcu_debug.h"

sc1445x_mcu_instance_t mcuInstance[MAX_MCU_SUPPORTED_CALLS];

int sc1445x_mcu_init(void)
{
    unsigned short h;
	sc1445x_amb_init();
   	sc1445x_ortp_init();
  	sc1445x_media_init( );
	sc1445x_mcu_scheduler_init(); 
	memset(mcuInstance,0,sizeof (sc1445x_mcu_instance_t)*MAX_MCU_SUPPORTED_CALLS);
     
    for (h=1; h<=MAX_MCU_SUPPORTED_CALLS; h++) 
  		    mcuInstance[h-1].handle = h;
      
 	return 0;	 
}

unsigned short sc1445x_mcu_instance_allocate(void)
{
	int i;
	unsigned short handle;
	for (i=0;i<MAX_MCU_SUPPORTED_CALLS;i++)
	{
		if (mcuInstance[i].state == MCU_CALL_STATE_IDLE)
		{
			handle = mcuInstance[i].handle;
			memset(&mcuInstance[i], 0, sizeof(sc1445x_mcu_instance_t));
			mcuInstance[i].handle = handle;
  			mcuInstance[i].state = MCU_CALL_STATE_ALLOCATED;
			PRINTINFO( "[%s]  allocate new handler[%d] \n", __FUNCTION__,(int)handle);
  			return (unsigned short)mcuInstance[i].handle;
		}
	}
	return (unsigned short)0;
}

 sc1445x_mcu_instance_t *sc1445x_get_mcu_instance(unsigned short handle)
{
	int i;
 	for (i=0;i<MAX_MCU_SUPPORTED_CALLS;i++)
	{
		if (mcuInstance[i].handle == handle)
		{
			PRINTINFO( "[%s]  get new instance [%x] \n", __FUNCTION__,(int)&mcuInstance[i]);
			return &mcuInstance[i];
		}
	}
	return NULL;
}

void sc1445x_mcu_instance_free(sc1445x_mcu_instance_t *pMcuInstance)
{
	int i;
 	for (i=0;i<MAX_MCU_SUPPORTED_CALLS;i++)
	{
		if (&mcuInstance[i]==pMcuInstance)
		{
			//fix 18 June, 2009
		 	PRINTINFO("[%s] freeing mcu instance [%x]\n ", __FUNCTION__, (int)pMcuInstance); 
			mcuInstance[i].state = MCU_CALL_STATE_IDLE;
			break  ;
		}
	} 
 }

int sc1445x_mcu_instance_start(sc1445x_mcu_instance_t *pMcuInstance,sc1445x_mcu_audio_media_stream_t *pMediaStream ) 
{
	int ret;
	unsigned char localIPAddr[64];
 	unsigned char remoteIPAddr[64];
	//fix 19 June, 2009
 
 	PRINTINFO( "[%s]  activate a new instance [%x] \n", __FUNCTION__,(int)pMcuInstance );
#ifdef  SIP_IPv6 
	strcpy(remoteIPAddr,pMediaStream->mediaParams.rAddress);
	sc1445x_mcu_getAddress_v6(localIPAddr);
#else
	sprintf((char*)remoteIPAddr, "%d.%d.%d.%d",  pMediaStream->mediaParams.rAddress[0],pMediaStream->mediaParams.rAddress[1], pMediaStream->mediaParams.rAddress[2], pMediaStream->mediaParams.rAddress[3]);
	sc1445x_mcu_getAddress(localIPAddr);
#endif
 	memcpy((char*)&pMcuInstance->mediaStream,(char*)pMediaStream,sizeof(sc1445x_mcu_audio_media_stream_t));
  	// Create RTP connection
  	pMcuInstance->pRtpSession =  sc1445x_ortp_stream_open (localIPAddr, 
													remoteIPAddr, 
													pMediaStream->mediaParams.lport, 
													pMediaStream->mediaParams.rport,
													(int)(pMediaStream->mediaParams.rtp_ptype),
													(int)(pMediaStream->mediaParams.ptype), 
													/*pMediaStream->mediaParams.Jitter*/40,(int)pMediaStream->mediaParams.tos );
 
	if (pMcuInstance->pRtpSession<=0) 		
	{
		goto _exit_start;
	}

#ifdef SRTP_ENABLED
  	ret = srtp_create_session(pMcuInstance->pRtpSession, &pMcuInstance->pSrtpSession, &pMediaStream->mediaParams.srtp_params ); 
	if (ret<0) {
		goto _exit_start;
	}
#endif

	if (sc1445x_amb_sound_open(8,0,ALSA_MAX_BYTES*100)) {
  		DPRINT( "[%s]Couldn't open sound ALSA Driver \n", __FUNCTION__);
		goto _exit_start;
	} 
 	 
	#ifdef REMOTE_LOOPBACK_TEST
		if (pMcuInstance->mediaStream.mediaAttr== MCU_MATTR_RLOOP_ID)
			loop_add_session_transport(pMcuInstance);
	#endif

 	sc1445x_mcu_endpoint_initiate((int)pMediaStream->mediaParams.ptype , &pMcuInstance->audioFramesBuffer,(int)pMcuInstance->mediaStream.mediaParams.rxRate,(int)pMcuInstance->mediaStream.mediaParams.txRate);    
 
	if (!sc1445x_mcu_audio_channel_activation(pMcuInstance))
	{

		if (pMediaStream->mediaAttr  == MCU_MATTR_SND_ID)
			pMcuInstance->state =(int) MCU_CALL_STATE_CONNECTED_TX;  
 		else if (pMediaStream->mediaAttr == MCU_MATTR_RCV_ID)
			pMcuInstance->state =(int) MCU_CALL_STATE_CONNECTED_RX;  
		else pMcuInstance->state =(int) MCU_CALL_STATE_CONNECTED_RX_TX;  
  	}else{
		DPRINT( "[%s]Couldn't activate audio channel \n", __FUNCTION__);
		goto _exit_start;
	}
    
	if (sc1445x_mcu_scheduler_add(pMcuInstance)!=0)
	{
 		DPRINT( "[%s]Couldn't add a new instance on MCU task \n", __FUNCTION__);
		goto _exit_start;
	}
 	PRINTINFO( "[%s]  A new instance is activated [%x] \n", __FUNCTION__,(int)pMcuInstance );
	return 0;
_exit_start:
	//fix 19 June, 2009
 	return -1;
}
  
int sc1445x_mcu_scheduler_instance_reopen(sc1445x_mcu_instance_t *pMcuInstance,sc1445x_mcu_audio_media_stream_t *pMediaStream ) 
{
	int ret;
	unsigned char localIPAddr[64];
 	unsigned char remoteIPAddr[64];
	//fix 19 June, 2009
  	PRINTINFO( "[%s]  reopen a new instance [%x] \n", __FUNCTION__,(int)pMcuInstance );
 	#ifdef  SIP_IPv6 
		strcpy(remoteIPAddr,pMediaStream->mediaParams.rAddress);
		sc1445x_mcu_getAddress_v6(localIPAddr);
	#else
		sprintf((char*)remoteIPAddr, "%d.%d.%d.%d",  pMediaStream->mediaParams.rAddress[0],pMediaStream->mediaParams.rAddress[1], pMediaStream->mediaParams.rAddress[2], pMediaStream->mediaParams.rAddress[3]);
		sc1445x_mcu_getAddress(localIPAddr);
	#endif

	memcpy((char*)&pMcuInstance->mediaStream,(char*)pMediaStream,sizeof(sc1445x_mcu_audio_media_stream_t));
  	
	// Create RTP connection
  	pMcuInstance->pRtpSession =  sc1445x_ortp_stream_open (localIPAddr, 
													remoteIPAddr, 
													pMediaStream->mediaParams.lport, 
													pMediaStream->mediaParams.rport,
													 pMediaStream->mediaParams.rtp_ptype, 
													 pMediaStream->mediaParams.ptype,
													/*pMediaStream->mediaParams.Jitter*/40, pMediaStream->mediaParams.tos  );
 
	if (pMcuInstance->pRtpSession<=0) 		
	{
		goto _exit_start;
	}

#ifdef SRTP_ENABLED
  	ret = srtp_create_session(pMcuInstance->pRtpSession, &pMcuInstance->pSrtpSession, &pMediaStream->mediaParams.srtp_params ); 
	if (ret<0) {
		goto _exit_start;
	}
#endif

	if (sc1445x_amb_sound_open(8,0,ALSA_MAX_BYTES*100)) {
  		DPRINT( "[%s]Couldn't open sound ALSA Driver \n", __FUNCTION__);
		goto _exit_start;
	} 
 	 
	#ifdef REMOTE_LOOPBACK_TEST
		if (pMcuInstance->mediaStream.mediaAttr== MCU_MATTR_RLOOP_ID)
			loop_add_session_transport(pMcuInstance);
	#endif

 	sc1445x_mcu_endpoint_initiate((int)pMediaStream->mediaParams.ptype,&pMcuInstance->audioFramesBuffer,(int)pMcuInstance->mediaStream.mediaParams.rxRate,(int)pMcuInstance->mediaStream.mediaParams.txRate);    
 
	if (!sc1445x_mcu_audio_channel_activation(pMcuInstance))
	{

		if (pMediaStream->mediaAttr  == MCU_MATTR_SND_ID)
			pMcuInstance->state =(int) MCU_CALL_STATE_CONNECTED_TX;  
 		else if (pMediaStream->mediaAttr == MCU_MATTR_RCV_ID)
			pMcuInstance->state =(int) MCU_CALL_STATE_CONNECTED_RX;  
		else pMcuInstance->state =(int) MCU_CALL_STATE_CONNECTED_RX_TX;  
  	}else{
		DPRINT( "[%s]Couldn't activate audio channel \n", __FUNCTION__);
		goto _exit_start;
	}
    
	if (sc1445x_mcu_scheduler_add(pMcuInstance)!=0)
	{
 		DPRINT( "[%s]Couldn't add a new instance on MCU task \n", __FUNCTION__);
		goto _exit_start;
	}
 	PRINTINFO( "[%s]  A new instance is activated [%x] \n", __FUNCTION__,(int)pMcuInstance );
	return 0;
_exit_start:
	//fix 19 June, 2009
 	return -1;
}

int sc1445x_mcu_instance_destroy(sc1445x_mcu_instance_t *pMcuInstance)
{
 	return  sc1445x_mcu_scheduler_del(pMcuInstance);
}
 
// fix 18 June, 2009
int sc1445x_mcu_instance_modify(sc1445x_mcu_instance_t *pMcuInstance,sc1445x_mcu_audio_media_stream_t *pMediaStream )  
{
	int addr_changed =0;
	// FAX Stream 
 
	if (pMediaStream->mediaType == MCU_MTYPE_FAX_ID) 
	{
		#ifdef SC1445x_AE_SUPPORT_FAX
			if (pMcuInstance->mediaStream.mediaType == MCU_MTYPE_AUDIO_ID)
			{
		 		unsigned char remoteIPAddr[16];

        /* Currently only ALSA channel "0" can switch to fax due to DSP limitations */
        if( pMcuInstance->alsaChannel )
        {
          DPRINT( "[%s] Only CHNL 0 can swhich to FAX!!! \n", __FUNCTION__);
 			    return -1;
        }

				sc1445x_ortp_stream_close(pMcuInstance->pRtpSession); 
  				pMcuInstance->mediaStream.mediaParams.lport  =  pMediaStream->mediaParams.lport ;  
				pMcuInstance->mediaStream.mediaParams.rport  =  pMediaStream->mediaParams.rport ;  
 				pMcuInstance->pRtpSession = NULL;
 				sprintf(remoteIPAddr, "%d.%d.%d.%d", pMediaStream->mediaParams.rAddress[0],pMediaStream->mediaParams.rAddress[1],pMediaStream->mediaParams.rAddress[2],pMediaStream->mediaParams.rAddress[3]);
			//[FTC]
				sc1445x_amb_switch_to_fax(pMcuInstance->alsaChannel);
   				sc1445x_t38_stream_open( remoteIPAddr,pMediaStream->mediaParams.lport,pMediaStream->mediaParams.rport, pMcuInstance->alsaChannel);
				pMcuInstance->mediaStream.mediaType = pMediaStream->mediaType;
				pMcuInstance->mediaStream.mediaFormat = pMediaStream->mediaFormat;
 			}else printf("FAX already .... \n");
		#endif

	}else if (pMediaStream->mediaType == MCU_MTYPE_AUDIO_ID) //Audio Stream 
	{
 
		if (pMcuInstance->mediaStream.mediaType != MCU_MTYPE_AUDIO_ID)
		{
			DPRINT( "[%s]Not Available action \n", __FUNCTION__);
 			return -1;//fix April 29, 2009
		}
		if (pMediaStream->mediaParams.rAddress[0] && pMediaStream->mediaParams.rAddress[1] && pMediaStream->mediaParams.rAddress[2] && pMediaStream->mediaParams.rAddress[3])
		{
			if (memcmp(pMcuInstance->mediaStream.mediaParams.rAddress, pMediaStream->mediaParams.rAddress,4))
				addr_changed=1;
		}
		
		if ((pMcuInstance->mediaStream.mediaParams.lport != pMediaStream->mediaParams .lport) ||
		  (pMcuInstance->mediaStream.mediaParams.rport  != pMediaStream->mediaParams.rport) ||
		 	 (addr_changed))
		{
 			memcpy(&pMcuInstance->mediaStream,pMediaStream, sizeof(sc1445x_mcu_audio_media_stream_t));
			PRINTINFO("[%s] mcu instance has been modified, send a notification for reseting it [%x]\n ", __FUNCTION__, (int) pMcuInstance);
  			sc1445x_mcu_scheduler_resetInstance(pMcuInstance);
 			return 0;
		}
 
  		if (pMcuInstance->mediaStream.mediaAttr != pMediaStream->mediaAttr)
		{
			sc1445x_mcu_media_attr tmpAttr = pMcuInstance->mediaStream.mediaAttr;	
			pMcuInstance->mediaStream.mediaAttr = pMediaStream->mediaAttr;
		
 			switch( pMediaStream->mediaAttr)
			{	
				case MCU_MATTR_RCV_ID:
 				case MCU_MATTR_SND_ID:
 				case MCU_MATTR_BOTH_ID:
					if (tmpAttr==MCU_MATTR_INA_ID) {
 						if (pMcuInstance->mediaStream.mediaParams.ptype!= pMediaStream->mediaParams.ptype)
						{
  							pMcuInstance->mediaStream.mediaParams.ptype = pMediaStream->mediaParams.ptype;
							pMcuInstance->mediaStream.mediaParams.rtp_ptype  = pMediaStream->mediaParams.rtp_ptype;
							sc1445x_mcu_scheduler_resetInstance(pMcuInstance);
							return 0;
  							//sc1445x_mcu_scheduler_changeCodec(pMcuInstance); 
						} else{
  							pMcuInstance->mediaStream.mediaParams.ptype = pMediaStream->mediaParams.ptype;
							pMcuInstance->mediaStream.mediaParams.rtp_ptype  = pMediaStream->mediaParams.rtp_ptype;
 							sc1445x_mcu_scheduler_resetInstance(pMcuInstance);
							return 0;

    				//		sc1445x_mcu_scheduler_resume(pMcuInstance);
					//		return 0;
						}

					}
					else{
 						if (pMcuInstance->mediaStream.mediaAttr == MCU_MATTR_SND_ID)
							pMcuInstance->state =(int) MCU_CALL_STATE_CONNECTED_TX;  
 						else if (pMcuInstance->mediaStream.mediaAttr == MCU_MATTR_RCV_ID)
							pMcuInstance->state =(int) MCU_CALL_STATE_CONNECTED_RX;  
						else pMcuInstance->state =(int) MCU_CALL_STATE_CONNECTED_RX_TX;  
  					}
		 			break;
				case MCU_MATTR_INA_ID:
      			sc1445x_mcu_scheduler_suspend(pMcuInstance);
				return 0;
  				break;

				default: 
					sc1445x_mcu_scheduler_suspend(pMcuInstance);
					break;
 			}
			
  		} 

		if (pMcuInstance->mediaStream.mediaParams.ptype!= pMediaStream->mediaParams.ptype)
		{

			printf("MCU MODIFY STEP 4 [%d][%d] \n",pMediaStream->mediaParams.ptype,pMediaStream->mediaParams.rtp_ptype);

 			pMcuInstance->mediaStream.mediaParams.ptype = pMediaStream->mediaParams.ptype;
			pMcuInstance->mediaStream.mediaParams.rtp_ptype  = pMediaStream->mediaParams.rtp_ptype;
			sc1445x_mcu_scheduler_resetInstance(pMcuInstance);
			return 0;
  			//sc1445x_mcu_scheduler_changeCodec(pMcuInstance); 
		} 
	 printf("MCU MODIFY STEP 3 \n");

	}
 	return 0;
}

int sc1445x_mcu_instance_start_loop(sc1445x_mcu_instance_t *pMcuInstance,sc1445x_mcu_audio_media_stream_t *pMediaStream ) 
{
  	memcpy((char*)&pMcuInstance->mediaStream,(char*)pMediaStream,sizeof(sc1445x_mcu_audio_media_stream_t));
	// Create RTP connection
  	if (sc1445x_amb_sound_open(8,0,ALSA_MAX_BYTES*100)) {
 		DPRINT( "[%s]Failed to open ALSA Driver for loopback \n", __FUNCTION__);
 		return -1;
	} 
  	sc1445x_mcu_endpoint_initiate((int)pMediaStream->mediaParams.ptype, &pMcuInstance->audioFramesBuffer,(int)pMcuInstance->mediaStream.mediaParams.rxRate,(int)pMcuInstance->mediaStream.mediaParams.txRate);    
  	if (!sc1445x_mcu_audio_channel_activation(pMcuInstance))
	{
		pMcuInstance->state =(int) MCU_CALL_STATE_CONNECTED_RX_TX; //TBS to support rx only, tx only
 
	}else{
 		DPRINT( "[%s]Couldn't activate audio channel for loopback \n", __FUNCTION__);
   		return sc1445x_mcu_instance_destroy_loop(pMcuInstance);	
	}
 	if (sc1445x_mcu_scheduler_add_loop(pMcuInstance)!=0)
	{
		DPRINT( "[%s]Couldn't add new MCU Instance for loopback \n", __FUNCTION__);	 
		return sc1445x_mcu_instance_destroy_loop(pMcuInstance);	
	}
 	return 0;
}
int sc1445x_mcu_instance_destroy_loop(sc1445x_mcu_instance_t *pMcuInstance)
{
  return sc1445x_mcu_scheduler_del_loop(pMcuInstance);
}

int  sc1445x_mcu_audio_channel_activation(sc1445x_mcu_instance_t *pMcuInstance)
{
 	sc1445x_ae_codec_type codec_type;
	unsigned short channel ;
	int ret;
  
	//TO change //instead of ptype to use codecType to convert codecTypes
 
 	pMcuInstance->codecSampleSize   = sc1445x_media_find_samplesize( pMcuInstance->mediaStream.mediaParams.ptype) ;
	if (pMcuInstance->mediaStream.mediaAttr== MCU_MATTR_RLOOP_ID)
	{
  		pMcuInstance->alsaChannel =-1;
		return 0;
 	}

	codec_type = sc1445x_media_find_codec_type(pMcuInstance->mediaStream.mediaParams.ptype, pMcuInstance->mediaStream.mediaParams.vad );	 
	ret = sc1445x_amb_activate_channel(codec_type, codec_type, &channel,  pMcuInstance->mediaStream.mediaParams.fxsLine);
     	
   	if (ret<0) 	return -1;
   	pMcuInstance->alsaChannel = channel;   	
 
     return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Internal MCU Block Functions
int sc1445x_mcu_set_telephone_event_profile(int type)
{
	return 0;
}
void sc1445x_mcu_send_dtmf(const sc1445x_mcu_key dtmfkey, unsigned short line)
{
	int i;
	int found = 0;
	sc1445x_ae_std_tone dtmfTone;
	dtmfTone = convertDtmf(dtmfkey);
  	for (i=0;i<MAX_MCU_SUPPORTED_CALLS;i++)
	{
		if (mcuInstance[i].state & MCU_CALL_STATE_CONNECTED_TX)
		{
			found =1;
			if (mcuInstance[i].mediaStream.mediaParams.dtmfMode ==MCU_DTMF_INBAND_ID)
				sc1445x_amb_play_dtmf((sc1445x_ae_std_tone)dtmfTone, 1, line);
			else if (mcuInstance[i].mediaStream.mediaParams.dtmfMode==MCU_DTMF_2833_ID){
				sc1445x_amb_play_dtmf(dtmfTone, 0, line);
				// FIX - BROADWORKS
 		 		sc1445x_ortp_send_dtmf_event(mcuInstance[i].pRtpSession ,(char)dtmfkey ,MCU_DTMF_DURATION, MCU_DTMF_VOLUME, 
				mcuInstance[i].audioFramesBuffer.txTimestamp+(mcuInstance[i].audioFramesBuffer.txPacketization*80));
				mcuInstance[i].audioFramesBuffer.txTimestamp+=3*(mcuInstance[i].audioFramesBuffer.txPacketization*80); 

			}else sc1445x_amb_play_dtmf(dtmfTone, 0, line);
		}
	} 
	if (!found){
 		sc1445x_amb_play_dtmf(dtmfTone, 0, line);
	}
}
 	
 void sc1445x_mcu_send_dtmf_rfc2833(const sc1445x_mcu_key dtmfkey )
{
	int i;
  	for (i=0;i<MAX_MCU_SUPPORTED_CALLS;i++)
	{
		if (mcuInstance[i].state & MCU_CALL_STATE_CONNECTED_TX)
	 	 sc1445x_ortp_send_dtmf_event(mcuInstance[i].pRtpSession ,(char)dtmfkey ,MCU_DTMF_DURATION, MCU_DTMF_VOLUME, mcuInstance[i].audioFramesBuffer.txTimestamp );
	} 
}

void sc1445x_mcu_send_dtmf_inband(const sc1445x_mcu_key dtmfkey, unsigned short line)
{
	int i;
	sc1445x_ae_std_tone dtmfTone;
	dtmfTone = convertDtmf(dtmfkey);
	for (i=0;i<MAX_MCU_SUPPORTED_CALLS;i++)
	{
		if (mcuInstance[i].state & MCU_CALL_STATE_CONNECTED_TX)
		 	 sc1445x_amb_play_dtmf((sc1445x_ae_std_tone)dtmfTone, 1, line);
	} 	 
}
 
void sc1445x_mcu_endpoint_initiate(int ptype, audio_frames_buffer_t* pAmbBufferHandler, int rxPTime,int txPTime)
{
	pAmbBufferHandler->rxPacketizationTime  = rxPTime; 
	pAmbBufferHandler->txPacketizationTime  = txPTime; 

    pAmbBufferHandler->rxPacketization  = rxPTime/10; 
	pAmbBufferHandler->txPacketization  = txPTime/10; 
	
	if (pAmbBufferHandler->rxPacketization<1 || pAmbBufferHandler->rxPacketization>SC1445x_MAX_PTIME)
		pAmbBufferHandler->rxPacketization=2;

	if (pAmbBufferHandler->txPacketization<1 || pAmbBufferHandler->txPacketization>SC1445x_MAX_PTIME)
		pAmbBufferHandler->txPacketization=2;
   	
   	pAmbBufferHandler->txStoredSamples  = 0;
  	pAmbBufferHandler->rxAudioSamplePos  = 0;
	pAmbBufferHandler->txAudioSamplesSize  = 0;

 	pAmbBufferHandler->txAudioPacketPos = 0;
	pAmbBufferHandler->rxMorePackets  = 0;
	pAmbBufferHandler->rxTimestamp  = 0;
	pAmbBufferHandler->txTimestamp  = 0;//to check
	pAmbBufferHandler->txTicks  = 0; 
 
	if (ptype==98 || ptype==99){ 
	//	pAmbBufferHandler->codecType  =pAmbBufferHandler->rxPacketization ;  
 	//	pAmbBufferHandler->rxTicks = 0x80;  
 		pAmbBufferHandler->txPacketization=1;
  	}
	else{
		;
// 		pAmbBufferHandler->codecType = 0;  
//		pAmbBufferHandler->rxTicks  = 0x0;  
 	}
 }

///////////////////////////////////////////////////////////////////////////////////////////////////////  


///////////////////////////////////////////////////////////////////////////////////////////////////////  
sc1445x_ae_std_tone convertTone(const sc1445x_mcu_tone_bmp toneBMP)
{
	switch (toneBMP)
	{
  		case	MCU_TONE_DL_BT:
 			return SC1445x_AE_STD_TONE_DIAL;
		case	MCU_TONE_BZ_BT:
			return SC1445x_AE_STD_TONE_BUSY;
		case	MCU_TONE_CW_BT:
			return SC1445x_AE_STD_TONE_WAITING;
 		case	MCU_TONE_WR_BT:
  		case	MCU_TONE_DT_BT:
 		case	MCU_TONE_RG_BT: //ringing call progress tone
 		case	MCU_TONE_RB_BT:
 			return SC1445x_AE_STD_TONE_RINGING;
 		case	MCU_TONE_CF_BT:
 		case	MCU_TONE_DL2_BT:
		case	MCU_TONE_CNG_BT:
			return SC1445x_AE_STD_TONE_CONGESTION;
		case	MCU_TONE_DISCN1_BT:
			return SC1445x_AE_STD_TONE_DISCONNECT1;
		case	MCU_TONE_DISCN2_BT:
			return SC1445x_AE_STD_TONE_DISCONNECT2;
		default :
 				DPRINT( "[%s]Tone not supported\n", __FUNCTION__);	 

	}
	return SC1445x_AE_STD_TONE_INVALID;
}
sc1445x_ae_std_tone convertDtmf(const sc1445x_mcu_key dtmfkey)
{
	switch (dtmfkey)
	{
  		case MCU_KEY_0_ID:
 			return SC1445x_AE_STD_TONE_0;
 		case MCU_KEY_1_ID:
 			return SC1445x_AE_STD_TONE_1;
 		case MCU_KEY_2_ID:
 			return SC1445x_AE_STD_TONE_2; 
 		case MCU_KEY_3_ID:
			return SC1445x_AE_STD_TONE_3;
 		case MCU_KEY_4_ID:
			return SC1445x_AE_STD_TONE_4;
 		case MCU_KEY_5_ID:
			return SC1445x_AE_STD_TONE_5;
 		case MCU_KEY_6_ID:
			return SC1445x_AE_STD_TONE_6;
 		case MCU_KEY_7_ID:
			return SC1445x_AE_STD_TONE_7;
 		case MCU_KEY_8_ID:
			return SC1445x_AE_STD_TONE_8;
 		case MCU_KEY_9_ID:
			return SC1445x_AE_STD_TONE_9;
 		case MCU_KEY_A_ID:
			return SC1445x_AE_STD_TONE_A;
 		case MCU_KEY_B_ID:
			return SC1445x_AE_STD_TONE_B;
 		case MCU_KEY_C_ID:
			return SC1445x_AE_STD_TONE_C;
  		case MCU_KEY_D_ID:
			return SC1445x_AE_STD_TONE_D;
 		case MCU_KEY_STAR_ID:
			return SC1445x_AE_STD_TONE_STAR;
 		case MCU_KEY_POUND_ID:
			return SC1445x_AE_STD_TONE_HASH;
 		}
		DPRINT( "[%s]Invalid key \n", __FUNCTION__);	 
 		return SC1445x_AE_STD_TONE_INVALID;
 }

sc1445x_ae_iface_mode convertAudioIface(const sc1445x_mcu_audio_mode audioMode)
{
	switch (audioMode)
	{
	case MCU_AUD_HANDSET_ID: return SC1445x_AE_IFACE_MODE_HANDSET;
 	case MCU_AUD_HANDSFREE_ID: return SC1445x_AE_IFACE_MODE_HANDS_FREE;
	case MCU_AUD_HEADSET_ID: return SC1445x_AE_IFACE_MODE_HEADSET;
	case MCU_AUD_BLUETOOTH_ID:	return SC1445x_AE_IFACE_MODE_WIRELESS;
	case MCU_AUD_OPENLISTEN_ID: return SC1445x_AE_IFACE_MODE_OPEN_LISTENING;
	case MCU_AUD_DECTHEADSET_ID:	return SC1445x_AE_IFACE_MODE_WIRELESS;
	}
	DPRINT( "[%s]Invalid Audio Mode \n", __FUNCTION__);	 
  	return 	SC1445x_AE_IFACE_MODE_INVALID;
}

#include <net/if.h>

#include <sys/socket.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h> 

void sc1445x_mcu_getAddress(unsigned char* address)
{
int fd;
struct ifreq ifr;
struct sockaddr_in *sin;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd )
	{
		memset(&ifr, 0, sizeof(struct ifreq));
		ifr.ifr_addr.sa_family = AF_INET;
		strncpy(ifr.ifr_name, "eth0", sizeof(ifr.ifr_name));
 		if (ioctl(fd, SIOCGIFADDR, (char *)&ifr) < 0) 
		{
 			strcpy((char*)address, "0.0.0.0");
			close (fd);
 			return ;
		}
  	}else{
 		strcpy((char*)address, "0.0.0.0");
		return ;
	}
	sin = (struct sockaddr_in *)&ifr.ifr_addr;
 	snprintf((char*)address,16,"%s", inet_ntoa(sin->sin_addr));
	close (fd);
}
 
// gets interface addresses and displays them using getifaddrs 
static void print_addr(struct sockaddr *sa, unsigned char* host)
{
 int err;
  if (sa == NULL) {
  printf("\n");
  return;
 }
 
 switch (sa->sa_family) {
 case AF_INET6:
   err = getnameinfo(sa, sizeof(struct sockaddr_in6), (char *)host, 64, NULL, 0, NI_NUMERICHOST);
   if (err>0) {
		perror("getnameinfo");
	    exit(EXIT_FAILURE);
	}
  break;
 default:
 //case AF_INET:
   err = getnameinfo(sa, sizeof(struct sockaddr_in),
      (char *)host, 64, NULL, 0, NI_NUMERICHOST);
  if (err>0) {
   perror("getnameinfo");
   exit(EXIT_FAILURE);
  }
 
  printf("\n");
  return;
 }
  printf("[%s]\n", host);
}

#ifdef SIP_IPv6
void sc1445x_mcu_getAddress_v6(unsigned char* address)
{
	unsigned char host[64];
	struct ifaddrs *ifap0, *ifap = NULL;
	char buf[BUFSIZ];

	memset(buf, 0, sizeof(buf));
	if (getifaddrs(&ifap0)) {
 		goto fin;
	}

	for (ifap = ifap0; ifap; ifap=ifap->ifa_next) 
	{
		if (ifap->ifa_addr == NULL)
			continue;

		if( ifap->ifa_addr->sa_family == AF_INET6 && (strcmp(ifap->ifa_name, "eth0") == 0) )
		{
			printf("Interface: %s\n", ifap->ifa_name);      
			printf("\tAddress: ");
			print_addr(ifap->ifa_addr, address);
			printf("\tNetmask: ");
			print_addr(ifap->ifa_netmask, host);
			printf("\tBcast: ");
			print_addr(ifap->ifa_broadaddr, host);
			printf("------\n");
		}
	}   
fin:
 freeifaddrs(ifap);   
}
#endif

